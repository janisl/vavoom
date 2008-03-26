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

#ifndef _SV_LOCAL_H
#define _SV_LOCAL_H

#define MAXHEALTH				100
#define DEFAULT_GRAVITY			1225.0

struct tmtrace_t;
class VMessageOut;

extern VGameInfo*		GGameInfo;
extern VWorldInfo*		GWorldInfo;
extern VLevelInfo*		GLevelInfo;

//==========================================================================
//
//	sv_acs
//
//	Action code scripts
//
//==========================================================================

//	Script types
enum
{
	SCRIPT_Closed		= 0,
	SCRIPT_Open			= 1,
	SCRIPT_Respawn		= 2,
	SCRIPT_Death		= 3,
	SCRIPT_Enter		= 4,
	SCRIPT_Pickup		= 5,
	SCRIPT_BlueReturn	= 6,
	SCRIPT_RedReturn	= 7,
	SCRIPT_WhiteReturn	= 8,
	SCRIPT_Lightning	= 12,
	SCRIPT_Unloading	= 13,
	SCRIPT_Disconnect	= 14,
};

class VAcs;
class VAcsObject;
struct VAcsInfo;

class VAcsLevel
{
private:
	bool AddToACSStore(int Type, VName Map, int Number, int Arg1, int Arg2,
		int Arg3, VEntity* Activator);

public:
	VLevel*				XLevel;

	TArray<VAcsObject*>	LoadedObjects;

	VAcsLevel(VLevel* ALevel);
	~VAcsLevel();

	VAcsObject* LoadObject(int Lump);
	VAcsInfo* FindScript(int Number, VAcsObject*& Object);
	VStr GetString(int Index);
	VAcsObject* GetObject(int Index);
	void StartTypedACScripts(int Type, int Arg1, int Arg2, int Arg3,
		VEntity* Activator, bool Always, bool RunNow);
	void Serialise(VStream& Strm);
	void CheckAcsStore();
	bool Start(int Number, int MapNum, int Arg1, int Arg2, int Arg3,
		VEntity* Activator, line_t* Line, int Side, bool Always,
		bool WantResult);
	bool Terminate(int Number, int MapNum);
	bool Suspend(int Number, int MapNum);
	VAcs* SpawnScript(VAcsInfo* Info, VAcsObject* Object, VEntity* Activator,
		line_t* Line, int Side, int Arg1, int Arg2, int Arg3, bool Always,
		bool Delayed);
};

class VAcsGrowingArray
{
private:
	vint32		Size;
	vint32*		Data;
public:
	VAcsGrowingArray();
	void Redim(int NewSize);
	void SetElemVal(int Index, int Value);
	int GetElemVal(int Index);
	void Serialise(VStream& Strm);
};

struct VAcsStore
{
	enum
	{
		Start,
		StartAlways,
		Terminate,
		Suspend
	};

	VName		Map;		//	Target map
	vuint8		Type;		//	Type of action
	vint8		PlayerNum;	//	Player who executes this script
	vint32		Script;		//	Script number on target map
	vint32		Args[3];	//	Arguments
};

class VAcsGlobal
{
public:
	enum
	{
		MAX_ACS_WORLD_VARS	= 256,
		MAX_ACS_GLOBAL_VARS	= 64,
	};

	int					WorldVars[MAX_ACS_WORLD_VARS];
	int					GlobalVars[MAX_ACS_GLOBAL_VARS];
	VAcsGrowingArray	WorldArrays[MAX_ACS_WORLD_VARS];
	VAcsGrowingArray	GlobalArrays[MAX_ACS_GLOBAL_VARS];
	TArray<VAcsStore>	Store;

	VAcsGlobal();

	void Serialise(VStream& Strm);
};

//==========================================================================
//
//	sv_world
//
//	Map utilites
//
//==========================================================================

struct opening_t
{
	float		top;
	float		bottom;
	float		range;
	float		lowfloor;
	sec_plane_t	*floor;
	sec_plane_t	*ceiling;
	opening_t	*next;
};

opening_t *SV_LineOpenings(const line_t* linedef, const TVec& point, int NoBlockFlags);

int P_BoxOnLineSide(float* tmbox, line_t* ld);

sec_region_t *SV_FindThingGap(sec_region_t *gaps, const TVec &point, float z1, float z2);
opening_t *SV_FindOpening(opening_t *gaps, float z1, float z2);
sec_region_t *SV_PointInRegion(sector_t *sector, const TVec &p);
int SV_PointContents(const sector_t *sector, const TVec &p);

//==========================================================================
//
//	sv_switch
//
//	Switches
//
//==========================================================================

void P_InitSwitchList();

void P_InitTerrainTypes();
struct VTerrainInfo* SV_TerrainType(int pic);
void P_FreeTerrainTypes();

//==========================================================================
//
//	sv_tick
//
//	Handling thinkers, running tics
//
//==========================================================================

void P_InitThinkers();
void P_Ticker();

extern int 			TimerGame; // tic countdown for deathmatch

//==========================================================================
//
//	sv_main
//
//==========================================================================

void SV_ChangeSky(const char*, const char*);
void SV_ChangeMusic(const char*);
void SV_ChangeLocalMusic(VBasePlayer*, const char*);
void SV_ReadMove();

void Draw_TeleportIcon();
void CL_Disconnect();
void SV_SetUserInfo(VBasePlayer* Player, const VStr& info);

void SV_DropClient(VBasePlayer* Player, bool crash);
void SV_ShutdownServer(bool);
void SV_SpawnServer(const char*, bool, bool);
void SV_SendServerInfoToClients();

extern int 		LeavePosition;
extern bool		completed;

//==========================================================================
//
//	sv_user
//
//==========================================================================

//==========================================================================
//
//	????
//
//==========================================================================

// Difficulty/skill settings/filters.
enum skill_t
{
	sk_baby,
	sk_easy,
	sk_medium,
	sk_hard,
	sk_nightmare
};

void G_TeleportNewMap(int map, int position);
void G_WorldDone();
void G_PlayerReborn(int player);
void G_StartNewInit();

extern VBasePlayer*		GPlayersBase[MAXPLAYERS]; // Bookkeeping on players - state.

extern skill_t			gameskill;
 
extern bool				netgame;                // only true if packets are broadcast
extern vuint8			deathmatch;   			// only if started as net death

extern bool				paused;

extern bool				sv_loading;

//==========================================================================
//
//	Inlines
//
//==========================================================================

inline int SV_GetPlayerNum(VBasePlayer* player)
{
	int		i;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (player == GPlayersBase[i])
		{
		    return i;
		}
	}
	return 0;
}

#endif
