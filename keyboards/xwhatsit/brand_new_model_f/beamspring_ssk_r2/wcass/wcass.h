/* Copyright 2020 Purdea Andrei
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "quantum.h"

/* This is a shortcut to help you visually see your layout.
 *
 * The first section contains all of the arguments representing the physical
 * layout of the board and position of the keys.
 *
 * The second converts the arguments into a two-dimensional array which
 * represents the switch matrix.
 */

#define LAYOUT_all( \
    K0_0,       K0_1, K0_2, K0_3, K0_4,   K0_5, K0_6, K0_7, K0_8,   K0_9, K0_10, K0_11, K0_12,       K0_13, K0_14, K0_15, \
                                                                                                                          \
    K1_0, K1_1, K1_2, K1_3, K1_4, K1_5, K1_6, K1_7, K1_8, K1_9, K1_10, K1_11, K1_12, K1_13, K1_14,   K1_15, K1_16, K1_17, \
    K2_0,   K2_1, K2_2, K2_3, K2_4, K2_5, K2_6, K2_7, K2_8, K2_9, K2_10, K2_11, K2_12,    K2_13,     K2_14, K2_15, K2_16, \
    K3_0,     K3_1, K3_2, K3_3, K3_4, K3_5, K3_6, K3_7, K3_8, K3_9, K3_10, K3_11, K3_12,   K3_13,    K3_14, K3_15, K3_16, \
    K4_0, K4_1, K4_2, K4_3, K4_4, K4_5, K4_6, K4_7, K4_8, K4_9, K4_10, K4_11, K4_12, K4_13, K4_14,   K4_15, K4_16, K4_17, \
    K5_0,  K5_1,  K5_2,                      K5_3,                      K5_4,  K5_5,  K5_6,  K5_7,   K5_8,  K5_9,  K5_10  \
) \
{ \
    { K4_0,  K4_1,  K4_6,  KC_NO, K4_10, K4_2,  K4_13, K4_15, K4_17, K4_16, K4_14, K4_11, KC_NO, K4_7,  K4_3  }, \
    { K5_0,  K5_1,  K4_5,  K4_8,  K5_4,  K5_2,  K5_6,  K5_8,  K5_10, K5_9,  K5_7,  K5_5,  K4_9,  K5_3,  K4_4  }, \
    { KC_NO, K3_1,  K3_5,  K3_7,  K3_10, K3_2,  KC_NO, K3_14, K3_16, K3_15, KC_NO, K4_12, K3_9,  K3_6,  K3_4  }, \
    { KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO }, \
    { K1_0,  K1_1,  K1_5,  K0_7,  K1_11, K1_3,  K1_13, K1_15, K1_17, K1_16, K1_14, K1_12, K1_9,  K1_7,  K1_4  }, \
    { K2_0,  K1_2,  K1_6,  K1_8,  K1_10, K2_3,  K2_12, K2_14, K2_16, K2_15, K2_13, K2_11, K2_9,  K2_7,  K2_4  }, \
    { K0_0,  K0_1,  K0_4,  K0_6,  K0_9,  K0_2,  K0_11, K0_13, K0_15, K0_14, K0_12, K0_10, K0_8,  K0_5,  K0_3  }, \
    { K3_0,  K2_1,  K2_5,  K2_8,  K2_10, K2_2,  K3_12, KC_NO, KC_NO, KC_NO, K3_13, K3_11, K3_8,  K2_6,  K3_3  }  \
}

