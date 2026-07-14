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
#    include "haptic.h"        // haptic_disable
#    include "solenoid.h"      // solenoid_shutdown
#    include "hardware/pio.h"  // pio1, PIO

#    include "hid_to_ps2.h"    // the portable driver's public surface

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
#        define PS2_USB_SETTLE_MS 0u        // bounded confirm window before latching PS/2 (§3.3)
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

typedef enum {
    PS2_MODE_UNDECIDED = 0,  // still latching
    PS2_MODE_HAPTIC,         // USB present -> normal USB keyboard + haptic; PS/2 never inits
    PS2_MODE_PS2,            // no USB -> haptic released, PS/2 running
} ps2_mode_t;

static ps2_mode_t s_ps2_mode          = PS2_MODE_UNDECIDED;
static uint32_t   s_ps2_boot_ms       = 0;  // timer_read32() at post_init, for the settle window
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
}

// NKRO: report->bits[] is already a bit-per-HID-usage bitmap, identical layout to
// our own - a straight 30-byte copy, then fold in the separate modifier byte.
static void ps2_send_nkro(report_nkro_t *report) {
    ps2_key_state_clear(s_ps2_latest_state);
    memcpy(s_ps2_latest_state, report->bits, NKRO_REPORT_BITS);
    ps2_key_state_set_mods(s_ps2_latest_state, report->mods);
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

// --- mode latch (§3.3) ---------------------------------------------------------
//
// Runs from housekeeping (i.e. after keyboard_init incl. the ~1 s capsense
// calibration + the rest of boot), so if a USB host is present it has already
// enumerated and usb_connected_state() reads true on the first sample. The settle
// window is only a bounded safety margin against an unusually slow host.
static void ps2_enter_ps2_mode(void) {
    // Release the two shared pins from the haptic subsystem *before* remuxing them
    // to PIO1. haptic_disable() is runtime-only (not persisted), so USB-mode haptic
    // still works on a later boot; solenoid_shutdown() drives GP29 to a safe level.
    haptic_disable();
    solenoid_shutdown();

    // ps2_initialize -> ps2_platform_init remuxes GP28/GP29 (SIO -> PIO1) and starts
    // the ps2out state machine. rx handler is NULL: the driver stashes host LED
    // state in dev.leds itself, which ps2_kb_leds() reads directly.
    ps2_initialize(&s_ps2_dev, PS2_PIO, PS2_SM, PS2_DATA_PIN, PS2_CLOCK_PIN, NULL);

    // From now on QMK pushes resolved reports to us instead of USB.
    host_set_driver(&s_ps2_host_driver);

    s_ps2_last_bat_ms = timer_read32();
    s_ps2_mode        = PS2_MODE_PS2;
}

static void ps2_latch_mode(void) {
    if (usb_connected_state()) {
        // A USB host is attached -> stay a normal USB keyboard; never init PS/2.
        s_ps2_mode = PS2_MODE_HAPTIC;
    } else if (timer_elapsed32(s_ps2_boot_ms) >= PS2_USB_SETTLE_MS) {
        // No USB after the confirm window -> this board is on a PS/2 connector.
        ps2_enter_ps2_mode();
    }
    // else: still inside the settle window, re-check next housekeeping call.
}

// --- per-loop PS/2 servicing (§3.2) -------------------------------------------
static void ps2_service(void) {
    // Re-announce BAT-complete until the host first talks to us. Our first 0xAA
    // lands seconds after power-on (boot + calibration + BAT delay), far past the
    // spec window, so a single announce can be missed; repeat until host_contacted.
    if (!s_ps2_dev.host_contacted && timer_elapsed32(s_ps2_last_bat_ms) >= PS2_REANNOUNCE_MS) {
        ps2_announce_bat(&s_ps2_dev);
        s_ps2_last_bat_ms = timer_read32();
    }

    // Bounded, blob-atomic drain. ps2_update emits at most one byte per call and
    // then must wait out the ~1 ms frame + inter-byte gap, so we call it repeatedly
    // to flush a multi-byte key at the wire floor instead of one byte per slow loop.
    // The time budget is honoured only at blob boundaries (ps2_tx_in_blob false), so
    // a multi-byte sequence is never split mid-flight - only the gap *between* keys
    // can be deferred, which the protocol allows.
    uint32_t start = ps2_now_us();
    do {
        ps2_update(&s_ps2_dev, ps2_now_us(), s_ps2_latest_state);
    } while (ps2_tx_in_blob(&s_ps2_dev) ||
             (ps2_tx_pending(&s_ps2_dev) &&
              (uint32_t)(ps2_now_us() - start) < PS2_DRAIN_BUDGET_US));
}

// --- QMK hooks -----------------------------------------------------------------
void keyboard_post_init_kb(void) {
    s_ps2_boot_ms = timer_read32();
    s_ps2_mode    = PS2_MODE_UNDECIDED;
    keyboard_post_init_user();  // preserve the keymap-level hook
}

void housekeeping_task_kb(void) {
    switch (s_ps2_mode) {
        case PS2_MODE_UNDECIDED:
            ps2_latch_mode();
            break;
        case PS2_MODE_PS2:
            ps2_service();
            break;
        case PS2_MODE_HAPTIC:
        default:
            // USB/haptic mode: PS/2 stays idle, default USB host driver untouched.
            break;
    }
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
