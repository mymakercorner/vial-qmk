CUSTOM_MATRIX=lite
HAPTIC_ENABLE = yes
HAPTIC_DRIVER = solenoid

# Wear leveling EEPROM in the RP flash is selected for QMK VIA/VIAL storage (ChibiOS EFL
# driver: same flash area and format as the former pico-sdk rp2040_flash driver)
EEPROM_DRIVER = wear_leveling
WEAR_LEVELING_DRIVER = embedded_flash

SRC += keyboards/leyden_jar/matrix.c keyboards/leyden_jar/dac.c keyboards/leyden_jar/common.c keyboards/leyden_jar/pio_matrix_scan.c
# The matrix scanner runs on the ChibiOS RP PIO driver, built only on request
OPT_DEFS += -DRP_PIO_REQUIRED=TRUE
SRC += keyboards/leyden_jar/io_expander.c keyboards/leyden_jar/leds.c keyboards/leyden_jar/util_protocol.c
QUANTUM_LIB_SRC += i2c_master.c
