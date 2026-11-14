#pragma once

#define DYNAMIC_KEYMAP_LAYER_COUNT 3

#define VIAL_KEYBOARD_UID {0xA6, 0xDA, 0xB1, 0xFA, 0xBD, 0x12, 0xB6, 0x9B}

/* Esc + F1 (WCASS matrix [4,0] and [4,1]) unlock Vial */
#define VIAL_UNLOCK_COMBO_ROWS {4, 4}
#define VIAL_UNLOCK_COMBO_COLS {0, 1}

/* Esc is the bootmagic key */
#define BOOTMAGIC_LITE_ROW 4
#define BOOTMAGIC_LITE_COLUMN 0

#define HAPTIC_OFF_IN_LOW_POWER 1
