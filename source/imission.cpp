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
#include "cl_local.h"

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

	msgLump = W_GetNumForName(VName(name, VName::AddLower8));
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

void IM_SkipIntermission()
{
	GClGame->skipintermission = true;
}

//==========================================================================
//
//	IM_Start
//
//==========================================================================

void IM_Start(void)
{
	GClGame->scores = scores;
	GClGame->im = &im;

	cl->intermission = 1;
	cl->palette = 0;
	AM_Stop();
	SN_StopAllSequences();

	GClGame->eventIintermissionStart();
}

//**************************************************************************
//
//	$Log$
//	Revision 1.8  2006/03/02 23:24:35  dj_jl
//	Wad lump names stored as names.
//
//	Revision 1.7  2006/02/20 22:52:56  dj_jl
//	Changed client state to a class.
//	
//	Revision 1.6  2006/02/09 22:35:54  dj_jl
//	Moved all client game code to classes.
//	
//	Revision 1.5  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
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
