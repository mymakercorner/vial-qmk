/* Copyright 2025 Eric Becourt (Rico https://mymakercorner.com)
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

/* PS/2 keyboard-device output on the solenoid connector. Enabling it is a plain
   #define here (QMK force-includes every config.h via -include, so it is visible
   to the ps2_glue.c guard); the matching SRC lines live in this variant's
   rules.mk. CLOCK on the haptic-enable pin, DATA on the solenoid pin. Haptic and
   PS/2 are mutually exclusive at runtime (boot-time latch in ps2_glue.c), so
   reusing these pins is intentional. */
#define PS2_DEVICE_ENABLE
#define PS2_CLOCK_PIN GP28
#define PS2_DATA_PIN GP29

/* Strip the driver's byte-level trace ring buffer (ps2_trace.h defaults it to 1
   for the standalone testbed). Nothing drains it in a production build -- the
   console readout lived behind PS2_FORCE_ENABLE -- so leaving it on would only
   cost a 128-entry buffer and a store on every PS/2 byte. For another console
   bring-up session, set this back to 1 and re-add PS2_FORCE_ENABLE below; the
   HID console itself needs nothing extra (keyboard.json already has
   "console": true, and the glue prints with xprintf, which emits regardless of
   whether debug output has been toggled on at runtime). */
#define PS2_TRACE_ENABLED 0

#define SOLENOID_MIN_DWELL 4
#define SOLENOID_MAX_DWELL 100

/* RP2040 has lots of RAM and flash, let's make use of this.
   Emulated EEPROM size is increased from 4KiB (the default) to 16KiB.
   We can also define and store in EEPROM up to 64 different VIAL macros.
 */
#define WEAR_LEVELING_BACKING_SIZE  32768
#define DYNAMIC_KEYMAP_MACRO_COUNT  64

#define MATRIX_FORMAT_WCASS
#define BOARD_MODEL_IS_F77

#define NB_CAL_BINS          3
#define ACTIVATION_OFFSETS  {10, 10, 10}

#define CONTROLLER_ROWS     8
#define CONTROLLER_COLS     16
