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

class VGameInfo : public VObject
{
	DECLARE_CLASS(VGameInfo, VObject, 0)

	VName			AcsHelper;
	VName			GenericConScript;

	vuint8			netgame;
	vuint8			deathmatch;
	vuint8			gameskill;
	vuint8			respawn;
	vuint8			nomonsters;
	vuint8			fastparm;

	vint32*			validcount;
	vint32			skyflatnum;

	VBasePlayer*	Players[MAXPLAYERS]; // Bookkeeping on players - state.

	vint32			RebornPosition;

	float			frametime;

	float			FloatBobOffsets[64];
	vint32			PhaseTable[64];

	VClass*			LevelInfoClass;
	VClass*			PlayerReplicationInfoClass;

	vint32			GameFilterFlag;

	TArray<VClass*>	PlayerClasses;

	enum
	{
		GIF_DefaultMonsterFallingDamage		= 0x00000001,
	};
	vuint32			Flags;

	VGameInfo()
	: PlayerClasses(E_NoInit)
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
	VWorldInfo* eventCreateWorldInfo()
	{
		P_PASS_SELF;
		EV_RET_REF(VWorldInfo, "CreateWorldInfo");
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
