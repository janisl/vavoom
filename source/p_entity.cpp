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
//#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(V, Entity);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

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

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VEntity::InitFuncIndexes
//
//==========================================================================

void VEntity::InitFuncIndexes()
{
	guard(VEntity::InitFuncIndexes);
	FIndex_OnMapSpawn = StaticClass()->GetMethodIndex(NAME_OnMapSpawn);
	FIndex_BeginPlay = StaticClass()->GetMethodIndex(NAME_BeginPlay);
	FIndex_Destroyed = StaticClass()->GetMethodIndex(NAME_Destroyed);
	FIndex_Touch = StaticClass()->GetMethodIndex(NAME_Touch);
	FIndex_BlockedByLine = StaticClass()->GetMethodIndex(NAME_BlockedByLine);
	FIndex_ApplyFriction = StaticClass()->GetMethodIndex(NAME_ApplyFriction);
	FIndex_PushLine = StaticClass()->GetMethodIndex(NAME_PushLine);
	FIndex_HandleFloorclip = StaticClass()->GetMethodIndex(NAME_HandleFloorclip);
	FIndex_CrossSpecialLine = StaticClass()->GetMethodIndex(NAME_CrossSpecialLine);
	FIndex_SectorChanged = StaticClass()->GetMethodIndex(NAME_SectorChanged);
	FIndex_RoughCheckThing = StaticClass()->GetMethodIndex(NAME_RoughCheckThing);
	FIndex_GiveInventory = StaticClass()->GetMethodIndex(NAME_GiveInventory);
	FIndex_TakeInventory = StaticClass()->GetMethodIndex(NAME_TakeInventory);
	FIndex_CheckInventory = StaticClass()->GetMethodIndex(NAME_CheckInventory);
	FIndex_GetSigilPieces = StaticClass()->GetMethodIndex(NAME_GetSigilPieces);
	FIndex_MoveThing = StaticClass()->GetMethodIndex(NAME_MoveThing);
	FIndex_GetStateTime = StaticClass()->GetMethodIndex(NAME_GetStateTime);
	unguard;
}

//==========================================================================
//
//  VEntity::Serialise
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
//	VEntity::DestroyThinker
//
//==========================================================================

void VEntity::DestroyThinker()
{
	guard(VEntity::DestroyThinker)
	if (Role == ROLE_Authority)
	{
		eventDestroyed();

		// stop any playing sound
		StopSound(0);
	}

	// unlink from sector and block lists
	UnlinkFromWorld();
	XLevel->DelSectorList();

	Super::DestroyThinker();
	unguard;
}

//==========================================================================
//
//	VEntity::AddedToLevel
//
//==========================================================================

void VEntity::AddedToLevel()
{
	guard(VEntity::AddedToLevel);
	//	Find an available net ID.
	int Id = 0;
	bool Used = false;
	do
	{
		Id++;
		Used = false;
		for (TThinkerIterator<VEntity> Other(XLevel); Other; ++Other)
		{
			if (Other->NetID == Id)
			{
				Used = true;
				break;
			}
		}
	}
	while (Used);
	NetID = Id;
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
	VStateLabel* Lbl = GetClass()->FindStateLabel(StateName);
	return Lbl ? Lbl->State : NULL;
	unguard;
}

//==========================================================================
//
//	VEntity::StartSound
//
//==========================================================================

void VEntity::StartSound(VName Sound, vint32 Channel, float Volume,
	float Attenuation)
{
	guard(VEntity::StartSound);
	Super::StartSound(Origin, NetID, GSoundManager->ResolveEntitySound(
		SoundClass, SoundGender, Sound), Channel, Volume, Attenuation);
	unguard;
}

//==========================================================================
//
//	VEntity::StartLocalSound
//
//==========================================================================

void VEntity::StartLocalSound(VName Sound, vint32 Channel, float Volume,
	float Attenuation)
{
	guard(VEntity::StartLocalSound);
	if (Player)
	{
		Player->eventClientStartSound(
			GSoundManager->ResolveEntitySound(SoundClass, SoundGender, Sound),
			TVec(0, 0, 0), 0, Channel, Volume, Attenuation);
	}
	unguard;
}

//==========================================================================
//
//	VEntity::StopSound
//
//==========================================================================

void VEntity::StopSound(vint32 channel)
{
	guard(VEntity::StopSound);
	Super::StopSound(NetID, channel);
	unguard;
}

//==========================================================================
//
//	VEntity::StartSoundSequence
//
//==========================================================================

void VEntity::StartSoundSequence(VName Name, vint32 ModeNum)
{
	guard(VEntity::StartSoundSequence);
	Super::StartSoundSequence(Origin, NetID, Name, ModeNum);
	unguard;
}

//==========================================================================
//
//	VEntity::AddSoundSequenceChoice
//
//==========================================================================

void VEntity::AddSoundSequenceChoice(VName Choice)
{
	guard(VEntity::AddSoundSequenceChoice);
	Super::AddSoundSequenceChoice(NetID, Choice);
	unguard;
}

//==========================================================================
//
//	VEntity::StopSoundSequence
//
//==========================================================================

void VEntity::StopSoundSequence()
{
	guard(VEntity::StopSoundSequence);
	Super::StopSoundSequence(NetID);
	unguard;
}

//==========================================================================
//
//	Script natives
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

IMPLEMENT_FUNCTION(VEntity, PlaySound)
{
	P_GET_FLOAT_OPT(Attenuation, 1.0);
	P_GET_FLOAT_OPT(Volume, 1.0);
	P_GET_INT(Channel);
	P_GET_NAME(SoundName);
	P_GET_SELF;
	Self->StartSound(SoundName, Channel, Volume, Attenuation);
}

IMPLEMENT_FUNCTION(VEntity, StopSound)
{
	P_GET_INT(Channel);
	P_GET_SELF;
	Self->StopSound(Channel);
}

IMPLEMENT_FUNCTION(VEntity, AreSoundsEquivalent)
{
	P_GET_NAME(Sound2);
	P_GET_NAME(Sound1);
	P_GET_SELF;
	RET_BOOL(GSoundManager->ResolveEntitySound(Self->SoundClass,
		Self->SoundGender, Sound1) == GSoundManager->ResolveEntitySound(
		Self->SoundClass, Self->SoundGender, Sound2));
}

IMPLEMENT_FUNCTION(VEntity, StartSoundSequence)
{
	P_GET_INT(ModeNum);
	P_GET_NAME(Name);
	P_GET_SELF;
	Self->StartSoundSequence(Name, ModeNum);
}

IMPLEMENT_FUNCTION(VEntity, AddSoundSequenceChoice)
{
	P_GET_NAME(Choice);
	P_GET_SELF;
	Self->AddSoundSequenceChoice(Choice);
}

IMPLEMENT_FUNCTION(VEntity, StopSoundSequence)
{
	P_GET_SELF;
	Self->StopSoundSequence();
}
