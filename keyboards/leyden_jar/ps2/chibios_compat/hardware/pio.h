/*
 * Minimal stand-in for pico-sdk's <hardware/pio.h> on the ChibiOS RP port.
 *
 * The portable PS/2 device driver core (hid_to_ps2.c / hid_to_ps2.h) stays
 * byte-identical to the driver repo's copy and keeps including <hardware/pio.h>.
 * QMK no longer builds the pico-sdk, so this header provides exactly the pico-sdk
 * surface the core uses, implemented on the ChibiOS RP register definitions:
 *   - the `PIO` handle type and the `uint` typedef,
 *   - pio0 / pio1 (/ pio2 on the RP2350),
 *   - pio_interrupt_get / pio_interrupt_clear (PIO IRQ flags 0-7),
 *   - pio_sm_put / pio_sm_get / pio_sm_is_rx_fifo_empty (non-blocking FIFO access).
 * Semantics follow pico-sdk: a `PIO` is a pointer to the block's registers.
 *
 * Only the PS/2 variants put this directory on the include path (EXTRAINCDIRS in
 * their rules.mk). Anything beyond the list above must use the ChibiOS RP PIO
 * driver (rp_pio.h) directly, as ps2_platform_chibios.c does.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <hal.h>

typedef unsigned int uint;
typedef PIO_TypeDef *PIO;

#define pio0 PIO0
#define pio1 PIO1
#if RP_HAS_PIO2 == TRUE
#    define pio2 PIO2
#endif

static inline bool pio_interrupt_get(PIO pio, uint pio_interrupt_num) {
    return (pio->IRQ & (1u << pio_interrupt_num)) != 0u;
}

static inline void pio_interrupt_clear(PIO pio, uint pio_interrupt_num) {
    pio->IRQ = 1u << pio_interrupt_num; // write 1 to clear
}

static inline void pio_sm_put(PIO pio, uint sm, uint32_t data) {
    pio->TXF[sm] = data;
}

static inline uint32_t pio_sm_get(PIO pio, uint sm) {
    return pio->RXF[sm];
}

static inline bool pio_sm_is_rx_fifo_empty(PIO pio, uint sm) {
    return (pio->FSTAT & (1u << (8u + sm))) != 0u; // FSTAT.RXEMPTY, bits 11:8
}
