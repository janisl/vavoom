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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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
   	if (!sv.active || sv.intermission)
    {
		con << "You are not in game!\n";
        return false;
	}
  	if (netgame)
    {
		con << "You cannot cheat in a network game!\n";
		return false;
	}
	if (gameskill == sk_nightmare)
	{
		con << "You are too good to cheat!\n";
		return false;
	}
	if (players[0].health <= 0)
	{
		// Dead players can't cheat
        con << "You must be alive to cheat\n";
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
//  AllMap_f
//
//==========================================================================

COMMAND(AllMap)
{
	if (CheatAllowed())
    {
		am_cheating = (am_cheating + 1) % 3;
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

		if (P_StartACS(script, 0, args, players[0].mo, NULL, 0))
		{
			con << "RUNNING SCRIPT " << script << endl;
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
		con << "MAP " << level.mapname
			<< "  X:" << players[0].mo->origin.x
			<< "  Y:" << players[0].mo->origin.y
			<< "  Z:" << players[0].mo->origin.z
			<< "  Ang:" << BAM2DEG(players[0].mo->angles.yaw)
			<< " Look:" << BAM2DEG(players[0].mo->angles.pitch)
			<< endl;
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
