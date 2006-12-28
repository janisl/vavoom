//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: vcc.h 1668 2006-08-19 12:00:48Z dj_jl $
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

class VTypeExpr;

//==========================================================================
//
//	VExpression
//
//==========================================================================

class VExpression
{
public:
	TType		Type;
	TType		RealType;
	int			Flags;
	TLocation	Loc;

	VExpression(const TLocation&);
	virtual ~VExpression();
	virtual VExpression* DoResolve(VEmitContext&) = 0;
	VExpression* Resolve(VEmitContext&);
	VExpression* ResolveBoolean(VEmitContext&);
	virtual VTypeExpr* ResolveAsType(VEmitContext&);
	virtual void RequestAddressOf();
	virtual void Emit(VEmitContext&) = 0;
	virtual void EmitBranchable(VEmitContext&, VLabel, bool);
	void EmitPushPointedCode(TType, VEmitContext&);
	virtual bool IsValidTypeExpression();
	virtual bool IsIntConst() const;
	virtual bool IsFloatConst() const;
	virtual vint32 GetIntConst() const;
	virtual float GetFloatConst() const;
	virtual VExpression* CreateTypeExprCopy();
	virtual bool AddDropResult();
};

//==========================================================================
//
//	VIntLiteral
//
//==========================================================================

class VIntLiteral : public VExpression
{
public:
	vint32		Value;

	VIntLiteral(vint32, const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	bool IsIntConst() const;
	vint32 GetIntConst() const;
};

//==========================================================================
//
//	VFloatLiteral
//
//==========================================================================

class VFloatLiteral : public VExpression
{
public:
	float		Value;

	VFloatLiteral(float, const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	bool IsFloatConst() const;
	float GetFloatConst() const;
};

//==========================================================================
//
//	VNameLiteral
//
//==========================================================================

class VNameLiteral : public VExpression
{
public:
	VName		Value;

	VNameLiteral(VName, const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VStringLiteral
//
//==========================================================================

class VStringLiteral : public VExpression
{
public:
	vint32		Value;

	VStringLiteral(vint32, const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VSelf
//
//==========================================================================

class VSelf : public VExpression
{
public:
	VSelf(const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VNoneLiteral
//
//==========================================================================

class VNoneLiteral : public VExpression
{
public:
	VNoneLiteral(const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VNullLiteral
//
//==========================================================================

class VNullLiteral : public VExpression
{
public:
	VNullLiteral(const TLocation&);
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

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
	VExpression* DoResolve(VEmitContext&);
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
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
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

	VArrayElement(VExpression*, VExpression*, const TLocation&);
	~VArrayElement();
	VExpression* DoResolve(VEmitContext&);
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
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDynamicClassCast
//
//==========================================================================

class VDynamicClassCast : public VExpression
{
public:
	VName				ClassName;
	VExpression*		op;

	VDynamicClassCast(VName, VExpression*, const TLocation&);
	~VDynamicClassCast();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VUnary
//
//==========================================================================

class VUnary : public VExpression
{
public:
	enum EUnaryOp
	{
		Plus,
		Minus,
		Not,
		BitInvert,
		TakeAddress,
	};
	EUnaryOp		Oper;
	VExpression*	op;

	VUnary(EUnaryOp, VExpression*, const TLocation&);
	~VUnary();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	void EmitBranchable(VEmitContext&, VLabel, bool);
};

//==========================================================================
//
//	VUnaryMutator
//
//==========================================================================

class VUnaryMutator : public VExpression
{
public:
	enum EIncDec
	{
		PreInc,
		PreDec,
		PostInc,
		PostDec,
		Inc,
		Dec,
	};
	EIncDec			Oper;
	VExpression*	op;

	VUnaryMutator(EIncDec, VExpression*, const TLocation&);
	~VUnaryMutator();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	bool AddDropResult();
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
//	VBinary
//
//==========================================================================

class VBinary : public VExpression
{
public:
	enum EBinOp
	{
		Add,
		Subtract,
		Multiply,
		Divide,
		Modulus,
		LShift,
		RShift,
		And,
		XOr,
		Or,
		Equals,
		NotEquals,
		Less,
		LessEquals,
		Greater,
		GreaterEquals,
	};
	EBinOp			Oper;
	VExpression*	op1;
	VExpression*	op2;

	VBinary(EBinOp, VExpression*, VExpression*, const TLocation&);
	~VBinary();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VBinaryLogical
//
//==========================================================================

class VBinaryLogical : public VExpression
{
public:
	enum ELogOp
	{
		And,
		Or,
	};
	ELogOp			Oper;
	VExpression*	op1;
	VExpression*	op2;

	VBinaryLogical(ELogOp, VExpression*, VExpression*, const TLocation&);
	~VBinaryLogical();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	void EmitBranchable(VEmitContext&, VLabel, bool);
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
	char		Name[128];
	VName		MetaClassName;

	VTypeExpr(TType, const TLocation&);
	VTypeExpr(TType, const TLocation&, VName);
	VExpression* DoResolve(VEmitContext&);
	VTypeExpr* ResolveAsType(VEmitContext&);
	void Emit(VEmitContext&);
	const char* GetName();
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
