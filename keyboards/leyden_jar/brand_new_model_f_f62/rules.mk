# MCU name
MCU = RP2040
BOOTLOADER = rp2040
BOARD = GENERIC_RP_RP2040

# Build Options
#   change yes to no to disable
#
BOOTMAGIC_ENABLE = yes
MOUSEKEY_ENABLE = yes       # Mouse keys
EXTRAKEY_ENABLE = yes       # Audio control and System control
CONSOLE_ENABLE = yes         # Console for debug
COMMAND_ENABLE = no         # Commands for debug and configuration
# Do not enable SLEEP_LED_ENABLE. it uses the same timer as BACKLIGHT_ENABLE
SLEEP_LED_ENABLE = no       # Breathing sleep LED during USB suspend
# if this doesn't work, see here: https://github.com/tmk/tmk_keyboard/wiki/FAQ#nkro-doesnt-work
NKRO_ENABLE = no            # USB Nkey Rollover
BACKLIGHT_ENABLE = no       # Enable keyboard backlight functionality
RGBLIGHT_ENABLE = no        # Enable keyboard RGB underglow
BLUETOOTH_ENABLE = no       # Enable Bluetooth
AUDIO_ENABLE = no           # Audio output
CUSTOM_MATRIX=lite
HAPTIC_ENABLE = yes
# We do no define the haptic driver, but we do later add the SOLENOID_ENABLE macro definition.
# This will trick QMK haptic management that there is defined solenoid driver.
# And we define inside our own solenoid.c file all functions that the haptic management code calls.
#HAPTIC_DRIVER = SOLENOID

# External EEPROM chip is selected for QMK VIA/VIAL storage
# But emulated wear leveling EEPROM is also available for the RP2040 chip
# Comment the line below and uncomment the two next line if you want it.
EEPROM_DRIVER = i2c
#EEPROM_DRIVER = wear_leveling
#WEAR_LEVELING_DRIVER = rp2040_flash

OPT_DEFS += -DSOLENOID_ENABLE # this is to trick QMK haptic code management that there is a solenoid driver defined
SRC += keyboards/leyden_jar/matrix.c keyboards/leyden_jar/dac.c keyboards/leyden_jar/common.c keyboards/leyden_jar/pio_matrix_scan.c
SRC += keyboards/leyden_jar/io_expander.c keyboards/leyden_jar/solenoid.c keyboards/leyden_jar/leds.c
QUANTUM_LIB_SRC += i2c_master.c
