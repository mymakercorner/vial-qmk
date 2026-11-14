# Generic RP2350B (QFN-80, GPIO0-47) board: the ChibiOS Pico 2 board files
# with the QFN-80 package selected. RP2350B_QFN80 must be visible to every
# ChibiOS and QMK compile unit (rp_registry.h: 48 GPIO lines, 6 IO_BANK0
# interrupt registers, ADC on GPIO40-47), so it is a compiler define rather
# than a board.h one: osal.h is included before board.h.
BOARDSRC = $(CHIBIOS)/os/hal/boards/RP_PICO2_RP2350/board.c
BOARDINC = $(CHIBIOS)/os/hal/boards/RP_PICO2_RP2350

ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)

OPT_DEFS += -DRP2350B_QFN80
