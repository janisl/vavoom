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
//	VTypeExpr::VTypeExpr
//
//==========================================================================

VTypeExpr::VTypeExpr(VFieldType AType, const TLocation& ALoc)
: VExpression(ALoc)
, MetaClassName(NAME_None)
{
	Type = AType;
}

//==========================================================================
//
//	VTypeExpr::VTypeExpr
//
//==========================================================================

VTypeExpr::VTypeExpr(VFieldType AType, const TLocation& ALoc, VName AMetaClassName)
: VExpression(ALoc)
, MetaClassName(AMetaClassName)
{
	Type = AType;
}

//==========================================================================
//
//	VTypeExpr::DoResolve
//
//==========================================================================

VExpression* VTypeExpr::DoResolve(VEmitContext& ec)
{
	return ResolveAsType(ec);
}

//==========================================================================
//
//	VTypeExpr::ResolveAsType
//
//==========================================================================

VTypeExpr* VTypeExpr::ResolveAsType(VEmitContext&)
{
	if (Type.Type == TYPE_Unknown)
	{
		ParseError(Loc, "Bad type");
		delete this;
		return NULL;
	}

	if (Type.Type == TYPE_Class && MetaClassName != NAME_None)
	{
		Type.Class = VMemberBase::StaticFindClass(MetaClassName);
		if (!Type.Class)
		{
			ParseError(Loc, "No such class %s", *MetaClassName);
			delete this;
			return NULL;
		}
	}
	return this;
}

//==========================================================================
//
//	VTypeExpr::Emit
//
//==========================================================================

void VTypeExpr::Emit(VEmitContext&)
{
	ParseError(Loc, "Should not happen");
}

//==========================================================================
//
//	VTypeExpr::GetName
//
//==========================================================================

VStr VTypeExpr::GetName() const
{
	return Type.GetName();
}

//==========================================================================
//
//	VTypeExpr::CreateTypeExprCopy
//
//==========================================================================

VExpression* VTypeExpr::CreateTypeExprCopy()
{
	return new VTypeExpr(Type, Loc, MetaClassName);
}

//==========================================================================
//
//	VPointerType::VPointerType
//
//==========================================================================

VPointerType::VPointerType(VExpression* AExpr, const TLocation& ALoc)
: VTypeExpr(TYPE_Unknown, ALoc)
, Expr(AExpr)
{
}

//==========================================================================
//
//	VPointerType::~VPointerType
//
//==========================================================================

VPointerType::~VPointerType()
{
	if (Expr)
	{
		delete Expr;
		Expr = NULL;
	}
}

//==========================================================================
//
//	VPointerType::ResolveAsType
//
//==========================================================================

VTypeExpr* VPointerType::ResolveAsType(VEmitContext& ec)
{
	if (Expr)
	{
		Expr = Expr->ResolveAsType(ec);
	}
	if (!Expr)
	{
		delete this;
		return NULL;
	}

	Type = Expr->Type.MakePointerType();
	return this;
}

//==========================================================================
//
//	VPointerType::CreateTypeExprCopy
//
//==========================================================================

VExpression* VPointerType::CreateTypeExprCopy()
{
	return new VPointerType(Expr->CreateTypeExprCopy(), Loc);
}

//==========================================================================
//
//	VFixedArrayType::VFixedArrayType
//
//==========================================================================

VFixedArrayType::VFixedArrayType(VExpression* AExpr, VExpression* ASizeExpr,
	const TLocation& ALoc)
: VTypeExpr(TYPE_Unknown, ALoc)
, Expr(AExpr)
, SizeExpr(ASizeExpr)
{
	if (!SizeExpr)
	{
		ParseError(Loc, "Array size expected");
	}
}

//==========================================================================
//
//	VFixedArrayType::~VFixedArrayType
//
//==========================================================================

VFixedArrayType::~VFixedArrayType()
{
	if (Expr)
	{
		delete Expr;
		Expr = NULL;
	}
	if (SizeExpr)
	{
		delete SizeExpr;
		SizeExpr = NULL;
	}
}

//==========================================================================
//
//	VFixedArrayType::ResolveAsType
//
//==========================================================================

VTypeExpr* VFixedArrayType::ResolveAsType(VEmitContext& ec)
{
	if (Expr)
	{
		Expr = Expr->ResolveAsType(ec);
	}
	if (SizeExpr)
	{
		SizeExpr = SizeExpr->Resolve(ec);
	}
	if (!Expr || !SizeExpr)
	{
		delete this;
		return NULL;
	}

	if (!SizeExpr->IsIntConst())
	{
		ParseError(SizeExpr->Loc, "Integer constant expected");
		delete this;
		return NULL;
	}

	vint32 Size = SizeExpr->GetIntConst();
	Type = Expr->Type.MakeArrayType(Size, Loc);
	return this;
}

//==========================================================================
//
//	VDynamicArrayType::VDynamicArrayType
//
//==========================================================================

VDynamicArrayType::VDynamicArrayType(VExpression* AExpr,
	const TLocation& ALoc)
: VTypeExpr(TYPE_Unknown, ALoc)
, Expr(AExpr)
{
}

//==========================================================================
//
//	VDynamicArrayType::~VDynamicArrayType
//
//==========================================================================

VDynamicArrayType::~VDynamicArrayType()
{
	if (Expr)
	{
		delete Expr;
		Expr = NULL;
	}
}

//==========================================================================
//
//	VDynamicArrayType::ResolveAsType
//
//==========================================================================

VTypeExpr* VDynamicArrayType::ResolveAsType(VEmitContext& ec)
{
	if (Expr)
	{
		Expr = Expr->ResolveAsType(ec);
	}
	if (!Expr)
	{
		delete this;
		return NULL;
	}

	Type = Expr->Type.MakeDynamicArrayType(Loc);
	return this;
}

//==========================================================================
//
//	VDynamicArrayType::CreateTypeExprCopy
//
//==========================================================================

VExpression* VDynamicArrayType::CreateTypeExprCopy()
{
	return new VDynamicArrayType(Expr->CreateTypeExprCopy(), Loc);
}
