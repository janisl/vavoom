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
//**	Game completion, final screen animation.
//**	
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	F_StartFinale
//
//==========================================================================

void F_StartFinale(void)
{
    cl.intermission = 2;
    automapactive = false;
	cl.palette = 0;

	clpr.Exec("F_StartFinale");
}

//===========================================================================
//
//	F_Ticker
//
//===========================================================================

void F_Ticker(void)
{
	clpr.SetGlobal("frametime", PassFloat(host_frametime));
	clpr.Exec("F_Ticker");
}

//===========================================================================
//
//	F_CheckPal
//
//===========================================================================

boolean F_CheckPal(event_t*)
{
#ifdef FIXME
	if (Game == Heretic)
	{
		if (event->type == ev_keydown &&
			FinaleStage == 1 && f_episode == 2)
		{
			// we're showing the water pic, make any key kick to demo mode
			FinaleStage++;
//FIXME		V_SetPalette((byte*)W_CacheLumpName("PLAYPAL", PU_CACHE));
			fpage1 = R_RegisterPic("TITLE", PIC_RAW);
			return true;
		}
	}
#endif
    return false;
}

//===========================================================================
//
//	F_Responder
//
//===========================================================================

boolean F_Responder(event_t *event)
{
    if (cls.state != ca_connected || cl.intermission != 2)
    	return false;
    return clpr.Exec("F_Responder", (int)event);
}

//==========================================================================
//
//	F_Drawer
//
//==========================================================================

void F_Drawer(void)
{
	clpr.SetGlobal("frametime", PassFloat(host_frametime));
	clpr.Exec("F_Drawer");
}

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2001/08/07 16:48:54  dj_jl
//	Beautification
//
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
