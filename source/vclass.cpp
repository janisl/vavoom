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

#include "gamedefs.h"

IMPLEMENT_CLASS(VClass)

//==========================================================================
//
//	VClass::VClass
//
//==========================================================================

VClass::VClass(void)
{
}

//==========================================================================
//
//	VClass::VClass
//
//==========================================================================

VClass::VClass(ENativeConstructor, size_t ASize, dword AClassFlags, 
	VClass *AParent, const char *AName, int AFlags, void(*ACtor)(void*))
	: VObject(EC_NativeConstructor, StaticClass(), AName, AFlags), 
	ParentClass(AParent), ClassSize(ASize),
	ClassFlags(AClassFlags), ClassConstructor(ACtor)
{
}

//==========================================================================
//
//	VClass::FindClass
//
//==========================================================================

VClass *VClass::FindClass(const char *AName)
{
	FName TempName(AName, FNAME_Find);
	if (TempName == NAME_None)
	{
		// No such name, no chance to find a class
		return NULL;
	}
	for (TObjectIterator<VClass> It; It; ++It)
	{
		if (It->GetFName() == TempName)
		{
			return *It;
		}
	}
	return NULL;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//
//	Revision 1.1  2001/12/27 17:35:41  dj_jl
//	Split VClass in seperate module
//	
//	Revision 1.1  2001/12/18 19:03:17  dj_jl
//	A lots of work on VObject
//	
//**************************************************************************
