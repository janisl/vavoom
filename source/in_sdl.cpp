//**************************************************************************
//**
//**    ##   ##    ##    ##   ##   ####     ####   ###     ###
//**    ##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**     ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**     ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**      ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**       #    ##    ##    #      ####     ####   ##       ##
//**
//**    $Id$
//**
//**    Copyright (C) 1999-2002 J306nis Legzdi267375
//**
//**    This program is free software; you can redistribute it and/or
//**  modify it under the terms of the GNU General Public License
//**  as published by the Free Software Foundation; either version 2
//**  of the License, or (at your option) any later version.
//**
//**    This program is distributed in the hope that it will be useful,
//**  but WITHOUT ANY WARRANTY; without even the implied warranty of
//**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//**  GNU General Public License for more details.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <SDL/SDL.h>
#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// static TCvarI m_filter("m_filter", "1", CVAR_ARCHIVE);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int mouse;
static int joy;

static int mouse_oldx;
static int mouse_oldy;

// tested with SDL 1.2.2
static int sym2key[SDLK_LAST] = {
//0,
  0,
//1,  2,  3,  4,  5,  6,  7,  8,            9,     10, 11, 12, 13,      14, 15, 16,
  1,  2,  3,  4,  5,  6,  7,  K_BACKSPACE,  K_TAB, 10, 11, 12, K_ENTER, 14, 15, 16,
//17, 18, 19,      20, 21, 22, 23, 24, 25, 26, 27,       28, 29, 30, 31, 32,
  17, 18, K_PAUSE, 20, 21, 22, 23, 24, 25, 26, K_ESCAPE, 28, 29, 30, 31, ' ',
//33, 34, 35, 36, 37, 38, 39,   40, 41, 42, 43, 44,  45,  46,  47,  48,
  33, 34, 35, 36, 37, 38, '\'', 40, 41, 42, 43, ',', '-', '.', '/', '0',
//49,  50,  51,  52,  53,  54,  55,  56,  57,  58, 59,  60, 61,  62, 63, 64,
  '1', '2', '3', '4', '5', '6', '7', '8', '9', 58, ';', 60, '=', 62, 63, 64,
//65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
  65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
//81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,  92, 93,  94, 95, 96,
  81, 82, 83, 84, 85, 86, 87, 88, 89, 90, '[', '\\', ']', 94, 95, '`',
//97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112,
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
//113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,      128,
  'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 123, 124, 125, 126, K_DELETE, 128,
//129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,
  129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,
//145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,
  145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,
//161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,
  161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,
//177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,
  177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,
//193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,
  193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,
//209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,
  209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,
//225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,
  225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,
//241,242,243,244,245,246,247,248,
  241,242,243,244,245,246,247,248,
//249,250,251,252,253,254,255, 256,
  249,250,251,252,253,254,255, K_PAD0,
//257,    258,    259,    260,    261,    262,    263,    264,
  K_PAD1, K_PAD2, K_PAD3, K_PAD4, K_PAD5, K_PAD6, K_PAD7, K_PAD8,
//265,    266,      267,         268,           269,        270,       271,        272,
  K_PAD9, K_PADDOT, K_PADDIVIDE, K_PADMULTIPLE, K_PADMINUS, K_PADPLUS, K_PADENTER, '=',
//273,       274,         275,          276,         277,      278,    279,   280,
  K_UPARROW, K_DOWNARROW, K_RIGHTARROW, K_LEFTARROW, K_INSERT, K_HOME, K_END, K_PAGEUP,
//281,        282,  283,  284,  285,  286,  287,  288,
  K_PAGEDOWN, K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7,
//289,  290,  291,   292,   293,   294, 295, 296, 297, 298, 299, 300,       301,        302,          303,      304,
  K_F8, K_F9, K_F10, K_F11, K_F12, 0,   0,   0,   0,   0,   0,   K_NUMLOCK, K_CAPSLOCK, K_SCROLLLOCK, K_RSHIFT, K_LSHIFT,
//305,     306,     307,    308,    309, 310, 311,    312,    313, 314, 315, 316,         317, 318,     319,    320,
  K_RCTRL, K_LCTRL, K_RALT, K_LALT, 0,   0,   K_LWIN, K_RWIN, 0,   0,   0,   K_PRINTSCRN, 0,   K_PAUSE, K_MENU, 0,
//321
  0
};

// CODE --------------------------------------------------------------------

//**************************************************************************
//**
//**    INPUT
//**
//**************************************************************************

//==========================================================================
//
//  IN_Init
//
//==========================================================================

void IN_Init(void)
{
	guard(IN_Init);
	// always off
	SDL_ShowCursor(0);
	// mouse and keyboard are setup using SDL's video interface
	mouse = 1;
	if (M_CheckParm("-nomouse"))
	{
		SDL_EventState(SDL_MOUSEMOTION,     SDL_IGNORE);
		SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
		SDL_EventState(SDL_MOUSEBUTTONUP,   SDL_IGNORE);
		mouse = 0;
	}
	else
	{
		// ignore mouse motion events in any case...
		SDL_EventState(SDL_MOUSEMOTION,     SDL_IGNORE);
		mouse_oldx = ScreenWidth / 2;
		mouse_oldy = ScreenHeight / 2;
		SDL_WarpMouse(mouse_oldx, mouse_oldy);
	}

	// initialize joystick
	joy = 1;
	if (M_CheckParm("-nojoy"))
	{
		joy = 0;
	}
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0 && joy)
	{
		joy = 0;
	}
	else
	{
		SDL_JoystickEventState(SDL_IGNORE);
		// we are on our own now...
	}
	unguard;
}

//==========================================================================
//
//  IN_ReadInput
//
//      Called by D_DoomLoop before processing each tic in a frame.
//      Can call D_PostEvent.
//      Asyncronous interrupt functions should maintain private ques that are
// read by the syncronous functions to be converted into events.
//
//==========================================================================

void IN_ReadInput(void)
{
	guard(IN_ReadInput);
	SDL_Event ev;
	event_t vev;
	int rel_x;
	int rel_y;
	int mouse_x;
	int mouse_y;

	SDL_PumpEvents();
	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			IN_KeyEvent(sym2key[ev.key.keysym.sym],
				(ev.key.state == SDL_PRESSED) ? 1 : 0);
			break;
#if 0
		case SDL_MOUSEMOTION:
			vev.type = ev_mouse;
			vev.data1 = 0;
			vev.data2 = ev.motion.xrel;
			vev.data3 = ev.motion.yrel;
			IN_PostEvent(&vev);
			break;
#endif /* 0 */
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			vev.type = (ev.button.state == SDL_PRESSED) ? ev_keydown : ev_keyup;
			if (ev.button.button == SDL_BUTTON_LEFT)
				vev.data1 = K_MOUSE1;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				vev.data1 = K_MOUSE2;
			else if (ev.button.button == SDL_BUTTON_MIDDLE)
				vev.data1 = K_MOUSE3;
			else
				break;
			vev.data2 = 0;
			vev.data3 = 0;
			IN_PostEvent(&vev);
			break;
		default:
			break;
		}
	}

	// read mouse separately
	if (mouse)
	{
		SDL_GetMouseState(&mouse_x, &mouse_y);
		vev.type = ev_mouse;
		vev.data1 = 0;
		vev.data2 = mouse_x - ScreenWidth / 2;
		vev.data3 = mouse_y - ScreenHeight / 2;
		IN_PostEvent(&vev);
		SDL_WarpMouse(ScreenWidth / 2, ScreenHeight / 2);
#if 0
		SDL_GetRelativeMouseState(&rel_x, &rel_y);
		vev.type = ev_mouse;
		vev.data1 = 0;
		vev.data2 = rel_x;
		vev.data3 = rel_y;
		IN_PostEvent(&vev);
#endif /* 0 */
	}

	// read joystick separately
/*
	if (joy)
		ReadJoystick();
*/
	unguard;
}

//==========================================================================
//
//  IN_Shutdown
//
//==========================================================================

void IN_Shutdown(void)
{
	// on
	SDL_ShowCursor(1);

	if (joy)
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//
//	Revision 1.1  2002/01/03 18:39:42  dj_jl
//	Added SDL port
//	
//**************************************************************************
