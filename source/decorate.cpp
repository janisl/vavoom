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
	OLDDEC_Projectile,
	OLDDEC_Pickup,
};

enum
{
	BOUNCE_None,
	BOUNCE_Doom,
	BOUNCE_Heretic,
	BOUNCE_Hexen
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VPackage*		DecPkg;
static VClass*			ActorClass;
static VClass*			ScriptedEntityClass;
static VClass*			FakeInventoryClass;
static VMethod*			FuncA_Scream;
static VMethod*			FuncA_NoBlocking;
static VMethod*			FuncA_ScreamAndUnblock;
static VMethod*			FuncA_ActiveSound;
static VMethod*			FuncA_ActiveAndUnblock;
static VMethod*			FuncA_ExplodeParms;

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
//	SetClassFieldStr
//
//==========================================================================

static void SetClassFieldStr(VClass* Class, const char* FieldName,
	const VStr& Value)
{
	guard(SetClassFieldStr);
	VField* F = Class->FindFieldChecked(FieldName);
	VStr* Ptr = (VStr*)(Class->Defaults + F->Ofs);
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
//	ParseFlag
//
//==========================================================================

static bool ParseFlag(VScriptParser* sc, VClass* Class, bool Value)
{
	guard(ParseFlag);
	sc->ExpectIdentifier();
	VStr Flag = sc->String;
	while (sc->Check("."))
	{
		sc->ExpectIdentifier();
		Flag += ".";
		Flag += sc->String;
	}
	if (!Flag.ICmp("NoGravity"))
	{
		SetClassFieldBool(Class, "bNoGravity", Value);
	}
	else if (!Flag.ICmp("FloorClip"))
	{
		SetClassFieldBool(Class, "bFloorClip", Value);
	}
	else if (!Flag.ICmp("NoBlockmap"))
	{
		SetClassFieldBool(Class, "bNoBlockmap", Value);
	}
	else if (!Flag.ICmp("Missile"))
	{
		SetClassFieldBool(Class, "bMissile", Value);
	}
	else if (!Flag.ICmp("DropOff"))
	{
		SetClassFieldBool(Class, "bDropOff", Value);
	}
	else if (!Flag.ICmp("DontSplash"))
	{
		SetClassFieldBool(Class, "bNoSplash", Value);
	}
	else if (!Flag.ICmp("CountItem"))
	{
		SetClassFieldBool(Class, "bCountItem", Value);
	}
	else if (!Flag.ICmp("SpawnCeiling"))
	{
		SetClassFieldBool(Class, "bSpawnCeiling", Value);
	}
	else if (!Flag.ICmp("Solid"))
	{
		SetClassFieldBool(Class, "bSolid", Value);
	}
	else if (!Flag.ICmp("FloatBob"))
	{
		SetClassFieldBool(Class, "bFloatBob", Value);
	}
	else if (!Flag.ICmp("NoTeleport"))
	{
		SetClassFieldBool(Class, "bNoTeleport", Value);
	}
	else if (!Flag.ICmp("CannotPush"))
	{
		SetClassFieldBool(Class, "bCannotPush", Value);
	}
	else if (!Flag.ICmp("SeekerMissile"))
	{
		SetClassFieldBool(Class, "bSeekerMissile", Value);
	}
	else if (!Flag.ICmp("NoBlood"))
	{
		SetClassFieldBool(Class, "bNoBlood", Value);
	}
	else if (!Flag.ICmp("NoRadiusDmg"))
	{
		SetClassFieldBool(Class, "bNoRadiusDamage", Value);
	}
	else if (!Flag.ICmp("Randomize"))
	{
		SetClassFieldBool(Class, "bRandomise", Value);
	}
	else if (!Flag.ICmp("Stealth"))
	{
		SetClassFieldBool(Class, "bStealth", Value);
	}
	else if (!Flag.ICmp("NoTarget"))
	{
		SetClassFieldBool(Class, "bNeverTarget", Value);
	}
	else if (!Flag.ICmp("DontMorph"))
	{
		SetClassFieldBool(Class, "bNoMorph", Value);
	}
	else if (!Flag.ICmp("NonShootable"))
	{
		SetClassFieldBool(Class, "bNonShootable", Value);
	}
	else if (!Flag.ICmp("Inventory.AutoActivate"))
	{
		SetClassFieldBool(Class, "bAutoActivate", Value);
	}
	else if (!Flag.ICmp("Inventory.AlwaysPickup"))
	{
		SetClassFieldBool(Class, "bAlwaysPickup", Value);
	}
	else if (!Flag.ICmp("Inventory.BigPowerup"))
	{
		SetClassFieldBool(Class, "bBigPowerup", Value);
	}

	else if (!Flag.ICmp("NoClip"))
	{
		SetClassFieldBool(Class, "bColideWithThings", !Value);
		SetClassFieldBool(Class, "bColideWithWorld", !Value);
	}
	else if (!Flag.ICmp("LowGravity"))
	{
		SetClassFieldFloat(Class, "Gravity", 0.125);
	}
	else if (!Flag.ICmp("DoomBounce"))
	{
		SetClassFieldByte(Class, "BounceType", Value ? BOUNCE_Doom : BOUNCE_None);
	}
	else if (!Flag.ICmp("HereticBounce"))
	{
		SetClassFieldByte(Class, "BounceType", Value ? BOUNCE_Heretic : BOUNCE_None);
	}
	else if (!Flag.ICmp("HexenBounce"))
	{
		SetClassFieldByte(Class, "BounceType", Value ? BOUNCE_Hexen : BOUNCE_None);
	}

	else if (!Flag.ICmp("NoLiftDrop"))
	{
		GCon->Logf("Unsupported flag NoLiftDrop");
	}
	else if (!Flag.ICmp("DontSquash"))
	{
		GCon->Logf("Unsupported flag DontSquash");
	}
	else if (!Flag.ICmp("NoTeleOther"))
	{
		GCon->Logf("Unsupported flag NoTeleOther");
	}
	else
	{
		sc->ExpectString();
		GCon->Logf("Unknown flag %s", *Flag);
		sc->SetEscape(false);
		SkipBlock(sc, 1);
		sc->SetEscape(true);
		sc->SetCMode(false);
		return false;
	}
	return true;
	unguard;
}

//==========================================================================
//
//	ParseStateString
//
//==========================================================================

static VStr ParseStateString(VScriptParser* sc)
{
	guard(ParseStateString);
	VStr		StateStr;

	sc->ExpectIdentifier();
	StateStr = sc->String;

	if (sc->Check("::"))
	{
		sc->ExpectIdentifier();
		StateStr += "::";
		StateStr += sc->String;
	}

	if (sc->Check("."))
	{
		sc->ExpectIdentifier();
		StateStr += ".";
		StateStr += sc->String;
	}

	return StateStr;
	unguard;
}

//==========================================================================
//
//	ResolveStateLabel
//
//==========================================================================

static VState* ResolveStateLabel(VScriptParser* sc, VClass* Class,
	VName LabelName, int Offset)
{
	guard(ResolveStateLabel);
	VClass* CheckClass = Class;
	VName CheckName = LabelName;

	const char* DCol = strstr(*LabelName, "::");
	if (DCol)
	{
		char ClassNameBuf[NAME_SIZE];
		strcpy(ClassNameBuf, *LabelName);
		ClassNameBuf[DCol - *LabelName] = 0;
		VName ClassName(ClassNameBuf);
		if (ClassName == "Super")
		{
			CheckClass = Class->GetSuperClass();
		}
		else
		{
			CheckClass = VClass::FindClass(*ClassName);
			if (!CheckClass)
			{
				sc->Error(va("No such class %s", *ClassName));
			}
		}
		CheckName = DCol + 2;
	}

	VState* State = CheckClass->FindStateLabelChecked(CheckName/*, false*/);
	int Count = Offset;
	while (Count--)
	{
		if (!State || !State->Next)
		{
			sc->Error("Bad jump offset");
		}
		State = State->Next;
	}
	return State;
	unguard;
}

//==========================================================================
//
//	ParseStates
//
//==========================================================================

static bool ParseStates(VScriptParser* sc, VClass* Class,
	TArray<VState*>& States)
{
	guard(ParseStates);
	VState* PrevState = NULL;
	VState* LoopStart = NULL;
	int NewLabelsStart = Class->StateLabels.Num();

	sc->Expect("{");
	//	Disable escape sequences in states.
	sc->SetEscape(false);
	while (!sc->Check("}"))
	{
		VStr TmpName = ParseStateString(sc);

		//	Goto command.
		if (!TmpName.ICmp("Goto"))
		{
			VName GotoLabel = *ParseStateString(sc);
			int GotoOffset = 0;
			if (sc->Check("+"))
			{
				sc->ExpectNumber();
				GotoOffset = sc->Number;
			}

			if (!PrevState && NewLabelsStart == Class->StateLabels.Num())
			{
				sc->Error("Goto before first state");
			}
			if (PrevState)
			{
				PrevState->GotoLabel = GotoLabel;
				PrevState->GotoOffset = GotoOffset;
			}
			for (int i = NewLabelsStart; i < Class->StateLabels.Num(); i++)
			{
				Class->StateLabels[i].GotoLabel = GotoLabel;
				Class->StateLabels[i].GotoOffset = GotoOffset;
			}
			NewLabelsStart = Class->StateLabels.Num();
			PrevState = NULL;
			continue;
		}

		//	Stop command.
		if (!TmpName.ICmp("Stop"))
		{
			if (!PrevState && NewLabelsStart == Class->StateLabels.Num())
			{
				sc->Error("Stop before first state");
				continue;
			}
			if (PrevState)
			{
				PrevState->NextState = NULL;
			}
			for (int i = NewLabelsStart; i < Class->StateLabels.Num(); i++)
			{
				Class->StateLabels[i].State = NULL;
			}
			NewLabelsStart = Class->StateLabels.Num();
			PrevState = NULL;
			continue;
		}

		//	Wait command.
		if (!TmpName.ICmp("Wait") || !TmpName.ICmp("Fail"))
		{
			if (!PrevState)
			{
				sc->Error(va("%s before first state", *TmpName));
				continue;
			}
			PrevState->NextState = PrevState;
			PrevState = NULL;
			continue;
		}

		//	Loop command.
		if (!TmpName.ICmp("Loop"))
		{
			if (!PrevState)
			{
				sc->Error("Loop before first state");
				continue;
			}
			PrevState->NextState = LoopStart;
			PrevState = NULL;
			continue;
		}

		//	Check for label.
		if (sc->Check(":"))
		{
			VStateLabel& Lbl = Class->StateLabels.Alloc();
			Lbl.Name = *TmpName;
			//FIXME add constructor.
			Lbl.State = NULL;
			Lbl.GotoLabel = NAME_None;
			Lbl.GotoOffset = 0;
			continue;
		}

		VState* State = new VState(va("S_%d", States.Num()));
		States.Append(State);
		State->Outer = Class;
		State->InClassIndex = States.Num() - 1;

		//	Sprite name
		if (TmpName.Length() != 4)
		{
			sc->Error("Invalid sprite name");
		}
		State->SpriteName = *TmpName.ToLower();

		//  Frame
		sc->ExpectString();
		char FChar = VStr::ToUpper(sc->String[0]);
		if (FChar < 'A' || FChar > ']')
		{
			sc->Error("Frames must be A-Z, [, \\ or ]");
		}
		State->Frame = FChar - 'A';
		VStr FramesString = sc->String;

		//  Tics
		bool Neg = sc->Check("-");
		sc->ExpectNumber();
		if (Neg)
		{
			State->Time = -sc->Number;
		}
		else
		{
			State->Time = float(sc->Number) / 35.0;
		}

		while (sc->GetString() && !sc->Crossed)
		{
			//	Check for bright parameter.
			if (!sc->String.ICmp("Bright"))
			{
				State->Frame |= FF_FULLBRIGHT;
				continue;
			}

			//	Check for offsets.
			if (!sc->String.ICmp("Offset"))
			{
				sc->Expect("(");
				Neg = sc->Check("-");
				sc->ExpectNumber();
				State->Misc1 = sc->Number * (Neg ? -1 : 1);
				sc->Expect(",");
				Neg = sc->Check("-");
				sc->ExpectNumber();
				State->Misc2 = sc->Number * (Neg ? -1 : 1);
				sc->Expect(")");
				continue;
			}

			GCon->Logf("State action %s", *sc->String);
			SkipBlock(sc, 2);
			sc->SetEscape(true);
			sc->SetCMode(false);
			return false;
		}
		sc->UnGet();

		//	Link previous state.
		if (PrevState)
		{
			PrevState->NextState = State;
		}

		//	Assign state to the labels.
		for (int i = NewLabelsStart; i < Class->StateLabels.Num(); i++)
		{
			Class->StateLabels[i].State = State;
			LoopStart = State;
		}
		NewLabelsStart = Class->StateLabels.Num();
		PrevState = State;

		for (size_t i = 1; i < FramesString.Length(); i++)
		{
			char FChar = VStr::ToUpper(FramesString[i]);
			if (FChar < 'A' || FChar > ']')
			{
				sc->Error("Frames must be A-Z, [, \\ or ]");
			}

			//	Create a new state.
			VState* s2 = new VState(va("S_%d", States.Num()));
			States.Append(s2);
			s2->Outer = Class;
			s2->InClassIndex = States.Num() - 1;
			s2->SpriteName = State->SpriteName;
			s2->Frame = (State->Frame & FF_FULLBRIGHT) | (FChar - 'A');
			s2->Time = State->Time;
			s2->Misc1 = State->Misc1;
			s2->Misc2 = State->Misc2;
			//s2->FunctionName = State->FunctionName;

			//	Link previous state.
			PrevState->NextState = s2;
			PrevState = s2;
		}
	}
	//	Re-enable escape sequences.
	sc->SetEscape(true);
	return true;
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
	//	Parse actor name. In order to allow dots in actor names, this is done
	// in non-C mode, so we have to do a little bit more complex parsing.
	sc->ExpectString();
	VStr NameStr;
	VStr ParentStr;
	int ColonPos = sc->String.IndexOf(':');
	if (ColonPos >= 0)
	{
		//	There's a colon inside, so plit up the string.
		NameStr = VStr(sc->String, 0, ColonPos);
		ParentStr = VStr(sc->String, ColonPos + 1, sc->String.Length() -
			ColonPos - 1);
	}
	else
	{
		NameStr = sc->String;
	}

	if (VClass::FindClass(*sc->String))
	{
		sc->Error(va("Redeclared class %s", *sc->String));
	}

	if (ColonPos < 0)
	{
		//	There's no colon, check if next string starts with it.
		sc->ExpectString();
		if (sc->String[0] == ':')
		{
			ColonPos = 0;
			ParentStr = VStr(sc->String, 1, sc->String.Length() - 1);
		}
		else
		{
			sc->UnGet();
		}
	}

	//	If we got colon but no parent class name, then get it.
	if (ColonPos >= 0 && !ParentStr)
	{
		sc->ExpectString();
		ParentStr = sc->String;
	}

	GCon->Logf("Actor %s of %s", *NameStr, *ParentStr);

	VClass* ParentClass = ActorClass;
	if (ParentStr)
	{
		ParentClass = VClass::FindClass(*ParentStr);
		if (!ParentClass)
		{
			//	Temporarely don't make it fatal error.
			GCon->Logf("Parent class %s not found", *ParentStr);
			if (sc->Check("replaces"))
			{
				sc->ExpectString();
			}
			sc->SetCMode(true);
			sc->SetEscape(false);
			sc->CheckNumber();
			sc->Expect("{");
			SkipBlock(sc, 1);
			sc->SetEscape(true);
			sc->SetCMode(false);
			return;
		}
		if (!ParentClass->IsChildOf(ScriptedEntityClass))
		{
			//	Temporarely don't make it fatal error.
			GCon->Logf("Parent class %s is not an actor class", *ParentStr);
			if (sc->Check("replaces"))
			{
				sc->ExpectString();
			}
			sc->SetCMode(true);
			sc->SetEscape(false);
			sc->CheckNumber();
			sc->Expect("{");
			SkipBlock(sc, 1);
			sc->SetEscape(true);
			sc->SetCMode(false);
			return;
		}
	}

	VClass* Class = ParentClass->CreateDerivedClass(*NameStr);
	Class->Outer = DecPkg;
	SetClassFieldBool(Class, "bNoPassMobj", true);

	VClass* ReplaceeClass = NULL;
	if (sc->Check("replaces"))
	{
		sc->ExpectString();
		ReplaceeClass = VClass::FindClass(*sc->String);
		if (!ReplaceeClass)
		{
			//	Temporarely don't make it fatal error.
			GCon->Logf("Replaced class %s not found", *sc->String);
			sc->SetCMode(true);
			sc->SetEscape(false);
			sc->CheckNumber();
			sc->Expect("{");
			SkipBlock(sc, 1);
			sc->SetEscape(true);
			sc->SetCMode(false);
			return;
		}
		if (!ReplaceeClass->IsChildOf(ScriptedEntityClass))
		{
			//	Temporarely don't make it fatal error.
			GCon->Logf("Replaced class %s is not an actor class", *sc->String);
			sc->SetCMode(true);
			sc->SetEscape(false);
			sc->CheckNumber();
			sc->Expect("{");
			SkipBlock(sc, 1);
			sc->SetEscape(true);
			sc->SetCMode(false);
			return;
		}
		GCon->Logf("%s replaces %s", *NameStr, *sc->String);
	}

	//	Time to switch to the C mode.
	sc->SetCMode(true);

	int GameFilter = 0;
	int DoomEdNum = -1;
	TArray<VState*> States;

	if (sc->CheckNumber())
	{
		if (sc->Number < -1 || sc->Number > 32767)
		{
			sc->Error("DoomEdNum is out of range [-1, 32767]");
		}
		DoomEdNum = sc->Number;
	}

	sc->Expect("{");
	while (!sc->Check("}"))
	{
		if (sc->Check("+"))
		{
			if (!ParseFlag(sc, Class, true))
			{
				return;
			}
			continue;
		}
		if (sc->Check("-"))
		{
			if (!ParseFlag(sc, Class, false))
			{
				return;
			}
			continue;
		}

		sc->ExpectIdentifier();
		VStr Prop = sc->String;
		while (sc->Check("."))
		{
			sc->ExpectIdentifier();
			Prop += ".";
			Prop += sc->String;
		}
		if (!Prop.ICmp("Radius"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Radius", sc->Float);
		}
		else if (!Prop.ICmp("Height"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Height", sc->Float);
		}
		else if (!Prop.ICmp("RenderStyle"))
		{
			int RenderStyle = 0;
			if (sc->Check("None"))
			{
				RenderStyle = STYLE_None;
			}
			else if (sc->Check("Normal"))
			{
				RenderStyle = STYLE_Normal;
			}
			else if (sc->Check("Fuzzy"))
			{
				RenderStyle = STYLE_Fuzzy;
			}
			else if (sc->Check("SoulTrans"))
			{
				RenderStyle = STYLE_SoulTrans;
			}
			else if (sc->Check("OptFuzzy"))
			{
				RenderStyle = STYLE_OptFuzzy;
			}
			else if (sc->Check("Translucent"))
			{
				RenderStyle = STYLE_Translucent;
			}
			else if (sc->Check("Add"))
			{
				RenderStyle = STYLE_Add;
			}
			else
			{
				sc->Error("Bad render style");
			}
			SetClassFieldByte(Class, "RenderStyle", RenderStyle);
		}
		else if (!Prop.ICmp("Alpha"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Alpha", MID(0.0, sc->Float, 1.0));
		}
		else if (!Prop.ICmp("Scale"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "ScaleX", sc->Float);
			SetClassFieldFloat(Class, "ScaleY", sc->Float);
		}
		else if (!Prop.ICmp("Health"))
		{
			sc->ExpectNumber();
			SetClassFieldInt(Class, "Health", sc->Number);
		}
		else if (!Prop.ICmp("Mass"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Mass", sc->Float);
		}
		else if (!Prop.ICmp("Speed"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Speed", sc->Float * 35.0);
			SetClassFieldFloat(Class, "StepSpeed", sc->Float);
		}
		else if (!Prop.ICmp("PainChance"))
		{
			sc->ExpectNumber();
			SetClassFieldFloat(Class, "PainChance", float(sc->Number) / 256.0);
		}
		else if (!Prop.ICmp("Damage"))
		{
			if (sc->Check("("))
			{
				GCon->Logf("Damage expression");
				sc->SetEscape(false);
				SkipBlock(sc, 1);
				sc->SetEscape(true);
				sc->SetCMode(false);
				return;
			}
			sc->ExpectNumber();
			SetClassFieldFloat(Class, "MissileDamage", sc->Number);
		}
		else if (!Prop.ICmp("Monster"))
		{
			SetClassFieldBool(Class, "bShootable", true);
			SetClassFieldBool(Class, "bCountKill", true);
			SetClassFieldBool(Class, "bSolid", true);
			SetClassFieldBool(Class, "bActivatePushWall", true);
			SetClassFieldBool(Class, "bActivateMCross", true);
			SetClassFieldBool(Class, "bNoPassMobj", false);
			SetClassFieldBool(Class, "bMonster", true);
		}
		else if (!Prop.ICmp("Projectile"))
		{
			SetClassFieldBool(Class, "bNoBlockmap", true);
			SetClassFieldBool(Class, "bNoGravity", true);
			SetClassFieldBool(Class, "bDropOff", true);
			SetClassFieldBool(Class, "bMissile", true);
			SetClassFieldBool(Class, "bActivateImpact", true);
			SetClassFieldBool(Class, "bActivatePCross", true);
			SetClassFieldBool(Class, "bNoTeleport", true);
		}
		else if (!Prop.ICmp("SeeSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "SightSound", *sc->String);
		}
		else if (!Prop.ICmp("ActiveSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "ActiveSound", *sc->String);
		}
		else if (!Prop.ICmp("AttackSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "AttackSound", *sc->String);
		}
		else if (!Prop.ICmp("PainSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "PainSound", *sc->String);
		}
		else if (!Prop.ICmp("DeathSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "DeathSound", *sc->String);
		}
		else if (!Prop.ICmp("BounceFactor"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "BounceFactor", sc->Float);
		}
		else if (!Prop.ICmp("Translation"))
		{
			SetClassFieldInt(Class, "Translation",
				R_ParseDecorateTranslation(sc));
		}
		else if (!Prop.ICmp("Inventory.PickupMessage"))
		{
			sc->ExpectString();
			SetClassFieldStr(Class, "PickupMessage", sc->String);
		}
		else if (!Prop.ICmp("Inventory.Amount"))
		{
			sc->ExpectNumber();
			SetClassFieldInt(Class, "Amount", sc->Number);
		}
		else if (!Prop.ICmp("Inventory.MaxAmount"))
		{
			sc->ExpectNumber();
			SetClassFieldInt(Class, "MaxAmount", sc->Number);
		}
		else if (!Prop.ICmp("Obituary"))
		{
			sc->ExpectString();
			GCon->Logf("Property Obituary is not yet supported");
		}
		else if (!Prop.ICmp("Decal"))
		{
			sc->ExpectString();
			GCon->Logf("Property Decal is not yet supported");
		}
		else if (!Prop.ICmp("States"))
		{
			if (!ParseStates(sc, Class, States))
			{
				return;
			}
		}
		else
		{
			GCon->Logf("Unknown property %s", *Prop);
			sc->SetEscape(false);
			SkipBlock(sc, 1);
			sc->SetEscape(true);
			sc->SetCMode(false);
			return;
		}
	}

	//	Set up linked list of states.
	if (States.Num())
	{
		Class->States = States[0];
		Class->NetStates = States[0];
		for (int i = 0; i < States.Num() - 1; i++)
		{
			States[i]->Next = States[i + 1];
			States[i]->NetNext = States[i + 1];
		}

		for (int i = 0; i < States.Num(); i++)
		{
			States[i]->SpriteIndex = VClass::FindSprite(States[i]->SpriteName);
			if (States[i]->GotoLabel != NAME_None)
			{
				States[i]->NextState = ResolveStateLabel(sc, Class,
					States[i]->GotoLabel, States[i]->GotoOffset);
			}
		}
	}

	for (int i = 0; i < Class->StateLabels.Num(); i++)
	{
		VStateLabel& Lbl = Class->StateLabels[i];
		if (Lbl.GotoLabel != NAME_None)
		{
			Lbl.State = ResolveStateLabel(sc, Class, Lbl.GotoLabel,
				Lbl.GotoOffset);
		}
	}

	if (DoomEdNum > 0)
	{
		mobjinfo_t& MI = VClass::GMobjInfos.Alloc();
		MI.class_id = Class;
		MI.doomednum = DoomEdNum;
		MI.GameFilter = GameFilter;
	}

	if (ReplaceeClass)
	{
		ReplaceeClass->Replacement = Class;
		Class->Replacee = ReplaceeClass;
	}
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

	//	Create class.
	VClass* Class = Type == OLDDEC_Pickup ?
		FakeInventoryClass->CreateDerivedClass(ClassName) :
		ActorClass->CreateDerivedClass(ClassName);
	Class->Outer = DecPkg;
	SetClassFieldBool(Class, "bNoPassMobj", true);
	if (Type == OLDDEC_Breakable)
	{
		SetClassFieldBool(Class, "bShootable", true);
	}
	if (Type == OLDDEC_Projectile)
	{
		SetClassFieldBool(Class, "bMissile", true);
		SetClassFieldBool(Class, "bDropOff", true);
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
	bool Explosive = false;

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
		else if ((Type == OLDDEC_Breakable || Type == OLDDEC_Projectile) &&
			sc->Check("DeathSprite"))
		{
			sc->ExpectString();
			if (sc->String.Length() != 4)
			{
				sc->Error("Sprite name must be 4 characters long");
			}
			DeathSprite = *sc->String.ToLower();
		}
		else if ((Type == OLDDEC_Breakable || Type == OLDDEC_Projectile) &&
			sc->Check("DeathFrames"))
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
			SetClassFieldFloat(Class, "ScaleX", sc->Float);
			SetClassFieldFloat(Class, "ScaleY", sc->Float);
		}
		else if (sc->Check("Alpha"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Alpha", MID(0.0, sc->Float, 1.0));
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
			SetClassFieldInt(Class, "Translation", (TRANSL_Standard <<
				TRANSL_TYPE_SHIFT) + sc->Number);
		}
		else if (sc->Check("Translation2"))
		{
			sc->ExpectNumber();
			if (sc->Number < 0 || sc->Number > MAX_LEVEL_TRANSLATIONS)
			{
				sc->Error(va("Translation2 is out of range [0, %d]",
					MAX_LEVEL_TRANSLATIONS));
			}
			SetClassFieldInt(Class, "Translation", (TRANSL_Level <<
				TRANSL_TYPE_SHIFT) + sc->Number);
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
		else if ((Type == OLDDEC_Breakable || Type == OLDDEC_Projectile) &&
			sc->Check("DeathSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "DeathSound", *sc->String);
		}
		else if (Type == OLDDEC_Breakable && sc->Check("BurnDeathSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "ActiveSound", *sc->String);
		}

		//	Projectile properties
		else if (Type == OLDDEC_Projectile && sc->Check("Speed"))
		{
			sc->ExpectFloat();
			SetClassFieldFloat(Class, "Speed", sc->Float * 35.0);
		}
		else if (Type == OLDDEC_Projectile && sc->Check("Damage"))
		{
			sc->ExpectNumber();
			SetClassFieldFloat(Class, "MissileDamage", sc->Number);
		}
		else if (Type == OLDDEC_Projectile && sc->Check("DamageType"))
		{
			if (sc->Check("Normal"))
			{
				SetClassFieldName(Class, "DamageType", NAME_None);
			}
			else
			{
				sc->ExpectString();
				SetClassFieldName(Class, "DamageType", *sc->String);
			}
		}
		else if (Type == OLDDEC_Projectile && sc->Check("SpawnSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "SightSound", *sc->String);
		}
		else if (Type == OLDDEC_Projectile && sc->Check("ExplosionRadius"))
		{
			sc->ExpectNumber();
			SetClassFieldFloat(Class, "ExplosionRadius", sc->Number);
			Explosive = true;
		}
		else if (Type == OLDDEC_Projectile && sc->Check("ExplosionDamage"))
		{
			sc->ExpectNumber();
			SetClassFieldFloat(Class, "ExplosionDamage", sc->Number);
			Explosive = true;
		}
		else if (Type == OLDDEC_Projectile && sc->Check("DoNotHurtShooter"))
		{
			SetClassFieldBool(Class, "bExplosionDontHurtSelf", true);
		}
		else if (Type == OLDDEC_Projectile && sc->Check("DoomBounce"))
		{
			SetClassFieldByte(Class, "BounceType", BOUNCE_Doom);
		}
		else if (Type == OLDDEC_Projectile && sc->Check("HereticBounce"))
		{
			SetClassFieldByte(Class, "BounceType", BOUNCE_Heretic);
		}
		else if (Type == OLDDEC_Projectile && sc->Check("HexenBounce"))
		{
			SetClassFieldByte(Class, "BounceType", BOUNCE_Hexen);
		}

		//	Pickup properties
		else if (Type == OLDDEC_Pickup && sc->Check("PickupMessage"))
		{
			sc->ExpectString();
			SetClassFieldStr(Class, "PickupMessage", sc->String);
		}
		else if (Type == OLDDEC_Pickup && sc->Check("PickupSound"))
		{
			sc->ExpectString();
			SetClassFieldName(Class, "PickupSound", *sc->String);
		}
		else if (Type == OLDDEC_Pickup && sc->Check("Respawns"))
		{
			SetClassFieldBool(Class, "bRespawns", true);
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
		else if (Type == OLDDEC_Pickup)
		{
			GCon->Logf("Unknown property %s", *sc->String);
			SkipBlock(sc, 1);
			break;
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
		if (!DiesAway && Type != OLDDEC_Projectile)
		{
			States[DeathEnd - 1]->Time = -1.0;
		}
		if (Type == OLDDEC_Projectile)
		{
			if (Explosive)
			{
				States[DeathStart]->Function = FuncA_ExplodeParms;
			}
		}
		else
		{
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
		}

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
			ParseOldDecoration(sc, OLDDEC_Pickup);
		}
		else if (sc->Check("projectile"))
		{
			ParseOldDecoration(sc, OLDDEC_Projectile);
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
	ScriptedEntityClass = VClass::FindClass("ScriptedEntity");
	FakeInventoryClass = VClass::FindClass("FakeInventory");
	FuncA_Scream = ActorClass->FindFunctionChecked("A_Scream");
	FuncA_NoBlocking = ActorClass->FindFunctionChecked("A_NoBlocking");
	FuncA_ScreamAndUnblock = ActorClass->FindFunctionChecked("A_ScreamAndUnblock");
	FuncA_ActiveSound = ActorClass->FindFunctionChecked("A_ActiveSound");
	FuncA_ActiveAndUnblock = ActorClass->FindFunctionChecked("A_ActiveAndUnblock");
	FuncA_ExplodeParms = ActorClass->FindFunctionChecked("A_ExplodeParms");

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
