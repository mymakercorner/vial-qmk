/* Copyright 2023 Eric Becourt (Rico https://mymakercorner.com)
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

#define LAYOUT_all( \
    k_0_0, k_0_1, k_0_2, k_0_3, k_0_4, k_0_5, k_0_6, k_0_7, k_0_8, k_0_9, k_0_10, k_0_11, k_0_12, k_0_13, k_0_14, k_0_15,\
    k_1_0, k_1_1, k_1_2, k_1_3, k_1_4, k_1_5, k_1_6, k_1_7, k_1_8, k_1_9, k_1_10, k_1_11, k_1_12,         k_1_14, k_1_15,\
    k_2_0,        k_2_2, k_2_3, k_2_4, k_2_5, k_2_6, k_2_7, k_2_8, k_2_9, k_2_10, k_2_11, k_2_12, k_2_13, k_2_14, k_2_15,\
    k_3_0, k_3_1, k_3_2, k_3_3, k_3_4, k_3_5, k_3_6, k_3_7, k_3_8, k_3_9, k_3_10, k_3_11,         k_3_13, k_3_14, k_3_15,\
    k_4_0, k_4_1, k_4_2,                      k_4_6,                      k_4_10,         k_4_12, k_4_13, k_4_14, k_4_15\
) \
{ \
    { k_0_0, k_0_1, k_0_2, k_0_3, k_0_4, k_0_5, k_0_6, k_0_7, k_0_8, k_0_9, k_0_10, k_0_11, k_0_12, k_0_13, k_0_14, k_0_15 },\
    { k_1_0, k_1_1, k_1_2, k_1_3, k_1_4, k_1_5, k_1_6, k_1_7, k_1_8, k_1_9, k_1_10, k_1_11, k_1_12, KC_NO,  k_1_14, k_1_15 },\
    { k_2_0, KC_NO, k_2_2, k_2_3, k_2_4, k_2_5, k_2_6, k_2_7, k_2_8, k_2_9, k_2_10, k_2_11, k_2_12, k_2_13, k_2_14, k_2_15 },\
    { k_3_0, k_3_1, k_3_2, k_3_3, k_3_4, k_3_5, k_3_6, k_3_7, k_3_8, k_3_9, k_3_10, k_3_11, KC_NO,  k_3_13, k_3_14, k_3_15 },\
    { k_4_0, k_4_1, k_4_2, KC_NO, KC_NO, KC_NO, k_4_6, KC_NO, KC_NO, KC_NO, k_4_10, KC_NO,  k_4_12, k_4_13, k_4_14, k_4_15 }\
}
