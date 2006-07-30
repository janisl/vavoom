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
//**	Input modue for Allegro library. Used only in Linux (in Windows I
//**  don't use Allegro, in DOS keyboard and mouse are handled directly).
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <allegro.h>
#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define KBDQUESIZE			32

// TYPES -------------------------------------------------------------------

class VAllegroInputDevice : public VInputDevice
{
public:
	VAllegroInputDevice();
	~VAllegroInputDevice();

	void ReadInput();

private:
	bool				keyboard_started;

	bool				mouse_started;
	int					old_mouse_x;
	int					old_mouse_y;

	bool				joystick_started;
	int					joy_oldx;
	int					joy_oldy;
	int					joy_oldb[MAX_JOYSTICK_BUTTONS];

	static const vuint8	scantokey[KEY_MAX];

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

const vuint8				VAllegroInputDevice::scantokey[KEY_MAX] =
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
	K_PADDOT, K_PADENTER, K_PRINTSCRN, K_PAUSE,
	K_ABNT_C1, K_YEN, K_KANA, K_CONVERT, K_NOCONVERT,
	K_AT, K_CIRCUMFLEX, K_COLON2, K_KANJI,
	K_LSHIFT, K_RSHIFT, K_LCTRL, K_RCTRL, K_LALT, K_RALT,
	K_LWIN, K_RWIN, K_MENU, K_SCROLLLOCK, K_NUMLOCK, K_CAPSLOCK
};

static VCvarI				m_filter("m_filter", "1", CVAR_Archive);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VAllegroInputDevice::VAllegroInputDevice
//
//==========================================================================

VAllegroInputDevice::VAllegroInputDevice()
: keyboard_started(false)
, mouse_started(false)
, old_mouse_x(0)
, old_mouse_y(0)
, joystick_started(false)
, joy_oldx(0)
, joy_oldy(0)
{
	guard(VAllegroInputDevice::VAllegroInputDevice);
	StartupKeyboard();
	StartupMouse();
	StartupJoystick();
	unguard;
}

//==========================================================================
//
//	VAllegroInputDevice::~VAllegroInputDevice
//
//==========================================================================

VAllegroInputDevice::~VAllegroInputDevice()
{
	guard(VAllegroInputDevice::~VAllegroInputDevice);
	ShutdownJoystick();
	ShutdownMouse();
	ShutdownKeyboard();
	unguard;
}

//==========================================================================
//
//  VAllegroInputDevice::ReadInput
//
//	Reads input from the input devices.
//
//==========================================================================

void VAllegroInputDevice::ReadInput()
{
	guard(VAllegroInputDevice::ReadInput);
	ReadKeyboard();
	ReadMouse();
	ReadJoystick();
	unguard;
}

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
	keyboardque[kbdhead & (KBDQUESIZE - 1)] = key;
	kbdhead++;
}
END_OF_FUNCTION(KeyboardHandler)

//==========================================================================
//
//	VAllegroInputDevice::StartupKeyboard
//
//	Installs the keyboard handler.
//
//==========================================================================

void VAllegroInputDevice::StartupKeyboard()
{
	guard(VAllegroInputDevice::StartupKeyboard);
	LOCK_FUNCTION((void*)KeyboardHandler);
	LOCK_DATA(keyboardque, sizeof(keyboardque));
	LOCK_VARIABLE(kbdhead);

	if (install_keyboard())
	{
		Sys_Error("Failed to initialize keyboard");
	}
	keyboard_lowlevel_callback = KeyboardHandler;
	keyboard_started = true;
	unguard;
}

//==========================================================================
//
//  VAllegroInputDevice::ReadKeyboard
//
//==========================================================================

void VAllegroInputDevice::ReadKeyboard()
{
	guard(VAllegroInputDevice::ReadKeyboard);
	unsigned char 	ch;

	if (!keyboard_started)
		return;

	while (kbdtail < kbdhead)
	{
		ch = keyboardque[kbdtail & (KBDQUESIZE - 1)];
		kbdtail++;

		GInput->KeyEvent(scantokey[ch & 0x7f], !(ch & 0x80));
	}
	unguard;
}

//==========================================================================
//
//  VAllegroInputDevice::ShutdownKeyboard
//
// 	Removes the keyboard handler.
//
//==========================================================================

void VAllegroInputDevice::ShutdownKeyboard()
{
	guard(VAllegroInputDevice::ShutdownKeyboard);
	if (keyboard_started)
	{
		remove_keyboard();
	}
	unguard;
}

//**************************************************************************
//**
//**	MOUSE
//**
//**************************************************************************

//==========================================================================
//
//	VAllegroInputDevice::StartupMouse
//
//	Initialises mouse
//
//==========================================================================

void VAllegroInputDevice::StartupMouse()
{
	guard(VAllegroInputDevice::StartupMouse);
	int		buts;

	if (GArgs.CheckParm("-nomouse"))
		return;

	buts = install_mouse();
	if (buts == -1)
	{
		return;
	}
	mouse_started = true;
	unguard;
}

//==========================================================================
//
//	VAllegroInputDevice::ReadMouse
//
// 	Reads mouse.
//
//==========================================================================

void VAllegroInputDevice::ReadMouse()
{
	guard(VAllegroInputDevice::ReadMouse);
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
//	VAllegroInputDevice::ShutdownMouse
//
//==========================================================================

void VAllegroInputDevice::ShutdownMouse()
{
	guard(VAllegroInputDevice::ShutdownMouse);
	if (!mouse_started)
		return;
	remove_mouse();
	unguard;
}

//**************************************************************************
//**
//**	JOYSTICK
//**
//**************************************************************************

//==========================================================================
//
//	VAllegroInputDevice::StartupJoystick
//
//	Initialises joystick
//
//==========================================================================

void VAllegroInputDevice::StartupJoystick()
{
	guard(VAllegroInputDevice::StartupJoystick);
	if (GArgs.CheckParm("-nojoy"))
		return;

	//	Detect the joystick type
	if (install_joystick(JOY_TYPE_AUTODETECT))
	{
		Sys_Error("Error initialising joystick\n%s\n", allegro_error);
	}

	//	Make sure that we really do have a joystick
	if (!num_joysticks)
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
		GInput->ReadKey();

		// Initialize the joystick driver
		if (install_joystick(JOY_TYPE_AUTODETECT))
		{
			Sys_Error("Error initialising joystick\n%s\n", allegro_error);
		}
	}

	//	Calibrate joystick
	while (joy[0].flags & JOYFLAG_CALIBRATE)
	{
		printf("%s and press a key:\n", calibrate_joystick_name(0));
		GInput->ReadKey();

		if (calibrate_joystick(0))
		{
			Sys_Error("Error calibrating joystick!\n");
		}
	}

	joystick_started = true;
	memset(joy_oldb, 0, sizeof(joy_oldb));
	unguard;
}

//==========================================================================
//
//	VAllegroInputDevice::ReadJoystick
//
//==========================================================================

void VAllegroInputDevice::ReadJoystick()
{
	guard(VAllegroInputDevice::ReadJoystick);
	int			i;
	event_t		event;

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
//	VAllegroInputDevice::ShutdownJoystick
//
//==========================================================================

void VAllegroInputDevice::ShutdownJoystick()
{
	guard(VAllegroInputDevice::ShutdownJoystick);
	if (joystick_started)
	{
		remove_joystick();
	}
	unguard;
}

//==========================================================================
//
//  VInputDevice::CreateDevice
//
//==========================================================================

VInputDevice* VInputDevice::CreateDevice()
{
	return new VAllegroInputDevice();
}
