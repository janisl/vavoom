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
//**	Status bar code.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	SB_VIEW_NORMAL,
	SB_VIEW_AUTOMAP,
	SB_VIEW_FULLSCREEN
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern refdef_t			refdef;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int 					sb_height = 32;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  SB_Init
//
//==========================================================================

void SB_Init(void)
{
	sb_height = GClGame->sb_height;
}

//==========================================================================
//
//  SB_Ticker
//
//==========================================================================

void SB_Ticker(void)
{
	if (cls.signon == SIGNONS)
	{
	    GClGame->eventStatusBarUpdateWidgets();
	}
}

//==========================================================================
//
//	SB_Responder
//
//==========================================================================

boolean SB_Responder(event_t *)
{
	return false;
}

//==========================================================================
//
//	SB_Drawer
//
//==========================================================================

void SB_Drawer()
{
	//	Update widget visibility
	GClGame->eventStatusBarDrawer(automapactive ? SB_VIEW_AUTOMAP :
		refdef.height == ScreenHeight ? SB_VIEW_FULLSCREEN : SB_VIEW_NORMAL);
}

//==========================================================================
//
//  SB_Start
//
//==========================================================================

void SB_Start(void)
{
	GClGame->eventStatusBarStartMap();
}
