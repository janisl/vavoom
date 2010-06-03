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

#include "vc_local.h"

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
//	VArrayElement::VArrayElement
//
//==========================================================================

VArrayElement::VArrayElement(VExpression* AOp, VExpression* AInd, const TLocation& ALoc)
: VExpression(ALoc)
, op(AOp)
, ind(AInd)
, AddressRequested(false)
, IsAssign(false)
{
	if (!ind)
	{
		ParseError(Loc, "Expression expected");
		return;
	}
}

//==========================================================================
//
//	VArrayElement::~VArrayElement
//
//==========================================================================

VArrayElement::~VArrayElement()
{
	if (op)
	{
		delete op;
		op = NULL;
	}
	if (ind)
	{
		delete ind;
		ind = NULL;
	}
}

//==========================================================================
//
//	VArrayElement::DoResolve
//
//==========================================================================

VExpression* VArrayElement::DoResolve(VEmitContext& ec)
{
	if (op)
		op = op->Resolve(ec);
	if (ind)
		ind = ind->Resolve(ec);
	if (!op || !ind)
	{
		delete this;
		return NULL;
	}

	if (ind->Type.Type != TYPE_Int)
	{
		ParseError(Loc, "Array index must be of integer type");
		delete this;
		return NULL;
	}
	if (op->Type.Type == TYPE_Array || op->Type.Type == TYPE_DynamicArray)
	{
		Flags = op->Flags;
		Type = op->Type.GetArrayInnerType();
		op->Flags &= ~FIELD_ReadOnly;
		op->RequestAddressOf();
	}
	else if (op->Type.Type == TYPE_Pointer)
	{
		Flags = 0;
		Type = op->Type.GetPointerInnerType();
	}
	else
	{
		ParseError(Loc, "Bad operation with array");
		delete this;
		return NULL;
	}

	RealType = Type;
	if (Type.Type == TYPE_Byte || Type.Type == TYPE_Bool)
	{
		Type = VFieldType(TYPE_Int);
	}
	return this;
}

//==========================================================================
//
//	VArrayElement::ResolveAssignmentTarget
//
//==========================================================================

VExpression* VArrayElement::ResolveAssignmentTarget(VEmitContext& ec)
{
	IsAssign = true;
	return Resolve(ec);
}

//==========================================================================
//
//	VArrayElement::RequestAddressOf
//
//==========================================================================

void VArrayElement::RequestAddressOf()
{
	if (Flags & FIELD_ReadOnly)
	{
		ParseError(op->Loc, "Tried to assign to a read-only field");
	}
	if (AddressRequested)
		ParseError(Loc, "Multiple address of");
	AddressRequested = true;
}

//==========================================================================
//
//	VArrayElement::Emit
//
//==========================================================================

void VArrayElement::Emit(VEmitContext& ec)
{
	op->Emit(ec);
	ind->Emit(ec);
	if (op->Type.Type == TYPE_DynamicArray)
	{
		if (IsAssign)
		{
			ec.AddStatement(OPC_DynArrayElementGrow, RealType);
		}
		else
		{
			ec.AddStatement(OPC_DynArrayElement, RealType);
		}
	}
	else
	{
		ec.AddStatement(OPC_ArrayElement, RealType);
	}
	if (!AddressRequested)
	{
		EmitPushPointedCode(RealType, ec);
	}
}

//==========================================================================
//
//	VDynArrayGetNum::VDynArrayGetNum
//
//==========================================================================

VDynArrayGetNum::VDynArrayGetNum(VExpression* AArrayExpr,
	const TLocation& ALoc)
: VExpression(ALoc)
, ArrayExpr(AArrayExpr)
{
	Flags = FIELD_ReadOnly;
}

//==========================================================================
//
//	VDynArrayGetNum::~VDynArrayGetNum
//
//==========================================================================

VDynArrayGetNum::~VDynArrayGetNum()
{
	if (ArrayExpr)
	{
		delete ArrayExpr;
		ArrayExpr = NULL;
	}
}

//==========================================================================
//
//	VDynArrayGetNum::DoResolve
//
//==========================================================================

VExpression* VDynArrayGetNum::DoResolve(VEmitContext&)
{
	Type = VFieldType(TYPE_Int);
	return this;
}

//==========================================================================
//
//	VDynArrayGetNum::Emit
//
//==========================================================================

void VDynArrayGetNum::Emit(VEmitContext& ec)
{
	ArrayExpr->Emit(ec);
	ec.AddStatement(OPC_DynArrayGetNum);
}

//==========================================================================
//
//	VDynArraySetNum::VDynArraySetNum
//
//==========================================================================

VDynArraySetNum::VDynArraySetNum(VExpression* AArrayExpr,
	VExpression* ANumExpr, const TLocation& ALoc)
: VExpression(ALoc)
, ArrayExpr(AArrayExpr)
, NumExpr(ANumExpr)
{
	Type = VFieldType(TYPE_Void);
}

//==========================================================================
//
//	VDynArraySetNum::~VDynArraySetNum
//
//==========================================================================

VDynArraySetNum::~VDynArraySetNum()
{
	if (ArrayExpr)
	{
		delete ArrayExpr;
		ArrayExpr = NULL;
	}
	if (NumExpr)
	{
		delete NumExpr;
		NumExpr = NULL;
	}
}

//==========================================================================
//
//	VDynArraySetNum::DoResolve
//
//==========================================================================

VExpression* VDynArraySetNum::DoResolve(VEmitContext&)
{
	return this;
}

//==========================================================================
//
//	VDynArraySetNum::Emit
//
//==========================================================================

void VDynArraySetNum::Emit(VEmitContext& ec)
{
	ArrayExpr->Emit(ec);
	NumExpr->Emit(ec);
	ec.AddStatement(OPC_DynArraySetNum, ArrayExpr->Type.GetArrayInnerType());
}

//==========================================================================
//
//	VDynArraySetNum::IsDynArraySetNum
//
//==========================================================================

bool VDynArraySetNum::IsDynArraySetNum() const
{
	return true;
}

//==========================================================================
//
//	VDynArrayInsert::VDynArrayInsert
//
//==========================================================================

VDynArrayInsert::VDynArrayInsert(VExpression* AArrayExpr,
	VExpression* AIndexExpr, VExpression* ACountExpr, const TLocation& ALoc)
: VExpression(ALoc)
, ArrayExpr(AArrayExpr)
, IndexExpr(AIndexExpr)
, CountExpr(ACountExpr)
{
}

//==========================================================================
//
//	VDynArrayInsert::~VDynArrayInsert
//
//==========================================================================

VDynArrayInsert::~VDynArrayInsert()
{
	if (ArrayExpr)
	{
		delete ArrayExpr;
		ArrayExpr = NULL;
	}
	if (IndexExpr)
	{
		delete IndexExpr;
		IndexExpr = NULL;
	}
	if (CountExpr)
	{
		delete CountExpr;
		CountExpr = NULL;
	}
}

//==========================================================================
//
//	VDynArrayInsert::DoResolve
//
//==========================================================================

VExpression* VDynArrayInsert::DoResolve(VEmitContext& ec)
{
	ArrayExpr->RequestAddressOf();

	//	Resolve arguments.
	if (IndexExpr)
	{
		IndexExpr = IndexExpr->Resolve(ec);
	}
	if (CountExpr)
	{
		CountExpr = CountExpr->Resolve(ec);
	}
	if (!IndexExpr || !CountExpr)
	{
		delete this;
		return NULL;
	}

	//	Check argument types.
	if (IndexExpr->Type.Type != TYPE_Int)
	{
		ParseError(Loc, "Index must be integer expression");
		delete this;
		return NULL;
	}
	if (CountExpr->Type.Type != TYPE_Int)
	{
		ParseError(Loc, "Count must be integer expression");
		delete this;
		return NULL;
	}

	Type = VFieldType(TYPE_Void);
	return this;
}

//==========================================================================
//
//	VDynArrayInsert::Emit
//
//==========================================================================

void VDynArrayInsert::Emit(VEmitContext& ec)
{
	ArrayExpr->Emit(ec);
	IndexExpr->Emit(ec);
	CountExpr->Emit(ec);
	ec.AddStatement(OPC_DynArrayInsert, ArrayExpr->Type.GetArrayInnerType());
}

//==========================================================================
//
//	VDynArrayRemove::VDynArrayRemove
//
//==========================================================================

VDynArrayRemove::VDynArrayRemove(VExpression* AArrayExpr,
	VExpression* AIndexExpr, VExpression* ACountExpr, const TLocation& ALoc)
: VExpression(ALoc)
, ArrayExpr(AArrayExpr)
, IndexExpr(AIndexExpr)
, CountExpr(ACountExpr)
{
}

//==========================================================================
//
//	VDynArrayRemove::~VDynArrayRemove
//
//==========================================================================

VDynArrayRemove::~VDynArrayRemove()
{
	if (ArrayExpr)
	{
		delete ArrayExpr;
		ArrayExpr = NULL;
	}
	if (IndexExpr)
	{
		delete IndexExpr;
		IndexExpr = NULL;
	}
	if (CountExpr)
	{
		delete CountExpr;
		CountExpr = NULL;
	}
}

//==========================================================================
//
//	VDynArrayRemove::DoResolve
//
//==========================================================================

VExpression* VDynArrayRemove::DoResolve(VEmitContext& ec)
{
	ArrayExpr->RequestAddressOf();

	//	Resolve arguments.
	if (IndexExpr)
	{
		IndexExpr = IndexExpr->Resolve(ec);
	}
	if (CountExpr)
	{
		CountExpr = CountExpr->Resolve(ec);
	}
	if (!IndexExpr || !CountExpr)
	{
		delete this;
		return NULL;
	}

	//	Check argument types.
	if (IndexExpr->Type.Type != TYPE_Int)
	{
		ParseError(Loc, "Index must be integer expression");
		delete this;
		return NULL;
	}
	if (CountExpr->Type.Type != TYPE_Int)
	{
		ParseError(Loc, "Count must be integer expression");
		delete this;
		return NULL;
	}

	Type = VFieldType(TYPE_Void);
	return this;
}

//==========================================================================
//
//	VDynArrayRemove::Emit
//
//==========================================================================

void VDynArrayRemove::Emit(VEmitContext& ec)
{
	ArrayExpr->Emit(ec);
	IndexExpr->Emit(ec);
	CountExpr->Emit(ec);
	ec.AddStatement(OPC_DynArrayRemove, ArrayExpr->Type.GetArrayInnerType());
}
