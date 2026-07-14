#include <stdint.h>
#include <stdbool.h>
#include "hid_to_ps2.h"
#include "ps2_trace.h"   // byte-level trace hooks (compiled out when PS2_TRACE_ENABLED=0)

#include "hid_to_ps2_set_1.h"
#include "hid_to_ps2_set_2.h"
#include "hid_to_ps2_set_3.h"

// Continuous idle (clock high, not busy) required before starting a frame. The
// PS/2 spec mandates >= 50 us of clock-high before a device may transmit; 100 us
// gives margin and still leaves the host room to inhibit between bytes.
#define PS2_INTERBYTE_GAP_US 100

// A whole frame transmits in ~1 ms. The "byte in flight" guard (busy bit 1) is
// meant to be cleared once we catch the PIO activity IRQ, but if that IRQ window
// falls entirely between two ps2_update polls (loop jitter) we miss it and the
// guard sticks forever, wedging all transmission. Clear a guard older than this
// as a fail-safe so busy can never stick.
#define PS2_TX_GUARD_TIMEOUT_US 3000

// Power-on self-test delay. A real keyboard runs its Basic Assurance Test (BAT)
// for roughly half a second after power-up, then announces completion with an
// unsolicited 0xAA. The spec allows 500-750 ms; ps2_update schedules the byte
// this far after its first poll (the first moment it has a clock reading).
#define PS2_BAT_DELAY_US 500000u

// Phases of dev->startup, the power-on BAT sequence handled by ps2_update.
enum ps2_startup_phase {
    PS2_STARTUP_DONE = 0,   // BAT-complete already announced; normal operation
    PS2_STARTUP_INIT = 1,   // just powered on; capture the clock on the first poll
    PS2_STARTUP_WAIT = 2,   // waiting out the BAT delay before sending 0xAA
};

static void ps2_set_defaults(ps2_device *dev);   // defined with the host-command handler below

// Host-to-device command bytes (host -> keyboard).
enum ps2_host_cmd {
    PS2_CMD_SET_LEDS      = 0xED,   // + 1 data byte: LED bitmap
    PS2_CMD_ECHO          = 0xEE,   // diagnostic echo
    PS2_CMD_SET_SCAN_SET  = 0xF0,   // + 1 data byte: 0 = query, 1/2/3 = select
    PS2_CMD_READ_ID       = 0xF2,   // reply with the 2-byte keyboard ID
    PS2_CMD_SET_TYPEMATIC = 0xF3,   // + 1 data byte: rate/delay
    PS2_CMD_ENABLE        = 0xF4,   // enable scanning (key output)
    PS2_CMD_DISABLE       = 0xF5,   // disable scanning
    PS2_CMD_SET_DEFAULTS  = 0xF6,   // restore power-on defaults
    // Set-3 per-key mode commands (see enum ps2_key_mode). Set-all take no data;
    // set-key are followed by a list of key scancodes. 0xFA doubles as the ACK
    // byte on the response side (PS2_RSP_ACK) - opposite direction, no conflict.
    PS2_CMD_SET_ALL_TYPEMATIC  = 0xF7,   // all keys typematic (repeat, no break)
    PS2_CMD_SET_ALL_MAKE_BREAK = 0xF8,   // all keys make + break, no repeat
    PS2_CMD_SET_ALL_MAKE       = 0xF9,   // all keys make only
    PS2_CMD_SET_ALL_TYPE_MB    = 0xFA,   // all keys typematic + make + break (default)
    PS2_CMD_SET_KEY_TYPEMATIC  = 0xFB,   // + scancode(s): those keys typematic
    PS2_CMD_SET_KEY_MAKE_BREAK = 0xFC,   // + scancode(s): those keys make + break
    PS2_CMD_SET_KEY_MAKE       = 0xFD,   // + scancode(s): those keys make only
    PS2_CMD_RESEND        = 0xFE,   // re-send the last byte (handled in ps2_update)
    PS2_CMD_RESET         = 0xFF,   // reset + self-test
};

// Device-to-host response bytes (keyboard -> host).
enum ps2_response {
    PS2_RSP_ID1         = 0x83,   // second keyboard ID byte (MF2)
    PS2_RSP_SELFTEST_OK = 0xAA,   // basic assurance test passed
    PS2_RSP_ID0         = 0xAB,   // first keyboard ID byte (MF2)
    PS2_RSP_ECHO        = 0xEE,   // reply to PS2_CMD_ECHO
    PS2_RSP_ACK         = 0xFA,   // command acknowledged
    PS2_RSP_RESEND      = 0xFE,   // NAK: parity error, please resend
};

// Argument values of PS2_CMD_SET_SCAN_SET, and the scancode set numbers.
enum ps2_scan_set {
    PS2_SCAN_SET_QUERY = 0,   // report the active set instead of changing it
    PS2_SCAN_SET_1     = 1,
    PS2_SCAN_SET_2     = 2,
    PS2_SCAN_SET_3     = 3,
};

#define PS2_LED_MASK 0x07   // Scroll | Num | Caps bits of the 0xED data byte

// Set-3 per-key mode bits, stored in dev->scs3_keymode[hid] and consulted only
// while current_set == 3 (sets 1/2 have fixed make/break/typematic behavior).
enum ps2_key_mode {
    PS2_KEYMODE_BREAK      = 0x01,   // key emits a break code on release
    PS2_KEYMODE_TYPEMATIC  = 0x02,   // key auto-repeats while held
    PS2_KEYMODE_COLLECTING = 0x80,   // scs3_pending_mode marker: mid 0xFB-0xFD key list
};
#define PS2_KEYMODE_DEFAULT (PS2_KEYMODE_BREAK | PS2_KEYMODE_TYPEMATIC)

// Typematic timing lookup tables, indexed by the 0xF3 rate/delay data byte:
//   repeat period = ps2_typematic_period_us[byte & 0x1F]      (bits 0-4: rate)
//   repeat delay  = ps2_typematic_delay_ms[(byte >> 5) & 0x03] (bits 5-6: delay)
// Values per the PS/2 (IBM AT) typematic spec. The power-on default of 10.9 cps
// / 500 ms is rate index 11 (91743 us) and delay index 1 (500 ms).
static const uint32_t ps2_typematic_period_us[32] = {
     33333,  37453,  41667,  45872,  48309,  54054,  58480,  62500,
     66667,  75188,  83333,  91743, 100000, 108696, 116279, 125000,
    133333, 149254, 166667, 181818, 200000, 217391, 232558, 250000,
    270270, 303030, 333333, 370370, 400000, 434783, 476190, 500000,
};
static const uint16_t ps2_typematic_delay_ms[4] = { 250, 500, 750, 1000 };

void ps2_initialize(ps2_device *dev, PIO pio, uint sm, uint data_pin, uint clock_pin, ps2_rx_handler rx) {
    // Platform-specific: configure the GPIOs and load + start the ps2out PIO
    // program (see ps2_platform_init - implemented per platform so this file has
    // no direct GPIO/PIO-mux dependency). On return the SM is running.
    ps2_platform_init(pio, sm, data_pin, clock_pin);

    dev->pio = pio;
    dev->sm = sm;
    dev->rx = rx;
    dev->queue.head = 0;
    dev->queue.tail = 0;
    dev->idle_since_us = 0;
    dev->busy = 0;
    dev->sent = 0;
    dev->last_tx = 0;
    for (uint i = 0; i < PS2_KEYBOARD_STATE_SIZE_BYTES; i++) {
        dev->prev_keyboard_state[i] = 0;   // no keys held yet
    }
    dev->startup = PS2_STARTUP_INIT;   // announce BAT-complete after the power-on delay
    dev->host_contacted = 0;           // no host byte seen yet

    ps2_set_defaults(dev);   // current_set, LEDs, typematic, scanning -> power-on state
}

void ps2_announce_bat(ps2_device *dev) {
    dev->startup = PS2_STARTUP_INIT;   // re-arm; step 1 re-sends 0xAA after the delay
}

// Dispatch table indexed by [set - 1][make ? 0 : 1]. The per-set make/break
// pointer tables are file-scope globals defined in the hid_to_ps2_set_*.h
// headers included above, so they are visible in this translation unit only.
static const uint8_t *const *const ps2_tables[3][2] = {
    { hid_to_ps2_set_1_make, hid_to_ps2_set_1_break },
    { hid_to_ps2_set_2_make, hid_to_ps2_set_2_break },
    { hid_to_ps2_set_3_make, hid_to_ps2_set_3_break },
};

// Enqueue one key's make/break blob for the current set and drive typematic
// state. `ctrl_held` is the live Ctrl state, needed only to tell Pause from
// Ctrl+Pause (Break). Internal: ps2_update turns a key-state bitmap into these calls.
static bool ps2_send_key(ps2_device *dev, uint32_t now_us, uint8_t hid_code, bool make,
                         bool ctrl_held) {
    if (!dev->scanning_enabled) {
        return false;                        // 0xF5 disabled key output
    }
    if (dev->current_set < PS2_SCAN_SET_1 || dev->current_set > PS2_SCAN_SET_3) {
        return false;                        // guard against an out-of-range set
    }

    const uint8_t *const *make_table  = ps2_tables[dev->current_set - 1][0];
    const uint8_t *const *break_table = ps2_tables[dev->current_set - 1][1];

    // Ctrl+Pause emits "Break" instead of "Pause" in sets 1/2; HID reports the
    // same usage (0x48) either way, so the choice is made here from the live Ctrl
    // state. Both are one-shots: the whole burst goes out on the make and nothing
    // on release, so neither repeats. (Set 3 has an ordinary Pause key, handled
    // by the normal table path below.)
    if (hid_code == HID_KEYCODE_PAUSE &&
        (dev->current_set == PS2_SCAN_SET_1 || dev->current_set == PS2_SCAN_SET_2)) {
        if (!make) {
            return false;                    // one-shot: release produces nothing
        }
        if (ctrl_held) {
            const uint8_t *seq = (dev->current_set == PS2_SCAN_SET_1)
                               ? ps2_ctrl_break_set_1 : ps2_ctrl_break_set_2;
            return ps2_scancode_queue_push(&dev->queue, seq);
        }
        // plain Pause: fall through to the normal make-table lookup below
    }

    if (make) {
        // Typematic eligibility. Sets 1/2 repeat any key that has a break code
        // (make-only keys such as Pause never repeat). Set 3 repeats a key only if
        // its per-key mode has the typematic bit, independent of any break code.
        // Per the spec, only the most recently pressed key repeats, so a new
        // typematic key replaces any earlier one.
        bool repeats = (dev->current_set == PS2_SCAN_SET_3)
                     ? (dev->scs3_keymode[hid_code] & PS2_KEYMODE_TYPEMATIC) != 0
                     : (break_table[hid_code] != NULL);
        if (repeats) {
            dev->repeat_key   = hid_code;
            dev->repeat_at_us = now_us + (uint32_t)dev->delay_ms * 1000u;
        }
        if (make_table[hid_code] == NULL) {
            return false;                    // unassigned key
        }
        return ps2_scancode_queue_push(&dev->queue, make_table[hid_code]);
    }

    // Release: stop repeating only if this is the current typematic key; releasing
    // some other key leaves an earlier-started repeat running. Clear this before
    // the break gate so a Set-3 typematic/no-break key still stops repeating.
    if (hid_code == dev->repeat_key) {
        dev->repeat_key = 0;
    }
    // Set 3 emits a break code only for keys whose per-key mode has the break bit;
    // sets 1/2 always send the break when the table has one.
    if (dev->current_set == PS2_SCAN_SET_3 &&
        (dev->scs3_keymode[hid_code] & PS2_KEYMODE_BREAK) == 0) {
        return false;
    }
    if (break_table[hid_code] == NULL) {
        return false;                        // unassigned key, or make-only (no break)
    }
    return ps2_scancode_queue_push(&dev->queue, break_table[hid_code]);
}

// Diff the current key-state bitmap against the previous one and emit make/break
// events for every change. Internal helper for ps2_update.
static void ps2_send_state(ps2_device *dev, uint32_t now_us, const uint8_t *keyboard_state) {
    // Live Ctrl state (either Left or Right), evaluated against the new bitmap so
    // a Pause pressed in the same cycle sees the current Ctrl. Only Pause uses it
    // (to pick Break vs Pause); every other key ignores it.
    bool ctrl_held =
        ((keyboard_state[HID_KEYCODE_LEFT_CONTROL  >> 3] >> (HID_KEYCODE_LEFT_CONTROL  & 7)) & 1u) ||
        ((keyboard_state[HID_KEYCODE_RIGHT_CONTROL >> 3] >> (HID_KEYCODE_RIGHT_CONTROL & 7)) & 1u);

    // Compare the new bitmap against the previous one byte by byte; each bit that
    // flipped is a make (now set) or break (now clear) event for that HID usage.
    for (uint8_t byte = 0; byte < PS2_KEYBOARD_STATE_SIZE_BYTES; byte++) {
        uint8_t changed = (uint8_t)(keyboard_state[byte] ^ dev->prev_keyboard_state[byte]);
        if (changed != 0) {
            for (uint8_t bit = 0; bit < 8; bit++) {
                if (changed & (1u << bit)) {
                    uint8_t hid  = (uint8_t)((byte << 3) | bit);
                    bool    make = (keyboard_state[byte] >> bit) & 1u;
                    ps2_send_key(dev, now_us, hid, make, ctrl_held);
                }
            }
            dev->prev_keyboard_state[byte] = keyboard_state[byte];
        }
    }
}

uint32_t ps2_encode_frame(uint8_t byte_val) {
    uint32_t parity = 1;
    uint32_t value  = (uint32_t)byte_val;

    for (uint32_t i = 0; i < 8; i++) {
        parity = parity ^ (value >> i & 1);
    }

    // Frame layout (LSB first): start(0), 8 data bits, odd parity, stop(1).
    // ps2out.pio drives the bus through `out pindirs` (open-collector: pindirs=1
    // pulls the line low, pindirs=0 releases it high via the pull-up), so every
    // bit must be inverted before it is pushed to the PIO.
    return ((1 << 10) | (parity << 9) | (value << 1)) ^ 0x7ff;
}

// --- Host command handling ---------------------------------------------------

// Reset the protocol configuration to the power-on defaults (scancode set 2,
// LEDs off, default typematic, scanning enabled). Leaves the PIO/queue alone.
static void ps2_set_defaults(ps2_device *dev) {
    dev->current_set      = PS2_SCAN_SET_2;
    dev->leds             = 0;
    dev->scanning_enabled = 1;
    dev->pending_cmd      = 0;
    dev->repeat_us        = 91743;   // spec default rate: ~10.9 cps
    dev->delay_ms         = 500;     // spec default delay before auto-repeat
    dev->repeat_key       = 0;       // nothing repeating
    for (uint16_t i = 0; i < 256; i++) {
        dev->scs3_keymode[i] = PS2_KEYMODE_DEFAULT;   // set 3: typematic + make/break
    }
    dev->scs3_pending_mode = 0;      // not collecting a 0xFB-0xFD key list
}

// Queue a single response byte to the host (ACK, echo, ID, ...) as a one-byte
// blob, reusing the scancode queue's pacing / inhibit / resend machinery.
static void ps2_queue_byte(ps2_device *dev, uint8_t b) {
    ps2_scancode_queue_push(&dev->queue, PS2_SCANCODE_SEQ(b));
}

// Map a Set-3 mode command (set-all 0xF7-0xFA or set-key 0xFB-0xFD) to its
// stored (typematic, break) mode bits.
static uint8_t ps2_keymode_for_cmd(uint8_t cmd) {
    switch (cmd) {
        case PS2_CMD_SET_ALL_TYPEMATIC:
        case PS2_CMD_SET_KEY_TYPEMATIC:
            return PS2_KEYMODE_TYPEMATIC;                        // repeat, no break

        case PS2_CMD_SET_ALL_MAKE_BREAK:
        case PS2_CMD_SET_KEY_MAKE_BREAK:
            return PS2_KEYMODE_BREAK;                            // break, no repeat

        case PS2_CMD_SET_ALL_MAKE:
        case PS2_CMD_SET_KEY_MAKE:
            return 0;                                            // make only

        default:                                                 // PS2_CMD_SET_ALL_TYPE_MB
            return PS2_KEYMODE_TYPEMATIC | PS2_KEYMODE_BREAK;
    }
}

// The 0xFB-0xFD commands name keys by Set-3 scancode, but the device indexes
// state by HID usage. Set-3 make codes are single bytes, so scan the make table
// for the scancode and return its HID (0 = no match). Only runs during host
// configuration, so the linear scan is cheap enough.
static uint8_t ps2_set3_scancode_to_hid(uint8_t scancode) {
    for (uint16_t hid = 0; hid < 256; hid++) {
        const uint8_t *seq = hid_to_ps2_set_3_make[hid];
        if (seq != NULL && seq[0] == 1 && seq[1] == scancode) {
            return (uint8_t)hid;
        }
    }
    return 0;
}

// Interpret one byte received from the host: change device state, queue the
// required responses, and notify the firmware of state changes. The caller
// (ps2_update) has already handled parity and the PS2_CMD_RESEND request, and
// flushed any pending key output before calling this.
static void ps2_process_host_byte(ps2_device *dev, uint8_t byte) {
    // Mid 0xFB-0xFD key list: each byte below the command range (< 0xED) is a
    // Set-3 key scancode to assign the pending mode to; a command byte ends the
    // list and is processed normally below.
    if (dev->scs3_pending_mode != 0) {
        if (byte < PS2_CMD_SET_LEDS) {
            uint8_t hid = ps2_set3_scancode_to_hid(byte);
            if (hid != 0) {
                dev->scs3_keymode[hid] = dev->scs3_pending_mode & 0x03;
            }
            ps2_queue_byte(dev, PS2_RSP_ACK);
            return;
        }
        dev->scs3_pending_mode = 0;
    }

    // Second byte of a two-byte command (its data argument)?
    if (dev->pending_cmd != 0) {
        uint8_t cmd = dev->pending_cmd;
        dev->pending_cmd = 0;

        switch (cmd) {
            case PS2_CMD_SET_LEDS:  // data is the Scroll/Num/Caps bitmap
                dev->leds = byte & PS2_LED_MASK;
                ps2_queue_byte(dev, PS2_RSP_ACK);
                if (dev->rx != NULL) {
                    dev->rx(PS2_CMD_SET_LEDS, dev->leds);
                }
                break;

            case PS2_CMD_SET_TYPEMATIC:  // decode rate/delay (ps2_update step 6 repeats)
                dev->repeat_us = ps2_typematic_period_us[byte & 0x1F];
                dev->delay_ms  = ps2_typematic_delay_ms[(byte >> 5) & 0x03];
                ps2_queue_byte(dev, PS2_RSP_ACK);
                break;

            case PS2_CMD_SET_SCAN_SET:
                if (byte == PS2_SCAN_SET_QUERY) {   // report the current set
                    ps2_queue_byte(dev, PS2_RSP_ACK);
                    ps2_queue_byte(dev, dev->current_set);
                } else if (byte >= PS2_SCAN_SET_1 && byte <= PS2_SCAN_SET_3) {
                    dev->current_set = byte;
                    ps2_queue_byte(dev, PS2_RSP_ACK);
                    if (dev->rx != NULL) {
                        dev->rx(PS2_CMD_SET_SCAN_SET, dev->current_set);
                    }
                } else {                            // invalid set: ACK and ignore
                    ps2_queue_byte(dev, PS2_RSP_ACK);
                }
                break;

            default:
                break;
        }
        return;
    }

    switch (byte) {
        case PS2_CMD_RESET:  // defaults, ACK now, self-test passed (0xAA) after BAT
            ps2_set_defaults(dev);
            ps2_queue_byte(dev, PS2_RSP_ACK);
            // Re-run the power-on BAT: ACK immediately, but only announce
            // completion with 0xAA once the self-test delay has elapsed, exactly
            // as at power-up. ps2_update step 1 schedules the 0xAA and holds off
            // key scanning until then. bat_due_us is set on the next poll.
            dev->startup = PS2_STARTUP_INIT;
            if (dev->rx != NULL) {
                dev->rx(PS2_CMD_SET_LEDS, dev->leds);   // LEDs were cleared by the reset
            }
            break;

        case PS2_CMD_ECHO:  // echo -> echo (not ACK)
            ps2_queue_byte(dev, PS2_RSP_ECHO);
            break;

        case PS2_CMD_READ_ID:  // ACK, then the 2-byte MF2 keyboard ID
            ps2_queue_byte(dev, PS2_RSP_ACK);
            ps2_queue_byte(dev, PS2_RSP_ID0);
            ps2_queue_byte(dev, PS2_RSP_ID1);
            break;

        case PS2_CMD_SET_LEDS:      // ACK, then wait for the data byte
        case PS2_CMD_SET_SCAN_SET:  // ACK, then wait for the data byte
        case PS2_CMD_SET_TYPEMATIC: // ACK, then wait for the data byte
            dev->pending_cmd = byte;
            ps2_queue_byte(dev, PS2_RSP_ACK);
            break;

        case PS2_CMD_ENABLE:  // enable scanning
            dev->scanning_enabled = 1;
            ps2_queue_byte(dev, PS2_RSP_ACK);
            if (dev->rx != NULL) {
                dev->rx(PS2_CMD_ENABLE, 1);
            }
            break;

        case PS2_CMD_DISABLE:  // disable scanning
            dev->scanning_enabled = 0;
            dev->repeat_key = 0;   // stop any auto-repeat while disabled
            ps2_queue_byte(dev, PS2_RSP_ACK);
            if (dev->rx != NULL) {
                dev->rx(PS2_CMD_DISABLE, 0);
            }
            break;

        case PS2_CMD_SET_DEFAULTS:  // restore defaults (stays enabled)
            ps2_set_defaults(dev);
            ps2_queue_byte(dev, PS2_RSP_ACK);
            if (dev->rx != NULL) {
                dev->rx(PS2_CMD_SET_LEDS, dev->leds);
            }
            break;

        case PS2_CMD_SET_ALL_TYPEMATIC:   // set 3: apply one mode to every key
        case PS2_CMD_SET_ALL_MAKE_BREAK:
        case PS2_CMD_SET_ALL_MAKE:
        case PS2_CMD_SET_ALL_TYPE_MB: {
            uint8_t mode = ps2_keymode_for_cmd(byte);
            for (uint16_t i = 0; i < 256; i++) {
                dev->scs3_keymode[i] = mode;
            }
            ps2_queue_byte(dev, PS2_RSP_ACK);
            break;
        }

        case PS2_CMD_SET_KEY_TYPEMATIC:   // set 3: mode for a following key list
        case PS2_CMD_SET_KEY_MAKE_BREAK:
        case PS2_CMD_SET_KEY_MAKE:
            dev->scs3_pending_mode = PS2_KEYMODE_COLLECTING | ps2_keymode_for_cmd(byte);
            ps2_queue_byte(dev, PS2_RSP_ACK);
            break;

        default:  // unknown command -> ask the host to resend (matches ps2x2pico)
            ps2_queue_byte(dev, PS2_RSP_RESEND);
            break;
    }
}

void ps2_update(ps2_device *dev, uint32_t now_us, const uint8_t *keyboard_state) {
    // 1. Power-on Basic Assurance Test. A real keyboard self-tests at power-up
    //    and only then announces completion with an unsolicited 0xAA, which the
    //    host waits for to detect the keyboard. We have no clock in ps2_initialize,
    //    so capture it on the first poll, then send 0xAA once the delay elapses.
    //    Key scanning is held off until then so the host sees 0xAA first.
    if (dev->startup != PS2_STARTUP_DONE) {
        if (dev->startup == PS2_STARTUP_INIT) {
            dev->bat_due_us = now_us + PS2_BAT_DELAY_US;
            dev->startup = PS2_STARTUP_WAIT;
        }
        if ((int32_t)(now_us - dev->bat_due_us) >= 0) {   // wrap-safe deadline compare
            ps2_queue_byte(dev, PS2_RSP_SELFTEST_OK);
            dev->startup = PS2_STARTUP_DONE;
        }
    } else {
        // 2. Turn the current key-state snapshot into make/break events (queued).
        ps2_send_state(dev, now_us, keyboard_state);
    }

    // 3. Track "busy" from the PIO activity/inhibit IRQ (irq 0, relative to sm).
    if (pio_interrupt_get(dev->pio, dev->sm)) {
        dev->busy = 1;                       // PIO transmitting, or host holding clock low
    } else {
        dev->busy &= 2;                      // keep only the "byte in flight" bit
        // Fail-safe: if the guard has been held longer than a whole frame without
        // the IRQ ever being observed, we missed the window - release it so the
        // send path can't wedge permanently.
        if ((dev->busy & 2) &&
            (uint32_t)(now_us - dev->guard_since_us) >= PS2_TX_GUARD_TIMEOUT_US) {
            dev->busy &= (uint8_t)~2;
        }
    }

    // 4. Host pulled clock low mid-frame to abort our send (irq 4): step the byte
    //    counter back so the same byte is re-sent once the host releases clock.
    if (pio_interrupt_get(dev->pio, dev->sm + 4)) {
        PS2_TRACE(now_us, PS2_TR_NOTE, PS2_TR_NOTE_TX_ABORT);
        if (dev->sent > 0) {
            dev->sent--;
        }
        pio_interrupt_clear(dev->pio, dev->sm + 4);
    }

    // 5. The bus must be idle for >= PS2_INTERBYTE_GAP_US before we may start a
    //    frame. Any busy period restarts the idle window.
    if (dev->busy != 0) {
        dev->idle_since_us = now_us;
    }

    // 6. Typematic auto-repeat: once the delay has elapsed, re-queue the current
    //    key's make code every repeat_us. QMK reports only press and release, so
    //    the device generates the repeats itself. Signed diff is wrap-safe.
    if (dev->repeat_key != 0 && dev->scanning_enabled &&
        (int32_t)(now_us - dev->repeat_at_us) >= 0) {
        const uint8_t *seq = ps2_tables[dev->current_set - 1][0][dev->repeat_key];
        if (seq != NULL) {
            ps2_scancode_queue_push(&dev->queue, seq);
        }
        dev->repeat_at_us = now_us + dev->repeat_us;
    }

    // 7. Send the next byte of the current blob once the idle window has elapsed.
    //    Unsigned subtraction is wrap-safe for the short gap we care about.
    if (!ps2_scancode_queue_empty(&dev->queue) && dev->busy == 0 &&
        (uint32_t)(now_us - dev->idle_since_us) >= PS2_INTERBYTE_GAP_US) {
        const uint8_t *blob = ps2_scancode_queue_peek(&dev->queue);
        if (dev->sent == blob[0]) {          // whole blob sent -> drop it
            dev->sent = 0;
            ps2_scancode_queue_pop(&dev->queue);
        } else {                             // send the next byte
            dev->sent++;
            dev->last_tx = blob[dev->sent];
            dev->busy |= 2;                  // guard the gap until the PIO asserts irq 0
            dev->guard_since_us = now_us;    // start the stale-guard timeout
            pio_sm_put(dev->pio, dev->sm, ps2_encode_frame(dev->last_tx));
            PS2_TRACE(now_us, PS2_TR_TX, dev->last_tx);
        }
    }

    // 8. Receive: a byte from the host preempts pending key output.
    if (!pio_sm_is_rx_fifo_empty(dev->pio, dev->sm)) {
        uint32_t fifo = pio_sm_get(dev->pio, dev->sm) >> 23;  // 9 bits: 8 data + parity

        bool parity = 1;
        for (uint8_t i = 0; i < 8; i++) {
            parity = parity ^ ((fifo >> i) & 1);
        }

        if (parity != ((fifo >> 8) & 1)) {   // bad parity -> ask the host to resend
            PS2_TRACE(now_us, PS2_TR_RX_PERR, (uint8_t)(fifo & 0xFF));
            pio_sm_put(dev->pio, dev->sm, ps2_encode_frame(PS2_RSP_RESEND));
            return;
        }

        uint8_t byte = fifo & 0xFF;
        if (byte == PS2_CMD_RESEND) {        // host asked us to resend the last byte
            PS2_TRACE(now_us, PS2_TR_RESEND, dev->last_tx);
            pio_sm_put(dev->pio, dev->sm, ps2_encode_frame(dev->last_tx));
            return;
        }

        PS2_TRACE(now_us, PS2_TR_RX, byte);
        dev->host_contacted = 1;                 // host is talking to us; stop re-announcing
        ps2_scancode_queue_clear(&dev->queue);   // host command wins: drop queued keys
        dev->sent = 0;
        ps2_process_host_byte(dev, byte);
    }
}