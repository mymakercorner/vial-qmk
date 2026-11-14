/*
 * ChibiOS / QMK (VIAL-QMK) implementation of ps2_platform_init (see hid_to_ps2.h)
 * for RP boards. Counterpart to ps2_platform_pico.c: it brings up the same
 * ps2out PIO program, but muxes the pins with the ChibiOS PAL and drives the PIO
 * through the ChibiOS RP PIO driver (rp_pio.h), like
 * keyboards/leyden_jar/pio_matrix_scan.c in the QMK tree.
 *
 * PLACEMENT: this file is QMK-only and lives here (keyboards/leyden_jar/ps2/) in
 * the VIAL-QMK fork. It is NOT part of the verbatim core the sync script copies
 * from the driver repo (the sync never touches it), and it is NOT built by that
 * repo's CMake/host tests.
 *
 * Header portability: QMK no longer builds the pico-sdk. hid_to_ps2.h still
 * includes <hardware/pio.h> for `PIO` and `uint`; in this tree that resolves to
 * ps2/chibios_compat/hardware/pio.h, a minimal stand-in on the ChibiOS registers
 * (a `PIO` is a PIO_TypeDef pointer, pio0/pio1 are PIO0/PIO1).
 */

#include "quantum.h"        // QMK + ChibiOS HAL: palSetLineMode, iomode_t, rp_pio.h
#include "hid_to_ps2.h"     // ps2_platform_init prototype, PIO / uint

// The pioasm header is used for its instruction array and wrap defines only; its
// pico-sdk section (struct pio_program, ps2out_program_init) is compiled out and
// transcribed into ps2_platform_init below.
#define PICO_NO_HARDWARE 1
#include "ps2out.pio.h"     // ps2out_program_instructions, ps2out_wrap*

static const rp_pio_program_t s_ps2out_program = {
    .instructions = ps2out_program_instructions,
    .length       = sizeof(ps2out_program_instructions) / sizeof(uint16_t),
    .origin       = -1,
};

// PIO clock of the ps2out program: 390.625 kHz, i.e. 2.56 us per cycle. That is
// the former fixed clkdiv of 320 at the RP2040's 125 MHz; expressed as a
// frequency it keeps the PS/2 bit timing (set by the program's delay cycles) the
// same at the RP2350's 150 MHz (clkdiv 384 there).
#define PS2OUT_PIO_FREQ_HZ 390625U

static const rp_pio_block_t *ps2_pio_block(PIO pio) {
    return pio == pio1 ? RP_PIO1_BLOCK : RP_PIO0_BLOCK;
}

// State machine configuration of the ps2out program: the register values of the
// former ps2out_program_init (pico-sdk section of ps2out.pio.h).
static void ps2out_sm_config(rp_pio_sm_config_t *c, const rp_pio_block_t *block, int32_t offset, uint data_pin, uint clock_pin) {
    pioSmConfigDefaultX(c);
    pioSmConfigSetWrapX(c, (uint32_t)offset + ps2out_wrap_target, (uint32_t)offset + ps2out_wrap);
    pioSmConfigSetSidesetX(c, 2, true, true);   // .side_set 1 opt pindirs
    pioSmConfigSetFrequencyX(c, PS2OUT_PIO_FREQ_HZ);
    pioSmConfigSetJmpPinX(c, pioGpioToRel(block, clock_pin));
    pioSmConfigSetSetPinsX(c, pioGpioToRel(block, clock_pin), 1);
    pioSmConfigSetSidesetPinsX(c, pioGpioToRel(block, data_pin));
    pioSmConfigSetOutPinsX(c, pioGpioToRel(block, data_pin), 1);
    pioSmConfigSetOutShiftX(c, true, true, 11);
    pioSmConfigSetInPinsX(c, pioGpioToRel(block, data_pin));
    pioSmConfigSetInShiftX(c, true, true, 9);
}

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
    const rp_pio_block_t *block = ps2_pio_block(pio);

    // 1. Claim the state machine. The first allocation on a block also takes it
    //    out of reset. PS/2 runs on PIO1 - PIO0 is the matrix scanner - but the
    //    block is derived from the handle to stay correct either way. No ISR: the
    //    driver core polls the PIO IRQ flags and FIFOs.
    const rp_pio_sm_t *smp = pioSmAlloc(block, sm, CORTEX_MAX_KERNEL_PRIORITY, NULL, NULL);
    if (smp == NULL) {
        dprintln("ERROR: PS/2 PIO state machine is not free");
        return;
    }

    // 2. Mux data + clock to the PIO. Same final pad state as the former
    //    palSetLineMode + pico-sdk pio_gpio_init pair (IE on, OD off).
    ps2_platform_reclaim_pins(pio, data_pin, clock_pin);

    // 3. Load the program into the PIO's instruction memory. ps2out is 32
    //    instructions and fills the memory, so offset is always 0 and it is the
    //    block's only program.
    int32_t offset = pioProgramLoad(block, &s_ps2out_program);
    if (offset < 0) {
        dprintln("ERROR: PS/2 PIO program does not fit");
        pioSmFree(smp);
        return;
    }

    // 4. Rewrite instruction 4 from `wait 1 pin, 1` to `wait 1 gpio, clock_pin`.
    //
    //    THIS IS LOAD-BEARING, not an optimisation. ps2out.pio waits for clock to
    //    go high with `wait 1 pin, 1`, but `pin, N` is relative to the state
    //    machine's IN base, which is DATA (the `in pins, 1` instructions must read
    //    data). So `pin, 1` assembles to "data + 1" and bakes in an assumption that
    //    clock == data + 1. That is true for upstream ps2x2pico, but FALSE here: the
    //    PS/2 daughterboard shares the solenoid connector, giving clock = GP28 and
    //    data = GP29, i.e. clock is data MINUS one. Unpatched, the SM would wait on
    //    GP30 and never proceed.
    //
    //    Encoding: WAIT = 0b001 (bits 15:13) | polarity 1 = wait-for-high (bit 7) |
    //    source 0b00 = GPIO (bits 6:5) | index (bits 4:0). Same opcode, same single
    //    cycle - only the pin addressing mode changes, so there is no timing effect.
    //    On the RP2350 the GPIO index is relative to the block's GPIO window
    //    (GPIOBASE), hence pioGpioToRel (identity on the RP2040 and with base 0).
    //
    //    Fragile in one way: the index 4 is a bare literal with no assertion behind
    //    it. If ps2out.pio ever gains or loses an instruction ahead of that wait,
    //    this silently rewrites the WRONG instruction. ps2out.pio carries a matching
    //    "HEADS UP" comment; keep the two in step.
    block->pio->INSTR_MEM[offset + 4] = (uint16_t)(0x2080u | (pioGpioToRel(block, clock_pin) & 0x1Fu));

    // 5. Configure and start the state machine.
    rp_pio_sm_config_t c;
    ps2out_sm_config(&c, block, offset, data_pin, clock_pin);
    pioSmInit(smp, (uint32_t)offset, &c);
    pioSmEnableX(smp);
}
