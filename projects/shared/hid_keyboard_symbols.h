
#ifndef HID_KEYBOARD_SYMBOLS_H
# define HID_KEYBOARD_SYMBOLS_H

# define LED_NUM_LOCK		1
# define LED_CAPS_LOCK		2
# define LED_SCROLL_LOCK	3
# define LED_COMPOSE		4
# define LED_KANA		5

/* for more definitions see HID usage table
   chapter 10, keyboard page */
# define MOD_CONTROL_LEFT	(1<<0)
# define MOD_SHIFT_LEFT		(1<<1)
# define MOD_ALT_LEFT		(1<<2)
# define MOD_GUI_LEFT		(1<<3)
# define MOD_CONTROL_RIGHT	(1<<4)
# define MOD_SHIFT_RIGHT	(1<<5)
# define MOD_ALT_RIGHT		(1<<6)
# define MOD_GUI_RIGHT		(1<<7)

# define KEY_A			4
# define KEY_B			5
# define KEY_C			6
# define KEY_D			7
# define KEY_E			8
# define KEY_F			9
# define KEY_G			10
# define KEY_H			11
# define KEY_I			12
# define KEY_J			13
# define KEY_K			14
# define KEY_L			15
# define KEY_M			16
# define KEY_N			17
# define KEY_O			18
# define KEY_P			19
# define KEY_Q			20
# define KEY_R			21
# define KEY_S			22
# define KEY_T			23
# define KEY_U			24
# define KEY_V			25
# define KEY_W			26
# define KEY_X			27
# define KEY_Y			28
# define KEY_Z			29

# define KEY_1			30
# define KEY_2			31
# define KEY_3			32
# define KEY_4			33
# define KEY_5			34
# define KEY_6			35
# define KEY_7			36
# define KEY_8			37
# define KEY_9			38
# define KEY_0			39

# define KEY_ENTER		40
# define KEY_ESCAPE		41
# define KEY_DELETE_BACKWARD	42
# define KEY_TAB		43
# define KEY_SPACE		44
# define KEY_MINUS		45
# define KEY_EQUAL		46
# define KEY_BRACKET_LEFT	47
# define KEY_BRACKET_RIGHT	48
# define KEY_BACKSLASH		49

# define KEY_SEMICOLON		51
# define KEY_SINGLEQUOTE	52
# define KEY_GRAVEACCENT	53
# define KEY_COMMA		54
# define KEY_PERIOD		55
# define KEY_SLASH		56
# define KEY_CAPSLOCK		57

# define KEY_F1			58
# define KEY_F2			59
# define KEY_F3			60
# define KEY_F4			61
# define KEY_F5			62
# define KEY_F6			63
# define KEY_F7			64
# define KEY_F8			65
# define KEY_F9			66
# define KEY_F10		67
# define KEY_F11		68
# define KEY_F12		69

# define KEY_PRINT		70
# define KEY_SCROLLLOCK		71
# define KEY_PAUSE		72
# define KEY_INSERT		73
# define KEY_HOME		74
# define KEY_PAGEUP		75
# define KEY_DELETE_FORWARD	76
# define KEY_END		77
# define KEY_PAGEDOWN		78
# define KEY_RIGHTARROW		79
# define KEY_LEFTARROW		80
# define KEY_DOWNARROW		81
# define KEY_UPARROW		82

# define KEY_LEFT_CONTROL	224
# define KEY_LEFT_SHIFT		225
# define KEY_LEFT_ALT		226
# define KEY_LEFT_GUI		227
# define KEY_RIGHT_CONTROL	228
# define KEY_RIGHT_SHIFT	229
# define KEY_RIGHT_ALT		230
# define KEY_RIGHT_GUI		231

#endif // HID_KEYBOARD_SYMBOLS_H

