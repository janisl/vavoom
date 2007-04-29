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

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	P_InitThinkers
//
//==========================================================================

void P_InitThinkers()
{
	VThinker::FIndex_Tick = VThinker::StaticClass()->GetFunctionIndex(NAME_Tick);
}

//==========================================================================
//
// RunThinkers
//
//==========================================================================

static void RunThinkers()
{
	guard(RunThinkers);
	for (VThinker* Th = GLevel->ThinkerHead; Th; Th = Th->Next)
	{
		if (!(Th->GetFlags() & _OF_DelayedDestroy))
		{
			Th->Tick(host_frametime);
		}
		else
		{
			GLevel->RemoveThinker(Th);
			Th->ConditionalDestroy();
		}
	}
	unguard;
}

//==========================================================================
//
// P_Ticker
//
//==========================================================================

void P_Ticker()
{
	guard(P_Ticker);
	int 	i;

	RunThinkers();
	GLevelInfo->eventUpdateSpecials();

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (GGameInfo->Players[i] &&
			GGameInfo->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned)
		{
			GGameInfo->Players[i]->eventSetViewPos();
		}
	}

	GLevel->Time += host_frametime;
	GLevel->TicTime++;
	unguard;
}

//==========================================================================
//
//	VLevel::AddThinker
//
//==========================================================================

void VLevel::AddThinker(VThinker* Th)
{
	guard(VLevel::AddThinker);
	Th->XLevel = this;
	Th->Level = LevelInfo;
	Th->Prev = ThinkerTail;
	Th->Next = NULL;
	if (ThinkerTail)
		ThinkerTail->Next = Th;
	else
		ThinkerHead = Th;
	ThinkerTail = Th;
	unguard;
}

//==========================================================================
//
//	VLevel::RemoveThinker
//
//==========================================================================

void VLevel::RemoveThinker(VThinker* Th)
{
	guard(VLevel::RemoveThinker);
	if (Th == ThinkerHead)
		ThinkerHead = Th->Next;
	else
		Th->Prev->Next = Th->Next;
	if (Th == ThinkerTail)
		ThinkerTail = Th->Prev;
	else
		Th->Next->Prev = Th->Prev;
	unguard;
}

//==========================================================================
//
//	VLevel::DestroyAllThinkers
//
//==========================================================================

void VLevel::DestroyAllThinkers()
{
	guard(VLevel::DestroyAllThinkers);
	for (VThinker* Th = ThinkerHead; Th; Th = Th->Next)
	{
		Th->ConditionalDestroy();
	}
	ThinkerHead = NULL;
	ThinkerTail = NULL;
	unguard;
}
