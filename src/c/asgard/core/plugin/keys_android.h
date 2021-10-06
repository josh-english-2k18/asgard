/*
 * keys_android.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The set of Android keyboard keys for the Asgard Game Engine, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_CORE_PLUGIN_KEYS_ANDROID_H)

#define _ASGARD_CORE_PLUGIN_KEYS_ANDROID_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_CORE_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_CORE_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(__ANDROID__)

// define keys (Android) public constants

#define OS_KEY_BACKSPACE					0
#define OS_KEY_TAB							1
#define OS_KEY_CLEAR						2
#define OS_KEY_RETURN						3
#define OS_KEY_SHIFT						4
#define OS_KEY_CONTROL						5
#define OS_KEY_CAPS_LOCK					6
#define OS_KEY_ESCAPE						7
#define OS_KEY_SPACE						8
#define OS_KEY_PAGE_UP						9
#define OS_KEY_PAGE_DOWN					10
#define OS_KEY_HOME							11
#define OS_KEY_END							12
#define OS_KEY_UP							13
#define OS_KEY_DOWN							14
#define OS_KEY_LEFT							15
#define OS_KEY_RIGHT						16
#define OS_KEY_SELECT						17
#define OS_KEY_PRINT						18
#define OS_KEY_EXECUTE						19
#define OS_KEY_PRINT_SCREEN					20
#define OS_KEY_INSERT						21
#define OS_KEY_DELETE						22

#define OS_KEY_NP_0							23
#define OS_KEY_NP_1							24
#define OS_KEY_NP_2							25
#define OS_KEY_NP_3							26
#define OS_KEY_NP_4							27
#define OS_KEY_NP_5							28
#define OS_KEY_NP_6							29
#define OS_KEY_NP_7							30
#define OS_KEY_NP_8							31
#define OS_KEY_NP_9							32
#define OS_KEY_NP_ADD						33
#define OS_KEY_NP_SUBTRACT					34
#define OS_KEY_NP_MULTIPLY					35
#define OS_KEY_NP_DIVIDE					36
#define OS_KEY_NP_DECIMAL					37

#define OS_KEY_F1							38
#define OS_KEY_F2							39
#define OS_KEY_F3							40
#define OS_KEY_F4							41
#define OS_KEY_F5							42
#define OS_KEY_F6							43
#define OS_KEY_F7							44
#define OS_KEY_F8							45
#define OS_KEY_F9							46
#define OS_KEY_F10							47
#define OS_KEY_F11							48
#define OS_KEY_F12							49

#define OS_KEY_0							50
#define OS_KEY_1							51
#define OS_KEY_2							52
#define OS_KEY_3							53
#define OS_KEY_4							54
#define OS_KEY_5							55
#define OS_KEY_6							56
#define OS_KEY_7							57
#define OS_KEY_8							58
#define OS_KEY_9							59

#define OS_KEY_A							60
#define OS_KEY_B							61
#define OS_KEY_C							62
#define OS_KEY_D							63
#define OS_KEY_E							64
#define OS_KEY_F							65
#define OS_KEY_G							66
#define OS_KEY_H							67
#define OS_KEY_I							68
#define OS_KEY_J							69
#define OS_KEY_K							70
#define OS_KEY_L							71
#define OS_KEY_M							72
#define OS_KEY_N							73
#define OS_KEY_O							74
#define OS_KEY_P							75
#define OS_KEY_Q							76
#define OS_KEY_R							77
#define OS_KEY_S							78
#define OS_KEY_T							79
#define OS_KEY_U							80
#define OS_KEY_V							81
#define OS_KEY_W							82
#define OS_KEY_X							83
#define OS_KEY_Y							84
#define OS_KEY_Z							85

#endif // __ANDROID__


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_CORE_PLUGIN_KEYS_ANDROID_H

