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

#include <stdlib.h>
#include "quantum.h"
#include "haptic.h"
#include "common.h"
#include "dac.h"
#include "io_expander.h"
#include "pio_matrix_scan.h"

#define UNCONNECTED_LEVEL 380

static uint16_t s_matrix_levels[CONTROLLER_COLS][CONTROLLER_ROWS];
static uint16_t s_sorted_levels[CONTROLLER_COLS * CONTROLLER_ROWS];
static uint16_t s_dac_threshold;
static bool s_is_keyboard_enabled;
static matrix_row_t s_logical_matrix_scan[MATRIX_ROWS];

#if defined(MATRIX_FORMAT_XWHATSIT)

    #if defined(BOARD_MODEL_IS_F77) || defined(BOARD_MODEL_IS_F62)

    static const uint8_t s_matrixToControllerCol[18] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 15, 255, 255, 255, 255, 255, 255, 255 };
    static const uint8_t s_matrixToControllerRow[8] = { 7, 6, 5, 4, 2, 0, 1, 3 };
    static const uint8_t s_matrixLayout = MATRIX_LAYOUT_IS_XWHATSIT;

    #endif

#elif defined(MATRIX_FORMAT_LEYDEN_JAR)

    static const uint8_t s_matrixToControllerCol[18] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
    static const uint8_t s_matrixToControllerRow[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    static const uint8_t s_matrixLayout = MATRIX_LAYOUT_IS_LEYDEN_JAR;

#endif

#define MATRIX_TO_CONTROLLER_COL(col) s_matrixToControllerCol[col]
#define MATRIX_TO_CONTROLLER_ROW(row) s_matrixToControllerRow[row]

static void leyden_jar_detect_levels(void) {
    for (int col = 0; col < CONTROLLER_COLS; col++) {
        for (int row=0; row<CONTROLLER_ROWS; row++) {
            s_matrix_levels[col][row] = 0;
        }
    }

    for (uint16_t dac_val=350; dac_val < 500; dac_val++) {
        dac_write_val(dac_val);
        wait_us(100);
        pio_raw_scan();

        const uint8_t* p_raw_vals = pio_get_scan_vals();

        for (int col = 0; col < CONTROLLER_COLS; col++) {
            for (int row = 0; row < CONTROLLER_ROWS; row++) {
                if (p_raw_vals[col] & (uint8_t)(1<<row)) {
                    s_matrix_levels[col][row] = dac_val;
                }
            }
        }
    }

    if (s_dac_threshold != 0)
    {
        dac_write_val(s_dac_threshold);
        wait_us(100);
    }
}

static int leyden_jar_compare_vals(const void* pVal1, const void* pVal2) {
    if (*(uint16_t*)pVal1 < *(uint16_t*)pVal2) {
        return -1;
    }
    else if (*(uint16_t*)pVal1 > *(uint16_t*)pVal2) {
        return 1;
    }
    else {
        return 0;
    }

}

static void leyden_jar_sort_level_values(void) {
    for (int col = 0; col < CONTROLLER_COLS; col++) {
        for (int row = 0; row < CONTROLLER_ROWS; row++) {
            s_sorted_levels[col * CONTROLLER_ROWS + row] = s_matrix_levels[col][row];
        }
    }

    qsort((void*)s_sorted_levels, CONTROLLER_COLS * CONTROLLER_ROWS, sizeof(uint16_t), leyden_jar_compare_vals);
}

/* We compute the threshold to were we consider that a key has been pressed.
 *
 * First step:
 * We detect the maximum level for all keys in the switch matrix and we sort those values in ascending order.
 * This will later be useful to retrieve the median and maximum values.
 *
 * Scenario 1:
 *   We try to manage the case were the controller is powered on while no columns/rows are soldered to a key matrix.
 *   This happens when you want to pre-flash the controller firmware before soldering the controller PCB
 *   to the key matrix.
 *   If we do nothing the controller can spam fake keypresses to the computer.
 *   We prevent this by taking the maximum detected level and comparing it to a low reference value.
 *   If the maximum detected level is equal or lower than this reference value then we know that no soldering to a
 *   key matrix have been done.
 *   We then configure the dac threshold with a value that will never trigger a key press:
 *     - 1023 for Model F keyboards.
 *     - 300 for Beamspring keyboards.
 *
 * Scenario  2:
 *   We detected that the controller PCB was properly soldered.
 *   The dac threshold value is then the median value of all detected levels with a fixed offset added to it.
 *   The offset value is defined by the ACTIVATION_OFFSET macro defined outside this file.
 *   The offset is positive for Model F keyboards and negative for Beamspring keyboards.
 *   Using the median value of all detected values has several advantages:
 *     - It gives close to an average value of levels.
 *     - It allows to ignore uninstalled flippers in the key matrix (lowest values).
 *     - It allows to ignore calibration pads in the key matrix (lowest or highest values).
 *     - It allows to ignore several pressed keys during boot up (does not work if all keys are pressed).
 *     - Consequently allows to use QMK BOOT_MAGIC lite feature. */

static void leyden_jar_compute_dac_threshold(int16_t activation_offset) {
    uint16_t median_val = s_sorted_levels[CONTROLLER_COLS * CONTROLLER_ROWS / 2];
    uint16_t max_val = s_sorted_levels[(CONTROLLER_COLS * CONTROLLER_ROWS) - 1];

    if (max_val <= UNCONNECTED_LEVEL) {
        if (activation_offset > 0) {
            s_dac_threshold = 1023;
        } else {
            s_dac_threshold = 300;
        }

    } else {
        s_dac_threshold = (uint16_t)((int16_t)median_val + activation_offset);
    }

}

void leyden_jar_init(void) {
    dac_init();
    io_expander_init();
    pio_matrix_scan_init(CONTROLLER_COLS == 18);

    s_dac_threshold = 0;
    s_is_keyboard_enabled = true;
}

void leyden_jar_calibrate(int16_t activation_offset) {
    leyden_jar_detect_levels();
    leyden_jar_sort_level_values();
    leyden_jar_compute_dac_threshold(activation_offset);
    dac_write_val(s_dac_threshold);
    wait_us(100);
}

void leyden_jar_update(void) {
    io_expander_update_state();
}

void leyden_jar_enable(bool enable) {
    s_is_keyboard_enabled = enable;
}

bool leyden_jar_is_enabled() {
    return s_is_keyboard_enabled;
}

void leyden_jar_logical_matrix_scan(matrix_row_t current_matrix[]) {
    pio_raw_scan();
    const uint8_t* p_raw_vals = pio_get_scan_vals();

    for (int row = 0; row < MATRIX_ROWS; row++) {
        current_matrix[row] = 0;
    }

    for (int col = 0; col < MATRIX_COLS; col++) {
        int physCol = (int)MATRIX_TO_CONTROLLER_COL(col);
        for (int row = 0; row < MATRIX_ROWS; row++) {
            int physicalRow = (int)MATRIX_TO_CONTROLLER_ROW(row);
            matrix_row_t rowVal = (matrix_row_t)((p_raw_vals[physCol] >> physicalRow) & 1);
            #ifdef BEAMSPRING_KEYBOARD
                rowVal = (~rowVal) & 1;
            #endif
            rowVal = rowVal << col;
            current_matrix[row] |= rowVal;
        }
    }
}

bool leyden_jar_set_detect_levels() {
    leyden_jar_detect_levels();

    return true;
}

bool leyden_jar_get_column_levels(uint16_t col_index, uint16_t* level_buffer_ptr, uint16_t level_buffer_size) {
    if (col_index >= CONTROLLER_COLS || level_buffer_size < CONTROLLER_ROWS) {
        return false;
    }

    for (int i=0; i<CONTROLLER_ROWS; i++) {
        level_buffer_ptr[i] = s_matrix_levels[col_index][i];
    }

    return true;
}

bool leyden_jar_get_dac_threshold(uint16_t* dac_threshold_ptr) {
    *dac_threshold_ptr = s_dac_threshold;
    return true;
}

bool leyden_jar_set_dac_threshold(uint16_t dac_threshold) {
    if (dac_threshold > 1023) {
        return false;
    }

    s_dac_threshold = dac_threshold;
    dac_write_val(s_dac_threshold);
    wait_us(100);

    return true;
}

bool leyden_jar_set_enable_keyboard(uint8_t enable) {
    leyden_jar_enable(enable != 0);

    return true;
}

bool leyden_jar_get_enable_keyboard(uint8_t* is_enabled) {
    *is_enabled = ( leyden_jar_is_enabled() == true ) ? 1 : 0;

    return true;
}

bool leyden_jar_set_scan_logical_matrix() {
    leyden_jar_logical_matrix_scan(s_logical_matrix_scan);

    return true;
}

bool leyden_jar_set_scan_physical_matrix(void) {
    pio_raw_scan();

    return true;
}

bool leyden_jar_get_logical_matrix_row(uint32_t* logical_row_ptr, uint8_t row_index) {
    if (row_index >= MATRIX_ROWS) {
        return false;
    }

    *logical_row_ptr = (uint32_t)s_logical_matrix_scan[row_index];

    return true;
}

bool leyden_jar_get_physical_matrix_values(uint8_t* scan_ptr, uint16_t scan_buffer_size) {
    if (scan_buffer_size < CONTROLLER_COLS) {
        return false;
    }

    const uint8_t* p_raw_vals = pio_get_scan_vals();
    memcpy(scan_ptr, p_raw_vals, CONTROLLER_COLS);

    return true;
}

bool leyden_jar_get_matrix_to_controller_cols(uint8_t* matrix_to_controller_cols, uint8_t buffer_size)
{
    if (buffer_size < CONTROLLER_COLS) {
        return false;
    }

    memcpy (matrix_to_controller_cols, s_matrixToControllerCol, CONTROLLER_COLS);

    return true;
}

bool leyden_jar_get_matrix_to_controller_rows(uint8_t* matrix_to_controller_rows, uint8_t buffer_size)
{
    if (buffer_size < CONTROLLER_ROWS) {
        return false;
    }

    memcpy (matrix_to_controller_rows, s_matrixToControllerRow, CONTROLLER_ROWS);

    return true;
}

uint8_t leyden_jar_get_matrix_to_controller_type(void)
{
    return s_matrixLayout;
}



void via_init_kb(void) {
    if (!via_eeprom_is_valid()) {
        eeconfig_disable();
    }
}
