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

#include "i2c_master.h"
#include "io_expander.h"

#define IO_EXPANDER_I2C_ADDR    (0x18<<1)
#define IO_EXPANDER_I2C_READ    (IO_EXPANDER_I2C_ADDR | 1)
#define IO_EXPANDER_I2C_WRITE   (IO_EXPANDER_I2C_ADDR | 0)
#define IO_EXPANDER_I2C_TIMEOUT 100

#define IO_EXPANDER_LED0   (1<<4)
#define IO_EXPANDER_LED1   (1<<5)
#define IO_EXPANDER_LED2   (1<<6)

/* PS/2 daughterboard presence, the one INPUT pin. The daughterboard feeds this pin
 * ~3.33V through a resistor divider off its 5V rail, taken upstream of the
 * daughterboard's current limiter; the main board's 47k pull-down holds the pin low
 * when no daughterboard is plugged in.
 *
 * INTENDED meaning: "a live PS/2 host is powering the daughterboard", i.e. USB's 5V
 * cannot reach the divider. That is what the mode latch in ps2_glue.c assumes.
 *
 * On daughterboard revisions BEFORE the 5V-isolation fix it means only "a
 * daughterboard is ATTACHED": the two 5V rails turned out not to be isolated
 * (verified on hardware 2026-08-17 -- daughterboard attached, no PS/2 cable at all,
 * USB alone pulled the pin high). The firmware is identical either way; on an
 * unfixed board the daughterboard must be unplugged to use the USB connector. */
#define IO_EXPANDER_PS2_DETECT (1<<3)

static uint8_t s_current_io_expander_state;
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

    i2c_status_t ret = i2c_transmit(IO_EXPANDER_I2C_WRITE, write_data, 2, IO_EXPANDER_I2C_TIMEOUT);
    if (ret != I2C_STATUS_SUCCESS) {
        return 0;
    }

    return 1;
}

int io_expander_init()
{
    i2c_init();

    /* We check that the I2C device is recognized */
    uint8_t dummy;
    i2c_status_t ret = i2c_receive(IO_EXPANDER_I2C_READ, &dummy, 1, IO_EXPANDER_I2C_TIMEOUT);
    if (ret != I2C_STATUS_SUCCESS) {
        return 0;
    }

    /* We set all pins to output type, except the PS/2 detect pin
     * Write Control Register command is 2 bytes
     * First byte contains 0x03 that tell that we will update the Control Register
     * Second byte contains the control register value, one bit per IO pin, 0 for output and 1 for input.
     * Only IO_EXPANDER_PS2_DETECT is an input; it is set on every board and not just the PS/2 ones,
     * because the main board's pull-down keeps it at a defined level whether or not a daughterboard
     * is plugged in. Pins configured as input ignore the Output Port Register, so
     * io_expander_set_output_pins_level() can keep writing the whole byte. */
    uint8_t write_data[2];
    write_data[0] = 0x03;
    write_data[1] = IO_EXPANDER_PS2_DETECT;
    ret = i2c_transmit(IO_EXPANDER_I2C_WRITE, write_data, 2, IO_EXPANDER_I2C_TIMEOUT);
    if (ret != I2C_STATUS_SUCCESS) {
        return 0;
    }

    /* We set all pins to output 0 */
    if (io_expander_set_output_pins_level(0) != 1) {
        return 0;
    }

    s_current_io_expander_state = 0;
    s_led0 = false;
    s_led1 = false;
    s_led2 = false;

    return 1;
}

bool io_expander_is_ps2_present(void) {
    /* Read Input Port Register command reads back register 0x00, which holds the live
     * level of every IO pin. See IO_EXPANDER_PS2_DETECT above for what drives it.
     * On any I2C failure we report "not present": the caller uses this to decide
     * between USB and PS/2 mode, and a failed read must never be what selects PS/2
     * (that would take the board off USB and leave it unusable). */
    uint8_t input_level;

    i2c_status_t ret = i2c_read_register(IO_EXPANDER_I2C_ADDR, 0x00, &input_level, 1, IO_EXPANDER_I2C_TIMEOUT);
    if (ret != I2C_STATUS_SUCCESS) {
        return false;
    }

    return (input_level & IO_EXPANDER_PS2_DETECT) != 0;
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

    if (s_led0) {
        new_state |= IO_EXPANDER_LED0;
    }
    if (s_led1) {
        new_state |= IO_EXPANDER_LED1;
    }
    if (s_led2) {
        new_state |= IO_EXPANDER_LED2;
    }

    if (new_state != s_current_io_expander_state) {
        s_current_io_expander_state = new_state;
        io_expander_set_output_pins_level(s_current_io_expander_state);
    }
}

