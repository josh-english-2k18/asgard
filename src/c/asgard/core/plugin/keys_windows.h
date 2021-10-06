/*
 * keys_windows.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The set of Windows keyboard keys for the Asgard Game Engine, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_CORE_PLUGIN_KEYS_WINDOWS_H)

#define _ASGARD_CORE_PLUGIN_KEYS_WINDOWS_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_CORE_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_CORE_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(WIN32)

// define keys (Windows) public constants

#define OS_KEY_BACKSPACE					VK_BACK
#define OS_KEY_TAB							VK_TAB
#define OS_KEY_CLEAR						VK_CLEAR
#define OS_KEY_RETURN						VK_RETURN
#define OS_KEY_SHIFT						VK_SHIFT
#define OS_KEY_CONTROL						VK_CONTROL
#define OS_KEY_CAPS_LOCK					VK_CAPITAL
#define OS_KEY_ESCAPE						VK_ESCAPE
#define OS_KEY_SPACE						VK_SPACE
#define OS_KEY_PAGE_UP						VK_PRIOR
#define OS_KEY_PAGE_DOWN					VK_NEXT
#define OS_KEY_HOME							VK_HOME
#define OS_KEY_END							VK_END
#define OS_KEY_UP							VK_UP
#define OS_KEY_DOWN							VK_DOWN
#define OS_KEY_LEFT							VK_LEFT
#define OS_KEY_RIGHT						VK_RIGHT
#define OS_KEY_SELECT						VK_SELECT
#define OS_KEY_PRINT						VK_PRINT
#define OS_KEY_EXECUTE						VK_EXECUTE
#define OS_KEY_PRINT_SCREEN					VK_SNAPSHOT
#define OS_KEY_INSERT						VK_INSERT
#define OS_KEY_DELETE						VK_DELETE

#define OS_KEY_NP_0							VK_NUMPAD0
#define OS_KEY_NP_1							VK_NUMPAD1
#define OS_KEY_NP_2							VK_NUMPAD2
#define OS_KEY_NP_3							VK_NUMPAD3
#define OS_KEY_NP_4							VK_NUMPAD4
#define OS_KEY_NP_5							VK_NUMPAD5
#define OS_KEY_NP_6							VK_NUMPAD6
#define OS_KEY_NP_7							VK_NUMPAD7
#define OS_KEY_NP_8							VK_NUMPAD8
#define OS_KEY_NP_9							VK_NUMPAD9
#define OS_KEY_NP_ADD						VK_ADD
#define OS_KEY_NP_SUBTRACT					VK_SUBTRACT
#define OS_KEY_NP_MULTIPLY					VK_MULTIPLY
#define OS_KEY_NP_DIVIDE					VK_DIVIDE
#define OS_KEY_NP_DECIMAL					VK_DECIMAL

#define OS_KEY_F1							VK_F1
#define OS_KEY_F2							VK_F2
#define OS_KEY_F3							VK_F3
#define OS_KEY_F4							VK_F4
#define OS_KEY_F5							VK_F5
#define OS_KEY_F6							VK_F6
#define OS_KEY_F7							VK_F7
#define OS_KEY_F8							VK_F8
#define OS_KEY_F9							VK_F9
#define OS_KEY_F10							VK_F10
#define OS_KEY_F11							VK_F11
#define OS_KEY_F12							VK_F12

#define OS_KEY_0							0x30
#define OS_KEY_1							0x31
#define OS_KEY_2							0x32
#define OS_KEY_3							0x33
#define OS_KEY_4							0x34
#define OS_KEY_5							0x35
#define OS_KEY_6							0x36
#define OS_KEY_7							0x37
#define OS_KEY_8							0x38
#define OS_KEY_9							0x39

#define OS_KEY_A							0x41
#define OS_KEY_B							0x42
#define OS_KEY_C							0x43
#define OS_KEY_D							0x44
#define OS_KEY_E							0x45
#define OS_KEY_F							0x46
#define OS_KEY_G							0x47
#define OS_KEY_H							0x48
#define OS_KEY_I							0x49
#define OS_KEY_J							0x4a
#define OS_KEY_K							0x4b
#define OS_KEY_L							0x4c
#define OS_KEY_M							0x4d
#define OS_KEY_N							0x4e
#define OS_KEY_O							0x4f
#define OS_KEY_P							0x50
#define OS_KEY_Q							0x51
#define OS_KEY_R							0x52
#define OS_KEY_S							0x53
#define OS_KEY_T							0x54
#define OS_KEY_U							0x55
#define OS_KEY_V							0x56
#define OS_KEY_W							0x57
#define OS_KEY_X							0x58
#define OS_KEY_Y							0x59
#define OS_KEY_Z							0x5a

#endif // WIN32


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_CORE_PLUGIN_KEYS_WINDOWS_H

