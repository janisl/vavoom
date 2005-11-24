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

// HEADER FILES ------------------------------------------------------------

#include "vcc.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// Static subsystem variables.
TArray<FNameEntry*>	FName::Names;			// Table of all names.
FNameEntry*			FName::NameHash[4096];  // Hashed names.
bool				FName::Initialised;	 	// Subsystem initialised.

#define REGISTER_NAME(name)		{ NAME_##name, NULL, #name },
static FNameEntry AutoNames[] =
{
#include "names.h"
};

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	GetTypeHash
//
//==========================================================================

inline dword GetTypeHash(const char *S)
{
	dword ret = 0;
	for (int i = 0; S[i]; i++)
	{
		ret ^= (byte)S[i] << ((i & 3) * 8);
	}
	return ret;
}

//==========================================================================
//
//	AllocateNameEntry
//
//==========================================================================

FNameEntry* AllocateNameEntry(const char* Name, dword Index, 
							  FNameEntry* HashNext)
{
	FNameEntry *E = new FNameEntry;
	memset(E, 0, sizeof(*E));
	strcpy(E->Name, Name);
	E->Index = Index;
	E->HashNext = HashNext;
	return E;
}

//==========================================================================
//
//	FName::FName
//
//==========================================================================

FName::FName(const char* Name, EFindName FindType)
{
	Index = NAME_None;
	if (!Name || !*Name)
	{
		return;
	}
	int HashIndex = GetTypeHash(Name) & 4095;
	FNameEntry *TempHash = NameHash[HashIndex];
	while (TempHash)
	{
		if (!strcmp(Name, TempHash->Name))
		{
			Index = TempHash->Index;
			break;
		}
		TempHash = TempHash->HashNext;
	}
	if (!TempHash && FindType == FNAME_Add)
	{
		Index = Names.Add();
		Names[Index] = AllocateNameEntry(Name, Index, NameHash[HashIndex]);
		NameHash[HashIndex] = Names[Index];
	}
}

//==========================================================================
//
//	FName::StaticInit
//
//==========================================================================

void FName::StaticInit()
{
	// Register hardcoded names
	for (int i = 0; i < ARRAY_COUNT(AutoNames); i++)
	{
		Names.Insert(AutoNames[i].Index);
		Names[AutoNames[i].Index] = &AutoNames[i];
		int HashIndex = GetTypeHash(AutoNames[i].Name) & 4095;
		AutoNames[i].HashNext = NameHash[HashIndex];
		NameHash[HashIndex] = &AutoNames[i];
	}
	// We are now initialised
	Initialised = true;
}

//==========================================================================
//
//	FName::StaticExit
//
//==========================================================================

void FName::StaticExit()
{
	//FIXME do we really need this?
	for (int i = NUM_HARDCODED_NAMES; i < Names.Num(); i++)
	{
		delete Names[i];
	}
	Names.Empty();
	Initialised = false;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2005/11/24 20:41:07  dj_jl
//	Cleaned up a bit.
//
//	Revision 1.2  2002/01/25 18:05:58  dj_jl
//	Better string hash function
//	
//	Revision 1.1  2002/01/11 08:17:31  dj_jl
//	Added name subsystem, removed support for unsigned ints
//	
//**************************************************************************
