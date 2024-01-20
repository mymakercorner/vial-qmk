CUSTOM_MATRIX = lite

# Wear leveling EEPROM for the RP2040 chip is selected for QMK VIA/VIAL storage
EEPROM_DRIVER = wear_leveling
WEAR_LEVELING_DRIVER = rp2040_flash

SRC += matrix.c dac.c
QUANTUM_LIB_SRC += i2c_master.c


