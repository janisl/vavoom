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
//**	Intermission screens.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

scores_t		scores[MAXPLAYERS];
im_t			im;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	LoadTextLump
//
//==========================================================================

void LoadTextLump(char *name, char *buf, int bufsize)
{
	int		msgSize;
	int		msgLump;

	msgLump = W_GetNumForName(name);
	msgSize = W_LumpLength(msgLump);
	if (msgSize >= bufsize)
	{
		Sys_Error("Message lump too long (%s)", name);
	}
	W_ReadLump(msgLump, buf);
	buf[msgSize] = 0; // Append terminator
}

//==========================================================================
//
//	IM_SkipIntermission
//
//==========================================================================

void IM_SkipIntermission(void)
{
	clpr.SetGlobal("skipintermission", true);
}

//==========================================================================
//
//	IM_Start
//
//==========================================================================

void IM_Start(void)
{
	clpr.SetGlobal("scores", (int)scores);
	clpr.SetGlobal("im", (int)&im);

	cl.intermission = 1;
	cl.palette = 0;
	C_ClearNotify();
	AM_Stop();
	SN_StopAllSequences();

	clpr.Exec("IM_Start");
}

//==========================================================================
//
//  IM_Drawer
//
//==========================================================================

void IM_Drawer(void)
{
	clpr.SetGlobal("frametime", PassFloat(host_frametime));
	clpr.Exec("IM_Drawer");
}

//==========================================================================
//
//  IM_Ticker
//
//	Updates stuff each frame
//
//==========================================================================

void IM_Ticker(void)
{
	clpr.SetGlobal("frametime", PassFloat(host_frametime));
	clpr.Exec("IM_UpdateStats");
}

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
