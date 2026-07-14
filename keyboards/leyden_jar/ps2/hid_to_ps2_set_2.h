#pragma once

#include "hid_to_ps2.h"   // Provides PS2_SCANCODE_SEQ()
#include "hid_usage_keyboard.h"   // Provides HID_KEYCODE_* names

/*
 * Scan code set 2, encoded the same way as set 1 (see hid_to_ps2_set_1.h):
 * a 256-entry table of pointers to length-prefixed PS2_SCANCODE_SEQ() blobs,
 * indexed by USB HID usage ID. Print Screen and Pause are ordinary entries
 * (Pause is 8 bytes here); unassigned keys and make-only breaks are NULL.
 */

const uint8_t *const hid_to_ps2_set_2_make[256] = {
    [HID_KEYCODE_A]               = PS2_SCANCODE_SEQ(0x1C),                                            // A
    [HID_KEYCODE_B]               = PS2_SCANCODE_SEQ(0x32),                                            // B
    [HID_KEYCODE_C]               = PS2_SCANCODE_SEQ(0x21),                                            // C
    [HID_KEYCODE_D]               = PS2_SCANCODE_SEQ(0x23),                                            // D
    [HID_KEYCODE_E]               = PS2_SCANCODE_SEQ(0x24),                                            // E
    [HID_KEYCODE_F]               = PS2_SCANCODE_SEQ(0x2B),                                            // F
    [HID_KEYCODE_G]               = PS2_SCANCODE_SEQ(0x34),                                            // G
    [HID_KEYCODE_H]               = PS2_SCANCODE_SEQ(0x33),                                            // H
    [HID_KEYCODE_I]               = PS2_SCANCODE_SEQ(0x43),                                            // I
    [HID_KEYCODE_J]               = PS2_SCANCODE_SEQ(0x3B),                                            // J
    [HID_KEYCODE_K]               = PS2_SCANCODE_SEQ(0x42),                                            // K
    [HID_KEYCODE_L]               = PS2_SCANCODE_SEQ(0x4B),                                            // L
    [HID_KEYCODE_M]               = PS2_SCANCODE_SEQ(0x3A),                                            // M
    [HID_KEYCODE_N]               = PS2_SCANCODE_SEQ(0x31),                                            // N
    [HID_KEYCODE_O]               = PS2_SCANCODE_SEQ(0x44),                                            // O
    [HID_KEYCODE_P]               = PS2_SCANCODE_SEQ(0x4D),                                            // P
    [HID_KEYCODE_Q]               = PS2_SCANCODE_SEQ(0x15),                                            // Q
    [HID_KEYCODE_R]               = PS2_SCANCODE_SEQ(0x2D),                                            // R
    [HID_KEYCODE_S]               = PS2_SCANCODE_SEQ(0x1B),                                            // S
    [HID_KEYCODE_T]               = PS2_SCANCODE_SEQ(0x2C),                                            // T
    [HID_KEYCODE_U]               = PS2_SCANCODE_SEQ(0x3C),                                            // U
    [HID_KEYCODE_V]               = PS2_SCANCODE_SEQ(0x2A),                                            // V
    [HID_KEYCODE_W]               = PS2_SCANCODE_SEQ(0x1D),                                            // W
    [HID_KEYCODE_X]               = PS2_SCANCODE_SEQ(0x22),                                            // X
    [HID_KEYCODE_Y]               = PS2_SCANCODE_SEQ(0x35),                                            // Y
    [HID_KEYCODE_Z]               = PS2_SCANCODE_SEQ(0x1A),                                            // Z
    [HID_KEYCODE_1]               = PS2_SCANCODE_SEQ(0x16),                                            // 1 !
    [HID_KEYCODE_2]               = PS2_SCANCODE_SEQ(0x1E),                                            // 2 @
    [HID_KEYCODE_3]               = PS2_SCANCODE_SEQ(0x26),                                            // 3 #
    [HID_KEYCODE_4]               = PS2_SCANCODE_SEQ(0x25),                                            // 4 $
    [HID_KEYCODE_5]               = PS2_SCANCODE_SEQ(0x2E),                                            // 5 %
    [HID_KEYCODE_6]               = PS2_SCANCODE_SEQ(0x36),                                            // 6 ^
    [HID_KEYCODE_7]               = PS2_SCANCODE_SEQ(0x3D),                                            // 7 &
    [HID_KEYCODE_8]               = PS2_SCANCODE_SEQ(0x3E),                                            // 8 *
    [HID_KEYCODE_9]               = PS2_SCANCODE_SEQ(0x46),                                            // 9 (
    [HID_KEYCODE_0]               = PS2_SCANCODE_SEQ(0x45),                                            // 0 )
    [HID_KEYCODE_ENTER]           = PS2_SCANCODE_SEQ(0x5A),                                            // Return
    [HID_KEYCODE_ESCAPE]          = PS2_SCANCODE_SEQ(0x76),                                            // Escape
    [HID_KEYCODE_BACKSPACE]       = PS2_SCANCODE_SEQ(0x66),                                            // Backspace
    [HID_KEYCODE_TAB]             = PS2_SCANCODE_SEQ(0x0D),                                            // Tab
    [HID_KEYCODE_SPACE]           = PS2_SCANCODE_SEQ(0x29),                                            // Space
    [HID_KEYCODE_MINUS]           = PS2_SCANCODE_SEQ(0x4E),                                            // - _
    [HID_KEYCODE_EQUAL]           = PS2_SCANCODE_SEQ(0x55),                                            // = +
    [HID_KEYCODE_BRACKET_LEFT]    = PS2_SCANCODE_SEQ(0x54),                                            // [ {
    [HID_KEYCODE_BRACKET_RIGHT]   = PS2_SCANCODE_SEQ(0x5B),                                            // ] }
    [HID_KEYCODE_BACKSLASH]       = PS2_SCANCODE_SEQ(0x5D),                                            // \ |
    [HID_KEYCODE_EUROPE_1]        = PS2_SCANCODE_SEQ(0x5D),                                            // Europe 1
    [HID_KEYCODE_SEMICOLON]       = PS2_SCANCODE_SEQ(0x4C),                                            // ; :
    [HID_KEYCODE_APOSTROPHE]      = PS2_SCANCODE_SEQ(0x52),                                            // ' "
    [HID_KEYCODE_GRAVE]           = PS2_SCANCODE_SEQ(0x0E),                                            // ` ~
    [HID_KEYCODE_COMMA]           = PS2_SCANCODE_SEQ(0x41),                                            // , <
    [HID_KEYCODE_PERIOD]          = PS2_SCANCODE_SEQ(0x49),                                            // . >
    [HID_KEYCODE_SLASH]           = PS2_SCANCODE_SEQ(0x4A),                                            // / ?
    [HID_KEYCODE_CAPS_LOCK]       = PS2_SCANCODE_SEQ(0x58),                                            // Caps Lock
    [HID_KEYCODE_F1]              = PS2_SCANCODE_SEQ(0x05),                                            // F1
    [HID_KEYCODE_F2]              = PS2_SCANCODE_SEQ(0x06),                                            // F2
    [HID_KEYCODE_F3]              = PS2_SCANCODE_SEQ(0x04),                                            // F3
    [HID_KEYCODE_F4]              = PS2_SCANCODE_SEQ(0x0C),                                            // F4
    [HID_KEYCODE_F5]              = PS2_SCANCODE_SEQ(0x03),                                            // F5
    [HID_KEYCODE_F6]              = PS2_SCANCODE_SEQ(0x0B),                                            // F6
    [HID_KEYCODE_F7]              = PS2_SCANCODE_SEQ(0x83),                                            // F7
    [HID_KEYCODE_F8]              = PS2_SCANCODE_SEQ(0x0A),                                            // F8
    [HID_KEYCODE_F9]              = PS2_SCANCODE_SEQ(0x01),                                            // F9
    [HID_KEYCODE_F10]             = PS2_SCANCODE_SEQ(0x09),                                            // F10
    [HID_KEYCODE_F11]             = PS2_SCANCODE_SEQ(0x78),                                            // F11
    [HID_KEYCODE_F12]             = PS2_SCANCODE_SEQ(0x07),                                            // F12
    [HID_KEYCODE_PRINT_SCREEN]    = PS2_SCANCODE_SEQ(0xE0, 0x12, 0xE0, 0x7C),                          // Print Screen
    [HID_KEYCODE_SCROLL_LOCK]     = PS2_SCANCODE_SEQ(0x7E),                                            // Scroll Lock
    [HID_KEYCODE_PAUSE]           = PS2_SCANCODE_SEQ(0xE1, 0x14, 0x77, 0xE1, 0xF0, 0x14, 0xF0, 0x77),  // Pause
    [HID_KEYCODE_INSERT]          = PS2_SCANCODE_SEQ(0xE0, 0x70),                                      // Insert
    [HID_KEYCODE_HOME]            = PS2_SCANCODE_SEQ(0xE0, 0x6C),                                      // Home
    [HID_KEYCODE_PAGE_UP]         = PS2_SCANCODE_SEQ(0xE0, 0x7D),                                      // Page Up
    [HID_KEYCODE_DELETE]          = PS2_SCANCODE_SEQ(0xE0, 0x71),                                      // Delete
    [HID_KEYCODE_END]             = PS2_SCANCODE_SEQ(0xE0, 0x69),                                      // End
    [HID_KEYCODE_PAGE_DOWN]       = PS2_SCANCODE_SEQ(0xE0, 0x7A),                                      // Page Down
    [HID_KEYCODE_ARROW_RIGHT]     = PS2_SCANCODE_SEQ(0xE0, 0x74),                                      // Right Arrow
    [HID_KEYCODE_ARROW_LEFT]      = PS2_SCANCODE_SEQ(0xE0, 0x6B),                                      // Left Arrow
    [HID_KEYCODE_ARROW_DOWN]      = PS2_SCANCODE_SEQ(0xE0, 0x72),                                      // Down Arrow
    [HID_KEYCODE_ARROW_UP]        = PS2_SCANCODE_SEQ(0xE0, 0x75),                                      // Up Arrow
    [HID_KEYCODE_NUM_LOCK]        = PS2_SCANCODE_SEQ(0x77),                                            // Num Lock
    [HID_KEYCODE_KEYPAD_DIVIDE]   = PS2_SCANCODE_SEQ(0xE0, 0x4A),                                      // Keypad /
    [HID_KEYCODE_KEYPAD_MULTIPLY] = PS2_SCANCODE_SEQ(0x7C),                                            // Keypad *
    [HID_KEYCODE_KEYPAD_SUBTRACT] = PS2_SCANCODE_SEQ(0x7B),                                            // Keypad -
    [HID_KEYCODE_KEYPAD_ADD]      = PS2_SCANCODE_SEQ(0x79),                                            // Keypad +
    [HID_KEYCODE_KEYPAD_ENTER]    = PS2_SCANCODE_SEQ(0xE0, 0x5A),                                      // Keypad Enter
    [HID_KEYCODE_KEYPAD_1]        = PS2_SCANCODE_SEQ(0x69),                                            // Keypad 1 End
    [HID_KEYCODE_KEYPAD_2]        = PS2_SCANCODE_SEQ(0x72),                                            // Keypad 2 Down
    [HID_KEYCODE_KEYPAD_3]        = PS2_SCANCODE_SEQ(0x7A),                                            // Keypad 3 Page Down
    [HID_KEYCODE_KEYPAD_4]        = PS2_SCANCODE_SEQ(0x6B),                                            // Keypad 4 Left
    [HID_KEYCODE_KEYPAD_5]        = PS2_SCANCODE_SEQ(0x73),                                            // Keypad 5
    [HID_KEYCODE_KEYPAD_6]        = PS2_SCANCODE_SEQ(0x74),                                            // Keypad 6 Right
    [HID_KEYCODE_KEYPAD_7]        = PS2_SCANCODE_SEQ(0x6C),                                            // Keypad 7 Home
    [HID_KEYCODE_KEYPAD_8]        = PS2_SCANCODE_SEQ(0x75),                                            // Keypad 8 Up
    [HID_KEYCODE_KEYPAD_9]        = PS2_SCANCODE_SEQ(0x7D),                                            // Keypad 9 Page Up
    [HID_KEYCODE_KEYPAD_0]        = PS2_SCANCODE_SEQ(0x70),                                            // Keypad 0 Insert
    [HID_KEYCODE_KEYPAD_DECIMAL]  = PS2_SCANCODE_SEQ(0x71),                                            // Keypad . Delete
    [HID_KEYCODE_EUROPE_2]        = PS2_SCANCODE_SEQ(0x61),                                            // Europe 2
    [HID_KEYCODE_APPLICATION]     = PS2_SCANCODE_SEQ(0xE0, 0x2F),                                      // App
    [HID_KEYCODE_POWER]           = PS2_SCANCODE_SEQ(0xE0, 0x37),                                      // Keyboard Power
    [HID_KEYCODE_KEYPAD_EQUAL]    = PS2_SCANCODE_SEQ(0x0F),                                            // Keypad =
    [HID_KEYCODE_F13]             = PS2_SCANCODE_SEQ(0x08),                                            // F13
    [HID_KEYCODE_F14]             = PS2_SCANCODE_SEQ(0x10),                                            // F14
    [HID_KEYCODE_F15]             = PS2_SCANCODE_SEQ(0x18),                                            // F15
    [HID_KEYCODE_F16]             = PS2_SCANCODE_SEQ(0x20),                                            // F16
    [HID_KEYCODE_F17]             = PS2_SCANCODE_SEQ(0x28),                                            // F17
    [HID_KEYCODE_F18]             = PS2_SCANCODE_SEQ(0x30),                                            // F18
    [HID_KEYCODE_F19]             = PS2_SCANCODE_SEQ(0x38),                                            // F19
    [HID_KEYCODE_F20]             = PS2_SCANCODE_SEQ(0x40),                                            // F20
    [HID_KEYCODE_F21]             = PS2_SCANCODE_SEQ(0x48),                                            // F21
    [HID_KEYCODE_F22]             = PS2_SCANCODE_SEQ(0x50),                                            // F22
    [HID_KEYCODE_F23]             = PS2_SCANCODE_SEQ(0x57),                                            // F23
    [HID_KEYCODE_F24]             = PS2_SCANCODE_SEQ(0x5F),                                            // F24
    [HID_KEYCODE_KEYPAD_COMMA]    = PS2_SCANCODE_SEQ(0x6D),                                            // Keypad , (Brazilian Keypad .)
    [HID_KEYCODE_INTERNATIONAL_1] = PS2_SCANCODE_SEQ(0x51),                                            // Int'l 1 (Ro)
    [HID_KEYCODE_INTERNATIONAL_2] = PS2_SCANCODE_SEQ(0x13),                                            // Int'l 2 (Katakana/Hiragana)
    [HID_KEYCODE_INTERNATIONAL_3] = PS2_SCANCODE_SEQ(0x6A),                                            // Int'l 3 (Yen)
    [HID_KEYCODE_INTERNATIONAL_4] = PS2_SCANCODE_SEQ(0x64),                                            // Int'l 4 (Henkan)
    [HID_KEYCODE_INTERNATIONAL_5] = PS2_SCANCODE_SEQ(0x67),                                            // Int'l 5 (Muhenkan)
    [HID_KEYCODE_INTERNATIONAL_6] = PS2_SCANCODE_SEQ(0x27),                                            // Int'l 6 (PC9800 Keypad ,)
    [HID_KEYCODE_LANG_1]          = PS2_SCANCODE_SEQ(0xF2),                                            // Lang 1 (Hangul/English)  make-only
    [HID_KEYCODE_LANG_2]          = PS2_SCANCODE_SEQ(0xF1),                                            // Lang 2 (Hanja)           make-only
    [HID_KEYCODE_LANG_3]          = PS2_SCANCODE_SEQ(0x63),                                            // Lang 3 (Katakana)
    [HID_KEYCODE_LANG_4]          = PS2_SCANCODE_SEQ(0x62),                                            // Lang 4 (Hiragana)
    [HID_KEYCODE_LANG_5]          = PS2_SCANCODE_SEQ(0x5F),                                            // Lang 5 (Zenkaku/Hankaku)
    [HID_KEYCODE_LEFT_CONTROL]    = PS2_SCANCODE_SEQ(0x14),                                            // Left Control
    [HID_KEYCODE_LEFT_SHIFT]      = PS2_SCANCODE_SEQ(0x12),                                            // Left Shift
    [HID_KEYCODE_LEFT_ALT]        = PS2_SCANCODE_SEQ(0x11),                                            // Left Alt
    [HID_KEYCODE_LEFT_GUI]        = PS2_SCANCODE_SEQ(0xE0, 0x1F),                                      // Left GUI
    [HID_KEYCODE_RIGHT_CONTROL]   = PS2_SCANCODE_SEQ(0xE0, 0x14),                                      // Right Control
    [HID_KEYCODE_RIGHT_SHIFT]     = PS2_SCANCODE_SEQ(0x59),                                            // Right Shift
    [HID_KEYCODE_RIGHT_ALT]       = PS2_SCANCODE_SEQ(0xE0, 0x11),                                      // Right Alt
    [HID_KEYCODE_RIGHT_GUI]       = PS2_SCANCODE_SEQ(0xE0, 0x27),                                      // Right GUI
};

const uint8_t *const hid_to_ps2_set_2_break[256] = {
    [HID_KEYCODE_A]               = PS2_SCANCODE_SEQ(0xF0, 0x1C),                          // A
    [HID_KEYCODE_B]               = PS2_SCANCODE_SEQ(0xF0, 0x32),                          // B
    [HID_KEYCODE_C]               = PS2_SCANCODE_SEQ(0xF0, 0x21),                          // C
    [HID_KEYCODE_D]               = PS2_SCANCODE_SEQ(0xF0, 0x23),                          // D
    [HID_KEYCODE_E]               = PS2_SCANCODE_SEQ(0xF0, 0x24),                          // E
    [HID_KEYCODE_F]               = PS2_SCANCODE_SEQ(0xF0, 0x2B),                          // F
    [HID_KEYCODE_G]               = PS2_SCANCODE_SEQ(0xF0, 0x34),                          // G
    [HID_KEYCODE_H]               = PS2_SCANCODE_SEQ(0xF0, 0x33),                          // H
    [HID_KEYCODE_I]               = PS2_SCANCODE_SEQ(0xF0, 0x43),                          // I
    [HID_KEYCODE_J]               = PS2_SCANCODE_SEQ(0xF0, 0x3B),                          // J
    [HID_KEYCODE_K]               = PS2_SCANCODE_SEQ(0xF0, 0x42),                          // K
    [HID_KEYCODE_L]               = PS2_SCANCODE_SEQ(0xF0, 0x4B),                          // L
    [HID_KEYCODE_M]               = PS2_SCANCODE_SEQ(0xF0, 0x3A),                          // M
    [HID_KEYCODE_N]               = PS2_SCANCODE_SEQ(0xF0, 0x31),                          // N
    [HID_KEYCODE_O]               = PS2_SCANCODE_SEQ(0xF0, 0x44),                          // O
    [HID_KEYCODE_P]               = PS2_SCANCODE_SEQ(0xF0, 0x4D),                          // P
    [HID_KEYCODE_Q]               = PS2_SCANCODE_SEQ(0xF0, 0x15),                          // Q
    [HID_KEYCODE_R]               = PS2_SCANCODE_SEQ(0xF0, 0x2D),                          // R
    [HID_KEYCODE_S]               = PS2_SCANCODE_SEQ(0xF0, 0x1B),                          // S
    [HID_KEYCODE_T]               = PS2_SCANCODE_SEQ(0xF0, 0x2C),                          // T
    [HID_KEYCODE_U]               = PS2_SCANCODE_SEQ(0xF0, 0x3C),                          // U
    [HID_KEYCODE_V]               = PS2_SCANCODE_SEQ(0xF0, 0x2A),                          // V
    [HID_KEYCODE_W]               = PS2_SCANCODE_SEQ(0xF0, 0x1D),                          // W
    [HID_KEYCODE_X]               = PS2_SCANCODE_SEQ(0xF0, 0x22),                          // X
    [HID_KEYCODE_Y]               = PS2_SCANCODE_SEQ(0xF0, 0x35),                          // Y
    [HID_KEYCODE_Z]               = PS2_SCANCODE_SEQ(0xF0, 0x1A),                          // Z
    [HID_KEYCODE_1]               = PS2_SCANCODE_SEQ(0xF0, 0x16),                          // 1 !
    [HID_KEYCODE_2]               = PS2_SCANCODE_SEQ(0xF0, 0x1E),                          // 2 @
    [HID_KEYCODE_3]               = PS2_SCANCODE_SEQ(0xF0, 0x26),                          // 3 #
    [HID_KEYCODE_4]               = PS2_SCANCODE_SEQ(0xF0, 0x25),                          // 4 $
    [HID_KEYCODE_5]               = PS2_SCANCODE_SEQ(0xF0, 0x2E),                          // 5 %
    [HID_KEYCODE_6]               = PS2_SCANCODE_SEQ(0xF0, 0x36),                          // 6 ^
    [HID_KEYCODE_7]               = PS2_SCANCODE_SEQ(0xF0, 0x3D),                          // 7 &
    [HID_KEYCODE_8]               = PS2_SCANCODE_SEQ(0xF0, 0x3E),                          // 8 *
    [HID_KEYCODE_9]               = PS2_SCANCODE_SEQ(0xF0, 0x46),                          // 9 (
    [HID_KEYCODE_0]               = PS2_SCANCODE_SEQ(0xF0, 0x45),                          // 0 )
    [HID_KEYCODE_ENTER]           = PS2_SCANCODE_SEQ(0xF0, 0x5A),                          // Return
    [HID_KEYCODE_ESCAPE]          = PS2_SCANCODE_SEQ(0xF0, 0x76),                          // Escape
    [HID_KEYCODE_BACKSPACE]       = PS2_SCANCODE_SEQ(0xF0, 0x66),                          // Backspace
    [HID_KEYCODE_TAB]             = PS2_SCANCODE_SEQ(0xF0, 0x0D),                          // Tab
    [HID_KEYCODE_SPACE]           = PS2_SCANCODE_SEQ(0xF0, 0x29),                          // Space
    [HID_KEYCODE_MINUS]           = PS2_SCANCODE_SEQ(0xF0, 0x4E),                          // - _
    [HID_KEYCODE_EQUAL]           = PS2_SCANCODE_SEQ(0xF0, 0x55),                          // = +
    [HID_KEYCODE_BRACKET_LEFT]    = PS2_SCANCODE_SEQ(0xF0, 0x54),                          // [ {
    [HID_KEYCODE_BRACKET_RIGHT]   = PS2_SCANCODE_SEQ(0xF0, 0x5B),                          // ] }
    [HID_KEYCODE_BACKSLASH]       = PS2_SCANCODE_SEQ(0xF0, 0x5D),                          // \ |
    [HID_KEYCODE_EUROPE_1]        = PS2_SCANCODE_SEQ(0xF0, 0x5D),                          // Europe 1
    [HID_KEYCODE_SEMICOLON]       = PS2_SCANCODE_SEQ(0xF0, 0x4C),                          // ; :
    [HID_KEYCODE_APOSTROPHE]      = PS2_SCANCODE_SEQ(0xF0, 0x52),                          // ' "
    [HID_KEYCODE_GRAVE]           = PS2_SCANCODE_SEQ(0xF0, 0x0E),                          // ` ~
    [HID_KEYCODE_COMMA]           = PS2_SCANCODE_SEQ(0xF0, 0x41),                          // , <
    [HID_KEYCODE_PERIOD]          = PS2_SCANCODE_SEQ(0xF0, 0x49),                          // . >
    [HID_KEYCODE_SLASH]           = PS2_SCANCODE_SEQ(0xF0, 0x4A),                          // / ?
    [HID_KEYCODE_CAPS_LOCK]       = PS2_SCANCODE_SEQ(0xF0, 0x58),                          // Caps Lock
    [HID_KEYCODE_F1]              = PS2_SCANCODE_SEQ(0xF0, 0x05),                          // F1
    [HID_KEYCODE_F2]              = PS2_SCANCODE_SEQ(0xF0, 0x06),                          // F2
    [HID_KEYCODE_F3]              = PS2_SCANCODE_SEQ(0xF0, 0x04),                          // F3
    [HID_KEYCODE_F4]              = PS2_SCANCODE_SEQ(0xF0, 0x0C),                          // F4
    [HID_KEYCODE_F5]              = PS2_SCANCODE_SEQ(0xF0, 0x03),                          // F5
    [HID_KEYCODE_F6]              = PS2_SCANCODE_SEQ(0xF0, 0x0B),                          // F6
    [HID_KEYCODE_F7]              = PS2_SCANCODE_SEQ(0xF0, 0x83),                          // F7
    [HID_KEYCODE_F8]              = PS2_SCANCODE_SEQ(0xF0, 0x0A),                          // F8
    [HID_KEYCODE_F9]              = PS2_SCANCODE_SEQ(0xF0, 0x01),                          // F9
    [HID_KEYCODE_F10]             = PS2_SCANCODE_SEQ(0xF0, 0x09),                          // F10
    [HID_KEYCODE_F11]             = PS2_SCANCODE_SEQ(0xF0, 0x78),                          // F11
    [HID_KEYCODE_F12]             = PS2_SCANCODE_SEQ(0xF0, 0x07),                          // F12
    [HID_KEYCODE_PRINT_SCREEN]    = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x7C, 0xE0, 0xF0, 0x12),  // Print Screen
    [HID_KEYCODE_SCROLL_LOCK]     = PS2_SCANCODE_SEQ(0xF0, 0x7E),                          // Scroll Lock
    // HID_KEYCODE_PAUSE has no break code - the make sequence is self-contained (NULL)
    [HID_KEYCODE_INSERT]          = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x70),                    // Insert
    [HID_KEYCODE_HOME]            = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x6C),                    // Home
    [HID_KEYCODE_PAGE_UP]         = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x7D),                    // Page Up
    [HID_KEYCODE_DELETE]          = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x71),                    // Delete
    [HID_KEYCODE_END]             = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x69),                    // End
    [HID_KEYCODE_PAGE_DOWN]       = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x7A),                    // Page Down
    [HID_KEYCODE_ARROW_RIGHT]     = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x74),                    // Right Arrow
    [HID_KEYCODE_ARROW_LEFT]      = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x6B),                    // Left Arrow
    [HID_KEYCODE_ARROW_DOWN]      = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x72),                    // Down Arrow
    [HID_KEYCODE_ARROW_UP]        = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x75),                    // Up Arrow
    [HID_KEYCODE_NUM_LOCK]        = PS2_SCANCODE_SEQ(0xF0, 0x77),                          // Num Lock
    [HID_KEYCODE_KEYPAD_DIVIDE]   = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x4A),                    // Keypad /
    [HID_KEYCODE_KEYPAD_MULTIPLY] = PS2_SCANCODE_SEQ(0xF0, 0x7C),                          // Keypad *
    [HID_KEYCODE_KEYPAD_SUBTRACT] = PS2_SCANCODE_SEQ(0xF0, 0x7B),                          // Keypad -
    [HID_KEYCODE_KEYPAD_ADD]      = PS2_SCANCODE_SEQ(0xF0, 0x79),                          // Keypad +
    [HID_KEYCODE_KEYPAD_ENTER]    = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x5A),                    // Keypad Enter
    [HID_KEYCODE_KEYPAD_1]        = PS2_SCANCODE_SEQ(0xF0, 0x69),                          // Keypad 1 End
    [HID_KEYCODE_KEYPAD_2]        = PS2_SCANCODE_SEQ(0xF0, 0x72),                          // Keypad 2 Down
    [HID_KEYCODE_KEYPAD_3]        = PS2_SCANCODE_SEQ(0xF0, 0x7A),                          // Keypad 3 Page Down
    [HID_KEYCODE_KEYPAD_4]        = PS2_SCANCODE_SEQ(0xF0, 0x6B),                          // Keypad 4 Left
    [HID_KEYCODE_KEYPAD_5]        = PS2_SCANCODE_SEQ(0xF0, 0x73),                          // Keypad 5
    [HID_KEYCODE_KEYPAD_6]        = PS2_SCANCODE_SEQ(0xF0, 0x74),                          // Keypad 6 Right
    [HID_KEYCODE_KEYPAD_7]        = PS2_SCANCODE_SEQ(0xF0, 0x6C),                          // Keypad 7 Home
    [HID_KEYCODE_KEYPAD_8]        = PS2_SCANCODE_SEQ(0xF0, 0x75),                          // Keypad 8 Up
    [HID_KEYCODE_KEYPAD_9]        = PS2_SCANCODE_SEQ(0xF0, 0x7D),                          // Keypad 9 Page Up
    [HID_KEYCODE_KEYPAD_0]        = PS2_SCANCODE_SEQ(0xF0, 0x70),                          // Keypad 0 Insert
    [HID_KEYCODE_KEYPAD_DECIMAL]  = PS2_SCANCODE_SEQ(0xF0, 0x71),                          // Keypad . Delete
    [HID_KEYCODE_EUROPE_2]        = PS2_SCANCODE_SEQ(0xF0, 0x61),                          // Europe 2
    [HID_KEYCODE_APPLICATION]     = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x2F),                    // App
    [HID_KEYCODE_POWER]           = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x37),                    // Keyboard Power
    [HID_KEYCODE_KEYPAD_EQUAL]    = PS2_SCANCODE_SEQ(0xF0, 0x0F),                          // Keypad =
    [HID_KEYCODE_F13]             = PS2_SCANCODE_SEQ(0xF0, 0x08),                          // F13
    [HID_KEYCODE_F14]             = PS2_SCANCODE_SEQ(0xF0, 0x10),                          // F14
    [HID_KEYCODE_F15]             = PS2_SCANCODE_SEQ(0xF0, 0x18),                          // F15
    [HID_KEYCODE_F16]             = PS2_SCANCODE_SEQ(0xF0, 0x20),                          // F16
    [HID_KEYCODE_F17]             = PS2_SCANCODE_SEQ(0xF0, 0x28),                          // F17
    [HID_KEYCODE_F18]             = PS2_SCANCODE_SEQ(0xF0, 0x30),                          // F18
    [HID_KEYCODE_F19]             = PS2_SCANCODE_SEQ(0xF0, 0x38),                          // F19
    [HID_KEYCODE_F20]             = PS2_SCANCODE_SEQ(0xF0, 0x40),                          // F20
    [HID_KEYCODE_F21]             = PS2_SCANCODE_SEQ(0xF0, 0x48),                          // F21
    [HID_KEYCODE_F22]             = PS2_SCANCODE_SEQ(0xF0, 0x50),                          // F22
    [HID_KEYCODE_F23]             = PS2_SCANCODE_SEQ(0xF0, 0x57),                          // F23
    [HID_KEYCODE_F24]             = PS2_SCANCODE_SEQ(0xF0, 0x5F),                          // F24
    [HID_KEYCODE_KEYPAD_COMMA]    = PS2_SCANCODE_SEQ(0xF0, 0x6D),                          // Keypad , (Brazilian Keypad .)
    [HID_KEYCODE_INTERNATIONAL_1] = PS2_SCANCODE_SEQ(0xF0, 0x51),                          // Int'l 1 (Ro)
    [HID_KEYCODE_INTERNATIONAL_2] = PS2_SCANCODE_SEQ(0xF0, 0x13),                          // Int'l 2 (Katakana/Hiragana)
    [HID_KEYCODE_INTERNATIONAL_3] = PS2_SCANCODE_SEQ(0xF0, 0x6A),                          // Int'l 3 (Yen)
    [HID_KEYCODE_INTERNATIONAL_4] = PS2_SCANCODE_SEQ(0xF0, 0x64),                          // Int'l 4 (Henkan)
    [HID_KEYCODE_INTERNATIONAL_5] = PS2_SCANCODE_SEQ(0xF0, 0x67),                          // Int'l 5 (Muhenkan)
    [HID_KEYCODE_INTERNATIONAL_6] = PS2_SCANCODE_SEQ(0xF0, 0x27),                          // Int'l 6 (PC9800 Keypad ,)
    // HID_KEYCODE_LANG_1 make-only (NULL break)
    // HID_KEYCODE_LANG_2 make-only (NULL break)
    [HID_KEYCODE_LANG_3]          = PS2_SCANCODE_SEQ(0xF0, 0x63),                          // Lang 3 (Katakana)
    [HID_KEYCODE_LANG_4]          = PS2_SCANCODE_SEQ(0xF0, 0x62),                          // Lang 4 (Hiragana)
    [HID_KEYCODE_LANG_5]          = PS2_SCANCODE_SEQ(0xF0, 0x5F),                          // Lang 5 (Zenkaku/Hankaku)
    [HID_KEYCODE_LEFT_CONTROL]    = PS2_SCANCODE_SEQ(0xF0, 0x14),                          // Left Control
    [HID_KEYCODE_LEFT_SHIFT]      = PS2_SCANCODE_SEQ(0xF0, 0x12),                          // Left Shift
    [HID_KEYCODE_LEFT_ALT]        = PS2_SCANCODE_SEQ(0xF0, 0x11),                          // Left Alt
    [HID_KEYCODE_LEFT_GUI]        = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x1F),                    // Left GUI
    [HID_KEYCODE_RIGHT_CONTROL]   = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x14),                    // Right Control
    [HID_KEYCODE_RIGHT_SHIFT]     = PS2_SCANCODE_SEQ(0xF0, 0x59),                          // Right Shift
    [HID_KEYCODE_RIGHT_ALT]       = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x11),                    // Right Alt
    [HID_KEYCODE_RIGHT_GUI]       = PS2_SCANCODE_SEQ(0xE0, 0xF0, 0x27),                    // Right GUI
};

/*
 * Ctrl + Pause emits "Break" instead of "Pause". The HID Pause key (0x48)
 * reports the same usage whether or not Ctrl is held, so the send layer picks
 * between hid_to_ps2_set_2_make[HID_KEYCODE_PAUSE] (plain Pause) and this blob
 * when Ctrl is down. Like Pause, Break is a one-shot: the whole make+break burst
 * is emitted on the press and nothing on release, so it never repeats. This
 * matches real IBM/AT hardware and ps2x2pico. Length-prefixed, table format.
 */
const uint8_t *const ps2_ctrl_break_set_2 = PS2_SCANCODE_SEQ(0xE0, 0x7E, 0xE0, 0xF0, 0x7E);
