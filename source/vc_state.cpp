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
#include "progdefs.h"
#endif

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VState::VState
//
//==========================================================================

VState::VState(VName AName, VMemberBase* AOuter, TLocation ALoc)
: VMemberBase(MEMBER_State, AName, AOuter, ALoc)
, SpriteName(NAME_None)
#ifndef IN_VCC
, SpriteIndex(0)
#endif
, Frame(0)
, Time(0)
, Misc1(0)
, Misc2(0)
, NextState(0)
, Function(0)
, Next(0)
#ifndef IN_VCC
, InClassIndex(-1)
, NetId(-1)
, NetNext(0)
#endif
, GotoLabel(NAME_None)
, GotoOffset(0)
#ifdef IN_VCC
, FunctionName(NAME_None)
#endif
{
}

//==========================================================================
//
//	VState::Serialise
//
//==========================================================================

void VState::Serialise(VStream& Strm)
{
	guard(VState::Serialise);
	VMemberBase::Serialise(Strm);
	Strm << SpriteName
		<< STRM_INDEX(Frame)
		<< Time
		<< STRM_INDEX(Misc1)
		<< STRM_INDEX(Misc2)
		<< NextState
		<< Function
		<< Next;
#ifndef IN_VCC
	if (Strm.IsLoading())
	{
		SpriteIndex = VClass::FindSprite(SpriteName);
		NetNext = Next;
	}
#endif
	unguard;
}

#ifdef IN_VCC

//==========================================================================
//
//	VState::~VState
//
//==========================================================================

VState::~VState()
{
}

//==========================================================================
//
//	VState::Define
//
//==========================================================================

bool VState::Define()
{
	bool Ret = true;

	if (Function && !Function->Define())
	{
		Ret = false;
	}

	return Ret;
}

//==========================================================================
//
//	VState::Emit
//
//==========================================================================

void VState::Emit()
{
	VEmitContext ec(this);
	if (GotoLabel != NAME_None)
	{
		NextState = ((VClass*)Outer)->ResolveStateLabel(Loc, GotoLabel,
			GotoOffset);
	}

	if (Function)
	{
		Function->Emit();
	}
	else if (FunctionName != NAME_None)
	{
		Function = ((VClass*)Outer)->CheckForMethod(FunctionName);
		if (!Function)
		{
			ParseError(Loc, "No such method %s", *FunctionName);
		}
		else
		{
			if (Function->ReturnType.Type != TYPE_Void)
			{
				ParseError(Loc, "State method must not return a value");
			}
			if (Function->NumParams)
			{
				ParseError(Loc, "State method must not take any arguments");
			}
			if (Function->Flags & FUNC_Static)
			{
				ParseError(Loc, "State method must not be static");
			}
			if (Function->Flags & FUNC_VarArgs)
			{
				ParseError(Loc, "State method must not have varargs");
			}
			if (!(Function->Flags & FUNC_Final))
			{
				ParseError(Loc, "State method must be final");
			}
		}
	}
}

#else

//==========================================================================
//
//	VState::IsInRange
//
//==========================================================================

bool VState::IsInRange(VState* Start, VState* End, int MaxDepth)
{
	guard(VState::IsInRange);
	int Depth = 0;
	VState* check = Start;
	do
	{
		if (check == this)
			return true;
		if (check)
			check = check->Next;
		Depth++;
	}
	while (Depth < MaxDepth && check != End);
	return false;
	unguard;
}

//==========================================================================
//
//	VState::IsInSequence
//
//==========================================================================

bool VState::IsInSequence(VState* Start)
{
	guard(VState::IsInRange);
	for (VState* check = Start; check;
		check = check->Next == check->NextState ? check->Next : NULL)
	{
		if (check == this)
		{
			return true;
		}
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VState::GetPlus
//
//==========================================================================

VState* VState::GetPlus(int Offset, bool IgnoreJump)
{
	guard(VState::GetPlus);
	check(Offset >= 0);
	VState* S = this;
	int Count = Offset;
	while (S && Count--)
	{
		if (!IgnoreJump && S->Next != S->NextState)
		{
			return NULL;
		}
		S = S->Next;
	}
	return S;
	unguard;
}

#endif
