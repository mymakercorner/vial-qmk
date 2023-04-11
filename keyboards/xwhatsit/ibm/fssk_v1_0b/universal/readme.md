# xwhatsit/ibm/fssk_v1_0b/universal

![xwhatsit/ibm/fssk_v1_0b/universal](https://deskthority.net/download/file.php?id=29306)

A short description of the keyboard/project

* Keyboard Maintainer: [Purdea Andrei](https://github.com/purdeaandrei)
* Hardware Supported: Supports IBM Model FSSK v1.0b keyboards, using one of the xwhatsit controller PCBs supported by the 'universal' driver: the through_hole controller, or one of the SMD Model F controllers.
  The controllers supported by this firmware are described in these threads:
  https://deskthority.net/viewtopic.php?f=7&t=23406
  https://deskthority.net/viewtopic.php?f=7&t=24597
Note: FSSK v1.0b was never officially released by i$, however the FEXT 1.0b board has been designed such that
if chopping it shorter no additional path cables would be required. This firmware supports such chopped boards.
Also tamsin is going to release a nicely modified FSSK v1.0b diptrace file too.
* Hardware Availability: Rare collector's item

Make example for this keyboard (after setting up your build environment):

    make xwhatsit/ibm/fssk_v1_0b/universal:default

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).
