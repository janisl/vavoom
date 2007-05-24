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

int VEntity::FIndex_OnMapSpawn;
int VEntity::FIndex_BeginPlay;
int VEntity::FIndex_Destroyed;
int VEntity::FIndex_Touch;
int VEntity::FIndex_BlockedByLine;
int VEntity::FIndex_ApplyFriction;
int VEntity::FIndex_PushLine;
int VEntity::FIndex_HandleFloorclip;
int VEntity::FIndex_CrossSpecialLine;
int VEntity::FIndex_SectorChanged;
int VEntity::FIndex_RoughCheckThing;
int VEntity::FIndex_GiveInventory;
int VEntity::FIndex_TakeInventory;
int VEntity::FIndex_CheckInventory;
int VEntity::FIndex_GetSigilPieces;
int VEntity::FIndex_MoveThing;
int VEntity::FIndex_GetStateTime;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  VEntity::SetState
//
//==========================================================================

void VEntity::Serialise(VStream& Strm)
{
	guard(VEntity::Serialise);
	Super::Serialise(Strm);

	if (Strm.IsLoading())
	{
		if (EntityFlags & EF_IsPlayer)
		{
			Player->MO = this;
		}
		SubSector = NULL;	//	Must mark as not linked
		LinkToWorld();
	}
	unguard;
}

//==========================================================================
//
//  VEntity::SetState
//
//  Returns true if the actor is still present.
//
//==========================================================================

bool VEntity::SetState(VState* InState)
{
	guard(VEntity::SetState);
	VState *st = InState;
	do
	{
		if (!st)
		{
			// Remove mobj
			State = NULL;
			StateTime = -1;
			DestroyThinker();
			return false;
		}

		State = st;
		StateTime = eventGetStateTime(st, st->Time);
		EntityFlags &= ~EF_FullBright;

		// Modified handling.
		// Call action functions when the state is set
		if (st->Function)
		{
			P_PASS_SELF;
			ExecuteFunction(st->Function);
		}

		if (!State)
		{
			return false;
		}
		st = State->NextState;
	}
	while (!StateTime);
	return true;
	unguard;
}

//==========================================================================
//
//  VEntity::SetInitialState
//
//  Returns true if the actor is still present.
//
//==========================================================================

void VEntity::SetInitialState(VState* InState)
{
	guard(VEntity::SetInitialState);
	State = InState;
	if (InState)
	{
		StateTime = eventGetStateTime(InState, InState->Time);
	}
	else
	{
		StateTime = -1.0;
	}
	unguard;
}

//==========================================================================
//
//	VEntity::AdvanceState
//
//==========================================================================

bool VEntity::AdvanceState(float deltaTime)
{
	guard(VEntity::AdvanceState);
	if (State && StateTime != -1.0)
	{
		StateTime -= deltaTime;
		// you can cycle through multiple states in a tic
		if (StateTime <= 0.0)
		{
			if (!SetState(State->NextState))
			{
				return false;	// freed itself
			}
		}
	}
	return true;
	unguard;
}

//==========================================================================
//
//	VEntity::FindState
//
//==========================================================================

VState* VEntity::FindState(VName StateName)
{
	guard(VEntity::FindState);
	VState* s = GetClass()->FindState(StateName);
	if (!s)
	{
		Host_Error("Can't find state %s", *StateName);
	}
	return s;
	unguard;
}

//==========================================================================
//
//	Entity.SetState
//
//==========================================================================

IMPLEMENT_FUNCTION(VEntity, SetState)
{
	P_GET_PTR(VState, state);
	P_GET_SELF;
	RET_BOOL(Self->SetState(state));
}

IMPLEMENT_FUNCTION(VEntity, SetInitialState)
{
	P_GET_PTR(VState, state);
	P_GET_SELF;
	Self->SetInitialState(state);
}

IMPLEMENT_FUNCTION(VEntity, AdvanceState)
{
	P_GET_FLOAT(deltaTime);
	P_GET_SELF;
	RET_BOOL(Self->AdvanceState(deltaTime));
}

IMPLEMENT_FUNCTION(VEntity, FindState)
{
	P_GET_NAME(StateName);
	P_GET_SELF;
	RET_PTR(Self->FindState(StateName));
}

//==========================================================================
//
//	VEntity::InitFuncIndexes
//
//==========================================================================

void VEntity::InitFuncIndexes()
{
	guard(VEntity::InitFuncIndexes);
	FIndex_OnMapSpawn = StaticClass()->GetFunctionIndex(NAME_OnMapSpawn);
	FIndex_BeginPlay = StaticClass()->GetFunctionIndex(NAME_BeginPlay);
	FIndex_Destroyed = StaticClass()->GetFunctionIndex(NAME_Destroyed);
	FIndex_Touch = StaticClass()->GetFunctionIndex(NAME_Touch);
	FIndex_BlockedByLine = StaticClass()->GetFunctionIndex(NAME_BlockedByLine);
	FIndex_ApplyFriction = StaticClass()->GetFunctionIndex(NAME_ApplyFriction);
	FIndex_PushLine = StaticClass()->GetFunctionIndex(NAME_PushLine);
	FIndex_HandleFloorclip = StaticClass()->GetFunctionIndex(NAME_HandleFloorclip);
	FIndex_CrossSpecialLine = StaticClass()->GetFunctionIndex(NAME_CrossSpecialLine);
	FIndex_SectorChanged = StaticClass()->GetFunctionIndex(NAME_SectorChanged);
	FIndex_RoughCheckThing = StaticClass()->GetFunctionIndex(NAME_RoughCheckThing);
	FIndex_GiveInventory = StaticClass()->GetFunctionIndex(NAME_GiveInventory);
	FIndex_TakeInventory = StaticClass()->GetFunctionIndex(NAME_TakeInventory);
	FIndex_CheckInventory = StaticClass()->GetFunctionIndex(NAME_CheckInventory);
	FIndex_GetSigilPieces = StaticClass()->GetFunctionIndex(NAME_GetSigilPieces);
	FIndex_MoveThing = StaticClass()->GetFunctionIndex(NAME_MoveThing);
	FIndex_GetStateTime = StaticClass()->GetFunctionIndex(NAME_GetStateTime);
	unguard;
}

//==========================================================================
//
//	EntInit
//
//==========================================================================

void EntInit()
{
	VEntity::InitFuncIndexes();
}

//==========================================================================
//
//	SV_FindClassFromEditorId
//
//==========================================================================

VClass* SV_FindClassFromEditorId(int Id)
{
	guard(SV_FindClassFromEditorId);
	for (int i = 0; i < VClass::GMobjInfos.Num(); i++)
	{
		if (Id == VClass::GMobjInfos[i].doomednum)
		{
			return VClass::GMobjInfos[i].class_id;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	SV_FindClassFromScriptId
//
//==========================================================================

VClass* SV_FindClassFromScriptId(int Id)
{
	guard(SV_FindClassFromScriptId);
	for (int i = 0; i < VClass::GScriptIds.Num(); i++)
	{
		if (Id == VClass::GScriptIds[i].doomednum)
		{
			return VClass::GScriptIds[i].class_id;
		}
	}
	return NULL;
	unguard;
}
