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
//**
//**	Dehacked patch parsing
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

static char*					Patch;
static char*					PatchPtr;
static char*					String;
static int						value;

static TArray<VName>			Sprites;
static TArray<VClass*>			EntClasses;
static TArray<VClass*>			WeaponClasses;
static TArray<VState*>			States;
static TArray<VState*>			CodePtrStates;
static TArray<VMethod*>			StateActions;
static TArray<VName>			Sounds;

static VClass*					GameInfoClass;

static TArray<FReplacedString>	SfxNames;
static TArray<FReplacedString>	MusicNames;
static TArray<FReplacedString>	SpriteNames;
static VLanguage*				EngStrings;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	GetLine
//
//==========================================================================

static bool GetLine()
{
	guard(GetLine);
	do
	{
		if (!*PatchPtr)
		{
			return false;
		}

		String = PatchPtr;

		while (*PatchPtr && *PatchPtr != '\n')
		{
			PatchPtr++;
		}
		if (*PatchPtr == '\n')
		{
			*PatchPtr = 0;
			PatchPtr++;
		}

		if (*String == '#')
		{
			*String = 0;
			continue;
		}

		while (*String && *String <= ' ')
		{
			String++;
		}
	} while (!*String);

	return true;
	unguard;
}

//==========================================================================
//
//	ParseParam
//
//==========================================================================

static bool ParseParam()
{
	guard(ParseParam);
	char	*val;

	if (!GetLine())
	{
		return false;
	}

	val = strchr(String, '=');

	if (!val)
	{
		return false;
	}

	value = atoi(val + 1);

	do
	{
		*val = 0;
		val--;
	}
	while (val >= String && *val <= ' ');

	return true;
	unguard;
}

//==========================================================================
//
//	GetClassFieldState
//
//==========================================================================

static VState* GetClassFieldState(VClass* Class, const char* FieldName)
{
	guard(GetClassFieldInt);
	VField* F = Class->FindFieldChecked(FieldName);
	VState** Ptr = (VState**)(Class->Defaults + F->Ofs);
	return *Ptr;
	unguard;
}

//==========================================================================
//
//	SetClassFieldInt
//
//==========================================================================

static void SetClassFieldInt(VClass* Class, const char* FieldName,
	int Value, int Idx = 0)
{
	guard(SetClassFieldInt);
	VField* F = Class->FindFieldChecked(FieldName);
	vint32* Ptr = (vint32*)(Class->Defaults + F->Ofs);
	Ptr[Idx] = Value;
	unguard;
}

//==========================================================================
//
//	SetClassFieldFloat
//
//==========================================================================

static void SetClassFieldFloat(VClass* Class, const char* FieldName,
	float Value)
{
	guard(SetClassFieldFloat);
	VField* F = Class->FindFieldChecked(FieldName);
	float* Ptr = (float*)(Class->Defaults + F->Ofs);
	*Ptr = Value;
	unguard;
}

//==========================================================================
//
//	SetClassFieldName
//
//==========================================================================

static void SetClassFieldName(VClass* Class, const char* FieldName,
	VName Value)
{
	guard(SetClassFieldInt);
	VField* F = Class->FindFieldChecked(FieldName);
	VName* Ptr = (VName*)(Class->Defaults + F->Ofs);
	*Ptr = Value;
	unguard;
}

//==========================================================================
//
//	SetClassFieldState
//
//==========================================================================

static void SetClassFieldState(VClass* Class, const char* FieldName,
	VState* Value)
{
	guard(SetClassFieldInt);
	VField* F = Class->FindFieldChecked(FieldName);
	VState** Ptr = (VState**)(Class->Defaults + F->Ofs);
	*Ptr = Value;
	unguard;
}

//==========================================================================
//
//	SetClassFieldBool
//
//==========================================================================

static void SetClassFieldBool(VClass* Class, const char* FieldName, int Value)
{
	guard(SetClassFieldBool);
	VField* F = Class->FindFieldChecked(FieldName);
	vuint32* Ptr = (vuint32*)(Class->Defaults + F->Ofs);
	if (Value)
		*Ptr |= F->Type.BitMask;
	else
		*Ptr &= ~F->Type.BitMask;
	unguard;
}

//==========================================================================
//
//	ReadThing
//
//==========================================================================

static void ReadThing(int num)
{
	guard(ReadThing);
	num--; // begin at 0 not 1;
	if (num < 0 || num >= EntClasses.Num())
	{
		dprintf("WARNING! Invalid thing num %d\n", num);
		while (ParseParam());
		return;
	}

	VClass* Ent = EntClasses[num];
	while (ParseParam())
	{
		if (!strcmp(String ,"ID #"))
		{
			int Idx = -1;
			for (int i = 0; i < VClass::GMobjInfos.Num(); i++)
			{
				if (VClass::GMobjInfos[i].class_id == Ent)
				{
					Idx = i;
					break;
				}
			}
			if (value)
			{
				if (Idx < 0)
				{
					Idx = VClass::GMobjInfos.Num();
					VClass::GMobjInfos.Alloc().class_id = Ent;
				}
				VClass::GMobjInfos[Idx].doomednum = value;
			}
			else if (Idx)
			{
				VClass::GMobjInfos.RemoveIndex(Idx);
			}
		}
		else if (!strcmp(String, "Hit points"))
		{
			SetClassFieldInt(Ent, "Health", value);
			SetClassFieldInt(Ent, "GibsHealth", -value);
		}
		else if (!strcmp(String, "Reaction time"))
		{
			SetClassFieldInt(Ent, "ReactionCount", value);
		}
		else if (!strcmp(String, "Missile damage"))
		{
			SetClassFieldInt(Ent, "MissileDamage", value);
		}
		else if (!strcmp(String, "Width"))
		{
			SetClassFieldFloat(Ent, "Radius", value / 65536.0);
		}
		else if (!strcmp(String, "Height"))
		{
			SetClassFieldFloat(Ent, "Height", value / 65536.0);
		}
		else if (!strcmp(String, "Mass"))
		{
			SetClassFieldFloat(Ent, "Mass", value == 0x7fffffff ? 99999.0 : value);
		}
		else if (!strcmp(String, "Speed"))
		{
			if (value < 100)
			{
				SetClassFieldFloat(Ent, "Speed", 0.0);
				SetClassFieldFloat(Ent, "StepSpeed", value);
			}
			else
			{
				SetClassFieldFloat(Ent, "Speed", 35.0 * value / 65536.0);
				SetClassFieldFloat(Ent, "StepSpeed", 0.0);
			}
		}
		else if (!strcmp(String, "Pain chance"))
		{
			SetClassFieldFloat(Ent, "PainChance", value / 256.0);
		}
		else if (!strcmp(String, "Bits"))
		{
			SetClassFieldBool(Ent, "bSpecial", value & 0x00000001);
			SetClassFieldBool(Ent, "bSolid", value & 0x00000002);
			SetClassFieldBool(Ent, "bShootable", value & 0x00000004);
			SetClassFieldBool(Ent, "bHidden", value & 0x00000008);
			SetClassFieldBool(Ent, "bNoBlockmap", value & 0x00000010);
			SetClassFieldBool(Ent, "bAmbush", value & 0x00000020);
			SetClassFieldBool(Ent, "bJustHit", value & 0x00000040);
			SetClassFieldBool(Ent, "bJustAttacked", value & 0x00000080);
			SetClassFieldBool(Ent, "bSpawnCeiling", value & 0x00000100);
			SetClassFieldBool(Ent, "bNoGravity", value & 0x00000200);
			SetClassFieldBool(Ent, "bDropOff", value & 0x00000400);
			SetClassFieldBool(Ent, "bPickUp", value & 0x00000800);
			SetClassFieldBool(Ent, "bColideWithThings", !(value & 0x00001000));
			SetClassFieldBool(Ent, "bColideWithWorld", !(value & 0x00001000));
			SetClassFieldBool(Ent, "bSlide", value & 0x00002000);
			SetClassFieldBool(Ent, "bFloat", value & 0x00004000);
			SetClassFieldBool(Ent, "bTeleport", value & 0x00008000);
			SetClassFieldBool(Ent, "bMissile", value & 0x00010000);
			SetClassFieldBool(Ent, "bActivatePCross", value & 0x00010000);
			SetClassFieldBool(Ent, "bNoTeleport", value & 0x00010000);
			SetClassFieldBool(Ent, "bDropped", value & 0x00020000);
			SetClassFieldBool(Ent, "bNoBlood", value & 0x00080000);
			SetClassFieldBool(Ent, "bCorpse", value & 0x00100000);
			SetClassFieldBool(Ent, "bInFloat", value & 0x00200000);
			SetClassFieldBool(Ent, "bCountKill", value & 0x00400000);
			SetClassFieldBool(Ent, "bMonster", value & 0x00400000);
			SetClassFieldBool(Ent, "bActivateMCross", value & 0x00400000);
			SetClassFieldBool(Ent, "bActivatePushWall", (value & 0x00400000) || num == 0);
			SetClassFieldBool(Ent, "bCountItem", value & 0x00800000);
			SetClassFieldBool(Ent, "bSkullFly", value & 0x01000000);
			SetClassFieldBool(Ent, "bNoDeathmatch", value & 0x02000000);
			SetClassFieldBool(Ent, "bNoPassMobj", num != 0 && !(value & 0x00400000) && !(value & 0x00010000));
			//	Translation
			SetClassFieldInt(Ent, "Translation", (value & 0x0c000000) >> 26);
			//	Alpha
			SetClassFieldFloat(Ent, "Alpha", (value & 0x00040000) ? 0.1 : 1.0);
			//	The following fields were not used
			//value & 0x10000000);
			//value & 0x20000000);
			//value & 0x40000000);
			//value & 0x80000000);
		}
		//
		//	States
		//
		else if (!strcmp(String, "Initial frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Bad thing state %d\n", value);
			}
			else
			{
				SetClassFieldState(Ent, "IdleState", States[value]);
			}
		}
		else if (!strcmp(String, "First moving frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Bad thing state %d\n", value);
			}
			else
			{
				SetClassFieldState(Ent, "SeeState", States[value]);
			}
		}
		else if (!strcmp(String, "Close attack frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Bad thing state %d\n", value);
			}
			else
			{
				SetClassFieldState(Ent, "MeleeState", States[value]);
			}
		}
		else if (!strcmp(String, "Far attack frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Bad thing state %d\n", value);
			}
			else
			{
				SetClassFieldState(Ent, "MissileState", States[value]);
			}
		}
		else if (!strcmp(String, "Injury frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Bad thing state %d\n", value);
			}
			else
			{
				SetClassFieldState(Ent, "PainState", States[value]);
			}
		}
		else if (!strcmp(String, "Death frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Bad thing state %d\n", value);
			}
			else
			{
				SetClassFieldState(Ent, "DeathState", States[value]);
			}
		}
		else if (!strcmp(String, "Exploding frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Bad thing state %d\n", value);
			}
			else
			{
				SetClassFieldState(Ent, "GibsDeathState", States[value]);
			}
		}
		else if (!strcmp(String, "Respawn frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Bad thing state %d\n", value);
			}
			else
			{
				SetClassFieldState(Ent, "RaiseState", States[value]);
			}
		}
		//
		//	Sounds
		//
		else if (!strcmp(String, "Alert sound"))
		{
			if (value < 0 || value >= Sounds.Num())
			{
				dprintf("WARNING! Bad sound index %d\n", value);
			}
			else
			{
				SetClassFieldName(Ent, "SightSound", Sounds[value]);
			}
		}
		else if (!strcmp(String, "Action sound"))
		{
			if (value < 0 || value >= Sounds.Num())
			{
				dprintf("WARNING! Bad sound index %d\n", value);
			}
			else
			{
				SetClassFieldName(Ent, "ActiveSound", Sounds[value]);
			}
		}
		else if (!strcmp(String, "Attack sound"))
		{
			if (value < 0 || value >= Sounds.Num())
			{
				dprintf("WARNING! Bad sound index %d\n", value);
			}
			else
			{
				SetClassFieldName(Ent, "AttackSound", Sounds[value]);
			}
		}
		else if (!strcmp(String, "Pain sound"))
		{
			if (value < 0 || value >= Sounds.Num())
			{
				dprintf("WARNING! Bad sound index %d\n", value);
			}
			else
			{
				SetClassFieldName(Ent, "PainSound", Sounds[value]);
			}
		}
		else if (!strcmp(String, "Death sound"))
		{
			if (value < 0 || value >= Sounds.Num())
			{
				dprintf("WARNING! Bad sound index %d\n", value);
			}
			else
			{
				SetClassFieldName(Ent, "DeathSound", Sounds[value]);
			}
		}
		else
		{
			dprintf("WARNING! Invalid mobj param %s\n", String);
		}
	}
	unguard;
}

//==========================================================================
//
//	ReadSound
//
//==========================================================================

static void ReadSound(int)
{
	guard(ReadSound);
	while (ParseParam())
	{
		if (!strcmp(String, "Offset"));				//Lump name offset - can't handle
		else if (!strcmp(String, "Zero/One"));		//Singularity - removed
		else if (!strcmp(String, "Value"));			//Priority
		else if (!strcmp(String, "Zero 1"));		//Lump num - can't be set
		else if (!strcmp(String, "Zero 2"));		//Data pointer - can't be set
		else if (!strcmp(String, "Zero 3"));		//Usefulness - removed
		else if (!strcmp(String, "Zero 4"));		//Link - removed
		else if (!strcmp(String, "Neg. One 1"));	//Link pitch - removed
		else if (!strcmp(String, "Neg. One 2"));	//Link volume - removed
		else dprintf("WARNING! Invalid sound param %s\n", String);
	}
	unguard;
}

//==========================================================================
//
//	ReadState
//
//==========================================================================

static void ReadState(int num)
{
	guard(ReadState);
	//	Check index.
	if (num >= States.Num() || num < 0)
	{
		dprintf("WARNING! Invalid state num %d\n", num);
		while (ParseParam());
		return;
	}

	//	State 0 is a special state.
	if (num == 0)
	{
		while (ParseParam());
		return;
	}

	while (ParseParam())
	{
		if (!strcmp(String, "Sprite number"))
		{
			if (value < 0 || value >= Sprites.Num())
			{
				dprintf("WARNING! Bad sprite index %d\n", value);
			}
			else
			{
				States[num]->SpriteName = Sprites[value];
				States[num]->SpriteIndex = VClass::FindSprite(Sprites[value]);
			}
		}
		else if (!strcmp(String, "Sprite subnumber"))
		{
			if (value & 0x8000)
			{
				value &= 0x7fff;
				value |= FF_FULLBRIGHT;
			}
			States[num]->Frame = value;
		}
		else if (!strcmp(String, "Duration"))
		{
			States[num]->Time = value < 0 ? value : value / 35.0;
		}
		else if (!strcmp(String, "Next frame"))
		{
			if (value >= States.Num() || value < 0)
			{
				dprintf("WARNING! Invalid next state %d\n", value);
			}
			else
			{
				States[num]->NextState = States[value];
			}
		}
		else if (!strcmp(String, "Unknown 1"))
		{
			States[num]->Misc1 = value;
		}
		else if (!strcmp(String, "Unknown 2"))
		{
			States[num]->Misc2 = value;
		}
		else if (!strcmp(String, "Action pointer"))
		{
			dprintf("WARNING! Tried to set action pointer.\n");
		}
		else
		{
			dprintf("WARNING! Invalid state param %s\n", String);
		}
	}
	unguard;
}

//==========================================================================
//
//	ReadSpriteName
//
//==========================================================================

static void ReadSpriteName(int)
{
	guard(ReadSpriteName);
	while (ParseParam())
	{
		if (!VStr::ICmp(String, "Offset"));	//	Can't handle
		else dprintf("WARNING! Invalid sprite name param %s\n", String);
	}
	unguard;
}

//==========================================================================
//
//	ReadAmmo
//
//==========================================================================

static void ReadAmmo(int num)
{
	guard(ReadAmmo);
	//	Check index.
	if (num >= 4 || num < 0)
	{
		dprintf("WARNING! Invalid ammo num %d\n", num);
		while (ParseParam());
		return;
	}

	while (ParseParam())
	{
		if (!VStr::ICmp(String, "Max ammo"))
		{
			SetClassFieldInt(GameInfoClass, "maxammo", value, num);
		}
		else if (!VStr::ICmp(String, "Per ammo"))
		{
			SetClassFieldInt(GameInfoClass, "clipammo", value, num);
		}
		else
		{
			dprintf("WARNING! Invalid ammo param %s\n", String);
		}
	}
	unguard;
}

//==========================================================================
//
//	ReadWeapon
//
//==========================================================================

static void ReadWeapon(int num)
{
	guard(ReadWeapon);
	//	Check index.
	if (num < 0 || num >= WeaponClasses.Num())
	{
		dprintf("WARNING! Invalid weapon num %d\n", num);
		while (ParseParam());
		return;
	}

	VClass* Weapon = WeaponClasses[num];
	while (ParseParam())
	{
		if (!VStr::ICmp(String, "Ammo type"))
		{
			SetClassFieldInt(Weapon, "Ammo", value);
		}
		else if (!VStr::ICmp(String, "Deselect frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Invalid weapon state %d\n", value);
			}
			else
			{
				SetClassFieldState(Weapon, "UpState", States[value]);
			}
		}
		else if (!VStr::ICmp(String, "Select frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Invalid weapon state %d\n", value);
			}
			else
			{
				SetClassFieldState(Weapon, "DownState", States[value]);
			}
		}
		else if (!VStr::ICmp(String, "Bobbing frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Invalid weapon state %d\n", value);
			}
			else
			{
				SetClassFieldState(Weapon, "ReadyState", States[value]);
			}
		}
		else if (!VStr::ICmp(String, "Shooting frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Invalid weapon state %d\n", value);
			}
			else
			{
				SetClassFieldState(Weapon, "AttackState", States[value]);
			}
		}
		else if (!VStr::ICmp(String, "Firing frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Invalid weapon state %d\n", value);
			}
			else
			{
				SetClassFieldState(Weapon, "FlashState", States[value]);
			}
		}
		else
		{
			dprintf("WARNING! Invalid weapon param %s\n", String);
		}
	}
	unguard;
}

//==========================================================================
//
//	ReadPointer
//
//==========================================================================

static void ReadPointer(int num)
{
	guard(ReadPointer);
	if (num < 0 || num >= CodePtrStates.Num())
	{
		dprintf("WARNING! Invalid pointer\n");
		while (ParseParam());
		return;
	}

	while (ParseParam())
	{
		if (!VStr::ICmp(String, "Codep Frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				dprintf("WARNING! Invalid source state %d\n", value);
			}
			else
			{
				CodePtrStates[num]->Function = StateActions[value];
			}
		}
		else
		{
			dprintf("WARNING! Invalid pointer param %s\n", String);
		}
	}
	unguard;
}

//==========================================================================
//
//	ReadCheats
//
//==========================================================================

static void ReadCheats(int)
{
	guard(ReadCheats);
	//	Old cheat handling is removed
	while (ParseParam());
	unguard;
}

//==========================================================================
//
//	ReadMisc
//
//==========================================================================

static void ReadMisc(int)
{
	guard(ReadMisc);
	while (ParseParam())
	{
		if (!VStr::ICmp(String, "Initial Health"))
		{
			SetClassFieldInt(GameInfoClass, "INITIAL_HEALTH", value);
		}
		else if (!VStr::ICmp(String, "Initial Bullets"))
		{
			SetClassFieldInt(GameInfoClass, "INITIAL_AMMO", value);
		}
		else if (!VStr::ICmp(String, "Max Health"));
		else if (!VStr::ICmp(String, "Max Armor"));
		else if (!VStr::ICmp(String, "Green Armor Class"));
		else if (!VStr::ICmp(String, "Blue Armor Class"));
		else if (!VStr::ICmp(String, "Max Soulsphere"))
		{
			SetClassFieldInt(GameInfoClass, "SOULSPHERE_MAX", value);
		}
		else if (!VStr::ICmp(String, "Soulsphere Health"))
		{
			SetClassFieldInt(GameInfoClass, "SOULSPHERE_HEALTH", value);
		}
		else if (!VStr::ICmp(String, "Megasphere Health"))
		{
			SetClassFieldInt(GameInfoClass, "MEGASPHERE_HEALTH", value);
		}
		else if (!VStr::ICmp(String, "God Mode Health"))
		{
			SetClassFieldInt(GameInfoClass, "GOD_HEALTH", value);
		}
		else if (!VStr::ICmp(String, "IDFA Armor"));		//	Cheat removed
		else if (!VStr::ICmp(String, "IDFA Armor Class"));	//	Cheat removed
		else if (!VStr::ICmp(String, "IDKFA Armor"));		//	Cheat removed
		else if (!VStr::ICmp(String, "IDKFA Armor Class"));//	Cheat removed
		else if (!VStr::ICmp(String, "BFG Cells/Shot"))
		{
			SetClassFieldInt(GameInfoClass, "BFGCELLS", value);
		}
		else if (!VStr::ICmp(String, "Monsters Infight"));	//	What's that?
		else dprintf("WARNING! Invalid misc %s\n", String);
	}
	unguard;
}

//==========================================================================
//
//	FindString
//
//==========================================================================

static void FindString(const char* oldStr, const char* newStr)
{
	guard(FindString);
	//	Sounds
	bool SoundFound = false;
	for (int i = 0; i < SfxNames.Num(); i++)
	{
		if (SfxNames[i].Old == oldStr)
		{
			SfxNames[i].New = newStr;
			SfxNames[i].Replaced = true;
			//	Continue, because other sounds can use the same sound
			SoundFound = true;
		}
	}
	if (SoundFound)
	{
		return;
	}

	//	Music
	bool SongFound = false;
	for (int i = 0; i < MusicNames.Num(); i++)
	{
		if (MusicNames[i].Old == oldStr)
		{
			MusicNames[i].New = newStr;
			MusicNames[i].Replaced = true;
			//	There could be duplicates
			SongFound = true;
		}
	}
	if (SongFound)
	{
		return;
	}

	//	Sprite names
	for (int i = 0; i < SpriteNames.Num(); i++)
	{
		if (SpriteNames[i].Old == oldStr)
		{
			SpriteNames[i].New = newStr;
			SpriteNames[i].Replaced = true;
			return;
		}
	}

	VName Id = EngStrings->GetStringId(oldStr);
	if (Id != NAME_None)
	{
		GLanguage.ReplaceString(Id, newStr);
		return;
	}

	dprintf("Not found old \"%s\" new \"%s\"\n", oldStr, newStr);
	unguard;
}

//==========================================================================
//
//	ReadText
//
//==========================================================================

static void ReadText(int oldSize)
{
	guard(ReadText);
	char	*lenPtr;
	int		newSize;
	char	*oldStr;
	char	*newStr;
	int		len;

	lenPtr = strtok(NULL, " ");
	if (!lenPtr)
	{
		return;
	}
	newSize = atoi(lenPtr);

	oldStr = new char[oldSize + 1];
	newStr = new char[newSize + 1];

	len = 0;
	while (*PatchPtr && len < oldSize)
	{
		if (*PatchPtr == '\r')
		{
			PatchPtr++;
			continue;
		}
		oldStr[len] = *PatchPtr;
		PatchPtr++;
		len++;
	}
	oldStr[len] = 0;

	len = 0;
	while (*PatchPtr && len < newSize)
	{
		if (*PatchPtr == '\r')
		{
			PatchPtr++;
			continue;
		}
		newStr[len] = *PatchPtr;
		PatchPtr++;
		len++;
	}
	newStr[len] = 0;

	FindString(oldStr, newStr);

	delete[] oldStr;
	delete[] newStr;

	GetLine();
	unguard;
}

//==========================================================================
//
//	LoadDehackedFile
//
//==========================================================================

static void LoadDehackedFile(VStream* Strm)
{
	guard(LoadDehackedFile);
	Patch = new char[Strm->TotalSize() + 1];
	Strm->Serialise(Patch, Strm->TotalSize());
	Patch[Strm->TotalSize()] = 0;
	delete Strm;
	PatchPtr = Patch;

	GetLine();
	while (*PatchPtr)
	{
		char* Section = strtok(String, " ");
		if (!Section)
			continue;

		char* numStr = strtok(NULL, " ");
		int i = 0;
		if (numStr)
		{
			i = atoi(numStr);
		}

		if (!strcmp(Section, "Thing"))
		{
			ReadThing(i);
		}
		else if (!strcmp(Section, "Sound"))
		{
			ReadSound(i);
		}
		else if (!strcmp(Section, "Frame"))
		{
			ReadState(i);
		}
		else if (!strcmp(Section, "Sprite"))
		{
			ReadSpriteName(i);
		}
		else if (!strcmp(Section, "Ammo"))
		{
			ReadAmmo(i);
		}
		else if (!strcmp(Section, "Weapon"))
		{
			ReadWeapon(i);
		}
		else if (!strcmp(Section, "Pointer"))
		{
			ReadPointer(i);
		}
		else if (!strcmp(Section, "Cheat"))
		{
			ReadCheats(i);
		}
		else if (!strcmp(Section, "Misc"))
		{
			ReadMisc(i);
		}
		else if (!strcmp(Section, "Text"))
		{
			ReadText(i);
		}
		else
		{
			dprintf("Don't know how to handle \"%s\"\n", String);
			GetLine();
		}
	}
	delete[] Patch;
	unguard;
}

//==========================================================================
//
//	ProcessDehackedFiles
//
//==========================================================================

void ProcessDehackedFiles()
{
	guard(ProcessDehackedFiles);
	int p = GArgs.CheckParm("-deh");
	int LumpNum = W_CheckNumForName("dehacked");
	if (!p && LumpNum < 0)
	{
		return;
	}

	//	Open dehinfo script.
	VStream* Strm = FL_OpenFileRead("dehinfo.txt");
	if (!Strm)
	{
		Sys_Error("dehinfo.txt is required to parse dehacked patches");
	}
	VScriptParser* sc = new VScriptParser("dehinfo.txt", Strm);

	//	Read sprite names.
	sc->Expect("sprites");
	sc->Expect("{");
	while (!sc->Check("}"))
	{
		sc->ExpectString();
		if (sc->String.Length() != 4)
		{
			sc->Error("Sprite name must be 4 characters long");
		}
		Sprites.Append(*sc->String.ToUpper());
	}

	//	Read states.
	sc->Expect("states");
	sc->Expect("{");
	States.Append(NULL);
	StateActions.Append(NULL);
	VState** StatesTail = &VClass::FindClass("Actor")->NetStates;
	while (*StatesTail)
	{
		StatesTail = &(*StatesTail)->NetNext;
	}
	while (!sc->Check("}"))
	{
		//	Class name
		sc->ExpectString();
		VClass* StatesClass = VClass::FindClass(*sc->String);
		//	Starting state specifier
		VState* S = NULL;
		VState** pState = NULL;
		if (sc->Check("First"))
		{
			S = StatesClass->NetStates;
			pState = &StatesClass->NetStates;
		}
		else if (sc->Check("Death"))
		{
			S = GetClassFieldState(StatesClass, "DeathState");
			pState = &StatesClass->NetStates;
			while (*pState && *pState != S)
			{
				pState = &(*pState)->NetNext;
			}
			if (!pState)
			{
				sc->Error("Bad state");
			}
		}
		else if (sc->Check("Extra"))
		{
			S = GetClassFieldState(StatesClass, "ExtraState");
			pState = &StatesClass->NetStates;
			while (*pState && *pState != S)
			{
				pState = &(*pState)->NetNext;
			}
			if (!pState)
			{
				sc->Error("Bad state");
			}
		}
		else
		{
			sc->Error("Expected First or Death");
		}
		//	Number of states
		sc->ExpectNumber();
		for (int i = 0; i < sc->Number; i++)
		{
			if (!S)
			{
				sc->Error("Given class doen't have that many states");
			}
			States.Append(S);
			StateActions.Append(S->Function);
			//	Move net links to actor class.
			*StatesTail = S;
			StatesTail = &S->NetNext;
			*pState = S->NetNext;
			S->NetNext = NULL;
			S = S->Next;
		}
	}
	dprintf("%d states\n", States.Num());

	//	Read code pointer state mapping.
	sc->Expect("code_pointer_states");
	sc->Expect("{");
	while (!sc->Check("}"))
	{
		sc->ExpectNumber();
		if (sc->Number < 0 || sc->Number >= States.Num())
		{
			sc->Error(va("Bad state index %d", sc->Number));
		}
		CodePtrStates.Append(States[sc->Number]);
	}

	//	Read sound names.
	sc->Expect("sounds");
	sc->Expect("{");
	Sounds.Append(NAME_None);
	while (!sc->Check("}"))
	{
		sc->ExpectString();
		Sounds.Append(*sc->String);
	}

	//	Create list of thing classes.
	sc->Expect("things");
	sc->Expect("{");
	while (!sc->Check("}"))
	{
		sc->ExpectString();
		VClass* C = VClass::FindClass(*sc->String);
		if (!C)
		{
			sc->Error(va("No such class %s", *sc->String));
		}
		EntClasses.Append(C);
	}

	//	Create list of weapon classes.
	sc->Expect("weapons");
	sc->Expect("{");
	while (!sc->Check("}"))
	{
		sc->ExpectString();
		VClass* C = VClass::FindClass(*sc->String);
		if (!C)
		{
			sc->Error(va("No such class %s", *sc->String));
		}
		WeaponClasses.Append(C);
	}

	//	Set original thing heights.
	sc->Expect("heights");
	sc->Expect("{");
	int HIdx = 0;
	while (!sc->Check("}"))
	{
		if (HIdx >= EntClasses.Num())
		{
			sc->Error("Too many heights");
		}
		sc->ExpectNumber();
		((VEntity*)EntClasses[HIdx]->Defaults)->Height = sc->Number;
		HIdx++;
	}

	GameInfoClass = VClass::FindClass("MainGameInfo");

	delete sc;

	//	Get lists of strings to replace.
	GSoundManager->GetSoundLumpNames(SfxNames);
	P_GetMusicLumpNames(MusicNames);
	VClass::GetSpriteNames(SpriteNames);
	EngStrings = new VLanguage();
	EngStrings->LoadStrings("en");

	//	Parse dehacked patches.
	if (p)
	{
		while (++p != GArgs.Count() && GArgs[p][0] != '-')
		{
			GCon->Logf("Processing dehacked patch %s", GArgs[p]);

			VStream* Strm = FL_OpenSysFileRead(GArgs[p]);
			if (!Strm)
			{
				GCon->Logf("No such file");
				continue;
			}
			LoadDehackedFile(Strm);
		}
	}
	if (LumpNum >= 0)
	{
		GCon->Logf("Processing dehacked patch lump");
		LoadDehackedFile(W_CreateLumpReaderNum(LumpNum));
	}

	for (int i = 0; i < EntClasses.Num(); i++)
	{
		//	Set all classes to use old style pickup handling.
		SetClassFieldBool(EntClasses[i], "bDehackedSpecial", true);
	}

	//	Do string replacement.
	GSoundManager->ReplaceSoundLumpNames(SfxNames);
	P_ReplaceMusicLumpNames(MusicNames);
	VClass::ReplaceSpriteNames(SpriteNames);

	VClass::StaticReinitStatesLookup();

	//	Clean up.
	Sprites.Clear();
	EntClasses.Clear();
	WeaponClasses.Clear();
	States.Clear();
	CodePtrStates.Clear();
	StateActions.Clear();
	Sounds.Clear();
	SfxNames.Clear();
	MusicNames.Clear();
	SpriteNames.Clear();
	delete EngStrings;
	unguard;
}
