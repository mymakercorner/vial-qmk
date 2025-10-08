#!/bin/sh
set -e -u

# build dfu-programmer with libusb static-linked in
LIBUSB_VER=1.0.29
DFU_PROG_VER=1.1.0

# manually rename zip file to change OS or Arch name convention
ZIPFILE="dfu-programmer-${DFU_PROG_VER}-$(uname -s)-$(uname -m).zip"

# only affects macOS (not sure how much, for this util/lib)
export MACOSX_DEPLOYMENT_TARGET=13.0

BLDDIR=$PWD/dfu_build
rm -rf  "$BLDDIR"
mkdir -p $BLDDIR

LIBUSB_ARCHIVE=libusb-$LIBUSB_VER.tar.bz2
DFU_PROG_ARCHIVE=dfu-programmer-$DFU_PROG_VER.tar.gz

mkdir -p $BLDDIR/dl
cd $BLDDIR/dl
curl -sSLO https://github.com/libusb/libusb/releases/download/v$LIBUSB_VER/$LIBUSB_ARCHIVE
curl -sSLO https://github.com/dfu-programmer/dfu-programmer/releases/download/v$DFU_PROG_VER/$DFU_PROG_ARCHIVE

cd $BLDDIR
tar -xvf dl/$LIBUSB_ARCHIVE
cd libusb-$LIBUSB_VER
./configure --prefix=$BLDDIR --disable-shared
make
make install

cd $BLDDIR
tar -xvf dl/$DFU_PROG_ARCHIVE
cd dfu-programmer-$DFU_PROG_VER
LIBUSB_EXTRA_LIBS=$(grep Libs.private: $BLDDIR/lib/pkgconfig/libusb-1.0.pc | cut -d' ' -f2-9)
./configure CFLAGS="-I$BLDDIR/include" LDFLAGS="-L$BLDDIR/lib $LIBUSB_EXTRA_LIBS"
make
cp -v src/dfu-programmer $BLDDIR/

cd $BLDDIR
zip $ZIPFILE dfu-programmer
