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

/* RP2040 I2C device 0 is used by QMK API.
 * It is used to drive the the DAC chip that gives the voltage theshold value for the capsense management
 * and the IO expander thats drives the status leds.
*/
#define I2C_DRIVER I2CD0
#define I2C1_SDA_PIN GP16
#define I2C1_SCL_PIN GP17
#define I2C1_CLOCK_SPEED 400000

#define HAPTIC_ENABLE_PIN GP28
#define SOLENOID_PIN GP29
#define SOLENOID_DEFAULT_DWELL 20
#define SOLENOID_MIN_DWELL 4
#define SOLENOID_MAX_DWELL 100

/* The default value is 1 (byte) but it is not enough to store all layout options of the F122
 * VIA/VIAL uses for each layout option:
 * - 1 bit for 2 choices
 * - 2 bits for 3-4 choices
 * - 3 bits for 5-8 choices
 * - 4 bits for 9-16 choices 
 * 
 * In the F122 case we need 11 bits, so 2 bytes to properly store layout information
 */
#define VIA_EEPROM_LAYOUT_OPTIONS_SIZE  2

#define MATRIX_FORMAT_LEYDEN_JAR

#define ACTIVATION_OFFSETS  {7,7,7,7,7,7,7}

#define CONTROLLER_ROWS     8
#define CONTROLLER_COLS     18

#define NB_CAL_BINS         7
#define NB_CUSTOM_CAL_BINS  2

#define CUSTOM_CAL_BIN_KEYS {{0,7,3},{1,6,3}}
