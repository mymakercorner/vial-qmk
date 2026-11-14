// Copyright 2026 QMK
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include_next <board.h>

#undef BOARD_RP_PICO2_RP2350
#define BOARD_GENERIC_RP2350B

#undef BOARD_NAME
#define BOARD_NAME "Generic RP2350B"

#if !defined(RP2350B_QFN80)
#    error "GENERIC_RP_RP2350B: RP2350B_QFN80 must come from board.mk"
#endif
