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
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
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

static FFunction *pf_SB_UpdateWidgets;
static FFunction *pf_SB_Drawer;
static FFunction *pf_SB_StartMap;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  SB_Init
//
//==========================================================================

void SB_Init(void)
{
	pf_SB_UpdateWidgets = clpr.FuncForName("SB_UpdateWidgets");
	pf_SB_Drawer = clpr.FuncForName("SB_Drawer");
	pf_SB_StartMap = clpr.FuncForName("SB_StartMap");
	sb_height = clpr.GetGlobal("sb_height");
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
	    clpr.Exec(pf_SB_UpdateWidgets);
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

void SB_Drawer(void)
{
	//	Update widget visibility
	clpr.Exec(pf_SB_Drawer, automapactive ? SB_VIEW_AUTOMAP :
		refdef.height == ScreenHeight ? SB_VIEW_FULLSCREEN : SB_VIEW_NORMAL);
}

//==========================================================================
//
//  SB_Start
//
//==========================================================================

void SB_Start(void)
{
	clpr.Exec(pf_SB_StartMap);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.10  2002/02/02 19:20:41  dj_jl
//	FFunction pointers used instead of the function numbers
//
//	Revision 1.9  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.8  2001/12/27 17:36:47  dj_jl
//	Some speedup
//	
//	Revision 1.7  2001/10/02 17:36:08  dj_jl
//	Removed status bar widgets
//	
//	Revision 1.6  2001/08/30 17:44:07  dj_jl
//	Removed memory leaks after startup
//	
//	Revision 1.5  2001/08/21 17:39:22  dj_jl
//	Real string pointers in progs
//	
//	Revision 1.4  2001/08/15 17:08:15  dj_jl
//	Fixed Strife status bar
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
