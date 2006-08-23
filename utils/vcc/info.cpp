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

#include "vcc.h"

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
//	InitInfoTables
//
//==========================================================================

void InitInfoTables()
{
}

//==========================================================================
//
//	FindState
//
//==========================================================================

VState* CheckForState(VName StateName, VClass* InClass)
{
	VMemberBase* m = VMemberBase::StaticFindMember(StateName, InClass,
		MEMBER_State);
	if (m)
	{
		return (VState*)m;
	}
	if (InClass->ParentClass)
	{
		return CheckForState(StateName, InClass->ParentClass);
	}
	return NULL;
}

//==========================================================================
//
//	FindState
//
//==========================================================================

VState* FindState(VName StateName, VClass* InClass)
{
	VMemberBase* m = VMemberBase::StaticFindMember(StateName, InClass,
		MEMBER_State);
	if (m)
	{
		return (VState*)m;
	}
	if (InClass->ParentClass)
	{
		return FindState(StateName, InClass->ParentClass);
	}
	ParseError("No such state %s", *StateName);
	return NULL;
}

//==========================================================================
//
//	AddToMobjInfo
//
//==========================================================================

void AddToMobjInfo(int Index, VClass* Class)
{
	mobjinfo_t& mi = mobj_info.Alloc();
	mi.doomednum = Index;
	mi.class_id = Class;
}

//==========================================================================
//
//	AddToScriptIds
//
//==========================================================================

void AddToScriptIds(int Index, VClass* Class)
{
	mobjinfo_t& mi = script_ids.Alloc();
	mi.doomednum = Index;
	mi.class_id = Class;
}
