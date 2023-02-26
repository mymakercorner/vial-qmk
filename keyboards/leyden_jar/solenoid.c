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

/* This file goal is to define all solenoid functions that the QMK haptic management code calls.
 *
 * We trick QMK by not defining a solenoid driver with HAPTIC_DRIVER macro in rules.mk
 * to later add the SOLENOID_ENABLE macro for the build process (see content of rules.mk to see how it's done).
 * This way QMK is fooled to have to call the solenoid related functions from the haptic management code.
 * Not many functions in this file implement what QMK haptic code management needs:
 *   - solenoid_setup
 *   - solenoid_shutdown
 *   - solenoid_set_dwell
 *   - solenoid_check
 *   - solenoid_fire_handler
 *
 * The other functions look like what you can see in official solenoid implementation,
 * but are static and ends with '_internal' string.
 *
 * The official QMK solenoid code allows to drive several solenoid devices.
 * We do not here so the code has been simplified quite a bit as a result.
 *
 * This code does not directly drive the solenoid with the RP2040 IO pins, but with IO pins driven from
 * an I2C IO expander; as QMK does not allow to define our own solenoid driver this is the reason why
 * we had to do all those tricks to make the solenoid work.
 * See io_expander.h and .cpp files for the io_expander implementation.
*/

#include "quantum.h"
#include "haptic.h"
#include "solenoid.h"
#include "usb_device_state.h"
#include "io_expander.h"

static uint8_t  solenoid_dwell      = SOLENOID_DEFAULT_DWELL;
static bool     solenoid_on         = false;
static bool     solenoid_buzzing    = false;
static uint16_t solenoid_start      = 0;

extern haptic_config_t haptic_config;

static void solenoid_fire_internal(void)
{
    if (!haptic_config.buzz && solenoid_on) { return; }
    if (haptic_config.buzz && solenoid_buzzing) { return; }

    solenoid_on      = true;
    solenoid_buzzing = true;
    solenoid_start   = timer_read();

    io_expander_set_solenoid_pulse_status(true);
}

static void solenoid_stop_internal(void) {
    io_expander_set_solenoid_pulse_status(false);
    solenoid_on      = false;
    solenoid_buzzing = false;
}

void solenoid_setup(void)
{
    io_expander_set_solenoid_pulse_status(false);

    if ((!HAPTIC_OFF_IN_LOW_POWER) || (usb_device_state == USB_DEVICE_STATE_CONFIGURED)) {
        solenoid_fire_internal();
    }
}

void solenoid_shutdown(void)
{
    io_expander_set_solenoid_pulse_status(false);

    /* We want to flush the solenoid status immediately as the firmware code will probably
     * soon after shut down or reset itself */
    io_expander_update_state();
}


void solenoid_set_dwell(uint8_t dwell)
{
    solenoid_dwell = dwell;
}

void solenoid_check(void)
{
    volatile uint16_t elapsed = 0;

    if (!solenoid_on) return;

    elapsed = timer_elapsed(solenoid_start);

    /* Check if it's time to finish this solenoid click cycle */
    if (elapsed > solenoid_dwell) {
        solenoid_stop_internal();
        return;
    }

    /* Check whether to buzz the solenoid on and off */
    if (haptic_config.buzz) {
        if ((elapsed % (SOLENOID_BUZZ_ACTUATED + SOLENOID_BUZZ_NONACTUATED)) < SOLENOID_BUZZ_ACTUATED) {
            if (!solenoid_buzzing) {
                solenoid_buzzing = true;
                io_expander_set_solenoid_pulse_status(true);
            }
        } else {
            if (solenoid_buzzing) {
                solenoid_buzzing = false;
                io_expander_set_solenoid_pulse_status(false);
            }
        }
    }
}

void solenoid_fire_handler(void)
{
    if (!solenoid_on) {
        solenoid_fire_internal();
    }
}

