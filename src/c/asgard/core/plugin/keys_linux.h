/*
 * keys_linux.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The set of Linux keyboard keys for the Asgard Game Engine, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_CORE_PLUGIN_KEYS_LINUX_H)

#define _ASGARD_CORE_PLUGIN_KEYS_LINUX_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_CORE_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_CORE_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(__linux__) && !defined(__ANDROID__)

// define keys (Linux) public constants

#define OS_KEY_BACKSPACE					XK_BackSpace
#define OS_KEY_TAB							XK_Tab
#define OS_KEY_CLEAR						XK_Clear
#define OS_KEY_RETURN						XK_Return
#define OS_KEY_SHIFT						XK_Shift_L // TODO: better way?
#define OS_KEY_CONTROL						XK_Control_L // TODO: better way?
#define OS_KEY_CAPS_LOCK					XK_Caps_Lock
#define OS_KEY_ESCAPE						XK_Escape
#define OS_KEY_SPACE						XK_space
#define OS_KEY_PAGE_UP						XK_Page_Up
#define OS_KEY_PAGE_DOWN					XK_Page_Down
#define OS_KEY_HOME							XK_Home
#define OS_KEY_END							XK_End
#define OS_KEY_UP							XK_Up
#define OS_KEY_DOWN							XK_Down
#define OS_KEY_LEFT							XK_Left
#define OS_KEY_RIGHT						XK_Right
#define OS_KEY_SELECT						XK_Select
#define OS_KEY_PRINT						XK_Print
#define OS_KEY_EXECUTE						XK_Execute
#define OS_KEY_PRINT_SCREEN					XK_Print
#define OS_KEY_INSERT						XK_Insert
#define OS_KEY_DELETE						XK_Delete

#define OS_KEY_NP_0							XK_KP_0
#define OS_KEY_NP_1							XK_KP_1
#define OS_KEY_NP_2							XK_KP_2
#define OS_KEY_NP_3							XK_KP_3
#define OS_KEY_NP_4							XK_KP_4
#define OS_KEY_NP_5							XK_KP_5
#define OS_KEY_NP_6							XK_KP_6
#define OS_KEY_NP_7							XK_KP_7
#define OS_KEY_NP_8							XK_KP_8
#define OS_KEY_NP_9							XK_KP_9
#define OS_KEY_NP_ADD						XK_KP_Add
#define OS_KEY_NP_SUBTRACT					XK_KP_Subtract
#define OS_KEY_NP_MULTIPLY					XK_KP_Multiply
#define OS_KEY_NP_DIVIDE					XK_KP_Divide
#define OS_KEY_NP_DECIMAL					XK_KP_Decimal

#define OS_KEY_F1							XK_F1
#define OS_KEY_F2							XK_F2
#define OS_KEY_F3							XK_F3
#define OS_KEY_F4							XK_F4
#define OS_KEY_F5							XK_F5
#define OS_KEY_F6							XK_F6
#define OS_KEY_F7							XK_F7
#define OS_KEY_F8							XK_F8
#define OS_KEY_F9							XK_F9
#define OS_KEY_F10							XK_F10
#define OS_KEY_F11							XK_F11
#define OS_KEY_F12							XK_F12

#define OS_KEY_0							XK_0
#define OS_KEY_1							XK_1
#define OS_KEY_2							XK_2
#define OS_KEY_3							XK_3
#define OS_KEY_4							XK_4
#define OS_KEY_5							XK_5
#define OS_KEY_6							XK_6
#define OS_KEY_7							XK_7
#define OS_KEY_8							XK_8
#define OS_KEY_9							XK_9

#define OS_KEY_A							XK_a
#define OS_KEY_B							XK_b
#define OS_KEY_C							XK_c
#define OS_KEY_D							XK_d
#define OS_KEY_E							XK_e
#define OS_KEY_F							XK_f
#define OS_KEY_G							XK_g
#define OS_KEY_H							XK_h
#define OS_KEY_I							XK_i
#define OS_KEY_J							XK_j
#define OS_KEY_K							XK_k
#define OS_KEY_L							XK_l
#define OS_KEY_M							XK_m
#define OS_KEY_N							XK_n
#define OS_KEY_O							XK_o
#define OS_KEY_P							XK_p
#define OS_KEY_Q							XK_q
#define OS_KEY_R							XK_r
#define OS_KEY_S							XK_s
#define OS_KEY_T							XK_t
#define OS_KEY_U							XK_u
#define OS_KEY_V							XK_v
#define OS_KEY_W							XK_w
#define OS_KEY_X							XK_x
#define OS_KEY_Y							XK_y
#define OS_KEY_Z							XK_z

#endif // __linux__ && !__ANDROID__


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_CORE_PLUGIN_KEYS_LINUX_H

