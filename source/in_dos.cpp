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
//**	Copyright (C) 1999-2006 Jānis Legzdiņš
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

class VDosInputDevice : public VInputDevice
{
public:
	VDosInputDevice();
	~VDosInputDevice();

	void ReadInput();

private:
	bool 			    keyboard_started;
	_go32_dpmi_seginfo	oldkeyinfo;
	_go32_dpmi_seginfo	newkeyinfo;
	bool 				nextkeyextended;
	int					pause_garbage;

	bool				mouse_started;
	int					old_mouse_x;
	int					old_mouse_y;

	bool				joystick_started;
	int					joy_oldx;
	int					joy_oldy;
	int					joy_oldb[MAX_JOYSTICK_BUTTONS];

	static const vuint8	scantokey[256];

	void StartupKeyboard();
	void ReadKeyboard();
	void ShutdownKeyboard();

	void StartupMouse();
	void ReadMouse();
	void ShutdownMouse();

	void StartupJoystick();
	void ReadJoystick();
	void ShutdownJoystick();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static vuint8				keyboardque[KBDQUESIZE];
static int 					kbdtail = 0;
static int					kbdhead = 0;

const vuint8				VDosInputDevice::scantokey[256] =
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

static VCvarI				m_filter("m_filter", "1", CVAR_Archive);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VDosInputDevice::VDosInputDevice
//
//==========================================================================

VDosInputDevice::VDosInputDevice()
: keyboard_started(false)
, mouse_started(false)
, old_mouse_x(0)
, old_mouse_y(0)
, joystick_started(false)
, joy_oldx(0)
, joy_oldy(0)
{
	guard(VDosInputDevice::VDosInputDevice);
	StartupMouse();
	StartupJoystick();
	StartupKeyboard();
	unguard;
}

//==========================================================================
//
//	VDosInputDevice::~VDosInputDevice
//
//==========================================================================

VDosInputDevice::~VDosInputDevice()
{
	guard(VDosInputDevice::~VDosInputDevice);
	ShutdownJoystick();
	ShutdownMouse();
	ShutdownKeyboard();
	unguard;
}

//==========================================================================
//
//  VDosInputDevice::ReadInput
//
//	Reads input from the input devices.
//
//==========================================================================

void VDosInputDevice::ReadInput()
{
	guard(VDosInputDevice::ReadInput);
	ReadKeyboard();
	ReadMouse();
	ReadJoystick();
	unguard;
}

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

static void KeyboardHandler()
{
	//	Get the scan code
	keyboardque[kbdhead & (KBDQUESIZE - 1)] = inportb(0x60);
	kbdhead++;

	//	Acknowledge the interrupt
	outportb(0x20, 0x20);
}
static void KeyboardHandlerEnd()
{}

//==========================================================================
//
//  VDosInputDevice::StartupKeyboard
//
//	Installs the keyboard handler.
//
//==========================================================================

void VDosInputDevice::StartupKeyboard()
{
	guard(VDosInputDevice::StartupKeyboard);
	nextkeyextended = false;
	pause_garbage = 0;

	newkeyinfo.pm_offset = (int)KeyboardHandler;
	newkeyinfo.pm_selector = _go32_my_cs();

	asm("cli");
	if (_go32_dpmi_get_protected_mode_interrupt_vector(9, &oldkeyinfo))
	{
		asm("sti");
		Sys_Error("Couldn't get old keyboard handler");
	}
	if (_go32_dpmi_allocate_iret_wrapper(&newkeyinfo))
	{
		asm("sti");
		Sys_Error("Couldn't alloc keyboard interrupt wrapper");
	}
	if (_go32_dpmi_set_protected_mode_interrupt_vector(9, &newkeyinfo))
	{
		asm("sti");
		Sys_Error("Couldn't set keyboard handler");
	}
	asm("sti");

	_go32_dpmi_lock_code((void*)KeyboardHandler,
		(long)KeyboardHandlerEnd - (long)KeyboardHandler);
	_go32_dpmi_lock_data(keyboardque, sizeof(keyboardque));
	_go32_dpmi_lock_data(&kbdhead, sizeof(kbdhead));

	keyboard_started = true;
	unguard;
}

//==========================================================================
//
//  VDosInputDevice::ReadKeyboard
//
//==========================================================================

void VDosInputDevice::ReadKeyboard()
{
	guard(VDosInputDevice::ReadKeyboard);
	unsigned char 	ch;

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
			GInput->KeyEvent(K_PAUSE, true);
			pause_garbage = 5;
			continue;
		}
		if (ch == 0xe0)
		{
			nextkeyextended = true;
			continue;
		}

		GInput->KeyEvent(scantokey[nextkeyextended ? (ch | 0x80) : (ch & 0x7f)],
			!(ch & 0x80));
		nextkeyextended = false;
	}
	unguard;
}

//==========================================================================
//
//  VDosInputDevice::ShutdownKeyboard
//
// 	Removes the keyboard handler.
//
//==========================================================================

void VDosInputDevice::ShutdownKeyboard()
{
	guard(VDosInputDevice::ShutdownKeyboard);
	if (keyboard_started)
	{
		keyboard_started = false;
		asm("cli");
		_go32_dpmi_set_protected_mode_interrupt_vector(9, &oldkeyinfo);
		_go32_dpmi_free_iret_wrapper(&newkeyinfo);
		asm("sti");
		_unlock_dpmi_data(keyboardque, sizeof(keyboardque));
		_unlock_dpmi_data(&kbdhead, sizeof(kbdhead));
	}
	unguard;
}

//**************************************************************************
//
//	MOUSE
//
//**************************************************************************

//==========================================================================
//
//  VDosInputDevice::StartupMouse
//
//	Initializes mouse
//
//==========================================================================

void VDosInputDevice::StartupMouse()
{
	guard(VDosInputDevice::StartupMouse);
	__dpmi_regs r;

	if (GArgs.CheckParm("-nomouse"))
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
	unguard;
}

//==========================================================================
//
//  VDosInputDevice::ReadMouse
//
// 	Reads mouse.
//
//==========================================================================

void VDosInputDevice::ReadMouse()
{
	guard(VDosInputDevice::ReadMouse);
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

		GInput->PostEvent(&event);
	}
	for (i = 0; i < 3; i++)
	{
		if ((buttons ^ lastbuttons) & (1 << i))
		{
			GInput->KeyEvent(K_MOUSE1 + i, buttons & (1 << i));
		}
	}
	lastbuttons = buttons;
	unguard;
}

//==========================================================================
//
//  VDosInputDevice::ShutdownMouse
//
//==========================================================================

void VDosInputDevice::ShutdownMouse()
{
	guard(VDosInputDevice::ShutdownMouse);
	if (mouse_started)
	{
		__dpmi_regs r;

		mouse_started = false;
		r.x.ax = 0;
		__dpmi_int(0x33, &r);
	}
	unguard;
}

//**************************************************************************
//
//	JOYSTICK
//
//**************************************************************************

//==========================================================================
//
//  VDosInputDevice::StartupJoystick
//
// 	Initializes joystick
//
//==========================================================================

void VDosInputDevice::StartupJoystick()
{
	guard(VDosInputDevice::StartupJoystick);
	if (GArgs.CheckParm("-nojoy"))
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
		GInput->ReadKey();

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
		GInput->ReadKey();

		if (calibrate_joystick(0))
		{
			Sys_Error("Error calibrating joystick!\n");
		}
	}

	memset(joy_oldb, 0, sizeof(joy_oldb));
	joystick_started = true;
	unguard;
}

//==========================================================================
//
//  VDosInputDevice::ReadJoystick
//
//==========================================================================

void VDosInputDevice::ReadJoystick()
{
	guard(VDosInputDevice::ReadJoystick);
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
		GInput->PostEvent(&event);
		joy_oldx = joy_x;
		joy_oldy = joy_y;
	}
	for (i = 0; i < joy[0].num_buttons; i++)
	{
		if (joy[0].button[i].b != joy_oldb[i])
		{
			GInput->KeyEvent(K_JOY1 + i, joy[0].button[i].b);
			joy_oldb[i] = joy[0].button[i].b;
		}
	}
	unguard;
}

//==========================================================================
//
//  VDosInputDevice::ShutdownJoystick
//
//==========================================================================

void VDosInputDevice::ShutdownJoystick()
{
	guard(VDosInputDevice::ShutdownJoystick);
	if (joystick_started)
	{
		joystick_started = false;
		remove_joystick();
	}
	unguard;
}

//**************************************************************************
//
//	MAIN
//
//**************************************************************************

//==========================================================================
//
//  VInputDevice::CreateDevice
//
//==========================================================================

VInputDevice* VInputDevice::CreateDevice()
{
	return new VDosInputDevice();
}
