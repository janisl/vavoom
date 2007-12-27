//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: dehacked.cpp 2903 2007-11-26 23:46:26Z dj_jl $
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

enum
{
	OLDDEC_Decoration,
	OLDDEC_Breakable,
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VPackage*		DecPkg;
static VClass*			ActorClass;
static VMethod*			FuncA_Scream;
static VMethod*			FuncA_NoBlocking;
static VMethod*			FuncA_ScreamAndUnblock;
static VMethod*			FuncA_ActiveSound;
static VMethod*			FuncA_ActiveAndUnblock;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	GetClassFieldFloat
//
//==========================================================================

static float GetClassFieldFloat(VClass* Class, const char* FieldName)
{
	guard(GetClassFieldFloat);
	VField* F = Class->FindFieldChecked(FieldName);
	float* Ptr = (float*)(Class->Defaults + F->Ofs);
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
//	SetClassFieldName
//
//==========================================================================

static void SetClassFieldName(VClass* Class, const char* FieldName,
	VName Value)
{
	guard(SetClassFieldName);
	VField* F = Class->FindFieldChecked(FieldName);
	VName* Ptr = (VName*)(Class->Defaults + F->Ofs);
	*Ptr = Value;
	unguard;
}

//==========================================================================
//
//	SkipBlock
//
//==========================================================================

static void SkipBlock(VScriptParser* sc, int Level)
{
	while (!sc->AtEnd() && Level > 0)
	{
		if (sc->Check("{"))
		{
			Level++;
		}
		else if (sc->Check("}"))
		{
			Level--;
		}
		else
		{
			sc->GetString();
		}
	}
}

//==========================================================================
//
//	ParseConst
//
//==========================================================================

static void ParseConst(VScriptParser* sc)
{
	guard(ParseConst);
	sc->SetCMode(true);
	sc->Expect("int");
	sc->ExpectString();
	VStr Name = sc->String;
	sc->Expect("=");
	sc->ExpectString();
	VStr Val = sc->String;
	sc->Expect(";");
	GCon->Logf("Constant %s with value %s", *Name, *Val);
	sc->SetCMode(false);
	unguard;
}

//==========================================================================
//
//	ParseClass
//
//==========================================================================

static void ParseClass(VScriptParser* sc)
{
	guard(ParseClass);
	sc->ExpectString();
	GCon->Logf("Class %s", *sc->String);
	sc->Expect("extends");
	sc->ExpectString();
	sc->Expect("native");
	sc->Expect("{");
	SkipBlock(sc, 1);
	unguard;
}

//==========================================================================
//
//	ParseEnum
//
//==========================================================================

static void ParseEnum(VScriptParser* sc)
{
	guard(ParseEnum);
	GCon->Logf("Enum");
	sc->Expect("{");
	SkipBlock(sc, 1);
	unguard;
}

//==========================================================================
//
//	ParseActor
//
//==========================================================================

static void ParseActor(VScriptParser* sc)
{
	guard(ParseActor);
	sc->ExpectString();
	GCon->Logf("Actor %s", *sc->String);
	sc->SetCMode(true);
	sc->SetEscape(false);
	if (sc->Check(":"))
	{
		sc->ExpectString();
	}
	if (sc->Check("replaces"))
	{
		sc->ExpectString();
	}
	sc->CheckNumber();
	while (!sc->Check("{"))
	{
		sc->ExpectString();
		GCon->Logf("Unknown token %s", *sc->String);
	}
	SkipBlock(sc, 1);
	sc->SetEscape(true);
	sc->SetCMode(false);
	unguard;
}

//==========================================================================
//
//	ParsePickup
//
//==========================================================================

static void ParsePickup(VScriptParser* sc)
{
	guard(ParsePickup);
	sc->ExpectString();
	GCon->Logf("Pickup %s", *sc->String);
	sc->Expect("{");
	SkipBlock(sc, 1);
	unguard;
}

//==========================================================================
//
//	ParseProjectile
//
//==========================================================================

static void ParseProjectile(VScriptParser* sc)
{
	guard(ParseProjectile);
	sc->ExpectString();
	GCon->Logf("Projectile %s", *sc->String);
	sc->Expect("{");
	SkipBlock(sc, 1);
	unguard;
}

//==========================================================================
//
//	ParseOldDecStates
//
//==========================================================================

static void ParseOldDecStates(VScriptParser* sc, TArray<VState*>& States,
	VClass* Class)
{
	guard(ParseOldDecStates);
	TArray<VStr> Tokens;
	sc->String.Split(",\t\r\n", Tokens);
	for (int TokIdx = 0; TokIdx < Tokens.Num(); TokIdx++)
	{
		const char* pFrame = *Tokens[TokIdx];
		int DurColon = Tokens[TokIdx].IndexOf(':');
		float Duration = 4;
		if (DurColon >= 0)
		{
			Duration = atoi(pFrame);
			pFrame = *Tokens[TokIdx] + DurColon + 1;
		}

		bool GotState = false;
		while (*pFrame)
		{
			if (*pFrame == ' ')
			{
			}
			else if (*pFrame == '*')
			{
				if (!GotState)
				{
					sc->Error("* must come after a frame");
				}
				States[States.Num() - 1]->Frame |= FF_FULLBRIGHT;
			}
			else if (*pFrame < 'A' || *pFrame > ']')
			{
				sc->Error("Frames must be A-Z, [, \\, or ]");
			}
			else
			{
				GotState = true;
				VState* State = new VState(va("S_%d", States.Num()));
				States.Append(State);
				State->Outer = Class;
				State->InClassIndex = States.Num() - 1;
				State->Frame = *pFrame - 'A';
				State->Time = Duration >= 0 ? float(Duration) / 35.0 : -1.0;
			}
			pFrame++;
		}
	}
	unguard;
}

//==========================================================================
//
//	ParseOldDecoration
//
//==========================================================================

static void ParseOldDecoration(VScriptParser* sc, int Type)
{
	guard(ParseOldDecoration);
	//	Get name of the class.
	sc->ExpectString();
	VName ClassName = *sc->String;
	if (Type == OLDDEC_Breakable)
	{
		GCon->Logf("Breakable %s", *ClassName);
	}

	//	Create class.
	VClass* Class = ActorClass->CreateDerivedClass(ClassName);
	Class->Outer = DecPkg;
	SetClassFieldBool(Class, "bNoPassMobj", true);
	if (Type == OLDDEC_Breakable)
	{
		SetClassFieldBool(Class, "bShootable", true);
	}

	//	Parse game filters.
	int GameFilter = 0;
	while (!sc->Check("{"))
	{
		if (sc->Check("Doom"))
		{
			GameFilter |= GAME_Doom;
		}
		else if (sc->Check("Heretic"))
		{
			GameFilter |= GAME_Heretic;
		}
		else if (sc->Check("Hexen"))
		{
			GameFilter |= GAME_Hexen;
		}
		else if (sc->Check("Strife"))
		{
			GameFilter |= GAME_Strife;
		}
		else if (sc->Check("Raven"))
		{
			GameFilter |= GAME_Raven;
		}
		else if (sc->Check("Any"))
		{
			GameFilter |= GAME_Any;
		}
		else if (GameFilter)
		{
			sc->Error("Unknown game filter");
		}
		else
		{
			sc->Error("Unknown identifier");
		}
	}

	int DoomEdNum = -1;
	int SpawnNum = -1;
	VName Sprite("tnt1");
	VName DeathSprite(NAME_None);
	TArray<VState*> States;
	int SpawnStart = 0;
	int SpawnEnd = 0;
	int DeathStart = 0;
	int DeathEnd = 0;
	bool DiesAway = false;
	bool SolidOnDeath = false;
	float DeathHeight = 0.0;
	int BurnStart = 0;
	int BurnEnd = 0;
	bool BurnsAway = false;
	bool SolidOnBurn = false;
	float BurnHeight = 0.0;
	int IceStart = 0;
	int IceEnd = 0;
	bool GenericIceDeath = false;

	while (!sc->Check("}"))
	{
		if (sc->Check("DoomEdNum"))
		{
			sc->ExpectNumber();
			if (sc->Number < -1 || sc->Number > 32767)
			{
				sc->Error("DoomEdNum is out of range [-1, 32767]");
			}
			DoomEdNum = sc->Number;
		}
		else if (sc->Check("SpawnNum"))
		{
			sc->ExpectNumber();
			if (sc->Number < 0 || sc->Number > 255)
			{
				sc->Error("SpawnNum is out of range [0, 255]");
			}
			SpawnNum = sc->Number;
		}

		//	Spawn state
		else if (sc->Check("Sprite"))
		{
			sc->ExpectString();
			if (sc->String.Length() != 4)
			{
				sc->Error("Sprite name must be 4 characters long");
			}
			Sprite = *sc->String.ToLower();
		}
		else if (sc->Check("Frames"))
		{
			sc->ExpectString();
			SpawnStart = States.Num();
			ParseOldDecStates(sc, States, Class);
			SpawnEnd = States.Num();
		}

		//	Death states
		else if (Type == OLDDEC_Breakable && sc->Check("DeathSprite"))
		{
			sc->ExpectString();
			if (sc->String.Length() != 4)
			{
				sc->Error("Sprite name must be 4 characters long");
			}
			DeathSprite = *sc->String.ToLower();
		}
		else if (Type == OLDDEC_Breakable && sc->Check("DeathFrames"))
		{
			sc->ExpectString();
			DeathStart = States.Num();
			ParseOldDecStates(sc, States, Class);
			DeathEnd = States.Num();
		}
		else if (Type == OLDDEC_Breakable && sc->Check("DiesAway"))
		{
			DiesAway = true;
		}
		else if (Type == OLDDEC_Breakable && sc->Check("BurnDeathFrames"))
		{
			sc->ExpectString();
			BurnStart = States.Num();
			ParseOldDecStates(sc, States, Class);
			BurnEnd = States.Num();
		}
		else if (Type == OLDDEC_Breakable && sc->Check("BurnsAway"))
		{
			BurnsAway = true;
		}
		else if (Type == OLDDEC_Breakable && sc->Check("IceDeathFrames"))
		{
			sc->ExpectString();
			IceStart = States.Num();
			ParseOldDecStates(sc, States, Class);

			//	Make a copy of the last state for A_FreezeDeathChunks
			VState* State = new VState(va("S_%d", States.Num()));
			States.Append(State);
			State->Outer = Class;
			State->InClassIndex = States.Num() - 1;
			State->Frame = States[States.Num() - 2]->Frame;

			IceEnd = States.Num();
		}
		else if (Type == OLDDEC_Breakable && sc->Check("GenericIceDeath"))
		{
			GenericIceDeath = true;
		}

		//	Misc properties
		else if (sc->Check("Radius"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Radius", sc->Float);
		}
		else if (sc->Check("Height"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Height", sc->Float);
		}
		else if (sc->Check("Mass"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Mass", sc->Float);
		}
		else if (sc->Check("Scale"))
		{
			sc->ExpectFloat();
			GCon->Logf("Decorate property Scale is not yet supported");
		}
		else if (sc->Check("Alpha"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Alpha", sc->Float);
		}
		else if (sc->Check("RenderStyle"))
		{
			int RenderStyle = 0;
			if (sc->Check("STYLE_None"))
			{
				RenderStyle = STYLE_None;
			}
			else if (sc->Check("STYLE_Normal"))
			{
				RenderStyle = STYLE_Normal;
			}
			else if (sc->Check("STYLE_Fuzzy"))
			{
				RenderStyle = STYLE_Fuzzy;
			}
			else if (sc->Check("STYLE_SoulTrans"))
			{
				RenderStyle = STYLE_SoulTrans;
			}
			else if (sc->Check("STYLE_OptFuzzy"))
			{
				RenderStyle = STYLE_OptFuzzy;
			}
			else if (sc->Check("STYLE_Translucent"))
			{
				RenderStyle = STYLE_Translucent;
			}
			else if (sc->Check("STYLE_Add"))
			{
				RenderStyle = STYLE_Add;
			}
			else
			{
				sc->Error("Bad render style");
			}
			SetClassFieldByte(Class, "RenderStyle", RenderStyle);
		}
		else if (sc->Check("Translation1"))
		{
			sc->ExpectNumber();
			if (sc->Number < 0 || sc->Number > 2)
			{
				sc->Error("Translation1 is out of range [0, 2]");
			}
			SetClassFieldInt(Class, "Translation", sc->Number);
		}
		else if (sc->Check("Translation2"))
		{
			sc->ExpectNumber();
			if (sc->Number < 0 || sc->Number > 255)
			{
				sc->Error("Translation2 is out of range [0, 255]");
			}
			GCon->Logf("Translatioin2 is not yet supported");
		}

		//	Breakable decoration properties.
		else if (Type == OLDDEC_Breakable && sc->Check("Health"))
		{
			sc->ExpectNumber();
			SetClassFieldInt(Class, "Health", sc->Number);
		}
		else if (Type == OLDDEC_Breakable && sc->Check("DeathHeight"))
		{
			sc->ExpectFloat();
			DeathHeight = sc->Float;
		}
		else if (Type == OLDDEC_Breakable && sc->Check("BurnHeight"))
		{
			sc->ExpectFloat();
			BurnHeight = sc->Float;
		}
		else if (Type == OLDDEC_Breakable && sc->Check("SolidOnDeath"))
		{
			SolidOnDeath = true;
		}
		else if (Type == OLDDEC_Breakable && sc->Check("SolidOnBurn"))
		{
			SolidOnBurn = true;
		}
		else if (Type == OLDDEC_Breakable && sc->Check("DeathSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "DeathSound", *sc->String);
		}
		else if (Type == OLDDEC_Breakable && sc->Check("BurnDeathSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "ActiveSound", *sc->String);
		}

		//	Compatibility flags
		else if (sc->Check("LowGravity"))
		{
			SetClassFieldFloat(Class, "Gravity", 0.125);
		}
		else if (sc->Check("FireDamage"))
		{
			SetClassFieldName(Class, "DamageType", "Fire");
		}

		//	Flags
		else if (sc->Check("Solid"))
		{
			SetClassFieldBool(Class, "bSolid", true);
		}
		else if (sc->Check("NoSector"))
		{
			SetClassFieldBool(Class, "bHidden", true);
		}
		else if (sc->Check("NoBlockmap"))
		{
			SetClassFieldBool(Class, "bNoBlockmap", true);
		}
		else if (sc->Check("SpawnCeiling"))
		{
			SetClassFieldBool(Class, "bSpawnCeiling", true);
		}
		else if (sc->Check("NoGravity"))
		{
			SetClassFieldBool(Class, "bNoGravity", true);
		}
		else if (sc->Check("Shadow"))
		{
			GCon->Logf("Shadow flag is not currently supported");
		}
		else if (sc->Check("NoBlood"))
		{
			SetClassFieldBool(Class, "bNoBlood", true);
		}
		else if (sc->Check("CountItem"))
		{
			SetClassFieldBool(Class, "bCountItem", true);
		}
		else if (sc->Check("WindThrust"))
		{
			SetClassFieldBool(Class, "bWindThrust", true);
		}
		else if (sc->Check("FloorClip"))
		{
			SetClassFieldBool(Class, "bFloorClip", true);
		}
		else if (sc->Check("SpawnFloat"))
		{
			SetClassFieldBool(Class, "bSpawnFloat", true);
		}
		else if (sc->Check("NoTeleport"))
		{
			SetClassFieldBool(Class, "bNoTeleport", true);
		}
		else if (sc->Check("Ripper"))
		{
			SetClassFieldBool(Class, "bRip", true);
		}
		else if (sc->Check("Pushable"))
		{
			SetClassFieldBool(Class, "bPushable", true);
		}
		else if (sc->Check("SlidesOnWalls"))
		{
			SetClassFieldBool(Class, "bSlide", true);
		}
		else if (sc->Check("CanPass"))
		{
			SetClassFieldBool(Class, "bNoPassMobj", false);
		}
		else if (sc->Check("CannotPush"))
		{
			SetClassFieldBool(Class, "bCannotPush", true);
		}
		else if (sc->Check("ThruGhost"))
		{
			SetClassFieldBool(Class, "bThruGhost", true);
		}
		else if (sc->Check("NoDamageThrust"))
		{
			SetClassFieldBool(Class, "bNoDamageThrust", true);
		}
		else if (sc->Check("Telestomp"))
		{
			SetClassFieldBool(Class, "bTelestomp", true);
		}
		else if (sc->Check("FloatBob"))
		{
			SetClassFieldBool(Class, "bFloatBob", true);
		}
		else if (sc->Check("ActivateImpact"))
		{
			SetClassFieldBool(Class, "bActivateImpact", true);
		}
		else if (sc->Check("CanPushWalls"))
		{
			SetClassFieldBool(Class, "bActivatePushWall", true);
		}
		else if (sc->Check("ActivateMCross"))
		{
			SetClassFieldBool(Class, "bActivateMCross", true);
		}
		else if (sc->Check("ActivatePCross"))
		{
			SetClassFieldBool(Class, "bActivatePCross", true);
		}
		else if (sc->Check("Reflective"))
		{
			SetClassFieldBool(Class, "bReflective", true);
		}
		else if (sc->Check("FloorHugger"))
		{
			SetClassFieldBool(Class, "bIgnoreFloorStep", true);
		}
		else if (sc->Check("CeilingHugger"))
		{
			SetClassFieldBool(Class, "bIgnoreCeilingStep", true);
		}
		else if (sc->Check("DontSplash"))
		{
			SetClassFieldBool(Class, "bNoSplash", true);
		}
		else
		{
			Sys_Error("Unknown property %s", *sc->String);
		}
	}

	if (SpawnEnd == 0)
	{
		sc->Error(va("%s has no Frames definition", *ClassName));
	}
	if (Type == OLDDEC_Breakable && DeathEnd == 0)
	{
		sc->Error(va("%s has no DeathFrames definition", *ClassName));
	}
	if (GenericIceDeath && IceEnd != 0)
	{
		sc->Error("IceDeathFrames and GenericIceDeath are mutually exclusive");
	}

	if (DoomEdNum > 0)
	{
		mobjinfo_t& MI = VClass::GMobjInfos.Alloc();
		MI.class_id = Class;
		MI.doomednum = DoomEdNum;
		MI.GameFilter = GameFilter;
	}
	if (SpawnNum > 0)
	{
		mobjinfo_t& SI = VClass::GScriptIds.Alloc();
		SI.class_id = Class;
		SI.doomednum = SpawnNum;
		SI.GameFilter = GameFilter;
	}

	//	Set up linked list of states.
	Class->States = States[0];
	Class->NetStates = States[0];
	for (int i = 0; i < States.Num() - 1; i++)
	{
		States[i]->Next = States[i + 1];
		States[i]->NetNext = States[i + 1];
	}

	//	Set up default sprite for all states.
	for (int i = 0; i < States.Num(); i++)
	{
		States[i]->SpriteName = Sprite;
		States[i]->SpriteIndex = VClass::FindSprite(Sprite);
	}
	//	Set death sprite if it's defined.
	if (DeathSprite != NAME_None && DeathEnd != 0)
	{
		for (int i = DeathStart; i < DeathEnd; i++)
		{
			States[i]->SpriteName = DeathSprite;
			States[i]->SpriteIndex = VClass::FindSprite(DeathSprite);
		}
	}

	//	Set up links of spawn states.
	if (SpawnEnd - SpawnStart == 1)
	{
		States[SpawnStart]->Time = -1.0;
	}
	else
	{
		for (int i = SpawnStart; i < SpawnEnd - 1; i++)
		{
			States[i]->NextState = States[i + 1];
		}
		States[SpawnEnd - 1]->NextState = States[SpawnStart];
	}
	Class->SetStateLabel("Spawn", States[SpawnStart]);

	//	Set up links of death states.
	if (DeathEnd != 0)
	{
		for (int i = DeathStart; i < DeathEnd - 1; i++)
		{
			States[i]->NextState = States[i + 1];
		}
		if (!DiesAway)
		{
			States[DeathEnd - 1]->Time = -1.0;
		}
		//	First death state plays death sound, second makes it
		// non-blocking unless it should stay solid.
		States[DeathStart]->Function = FuncA_Scream;
		if (!SolidOnDeath)
		{
			if (DeathEnd - DeathStart > 1)
			{
				States[DeathStart + 1]->Function = FuncA_NoBlocking;
			}
			else
			{
				States[DeathStart]->Function = FuncA_ScreamAndUnblock;
			}
		}

		if (!DeathHeight)
		{
			DeathHeight = GetClassFieldFloat(Class, "Height");
		}
		SetClassFieldFloat(Class, "DeathHeight", DeathHeight);

		Class->SetStateLabel("Death", States[DeathStart]);
	}

	//	Set up links of burn death states.
	if (BurnEnd != 0)
	{
		for (int i = BurnStart; i < BurnEnd - 1; i++)
		{
			States[i]->NextState = States[i + 1];
		}
		if (!BurnsAway)
		{
			States[BurnEnd - 1]->Time = -1.0;
		}
		//	First death state plays active sound, second makes it
		// non-blocking unless it should stay solid.
		States[BurnStart]->Function = FuncA_ActiveSound;
		if (!SolidOnBurn)
		{
			if (BurnEnd - BurnStart > 1)
			{
				States[BurnStart + 1]->Function = FuncA_NoBlocking;
			}
			else
			{
				States[BurnStart]->Function = FuncA_ActiveAndUnblock;
			}
		}

		if (!BurnHeight)
		{
			BurnHeight = GetClassFieldFloat(Class, "Height");
		}
		SetClassFieldFloat(Class, "BurnHeight", BurnHeight);

		Class->SetStateLabel("Burn", States[BurnStart]);
	}

	//	Set up links of ice death states.
	if (IceEnd != 0)
	{
		for (int i = IceStart; i < IceEnd - 1; i++)
		{
			States[i]->NextState = States[i + 1];
		}

		States[IceEnd - 2]->Time = 5.0 / 35.0;
		//States[IceEnd - 2]->Function = FuncA_FreezeDeath;

		States[IceEnd - 1]->NextState = States[IceEnd - 1];
		States[IceEnd - 1]->Time = 1.0 / 35.0;
		//States[IceEnd - 2]->Function = FuncA_FreezeDeathChunks;

		Class->SetStateLabel("Ice", States[IceStart]);
	}
	else if (GenericIceDeath)
	{
		Class->SetStateLabel("Ice", Class->FindStateLabel("GenericIceDeath"));
	}
	unguard;
}

//==========================================================================
//
//	ParseDecorate
//
//==========================================================================

static void ParseDecorate(VScriptParser* sc)
{
	guard(ParseDecorate);
	while (!sc->AtEnd())
	{
		if (sc->Check("#include"))
		{
			sc->ExpectString();
			int Lump = W_CheckNumForFileName(sc->String);
			//	Check WAD lump only if it's no longer than 8 characters and
			// has no path separator.
			if (Lump < 0 && sc->String.Length() <= 8 &&
				sc->String.IndexOf('/') < 0)
			{
				Lump = W_CheckNumForName(VName(*sc->String, VName::AddLower8));
			}
			if (Lump < 0)
			{
				sc->Error(va("Lump %s not found", *sc->String));
			}
			ParseDecorate(new VScriptParser(sc->String,
				W_CreateLumpReaderNum(Lump)));
		}
		else if (sc->Check("const"))
		{
			ParseConst(sc);
		}
		else if (sc->Check("enum"))
		{
			ParseEnum(sc);
		}
		else if (sc->Check("class"))
		{
			ParseClass(sc);
		}
		else if (sc->Check("actor"))
		{
			ParseActor(sc);
		}
		else if (sc->Check("breakable"))
		{
			ParseOldDecoration(sc, OLDDEC_Breakable);
		}
		else if (sc->Check("pickup"))
		{
			ParsePickup(sc);
		}
		else if (sc->Check("projectile"))
		{
			ParseProjectile(sc);
		}
		else
		{
			ParseOldDecoration(sc, OLDDEC_Decoration);
		}
	}
	delete sc;
	unguard;
}

//==========================================================================
//
//	ProcessDecorateScripts
//
//==========================================================================

void ProcessDecorateScripts()
{
	guard(ProcessDecorateScripts);
	DecPkg = new VPackage(NAME_decorate);
	ActorClass = VClass::FindClass("Actor");
	FuncA_Scream = ActorClass->FindFunctionChecked("A_Scream");
	FuncA_NoBlocking = ActorClass->FindFunctionChecked("A_NoBlocking");
	FuncA_ScreamAndUnblock = ActorClass->FindFunctionChecked("A_ScreamAndUnblock");
	FuncA_ActiveSound = ActorClass->FindFunctionChecked("A_ActiveSound");
	FuncA_ActiveAndUnblock = ActorClass->FindFunctionChecked("A_ActiveAndUnblock");

	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) == NAME_decorate)
		{
			ParseDecorate(new VScriptParser(*W_LumpName(Lump),
				W_CreateLumpReaderNum(Lump)));
		}
	}

	VClass::StaticReinitStatesLookup();
	unguard;
}
