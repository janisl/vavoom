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

// Static subsystem variables.
TArray<FNameEntry*>	FName::Names;			 // Table of all names.
TArray<int>			FName::Available;       // Indices of available names.
FNameEntry*			FName::NameHash[4096];  // Hashed names.
bool				FName::Initialized;	 // Subsystem initialized.

#define REGISTER_NAME(name)		{ NAME_##name, OF_Native, NULL, #name },
#define REG_NAME_HIGH(name)		{ NAME_##name, OF_Native, NULL, #name },
FNameEntry AutoNames[] =
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
	return ((byte *)S)[0] | ((byte *)S)[1];
}

//==========================================================================
//
//
//
//==========================================================================

FArchive& operator << (FArchive& Ar, FNameEntry& E)
{
	byte Size;
	if (Ar.IsSaving())
	{
		Size = strlen(E.Name) + 1;
	}
	Ar << Size;
	Ar.Serialize(E.Name, Size);
	return Ar;
}

//==========================================================================
//
//	AllocateNameEntry
//
//==========================================================================

FNameEntry* AllocateNameEntry(const char* Name, dword Index, 
							  dword Flags, FNameEntry* HashNext)
{
	FNameEntry *E = Z_CNew<FNameEntry>();
	strcpy(E->Name, Name);
	E->Index = Index;
	E->Flags = Flags;
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
	if (!Name || !*Name)
	{
		Name = "None";
	}
	Index = 0;
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
	if (!TempHash)
	{
		if (FindType == FNAME_Add)
		{
			if (Available.Num())
			{
				Index = Available.Pop();
			}
			else
			{
				Index = Names.Add();
			}
			Names[Index] = AllocateNameEntry(Name, Index, 0, NameHash[HashIndex]);
			NameHash[HashIndex] = Names[Index];
		}
	}
}

//==========================================================================
//
//	FName::StaticInit
//
//==========================================================================

void FName::StaticInit()
{
	int i;

	// Register hardcoded names
	for (i = 0; i < ARRAY_COUNT(AutoNames); i++)
	{
		Hardcode(&AutoNames[i]);
	}
	// Find free indices
	for (i = 0; i < Names.Num(); i++)
	{
		if (!Names[i])
		{
			Available.AddItem(i);
		}
	}
	// We are now initialized
	Initialized = true;
}

//==========================================================================
//
//	FName::StaticExit
//
//==========================================================================

void FName::StaticExit()
{
	int i;

	//FIXME do we really need this?
	for (i = 0; i < Names.Num(); i++)
	{
		if (Names[i] && !(Names[i]->Flags & OF_Native))
		{
			Z_Free(Names[i]);
		}
	}
	Names.Empty();
	Available.Empty();
	Initialized = false;
}

//==========================================================================
//
//	FName::Hardcode
//
//==========================================================================

void FName::Hardcode(FNameEntry* AutoName)
{
	if (Names.Num() < AutoName->Index)
	{
		Names.AddZeroed(AutoName->Index - Names.Num());
	}
	Names.Insert(AutoName->Index);
	Names[AutoName->Index] = AutoName;
	int HashIndex = GetTypeHash(AutoName->Name) & 4095;
	AutoName->HashNext = NameHash[HashIndex];
	NameHash[HashIndex] = AutoName;
}

//==========================================================================
//
//	FName::DeleteEntry
//
//==========================================================================

void FName::DeleteEntry(int i)
{
	if (Names[i] && !(Names[i]->Flags & OF_Native))
	{
		Z_Free(Names[i]);
		Names[i] = NULL;
		Available.AddItem(i);
	}
}

//**************************************************************************
//
//	$Log$
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
