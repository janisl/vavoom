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
//	VVector
//
//==========================================================================

class VVector : public VExpression
{
public:
	VExpression*	op1;
	VExpression*	op2;
	VExpression*	op3;

	VVector(VExpression*, VExpression*, VExpression*, const TLocation&);
	~VVector();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VSingleName
//
//==========================================================================

class VSingleName : public VExpression
{
public:
	VName			Name;

	VSingleName(VName, const TLocation&);
	VExpression* IntResolve(VEmitContext&, bool);
	VExpression* DoResolve(VEmitContext&);
	VExpression* ResolveAssignmentTarget(VEmitContext&);
	VTypeExpr* ResolveAsType(VEmitContext&);
	void Emit(VEmitContext&);
	bool IsValidTypeExpression();
	VExpression* CreateTypeExprCopy();
};

//==========================================================================
//
//	VDoubleName
//
//==========================================================================

class VDoubleName : public VExpression
{
public:
	VName			Name1;
	VName			Name2;

	VDoubleName(VName, VName, const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	VTypeExpr* ResolveAsType(VEmitContext&);
	void Emit(VEmitContext&);
	bool IsValidTypeExpression();
	VExpression* CreateTypeExprCopy();
};

//==========================================================================
//
//	VDefaultObject
//
//==========================================================================

class VDefaultObject : public VExpression
{
public:
	VExpression*		op;

	VDefaultObject(VExpression*, const TLocation&);
	~VDefaultObject();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	bool IsDefaultObject() const;
};

//==========================================================================
//
//	VPushPointed
//
//==========================================================================

class VPushPointed : public VExpression
{
public:
	VExpression*	op;
	bool			AddressRequested;

	VPushPointed(VExpression*);
	~VPushPointed();
	VExpression* DoResolve(VEmitContext&);
	void RequestAddressOf();
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VConditional
//
//==========================================================================

class VConditional : public VExpression
{
public:
	VExpression*		op;
	VExpression*		op1;
	VExpression*		op2;

	VConditional(VExpression*, VExpression*, VExpression*, const TLocation&);
	~VConditional();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDropResult
//
//==========================================================================

class VDropResult : public VExpression
{
public:
	VExpression*		op;

	VDropResult(VExpression*);
	~VDropResult();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VClassConstant
//
//==========================================================================

class VClassConstant : public VExpression
{
public:
	VClass*		Class;

	VClassConstant(VClass* AClass, const TLocation& ALoc);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VConstantValue
//
//==========================================================================

class VConstantValue : public VExpression
{
public:
	VConstant*		Const;

	VConstantValue(VConstant* AConst, const TLocation& ALoc);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	bool IsIntConst() const;
	bool IsFloatConst() const;
	vint32 GetIntConst() const;
	float GetFloatConst() const;
};
