//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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
//**  element must be thinker_t.
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

static int	pf_UpdateSpecials;
static int	pf_SetViewPos;

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
	level.thinkers.prev = level.thinkers.next  = &level.thinkers;
}

//==========================================================================
//
//	P_AddThinker
//
//	Adds a new thinker at the end of the list.
//
//==========================================================================

void P_AddThinker(thinker_t *thinker)
{
	level.thinkers.prev->next = thinker;
	thinker->next = &level.thinkers;
	thinker->prev = level.thinkers.prev;
	level.thinkers.prev = thinker;
}

//==========================================================================
//
//	P_RemoveThinker
//
//	Deallocation is lazy -- it will not actually be freed until its
// thinking turn comes up.
//
//==========================================================================

void P_RemoveThinker(thinker_t *thinker)
{
	thinker->function = NULL;
}

//==========================================================================
//
//  P_ProgsThinker
//
//==========================================================================

void P_SpecialThinker(special_t* special)
{
	svpr.Exec(special->funcnum, (int)special);
}

//==========================================================================
//
// RunThinkers
//
//==========================================================================

static void RunThinkers(void)
{
	thinker_t *currentthinker;

	currentthinker = level.thinkers.next;
	while (currentthinker != &level.thinkers)
	{
		if (currentthinker->function)
		{
			currentthinker->function(currentthinker);
		}
        else
		{
			// Time to remove it
			currentthinker->next->prev = currentthinker->prev;
			currentthinker->prev->next = currentthinker->next;
			Z_Free(currentthinker);
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

