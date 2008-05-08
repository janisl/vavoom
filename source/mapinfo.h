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

enum
{
	MAPINFOF_DoubleSky					= 0x00000001,	// parallax sky: sky2 behind sky1
	MAPINFOF_Lightning					= 0x00000002,	// Use of lightning on the level flashes from sky1 to sky2
	MAPINFOF_Map07Special				= 0x00000004,
	MAPINFOF_BaronSpecial				= 0x00000008,
	MAPINFOF_CyberDemonSpecial			= 0x00000010,
	MAPINFOF_SpiderMastermindSpecial	= 0x00000020,
	MAPINFOF_MinotaurSpecial			= 0x00000040,
	MAPINFOF_DSparilSpecial				= 0x00000080,
	MAPINFOF_IronLichSpecial			= 0x00000100,
	MAPINFOF_SpecialActionOpenDoor		= 0x00000200,
	MAPINFOF_SpecialActionLowerFloor	= 0x00000400,
	MAPINFOF_SpecialActionKillMonsters	= 0x00000800,
	MAPINFOF_NoIntermission				= 0x00001000,
	MAPINFOF_AllowMonsterTelefrags		= 0x00002000,
	MAPINFOF_NoAllies					= 0x00004000,
	MAPINFOF_DeathSlideShow				= 0x00008000,
	MAPINFOF_ForceNoSkyStretch			= 0x00010000,
	MAPINFOF_LookupName					= 0x00020000,
	MAPINFOF_FallingDamage				= 0x00040000,
	MAPINFOF_OldFallingDamage			= 0x00080000,
	MAPINFOF_StrifeFallingDamage		= 0x00100000,
	MAPINFOF_MonsterFallingDamage		= 0x00200000,
	MAPINFOF_NoFreelook					= 0x00400000,
	MAPINFOF_NoJump						= 0x00800000,
	MAPINFOF_NoAutoSndSeq				= 0x01000000,
	MAPINFOF_ActivateOwnSpecial			= 0x02000000,
	MAPINFOF_MissilesActivateImpact		= 0x04000000,
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
	vuint32		Fade;
	vuint32		OutsideFog;
	float		Gravity;		// Map gravity
	float		AirControl;		// Air control in this map.
	vuint32		Flags;
	VName		TitlePatch;
	vint32		ParTime;
	vint32		SuckTime;

	VStr GetName() const
	{
		return Flags & MAPINFOF_LookupName ? GLanguage[*Name] : Name;
	}
};

enum
{
	CLUSTERF_Hub				= 0x01,
	CLUSTERF_EnterTextIsLump	= 0x02,
	CLUSTERF_ExitTextIsLump		= 0x04,
	CLUSTERF_FinalePic			= 0x80,
	CLUSTERF_LookupEnterText	= 0x10,
	CLUSTERF_LookupExitText		= 0x20,
};

struct VClusterDef
{
	vint32		Cluster;
	vint32		Flags;
	VStr		EnterText;
	VStr		ExitText;
	VName		Flat;
	VName		Music;
	vint32		CDTrack;
	vint32		CDId;
};

enum
{
	EPISODEF_LookupText		= 0x0001,
	EPISODEF_NoSkillMenu	= 0x0002,
};

struct VEpisodeDef
{
	VName		Name;
	VName		TeaserName;
	VStr		Text;
	VName		PicName;
	vuint32		Flags;
	VStr		Key;
};

void InitMapInfo();
void ShutdownMapInfo();
const mapInfo_t& P_GetMapInfo(VName);
const char* P_GetMapName(int);
VName P_GetMapLumpName(int);
VName P_TranslateMap(int);
VName P_GetMapNameByLevelNum(int);
void P_PutMapSongLump(int, VName);
const VClusterDef* P_GetClusterDef(int);
int P_GetNumEpisodes();
VEpisodeDef* P_GetEpisodeDef(int);
void P_GetMusicLumpNames(TArray<FReplacedString>&);
void P_ReplaceMusicLumpNames(TArray<FReplacedString>&);
void P_SetParTime(VName, int);
int P_GetCDStartTrack();
int P_GetCDEnd1Track();
int P_GetCDEnd2Track();
int P_GetCDEnd3Track();
int P_GetCDIntermissionTrack();
int P_GetCDTitleTrack();
