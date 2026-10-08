/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: KeyDefs.h ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information					         
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  KeyDefs.h
//
// Created:    Mike Morrison, 1995
//						 Colin Day, June 2001
//
// Desc:       Basic keyboard key definitions.
//
// Key values are the original game input protocol. Native providers translate
// their physical keys at admission; no DirectInput SDK types are needed here.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __KEYDEFS_H_
#define __KEYDEFS_H_

#include "Lib/BaseType.h"

//=============================================================================
/** The key tables */
//=============================================================================

enum KeyDefType : UnsignedInt
{
	// keypad keys ---------------------------------------------------------------- 
	KEY_KP0 								= 0x52,
	KEY_KP1 								= 0x4F,
	KEY_KP2 								= 0x50,
	KEY_KP3 								= 0x51,
	KEY_KP4 								= 0x4B,
	KEY_KP5 								= 0x4C,
	KEY_KP6 								= 0x4D,
	KEY_KP7 								= 0x47,
	KEY_KP8 								= 0x48,
	KEY_KP9 								= 0x49,
	KEY_KPDEL 							= 0x53,
	KEY_KPSTAR 							= 0x37,
	KEY_KPMINUS 						= 0x4A,
	KEY_KPPLUS 							= 0x4E,

	// regular keys ---------------------------------------------------------------
	KEY_ESC 								= 0x01,
	KEY_BACKSPACE 					= 0x0E,
	KEY_ENTER 							= 0x1C,
	KEY_SPACE 							= 0x39,
	KEY_TAB 								= 0x0F,
	KEY_F1 									= 0x3B,
	KEY_F2 									= 0x3C,
	KEY_F3 									= 0x3D,
	KEY_F4 									= 0x3E,
	KEY_F5 									= 0x3F,
	KEY_F6 									= 0x40,
	KEY_F7 									= 0x41,
	KEY_F8 									= 0x42,
	KEY_F9 									= 0x43,
	KEY_F10 								= 0x44,
	KEY_F11 								= 0x57,
	KEY_F12 								= 0x58,
	KEY_A 									= 0x1E,
	KEY_B 									= 0x30,
	KEY_C 									= 0x2E,
	KEY_D 									= 0x20,
	KEY_E 									= 0x12,
	KEY_F 									= 0x21,
	KEY_G 									= 0x22,
	KEY_H 									= 0x23,
	KEY_I 									= 0x17,
	KEY_J 									= 0x24,
	KEY_K 									= 0x25,
	KEY_L 									= 0x26,
	KEY_M 									= 0x32,
	KEY_N 									= 0x31,
	KEY_O 									= 0x18,
	KEY_P 									= 0x19,
	KEY_Q 									= 0x10,
	KEY_R 									= 0x13,
	KEY_S 									= 0x1F,
	KEY_T 									= 0x14,
	KEY_U 									= 0x16,
	KEY_V 									= 0x2F,
	KEY_W 									= 0x11,
	KEY_X 									= 0x2D,
	KEY_Y 									= 0x15,
	KEY_Z 									= 0x2C,
	KEY_1 									= 0x02,
	KEY_2 									= 0x03,
	KEY_3 									= 0x04,
	KEY_4 									= 0x05,
	KEY_5 									= 0x06,
	KEY_6 									= 0x07,
	KEY_7 									= 0x08,
	KEY_8 									= 0x09,
	KEY_9 									= 0x0A,
	KEY_0 									= 0x0B,
	KEY_MINUS 							= 0x0C,
	KEY_EQUAL 							= 0x0D,
	KEY_LBRACKET 						= 0x1A,
	KEY_RBRACKET 						= 0x1B,
	KEY_SEMICOLON 					= 0x27,
	KEY_APOSTROPHE 					= 0x28,
	KEY_TICK 								= 0x29,
	KEY_BACKSLASH 					= 0x2B,
	KEY_COMMA 							= 0x33,
	KEY_PERIOD 							= 0x34,
	KEY_SLASH 							= 0x35,

	// special keys ---------------------------------------------------------------
	KEY_SYSREQ 							= 0xB7,

	KEY_CAPS 								= 0x3A,
	KEY_NUM 								= 0x45,
	KEY_SCROLL 							= 0x46,
	KEY_LCTRL 							= 0x1D,
	KEY_LALT 								= 0x38,
	KEY_LSHIFT 							= 0x2A,
	KEY_RSHIFT 							= 0x36,

	KEY_UP 									= 0xC8,
	KEY_DOWN 								= 0xD0,
	KEY_LEFT 								= 0xCB,
	KEY_RIGHT 							= 0xCD,
	KEY_RALT 								= 0xB8,
	KEY_RCTRL 							= 0x9D,
	KEY_HOME 								= 0xC7,
	KEY_END 								= 0xCF,
	KEY_PGUP 								= 0xC9,
	KEY_PGDN 								= 0xD1,
	KEY_INS 								= 0xD2,
	KEY_DEL 								= 0xD3,
	KEY_KPENTER 						= 0x9C,
	KEY_KPSLASH 						= 0xB5,

	KEY_102 								= 0x56,

	// Japanese keyboard keys -----------------------------------------------------
	KEY_KANA 								= 0x70,
	KEY_CONVERT 						= 0x79,
	KEY_NOCONVERT 					= 0x7B,
	KEY_YEN 								= 0x7D,
	KEY_CIRCUMFLEX 					= 0x90,
	KEY_KANJI 							= 0x94,

	// specials -------------------------------------------------------------------
	KEY_NONE								= 0x00,		///< to report end of key stream
	KEY_LOST								= 0xFF		///< to report lost keyboard focus

};	// end KeyDefType

// state for keyboard IO ------------------------------------------------------
enum
{
	KEY_STATE_NONE								= 0x0000, // No modifier state
	KEY_STATE_UP									= 0x0001,	// Key is up (default state)
	KEY_STATE_DOWN								= 0x0002,	// Key is down
	KEY_STATE_LCONTROL						= 0x0004,	// Left control is pressed
	KEY_STATE_RCONTROL						= 0x0008,	// Right control is pressed
	KEY_STATE_LSHIFT							= 0x0010,	// left shift is pressed
	KEY_STATE_RSHIFT							= 0x0020,	// right shift is pressed
	KEY_STATE_LALT								= 0x0040,	// left alt is pressed
	KEY_STATE_RALT								= 0x0080,	// right alt is pressed
	KEY_STATE_AUTOREPEAT					= 0x0100,	// Key is down due to autorepeat (only seen in conjunction with KEY_STATE_DOWN)
	KEY_STATE_CAPSLOCK						= 0x0200, // Caps Lock key is on.
	KEY_STATE_SHIFT2							= 0x0400, // Alternate shift key is pressed (I think this is for foreign keyboards..)

	// modifier combinations when left/right isn't a factor
	KEY_STATE_CONTROL		= (KEY_STATE_LCONTROL | KEY_STATE_RCONTROL),
	KEY_STATE_SHIFT			= (KEY_STATE_LSHIFT | KEY_STATE_RSHIFT | KEY_STATE_SHIFT2 ),
	KEY_STATE_ALT				= (KEY_STATE_LALT | KEY_STATE_RALT)

};	// end KeyStateType

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////

#endif // __KEYDEFS_H_
