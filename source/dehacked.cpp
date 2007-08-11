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

struct VCodePtrInfo
{
	VStr		Name;
	VMethod*	Method;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char*					Patch;
static char*					PatchPtr;
static char*					String;
static char*					ValueString;
static int						value;

static TArray<VName>			Sprites;
static TArray<VClass*>			EntClasses;
static TArray<VClass*>			WeaponClasses;
static TArray<VClass*>			AmmoClasses;
static TArray<VState*>			States;
static TArray<VState*>			CodePtrStates;
static TArray<VMethod*>			StateActions;
static TArray<VCodePtrInfo>		CodePtrs;
static TArray<VName>			Sounds;

static VClass*					GameInfoClass;
static VClass*					BfgClass;
static VClass*					SoulsphereClass;
static VClass*					MegaHealthClass;

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

	ValueString = val + 1;
	while (*ValueString && (vuint8)*ValueString <= ' ')
	{
		ValueString++;
	}
	value = atoi(ValueString);

	do
	{
		*val = 0;
		val--;
	}
	while (val >= String && (vuint8)*val <= ' ');

	return true;
	unguard;
}

//==========================================================================
//
//	GetClassFieldInt
//
//==========================================================================

static int GetClassFieldInt(VClass* Class, const char* FieldName)
{
	guard(GetClassFieldInt);
	VField* F = Class->FindFieldChecked(FieldName);
	int* Ptr = (int*)(Class->Defaults + F->Ofs);
	return *Ptr;
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
//	GetClassFieldClass
//
//==========================================================================

static VClass* GetClassFieldClass(VClass* Class, const char* FieldName)
{
	guard(GetClassFieldClass);
	VField* F = Class->FindFieldChecked(FieldName);
	VClass** Ptr = (VClass**)(Class->Defaults + F->Ofs);
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
	guard(SetClassFieldState);
	VField* F = Class->FindFieldChecked(FieldName);
	VState** Ptr = (VState**)(Class->Defaults + F->Ofs);
	*Ptr = Value;
	unguard;
}

//==========================================================================
//
//	SetClassFieldClass
//
//==========================================================================

static void SetClassFieldClass(VClass* Class, const char* FieldName,
	VClass* Value)
{
	guard(SetClassFieldClass);
	VField* F = Class->FindFieldChecked(FieldName);
	VClass** Ptr = (VClass**)(Class->Defaults + F->Ofs);
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
//	ParseFlags
//
//==========================================================================

static int ParseFlags()
{
	guard(ParseFlags);
	VStr FlagsStr = VStr(ValueString).Replace(" ", "");
	TArray<VStr> FlagsArray;
	FlagsStr.Split('+', FlagsArray);
	int Flags = 0;
	for (int i = 0; i < FlagsArray.Num(); i++)
	{
		VStr Flag(FlagsArray[i].ToUpper());
		if (Flag == "SPECIAL")
			Flags |= 0x00000001;
		else if (Flag == "SOLID")
			Flags |= 0x00000002;
		else if (Flag == "SHOOTABLE")
			Flags |= 0x00000004;
		else if (Flag == "NOSECTOR")
			Flags |= 0x00000008;
		else if (Flag == "NOBLOCKMAP")
			Flags |= 0x00000010;
		else if (Flag == "AMBUSH")
			Flags |= 0x00000020;
		else if (Flag == "JUSTHIT")
			Flags |= 0x00000040;
		else if (Flag == "JUSTATTACKED")
			Flags |= 0x00000080;
		else if (Flag == "SPAWNCEILING")
			Flags |= 0x00000100;
		else if (Flag == "NOGRAVITY")
			Flags |= 0x00000200;
		else if (Flag == "DROPOFF")
			Flags |= 0x00000400;
		else if (Flag == "PICKUP")
			Flags |= 0x00000800;
		else if (Flag == "NOCLIP")
			Flags |= 0x00001000;
		else if (Flag == "SLIDE")
			Flags |= 0x00002000;
		else if (Flag == "FLOAT")
			Flags |= 0x00004000;
		else if (Flag == "TELEPORT")
			Flags |= 0x00008000;
		else if (Flag == "MISSILE")
			Flags |= 0x00010000;
		else if (Flag == "DROPPED")
			Flags |= 0x00020000;
		else if (Flag == "SHADOW")
			Flags |= 0x00040000;
		else if (Flag == "NOBLOOD")
			Flags |= 0x00080000;
		else if (Flag == "CORPSE")
			Flags |= 0x00100000;
		else if (Flag == "INFLOAT")
			Flags |= 0x00200000;
		else if (Flag == "COUNTKILL")
			Flags |= 0x00400000;
		else if (Flag == "COUNTITEM")
			Flags |= 0x00800000;
		else if (Flag == "SKULLFLY")
			Flags |= 0x01000000;
		else if (Flag == "NOTDMATCH")
			Flags |= 0x02000000;
		else if (Flag == "TRANSLATION1" || Flag == "TRANSLATION")
			Flags |= 0x04000000;
		else if (Flag == "TRANSLATION2" || Flag == "UNUSED1")
			Flags |= 0x08000000;
		else if (Flag == "UNUSED2")
			Flags |= 0x10000000;
		else if (Flag == "UNUSED3")
			Flags |= 0x20000000;
		else if (Flag == "UNUSED4")
			Flags |= 0x40000000;
		else if (Flag == "TRANSLUCENT")
			Flags |= 0x80000000;
		else
			dprintf("WARINIG! Unknown flag %s\n", *Flag);
	}
	return Flags;
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
	if (num < 1 || num > EntClasses.Num())
	{
		dprintf("WARNING! Invalid thing num %d\n", num);
		while (ParseParam());
		return;
	}

	VClass* Ent = EntClasses[num - 1];
	while (ParseParam())
	{
		if (!VStr::ICmp(String ,"ID #"))
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
		else if (!VStr::ICmp(String, "Hit points"))
		{
			SetClassFieldInt(Ent, "Health", value);
			SetClassFieldInt(Ent, "GibsHealth", -value);
		}
		else if (!VStr::ICmp(String, "Reaction time"))
		{
			SetClassFieldInt(Ent, "ReactionCount", value);
		}
		else if (!VStr::ICmp(String, "Missile damage"))
		{
			SetClassFieldInt(Ent, "MissileDamage", value);
		}
		else if (!VStr::ICmp(String, "Width"))
		{
			SetClassFieldFloat(Ent, "Radius", value / 65536.0);
		}
		else if (!VStr::ICmp(String, "Height"))
		{
			SetClassFieldFloat(Ent, "Height", value / 65536.0);
		}
		else if (!VStr::ICmp(String, "Mass"))
		{
			SetClassFieldFloat(Ent, "Mass", value == 0x7fffffff ? 99999.0 : value);
		}
		else if (!VStr::ICmp(String, "Speed"))
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
		else if (!VStr::ICmp(String, "Pain chance"))
		{
			SetClassFieldFloat(Ent, "PainChance", value / 256.0);
		}
		else if (!VStr::ICmp(String, "Bits"))
		{
			if (*ValueString < '0' || *ValueString > '9')
			{
				value = ParseFlags();
			}
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
			SetClassFieldBool(Ent, "bActivatePushWall", (value & 0x00400000) || num == 1);
			SetClassFieldBool(Ent, "bCountItem", value & 0x00800000);
			SetClassFieldBool(Ent, "bSkullFly", value & 0x01000000);
			SetClassFieldBool(Ent, "bNoDeathmatch", value & 0x02000000);
			SetClassFieldBool(Ent, "bNoPassMobj", num != 1 && !(value & 0x00400000) && !(value & 0x00010000));
			//	Translation
			SetClassFieldInt(Ent, "Translation", (value & 0x0c000000) >> 26);
			//	Alpha
			SetClassFieldFloat(Ent, "Alpha", (value & 0x00040000) ? 0.1 : (value & 0x80000000) ? 0.5 : 1.0);
			//	The following fields were not used
			//value & 0x10000000);
			//value & 0x20000000);
			//value & 0x40000000);
		}
		//
		//	States
		//
		else if (!VStr::ICmp(String, "Initial frame"))
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
		else if (!VStr::ICmp(String, "First moving frame"))
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
		else if (!VStr::ICmp(String, "Close attack frame"))
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
		else if (!VStr::ICmp(String, "Far attack frame"))
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
		else if (!VStr::ICmp(String, "Injury frame"))
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
		else if (!VStr::ICmp(String, "Death frame"))
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
		else if (!VStr::ICmp(String, "Exploding frame"))
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
		else if (!VStr::ICmp(String, "Respawn frame"))
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
		else if (!VStr::ICmp(String, "Alert sound"))
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
		else if (!VStr::ICmp(String, "Action sound"))
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
		else if (!VStr::ICmp(String, "Attack sound"))
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
		else if (!VStr::ICmp(String, "Pain sound"))
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
		else if (!VStr::ICmp(String, "Death sound"))
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
		if (!VStr::ICmp(String, "Offset"));				//Lump name offset - can't handle
		else if (!VStr::ICmp(String, "Zero/One"));		//Singularity - removed
		else if (!VStr::ICmp(String, "Value"));			//Priority
		else if (!VStr::ICmp(String, "Zero 1"));		//Lump num - can't be set
		else if (!VStr::ICmp(String, "Zero 2"));		//Data pointer - can't be set
		else if (!VStr::ICmp(String, "Zero 3"));		//Usefulness - removed
		else if (!VStr::ICmp(String, "Zero 4"));		//Link - removed
		else if (!VStr::ICmp(String, "Neg. One 1"));	//Link pitch - removed
		else if (!VStr::ICmp(String, "Neg. One 2"));	//Link volume - removed
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
		if (!VStr::ICmp(String, "Sprite number"))
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
		else if (!VStr::ICmp(String, "Sprite subnumber"))
		{
			if (value & 0x8000)
			{
				value &= 0x7fff;
				value |= FF_FULLBRIGHT;
			}
			States[num]->Frame = value;
		}
		else if (!VStr::ICmp(String, "Duration"))
		{
			States[num]->Time = value < 0 ? value : value / 35.0;
		}
		else if (!VStr::ICmp(String, "Next frame"))
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
		else if (!VStr::ICmp(String, "Unknown 1"))
		{
			States[num]->Misc1 = value;
		}
		else if (!VStr::ICmp(String, "Unknown 2"))
		{
			States[num]->Misc2 = value;
		}
		else if (!VStr::ICmp(String, "Action pointer"))
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
	if (num >= AmmoClasses.Num() || num < 0)
	{
		dprintf("WARNING! Invalid ammo num %d\n", num);
		while (ParseParam());
		return;
	}

	VClass* Ammo = AmmoClasses[num];
	while (ParseParam())
	{
		if (!VStr::ICmp(String, "Max ammo"))
		{
			SetClassFieldInt(Ammo, "MaxAmount", value);
			SetClassFieldInt(Ammo, "BackpackMaxAmount", value * 2);
		}
		else if (!VStr::ICmp(String, "Per ammo"))
		{
			SetClassFieldInt(Ammo, "Amount", value);
			SetClassFieldInt(Ammo, "BackpackAmount", value);
		}
		else
		{
			dprintf("WARNING! Invalid ammo param %s\n", String);
		}
	}

	//	Fix up amounts in derived classes
	for (VClass* C = VMemberBase::GClasses; C; C = C->LinkNext)
	{
		if (!C->IsChildOf(Ammo))
		{
			continue;
		}
		if (C == Ammo)
		{
			continue;
		}
		SetClassFieldInt(C, "Amount", GetClassFieldInt(Ammo, "Amount") * 5);
		SetClassFieldInt(C, "MaxAmount", GetClassFieldInt(Ammo, "MaxAmount"));
		SetClassFieldInt(C, "BackpackAmount", GetClassFieldInt(Ammo, "BackpackAmount") * 5);
		SetClassFieldInt(C, "BackpackMaxAmount", GetClassFieldInt(Ammo, "BackpackMaxAmount"));
	}

	//	Fix up amounts in weapon classes.
	for (int i = 0; i < WeaponClasses.Num(); i++)
	{
		VClass* C = WeaponClasses[i];
		if (GetClassFieldClass(C, "AmmoType1") == Ammo)
		{
			SetClassFieldInt(C, "AmmoGive1", GetClassFieldInt(Ammo, "Amount") * 2);
		}
		if (GetClassFieldClass(C, "AmmoType2") == Ammo)
		{
			SetClassFieldInt(C, "AmmoGive2", GetClassFieldInt(Ammo, "Amount") * 2);
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
			if (value < AmmoClasses.Num())
			{
				SetClassFieldClass(Weapon, "AmmoType1", AmmoClasses[value]);
				SetClassFieldInt(Weapon, "AmmoGive1",
					GetClassFieldInt(AmmoClasses[value], "Amount") * 2);
				if (GetClassFieldInt(Weapon, "AmmoUse1") == 0)
				{
					SetClassFieldInt(Weapon, "AmmoUse1", 1);
				}
			}
			else
			{
				SetClassFieldClass(Weapon, "AmmoType1", NULL);
			}
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
//	ReadCodePtr
//
//==========================================================================

static void ReadCodePtr(int)
{
	guard(ReadCodePtr);
	while (ParseParam())
	{
		if (!VStr::NICmp(String, "Frame", 5) && (vuint8)String[5] <= ' ')
		{
			int Index = atoi(String + 5);
			if (Index < 0 || Index >= States.Num())
			{
				dprintf("Bad frame index %d", Index);
				continue;
			}
			VState* State = States[Index];

			if ((ValueString[0] == 'A' || ValueString[0] == 'a') &&
				ValueString[1] == '_')
			{
				ValueString += 2;
			}

			bool Found = false;
			for (int i = 0; i < CodePtrs.Num(); i++)
			{
				if (!CodePtrs[i].Name.ICmp(ValueString))
				{
					State->Function = CodePtrs[i].Method;
					Found = true;
					break;
				}
			}
			if (!Found)
			{
				dprintf("WARNING! Invalid code pointer %s\n", ValueString);
			}
		}
		else
		{
			dprintf("WARNING! Invalid code pointer param %s\n", String);
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
			SetClassFieldInt(SoulsphereClass, "MaxAmount", value);
		}
		else if (!VStr::ICmp(String, "Soulsphere Health"))
		{
			SetClassFieldInt(SoulsphereClass, "Amount", value);
		}
		else if (!VStr::ICmp(String, "Megasphere Health"))
		{
			SetClassFieldInt(MegaHealthClass, "Amount", value);
			SetClassFieldInt(MegaHealthClass, "MaxAmount", value);
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
			SetClassFieldInt(BfgClass, "AmmoUse1", value);
		}
		else if (!VStr::ICmp(String, "Monsters Infight"));	//	What's that?
		else dprintf("WARNING! Invalid misc %s\n", String);
	}
	unguard;
}

//==========================================================================
//
//	ReadPars
//
//==========================================================================

static void ReadPars(int)
{
	guard(ReadPars);
	while (GetLine())
	{
		if (strchr(String, '='))
		{
			dprintf("WARNING! Unknown key in Pars section.\n");
			continue;
		}
		if (VStr::NICmp(String, "par", 3) || (vuint8)String[3] > ' ')
		{
			return;
		}
		strtok(String, " ");
		char* Num1 = strtok(NULL, " ");
		char* Num2 = strtok(NULL, " ");
		char* Num3 = strtok(NULL, " ");
		if (!Num1 || !Num2)
		{
			dprintf("WARNING! Bad par time\n");
			continue;
		}
		VName MapName;
		int Par;
		if (Num3)
		{
			MapName = va("e%cm%c", Num1[0], Num2[0]);
			Par = atoi(Num3);
		}
		else
		{
			MapName = va("map%02d", atoi(Num1) % 100);
			Par = atoi(Num2);
		}
		P_SetParTime(MapName, Par);
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
//	ReplaceSpecialChars
//
//==========================================================================

static VStr ReplaceSpecialChars(VStr& In)
{
	guard(ReplaceSpecialChars);
	VStr Ret;
	const char* pStr = *In;
	while (*pStr)
	{
		char c = *pStr++;
		if (c != '\\')
		{
			Ret += c;
		}
		else
		{
			switch (*pStr)
			{
			case 'n':
			case 'N':
				Ret += '\n';
				break;
			case 't':
			case 'T':
				Ret += '\t';
				break;
			case 'r':
			case 'R':
				Ret += '\r';
				break;
			case 'x':
			case 'X':
				c = 0;
				pStr++;
				for (int i = 0; i < 2; i++)
				{
					c <<= 4;
					if (*pStr >= '0' && *pStr <= '9')
						c += *pStr - '0';
					else if (*pStr >= 'a' && *pStr <= 'f')
						c += 10 + *pStr - 'a';
					else if (*pStr >= 'A' && *pStr <= 'F')
						c += 10 + *pStr - 'A';
					else
						break;
					pStr++;
				}
				Ret += c;
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				c = 0;
				for (int i = 0; i < 3; i++)
				{
					c <<= 3;
					if (*pStr >= '0' && *pStr <= '7')
						c += *pStr - '0';
					else
						break;
					pStr++;
				}
				Ret += c;
				break;
			default:
				Ret += *pStr;
				break;
			}
			pStr++;
		}
	}
	return Ret;
	unguard;
}

//==========================================================================
//
//	ReadStrings
//
//==========================================================================

static void ReadStrings(int)
{
	guard(ReadStrings);
	while (ParseParam())
	{
		VName Id = *VStr(String).ToLower();
		VStr Val;
		do
		{
			char* End = ValueString + VStr::Length(ValueString) - 1;
			while (End >= ValueString && (vuint8)*End <= ' ')
			{
				End--;
			}
			End[1] = 0;
			if (End >= ValueString && *End == '\\')
			{
				*End = 0;
				Val += ValueString;
				ValueString = PatchPtr;
				while (*PatchPtr && *PatchPtr != '\n')
				{
					PatchPtr++;
				}
				if (*PatchPtr == '\n')
				{
					*PatchPtr = 0;
					PatchPtr++;
				}
				while (*ValueString && *ValueString <= ' ')
				{
					ValueString++;
				}
			}
			else
			{
				Val += ValueString;
				ValueString = NULL;
			}
		}
		while (ValueString && *ValueString);
		Val = ReplaceSpecialChars(Val);
		GLanguage.ReplaceString(Id, Val);
	}
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

		if (!VStr::ICmp(Section, "Thing"))
		{
			ReadThing(i);
		}
		else if (!VStr::ICmp(Section, "Sound"))
		{
			ReadSound(i);
		}
		else if (!VStr::ICmp(Section, "Frame"))
		{
			ReadState(i);
		}
		else if (!VStr::ICmp(Section, "Sprite"))
		{
			ReadSpriteName(i);
		}
		else if (!VStr::ICmp(Section, "Ammo"))
		{
			ReadAmmo(i);
		}
		else if (!VStr::ICmp(Section, "Weapon"))
		{
			ReadWeapon(i);
		}
		else if (!VStr::ICmp(Section, "Pointer"))
		{
			ReadPointer(i);
		}
		else if (!VStr::ICmp(Section, "Cheat"))
		{
			ReadCheats(i);
		}
		else if (!VStr::ICmp(Section, "Misc"))
		{
			ReadMisc(i);
		}
		else if (!VStr::ICmp(Section, "Text"))
		{
			ReadText(i);
		}
		else if (!VStr::ICmp(Section, "[Strings]"))
		{
			ReadStrings(i);
		}
		else if (!VStr::ICmp(Section, "[Pars]"))
		{
			ReadPars(i);
		}
		else if (!VStr::ICmp(Section, "[CodePtr]"))
		{
			ReadCodePtr(i);
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
		if (!StatesClass)
		{
			Sys_Error("No such class %s", *sc->String);
		}
		//	Starting state specifier
		VState* S = NULL;
		VState** pState = NULL;
		if (sc->Check("First"))
		{
			S = StatesClass->NetStates;
			pState = &StatesClass->NetStates;
		}
		else if (sc->Check("Spawn"))
		{
			S = GetClassFieldState(StatesClass, "IdleState");
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

	//	Read code pointers.
	sc->Expect("code_pointers");
	sc->Expect("{");
	VCodePtrInfo& ANull = CodePtrs.Alloc();
	ANull.Name = "NULL";
	ANull.Method = NULL;
	while (!sc->Check("}"))
	{
		sc->ExpectString();
		VStr Name = sc->String;
		sc->ExpectString();
		VStr ClassName = sc->String;
		sc->ExpectString();
		VStr MethodName = sc->String;
		VClass* Class = VClass::FindClass(*ClassName);
		if (!Class)
		{
			sc->Error("No such class");
		}
		VMethod* Method = Class->FindFunction(*MethodName);
		if (!Method)
		{
			sc->Error(va("No such method %s", *MethodName));
		}
		VCodePtrInfo& P = CodePtrs.Alloc();
		P.Name = Name;
		P.Method = Method;
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

	//	Create list of ammo classes.
	sc->Expect("ammo");
	sc->Expect("{");
	while (!sc->Check("}"))
	{
		sc->ExpectString();
		VClass* C = VClass::FindClass(*sc->String);
		if (!C)
		{
			sc->Error(va("No such class %s", *sc->String));
		}
		AmmoClasses.Append(C);
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
	BfgClass = VClass::FindClass("BFG9000");
	SoulsphereClass = VClass::FindClass("Soulsphere");
	MegaHealthClass = VClass::FindClass("MegasphereHealth");

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
	SetClassFieldBool(GameInfoClass, "bDehacked", true);

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
	CodePtrs.Clear();
	Sounds.Clear();
	SfxNames.Clear();
	MusicNames.Clear();
	SpriteNames.Clear();
	delete EngStrings;
	unguard;
}
