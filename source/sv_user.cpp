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
#include "network.h"
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
//	SV_ReadFromUserInfo
//
//==========================================================================

void SV_ReadFromUserInfo(VBasePlayer* Player)
{
	guard(SV_ReadFromUserInfo);
	if (!sv_loading)
	{
		Player->BaseClass = atoi(*Info_ValueForKey(Player->UserInfo, "class"));
	}
	Player->PlayerName = Info_ValueForKey(Player->UserInfo, "name");
	Player->Colour = atoi(*Info_ValueForKey(Player->UserInfo, "colour"));
	Player->eventUserinfoChanged();
	unguard;
}

//==========================================================================
//
//	SV_SetUserInfo
//
//==========================================================================

void SV_SetUserInfo(VBasePlayer* Player, const VStr& info)
{
	guard(SV_SetUserInfo);
	if (!sv_loading)
	{
		Player->UserInfo = info;
		SV_ReadFromUserInfo(Player);
	}
	unguard;
}

//==========================================================================
//
//	VServerNetContext::GetLevel
//
//==========================================================================

VLevel* VServerNetContext::GetLevel()
{
	return GLevel;
}

//==========================================================================
//
//	COMMAND SetInfo
//
//==========================================================================

COMMAND(SetInfo)
{
	guard(COMMAND SetInfo);
	if (Source != SRC_Client)
	{
		GCon->Log("SetInfo is not valid from console");
		return;
	}

	if (Args.Num() != 3)
	{
		return;
	}

	Info_SetValueForKey(Player->UserInfo, *Args[1], *Args[2]);
	SV_ReadFromUserInfo(Player);
	unguard;
}

//==========================================================================
//
//	Natives.
//
//==========================================================================

IMPLEMENT_FUNCTION(VBasePlayer, ServerSetUserInfo)
{
	P_GET_STR(Info);
	P_GET_SELF;
	SV_SetUserInfo(Self, Info);
}
