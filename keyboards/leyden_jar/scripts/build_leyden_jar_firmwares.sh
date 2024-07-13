#!/usr/bin/env bash

qmk compile -kb leyden_jar/f122 -km vial
qmk compile -kb leyden_jar/f122 -km vial-ansi
qmk compile -kb leyden_jar/f122 -km vial-ansi-hhkb-split-rshift
qmk compile -kb leyden_jar/f122 -km vial-ansi-hhkb-split-rshift-lctrl-caps-revert
qmk compile -kb leyden_jar/f122 -km vial-ansi-hhkb-split-rshift-split-backspace
qmk compile -kb leyden_jar/f122 -km vial-ansi-hhkb-split-rshift-split-backspace-lctrl-caps-revert
qmk compile -kb leyden_jar/f122 -km vial-iso
qmk compile -kb leyden_jar/f122 -km vial-iso-hhkb-split-rshift
qmk compile -kb leyden_jar/f122 -km vial-iso-hhkb-split-rshift-lctrl-caps-revert
qmk compile -kb leyden_jar/f122 -km vial-iso-hhkb-split-rshift-split-backspace
qmk compile -kb leyden_jar/f122 -km vial-iso-hhkb-split-rshift-split-backspace-lctrl-caps-revert

qmk compile -kb leyden_jar/b122 -km vial
qmk compile -kb leyden_jar/b122 -km vial-ansi
qmk compile -kb leyden_jar/b122 -km vial-iso

qmk compile -kb leyden_jar/b104 -km vial
qmk compile -kb leyden_jar/b104 -km vial-ansi
qmk compile -kb leyden_jar/b104 -km vial-iso

