//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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

#define TOCENTER 		-128

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

	void KeyDown(char *c);
	void KeyUp(char *c);
	float KeyState(void);
};

class TCmdKeyDown : public TCommand
{
 public:
	TCmdKeyDown(const char *AName, TKButton &AKey) : TCommand(AName), Key(AKey) { }
    void Run(void);

	TKButton	&Key;
};

class TCmdKeyUp : public TCommand
{
 public:
	TCmdKeyUp(const char *AName, TKButton &AKey) : TCommand(AName), Key(AKey)	{ }
    void Run(void);

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

static bool		roll_centering;

static TCvarI	allways_run("allways_run", "1", CVAR_ARCHIVE);
static TCvarI	artiskip("artiskip", "1", CVAR_ARCHIVE);	// whether shift-enter skips an artifact

static TCvarF	cl_forwardspeed("cl_forwardspeed", "200", CVAR_ARCHIVE);
static TCvarF	cl_backspeed("cl_backspeed", "200", CVAR_ARCHIVE);
static TCvarF	cl_sidespeed("cl_sidespeed", "350", CVAR_ARCHIVE);
static TCvarF	cl_flyspeed("cl_flyspeed", "80", CVAR_ARCHIVE);

static TCvarF	cl_movespeedkey("cl_movespeedkey", "2.0", CVAR_ARCHIVE);

static TCvarF	cl_yawspeed("cl_yawspeed", "140", CVAR_ARCHIVE);
static TCvarF	cl_pitchspeed("cl_pitchspeed", "150", CVAR_ARCHIVE);
static TCvarF	cl_rollspeed("cl_rollspeed", "32", CVAR_ARCHIVE);
static TCvarF	cl_pitchdriftspeed("cl_pitchdriftspeed", "270", CVAR_ARCHIVE);

static TCvarF	cl_anglespeedkey("cl_anglespeedkey", "1.5", CVAR_ARCHIVE);

static TCvarF	mouse_x_sensitivity("mouse_x_sensitivity", "3.0", CVAR_ARCHIVE);
static TCvarF	mouse_y_sensitivity("mouse_y_sensitivity", "3.0", CVAR_ARCHIVE);
static TCvarI	mouse_look("mouse_look", "1", CVAR_ARCHIVE);
static TCvarI	invert_mouse("invert_mouse", "0", CVAR_ARCHIVE);
static TCvarI	lookstrafe("lookstrafe", "0", CVAR_ARCHIVE);
static TCvarI	lookspring("lookspring", "0", CVAR_ARCHIVE);

static TCvarF	m_yaw("m_yaw", "0.022", CVAR_ARCHIVE);
static TCvarF	m_pitch("m_pitch", "0.022", CVAR_ARCHIVE);
static TCvarF	m_forward("m_forward", "1.0", CVAR_ARCHIVE);
static TCvarF	m_side("m_side", "0.8", CVAR_ARCHIVE);

static TCvarF	joy_yaw("joy_yaw", "1", CVAR_ARCHIVE);

BUTTON(Forward)
BUTTON(Backward)
BUTTON(Left)
BUTTON(Right)
BUTTON(LookUp)
BUTTON(LookDown)
BUTTON(LookCenter)
BUTTON(MoveLeft)
BUTTON(MoveRight)
BUTTON(FlyUp)
BUTTON(FlyDown)
BUTTON(FlyCenter)
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
BUTTON(TiltLeft)
BUTTON(TiltRight)

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TKButton::KeyDown
//
//==========================================================================

void TKButton::KeyDown(char *c)
{
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
		con << "Three keys down for a button!\n";
		return;
	}
	
	if (state & 1)
		return;		// still down
	state |= 1 + 2;	// down + impulse down
}

//==========================================================================
//
//	TKButton::KeyUp
//
//==========================================================================

void TKButton::KeyUp(char *c)
{
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

float TKButton::KeyState(void)
{
	float		val;
	bool		impulsedown, impulseup, down;
	
	impulsedown = state & 2;
	impulseup = state & 4;
	down = state & 1;
	val = 0;
	
	if (impulsedown && !impulseup)
		if (down)
			val = 0.5;	// pressed and held this frame
		else
			val = 0;	//	I_Error ();
	if (impulseup && !impulsedown)
		if (down)
			val = 0;	//	I_Error ();
		else
			val = 0;	// released this frame
	if (!impulsedown && !impulseup)
		if (down)
			val = 1.0;	// held the entire frame
		else
			val = 0;	// up the entire frame
	if (impulsedown && impulseup)
		if (down)
			val = 0.75;	// released and re-pressed this frame
		else
			val = 0.25;	// pressed and released this frame

	state &= 1;		// clear impulses
	
	return val;
}

//==========================================================================
//
//	TCmdKeyDown::Run
//
//==========================================================================

void TCmdKeyDown::Run(void)
{
	Key.KeyDown(Argv(1));
}

//==========================================================================
//
//	TCmdKeyUp::Run
//
//==========================================================================

void TCmdKeyUp::Run(void)
{
	Key.KeyUp(Argv(1));
}

//==========================================================================
//
//	COMMAND Impulse
//
//==========================================================================

COMMAND(Impulse)
{
	impulse_cmd = atoi(Argv(1));
}

//==========================================================================
//
//	COMMAND ToggleAlwaysRun
//
//==========================================================================

COMMAND(ToggleAlwaysRun)
{
	allways_run = !allways_run;
}

//==========================================================================
//
//	COMMAND CenterTilt
//
//==========================================================================

COMMAND(CenterTilt)
{
	roll_centering = true;
}

void V_StartPitchDrift(void)
{
	cl.centering = true;
}
void V_StopPitchDrift(void)
{
	cl.centering = false;
}

//==========================================================================
//
//	AdjustAngles
//
//==========================================================================

static void AdjustAngles(void)
{
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
    	cl.viewangles.yaw -= DEG2BAM(KeyRight.KeyState() * cl_yawspeed * speed);
    	cl.viewangles.yaw += DEG2BAM(KeyLeft.KeyState() * cl_yawspeed * speed);
		if (joyxmove > 0)
	    	cl.viewangles.yaw -= DEG2BAM(joy_yaw * speed);
		if (joyxmove < 0)
	    	cl.viewangles.yaw += DEG2BAM(joy_yaw * speed);
    }
	if (!(KeyStrafe.state & 1) &&
		(!lookstrafe || (!mouse_look && !(KeyMouseLook.state & 1))))
	{
		cl.viewangles.yaw -= DEG2BAM(mousex * m_yaw);
	}

	//	PITCH
	float up = KeyLookUp.KeyState();
	float down = KeyLookDown.KeyState();
	cl.viewangles.pitch -= DEG2BAM(cl_pitchspeed * up * speed);
	cl.viewangles.pitch += DEG2BAM(cl_pitchspeed * down * speed);
	if (up || down || (KeyMouseLook.state & 1))
	{
		V_StopPitchDrift();
	}
	if ((mouse_look || (KeyMouseLook.state & 1)) && !(KeyStrafe.state & 1))
	{
		cl.viewangles.pitch -= DEG2BAM(mousey * m_pitch);
	}

	//	Center look
	if ((KeyLookCenter.state & 1) || (KeyFlyCenter.state & 1))
	{
		V_StartPitchDrift();
	}
	if (cl.centering)
	{
		angle_t adelta = DEG2BAM(cl_pitchdriftspeed * host_frametime);
		if (abs(cl.viewangles.pitch) < (int)adelta)
		{
			cl.viewangles.pitch = 0;
			cl.centering = false;
		}
		else
		{
			if (cl.viewangles.pitch < ANG180)
			{
				cl.viewangles.pitch -= adelta;
			}
			else if (cl.viewangles.pitch > ANG180)
			{
				cl.viewangles.pitch += adelta;
			}
		}
	}

	//	ROLL
	if (cl.health <= 0)
 	{
 		if (cl.viewangles.roll < 75 * ANG1)
		{
			cl.viewangles.roll += DEG2BAM(80 * host_frametime);
		}
 		if (cl.viewangles.roll > (angle_t)285 * ANG1)
		{
			cl.viewangles.roll -= DEG2BAM(80 * host_frametime);
		}
		roll_centering = false;
	}
	else
	{
    	cl.viewangles.roll += DEG2BAM(KeyTiltRight.KeyState() * cl_rollspeed * speed);
    	cl.viewangles.roll -= DEG2BAM(KeyTiltLeft.KeyState() * cl_rollspeed * speed);
		if (roll_centering)
		{
			if (abs(cl.viewangles.roll) < (int)DEG2BAM(cl_rollspeed * host_frametime))
			{
				cl.viewangles.roll = 0;
				roll_centering = false;
			}
			else
			{
				if (cl.viewangles.roll < ANG180)
				{
					cl.viewangles.roll -= DEG2BAM(cl_rollspeed * host_frametime);
				}
				else if (cl.viewangles.roll > ANG180)
				{
					cl.viewangles.roll += DEG2BAM(cl_rollspeed * host_frametime);
				}
			}
		}
	}

	//	Check angles
	if ((int)cl.viewangles.pitch > (int)(80 * ANG1))
	{
		cl.viewangles.pitch = (angle_t)(80 * ANG1);
	}
	if ((int)cl.viewangles.pitch < (int)(-70 * ANG1))
	{
		cl.viewangles.pitch = (angle_t)(-70 * ANG1);
	}

	if ((int)cl.viewangles.roll > (int)(80 * ANG1))
	{
		cl.viewangles.roll = (angle_t)(80 * ANG1);
	}
	if ((int)cl.viewangles.roll < (int)(-80 * ANG1))
	{
		cl.viewangles.roll = (angle_t)(-80 * ANG1);
	}
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
	if (KeyFlyCenter.KeyState())
	{
		flyheight = TOCENTER;
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
}

//==========================================================================
//
//	CL_SendMove
//
//==========================================================================

void CL_SendMove(void)
{
	byte		buf[MAX_DATAGRAM];
	TMessage	msg(buf, MAX_DATAGRAM);
	ticcmd_t	cmd;

	if (cls.state != ca_connected)
	{
		return;
	}

	if (cls.demoplayback)
	{
		cls.message.Clear();
		return;
	}
	
	if (cls.signon == SIGNONS)
	{
		AdjustAngles();
		BuildTiccmd(&cmd);
	    mousex = mousey = 0;
		msg.Clear();
		msg << (byte)clc_move
			<< (byte)(cl.viewangles.yaw >> 24)
			<< (byte)(cl.viewangles.pitch >> 24)
			<< (byte)(cl.viewangles.roll >> 24)
			<< cmd.forwardmove
			<< cmd.sidemove
			<< cmd.flymove
			<< cmd.buttons
			<< cmd.impulse;

		NET_SendUnreliableMessage(cls.netcon, &msg);
	}

	// send the reliable message
	if (!cls.message.CurSize)
	{
		return;		// no message at all
	}

	if (!NET_CanSendMessage(cls.netcon))
	{
		cond << "CL_WriteToServer: can't send\n";
		return;
	}

	if (NET_SendMessage(cls.netcon, &cls.message) == -1)
	{
		Host_Error("CL_WriteToServer: lost server connection");
	}

	cls.message.Clear();
}

//==========================================================================
//
//	CL_Responder
//
//	Get info needed to make ticcmd_ts for the players.
// 
//==========================================================================

boolean CL_Responder(event_t* ev)
{ 
    switch (ev->type) 
    { 
	 case ev_mouse:
		mousex = ev->data2 * mouse_x_sensitivity;
		mousey = ev->data3 * mouse_y_sensitivity;
       	if ((int)invert_mouse)
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
}

//==========================================================================
//
//	CL_ClearInput
//
//==========================================================================

void CL_ClearInput(void)
{
    // clear cmd building stuff
    joyxmove = joyymove = 0; 
    mousex = mousey = 0;
	impulse_cmd = 0;
}

