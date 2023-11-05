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
    k_esc,   f_1,f_2,f_3,f_4,f_5,f_6,f_7,f_8,f_9,f_10,f_11,f_12,                                              k_prsc,k_scr,  k_paus,  \
    k_tilde, k_1, k_2, k_3, k_4, k_5, k_6, k_7, k_8, k_9, k_0, k_minus, k_equals, k_backslash, k_backtick,    k_ins, k_home, k_pgup,    k_num,k_slash,k_ast,     k_kpminus,\
    k_tab,   k_q, k_w, k_e, k_r, k_t, k_y, k_u, k_i, k_o, k_p, k_squarebrop, k_squarebrcl,k_backspace,        k_del, k_end, k_pgdn,     k_kp7,k_kp8,k_kp9,       k_kpplus,\
    k_caps,   k_a, k_s, k_d, k_f, k_g, k_h, k_j, k_k, k_l, k_semicolon, k_singlequote, k_hashtag, k_return,   k_ex1, k_ex2, k_ex3,      k_kp4,k_kp5,k_kp6,       k_kpplus2,\
    k_lshift, k_nonusbs, k_z, k_x, k_c, k_v, k_b, k_n, k_m, k_cm,k_period, k_forwardslash, k_rshift, k_fn,    k_ex4, k_up,  k_ex5,      k_kp1,k_kp2,k_kp3,       k_kpent,\
    k_lctrl, k_lgui, k_lalt,         k_space,                     k_extra, k_ralt, k_menu, k_rctrl,           k_left,k_down,k_right,    k_kpx,k_kp0,k_kpdel,     k_kpent2\
) \
{ \
    { KC_NO,    k_nonusbs,  k_kp1,  k_kp6,    k_kp2,     k_ex5,   k_c,        k_g,     k_n,       k_up,      k_s,        k_ex1,        k_singlequote,k_period,       k_k         }, \
    { k_lshift, k_z,        k_kpx,  k_kpent,  k_kp3,     k_right, k_v,        k_b,     k_m,       k_ex4,     k_x,        k_fn,         k_rshift,     k_forwardslash, k_cm        }, \
    { k_caps,   k_a,        k_kp4,  k_kpplus2,k_kp5,     k_ex3,   k_f,        k_h,     k_j,       k_ex2,     k_d,        k_return,     k_hashtag,    k_semicolon,    k_l         }, \
    { k_lctrl,  k_lgui,     k_kp0,  k_kpent2, k_kpdel,   k_down,  KC_NO,      KC_NO,   k_space,   k_left,    k_lalt,     k_rctrl,      k_menu,       k_ralt,         k_extra     }, \
    { k_esc,    f_1,        f_2,    k_paus,   k_scr,     f_7,     f_4,        f_5,     f_6,       k_prsc,    f_3,        f_11,         f_10,         f_9,            f_8         }, \
    { k_tilde,  k_q,        k_slash,k_kpminus,k_kp9,     k_home,  k_r,        k_6,     k_u,       k_del,     k_3,        k_backtick,   k_backslash,  k_p,            k_9         }, \
    { k_1,      k_2,        k_num,  k_ast,    k_ast,     k_pgup,  k_5,        k_7,     k_8,       k_ins,     k_4,        f_12,         k_equals,     k_minus,        k_0         }, \
    { k_tab,    k_w,        k_kp7,  k_kpplus, k_kp8,     k_pgdn,  k_t,        k_y,     k_i,       k_end,     k_e,        k_backspace,  k_squarebrcl, k_squarebrop,   k_o         } \
}

