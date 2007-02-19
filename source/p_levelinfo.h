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

class VLevelInfo : public VThinker
{
	DECLARE_CLASS(VLevelInfo, VThinker, 0)

	enum { MAX_TID_COUNT = 200 };

	VGameInfo*		Game;
	VWorldInfo*		World;

	enum
	{
		LIF_BegunPlay					= 0x00000001,
	};
	vuint32			LevelInfoFlags;

	// Maintain single and multi player starting spots.
	mthing_t		DeathmatchStarts[MAXDEATHMATCHSTARTS];  // Player spawn spots for deathmatch.
	int				NumDeathmatchStarts;
	mthing_t		PlayerStarts[MAX_PLAYER_STARTS * MAXPLAYERS];// Player spawn spots.

	int				TIDList[MAX_TID_COUNT + 1];	// +1 for termination marker
	VEntity*		TIDMobj[MAX_TID_COUNT];

	float			Gravity;								// Level Gravity

	VLevelInfo();

	void eventSpawnSpecials()
	{
		P_PASS_SELF;
		EV_RET_VOID("SpawnSpecials");
	}
	void eventUpdateSpecials()
	{
		P_PASS_SELF;
		EV_RET_VOID("UpdateSpecials");
	}
	void eventAfterUnarchiveThinkers()
	{
		P_PASS_SELF;
		EV_RET_VOID("AfterUnarchiveThinkers");
	}
	line_t* eventFindLine(int lineTag, int *searchPosition)
	{
		P_PASS_SELF;
		P_PASS_INT(lineTag);
		P_PASS_PTR(searchPosition);
		EV_RET_PTR(line_t, "FindLine");
	}
	void eventPolyThrustMobj(VEntity* A, TVec thrustDir, polyobj_t* po)
	{
		P_PASS_SELF;
		P_PASS_REF(A);
		P_PASS_VEC(thrustDir);
		P_PASS_PTR(po);
		EV_RET_VOID("PolyThrustMobj");
	}
	bool eventTagBusy(int tag)
	{
		P_PASS_SELF;
		P_PASS_INT(tag);
		EV_RET_BOOL("TagBusy");
	}
	bool eventPolyBusy(int polyobj)
	{
		P_PASS_SELF;
		P_PASS_INT(polyobj);
		EV_RET_BOOL("PolyBusy");
	}
	int eventThingCount(int type, int tid)
	{
		P_PASS_SELF;
		P_PASS_INT(type);
		P_PASS_INT(tid);
		EV_RET_INT("ThingCount");
	}
	VEntity* eventFindMobjFromTID(int tid, int *searchPosition)
	{
		P_PASS_SELF;
		P_PASS_INT(tid);
		P_PASS_PTR(searchPosition);
		EV_RET_REF(VEntity, "FindMobjFromTID");
	}
	bool eventExecuteActionSpecial(int Special, int Arg1, int Arg2, int Arg3,
		int Arg4, int Arg5, line_t* Line, int Side, VEntity* A)
	{
		P_PASS_SELF;
		P_PASS_INT(Special);
		P_PASS_INT(Arg1);
		P_PASS_INT(Arg2);
		P_PASS_INT(Arg3);
		P_PASS_INT(Arg4);
		P_PASS_INT(Arg5);
		P_PASS_PTR(Line);
		P_PASS_INT(Side);
		P_PASS_REF(A);
		EV_RET_BOOL("ExecuteActionSpecial");
	}
	int eventEV_ThingProjectile(int tid, int type, int angle, int speed,
		int vspeed, int gravity, int newtid)
	{
		P_PASS_SELF;
		P_PASS_INT(tid);
		P_PASS_INT(type);
		P_PASS_INT(angle);
		P_PASS_INT(speed);
		P_PASS_INT(vspeed);
		P_PASS_INT(gravity);
		P_PASS_INT(newtid);
		EV_RET_INT("EV_ThingProjectile");
	}
	void eventStartPlaneWatcher(VEntity* it, line_t* line, int lineSide,
		bool ceiling, int tag, int height, int special, int arg1, int arg2,
		int arg3, int arg4, int arg5)
	{
		P_PASS_SELF;
		P_PASS_REF(it);
		P_PASS_PTR(line);
		P_PASS_INT(lineSide);
		P_PASS_BOOL(ceiling);
		P_PASS_INT(tag);
		P_PASS_INT(height);
		P_PASS_INT(special);
		P_PASS_INT(arg1);
		P_PASS_INT(arg2);
		P_PASS_INT(arg3);
		P_PASS_INT(arg4);
		P_PASS_INT(arg5);
		EV_RET_VOID("StartPlaneWatcher");
	}
	void eventSpawnMapThing(mthing_t* mthing)
	{
		P_PASS_SELF;
		P_PASS_PTR(mthing);
		EV_RET_VOID("SpawnMapThing");
	}
};
