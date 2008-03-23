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
//**
//**	Self registering cheat commands.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  CheatAllowed
//
//==========================================================================

static bool CheatAllowed()
{
   	if (!sv.active || sv.intermission || !GGameInfo->Players[0])
    {
		GCon->Log("You are not in game!");
        return false;
	}
  	if (netgame)
    {
		GCon->Log("You cannot cheat in a network game!");
		return false;
	}
	if (gameskill == sk_nightmare)
	{
		GCon->Log("You are too good to cheat!");
		return false;
	}
	if (GGameInfo->Players[0]->Health <= 0)
	{
		// Dead players can't cheat
        GCon->Log("You must be alive to cheat");
		return false;
	}
	return true;
}

//==========================================================================
//
//  God_f
//
// 	Cheat code GOD
//
//==========================================================================

COMMAND(God)
{
	if (CheatAllowed())
    {
		GGameInfo->Players[0]->eventCheat_God();
    }
}

//==========================================================================
//
//  NoClip_f
//
//==========================================================================

COMMAND(NoClip)
{
	if (CheatAllowed())
    {
		GGameInfo->Players[0]->eventCheat_NoClip();
    }
}

//==========================================================================
//
//  Gimme_f
//
//==========================================================================

COMMAND(Gimme)
{
	if (CheatAllowed())
    {
		GGameInfo->Players[0]->eventCheat_Gimme();
    }
}

//==========================================================================
//
//  KillAll_f
//
//==========================================================================

COMMAND(KillAll)
{
	if (CheatAllowed())
    {
		GGameInfo->Players[0]->eventCheat_KillAll();
    }
}

//==========================================================================
//
//  Morph_f
//
//==========================================================================

COMMAND(Morph)
{
	if (CheatAllowed())
    {
		GGameInfo->Players[0]->eventCheat_Morph();
    }
}

//==========================================================================
//
//  NoWeapons_f
//
//==========================================================================

COMMAND(NoWeapons)
{
	if (CheatAllowed())
    {
		GGameInfo->Players[0]->eventCheat_NoWeapons();
    }
}

//==========================================================================
//
//  Class_f
//
//==========================================================================

COMMAND(ChangeClass)
{
	if (CheatAllowed())
    {
		GGameInfo->Players[0]->eventCheat_Class();
    }
}

//==========================================================================
//
//	Script_f
//
//==========================================================================

COMMAND(Script)
{
	if (CheatAllowed())
    {
		int 	script;

        if (Args.Num() != 2)
			return;
		script = atoi(*Args[1]);
		if (script < 1)
			return;
		if (script > 9999)
			return;

		if (GLevel->Acs->Start(script, 0, 0, 0, 0, GGameInfo->Players[0]->MO,
			NULL, 0, false, false))
		{
			GCon->Logf("Running script %d", script);
		}
    }
}

//==========================================================================
//
//  MyPos_f
//
//==========================================================================

COMMAND(MyPos)
{
	if (CheatAllowed())
    {
		GCon->Logf("MAP %s  X:%f  Y:%f  Z:%f  Yaw:%f Pitch:%f",
			*GLevel->MapName, GGameInfo->Players[0]->MO->Origin.x,
			GGameInfo->Players[0]->MO->Origin.y, GGameInfo->Players[0]->MO->Origin.z,
			GGameInfo->Players[0]->MO->Angles.yaw, GGameInfo->Players[0]->MO->Angles.pitch);
	}
}

//==========================================================================
//
//  Fly_f
//
//==========================================================================

COMMAND(Fly)
{
	if (CheatAllowed())
    {
		GGameInfo->Players[0]->eventCheat_Fly();
	}
}

//==========================================================================
//
//  NoTaget_f
//
//==========================================================================

COMMAND(NoTarget)
{
	if (CheatAllowed())
    {
		GGameInfo->Players[0]->eventCheat_NoTarget();
	}
}
