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

IMPLEMENT_CLASS(V, Thinker)

static FFunction *pf_UpdateSpecials;
static int FIndex_Tick;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	P_InitThinkers
//
//==========================================================================

void P_InitThinkers(void)
{
	pf_UpdateSpecials = svpr.FuncForName("P_UpdateSpecials");
	FIndex_Tick = VThinker::StaticClass()->GetFunctionIndex("Tick");
}

//==========================================================================
//
//	SV_DestroyAllThinkers
//
//==========================================================================

void SV_DestroyAllThinkers(void)
{
	guard(SV_DestroyAllThinkers);
	for (TObjectIterator<VThinker> It; It; ++It)
	{
		delete *It;
	}
	VObject::CollectGarbage();
	unguard;
}

//==========================================================================
//
//	VThinker::Tick
//
//==========================================================================

void VThinker::Tick(float DeltaTime)
{
	guard(VThinker::Tick);
	svpr.Exec(GetVFunction(FIndex_Tick), (int)this, PassFloat(DeltaTime));
	unguard;
}

//==========================================================================
//
// RunThinkers
//
//==========================================================================

static void RunThinkers(void)
{
	guard(RunThinkers);
	for (TObjectIterator<VThinker> It; It; ++It)
	{
		if (!(It->GetFlags() & OF_Destroyed))
		{
			It->Tick(host_frametime);
		}
	}
	unguard;
}

//==========================================================================
//
// P_Ticker
//
//==========================================================================

void P_Ticker(void)
{
	guard(P_Ticker);
	int 	i;

	RunThinkers();
	svpr.Exec(pf_UpdateSpecials);
    P_UpdateButtons();

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (svvars.Players[i] && svvars.Players[i]->bSpawned)
		{
			svvars.Players[i]->eventSetViewPos();
		}
	}

	level.time += host_frametime;
    level.tictime++;
	unguard;
}

//**************************************************************************
//
//	$Log$
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
