#!/usr/bin/env bash

qmk compile -kb leyden_jar/f122 -km vial &
qmk compile -kb leyden_jar/f122 -km vial-ansi &
qmk compile -kb leyden_jar/f122 -km vial-ansi-hhkb-split-rshift &
qmk compile -kb leyden_jar/f122 -km vial-ansi-hhkb-split-rshift-hhkb-ctrl-caps &
qmk compile -kb leyden_jar/f122 -km vial-ansi-hhkb-split-rshift-split-backspace &
qmk compile -kb leyden_jar/f122 -km vial-ansi-hhkb-split-rshift-split-backspace-hhkb-ctrl-caps &
qmk compile -kb leyden_jar/f122 -km vial-iso &
qmk compile -kb leyden_jar/f122 -km vial-iso-hhkb-split-rshift &
qmk compile -kb leyden_jar/f122 -km vial-iso-hhkb-split-rshift-hhkb-ctrl-caps &
qmk compile -kb leyden_jar/f122 -km vial-iso-hhkb-split-rshift-split-backspace &
qmk compile -kb leyden_jar/f122 -km vial-iso-hhkb-split-rshift-split-backspace-hhkb-ctrl-caps &

wait

qmk compile -kb leyden_jar/b122 -km vial &
qmk compile -kb leyden_jar/b122 -km vial-ansi &
qmk compile -kb leyden_jar/b122 -km vial-iso &

qmk compile -kb leyden_jar/b104 -km vial &
qmk compile -kb leyden_jar/b104 -km vial-ansi &
qmk compile -kb leyden_jar/b104 -km vial-iso &

qmk compile -kb leyden_jar/f104 -km vial &

qmk compile -kb leyden_jar/f77 -km vial &

qmk compile -kb leyden_jar/f62 -km vial &

qmk compile -kb leyden_jar/f50 -km vial &

qmk compile -kb leyden_jar/f50_rev2 -km vial &

qmk compile -kb leyden_jar/f4x12 -km vial &

qmk compile -kb leyden_jar/f5x12 -km vial &

qmk compile -kb leyden_jar/f20 -km vial &

wait