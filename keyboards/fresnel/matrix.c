/* Copyright 2023 Eric Becourt (Rico https://mymakercorner.com)
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
#include "dac.h"

static matrix_row_t s_previous_matrix[MATRIX_ROWS];

//static uint8_t s_col_io_pins[MATRIX_COLS] = MATRIX_COL_PINS;
//static uint8_t s_row_io_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;

//#define MATRIX_COLS 18
//#define MATRIX_ROWS 6

static uint8_t s_col_io_pins[MATRIX_COLS] = {GP1, GP2, GP3, GP4, GP5, GP8, GP9, GP10, GP11, GP12, GP13, GP14, GP15, GP0, GP17, GP18, GP26, GP27};
static uint8_t s_row_io_pins[MATRIX_ROWS] = {GP25, GP24, GP28, GP29, GP23, GP22};

void matrix_init_custom(void) {
    dac_init();

    //dac_write_val(8); // This should translate to ~50mV
    dac_write_val(4); // This should translate to ~25mV
    for (int i = 0; i < MATRIX_ROWS; i++) {
        s_previous_matrix[i] = 0;
    }

    for (int i = 0; i < MATRIX_COLS; i++) {
        setPinOutput(s_col_io_pins[i]);
        writePinLow(s_col_io_pins[i]);
    }

    for (int i = 0; i < MATRIX_ROWS; i++) {
        setPinInput(s_row_io_pins[i]);
    }

    wait_ms(100);
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool matrix_has_changed = false;

    for (int row = 0; row < MATRIX_ROWS; row++) {
        current_matrix[row] = 0;
    }

    for (int col = 0; col < MATRIX_COLS; col++) {
        writePinHigh(s_col_io_pins[col]);
        wait_us(40);

        for (int row = 0; row < MATRIX_ROWS; row++) {
            matrix_row_t rowVal = readPin(s_row_io_pins[row]);
            rowVal = (~rowVal) & 1;
            rowVal = rowVal << col;
            current_matrix[row] |= rowVal;
        }

        writePinLow(s_col_io_pins[col]);
        wait_us(40);
    }

    for (int row = 0; row < MATRIX_ROWS; row++) {
        if (s_previous_matrix[row] != current_matrix[row]) {
            matrix_has_changed = true;
        }
        s_previous_matrix[row] = current_matrix[row];
    }

    return matrix_has_changed;
}
