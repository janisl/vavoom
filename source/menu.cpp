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
//**	DOOM selection menu, options, episode etc.
//**	Sliders and icons. Kinda widget stuff.
//**	
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "cl_local.h"
#include "ui.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void SV_ShutdownServer(bool crash);
void CL_Disconnect();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	COMMAND SetMenu
//
//==========================================================================

COMMAND(SetMenu)
{
	GClGame->eventSetMenu(Args.Num() > 1 ? *Args[1] : "");
}

//==========================================================================
//
//	MN_Init
//
//==========================================================================

void MN_Init()
{
#ifdef SERVER
	GClGame->ClientFlags |= VClientGameBase::CF_LocalServer;
#else
	GClGame->ClientFlags &= ~VClientGameBase::CF_LocalServer;
#endif
	VRootWindow::StaticInit();
	GClGame->eventRootWindowCreated();
}

//==========================================================================
//
//  MN_ActivateMenu
//
//==========================================================================

void MN_ActivateMenu()
{
    // intro might call this repeatedly
    if (!MN_Active())
	{
		GClGame->eventSetMenu("Main");
	}
}

//==========================================================================
//
//	MN_DeactivateMenu
//
//==========================================================================

void MN_DeactivateMenu()
{
	GClGame->eventDeactivateMenu();
}

//==========================================================================
//
//  MN_Responder
//
//==========================================================================

bool MN_Responder(event_t* event)
{
	if (GClGame->eventMessageBoxResponder(event))
	{
		return true;
	}

    // Pop-up menu?
    if (!MN_Active() && event->type == ev_keydown && !C_Active() &&
		(cls.state != ca_connected || cls.demoplayback) &&
		event->data1 != '`' && (event->data1 < K_F1 || event->data1 > K_F12))
	{
		MN_ActivateMenu();
		return true;
	}

	return GClGame->eventMenuResponder(event);
}

//==========================================================================
//
//	MN_Drawer
//
//==========================================================================

void MN_Drawer()
{
	GClGame->eventMessageBoxDrawer();
}

//==========================================================================
//
//	MN_Active
//
//==========================================================================

bool MN_Active()
{
	return GClGame->eventMenuActive() || GClGame->eventMessageBoxActive();
}
