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
//**	THINKERS
//**
//**	All thinkers should be allocated by Z_Malloc so they can be operated
//**  on uniformly. The actual structures will vary in size, but the first
//**  element must be VThinker.
//**	
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "network.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int			VThinker::FIndex_Tick;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(V, Thinker)

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VThinker::VThinker
//
//==========================================================================

VThinker::VThinker()
{
}

//==========================================================================
//
//	VThinker::Destroy
//
//==========================================================================

void VThinker::Destroy()
{
	guard(VThinker::Destroy);
	if (XLevel == GLevel && GLevel)
	{
		for (int i = 0; i < MAXPLAYERS; i++)
		{
			if (GGameInfo->Players[i])
			{
				VThinkerChannel* Chan = GGameInfo->Players[i]->Net->ThinkerChannels.FindPtr(this);
				if (Chan)
				{
					Chan->Close();
				}
			}
		}
	}

#ifdef CLIENT
	if (XLevel == GClLevel && GClLevel && cl->Net)
	{
		VThinkerChannel* Chan = cl->Net->ThinkerChannels.FindPtr(this);
		if (Chan)
		{
			Chan->Close();
		}
	}
#endif

	Super::Destroy();
	unguard;
}

//==========================================================================
//
//	VThinker::Serialise
//
//==========================================================================

void VThinker::Serialise(VStream& Strm)
{
	guard(VThinker::Serialise);
	Super::Serialise(Strm);
	if (Strm.IsLoading())
	{
		XLevel->AddThinker(this);
	}
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
	P_PASS_SELF;
	P_PASS_FLOAT(DeltaTime);
	EV_RET_VOID(FIndex_Tick);
	unguard;
}

//==========================================================================
//
//	VThinker::DestroyThinker
//
//==========================================================================

void VThinker::DestroyThinker()
{
	guard(VThinker::DestroyThinker);
	SetFlags(_OF_DelayedDestroy);
	unguard;
}

//==========================================================================
//
//	Script natives
//
//==========================================================================

IMPLEMENT_FUNCTION(VThinker, Spawn)
{
	P_GET_PTR_OPT(mthing_t, mthing, NULL);
	P_GET_AVEC_OPT(AAngles, TAVec(0, 0, 0));
	P_GET_VEC_OPT(AOrigin, TVec(0, 0, 0));
	P_GET_PTR(VClass, Class);
	P_GET_SELF;
	VEntity* SelfEnt = Cast<VEntity>(Self);
	//	If spawner is entity, default to it's origin and angles.
	if (SelfEnt)
	{
		if (!specified_AOrigin)
			AOrigin = SelfEnt->Origin;
		if (!specified_AAngles)
			AAngles = SelfEnt->Angles;
	}
	RET_REF(Self->XLevel->SpawnThinker(Class, AOrigin, AAngles, mthing));
}

IMPLEMENT_FUNCTION(VThinker, Destroy)
{
	P_GET_SELF;
	Self->DestroyThinker();
}

IMPLEMENT_FUNCTION(VThinker, NextThinker)
{
	P_GET_PTR(VClass, Class);
	P_GET_REF(VThinker, th);
	P_GET_SELF;
	if (!th)
	{
		th = Self->XLevel->ThinkerHead;
	}
	else
	{
		th = th->Next;
	}
	VThinker* Ret = NULL;
	while (th)
	{
		if (th->IsA(Class) && !(th->GetFlags() & _OF_DelayedDestroy))
		{
			Ret = th;
			break;
		}
		th = th->Next;
	}
	RET_REF(Ret);
}
