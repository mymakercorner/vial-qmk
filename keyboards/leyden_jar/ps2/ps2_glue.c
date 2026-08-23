/*
 * ps2_glue.c - QMK / VIAL-QMK glue for the framework-agnostic PS/2 device driver
 * (ps2/hid_to_ps2.*). QMK-only: this file has no home in the standalone testbed
 * repo and is never touched by the sync script. See docs/QMK_Integration_Design.md
 * in the driver repo for the full rationale; section references below point there.
 *
 * Responsibilities:
 *   - Boot-time USB-vs-PS/2 mode latch, decided once (§3.3). PS/2 shares the
 *     solenoid connector pins (GP28 clock / GP29 data), so only one of haptic or
 *     PS/2 can own them; the choice is latched at boot from USB-enumeration state.
 *   - A custom host_driver_t so QMK pushes every *resolved* keyboard report to us
 *     (layers/macros/remaps already applied) instead of to USB (§3.1).
 *   - Protocol servicing on a dedicated ChibiOS thread at NORMALPRIO+1 (§3.4), so
 *     ps2_update never runs on the QMK main loop and the capsense scan keeps its
 *     period. Replaced the bounded blob-atomic drain of §3.2.
 *   - PS/2-host LEDs handed back to QMK's own led_task via keyboard_leds() (§6).
 *   - BAT (0xAA) re-announce until the host first talks to us (§7.5).
 *
 * The whole file is compiled only when PS2_DEVICE_ENABLE is set - a plain #define
 * in the variant's config.h (QMK force-includes every config.h via -include, so it
 * reaches this guard before any of our own includes). The matching SRC lines live
 * in that variant's rules.mk; there is deliberately no shared leyden_jar/rules.mk
 * gate, because QMK includes the shared rules.mk before the variant's and so could
 * not see a per-variant flag.
 */

#ifdef PS2_DEVICE_ENABLE

// PS/2 shares the single-controller haptic connector and matrix assumptions; it
// must never build on a split keyboard. Caught here at compile time (the old make
// $(error) guard lived in the shared rules.mk that no longer exists).
#    ifdef SPLIT_KEYBOARD
#        error "PS2_DEVICE_ENABLE is not supported on split keyboards"
#    endif

#    include <stdint.h>
#    include <string.h>

#    include <hal.h>          // ChibiOS: -> osal.h -> ch.h, i.e. chThdCreateStatic,
                               // chBSemWaitTimeout, chSysLock, TIME_US2I, NORMALPRIO.
                               // quantum.h reaches these transitively, but the PS/2
                               // thread (§3.4) depends on the kernel API directly, so
                               // say so - same as platforms/chibios/drivers/serial.c.
#    include "quantum.h"       // QMK core: hooks, timer_read32/elapsed32, led_t, report_*_t
#    include "host.h"          // host_set_driver, host_driver_t
#    include "io_expander.h"   // io_expander_is_ps2_present (boot-time mode select)
#    include "usb_device_state.h"  // struct usb_device_state (notify hook signature)
#    include "usb_util.h"      // usb_disconnect (kills the USB device in PS/2 mode)
#    include "haptic.h"        // haptic_disable
#    include "solenoid.h"      // solenoid_shutdown
#    include "hardware/pio.h"  // pio1, PIO
#    ifdef PS2_DEBUG_CONSOLE
#        include "hardware/gpio.h"  // gpio_get for the debug heartbeat's live line-level read
#    endif

#    include "hid_to_ps2.h"    // the portable driver's public surface

// Re-mux the two PS/2 pads back to the PIO after something else has grabbed one.
// Lives in ps2_platform_chibios.c (the file that owns pin muxing); declared here
// rather than in hid_to_ps2.h because both files are QMK-only and the portable
// header must stay byte-identical to the driver repo's copy.
void ps2_platform_reclaim_pins(PIO pio, uint data_pin, uint clock_pin);

#    ifdef PS2_DEBUG_CONSOLE
#        include "print.h"      // xprintf -> USB-CDC console (sendchar) on RP2040/ChibiOS
#        include "ps2_trace.h"  // ps2_trace_pop / ps2_trace_dropped for the debug drain
#        include "usb_main.h"   // USB_DRIVER (USBD1) + USB_ACTIVE for ps2_console_ready()
#    endif

// Free-running microsecond clock for ps2_update. We forward-declare the Pico SDK
// symbol rather than #include <hardware/timer.h>: once quantum.h / ChibiOS headers
// are in scope a TIMER macro is defined, and timer.h's invalid_params_if(TIMER, ...)
// then fails to preprocess under -Werror. Two more subtleties forced the exact form
// below:
//   - time_us_32() is a *static inline* in that header (returns timer_hw->timerawl),
//     so there is no linkable symbol - forward-declaring it gives an undefined ref.
//   - time_us_64() IS a real function (hardware_timer/timer.c, compiled via the
//     RP2040.mk PICOSDKSRC list, same as pio.c/clocks.c the matrix scanner links).
// So we call time_us_64() and truncate, exactly like the bare-Pico testbed main.cpp.
// The low 32 bits are the same µs counter; ps2_update does wrap-safe uint32 math.
uint64_t time_us_64(void);
static inline uint32_t ps2_now_us(void) {
    return (uint32_t)time_us_64();
}

// --- required board pins (no defaults; a PS/2 board must declare them) ---------
#    if !defined(PS2_CLOCK_PIN) || !defined(PS2_DATA_PIN)
#        error "PS2_DEVICE_ENABLE requires PS2_CLOCK_PIN and PS2_DATA_PIN (config.h)"
#    endif

// --- tunables: config.h may override; sane defaults otherwise (§5.1) -----------
// Sleep tiers for the PS/2 servicing thread (§3.4). These replace the §3.2 drain
// budget: the thread calls ps2_update once per wake and sleeps, so there is no
// busy-wait left to bound. PS2_DRAIN_BUDGET_US is deliberately gone.
//
// The two fast tiers exist for DIFFERENT reasons - do not merge them:
//
//   TX_POLL (dev.busy != 0, a frame on the wire, ~0.71 ms). Polling cannot make
//     progress here: ps2_update step 7 is gated on busy == 0. It is PROTOCOL
//     SAFETY. This is the only window in which ps2out's `irq wait` can fire, i.e.
//     the only window where the host can inhibit us mid-frame, stall the state
//     machine, and start a race we must answer before it releases Clock. Budget at
//     the spec floor is ~26-100 us (§3.4 "What happens when the host talks to us"),
//     which today's blob-atomic drain meets only by accident, by spinning.
//     RAISE if measurement shows the thread is starving the capsense scan; LOWER if
//     a host misses abort servicing. Must stay >= CH_CFG_ST_TIMEDELTA (20 us).
//
//   GAP_POLL (bus idle, bytes still queued). Nothing is on the wire, `sendloop` is
//     not executing, so NO abort is possible. Purely waiting out the driver's
//     100 us inter-byte gap so step 7 can fire. This is THROUGHPUT, not safety.
//
//   IDLE_MS. Bounded by the spec's 20 ms command-response deadline, 4x margin. A
//     keystroke never waits this long: the report callbacks signal the semaphore.
#    ifndef PS2_THREAD_TX_POLL_US
#        define PS2_THREAD_TX_POLL_US 50u    // frame on the wire (Case B window)
#    endif
#    ifndef PS2_THREAD_GAP_POLL_US
#        define PS2_THREAD_GAP_POLL_US 100u  // inter-byte gap, intra- or inter-blob
#    endif
#    ifndef PS2_THREAD_IDLE_MS
#        define PS2_THREAD_IDLE_MS 5u        // idle, vs the 20 ms response bound
#    endif
// Priority must be ABOVE the QMK main loop (the ChibiOS main thread, NORMALPRIO),
// or we would only run when it blocks - which reproduces the coupling §3.4 exists
// to remove. +1 is enough: nothing else in a non-split build sits between us and
// HIGHPRIO. Stack: ps2_update has no recursion and small locals; 1024 is the
// serial_protocol.c precedent for a protocol state machine, trim on measurement.
#    ifndef PS2_THREAD_PRIO
#        define PS2_THREAD_PRIO (NORMALPRIO + 1)
#    endif
#    ifndef PS2_THREAD_STACK
#        define PS2_THREAD_STACK 1024
#    endif
#    ifndef PS2_REANNOUNCE_MS
#        define PS2_REANNOUNCE_MS 1000u     // resend 0xAA cadence until host contact (§7.5)
#    endif

// PS/2 runs on PIO1 (PIO0 is fully owned by the matrix scanner). ps2out fills a
// PIO's instruction memory, so state machine 0 is the only tenant.
#    define PS2_PIO pio1
#    define PS2_SM  0u

// --- driver + latch state ------------------------------------------------------
static ps2_device s_ps2_dev;

// The latest fully-resolved pressed-set, rebuilt by the host_driver callbacks and
// consumed by ps2_update. 32-byte HID-usage bitmap (see hid_to_ps2.h).
static uint8_t s_ps2_latest_state[PS2_KEYBOARD_STATE_SIZE_BYTES];

// Set by the host_driver callbacks whenever they rewrite s_ps2_latest_state;
// consumed by the PS/2 thread. When clear we hand ps2_update NULL instead of the
// bitmap, which skips its 32-byte diff - ~704 of the function's ~810 cycles
// (~5.7 us of ~6.5 us at 125 MHz, measured from the -Os disassembly). That diff
// is unconditional and finds nothing when the report has not moved, and the
// thread re-enters ps2_update every 50 us while a frame is on the wire, so this
// is most of the cost of servicing PS/2 while typing.
//
// Correctness rests on two things, both easy to get wrong:
//   - the flag is cleared in the SAME critical section that takes the snapshot.
//     Clearing it afterwards would let a report arriving mid-diff be erased and
//     lost - a real dropped keystroke, not just a delayed one.
//   - it is NOT consumed until the driver has finished BAT, because ps2_update
//     ignores the bitmap entirely until then (its step 1 / step 2 are exclusive).
//     Without that guard a key held through the 500 ms BAT window never emits.
static bool s_ps2_state_dirty = false;

// Snapshot handed to ps2_update, so the driver never reads the live bitmap while
// the report callbacks are writing it. Owned by the PS/2 thread; only ever filled
// inside the chSysLock below.
static uint8_t s_ps2_state_snapshot[PS2_KEYBOARD_STATE_SIZE_BYTES];

// Wake signal for the PS/2 thread: raised by the report callbacks so a keystroke
// never waits out the idle tier. A BINARY SEMAPHORE specifically because it
// LATCHES - a signal raised while the thread is running (rather than parked) is
// remembered. QMK's own RP2040 PIO PS/2 host driver uses thread_reference_t +
// osalThreadResumeI instead, which is lighter but silently no-ops on a NULL
// reference; that is safe for its ISR-serving-a-parked-caller shape and would
// drop reports here. Statically initialised (taken) so it is valid before the
// thread or the host driver exist.
static BSEMAPHORE_DECL(s_ps2_wake, true);
static THD_WORKING_AREA(s_ps2_thread_wa, PS2_THREAD_STACK);
static THD_FUNCTION(ps2_thread, arg);   // defined below, created in ps2_enter_ps2_mode

typedef enum {
    PS2_MODE_UNDECIDED = 0,  // initial sentinel; overwritten by the boot-time decision
    PS2_MODE_HAPTIC,         // USB present -> normal USB keyboard + haptic; PS/2 never inits
    PS2_MODE_PS2,            // no USB -> haptic released, PS/2 running
} ps2_mode_t;

static ps2_mode_t s_ps2_mode          = PS2_MODE_UNDECIDED;  // decided once in keyboard_post_init_kb
#    ifdef PS2_DEBUG_CONSOLE
static uint8_t    s_ps2_detect_read   = 0xFFu;  // presence-pin read at boot (0xFF = never taken)
#    endif
static uint32_t   s_ps2_last_bat_ms   = 0;  // last 0xAA re-announce (§7.5)

#    ifdef PS2_DEBUG_CONSOLE
// --- "while you weren't looking" latches (PS2_DEBUG_CONSOLE only) ---------------
//
// The 1 Hz heartbeat in ps2_service_main is LIVE-only: it xprintf's
// unconditionally, and xprintf on an unenumerated USB silently discards, so every
// heartbeat emitted during a PS/2-only run is lost. That is precisely the run we
// need to observe - the "no keys on PS/2" fault only reproduces with USB absent,
// and plugging USB in to watch it makes it go away.
//
// So these accumulate from boot instead, and are dumped ONCE when the console
// first attaches (housekeeping_task_kb). Together they bisect the whole path from
// keyswitch to wire. Read the FIRST one that is zero - that is where the chain
// breaks:
//
//   hb      > 0  ->  the QMK main loop is running at all
//   press   > 0  ->  the capacitive matrix produced key events (keys are SEEN)
//   publish > 0  ->  resolved reports reached our host driver (glue is installed)
//   keysmax > 0  ->  at least one of those reports actually carried a held key
//   thread  > 0  ->  the PS/2 thread is being scheduled
//   state   > 0  ->  a snapshot was handed to ps2_update; this is separately
//                    interesting because it is gated on ps2_startup_complete(), so
//                    publish>0 with state==0 means BAT never finished and the
//                    driver was discarding every keystroke on purpose
//   TX lines in the trace that follows  ->  bytes actually reached the PIO
//   contacted = 1                       ->  the host answered us
//
// Written from two threads (press/publish on the QMK main thread, thread/state on
// the PS/2 thread) and read from a debug print. Torn reads are possible and
// harmless - these are counters, not invariants. Do not build logic on them.
static volatile uint32_t s_dbg_hb_ticks  = 0;  // 1 Hz heartbeat iterations since boot
static volatile uint32_t s_dbg_press_n   = 0;  // key presses seen by pre_process_record_kb
static volatile uint32_t s_dbg_publish_n = 0;  // ps2_publish_state() calls
static volatile uint32_t s_dbg_thread_n  = 0;  // ps2_thread loop iterations
static volatile uint32_t s_dbg_state_n   = 0;  // non-NULL snapshots handed to ps2_update
static volatile uint8_t  s_dbg_keys_max  = 0;  // most usage bits ever set in one published report
static volatile uint8_t  s_dbg_usage_1st = 0;  // first non-zero HID usage published (which key it was)

// Chain bisection, rev 8. press/proc/publish are three consecutive points on the
// path from keyswitch to wire, so the first one that is zero names the broken link:
//   press=0            -> the matrix produced nothing (fault is upstream of PS/2)
//   press>0, proc=0    -> dropped between pre_process_record_kb and
//                         process_record_kb, i.e. inside pre_process_record_quantum
//   proc>0, publish=0  -> the keycode registered no HID report at all: an unmapped
//                         / transparent / layer entry in the Vial keymap, or a
//                         later stage of process_record_quantum consuming it
// The two timestamps exist because "did this happen before or after USB was plugged
// in?" decided between two live theories and could not be answered from counters
// alone - a cumulative counter cannot say WHEN. kc1 is the QMK keycode (not the HID
// usage) of the very first press, which distinguishes a real key from a layer/KC_NO.
static volatile uint32_t s_dbg_proc_n      = 0;  // key presses reaching process_record_kb
static volatile uint32_t s_dbg_t_usb_ms    = 0;  // uptime at which USB first went ACTIVE
static volatile uint32_t s_dbg_t_press1_ms = 0;  // uptime of the first key press
static volatile uint16_t s_dbg_kc1         = 0;  // QMK keycode of that first press
// Uptime of the FIRST heartbeat tick, i.e. the first moment housekeeping_task_kb was
// reached. This is the one number that separates "the capsense matrix saw nothing"
// from "the QMK main loop never ran", which look identical in press= (that counter
// increments from keyboard_task(), so a parked loop reports zero presses exactly like
// a dead matrix). A cumulative tick COUNT cannot answer it - only the first timestamp
// can. If this lands near the moment USB went ACTIVE (usb@), the loop was parked in
// protocol_pre_task's `while (USB_DRIVER.state == USB_SUSPENDED)` until the cable
// arrived; if it lands near zero, the loop ran from boot and the matrix is the
// suspect. Trustworthy despite the xprintf-blocking caveat on hb=, because before USB
// is ACTIVE usb_endpoint_in_send returns immediately instead of blocking.
static volatile uint32_t s_dbg_t_hb1_ms    = 0;  // uptime of the first heartbeat tick

// DIRECT observation of the USB_SUSPENDED parking condition, sampled from the PS/2
// thread. This is the trick that makes the theory testable at all: when
// protocol_pre_task parks the QMK main loop in
//   while (USB_DRIVER.state == USB_SUSPENDED) { suspend_power_down(); }
// nothing on the main thread can report it - housekeeping, the heartbeat and every
// counter above are frozen precisely while the interesting thing is happening. Our
// PS/2 thread is a separate ChibiOS thread at NORMALPRIO+1 and keeps running, so it
// can watch. susp/thread is the fraction of the run spent parked; susp1@ is when it
// first happened. If susp1@ is near zero and susp/thread is close to 1 up until the
// cable goes in, the main loop was parked for the whole PS/2-only phase and that is
// the root cause - no inference from hb= required.
static volatile uint32_t s_dbg_susp_n      = 0;  // thread samples seeing USB_SUSPENDED
static volatile uint32_t s_dbg_t_susp1_ms  = 0;  // uptime of the first such sample

// Frozen, never-consumed copy of the first trace events the drain ever pops.
//
// ps2_trace_pop() CONSUMES. The drain starts the instant USB goes ACTIVE, which is
// milliseconds after boot when the cable is already plugged - but QMK Toolbox
// attaches its listener seconds later, and anything xprintf'd in between is dropped
// on the floor by the host, not buffered. So the boot burst was being popped,
// printed into nowhere, and destroyed. An empty trace window then reads exactly like
// "the driver never transmitted", which is how the 2026-08-23 session lost an
// evening. This copy survives so the burst can be re-printed for as long as the
// board is up. 32 events is enough for the BAT + the host's init dialogue.
#        define PS2_DBG_BURST_MAX 32
static ps2_trace_evt s_dbg_burst[PS2_DBG_BURST_MAX];
static uint8_t       s_dbg_burst_n = 0;
#    endif

// --- host_driver_t callbacks (§3.1) -------------------------------------------
//
// These fire on the QMK MAIN THREAD (from the report pipeline) whenever the
// resolved report changes. They only *capture* the pressed set and wake the PS/2
// thread - cheap, no PIO work; the actual wire traffic + edge detection +
// typematic happen in ps2_update, on the PS/2 thread (§3.4).

// Hand a freshly built pressed-set to the PS/2 thread. Both callbacks below build
// into a LOCAL first and publish through here, rather than editing
// s_ps2_latest_state in place: they work by clear-then-set-bits, so an in-place
// build is briefly all-zeros, and the thread reading at that instant would see
// every held key released and emit a burst of spurious breaks.
//
// The copy and the dirty flag are one critical section - see s_ps2_state_dirty.
// chSysLock on ARMv6-M masks all interrupts (no BASEPRI), so this is ~0.5 us of
// latency added to the system tick and USB; a 32-byte memcpy is worth that.
// chBSemSignal is deliberately OUTSIDE the lock: it is the normal-context API,
// not the I-class one.
static void ps2_publish_state(const uint8_t *next) {
#    ifdef PS2_DEBUG_CONSOLE
    // Counted BEFORE the copy and deliberately OUTSIDE the chSysLock below: this is
    // a 32-byte popcount and that lock masks all interrupts on ARMv6-M. `next` is
    // the caller's freshly built local, so it is stable here.
    {
        uint8_t keys = 0;
        for (uint8_t i = 0; i < PS2_KEYBOARD_STATE_SIZE_BYTES; i++) {
            for (uint8_t bit = 0; bit < 8; bit++) {
                if (next[i] & (uint8_t)(1u << bit)) {
                    keys++;
                    if (s_dbg_usage_1st == 0) {
                        // Bit layout matches ps2_key_state_set(): usage = (index << 3) | bit.
                        s_dbg_usage_1st = (uint8_t)(((unsigned)i << 3) | bit);
                    }
                }
            }
        }
        if (keys > s_dbg_keys_max) {
            s_dbg_keys_max = keys;
        }
        s_dbg_publish_n++;
    }
#    endif

    chSysLock();
    memcpy(s_ps2_latest_state, next, sizeof(s_ps2_latest_state));
    s_ps2_state_dirty = true;
    chSysUnlock();

    chBSemSignal(&s_ps2_wake);
}

// 6KRO: report is mods (bitfield) + up to 6 held usage codes.
static void ps2_send_6kro(report_keyboard_t *report) {
    uint8_t next[PS2_KEYBOARD_STATE_SIZE_BYTES];

    ps2_key_state_clear(next);
    for (uint8_t i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
        uint8_t code = report->keys[i];
        if (code) {
            ps2_key_state_set(next, code);
        }
    }
    ps2_key_state_set_mods(next, report->mods);
    ps2_publish_state(next);
}

// NKRO: report->bits[] is already a bit-per-HID-usage bitmap, identical layout to
// our own - a straight 30-byte copy, then fold in the separate modifier byte.
static void ps2_send_nkro(report_nkro_t *report) {
    uint8_t next[PS2_KEYBOARD_STATE_SIZE_BYTES];

    ps2_key_state_clear(next);
    memcpy(next, report->bits, NKRO_REPORT_BITS);
    ps2_key_state_set_mods(next, report->mods);
    ps2_publish_state(next);
}

// A PS/2 keyboard has no pointer / consumer / system output: swallow these.
static void ps2_send_mouse(report_mouse_t *report) {
    (void)report;
}
static void ps2_send_extra(report_extra_t *report) {
    (void)report;
}

// Hand the PS/2 host's LED state back to QMK's led_task, which runs the same
// led_update_kb() (leds.c) as in USB mode (§6). The driver stashes the host's
// 0xED payload in dev.leds (bit0 Scroll, bit1 Num, bit2 Caps); permute into
// led_t order (bit0 Num, bit1 Caps, bit2 Scroll).
static uint8_t ps2_kb_leds(void) {
    led_t out    = {0};
    out.num_lock    = (s_ps2_dev.leds >> 1) & 1u;
    out.caps_lock   = (s_ps2_dev.leds >> 2) & 1u;
    out.scroll_lock = (s_ps2_dev.leds >> 0) & 1u;
    return out.raw;
}

static host_driver_t s_ps2_host_driver = {
    .keyboard_leds = ps2_kb_leds,
    .send_keyboard = ps2_send_6kro,
    .send_nkro     = ps2_send_nkro,
    .send_mouse    = ps2_send_mouse,
    .send_extra    = ps2_send_extra,
};

// --- mode entry (§3.3) ---------------------------------------------------------
//
// Switch the board from USB/haptic to PS/2 device mode. Called once, from
// keyboard_post_init_kb (see the boot-order note there). Releases the shared
// GP28/GP29 pins from the haptic subsystem and hands them plus the resolved-report
// stream to the PS/2 driver.
static void ps2_enter_ps2_mode(void) {
    // Quiesce the haptic subsystem *before* remuxing the two shared pins to PIO1.
    // Neither call actually releases a pin - QMK has no pin-release API, and both
    // only do SIO writes - but they stop the haptic/solenoid tasks from driving
    // GP28/GP29 and leave them at a safe level; the real hand-off is the
    // palSetLineMode inside ps2_platform_init below. haptic_disable() is
    // runtime-only (not persisted to EEPROM), so USB-mode haptic still works on a
    // later boot. What this does NOT cover is a *re-mux* by
    // haptic_notify_usb_device_state_change() after we take the pins - see
    // notify_usb_device_state_change_kb() near the bottom of this file.
    haptic_disable();
    solenoid_shutdown();

    // ps2_initialize -> ps2_platform_init remuxes GP28/GP29 (SIO -> PIO1) and starts
    // the ps2out state machine. rx handler is NULL: the driver stashes host LED
    // state in dev.leds itself, which ps2_kb_leds() reads directly.
    ps2_initialize(&s_ps2_dev, PS2_PIO, PS2_SM, PS2_DATA_PIN, PS2_CLOCK_PIN, NULL);

    s_ps2_last_bat_ms = timer_read32();

    // Start PS/2 servicing on its own thread (§3.4). Created here, and ONLY here,
    // so it never exists in haptic mode. Ordering matters twice over:
    //   - after ps2_initialize, or the thread would poll a PIO that is not running;
    //   - before host_set_driver, so the report callbacks cannot signal / dirty the
    //     state before there is anything to consume it. (The semaphore itself is
    //     statically initialised, so an early signal would be harmless anyway - but
    //     relying on that would be relying on an accident.)
    chThdCreateStatic(s_ps2_thread_wa, sizeof(s_ps2_thread_wa),
                      PS2_THREAD_PRIO, ps2_thread, NULL);

    // Ask QMK to push resolved reports to us instead of USB. NOTE: this initial
    // install is clobbered moments later by protocol_post_init()'s
    // host_set_driver(&chibios_driver) (see the boot-order note in
    // ps2_service_main), so ps2_service_main re-asserts it every loop. Kept here so
    // the intent is explicit and the driver is installed as early as possible.
    host_set_driver(&s_ps2_host_driver);

    s_ps2_mode = PS2_MODE_PS2;

    // SHUT THE USB DEVICE DOWN. LOAD-BEARING, not tidiness: without it a PS/2-only
    // board emits no keys at all. Root-caused 2026-08-23.
    //
    // tmk_core/protocol/chibios/chibios.c protocol_pre_task() runs at the top of
    // every main-loop iteration and does:
    //
    //     if (USB_DRIVER.state == USB_SUSPENDED) {
    //         while (USB_DRIVER.state == USB_SUSPENDED) { suspend_power_down(); ... }
    //     }
    //
    // With no USB cable there is no bus, so the RP2040 device controller signals
    // suspend almost at once (measured: 135 ms after boot) and QMK parks its ENTIRE
    // main loop there - no matrix scan, no keyboard_task, no reports. Meanwhile the
    // PS/2 thread created above is a separate ChibiOS thread and keeps running, so
    // the host sees a perfectly healthy keyboard (BAT answered, set 2, scanning,
    // commands acknowledged) that simply never types. That split behaviour is what
    // made this so hard to find - and it is why it never reproduced on the bench:
    // attaching debug USB to observe it prevents the suspend outright.
    //
    // usbDisconnectBus + usbStop leave the driver in USB_STOP, never USB_SUSPENDED,
    // so the parking loop's condition cannot be true. This is exactly what QMK's own
    // split keyboards do for the half with no host (split_util.c
    // is_keyboard_master_impl, comment: "Avoid NO_USB_STARTUP_CHECK - Disable
    // USB..."). Note split pairs its 2 s usb_connected_state() poll WITH this call;
    // this file's pre-2026-08-18 latch did the same poll and omitted the disconnect,
    // which is exactly why that one did not work either - verified on hardware at
    // commit 9a342c8390, which fails identically.
    //
    // Chosen over `#define NO_USB_STARTUP_CHECK` (what QMK auto-applies for
    // BLUETOOTH_ENABLE, the other "output is not USB" case) because that flag is
    // compile-time and would also kill remote wakeup in HAPTIC mode, where USB *is*
    // the output and waking a sleeping host by keypress must keep working. This call
    // is runtime and mode-scoped: haptic mode never reaches this line.
    //
    // CONSEQUENCE, deliberate: in PS/2 mode there is no USB device at all - no HID
    // console for xprintf, no VIA/Vial. TO DEBUG PS/2 MODE, TEMPORARILY COMMENT OUT
    // THE usb_disconnect() CALL BELOW and rebuild. It is deliberately NOT wrapped in
    // `#ifdef PS2_DEBUG_CONSOLE`: that would make the debug build stop exercising the
    // production path, which is the same trap that hid this bug for a month and that
    // f77/config.h already warns about for PS2_FORCE_ENABLE. Commenting out a line is
    // visible and deliberate; a silent build-flavour divergence is not.
    usb_disconnect();

#    ifdef PS2_DEBUG_CONSOLE
    // Only reaches a console if the usb_disconnect() above is commented out.
    xprintf("\n[ps2] entered PS/2 mode. clk=GP%u data=GP%u\n",
            (unsigned)PS2_CLOCK_PIN, (unsigned)PS2_DATA_PIN);
#    endif
}

// --- debug trace readout (PS2_DEBUG_CONSOLE only) ------------------------------
//
// Pop the driver's byte-level trace ring buffer and print it over the USB-CDC
// console (xprintf -> sendchar). Kept in the glue so ps2_trace.c stays byte-
// identical to the standalone testbed (its own ps2_trace_drain_printf uses stdio
// printf, which ChibiOS/newlib does not route to the console). Throttled and
// only called at blob boundaries, so it never perturbs a live PS/2 transaction.
#    ifdef PS2_DEBUG_CONSOLE
// "Will an xprintf from here actually reach the console?" - the ONLY correct
// predicate for that, and not the obvious one.
//
// sendchar -> send_report_buffered -> usb_endpoint_in_send, which returns false
// immediately unless `usbGetDriverStateI(usbp) == USB_ACTIVE` (usb_driver.c). It
// never looks at usb_device_state. So USB_ACTIVE is the whole condition.
//
// This originally tested usb_device_state_get_configure_state() == CONFIGURED,
// which cost a bench session on 2026-08-23: the heartbeat printed happily (it is
// ungated) while the boot latch and the entire trace drain stayed silent, and an
// empty trace ring reads exactly like "the driver never put a byte on the wire".
// That field is a SEPARATE bookkeeping variable maintained by usb_event_queue_task
// draining an event queue: set_suspend() writes SUSPEND(3), set_reset() writes
// INIT(1), and only a matching WAKEUP/CONFIGURED event writes CONFIGURED(2) back.
// Miss or reorder one queued event and it sticks at the wrong value forever, with
// a perfectly working console as the only evidence to the contrary. Do not gate
// console output on it. The heartbeat prints both values (usb= and cfg=) so the
// divergence stays visible.
static inline bool ps2_console_ready(void) {
    return USB_DRIVER.state == USB_ACTIVE;
}

// Build stamp, printed in every heartbeat as rev=. BUMP THIS on any change to the
// debug instrumentation. It exists because "no output" and "old binary still on the
// board" are indistinguishable from a pasted console line, and we burned several
// bench rounds on exactly that confusion. rev is the only field that proves which
// binary is actually running.
//   6 = usb=/cfg= added to the heartbeat, ps2_console_ready() gate
//   7 = latch reprints every 5s (was one-shot), frozen boot burst replay
//   8 = chain line: proc= counter, usb@/press1@ timestamps, kc1=
//   9 = boot burst printed at most twice (xprintf blocking was stalling the loop)
//  10 = hb1@ timestamp: separates "matrix saw nothing" from "main loop never ran"
//  11 = park line: PS/2 thread samples USB_SUSPENDED directly (main loop cannot)
//  12 = NO_USB_STARTUP_CHECK added in f77/config.h (superseded by 13)
//  13 = usb_disconnect() in ps2_enter_ps2_mode instead, split-keyboard style;
//       NOTE no console in PS/2 mode now - comment that call out to debug
#        define PS2_DBG_REV 13

// Shared by the live drain and the frozen boot-burst replay.
static const char *ps2_trace_tag_name(uint8_t tag) {
    switch (tag) {
        case PS2_TR_TX:      return "TX  ";
        case PS2_TR_RX:      return "RX  ";
        case PS2_TR_RX_PERR: return "RX! ";
        case PS2_TR_RESEND:  return "RSND";
        case PS2_TR_BAT:     return "BAT ";
        case PS2_TR_NOTE:    return "NOTE";
        default:             return "??? ";
    }
}

// Re-print the frozen opening events. Safe to call repeatedly - it consumes nothing.
//
// THROTTLED HARD, and not for tidiness: xprintf -> send_report_buffered passes
// TIME_MS2I(100) as its timeout, so a single line can block the calling thread for
// up to 100 ms when the console endpoint backs up. Dumping ~16 lines every 5 s from
// housekeeping was stalling the QMK main loop badly enough to corrupt the very
// measurement it was there to provide: hb fell ~38% behind uptime and looked exactly
// like the USB_SUSPENDED parking theory we were trying to test. Print the burst a
// couple of times so a late-attaching console still catches it, then stop.
static void ps2_debug_print_burst(void) {
    static uint8_t prints = 0;
    if (prints >= 2) {
        return;
    }
    prints++;
    xprintf("[ps2] boot burst: %u frozen event(s)\n", (unsigned)s_dbg_burst_n);
    for (uint8_t i = 0; i < s_dbg_burst_n; i++) {
        xprintf("[ps2]   [%lu] %s %02X\n",
                (unsigned long)s_dbg_burst[i].t_us,
                ps2_trace_tag_name(s_dbg_burst[i].tag),
                (unsigned)s_dbg_burst[i].a);
    }
}

static void ps2_debug_drain_trace(void) {
    static uint32_t last_ms   = 0;
    static uint32_t prev_us   = 0;
    static bool     have_prev = false;
    static uint32_t seen_drop = 0;

    // HOLD the ring until the console can actually receive it. ps2_trace_pop()
    // CONSUMES an event, while xprintf on an unconfigured USB silently discards the
    // characters - so draining before enumeration destroys the history instead of
    // printing it. Gating here is what makes the only reproduction that matters
    // observable: boot the board on PS/2 power ALONE (no USB, so the board and the
    // host power up together, as they do in production), let the race happen, and
    // only then plug USB in - the boot burst is still sitting in the ring and dumps
    // on the first drain after enumeration.
    //
    // The ring is 128 events and drops the OLDEST when full (ps2_trace.c), so the
    // burst we want is what gets sacrificed if traffic keeps coming. Plug USB in
    // promptly after a failing boot and do not type in the meantime; check the
    // "event(s) dropped total" line below before trusting a capture.
    if (!ps2_console_ready()) {
        return;
    }

    if (timer_elapsed32(last_ms) < 250u) {
        return;  // rate-limit console traffic to ~4 Hz
    }
    last_ms = timer_read32();

    ps2_trace_evt e;
    while (ps2_trace_pop(&e)) {
        int32_t dt = have_prev ? (int32_t)(e.t_us - prev_us) : 0;
        prev_us    = e.t_us;
        have_prev  = true;

        // Freeze the opening events before printing them - see s_dbg_burst. The pop
        // above has already removed this event from the ring, so if the console is
        // not being listened to yet this copy is the only record that survives.
        if (s_dbg_burst_n < PS2_DBG_BURST_MAX) {
            s_dbg_burst[s_dbg_burst_n++] = e;
        }

        xprintf("[%lu +%ld] %s %02X\n",
                (unsigned long)e.t_us, (long)dt, ps2_trace_tag_name(e.tag), (unsigned)e.a);
    }

    uint32_t dropped = ps2_trace_dropped();
    if (dropped != seen_drop) {
        xprintf("[trace] %lu event(s) dropped total\n", (unsigned long)dropped);
        seen_drop = dropped;
    }
}
#    endif  // PS2_DEBUG_CONSOLE

// --- main-thread housekeeping (§3.4) ------------------------------------------
//
// Everything here concerns QMK's own plumbing, not the PS/2 wire, so it stays on
// the QMK main thread. The protocol half lives in ps2_service_protocol() and is
// called ONLY from the PS/2 thread - see the single-owner note there.
static void ps2_service_main(void) {
    // Re-assert our host driver if anything has replaced it. This is REQUIRED, not
    // defensive: QMK's boot order is protocol_pre_init -> keyboard_init ->
    // protocol_post_init (quantum/main.c). Our latch runs inside keyboard_init
    // (keyboard_post_init_kb) and calls host_set_driver(&s_ps2_host_driver), but
    // protocol_post_init() then runs and unconditionally does
    // host_set_driver(&chibios_driver) - clobbering ours, so every resolved report
    // would go to USB and never reach ps2_send_6kro/nkro (=> no keystrokes on the
    // PS/2 wire, even though BAT/host-command replies still work via ps2_update).
    // housekeeping runs in the main loop, after protocol_post_init, so re-installing
    // here wins the race and also survives any later re-install (e.g. USB resume).
    if (host_get_driver() != &s_ps2_host_driver) {
        host_set_driver(&s_ps2_host_driver);
    }

#    ifdef PS2_DEBUG_CONSOLE
    // Unconditional ~1 Hz heartbeat: proves the HID console transport works even
    // when there is zero PS/2 traffic, and dumps the state we need to bisect the
    // "no keys" fault. keys = number of usage bits currently set in the resolved
    // report we've been handed (press a key on the F77 and this should rise); drv =
    // 1 iff our PS/2 host driver is the installed one (0 => still clobbered).
    //
    // Reads s_ps2_dev unlocked while the PS/2 thread mutates it, so startup/busy/
    // sent/qpend here are a torn sample, not a consistent snapshot - fine for a
    // 1 Hz debug print, but do not reason about invariants across these fields.
    // s_ps2_latest_state is safe: this runs on the main thread, which is its writer.
    static uint32_t s_hb_ms = 0;
    if (timer_elapsed32(s_hb_ms) >= 1000u) {
        s_hb_ms = timer_read32();
        if (s_dbg_hb_ticks == 0) {
            s_dbg_t_hb1_ms = s_hb_ms;
        }
        s_dbg_hb_ticks++;
        uint8_t keys = 0;
        for (uint8_t i = 0; i < PS2_KEYBOARD_STATE_SIZE_BYTES; i++) {
            uint8_t b = s_ps2_latest_state[i];
            while (b) { keys += (uint8_t)(b & 1u); b >>= 1; }
        }
        // Live wire levels: PS/2 idle is BOTH lines high (clk=1 dat=1). If clk=0 the
        // ps2out PIO's `wait 1 gpio <clock>` never releases, so no frame can start and
        // qpend stays stuck at 1 - i.e. a missing pull-up / held-low / miswired clock.
        // busy: bit0 = PIO active or host-inhibit (clock low), bit1 = byte-in-flight guard.
        // Pico SDK gpio_get (reads sio_hw->gpio_in) rather than ChibiOS palReadLine:
        // the latter expands to SIO->GPIO_IN, and the Pico SDK header #defines
        // GPIO_IN 0, so the PAL macro fails to compile in this mixed TU. The pad
        // input reaches the SIO even while the pin is muxed to PIO, so this is the
        // true line level. PS2_CLOCK_PIN/PS2_DATA_PIN (GPnn) are plain pad numbers.
        unsigned clk = (unsigned)gpio_get(PS2_CLOCK_PIN);
        unsigned dat = (unsigned)gpio_get(PS2_DATA_PIN);
        // usb / cfg are the two USB state variables that must NOT be conflated - see
        // ps2_console_ready(). usb is ChibiOS's real driver state (usbstate_t:
        // 0=UNINIT 1=STOP 2=READY 3=SELECTED 4=ACTIVE 5=SUSPENDED - only ACTIVE
        // delivers anything); cfg is QMK's separate usb_device_state bookkeeping
        // (usb_configure_state_t: 1=INIT 2=CONFIGURED 3=SUSPEND). Seeing a line at all
        // already proves usb==4, so cfg!=2 here means that field has gone stale and
        // anything else gated on it is silently dead.
        xprintf("[ps2] hb rev=%u startup=%u set=%u scan=%u contacted=%u keys=%u drv=%u qpend=%u busy=%u sent=%u clk=%u dat=%u usb=%u cfg=%u\n",
                (unsigned)PS2_DBG_REV,
                (unsigned)s_ps2_dev.startup, (unsigned)s_ps2_dev.current_set,
                (unsigned)s_ps2_dev.scanning_enabled, (unsigned)s_ps2_dev.host_contacted,
                (unsigned)keys, (unsigned)(host_get_driver() == &s_ps2_host_driver),
                (unsigned)ps2_tx_pending(&s_ps2_dev),
                (unsigned)s_ps2_dev.busy, (unsigned)s_ps2_dev.sent, clk, dat,
                (unsigned)USB_DRIVER.state,
                (unsigned)usb_device_state_get_configure_state());
    }

    // Report the first host contact once, then stream the byte trace.
    //
    // NOTE these ran at a blob boundary when servicing was inline; they now run on
    // the main thread, concurrently with the PS/2 thread, so a trace dump can land
    // mid-sequence. That is acceptable - the trace ring is only read here, and the
    // whole block is PS2_DEBUG_CONSOLE-only. It is deliberately NOT moved into the
    // PS/2 thread: xprintf would put a printf-sized stack requirement on a thread
    // sized for ps2_update, and console I/O has no business in a 50 us poll.
    static bool announced_contact = false;
    if (!announced_contact && s_ps2_dev.host_contacted) {
        announced_contact = true;
        xprintf("[ps2] host_contacted=1 (host is talking to us)\n");
    }
    ps2_debug_drain_trace();
#    endif
}

// --- PS/2 protocol servicing - PS/2 THREAD ONLY (§3.4) ------------------------
//
// SINGLE OWNER: ps2_update is a state machine with no internal locking, so this
// must never be called from anywhere but ps2_thread(). housekeeping_task_kb no
// longer touches it. The BAT re-announce lives here rather than on the main thread
// for the same reason - ps2_announce_bat pushes onto dev->queue, which ps2_update
// pops, and ps2_scancode_queue.h documents that single-producer/single-consumer
// assumption explicitly. Keeping both on this side preserves it instead of
// quietly breaking it.
//
// Exactly ONE ps2_update per call. The old §3.2 blob-atomic drain loop is gone:
// its job was to flush a multi-byte key at the wire floor despite a multi-ms main
// loop, and the sleep tiers now do that without blocking anything. ps2_update
// emits at most one byte and then waits out the ~0.71 ms frame; the thread simply
// comes back when that frame is due to be over.
static void ps2_service_protocol(void) {
    // Re-announce BAT-complete until the host first talks to us. Our first 0xAA
    // lands seconds after power-on (boot + calibration + BAT delay), far past the
    // spec window, so a single announce can be missed; repeat until host_contacted.
    if (!s_ps2_dev.host_contacted && timer_elapsed32(s_ps2_last_bat_ms) >= PS2_REANNOUNCE_MS) {
        ps2_announce_bat(&s_ps2_dev);
        s_ps2_last_bat_ms = timer_read32();
    }

    // Take the changed-latch and the snapshot in ONE critical section, so a report
    // landing between the two cannot be erased (see s_ps2_state_dirty). Held off
    // until BAT is done, because ps2_update discards the bitmap until then.
    const uint8_t *state = NULL;
    chSysLock();
    if (s_ps2_state_dirty && ps2_startup_complete(&s_ps2_dev)) {
        memcpy(s_ps2_state_snapshot, s_ps2_latest_state, sizeof(s_ps2_state_snapshot));
        s_ps2_state_dirty = false;
        state             = s_ps2_state_snapshot;
    }
    chSysUnlock();

#    ifdef PS2_DEBUG_CONSOLE
    if (state != NULL) {
        s_dbg_state_n++;
    }
#    endif

    ps2_update(&s_ps2_dev, ps2_now_us(), state);
}

// How long the thread may sleep before it must look at the bus again. See the
// PS2_THREAD_*_US block near the top for why the two fast tiers differ - the
// short one is protocol safety, the longer one is throughput.
static sysinterval_t ps2_thread_delay(void) {
    // A frame is on the wire (or the host is holding clock low - dev.busy bit0 is
    // also raised at receivecheck). Step 7 is gated on busy == 0 so we cannot make
    // progress; we are here to catch a host inhibit before it releases Clock.
    if (s_ps2_dev.busy != 0) {
        return TIME_US2I(PS2_THREAD_TX_POLL_US);
    }
    // Bus idle with bytes still to go: waiting out the driver's inter-byte gap.
    // Deliberately NOT ps2_tx_in_blob - that stays true across a blob's internal
    // gaps too, so it would pull those into the 50 us tier for no reason, and
    // ps2_tx_pending already covers everything it would catch.
    if (ps2_tx_pending(&s_ps2_dev)) {
        return TIME_US2I(PS2_THREAD_GAP_POLL_US);
    }
    return TIME_MS2I(PS2_THREAD_IDLE_MS);
}

// The thread. Never returns: CH_CFG_USE_WAITEXIT is FALSE in this ChibiOS config,
// so there is no chThdWait() and nothing may join it.
static THD_FUNCTION(ps2_thread, arg) {
    (void)arg;
    // No-op here - CH_CFG_USE_REGISTRY is FALSE, so this compiles to (void)name
    // (chregistry.h). Kept because it is the tree's idiom and costs nothing; do
    // not expect to see the name in a debugger.
    chRegSetThreadName("ps2");

    while (true) {
#    ifdef PS2_DEBUG_CONSOLE
        s_dbg_thread_n++;
        // Sampled here, not on the main thread, because the main thread is exactly
        // what stops running when this condition is true. See s_dbg_susp_n.
        if (USB_DRIVER.state == USB_SUSPENDED) {
            if (s_dbg_susp_n == 0) {
                s_dbg_t_susp1_ms = timer_read32();
            }
            s_dbg_susp_n++;
        }
#    endif
        ps2_service_protocol();
        // Wakes on whichever comes first: the deadline, or a report change
        // signalled from ps2_publish_state(). The timeout is the safety net - if
        // the signal path ever breaks, PS/2 goes slow rather than silent.
        chBSemWaitTimeout(&s_ps2_wake, ps2_thread_delay());
    }
}

// --- QMK hooks -----------------------------------------------------------------
// The USB-vs-PS/2 mode is decided ONCE here, not polled per loop. This hook is the
// last thing keyboard_init runs (keyboard_post_init_quantum, keyboard.c ~548), hence
// *after* haptic_init (keyboard.c ~541): the shared GP28/GP29 pins are already owned
// by haptic, so it is safe to take them for PS/2. Deciding any earlier (e.g. in
// matrix_init_custom, keyboard.c ~475) would be undone when haptic_init later re-grabs
// those pins. The IO expander is ready by then either way - io_expander_init() runs
// from leyden_jar_init() inside matrix_init_custom, well before this hook.
//
// The mode comes from a HARDWARE PRESENCE PIN, not from USB. IO expander pin 3 is
// high only when a live PS/2 host is powering the daughterboard (the divider is
// upstream of the daughterboard's current limiter, so USB 5V cannot reach it), and
// the main board's 47k pull-down holds it low otherwise. The user chooses the mode
// physically, by which of the two connectors they plug the cable into.
//
// This REPLACED a timing-based latch that spun up to PS2_USB_SETTLE_MS (2000 ms)
// waiting for usb_connected_state() and inferred "no USB, therefore PS/2" from the
// timeout. Do not go back to that shape. It cost every PS/2 boot a full 2 s before
// the first BAT could go out, and being wrong about the window was silent and
// nasty: an earlier 0 ms default read USB as absent on a cold power-cycle merely
// because enumeration had not finished, latched PS/2 with USB actually present, and
// host_set_driver then took the reports away from USB - keys and solenoid dead,
// while VIA/raw-HID still answered and hid the cause. The presence pin decides the
// same question with no window to get wrong.
//
// Note both connections CAN be live at once, but only with the case open, which is
// a bench-only condition: PS/2 output plus a working USB console for xprintf. It is
// not a configuration any user reaches, so the rest of the file still treats USB
// device-state events in PS/2 mode as rare (see notify_usb_device_state_change_kb).
void keyboard_post_init_kb(void) {
    // ONE read of the presence pin, whatever the build does with it afterwards. It
    // is an I2C transaction, so reading it twice (once to decide, once to report)
    // would be both wasteful and a change to what we are trying to observe.
    const bool ps2_present = io_expander_is_ps2_present();

#    ifdef PS2_DEBUG_CONSOLE
    // Latched for the one-shot report in housekeeping_task_kb. Recorded even when
    // PS2_FORCE_ENABLE overrides it below, so a forced bench build still shows what
    // a production build WOULD have decided.
    s_ps2_detect_read = ps2_present ? 1u : 0u;
#    endif

#    ifdef PS2_FORCE_ENABLE
    // DEBUG: ignore the presence pin and always come up in PS/2 mode, for a bench
    // board with no daughterboard fitted or a suspect detect pin. Never enable in a
    // production build. NOTE this SKIPS the presence-pin decision entirely, so a
    // fault that lives in that path cannot reproduce under this flag - which is why
    // the console output is on PS2_DEBUG_CONSOLE and no longer on this one.
    (void)ps2_present;
    ps2_enter_ps2_mode();
#    else
    if (ps2_present) {
        ps2_enter_ps2_mode();          // live PS/2 host on the daughterboard
    } else {
        s_ps2_mode = PS2_MODE_HAPTIC;  // no PS/2 hardware -> normal USB keyboard + haptic
    }
#    endif

    keyboard_post_init_user();  // preserve the keymap-level hook
}

void housekeeping_task_kb(void) {
#    ifdef PS2_DEBUG_CONSOLE
    // Report the boot latch every 5 s, FOREVER - never one-shot.
    //
    // This was `if (!reported_latch && ...)` and it cost the 2026-08-23 session. USB
    // is already ACTIVE within milliseconds of boot when the cable is plugged, so the
    // one-shot fired immediately, printed into a console whose listener (QMK Toolbox)
    // does not attach for several more seconds, and then latched itself off for good.
    // Undelivered xprintf output is dropped by the host, not buffered. A one-shot
    // boot print is therefore unobservable BY CONSTRUCTION on this setup - the same
    // trap that moved this block out of keyboard_post_init_kb in the first place;
    // moving it here only narrowed the window instead of closing it.
    //
    // Repeating is strictly better than one-shot anyway: the counters keep
    // accumulating, so a later line also shows what has happened SINCE you attached.
    //
    // Deliberately OUTSIDE the PS2_MODE_PS2 test below: if the presence read came
    // back 0 the board is in HAPTIC mode, ps2_service_main never runs, and the total
    // absence of a heartbeat is otherwise indistinguishable from a board that
    // crashed. mode=1 is HAPTIC, mode=2 is PS2 (ps2_mode_t); detect is what
    // io_expander_is_ps2_present() returned, which is 0 both when the pin reads low
    // AND when the I2C read fails outright.
    static uint32_t latch_ms   = 0;
    static bool     latch_seen = false;
    if (ps2_console_ready() && s_dbg_t_usb_ms == 0) {
        // First moment USB is ACTIVE. Stamped here rather than in a USB callback
        // because this is the same predicate the console itself uses, so the number
        // means exactly "from here on, output could have been delivered".
        s_dbg_t_usb_ms = timer_read32();
    }
    if (ps2_console_ready() && (!latch_seen || timer_elapsed32(latch_ms) >= 5000u)) {
        latch_seen = true;
        latch_ms   = timer_read32();
        xprintf("[ps2] boot latch: detect=%u mode=%u\n",
                (unsigned)s_ps2_detect_read, (unsigned)s_ps2_mode);
        // Everything that happened BEFORE this console existed - see the latch block
        // near s_dbg_hb_ticks for how to read it. `up` is ms since boot, i.e. how long
        // the unobserved run actually lasted; if it is small you plugged USB in before
        // the fault had a chance to happen. usage1 is the HID usage of the first key
        // that ever reached us (04 = 'a'), or 00 if none ever did.
        xprintf("[ps2] pre-console: up=%lums hb=%lu press=%lu publish=%lu keysmax=%u usage1=%02X thread=%lu state=%lu contacted=%u\n",
                (unsigned long)timer_read32(),
                (unsigned long)s_dbg_hb_ticks,
                (unsigned long)s_dbg_press_n,
                (unsigned long)s_dbg_publish_n,
                (unsigned)s_dbg_keys_max,
                (unsigned)s_dbg_usage_1st,
                (unsigned long)s_dbg_thread_n,
                (unsigned long)s_dbg_state_n,
                (unsigned)s_ps2_dev.host_contacted);
        // The one line to read off the screen when copy/paste is not available.
        // usb@ is when USB first went ACTIVE, press1@ when the first key was pressed
        // - their ORDER is the answer to "was the board still PS/2-only when you
        // typed?", which no cumulative counter can express.
        xprintf("[ps2] park: susp=%lu/%lu susp1@%lums\n",
                (unsigned long)s_dbg_susp_n,
                (unsigned long)s_dbg_thread_n,
                (unsigned long)s_dbg_t_susp1_ms);
        xprintf("[ps2] chain: press=%lu proc=%lu publish=%lu | hb1@%lums usb@%lums press1@%lums kc1=%04X\n",
                (unsigned long)s_dbg_press_n,
                (unsigned long)s_dbg_proc_n,
                (unsigned long)s_dbg_publish_n,
                (unsigned long)s_dbg_t_hb1_ms,
                (unsigned long)s_dbg_t_usb_ms,
                (unsigned long)s_dbg_t_press1_ms,
                (unsigned)s_dbg_kc1);
        ps2_debug_print_burst();
    }
#    endif

    // Only PS/2 mode needs servicing; HAPTIC mode leaves the default USB host driver
    // untouched. The mode was already latched in keyboard_post_init_kb, so there is
    // no UNDECIDED state to poll here.
    //
    // ONLY the main half. The protocol half (ps2_update, BAT, the snapshot) belongs
    // to the PS/2 thread and must not be re-entered from here - see the single-owner
    // note on ps2_service_protocol(). This is the change that gives the matrix scan
    // its period back: the loop no longer busy-waits up to 10 ms draining bytes, nor
    // blocks for the ~9 ms a committed Pause blob takes.
    if (s_ps2_mode == PS2_MODE_PS2) {
        ps2_service_main();
    }
}

// Take the PS/2 pins back from the haptic subsystem. THIS IS LOAD-BEARING, not
// defensive - it fixes the stuck bus (clk stuck low, `busy=1 sent=0 clk=0 dat=1`)
// seen at M4 bring-up. The path:
//
//   f77/keymaps/vial/config.h sets HAPTIC_OFF_IN_LOW_POWER 1, which turns on the
//   `#if defined(HAPTIC_ENABLE) && HAPTIC_OFF_IN_LOW_POWER` gate in
//   tmk_core/protocol/usb_device_state.c, so every USB device-state transition
//   calls haptic_notify_usb_device_state_change() (quantum/haptic.c). That does
//   update_haptic_enable_gpios() - haptic is disabled here, so it writes
//   HAPTIC_ENABLE_PIN_WRITE_INACTIVE() == gpio_write_pin_low(GP28) - and then
//   gpio_set_pin_output(HAPTIC_ENABLE_PIN), which re-muxes GP28 from PIO1 back to
//   SIO *as a driven output*. GP28 is our PS/2 CLOCK, so the bus is pinned low and
//   the ps2out program's `wait 1 gpio <clock>` never releases. GP29 (data) is not
//   on this path, which is exactly why the fault showed as clk=0 with dat=1.
//
// A bare gpio_write_pin on a PIO-owned pad is harmless (the earlier analysis was
// right about that); it is the gpio_set_pin_output *re-mux* that steals the pin.
// This fires on configure / suspend / resume / reset / SET_PROTOCOL / host-LED /
// idle-rate changes, i.e. any time after our boot latch - and it is why the M4
// debug harness worked: it was built with HAPTIC_ENABLE=no, compiling the whole
// path out.
//
// notify_usb_device_state_change_kb is the right seam: it is a weak symbol called
// from the very same notify_usb_device_state_change() a few lines *after* the
// haptic call (usb_device_state.c), so the repair happens in the same invocation
// and cannot be missed. It also keeps the fix inside keyboards/leyden_jar/ - no
// QMK core changes. Worst case a USB event lands mid-frame and corrupts one byte;
// the host's resend handling covers that, and these events are rare.
void notify_usb_device_state_change_kb(struct usb_device_state usb_device_state) {
    if (s_ps2_mode == PS2_MODE_PS2) {
        ps2_platform_reclaim_pins(PS2_PIO, PS2_DATA_PIN, PS2_CLOCK_PIN);
    }
    notify_usb_device_state_change_user(usb_device_state);  // preserve the keymap-level hook
}

// In PS/2 mode the solenoid connector carries the PS/2 bus (GP28/GP29 are muxed to
// PIO1), so the haptic controls are meaningless and must not run. We latch haptic
// off once at mode entry (haptic_disable), but a user could still press HPT_* keys.
// No haptic keycode re-muxes the pins - haptic_enable/toggle/reset only do SIO
// writes that a PIO-owned pad ignores - so the bus is electrically safe either way.
// We block them anyway because re-enabling flips haptic_config.enable and *persists
// it to EEPROM* (an untimely RP2040 flash write that stalls PS/2 servicing and
// changes saved config), and lets the solenoid task churn.
//
// This MUST be pre_process (not process_record_kb): process_haptic() runs in
// process_record_quantum_helper *before* process_record_kb, so a process_record_kb
// veto would be too late. pre_process_record_quantum gates process_record entirely
// (action.c), so returning false here drops the key before process_haptic sees it.
bool pre_process_record_kb(uint16_t keycode, keyrecord_t *record) {
#    ifdef PS2_DEBUG_CONSOLE
    // Every key PRESS the matrix produced, counted before any keycode filtering.
    // This is the link that says whether the capacitive matrix is seeing keys at
    // all: if this stays 0 while you hold keys down, the fault is upstream of PS/2
    // entirely (matrix / calibration / thresholds), not in the driver.
    if (record->event.pressed) {
        if (s_dbg_press_n == 0) {
            s_dbg_t_press1_ms = timer_read32();
            s_dbg_kc1         = keycode;
        }
        s_dbg_press_n++;
    }
#    endif

    if (s_ps2_mode == PS2_MODE_PS2 &&
        keycode >= QK_HAPTIC_ON && keycode <= QK_HAPTIC_CONTINUOUS_DOWN) {
        return false;  // swallow every haptic keycode while PS/2 owns GP28/GP29
    }
    return pre_process_record_user(keycode, record);
}

// Present ONLY to count presses that survive pre_process_record_quantum, splitting
// the "press seen but nothing published" gap in two. No other process_record_kb
// exists anywhere in keyboards/leyden_jar/, so this does not displace one.
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
#    ifdef PS2_DEBUG_CONSOLE
    if (record->event.pressed) {
        s_dbg_proc_n++;
    }
#    endif
    return process_record_user(keycode, record);
}

#endif  // PS2_DEVICE_ENABLE
