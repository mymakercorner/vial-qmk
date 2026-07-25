/*
 * ChibiOS / QMK (VIAL-QMK) implementation of ps2_platform_init (see hid_to_ps2.h)
 * for RP2040 boards. Counterpart to ps2_platform_pico.c: it brings up the same
 * ps2out PIO program, but muxes the pins with the ChibiOS PAL and takes the PIO
 * block out of reset the ChibiOS way, mirroring the idioms already used by
 * keyboards/leyden_jar/pio_matrix_scan.c in the QMK tree.
 *
 * PLACEMENT: this file is QMK-only and lives here (keyboards/leyden_jar/ps2/) in
 * the VIAL-QMK fork. It is NOT part of the verbatim core the sync script copies
 * from the driver repo (the sync never touches it), and it is NOT built by that
 * repo's CMake/host tests. Symbols/macros below were verified against the fork:
 *   - RESETS_ALLREG_PIO0/1                    lib/chibios/.../RP2040/rp2040.h
 *   - PAL_RP_PAD_* / PAL_MODE_ALTERNATE_PIO1  lib/chibios/.../GPIOv1/hal_pal_lld.h
 *   - pio_gpio_init works under this port     col_*_pio_init in the matrix scanner
 *
 * Header portability: hid_to_ps2.h no longer includes <pico/stdlib.h> (the core
 * needs only PIO + uint, from <hardware/pio.h>), so it compiles under ChibiOS.
 * This file gets palSetLineMode / hal_lld_peripheral_unreset / iomode_t from
 * quantum.h, and the PIO API from <hardware/pio.h>.
 */

#include "quantum.h"        // QMK + ChibiOS HAL: palSetLineMode, hal_lld_peripheral_unreset, iomode_t
#include "hardware/pio.h"   // PIO, pio_add_program, pio0/pio1, uint
#include "hid_to_ps2.h"     // ps2_platform_init prototype
#include "ps2out.pio.h"     // ps2out_program, ps2out_program_init

// Pad configuration for both PS/2 lines: input-enabled (we read both), Schmitt
// trigger, 2 mA drive, default (slow) slew - matching ps2_platform_pico.c's pad
// setup - plus the alternate-function select that hands the pad to the PIO block.
// No internal pulls: the PS/2 bus is open-collector with external pull-ups.
// Direction is the PIO's job (pindirs).
static inline iomode_t ps2_pad_mode(PIO pio) {
    return PAL_RP_PAD_IE | PAL_RP_PAD_SCHMITT | PAL_RP_PAD_DRIVE2 |
           (pio == pio1 ? PAL_MODE_ALTERNATE_PIO1 : PAL_MODE_ALTERNATE_PIO0);
}

// Re-assert PIO ownership of the two PS/2 pads without disturbing the running
// state machine. Needed because QMK's haptic subsystem shares these pins and can
// steal one back with gpio_set_pin_output() long after ps2_platform_init ran -
// see the notify_usb_device_state_change_kb() hook in ps2_glue.c for the exact
// path. Writing the same pad/ctrl values the pins already hold is a no-op, so
// this is safe to call at any time, including from interrupt context.
void ps2_platform_reclaim_pins(PIO pio, uint data_pin, uint clock_pin) {
    const iomode_t mode = ps2_pad_mode(pio);
    palSetLineMode(data_pin, mode);
    palSetLineMode(clock_pin, mode);
}

void ps2_platform_init(PIO pio, uint sm, uint data_pin, uint clock_pin) {
    // 1. Bring the PIO block out of reset. ChibiOS holds peripherals in reset
    //    until asked (the Pico SDK build does this implicitly). PS/2 runs on PIO1
    //    - PIO0 is the matrix scanner - but derive it from the handle to stay
    //    correct either way. Cf. pio_matrix_scan_init(): hal_lld_peripheral_unreset
    //    (RESETS_ALLREG_PIO0).
    hal_lld_peripheral_unreset(pio == pio1 ? RESETS_ALLREG_PIO1 : RESETS_ALLREG_PIO0);

    // 2. Mux data + clock to the PIO. ps2out_program_init's pio_gpio_init finalises
    //    the pad for PIO use below, exactly as the matrix's col_*_pio_init does
    //    after its own palSetLineMode.
    ps2_platform_reclaim_pins(pio, data_pin, clock_pin);

    // 3. Load the program, patch the clock-wait to an ABSOLUTE gpio (identical to
    //    the pico path - the patch is portable PIO, only the pin mux differs), then
    //    configure + start the state machine. ps2out_program_init calls
    //    pio_gpio_init, which is fine under this ChibiOS port.
    uint offset = pio_add_program(pio, &ps2out_program);
    pio->instr_mem[offset + 4] = (uint16_t)(0x2080u | (clock_pin & 0x1Fu));
    ps2out_program_init(pio, sm, offset, data_pin, clock_pin);
}
