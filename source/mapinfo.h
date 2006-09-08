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

struct mapInfo_t
{
	VName		LumpName;
	vint32		LevelNum;		// Level number for action specials
	int			cluster;		// Defines what cluster level belongs to
	int			warpTrans;		// Actual map number in case maps are not sequential
	VName		NextMap;		// Map to teleport to upon exit of timed deathmatch
	VName		SecretMap;		// Map to teleport upon secret exit
	int			cdTrack;		// CD track to play during level
	char		name[32];		// Name of map
	int			sky1Texture;	// Default sky texture
	int			sky2Texture;	// Alternate sky displayed in Sky2 sectors
	float		sky1ScrollDelta;// Default sky texture speed
	float		sky2ScrollDelta;// Alternate sky texture speed
	boolean		doubleSky;		// parallax sky: sky2 behind sky1
	boolean		lightning;		// Use of lightning on the level flashes from sky1 to sky2
	VName		FadeTable;		// Fade table {fogmap}
	VName		SongLump;		// Background music (MUS or MIDI)
	VName		SkyBox;			// Sky box
	float		Gravity;		// Map gravity
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void InitMapInfo();
void ShutdownMapInfo();
void P_GetMapInfo(VName, mapInfo_t&);
char* P_GetMapName(int);
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
