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

enum { MAX_ARG_COUNT = 16 };

class VExpression
{
public:
	TType		Type;
	TType		RealType;
	int			Flags;
	TLocation	Loc;

	VExpression(const TLocation& ALoc);
	virtual ~VExpression();
	virtual void Emit() = 0;
	virtual VExpression* DoResolve() = 0;
	VExpression* Resolve();
	VExpression* ResolveTopLevel();
	virtual void RequestAddressOf();
	void EmitPushPointedCode(TType type);
	virtual bool IsSingleName();
	virtual bool GetIntConst(vint32&);
};

class VIntLiteral : public VExpression
{
public:
	vint32		Value;

	VIntLiteral(vint32 AValue, const TLocation& ALoc);
	VExpression* DoResolve();
	void Emit();
	bool GetIntConst(vint32&);
};

class VFloatLiteral : public VExpression
{
public:
	float		Value;

	VFloatLiteral(float AValue, const TLocation& ALoc);
	VExpression* DoResolve();
	void Emit();
};

class VNameLiteral : public VExpression
{
public:
	VName		Value;

	VNameLiteral(VName AValue, const TLocation& ALoc);
	VExpression* DoResolve();
	void Emit();
};

class VStringLiteral : public VExpression
{
public:
	vint32		Value;

	VStringLiteral(vint32 AValue, const TLocation& ALoc);
	VExpression* DoResolve();
	void Emit();
};

class VSelf : public VExpression
{
public:
	VSelf(const TLocation& ALoc);
	VExpression* DoResolve();
	void Emit();
};

class VNoneLiteral : public VExpression
{
public:
	VNoneLiteral(const TLocation& ALoc);
	VExpression* DoResolve();
	void Emit();
};

class VNullLiteral : public VExpression
{
public:
	VNullLiteral(const TLocation& ALoc);
	VExpression* DoResolve();
	void Emit();
};

class VVector : public VExpression
{
public:
	VExpression*	op1;
	VExpression*	op2;
	VExpression*	op3;

	VVector(VExpression* AOp1, VExpression* AOp2, VExpression* AOp3, const TLocation& ALoc);
	~VVector();
	VExpression* DoResolve();
	void Emit();
};

class VSingleName : public VExpression
{
public:
	VName			Name;

	VSingleName(VName AName, const TLocation& ALoc);
	VExpression* DoResolve();
	void Emit();
	bool IsSingleName();
};

class VDoubleName : public VExpression
{
public:
	VName			Name1;
	VName			Name2;

	VDoubleName(VName AName1, VName AName2, const TLocation& ALoc);
	VExpression* DoResolve();
	void Emit();
};

class VPointerField : public VExpression
{
public:
	VExpression*		op;
	VName				FieldName;

	VPointerField(VExpression* AOp, VName AFieldName, const TLocation& ALoc);
	~VPointerField();
	VExpression* DoResolve();
	void Emit();
};

class VDotField : public VExpression
{
public:
	VExpression*		op;
	VName				FieldName;

	VDotField(VExpression* AOp, VName AFieldName, const TLocation& ALoc);
	~VDotField();
	VExpression* DoResolve();
	void Emit();
};

class VArrayElement : public VExpression
{
public:
	VExpression*		op;
	VExpression*		ind;
	bool				AddressRequested;

	VArrayElement(VExpression* AOp, VExpression* AInd, const TLocation& ALoc);
	~VArrayElement();
	VExpression* DoResolve();
	void RequestAddressOf();
	void Emit();
};

class VBaseInvocation : public VExpression
{
public:
	VName			Name;
	int				NumArgs;
	VExpression*	Args[MAX_ARG_COUNT + 1];

	VBaseInvocation(VName AName, int ANumArgs, VExpression** AArgs,
		const TLocation& ALoc);
	~VBaseInvocation();
	VExpression* DoResolve();
	void Emit();
};

class VCastOrInvocation : public VExpression
{
public:
	VName			Name;
	int				NumArgs;
	VExpression*	Args[MAX_ARG_COUNT + 1];

	VCastOrInvocation(VName AName, const TLocation& ALoc, int ANumArgs,
		VExpression** AArgs);
	~VCastOrInvocation();
	VExpression* DoResolve();
	void Emit();
};

class VDotInvocation : public VExpression
{
public:
	VExpression*	SelfExpr;
	VName			MethodName;
	int				NumArgs;
	VExpression*	Args[MAX_ARG_COUNT + 1];

	VDotInvocation(VExpression* ASelfExpr, VName AMethodName,
		const TLocation& ALoc, int ANumArgs, VExpression** AArgs);
	~VDotInvocation();
	VExpression* DoResolve();
	void Emit();
};

class VUnary : public VExpression
{
public:
	EPunctuation		Oper;	//	Operation
	VExpression*		op;		//	Value

	VUnary(EPunctuation AOper, VExpression* AOp, const TLocation& ALoc);
	~VUnary();
	VExpression* DoResolve();
	void Emit();
	bool GetIntConst(vint32&);
};

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
	EIncDec			Oper;	//	Operation
	VExpression*	op;		//	Value

	VUnaryMutator(EIncDec AOper, VExpression* AOp, const TLocation& ALoc);
	~VUnaryMutator();
	VExpression* DoResolve();
	void Emit();
};

class VPushPointed : public VExpression
{
public:
	VExpression*	op;		//	Value
	bool			AddressRequested;

	VPushPointed(VExpression* AOp);
	~VPushPointed();
	VExpression* DoResolve();
	void RequestAddressOf();
	void Emit();
};

class VBinary : public VExpression
{
public:
	EPunctuation		Oper;	//	Operation
	VExpression*		op1;	//	Variable
	VExpression*		op2;	//	Value

	VBinary(EPunctuation AOper, VExpression* AOp1, VExpression* AOp2, const TLocation& ALoc);
	~VBinary();
	VExpression* DoResolve();
	void Emit();
	bool GetIntConst(vint32&);
};

class VConditional : public VExpression
{
public:
	VExpression*		op;		//	Condition
	VExpression*		op1;	//	Value1
	VExpression*		op2;	//	Value2

	VConditional(VExpression* AOp, VExpression* AOp1, VExpression* AOp2, const TLocation& ALoc);
	~VConditional();
	VExpression* DoResolve();
	void Emit();
};

class VAssignment : public VExpression
{
public:
	EPunctuation		Oper;	//	Operation
	VExpression*		op1;	//	Variable
	VExpression*		op2;	//	Value

	VAssignment(EPunctuation AOper, VExpression* AOp1, VExpression* AOp2, const TLocation& ALoc);
	~VAssignment();
	VExpression* DoResolve();
	void Emit();
};

class VLocalEntry
{
public:
	VName			Name;
	TLocation		Loc;
	VExpression*	Value;
	int				PointerLevel;
	int				ArraySize;

	VLocalEntry()
	: Name(NAME_None)
	, Value(NULL)
	, PointerLevel(0)
	, ArraySize(-1)
	{}
};

class VLocalDecl : public VExpression
{
public:
	TType				BaseType;
	VName				TypeName;
	TArray<VLocalEntry>	Vars;

	VLocalDecl(const TLocation& ALoc);
	~VLocalDecl();

	VExpression* DoResolve();
	void Emit();

	void Declare();
	void EmitInitialisations();
};
