CUSTOM_MATRIX=lite
HAPTIC_ENABLE = yes
HAPTIC_DRIVER = solenoid

# Wear leveling EEPROM for the RP2040 chip is selected for QMK VIA/VIAL storage
EEPROM_DRIVER = wear_leveling
WEAR_LEVELING_DRIVER = rp2040_flash

SRC += keyboards/leyden_jar/matrix.c keyboards/leyden_jar/dac.c keyboards/leyden_jar/common.c keyboards/leyden_jar/pio_matrix_scan.c
SRC += keyboards/leyden_jar/io_expander.c keyboards/leyden_jar/leds.c keyboards/leyden_jar/util_protocol.c

# PS/2 keyboard-device output (enabled via `#define PS2_DEVICE_ENABLE` in config.h;
# shares the solenoid connector, mutually exclusive with haptic at runtime). Listed
# here per-variant rather than in a shared keyboards/leyden_jar/rules.mk because QMK
# includes the shared rules.mk BEFORE this one, so a shared gate could not see a
# flag set here.
SRC += keyboards/leyden_jar/ps2/hid_to_ps2.c keyboards/leyden_jar/ps2/ps2_trace.c
SRC += keyboards/leyden_jar/ps2/ps2_glue.c keyboards/leyden_jar/ps2/ps2_platform_chibios.c
VPATH += keyboards/leyden_jar/ps2

QUANTUM_LIB_SRC += i2c_master.c

