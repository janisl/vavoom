//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id$
//**
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
//**
//**	This program is free software; you can redistribute it and/or
//**  modify it under the terms of the GNU General Public License
//**  as published by the Free Software Foundation; either version 2
//**  of the License, or (at your option) any later version.
//**
//**	This program is distributed in the hope that it will be useful,
//**  but WITHOUT ANY WARRANTY; without even the implied warranty of
//**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//**  GNU General Public License for more details.
//**
//**************************************************************************
//**
//**	Input modue for Allegro library. Used only in Linux (in Windows I
//**  don't use Allegro, in DOS keyboard and mouse are handled directly).
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <allegro.h>
//#include <signal.h>
#undef stricmp
#undef strnicmp

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define KBDQUESIZE			32

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static boolean 			    keyboard_started = false;
static byte					keyboardque[KBDQUESIZE];
static int 					kbdtail = 0;
static int					kbdhead = 0;
static byte					scantokey[KEY_MAX] =
{
	0,
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	K_PAD0, K_PAD1, K_PAD2, K_PAD3, K_PAD4,
	K_PAD5, K_PAD6, K_PAD7, K_PAD8, K_PAD9,
	K_F1, K_F2, K_F3, K_F4, K_F5, K_F6,
	K_F7, K_F8, K_F9, K_F10, K_F11, K_F12,
	K_ESCAPE, '`', '-', '=', K_BACKSPACE, K_TAB,
	'[', ']', K_ENTER, ';', '\'', '\\', '\\', ',', '.', '/', ' ',
	K_INSERT, K_DELETE, K_HOME, K_END, K_PAGEUP, K_PAGEDOWN,
	K_LEFTARROW, K_RIGHTARROW, K_UPARROW, K_DOWNARROW,
	K_PADDIVIDE, K_PADMULTIPLE, K_PADMINUS, K_PADPLUS,
	K_PADDOT, K_PADENTER, K_PRINTSCRN, K_PAUSE, 0, 0,
	K_LSHIFT, K_RSHIFT, K_LCTRL, K_RCTRL, K_LALT, K_RALT, 
	K_LWIN, K_RWIN, K_MENU, K_SCROLLLOCK, K_NUMLOCK, K_CAPSLOCK
};

static boolean				mouse_started    = false;
static int					old_mouse_x;
static int					old_mouse_y;
static TCvarI				m_filter("m_filter", "1", CVAR_ARCHIVE);

static boolean				joystick_started = false;
static int					joy_oldx = 0;
static int					joy_oldy = 0;
static int					joy_oldb[MAX_JOYSTICK_BUTTONS];

// CODE --------------------------------------------------------------------

//**************************************************************************
//**
//**	KEYBOARD
//**
//**************************************************************************

//==========================================================================
//
//  KeyboardHandler
//
// 	Keyboard interrupt handler
//
//==========================================================================

static void KeyboardHandler(int key)
{
	// Get the scan code

	keyboardque[kbdhead & (KBDQUESIZE - 1)] = key;
	kbdhead++;
}
END_OF_FUNCTION(KeyboardHandler)

//==========================================================================
//
//  ReadKeyboard
//
//==========================================================================

static void ReadKeyboard(void)
{
    unsigned char 	ch;
//	static   int  	ctrl = 0;

    if (!keyboard_started)
    	return;

	while (kbdtail < kbdhead)
	{
	    ch = keyboardque[kbdtail&(KBDQUESIZE-1)];
		kbdtail++;

		IN_KeyEvent(scantokey[ch & 0x7f], !(ch & 0x80));

//		if (event.data1 == K_LCTRL || event.data1 == K_RCTRL)
//			ctrl = (event.type == ev_keydown);
//	    if ((ctrl) && (ch == 0x2e)) // crtl-c
//    	{
//	    	raise(SIGINT);
//	    }
	}
}

//==========================================================================
//
//	StartupKeyboard
//
//	Installs the keyboard handler.
//
//==========================================================================

static void StartupKeyboard(void)
{
    LOCK_FUNCTION(KeyboardHandler);
    LOCK_DATA(keyboardque, sizeof(keyboardque));
    LOCK_VARIABLE(kbdhead);

    if (install_keyboard())
    {
		Sys_Error("Failed to initialize keyboard");
    }
    keyboard_lowlevel_callback = KeyboardHandler;
    keyboard_started = true;
}

//==========================================================================
//
//  ShutdownKeyboard
//
// 	Removes the keyboard handler.
//
//==========================================================================

static void ShutdownKeyboard(void)
{
	if (keyboard_started)
    {
	    remove_keyboard();
	}
}

//**************************************************************************
//**
//**	MOUSE
//**
//**************************************************************************

//==========================================================================
//
// 	StartupMouse
//
//	Initializes mouse
//
//==========================================================================

static void StartupMouse(void)
{
    int		buts;
    
    if (M_CheckParm("-nomouse"))
    	return;

    buts = install_mouse();
    if (buts == -1)
    {
		return;
    }
    mouse_started = true;
}

//==========================================================================
//
//  ReadMouse
//
// 	Reads mouse.
//
//==========================================================================

static void ReadMouse(void)
{
	int			i;
    event_t 	event;
    int 		xmickeys;
    int			ymickeys;
    int			mouse_x;
    int			mouse_y;
    int			buttons;
    static int 	lastbuttons = 0;

    if (!mouse_started)
		return;

    poll_mouse();
    get_mouse_mickeys(&xmickeys, &ymickeys);
    buttons = mouse_b;

	if (m_filter == 2)
	{
		mouse_x = (xmickeys + old_mouse_x) / 2;
		mouse_y = (ymickeys + old_mouse_y) / 2;
		old_mouse_x = mouse_x;
		old_mouse_y = mouse_y;
	}
	else if (m_filter == 1)
	{
		mouse_x = (xmickeys + old_mouse_x) / 2;
		mouse_y = (ymickeys + old_mouse_y) / 2;
		old_mouse_x = xmickeys;
		old_mouse_y = ymickeys;
	}
	else
	{
		mouse_x = xmickeys;
		mouse_y = ymickeys;
		old_mouse_x = 0;
		old_mouse_y = 0;
	}

    if (mouse_x || mouse_y)
    {
      	event.type  = ev_mouse;
      	event.data1 = 0;
      	event.data2 = mouse_x;
      	event.data3 = -mouse_y;

      	IN_PostEvent(&event);
    }
	for (i = 0; i < 3; i++)
    {
	    if ((buttons ^ lastbuttons) & (1 << i))
    	{
    		IN_KeyEvent(K_MOUSE1 + i, buttons & (1 << i));
	    }
    }
  	lastbuttons = buttons;
}

//==========================================================================
//
//	ShutdownMouse
//
//==========================================================================

static void ShutdownMouse(void)
{
    if (!mouse_started)
    	return;
    remove_mouse();
}

//**************************************************************************
//**
//**	JOYSTICK
//**
//**************************************************************************

//==========================================================================
//
//	StartupJoystick
//
// 	Initializes joystick
//
//==========================================================================

static void StartupJoystick(void)
{
	if (M_CheckParm("-nojoy"))
    	return;

	//	Detect the joystick type
	if (install_joystick(JOY_TYPE_AUTODETECT))
	{
		Sys_Error("Error initialising joystick\n%s\n", allegro_error);
	}

	//	Make sure that we really do have a joystick
	if (joy_type == JOY_TYPE_NONE)
	{
		return;
	}

	//	When initialising joystick, it must be centered, so we have to remove
    // it, give a message to center joystick, wait for keypress and then
    // reinitialize it.
	if (joy[0].flags & JOYFLAG_CALIBRATE)
    {
		remove_joystick();

		printf("CENTER the joystick and press a key:\n");
		IN_ReadKey();

		// Initialize the joystick driver
		if (install_joystick(joy_type))
		{
			Sys_Error("Error initialising joystick\n%s\n", allegro_error);
		}
    }

	//	Calibrate joystick
	while (joy[0].flags & JOYFLAG_CALIBRATE)
	{
	   	printf("%s and press a key:\n", calibrate_joystick_name(0));
	    IN_ReadKey();

		if (calibrate_joystick(0))
		{
			Sys_Error("Error calibrating joystick!\n");
		}
	}

    joystick_started = true;
	memset(joy_oldb, 0, sizeof(joy_oldb));
}

//==========================================================================
//
//  ReadJoystick
//
//==========================================================================

static void ReadJoystick(void)
{
	int			i;
    event_t 	event;

    if (!joystick_started)
    	return;

    poll_joystick();

    if ((joy_oldx != joy_x) || (joy_oldy != joy_y))
    {
		event.type = ev_joystick;
		event.data1 = 0;
		event.data2 = (abs(joy_x) < 4)? 0 : joy_x;
		event.data3 = (abs(joy_y) < 4)? 0 : joy_y;
		IN_PostEvent(&event);

	    joy_oldx = joy_x;
    	joy_oldy = joy_y;
    }
    for (i = 0; i < joy[0].num_buttons; i++)
    {
		if (joy[0].button[i].b != joy_oldb[i])
    	{
        	IN_KeyEvent(K_JOY1 + i, joy[0].button[i].b);
            joy_oldb[i] = joy[0].button[i].b;
    	}
	}
}

//==========================================================================
//
//	ShutdownJoystick
//
//==========================================================================

static void ShutdownJoystick(void)
{
	if (joystick_started)
    {
    	remove_joystick();
    }
}

//==========================================================================
//
//  IN_Init
//
//==========================================================================

void IN_Init(void)
{
	StartupKeyboard();
    StartupMouse();
	StartupJoystick();
}

//==========================================================================
//
//  IN_ReadInput
//
// 	Called by D_DoomLoop before processing each tic in a frame.
// 	Can call D_PostEvent.
// 	Asyncronous interrupt functions should maintain private ques that are
// read by the syncronous functions to be converted into events.
//
//==========================================================================

void IN_ReadInput()
{
    ReadKeyboard();
	ReadMouse();
	ReadJoystick();
}

//==========================================================================
//
//  IN_Shutdown
//
//==========================================================================

void IN_Shutdown(void)
{
    ShutdownJoystick();
    ShutdownMouse();
    ShutdownKeyboard();
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
