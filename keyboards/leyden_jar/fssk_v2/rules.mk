CUSTOM_MATRIX=lite
HAPTIC_ENABLE = yes
HAPTIC_DRIVER = solenoid

# Wear leveling EEPROM in the RP flash is selected for QMK VIA/VIAL storage (ChibiOS EFL driver)
EEPROM_DRIVER = wear_leveling
WEAR_LEVELING_DRIVER = embedded_flash

SRC += keyboards/leyden_jar/matrix.c keyboards/leyden_jar/dac.c keyboards/leyden_jar/common.c keyboards/leyden_jar/pio_matrix_scan.c
# The matrix scanner runs on the ChibiOS RP PIO driver, built only on request
OPT_DEFS += -DRP_PIO_REQUIRED=TRUE
# io_expander.c: LEDs and PS/2 presence on GPIOs on V2 (LEYDEN_JAR_V2 in config.h)
SRC += keyboards/leyden_jar/io_expander.c keyboards/leyden_jar/leds.c keyboards/leyden_jar/util_protocol.c

# PS/2 keyboard-device output (PS2_DEVICE_ENABLE in config.h), see keyboards/leyden_jar/f77/rules.mk
SRC += keyboards/leyden_jar/ps2/hid_to_ps2.c keyboards/leyden_jar/ps2/ps2_trace.c
SRC += keyboards/leyden_jar/ps2/ps2_glue.c keyboards/leyden_jar/ps2/ps2_platform_chibios.c
VPATH += keyboards/leyden_jar/ps2
# <hardware/pio.h> for the verbatim driver core: minimal pico-sdk stand-in on ChibiOS
EXTRAINCDIRS += keyboards/leyden_jar/ps2/chibios_compat

QUANTUM_LIB_SRC += i2c_master.c
