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
	VThinker::FIndex_Tick = VThinker::StaticClass()->GetFunctionIndex("Tick");
}

//==========================================================================
//
//	SV_DestroyAllThinkers
//
//==========================================================================

void SV_DestroyAllThinkers()
{
	guard(SV_DestroyAllThinkers);
	if (!GLevel)
		return;
	for (VThinker* Th = GLevel->ThinkerHead; Th; Th = Th->Next)
	{
		Th->ConditionalDestroy();
	}
	VObject::CollectGarbage();
	GLevel->ThinkerHead = NULL;
	GLevel->ThinkerTail = NULL;
	unguard;
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
    P_UpdateButtons();

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (GGameInfo->Players[i] && GGameInfo->Players[i]->bSpawned)
		{
			GGameInfo->Players[i]->eventSetViewPos();
		}
	}

	level.time += host_frametime;
	level.tictime++;
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
	Th->Level = GLevelInfo;
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

//**************************************************************************
//
//	$Log$
//	Revision 1.26  2006/03/06 13:05:51  dj_jl
//	Thunbker list in level, client now uses entity class.
//
//	Revision 1.25  2006/02/28 18:06:28  dj_jl
//	Put thinkers back in linked list.
//	
//	Revision 1.24  2006/02/26 20:52:48  dj_jl
//	Proper serialisation of level and players.
//	
//	Revision 1.23  2006/02/25 17:14:19  dj_jl
//	Implemented proper serialisation of the objects.
//	
//	Revision 1.22  2006/02/15 23:28:18  dj_jl
//	Moved all server progs global variables to classes.
//	
//	Revision 1.21  2005/12/27 22:24:00  dj_jl
//	Created level info class, moved action special handling to it.
//	
//	Revision 1.20  2005/11/24 20:09:23  dj_jl
//	Removed unused fields from Object class.
//	
//	Revision 1.19  2004/12/27 12:23:17  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.18  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//	
//	Revision 1.17  2003/11/12 16:47:40  dj_jl
//	Changed player structure into a class
//	
//	Revision 1.16  2003/07/11 16:45:20  dj_jl
//	Made array of players with pointers
//	
//	Revision 1.15  2003/03/08 12:10:13  dj_jl
//	API fixes.
//	
//	Revision 1.14  2002/07/23 13:10:38  dj_jl
//	Some fixes for switching to floating-point time.
//	
//	Revision 1.13  2002/07/13 07:50:58  dj_jl
//	Added guarding.
//	
//	Revision 1.12  2002/04/11 16:42:10  dj_jl
//	Renamed Think to Tick.
//	
//	Revision 1.11  2002/02/15 19:12:04  dj_jl
//	Property namig style change
//	
//	Revision 1.10  2002/02/02 19:20:41  dj_jl
//	FFunction pointers used instead of the function numbers
//	
//	Revision 1.9  2002/01/21 18:25:09  dj_jl
//	Changed guarding
//	
//	Revision 1.8  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.7  2001/12/27 17:33:29  dj_jl
//	Removed thinker list
//	
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
