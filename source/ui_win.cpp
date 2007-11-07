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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(V, Window);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VWindow::VWindow
//
//==========================================================================

VWindow::VWindow()
: WindowType(WIN_Normal)
{
	WindowFlags |= WF_IsSensitive;
}

//==========================================================================
//
//	VWindow::GetRootWindow
//
//==========================================================================

VRootWindow *VWindow::GetRootWindow()
{
	guard(VWindow::GetRootWindow);
	VWindow *win = this;
	while (win->WindowType < WIN_Root)
	{
		win = static_cast<VWindow*>(win->ParentWidget);
	}
	return (VRootWindow *)win;
	unguard;
}

//==========================================================================
//
//	VWindow::SetSensitivity
//
//==========================================================================

void VWindow::SetSensitivity(bool NewSensitivity)
{
	guard(VWindow::SetSensitivity);
	if (!!(WindowFlags & WF_IsSensitive) != NewSensitivity)
	{
		if (NewSensitivity)
			WindowFlags |= WF_IsSensitive;
		else
			WindowFlags &= ~WF_IsSensitive;
		SensitivityChanged(NewSensitivity);
	}
	unguard;
}

//==========================================================================
//
//	Natives
//
//==========================================================================

IMPLEMENT_FUNCTION(VWindow, SetSensitivity)
{
	P_GET_BOOL(bNewSensitivity);
	P_GET_SELF;
	Self->SetSensitivity(bNewSensitivity);
}
IMPLEMENT_FUNCTION(VWindow, Enable)
{
	P_GET_SELF;
	Self->Enable();
}
IMPLEMENT_FUNCTION(VWindow, Disable)
{
	P_GET_SELF;
	Self->Disable();
}
IMPLEMENT_FUNCTION(VWindow, IsSensitive)
{
	P_GET_SELF;
	RET_BOOL(Self->IsSensitive());
}

IMPLEMENT_FUNCTION(VWindow, GetRootWindow)
{
	P_GET_SELF;
	RET_REF(Self->GetRootWindow());
}
