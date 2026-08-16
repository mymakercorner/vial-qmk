#pragma once

#include <stdint.h>
#include <hardware/pio.h>   // PIO, and the `uint` typedef (via pico/types.h)

#include "ps2_scancode_queue.h"

/*
 * NOTE: this header intentionally does NOT include <pico/stdlib.h>. The driver
 * core touches no GPIO/stdlib symbols (only PIO FIFO/IRQ calls and the `uint` /
 * `PIO` types, all from <hardware/pio.h>), so the same header compiles under both
 * the bare Pico SDK and QMK/ChibiOS. The GPIO API lives only in the per-platform
 * ps2_platform_*.c files, which include what they need themselves.
 */

/*
 * Size of the key-state bitmap used by ps2_update: one bit per HID usage ID,
 * 256 usages / 8 = 32 bytes. bit (hid & 7) of byte (hid >> 3) set == key down.
 */
#define PS2_KEYBOARD_STATE_SIZE_BYTES 32

/*
 * PS2_SCANCODE_SEQ(...) builds a length-prefixed PS/2 scancode blob as a
 * compound literal: element [0] is the number of scancode bytes (computed with
 * sizeof, so it can never drift out of sync with the data), followed by the
 * bytes in transmission order. Used by the hid_to_ps2_set_*.h lookup tables.
 * Standard C99/C11, no GNU extensions.
 */
#define PS2_SCANCODE_SEQ(...) \
    (const uint8_t[]){ (uint8_t)sizeof((const uint8_t[]){ __VA_ARGS__ }), __VA_ARGS__ }

/*
 * Optional notification of host-driven state changes, so the firmware can react
 * (typically to drive the keyboard LEDs). Called with the command byte that
 * caused the change and its relevant value: (0xED, LED bitmap), (0xF0, new
 * scancode set), (0xF4, 1) enabled, (0xF5, 0) disabled. May be NULL. The device
 * handles the PS/2 protocol itself; this is only a notification hook.
 */
typedef void (*ps2_rx_handler)(uint8_t event, uint8_t value);

// Phases of dev->startup, the power-on BAT sequence handled by ps2_update.
// Public because ps2_startup_complete() below is part of the NULL-keyboard_state
// contract on ps2_update, and a caller cannot honour that contract blind.
enum ps2_startup_phase {
    PS2_STARTUP_DONE = 0,   // BAT-complete already announced; normal operation
    PS2_STARTUP_INIT = 1,   // just powered on; capture the clock on the first poll
    PS2_STARTUP_WAIT = 2,   // waiting out the BAT delay before sending 0xAA
};

typedef struct {
    PIO      pio;
    uint     sm;
    ps2_scancode_queue queue;   // pending outgoing scancode blobs
    ps2_rx_handler rx;          // host -> device command handler (may be NULL)
    uint32_t idle_since_us;     // when the bus last became idle (clock high, not busy)
    uint8_t  busy;              // bit0 = PIO active / host inhibit, bit1 = byte in flight
    uint32_t guard_since_us;    // when bit1 (byte-in-flight guard) was last set, for a stale-guard timeout
    uint8_t  sent;              // bytes of the current blob already sent
    uint8_t  last_tx;           // last byte sent (for host resend request)
    uint8_t  current_set;       // active scancode set (1, 2 or 3); powers on as 2
    uint8_t  leds;              // host LED bitmap: bit0 Scroll, bit1 Num, bit2 Caps
    uint8_t  scanning_enabled;  // 0xF4 enables key output, 0xF5 disables it
    uint8_t  pending_cmd;       // two-byte command awaiting its data byte (0 = none)
    uint32_t repeat_us;         // typematic repeat period (0xF3-decoded)
    uint16_t delay_ms;          // typematic delay before repeat begins (0xF3-decoded)
    uint8_t  repeat_key;        // HID code of the key currently repeating (0 = none)
    uint32_t repeat_at_us;      // when the next typematic make is due (valid if repeat_key)
    uint8_t  startup;           // power-on BAT phase (see enum ps2_startup_phase above)
    uint8_t  host_contacted;    // set once any valid byte has been received from the host
    uint32_t bat_due_us;        // when the power-on 0xAA (BAT-complete) is due to be sent
    uint8_t  scs3_keymode[256]; // Set-3 per-key mode bits (typematic/break); read only when current_set == 3
    uint8_t  scs3_pending_mode; // 0xFB-0xFD key-list collecting state (0 = idle; see ps2_key_mode in the .c)
    // last key-state bitmap seen by ps2_update, for edge detection:
    uint8_t  prev_keyboard_state[PS2_KEYBOARD_STATE_SIZE_BYTES];
} ps2_device;

/*
 * Encode a byte into an 11-bit PS/2 frame (start, 8 data, odd parity, stop),
 * inverted for the open-collector drive of ps2out.pio.
 */
uint32_t ps2_encode_frame(uint8_t byte_val);

// Set up the pins and PIO, and initialise the device state.
void ps2_initialize(ps2_device *dev, PIO pio, uint sm, uint data_pin, uint clock_pin, ps2_rx_handler rx);

/*
 * Platform hardware bring-up, called by ps2_initialize. Configures the data/clock
 * GPIOs and loads + starts the ps2out PIO program on (pio, sm); on return the
 * state machine is running and both lines idle high. This is the ONLY part of the
 * driver that touches a platform's GPIO/PIO-mux API, so it is provided by a
 * per-platform translation unit rather than hid_to_ps2.c: the bare Pico SDK build
 * links ps2_platform_pico.c (gpio_*), the QMK build links a ChibiOS/PAL version.
 * The rest of the driver depends only on the portable Pico-SDK PIO FIFO/IRQ calls.
 */
void ps2_platform_init(PIO pio, uint sm, uint data_pin, uint clock_pin);

/*
 * Re-arm the power-on BAT sequence so an unsolicited 0xAA (BAT-complete) is sent
 * again after the usual delay. A real keyboard only announces once, but when the
 * device is powered independently of the host (debug bring-up), the host may not
 * have been listening at power-up; calling this lets the firmware re-announce
 * until the host makes contact. Does not touch configuration or the queue.
 */
void ps2_announce_bat(ps2_device *dev);

/*
 * Poll once from the main loop, given the full current key state. This is the
 * single entry point the firmware driver calls each iteration.
 *   `now_us` is the caller's microsecond clock (e.g. (uint32_t)time_us_64() on
 *     the Pico), kept as a parameter so the transport depends on no timer API.
 *   `keyboard_state` is the current pressed-set as a PS2_KEYBOARD_STATE_SIZE_BYTES
 *     byte bitmap (bit[usage] = key down). ps2_update diffs it against the
 *     previous call to produce the make/break edges the PS/2 protocol needs, so
 *     the driver only has to report which keys are down; build it with the
 *     helpers below.
 *
 *     Pass NULL to mean "the pressed-set has not changed since the last call
 *     that supplied one". The diff is then skipped and every other step (host
 *     RX, typematic, the send path) runs exactly as normal. This is purely an
 *     optimisation, and an OPTIONAL one - a caller that always passes the bitmap
 *     is correct, just slower. It matters because the diff is unconditional over
 *     all 32 bytes and dominates the call: measured from the -Os ARM build it is
 *     ~704 of ~810 cycles (~5.7 us of ~6.5 us at 125 MHz), and a firmware that
 *     polls faster than its report rate - a drain loop, or a dedicated servicing
 *     thread - repeats that work on every poll for no result.
 *
 *     A caller using NULL must latch "changed" itself, and must clear that latch
 *     in the same atomic step as taking the snapshot it passes in. Clearing it
 *     afterwards races: a change arriving mid-diff would be erased and lost.
 * When several keys change between calls, the highest HID usage that went down
 * becomes the typematic key.
 */
void ps2_update(ps2_device *dev, uint32_t now_us, const uint8_t *keyboard_state);

// --- Outgoing-transmit status, for a caller that drains in a loop -------------
/*
 * A firmware whose main loop runs slower than the PS/2 byte rate can call
 * ps2_update repeatedly in one iteration to flush queued scancodes sooner (each
 * ps2_update emits at most one byte). These predicates let such a drain loop know
 * when to stop, without reaching into the device internals.
 */

// True while a scancode blob is only partially transmitted (some bytes sent, more
// to go, or the final byte still in flight before the blob is retired). A drain
// loop must keep going while this holds so a multi-byte sequence (e.g. an E0/E1
// key) is never split by a long inter-byte gap; check any time/byte budget only
// when this is false, i.e. at a blob boundary.
static inline bool ps2_tx_in_blob(const ps2_device *dev) {
    return dev->sent != 0;
}

// True while any outgoing scancode work remains: a blob queued, or one in
// progress. When this is false all key output has drained.
static inline bool ps2_tx_pending(const ps2_device *dev) {
    return !ps2_scancode_queue_empty(&dev->queue) || dev->sent != 0;
}

// True once the power-on BAT announcement has been made. Until then ps2_update
// ignores keyboard_state completely (its step 1 and step 2 are exclusive), so a
// caller using the NULL "unchanged" contract must not consume its changed-latch
// before this returns true - a key held through the 500 ms BAT window would be
// latched, cleared, and then never diffed.
static inline bool ps2_startup_complete(const ps2_device *dev) {
    return dev->startup == PS2_STARTUP_DONE;
}

// --- Helpers to build the bitmap ps2_update expects --------------------------

// Clear all bits (no keys pressed).
static inline void ps2_key_state_clear(uint8_t *keyboard_state) {
    for (uint8_t i = 0; i < PS2_KEYBOARD_STATE_SIZE_BYTES; i++) {
        keyboard_state[i] = 0;
    }
}

// Mark one HID usage ID as pressed.
static inline void ps2_key_state_set(uint8_t *keyboard_state, uint8_t hid) {
    keyboard_state[hid >> 3] |= (uint8_t)(1u << (hid & 7));
}

/*
 * Mark the modifier keys from an 8-bit HID modifier bitfield (the boot-report
 * byte 0). Bit i maps to usage 0xE0 + i: LCtrl, LShift, LAlt, LGUI, RCtrl,
 * RShift, RAlt, RGUI. Use for 6KRO where control keys arrive as a bitfield.
 */
static inline void ps2_key_state_set_mods(uint8_t *keyboard_state, uint8_t mods) {
    for (uint8_t i = 0; i < 8; i++) {
        if (mods & (1u << i)) {
            ps2_key_state_set(keyboard_state, (uint8_t)(0xE0 + i));
        }
    }
}