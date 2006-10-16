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

VStr LoadTextLump(VName name)
{
	VStream* Strm = W_CreateLumpReaderName(name);
	int msgSize = Strm->TotalSize();
	char* buf = new char[msgSize + 1];
	Strm->Serialise(buf, msgSize);
	delete Strm;
	buf[msgSize] = 0; // Append terminator
	VStr Ret = buf;
	delete[] buf;
	return Ret;
}

//==========================================================================
//
//	IM_SkipIntermission
//
//==========================================================================

void IM_SkipIntermission()
{
	GClGame->ClientFlags |= VClientGameBase::CF_SkipIntermission;
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

	GClGame->intermission = 1;
	AM_Stop();
	GAudio->StopAllSequences();

	GClGame->eventIintermissionStart();
}
