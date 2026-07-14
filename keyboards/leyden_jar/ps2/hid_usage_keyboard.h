#pragma once

#include <stdint.h>

/*
 * HID Keyboard/Keypad usage IDs (USB HID Usage Table, Usage Page 0x07), given
 * readable names so the scancode tables in hid_to_ps2_set_*.h and the send-path
 * key logic can refer to keys by name instead of bare hex.
 *
 * Values ARE the HID usage IDs, so a constant here indexes the tables directly:
 *     hid_to_ps2_set_2_make[HID_KEYCODE_PAUSE]
 *
 * Source of names + IDs: docs/translate.pdf ("USB HID to PS/2 Scan Code
 * Translation Table", Microsoft 2004) cross-checked against the USB HID Usage
 * Tables. Scope is Usage Page 0x07 only (0x00-0xA4, 0xE0-0xE7); the System
 * (page 0x01) and Consumer/media (page 0x0C) keys that translate.pdf also lists
 * are a different usage page and are intentionally out of scope here.
 *
 * Prefix note: HID_KEYCODE_ is deliberately distinct from TinyUSB's HID_KEY_
 * and QMK's KC_ prefixes (same values, different name) so this header can coexist
 * with either when the project is eventually integrated into QMK.
 */

typedef enum {
    HID_KEYCODE_NONE               = 0x00,  // No event / reserved
    HID_KEYCODE_ERROR_ROLLOVER     = 0x01,  // Too many keys pressed (overrun)
    HID_KEYCODE_POST_FAIL          = 0x02,  // ErrorPOSTFail
    HID_KEYCODE_ERROR_UNDEFINED    = 0x03,  // ErrorUndefined

    // --- Letters ---
    HID_KEYCODE_A                  = 0x04,
    HID_KEYCODE_B                  = 0x05,
    HID_KEYCODE_C                  = 0x06,
    HID_KEYCODE_D                  = 0x07,
    HID_KEYCODE_E                  = 0x08,
    HID_KEYCODE_F                  = 0x09,
    HID_KEYCODE_G                  = 0x0A,
    HID_KEYCODE_H                  = 0x0B,
    HID_KEYCODE_I                  = 0x0C,
    HID_KEYCODE_J                  = 0x0D,
    HID_KEYCODE_K                  = 0x0E,
    HID_KEYCODE_L                  = 0x0F,
    HID_KEYCODE_M                  = 0x10,
    HID_KEYCODE_N                  = 0x11,
    HID_KEYCODE_O                  = 0x12,
    HID_KEYCODE_P                  = 0x13,
    HID_KEYCODE_Q                  = 0x14,
    HID_KEYCODE_R                  = 0x15,
    HID_KEYCODE_S                  = 0x16,
    HID_KEYCODE_T                  = 0x17,
    HID_KEYCODE_U                  = 0x18,
    HID_KEYCODE_V                  = 0x19,
    HID_KEYCODE_W                  = 0x1A,
    HID_KEYCODE_X                  = 0x1B,
    HID_KEYCODE_Y                  = 0x1C,
    HID_KEYCODE_Z                  = 0x1D,

    // --- Number row (unshifted / shifted legend) ---
    HID_KEYCODE_1                  = 0x1E,  // 1 !
    HID_KEYCODE_2                  = 0x1F,  // 2 @
    HID_KEYCODE_3                  = 0x20,  // 3 #
    HID_KEYCODE_4                  = 0x21,  // 4 $
    HID_KEYCODE_5                  = 0x22,  // 5 %
    HID_KEYCODE_6                  = 0x23,  // 6 ^
    HID_KEYCODE_7                  = 0x24,  // 7 &
    HID_KEYCODE_8                  = 0x25,  // 8 *
    HID_KEYCODE_9                  = 0x26,  // 9 (
    HID_KEYCODE_0                  = 0x27,  // 0 )

    // --- Editing / whitespace ---
    HID_KEYCODE_ENTER              = 0x28,  // Return
    HID_KEYCODE_ESCAPE             = 0x29,
    HID_KEYCODE_BACKSPACE          = 0x2A,
    HID_KEYCODE_TAB                = 0x2B,
    HID_KEYCODE_SPACE              = 0x2C,

    // --- Punctuation ---
    HID_KEYCODE_MINUS              = 0x2D,  // - _
    HID_KEYCODE_EQUAL              = 0x2E,  // = +
    HID_KEYCODE_BRACKET_LEFT       = 0x2F,  // [ {
    HID_KEYCODE_BRACKET_RIGHT      = 0x30,  // ] }
    HID_KEYCODE_BACKSLASH          = 0x31,  // \ |
    HID_KEYCODE_EUROPE_1           = 0x32,  // Non-US # ~ (AT-101 key 42)
    HID_KEYCODE_SEMICOLON          = 0x33,  // ; :
    HID_KEYCODE_APOSTROPHE         = 0x34,  // ' "
    HID_KEYCODE_GRAVE              = 0x35,  // ` ~
    HID_KEYCODE_COMMA              = 0x36,  // , <
    HID_KEYCODE_PERIOD             = 0x37,  // . >
    HID_KEYCODE_SLASH              = 0x38,  // / ?
    HID_KEYCODE_CAPS_LOCK          = 0x39,

    // --- Function keys F1-F12 ---
    HID_KEYCODE_F1                 = 0x3A,
    HID_KEYCODE_F2                 = 0x3B,
    HID_KEYCODE_F3                 = 0x3C,
    HID_KEYCODE_F4                 = 0x3D,
    HID_KEYCODE_F5                 = 0x3E,
    HID_KEYCODE_F6                 = 0x3F,
    HID_KEYCODE_F7                 = 0x40,
    HID_KEYCODE_F8                 = 0x41,
    HID_KEYCODE_F9                 = 0x42,
    HID_KEYCODE_F10                = 0x43,
    HID_KEYCODE_F11                = 0x44,
    HID_KEYCODE_F12                = 0x45,

    // --- Control cluster ---
    HID_KEYCODE_PRINT_SCREEN       = 0x46,
    HID_KEYCODE_SCROLL_LOCK        = 0x47,
    HID_KEYCODE_PAUSE              = 0x48,
    HID_KEYCODE_INSERT             = 0x49,
    HID_KEYCODE_HOME               = 0x4A,
    HID_KEYCODE_PAGE_UP            = 0x4B,
    HID_KEYCODE_DELETE             = 0x4C,
    HID_KEYCODE_END                = 0x4D,
    HID_KEYCODE_PAGE_DOWN          = 0x4E,
    HID_KEYCODE_ARROW_RIGHT        = 0x4F,
    HID_KEYCODE_ARROW_LEFT         = 0x50,
    HID_KEYCODE_ARROW_DOWN         = 0x51,
    HID_KEYCODE_ARROW_UP           = 0x52,

    // --- Keypad ---
    HID_KEYCODE_NUM_LOCK           = 0x53,
    HID_KEYCODE_KEYPAD_DIVIDE      = 0x54,  // Keypad /
    HID_KEYCODE_KEYPAD_MULTIPLY    = 0x55,  // Keypad *
    HID_KEYCODE_KEYPAD_SUBTRACT    = 0x56,  // Keypad -
    HID_KEYCODE_KEYPAD_ADD         = 0x57,  // Keypad +
    HID_KEYCODE_KEYPAD_ENTER       = 0x58,
    HID_KEYCODE_KEYPAD_1           = 0x59,  // 1 / End
    HID_KEYCODE_KEYPAD_2           = 0x5A,  // 2 / Down
    HID_KEYCODE_KEYPAD_3           = 0x5B,  // 3 / Page Down
    HID_KEYCODE_KEYPAD_4           = 0x5C,  // 4 / Left
    HID_KEYCODE_KEYPAD_5           = 0x5D,  // 5
    HID_KEYCODE_KEYPAD_6           = 0x5E,  // 6 / Right
    HID_KEYCODE_KEYPAD_7           = 0x5F,  // 7 / Home
    HID_KEYCODE_KEYPAD_8           = 0x60,  // 8 / Up
    HID_KEYCODE_KEYPAD_9           = 0x61,  // 9 / Page Up
    HID_KEYCODE_KEYPAD_0           = 0x62,  // 0 / Insert
    HID_KEYCODE_KEYPAD_DECIMAL     = 0x63,  // . / Delete

    // --- Extras ---
    HID_KEYCODE_EUROPE_2           = 0x64,  // Non-US \ | (AT-101 key 45)
    HID_KEYCODE_APPLICATION        = 0x65,  // App / Menu key
    HID_KEYCODE_POWER              = 0x66,  // Keyboard Power
    HID_KEYCODE_KEYPAD_EQUAL       = 0x67,  // Keypad =

    // --- Function keys F13-F24 ---
    HID_KEYCODE_F13                = 0x68,
    HID_KEYCODE_F14                = 0x69,
    HID_KEYCODE_F15                = 0x6A,
    HID_KEYCODE_F16                = 0x6B,
    HID_KEYCODE_F17                = 0x6C,
    HID_KEYCODE_F18                = 0x6D,
    HID_KEYCODE_F19                = 0x6E,
    HID_KEYCODE_F20                = 0x6F,
    HID_KEYCODE_F21                = 0x70,
    HID_KEYCODE_F22                = 0x71,
    HID_KEYCODE_F23                = 0x72,
    HID_KEYCODE_F24                = 0x73,

    // --- Editing / application commands ---
    HID_KEYCODE_EXECUTE            = 0x74,
    HID_KEYCODE_HELP               = 0x75,
    HID_KEYCODE_MENU               = 0x76,
    HID_KEYCODE_SELECT             = 0x77,
    HID_KEYCODE_STOP               = 0x78,
    HID_KEYCODE_AGAIN              = 0x79,
    HID_KEYCODE_UNDO               = 0x7A,
    HID_KEYCODE_CUT                = 0x7B,
    HID_KEYCODE_COPY               = 0x7C,
    HID_KEYCODE_PASTE              = 0x7D,
    HID_KEYCODE_FIND               = 0x7E,
    HID_KEYCODE_MUTE               = 0x7F,
    HID_KEYCODE_VOLUME_UP          = 0x80,
    HID_KEYCODE_VOLUME_DOWN        = 0x81,

    // --- Locking lock keys (rare) ---
    HID_KEYCODE_LOCKING_CAPS_LOCK  = 0x82,
    HID_KEYCODE_LOCKING_NUM_LOCK   = 0x83,
    HID_KEYCODE_LOCKING_SCROLL_LOCK = 0x84,

    // --- Keypad + international extras ---
    HID_KEYCODE_KEYPAD_COMMA       = 0x85,  // Brazilian keypad .
    HID_KEYCODE_KEYPAD_EQUAL_AS400 = 0x86,
    HID_KEYCODE_INTERNATIONAL_1    = 0x87,  // Ro
    HID_KEYCODE_INTERNATIONAL_2    = 0x88,  // Katakana/Hiragana
    HID_KEYCODE_INTERNATIONAL_3    = 0x89,  // Yen
    HID_KEYCODE_INTERNATIONAL_4    = 0x8A,  // Henkan
    HID_KEYCODE_INTERNATIONAL_5    = 0x8B,  // Muhenkan
    HID_KEYCODE_INTERNATIONAL_6    = 0x8C,  // PC9800 keypad ,
    HID_KEYCODE_INTERNATIONAL_7    = 0x8D,
    HID_KEYCODE_INTERNATIONAL_8    = 0x8E,
    HID_KEYCODE_INTERNATIONAL_9    = 0x8F,

    // --- Language keys ---
    HID_KEYCODE_LANG_1             = 0x90,  // Hangul/English
    HID_KEYCODE_LANG_2             = 0x91,  // Hanja
    HID_KEYCODE_LANG_3             = 0x92,  // Katakana
    HID_KEYCODE_LANG_4             = 0x93,  // Hiragana
    HID_KEYCODE_LANG_5             = 0x94,  // Zenkaku/Hankaku
    HID_KEYCODE_LANG_6             = 0x95,
    HID_KEYCODE_LANG_7             = 0x96,
    HID_KEYCODE_LANG_8             = 0x97,
    HID_KEYCODE_LANG_9             = 0x98,

    // --- Additional editing commands ---
    HID_KEYCODE_ALTERNATE_ERASE    = 0x99,
    HID_KEYCODE_SYSREQ_ATTENTION   = 0x9A,  // SysReq / Attention
    HID_KEYCODE_CANCEL             = 0x9B,
    HID_KEYCODE_CLEAR              = 0x9C,
    HID_KEYCODE_PRIOR              = 0x9D,
    HID_KEYCODE_RETURN             = 0x9E,  // distinct from ENTER (0x28)
    HID_KEYCODE_SEPARATOR          = 0x9F,
    HID_KEYCODE_OUT                = 0xA0,
    HID_KEYCODE_OPER               = 0xA1,
    HID_KEYCODE_CLEAR_AGAIN        = 0xA2,
    HID_KEYCODE_CRSEL_PROPS        = 0xA3,  // CrSel / Props
    HID_KEYCODE_EXSEL              = 0xA4,

    // 0xA5-0xDF reserved by the HID spec

    // --- Modifier keys ---
    HID_KEYCODE_LEFT_CONTROL       = 0xE0,
    HID_KEYCODE_LEFT_SHIFT         = 0xE1,
    HID_KEYCODE_LEFT_ALT           = 0xE2,
    HID_KEYCODE_LEFT_GUI           = 0xE3,  // Left Windows / Command
    HID_KEYCODE_RIGHT_CONTROL      = 0xE4,
    HID_KEYCODE_RIGHT_SHIFT        = 0xE5,
    HID_KEYCODE_RIGHT_ALT          = 0xE6,
    HID_KEYCODE_RIGHT_GUI          = 0xE7,  // Right Windows / Command
} hid_keyboard_usage;
