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
//**	THINKERS
//**
//**	All thinkers should be allocated by Z_Malloc so they can be operated
//**  on uniformly. The actual structures will vary in size, but the first
//**  element must be VThinker.
//**	
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(VThinker)

static int	pf_UpdateSpecials;
static int	pf_SetViewPos;
static int	pf_RunThink;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	P_InitThinkers
//
//==========================================================================

void P_InitThinkers(void)
{
	pf_UpdateSpecials = svpr.FuncNumForName("P_UpdateSpecials");
    pf_SetViewPos = svpr.FuncNumForName("SetViewPos");
	pf_RunThink = svpr.FuncNumForName("RunThink");
	level.thinkerHead = NULL;
 	level.thinkerTail = NULL;
}

//==========================================================================
//
//	SV_DestroyAllThinkers
//
//==========================================================================

void SV_DestroyAllThinkers(void)
{
	for (VThinker *th = level.thinkerHead; th; th = th->next)
	{
		th->Destroy();
	}
	level.thinkerHead = NULL;
 	level.thinkerTail = NULL;
}

//==========================================================================
//
//	P_AddThinker
//
//	Adds a new thinker at the end of the list.
//
//==========================================================================

void P_AddThinker(VThinker *thinker)
{
	if (level.thinkerHead)
	{
		thinker->next = NULL;
		thinker->prev = level.thinkerTail;
		level.thinkerTail->next = thinker;
		level.thinkerTail = thinker;
	}
	else
	{
		thinker->prev = NULL;
		thinker->next = NULL;
		level.thinkerHead = thinker;
		level.thinkerTail = thinker;
	}
}

//==========================================================================
//
//	P_RemoveThinker
//
//	Deallocation is lazy -- it will not actually be freed until its
// thinking turn comes up.
//
//==========================================================================

void P_RemoveThinker(VThinker *thinker)
{
	thinker->destroyed = true;
}

//==========================================================================
//
// RunThinkers
//
//==========================================================================

static void RunThinkers(void)
{
	VThinker *currentthinker;

	currentthinker = level.thinkerHead;
	while (currentthinker)
	{
		if (!currentthinker->destroyed)
		{
			svpr.Exec(pf_RunThink, (int)currentthinker);
		}
        else
		{
			// Time to remove it
			if (currentthinker->next)
			{
				currentthinker->next->prev = currentthinker->prev;
			}
			else
			{
				level.thinkerTail = currentthinker->prev;
			}
			if (currentthinker->prev)
			{
				currentthinker->prev->next = currentthinker->next;
			}
			else
			{
				level.thinkerHead = currentthinker->next;
			}
			currentthinker->Destroy();
		}
		currentthinker = currentthinker->next;
	}
}

//==========================================================================
//
// P_Ticker
//
//==========================================================================

void P_Ticker(void)
{
	int 	i;

	RunThinkers();
	svpr.Exec(pf_UpdateSpecials);
    P_UpdateButtons();

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (players[i].active && players[i].spawned)
		{
			svpr.Exec(pf_SetViewPos, (int)&players[i]);
		}
	}

	level.time += host_frametime;
    level.tictime++;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2001/12/18 19:03:17  dj_jl
//	A lots of work on VObject
//
//	Revision 1.5  2001/12/04 18:14:46  dj_jl
//	Renamed thinker_t to VThinker
//	
//	Revision 1.4  2001/09/20 16:30:28  dj_jl
//	Started to use object-oriented stuff in progs
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
