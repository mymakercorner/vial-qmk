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
#include "io_expander.h"
#include "i2c_master.h"

#define IO_EXPANDER_I2C_ADDR    0x18
#define IO_EXPANDER_I2C_TIMEOUT 100

#define SOLENOID_ENABLE_VAL (1<<2)
#define SOLENOID_PULSE_VAL  (1<<3)
#define SOLENOID_LED0_VAL   (1<<4)
#define SOLENOID_LED1_VAL   (1<<5)
#define SOLENOID_LED2_VAL   (1<<6)

static const I2CConfig i2cconfig = {
#if defined(USE_I2CV1_CONTRIB)
    I2C1_CLOCK_SPEED,
#elif defined(USE_I2CV1)
    I2C1_OPMODE,
    I2C1_CLOCK_SPEED,
    I2C1_DUTY_CYCLE,
#else
    #error I2C config setting type unknown
#endif
};

static uint8_t s_current_io_expander_state;
static bool s_solenoid_enable;
static bool s_solenoid_pulse;
static bool s_led0;
static bool s_led1;
static bool s_led2;

static int io_expander_set_output_pins_level(uint8_t pins_level) {
    /* Write Output Port Register command is 2 bytes
     * First byte contains 0x01 that tell that we will update the Output Port Register
     * Second byte contains the output port register value, each bit (LSb is O0, Msb is O7) telling if output level is high or low */
    uint8_t write_data[2];
    write_data[0] = 0x01;
    write_data[1] = pins_level;

    i2cStart(&I2CD2, &i2cconfig);
    msg_t status = i2cMasterTransmitTimeout(&I2CD2, IO_EXPANDER_I2C_ADDR, write_data, 2, 0, 0, TIME_MS2I(IO_EXPANDER_I2C_TIMEOUT));
    if (status != MSG_OK)
    {
        i2cStop(&I2CD2);
        return 0;
    }

    return 1;
}

int io_expander_init()
{
    /* Try releasing special pins for a short time */
    palSetLineMode(26, PAL_MODE_INPUT);
    palSetLineMode(27, PAL_MODE_INPUT);

    chThdSleepMilliseconds(10);

    palSetLineMode(27, PAL_MODE_ALTERNATE(I2C1_SCL_PAL_MODE));
    palSetLineMode(26, PAL_MODE_ALTERNATE(I2C1_SDA_PAL_MODE));

    /* We check that the I2C device is recognized */
    i2cStart(&I2CD2, &i2cconfig);
    uint8_t dummy;
    msg_t status = i2cMasterReceiveTimeout(&I2CD2, IO_EXPANDER_I2C_ADDR, &dummy, 1, TIME_MS2I(IO_EXPANDER_I2C_TIMEOUT));
    if (status != MSG_OK)
    {
        i2cStop(&I2CD2);
        return 0;
    }

    /* We set all pins to output type
     * Write Control Register command is 2 bytes
     * First byte contains 0x03 that tell that we will update the Control Register
     * Second byte contains the control register value, here 0x00 to tell that all IO pins are configured as output */
    uint8_t write_data[2];
    write_data[0] = 0x03;
    write_data[1] = 0x00;
    status = i2cMasterTransmitTimeout(&I2CD2, IO_EXPANDER_I2C_ADDR, write_data, 2, 0, 0, TIME_MS2I(IO_EXPANDER_I2C_TIMEOUT));
    if (status != MSG_OK)
    {
        i2cStop(&I2CD2);
        return 0;
    }

    /* We set all pins to output 0 */
    if (io_expander_set_output_pins_level(0) != 1)
        return 0;

    s_current_io_expander_state = 0;
    s_solenoid_enable = false;
    s_solenoid_pulse = false;
    s_led0 = false;
    s_led1 = false;
    s_led2 = false;

    return 1;
}

void io_expander_set_solenoid_enable_status(bool enable) {
    s_solenoid_enable = enable;
}

void io_expander_set_solenoid_pulse_status(bool enable) {
    s_solenoid_pulse = enable;
}

void io_expander_set_led0_status(bool enable) {
    s_led0 = enable;
}

void io_expander_set_led1_status(bool enable) {
    s_led1 = enable;
}

void io_expander_set_led2_status(bool enable) {
    s_led2 = enable;
}

void io_expander_update_state(void) {
    uint8_t new_state = 0;

    if (s_solenoid_enable) {
        new_state |= SOLENOID_ENABLE_VAL;
    }
    if (s_solenoid_pulse) {
        new_state |= SOLENOID_PULSE_VAL;
    }
    if (s_led0) {
        new_state |= SOLENOID_LED0_VAL;
    }
    if (s_led1) {
        new_state |= SOLENOID_LED1_VAL;
    }
    if (s_led2) {
        new_state |= SOLENOID_LED2_VAL;
    }

    if (new_state != s_current_io_expander_state)
    {
        s_current_io_expander_state = new_state;
        io_expander_set_output_pins_level(s_current_io_expander_state);
    }
}

