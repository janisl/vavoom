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
	//	Close any thinker channels.
	if (XLevel)
	{
		XLevel->NetContext->ThinkerDestroyed(this);
	}

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
	EV_RET_VOID_IDX(FIndex_Tick);
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
//	VThinker::AddedToLevel
//
//==========================================================================

void VThinker::AddedToLevel()
{
}

//==========================================================================
//
//	VThinker::RemovedFromLevel
//
//==========================================================================

void VThinker::RemovedFromLevel()
{
}

//==========================================================================
//
//	VThinker::StartSound
//
//==========================================================================

void VThinker::StartSound(const TVec &origin, vint32 origin_id,
	vint32 sound_id, vint32 channel, float volume, float Attenuation)
{
	guard(VThinker::StartSound);
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (!Level->Game->Players[i])
			continue;
		if (!(Level->Game->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned))
			continue;
		Level->Game->Players[i]->eventClientStartSound(sound_id, origin,
			origin_id, channel, volume, Attenuation);
	}
	unguard;
}

//==========================================================================
//
//	VThinker::StopSound
//
//==========================================================================

void VThinker::StopSound(vint32 origin_id, vint32 channel)
{
	guard(VThinker::StopSound);
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (!Level->Game->Players[i])
			continue;
		if (!(Level->Game->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned))
			continue;
		Level->Game->Players[i]->eventClientStopSound(origin_id, channel);
	}
	unguard;
}

//==========================================================================
//
//	VThinker::StartSoundSequence
//
//==========================================================================

void VThinker::StartSoundSequence(const TVec& Origin, vint32 OriginId,
	VName Name, vint32 ModeNum)
{
	guard(VThinker::StartSoundSequence);
	//	Remove any existing sequences of this origin
	for (int i = 0; i < sv_ActiveSequences.Num(); i++)
	{
		if (sv_ActiveSequences[i].OriginId == OriginId)
		{
			sv_ActiveSequences.RemoveIndex(i);
			i--;
		}
	}
	VSndSeqInfo& Seq = sv_ActiveSequences.Alloc();
	Seq.Name = Name;
	Seq.OriginId = OriginId;
	Seq.Origin = Origin;
	Seq.ModeNum = ModeNum;

	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (!Level->Game->Players[i])
			continue;
		if (!(Level->Game->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned))
			continue;
		Level->Game->Players[i]->eventClientStartSequence(Origin, OriginId,
			Name, ModeNum);
	}
	unguard;
}

//==========================================================================
//
//	VThinker::AddSoundSequenceChoice
//
//==========================================================================

void VThinker::AddSoundSequenceChoice(int origin_id, VName Choice)
{
	guard(VThinker::AddSoundSequenceChoice);
	//	Remove it from server's sequences list.
	for (int i = 0; i < sv_ActiveSequences.Num(); i++)
	{
		if (sv_ActiveSequences[i].OriginId == origin_id)
		{
			sv_ActiveSequences[i].Choices.Append(Choice);
		}
	}

	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (!Level->Game->Players[i])
			continue;
		if (!(Level->Game->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned))
			continue;
		Level->Game->Players[i]->eventClientAddSequenceChoice(origin_id,
			Choice);
	}
	unguard;
}

//==========================================================================
//
//	VThinker::StopSoundSequence
//
//==========================================================================

void VThinker::StopSoundSequence(int origin_id)
{
	guard(VThinker::StopSoundSequence);
	//	Remove it from server's sequences list.
	for (int i = 0; i < sv_ActiveSequences.Num(); i++)
	{
		if (sv_ActiveSequences[i].OriginId == origin_id)
		{
			sv_ActiveSequences.RemoveIndex(i);
			i--;
		}
	}

	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (!Level->Game->Players[i])
			continue;
		if (!(Level->Game->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned))
			continue;
		Level->Game->Players[i]->eventClientStopSequence(origin_id);
	}
	unguard;
}

//==========================================================================
//
//	VThinker::BroadcastPrintf
//
//==========================================================================

void VThinker::BroadcastPrintf(const char *s, ...)
{
	guard(VThinker::BroadcastPrintf);
	va_list	v;
	char	buf[1024];

	va_start(v, s);
	vsprintf(buf, s, v);
	va_end(v);

	for (int i = 0; i < svs.max_clients; i++)
		if (Level->Game->Players[i])
			Level->Game->Players[i]->eventClientPrint(buf);
	unguard;
}

//==========================================================================
//
//	VThinker::BroadcastCentrePrintf
//
//==========================================================================

void VThinker::BroadcastCentrePrintf(const char *s, ...)
{
	guard(VThinker::BroadcastCentrePrintf);
	va_list	v;
	char	buf[1024];

	va_start(v, s);
	vsprintf(buf, s, v);
	va_end(v);

	for (int i = 0; i < svs.max_clients; i++)
		if (Level->Game->Players[i])
			Level->Game->Players[i]->eventClientCentrePrint(buf);
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

IMPLEMENT_FUNCTION(VThinker, bprint)
{
	VStr Msg = PF_FormatString();
	P_GET_SELF;
	Self->BroadcastPrintf(*Msg);
}

IMPLEMENT_FUNCTION(VThinker, AllocDlight)
{
	P_GET_REF(VThinker, Owner);
	P_GET_SELF;
	RET_PTR(Self->XLevel->RenderData->AllocDlight(Owner));
}

IMPLEMENT_FUNCTION(VThinker, NewParticle)
{
	P_GET_SELF;
	RET_PTR(Self->XLevel->RenderData->NewParticle());
}
