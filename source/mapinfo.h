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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	MAPINFOF_DoubleSky		= 0x00000001,	// parallax sky: sky2 behind sky1
	MAPINFOF_Lightning		= 0x00000002,	// Use of lightning on the level flashes from sky1 to sky2
};

struct mapInfo_t
{
	VName		LumpName;
	VStr		Name;			// Name of map
	vint32		LevelNum;		// Level number for action specials
	vint32		Cluster;		// Defines what cluster level belongs to
	vint32		WarpTrans;		// Actual map number in case maps are not sequential
	VName		NextMap;		// Map to teleport to upon exit of timed deathmatch
	VName		SecretMap;		// Map to teleport upon secret exit
	VName		SongLump;		// Background music (MUS or MIDI)
	vint32		CDTrack;		// CD track to play during level
	vint32		Sky1Texture;	// Default sky texture
	vint32		Sky2Texture;	// Alternate sky displayed in Sky2 sectors
	float		Sky1ScrollDelta;// Default sky texture speed
	float		Sky2ScrollDelta;// Alternate sky texture speed
	VName		SkyBox;			// Sky box
	VName		FadeTable;		// Fade table {fogmap}
	float		Gravity;		// Map gravity
	vuint32		Flags;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void InitMapInfo();
void ShutdownMapInfo();
const mapInfo_t& P_GetMapInfo(VName);
const char* P_GetMapName(int);
VName P_GetMapLumpName(int);
VName P_TranslateMap(int);
VName P_GetMapNameByLevelNum(int);
void P_PutMapSongLump(int, VName);
int P_GetCDStartTrack();
int P_GetCDEnd1Track();
int P_GetCDEnd2Track();
int P_GetCDEnd3Track();
int P_GetCDIntermissionTrack();
int P_GetCDTitleTrack();

// PUBLIC DATA DECLARATIONS ------------------------------------------------
