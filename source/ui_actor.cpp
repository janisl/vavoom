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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "ui.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VActorDisplayWindow : public VWindow
{
	DECLARE_CLASS(VActorDisplayWindow, VWindow, 0)

	VState*		CastState;
	float		CastTime;
	float		StateTime;
	VState*		NextState;

	VActorDisplayWindow();

	void SetState(VState*);
	void DrawWindow();

	DECLARE_FUNCTION(SetState)
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

bool R_DrawStateModelFrame(VState* State, float Inter, const TVec& Origin,
	float Angle);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(V, ActorDisplayWindow);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VActorDisplayWindow::VActorDisplayWindow
//
//==========================================================================

VActorDisplayWindow::VActorDisplayWindow()
{
}

//==========================================================================
//
//	VActorDisplayWindow::SetState
//
//==========================================================================

void VActorDisplayWindow::SetState(VState* AState)
{
	guard(VActorDisplayWindow::SetState);
	CastState = AState;
	StateTime = CastState->Time;
	NextState = CastState->NextState;
	unguard;
}

//==========================================================================
//
//	VActorDisplayWindow::DrawWindow
//
//==========================================================================

void VActorDisplayWindow::DrawWindow()
{
	guard(VActorDisplayWindow::DrawWindow);
	// draw the current frame in the middle of the screen
	float TimeFrac = 0.0;
	if (StateTime > 0.0)
	{
		TimeFrac = 1.0 - CastTime / StateTime;
		TimeFrac = MID(0.0, TimeFrac, 1.0);
	}
	if (!R_DrawStateModelFrame(CastState, TimeFrac, TVec(-128.0, 0.0, -48.0), 0.0))
	{
		SCR_SetVirtualScreen(320, 200);
		R_DrawSpritePatch(160, 170, CastState->SpriteIndex, CastState->Frame, 0);
		SCR_SetVirtualScreen(640, 480);
	}
	unguard;
}

IMPLEMENT_FUNCTION(VActorDisplayWindow, SetState)
{
	P_GET_PTR(VState, State);
	P_GET_SELF;
	Self->SetState(State);
}
