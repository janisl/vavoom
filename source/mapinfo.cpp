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
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
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
#define DEFAULT_FADE_TABLE	"COLORMAP"

enum
{
	MCMD_SKY1,
	MCMD_SKY2,
	MCMD_SKYBOX,
	MCMD_DOUBLESKY,
	MCMD_LIGHTNING,
	MCMD_FADETABLE,
	MCMD_CLUSTER,
	MCMD_WARPTRANS,
	MCMD_NEXT,
	MCMD_SECRET,
	MCMD_MAPALIAS,
	MCMD_MUSIC,
	MCMD_CDTRACK,
	MCMD_CD_STARTTRACK,
	MCMD_CD_END1TRACK,
	MCMD_CD_END2TRACK,
	MCMD_CD_END3TRACK,
	MCMD_CD_INTERTRACK,
	MCMD_CD_TITLETRACK,
};

// TYPES -------------------------------------------------------------------

struct FMapSongInfo
{
	char	MapName[16];
	char	SongName[16];
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

char *P_TranslateMap(int map);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void ParseMapInfo();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static mapInfo_t	MapInfo[MAX_MAPS];
static int 			MapCount;

static const char *MapCmdNames[] =
{
	"SKY1",
	"SKY2",
	"SKYBOX",
	"DOUBLESKY",
	"LIGHTNING",
	"FADETABLE",
	"CLUSTER",
	"WARPTRANS",
	"NEXT",
	"SECRET",
	"MAPALIAS",
    "MUSIC",
	"CDTRACK",
	"CD_START_TRACK",
	"CD_END1_TRACK",
	"CD_END2_TRACK",
	"CD_END3_TRACK",
	"CD_INTERMISSION_TRACK",
	"CD_TITLE_TRACK",
	NULL
};

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
	strcpy(info->nextMap, "MAP01"); // Always go to map 1 if not specified
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
	strcpy(info->fadetable, DEFAULT_FADE_TABLE);
	strcpy(info->name, UNKNOWN_MAP_NAME);

	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) == NAME_mapinfo)
		{
			SC_OpenLumpNum(Lump);
			ParseMapInfo();
		}
	}
	//	Optionally parse script file.
	VStr filename = FL_FindFile("scripts/mapinfo.txt");
	if (fl_devmode && filename)
	{
		SC_OpenFile(*filename);
		ParseMapInfo();
	}

	for (int i = 1; i < MapCount; i++)
	{
		if (MapInfo[i].nextMap[0] >= '0' && MapInfo[i].nextMap[0] <= '9')
		{
			strcpy(MapInfo[i].nextMap, P_TranslateMap(atoi(MapInfo[i].nextMap)));
		}
	}
	unguard;
}

//==========================================================================
//
//	ParseMapInfo
//
//==========================================================================

static void ParseMapInfo()
{
	guard(ParseMapInfo);
	mapInfo_t 	*info;
	int 		mcmdValue;
	int			NumMapAlias;
	char		MapLumpName[12];

	while(SC_GetString())
	{
		if (SC_Compare("MAP") == false)
		{
			SC_ScriptError(NULL);
		}

		SC_MustGetString();
		if (sc_String[0] >= '0' && sc_String[0] <= '9')
		{
			//	Map number, for Hexen compatibility
			SC_UnGet();
			SC_MustGetNumber();
			if (sc_Number < 1 || sc_Number > 99)
			{
				SC_ScriptError(NULL);
			}
			sprintf(MapLumpName, "MAP%02d", sc_Number);
		}
		else
		{
			//	Map name
			if (strlen(sc_String) > 8)
			{
				SC_ScriptError(NULL);
			}
			W_CleanupName(sc_String, MapLumpName);
		}

		//	Check for replaced map info.
		info = NULL;
		for (int i = 1; i < MAX_MAPS; i++)
		{
			if (!stricmp(MapLumpName, MapInfo[i].lumpname))
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
		strcpy(info->lumpname, MapLumpName);

		NumMapAlias = 0;

		// Map name must follow the number
		SC_MustGetString();
		strcpy(info->name, sc_String);

		//	Set song lump name from SNDINFO script
		for (int i = 0; i < MapSongList.Num(); i++)
		{
			if (!stricmp(MapSongList[i].MapName, info->lumpname))
			{
				strcpy(info->songLump, MapSongList[i].SongName);
			}
		}

		// Process optional tokens
		while (SC_GetString())
		{
			if (SC_Compare("MAP"))
			{ // Start next map definition
				SC_UnGet();
				break;
			}
			mcmdValue = SC_MustMatchString(MapCmdNames);
			switch(mcmdValue)
			{
				case MCMD_CLUSTER:
					SC_MustGetNumber();
					info->cluster = sc_Number;
					break;
				case MCMD_WARPTRANS:
					SC_MustGetNumber();
					info->warpTrans = sc_Number;
					break;
				case MCMD_NEXT:
					SC_MustGetString();
					strcpy(info->nextMap, sc_String);
					break;
				case MCMD_SECRET:
					SC_MustGetString();
					strcpy(info->secretMap, sc_String);
					break;
				case MCMD_MAPALIAS:
					SC_MustGetStringName("{");
					SC_MustGetString();
					while (!SC_Compare("}"))
					{
						if (NumMapAlias == MAX_MAP_ALIAS)
						{
							SC_ScriptError("Too many map alias");
						}
						SC_UnGet();
						SC_MustGetNumber();
						info->mapalias[NumMapAlias].num = sc_Number;
						SC_MustGetString();
						strcpy(info->mapalias[NumMapAlias].name, sc_String);
						SC_MustGetString();
					}
					break;
				case MCMD_CDTRACK:
					SC_MustGetNumber();
					info->cdTrack = sc_Number;
					break;
				case MCMD_SKY1:
					SC_MustGetString();
					info->sky1Texture = GTextureManager.CheckNumForName(
						VName(sc_String, VName::AddLower8),
						TEXTYPE_Wall, true, false);
					SC_MustGetNumber();
					info->sky1ScrollDelta = (float)sc_Number * 35.0 / 256.0;
					break;
				case MCMD_SKY2:
					SC_MustGetString();
					info->sky2Texture = GTextureManager.CheckNumForName(
						VName(sc_String, VName::AddLower8),
						TEXTYPE_Wall, true, false);
					SC_MustGetNumber();
					info->sky2ScrollDelta = (float)sc_Number * 35.0 / 256.0;
					break;
				case MCMD_SKYBOX:
					SC_MustGetString();
					strcpy(info->skybox, sc_String);
					break;
				case MCMD_DOUBLESKY:
					info->doubleSky = true;
					break;
				case MCMD_LIGHTNING:
					info->lightning = true;
					break;
				case MCMD_FADETABLE:
					SC_MustGetString();
					strcpy(info->fadetable, sc_String);
					break;
				case MCMD_MUSIC:
					SC_MustGetString();
					strcpy(info->songLump, sc_String);
					break;
				case MCMD_CD_STARTTRACK:
				case MCMD_CD_END1TRACK:
				case MCMD_CD_END2TRACK:
				case MCMD_CD_END3TRACK:
				case MCMD_CD_INTERTRACK:
				case MCMD_CD_TITLETRACK:
					SC_MustGetNumber();
					cd_NonLevelTracks[mcmdValue-MCMD_CD_STARTTRACK] = sc_Number;
					break;
			}
		}
		if (info->doubleSky)
			GTextureManager.SetFrontSkyLayer(info->sky1Texture);
	}
	SC_Close();
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

void P_GetMapInfo(const char *map, mapInfo_t &info)
{
	guard(P_GetMapInfo);
	for (int i = 1; i < MAX_MAPS; i++)
	{
		if (!stricmp(map, MapInfo[i].lumpname))
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

char* P_GetMapLumpName(int map)
{
	return MapInfo[QualifyMap(map)].lumpname;
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

char *P_TranslateMap(int map)
{
	guard(P_TranslateMap);
	int i;

	for (i = 1; i < MAX_MAPS; i++)
	{
		if (MapInfo[i].warpTrans == map)
		{
			return MapInfo[i].lumpname;
		}
	}
	// Not found
	return MapInfo[1].lumpname;
	unguard;
}

//==========================================================================
//
// P_PutMapSongLump
//
//==========================================================================

void P_PutMapSongLump(int map, const char *lumpName)
{
	guard(P_PutMapSongLump);
	FMapSongInfo& ms = MapSongList.Alloc();
	sprintf(ms.MapName, "MAP%02d", map);
	strcpy(ms.SongName, lumpName);
	unguard;
}

//==========================================================================
//
// P_GetCDStartTrack
//
//==========================================================================

int P_GetCDStartTrack(void)
{
	return cd_NonLevelTracks[MCMD_CD_STARTTRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDEnd1Track
//
//==========================================================================

int P_GetCDEnd1Track(void)
{
	return cd_NonLevelTracks[MCMD_CD_END1TRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDEnd2Track
//
//==========================================================================

int P_GetCDEnd2Track(void)
{
	return cd_NonLevelTracks[MCMD_CD_END2TRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDEnd3Track
//
//==========================================================================

int P_GetCDEnd3Track(void)
{
	return cd_NonLevelTracks[MCMD_CD_END3TRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDIntermissionTrack
//
//==========================================================================

int P_GetCDIntermissionTrack(void)
{
	return cd_NonLevelTracks[MCMD_CD_INTERTRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDTitleTrack
//
//==========================================================================

int P_GetCDTitleTrack(void)
{
	return cd_NonLevelTracks[MCMD_CD_TITLETRACK-MCMD_CD_STARTTRACK];
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
		if (W_CheckNumForName(VName(MapInfo[i].lumpname, VName::AddLower8)) >= 0)
		{
			GCon->Logf("%s - %s", MapInfo[i].lumpname, MapInfo[i].name);
		}
	}
	unguard;
}
