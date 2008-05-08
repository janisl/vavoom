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

static VCvarF	sv_gravity("sv_gravity", "800.0", CVAR_ServerInfo);
static VCvarF	sv_aircontrol("sv_aircontrol", "0.00390625", CVAR_ServerInfo);

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
	const VClusterDef* CInfo = P_GetClusterDef(Info.Cluster);

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

	if (Info.Gravity)
	{
		Gravity = Info.Gravity * DEFAULT_GRAVITY / 800.0;
	}
	else
	{
		Gravity = sv_gravity * DEFAULT_GRAVITY / 800.0;
	}

	if (Info.AirControl)
	{
		AirControl = Info.AirControl;
	}
	else
	{
		AirControl = sv_aircontrol;
	}

	//	Copy flags from mapinfo.
	LevelInfoFlags = Info.Flags & ~(LIF_ClusterHub | LIF_BegunPlay);

	if (CInfo->Flags & CLUSTERF_Hub)
	{
		LevelInfoFlags |= LIF_ClusterHub;
	}

	//	No auto sequences flag sets all sectors to use sequence 0 by
	// default.
	if (Info.Flags & MAPINFOF_NoAutoSndSeq)
	{
		for (int i = 0; i < XLevel->NumSectors; i++)
		{
			XLevel->Sectors[i].seqType = 0;
		}
	}

	if (Info.Flags & MAPINFOF_ClipMidTex)
	{
		for (int i = 0; i < XLevel->NumLines; i++)
		{
			XLevel->Lines[i].flags |= ML_CLIP_MIDTEX;
		}
	}
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
		if (Sector->SectorFlags & sector_t::SF_Silent)
		{
			return;
		}
		StartSound(Sector->soundorg, (Sector - XLevel->Sectors) +
			(SNDORG_Sector << 24), SoundId, Channel, Volume, Attenuation,
			false);
	}
	else
	{
		StartSound(TVec(0, 0, 0), 0, SoundId, Channel, Volume,
			Attenuation, false);
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
	StopSound((sector - XLevel->Sectors) + (SNDORG_Sector << 24), channel);
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
		if (Sector->SectorFlags & sector_t::SF_Silent)
		{
			return;
		}
		StartSoundSequence(Sector->soundorg, (Sector - XLevel->Sectors) +
			(SNDORG_Sector << 24), Name, ModeNum);
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
	StopSoundSequence((sector - XLevel->Sectors) + (SNDORG_Sector << 24));
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
	if (Poly->subsector->sector->SectorFlags & sector_t::SF_Silent)
	{
		return;
	}
	StartSoundSequence(Poly->startSpot, (Poly - XLevel->PolyObjs) +
		(SNDORG_PolyObj << 24), Name, ModeNum);
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
	StopSoundSequence((poly - XLevel->PolyObjs) + (SNDORG_PolyObj << 24));
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
//	VLevelInfo::FindMobjFromTID
//
//==========================================================================

VEntity* VLevelInfo::FindMobjFromTID(int tid, VEntity* Prev)
{
	guard(VLevelInfo::FindMobjFromTID);
	for (VEntity* E = Prev ? Prev->TIDHashNext : TIDHash[tid &
		(TID_HASH_SIZE - 1)]; E; E = E->TIDHashNext)
	{
		if (E->TID == tid)
		{
			return E;
		}
	}
	return NULL;
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
	P_GET_PTR(vuint8, pQuest);
	P_GET_NAME(DefaultSound);
	P_GET_BOOL(useAgain);
	P_GET_INT(SideNum);
	P_GET_SELF;
	bool Quest;
	bool Ret = Self->ChangeSwitchTexture(SideNum, useAgain, DefaultSound,
		Quest);
	*pQuest = Quest;
	RET_BOOL(Ret);
}

IMPLEMENT_FUNCTION(VLevelInfo, ForceLightning)
{
	P_GET_SELF;
	Self->ForceLightning();
}

IMPLEMENT_FUNCTION(VLevelInfo, FindMobjFromTID)
{
	P_GET_REF(VEntity, Prev);
	P_GET_INT(tid);
	P_GET_SELF;
	RET_REF(Self->FindMobjFromTID(tid, Prev));
}
