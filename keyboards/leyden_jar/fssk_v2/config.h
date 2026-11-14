/* Copyright 2026 Eric Becourt (Rico https://mymakercorner.com)
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

/* FSSK (Model F Labs, Round 2) on the Leyden Jar V2 controller (RP2350B, QFN-80).
   Prototype bring-up target. Compared with the Rev 3 targets:
   - no PCA9557 IO expander: the J2 LEDs and the PS/2 daughterboard presence pin are
     plain GPIOs (see the LEYDEN_JAR_V2 part of io_expander.c);
   - 8 MB flash (RP_FLASH_SIZE in mcuconf.h), wear-leveling EEPROM at the top of it;
   - the core runs from an external 1.1 V LDO; the on-chip regulator is left as the
     bootrom configured it (no overclocking, nothing to program). */
#define LEYDEN_JAR_V2
#define LEYDEN_JAR_LED0_PIN       GP44
#define LEYDEN_JAR_LED1_PIN       GP43
#define LEYDEN_JAR_LED2_PIN       GP42
#define LEYDEN_JAR_PS2_DETECT_PIN GP40

/* RP2350 I2C device 0 is used by QMK API.
 * It drives the DAC chip that gives the voltage threshold value for the capsense management.
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

/* PS/2 keyboard-device output on the J4 Ext port (PS/2 AT daughterboard), same pins
   and boot-time haptic/PS/2 latch as the Rev 3 F77, see keyboards/leyden_jar/f77/config.h. */
#define PS2_DEVICE_ENABLE
#define PS2_CLOCK_PIN GP28
#define PS2_DATA_PIN GP29
#define PS2_TRACE_ENABLED 0

/* Emulated EEPROM size is increased from 4KiB (the default) to 16KiB.
   We can also define and store in EEPROM up to 64 different VIAL macros.
 */
#define WEAR_LEVELING_BACKING_SIZE  32768
#define DYNAMIC_KEYMAP_MACRO_COUNT  64

/* Vial layout options: 8 bits (Extra Nav Pads, Split Backspace, Right Shift x2,
   Enter x2, Split Space, ISO Left Shift), sized as in the xwhatsit FSSK firmware. */
#define VIA_EEPROM_LAYOUT_OPTIONS_SIZE 2

/* WCASS matrix order, so that the xwhatsit/WCASS FSSK Vial definition and saved
   layouts apply as they are. The FSSK's 15 matrix columns use the common WCASS
   column map of common.c (matrix column n = controller column n). */
#define MATRIX_FORMAT_WCASS
#define BOARD_MODEL_IS_FSSK

#define NB_CAL_BINS          3
#define ACTIVATION_OFFSETS  {10, 10, 10}

#define CONTROLLER_ROWS     8
#define CONTROLLER_COLS     16
