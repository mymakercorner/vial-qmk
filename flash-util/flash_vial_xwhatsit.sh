#!/bin/sh
set -u

abort() {
	printf '%s\n' "$1" 1>&2
	printf "(Press enter to finish) " && read _IGNORE
	exit 2
}

# dfu-programmer 1.1.0 on macOS sometimes hangs on atmega32u2 reset ?
_timeout() {
	if command -v timeout >/dev/null 2>&1; then
		timeout "$@"
	elif command -v perl >/dev/null 2>&1; then
		perl -e 'alarm shift; exec @ARGV' "$@"
	else
		shift # no timeout after all
		"$@"
	fi
}

if [ $# != 2 ]; then
	abort "USAGE: $0 path/to/firwmare.hex path/to/keymap.hex"
fi

DFU_PROGRAMMER=${DFU_PROGRAMMER:-"dfu-programmer"}
THISDIR=$(dirname "$0")
FW=$1
KM=$2

if ! [ -e "$FW" ]; then
	abort "ERROR: firmware file '$FW' not found"
fi
if ! [ -e "$KM" ]; then
	abort "ERROR: keymap file '$KM' not found"
fi

if [ Darwin = "$(uname)" ] && ! "$DFU_PROGRAMMER" --version >/dev/null 2>&1; then
	DFU_PROGRAMMER=$THISDIR/dfu-programmer.macos
fi
if ! "$DFU_PROGRAMMER" --version; then
	if [ Darwin = "$(uname)" ]; then
		abort "ERROR: please install 'dfu-programmer' with homebrew or macports"
	fi
	abort "ERROR: dfu-programmer tool not found"
fi

get_bootloader() {
	"$DFU_PROGRAMMER" atmega32u2 get bootloader-version
}

echo "Preparing to flash firmware for: $(basename "$KM" .hex) ..."
echo
echo Please plug in keyboard to be flashed, and put into bootloader/flash mode.
echo Waiting for keyboard...
while ! get_bootloader 2>/dev/null; do sleep 2 ; done

echo
echo "*** Wiping existing flash memory and EEPROM..."
echo
"$DFU_PROGRAMMER" atmega32u2 erase --force
"$DFU_PROGRAMMER" atmega32u2 flash --force --suppress-validation --eeprom "$THISDIR/reset.eep"
"$DFU_PROGRAMMER" atmega32u2 flash --force "$THISDIR/eeprom_eraser.hex"

echo
echo "*** Resetting controller for main flash ..."
echo
_timeout 4 "$DFU_PROGRAMMER" atmega32u2 reset
while ! get_bootloader 2>/dev/null ; do sleep 2 ; done

echo
echo "*** Flashing firmware '$(basename "$FW")' ..."
echo
"$DFU_PROGRAMMER" atmega32u2 erase --force
"$DFU_PROGRAMMER" atmega32u2 flash --force "$FW"

echo
echo "*** Flashing keymap '$(basename "$KM")' ..."
echo
"$DFU_PROGRAMMER" atmega32u2 flash --force --eeprom "$KM"

echo
echo "*** Rebooting keyboard controller ..."
_timeout 4 "$DFU_PROGRAMMER" atmega32u2 reset

echo
echo ...done!
printf "(Press enter to finish) " && read _IGNORE
