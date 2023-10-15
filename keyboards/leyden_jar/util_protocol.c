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
#include "util_protocol.h"
#include "common.h"

//#ifdef VIA_ENABLE

void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    uint8_t command_get_set = data[0];
    uint8_t command_id = data[1];
    uint8_t* command_payload = data + 4;

    bool protocol_answer_ok = false;

    uint16_t magic_id = *(uint16_t *)(data + 2);
    if (magic_id == LEYDEN_JAR_PROTOCOL_MAGIC)
    {
        switch ( command_get_set ) {
            case id_get_keyboard_value: {
                switch( command_id ) {
                    case id_leyden_jar_protocol_version: {
                        uint8_t* major_ptr = command_payload;
                        uint8_t* mid_ptr = command_payload + 1;
                        uint16_t* minor_ptr = (uint16_t *)(command_payload + 2);
                        *major_ptr = LEYDEN_JAR_PROTOCOL_MAJOR;
                        *mid_ptr = LEYDEN_JAR_PROTOCOL_MID;
                        *minor_ptr = LEYDEN_JAR_PROTOCOL_MINOR;
                        protocol_answer_ok = true;
                        break;
                    }
                    case id_leyden_jar_details: {
                        command_payload[0] = MATRIX_ROWS;
                        command_payload[1] = MATRIX_COLS;
                        command_payload[2] = CONTROLLER_ROWS;
                        command_payload[3] = CONTROLLER_COLS;
                        command_payload[4] = (leyden_jar_is_beamspring() == true) ? 1 : 0;
                        command_payload[5] = NB_CAL_BINS;
                        protocol_answer_ok = true;
                        break;
                    }
                    case id_leyden_jar_matrix_mapping: {
                        command_payload[0] = leyden_jar_get_matrix_to_controller_type();
                        leyden_jar_get_matrix_to_controller_cols(command_payload + 1, CONTROLLER_COLS);
                        leyden_jar_get_matrix_to_controller_rows(command_payload + 1 + CONTROLLER_COLS, CONTROLLER_ROWS);
                        protocol_answer_ok = true;
                        break;
                    }
                    case id_leyden_jar_bin_map: {
                        uint16_t col_index = *(uint16_t *)command_payload;
                        uint8_t *bin_map_ptr = command_payload + 2;
                        uint16_t max_buffer_size = (length - 4) / sizeof(uint8_t);
                        protocol_answer_ok = leyden_jar_get_column_bin_map(col_index, bin_map_ptr, max_buffer_size);
                        break;
                    }
                    case id_leyden_jar_col_levels: {
                        uint16_t col_index = *(uint16_t *)command_payload;
                        uint16_t *col_level_ptr = (uint16_t *)(command_payload + 2);
                        uint16_t max_buffer_size = (length - 4) / sizeof(uint16_t);
                        protocol_answer_ok = leyden_jar_get_column_levels(col_index, col_level_ptr, max_buffer_size);
                        break;
                    }
                    case id_leyden_jar_logical_matrix_row: {
                        uint16_t row_index = *(uint16_t *)command_payload;
                        protocol_answer_ok = leyden_jar_get_logical_matrix_row((uint32_t*)(command_payload + 4), row_index);
                        break;
                    }
                    case id_leyden_jar_physical_matrix_vals: {
                        uint16_t max_buffer_size = (length - 4) / sizeof(uint8_t);
                        protocol_answer_ok = leyden_jar_get_physical_matrix_values(command_payload, max_buffer_size);
                        break;
                    }
                    case id_leyden_jar_dac_threshold: {
                        uint16_t bin_number = *((uint16_t *)command_payload);
                        uint16_t *threshold_ptr = (uint16_t *)(command_payload + 2);
                        protocol_answer_ok = leyden_jar_get_dac_threshold(threshold_ptr, bin_number);
                        break;
                    }
                    case id_leyden_jar_dac_ref_level: {
                        uint16_t bin_number = *((uint16_t *)command_payload);
                        uint16_t *ref_level_ptr = (uint16_t *)(command_payload + 2);
                        protocol_answer_ok = leyden_jar_get_dac_ref_level(ref_level_ptr, bin_number);
                        break;
                    }
                    case id_leyden_jar_enable_keyboard: {
                        uint8_t* enable_status_ptr = command_payload;
                        protocol_answer_ok = leyden_jar_get_enable_keyboard(enable_status_ptr);
                        break;
                    }
                }
                break;
            }
            case id_set_keyboard_value: {
                switch( command_id ) {
                    case id_leyden_jar_dac_threshold: {
                        uint16_t threshold = *(uint16_t *)command_payload;
                        uint8_t bin_number = *(command_payload + 2);
                        protocol_answer_ok = leyden_jar_set_dac_threshold(threshold, bin_number);
                        break;
                    }
                    case id_leyden_jar_enable_keyboard: {
                        uint8_t enable_status = *command_payload;
                        protocol_answer_ok = leyden_jar_set_enable_keyboard(enable_status);
                        break;
                    }
                    case id_leyden_jar_detect_levels: {
                        protocol_answer_ok = leyden_jar_set_detect_levels();
                        break;
                    }
                    case id_leyden_jar_scan_logical_matrix: {
                        protocol_answer_ok = leyden_jar_set_scan_logical_matrix();
                        break;
                    }
                    case id_leyden_jar_scan_physical_matrix: {
                        protocol_answer_ok = leyden_jar_set_scan_physical_matrix();
                        break;
                    }
                    case id_leyden_jar_enter_bootloader: {
                        reset_keyboard();
                        // we should not be reaching the code after this point
                        break;
                    }
                    case id_leyden_jar_reboot: {
                        soft_reset_keyboard();
                        // we should not be reaching the code after this point
                        break;
                    }
                    case id_leyden_jar_erase_eeprom: {
                        eeconfig_disable();
                        soft_reset_keyboard();
                        // we should not be reaching the code after this point
                        break;
                    }
                }
                break;
            }
        }
    }

    if (protocol_answer_ok == false) {
        data[0] = id_unhandled;
    }
}

//#endif
