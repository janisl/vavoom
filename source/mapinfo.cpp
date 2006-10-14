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

static mapInfo_t			DefaultMap;
static TArray<mapInfo_t>	MapInfo;
static TArray<FMapSongInfo>	MapSongList;
static VClusterDef			DefaultClusterDef;
static TArray<VClusterDef>	ClusterDefs;
// Non-level specific song cd track numbers
static int					cd_NonLevelTracks[6];

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

	// Put defaults into MapInfo[0]
	info = &DefaultMap;
	info->Name = "Unnamed";
	info->Cluster = 0;
	info->WarpTrans = 0;
	info->NextMap = NAME_None;
	info->CDTrack = 0;
	info->Sky1Texture = GTextureManager.CheckNumForName("sky1",
		TEXTYPE_Wall, true, false);
	info->Sky2Texture = info->Sky1Texture;
	info->Sky1ScrollDelta = 0.0;
	info->Sky2ScrollDelta = 0.0;
	info->FadeTable = NAME_colormap;
	info->Gravity = 0.0;
	info->Flags = 0;

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

	for (int i = 0; i < MapInfo.Num(); i++)
	{
		if (VStr(MapInfo[i].NextMap).StartsWith("&wt@"))
		{
			MapInfo[i].NextMap = P_TranslateMap(atoi(
				*MapInfo[i].NextMap + 4));
		}
		if (VStr(MapInfo[i].SecretMap).StartsWith("&wt@"))
		{
			MapInfo[i].SecretMap = P_TranslateMap(atoi(
				*MapInfo[i].SecretMap + 4));
		}
	}
	unguard;
}

//==========================================================================
//
//	ParseMap
//
//==========================================================================

static VName ParseNextMapName(VScriptParser* sc, bool HexenMode)
{
	if (sc->CheckNumber())
	{
		if (HexenMode)
			return va("&wt@%02d", sc->Number);
		else
			return va("map%02d", sc->Number);
	}
	else if (sc->Check("endbunny"))
	{
		return "EndGameBunny";
	}
	else if (sc->Check("endcast"))
	{
		return "EndGameCast";
	}
	else if (sc->Check("enddemon"))
	{
		return "EndGameDemon";
	}
	else if (sc->Check("endchess"))
	{
		return "EndGameChess";
	}
	else if (sc->Check("endunderwater"))
	{
		return "EndGameUnderwater";
	}
	else if (sc->Check("endbuystrife"))
	{
		return "EndGameBuyStrife";
	}
	else if (sc->Check("endpic"))
	{
		sc->ExpectName8();
		return va("EndGameCustomPic%s", *sc->Name8);
	}
	else
	{
		sc->ExpectString();
		if (sc->String.ToLower().StartsWith("endgame"))
		{
			switch (sc->String[7])
			{
			case '1':
				return "EndGamePic1";
			case '2':
				return "EndGamePic2";
			case '3':
				return "EndGameBunny";
			case 'c':
			case 'C':
				return "EndGameCast";
			case 'w':
			case 'W':
				return "EndGameUnderwater";
			case 's':
			case 'S':
				return "EndGameStrife";
			default:
				return "EndGamePic3";
			}
		}
		else
		{
			return VName(*sc->String, VName::AddLower8);
		}
	}
}

//==========================================================================
//
//	ParseMap
//
//==========================================================================

static void ParseMap(VScriptParser* sc, bool IsDefault, bool& HexenMode)
{
	mapInfo_t* info = NULL;
	if (IsDefault)
	{
		info = &DefaultMap;
		info->LumpName = NAME_None;
		info->Name = VStr();
		info->LevelNum = 0;
		info->Cluster = 0;
		info->WarpTrans = 0;
		info->NextMap = NAME_None;
		info->SecretMap = NAME_None;
		info->SongLump = NAME_None;
		info->CDTrack = 0;
		info->Sky1Texture = GTextureManager.DefaultTexture;
		info->Sky2Texture = GTextureManager.DefaultTexture;
		info->Sky1ScrollDelta = 0;
		info->Sky2ScrollDelta = 0;
		info->SkyBox = NAME_None;
		info->FadeTable = NAME_colormap;
		info->Gravity = 0;
		info->Flags = 0;
	}
	else
	{
		VName MapLumpName;
		if (sc->CheckNumber())
		{
			//	Map number, for Hexen compatibility
			HexenMode = true;
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
		for (int i = 0; i < MapInfo.Num(); i++)
		{
			if (MapLumpName == MapInfo[i].LumpName)
			{
				info = &MapInfo[i];
				break;
			}
		}
		if (!info)
		{
			info = &MapInfo.Alloc();
		}

		// Copy defaults to current map definition
		info->LumpName = MapLumpName;
		info->LevelNum = DefaultMap.LevelNum;
		info->Cluster = DefaultMap.Cluster;
		info->WarpTrans = DefaultMap.WarpTrans;
		info->NextMap = DefaultMap.NextMap;
		info->SecretMap = DefaultMap.SecretMap;
		info->SongLump = DefaultMap.SongLump;
		info->CDTrack = DefaultMap.CDTrack;
		info->Sky1Texture = DefaultMap.Sky1Texture;
		info->Sky2Texture = DefaultMap.Sky2Texture;
		info->Sky1ScrollDelta = DefaultMap.Sky1ScrollDelta;
		info->Sky2ScrollDelta = DefaultMap.Sky2ScrollDelta;
		info->SkyBox = DefaultMap.SkyBox;
		info->FadeTable = DefaultMap.FadeTable;
		info->Gravity = DefaultMap.Gravity;
		info->Flags = DefaultMap.Flags;

		if (HexenMode)
		{
			info->Flags |= MAPINFOF_NoIntermission;
		}

		// Map name must follow the number
		if (sc->Check("lookup"))
		{
			info->Flags |= MAPINFOF_LookupName;
			sc->ExpectString();
			info->Name = sc->String.ToLower();
		}
		else
		{
			info->Flags &= ~MAPINFOF_LookupName;
			sc->ExpectString();
			info->Name = sc->String;
		}

		//	Set song lump name from SNDINFO script
		for (int i = 0; i < MapSongList.Num(); i++)
		{
			if (MapSongList[i].MapName == info->LumpName)
			{
				info->SongLump = MapSongList[i].SongName;
			}
		}

		//	Set default levelnum for this map.
		const char* mn = *MapLumpName;
		if (mn[0] == 'm' && mn[1] == 'a' && mn[2] == 'p' && mn[5] == 0)
		{
			int num = atoi(mn + 3);
			if  (num >= 1 && num <= 99)
				info->LevelNum = num;
		}
		else if (mn[0] == 'e' && mn[1] >= '0' && mn[1] <= '9' &&
			mn[2] == 'm' && mn[3] >= '0' && mn[3] <= '9')
		{
			info->LevelNum = (mn[1] - '1') * 10 + (mn[3] - '0');
		}
	}

	// Process optional tokens
	while (1)
	{
		if (sc->Check("levelnum"))
		{
			sc->ExpectNumber();
			info->LevelNum = sc->Number;
		}
		else if (sc->Check("cluster"))
		{
			sc->ExpectNumber();
			info->Cluster = sc->Number;
			if (!P_GetClusterDef(info->Cluster))
			{
				//	Add empty cluster def if it doesn't exist yet.
				VClusterDef& C = ClusterDefs.Alloc();
				C.Cluster = info->Cluster;
				C.Flags = 0;
				C.EnterText = VStr();
				C.ExitText = VStr();
				C.Flat = NAME_None;
				C.Music = NAME_None;
				C.CDTrack = 0;
				C.CDId = 0;
			}
		}
		else if (sc->Check("warptrans"))
		{
			sc->ExpectNumber();
			info->WarpTrans = sc->Number;
		}
		else if (sc->Check("next"))
		{
			info->NextMap = ParseNextMapName(sc, HexenMode);
		}
		else if (sc->Check("secret") || sc->Check("secretnext"))
		{
			info->SecretMap = ParseNextMapName(sc, HexenMode);
		}
		else if (sc->Check("sky1"))
		{
			sc->ExpectName8();
			info->Sky1Texture = GTextureManager.CheckNumForName(
				sc->Name8, TEXTYPE_Wall, true, false);
			sc->ExpectFloat();
			if (HexenMode)
			{
				sc->Float /= 256.0;
			}
			info->Sky1ScrollDelta = sc->Float * 35.0;
		}
		else if (sc->Check("sky2"))
		{
			sc->ExpectName8();
			info->Sky2Texture = GTextureManager.CheckNumForName(
				sc->Name8, TEXTYPE_Wall, true, false);
			sc->ExpectFloat();
			if (HexenMode)
			{
				sc->Float /= 256.0;
			}
			info->Sky2ScrollDelta = sc->Float * 35.0;
		}
		else if (sc->Check("skybox"))
		{
			sc->ExpectString();
			info->SkyBox = *sc->String;
		}
		else if (sc->Check("doublesky"))
		{
			info->Flags |= MAPINFOF_DoubleSky;
		}
		else if (sc->Check("lightning"))
		{
			info->Flags |= MAPINFOF_Lightning;
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
			info->CDTrack = sc->Number;
		}
		else if (sc->Check("gravity"))
		{
			sc->ExpectNumber();
			info->Gravity = (float)sc->Number;
		}
		else if (sc->Check("map07special"))
		{
			info->Flags |= MAPINFOF_Map07Special;
		}
		else if (sc->Check("baronspecial"))
		{
			info->Flags |= MAPINFOF_BaronSpecial;
		}
		else if (sc->Check("cyberdemonspecial"))
		{
			info->Flags |= MAPINFOF_CyberDemonSpecial;
		}
		else if (sc->Check("spidermastermindspecial"))
		{
			info->Flags |= MAPINFOF_SpiderMastermindSpecial;
		}
		else if (sc->Check("minotaurspecial"))
		{
			info->Flags |= MAPINFOF_MinotaurSpecial;
		}
		else if (sc->Check("dsparilspecial"))
		{
			info->Flags |= MAPINFOF_DSparilSpecial;
		}
		else if (sc->Check("ironlichspecial"))
		{
			info->Flags |= MAPINFOF_IronLichSpecial;
		}
		else if (sc->Check("specialaction_exitlevel"))
		{
			info->Flags &= ~(MAPINFOF_SpecialActionOpenDoor |
				MAPINFOF_SpecialActionLowerFloor);
		}
		else if (sc->Check("specialaction_opendoor"))
		{
			info->Flags &= ~MAPINFOF_SpecialActionLowerFloor;
			info->Flags |= MAPINFOF_SpecialActionOpenDoor;
		}
		else if (sc->Check("specialaction_lowerfloor"))
		{
			info->Flags |= MAPINFOF_SpecialActionLowerFloor;
			info->Flags &= ~MAPINFOF_SpecialActionOpenDoor;
		}
		else if (sc->Check("specialaction_killmonsters"))
		{
			info->Flags |= MAPINFOF_SpecialActionKillMonsters;
		}
		else if (sc->Check("intermission"))
		{
			info->Flags &= ~MAPINFOF_NoIntermission;
		}
		else if (sc->Check("nointermission"))
		{
			info->Flags |= MAPINFOF_NoIntermission;
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
			break;
		}
	}

	if (info->Flags & MAPINFOF_DoubleSky)
	{
		GTextureManager.SetFrontSkyLayer(info->Sky1Texture);
	}

	if (!IsDefault)
	{
		//	Avoid duplicate levelnums, later one takes precedance.
		for (int i = 0; i < MapInfo.Num(); i++)
		{
			if (MapInfo[i].LevelNum == info->LevelNum &&
				&MapInfo[i] != info)
			{
				MapInfo[i].LevelNum = 0;
			}
		}
	}
}

//==========================================================================
//
//	ParseClusterDef
//
//==========================================================================

static void ParseClusterDef(VScriptParser* sc)
{
	VClusterDef* CDef = NULL;
	sc->ExpectNumber();

	//	Check for replaced cluster def.
	for (int i = 0; i < ClusterDefs.Num(); i++)
	{
		if (sc->Number == ClusterDefs[i].Cluster)
		{
			CDef = &ClusterDefs[i];
			break;
		}
	}
	if (!CDef)
	{
		CDef = &ClusterDefs.Alloc();
	}

	//	Set defaults.
	CDef->Cluster = sc->Number;
	CDef->Flags = 0;
	CDef->EnterText = VStr();
	CDef->ExitText = VStr();
	CDef->Flat = NAME_None;
	CDef->Music = NAME_None;
	CDef->CDTrack = 0;
	CDef->CDId = 0;

	while (1)
	{
		if (sc->Check("hub"))
		{
			CDef->Flags |= CLUSTERF_Hub;
		}
		else if (sc->Check("entertext"))
		{
			if (sc->Check("lookup"))
			{
				CDef->Flags |= CLUSTERF_LookupEnterText;
				sc->ExpectString();
				CDef->EnterText = sc->String;
			}
			else
			{
				CDef->Flags &= ~CLUSTERF_LookupEnterText;
				sc->ExpectString();
				CDef->EnterText = sc->String;
			}
		}
		else if (sc->Check("entertextislump"))
		{
			CDef->Flags |= CLUSTERF_EnterTextIsLump;
		}
		else if (sc->Check("exittext"))
		{
			if (sc->Check("lookup"))
			{
				CDef->Flags |= CLUSTERF_LookupExitText;
				sc->ExpectString();
				CDef->ExitText = sc->String.ToLower();
			}
			else
			{
				CDef->Flags &= ~CLUSTERF_LookupExitText;
				sc->ExpectString();
				CDef->ExitText = sc->String;
			}
		}
		else if (sc->Check("exittextislump"))
		{
			CDef->Flags |= CLUSTERF_ExitTextIsLump;
		}
		else if (sc->Check("flat"))
		{
			sc->ExpectName8();
			CDef->Flat = sc->Name8;
			CDef->Flags &= ~CLUSTERF_FinalePic;
		}
		else if (sc->Check("pic"))
		{
			sc->ExpectName8();
			CDef->Flat = sc->Name8;
			CDef->Flags |= CLUSTERF_FinalePic;
		}
		else if (sc->Check("music"))
		{
			sc->ExpectName8();
			CDef->Music = sc->Name8;
		}
		else if (sc->Check("cdtrack"))
		{
			sc->ExpectNumber();
			CDef->CDTrack = sc->Number;
		}
		else if (sc->Check("cdid"))
		{
			sc->ExpectNumber();
			CDef->CDId = sc->Number;
		}
		else
		{
			break;
		}
	}

	//	Make sure text lump names are in lower case.
	if (CDef->Flags & CLUSTERF_EnterTextIsLump)
	{
		CDef->EnterText = CDef->EnterText.ToLower();
	}
	if (CDef->Flags & CLUSTERF_ExitTextIsLump)
	{
		CDef->ExitText = CDef->ExitText.ToLower();
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
	bool HexenMode = false;
	while (!sc->AtEnd())
	{
		if (sc->Check("map"))
		{
			ParseMap(sc, false, HexenMode);
		}
		else if (sc->Check("defaultmap"))
		{
			ParseMap(sc, true, HexenMode);
		}
		else if (sc->Check("clusterdef"))
		{
			ParseClusterDef(sc);
		}
		else
		{
			sc->Error(va("Invalid command %s", *sc->String));
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
	return (map < 0 || map >= MapInfo.Num()) ? 0 : map;
}

//==========================================================================
//
//	P_GetMapInfo
//
//==========================================================================

const mapInfo_t& P_GetMapInfo(VName map)
{
	guard(P_GetMapInfo);
	for (int i = 0; i < MapInfo.Num(); i++)
	{
		if (map == MapInfo[i].LumpName)
		{
			return MapInfo[i];
		}
	}
	return DefaultMap;
	unguard;
}

//==========================================================================
//
//	P_GetMapName
//
//==========================================================================

const char* P_GetMapName(int map)
{
	return *MapInfo[QualifyMap(map)].Name;
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
// P_TranslateMap
//
// Returns the actual map number given a warp map number.
//
//==========================================================================

VName P_TranslateMap(int map)
{
	guard(P_TranslateMap);
	for (int i = MapInfo.Num() - 1; i >= 0; i--)
	{
		if (MapInfo[i].WarpTrans == map)
		{
			return MapInfo[i].LumpName;
		}
	}
	// Not found
	return MapInfo[0].LumpName;
	unguard;
}

//==========================================================================
//
//	P_GetMapNameByLevelNum
//
//	Returns the actual map name given a level number.
//
//==========================================================================

VName P_GetMapNameByLevelNum(int map)
{
	guard(P_GetMapNameByLevelNum);
	for (int i = 0; i < MapInfo.Num(); i++)
	{
		if (MapInfo[i].LevelNum == map)
		{
			return MapInfo[i].LumpName;
		}
	}
	// Not found, use map##
	return va("map%02d", map);
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
//	P_GetClusterDef
//
//==========================================================================

const VClusterDef* P_GetClusterDef(int Cluster)
{
	guard(P_GetClusterDef);
	for (int i = 0; i < ClusterDefs.Num(); i++)
	{
		if (Cluster == ClusterDefs[i].Cluster)
		{
			return &ClusterDefs[i];
		}
	}
	return &DefaultClusterDef;
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
	for (int i = 0; i < MapInfo.Num(); i++)
	{
		if (W_CheckNumForName(MapInfo[i].LumpName) >= 0)
		{
			GCon->Log(VStr(MapInfo[i].LumpName) + " - " + MapInfo[i].Name);
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
	DefaultMap.Name.Clean();
	MapInfo.Clear();
	MapSongList.Clear();
	ClusterDefs.Clear();
	unguard;
}
