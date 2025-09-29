### vial-qmk for xwhatsit controller based keyboards

For (many) Model-F reproduction keyboards produced by: https://www.modelfkeyboards.com/

QMK open-source keyboard firmware: https://github.com/qmk/qmk_firmware

Vial is a fork of QMK to enable re-configuring without re-flashing: https://github.com/vial-kb/vial-qmk

QMK fork with support for xwhatsit based keyboards by Purdea Andrei: http://purdea.ro/qmk_firmware

That host went offline in mid-2025, branches are mirrored at: https://github.com/ploxiln/qmk_firmware

Support for xwhatstit based keyboards added to Vial by NathanA: https://www.newfxx-firmware.nconx.com/

That Vial support firmare is provided as an archive which contains built firmware,
windows and macOS tool to check capacitive keys, some patches and keyboard config files,
and a `build.sh` script which conglomerates the source tree that generated the built
artifacts by cherry-picking commits, applying patches, and copying configs ... it's a bit messy.
So, I made this repo/branch by following along and making attributed and dated commits.

So anyway, with this repo/branch you can compile the xwhatsit "pandrew-util" for linux like so:

```sh
sudo apt install build-essential libhidapi-dev qtbase5-dev
# or equivalent for your linux distro

git clone https://github.com/ploxiln/vial-qmk.git
cd vial-qmk/keyboards/xwhatsit/util/util

qmake util.pro  # or qmake-qt5
make

# hopefully that worked, now you can run it:
./util

# probably best to copy it somewhere better:
sudo cp -v util /usr/local/bin/pandrew-util
```

I switched from `-lhidapi-libusb` to `-lhidapi-hidraw` in `keyboards/xwhatsit/util/util/util.pro`
because hidraw seems to be the api used by the Vial app: https://get.vial.today/manual/linux-udev.html

You can also download "pandrew-util" built for linux from https://github.com/ploxiln/vial-qmk/releases
