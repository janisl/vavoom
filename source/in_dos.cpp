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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************
//**
//**	Input for DOS. Keyboard and mouse are DOS specific.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <allegro.h>
#include <go32.h>

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#ifndef MAX_JOYSTICK_BUTTONS
#define MAX_JOYSTICK_BUTTONS     16
#endif
#define KBDQUESIZE			32

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static boolean 			    keyboard_started = false;
static _go32_dpmi_seginfo	oldkeyinfo;
static _go32_dpmi_seginfo	newkeyinfo;
static boolean 				nextkeyextended;
static byte					keyboardque[KBDQUESIZE];
static int 					kbdtail = 0;
static int					kbdhead = 0;
static int					pause_garbage;
static byte					scantokey[256] =
{
	0, K_ESCAPE, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', K_BACKSPACE, K_TAB,
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', K_ENTER, K_LCTRL, 'a', 's',
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', K_LSHIFT, '\\', 'z', 'x', 'c', 'v',
	'b', 'n', 'm', ',', '.', '/', K_RSHIFT, K_PADMULTIPLE, K_LALT, ' ', K_CAPSLOCK, K_F1, K_F2, K_F3, K_F4, K_F5,
	K_F6, K_F7, K_F8, K_F9, K_F10, K_NUMLOCK, K_SCROLLLOCK, K_PAD7, K_PAD8, K_PAD9, K_PADMINUS, K_PAD4, K_PAD5, K_PAD6, K_PADPLUS, K_PAD1, 
	K_PAD2, K_PAD3, K_PAD0, K_PADDOT, 0, 0, 0, K_F11, K_F12, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, K_PADENTER, K_RCTRL, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, K_PADDIVIDE, 0, K_PRINTSCRN, K_RALT, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, K_HOME, K_UPARROW, K_PAGEUP, 0, K_LEFTARROW, 0, K_RIGHTARROW, 0, K_END,
	K_DOWNARROW, K_PAGEDOWN, K_INSERT, K_DELETE, 0, 0, 0, 0, 0, 0, 0, K_LWIN, K_RWIN, K_MENU, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
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
//
//	KEYBOARD
//
//**************************************************************************

//==========================================================================
//
//  KeyboardHandler
//
// 	Keyboard interrupt handler
//
//==========================================================================

static void KeyboardHandler(void)
{
	// Get the scan code

	keyboardque[kbdhead & (KBDQUESIZE - 1)] = inportb(0x60);
	kbdhead++;

	// acknowledge the interrupt

    outportb(0x20, 0x20);
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
	    ch = keyboardque[kbdtail & (KBDQUESIZE - 1)];
		kbdtail++;

		if (pause_garbage)
        {
        	pause_garbage--;
            continue;
        }

	    if (ch == 0xe1)
	    {
        	IN_KeyEvent(K_PAUSE, true);
            pause_garbage = 5;
            continue;
	    }
	    if (ch == 0xe0)
	    {
	      	nextkeyextended = true;
            continue;
	    }

		IN_KeyEvent(scantokey[nextkeyextended ? (ch | 0x80) : (ch & 0x7f)],
            !(ch & 0x80));
		nextkeyextended = false;

//		if (event.data1 == K_LCTRL || event.data1 == K_RCTRL)
//  			ctrl = (event.type == ev_keydown);
//	    if ((ctrl) && (ch & 0x7f == 0x2e)) // crtl-c
//    	{
//			asm ("movb $0x79, %%al
//		     call ___djgpp_hw_exception"
//		     : : :"%eax","%ebx","%ecx","%edx","%esi","%edi","memory");
//	    }
	}
}

//==========================================================================
//
//  StartupKeyboard
//
//	Installs the keyboard handler.
//
//==========================================================================

static void StartupKeyboard(void)
{
	LOCK_FUNCTION(KeyboardHandler);
    LOCK_DATA(keyboardque, sizeof(keyboardque));
    LOCK_VARIABLE(kbdhead);

    nextkeyextended = false;
	pause_garbage = 0;

    asm("cli");
    _go32_dpmi_get_protected_mode_interrupt_vector(9, &oldkeyinfo);
    newkeyinfo.pm_offset = (int)KeyboardHandler;
    newkeyinfo.pm_selector = _go32_my_cs();
    _go32_dpmi_allocate_iret_wrapper(&newkeyinfo);
    _go32_dpmi_set_protected_mode_interrupt_vector(9, &newkeyinfo);
    asm("sti");

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
		keyboard_started = false;
	    asm("cli");
	    _go32_dpmi_set_protected_mode_interrupt_vector(9, &oldkeyinfo);
	    _go32_dpmi_free_iret_wrapper(&newkeyinfo);
	    asm("sti");
	}
}

//**************************************************************************
//
//	MOUSE
//
//**************************************************************************

//==========================================================================
//
//  StartupMouse
//
//	Initializes mouse
//
//==========================================================================

static void StartupMouse(void)
{
    __dpmi_regs r;

    if (M_CheckParm("-nomouse"))
    {
    	return;
    }

    r.x.ax = 0;
    __dpmi_int(0x33, &r);

    if (r.x.ax == 0)
    	return;

   	mouse_started = true;

    //hide cursor
    r.x.ax = 2;
    __dpmi_int(0x33,&r);

    //reset mickey count
    r.x.ax = 0x0b;
    __dpmi_int(0x33,&r);
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
    __dpmi_regs r;
    event_t 	event;
    int 		xmickeys;
    int			ymickeys;
    int			mouse_x;
    int			mouse_y;
    int			buttons;
    static int 	lastbuttons = 0;

    if (!mouse_started)
		return;

    r.x.ax = 0x0b;
    __dpmi_int(0x33, &r);
    xmickeys = (signed short)r.x.cx;
    ymickeys = (signed short)r.x.dx;
    r.x.ax=0x03;
    __dpmi_int(0x33, &r);
    buttons = r.x.bx;

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
//  ShutdownMouse
//
//==========================================================================

static void ShutdownMouse(void)
{
	if (mouse_started)
    {
    	__dpmi_regs r;

    	mouse_started = false;
	    r.x.ax = 0;
	    __dpmi_int(0x33, &r);
	}
}

//**************************************************************************
//
//	JOYSTICK
//
//**************************************************************************

//==========================================================================
//
//  StartupJoystick
//
// 	Initializes joystick
//
//==========================================================================

static void StartupJoystick(void)
{
	if (M_CheckParm("-nojoy"))
    {
    	return;
	}

	// Initialize the joystick driver
	if (install_joystick(JOY_TYPE_AUTODETECT))
	{
		Sys_Error("Error initialising joystick\n%s\n", allegro_error);
	}

	// make sure that we really do have a joystick
	if (!num_joysticks)
	{
		return;
	}

	if (joy[0].flags & JOYFLAG_CALIBRATE)
    {
		remove_joystick();

		printf("CENTER the joystick and press a key:\n");
		IN_ReadKey();

		// Initialize the joystick driver
		if (install_joystick(JOY_TYPE_AUTODETECT))
		{
			Sys_Error("Error initialising joystick\n%s\n", allegro_error);
		}
    }

	//	Calibrate joystick num_joysticks
	while (joy[0].flags & JOYFLAG_CALIBRATE)
	{
	   	printf("%s and press a key:\n", calibrate_joystick_name(0));
	    IN_ReadKey();

		if (calibrate_joystick(0))
		{
			Sys_Error("Error calibrating joystick!\n");
		}
	}

	memset(joy_oldb, 0, sizeof(joy_oldb));
    joystick_started = true;
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
//  ShutdownJoystick
//
//==========================================================================

static void ShutdownJoystick(void)
{
	if (joystick_started)
    {
    	joystick_started = false;
    	remove_joystick();
    }
}

//**************************************************************************
//
//	MAIN
//
//**************************************************************************

//==========================================================================
//
//  IN_Init
//
//==========================================================================

void IN_Init(void)
{
    StartupMouse();
	StartupJoystick();
	StartupKeyboard();
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

void IN_ReadInput(void)
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
