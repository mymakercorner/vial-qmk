#!/bin/sh
set -e -u
VERSION=${1:-dev}
# you can set DOCKER=podman in the env
DOCKER=${DOCKER:-docker}
QMAKE=x86_64-w64-mingw32.static-qmake-qt5

cd "$(dirname "$0")"
$DOCKER build -t winbuilder windows_builder
$DOCKER run --rm --volume $PWD/../../xwhatsit:/mnt/xwhatsit \
            winbuilder bash -c "
set -e -u
cd /mnt/xwhatsit/util/
rm -rf winbuild
mkdir  winbuild
cd     winbuild

$QMAKE ../util/util.pro
make
mv -v release/util.exe pandrew-util.exe
"

cd winbuild
zip ../pandrew-util-win-x64-$VERSION.zip pandrew-util.exe
