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
	virtual void Emit() = 0;
	virtual VExpression* DoResolve() = 0;
	VExpression* Resolve();
	VExpression* ResolveBoolean();
	virtual VTypeExpr* ResolveAsType();
	virtual void RequestAddressOf();
	void EmitPushPointedCode(TType);
	virtual bool IsSingleName();
	virtual bool GetIntConst(vint32&);
	virtual bool GetFloatConst(float&);
	virtual VExpression* CreateTypeExprCopy();
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
	VExpression* DoResolve();
	void Emit();
	bool GetIntConst(vint32&);
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
	VExpression* DoResolve();
	void Emit();
	bool GetFloatConst(float&);
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
	VExpression* DoResolve();
	void Emit();
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
	VExpression* DoResolve();
	void Emit();
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
	VExpression* DoResolve();
	void Emit();
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
	VExpression* DoResolve();
	void Emit();
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
	VExpression* DoResolve();
	void Emit();
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
	VExpression* DoResolve();
	void Emit();
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
	VExpression* DoResolve();
	VTypeExpr* ResolveAsType();
	void Emit();
	bool IsSingleName();
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
	VExpression* DoResolve();
	void Emit();
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
	VExpression* DoResolve();
	void Emit();
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
	VExpression* DoResolve();
	void Emit();
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
	VExpression* DoResolve();
	void RequestAddressOf();
	void Emit();
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
	VExpression* DoResolve();
	void Emit();
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
	VExpression* DoResolve();
	void Emit();
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
	VExpression* DoResolve();
	void Emit();
};

//==========================================================================
//
//	VUnary
//
//==========================================================================

class VUnary : public VExpression
{
public:
	EPunctuation		Oper;
	VExpression*		op;

	VUnary(EPunctuation, VExpression*, const TLocation&);
	~VUnary();
	VExpression* DoResolve();
	void Emit();
	bool GetIntConst(vint32&);
	bool GetFloatConst(float&);
};

//==========================================================================
//
//	VUnaryMutator
//
//==========================================================================

enum EIncDec
{
	INCDEC_PreInc,
	INCDEC_PreDec,
	INCDEC_PostInc,
	INCDEC_PostDec,
};

class VUnaryMutator : public VExpression
{
public:
	EIncDec			Oper;
	VExpression*	op;

	VUnaryMutator(EIncDec, VExpression*, const TLocation&);
	~VUnaryMutator();
	VExpression* DoResolve();
	void Emit();
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
	VExpression* DoResolve();
	void RequestAddressOf();
	void Emit();
};

//==========================================================================
//
//	VBinary
//
//==========================================================================

class VBinary : public VExpression
{
public:
	EPunctuation		Oper;
	VExpression*		op1;
	VExpression*		op2;

	VBinary(EPunctuation, VExpression*, VExpression*, const TLocation&);
	~VBinary();
	VExpression* DoResolve();
	void Emit();
	bool GetIntConst(vint32&);
	bool GetFloatConst(float&);
};

//==========================================================================
//
//	VBinaryLogical
//
//==========================================================================

class VBinaryLogical : public VExpression
{
public:
	EPunctuation		Oper;
	VExpression*		op1;
	VExpression*		op2;

	VBinaryLogical(EPunctuation, VExpression*, VExpression*, const TLocation&);
	~VBinaryLogical();
	VExpression* DoResolve();
	void Emit();
	bool GetIntConst(vint32&);
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
	VExpression* DoResolve();
	void Emit();
};

//==========================================================================
//
//	VAssignment
//
//==========================================================================

class VAssignment : public VExpression
{
public:
	EPunctuation		Oper;
	VExpression*		op1;
	VExpression*		op2;

	VAssignment(EPunctuation, VExpression*, VExpression*, const TLocation&);
	~VAssignment();
	VExpression* DoResolve();
	void Emit();
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
	VExpression* DoResolve();
	void Emit();
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

	VTypeExpr(TType, const TLocation&);
	VExpression* DoResolve();
	VTypeExpr* ResolveAsType();
	void Emit();
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
	VTypeExpr* ResolveAsType();
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
	VTypeExpr* ResolveAsType();
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

	VExpression* DoResolve();
	void Emit();

	void Declare();
	void EmitInitialisations();
};
