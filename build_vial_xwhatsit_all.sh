#!/bin/sh
set -e -u
cd "$(dirname "$0")"
VERSION=${1:-dev}

DIST_DIR=newfxx_vial_$VERSION
BUILD_DIR=firmwares
KEYMAP_DIR=keymaps
FLASH_UNIX_DIR=flash-scripts-unix
FLASH_WIND_DIR=flash-scripts-win
# "flash-util" dir less easy to change

git submodule init
git submodule update

if [ -d "$DIST_DIR" ]; then
	rm -rv "$DIST_DIR"
fi
mkdir -v -p "$DIST_DIR/$BUILD_DIR"  \
            "$DIST_DIR/$KEYMAP_DIR" \
            "$DIST_DIR/$FLASH_UNIX_DIR" \
            "$DIST_DIR/$FLASH_WIND_DIR"

# '../' assumes flash scripts one dir below dist dir
FLASH_UNIX_TMPL='#!/bin/sh
cd "$(dirname "$0")"
exec ../flash-util/flash_vial_xwhatsit.sh "../%s" "../%s"
'
# windows cmd.exe does generally support '/' as separator too
# (first '%' char is doubled to escape for printf)
FLASH_WIND_TMPL='@echo off
setlocal enabledelayedexpansion
chdir "%%~dp0"
../flash-util/flash_vial_xwhatsit.bat "../%s" "../%s"
'

# avoid warnings from qmk compile command due to forking from old source tree:
# > qmk_firmware/lib/python/qmk/decorators.py:20: UserWarning:
#      cli._subcommand has been deprecated, please use cli.subcommand_name to get the subcommand name instead.
export PYTHONWARNINGS=ignore::UserWarning

for CONF in keyboards/xwhatsit/brand_new_model_f/*/wcass/build.json ; do
	MODEL=$(echo "$CONF" | cut -d/ -f4)
	case $MODEL in
	fssk_round_1           ) SHORT=fssk      ;;
	fssk_round2            ) SHORT=fssk_r2   ;;
	f104_round_1_nonshrunk ) SHORT=f104_ns   ;;
	f104_round_1_shrunk    ) SHORT=f104      ;;
	f104_round2            ) SHORT=f104_r2   ;;
	f15_left               ) SHORT=f15_l     ;;
	f15_right              ) SHORT=f15_r     ;;
	fortho_left            ) SHORT=fortho_l  ;;
	fortho_right           ) SHORT=fortho_r  ;;
	beamspring_ssk         ) SHORT=bssk      ;;
	beamspring_full        ) SHORT=b104      ;;
	beamspring_ssk_r2      ) SHORT=bssk_r2   ;;
	beamspring_60pct       ) SHORT=b62       ;;
	*                      ) SHORT=$MODEL    ;;
	esac

	echo
	echo "<===== $SHORT | $MODEL =====>"
	echo
	FIRMWARE=xwhatsit_new_${SHORT}_vial_${VERSION}.hex
	qmk compile "$CONF"
	echo
	mv -v xwhatsit_brand_new_model_f_${MODEL}_*.hex "$DIST_DIR/$BUILD_DIR/$FIRMWARE"

	## create flash scripts for all keymaps for $MODEL
	for KEYMAP_FILE in keyboards/xwhatsit/brand_new_model_f/$MODEL/wcass/keymaps/*.hex ; do
		KEYMAP_NAME=$(basename "$KEYMAP_FILE" .hex)
		SCRIPT_UNIX=$DIST_DIR/$FLASH_UNIX_DIR/flash-$KEYMAP_NAME.sh
		SCRIPT_WIND=$DIST_DIR/$FLASH_WIND_DIR/flash-$KEYMAP_NAME.bat
		cp -v "$KEYMAP_FILE"            "$DIST_DIR/$KEYMAP_DIR/"
		cp -v "${KEYMAP_FILE%.hex}.vil" "$DIST_DIR/$KEYMAP_DIR/" || true
		# .vil keymaps missing for f104_round_1_nonshrunk ?

		printf "$FLASH_UNIX_TMPL" "$BUILD_DIR/$FIRMWARE" "$KEYMAP_DIR/$KEYMAP_NAME.hex" \
		        > "$SCRIPT_UNIX"
		chmod a+x "$SCRIPT_UNIX"
		echo   ">  $SCRIPT_UNIX"

		printf "$FLASH_WIND_TMPL" "$BUILD_DIR/$FIRMWARE" "$KEYMAP_DIR/$KEYMAP_NAME.hex" \
		        > "$SCRIPT_WIND"
		echo   ">  $SCRIPT_WIND"
	done
done

echo
echo "<===== dfu-programmer =====>"
echo
DFU_PROG_VERSION=1.1.0
rm -vf flash-util/dfu-programmer* flash-util/libusb*

download() {
	URL_PRE=$1
	ARCHIVE=$2
	FILES=$3

	curl --silent --show-error -L --remote-name "$URL_PRE/$ARCHIVE"

	unzip "$ARCHIVE" $FILES -d flash-util
	rm -v "$ARCHIVE"
}

download "https://github.com/dfu-programmer/dfu-programmer/releases/download/v$DFU_PROG_VERSION" \
         "dfu-programmer-x64-${DFU_PROG_VERSION}.zip" "dfu-programmer.exe libusb-1.0.dll"

download "https://github.com/ploxiln/vial-qmk/releases/download/newfxx_r5_util/" \
         "dfu-programmer-${DFU_PROG_VERSION}_macos_arm64.zip" "dfu-programmer"

mv -v flash-util/dfu-programmer flash-util/dfu-programmer.macos

echo
echo "<===== making zip =====>"
ZIPFILE=$DIST_DIR.zip
rm -vf "$ZIPFILE"
cp -rv flash-util "$DIST_DIR/"
zip -r "$ZIPFILE" "$DIST_DIR"
