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

struct VDehFlag
{
	int				Which;
	const char*		Name;
	int				Mask;
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
static VClass*					DoomPlayerClass;
static VClass*					BfgClass;
static VClass*					HealthBonusClass;
static VClass*					SoulsphereClass;
static VClass*					MegaHealthClass;
static VClass*					GreenArmorClass;
static VClass*					BlueArmorClass;
static VClass*					ArmorBonusClass;

static TArray<FReplacedString>	SfxNames;
static TArray<FReplacedString>	MusicNames;
static TArray<FReplacedString>	SpriteNames;
static VLanguage*				EngStrings;

static const VDehFlag DehFlags[] =
{
	{ 0, "SPECIAL", 0x00000001 },
	{ 0, "SOLID", 0x00000002 },
	{ 0, "SHOOTABLE", 0x00000004 },
	{ 0, "NOSECTOR", 0x00000008 },
	{ 0, "NOBLOCKMAP", 0x00000010 },
	{ 0, "AMBUSH", 0x00000020 },
	{ 0, "JUSTHIT", 0x00000040 },
	{ 0, "JUSTATTACKED", 0x00000080 },
	{ 0, "SPAWNCEILING", 0x00000100 },
	{ 0, "NOGRAVITY", 0x00000200 },
	{ 0, "DROPOFF", 0x00000400 },
	{ 0, "PICKUP", 0x00000800 },
	{ 0, "NOCLIP", 0x00001000 },
	{ 0, "FLOAT", 0x00004000 },
	{ 0, "TELEPORT", 0x00008000 },
	{ 0, "MISSILE", 0x00010000 },
	{ 0, "DROPPED", 0x00020000 },
	{ 0, "SHADOW", 0x00040000 },
	{ 0, "NOBLOOD", 0x00080000 },
	{ 0, "CORPSE", 0x00100000 },
	{ 0, "INFLOAT", 0x00200000 },
	{ 0, "COUNTKILL", 0x00400000 },
	{ 0, "COUNTITEM", 0x00800000 },
	{ 0, "SKULLFLY", 0x01000000 },
	{ 0, "NOTDMATCH", 0x02000000 },
	{ 0, "TRANSLATION1", 0x04000000 },
	{ 0, "TRANSLATION", 0x04000000 },
	{ 0, "TRANSLATION2", 0x08000000 },
	{ 0, "UNUSED1", 0x08000000 },
	{ 0, "TRANSLUC25", 0x10000000 },
	{ 0, "TRANSLUC75", 0x20000000 },
	{ 0, "STEALTH", 0x40000000 },
	{ 0, "UNUSED4", 0x40000000 },
	{ 0, "TRANSLUCENT", 0x80000000 },
	{ 0, "TRANSLUC50", 0x80000000 },
	{ 1, "LOGRAV", 0x00000001 },
	{ 1, "WINDTHRUST", 0x00000002 },
	{ 1, "FLOORBOUNCE", 0x00000004 },
	{ 1, "BLASTED", 0x00000008 },
	{ 1, "FLY", 0x00000010 },
	{ 1, "FLOORCLIP", 0x00000020 },
	{ 1, "SPAWNFLOAT", 0x00000040 },
	{ 1, "NOTELEPORT", 0x00000080 },
	{ 1, "RIP", 0x00000100 },
	{ 1, "PUSHABLE", 0x00000200 },
	{ 1, "CANSLIDE", 0x00000400 },
	{ 1, "ONMOBJ", 0x00000800 },
	{ 1, "PASSMOBJ", 0x00001000 },
	{ 1, "CANNOTPUSH", 0x00002000 },
	{ 1, "THRUGHOST", 0x00004000 },
	{ 1, "BOSS", 0x00008000 },
	{ 1, "FIREDAMAGE", 0x00010000 },
	{ 1, "NODMGTHRUST", 0x00020000 },
	{ 1, "TELESTOMP", 0x00040000 },
	{ 1, "DONTDRAW", 0x00080000 },
	{ 1, "FLOATBOB", 0x00100000 },
	{ 1, "IMPACT", 0x00200000 },
	{ 1, "PUSHWALL", 0x00400000 },
	{ 1, "MCROSS", 0x00800000 },
	{ 1, "PCROSS", 0x01000000 },
	{ 1, "CANTLEAVEFLOORPIC", 0x02000000 },
	{ 1, "NONSHOOTABLE", 0x04000000 },
	{ 1, "INVULNERABLE", 0x08000000 },
	{ 1, "DORMANT", 0x10000000 },
	{ 1, "ICEDAMAGE", 0x20000000 },
	{ 1, "SEEKERMISSILE", 0x40000000 },
	{ 1, "REFLECTIVE", 0x80000000 },
	//	Ignored flags
	{ 0, "SLIDE", 0 },
	{ 0, "UNUSED2", 0 },
	{ 0, "UNUSED3", 0 },
};

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
			String = NULL;
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
		char* End = String + VStr::Length(String);
		while (End > String && End[-1] <= ' ')
		{
			End[-1] = 0;
			End--;
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
//	SetClassFieldByte
//
//==========================================================================

static void SetClassFieldByte(VClass* Class, const char* FieldName,
	vuint8 Value)
{
	guard(SetClassFieldByte);
	VField* F = Class->FindFieldChecked(FieldName);
	vuint8* Ptr = Class->Defaults + F->Ofs;
	*Ptr = Value;
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
//	ParseFlag
//
//==========================================================================

static void ParseFlag(const VStr& FlagName, int* Values, bool* Changed)
{
	guard(ParseFlags);
	if (FlagName.Length() == 0)
	{
		return;
	}
	if (FlagName[0] >= '0' && FlagName[0] <= '9')
	{
		//	Clear flags that were not used by Doom engine as well as SLIDE
		// flag which desn't exist anymore.
		Values[0] = atoi(*FlagName) & 0x0fffdfff;
		Changed[0] = true;
		return;
	}
	for (size_t i = 0; i < ARRAY_COUNT(DehFlags); i++)
	{
		if (FlagName == DehFlags[i].Name)
		{
			Values[DehFlags[i].Which] |= DehFlags[i].Mask;
			Changed[DehFlags[i].Which] = true;
			return;
		}
	}
	GCon->Logf("WARINIG! Unknown flag %s", *FlagName);
	unguard;
}

//==========================================================================
//
//	DoThingState
//
//==========================================================================

static void DoThingState(VClass* Ent, const char* StateLabel)
{
	guard(DoThingState);
	if (value < 0 || value >= States.Num())
	{
		GCon->Logf("WARNING! Bad thing state %d", value);
	}
	else
	{
		Ent->SetStateLabel(StateLabel, States[value]);
	}
	unguard;
}

//==========================================================================
//
//	DoThingSound
//
//==========================================================================

static void DoThingSound(VClass* Ent, const char* FieldName)
{
	//	If it's not a number, treat it like a sound defined in SNDINFO
	if (ValueString[0] < '0' || ValueString[0] > '9')
	{
		SetClassFieldName(Ent, FieldName, ValueString);
	}
	else if (value < 0 || value >= Sounds.Num())
	{
		GCon->Logf("WARNING! Bad sound index %d", value);
	}
	else
	{
		SetClassFieldName(Ent, FieldName, Sounds[value]);
	}
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
		GCon->Logf("WARNING! Invalid thing num %d", num);
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
				if (VClass::GMobjInfos[i].Class == Ent)
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
					VClass::GMobjInfos.Alloc().Class = Ent;
					VClass::GMobjInfos[Idx].GameFilter = 0;
				}
				VClass::GMobjInfos[Idx].DoomEdNum = value;
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
				SetClassFieldFloat(Ent, "Speed", 35.0 * value);
			}
			else
			{
				SetClassFieldFloat(Ent, "Speed", 35.0 * value / 65536.0);
			}
		}
		else if (!VStr::ICmp(String, "Pain chance"))
		{
			SetClassFieldFloat(Ent, "PainChance", value / 256.0);
		}
		else if (!VStr::ICmp(String, "Translucency"))
		{
			SetClassFieldFloat(Ent, "Alpha", value / 65536.0);
			SetClassFieldByte(Ent, "RenderStyle", STYLE_Translucent);
		}
		else if (!VStr::ICmp(String, "Alpha"))
		{
			SetClassFieldFloat(Ent, "Alpha", atof(ValueString));
		}
		else if (!VStr::ICmp(String, "Render Style"))
		{
			int RenderStyle = STYLE_Normal;
			if (!VStr::ICmp(ValueString, "STYLE_None"))
			{
				RenderStyle = STYLE_None;
			}
			else if (!VStr::ICmp(ValueString, "STYLE_Normal"))
			{
				RenderStyle = STYLE_Normal;
			}
			else if (!VStr::ICmp(ValueString, "STYLE_Fuzzy"))
			{
				RenderStyle = STYLE_Fuzzy;
			}
			else if (!VStr::ICmp(ValueString, "STYLE_SoulTrans"))
			{
				RenderStyle = STYLE_SoulTrans;
			}
			else if (!VStr::ICmp(ValueString, "STYLE_OptFuzzy"))
			{
				RenderStyle = STYLE_OptFuzzy;
			}
			else if (!VStr::ICmp(ValueString, "STYLE_Translucent"))
			{
				RenderStyle = STYLE_Translucent;
			}
			else if (!VStr::ICmp(ValueString, "STYLE_Add"))
			{
				RenderStyle = STYLE_Add;
			}
			else
			{
				GCon->Logf("Bad render style %s", ValueString);
			}
			SetClassFieldByte(Ent, "RenderStyle", RenderStyle);
		}
		else if (!VStr::ICmp(String, "Scale"))
		{
			float Scale = atof(ValueString);
			Scale = MID(0.0001, Scale, 256.0);
			SetClassFieldFloat(Ent, "ScaleX", Scale);
			SetClassFieldFloat(Ent, "ScaleY", Scale);
		}
		else if (!VStr::ICmp(String, "Bits"))
		{
			TArray<VStr> Flags;
			VStr Tmp(ValueString);
			Tmp.Split(" ,+|\t\f\r", Flags);
			int Values[2] = { 0, 0 };
			bool Changed[2] = { false, false };
			for (int i = 0; i < Flags.Num(); i++)
			{
				ParseFlag(Flags[i].ToUpper(), Values, Changed);
			}
			if (Changed[0])
			{
				SetClassFieldBool(Ent, "bSpecial", Values[0] & 0x00000001);
				SetClassFieldBool(Ent, "bSolid", Values[0] & 0x00000002);
				SetClassFieldBool(Ent, "bShootable", Values[0] & 0x00000004);
				SetClassFieldBool(Ent, "bNoSector", Values[0] & 0x00000008);
				SetClassFieldBool(Ent, "bNoBlockmap", Values[0] & 0x00000010);
				SetClassFieldBool(Ent, "bAmbush", Values[0] & 0x00000020);
				SetClassFieldBool(Ent, "bJustHit", Values[0] & 0x00000040);
				SetClassFieldBool(Ent, "bJustAttacked", Values[0] & 0x00000080);
				SetClassFieldBool(Ent, "bSpawnCeiling", Values[0] & 0x00000100);
				SetClassFieldBool(Ent, "bNoGravity", Values[0] & 0x00000200);
				SetClassFieldBool(Ent, "bDropOff", Values[0] & 0x00000400);
				SetClassFieldBool(Ent, "bPickUp", Values[0] & 0x00000800);
				SetClassFieldBool(Ent, "bColideWithThings", !(Values[0] & 0x00001000));
				SetClassFieldBool(Ent, "bColideWithWorld", !(Values[0] & 0x00001000));
				SetClassFieldBool(Ent, "bFloat", Values[0] & 0x00004000);
				SetClassFieldBool(Ent, "bTeleport", Values[0] & 0x00008000);
				SetClassFieldBool(Ent, "bMissile", Values[0] & 0x00010000);
				SetClassFieldBool(Ent, "bDropped", Values[0] & 0x00020000);
				SetClassFieldBool(Ent, "bShadow", Values[0] & 0x00040000);
				SetClassFieldBool(Ent, "bNoBlood", Values[0] & 0x00080000);
				SetClassFieldBool(Ent, "bCorpse", Values[0] & 0x00100000);
				SetClassFieldBool(Ent, "bInFloat", Values[0] & 0x00200000);
				SetClassFieldBool(Ent, "bCountKill", Values[0] & 0x00400000);
				SetClassFieldBool(Ent, "bCountItem", Values[0] & 0x00800000);
				SetClassFieldBool(Ent, "bSkullFly", Values[0] & 0x01000000);
				SetClassFieldBool(Ent, "bNoDeathmatch", Values[0] & 0x02000000);
				SetClassFieldBool(Ent, "bStealth", Values[0] & 0x40000000);

				//	Set additional flags for missiles.
				SetClassFieldBool(Ent, "bActivatePCross", Values[0] & 0x00010000);
				SetClassFieldBool(Ent, "bNoTeleport", Values[0] & 0x00010000);

				//	Set additional flags for monsters.
				SetClassFieldBool(Ent, "bMonster", Values[0] & 0x00400000);
				SetClassFieldBool(Ent, "bActivateMCross", Values[0] & 0x00400000);
				//	Set push wall for both monsters and the player.
				SetClassFieldBool(Ent, "bActivatePushWall", (Values[0] & 0x00400000) || num == 1);
				//	Also set pass mobj flag.
				SetClassFieldBool(Ent, "bPassMobj", num == 1 ||
					(Values[0] & 0x00400000) || (Values[0] & 0x00010000));

				//	Translation
				SetClassFieldInt(Ent, "Translation", Values[0] & 0x0c000000 ?
					(TRANSL_Standard << TRANSL_TYPE_SHIFT) + ((Values[0] & 0x0c000000) >> 26) - 1 : 0);

				//	Alpha and render style.
				SetClassFieldFloat(Ent, "Alpha", (Values[0] & 0x00040000) ? 0.1 :
					(Values[0] & 0x80000000) ? 0.5 :
					(Values[0] & 0x10000000) ? 0.25 :
					(Values[0] & 0x20000000) ? 0.75 : 1.0);
				SetClassFieldByte(Ent, "RenderStyle", (Values[0] & 0x00040000) ?
					STYLE_OptFuzzy : (Values[0] & 0xb0000000) ? STYLE_Translucent : STYLE_Normal);
			}
			if (Changed[1])
			{
				SetClassFieldBool(Ent, "bWindThrust", Values[1] & 0x00000002);
				SetClassFieldBool(Ent, "bBlasted", Values[1] & 0x00000008);
				SetClassFieldBool(Ent, "bFly", Values[1] & 0x00000010);
				SetClassFieldBool(Ent, "bFloorClip", Values[1] & 0x00000020);
				SetClassFieldBool(Ent, "bSpawnFloat", Values[1] & 0x00000040);
				SetClassFieldBool(Ent, "bNoTeleport", Values[1] & 0x00000080);
				SetClassFieldBool(Ent, "bRip", Values[1] & 0x00000100);
				SetClassFieldBool(Ent, "bPushable", Values[1] & 0x00000200);
				SetClassFieldBool(Ent, "bSlide", Values[1] & 0x00000400);
				SetClassFieldBool(Ent, "bOnMobj", Values[1] & 0x00000800);
				SetClassFieldBool(Ent, "bPassMobj", Values[1] & 0x00001000);
				SetClassFieldBool(Ent, "bCannotPush", Values[1] & 0x00002000);
				SetClassFieldBool(Ent, "bThruGhost", Values[1] & 0x00004000);
				SetClassFieldBool(Ent, "bBoss", Values[1] & 0x00008000);
				SetClassFieldBool(Ent, "bNoDamageThrust", Values[1] & 0x00020000);
				SetClassFieldBool(Ent, "bTelestomp", Values[1] & 0x00040000);
				SetClassFieldBool(Ent, "bInvisible", Values[1] & 0x00080000);
				SetClassFieldBool(Ent, "bFloatBob", Values[1] & 0x00100000);
				SetClassFieldBool(Ent, "bActivateImpact", Values[1] & 0x00200000);
				SetClassFieldBool(Ent, "bActivatePushWall", Values[1] & 0x00400000);
				SetClassFieldBool(Ent, "bActivateMCross", Values[1] & 0x00800000);
				SetClassFieldBool(Ent, "bActivatePCross", Values[1] & 0x01000000);
				SetClassFieldBool(Ent, "bCantLeaveFloorpic", Values[1] & 0x02000000);
				SetClassFieldBool(Ent, "bNonShootable", Values[1] & 0x04000000);
				SetClassFieldBool(Ent, "bInvulnerable", Values[1] & 0x08000000);
				SetClassFieldBool(Ent, "bDormant", Values[1] & 0x10000000);
				SetClassFieldBool(Ent, "bSeekerMissile", Values[1] & 0x40000000);
				SetClassFieldBool(Ent, "bReflective", Values[1] & 0x80000000);
				//	Things that used to be flags before.
				if (Values[1] & 0x00000001)
					SetClassFieldFloat(Ent, "Gravity", 0.125);
				if (Values[1] & 0x00010000)
					SetClassFieldName(Ent, "DamageType", "Fire");
				else if (Values[1] & 0x20000000)
					SetClassFieldName(Ent, "DamageType", "Ice");
				if (Values[1] & 0x00000004)
					SetClassFieldByte(Ent, "BounceType", 1);
			}
		}
		//
		//	States
		//
		else if (!VStr::ICmp(String, "Initial frame"))
		{
			DoThingState(Ent, "Spawn");
		}
		else if (!VStr::ICmp(String, "First moving frame"))
		{
			DoThingState(Ent, "See");
		}
		else if (!VStr::ICmp(String, "Close attack frame"))
		{
			//	Don't change melee state for players
			if (num != 1)
			{
				DoThingState(Ent, "Melee");
			}
		}
		else if (!VStr::ICmp(String, "Far attack frame"))
		{
			//	Don't change missile state for players
			if (num != 1)
			{
				DoThingState(Ent, "Missile");
			}
		}
		else if (!VStr::ICmp(String, "Injury frame"))
		{
			DoThingState(Ent, "Pain");
		}
		else if (!VStr::ICmp(String, "Death frame"))
		{
			DoThingState(Ent, "Death");
		}
		else if (!VStr::ICmp(String, "Exploding frame"))
		{
			DoThingState(Ent, "XDeath");
		}
		else if (!VStr::ICmp(String, "Respawn frame"))
		{
			DoThingState(Ent, "Raise");
		}
		//
		//	Sounds
		//
		else if (!VStr::ICmp(String, "Alert sound"))
		{
			DoThingSound(Ent, "SightSound");
		}
		else if (!VStr::ICmp(String, "Action sound"))
		{
			DoThingSound(Ent, "ActiveSound");
		}
		else if (!VStr::ICmp(String, "Attack sound"))
		{
			DoThingSound(Ent, "AttackSound");
		}
		else if (!VStr::ICmp(String, "Pain sound"))
		{
			DoThingSound(Ent, "PainSound");
		}
		else if (!VStr::ICmp(String, "Death sound"))
		{
			DoThingSound(Ent, "DeathSound");
		}
		else
		{
			GCon->Logf("WARNING! Invalid mobj param %s", String);
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
		else GCon->Logf("WARNING! Invalid sound param %s", String);
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
		GCon->Logf("WARNING! Invalid state num %d", num);
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
				GCon->Logf("WARNING! Bad sprite index %d", value);
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
				GCon->Logf("WARNING! Invalid next state %d", value);
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
			GCon->Logf("WARNING! Tried to set action pointer.");
		}
		else
		{
			GCon->Logf("WARNING! Invalid state param %s", String);
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
		else GCon->Logf("WARNING! Invalid sprite name param %s", String);
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
		GCon->Logf("WARNING! Invalid ammo num %d", num);
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
			GCon->Logf("WARNING! Invalid ammo param %s", String);
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
//	DoWeaponState
//
//==========================================================================

static void DoWeaponState(VClass* Weapon, const char* StateLabel)
{
	guard(DoWeaponState);
	if (value < 0 || value >= States.Num())
	{
		GCon->Logf("WARNING! Invalid weapon state %d", value);
	}
	else
	{
		Weapon->SetStateLabel(StateLabel, States[value]);
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
		GCon->Logf("WARNING! Invalid weapon num %d", num);
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
		else if (!VStr::ICmp(String, "Ammo use") ||
			!VStr::ICmp(String, "Ammo per shot"))
		{
			SetClassFieldInt(Weapon, "AmmoUse1", value);
		}
		else if (!VStr::ICmp(String, "Deselect frame"))
		{
			DoWeaponState(Weapon, "Select");
		}
		else if (!VStr::ICmp(String, "Select frame"))
		{
			DoWeaponState(Weapon, "Deselect");
		}
		else if (!VStr::ICmp(String, "Bobbing frame"))
		{
			DoWeaponState(Weapon, "Ready");
		}
		else if (!VStr::ICmp(String, "Shooting frame"))
		{
			DoWeaponState(Weapon, "Fire");
		}
		else if (!VStr::ICmp(String, "Firing frame"))
		{
			DoWeaponState(Weapon, "Flash");
		}
		else
		{
			GCon->Logf("WARNING! Invalid weapon param %s", String);
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
		GCon->Logf("WARNING! Invalid pointer");
		while (ParseParam());
		return;
	}

	while (ParseParam())
	{
		if (!VStr::ICmp(String, "Codep Frame"))
		{
			if (value < 0 || value >= States.Num())
			{
				GCon->Logf("WARNING! Invalid source state %d", value);
			}
			else
			{
				CodePtrStates[num]->Function = StateActions[value];
			}
		}
		else
		{
			GCon->Logf("WARNING! Invalid pointer param %s", String);
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
				GCon->Logf("Bad frame index %d", Index);
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
				GCon->Logf("WARNING! Invalid code pointer %s", ValueString);
			}
		}
		else
		{
			GCon->Logf("WARNING! Invalid code pointer param %s", String);
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
//	DoPowerupColour
//
//==========================================================================

static void DoPowerupColour(const char* ClassName)
{
	guard(DoPowerupColour);
	VClass* Power = VClass::FindClass(ClassName);
	if (!Power)
	{
		GCon->Logf("Can't find powerup class %s", ClassName);
		return;
	}

	int r, g, b;
	float a;
	if (sscanf(ValueString, "%d %d %d %f", &r, &g, &b, &a) != 4)
	{
		GCon->Logf("Bad powerup colour %s", ValueString);
		return;
	}
	r = MID(0, r, 255);
	g = MID(0, g, 255);
	b = MID(0, b, 255);
	a = MID(0.0, a, 1.0);
	SetClassFieldInt(Power, "BlendColour", (r << 16) | (g << 8) | b |
		int(a * 255) << 24);
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
			TArray<VDropItemInfo>& List = *(TArray<VDropItemInfo>*)(
				DoomPlayerClass->Defaults +
				DoomPlayerClass->FindFieldChecked("DropItemList")->Ofs);
			for (int i = 0; i < List.Num(); i++)
			{
				if (List[i].Type && List[i].Type->Name == "Clip")
				{
					List[i].Amount = value;
				}
			}
		}
		else if (!VStr::ICmp(String, "Max Health"))
		{
			SetClassFieldInt(HealthBonusClass, "MaxAmount", 2 * value);
		}
		else if (!VStr::ICmp(String, "Max Armor"))
		{
			SetClassFieldInt(ArmorBonusClass, "MaxSaveAmount", value);
		}
		else if (!VStr::ICmp(String, "Green Armor Class"))
		{
			SetClassFieldInt(GreenArmorClass, "SaveAmount", 100 * value);
			SetClassFieldFloat(GreenArmorClass, "SavePercent", value == 1 ? 1.0 / 3.0 : 1.0 / 2.0);
		}
		else if (!VStr::ICmp(String, "Blue Armor Class"))
		{
			SetClassFieldInt(BlueArmorClass, "SaveAmount", 100 * value);
			SetClassFieldFloat(BlueArmorClass, "SavePercent", value == 1 ? 1.0 / 3.0 : 1.0 / 2.0);
		}
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
		else if (!VStr::ICmp(String, "Powerup Color Invulnerability"))
		{
			DoPowerupColour("PowerInvulnerable");
		}
		else if (!VStr::ICmp(String, "Powerup Color Berserk"))
		{
			DoPowerupColour("PowerStrength");
		}
		else if (!VStr::ICmp(String, "Powerup Color Invisibility"))
		{
			DoPowerupColour("PowerInvisibility");
		}
		else if (!VStr::ICmp(String, "Powerup Color Radiation Suit"))
		{
			DoPowerupColour("PowerIronFeet");
		}
		else if (!VStr::ICmp(String, "Powerup Color Infrared"))
		{
			DoPowerupColour("PowerLightAmp");
		}
		else if (!VStr::ICmp(String, "Powerup Color Tome of Power"))
		{
			DoPowerupColour("PowerWeaponLevel2");
		}
		else if (!VStr::ICmp(String, "Powerup Color Wings of Wrath"))
		{
			DoPowerupColour("PowerFlight");
		}
		else if (!VStr::ICmp(String, "Powerup Color Speed"))
		{
			DoPowerupColour("PowerSpeed");
		}
		else if (!VStr::ICmp(String, "Powerup Color Minotaur"))
		{
			DoPowerupColour("PowerMinotaur");
		}
		else GCon->Logf("WARNING! Invalid misc %s", String);
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
			GCon->Logf("WARNING! Unknown key in Pars section.");
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
			GCon->Logf("WARNING! Bad par time");
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

	GCon->Logf("Not found old \"%s\" new \"%s\"", oldStr, newStr);
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

	if (!VStr::NCmp(Patch, "Patch File for DeHackEd v", 25))
	{
		if (Patch[25] < '3')
		{
			delete[] Patch;
			GCon->Logf("DeHackEd patch is too old");
			return;
		}
		GetLine();
		int DVer = -1;
		int PFmt = -1;
		while (ParseParam())
		{
			if (!VStr::ICmp(String, "Doom version"))
			{
				DVer = value;
			}
			else if (!VStr::ICmp(String, "Patch format"))
			{
				PFmt = value;
			}
			else GCon->Logf("Unknown parameter %s", String);
		}
		if (!String || DVer == -1 || PFmt == -1)
		{
			delete[] Patch;
			GCon->Logf("Not a DeHackEd patch file");
			return;
		}
	}
	else
	{
		GCon->Logf("Missing DeHackEd header, assuming BEX file");
		GetLine();
	}

	while (String)
	{
		char* Section = strtok(String, " ");
		if (!Section)
		{
			GetLine();
			continue;
		}

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
			GCon->Logf("Don't know how to handle \"%s\"", String);
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
	VState** StatesTail = &VClass::FindClass("Entity")->NetStates;
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
		else
		{
			sc->ExpectString();
			VStateLabel* Lbl = StatesClass->FindStateLabel(*sc->String);
			if (!Lbl)
			{
				sc->Error(va("No such state %s in class %s",
					*sc->String, StatesClass->GetName()));
			}
			S = Lbl->State;
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
		VMethod* Method = Class->FindMethod(*MethodName);
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
	DoomPlayerClass = VClass::FindClass("DoomPlayer");
	BfgClass = VClass::FindClass("BFG9000");
	HealthBonusClass = VClass::FindClass("HealthBonus");
	SoulsphereClass = VClass::FindClass("Soulsphere");
	MegaHealthClass = VClass::FindClass("MegasphereHealth");
	GreenArmorClass = VClass::FindClass("GreenArmor");
	BlueArmorClass = VClass::FindClass("BlueArmor");
	ArmorBonusClass = VClass::FindClass("ArmorBonus");

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
	AmmoClasses.Clear();
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
