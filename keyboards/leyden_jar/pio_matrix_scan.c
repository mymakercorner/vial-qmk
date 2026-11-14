/* Copyright 2022 Eric Becourt (Rico https://mymakercorner.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "quantum.h"
#include "pio_matrix_scan.h"

/* The pioasm headers are used for their instruction arrays and wrap defines
   only. Their pico-sdk section (struct pio_program, col_*_pio_init helpers) is
   compiled out: the state machine configuration it carried now lives in
   col_sm_config() below, on top of the ChibiOS RP PIO driver. */
#define PICO_NO_HARDWARE 1
#include "col_0_7_pio.pio.h"
#include "col_8_15_pio.pio.h"
#include "col_16_17_pio.pio.h"

/* PIO block and pins. The col_8_15 and col_16_17 programs chain on the absolute
   PIO IRQ flags 0 and 1 raised by the previous program, so the state machine
   indices are kept as they were (0, 1, 2) although nothing depends on them. */
#define MATRIX_PIO_BLOCK RP_PIO0_BLOCK
#define MATRIX_ROW_PIN 0U
#define COL_0_7_PIN 8U
#define COL_8_15_PIN 18U
#define COL_16_17_PIN 26U

/* Each PIO clock tick is 0.1 us whatever the system clock: clkdiv 12.5 on the
   RP2040 at 125 MHz (the value the former pico-sdk float divider produced),
   15.0 on the RP2350 at 150 MHz. */
#define MATRIX_PIO_FREQ_HZ 10000000U

const uint8_t s_TxColScanData[16] = {
    0x01, 0x00,
    0x02, 0x00,
    0x04, 0x00,
    0x08, 0x00,
    0x10, 0x00,
    0x20, 0x00,
    0x40, 0x00,
    0x80, 0x00,
};

const uint32_t s_TxCol2ScanData = ((1 << 0) | (2 << 4));

static uint8_t s_RawMatrixScanValues[18];
static bool s_enable_extra_cols;

static const rp_pio_sm_t *s_sm_col_0_7;
static const rp_pio_sm_t *s_sm_col_8_15;
static const rp_pio_sm_t *s_sm_col_16_17;

static const rp_pio_program_t s_col_0_7_program = {
    .instructions = col_0_7_pio_program_instructions,
    .length       = sizeof(col_0_7_pio_program_instructions) / sizeof(uint16_t),
    .origin       = -1,
};

static const rp_pio_program_t s_col_8_15_program = {
    .instructions = col_8_15_pio_program_instructions,
    .length       = sizeof(col_8_15_pio_program_instructions) / sizeof(uint16_t),
    .origin       = -1,
};

static const rp_pio_program_t s_col_16_17_program = {
    .instructions = col_16_17_pio_program_instructions,
    .length       = sizeof(col_16_17_pio_program_instructions) / sizeof(uint16_t),
    .origin       = -1,
};

/* Configuration shared by the three column programs, same register values as
   the former col_*_pio_init() helpers: OUT pins on the column group, IN pins on
   ROW0-7, TX and RX shifting right with autopull / autopush at the given
   thresholds. */
static void col_sm_config(rp_pio_sm_config_t *cfg, int32_t offset, uint32_t wrap_target, uint32_t wrap, uint32_t col_pin, uint32_t col_count, uint32_t pull_thresh, uint32_t push_thresh) {
    pioSmConfigDefaultX(cfg);
    pioSmConfigSetWrapX(cfg, (uint32_t)offset + wrap_target, (uint32_t)offset + wrap);
    pioSmConfigSetOutPinsX(cfg, pioGpioToRel(MATRIX_PIO_BLOCK, col_pin), col_count);
    pioSmConfigSetInPinsX(cfg, pioGpioToRel(MATRIX_PIO_BLOCK, MATRIX_ROW_PIN));
    pioSmConfigSetFrequencyX(cfg, MATRIX_PIO_FREQ_HZ);
    pioSmConfigSetOutShiftX(cfg, true, true, pull_thresh);
    pioSmConfigSetInShiftX(cfg, true, true, push_thresh);
}

/* Claims a state machine, drives its column pins as outputs and loads its
   configuration. The state machine is left disabled, at the program start. */
static const rp_pio_sm_t *col_sm_start(uint32_t sm_index, const rp_pio_sm_config_t *cfg, int32_t offset, uint32_t col_pin, uint32_t col_count) {
    const rp_pio_sm_t *sm = pioSmAlloc(MATRIX_PIO_BLOCK, sm_index, CORTEX_MAX_KERNEL_PRIORITY, NULL, NULL);
    if (sm == NULL) {
        return NULL;
    }
    pioSmSetConsecutivePindirsX(sm, col_pin, col_count, true);
    pioSmInit(sm, (uint32_t)offset, cfg);
    return sm;
}

void pio_matrix_scan_init(bool enable_extra_cols) {
    s_enable_extra_cols = enable_extra_cols;

    /* Configuration of column pins using ChibiOS hal. Input enable matches what
       the former pico-sdk pio_gpio_init() left set on these pads. */

    iomode_t colums_pin_mode = PAL_RP_GPIO_OE | PAL_RP_PAD_IE | PAL_RP_PAD_SLEWFAST | PAL_RP_PAD_DRIVE8 | PAL_MODE_ALTERNATE_PIO0;
    for (int i=0; i<8; i++) {
        palSetLineMode(i+COL_0_7_PIN, colums_pin_mode);
        wait_us(50);
        palSetLineMode(i+COL_8_15_PIN, colums_pin_mode);
        wait_us(50);
    }

    if (s_enable_extra_cols == true) {
        palSetLineMode(COL_16_17_PIN, colums_pin_mode);
        wait_us(50);
        palSetLineMode(COL_16_17_PIN + 1, colums_pin_mode);
        wait_us(50);
    }

    /* Configuration of row pins using ChibiOS hal */

    iomode_t rows_pin_mode = PAL_RP_PAD_IE | PAL_RP_PAD_SCHMITT | PAL_MODE_ALTERNATE_PIO0;
    for (int i=0; i<8; i++) {
        palSetLineMode(i+MATRIX_ROW_PIN, rows_pin_mode);
    }

    /* Load the PIO programs. Like pico-sdk's pio_add_program(), pioProgramLoad()
       relocates the JMP targets to the load offset; it also takes PIO0 out of
       reset. */

    int32_t offset0 = pioProgramLoad(MATRIX_PIO_BLOCK, &s_col_0_7_program);
    int32_t offset1 = pioProgramLoad(MATRIX_PIO_BLOCK, &s_col_8_15_program);
    int32_t offset2 = 0;
    if (s_enable_extra_cols == true) {
        offset2 = pioProgramLoad(MATRIX_PIO_BLOCK, &s_col_16_17_program);
    }
    if ((offset0 < 0) || (offset1 < 0) || (offset2 < 0)) {
        dprintln("ERROR: matrix scan PIO programs do not fit in PIO0");
        return;
    }

    /* Configure the state machines. Only col_0_7 also maps a SET pin. */

    rp_pio_sm_config_t cfg;

    col_sm_config(&cfg, offset0, col_0_7_pio_wrap_target, col_0_7_pio_wrap, COL_0_7_PIN, 8, 32, 32);
    pioSmConfigSetSetPinsX(&cfg, pioGpioToRel(MATRIX_PIO_BLOCK, COL_0_7_PIN), 1);
    s_sm_col_0_7 = col_sm_start(0, &cfg, offset0, COL_0_7_PIN, 8);

    col_sm_config(&cfg, offset1, col_8_15_pio_wrap_target, col_8_15_pio_wrap, COL_8_15_PIN, 8, 32, 32);
    s_sm_col_8_15 = col_sm_start(1, &cfg, offset1, COL_8_15_PIN, 8);

    if (s_enable_extra_cols == true) {
        col_sm_config(&cfg, offset2, col_16_17_pio_wrap_target, col_16_17_pio_wrap, COL_16_17_PIN, 2, 8, 16);
        s_sm_col_16_17 = col_sm_start(2, &cfg, offset2, COL_16_17_PIN, 2);
    }

    if ((s_sm_col_0_7 == NULL) || (s_sm_col_8_15 == NULL) || ((s_enable_extra_cols == true) && (s_sm_col_16_17 == NULL))) {
        dprintln("ERROR: matrix scan PIO state machines 0-2 are not free");
        return;
    }

    /* Start the state machines */

    pioSmEnableX(s_sm_col_0_7);
    pioSmEnableX(s_sm_col_8_15);
    if (s_enable_extra_cols == true) {
        pioSmEnableX(s_sm_col_16_17);
    }
}

void pio_raw_scan() {
    /* We push FIFO data to the PIO state machine that handles column 16 to 17 first,
     that is the state machine that runs the col_16_17_pio program.
     As this program waits col_8_15_pio to finish no data will be immediately consumed. */

    if (s_enable_extra_cols == true) {

        pioSmPut(s_sm_col_16_17, s_TxCol2ScanData);
    }

    /* We then push FIFO data to the PIO state machine that handles column 8 to 15,
     that is the state machine that runs the col_8_15_pio program.
     As this program waits col_0_7_pio to finish no data will be immediately consumed. */

    pioSmPut(s_sm_col_8_15, *(uint32_t*)(s_TxColScanData + 0));
    pioSmPut(s_sm_col_8_15, *(uint32_t*)(s_TxColScanData + 4));
    pioSmPut(s_sm_col_8_15, *(uint32_t*)(s_TxColScanData + 8));
    pioSmPut(s_sm_col_8_15, *(uint32_t*)(s_TxColScanData + 12));

    /* We last push FIFO data to the PIO state machine that handles column 0 to 7,
     that is the state machine that runs the col_0_7_pio program.
     As soon as data is pushed the program will resume it's execution and consume data.
     This program will awaken the col_8_15_pio program when all columns will be scanned. */

    pioSmPut(s_sm_col_0_7, *(uint32_t*)(s_TxColScanData + 0));
    pioSmPut(s_sm_col_0_7, *(uint32_t*)(s_TxColScanData + 4));
    pioSmPut(s_sm_col_0_7, *(uint32_t*)(s_TxColScanData + 8));
    pioSmPut(s_sm_col_0_7, *(uint32_t*)(s_TxColScanData + 12));

    /* Read results send by col_0_7_pio program to the FIFO */

    *(uint32_t*)(s_RawMatrixScanValues + 0) = pioSmGet(s_sm_col_0_7);
    *(uint32_t*)(s_RawMatrixScanValues + 4) = pioSmGet(s_sm_col_0_7);

    /* Read results send by col_8_15_pio program to the FIFO */

    *(uint32_t*)(s_RawMatrixScanValues + 8) = pioSmGet(s_sm_col_8_15);
    *(uint32_t*)(s_RawMatrixScanValues + 12) = pioSmGet(s_sm_col_8_15);

    // Read results send by col_16_17_pio program to the FIFO
    if (s_enable_extra_cols == true) {
        *(uint16_t*)(s_RawMatrixScanValues + 16) = (uint16_t)(pioSmGet(s_sm_col_16_17)>>16);
    }
}

const uint8_t* pio_get_scan_vals(void) {
    return s_RawMatrixScanValues;
}
