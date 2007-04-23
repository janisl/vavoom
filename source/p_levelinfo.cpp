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
	if (Info.Flags & MAPINFOF_NoSoundClipping)
		LevelInfoFlags |= LIF_NoSoundClipping;
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
