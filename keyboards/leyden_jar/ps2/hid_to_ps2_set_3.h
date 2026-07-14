#pragma once

#include "hid_to_ps2.h"   // Provides PS2_SCANCODE_SEQ()
#include "hid_usage_keyboard.h"   // Provides HID_KEYCODE_* names

/*
 * Scan code set 3, encoded the same way as set 1 (see hid_to_ps2_set_1.h):
 * a 256-entry table of pointers to length-prefixed PS2_SCANCODE_SEQ() blobs,
 * indexed by USB HID usage ID. Set 3 has no multi-byte keys, so every entry
 * is short, but the encoding is kept uniform with sets 1 and 2 so the send
 * path stays identical. Unassigned keys are NULL.
 *
 * Coverage: Set 3 predates the PC-era extended keys, so F13-F24, Keypad =,
 * Keyboard Power, the International (Int'l 1-6) and Language (Lang 1-5) keys
 * have no documented Set 3 code in any known source (Chapweske, Brouwer, the
 * IBM 122-key terminal layout, Microsoft's HID->PS/2 table). Real Set 3
 * keyboards typically emit nothing for these, so those entries are left NULL
 * intentionally - not forgotten. Application/Menu (0x65 = 8D) is the one such
 * extended key that IS documented (Chapweske + Brouwer), so it is included.
 */

const uint8_t *const hid_to_ps2_set_3_make[256] = {
    [HID_KEYCODE_A]               = PS2_SCANCODE_SEQ(0x1C),  // A
    [HID_KEYCODE_B]               = PS2_SCANCODE_SEQ(0x32),  // B
    [HID_KEYCODE_C]               = PS2_SCANCODE_SEQ(0x21),  // C
    [HID_KEYCODE_D]               = PS2_SCANCODE_SEQ(0x23),  // D
    [HID_KEYCODE_E]               = PS2_SCANCODE_SEQ(0x24),  // E
    [HID_KEYCODE_F]               = PS2_SCANCODE_SEQ(0x2B),  // F
    [HID_KEYCODE_G]               = PS2_SCANCODE_SEQ(0x34),  // G
    [HID_KEYCODE_H]               = PS2_SCANCODE_SEQ(0x33),  // H
    [HID_KEYCODE_I]               = PS2_SCANCODE_SEQ(0x43),  // I
    [HID_KEYCODE_J]               = PS2_SCANCODE_SEQ(0x3B),  // J
    [HID_KEYCODE_K]               = PS2_SCANCODE_SEQ(0x42),  // K
    [HID_KEYCODE_L]               = PS2_SCANCODE_SEQ(0x4B),  // L
    [HID_KEYCODE_M]               = PS2_SCANCODE_SEQ(0x3A),  // M
    [HID_KEYCODE_N]               = PS2_SCANCODE_SEQ(0x31),  // N
    [HID_KEYCODE_O]               = PS2_SCANCODE_SEQ(0x44),  // O
    [HID_KEYCODE_P]               = PS2_SCANCODE_SEQ(0x4D),  // P
    [HID_KEYCODE_Q]               = PS2_SCANCODE_SEQ(0x15),  // Q
    [HID_KEYCODE_R]               = PS2_SCANCODE_SEQ(0x2D),  // R
    [HID_KEYCODE_S]               = PS2_SCANCODE_SEQ(0x1B),  // S
    [HID_KEYCODE_T]               = PS2_SCANCODE_SEQ(0x2C),  // T
    [HID_KEYCODE_U]               = PS2_SCANCODE_SEQ(0x3C),  // U
    [HID_KEYCODE_V]               = PS2_SCANCODE_SEQ(0x2A),  // V
    [HID_KEYCODE_W]               = PS2_SCANCODE_SEQ(0x1D),  // W
    [HID_KEYCODE_X]               = PS2_SCANCODE_SEQ(0x22),  // X
    [HID_KEYCODE_Y]               = PS2_SCANCODE_SEQ(0x35),  // Y
    [HID_KEYCODE_Z]               = PS2_SCANCODE_SEQ(0x1A),  // Z
    [HID_KEYCODE_1]               = PS2_SCANCODE_SEQ(0x16),  // 1 !
    [HID_KEYCODE_2]               = PS2_SCANCODE_SEQ(0x1E),  // 2 @
    [HID_KEYCODE_3]               = PS2_SCANCODE_SEQ(0x26),  // 3 #
    [HID_KEYCODE_4]               = PS2_SCANCODE_SEQ(0x25),  // 4 $
    [HID_KEYCODE_5]               = PS2_SCANCODE_SEQ(0x2E),  // 5 %
    [HID_KEYCODE_6]               = PS2_SCANCODE_SEQ(0x36),  // 6 ^
    [HID_KEYCODE_7]               = PS2_SCANCODE_SEQ(0x3D),  // 7 &
    [HID_KEYCODE_8]               = PS2_SCANCODE_SEQ(0x3E),  // 8 *
    [HID_KEYCODE_9]               = PS2_SCANCODE_SEQ(0x46),  // 9 (
    [HID_KEYCODE_0]               = PS2_SCANCODE_SEQ(0x45),  // 0 )
    [HID_KEYCODE_ENTER]           = PS2_SCANCODE_SEQ(0x5A),  // Return
    [HID_KEYCODE_ESCAPE]          = PS2_SCANCODE_SEQ(0x08),  // Escape
    [HID_KEYCODE_BACKSPACE]       = PS2_SCANCODE_SEQ(0x66),  // Backspace
    [HID_KEYCODE_TAB]             = PS2_SCANCODE_SEQ(0x0D),  // Tab
    [HID_KEYCODE_SPACE]           = PS2_SCANCODE_SEQ(0x29),  // Space
    [HID_KEYCODE_MINUS]           = PS2_SCANCODE_SEQ(0x4E),  // - _
    [HID_KEYCODE_EQUAL]           = PS2_SCANCODE_SEQ(0x55),  // = +
    [HID_KEYCODE_BRACKET_LEFT]    = PS2_SCANCODE_SEQ(0x54),  // [ {
    [HID_KEYCODE_BRACKET_RIGHT]   = PS2_SCANCODE_SEQ(0x5B),  // ] }
    [HID_KEYCODE_BACKSLASH]       = PS2_SCANCODE_SEQ(0x5C),  // \ |
    [HID_KEYCODE_EUROPE_1]        = PS2_SCANCODE_SEQ(0x53),  // Europe 1
    [HID_KEYCODE_SEMICOLON]       = PS2_SCANCODE_SEQ(0x4C),  // ; :
    [HID_KEYCODE_APOSTROPHE]      = PS2_SCANCODE_SEQ(0x52),  // ' "
    [HID_KEYCODE_GRAVE]           = PS2_SCANCODE_SEQ(0x0E),  // ` ~
    [HID_KEYCODE_COMMA]           = PS2_SCANCODE_SEQ(0x41),  // , <
    [HID_KEYCODE_PERIOD]          = PS2_SCANCODE_SEQ(0x49),  // . >
    [HID_KEYCODE_SLASH]           = PS2_SCANCODE_SEQ(0x4A),  // / ?
    [HID_KEYCODE_CAPS_LOCK]       = PS2_SCANCODE_SEQ(0x14),  // Caps Lock
    [HID_KEYCODE_F1]              = PS2_SCANCODE_SEQ(0x07),  // F1
    [HID_KEYCODE_F2]              = PS2_SCANCODE_SEQ(0x0F),  // F2
    [HID_KEYCODE_F3]              = PS2_SCANCODE_SEQ(0x17),  // F3
    [HID_KEYCODE_F4]              = PS2_SCANCODE_SEQ(0x1F),  // F4
    [HID_KEYCODE_F5]              = PS2_SCANCODE_SEQ(0x27),  // F5
    [HID_KEYCODE_F6]              = PS2_SCANCODE_SEQ(0x2F),  // F6
    [HID_KEYCODE_F7]              = PS2_SCANCODE_SEQ(0x37),  // F7
    [HID_KEYCODE_F8]              = PS2_SCANCODE_SEQ(0x3F),  // F8
    [HID_KEYCODE_F9]              = PS2_SCANCODE_SEQ(0x47),  // F9
    [HID_KEYCODE_F10]             = PS2_SCANCODE_SEQ(0x4F),  // F10
    [HID_KEYCODE_F11]             = PS2_SCANCODE_SEQ(0x56),  // F11
    [HID_KEYCODE_F12]             = PS2_SCANCODE_SEQ(0x5E),  // F12
    [HID_KEYCODE_PRINT_SCREEN]    = PS2_SCANCODE_SEQ(0x57),  // Print Screen
    [HID_KEYCODE_SCROLL_LOCK]     = PS2_SCANCODE_SEQ(0x5F),  // Scroll Lock
    [HID_KEYCODE_PAUSE]           = PS2_SCANCODE_SEQ(0x62),  // Pause
    [HID_KEYCODE_INSERT]          = PS2_SCANCODE_SEQ(0x67),  // Insert
    [HID_KEYCODE_HOME]            = PS2_SCANCODE_SEQ(0x6E),  // Home
    [HID_KEYCODE_PAGE_UP]         = PS2_SCANCODE_SEQ(0x6F),  // Page Up
    [HID_KEYCODE_DELETE]          = PS2_SCANCODE_SEQ(0x64),  // Delete
    [HID_KEYCODE_END]             = PS2_SCANCODE_SEQ(0x65),  // End
    [HID_KEYCODE_PAGE_DOWN]       = PS2_SCANCODE_SEQ(0x6D),  // Page Down
    [HID_KEYCODE_ARROW_RIGHT]     = PS2_SCANCODE_SEQ(0x6A),  // Right Arrow
    [HID_KEYCODE_ARROW_LEFT]      = PS2_SCANCODE_SEQ(0x61),  // Left Arrow
    [HID_KEYCODE_ARROW_DOWN]      = PS2_SCANCODE_SEQ(0x60),  // Down Arrow
    [HID_KEYCODE_ARROW_UP]        = PS2_SCANCODE_SEQ(0x63),  // Up Arrow
    [HID_KEYCODE_NUM_LOCK]        = PS2_SCANCODE_SEQ(0x76),  // Num Lock
    [HID_KEYCODE_KEYPAD_DIVIDE]   = PS2_SCANCODE_SEQ(0x77),  // Keypad /
    [HID_KEYCODE_KEYPAD_MULTIPLY] = PS2_SCANCODE_SEQ(0x7E),  // Keypad *
    [HID_KEYCODE_KEYPAD_SUBTRACT] = PS2_SCANCODE_SEQ(0x84),  // Keypad -
    [HID_KEYCODE_KEYPAD_ADD]      = PS2_SCANCODE_SEQ(0x7C),  // Keypad +
    [HID_KEYCODE_KEYPAD_ENTER]    = PS2_SCANCODE_SEQ(0x79),  // Keypad Enter
    [HID_KEYCODE_KEYPAD_1]        = PS2_SCANCODE_SEQ(0x69),  // Keypad 1 End
    [HID_KEYCODE_KEYPAD_2]        = PS2_SCANCODE_SEQ(0x72),  // Keypad 2 Down
    [HID_KEYCODE_KEYPAD_3]        = PS2_SCANCODE_SEQ(0x7A),  // Keypad 3 Page Down
    [HID_KEYCODE_KEYPAD_4]        = PS2_SCANCODE_SEQ(0x6B),  // Keypad 4 Left
    [HID_KEYCODE_KEYPAD_5]        = PS2_SCANCODE_SEQ(0x73),  // Keypad 5
    [HID_KEYCODE_KEYPAD_6]        = PS2_SCANCODE_SEQ(0x74),  // Keypad 6 Right
    [HID_KEYCODE_KEYPAD_7]        = PS2_SCANCODE_SEQ(0x6C),  // Keypad 7 Home
    [HID_KEYCODE_KEYPAD_8]        = PS2_SCANCODE_SEQ(0x75),  // Keypad 8 Up
    [HID_KEYCODE_KEYPAD_9]        = PS2_SCANCODE_SEQ(0x7D),  // Keypad 9 Page Up
    [HID_KEYCODE_KEYPAD_0]        = PS2_SCANCODE_SEQ(0x70),  // Keypad 0 Insert
    [HID_KEYCODE_KEYPAD_DECIMAL]  = PS2_SCANCODE_SEQ(0x71),  // Keypad . Delete
    [HID_KEYCODE_EUROPE_2]        = PS2_SCANCODE_SEQ(0x13),  // Europe 2
    [HID_KEYCODE_APPLICATION]     = PS2_SCANCODE_SEQ(0x8D),  // Application (Menu)
    [HID_KEYCODE_LEFT_CONTROL]    = PS2_SCANCODE_SEQ(0x11),  // Left Control
    [HID_KEYCODE_LEFT_SHIFT]      = PS2_SCANCODE_SEQ(0x12),  // Left Shift
    [HID_KEYCODE_LEFT_ALT]        = PS2_SCANCODE_SEQ(0x19),  // Left Alt
    [HID_KEYCODE_LEFT_GUI]        = PS2_SCANCODE_SEQ(0x8B),  // Left GUI
    [HID_KEYCODE_RIGHT_CONTROL]   = PS2_SCANCODE_SEQ(0x58),  // Right Control
    [HID_KEYCODE_RIGHT_SHIFT]     = PS2_SCANCODE_SEQ(0x59),  // Right Shift
    [HID_KEYCODE_RIGHT_ALT]       = PS2_SCANCODE_SEQ(0x39),  // Right Alt
    [HID_KEYCODE_RIGHT_GUI]       = PS2_SCANCODE_SEQ(0x8C),  // Right GUI
};

const uint8_t *const hid_to_ps2_set_3_break[256] = {
    [HID_KEYCODE_A]               = PS2_SCANCODE_SEQ(0xF0, 0x1C),  // A
    [HID_KEYCODE_B]               = PS2_SCANCODE_SEQ(0xF0, 0x32),  // B
    [HID_KEYCODE_C]               = PS2_SCANCODE_SEQ(0xF0, 0x21),  // C
    [HID_KEYCODE_D]               = PS2_SCANCODE_SEQ(0xF0, 0x23),  // D
    [HID_KEYCODE_E]               = PS2_SCANCODE_SEQ(0xF0, 0x24),  // E
    [HID_KEYCODE_F]               = PS2_SCANCODE_SEQ(0xF0, 0x2B),  // F
    [HID_KEYCODE_G]               = PS2_SCANCODE_SEQ(0xF0, 0x34),  // G
    [HID_KEYCODE_H]               = PS2_SCANCODE_SEQ(0xF0, 0x33),  // H
    [HID_KEYCODE_I]               = PS2_SCANCODE_SEQ(0xF0, 0x43),  // I
    [HID_KEYCODE_J]               = PS2_SCANCODE_SEQ(0xF0, 0x3B),  // J
    [HID_KEYCODE_K]               = PS2_SCANCODE_SEQ(0xF0, 0x42),  // K
    [HID_KEYCODE_L]               = PS2_SCANCODE_SEQ(0xF0, 0x4B),  // L
    [HID_KEYCODE_M]               = PS2_SCANCODE_SEQ(0xF0, 0x3A),  // M
    [HID_KEYCODE_N]               = PS2_SCANCODE_SEQ(0xF0, 0x31),  // N
    [HID_KEYCODE_O]               = PS2_SCANCODE_SEQ(0xF0, 0x44),  // O
    [HID_KEYCODE_P]               = PS2_SCANCODE_SEQ(0xF0, 0x4D),  // P
    [HID_KEYCODE_Q]               = PS2_SCANCODE_SEQ(0xF0, 0x15),  // Q
    [HID_KEYCODE_R]               = PS2_SCANCODE_SEQ(0xF0, 0x2D),  // R
    [HID_KEYCODE_S]               = PS2_SCANCODE_SEQ(0xF0, 0x1B),  // S
    [HID_KEYCODE_T]               = PS2_SCANCODE_SEQ(0xF0, 0x2C),  // T
    [HID_KEYCODE_U]               = PS2_SCANCODE_SEQ(0xF0, 0x3C),  // U
    [HID_KEYCODE_V]               = PS2_SCANCODE_SEQ(0xF0, 0x2A),  // V
    [HID_KEYCODE_W]               = PS2_SCANCODE_SEQ(0xF0, 0x1D),  // W
    [HID_KEYCODE_X]               = PS2_SCANCODE_SEQ(0xF0, 0x22),  // X
    [HID_KEYCODE_Y]               = PS2_SCANCODE_SEQ(0xF0, 0x35),  // Y
    [HID_KEYCODE_Z]               = PS2_SCANCODE_SEQ(0xF0, 0x1A),  // Z
    [HID_KEYCODE_1]               = PS2_SCANCODE_SEQ(0xF0, 0x16),  // 1 !
    [HID_KEYCODE_2]               = PS2_SCANCODE_SEQ(0xF0, 0x1E),  // 2 @
    [HID_KEYCODE_3]               = PS2_SCANCODE_SEQ(0xF0, 0x26),  // 3 #
    [HID_KEYCODE_4]               = PS2_SCANCODE_SEQ(0xF0, 0x25),  // 4 $
    [HID_KEYCODE_5]               = PS2_SCANCODE_SEQ(0xF0, 0x2E),  // 5 %
    [HID_KEYCODE_6]               = PS2_SCANCODE_SEQ(0xF0, 0x36),  // 6 ^
    [HID_KEYCODE_7]               = PS2_SCANCODE_SEQ(0xF0, 0x3D),  // 7 &
    [HID_KEYCODE_8]               = PS2_SCANCODE_SEQ(0xF0, 0x3E),  // 8 *
    [HID_KEYCODE_9]               = PS2_SCANCODE_SEQ(0xF0, 0x46),  // 9 (
    [HID_KEYCODE_0]               = PS2_SCANCODE_SEQ(0xF0, 0x45),  // 0 )
    [HID_KEYCODE_ENTER]           = PS2_SCANCODE_SEQ(0xF0, 0x5A),  // Return
    [HID_KEYCODE_ESCAPE]          = PS2_SCANCODE_SEQ(0xF0, 0x08),  // Escape
    [HID_KEYCODE_BACKSPACE]       = PS2_SCANCODE_SEQ(0xF0, 0x66),  // Backspace
    [HID_KEYCODE_TAB]             = PS2_SCANCODE_SEQ(0xF0, 0x0D),  // Tab
    [HID_KEYCODE_SPACE]           = PS2_SCANCODE_SEQ(0xF0, 0x29),  // Space
    [HID_KEYCODE_MINUS]           = PS2_SCANCODE_SEQ(0xF0, 0x4E),  // - _
    [HID_KEYCODE_EQUAL]           = PS2_SCANCODE_SEQ(0xF0, 0x55),  // = +
    [HID_KEYCODE_BRACKET_LEFT]    = PS2_SCANCODE_SEQ(0xF0, 0x54),  // [ {
    [HID_KEYCODE_BRACKET_RIGHT]   = PS2_SCANCODE_SEQ(0xF0, 0x5B),  // ] }
    [HID_KEYCODE_BACKSLASH]       = PS2_SCANCODE_SEQ(0xF0, 0x5C),  // \ |
    [HID_KEYCODE_EUROPE_1]        = PS2_SCANCODE_SEQ(0xF0, 0x53),  // Europe 1
    [HID_KEYCODE_SEMICOLON]       = PS2_SCANCODE_SEQ(0xF0, 0x4C),  // ; :
    [HID_KEYCODE_APOSTROPHE]      = PS2_SCANCODE_SEQ(0xF0, 0x52),  // ' "
    [HID_KEYCODE_GRAVE]           = PS2_SCANCODE_SEQ(0xF0, 0x0E),  // ` ~
    [HID_KEYCODE_COMMA]           = PS2_SCANCODE_SEQ(0xF0, 0x41),  // , <
    [HID_KEYCODE_PERIOD]          = PS2_SCANCODE_SEQ(0xF0, 0x49),  // . >
    [HID_KEYCODE_SLASH]           = PS2_SCANCODE_SEQ(0xF0, 0x4A),  // / ?
    [HID_KEYCODE_CAPS_LOCK]       = PS2_SCANCODE_SEQ(0xF0, 0x14),  // Caps Lock
    [HID_KEYCODE_F1]              = PS2_SCANCODE_SEQ(0xF0, 0x07),  // F1
    [HID_KEYCODE_F2]              = PS2_SCANCODE_SEQ(0xF0, 0x0F),  // F2
    [HID_KEYCODE_F3]              = PS2_SCANCODE_SEQ(0xF0, 0x17),  // F3
    [HID_KEYCODE_F4]              = PS2_SCANCODE_SEQ(0xF0, 0x1F),  // F4
    [HID_KEYCODE_F5]              = PS2_SCANCODE_SEQ(0xF0, 0x27),  // F5
    [HID_KEYCODE_F6]              = PS2_SCANCODE_SEQ(0xF0, 0x2F),  // F6
    [HID_KEYCODE_F7]              = PS2_SCANCODE_SEQ(0xF0, 0x37),  // F7
    [HID_KEYCODE_F8]              = PS2_SCANCODE_SEQ(0xF0, 0x3F),  // F8
    [HID_KEYCODE_F9]              = PS2_SCANCODE_SEQ(0xF0, 0x47),  // F9
    [HID_KEYCODE_F10]             = PS2_SCANCODE_SEQ(0xF0, 0x4F),  // F10
    [HID_KEYCODE_F11]             = PS2_SCANCODE_SEQ(0xF0, 0x56),  // F11
    [HID_KEYCODE_F12]             = PS2_SCANCODE_SEQ(0xF0, 0x5E),  // F12
    [HID_KEYCODE_PRINT_SCREEN]    = PS2_SCANCODE_SEQ(0xF0, 0x57),  // Print Screen
    [HID_KEYCODE_SCROLL_LOCK]     = PS2_SCANCODE_SEQ(0xF0, 0x5F),  // Scroll Lock
    [HID_KEYCODE_PAUSE]           = PS2_SCANCODE_SEQ(0xF0, 0x62),  // Pause
    [HID_KEYCODE_INSERT]          = PS2_SCANCODE_SEQ(0xF0, 0x67),  // Insert
    [HID_KEYCODE_HOME]            = PS2_SCANCODE_SEQ(0xF0, 0x6E),  // Home
    [HID_KEYCODE_PAGE_UP]         = PS2_SCANCODE_SEQ(0xF0, 0x6F),  // Page Up
    [HID_KEYCODE_DELETE]          = PS2_SCANCODE_SEQ(0xF0, 0x64),  // Delete
    [HID_KEYCODE_END]             = PS2_SCANCODE_SEQ(0xF0, 0x65),  // End
    [HID_KEYCODE_PAGE_DOWN]       = PS2_SCANCODE_SEQ(0xF0, 0x6D),  // Page Down
    [HID_KEYCODE_ARROW_RIGHT]     = PS2_SCANCODE_SEQ(0xF0, 0x6A),  // Right Arrow
    [HID_KEYCODE_ARROW_LEFT]      = PS2_SCANCODE_SEQ(0xF0, 0x61),  // Left Arrow
    [HID_KEYCODE_ARROW_DOWN]      = PS2_SCANCODE_SEQ(0xF0, 0x60),  // Down Arrow
    [HID_KEYCODE_ARROW_UP]        = PS2_SCANCODE_SEQ(0xF0, 0x63),  // Up Arrow
    [HID_KEYCODE_NUM_LOCK]        = PS2_SCANCODE_SEQ(0xF0, 0x76),  // Num Lock
    [HID_KEYCODE_KEYPAD_DIVIDE]   = PS2_SCANCODE_SEQ(0xF0, 0x77),  // Keypad /
    [HID_KEYCODE_KEYPAD_MULTIPLY] = PS2_SCANCODE_SEQ(0xF0, 0x7E),  // Keypad *
    [HID_KEYCODE_KEYPAD_SUBTRACT] = PS2_SCANCODE_SEQ(0xF0, 0x84),  // Keypad -
    [HID_KEYCODE_KEYPAD_ADD]      = PS2_SCANCODE_SEQ(0xF0, 0x7C),  // Keypad +
    [HID_KEYCODE_KEYPAD_ENTER]    = PS2_SCANCODE_SEQ(0xF0, 0x79),  // Keypad Enter
    [HID_KEYCODE_KEYPAD_1]        = PS2_SCANCODE_SEQ(0xF0, 0x69),  // Keypad 1 End
    [HID_KEYCODE_KEYPAD_2]        = PS2_SCANCODE_SEQ(0xF0, 0x72),  // Keypad 2 Down
    [HID_KEYCODE_KEYPAD_3]        = PS2_SCANCODE_SEQ(0xF0, 0x7A),  // Keypad 3 Page Down
    [HID_KEYCODE_KEYPAD_4]        = PS2_SCANCODE_SEQ(0xF0, 0x6B),  // Keypad 4 Left
    [HID_KEYCODE_KEYPAD_5]        = PS2_SCANCODE_SEQ(0xF0, 0x73),  // Keypad 5
    [HID_KEYCODE_KEYPAD_6]        = PS2_SCANCODE_SEQ(0xF0, 0x74),  // Keypad 6 Right
    [HID_KEYCODE_KEYPAD_7]        = PS2_SCANCODE_SEQ(0xF0, 0x6C),  // Keypad 7 Home
    [HID_KEYCODE_KEYPAD_8]        = PS2_SCANCODE_SEQ(0xF0, 0x75),  // Keypad 8 Up
    [HID_KEYCODE_KEYPAD_9]        = PS2_SCANCODE_SEQ(0xF0, 0x7D),  // Keypad 9 Page Up
    [HID_KEYCODE_KEYPAD_0]        = PS2_SCANCODE_SEQ(0xF0, 0x70),  // Keypad 0 Insert
    [HID_KEYCODE_KEYPAD_DECIMAL]  = PS2_SCANCODE_SEQ(0xF0, 0x71),  // Keypad . Delete
    [HID_KEYCODE_EUROPE_2]        = PS2_SCANCODE_SEQ(0xF0, 0x13),  // Europe 2
    [HID_KEYCODE_APPLICATION]     = PS2_SCANCODE_SEQ(0xF0, 0x8D),  // Application (Menu)
    [HID_KEYCODE_LEFT_CONTROL]    = PS2_SCANCODE_SEQ(0xF0, 0x11),  // Left Control
    [HID_KEYCODE_LEFT_SHIFT]      = PS2_SCANCODE_SEQ(0xF0, 0x12),  // Left Shift
    [HID_KEYCODE_LEFT_ALT]        = PS2_SCANCODE_SEQ(0xF0, 0x19),  // Left Alt
    [HID_KEYCODE_LEFT_GUI]        = PS2_SCANCODE_SEQ(0xF0, 0x8B),  // Left GUI
    [HID_KEYCODE_RIGHT_CONTROL]   = PS2_SCANCODE_SEQ(0xF0, 0x58),  // Right Control
    [HID_KEYCODE_RIGHT_SHIFT]     = PS2_SCANCODE_SEQ(0xF0, 0x59),  // Right Shift
    [HID_KEYCODE_RIGHT_ALT]       = PS2_SCANCODE_SEQ(0xF0, 0x39),  // Right Alt
    [HID_KEYCODE_RIGHT_GUI]       = PS2_SCANCODE_SEQ(0xF0, 0x8C),  // Right GUI
};
