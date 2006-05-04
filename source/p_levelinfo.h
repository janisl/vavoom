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

class VLevelInfo : public VThinker
{
	DECLARE_CLASS(VLevelInfo, VThinker, 0)

	enum { MAX_TID_COUNT = 200 };

	VGameInfo*		Game;

	// Maintain single and multi player starting spots.
	mthing_t		DeathmatchStarts[MAXDEATHMATCHSTARTS];  // Player spawn spots for deathmatch.
	int				NumDeathmatchStarts;
	mthing_t		PlayerStarts[MAX_PLAYER_STARTS * MAXPLAYERS];// Player spawn spots.

	int				TIDList[MAX_TID_COUNT + 1];	// +1 for termination marker
	VEntity*		TIDMobj[MAX_TID_COUNT];

	TVec			trace_start;
	TVec			trace_end;
	TVec			trace_plane_normal;

	TVec			linestart;
	TVec			lineend;

	VLevelInfo();

	void eventSpawnSpecials()
	{
		svpr.Exec(GetVFunction("SpawnSpecials"), (int)this);
	}
	void eventUpdateSpecials()
	{
		svpr.Exec(GetVFunction("UpdateSpecials"), (int)this);
	}
	void eventAfterUnarchiveThinkers()
	{
		svpr.Exec(GetVFunction("AfterUnarchiveThinkers"), (int)this);
	}
	line_t* eventFindLine(int lineTag, int *searchPosition)
	{
		return (line_t*)svpr.Exec(GetVFunction("FindLine"), (int)this,
			lineTag, (int)searchPosition);
	}
	void eventPolyThrustMobj(VEntity* A, TVec thrustDir, polyobj_t* po)
	{
		svpr.Exec(GetVFunction("PolyThrustMobj"), (int)this, (int)A,
			PassFloat(thrustDir.x), PassFloat(thrustDir.y), PassFloat(thrustDir.z), (int)po);
	}
	bool eventTagBusy(int tag)
	{
		return !!svpr.Exec(GetVFunction("TagBusy"), (int)this, tag);
	}
	bool eventPolyBusy(int polyobj)
	{
		return !!svpr.Exec(GetVFunction("PolyBusy"), (int)this, polyobj);
	}
	int eventThingCount(int type, int tid)
	{
		return svpr.Exec(GetVFunction("ThingCount"), (int)this, type, tid);
	}
	VEntity* eventFindMobjFromTID(int tid, int *searchPosition)
	{
		return (VEntity*)svpr.Exec(GetVFunction("FindMobjFromTID"), (int)this, tid, (int)searchPosition);
	}
	bool eventExecuteActionSpecial(int Special, int Arg1, int Arg2, int Arg3,
		int Arg4, int Arg5, line_t* Line, int Side, VEntity* A)
	{
		return !!svpr.Exec(GetVFunction("ExecuteActionSpecial"), (int)this,
			Special, Arg1, Arg2, Arg3, Arg4, Arg5, (int)Line, Side, (int)A);
	}
	int eventEV_ThingProjectile(int tid, int type, int angle, int speed,
		int vspeed, int gravity, int newtid)
	{
		return svpr.Exec(GetVFunction("EV_ThingProjectile"), (int)this, tid,
			type, angle, speed, vspeed, gravity, newtid);
	}
	void eventStartPlaneWatcher(VEntity* it, line_t* line, int lineSide,
		bool ceiling, int tag, int height, int special, int arg1, int arg2,
		int arg3, int arg4, int arg5)
	{
		svpr.Exec(GetVFunction("StartPlaneWatcher"), (int)this, (int)it,
			(int)line, lineSide, ceiling, tag, height, special, arg1,
			arg2, arg3, arg4, arg5);
	}
	void eventSpawnMapThing(mthing_t* mthing)
	{
		svpr.Exec(GetVFunction("SpawnMapThing"), (int)this, (int)mthing);
	}
};
