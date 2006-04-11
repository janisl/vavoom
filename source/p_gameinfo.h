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
		svpr.Exec(GetVFunction("Init"), (int)this);
	}
	void eventInitNewGame(int skill)
	{
		svpr.Exec(GetVFunction("InitNewGame"), (int)this, skill);
	}
	VLevelInfo* eventCreateLevelInfo()
	{
		return (VLevelInfo*)svpr.Exec(GetVFunction("CreateLevelInfo"), (int)this);
	}
	void eventTranslateLevel(VLevel* InLevel)
	{
		svpr.Exec(GetVFunction("TranslateLevel"), (int)this, (int)InLevel);
	}
	void eventSpawnWorld(VLevel* InLevel)
	{
		svpr.Exec(GetVFunction("SpawnWorld"), (int)this, (int)InLevel);
	}
	VName eventGetConScriptName(VName LevelName)
	{
		vint32 Tmp = svpr.Exec(GetVFunction("GetConScriptName"), (int)this, LevelName.GetIndex());
		return *(VName*)&Tmp;
	}
};

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2006/04/11 18:28:20  dj_jl
//	Removed Strife specific mapinfo extensions.
//
//	Revision 1.1  2006/03/06 13:12:12  dj_jl
//	Client now uses entity class.
//	
//**************************************************************************
