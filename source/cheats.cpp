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

static boolean CheatAllowed(void)
{
   	if (!sv.active || sv.intermission || !svvars.Players[0])
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
	if (svvars.Players[0]->Health <= 0)
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
		svpr.Exec("Cheat_God");
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
		svpr.Exec("Cheat_NoClip");
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
		svpr.Exec("Cheat_Gimme");
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
		svpr.Exec("Cheat_KillAll");
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
		svpr.Exec("Cheat_Morph");
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
		svpr.Exec("Cheat_NoWeapons");
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
		svpr.Exec("Cheat_Class");
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
		int 	args[3];

        if (Argc() != 2) return;
		script = atoi(Argv(1));
		if (script < 1) return;
		if (script > 99) return;
		args[0] = args[1] = args[2] = 0;

		if (P_StartACS(script, 0, args, svvars.Players[0]->MO, NULL, 0))
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
			level.mapname, svvars.Players[0]->MO->Origin.x,
			svvars.Players[0]->MO->Origin.y, svvars.Players[0]->MO->Origin.z,
			svvars.Players[0]->MO->Angles.yaw, svvars.Players[0]->MO->Angles.pitch);
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.10  2003/11/12 16:47:40  dj_jl
//	Changed player structure into a class
//
//	Revision 1.9  2003/07/11 16:45:20  dj_jl
//	Made array of players with pointers
//	
//	Revision 1.8  2002/07/23 16:29:55  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.7  2002/02/15 19:12:29  dj_jl
//	Property namig style change
//	
//	Revision 1.6  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.5  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.4  2001/10/04 17:18:23  dj_jl
//	Implemented the rest of cvar flags
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
