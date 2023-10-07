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

#pragma once

#define LEYDEN_JAR_PROTOCOL_MAJOR   0
#define LEYDEN_JAR_PROTOCOL_MID     9
#define LEYDEN_JAR_PROTOCOL_MINOR   0
#define LEYDEN_JAR_PROTOCOL_MAGIC   0x21C0

enum leyden_jar_keyboard_value_id {
    id_leyden_jar_offset = 0x80, //Sufficiently high value to be safe to use
    id_leyden_jar_protocol_version,
    id_leyden_jar_details,
    id_leyden_jar_enable_keyboard,
    id_leyden_jar_detect_levels,
    id_leyden_jar_dac_threshold,
    id_leyden_jar_col_levels,
    id_leyden_jar_scan_logical_matrix,
    id_leyden_jar_scan_physical_matrix,
    id_leyden_jar_enter_bootloader,
    id_leyden_jar_reboot,
    id_leyden_jar_erase_eeprom,
    id_leyden_jar_logical_matrix_row,
    id_leyden_jar_physical_matrix_vals,
    id_leyden_jar_matrix_mapping
};
