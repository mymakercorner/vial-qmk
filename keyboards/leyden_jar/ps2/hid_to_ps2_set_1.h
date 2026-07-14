#pragma once

#include "hid_to_ps2.h"   // Provides PS2_SCANCODE_SEQ()
#include "hid_usage_keyboard.h"   // Provides HID_KEYCODE_* names

/*
 * Note: at the request of a human (Rico), Claude was used to find this
 * scancode packing method, in order to avoid the special-case handling that
 * Print Screen and Pause previously required. The previous packed uint32_t
 * encoding could hold at most 3 scancode bytes, but those two keys need 4-6.
 *
 * Encoding:
 *   Each table is a 256-entry array of pointers, indexed directly by the USB
 *   HID Keyboard/Keypad usage ID (page 0x07). Each pointer refers to a
 *   length-prefixed blob: element [0] is the number of scancode bytes that
 *   follow, then the bytes in transmission order.
 *
 *   Unassigned keys are NULL for free (no 0x00000000 placeholder needed).
 *   Print Screen and Pause are ordinary entries now - no branch in the send
 *   path, no separate uint8_t arrays.
 *
 * PS2_SCANCODE_SEQ() (defined in hid_to_ps2.h) builds one such blob as a
 * compound literal (static storage duration at file scope) and computes its
 * length with sizeof, so the length byte can never drift out of sync with the
 * data.
 *
 * Designated initializers ([index] = ...) mean row order no longer has to be
 * hand-counted - the index is stated explicitly, which kills the ordering
 * fragility that caused the earlier audit bugs.
 *
 * Intended send loop (see upcoming send-path work):
 *
 *     const uint8_t *seq = hid_to_ps2_set_1_make[hid];
 *     if (seq) {
 *         uint8_t len = *seq++;
 *         while (len--) send_byte(*seq++);   // via byte_to_ps2_frame()
 *     }
 *
 * A NULL entry (unassigned key, or the break of a make-only key) sends nothing.
 */

const uint8_t *const hid_to_ps2_set_1_make[256] = {
    [HID_KEYCODE_A]               = PS2_SCANCODE_SEQ(0x1E),                                // A
    [HID_KEYCODE_B]               = PS2_SCANCODE_SEQ(0x30),                                // B
    [HID_KEYCODE_C]               = PS2_SCANCODE_SEQ(0x2E),                                // C
    [HID_KEYCODE_D]               = PS2_SCANCODE_SEQ(0x20),                                // D
    [HID_KEYCODE_E]               = PS2_SCANCODE_SEQ(0x12),                                // E
    [HID_KEYCODE_F]               = PS2_SCANCODE_SEQ(0x21),                                // F
    [HID_KEYCODE_G]               = PS2_SCANCODE_SEQ(0x22),                                // G
    [HID_KEYCODE_H]               = PS2_SCANCODE_SEQ(0x23),                                // H
    [HID_KEYCODE_I]               = PS2_SCANCODE_SEQ(0x17),                                // I
    [HID_KEYCODE_J]               = PS2_SCANCODE_SEQ(0x24),                                // J
    [HID_KEYCODE_K]               = PS2_SCANCODE_SEQ(0x25),                                // K
    [HID_KEYCODE_L]               = PS2_SCANCODE_SEQ(0x26),                                // L
    [HID_KEYCODE_M]               = PS2_SCANCODE_SEQ(0x32),                                // M
    [HID_KEYCODE_N]               = PS2_SCANCODE_SEQ(0x31),                                // N
    [HID_KEYCODE_O]               = PS2_SCANCODE_SEQ(0x18),                                // O
    [HID_KEYCODE_P]               = PS2_SCANCODE_SEQ(0x19),                                // P
    [HID_KEYCODE_Q]               = PS2_SCANCODE_SEQ(0x10),                                // Q
    [HID_KEYCODE_R]               = PS2_SCANCODE_SEQ(0x13),                                // R
    [HID_KEYCODE_S]               = PS2_SCANCODE_SEQ(0x1F),                                // S
    [HID_KEYCODE_T]               = PS2_SCANCODE_SEQ(0x14),                                // T
    [HID_KEYCODE_U]               = PS2_SCANCODE_SEQ(0x16),                                // U
    [HID_KEYCODE_V]               = PS2_SCANCODE_SEQ(0x2F),                                // V
    [HID_KEYCODE_W]               = PS2_SCANCODE_SEQ(0x11),                                // W
    [HID_KEYCODE_X]               = PS2_SCANCODE_SEQ(0x2D),                                // X
    [HID_KEYCODE_Y]               = PS2_SCANCODE_SEQ(0x15),                                // Y
    [HID_KEYCODE_Z]               = PS2_SCANCODE_SEQ(0x2C),                                // Z
    [HID_KEYCODE_1]               = PS2_SCANCODE_SEQ(0x02),                                // 1 !
    [HID_KEYCODE_2]               = PS2_SCANCODE_SEQ(0x03),                                // 2 @
    [HID_KEYCODE_3]               = PS2_SCANCODE_SEQ(0x04),                                // 3 #
    [HID_KEYCODE_4]               = PS2_SCANCODE_SEQ(0x05),                                // 4 $
    [HID_KEYCODE_5]               = PS2_SCANCODE_SEQ(0x06),                                // 5 %
    [HID_KEYCODE_6]               = PS2_SCANCODE_SEQ(0x07),                                // 6 ^
    [HID_KEYCODE_7]               = PS2_SCANCODE_SEQ(0x08),                                // 7 &
    [HID_KEYCODE_8]               = PS2_SCANCODE_SEQ(0x09),                                // 8 *
    [HID_KEYCODE_9]               = PS2_SCANCODE_SEQ(0x0A),                                // 9 (
    [HID_KEYCODE_0]               = PS2_SCANCODE_SEQ(0x0B),                                // 0 )
    [HID_KEYCODE_ENTER]           = PS2_SCANCODE_SEQ(0x1C),                                // Return
    [HID_KEYCODE_ESCAPE]          = PS2_SCANCODE_SEQ(0x01),                                // Escape
    [HID_KEYCODE_BACKSPACE]       = PS2_SCANCODE_SEQ(0x0E),                                // Backspace
    [HID_KEYCODE_TAB]             = PS2_SCANCODE_SEQ(0x0F),                                // Tab
    [HID_KEYCODE_SPACE]           = PS2_SCANCODE_SEQ(0x39),                                // Space
    [HID_KEYCODE_MINUS]           = PS2_SCANCODE_SEQ(0x0C),                                // - _
    [HID_KEYCODE_EQUAL]           = PS2_SCANCODE_SEQ(0x0D),                                // = +
    [HID_KEYCODE_BRACKET_LEFT]    = PS2_SCANCODE_SEQ(0x1A),                                // [ {
    [HID_KEYCODE_BRACKET_RIGHT]   = PS2_SCANCODE_SEQ(0x1B),                                // ] }
    [HID_KEYCODE_BACKSLASH]       = PS2_SCANCODE_SEQ(0x2B),                                // \ |
    [HID_KEYCODE_EUROPE_1]        = PS2_SCANCODE_SEQ(0x2B),                                // Europe 1
    [HID_KEYCODE_SEMICOLON]       = PS2_SCANCODE_SEQ(0x27),                                // ; :
    [HID_KEYCODE_APOSTROPHE]      = PS2_SCANCODE_SEQ(0x28),                                // ' "
    [HID_KEYCODE_GRAVE]           = PS2_SCANCODE_SEQ(0x29),                                // ` ~
    [HID_KEYCODE_COMMA]           = PS2_SCANCODE_SEQ(0x33),                                // , <
    [HID_KEYCODE_PERIOD]          = PS2_SCANCODE_SEQ(0x34),                                // . >
    [HID_KEYCODE_SLASH]           = PS2_SCANCODE_SEQ(0x35),                                // / ?
    [HID_KEYCODE_CAPS_LOCK]       = PS2_SCANCODE_SEQ(0x3A),                                // Caps Lock
    [HID_KEYCODE_F1]              = PS2_SCANCODE_SEQ(0x3B),                                // F1
    [HID_KEYCODE_F2]              = PS2_SCANCODE_SEQ(0x3C),                                // F2
    [HID_KEYCODE_F3]              = PS2_SCANCODE_SEQ(0x3D),                                // F3
    [HID_KEYCODE_F4]              = PS2_SCANCODE_SEQ(0x3E),                                // F4
    [HID_KEYCODE_F5]              = PS2_SCANCODE_SEQ(0x3F),                                // F5
    [HID_KEYCODE_F6]              = PS2_SCANCODE_SEQ(0x40),                                // F6
    [HID_KEYCODE_F7]              = PS2_SCANCODE_SEQ(0x41),                                // F7
    [HID_KEYCODE_F8]              = PS2_SCANCODE_SEQ(0x42),                                // F8
    [HID_KEYCODE_F9]              = PS2_SCANCODE_SEQ(0x43),                                // F9
    [HID_KEYCODE_F10]             = PS2_SCANCODE_SEQ(0x44),                                // F10
    [HID_KEYCODE_F11]             = PS2_SCANCODE_SEQ(0x57),                                // F11
    [HID_KEYCODE_F12]             = PS2_SCANCODE_SEQ(0x58),                                // F12
    [HID_KEYCODE_PRINT_SCREEN]    = PS2_SCANCODE_SEQ(0xE0, 0x2A, 0xE0, 0x37),              // Print Screen
    [HID_KEYCODE_SCROLL_LOCK]     = PS2_SCANCODE_SEQ(0x46),                                // Scroll Lock
    [HID_KEYCODE_PAUSE]           = PS2_SCANCODE_SEQ(0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5),  // Pause
    [HID_KEYCODE_INSERT]          = PS2_SCANCODE_SEQ(0xE0, 0x52),                          // Insert
    [HID_KEYCODE_HOME]            = PS2_SCANCODE_SEQ(0xE0, 0x47),                          // Home
    [HID_KEYCODE_PAGE_UP]         = PS2_SCANCODE_SEQ(0xE0, 0x49),                          // Page Up
    [HID_KEYCODE_DELETE]          = PS2_SCANCODE_SEQ(0xE0, 0x53),                          // Delete
    [HID_KEYCODE_END]             = PS2_SCANCODE_SEQ(0xE0, 0x4F),                          // End
    [HID_KEYCODE_PAGE_DOWN]       = PS2_SCANCODE_SEQ(0xE0, 0x51),                          // Page Down
    [HID_KEYCODE_ARROW_RIGHT]     = PS2_SCANCODE_SEQ(0xE0, 0x4D),                          // Right Arrow
    [HID_KEYCODE_ARROW_LEFT]      = PS2_SCANCODE_SEQ(0xE0, 0x4B),                          // Left Arrow
    [HID_KEYCODE_ARROW_DOWN]      = PS2_SCANCODE_SEQ(0xE0, 0x50),                          // Down Arrow
    [HID_KEYCODE_ARROW_UP]        = PS2_SCANCODE_SEQ(0xE0, 0x48),                          // Up Arrow
    [HID_KEYCODE_NUM_LOCK]        = PS2_SCANCODE_SEQ(0x45),                                // Num Lock
    [HID_KEYCODE_KEYPAD_DIVIDE]   = PS2_SCANCODE_SEQ(0xE0, 0x35),                          // Keypad /
    [HID_KEYCODE_KEYPAD_MULTIPLY] = PS2_SCANCODE_SEQ(0x37),                                // Keypad *
    [HID_KEYCODE_KEYPAD_SUBTRACT] = PS2_SCANCODE_SEQ(0x4A),                                // Keypad -
    [HID_KEYCODE_KEYPAD_ADD]      = PS2_SCANCODE_SEQ(0x4E),                                // Keypad +
    [HID_KEYCODE_KEYPAD_ENTER]    = PS2_SCANCODE_SEQ(0xE0, 0x1C),                          // Keypad Enter
    [HID_KEYCODE_KEYPAD_1]        = PS2_SCANCODE_SEQ(0x4F),                                // Keypad 1 End
    [HID_KEYCODE_KEYPAD_2]        = PS2_SCANCODE_SEQ(0x50),                                // Keypad 2 Down
    [HID_KEYCODE_KEYPAD_3]        = PS2_SCANCODE_SEQ(0x51),                                // Keypad 3 Page Down
    [HID_KEYCODE_KEYPAD_4]        = PS2_SCANCODE_SEQ(0x4B),                                // Keypad 4 Left
    [HID_KEYCODE_KEYPAD_5]        = PS2_SCANCODE_SEQ(0x4C),                                // Keypad 5
    [HID_KEYCODE_KEYPAD_6]        = PS2_SCANCODE_SEQ(0x4D),                                // Keypad 6 Right
    [HID_KEYCODE_KEYPAD_7]        = PS2_SCANCODE_SEQ(0x47),                                // Keypad 7 Home
    [HID_KEYCODE_KEYPAD_8]        = PS2_SCANCODE_SEQ(0x48),                                // Keypad 8 Up
    [HID_KEYCODE_KEYPAD_9]        = PS2_SCANCODE_SEQ(0x49),                                // Keypad 9 Page Up
    [HID_KEYCODE_KEYPAD_0]        = PS2_SCANCODE_SEQ(0x52),                                // Keypad 0 Insert
    [HID_KEYCODE_KEYPAD_DECIMAL]  = PS2_SCANCODE_SEQ(0x53),                                // Keypad . Delete
    [HID_KEYCODE_EUROPE_2]        = PS2_SCANCODE_SEQ(0x56),                                // Europe 2
    [HID_KEYCODE_APPLICATION]     = PS2_SCANCODE_SEQ(0xE0, 0x5D),                          // App
    [HID_KEYCODE_POWER]           = PS2_SCANCODE_SEQ(0xE0, 0x5E),                          // Keyboard Power
    [HID_KEYCODE_KEYPAD_EQUAL]    = PS2_SCANCODE_SEQ(0x59),                                // Keypad =
    [HID_KEYCODE_F13]             = PS2_SCANCODE_SEQ(0x64),                                // F13
    [HID_KEYCODE_F14]             = PS2_SCANCODE_SEQ(0x65),                                // F14
    [HID_KEYCODE_F15]             = PS2_SCANCODE_SEQ(0x66),                                // F15
    [HID_KEYCODE_F16]             = PS2_SCANCODE_SEQ(0x67),                                // F16
    [HID_KEYCODE_F17]             = PS2_SCANCODE_SEQ(0x68),                                // F17
    [HID_KEYCODE_F18]             = PS2_SCANCODE_SEQ(0x69),                                // F18
    [HID_KEYCODE_F19]             = PS2_SCANCODE_SEQ(0x6A),                                // F19
    [HID_KEYCODE_F20]             = PS2_SCANCODE_SEQ(0x6B),                                // F20
    [HID_KEYCODE_F21]             = PS2_SCANCODE_SEQ(0x6C),                                // F21
    [HID_KEYCODE_F22]             = PS2_SCANCODE_SEQ(0x6D),                                // F22
    [HID_KEYCODE_F23]             = PS2_SCANCODE_SEQ(0x6E),                                // F23
    [HID_KEYCODE_F24]             = PS2_SCANCODE_SEQ(0x76),                                // F24
    [HID_KEYCODE_KEYPAD_COMMA]    = PS2_SCANCODE_SEQ(0x7E),                                // Keypad , (Brazilian Keypad .)
    [HID_KEYCODE_INTERNATIONAL_1] = PS2_SCANCODE_SEQ(0x73),                                // Int'l 1 (Ro)
    [HID_KEYCODE_INTERNATIONAL_2] = PS2_SCANCODE_SEQ(0x70),                                // Int'l 2 (Katakana/Hiragana)
    [HID_KEYCODE_INTERNATIONAL_3] = PS2_SCANCODE_SEQ(0x7D),                                // Int'l 3 (Yen)
    [HID_KEYCODE_INTERNATIONAL_4] = PS2_SCANCODE_SEQ(0x79),                                // Int'l 4 (Henkan)
    [HID_KEYCODE_INTERNATIONAL_5] = PS2_SCANCODE_SEQ(0x7B),                                // Int'l 5 (Muhenkan)
    [HID_KEYCODE_INTERNATIONAL_6] = PS2_SCANCODE_SEQ(0x5C),                                // Int'l 6 (PC9800 Keypad ,)
    [HID_KEYCODE_LANG_1]          = PS2_SCANCODE_SEQ(0xF2),                                // Lang 1 (Hangul/English)  make-only
    [HID_KEYCODE_LANG_2]          = PS2_SCANCODE_SEQ(0xF1),                                // Lang 2 (Hanja)           make-only
    [HID_KEYCODE_LANG_3]          = PS2_SCANCODE_SEQ(0x78),                                // Lang 3 (Katakana)
    [HID_KEYCODE_LANG_4]          = PS2_SCANCODE_SEQ(0x77),                                // Lang 4 (Hiragana)
    [HID_KEYCODE_LANG_5]          = PS2_SCANCODE_SEQ(0x76),                                // Lang 5 (Zenkaku/Hankaku)
    [HID_KEYCODE_LEFT_CONTROL]    = PS2_SCANCODE_SEQ(0x1D),                                // Left Control
    [HID_KEYCODE_LEFT_SHIFT]      = PS2_SCANCODE_SEQ(0x2A),                                // Left Shift
    [HID_KEYCODE_LEFT_ALT]        = PS2_SCANCODE_SEQ(0x38),                                // Left Alt
    [HID_KEYCODE_LEFT_GUI]        = PS2_SCANCODE_SEQ(0xE0, 0x5B),                          // Left GUI
    [HID_KEYCODE_RIGHT_CONTROL]   = PS2_SCANCODE_SEQ(0xE0, 0x1D),                          // Right Control
    [HID_KEYCODE_RIGHT_SHIFT]     = PS2_SCANCODE_SEQ(0x36),                                // Right Shift
    [HID_KEYCODE_RIGHT_ALT]       = PS2_SCANCODE_SEQ(0xE0, 0x38),                          // Right Alt
    [HID_KEYCODE_RIGHT_GUI]       = PS2_SCANCODE_SEQ(0xE0, 0x5C),                          // Right GUI
};

const uint8_t *const hid_to_ps2_set_1_break[256] = {
    [HID_KEYCODE_A]               = PS2_SCANCODE_SEQ(0x9E),                                // A
    [HID_KEYCODE_B]               = PS2_SCANCODE_SEQ(0xB0),                                // B
    [HID_KEYCODE_C]               = PS2_SCANCODE_SEQ(0xAE),                                // C
    [HID_KEYCODE_D]               = PS2_SCANCODE_SEQ(0xA0),                                // D
    [HID_KEYCODE_E]               = PS2_SCANCODE_SEQ(0x92),                                // E
    [HID_KEYCODE_F]               = PS2_SCANCODE_SEQ(0xA1),                                // F
    [HID_KEYCODE_G]               = PS2_SCANCODE_SEQ(0xA2),                                // G
    [HID_KEYCODE_H]               = PS2_SCANCODE_SEQ(0xA3),                                // H
    [HID_KEYCODE_I]               = PS2_SCANCODE_SEQ(0x97),                                // I
    [HID_KEYCODE_J]               = PS2_SCANCODE_SEQ(0xA4),                                // J
    [HID_KEYCODE_K]               = PS2_SCANCODE_SEQ(0xA5),                                // K
    [HID_KEYCODE_L]               = PS2_SCANCODE_SEQ(0xA6),                                // L
    [HID_KEYCODE_M]               = PS2_SCANCODE_SEQ(0xB2),                                // M
    [HID_KEYCODE_N]               = PS2_SCANCODE_SEQ(0xB1),                                // N
    [HID_KEYCODE_O]               = PS2_SCANCODE_SEQ(0x98),                                // O
    [HID_KEYCODE_P]               = PS2_SCANCODE_SEQ(0x99),                                // P
    [HID_KEYCODE_Q]               = PS2_SCANCODE_SEQ(0x90),                                // Q
    [HID_KEYCODE_R]               = PS2_SCANCODE_SEQ(0x93),                                // R
    [HID_KEYCODE_S]               = PS2_SCANCODE_SEQ(0x9F),                                // S
    [HID_KEYCODE_T]               = PS2_SCANCODE_SEQ(0x94),                                // T
    [HID_KEYCODE_U]               = PS2_SCANCODE_SEQ(0x96),                                // U
    [HID_KEYCODE_V]               = PS2_SCANCODE_SEQ(0xAF),                                // V
    [HID_KEYCODE_W]               = PS2_SCANCODE_SEQ(0x91),                                // W
    [HID_KEYCODE_X]               = PS2_SCANCODE_SEQ(0xAD),                                // X
    [HID_KEYCODE_Y]               = PS2_SCANCODE_SEQ(0x95),                                // Y
    [HID_KEYCODE_Z]               = PS2_SCANCODE_SEQ(0xAC),                                // Z
    [HID_KEYCODE_1]               = PS2_SCANCODE_SEQ(0x82),                                // 1 !
    [HID_KEYCODE_2]               = PS2_SCANCODE_SEQ(0x83),                                // 2 @
    [HID_KEYCODE_3]               = PS2_SCANCODE_SEQ(0x84),                                // 3 #
    [HID_KEYCODE_4]               = PS2_SCANCODE_SEQ(0x85),                                // 4 $
    [HID_KEYCODE_5]               = PS2_SCANCODE_SEQ(0x86),                                // 5 %
    [HID_KEYCODE_6]               = PS2_SCANCODE_SEQ(0x87),                                // 6 ^
    [HID_KEYCODE_7]               = PS2_SCANCODE_SEQ(0x88),                                // 7 &
    [HID_KEYCODE_8]               = PS2_SCANCODE_SEQ(0x89),                                // 8 *
    [HID_KEYCODE_9]               = PS2_SCANCODE_SEQ(0x8A),                                // 9 (
    [HID_KEYCODE_0]               = PS2_SCANCODE_SEQ(0x8B),                                // 0 )
    [HID_KEYCODE_ENTER]           = PS2_SCANCODE_SEQ(0x9C),                                // Return
    [HID_KEYCODE_ESCAPE]          = PS2_SCANCODE_SEQ(0x81),                                // Escape
    [HID_KEYCODE_BACKSPACE]       = PS2_SCANCODE_SEQ(0x8E),                                // Backspace
    [HID_KEYCODE_TAB]             = PS2_SCANCODE_SEQ(0x8F),                                // Tab
    [HID_KEYCODE_SPACE]           = PS2_SCANCODE_SEQ(0xB9),                                // Space
    [HID_KEYCODE_MINUS]           = PS2_SCANCODE_SEQ(0x8C),                                // - _
    [HID_KEYCODE_EQUAL]           = PS2_SCANCODE_SEQ(0x8D),                                // = +
    [HID_KEYCODE_BRACKET_LEFT]    = PS2_SCANCODE_SEQ(0x9A),                                // [ {
    [HID_KEYCODE_BRACKET_RIGHT]   = PS2_SCANCODE_SEQ(0x9B),                                // ] }
    [HID_KEYCODE_BACKSLASH]       = PS2_SCANCODE_SEQ(0xAB),                                // \ |
    [HID_KEYCODE_EUROPE_1]        = PS2_SCANCODE_SEQ(0xAB),                                // Europe 1
    [HID_KEYCODE_SEMICOLON]       = PS2_SCANCODE_SEQ(0xA7),                                // ; :
    [HID_KEYCODE_APOSTROPHE]      = PS2_SCANCODE_SEQ(0xA8),                                // ' "
    [HID_KEYCODE_GRAVE]           = PS2_SCANCODE_SEQ(0xA9),                                // ` ~
    [HID_KEYCODE_COMMA]           = PS2_SCANCODE_SEQ(0xB3),                                // , <
    [HID_KEYCODE_PERIOD]          = PS2_SCANCODE_SEQ(0xB4),                                // . >
    [HID_KEYCODE_SLASH]           = PS2_SCANCODE_SEQ(0xB5),                                // / ?
    [HID_KEYCODE_CAPS_LOCK]       = PS2_SCANCODE_SEQ(0xBA),                                // Caps Lock
    [HID_KEYCODE_F1]              = PS2_SCANCODE_SEQ(0xBB),                                // F1
    [HID_KEYCODE_F2]              = PS2_SCANCODE_SEQ(0xBC),                                // F2
    [HID_KEYCODE_F3]              = PS2_SCANCODE_SEQ(0xBD),                                // F3
    [HID_KEYCODE_F4]              = PS2_SCANCODE_SEQ(0xBE),                                // F4
    [HID_KEYCODE_F5]              = PS2_SCANCODE_SEQ(0xBF),                                // F5
    [HID_KEYCODE_F6]              = PS2_SCANCODE_SEQ(0xC0),                                // F6
    [HID_KEYCODE_F7]              = PS2_SCANCODE_SEQ(0xC1),                                // F7
    [HID_KEYCODE_F8]              = PS2_SCANCODE_SEQ(0xC2),                                // F8
    [HID_KEYCODE_F9]              = PS2_SCANCODE_SEQ(0xC3),                                // F9
    [HID_KEYCODE_F10]             = PS2_SCANCODE_SEQ(0xC4),                                // F10
    [HID_KEYCODE_F11]             = PS2_SCANCODE_SEQ(0xD7),                                // F11
    [HID_KEYCODE_F12]             = PS2_SCANCODE_SEQ(0xD8),                                // F12
    [HID_KEYCODE_PRINT_SCREEN]    = PS2_SCANCODE_SEQ(0xE0, 0xB7, 0xE0, 0xAA),              // Print Screen
    [HID_KEYCODE_SCROLL_LOCK]     = PS2_SCANCODE_SEQ(0xC6),                                // Scroll Lock
    // HID_KEYCODE_PAUSE has no break code - the make sequence is self-contained (NULL)
    [HID_KEYCODE_INSERT]          = PS2_SCANCODE_SEQ(0xE0, 0xD2),                          // Insert
    [HID_KEYCODE_HOME]            = PS2_SCANCODE_SEQ(0xE0, 0xC7),                          // Home
    [HID_KEYCODE_PAGE_UP]         = PS2_SCANCODE_SEQ(0xE0, 0xC9),                          // Page Up
    [HID_KEYCODE_DELETE]          = PS2_SCANCODE_SEQ(0xE0, 0xD3),                          // Delete
    [HID_KEYCODE_END]             = PS2_SCANCODE_SEQ(0xE0, 0xCF),                          // End
    [HID_KEYCODE_PAGE_DOWN]       = PS2_SCANCODE_SEQ(0xE0, 0xD1),                          // Page Down
    [HID_KEYCODE_ARROW_RIGHT]     = PS2_SCANCODE_SEQ(0xE0, 0xCD),                          // Right Arrow
    [HID_KEYCODE_ARROW_LEFT]      = PS2_SCANCODE_SEQ(0xE0, 0xCB),                          // Left Arrow
    [HID_KEYCODE_ARROW_DOWN]      = PS2_SCANCODE_SEQ(0xE0, 0xD0),                          // Down Arrow
    [HID_KEYCODE_ARROW_UP]        = PS2_SCANCODE_SEQ(0xE0, 0xC8),                          // Up Arrow
    [HID_KEYCODE_NUM_LOCK]        = PS2_SCANCODE_SEQ(0xC5),                                // Num Lock
    [HID_KEYCODE_KEYPAD_DIVIDE]   = PS2_SCANCODE_SEQ(0xE0, 0xB5),                          // Keypad /
    [HID_KEYCODE_KEYPAD_MULTIPLY] = PS2_SCANCODE_SEQ(0xB7),                                // Keypad *
    [HID_KEYCODE_KEYPAD_SUBTRACT] = PS2_SCANCODE_SEQ(0xCA),                                // Keypad -
    [HID_KEYCODE_KEYPAD_ADD]      = PS2_SCANCODE_SEQ(0xCE),                                // Keypad +
    [HID_KEYCODE_KEYPAD_ENTER]    = PS2_SCANCODE_SEQ(0xE0, 0x9C),                          // Keypad Enter
    [HID_KEYCODE_KEYPAD_1]        = PS2_SCANCODE_SEQ(0xCF),                                // Keypad 1 End
    [HID_KEYCODE_KEYPAD_2]        = PS2_SCANCODE_SEQ(0xD0),                                // Keypad 2 Down
    [HID_KEYCODE_KEYPAD_3]        = PS2_SCANCODE_SEQ(0xD1),                                // Keypad 3 Page Down
    [HID_KEYCODE_KEYPAD_4]        = PS2_SCANCODE_SEQ(0xCB),                                // Keypad 4 Left
    [HID_KEYCODE_KEYPAD_5]        = PS2_SCANCODE_SEQ(0xCC),                                // Keypad 5
    [HID_KEYCODE_KEYPAD_6]        = PS2_SCANCODE_SEQ(0xCD),                                // Keypad 6 Right
    [HID_KEYCODE_KEYPAD_7]        = PS2_SCANCODE_SEQ(0xC7),                                // Keypad 7 Home
    [HID_KEYCODE_KEYPAD_8]        = PS2_SCANCODE_SEQ(0xC8),                                // Keypad 8 Up
    [HID_KEYCODE_KEYPAD_9]        = PS2_SCANCODE_SEQ(0xC9),                                // Keypad 9 Page Up
    [HID_KEYCODE_KEYPAD_0]        = PS2_SCANCODE_SEQ(0xD2),                                // Keypad 0 Insert
    [HID_KEYCODE_KEYPAD_DECIMAL]  = PS2_SCANCODE_SEQ(0xD3),                                // Keypad . Delete
    [HID_KEYCODE_EUROPE_2]        = PS2_SCANCODE_SEQ(0xD6),                                // Europe 2
    [HID_KEYCODE_APPLICATION]     = PS2_SCANCODE_SEQ(0xE0, 0xDD),                          // App
    [HID_KEYCODE_POWER]           = PS2_SCANCODE_SEQ(0xE0, 0xDE),                          // Keyboard Power
    [HID_KEYCODE_KEYPAD_EQUAL]    = PS2_SCANCODE_SEQ(0xD9),                                // Keypad =
    [HID_KEYCODE_F13]             = PS2_SCANCODE_SEQ(0xE4),                                // F13
    [HID_KEYCODE_F14]             = PS2_SCANCODE_SEQ(0xE5),                                // F14
    [HID_KEYCODE_F15]             = PS2_SCANCODE_SEQ(0xE6),                                // F15
    [HID_KEYCODE_F16]             = PS2_SCANCODE_SEQ(0xE7),                                // F16
    [HID_KEYCODE_F17]             = PS2_SCANCODE_SEQ(0xE8),                                // F17
    [HID_KEYCODE_F18]             = PS2_SCANCODE_SEQ(0xE9),                                // F18
    [HID_KEYCODE_F19]             = PS2_SCANCODE_SEQ(0xEA),                                // F19
    [HID_KEYCODE_F20]             = PS2_SCANCODE_SEQ(0xEB),                                // F20
    [HID_KEYCODE_F21]             = PS2_SCANCODE_SEQ(0xEC),                                // F21
    [HID_KEYCODE_F22]             = PS2_SCANCODE_SEQ(0xED),                                // F22
    [HID_KEYCODE_F23]             = PS2_SCANCODE_SEQ(0xEE),                                // F23
    [HID_KEYCODE_F24]             = PS2_SCANCODE_SEQ(0xF6),                                // F24
    [HID_KEYCODE_KEYPAD_COMMA]    = PS2_SCANCODE_SEQ(0xFE),                                // Keypad , (Brazilian Keypad .)
    [HID_KEYCODE_INTERNATIONAL_1] = PS2_SCANCODE_SEQ(0xF3),                                // Int'l 1 (Ro)
    [HID_KEYCODE_INTERNATIONAL_2] = PS2_SCANCODE_SEQ(0xF0),                                // Int'l 2 (Katakana/Hiragana)
    [HID_KEYCODE_INTERNATIONAL_3] = PS2_SCANCODE_SEQ(0xFD),                                // Int'l 3 (Yen)
    [HID_KEYCODE_INTERNATIONAL_4] = PS2_SCANCODE_SEQ(0xF9),                                // Int'l 4 (Henkan)
    [HID_KEYCODE_INTERNATIONAL_5] = PS2_SCANCODE_SEQ(0xFB),                                // Int'l 5 (Muhenkan)
    [HID_KEYCODE_INTERNATIONAL_6] = PS2_SCANCODE_SEQ(0xDC),                                // Int'l 6 (PC9800 Keypad ,)
    // HID_KEYCODE_LANG_1 make-only (NULL break)
    // HID_KEYCODE_LANG_2 make-only (NULL break)
    [HID_KEYCODE_LANG_3]          = PS2_SCANCODE_SEQ(0xF8),                                // Lang 3 (Katakana)
    [HID_KEYCODE_LANG_4]          = PS2_SCANCODE_SEQ(0xF7),                                // Lang 4 (Hiragana)
    [HID_KEYCODE_LANG_5]          = PS2_SCANCODE_SEQ(0xF6),                                // Lang 5 (Zenkaku/Hankaku)
    [HID_KEYCODE_LEFT_CONTROL]    = PS2_SCANCODE_SEQ(0x9D),                                // Left Control
    [HID_KEYCODE_LEFT_SHIFT]      = PS2_SCANCODE_SEQ(0xAA),                                // Left Shift
    [HID_KEYCODE_LEFT_ALT]        = PS2_SCANCODE_SEQ(0xB8),                                // Left Alt
    [HID_KEYCODE_LEFT_GUI]        = PS2_SCANCODE_SEQ(0xE0, 0xDB),                          // Left GUI
    [HID_KEYCODE_RIGHT_CONTROL]   = PS2_SCANCODE_SEQ(0xE0, 0x9D),                          // Right Control
    [HID_KEYCODE_RIGHT_SHIFT]     = PS2_SCANCODE_SEQ(0xB6),                                // Right Shift
    [HID_KEYCODE_RIGHT_ALT]       = PS2_SCANCODE_SEQ(0xE0, 0xB8),                          // Right Alt
    [HID_KEYCODE_RIGHT_GUI]       = PS2_SCANCODE_SEQ(0xE0, 0xDC),                          // Right GUI
};

/*
 * Ctrl + Pause emits "Break" instead of "Pause". The HID Pause key (0x48)
 * reports the same usage whether or not Ctrl is held, so the send layer picks
 * between hid_to_ps2_set_1_make[HID_KEYCODE_PAUSE] (plain Pause) and this blob
 * when Ctrl is down. Like Pause, Break is a one-shot: the whole make+break burst
 * is emitted on the press and nothing on release, so it never repeats. This
 * matches real IBM/AT hardware and ps2x2pico. Length-prefixed, table format.
 */
const uint8_t *const ps2_ctrl_break_set_1 = PS2_SCANCODE_SEQ(0xE0, 0x46, 0xE0, 0xC6);
