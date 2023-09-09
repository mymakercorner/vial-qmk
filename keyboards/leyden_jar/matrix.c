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
#include "common.h"
#include "pio_matrix_scan.h"
#include "io_expander.h"

//static const uint8_t s_physicalToUniversalCol[18] =
//{
//    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 255, 255, 255, 255, 255, 10, 255, 255
//};
//static const uint8_t s_physicalToUniversalRow[8] =
//{
//    5, 6, 4, 7, 3, 2, 1, 0
//};
//#define PHYSICAL_TO_UNIVERSAL_COL(col) s_physicalToUniversalCol[col]
//#define PHYSICAL_TO_UNIVERSAL_ROW(row) s_physicalToUniversalRow[row]

#if defined(MATRIX_FORMAT_IS_UNIVERSAL)

    #if defined(BOARD_MODEL_IS_F77) || defined(BOARD_MODEL_IS_F62)

    static const uint8_t s_matrixToControllerCol[18] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 15, 255, 255, 255, 255, 255, 255, 255 };
    static const uint8_t s_matrixToControllerRow[8] = { 7, 6, 5, 4, 2, 0, 1, 3 };

    #endif

#elif defined(MATRIX_FORMAT_IS_NATIVE)

#if 0

    static const uint8_t s_matrixToControllerCol[18] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
    static const uint8_t s_matrixToControllerRow[8] = { 7, 6, 5, 4, 3, 2, 1, 0 };

#else

    static const uint8_t s_matrixToControllerCol[18] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
    static const uint8_t s_matrixToControllerRow[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

#endif

#endif

#define MATRIX_TO_CONTROLLER_COL(col) s_matrixToControllerCol[col]
#define MATRIX_TO_CONTROLLER_ROW(row) s_matrixToControllerRow[row]

matrix_row_t s_previous_matrix[MATRIX_ROWS];

void matrix_init_custom(void) {
    leyden_jar_init();

    leyden_jar_calibrate(ACTIVATION_OFFSET);

    for (int i = 0; i < MATRIX_ROWS; i++) {
        s_previous_matrix[i] = 0;
    }
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool matrix_has_changed = false;

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

    for (int row = 0; row < MATRIX_ROWS; row++) {
        if (s_previous_matrix[row] != current_matrix[row]) {
            matrix_has_changed = true;
        }
        s_previous_matrix[row] = current_matrix[row];
    }

    leyden_jar_update();

    return matrix_has_changed;
}
