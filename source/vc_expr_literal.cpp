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
#include "vc_local.h"
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
//	VIntLiteral::VIntLiteral
//
//==========================================================================

VIntLiteral::VIntLiteral(vint32 AValue, const TLocation& ALoc)
: VExpression(ALoc)
, Value(AValue)
{
	Type = TYPE_Int;
}

//==========================================================================
//
//	VIntLiteral::DoResolve
//
//==========================================================================

VExpression* VIntLiteral::DoResolve(VEmitContext&)
{
	return this;
}

//==========================================================================
//
//	VIntLiteral::Emit
//
//==========================================================================

void VIntLiteral::Emit(VEmitContext& ec)
{
	ec.EmitPushNumber(Value);
}

//==========================================================================
//
//	VIntLiteral::GetIntConst
//
//==========================================================================

vint32 VIntLiteral::GetIntConst() const
{
	return Value;
}

//==========================================================================
//
//	VIntLiteral::IsIntConst
//
//==========================================================================

bool VIntLiteral::IsIntConst() const
{
	return true;
}

//==========================================================================
//
//	VFloatLiteral::VFloatLiteral
//
//==========================================================================

VFloatLiteral::VFloatLiteral(float AValue, const TLocation& ALoc)
: VExpression(ALoc)
, Value(AValue)
{
	Type = TYPE_Float;
}

//==========================================================================
//
//	VFloatLiteral::DoResolve
//
//==========================================================================

VExpression* VFloatLiteral::DoResolve(VEmitContext&)
{
	return this;
}

//==========================================================================
//
//	VFloatLiteral::Emit
//
//==========================================================================

void VFloatLiteral::Emit(VEmitContext& ec)
{
	ec.AddStatement(OPC_PushNumber, Value);
}

//==========================================================================
//
//	VFloatLiteral::IsFloatConst
//
//==========================================================================

bool VFloatLiteral::IsFloatConst() const
{
	return true;
}

//==========================================================================
//
//	VFloatLiteral::GetFloatConst
//
//==========================================================================

float VFloatLiteral::GetFloatConst() const
{
	return Value;
}

//==========================================================================
//
//	VNameLiteral::VNameLiteral
//
//==========================================================================

VNameLiteral::VNameLiteral(VName AValue, const TLocation& ALoc)
: VExpression(ALoc)
, Value(AValue)
{
	Type = TYPE_Name;
}

//==========================================================================
//
//	VNameLiteral::DoResolve
//
//==========================================================================

VExpression* VNameLiteral::DoResolve(VEmitContext&)
{
	return this;
}

//==========================================================================
//
//	VNameLiteral::Emit
//
//==========================================================================

void VNameLiteral::Emit(VEmitContext& ec)
{
	ec.AddStatement(OPC_PushName, Value);
}

//==========================================================================
//
//	VStringLiteral::VStringLiteral
//
//==========================================================================

VStringLiteral::VStringLiteral(vint32 AValue, const TLocation& ALoc)
: VExpression(ALoc)
, Value(AValue)
{
	Type = TYPE_String;
}

//==========================================================================
//
//	VStringLiteral::DoResolve
//
//==========================================================================

VExpression* VStringLiteral::DoResolve(VEmitContext&)
{
	return this;
}

//==========================================================================
//
//	VStringLiteral::Emit
//
//==========================================================================

void VStringLiteral::Emit(VEmitContext& ec)
{
	ec.AddStatement(OPC_PushString, Value);
}

//==========================================================================
//
//	VStringLiteral::IsStrConst
//
//==========================================================================

bool VStringLiteral::IsStrConst() const
{
	return true;
}

//==========================================================================
//
//	VStringLiteral::GetStrConst
//
//==========================================================================

VStr VStringLiteral::GetStrConst(VPackage* Pkg) const
{
	return &Pkg->Strings[Value];
}

//==========================================================================
//
//	VSelf::VSelf
//
//==========================================================================

VSelf::VSelf(const TLocation& ALoc)
: VExpression(ALoc)
{
}

//==========================================================================
//
//	VSelf::DoResolve
//
//==========================================================================

VExpression* VSelf::DoResolve(VEmitContext& ec)
{
	if (!ec.SelfClass)
	{
		ParseError(Loc, "self used outside member function\n");
		delete this;
		return NULL;
	}
	if (ec.CurrentFunc->Flags & FUNC_Static)
	{
		ParseError(Loc, "self used in a static method\n");
		delete this;
		return NULL;
	}
	Type = VFieldType(ec.SelfClass);
	return this;
}

//==========================================================================
//
//	VSelf::Emit
//
//==========================================================================

void VSelf::Emit(VEmitContext& ec)
{
	ec.AddStatement(OPC_LocalValue0);
}

//==========================================================================
//
//	VNoneLiteral::VNoneLiteral
//
//==========================================================================

VNoneLiteral::VNoneLiteral(const TLocation& ALoc)
: VExpression(ALoc)
{
	Type = VFieldType((VClass*)NULL);
}

//==========================================================================
//
//	VNoneLiteral::DoResolve
//
//==========================================================================

VExpression* VNoneLiteral::DoResolve(VEmitContext&)
{
	return this;
}

//==========================================================================
//
//	VNoneLiteral::Emit
//
//==========================================================================

void VNoneLiteral::Emit(VEmitContext& ec)
{
	ec.AddStatement(OPC_PushNull);
}

//==========================================================================
//
//	VNullLiteral::VNullLiteral
//
//==========================================================================

VNullLiteral::VNullLiteral(const TLocation& ALoc)
: VExpression(ALoc)
{
	Type = VFieldType(TYPE_Void).MakePointerType();
}

//==========================================================================
//
//	VNullLiteral::DoResolve
//
//==========================================================================

VExpression* VNullLiteral::DoResolve(VEmitContext&)
{
	return this;
}

//==========================================================================
//
//	VNullLiteral::Emit
//
//==========================================================================

void VNullLiteral::Emit(VEmitContext& ec)
{
	ec.AddStatement(OPC_PushNull);
}
