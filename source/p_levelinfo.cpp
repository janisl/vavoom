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

IMPLEMENT_CLASS(V, LevelInfo)

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VLevelInfo::VLevelInfo
//
//==========================================================================

VLevelInfo::VLevelInfo()
{
	Level = this;
}

//==========================================================================
//
//	VLevelInfo::SetMapInfo
//
//==========================================================================

void VLevelInfo::SetMapInfo(const mapInfo_t& Info)
{
	guard(VLevelInfo::SetMapInfo);
	LevelName = Info.Name;
	LevelNum = Info.LevelNum;
	Cluster = Info.Cluster;

	NextMap = Info.NextMap;
	SecretMap = Info.SecretMap;

	ParTime = Info.ParTime;
	SuckTime = Info.SuckTime;

	Sky1Texture = Info.Sky1Texture;
	Sky2Texture = Info.Sky2Texture;
	Sky1ScrollDelta = Info.Sky1ScrollDelta;
	Sky2ScrollDelta = Info.Sky2ScrollDelta;
	SkyBox = Info.SkyBox;

	FadeTable = Info.FadeTable;
	Fade = Info.Fade;
	OutsideFog = Info.OutsideFog;

	SongLump = Info.SongLump;
	CDTrack = Info.CDTrack;

	//	Copy flags from mapinfo.
	if (Info.Flags & MAPINFOF_DoubleSky)
		LevelInfoFlags |= LIF_DoubleSky;
	if (Info.Flags & MAPINFOF_Lightning)
		LevelInfoFlags |= LIF_Lightning;
	if (Info.Flags & MAPINFOF_Map07Special)
		LevelInfoFlags |= LIF_Map07Special;
	if (Info.Flags & MAPINFOF_BaronSpecial)
		LevelInfoFlags |= LIF_BaronSpecial;
	if (Info.Flags & MAPINFOF_CyberDemonSpecial)
		LevelInfoFlags |= LIF_CyberDemonSpecial;
	if (Info.Flags & MAPINFOF_SpiderMastermindSpecial)
		LevelInfoFlags |= LIF_SpiderMastermindSpecial;
	if (Info.Flags & MAPINFOF_MinotaurSpecial)
		LevelInfoFlags |= LIF_MinotaurSpecial;
	if (Info.Flags & MAPINFOF_DSparilSpecial)
		LevelInfoFlags |= LIF_DSparilSpecial;
	if (Info.Flags & MAPINFOF_IronLichSpecial)
		LevelInfoFlags |= LIF_IronLichSpecial;
	if (Info.Flags & MAPINFOF_SpecialActionOpenDoor)
		LevelInfoFlags |= LIF_SpecialActionOpenDoor;
	if (Info.Flags & MAPINFOF_SpecialActionLowerFloor)
		LevelInfoFlags |= LIF_SpecialActionLowerFloor;
	if (Info.Flags & MAPINFOF_SpecialActionKillMonsters)
		LevelInfoFlags |= LIF_SpecialActionKillMonsters;
	if (Info.Flags & MAPINFOF_NoIntermission)
		LevelInfoFlags |= LIF_NoIntermission;
	if (Info.Flags & MAPINFOF_AllowMonsterTelefrags)
		LevelInfoFlags |= LIF_AllowMonsterTelefrags;
	if (Info.Flags & MAPINFOF_NoAllies)
		LevelInfoFlags |= LIF_NoAllies;
	if (Info.Flags & MAPINFOF_DeathSlideShow)
		LevelInfoFlags |= LIF_DeathSlideShow;
	if (Info.Flags & MAPINFOF_ForceNoSkyStretch)
		LevelInfoFlags |= LIF_ForceNoSkyStretch;
	if (Info.Flags & MAPINFOF_LookupName)
		LevelInfoFlags |= LIF_LookupName;
	if (Info.Flags & MAPINFOF_FallingDamage)
		LevelInfoFlags |= LIF_FallingDamage;
	if (Info.Flags & MAPINFOF_OldFallingDamage)
		LevelInfoFlags |= LIF_OldFallingDamage;
	if (Info.Flags & MAPINFOF_StrifeFallingDamage)
		LevelInfoFlags |= LIF_StrifeFallingDamage;
	unguard;
}

//==========================================================================
//
//	VLevelInfo::SectorStartSound
//
//==========================================================================

void VLevelInfo::SectorStartSound(const sector_t* Sector, int SoundId,
	int Channel, float Volume, float Attenuation)
{
	guard(VLevelInfo::SectorStartSound);
	if (Sector)
	{
		StartSound(Sector->soundorg, (Sector - XLevel->Sectors) +
			GMaxEntities, SoundId, Channel, Volume, Attenuation);
	}
	else
	{
		StartSound(TVec(0, 0, 0), 0, SoundId, Channel, Volume,
			Attenuation);
	}
	unguard;
}

//==========================================================================
//
//	VLevelInfo::SectorStopSound
//
//==========================================================================

void VLevelInfo::SectorStopSound(const sector_t *sector, int channel)
{
	guard(VLevelInfo::SectorStopSound);
	StopSound((sector - XLevel->Sectors) + GMaxEntities, channel);
	unguard;
}

//==========================================================================
//
//	VLevelInfo::SectorStartSequence
//
//==========================================================================

void VLevelInfo::SectorStartSequence(const sector_t* Sector, VName Name,
	int ModeNum)
{
	guard(VLevelInfo::SectorStartSequence);
	if (Sector)
	{
		StartSoundSequence(Sector->soundorg, (Sector - XLevel->Sectors) +
			GMaxEntities, Name, ModeNum);
	}
	else
	{
		StartSoundSequence(TVec(0, 0, 0), 0, Name, ModeNum);
	}
	unguard;
}

//==========================================================================
//
//	VLevelInfo::SectorStopSequence
//
//==========================================================================

void VLevelInfo::SectorStopSequence(const sector_t *sector)
{
	guard(VLevelInfo::SectorStopSequence);
	StopSoundSequence((sector - XLevel->Sectors) + GMaxEntities);
	unguard;
}

//==========================================================================
//
//	VLevelInfo::PolyobjStartSequence
//
//==========================================================================

void VLevelInfo::PolyobjStartSequence(const polyobj_t* Poly, VName Name,
	int ModeNum)
{
	guard(VLevelInfo::PolyobjStartSequence);
	StartSoundSequence(Poly->startSpot, (Poly - XLevel->PolyObjs) +
		GMaxEntities + XLevel->NumSectors, Name, ModeNum);
	unguard;
}

//==========================================================================
//
//	VLevelInfo::PolyobjStopSequence
//
//==========================================================================

void VLevelInfo::PolyobjStopSequence(const polyobj_t *poly)
{
	guard(VLevelInfo::PolyobjStopSequence);
	StopSoundSequence((poly - XLevel->PolyObjs) + GMaxEntities + XLevel->NumSectors);
	unguard;
}

//==========================================================================
//
//	VLevelInfo::ExitLevel
//
//==========================================================================

void VLevelInfo::ExitLevel(int Position)
{
	guard(VLevelInfo::ExitLevel);
	LeavePosition = Position;
	completed = true;
	unguard;
}

//==========================================================================
//
//	VLevelInfo::SecretExitLevel
//
//==========================================================================

void VLevelInfo::SecretExitLevel(int Position)
{
	guard(VLevelInfo::SecretExitLevel);
	if (SecretMap == NAME_None)
	{
		// No secret map, use normal exit
		ExitLevel(Position);
		return;
	}

	LeavePosition = Position;
	completed = true;

	NextMap = SecretMap; 	// go to secret level

	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (Game->Players[i])
		{
			Game->Players[i]->PlayerFlags |= VBasePlayer::PF_DidSecret;
		}
	}
	unguard;
} 

//==========================================================================
//
//	VLevelInfo::Completed
//
//	Starts intermission routine, which is used only during hub exits,
// and DeathMatch games.
//
//==========================================================================

void VLevelInfo::Completed(int InMap, int InPosition, int SaveAngle)
{
	guard(VLevelInfo::Completed);
	int Map = InMap;
	int Position = InPosition;
	if (Map == -1 && Position == -1)
	{
		if (!deathmatch)
		{
			for (int i = 0; i < svs.max_clients; i++)
			{
				if (Game->Players[i])
				{
					Game->Players[i]->eventClientFinale(
						VStr(NextMap).StartsWith("EndGame") ? *NextMap : "");
				}
			}
			sv.intermission = 2;
			return;
		}
		Map = 1;
		Position = 0;
	}
	NextMap = P_GetMapNameByLevelNum(Map);

	LeavePosition = Position;
	completed = true;
	unguard;
}

//==========================================================================
//
//	VLevelInfo::ForceLightning
//
//==========================================================================

void VLevelInfo::ForceLightning()
{
	guard(VLevelInfo::ForceLightning);
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (!Game->Players[i])
			continue;
		if (!(Game->Players[i]->PlayerFlags & VBasePlayer::PF_Spawned))
			continue;
		Game->Players[i]->eventClientForceLightning();
	}
	unguard;
}

//==========================================================================
//
//	VLevelInfo natives
//
//==========================================================================

IMPLEMENT_FUNCTION(VLevelInfo, AddStaticLight)
{
	P_GET_FLOAT(Radius);
	P_GET_VEC(Origin);
	P_GET_SELF;
	rep_light_t* OldLights = Self->XLevel->StaticLights;
	Self->XLevel->NumStaticLights++;
	Self->XLevel->StaticLights = new rep_light_t[Self->XLevel->NumStaticLights];
	if (OldLights)
	{
		memcpy(Self->XLevel->StaticLights, OldLights,
			(Self->XLevel->NumStaticLights - 1) * sizeof(rep_light_t));
		delete[] OldLights;
	}
	rep_light_t& L = Self->XLevel->StaticLights[Self->XLevel->NumStaticLights - 1];
	L.Origin = Origin;
	L.Radius = Radius;
	L.Colour = 0xffffffff;
}

IMPLEMENT_FUNCTION(VLevelInfo, AddStaticLightRGB)
{
	P_GET_INT(Colour);
	P_GET_FLOAT(Radius);
	P_GET_VEC(Origin);
	P_GET_SELF;
	rep_light_t* OldLights = Self->XLevel->StaticLights;
	Self->XLevel->NumStaticLights++;
	Self->XLevel->StaticLights = new rep_light_t[Self->XLevel->NumStaticLights];
	if (OldLights)
	{
		memcpy(Self->XLevel->StaticLights, OldLights,
			(Self->XLevel->NumStaticLights - 1) * sizeof(rep_light_t));
		delete[] OldLights;
	}
	rep_light_t& L = Self->XLevel->StaticLights[Self->XLevel->NumStaticLights - 1];
	L.Origin = Origin;
	L.Radius = Radius;
	L.Colour = Colour;
}

IMPLEMENT_FUNCTION(VLevelInfo, SectorStartSequence)
{
	P_GET_INT(ModeNum);
	P_GET_NAME(name);
	P_GET_PTR(sector_t, sec);
	P_GET_SELF;
	Self->SectorStartSequence(sec, name, ModeNum);
}

IMPLEMENT_FUNCTION(VLevelInfo, SectorStopSequence)
{
	P_GET_PTR(sector_t, sec);
	P_GET_SELF;
	Self->SectorStopSequence(sec);
}

IMPLEMENT_FUNCTION(VLevelInfo, PolyobjStartSequence)
{
	P_GET_INT(ModeNum);
	P_GET_NAME(name);
	P_GET_PTR(polyobj_t, poly);
	P_GET_SELF;
	Self->PolyobjStartSequence(poly, name, ModeNum);
}

IMPLEMENT_FUNCTION(VLevelInfo, PolyobjStopSequence)
{
	P_GET_PTR(polyobj_t, poly);
	P_GET_SELF;
	Self->PolyobjStopSequence(poly);
}

IMPLEMENT_FUNCTION(VLevelInfo, ExitLevel)
{
	P_GET_INT(Position);
	P_GET_SELF;
	Self->ExitLevel(Position);
}

IMPLEMENT_FUNCTION(VLevelInfo, SecretExitLevel)
{
	P_GET_INT(Position);
	P_GET_SELF;
	Self->SecretExitLevel(Position);
}

IMPLEMENT_FUNCTION(VLevelInfo, Completed)
{
	P_GET_INT(SaveAngle);
	P_GET_INT(pos);
	P_GET_INT(map);
	P_GET_SELF;
	Self->Completed(map, pos, SaveAngle);
}

IMPLEMENT_FUNCTION(VLevelInfo, ChangeSwitchTexture)
{
	P_GET_NAME(DefaultSound);
	P_GET_BOOL(useAgain);
	P_GET_PTR(line_t, line);
	P_GET_SELF;
	Self->ChangeSwitchTexture(line, useAgain, DefaultSound);
}

IMPLEMENT_FUNCTION(VLevelInfo, ForceLightning)
{
	P_GET_SELF;
	Self->ForceLightning();
}
