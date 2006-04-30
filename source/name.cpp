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

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

TArray<VNameEntry*>	VName::Names;
VNameEntry*			VName::HashTable[4096];
bool				VName::Initialised;

#define REGISTER_NAME(name)		{ NULL, NAME_##name, #name },
static VNameEntry AutoNames[] =
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

inline vuint32 GetTypeHash(const char *S)
{
	return ((vuint8*)S)[0] | ((vuint8*)S)[1];
}

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
		Size = strlen(E.Name) + 1;
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
	int Size = sizeof(VNameEntry) - NAME_SIZE + strlen(Name) + 1;
	VNameEntry* E = (VNameEntry*)Z_Malloc(Size, PU_STATIC, 0);
	memset(E, 0, Size);
	strcpy(E->Name, Name);
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
			NameBuf[i] = tolower(Name[i]);
		}
		NameBuf[8] = 0;
	}
	else
	{
		strncpy(NameBuf, Name, NAME_SIZE);
		NameBuf[NAME_SIZE - 1] = 0;
	}

	//	Search in cache.
	int HashIndex = GetTypeHash(NameBuf) & 4095;
	VNameEntry* TempHash = HashTable[HashIndex];
	while (TempHash)
	{
		if (!strcmp(NameBuf, TempHash->Name))
		{
			Index = TempHash->Index;
			break;
		}
		TempHash = TempHash->HashNext;
	}

	//	Add new name if not found.
	if (!TempHash && (FindType == Add || FindType == AddLower8))
	{
		Index = Names.Add();
		Names[Index] = AllocateNameEntry(NameBuf, Index, HashTable[HashIndex]);
		HashTable[HashIndex] = Names[Index];
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
		Names.AddItem(&AutoNames[i]);
		int HashIndex = GetTypeHash(AutoNames[i].Name) & 4095;
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
	Names.Empty();
	Initialised = false;
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.10  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//
//	Revision 1.9  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//	
//	Revision 1.8  2005/11/22 19:10:36  dj_jl
//	Cleaned up a bit.
//	
//	Revision 1.7  2005/05/26 16:49:14  dj_jl
//	Added lowercased max 8 chars names.
//	
//	Revision 1.6  2005/04/28 07:16:15  dj_jl
//	Fixed some warnings, other minor fixes.
//	
//	Revision 1.5  2004/12/03 16:15:47  dj_jl
//	Implemented support for extended ACS format scripts, functions, libraries and more.
//	
//	Revision 1.4  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.3  2002/01/11 08:10:12  dj_jl
//	Map empty strings to NAME_None
//	
//	Revision 1.2  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2001/12/18 18:57:11  dj_jl
//	Added global name subsystem
//	
//**************************************************************************
