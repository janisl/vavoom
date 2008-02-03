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
//	VConstant::VConstant
//
//==========================================================================

VConstant::VConstant(VName AName, VMemberBase* AOuter, TLocation ALoc)
: VMemberBase(MEMBER_Const, AName, AOuter, ALoc)
, Type(TYPE_Unknown)
, Value(0)
#ifdef IN_VCC
, ValueExpr(NULL)
, PrevEnumValue(NULL)
#endif
{
}

//==========================================================================
//
//	VConstant::Serialise
//
//==========================================================================

void VConstant::Serialise(VStream& Strm)
{
	guard(VConstant::Serialise);
	VMemberBase::Serialise(Strm);
	Strm << Type;
	switch (Type)
	{
	case TYPE_Float:
		Strm << FloatValue;
		break;

	case TYPE_Name:
		Strm << *(VName*)&Value;
		break;

	default:
		Strm << STRM_INDEX(Value);
		break;
	}
	unguard;
}

#ifdef IN_VCC

//==========================================================================
//
//	VConstant::~VConstant
//
//==========================================================================

VConstant::~VConstant()
{
	if (ValueExpr)
		delete ValueExpr;
}

//==========================================================================
//
//	VConstant::Define
//
//==========================================================================

bool VConstant::Define()
{
	if (PrevEnumValue)
	{
		Value = PrevEnumValue->Value + 1;
		return true;
	}

	if (ValueExpr)
	{
		VEmitContext ec(this);
		ValueExpr = ValueExpr->Resolve(ec);
	}
	if (!ValueExpr)
	{
		return false;
	}

	switch (Type)
	{
	case TYPE_Int:
		if (!ValueExpr->IsIntConst())
		{
			ParseError(ValueExpr->Loc, "Integer constant expected");
			return false;
		}
		Value = ValueExpr->GetIntConst();
		break;

	case TYPE_Float:
		if (!ValueExpr->IsFloatConst())
		{
			ParseError(ValueExpr->Loc, "Float constant expected");
			return false;
		}
		FloatValue = ValueExpr->GetFloatConst();
		break;

	default:
		ParseError(Loc, "Unsupported type of constant");
		return false;
	}
	return true;
}

#endif
