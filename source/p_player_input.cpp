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
//**	KEY BUTTONS
//**
//**	Continuous button event tracking is complicated by the fact that two
//**  different input sources (say, mouse button 1 and the control key) can
//**  both press the same button, but the button should only be released
//**  when both of the pressing key have been released.
//**
//**	When a key event issues a button command (+forward, +attack, etc),
//**  it appends its key number as a parameter to the command so it can be
//**  matched up with the release.
//**
//**	state bit 0 is the current state of the key
//**	state bit 1 is edge triggered on the up to down transition
//**	state bit 2 is edge triggered on the down to up transition
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define BUTTON(name) \
static TKButton		Key ## name; \
static TCmdKeyDown	name ## Down_f("+" #name, Key ## name); \
static TCmdKeyUp	name ## Up_f("-" #name, Key ## name);

// TYPES -------------------------------------------------------------------

class TKButton
{
public:
	int		down[2];		// key nums holding it down
	int		state;			// low bit is down state

	void KeyDown(const char* c);
	void KeyUp(const char* c);
	float KeyState();
};

class TCmdKeyDown : public VCommand
{
public:
	TCmdKeyDown(const char *AName, TKButton &AKey) : VCommand(AName), Key(AKey) { }
	void Run();

	TKButton	&Key;
};

class TCmdKeyUp : public VCommand
{
public:
	TCmdKeyUp(const char *AName, TKButton &AKey) : VCommand(AName), Key(AKey)	{ }
	void Run();

	TKButton	&Key;
};

enum
{
	INPUT_OLDBUTTONS,
	INPUT_BUTTONS,
	INPUT_PITCH,
	INPUT_YAW,
	INPUT_ROLL,
	INPUT_FORWARDMOVE,
	INPUT_SIDEMOVE,
	INPUT_UPMOVE,
	MODINPUT_OLDBUTTONS,
	MODINPUT_BUTTONS,
	MODINPUT_PITCH,
	MODINPUT_YAW,
	MODINPUT_ROLL,
	MODINPUT_FORWARDMOVE,
	MODINPUT_SIDEMOVE,
	MODINPUT_UPMOVE
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// mouse values are used once
static float	mousex;
static float	mousey;
// joystick values are repeated 
static int		joyxmove;
static int		joyymove;

static int		impulse_cmd;

static VCvarI	allways_run("allways_run", "1", CVAR_Archive);
static VCvarI	artiskip("artiskip", "1", CVAR_Archive);	// whether shift-enter skips an artifact

static VCvarF	cl_forwardspeed("cl_forwardspeed", "200", CVAR_Archive);
static VCvarF	cl_backspeed("cl_backspeed", "200", CVAR_Archive);
static VCvarF	cl_sidespeed("cl_sidespeed", "200", CVAR_Archive);
static VCvarF	cl_flyspeed("cl_flyspeed", "80", CVAR_Archive);

static VCvarF	cl_movespeedkey("cl_movespeedkey", "2.0", CVAR_Archive);

static VCvarF	cl_yawspeed("cl_yawspeed", "140", CVAR_Archive);
static VCvarF	cl_pitchspeed("cl_pitchspeed", "150", CVAR_Archive);
static VCvarF	cl_pitchdriftspeed("cl_pitchdriftspeed", "270", CVAR_Archive);

static VCvarF	cl_anglespeedkey("cl_anglespeedkey", "1.5", CVAR_Archive);

static VCvarF	cl_deathroll("cl_deathroll", "75", CVAR_Archive);
static VCvarF	cl_deathrollspeed("cl_deathrollspeed", "80", CVAR_Archive);

static VCvarF	mouse_x_sensitivity("mouse_x_sensitivity", "5.0", CVAR_Archive);
static VCvarF	mouse_y_sensitivity("mouse_y_sensitivity", "5.0", CVAR_Archive);
static VCvarI	mouse_look("mouse_look", "1", CVAR_Archive);
static VCvarI	invert_mouse("invert_mouse", "0", CVAR_Archive);
static VCvarI	lookstrafe("lookstrafe", "0", CVAR_Archive);
static VCvarI	lookspring("lookspring", "0", CVAR_Archive);

static VCvarF	m_yaw("m_yaw", "0.022", CVAR_Archive);
static VCvarF	m_pitch("m_pitch", "0.022", CVAR_Archive);
static VCvarF	m_forward("m_forward", "1.0", CVAR_Archive);
static VCvarF	m_side("m_side", "0.8", CVAR_Archive);

static VCvarF	joy_yaw("joy_yaw", "140", CVAR_Archive);

BUTTON(Forward)
BUTTON(Backward)
BUTTON(Left)
BUTTON(Right)
BUTTON(LookUp)
BUTTON(LookDown)
BUTTON(LookCentre)
BUTTON(MoveLeft)
BUTTON(MoveRight)
BUTTON(FlyUp)
BUTTON(FlyDown)
BUTTON(FlyCentre)
BUTTON(Attack)
BUTTON(Use)
BUTTON(Jump)
BUTTON(AltAttack)
BUTTON(Button5)
BUTTON(Button6)
BUTTON(Button7)
BUTTON(Button8)
BUTTON(Speed)
BUTTON(Strafe)
BUTTON(MouseLook)

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TKButton::KeyDown
//
//==========================================================================

void TKButton::KeyDown(const char* c)
{
	guard(TKButton::KeyDown);
	int		k;

	if (c[0])
	{
		k = atoi(c);
	}
	else
	{
		k = -1;		// typed manually at the console for continuous down
	}

	if (k == down[0] || k == down[1])
	{
		return;		// repeating key
	}
	
	if (!down[0])
	{
		down[0] = k;
	}
	else if (!down[1])
	{
		down[1] = k;
	}
	else
	{
		GCon->Log(NAME_Dev, "Three keys down for a button!");
		return;
	}
	
	if (state & 1)
	{
		return;		// still down
	}
	state |= 1 + 2;	// down + impulse down
	unguard;
}

//==========================================================================
//
//	TKButton::KeyUp
//
//==========================================================================

void TKButton::KeyUp(const char* c)
{
	guard(TKButton::KeyUp);
	int		k;
	
	if (c[0])
	{
		k = atoi(c);
	}
	else
	{ // typed manually at the console, assume for unsticking, so clear all
		down[0] = down[1] = 0;
		state = 4;	// impulse up
		return;
	}

	if (down[0] == k)
	{
		down[0] = 0;
	}
	else if (down[1] == k)
	{
		down[1] = 0;
	}
	else
	{
		return;		// key up without coresponding down (menu pass through)
	}
	if (down[0] || down[1])
	{
		return;		// some other key is still holding it down
	}

	if (!(state & 1))
	{
		return;		// still up (this should not happen)
	}
	state &= ~1;		// now up
	state |= 4; 		// impulse up
	unguard;
}

//==========================================================================
//
//	TKButton::KeyState
//
//	Returns 0.25 if a key was pressed and released during the frame,
//	0.5 if it was pressed and held
//	0 if held then released, and
//	1.0 if held for the entire time
//
//==========================================================================

float TKButton::KeyState()
{
	guard(TKButton::KeyState);
	static const float newVal[8] =
	{
		0.0f,	// up the entire frame
		1.0f,	// held the entire frame
		0.0f,	// Sys_Error();
		0.5f,	// pressed and held this frame
		0.0f,	// released this frame
		0.0f,	// Sys_Error();
		0.25f,	// pressed and released this frame
		0.75f	// released and re-pressed this frame
	};

	float val = newVal[state & 7];
	state &= 1;		// clear impulses
	
	return val;
	unguard;
}

//==========================================================================
//
//	TCmdKeyDown::Run
//
//==========================================================================

void TCmdKeyDown::Run()
{
	guard(TCmdKeyDown::Run);
	Key.KeyDown(Args.Num() > 1 ? *Args[1] : "");
	unguard;
}

//==========================================================================
//
//	TCmdKeyUp::Run
//
//==========================================================================

void TCmdKeyUp::Run()
{
	guard(TCmdKeyUp::Run);
	Key.KeyUp(Args.Num() > 1 ? *Args[1] : "");
	unguard;
}

//==========================================================================
//
//	COMMAND Impulse
//
//==========================================================================

COMMAND(Impulse)
{
	guard(COMMAND Impulse);
	if (Args.Num() < 2)
	{
		return;
	}
	impulse_cmd = atoi(*Args[1]);
	unguard;
}

//==========================================================================
//
//	COMMAND ToggleAlwaysRun
//
//==========================================================================

COMMAND(ToggleAlwaysRun)
{
	guard(COMMAND ToggleAlwaysRun);
	allways_run = !allways_run;
#ifdef CLIENT
	if (cl)
	{
		cl->Printf(allways_run ? "Always run on" : "Always run off");
	}
	else
#endif
	{
		GCon->Log(allways_run ? "Always run on" : "Always run off");
	}
	unguard;
}

//==========================================================================
//
//	COMMAND Use
//
//==========================================================================

COMMAND(Use)
{
	guard(COMMAND Use);
	if (Args.Num() < 1)
	{
		return;
	}
#ifdef CLIENT
	cl->eventUseInventory(*Args[1]);
#endif
	unguard;
}

//==========================================================================
//
//	VBasePlayer::StartPitchDrift
//
//==========================================================================

void VBasePlayer::StartPitchDrift()
{
	PlayerFlags |= PF_Centreing;
}

//==========================================================================
//
//	VBasePlayer::StopPitchDrift
//
//==========================================================================

void VBasePlayer::StopPitchDrift()
{
	PlayerFlags &= ~PF_Centreing;
}

//==========================================================================
//
//	VBasePlayer::AdjustAngles
//
//==========================================================================

void VBasePlayer::AdjustAngles()
{
	guard(VBasePlayer::AdjustAngles);
	float speed;

	if (KeySpeed.state & 1)
	{
		speed = host_frametime * cl_anglespeedkey;
	}
	else
	{
		speed = host_frametime;
	}

	if ((KeyMouseLook.state & 4) && lookspring)
	{
		StartPitchDrift();
	}
	KeyMouseLook.state &= 1;

	//	YAW
	if (!(KeyStrafe.state & 1))
	{
		ViewAngles.yaw -= KeyRight.KeyState() * cl_yawspeed * speed;
		ViewAngles.yaw += KeyLeft.KeyState() * cl_yawspeed * speed;
		if (joyxmove > 0)
		{
			ViewAngles.yaw -= joy_yaw * speed;
		}
		if (joyxmove < 0)
		{
			ViewAngles.yaw += joy_yaw * speed;
		}
	}
	if (!(KeyStrafe.state & 1) &&
		(!lookstrafe || (!mouse_look && !(KeyMouseLook.state & 1))))
	{
		ViewAngles.yaw -= mousex * m_yaw;
	}
	ViewAngles.yaw = AngleMod(ViewAngles.yaw);

	//	PITCH
	float up = KeyLookUp.KeyState();
	float down = KeyLookDown.KeyState();
	ViewAngles.pitch -= cl_pitchspeed * up * speed;
	ViewAngles.pitch += cl_pitchspeed * down * speed;
	if (up || down || (KeyMouseLook.state & 1))
	{
		StopPitchDrift();
	}
	if ((mouse_look || (KeyMouseLook.state & 1)) && !(KeyStrafe.state & 1))
	{
		ViewAngles.pitch -= mousey * m_pitch;
	}

	//	Centre look
	if ((KeyLookCentre.state & 1) || (KeyFlyCentre.state & 1))
	{
		StartPitchDrift();
	}
	if (PlayerFlags & PF_Centreing)
	{
		float adelta = cl_pitchdriftspeed * host_frametime;
		if (fabs(ViewAngles.pitch) < adelta)
		{
			ViewAngles.pitch = 0;
			PlayerFlags &= ~PF_Centreing;
		}
		else
		{
			if (ViewAngles.pitch > 0.0)
			{
				ViewAngles.pitch -= adelta;
			}
			else if (ViewAngles.pitch < 0.0)
			{
				ViewAngles.pitch += adelta;
			}
		}
	}

	//	ROLL
	if (Health <= 0)
	{
		if (ViewAngles.roll >= 0 && ViewAngles.roll < cl_deathroll)
		{
			ViewAngles.roll += cl_deathrollspeed * host_frametime;
		}
		if (ViewAngles.roll < 0 && ViewAngles.roll > -cl_deathroll)
		{
			ViewAngles.roll -= cl_deathrollspeed * host_frametime;
		}
	}
	else
	{
		ViewAngles.roll = 0.0;
	}

	//	Check angles
	if (ViewAngles.pitch > 80.0)
	{
		ViewAngles.pitch = 80.0;
	}
	if (ViewAngles.pitch < -70.0)
	{
		ViewAngles.pitch = -70.0;
	}

	if (ViewAngles.roll > 80.0)
	{
		ViewAngles.roll = 80.0;
	}
	if (ViewAngles.roll < -80.0)
	{
		ViewAngles.roll = -80.0;
	}

	if (Level->LevelInfoFlags & VLevelInfo::LIF_NoFreelook)
	{
		ViewAngles.pitch = 0;
	}
	unguard;
}

//==========================================================================
//
//	VBasePlayer::HandleInput
//
//	Creates movement commands from all of the available inputs.
//
//==========================================================================

void VBasePlayer::HandleInput()
{
	guard(VBasePlayer::HandleInput);
	float		forward;
	float		side;
	float		flyheight;

	AdjustAngles();

	forward = side = flyheight = 0;

	// let movement keys cancel each other out
	if (KeyStrafe.state & 1)
	{
		side += KeyRight.KeyState() * cl_sidespeed;
		side -= KeyLeft.KeyState() * cl_sidespeed;
		if (joyxmove > 0)
		{
			side += cl_sidespeed;
		}
		if (joyxmove < 0)
		{
			side -= cl_sidespeed;
		}
	}

	forward += KeyForward.KeyState() * cl_forwardspeed;
	forward -= KeyBackward.KeyState() * cl_backspeed;

	side += KeyMoveRight.KeyState() * cl_sidespeed;
	side -= KeyMoveLeft.KeyState() * cl_sidespeed;

	if (joyymove < 0) 
	{
		forward += cl_forwardspeed;
	}
	if (joyymove > 0) 
	{
		forward -= cl_backspeed;
	}

	// Fly up/down/drop keys
	flyheight += KeyFlyUp.KeyState() * cl_flyspeed; // note that the actual flyheight will be twice this
	flyheight -= KeyFlyDown.KeyState() * cl_flyspeed;

	if ((!mouse_look && !(KeyMouseLook.state & 1)) || (KeyStrafe.state & 1))
	{
		forward += m_forward * mousey;
	}

	if ((KeyStrafe.state & 1) ||
		(lookstrafe && (mouse_look || (KeyMouseLook.state & 1))))
	{
		side += m_side * mousex;
	}

	forward = MID(forward, -cl_backspeed, cl_forwardspeed);
	side = MID(side, -cl_sidespeed, cl_sidespeed);

	if (allways_run || (KeySpeed.state & 1))
	{
		forward *= cl_movespeedkey;
		side *= cl_movespeedkey;
		flyheight *= cl_movespeedkey;
	}

	flyheight = MID(flyheight, -127, 127);
	if (KeyFlyCentre.KeyState())
	{
		flyheight = TOCENTRE;
	}

	//
	//	BUTTONS
	//

	Buttons = 0;

	// Fire buttons
	if (KeyAttack.KeyState())
	{
		Buttons |= BT_ATTACK;
	}

	// Use buttons
	if (KeyUse.KeyState())
	{
		Buttons |= BT_USE;
	}

	// Jumping
	if (KeyJump.KeyState())
	{
		Buttons |= BT_JUMP;
	}

	if (KeyAltAttack.KeyState())
	{
		Buttons |= BT_ALT_ATTACK;
	}

	if (KeyButton5.KeyState())
	{
		Buttons |= 0x10;
	}

	if (KeyButton6.KeyState())
	{
		Buttons |= 0x20;
	}

	if (KeyButton7.KeyState())
	{
		Buttons |= 0x40;
	}

	if (KeyButton8.KeyState())
	{
		Buttons |= 0x80;
	}

	//
	//	IMPULSE
	//
	if (impulse_cmd)
	{
		eventServerImpulse(impulse_cmd);
		impulse_cmd = 0;
	}

	ClientForwardMove = forward;
	ClientSideMove = side;
	FlyMove = flyheight;

	mousex = 0;
	mousey = 0;
	unguard;
}

//==========================================================================
//
//	VBasePlayer::Responder
//
//	Get info needed to make movement commands for the players.
//
//==========================================================================

bool VBasePlayer::Responder(event_t* ev)
{
	guard(VBasePlayer::Responder);
	switch (ev->type) 
	{
	case ev_mouse:
		mousex = ev->data2 * mouse_x_sensitivity;
		mousey = ev->data3 * mouse_y_sensitivity;
		if (invert_mouse)
		{
			mousey = -mousey;
		}
		return true;    // eat events

	case ev_joystick:
		joyxmove = ev->data2;
		joyymove = ev->data3;
		return true;    // eat events

	default:
		break;
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VBasePlayer::ClearInput
//
//==========================================================================

void VBasePlayer::ClearInput()
{
	guard(VBasePlayer::ClearInput);
	// clear cmd building stuff
	joyxmove = joyymove = 0; 
	mousex = mousey = 0;
	impulse_cmd = 0;
	unguard;
}

//==========================================================================
//
//	VBasePlayer::AcsGetInput
//
//==========================================================================

int VBasePlayer::AcsGetInput(int InputType)
{
	guard(VBasePlayer::AcsGetInput);
	int Btn;
	int Ret = 0;
	switch (InputType)
	{
	case INPUT_OLDBUTTONS:
	case MODINPUT_OLDBUTTONS:
	case INPUT_BUTTONS:
	case MODINPUT_BUTTONS:
		if (InputType == INPUT_OLDBUTTONS ||
			InputType == MODINPUT_OLDBUTTONS)
		{
			Btn = OldButtons;
		}
		else
		{
			Btn = Buttons;
		}
		//	Convert buttons to what ACS expects.
		if (Btn & BT_ATTACK)
		{
			Ret |= 1;
		}
		if (Btn & BT_USE)
		{
			Ret |= 2;
		}
		if (Btn & BT_JUMP)
		{
			Ret |= 4;
		}
		if (Btn & BT_ALT_ATTACK)
		{
			Ret |= 32;
		}
		return Ret;

	case INPUT_PITCH:
		return (int)(AngleMod(OldViewAngles.pitch) * 0x10000 / 360);

	case MODINPUT_PITCH:
		return (int)(AngleMod(ViewAngles.pitch) * 0x10000 / 360);

	case INPUT_YAW:
		return (int)(AngleMod(OldViewAngles.yaw) * 0x10000 / 360);

	case MODINPUT_YAW:
		return (int)(AngleMod(ViewAngles.yaw) * 0x10000 / 360);

	case INPUT_ROLL:
		return (int)(AngleMod(OldViewAngles.roll) * 0x10000 / 360);

	case MODINPUT_ROLL:
		return (int)(AngleMod(ViewAngles.roll) * 0x10000 / 360);

	case INPUT_FORWARDMOVE:
		return (int)(ClientForwardMove * 0x32 / 400);

	case MODINPUT_FORWARDMOVE:
		return (int)(ForwardMove * 0x32 / 400);

	case INPUT_SIDEMOVE:
		return (int)(ClientSideMove * 0x32 / 400);

	case MODINPUT_SIDEMOVE:
		return (int)(SideMove * 0x32 / 400);

	case INPUT_UPMOVE:
	case MODINPUT_UPMOVE:
		return (int)(FlyMove * 3 * 256 / 80);
	}
	return 0;
	unguard;
}
