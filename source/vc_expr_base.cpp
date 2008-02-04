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
//	VExpression::VExpression
//
//==========================================================================

VExpression::VExpression(const TLocation& ALoc)
: Type(TYPE_Void)
, RealType(TYPE_Void)
, Flags(0)
, Loc(ALoc)
{
}

//==========================================================================
//
//	VExpression::~VExpression
//
//==========================================================================

VExpression::~VExpression()
{
}

//==========================================================================
//
//	VExpression::Resolve
//
//==========================================================================

VExpression* VExpression::Resolve(VEmitContext& ec)
{
	VExpression* e = DoResolve(ec);
	return e;
}

//==========================================================================
//
//	VExpression::ResolveBoolean
//
//==========================================================================

VExpression* VExpression::ResolveBoolean(VEmitContext& ec)
{
	VExpression* e = Resolve(ec);
	if (!e)
	{
		return NULL;
	}

	switch (e->Type.Type)
	{
	case TYPE_Int:
	case TYPE_Byte:
	case TYPE_Bool:
	case TYPE_Float:
	case TYPE_Name:
		break;

#ifdef IN_VCC
	case TYPE_Pointer:
	case TYPE_Reference:
	case TYPE_Class:
	case TYPE_State:
		e = new VPointerToBool(e);
		break;

	case TYPE_String:
		e = new VStringToBool(e);
		break;

	case TYPE_Delegate:
		e = new VDelegateToBool(e);
		break;
#endif

	default:
		ParseError(Loc, "Expression type mistmatch, boolean expression expected");
		delete e;
		return NULL;
	}
	return e;
}

//==========================================================================
//
//	VExpression::ResolveAsType
//
//==========================================================================

VTypeExpr* VExpression::ResolveAsType(VEmitContext&)
{
	ParseError(Loc, "Invalid type expression");
	delete this;
	return NULL;
}

//==========================================================================
//
//	VExpression::ResolveAssignmentTarget
//
//==========================================================================

VExpression* VExpression::ResolveAssignmentTarget(VEmitContext& ec)
{
	return Resolve(ec);
}

//==========================================================================
//
//	VExpression::ResolveIterator
//
//==========================================================================

VExpression* VExpression::ResolveIterator(VEmitContext&)
{
	ParseError(Loc, "Iterator method expected");
	delete this;
	return NULL;
}

//==========================================================================
//
//	VExpression::RequestAddressOf
//
//==========================================================================

void VExpression::RequestAddressOf()
{
	ParseError(Loc, "Bad address operation");
}

//==========================================================================
//
//	VExpression::EmitBranchable
//
//==========================================================================

void VExpression::EmitBranchable(VEmitContext& ec, VLabel Lbl, bool OnTrue)
{
	Emit(ec);
	if (OnTrue)
	{
		ec.AddStatement(OPC_IfGoto, Lbl);
	}
	else
	{
		ec.AddStatement(OPC_IfNotGoto, Lbl);
	}
}

//==========================================================================
//
//	VExpression::EmitPushPointedCode
//
//==========================================================================

void VExpression::EmitPushPointedCode(VFieldType type, VEmitContext& ec)
{
	switch (type.Type)
	{
	case TYPE_Int:
	case TYPE_Float:
	case TYPE_Name:
		ec.AddStatement(OPC_PushPointed);
		break;

	case TYPE_Byte:
		ec.AddStatement(OPC_PushPointedByte);
		break;

	case TYPE_Bool:
		if (type.BitMask & 0x000000ff)
			ec.AddStatement(OPC_PushBool0, (int)(type.BitMask));
		else if (type.BitMask & 0x0000ff00)
			ec.AddStatement(OPC_PushBool1, (int)(type.BitMask >> 8));
		else if (type.BitMask & 0x00ff0000)
			ec.AddStatement(OPC_PushBool2, (int)(type.BitMask >> 16));
		else
			ec.AddStatement(OPC_PushBool3, (int)(type.BitMask >> 24));
		break;

	case TYPE_Pointer:
	case TYPE_Reference:
	case TYPE_Class:
	case TYPE_State:
		ec.AddStatement(OPC_PushPointedPtr);
		break;

	case TYPE_Vector:
		ec.AddStatement(OPC_VPushPointed);
		break;

	case TYPE_String:
		ec.AddStatement(OPC_PushPointedStr);
		break;

	case TYPE_Delegate:
		ec.AddStatement(OPC_PushPointedDelegate);
		break;

	default:
		ParseError(Loc, "Bad push pointed");
	}
}

//==========================================================================
//
//	VExpression::IsValidTypeExpression
//
//==========================================================================

bool VExpression::IsValidTypeExpression()
{
	return false;
}

//==========================================================================
//
//	VExpression::IsIntConst
//
//==========================================================================

bool VExpression::IsIntConst() const
{
	return false;
}

//==========================================================================
//
//	VExpression::IsFloatConst
//
//==========================================================================

bool VExpression::IsFloatConst() const
{
	return false;
}

//==========================================================================
//
//	VExpression::GetIntConst
//
//==========================================================================

vint32 VExpression::GetIntConst() const
{
	ParseError(Loc, "Integer constant expected");
	return 0;
}

//==========================================================================
//
//	VExpression::GetFloatConst
//
//==========================================================================

float VExpression::GetFloatConst() const
{
	ParseError(Loc, "Float constant expected");
	return 0.0;
}

//==========================================================================
//
//	VExpression::IsDefaultObject
//
//==========================================================================

bool VExpression::IsDefaultObject() const
{
	return false;
}

//==========================================================================
//
//	VExpression::IsPropertyAssign
//
//==========================================================================

bool VExpression::IsPropertyAssign() const
{
	return false;
}

//==========================================================================
//
//	VExpression::IsDynArraySetNum
//
//==========================================================================

bool VExpression::IsDynArraySetNum() const
{
	return false;
}

//==========================================================================
//
//	VExpression::CreateTypeExprCopy
//
//==========================================================================

VExpression* VExpression::CreateTypeExprCopy()
{
	ParseError(Loc, "Not a type");
#ifdef IN_VCC
	return new VTypeExpr(TYPE_Unknown, Loc);
#endif
}

//==========================================================================
//
//	VExpression::AddDropResult
//
//==========================================================================

bool VExpression::AddDropResult()
{
	return false;
}
