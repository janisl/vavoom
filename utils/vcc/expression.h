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
//	VPointerField
//
//==========================================================================

class VPointerField : public VExpression
{
public:
	VExpression*		op;
	VName				FieldName;

	VPointerField(VExpression*, VName, const TLocation&);
	~VPointerField();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDotField
//
//==========================================================================

class VDotField : public VExpression
{
public:
	VExpression*		op;
	VName				FieldName;

	VDotField(VExpression*, VName, const TLocation&);
	~VDotField();
	VExpression* IntResolve(VEmitContext&, bool);
	VExpression* DoResolve(VEmitContext&);
	VExpression* ResolveAssignmentTarget(VEmitContext&);
	void Emit(VEmitContext&);
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
//	VArrayElement
//
//==========================================================================

class VArrayElement : public VExpression
{
public:
	VExpression*		op;
	VExpression*		ind;
	bool				AddressRequested;
	bool				IsAssign;

	VArrayElement(VExpression*, VExpression*, const TLocation&);
	~VArrayElement();
	VExpression* DoResolve(VEmitContext&);
	VExpression* ResolveAssignmentTarget(VEmitContext&);
	void RequestAddressOf();
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VBaseInvocation
//
//==========================================================================

class VBaseInvocation : public VExpression
{
public:
	VName			Name;
	int				NumArgs;
	VExpression*	Args[MAX_PARAMS + 1];

	VBaseInvocation(VName, int, VExpression**, const TLocation&);
	~VBaseInvocation();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VCastOrInvocation
//
//==========================================================================

class VCastOrInvocation : public VExpression
{
public:
	VName			Name;
	int				NumArgs;
	VExpression*	Args[MAX_PARAMS + 1];

	VCastOrInvocation(VName, const TLocation&, int, VExpression**);
	~VCastOrInvocation();
	VExpression* DoResolve(VEmitContext&);
	VExpression* ResolveIterator(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDotInvocation
//
//==========================================================================

class VDotInvocation : public VExpression
{
public:
	VExpression*	SelfExpr;
	VName			MethodName;
	int				NumArgs;
	VExpression*	Args[MAX_PARAMS + 1];

	VDotInvocation(VExpression*, VName, const TLocation&, int, VExpression**);
	~VDotInvocation();
	VExpression* DoResolve(VEmitContext&);
	VExpression* ResolveIterator(VEmitContext&);
	void Emit(VEmitContext&);
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
//	VAssignment
//
//==========================================================================

class VAssignment : public VExpression
{
public:
	enum EAssignOper
	{
		Assign,
		AddAssign,
		MinusAssign,
		MultiplyAssign,
		DivideAssign,
		ModAssign,
		AndAssign,
		OrAssign,
		XOrAssign,
		LShiftAssign,
		RShiftAssign,
	};
	EAssignOper		Oper;
	VExpression*	op1;
	VExpression*	op2;

	VAssignment(EAssignOper, VExpression*, VExpression*, const TLocation&);
	~VAssignment();
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

//==========================================================================
//
//	VLocalEntry
//
//==========================================================================

class VLocalEntry
{
public:
	VExpression*	TypeExpr;
	VName			Name;
	TLocation		Loc;
	VExpression*	Value;

	VLocalEntry()
	: TypeExpr(NULL)
	, Name(NAME_None)
	, Value(NULL)
	{}
};

//==========================================================================
//
//	VLocalDecl
//
//==========================================================================

class VLocalDecl : public VExpression
{
public:
	TArray<VLocalEntry>	Vars;

	VLocalDecl(const TLocation&);
	~VLocalDecl();

	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);

	void Declare(VEmitContext&);
	void EmitInitialisations(VEmitContext&);
};
