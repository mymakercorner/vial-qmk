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

    // 3. Load the program into the PIO's instruction memory. ps2out_program_init
    //    below only configures the state machine (clkdiv/pins/wrap) - it does not
    //    write instructions - so this add is required or the SM runs uninitialised
    //    memory. ps2out is 32 instructions and fills the memory, so offset is
    //    always 0 and it is the block's only program.
    uint offset = pio_add_program(pio, &ps2out_program);

    // 4. Rewrite instruction 4 from `wait 1 pin, 1` to `wait 1 gpio, clock_pin`.
    //
    //    THIS IS LOAD-BEARING, not an optimisation. ps2out.pio waits for clock to
    //    go high with `wait 1 pin, 1`, but `pin, N` is relative to the state
    //    machine's IN base, and ps2out_program_init sets that base to DATA
    //    (sm_config_set_in_pins(&c, dat) - the `in pins, 1` instructions must read
    //    data). So `pin, 1` assembles to "data + 1" and bakes in an assumption that
    //    clock == data + 1. That is true for upstream ps2x2pico, and QMK's own PIO
    //    PS/2 *host* driver hard-enforces it (#error if data+1 != clock). It is
    //    FALSE here: the PS/2 daughterboard shares the solenoid connector, giving
    //    clock = GP28 and data = GP29, i.e. clock is data MINUS one. Unpatched, the
    //    SM would wait on GP30 and never proceed.
    //
    //    Encoding: WAIT = 0b001 (bits 15:13) | polarity 1 = wait-for-high (bit 7) |
    //    source 0b00 = absolute GPIO (bits 6:5) | index = the GPIO number (bits 4:0).
    //    Same opcode, same single cycle - only the pin addressing mode changes, so
    //    there is no timing effect. Identical to the bare-Pico path in the driver
    //    repo's ps2_platform_pico.c; the patch is portable PIO, only the pin muxing
    //    above differs between the two platforms.
    //
    //    Fragile in one way: the index 4 is a bare literal with no assertion behind
    //    it. If ps2out.pio ever gains or loses an instruction ahead of that wait,
    //    this silently rewrites the WRONG instruction. ps2out.pio carries a matching
    //    "HEADS UP" comment; keep the two in step.
    pio->instr_mem[offset + 4] = (uint16_t)(0x2080u | (clock_pin & 0x1Fu));

    // 5. Configure and start the state machine. ps2out_program_init calls
    //    pio_gpio_init, which is fine under this ChibiOS port.
    ps2out_program_init(pio, sm, offset, data_pin, clock_pin);
}
