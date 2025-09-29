#!/bin/sh
set -e -u

VERSION=${1:-dev}
# you can set DOCKER=podman in the env
DOCKER=${DOCKER:-docker}

cd "$(dirname "$0")"
$DOCKER build -t linbuilder linux_builder
$DOCKER run --rm --volume $PWD/../../xwhatsit:/mnt/xwhatsit \
            linbuilder bash -c "
set -e -u
cd /mnt/xwhatsit/util/
rm -rf   linbuild
mkdir -p linbuild
cd       linbuild

qmake ../util/util.pro
make
mkdir -p appdir/usr/bin
mkdir -p appdir/usr/share/applications
mkdir -p appdir/usr/share/icons/hicolor/128x128/apps
cp -v    util                               appdir/usr/bin/pandrew-util
cp -v ../linux_builder/pandrew-util.desktop appdir/usr/share/applications/pandrew-util.desktop
cp -v ../linux_builder/modelfkey_128.png    appdir/usr/share/icons/hicolor/128x128/apps/modelfkey.png
/opt/linuxdeployqt/AppRun appdir/usr/share/applications/pandrew-util.desktop -appimage -no-translations
mv -v 'pandrew-util-$(uname -m).AppImage' '../pandrew-util-linux-$(uname -m)-$VERSION.AppImage'
"
