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

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VLockDef*		LockDefs[256];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	ParseLockDefs
//
//==========================================================================

static void ParseLockDefs(VScriptParser* sc)
{
	guard(ParseLockDefs);
	while (!sc->AtEnd())
	{
		if (sc->Check("ClearLocks"))
		{
			for (int i = 0; i < 256; i++)
			{
				if (LockDefs[i])
				{
					delete LockDefs[i];
					LockDefs[i] = NULL;
				}
			}
		}
		else if (sc->Check("Lock"))
		{
			//	Lock number
			sc->ExpectNumber();
			int Lock = sc->Number;
			if (Lock <= 0 || Lock >= 255)
			{
				sc->Error("Bad lock number");
			}
			if (LockDefs[Lock])
			{
				delete LockDefs[Lock];
			}
			VLockDef* LDef = new VLockDef;
			LockDefs[Lock] = LDef;
			LDef->MapColour = 0;
			LDef->LockedSound = "misc/keytry";

			//	Skip game specifier
			sc->Check("Doom") || sc->Check("Heretic") ||
				sc->Check("Hexen") || sc->Check("Strife");

			sc->Expect("{");
			while (!sc->Check("}"))
			{
				if (sc->Check("Message"))
				{
					sc->ExpectString();
					LDef->Message = sc->String;
				}
				else if (sc->Check("RemoteMessage"))
				{
					sc->ExpectString();
					LDef->RemoteMessage = sc->String;
				}
				else if (sc->Check("MapColor"))
				{
					sc->ExpectNumber();
					int r = sc->Number;
					sc->ExpectNumber();
					int g = sc->Number;
					sc->ExpectNumber();
					int b = sc->Number;
					LDef->MapColour = 0xff000000 | (r << 16) | (g << 8) | b;
				}
				else if (sc->Check("LockedSound"))
				{
					sc->ExpectString();
					LDef->LockedSound = *sc->String;
				}
				else if (sc->Check("Any"))
				{
					sc->Expect("{");
					VLockGroup& Grp = LDef->Locks.Alloc();
					while (!sc->Check("}"))
					{
						sc->ExpectString();
						VClass* Cls = VClass::FindClass(*sc->String);
						if (!Cls)
						{
							GCon->Logf("No such class %s", *sc->String);
						}
						else
						{
							Grp.AnyKeyList.Append(Cls);
						}
					}
				}
				else
				{
					sc->ExpectString();
					VClass* Cls = VClass::FindClass(*sc->String);
					if (!Cls)
					{
						GCon->Logf("No such class %s", *sc->String);
					}
					else
					{
						LDef->Locks.Alloc().AnyKeyList.Append(Cls);
					}
				}
			}
			//	Copy message if other one is not defined
			if (LDef->Message.IsEmpty())
			{
				LDef->Message = LDef->RemoteMessage;
			}
			if (LDef->RemoteMessage.IsEmpty())
			{
				LDef->RemoteMessage = LDef->Message;
			}
		}
		else
		{
			sc->Error("Bad syntax");
		}
	}
	delete sc;
	unguard;
}

//==========================================================================
//
//	InitLockDefs
//
//==========================================================================

void InitLockDefs()
{
	guard(InitLockDefs);
	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) == NAME_lockdefs)
		{
			ParseLockDefs(new VScriptParser(*W_LumpName(Lump),
				W_CreateLumpReaderNum(Lump)));
		}
	}
	unguard;
}

//==========================================================================
//
//	ShutdownLockDefs
//
//==========================================================================

void ShutdownLockDefs()
{
	guard(ShutdownLockDefs);
	for (int i = 0; i < 256; i++)
	{
		if (LockDefs[i])
		{
			delete LockDefs[i];
			LockDefs[i] = NULL;
		}
	}
	unguard;
}

//==========================================================================
//
//	GetLockDef
//
//==========================================================================

VLockDef* GetLockDef(int Lock)
{
	guard(GetLockDef);
	return Lock < 0 || Lock > 255 ? NULL : LockDefs[Lock];
	unguard;
}
