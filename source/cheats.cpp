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

static bool CheatAllowed(VBasePlayer* Player)
{
	if (sv.intermission)
	{
		Player->Printf("You are not in game!");
		return false;
	}
	if (GGameInfo->NetMode >= NM_DedicatedServer)
	{
		Player->Printf("You cannot cheat in a network game!");
		return false;
	}
	if (GGameInfo->WorldInfo->Flags & VWorldInfo::WIF_SkillDisableCheats)
	{
		Player->Printf("You are too good to cheat!");
		return false;
	}
	if (Player->Health <= 0)
	{
		// Dead players can't cheat
		Player->Printf("You must be alive to cheat");
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
	if (Source == SRC_Command)
	{
		ForwardToServer();
		return;
	}

	if (CheatAllowed(Player))
	{
		Player->eventCheat_God();
	}
}

//==========================================================================
//
//  NoClip_f
//
//==========================================================================

COMMAND(NoClip)
{
	if (Source == SRC_Command)
	{
		ForwardToServer();
		return;
	}

	if (CheatAllowed(Player))
	{
		Player->eventCheat_NoClip();
	}
}

//==========================================================================
//
//  Gimme_f
//
//==========================================================================

COMMAND(Gimme)
{
	if (Source == SRC_Command)
	{
		ForwardToServer();
		return;
	}

	if (CheatAllowed(Player))
	{
		Player->eventCheat_Gimme();
	}
}

//==========================================================================
//
//  KillAll_f
//
//==========================================================================

COMMAND(KillAll)
{
	if (Source == SRC_Command)
	{
		ForwardToServer();
		return;
	}

	if (CheatAllowed(Player))
	{
		Player->eventCheat_KillAll();
	}
}

//==========================================================================
//
//  Morph_f
//
//==========================================================================

COMMAND(Morph)
{
	if (Source == SRC_Command)
	{
		ForwardToServer();
		return;
	}

	if (CheatAllowed(Player))
	{
		Player->eventCheat_Morph();
	}
}

//==========================================================================
//
//  NoWeapons_f
//
//==========================================================================

COMMAND(NoWeapons)
{
	if (Source == SRC_Command)
	{
		ForwardToServer();
		return;
	}

	if (CheatAllowed(Player))
	{
		Player->eventCheat_NoWeapons();
	}
}

//==========================================================================
//
//  Class_f
//
//==========================================================================

COMMAND(ChangeClass)
{
	if (Source == SRC_Command)
	{
		ForwardToServer();
		return;
	}

	if (CheatAllowed(Player))
	{
		Player->eventCheat_Class();
	}
}

//==========================================================================
//
//	Script_f
//
//==========================================================================

COMMAND(Script)
{
	if (Source == SRC_Command)
	{
		ForwardToServer();
		return;
	}

	if (CheatAllowed(Player))
	{
		int 	script;

		if (Args.Num() != 2)
			return;
		script = atoi(*Args[1]);
		if (script < 1)
			return;
		if (script > 9999)
			return;

		if (Player->Level->XLevel->Acs->Start(script, 0, 0, 0, 0, Player->MO,
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
	if (Source == SRC_Command)
	{
		ForwardToServer();
		return;
	}

	if (CheatAllowed(Player))
	{
		Player->Printf("MAP %s  X:%f  Y:%f  Z:%f  Yaw:%f Pitch:%f",
			*GLevel->MapName, Player->MO->Origin.x,
			Player->MO->Origin.y, Player->MO->Origin.z,
			Player->MO->Angles.yaw, Player->MO->Angles.pitch);
	}
}

//==========================================================================
//
//  Fly_f
//
//==========================================================================

COMMAND(Fly)
{
	if (Source == SRC_Command)
	{
		ForwardToServer();
		return;
	}

	if (CheatAllowed(Player))
	{
		Player->eventCheat_Fly();
	}
}

//==========================================================================
//
//  NoTarget_f
//
//==========================================================================

COMMAND(NoTarget)
{
	if (Source == SRC_Command)
	{
		ForwardToServer();
		return;
	}

	if (CheatAllowed(Player))
	{
		Player->eventCheat_NoTarget();
	}
}

//==========================================================================
//
//  Anubis_f
//
//==========================================================================

COMMAND(Anubis)
{
	if (Source == SRC_Command)
	{
		ForwardToServer();
		return;
	}

	if (CheatAllowed(Player))
	{
		Player->eventCheat_Anubis();
	}
}

//==========================================================================
//
//  Freeze_f
//
//==========================================================================

COMMAND(Freeze)
{
	if (Source == SRC_Command)
	{
		ForwardToServer();
		return;
	}

	if (CheatAllowed(Player))
	{
		Player->eventCheat_Freeze();
	}
}
