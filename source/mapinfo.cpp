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
//**	Copyright (C) 1999-2002 J�nis Legzdi��
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
	MCMD_ROGUECONSCRIPT,
	MCMD_ACSHELPER,
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
	"ROGUECONSCRIPT",
	"ACSHELPER",
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
		for (TArray<FMapSongInfo>::TIterator SongIt(MapSongList); SongIt; ++SongIt)
		{
			if (!stricmp(SongIt->MapName, info->lumpname))
			{
				strcpy(info->songLump, SongIt->SongName);
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
				case MCMD_ROGUECONSCRIPT:
					SC_MustGetString();
					strcpy(info->speechLump, sc_String);
					break;
				case MCMD_ACSHELPER:
					SC_MustGetString();
					strcpy(info->acsLump, sc_String);
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
	int i = MapSongList.Add();
	sprintf(MapSongList[i].MapName, "MAP%02d", map);
	strcpy(MapSongList[i].SongName, lumpName);
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

/*
	int		cluster;    // Defines what cluster level belongs to
	int		warpTrans;  // Actual map number in case maps are not sequential
	int		nextMap;    // Map to teleport to upon exit of timed deathmatch
	int		cdTrack;    // CD track to play during level
	char	fadetable[12];// Fade table {fogmap}
	char	songLump[12];// Background music (MUS or MIDI)
*/

//**************************************************************************
//
//	$Log$
//	Revision 1.18  2006/03/04 16:01:34  dj_jl
//	File system API now uses strings.
//
//	Revision 1.17  2006/03/02 23:24:35  dj_jl
//	Wad lump names stored as names.
//	
//	Revision 1.16  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//	
//	Revision 1.15  2006/01/18 19:55:26  dj_jl
//	Fixed doublesky.
//	
//	Revision 1.14  2006/01/03 18:36:12  dj_jl
//	Fixed map infos redefining maps.
//	
//	Revision 1.13  2005/11/08 18:36:43  dj_jl
//	Parse all mapinfo scripts.
//	
//	Revision 1.12  2005/05/26 16:52:29  dj_jl
//	Created texture manager class
//	
//	Revision 1.11  2004/10/11 06:50:54  dj_jl
//	ACS helper scripts lump.
//	
//	Revision 1.10  2002/07/27 18:10:11  dj_jl
//	Implementing Strife conversations.
//	
//	Revision 1.9  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.8  2002/03/12 19:22:22  dj_jl
//	Fixed next maps in Hexen.
//	
//	Revision 1.7  2002/03/02 17:31:28  dj_jl
//	Added P_GetMapLumpName for Hexen net menu.
//	
//	Revision 1.6  2002/02/26 17:53:08  dj_jl
//	Fixes for menus.
//	
//	Revision 1.5  2002/01/28 18:41:43  dj_jl
//	Fixed Hexen music
//	
//	Revision 1.4  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.2  2001/10/12 17:31:13  dj_jl
//	no message
//	
//	Revision 1.1  2001/10/08 17:30:23  dj_jl
//	Renamed to mapinfo.*
//	
//	Revision 1.4  2001/08/04 17:27:39  dj_jl
//	Added consts to script functions
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
