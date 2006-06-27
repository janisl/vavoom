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

struct tmtrace_t;

class VGameInfo : public VObject
{
	DECLARE_CLASS(VGameInfo, VObject, 0)

	VName			AcsHelper;
	VName			GenericConScript;

	int				netgame;
	int				deathmatch;
	int				gameskill;
	int				respawn;
	int				nomonsters;
	int				fastparm;

	int*			validcount;
	int				skyflatnum;

	VBasePlayer*	Players[MAXPLAYERS]; // Bookkeeping on players - state.

	level_t*		level;

	tmtrace_t*		tmtrace;

	int				RebornPosition;

	float			frametime;

	int				num_stats;

	float			FloatBobOffsets[64];
	int				PhaseTable[64];

	VGameInfo()
	{}

	void eventInit()
	{
		P_PASS_SELF;
		EV_RET_VOID("Init");
	}
	void eventInitNewGame(int skill)
	{
		P_PASS_SELF;
		P_PASS_INT(skill);
		EV_RET_VOID("InitNewGame");
	}
	VLevelInfo* eventCreateLevelInfo()
	{
		P_PASS_SELF;
		EV_RET_REF(VLevelInfo, "CreateLevelInfo");
	}
	void eventTranslateLevel(VLevel* InLevel)
	{
		P_PASS_SELF;
		P_PASS_REF(InLevel);
		EV_RET_VOID("TranslateLevel");
	}
	void eventSpawnWorld(VLevel* InLevel)
	{
		P_PASS_SELF;
		P_PASS_REF(InLevel);
		EV_RET_VOID("SpawnWorld");
	}
	VName eventGetConScriptName(VName LevelName)
	{
		P_PASS_SELF;
		P_PASS_NAME(LevelName);
		EV_RET_NAME("GetConScriptName");
	}
};
