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

#define MAX_MAPS			99

#define UNKNOWN_MAP_NAME	"DEVELOPMENT MAP"
#define DEFAULT_SKY_NAME	"SKY1"
#define DEFAULT_FADE_TABLE	"colormap"

enum
{
	CD_STARTTRACK,
	CD_END1TRACK,
	CD_END2TRACK,
	CD_END3TRACK,
	CD_INTERTRACK,
	CD_TITLETRACK,
};

// TYPES -------------------------------------------------------------------

struct FMapSongInfo
{
	VName	MapName;
	VName	SongName;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

VName P_TranslateMap(int map);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void ParseMapInfo(VScriptParser* sc);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static mapInfo_t	MapInfo[MAX_MAPS];
static int 			MapCount;

static int cd_NonLevelTracks[6]; // Non-level specific song cd track numbers 

static TArray<FMapSongInfo>		MapSongList;

// CODE --------------------------------------------------------------------

//==========================================================================
//
// InitMapInfo
//
//==========================================================================

void InitMapInfo()
{
	guard(InitMapInfo);
	mapInfo_t 	*info;

	MapCount = 1;

	// Put defaults into MapInfo[0]
	info = MapInfo;
	info->cluster = 0;
	info->warpTrans = 0;
	info->NextMap = "map01"; // Always go to map 1 if not specified
	info->cdTrack = 1;
	info->sky1Texture = GTextureManager.CheckNumForName(
		VName(DEFAULT_SKY_NAME, VName::AddLower8),
		TEXTYPE_Wall, true, false);
	if (info->sky1Texture < 0)
		info->sky1Texture = GTextureManager.CheckNumForName(
			VName("SKYMNT02", VName::AddLower8),
			TEXTYPE_Wall, true, false);
	info->sky2Texture = info->sky1Texture;
	info->sky1ScrollDelta = 0.0;
	info->sky2ScrollDelta = 0.0;
	info->doubleSky = false;
	info->lightning = false;
	info->FadeTable = DEFAULT_FADE_TABLE;
	info->Gravity = 0.0;
	VStr::Cpy(info->name, UNKNOWN_MAP_NAME);

	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) == NAME_mapinfo)
		{
			ParseMapInfo(new VScriptParser(*W_LumpName(Lump),
				W_CreateLumpReaderNum(Lump)));
		}
	}
	//	Optionally parse script file.
	if (fl_devmode && FL_FileExists("scripts/mapinfo.txt"))
	{
		ParseMapInfo(new VScriptParser("scripts/mapinfo.txt",
			FL_OpenFileRead("scripts/mapinfo.txt")));
	}

	for (int i = 1; i < MapCount; i++)
	{
		if ((*MapInfo[i].NextMap)[0] >= '0' &&
			(*MapInfo[i].NextMap)[0] <= '9')
		{
			MapInfo[i].NextMap = P_TranslateMap(atoi(*MapInfo[i].NextMap));
		}
	}
	unguard;
}

//==========================================================================
//
//	ParseMap
//
//==========================================================================

static void ParseMap(VScriptParser* sc)
{
	VName MapLumpName;
	if (sc->CheckNumber())
	{
		//	Map number, for Hexen compatibility
		if (sc->Number < 1 || sc->Number > 99)
		{
			sc->Error("Map number out or range");
		}
		MapLumpName = va("map%02d", sc->Number);
	}
	else
	{
		//	Map name
		sc->ExpectName8();
		MapLumpName = sc->Name8;
	}

	//	Check for replaced map info.
	mapInfo_t* info = NULL;
	for (int i = 1; i < MAX_MAPS; i++)
	{
		if (MapLumpName == MapInfo[i].LumpName)
		{
			info = &MapInfo[i];
			memcpy(info, &MapInfo[0], sizeof(*info));

			// The warp translation defaults to the map	index
			info->warpTrans = i;
			break;
		}
	}
	if (!info)
	{
		info = &MapInfo[MapCount];

		// Copy defaults to current map definition
		memcpy(info, &MapInfo[0], sizeof(*info));

		// The warp translation defaults to the map	index
		info->warpTrans = MapCount;

		MapCount++;
	}
	info->LumpName = MapLumpName;

	int NumMapAlias = 0;

	// Map name must follow the number
	sc->ExpectString();
	VStr::Cpy(info->name, *sc->String);

	//	Set song lump name from SNDINFO script
	for (int i = 0; i < MapSongList.Num(); i++)
	{
		if (MapSongList[i].MapName == info->LumpName)
		{
			info->SongLump = MapSongList[i].SongName;
		}
	}

	// Process optional tokens
	while (1)
	{
		if (sc->Check("cluster"))
		{
			sc->ExpectNumber();
			info->cluster = sc->Number;
		}
		else if (sc->Check("warptrans"))
		{
			sc->ExpectNumber();
			info->warpTrans = sc->Number;
		}
		else if (sc->Check("next"))
		{
			sc->ExpectName8();
			info->NextMap = sc->Name8;
		}
		else if (sc->Check("secret"))
		{
			sc->ExpectName8();
			info->SecretMap = sc->Name8;
		}
		else if (sc->Check("sky1"))
		{
			sc->ExpectName8();
			info->sky1Texture = GTextureManager.CheckNumForName(
				sc->Name8, TEXTYPE_Wall, true, false);
			sc->ExpectNumber();
			info->sky1ScrollDelta = (float)sc->Number * 35.0 / 256.0;
		}
		else if (sc->Check("sky2"))
		{
			sc->ExpectName8();
			info->sky2Texture = GTextureManager.CheckNumForName(
				sc->Name8, TEXTYPE_Wall, true, false);
			sc->ExpectNumber();
			info->sky2ScrollDelta = (float)sc->Number * 35.0 / 256.0;
		}
		else if (sc->Check("skybox"))
		{
			sc->ExpectString();
			info->SkyBox = *sc->String;
		}
		else if (sc->Check("doublesky"))
		{
			info->doubleSky = true;
		}
		else if (sc->Check("lightning"))
		{
			info->lightning = true;
		}
		else if (sc->Check("fadetable"))
		{
			sc->ExpectName8();
			info->FadeTable = sc->Name8;
		}
    	else if (sc->Check("music"))
		{
			sc->ExpectName8();
			info->SongLump = sc->Name8;
		}
		else if (sc->Check("cdtrack"))
		{
			sc->ExpectNumber();
			info->cdTrack = sc->Number;
		}
		else if (sc->Check("gravity"))
		{
			sc->ExpectNumber();
			info->Gravity = (float)sc->Number;
		}
		else if (sc->Check("mapalias"))
		{
			sc->Expect("{");
			while (!sc->Check("}"))
			{
				if (NumMapAlias == MAX_MAP_ALIAS)
				{
					sc->Error("Too many map aliases");
				}
				sc->ExpectNumber();
				info->mapalias[NumMapAlias].Num = sc->Number;
				sc->ExpectName8();
				info->mapalias[NumMapAlias].Name = sc->Name8;
				NumMapAlias++;
			}
		}
		else
		{
			break;
		}
	}

	if (info->doubleSky)
	{
		GTextureManager.SetFrontSkyLayer(info->sky1Texture);
	}
}

//==========================================================================
//
//	ParseMapInfo
//
//==========================================================================

static void ParseMapInfo(VScriptParser* sc)
{
	guard(ParseMapInfo);
	while (!sc->AtEnd())
	{
		if (sc->Check("map"))
		{
			ParseMap(sc);
		}
		else if (sc->Check("cd_start_track"))
		{
			sc->ExpectNumber();
			cd_NonLevelTracks[CD_STARTTRACK] = sc->Number;
		}
		else if (sc->Check("cd_end1_track"))
		{
			sc->ExpectNumber();
			cd_NonLevelTracks[CD_END1TRACK] = sc->Number;
		}
		else if (sc->Check("cd_end2_track"))
		{
			sc->ExpectNumber();
			cd_NonLevelTracks[CD_END2TRACK] = sc->Number;
		}
		else if (sc->Check("cd_end3_track"))
		{
			sc->ExpectNumber();
			cd_NonLevelTracks[CD_END3TRACK] = sc->Number;
		}
		else if (sc->Check("cd_intermission_track"))
		{
			sc->ExpectNumber();
			cd_NonLevelTracks[CD_INTERTRACK] = sc->Number;
		}
		else if (sc->Check("cd_title_track"))
		{
			sc->ExpectNumber();
			cd_NonLevelTracks[CD_TITLETRACK] = sc->Number;
		}
		else
		{
			sc->Error("Invalid command");
		}
	}
	delete sc;
	unguard;
}

//==========================================================================
//
// QualifyMap
//
//==========================================================================

static int QualifyMap(int map)
{
	return (map < 1 || map > MapCount) ? 0 : map;
}

//==========================================================================
//
//	P_GetMapInfo
//
//==========================================================================

void P_GetMapInfo(VName map, mapInfo_t &info)
{
	guard(P_GetMapInfo);
	for (int i = 1; i < MAX_MAPS; i++)
	{
		if (map == MapInfo[i].LumpName)
		{
			info = MapInfo[i];
			return;
		}
	}
	info = MapInfo[0];
	unguard;
}

//==========================================================================
//
//	P_GetMapName
//
//==========================================================================

char* P_GetMapName(int map)
{
	return MapInfo[QualifyMap(map)].name;
}

//==========================================================================
//
//	P_GetMapLumpName
//
//==========================================================================

VName P_GetMapLumpName(int map)
{
	return MapInfo[QualifyMap(map)].LumpName;
}

//==========================================================================
//
// P_GetMapCluster
//
//==========================================================================

/*
int P_GetMapCluster(int map)
{
	return MapInfo[QualifyMap(map)].cluster;
}
*/

//==========================================================================
//
// P_GetMapWarpTrans
//
//==========================================================================

/*
int P_GetMapWarpTrans(int map)
{
	return MapInfo[QualifyMap(map)].warpTrans;
}
*/

//==========================================================================
//
// P_TranslateMap
//
// Returns the actual map number given a warp map number.
//
//==========================================================================

VName P_TranslateMap(int map)
{
	guard(P_TranslateMap);
	for (int i = 1; i < MAX_MAPS; i++)
	{
		if (MapInfo[i].warpTrans == map)
		{
			return MapInfo[i].LumpName;
		}
	}
	// Not found
	return MapInfo[1].LumpName;
	unguard;
}

//==========================================================================
//
// P_PutMapSongLump
//
//==========================================================================

void P_PutMapSongLump(int map, VName lumpName)
{
	guard(P_PutMapSongLump);
	FMapSongInfo& ms = MapSongList.Alloc();
	ms.MapName = va("map%02d", map);
	ms.SongName = lumpName;
	unguard;
}

//==========================================================================
//
// P_GetCDStartTrack
//
//==========================================================================

int P_GetCDStartTrack()
{
	return cd_NonLevelTracks[CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDEnd1Track
//
//==========================================================================

int P_GetCDEnd1Track()
{
	return cd_NonLevelTracks[CD_END1TRACK];
}

//==========================================================================
//
// P_GetCDEnd2Track
//
//==========================================================================

int P_GetCDEnd2Track()
{
	return cd_NonLevelTracks[CD_END2TRACK];
}

//==========================================================================
//
// P_GetCDEnd3Track
//
//==========================================================================

int P_GetCDEnd3Track()
{
	return cd_NonLevelTracks[CD_END3TRACK];
}

//==========================================================================
//
// P_GetCDIntermissionTrack
//
//==========================================================================

int P_GetCDIntermissionTrack()
{
	return cd_NonLevelTracks[CD_INTERTRACK];
}

//==========================================================================
//
// P_GetCDTitleTrack
//
//==========================================================================

int P_GetCDTitleTrack()
{
	return cd_NonLevelTracks[CD_TITLETRACK];
}

//==========================================================================
//
//	COMMAND MapList
//
//==========================================================================

COMMAND(MapList)
{
	guard(COMMAND MapList);
	for (int i = 0; i < MapCount; i++)
	{
		if (W_CheckNumForName(MapInfo[i].LumpName) >= 0)
		{
			GCon->Logf("%s - %s", *MapInfo[i].LumpName, MapInfo[i].name);
		}
	}
	unguard;
}

//==========================================================================
//
//	ShutdownMapInfo
//
//==========================================================================

void ShutdownMapInfo()
{
	guard(ShutdownMapInfo);
	MapSongList.Clear();
	unguard;
}
