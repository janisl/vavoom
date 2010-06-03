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
static TArray<VEpisodeDef>	EpisodeDefs;
static TArray<VSkillDef>	SkillDefs;
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
	for (int i = 0; i < EpisodeDefs.Num(); i++)
	{
		if (VStr(EpisodeDefs[i].Name).StartsWith("&wt@"))
		{
			EpisodeDefs[i].Name = P_TranslateMap(atoi(
				*EpisodeDefs[i].Name + 4));
		}
		if (VStr(EpisodeDefs[i].TeaserName).StartsWith("&wt@"))
		{
			EpisodeDefs[i].TeaserName = P_TranslateMap(atoi(
				*EpisodeDefs[i].TeaserName + 4));
		}
	}

	//	Set up default map info returned for maps that have not defined in
	// MAPINFO
	DefaultMap.Name = "Unnamed";
	DefaultMap.Sky1Texture = GTextureManager.CheckNumForName("sky1",
		TEXTYPE_Wall, true, true);
	DefaultMap.Sky2Texture = DefaultMap.Sky1Texture;
	DefaultMap.FadeTable = NAME_colormap;
	DefaultMap.HorizWallShade = -8;
	DefaultMap.VertWallShade = 8;
	unguard;
}

//==========================================================================
//
//	SetMapDefaults
//
//==========================================================================

static void SetMapDefaults(mapInfo_t& Info)
{
	guard(SetMapDefaults);
	Info.LumpName = NAME_None;
	Info.Name = VStr();
	Info.LevelNum = 0;
	Info.Cluster = 0;
	Info.WarpTrans = 0;
	Info.NextMap = NAME_None;
	Info.SecretMap = NAME_None;
	Info.SongLump = NAME_None;
	Info.CDTrack = 0;
	Info.Sky1Texture = GTextureManager.DefaultTexture;
	Info.Sky2Texture = GTextureManager.DefaultTexture;
	Info.Sky1ScrollDelta = 0;
	Info.Sky2ScrollDelta = 0;
	Info.SkyBox = NAME_None;
	Info.FadeTable = NAME_colormap;
	Info.Fade = 0;
	Info.OutsideFog = 0;
	Info.Gravity = 0;
	Info.AirControl = 0;
	Info.Flags = 0;
	Info.Flags2 = 0;
	Info.TitlePatch = NAME_None;
	Info.ParTime = 0;
	Info.SuckTime = 0;
	Info.HorizWallShade = -8;
	Info.VertWallShade = 8;
	Info.Infighting = 0;
	Info.SpecialActions.Clear();
	Info.RedirectType = NAME_None;
	Info.RedirectMap = NAME_None;
	Info.ExitPic = NAME_None;
	Info.EnterPic = NAME_None;
	Info.InterMusic = NAME_None;

	if (GGameInfo->Flags & VGameInfo::GIF_DefaultLaxMonsterActivation)
	{
		Info.Flags2 |= MAPINFOF2_LaxMonsterActivation;
	}
	unguard;
}

//==========================================================================
//
//	ParseNextMapName
//
//==========================================================================

static VName ParseNextMapName(VScriptParser* sc, bool HexenMode)
{
	guard(ParseNextMapName);
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
	unguard;
}

//==========================================================================
//
//	DoCompatFlag
//
//==========================================================================

static void DoCompatFlag(VScriptParser* sc, mapInfo_t* info, int Flag)
{
	guard(DoCompatFlag);
	int Set = 1;
	if (sc->CheckNumber())
	{
		Set = sc->Number;
	}
	if (Set)
	{
		info->Flags2 |= Flag;
	}
	else
	{
		info->Flags2 &= ~Flag;
	}
	unguard;
}

//==========================================================================
//
//	ParseMapCommon
//
//==========================================================================

static void ParseMapCommon(VScriptParser* sc, mapInfo_t* info, bool& HexenMode)
{
	guard(ParseMapCommon);
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
			if (P_GetClusterDef(info->Cluster) == &DefaultClusterDef)
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
				if (HexenMode)
				{
					C.Flags |= CLUSTERF_Hub;
				}
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
			info->Sky1Texture = GTextureManager.NumForName(
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
			info->Sky2Texture = GTextureManager.NumForName(
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
		else if (sc->Check("forcenoskystretch"))
		{
			info->Flags |= MAPINFOF_ForceNoSkyStretch;
		}
		else if (sc->Check("fadetable"))
		{
			sc->ExpectName8();
			info->FadeTable = sc->Name8;
		}
		else if (sc->Check("fade"))
		{
			sc->ExpectString();
			info->Fade = M_ParseColour(sc->String);
		}
		else if (sc->Check("outsidefog"))
		{
			sc->ExpectString();
			info->OutsideFog = M_ParseColour(sc->String);
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
		else if (sc->Check("aircontrol"))
		{
			sc->ExpectFloat();
			info->AirControl = sc->Float;
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
		else if (sc->Check("titlepatch"))
		{
			sc->ExpectName8();
			info->TitlePatch = sc->Name8;
		}
		else if (sc->Check("par"))
		{
			sc->ExpectNumber();
			info->ParTime = sc->Number;
		}
		else if (sc->Check("sucktime"))
		{
			sc->ExpectNumber();
			info->SuckTime = sc->Number;
		}
		else if (sc->Check("nosoundclipping"))
		{
			//	Ignored
		}
		else if (sc->Check("allowmonstertelefrags"))
		{
			info->Flags |= MAPINFOF_AllowMonsterTelefrags;
		}
		else if (sc->Check("noallies"))
		{
			info->Flags |= MAPINFOF_NoAllies;
		}
		else if (sc->Check("fallingdamage"))
		{
			info->Flags &= ~(MAPINFOF_OldFallingDamage |
				MAPINFOF_StrifeFallingDamage);
			info->Flags |= MAPINFOF_FallingDamage;
		}
		else if (sc->Check("oldfallingdamage") ||
			sc->Check("forcefallingdamage"))
		{
			info->Flags &= ~(MAPINFOF_FallingDamage |
				MAPINFOF_StrifeFallingDamage);
			info->Flags |= MAPINFOF_OldFallingDamage;
		}
		else if (sc->Check("strifefallingdamage"))
		{
			info->Flags &= ~(MAPINFOF_OldFallingDamage |
				MAPINFOF_FallingDamage);
			info->Flags |= MAPINFOF_StrifeFallingDamage;
		}
		else if (sc->Check("nofallingdamage"))
		{
			info->Flags &= ~(MAPINFOF_OldFallingDamage |
				MAPINFOF_StrifeFallingDamage | MAPINFOF_FallingDamage);
		}
		else if (sc->Check("monsterfallingdamage"))
		{
			info->Flags |= MAPINFOF_MonsterFallingDamage;
		}
		else if (sc->Check("nomonsterfallingdamage"))
		{
			info->Flags &= ~MAPINFOF_MonsterFallingDamage;
		}
		else if (sc->Check("deathslideshow"))
		{
			info->Flags |= MAPINFOF_DeathSlideShow;
		}
		else if (sc->Check("allowfreelook"))
		{
			info->Flags &= ~MAPINFOF_NoFreelook;
		}
		else if (sc->Check("nofreelook"))
		{
			info->Flags |= MAPINFOF_NoFreelook;
		}
		else if (sc->Check("allowjump"))
		{
			info->Flags &= ~MAPINFOF_NoJump;
		}
		else if (sc->Check("nojump"))
		{
			info->Flags |= MAPINFOF_NoJump;
		}
		else if (sc->Check("noautosequences"))
		{
			info->Flags |= MAPINFOF_NoAutoSndSeq;
		}
		else if (sc->Check("activateowndeathspecials"))
		{
			info->Flags |= MAPINFOF_ActivateOwnSpecial;
		}
		else if (sc->Check("killeractivatesdeathspecials"))
		{
			info->Flags &= ~MAPINFOF_ActivateOwnSpecial;
		}
		else if (sc->Check("missilesactivateimpactlines"))
		{
			info->Flags |= MAPINFOF_MissilesActivateImpact;
		}
		else if (sc->Check("missileshootersactivetimpactlines"))
		{
			info->Flags &= ~MAPINFOF_MissilesActivateImpact;
		}
		else if (sc->Check("filterstarts"))
		{
			info->Flags |= MAPINFOF_FilterStarts;
		}
		else if (sc->Check("infiniteflightpowerup"))
		{
			info->Flags |= MAPINFOF_InfiniteFlightPowerup;
		}
		else if (sc->Check("noinfiniteflightpowerup"))
		{
			info->Flags &= ~MAPINFOF_InfiniteFlightPowerup;
		}
		else if (sc->Check("clipmidtextures"))
		{
			info->Flags |= MAPINFOF_ClipMidTex;
		}
		else if (sc->Check("wrapmidtextures"))
		{
			info->Flags |= MAPINFOF_WrapMidTex;
		}
		else if (sc->Check("keepfullinventory"))
		{
			info->Flags |= MAPINFOF_KeepFullInventory;
		}
		else if (sc->Check("compat_shorttex"))
		{
			DoCompatFlag(sc, info, MAPINFOF2_CompatShortTex);
		}
		else if (sc->Check("compat_stairs"))
		{
			DoCompatFlag(sc, info, MAPINFOF2_CompatStairs);
		}
		else if (sc->Check("compat_limitpain"))
		{
			DoCompatFlag(sc, info, MAPINFOF2_CompatLimitPain);
		}
		else if (sc->Check("compat_nopassover"))
		{
			DoCompatFlag(sc, info, MAPINFOF2_CompatNoPassOver);
		}
		else if (sc->Check("compat_notossdrops"))
		{
			DoCompatFlag(sc, info, MAPINFOF2_CompatNoTossDrops);
		}
		else if (sc->Check("compat_useblocking"))
		{
			DoCompatFlag(sc, info, MAPINFOF2_CompatUseBlocking);
		}
		else if (sc->Check("compat_nodoorlight"))
		{
			DoCompatFlag(sc, info, MAPINFOF2_CompatNoDoorLight);
		}
		else if (sc->Check("compat_ravenscroll"))
		{
			DoCompatFlag(sc, info, MAPINFOF2_CompatRavenScroll);
		}
		else if (sc->Check("compat_soundtarget"))
		{
			DoCompatFlag(sc, info, MAPINFOF2_CompatSoundTarget);
		}
		else if (sc->Check("compat_dehhealth"))
		{
			DoCompatFlag(sc, info, MAPINFOF2_CompatDehHealth);
		}
		else if (sc->Check("compat_trace"))
		{
			DoCompatFlag(sc, info, MAPINFOF2_CompatTrace);
		}
		else if (sc->Check("compat_dropoff"))
		{
			DoCompatFlag(sc, info, MAPINFOF2_CompatDropOff);
		}
		else if (sc->Check("compat_boomscroll") ||
			sc->Check("additive_scrollers"))
		{
			DoCompatFlag(sc, info, MAPINFOF2_CompatBoomScroll);
		}
		else if (sc->Check("compat_invisibility"))
		{
			DoCompatFlag(sc, info, MAPINFOF2_CompatInvisibility);
		}
		else if (sc->Check("evenlighting"))
		{
			info->HorizWallShade = 0;
			info->VertWallShade = 0;
		}
		else if (sc->Check("vertwallshade"))
		{
			sc->ExpectNumber();
			info->VertWallShade = MID(-128, sc->Number, 127);
		}
		else if (sc->Check("horizwallshade"))
		{
			sc->ExpectNumber();
			info->HorizWallShade = MID(-128, sc->Number, 127);
		}
		else if (sc->Check("noinfighting"))
		{
			info->Infighting = -1;
		}
		else if (sc->Check("normalinfighting"))
		{
			info->Infighting = 0;
		}
		else if (sc->Check("totalinfighting"))
		{
			info->Infighting = 1;
		}
		else if (sc->Check("specialaction"))
		{
			VMapSpecialAction& A = info->SpecialActions.Alloc();
			sc->SetCMode(true);
			sc->ExpectString();
			A.TypeName = *sc->String.ToLower();
			sc->Expect(",");
			sc->ExpectString();
			A.Special = 0;
			for (int i = 0; i < LineSpecialInfos.Num(); i++)
			{
				if (!LineSpecialInfos[i].Name.ICmp(sc->String))
				{
					A.Special = LineSpecialInfos[i].Number;
					break;
				}
			}
			if (!A.Special)
			{
				GCon->Logf("Unknown action special %s", *sc->String);
			}
			memset(A.Args, 0, sizeof(A.Args));
			for (int i = 0; i < 5 && sc->Check(","); i++)
			{
				sc->ExpectNumber();
				A.Args[i] = sc->Number;
			}
			sc->SetCMode(false);
		}
		else if (sc->Check("redirect"))
		{
			sc->ExpectString();
			info->RedirectType = *sc->String.ToLower();
			info->RedirectMap = ParseNextMapName(sc, HexenMode);
		}
		else if (sc->Check("strictmonsteractivation"))
		{
			info->Flags2 &= ~MAPINFOF2_LaxMonsterActivation;
			info->Flags2 |= MAPINFOF2_HaveMonsterActivation;
		}
		else if (sc->Check("laxmonsteractivation"))
		{
			info->Flags2 |= MAPINFOF2_LaxMonsterActivation;
			info->Flags2 |= MAPINFOF2_HaveMonsterActivation;
		}
		else if (sc->Check("interpic") || sc->Check("exitpic"))
		{
			sc->ExpectName8();
			info->ExitPic = *sc->String.ToLower();
		}
		else if (sc->Check("enterpic"))
		{
			sc->ExpectName8();
			info->EnterPic = *sc->String.ToLower();
		}
		else if (sc->Check("intermusic"))
		{
			sc->ExpectString();
			info->InterMusic = *sc->String.ToLower();
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
		//	These are stubs for now.
		else if (sc->Check("cdid"))
		{
			GCon->Logf("Unimplemented MAPINFO comand cdid");
			sc->ExpectString();
		}
		else if (sc->Check("noinventorybar"))
		{
			GCon->Logf("Unimplemented MAPINFO comand noinventorybar");
		}
		else if (sc->Check("airsupply"))
		{
			GCon->Logf("Unimplemented MAPINFO comand airsupply");
			sc->ExpectNumber();
		}
		else if (sc->Check("sndseq"))
		{
			GCon->Logf("Unimplemented MAPINFO comand sndseq");
			sc->ExpectName8();
		}
		else if (sc->Check("sndinfo"))
		{
			GCon->Logf("Unimplemented MAPINFO comand sndinfo");
			sc->ExpectName8();
		}
		else if (sc->Check("soundinfo"))
		{
			GCon->Logf("Unimplemented MAPINFO comand soundinfo");
			sc->ExpectName8();
		}
		else if (sc->Check("allowcrouch"))
		{
			GCon->Logf("Unimplemented MAPINFO comand allowcrouch");
		}
		else if (sc->Check("nocrouch"))
		{
			GCon->Logf("Unimplemented MAPINFO comand nocrouch");
		}
		else if (sc->Check("pausemusicinmenus"))
		{
			GCon->Logf("Unimplemented MAPINFO comand pausemusicinmenus");
		}
		else if (sc->Check("bordertexture"))
		{
			GCon->Logf("Unimplemented MAPINFO comand bordertexture");
			sc->ExpectName8();
		}
		else if (sc->Check("f1"))
		{
			GCon->Logf("Unimplemented MAPINFO comand f1");
			sc->ExpectString();
		}
		else if (sc->Check("allowrespawn"))
		{
			GCon->Logf("Unimplemented MAPINFO comand allowrespawn");
		}
		else if (sc->Check("teamdamage"))
		{
			GCon->Logf("Unimplemented MAPINFO comand teamdamage");
			sc->ExpectFloat();
		}
		else if (sc->Check("fogdensity"))
		{
			GCon->Logf("Unimplemented MAPINFO comand fogdensity");
			sc->ExpectNumber();
		}
		else if (sc->Check("outsidefogdensity"))
		{
			GCon->Logf("Unimplemented MAPINFO comand outsidefogdensity");
			sc->ExpectNumber();
		}
		else if (sc->Check("skyfog"))
		{
			GCon->Logf("Unimplemented MAPINFO comand skyfog");
			sc->ExpectNumber();
		}
		else if (sc->Check("teamplayon"))
		{
			GCon->Logf("Unimplemented MAPINFO comand teamplayon");
		}
		else if (sc->Check("teamplayoff"))
		{
			GCon->Logf("Unimplemented MAPINFO comand teamplayoff");
		}
		else if (sc->Check("checkswitchrange"))
		{
			GCon->Logf("Unimplemented MAPINFO comand checkswitchrange");
		}
		else if (sc->Check("nocheckswitchrange"))
		{
			GCon->Logf("Unimplemented MAPINFO comand nocheckswitchrange");
		}
		else if (sc->Check("translator"))
		{
			GCon->Logf("Unimplemented MAPINFO comand translator");
			sc->ExpectString();
		}
		else if (sc->Check("unfreezesingleplayerconversations"))
		{
			GCon->Logf("Unimplemented MAPINFO comand unfreezesingleplayerconversations");
		}
		else
		{
			break;
		}
	}

	//	Second sky defaults to first sky
	if (info->Sky2Texture == GTextureManager.DefaultTexture)
	{
		info->Sky2Texture = info->Sky1Texture;
	}

	if (info->Flags & MAPINFOF_DoubleSky)
	{
		GTextureManager.SetFrontSkyLayer(info->Sky1Texture);
	}
	unguard;
}

//==========================================================================
//
//	ParseMap
//
//==========================================================================

static void ParseMap(VScriptParser* sc, bool& HexenMode, mapInfo_t& Default)
{
	guard(ParseMap);
	mapInfo_t* info = NULL;
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
	info->LevelNum = Default.LevelNum;
	info->Cluster = Default.Cluster;
	info->WarpTrans = Default.WarpTrans;
	info->NextMap = Default.NextMap;
	info->SecretMap = Default.SecretMap;
	info->SongLump = Default.SongLump;
	info->CDTrack = Default.CDTrack;
	info->Sky1Texture = Default.Sky1Texture;
	info->Sky2Texture = Default.Sky2Texture;
	info->Sky1ScrollDelta = Default.Sky1ScrollDelta;
	info->Sky2ScrollDelta = Default.Sky2ScrollDelta;
	info->SkyBox = Default.SkyBox;
	info->FadeTable = Default.FadeTable;
	info->Fade = Default.Fade;
	info->OutsideFog = Default.OutsideFog;
	info->Gravity = Default.Gravity;
	info->AirControl = Default.AirControl;
	info->Flags = Default.Flags;
	info->Flags2 = Default.Flags2;
	info->TitlePatch = Default.TitlePatch;
	info->ParTime = Default.ParTime;
	info->SuckTime = Default.SuckTime;
	info->HorizWallShade = Default.HorizWallShade;
	info->VertWallShade = Default.VertWallShade;
	info->Infighting = Default.Infighting;
	info->SpecialActions = Default.SpecialActions;
	info->RedirectType = Default.RedirectType;
	info->RedirectMap = Default.RedirectMap;
	info->ExitPic = Default.ExitPic;
	info->EnterPic = Default.EnterPic;
	info->InterMusic = Default.InterMusic;

	if (HexenMode)
	{
		info->Flags |= MAPINFOF_NoIntermission |
			MAPINFOF_FallingDamage |
			MAPINFOF_MonsterFallingDamage |
			MAPINFOF_NoAutoSndSeq |
			MAPINFOF_ActivateOwnSpecial |
			MAPINFOF_MissilesActivateImpact |
			MAPINFOF_InfiniteFlightPowerup;
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

	ParseMapCommon(sc, info, HexenMode);

	//	Avoid duplicate levelnums, later one takes precedance.
	for (int i = 0; i < MapInfo.Num(); i++)
	{
		if (MapInfo[i].LevelNum == info->LevelNum &&
			&MapInfo[i] != info)
		{
			MapInfo[i].LevelNum = 0;
		}
	}
	unguard;
}

//==========================================================================
//
//	ParseClusterDef
//
//==========================================================================

static void ParseClusterDef(VScriptParser* sc)
{
	guard(ParseClusterDef);
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
				CDef->EnterText = sc->String.ToLower();
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
		else if (sc->Check("name"))
		{
			sc->Check("lookup");
			sc->ExpectString();
			GCon->Logf("Unimplemented MAPINFO cluster comand name");
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
	unguard;
}

//==========================================================================
//
//	ParseEpisodeDef
//
//==========================================================================

static void ParseEpisodeDef(VScriptParser* sc)
{
	guard(ParseEpisodeDef);
	VEpisodeDef* EDef = NULL;
	int EIdx = 0;
	sc->ExpectName8();

	//	Check for replaced episode.
	for (int i = 0; i < EpisodeDefs.Num(); i++)
	{
		if (sc->Name8 == EpisodeDefs[i].Name)
		{
			EDef = &EpisodeDefs[i];
			EIdx = i;
			break;
		}
	}
	if (!EDef)
	{
		EDef = &EpisodeDefs.Alloc();
		EIdx = EpisodeDefs.Num() - 1;
	}

	//	Check for removal of an episode.
	if (sc->Check("remove"))
	{
		EpisodeDefs.RemoveIndex(EIdx);
		return;
	}

	//	Set defaults.
	EDef->Name = sc->Name8;
	EDef->TeaserName = NAME_None;
	EDef->Text = VStr();
	EDef->PicName = NAME_None;
	EDef->Flags = 0;
	EDef->Key = VStr();

	if (sc->Check("teaser"))
	{
		sc->ExpectName8();
		EDef->TeaserName = sc->Name8;
	}

	while (1)
	{
		if (sc->Check("name"))
		{
			if (sc->Check("lookup"))
			{
				EDef->Flags |= EPISODEF_LookupText;
				sc->ExpectString();
				EDef->Text = sc->String.ToLower();
			}
			else
			{
				EDef->Flags &= ~EPISODEF_LookupText;
				sc->ExpectString();
				EDef->Text = sc->String;
			}
		}
		else if (sc->Check("picname"))
		{
			sc->ExpectName8();
			EDef->PicName = sc->Name8;
		}
		else if (sc->Check("key"))
		{
			sc->ExpectString();
			EDef->Key = sc->String.ToLower();
		}
		else if (sc->Check("noskillmenu"))
		{
			EDef->Flags |= EPISODEF_NoSkillMenu;
		}
		else if (sc->Check("optional"))
		{
			EDef->Flags |= EPISODEF_Optional;
		}
		else
		{
			break;
		}
	}
	unguard;
}

//==========================================================================
//
//	ParseSkillDef
//
//==========================================================================

static void ParseSkillDef(VScriptParser* sc)
{
	guard(ParseSkillDef);
	VSkillDef* SDef = NULL;
	sc->ExpectString();

	//	Check for replaced skill.
	for (int i = 0; i < SkillDefs.Num(); i++)
	{
		if (!sc->String.ICmp(SkillDefs[i].Name))
		{
			SDef = &SkillDefs[i];
			break;
		}
	}
	if (!SDef)
	{
		SDef = &SkillDefs.Alloc();
		SDef->Name = sc->String;
	}

	//	Set defaults.
	SDef->AmmoFactor = 1.0;
	SDef->DoubleAmmoFactor = 2.0;
	SDef->DamageFactor = 1.0;
	SDef->RespawnTime = 0.0;
	SDef->RespawnLimit = 0;
	SDef->Aggressiveness = 1.0;
	SDef->SpawnFilter = 0;
	SDef->AcsReturn = SkillDefs.Num() - 1;
	SDef->MenuName.Clean();
	SDef->PlayerClassNames.Clear();
	SDef->ConfirmationText.Clean();
	SDef->Key.Clean();
	SDef->TextColour.Clean();
	SDef->Flags = 0;

	while (1)
	{
		if (sc->Check("AmmoFactor"))
		{
			sc->ExpectFloat();
			SDef->AmmoFactor = sc->Float;
		}
		else if (sc->Check("DoubleAmmoFactor"))
		{
			sc->ExpectFloat();
			SDef->DoubleAmmoFactor = sc->Float;
		}
		else if (sc->Check("DamageFactor"))
		{
			sc->ExpectFloat();
			SDef->DamageFactor = sc->Float;
		}
		else if (sc->Check("FastMonsters"))
		{
			SDef->Flags |= SKILLF_FastMonsters;
		}
		else if (sc->Check("DisableCheats"))
		{
			SDef->Flags |= SKILLF_DisableCheats;
		}
		else if (sc->Check("EasyBossBrain"))
		{
			SDef->Flags |= SKILLF_EasyBossBrain;
		}
		else if (sc->Check("AutoUseHealth"))
		{
			SDef->Flags |= SKILLF_AutoUseHealth;
		}
		else if (sc->Check("RespawnTime"))
		{
			sc->ExpectFloat();
			SDef->RespawnTime = sc->Float;
		}
		else if (sc->Check("RespawnLimit"))
		{
			sc->ExpectNumber();
			SDef->RespawnLimit = sc->Number;
		}
		else if (sc->Check("Aggressiveness"))
		{
			sc->ExpectFloat();
			SDef->Aggressiveness = 1.0 - MID(0.0, sc->Float, 1.0);
		}
		else if (sc->Check("SpawnFilter"))
		{
			if (sc->CheckNumber())
			{
				if (sc->Number > 0)
				{
					SDef->SpawnFilter = 1 << (sc->Number - 1);
				}
			}
			else
			{
				if (sc->Check("Baby"))
				{
					SDef->SpawnFilter = 1;
				}
				else if (sc->Check("Easy"))
				{
					SDef->SpawnFilter = 2;
				}
				else if (sc->Check("Normal"))
				{
					SDef->SpawnFilter = 4;
				}
				else if (sc->Check("Hard"))
				{
					SDef->SpawnFilter = 8;
				}
				else if (sc->Check("Nightmare"))
				{
					SDef->SpawnFilter = 16;
				}
				else
				{
					sc->ExpectString();
				}
			}
		}
		else if (sc->Check("ACSReturn"))
		{
			sc->ExpectNumber();
			SDef->AcsReturn = sc->Number;
		}
		else if (sc->Check("Name"))
		{
			sc->ExpectString();
			SDef->MenuName = sc->String;
			SDef->Flags &= ~SKILLF_MenuNameIsPic;
		}
		else if (sc->Check("PlayerClassName"))
		{
			VSkillPlayerClassName& CN = SDef->PlayerClassNames.Alloc();
			sc->ExpectString();
			CN.ClassName = sc->String;
			sc->ExpectString();
			CN.MenuName = sc->String;
		}
		else if (sc->Check("PicName"))
		{
			sc->ExpectString();
			SDef->MenuName = sc->String.ToLower();
			SDef->Flags |= SKILLF_MenuNameIsPic;
		}
		else if (sc->Check("MustConfirm"))
		{
			SDef->Flags |= SKILLF_MustConfirm;
			if (sc->CheckQuotedString())
			{
				SDef->ConfirmationText = sc->String;
			}
		}
		else if (sc->Check("Key"))
		{
			sc->ExpectString();
			SDef->Key = sc->String;
		}
		else if (sc->Check("TextColor"))
		{
			sc->ExpectString();
			SDef->TextColour = sc->String;
		}
		else
		{
			break;
		}
	}
	unguard;
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

	//	Set up default map info.
	mapInfo_t Default;
	SetMapDefaults(Default);

	while (!sc->AtEnd())
	{
		if (sc->Check("map"))
		{
			ParseMap(sc, HexenMode, Default);
		}
		else if (sc->Check("defaultmap"))
		{
			SetMapDefaults(Default);
			ParseMapCommon(sc, &Default, HexenMode);
		}
		else if (sc->Check("adddefaultmap"))
		{
			ParseMapCommon(sc, &Default, HexenMode);
		}
		else if (sc->Check("clusterdef"))
		{
			ParseClusterDef(sc);
		}
		else if (sc->Check("episode"))
		{
			ParseEpisodeDef(sc);
		}
		else if (sc->Check("clearepisodes"))
		{
			EpisodeDefs.Clear();
		}
		else if (sc->Check("skill"))
		{
			ParseSkillDef(sc);
		}
		else if (sc->Check("clearskills"))
		{
			SkillDefs.Clear();
		}
		else
		{
			sc->Error(va("Invalid command %s", *sc->String));
		}
	}
	delete sc;
	sc = NULL;
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
	return *MapInfo[QualifyMap(map)].GetName();
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
//	P_GetNumEpisodes
//
//==========================================================================

int P_GetNumEpisodes()
{
	return EpisodeDefs.Num();
}

//==========================================================================
//
//	P_GetEpisodeDef
//
//==========================================================================

VEpisodeDef* P_GetEpisodeDef(int Index)
{
	return &EpisodeDefs[Index];
}

//==========================================================================
//
//	P_GetNumSkills
//
//==========================================================================

int P_GetNumSkills()
{
	return SkillDefs.Num();
}

//==========================================================================
//
//	P_GetSkillDef
//
//==========================================================================

const VSkillDef* P_GetSkillDef(int Index)
{
	return &SkillDefs[Index];
}

//==========================================================================
//
//	P_GetMusicLumpNames
//
//==========================================================================

void P_GetMusicLumpNames(TArray<FReplacedString>& List)
{
	guard(P_GetMusicLumpNames);
	for (int i = 0; i < MapInfo.Num(); i++)
	{
		const char* MName = *MapInfo[i].SongLump;
		if (MName[0] == 'd' && MName[1] == '_')
		{
			FReplacedString& R = List.Alloc();
			R.Index = i;
			R.Replaced = false;
			R.Old = MName + 2;
		}
	}
	unguard;
}

//==========================================================================
//
//	P_ReplaceMusicLumpNames
//
//==========================================================================

void P_ReplaceMusicLumpNames(TArray<FReplacedString>& List)
{
	guard(P_ReplaceMusicLumpNames);
	for (int i = 0; i < List.Num(); i++)
	{
		if (List[i].Replaced)
		{
			MapInfo[List[i].Index].SongLump = VName(*(VStr("d_") +
				List[i].New), VName::AddLower8);
		}
	}
	unguard;
}

//==========================================================================
//
//	P_SetParTime
//
//==========================================================================

void P_SetParTime(VName Map, int Par)
{
	guard(P_SetParTime);
	for (int i = 0; i < MapInfo.Num(); i++)
	{
		if (MapInfo[i].LumpName == Map)
		{
			MapInfo[i].ParTime = Par;
			return;
		}
	}
	GCon->Logf("WARNING! No such map %s", *Map);
	unguard;
}

//==========================================================================
//
//	IsMapPresent
//
//==========================================================================

bool IsMapPresent(VName MapName)
{
	guard(IsMapPresent);
	if (W_CheckNumForName(MapName) >= 0)
	{
		return true;
	}
	VStr FileName = va("maps/%s.wad", *MapName);
	if (FL_FileExists(FileName))
	{
		return true;
	}
	return false;
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
		if (IsMapPresent(MapInfo[i].LumpName))
		{
			GCon->Log(VStr(MapInfo[i].LumpName) + " - " +
				((MapInfo[i].Flags & MAPINFOF_LookupName) ?
				GLanguage[*MapInfo[i].Name] : MapInfo[i].Name));
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
	EpisodeDefs.Clear();
	SkillDefs.Clear();
	unguard;
}
