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

// CODE --------------------------------------------------------------------

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
//	Entity.StartSoundSequence
//
//==========================================================================

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
