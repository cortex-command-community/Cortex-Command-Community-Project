/**
 * allegro/keyboard.h — Emscripten stub
 * UInputMan includes this for keyboard constant definitions.
 * On the web build, SDL3 provides key constants; we define Allegro compat macros here.
 */
#pragma once

// Allegro key code constants — mapped to matching SDL3 scancodes where possible.
// UInputMan only uses these as integer constants for comparison.
#define KEY_A           1
#define KEY_B           2
#define KEY_C           3
#define KEY_D           4
#define KEY_E           5
#define KEY_F           6
#define KEY_G           7
#define KEY_H           8
#define KEY_I           9
#define KEY_J           10
#define KEY_K           11
#define KEY_L           12
#define KEY_M           13
#define KEY_N           14
#define KEY_O           15
#define KEY_P           16
#define KEY_Q           17
#define KEY_R           18
#define KEY_S           19
#define KEY_T           20
#define KEY_U           21
#define KEY_V           22
#define KEY_W           23
#define KEY_X           24
#define KEY_Y           25
#define KEY_Z           26
#define KEY_0           27
#define KEY_1           28
#define KEY_2           29
#define KEY_3           30
#define KEY_4           31
#define KEY_5           32
#define KEY_6           33
#define KEY_7           34
#define KEY_8           35
#define KEY_9           36
#define KEY_F1          59
#define KEY_F2          60
#define KEY_F3          61
#define KEY_F4          62
#define KEY_F5          63
#define KEY_F6          64
#define KEY_F7          65
#define KEY_F8          66
#define KEY_F9          67
#define KEY_F10         68
#define KEY_F11         87
#define KEY_F12         88
#define KEY_ESCAPE      1
#define KEY_TILDE       41
#define KEY_MINUS       12
#define KEY_EQUALS      13
#define KEY_BACKSPACE   14
#define KEY_TAB         15
#define KEY_OPENBRACE   26
#define KEY_CLOSEBRACE  27
#define KEY_ENTER       28
#define KEY_COLON       39
#define KEY_QUOTE       40
#define KEY_BACKSLASH   43
#define KEY_COMMA       51
#define KEY_STOP        52
#define KEY_SLASH       53
#define KEY_SPACE       57
#define KEY_INSERT      82
#define KEY_DEL         83
#define KEY_HOME        71
#define KEY_END         79
#define KEY_PGUP        73
#define KEY_PGDN        81
#define KEY_LEFT        75
#define KEY_RIGHT       77
#define KEY_UP          72
#define KEY_DOWN        80
#define KEY_LSHIFT      42
#define KEY_RSHIFT      54
#define KEY_LCONTROL    29
#define KEY_RCONTROL    97
#define KEY_ALT         56
#define KEY_ALTGR       100
#define KEY_LWIN        91
#define KEY_RWIN        92
#define KEY_MENU        93
#define KEY_NUMLOCK     69
#define KEY_CAPSLOCK    58
#define KEY_PAD_0       82
#define KEY_PAD_1       79
#define KEY_PAD_2       80
#define KEY_PAD_3       81
#define KEY_PAD_4       75
#define KEY_PAD_5       76
#define KEY_PAD_6       77
#define KEY_PAD_7       71
#define KEY_PAD_8       72
#define KEY_PAD_9       73
#define KEY_PAD_SLASH   98
#define KEY_PAD_ASTERISK 55
#define KEY_PAD_MINUS   74
#define KEY_PAD_PLUS    78
#define KEY_PAD_DELETE  83
#define KEY_PAD_ENTER   96
#define KEY_MAX         128

// Modifier flags
#define KB_SHIFT_FLAG   0x0001
#define KB_CTRL_FLAG    0x0002
#define KB_ALT_FLAG     0x0004
#define KB_CAPSLOCK_FLAG 0x0040
#define KB_NUMLOCK_FLAG  0x0020

extern volatile int key[KEY_MAX];
extern volatile int key_shifts;
