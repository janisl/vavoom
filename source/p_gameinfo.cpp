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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(V, GameInfo)

VGameInfo*		GGameInfo;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VGameInfo::VGameInfo
//
//==========================================================================

VGameInfo::VGameInfo()
: PlayerClasses(E_NoInit)
{
}

//==========================================================================
//
//	COMMAND ClearPlayerClasses
//
//==========================================================================

COMMAND(ClearPlayerClasses)
{
	guard(COMMAND ClearPlayerClasses);
	if (!ParsingKeyConf)
	{
		return;
	}

	GGameInfo->PlayerClasses.Clear();
	unguard;
}

//==========================================================================
//
//	COMMAND AddPlayerClass
//
//==========================================================================

COMMAND(AddPlayerClass)
{
	guard(COMMAND AddPlayerClass);
	if (!ParsingKeyConf)
	{
		return;
	}

	if (Args.Num() < 2)
	{
		GCon->Logf("Player class name missing");
		return;
	}

	VClass* Class = VClass::FindClassNoCase(*Args[1]);
	if (!Class)
	{
		GCon->Logf("No such class %s", *Args[1]);
		return;
	}

	VClass* PPClass = VClass::FindClass("PlayerPawn");
	if (!PPClass)
	{
		GCon->Logf("Can't find PlayerPawn class");
		return;
	}

	if (!Class->IsChildOf(PPClass))
	{
		GCon->Logf("%s is not a player pawn class", *Args[1]);
		return;
	}

	GGameInfo->PlayerClasses.Append(Class);
	unguard;
}
