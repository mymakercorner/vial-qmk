#!/bin/sh
set -e -u
VERSION=${1:-dev}

# assumes you "brew install qt@5 cmake"

QTDIR="/opt/homebrew/opt/qt@5"
export PATH="$QTDIR/bin:$PATH"

HIDAPI_VERSION="0.15.0"
HIDAPI_URL="https://github.com/libusb/hidapi/archive/refs/tags/hidapi-$HIDAPI_VERSION.tar.gz"

# also set in util/util.pro
export MACOSX_DEPLOYMENT_TARGET="13.0"

cd "$(dirname "$0")"
BLDDIR=$(pwd)/macbuild

rm -rf "$BLDDIR"
mkdir  "$BLDDIR"
cd     "$BLDDIR"

curl -L -O "$HIDAPI_URL"
tar -xvf  "hidapi-$HIDAPI_VERSION.tar.gz"
cd "hidapi-hidapi-$HIDAPI_VERSION"

cmake -S . -B build -DBUILD_SHARED_LIBS=OFF \
      -DCMAKE_INSTALL_PREFIX="$BLDDIR"

cmake --build   build
cmake --install build

cd "$BLDDIR"
mkdir util
cd    util
export HIDAPI_PREFIX="$BLDDIR"
qmake ../../util/util.pro
make

cd "$BLDDIR"
mkdir dmgdir
mv -v util/util.app dmgdir/pandrew-util.app
macdeployqt        dmgdir/pandrew-util.app
# update "ad-hoc" signature
codesign --force --deep --sign - dmgdir/pandrew-util.app
# format ULMO for lzma compression (supported by macOS 10.15+)
hdiutil create -fs HFS+ -format ULMO -srcfolder dmgdir \
        -volname "pandrew-util-$VERSION" "../pandrew-util-macos-$(uname -m)-$VERSION.dmg"
