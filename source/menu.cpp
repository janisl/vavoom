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
//**	DOOM selection menu, options, episode etc.
//**	Sliders and icons. Kinda widget stuff.
//**	
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void SV_ShutdownServer(boolean crash);
void CL_Disconnect(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int pf_MN_SetMenu;
static int pf_MN_DeactivateMenu;
static int pf_MB_Responder;
static int pf_MN_Responder;
static int pf_MN_Drawer;
static int pf_MB_Drawer;
static int pf_MN_Active;
static int pf_MB_Active;
static int pg_frametime;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	COMMAND SetMenu
//
//==========================================================================

COMMAND(SetMenu)
{
	clpr.Exec(pf_MN_SetMenu, (int)Argv(1));
}

//==========================================================================
//
//	MN_Init
//
//==========================================================================

void MN_Init(void)
{
#ifdef SERVER
	clpr.SetGlobal("local_server", 1);
#else
	clpr.SetGlobal("local_server", 0);
#endif
	pf_MN_SetMenu = clpr.FuncNumForName("MN_SetMenu");
	pf_MN_DeactivateMenu = clpr.FuncNumForName("MN_DeactivateMenu");
	pf_MB_Responder = clpr.FuncNumForName("MB_Responder");
	pf_MN_Responder = clpr.FuncNumForName("MN_Responder");
	pf_MN_Drawer = clpr.FuncNumForName("MN_Drawer");
	pf_MB_Drawer = clpr.FuncNumForName("MB_Drawer");
	pf_MN_Active = clpr.FuncNumForName("MN_Active");
	pf_MB_Active = clpr.FuncNumForName("MB_Active");
	pg_frametime = clpr.GlobalNumForName("frametime");
}

//==========================================================================
//
//  MN_ActivateMenu
//
//==========================================================================

void MN_ActivateMenu(void)
{
    // intro might call this repeatedly
    if (!MN_Active())
	{
		clpr.Exec(pf_MN_SetMenu, (int)"Main");
	}
}

//==========================================================================
//
//	MN_DeactivateMenu
//
//==========================================================================

void MN_DeactivateMenu(void)
{
	clpr.Exec(pf_MN_DeactivateMenu);
}

//==========================================================================
//
//  MN_Responder
//
//==========================================================================

boolean MN_Responder(event_t* event)
{
	if (clpr.Exec(pf_MB_Responder, (int)event))
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

	return clpr.Exec(pf_MN_Responder, (int)event);
}

//==========================================================================
//
//	MN_Drawer
//
//==========================================================================

void MN_Drawer(void)
{
	clpr.SetGlobal(pg_frametime, PassFloat(host_frametime));
	clpr.Exec(pf_MN_Drawer);
	clpr.Exec(pf_MB_Drawer);
}

//==========================================================================
//
//	MN_Active
//
//==========================================================================

boolean MN_Active(void)
{
	return clpr.Exec(pf_MN_Active) || clpr.Exec(pf_MB_Active);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.9  2001/12/27 17:36:47  dj_jl
//	Some speedup
//
//	Revision 1.8  2001/10/09 17:25:02  dj_jl
//	Finished slist moving stuff
//	
//	Revision 1.7  2001/10/08 17:34:57  dj_jl
//	A lots of small changes and cleanups
//	
//	Revision 1.6  2001/09/25 17:04:45  dj_jl
//	Replaced menu commands with command SetMenu
//	
//	Revision 1.5  2001/08/30 17:39:51  dj_jl
//	Moved view border and message box to progs
//	
//	Revision 1.4  2001/08/07 16:49:26  dj_jl
//	Added C_Active
//	
//	Revision 1.3  2001/07/31 17:09:34  dj_jl
//	Removed seting shareware and ExtendedWAD in progs
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
