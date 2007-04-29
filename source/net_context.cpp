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
//	VNetContext::VNetContext
//
//==========================================================================

VNetContext::VNetContext()
: RoleField(NULL)
, RemoteRoleField(NULL)
, ServerConnection(NULL)
{
	RoleField = VThinker::StaticClass()->FindFieldChecked("Role");
	RemoteRoleField = VThinker::StaticClass()->FindFieldChecked("RemoteRole");
}

//==========================================================================
//
//	VNetContext::~VNetContext
//
//==========================================================================

VNetContext::~VNetContext()
{
}

//==========================================================================
//
//	VNetContext::ThinkerDestroyed
//
//==========================================================================

void VNetContext::ThinkerDestroyed(VThinker* Th)
{
	guard(VNetContext::ThinkerDestroyed);
	if (ServerConnection)
	{
		VThinkerChannel* Chan = ServerConnection->ThinkerChannels.FindPtr(Th);
		if (Chan)
		{
			Chan->Close();
		}
	}
	else
	{
		for (int i = 0; i < ClientConnections.Num(); i++)
		{
			VThinkerChannel* Chan = ClientConnections[i]->ThinkerChannels.FindPtr(Th);
			if (Chan)
			{
				Chan->Close();
			}
		}
	}
	unguard;
}
