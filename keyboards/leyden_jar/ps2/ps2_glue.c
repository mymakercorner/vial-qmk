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
 *   - Per-loop protocol servicing: a bounded, blob-atomic drain of ps2_update in
 *     housekeeping_task_kb (§3.2).
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

#    include "quantum.h"       // QMK core: hooks, timer_read32/elapsed32, led_t, report_*_t
#    include "host.h"          // host_set_driver, host_driver_t
#    include "usb_util.h"      // usb_connected_state
#    include "usb_device_state.h"  // struct usb_device_state (notify hook signature)
#    include "haptic.h"        // haptic_disable
#    include "solenoid.h"      // solenoid_shutdown
#    include "hardware/pio.h"  // pio1, PIO
#    ifdef PS2_FORCE_ENABLE
#        include "hardware/gpio.h"  // gpio_get for the debug heartbeat's live line-level read
#    endif

#    include "hid_to_ps2.h"    // the portable driver's public surface

// Re-mux the two PS/2 pads back to the PIO after something else has grabbed one.
// Lives in ps2_platform_chibios.c (the file that owns pin muxing); declared here
// rather than in hid_to_ps2.h because both files are QMK-only and the portable
// header must stay byte-identical to the driver repo's copy.
void ps2_platform_reclaim_pins(PIO pio, uint data_pin, uint clock_pin);

#    ifdef PS2_FORCE_ENABLE
#        include "print.h"      // xprintf -> USB-CDC console (sendchar) on RP2040/ChibiOS
#        include "ps2_trace.h"  // ps2_trace_pop / ps2_trace_dropped for the debug drain
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
#    ifndef PS2_DRAIN_BUDGET_US
#        define PS2_DRAIN_BUDGET_US 10000u  // soft cap on one housekeeping drain (§3.2)
#    endif
#    ifndef PS2_USB_SETTLE_MS
// How long USB may stay un-enumerated after boot before we conclude "no USB host"
// and latch to PS/2 (§3.3). MUST be non-zero: usb_connected_state() only reads true
// after enumeration, which is not guaranteed by the first housekeeping tick. With a
// 0 window a single early false reading latches to PS/2 even though USB *is* present
// (host_set_driver then steals keyboard reports off USB -> keys dead, haptic off).
// The latch re-checks every tick and picks HAPTIC the instant USB comes up, so this
// is only the worst-case wait a genuine PS/2-powered board takes before starting.
#        define PS2_USB_SETTLE_MS 2000u     // mirrors SPLIT_USB_TIMEOUT
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
// consumed in ps2_service. When clear we hand ps2_update NULL instead of the
// bitmap, which skips its 32-byte diff - ~704 of the function's ~810 cycles
// (~5.7 us of ~6.5 us at 125 MHz, measured from the -Os disassembly). That diff
// is unconditional and finds nothing when the report has not moved, and the
// drain loop below re-enters ps2_update many times per multi-byte key, so this
// is most of the cost of servicing PS/2 while typing.
//
// Correctness rests on two things, both easy to get wrong:
//   - the flag is cleared in the SAME step that takes the snapshot. Clearing it
//     afterwards would let a report arriving mid-diff be erased and lost.
//   - it is NOT consumed until the driver has finished BAT, because ps2_update
//     ignores the bitmap entirely until then (its step 1 / step 2 are exclusive).
//     Without that guard a key held through the 500 ms BAT window never emits.
static bool s_ps2_state_dirty = false;

// Snapshot handed to ps2_update, so the driver never reads the live bitmap the
// report callbacks are writing. Today both run on the QMK main thread and the
// copy is strictly speaking redundant; it is here because the snapshot + flag
// clear must become one atomic step the moment PS/2 servicing moves to its own
// thread, and that is easier to get right if the shape is already correct.
static uint8_t s_ps2_state_snapshot[PS2_KEYBOARD_STATE_SIZE_BYTES];

typedef enum {
    PS2_MODE_UNDECIDED = 0,  // initial sentinel; overwritten by the boot-time decision
    PS2_MODE_HAPTIC,         // USB present -> normal USB keyboard + haptic; PS/2 never inits
    PS2_MODE_PS2,            // no USB -> haptic released, PS/2 running
} ps2_mode_t;

static ps2_mode_t s_ps2_mode          = PS2_MODE_UNDECIDED;  // decided once in keyboard_post_init_kb
static uint32_t   s_ps2_last_bat_ms   = 0;  // last 0xAA re-announce (§7.5)

// --- host_driver_t callbacks (§3.1) -------------------------------------------
//
// These fire (from QMK's report pipeline) whenever the resolved report changes.
// They only *capture* the pressed set into s_ps2_latest_state - cheap, no PIO work;
// the actual wire traffic + edge detection + typematic happen in ps2_update,
// driven from the housekeeping poll below.

// 6KRO: report is mods (bitfield) + up to 6 held usage codes.
static void ps2_send_6kro(report_keyboard_t *report) {
    ps2_key_state_clear(s_ps2_latest_state);
    for (uint8_t i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
        uint8_t code = report->keys[i];
        if (code) {
            ps2_key_state_set(s_ps2_latest_state, code);
        }
    }
    ps2_key_state_set_mods(s_ps2_latest_state, report->mods);
    s_ps2_state_dirty = true;
}

// NKRO: report->bits[] is already a bit-per-HID-usage bitmap, identical layout to
// our own - a straight 30-byte copy, then fold in the separate modifier byte.
static void ps2_send_nkro(report_nkro_t *report) {
    ps2_key_state_clear(s_ps2_latest_state);
    memcpy(s_ps2_latest_state, report->bits, NKRO_REPORT_BITS);
    ps2_key_state_set_mods(s_ps2_latest_state, report->mods);
    s_ps2_state_dirty = true;
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

    // Ask QMK to push resolved reports to us instead of USB. NOTE: this initial
    // install is clobbered moments later by protocol_post_init()'s
    // host_set_driver(&chibios_driver) (see the boot-order note in ps2_service),
    // so ps2_service re-asserts it every loop. Kept here so the intent is explicit
    // and the driver is installed as early as possible.
    host_set_driver(&s_ps2_host_driver);

    s_ps2_last_bat_ms = timer_read32();
    s_ps2_mode        = PS2_MODE_PS2;

#    ifdef PS2_FORCE_ENABLE
    xprintf("\n[ps2] entered PS/2 mode (FORCED, USB console live). clk=GP%u data=GP%u\n",
            (unsigned)PS2_CLOCK_PIN, (unsigned)PS2_DATA_PIN);
#    endif
}

// --- debug trace readout (PS2_FORCE_ENABLE only) ------------------------------
//
// Pop the driver's byte-level trace ring buffer and print it over the USB-CDC
// console (xprintf -> sendchar). Kept in the glue so ps2_trace.c stays byte-
// identical to the standalone testbed (its own ps2_trace_drain_printf uses stdio
// printf, which ChibiOS/newlib does not route to the console). Throttled and
// only called at blob boundaries, so it never perturbs a live PS/2 transaction.
#    ifdef PS2_FORCE_ENABLE
static void ps2_debug_drain_trace(void) {
    static uint32_t last_ms   = 0;
    static uint32_t prev_us   = 0;
    static bool     have_prev = false;
    static uint32_t seen_drop = 0;

    if (timer_elapsed32(last_ms) < 250u) {
        return;  // rate-limit console traffic to ~4 Hz
    }
    last_ms = timer_read32();

    ps2_trace_evt e;
    while (ps2_trace_pop(&e)) {
        int32_t dt = have_prev ? (int32_t)(e.t_us - prev_us) : 0;
        prev_us    = e.t_us;
        have_prev  = true;

        const char *k;
        switch (e.tag) {
            case PS2_TR_TX:      k = "TX  "; break;
            case PS2_TR_RX:      k = "RX  "; break;
            case PS2_TR_RX_PERR: k = "RX! "; break;
            case PS2_TR_RESEND:  k = "RSND"; break;
            case PS2_TR_BAT:     k = "BAT "; break;
            case PS2_TR_NOTE:    k = "NOTE"; break;
            default:             k = "??? "; break;
        }
        xprintf("[%lu +%ld] %s %02X\n",
                (unsigned long)e.t_us, (long)dt, k, (unsigned)e.a);
    }

    uint32_t dropped = ps2_trace_dropped();
    if (dropped != seen_drop) {
        xprintf("[trace] %lu event(s) dropped total\n", (unsigned long)dropped);
        seen_drop = dropped;
    }
}
#    endif  // PS2_FORCE_ENABLE

// --- per-loop PS/2 servicing (§3.2) -------------------------------------------
static void ps2_service(void) {
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

#    ifdef PS2_FORCE_ENABLE
    // Unconditional ~1 Hz heartbeat: proves the HID console transport works even
    // when there is zero PS/2 traffic, and dumps the state we need to bisect the
    // "no keys" fault. keys = number of usage bits currently set in the resolved
    // report we've been handed (press a key on the F77 and this should rise); drv =
    // 1 iff our PS/2 host driver is the installed one (0 => still clobbered).
    static uint32_t s_hb_ms = 0;
    if (timer_elapsed32(s_hb_ms) >= 1000u) {
        s_hb_ms = timer_read32();
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
        xprintf("[ps2] hb startup=%u set=%u scan=%u contacted=%u keys=%u drv=%u qpend=%u busy=%u sent=%u clk=%u dat=%u\n",
                (unsigned)s_ps2_dev.startup, (unsigned)s_ps2_dev.current_set,
                (unsigned)s_ps2_dev.scanning_enabled, (unsigned)s_ps2_dev.host_contacted,
                (unsigned)keys, (unsigned)(host_get_driver() == &s_ps2_host_driver),
                (unsigned)ps2_tx_pending(&s_ps2_dev),
                (unsigned)s_ps2_dev.busy, (unsigned)s_ps2_dev.sent, clk, dat);
    }

    // Report the first host contact once, then stream the byte trace. Both run at
    // a blob boundary (before the drain loop), never mid-sequence.
    static bool announced_contact = false;
    if (!announced_contact && s_ps2_dev.host_contacted) {
        announced_contact = true;
        xprintf("[ps2] host_contacted=1 (host is talking to us)\n");
    }
    ps2_debug_drain_trace();
#    endif

    // Re-announce BAT-complete until the host first talks to us. Our first 0xAA
    // lands seconds after power-on (boot + calibration + BAT delay), far past the
    // spec window, so a single announce can be missed; repeat until host_contacted.
    if (!s_ps2_dev.host_contacted && timer_elapsed32(s_ps2_last_bat_ms) >= PS2_REANNOUNCE_MS) {
        ps2_announce_bat(&s_ps2_dev);
        s_ps2_last_bat_ms = timer_read32();
    }

    // Take the changed-latch and the snapshot in one step, so a report landing
    // between the two cannot be erased (see s_ps2_state_dirty). Held off until
    // BAT is done, because ps2_update discards the bitmap until then.
    const uint8_t *state = NULL;
    if (s_ps2_state_dirty && ps2_startup_complete(&s_ps2_dev)) {
        memcpy(s_ps2_state_snapshot, s_ps2_latest_state, sizeof(s_ps2_state_snapshot));
        s_ps2_state_dirty = false;
        state             = s_ps2_state_snapshot;
    }

    // Bounded, blob-atomic drain. ps2_update emits at most one byte per call and
    // then must wait out the ~1 ms frame + inter-byte gap, so we call it repeatedly
    // to flush a multi-byte key at the wire floor instead of one byte per slow loop.
    // The time budget is honoured only at blob boundaries (ps2_tx_in_blob false), so
    // a multi-byte sequence is never split mid-flight - only the gap *between* keys
    // can be deferred, which the protocol allows.
    uint32_t start = ps2_now_us();
    do {
        ps2_update(&s_ps2_dev, ps2_now_us(), state);
        // Only the first pass carries the bitmap: it has already been diffed into
        // make/break events, and every later pass in this drain is here purely to
        // push bytes. Re-passing it would repeat the 32-byte diff - the dominant
        // cost of ps2_update - once per byte of the blob, for no result.
        state = NULL;
    } while (ps2_tx_in_blob(&s_ps2_dev) ||
             (ps2_tx_pending(&s_ps2_dev) &&
              (uint32_t)(ps2_now_us() - start) < PS2_DRAIN_BUDGET_US));
}

// --- QMK hooks -----------------------------------------------------------------
// The USB-vs-PS/2 mode is decided ONCE here, not polled per loop. This hook is the
// last thing keyboard_init runs (keyboard_post_init_quantum, keyboard.c ~548), hence
// *after* haptic_init (keyboard.c ~541): the shared GP28/GP29 pins are already owned
// by haptic, so it is safe to take them for PS/2. Deciding any earlier (e.g. in
// matrix_init_custom, keyboard.c ~475) would be undone when haptic_init later re-grabs
// those pins. We block up to PS2_USB_SETTLE_MS for USB to enumerate - ChibiOS services
// USB from irq/threads during the wait, so usb_connected_state() still updates - the
// same idiom QMK split keyboards use for USB detection in split_pre_init.
void keyboard_post_init_kb(void) {
#    ifdef PS2_FORCE_ENABLE
    // DEBUG: ignore USB so the USB-CDC console stays live for trace readout while PS/2
    // runs (host_set_driver only steals keyboard reports, not the console endpoint).
    // Never enable in a production build.
    ps2_enter_ps2_mode();
#    else
    uint32_t start = timer_read32();
    while (!usb_connected_state() && timer_elapsed32(start) < PS2_USB_SETTLE_MS) {
        // spin until the host enumerates us, or the settle window expires
    }
    if (usb_connected_state()) {
        s_ps2_mode = PS2_MODE_HAPTIC;  // USB host present -> normal USB keyboard + haptic
    } else {
        ps2_enter_ps2_mode();          // no USB after the window -> this board is on PS/2
    }
#    endif

    keyboard_post_init_user();  // preserve the keymap-level hook
}

void housekeeping_task_kb(void) {
    // Only PS/2 mode needs periodic servicing; HAPTIC mode leaves the default USB host
    // driver untouched. The mode was already latched in keyboard_post_init_kb, so there
    // is no UNDECIDED state to poll here.
    if (s_ps2_mode == PS2_MODE_PS2) {
        ps2_service();
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
    if (s_ps2_mode == PS2_MODE_PS2 &&
        keycode >= QK_HAPTIC_ON && keycode <= QK_HAPTIC_CONTINUOUS_DOWN) {
        return false;  // swallow every haptic keycode while PS/2 owns GP28/GP29
    }
    return pre_process_record_user(keycode, record);
}

#endif  // PS2_DEVICE_ENABLE
