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

#ifdef IN_VCC
#include "../utils/vcc/vcc.h"
#else
#include "gamedefs.h"
#endif

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

TArray<VNameEntry*>	VName::Names;
VNameEntry*			VName::HashTable[VName::HASH_SIZE];
bool				VName::Initialised;

#define REGISTER_NAME(name)		{ NULL, NAME_##name, #name },
static VNameEntry AutoNames[] =
{
#include "../libs/core/names.h"
};

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	operator VStream << VNameEntry
//
//==========================================================================

VStream& operator<<(VStream& Strm, VNameEntry& E)
{
	guard(operator VStream << VNameEntry);
	vuint8 Size;
	if (Strm.IsSaving())
	{
		Size = VStr::Length(E.Name) + 1;
	}
	Strm << Size;
	Strm.Serialise(E.Name, Size);
	return Strm;
	unguard;
}

//==========================================================================
//
//	AllocateNameEntry
//
//==========================================================================

VNameEntry* AllocateNameEntry(const char* Name, vint32 Index,
	VNameEntry* HashNext)
{
	guard(AllocateNameEntry);
	int Size = sizeof(VNameEntry) - NAME_SIZE + VStr::Length(Name) + 1;
	VNameEntry* E = (VNameEntry*)Z_Malloc(Size);
	memset(E, 0, Size);
	VStr::Cpy(E->Name, Name);
	E->Index = Index;
	E->HashNext = HashNext;
	return E;
	unguard;
}

//==========================================================================
//
//	VName::VName
//
//==========================================================================

VName::VName(const char* Name, ENameFindType FindType)
{
	guard(VName::VName);
	char		NameBuf[NAME_SIZE];

	Index = NAME_None;
	//	Make sure name is valid.
	if (!Name || !*Name)
	{
		return;
	}

	//	Copy name localy, make sure it's not longer than 64 characters.
	if (FindType == AddLower8)
	{
		for (int i = 0; i < 8; i++)
		{
			NameBuf[i] = VStr::ToLower(Name[i]);
		}
		NameBuf[8] = 0;
	}
	else if (FindType == AddLower)
	{
		for (int i = 0; i < NAME_SIZE; i++)
		{
			NameBuf[i] = VStr::ToLower(Name[i]);
		}
		NameBuf[NAME_SIZE - 1] = 0;
	}
	else
	{
		VStr::NCpy(NameBuf, Name, NAME_SIZE);
		NameBuf[NAME_SIZE - 1] = 0;
	}

	//	Search in cache.
	int HashIndex = GetTypeHash(NameBuf) & (HASH_SIZE - 1);
	VNameEntry* TempHash = HashTable[HashIndex];
	while (TempHash)
	{
		if (!VStr::Cmp(NameBuf, TempHash->Name))
		{
			Index = TempHash->Index;
			break;
		}
		TempHash = TempHash->HashNext;
	}

	//	Add new name if not found.
	if (!TempHash && FindType != Find)
	{
		Index = Names.Num();
		Names.Append(AllocateNameEntry(NameBuf, Index, HashTable[HashIndex]));
		HashTable[HashIndex] = Names[Index];
	}

	//	Map 'none' to 'None'.
	if (Index == NAME_none)
	{
		Index = NAME_None;
	}
	unguard;
}

//==========================================================================
//
//	VName::StaticInit
//
//==========================================================================

void VName::StaticInit()
{
	guard(VName::StaticInit);
	//	Register hardcoded names.
	for (int i = 0; i < (int)ARRAY_COUNT(AutoNames); i++)
	{
		Names.Append(&AutoNames[i]);
		int HashIndex = GetTypeHash(AutoNames[i].Name) & (HASH_SIZE - 1);
		AutoNames[i].HashNext = HashTable[HashIndex];
		HashTable[HashIndex] = &AutoNames[i];
	}
	//	We are now initialised.
	Initialised = true;
	unguard;
}

//==========================================================================
//
//	VName::StaticExit
//
//==========================================================================

void VName::StaticExit()
{
	guard(VName::StaticExit);
	for (int i = NUM_HARDCODED_NAMES; i < Names.Num(); i++)
	{
		Z_Free(Names[i]);
	}
	Names.Clear();
	Initialised = false;
	unguard;
}
