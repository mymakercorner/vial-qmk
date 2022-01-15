/*
Copyright 2022 Eric Becourt (Rico)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "quantum.h"

#define LAYOUT_all( \
    K0_00, K0_01, K0_02, K0_03, K0_04, K0_05, K0_06, K0_07, K0_08,        K0_10, K0_11, K0_12, K0_13, K0_14, K0_15,\
    K1_00, K1_01, K1_02, K1_03, K1_04, K1_05, K1_06, K1_07, K1_08, K1_09, K1_10, K1_11, K1_12, K1_13, K1_14, K1_15,\
    K2_00, K2_01, K2_02, K2_03, K2_04, K2_05, K2_06, K2_07, K2_08, K2_09, K2_10, K2_11, K2_12,        K2_14, K2_15,\
    K3_00, K3_01, K3_02, K3_03, K3_04, K3_05, K3_06, K3_07, K3_08, K3_09, K3_10, K3_11,        K3_13, K3_14, K3_15,\
    K4_00, K4_01, K4_02, K4_03, K4_04, K4_05, K4_06, K4_07, K4_08, K4_09, K4_10, K4_11,        K4_13, K4_14, K4_15,\
    K5_00, K5_01, K5_02,                      K5_06,                      K5_10, K5_11, K5_12, K5_13, K5_14, K5_15\
) \
{ \
    { K0_00, K0_01, K0_02, K0_03, K0_04, K0_05, K0_06, K0_07, K0_08, KC_NO, K0_10, K0_11, K0_12, K0_13, K0_14, K0_15 }, \
    { K1_00, K1_01, K1_02, K1_03, K1_04, K1_05, K1_06, K1_07, K1_08, K1_09, K1_10, K1_11, K1_12, K1_13, K1_14, K1_15 }, \
    { K2_00, K2_01, K2_02, K2_03, K2_04, K2_05, K2_06, K2_07, K2_08, K2_09, K2_10, K2_11, K2_12, KC_NO, K2_14, K2_15 }, \
    { K3_00, K3_01, K3_02, K3_03, K3_04, K3_05, K3_06, K3_07, K3_08, K3_09, K3_10, K3_11, KC_NO, K3_13, K3_14, K3_15 }, \
    { K4_00, K4_01, K4_02, K4_03, K4_04, K4_05, K4_06, K4_07, K4_08, K4_09, K4_10, K4_11, KC_NO, K4_13, K4_14, K4_15 }, \
    { K5_00, K5_01, K5_02, KC_NO, KC_NO, KC_NO, K5_06, KC_NO, KC_NO, KC_NO, K5_10, K5_11, K5_12, K5_13, K5_14, K5_15 } \
}
