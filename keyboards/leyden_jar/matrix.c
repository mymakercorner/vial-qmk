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
#include "split_util.h"
#include "common.h"
#include "pio_matrix_scan.h"
#include "io_expander.h"

#ifdef SPLIT_KEYBOARD
#    define ROWS_PER_HAND (MATRIX_ROWS / 2)
#else
#    define ROWS_PER_HAND (MATRIX_ROWS)
#endif

matrix_row_t s_previous_matrix[ROWS_PER_HAND];

void matrix_init_custom(void) {
    leyden_jar_init();
    leyden_jar_calibrate();

    for (int i = 0; i < ROWS_PER_HAND; i++) {
        s_previous_matrix[i] = 0;
    }
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool matrix_has_changed = false;

    if (leyden_jar_is_enabled() == true) {
        leyden_jar_logical_matrix_scan(current_matrix);

        for (int row = 0; row < ROWS_PER_HAND; row++) {
            if (s_previous_matrix[row] != current_matrix[row]) {
                matrix_has_changed = true;
            }
            s_previous_matrix[row] = current_matrix[row];
        }

        leyden_jar_update();
    }

    return matrix_has_changed;
}

/*
 * This empty function disables bootmagic lite feature, were plugging in the keyboard while a particuliar key is pressed
 * allows the MCU to go to the bootloader.
 * We do that because when VIA/Vial features are activated this also activate the bootmagic lite feature, even if the
 * value of "bootmagic" field in info.json has the value false.
 *
 * Doing this is particuliarly critical on beam spring boards where an unpopulated pad (or a malfunctioning key) triggers a keypress.
 * It can be quite confusing to try flashing a new firmware and have the keyboard go directly into the bootloader after doing so.
 * As this exact situation occured to Ellipse and me (and was quite hard to figure out) it has been decided to deactivate the feature.
 *
 * There are still several other ways to go the bootloader:
 * - At any time use the key combination configured in the keymap.
 * - Use the Leyden Jar Diagnostic Tool.
 */

void bootmagic_lite(void){}

/*
 * This function overrides the default QMK implementation of left/right keyboard part detection in case of split keyboards.
 * The default implementation is in file 'quantum/split_util.c' declared as weak linking attribute to be able to be overriden on specific keyboards.
 * There is also a bit of simplification in the code done by removing cases scenario that we don't want covered in the Leyden Jar.
 *
 * We removed the SPLIT_HAND_MATRIX_GRID scenario: we cannot do this implementation as the keyboard PCBs are capacitive and also we don't have control on how the PCB is designed.
 *
 * We also removed the EE_HANDS scenario: using this feature looks convoluted emough on AVR CPUs to discourage average users and it looks even worse when using RP2040 MCUs.
 * Really removing the possibility to drag and drop uf2 files in the RP2040 boot disk folder and replacing it by complex command line commands is not at all user friendly.
 *
 * The scenarios that are still managed and available by this function are:
 *
 * 1) Decide at compile time if left or right part is the master (the master must be connected to the PC by USB).
 *
 * 2) Detect the left or right part by reading an IO pin and taking the decision based on the read level (low or high).
 * In this case either the left or right part can be the master (that is connected to the PC by USB).
 * This is the scenario where this function override really does its job.
 * We'd like to use one of the extra columns IO pins GP26/GP27 for this but we dont want to change how those IO pins are wired controller side.
 * So it is difficult for us to drive this IO pin high if wanted.
 * Default implentation declares this IO pin with no internal pullups or pulldown resitors enabled.
 * On our implementation we declare this IO pin as input with internal pullup activated so by default the IO pin is in a high state.
 * Pulling this IO pin low is then very simple: just connect IO pin hole to ground, this can be done fron the keyboard PCB for example.
 */

bool is_keyboard_left_impl(void) {
#if defined(SPLIT_HAND_PIN)
    setPinInputHigh(SPLIT_HAND_PIN);
    wait_us(100);
    // Test pin SPLIT_HAND_PIN for High/Low, if low it's right hand
#    ifdef SPLIT_HAND_PIN_LOW_IS_LEFT
    return !readPin(SPLIT_HAND_PIN);
#    else
    return readPin(SPLIT_HAND_PIN);
#    endif
#elif defined(MASTER_RIGHT)
    return !is_keyboard_master();
#else
    return is_keyboard_master();
#endif
}
