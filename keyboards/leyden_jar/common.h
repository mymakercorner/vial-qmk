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

#pragma once

void leyden_jar_init(void);
void leyden_jar_calibrate(int16_t activation_offseet);
void leyden_jar_update(void);
void leyden_jar_enable(bool enable);
bool leyden_jar_is_enabled(void);
void leyden_jar_logical_matrix_scan(matrix_row_t current_matrix[]);
const uint8_t* leyden_jar_physical_matrix_scan(void);

bool leyden_jar_set_detect_levels(void);
bool leyden_jar_get_column_levels(uint16_t col_index, uint16_t* level_buffer_ptr, uint16_t level_buffer_size);
bool leyden_jar_get_dac_threshold(uint16_t* dac_threshold_ptr);
bool leyden_jar_set_dac_threshold(uint16_t dac_threshold);
bool leyden_jar_set_enable_keyboard(uint8_t enable);
bool leyden_jar_get_enable_keyboard(uint8_t* is_enabled);
bool leyden_jar_get_logical_matrix_scan(uint8_t* scan_ptr, uint16_t scan_buffer_size);
bool leyden_jar_get_physical_matrix_scan(uint8_t* scan_ptr, uint16_t scan_buffer_size);
