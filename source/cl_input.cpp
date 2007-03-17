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

#define TOCENTRE		-128

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
static VCvarF	cl_sidespeed("cl_sidespeed", "350", CVAR_Archive);
static VCvarF	cl_flyspeed("cl_flyspeed", "80", CVAR_Archive);

static VCvarF	cl_movespeedkey("cl_movespeedkey", "2.0", CVAR_Archive);

static VCvarF	cl_yawspeed("cl_yawspeed", "140", CVAR_Archive);
static VCvarF	cl_pitchspeed("cl_pitchspeed", "150", CVAR_Archive);
static VCvarF	cl_pitchdriftspeed("cl_pitchdriftspeed", "270", CVAR_Archive);

static VCvarF	cl_anglespeedkey("cl_anglespeedkey", "1.5", CVAR_Archive);

static VCvarF	cl_deathroll("cl_deathroll", "75", CVAR_Archive);
static VCvarF	cl_deathrollspeed("cl_deathrollspeed", "80", CVAR_Archive);

static VCvarF	mouse_x_sensitivity("mouse_x_sensitivity", "3.0", CVAR_Archive);
static VCvarF	mouse_y_sensitivity("mouse_y_sensitivity", "3.0", CVAR_Archive);
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
BUTTON(Button4)
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
		k = atoi(c);
	else
		k = -1;		// typed manually at the console for continuous down

	if (k == down[0] || k == down[1])
		return;		// repeating key
	
	if (!down[0])
		down[0] = k;
	else if (!down[1])
		down[1] = k;
	else
	{
		GCon->Log(NAME_Dev, "Three keys down for a button!");
		return;
	}
	
	if (state & 1)
		return;		// still down
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
		k = atoi(c);
	else
	{ // typed manually at the console, assume for unsticking, so clear all
		down[0] = down[1] = 0;
		state = 4;	// impulse up
		return;
	}

	if (down[0] == k)
		down[0] = 0;
	else if (down[1] == k)
		down[1] = 0;
	else
		return;		// key up without coresponding down (menu pass through)
	if (down[0] || down[1])
		return;		// some other key is still holding it down

	if (!(state & 1))
		return;		// still up (this should not happen)
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
		return;
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
	unguard;
}

//==========================================================================
//
//	COMMAND CentreTilt
//
//==========================================================================

void V_StartPitchDrift()
{
	cl->PlayerFlags |= VBasePlayer::PF_Centreing;
}
void V_StopPitchDrift()
{
	cl->PlayerFlags &= ~VBasePlayer::PF_Centreing;
}

//==========================================================================
//
//	AdjustAngles
//
//==========================================================================

static void AdjustAngles()
{
	guard(AdjustAngles);
	float speed;

	if (KeySpeed.state & 1)
		speed = host_frametime * cl_anglespeedkey;
	else
		speed = host_frametime;


	if ((KeyMouseLook.state & 4) && lookspring)
	{
		V_StartPitchDrift();
	}
	KeyMouseLook.state &= 1;

	//	YAW
	if (!(KeyStrafe.state & 1))
	{ 
		cl->ViewAngles.yaw -= KeyRight.KeyState() * cl_yawspeed * speed;
		cl->ViewAngles.yaw += KeyLeft.KeyState() * cl_yawspeed * speed;
		if (joyxmove > 0)
			cl->ViewAngles.yaw -= joy_yaw * speed;
		if (joyxmove < 0)
			cl->ViewAngles.yaw += joy_yaw * speed;
	}
	if (!(KeyStrafe.state & 1) &&
		(!lookstrafe || (!mouse_look && !(KeyMouseLook.state & 1))))
	{
		cl->ViewAngles.yaw -= mousex * m_yaw;
	}
	cl->ViewAngles.yaw = AngleMod(cl->ViewAngles.yaw);

	//	PITCH
	float up = KeyLookUp.KeyState();
	float down = KeyLookDown.KeyState();
	cl->ViewAngles.pitch -= cl_pitchspeed * up * speed;
	cl->ViewAngles.pitch += cl_pitchspeed * down * speed;
	if (up || down || (KeyMouseLook.state & 1))
	{
		V_StopPitchDrift();
	}
	if ((mouse_look || (KeyMouseLook.state & 1)) && !(KeyStrafe.state & 1))
	{
		cl->ViewAngles.pitch -= mousey * m_pitch;
	}

	//	Centre look
	if ((KeyLookCentre.state & 1) || (KeyFlyCentre.state & 1))
	{
		V_StartPitchDrift();
	}
	if (cl->PlayerFlags & VBasePlayer::PF_Centreing)
	{
		float adelta = cl_pitchdriftspeed * host_frametime;
		if (fabs(cl->ViewAngles.pitch) < adelta)
		{
			cl->ViewAngles.pitch = 0;
			cl->PlayerFlags &= ~VBasePlayer::PF_Centreing;
		}
		else
		{
			if (cl->ViewAngles.pitch > 0.0)
			{
				cl->ViewAngles.pitch -= adelta;
			}
			else if (cl->ViewAngles.pitch < 0.0)
			{
				cl->ViewAngles.pitch += adelta;
			}
		}
	}

	//	ROLL
	if (cl->Health <= 0)
 	{
 		if (cl->ViewAngles.roll >= 0 && cl->ViewAngles.roll < cl_deathroll)
		{
			cl->ViewAngles.roll += cl_deathrollspeed * host_frametime;
		}
 		if (cl->ViewAngles.roll < 0 && cl->ViewAngles.roll > -cl_deathroll)
		{
			cl->ViewAngles.roll -= cl_deathrollspeed * host_frametime;
		}
	}
	else
	{
		cl->ViewAngles.roll = 0.0;
	}

	//	Check angles
	if (cl->ViewAngles.pitch > 80.0)
	{
		cl->ViewAngles.pitch = 80.0;
	}
	if (cl->ViewAngles.pitch < -70.0)
	{
		cl->ViewAngles.pitch = -70.0;
	}

	if (cl->ViewAngles.roll > 80.0)
	{
		cl->ViewAngles.roll = 80.0;
	}
	if (cl->ViewAngles.roll < -80.0)
	{
		cl->ViewAngles.roll = -80.0;
	}
	unguard;
}

//==========================================================================
//
//	BuildTiccmd
//
//	Builds a ticcmd from all of the available inputs or reads it from the
// demo buffer. If recording a demo, write it out
//
//==========================================================================

static void BuildTiccmd(ticcmd_t* cmd)
{
	guard(BuildTiccmd);
	float		forward;
	float		side;
	float		flyheight;

	memset(cmd, 0, sizeof(*cmd));

	forward = side = flyheight = 0;
    
	// let movement keys cancel each other out
	if (KeyStrafe.state & 1)
	{ 
		side += KeyRight.KeyState() * cl_sidespeed;
		side -= KeyLeft.KeyState() * cl_sidespeed;
		if (joyxmove > 0)
			side += cl_sidespeed;
		if (joyxmove < 0)
			side -= cl_sidespeed;
	}
 
	forward += KeyForward.KeyState() * cl_forwardspeed;
	forward -= KeyBackward.KeyState() * cl_backspeed;

	side += KeyMoveRight.KeyState() * cl_sidespeed;
	side -= KeyMoveLeft.KeyState() * cl_sidespeed;

	if (joyymove < 0) 
		forward += cl_forwardspeed;
	if (joyymove > 0) 
		forward -= cl_backspeed;

	// Fly up/down/drop keys
	flyheight += KeyFlyUp.KeyState() * cl_flyspeed; // note that the actual flyheight will be twice this
	flyheight -= KeyFlyDown.KeyState() * cl_flyspeed;

	if (allways_run || (KeySpeed.state & 1))
	{
		forward *= cl_movespeedkey;
		side *= cl_movespeedkey;
		flyheight *= cl_movespeedkey;
	}

	if ((!mouse_look && !(KeyMouseLook.state & 1)) || (KeyStrafe.state & 1))
	{
		forward += m_forward * mousey;
	}

	if ((KeyStrafe.state & 1) ||
		(lookstrafe && (mouse_look || (KeyMouseLook.state & 1))))
	{
		side += m_side * mousex;
	}

	if (flyheight > 127)
		flyheight = 127;
	if (flyheight < -127)
		flyheight = -127;
	if (KeyFlyCentre.KeyState())
	{
		flyheight = TOCENTRE;
	}

 	//
	//	BUTTONS
	//

	// Fire buttons
	if (KeyAttack.KeyState())
	{
		cmd->buttons |= BT_ATTACK;
	}

	// Use buttons
	if (KeyUse.KeyState())
	{ 
		cmd->buttons |= BT_USE;
	} 

	// Jumping
	if (KeyJump.KeyState())
	{
		cmd->buttons |= BT_JUMP;
	}

	if (KeyButton4.KeyState())
	{
		cmd->buttons |= 0x08;
	}

	if (KeyButton5.KeyState())
	{
		cmd->buttons |= 0x10;
	}

	if (KeyButton6.KeyState())
	{
		cmd->buttons |= 0x20;
	}

	if (KeyButton7.KeyState())
	{
		cmd->buttons |= 0x40;
	}

	if (KeyButton8.KeyState())
	{
		cmd->buttons |= 0x80;
	}

	//
	//	IMPULSE
	//
	if (impulse_cmd)
	{
		cmd->impulse = impulse_cmd;
		impulse_cmd = 0;
	}

	cmd->forwardmove = (int)forward;
	cmd->sidemove = (int)side;
	cmd->flymove = (int)flyheight;
	unguard;
}

//==========================================================================
//
//	CL_SendMove
//
//==========================================================================

void CL_SendMove()
{
	guard(CL_SendMove);
	ticcmd_t	cmd;

	if (cls.state != ca_connected)
	{
		return;
	}

	if (cls.demoplayback)
	{
		cls.message->Clear();
		return;
	}
	
	if (cls.signon == SIGNONS)
	{
		AdjustAngles();
		BuildTiccmd(&cmd);
		mousex = mousey = 0;

		VMessageOut msg(MAX_MSGLEN << 3);
		msg << (byte)clc_move
			<< (byte)(AngleToByte(cl->ViewAngles.yaw))
			<< (byte)(AngleToByte(cl->ViewAngles.pitch))
			<< (byte)(AngleToByte(cl->ViewAngles.roll))
			<< cmd.forwardmove
			<< cmd.sidemove
			<< cmd.flymove
			<< cmd.buttons
			<< cmd.impulse;
		cls.netcon->SendUnreliableMessage(&msg);
	}

	// send the reliable message
	if (!cls.message->GetNumBits())
	{
		return;		// no message at all
	}

	if (!cls.netcon->CanSendMessage())
	{
		GCon->Log(NAME_Dev, "CL_WriteToServer: can't send");
		return;
	}

	if (cls.netcon->SendMessage(cls.message) == -1)
	{
		Host_Error("CL_WriteToServer: lost server connection");
	}

	cls.message->Clear();
	unguard;
}

//==========================================================================
//
//	CL_Responder
//
//	Get info needed to make ticcmd_ts for the players.
// 
//==========================================================================

bool CL_Responder(event_t* ev)
{
	guard(CL_Responder);
	switch (ev->type) 
	{
	case ev_mouse:
		mousex = ev->data2 * mouse_x_sensitivity;
		mousey = ev->data3 * mouse_y_sensitivity;
		if (invert_mouse)
			mousey = -mousey;
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
//	CL_ClearInput
//
//==========================================================================

void CL_ClearInput()
{
	guard(CL_ClearInput);
	// clear cmd building stuff
	joyxmove = joyymove = 0; 
	mousex = mousey = 0;
	impulse_cmd = 0;
	unguard;
}
