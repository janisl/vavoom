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

//==========================================================================
//
//	VTypeExpr
//
//==========================================================================

class VTypeExpr : public VExpression
{
public:
	VName		MetaClassName;

	VTypeExpr(VFieldType, const TLocation&);
	VTypeExpr(VFieldType, const TLocation&, VName);
	VExpression* DoResolve(VEmitContext&);
	VTypeExpr* ResolveAsType(VEmitContext&);
	void Emit(VEmitContext&);
	VStr GetName() const;
	VExpression* CreateTypeExprCopy();
};

//==========================================================================
//
//	VPointerType
//
//==========================================================================

class VPointerType : public VTypeExpr
{
public:
	VExpression*	Expr;

	VPointerType(VExpression*, const TLocation&);
	~VPointerType();
	VTypeExpr* ResolveAsType(VEmitContext&);
	VExpression* CreateTypeExprCopy();
};

//==========================================================================
//
//	VFixedArrayType
//
//==========================================================================

class VFixedArrayType : public VTypeExpr
{
public:
	VExpression*	Expr;
	VExpression*	SizeExpr;

	VFixedArrayType(VExpression*, VExpression*, const TLocation&);
	~VFixedArrayType();
	VTypeExpr* ResolveAsType(VEmitContext&);
};

//==========================================================================
//
//	VDynamicArrayType
//
//==========================================================================

class VDynamicArrayType : public VTypeExpr
{
public:
	VExpression*	Expr;

	VDynamicArrayType(VExpression*, const TLocation&);
	~VDynamicArrayType();
	VTypeExpr* ResolveAsType(VEmitContext&);
	VExpression* CreateTypeExprCopy();
};
