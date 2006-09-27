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

#include "vcc.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

//==========================================================================
//
//	VStateConstant
//
//==========================================================================

class VStateConstant : public VExpression
{
public:
	VState*		State;

	VStateConstant(VState* AState, const TLocation& ALoc);
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

//==========================================================================
//
//	VDynamicCast
//
//==========================================================================

class VDynamicCast : public VExpression
{
public:
	VClass*				Class;
	VExpression*		op;

	VDynamicCast(VClass* AClass, VExpression* AOp, const TLocation& ALoc);
	~VDynamicCast();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VLocalVar
//
//==========================================================================

class VLocalVar : public VExpression
{
public:
	int				num;
	bool			AddressRequested;
	bool			PushOutParam;

	VLocalVar(int ANum, const TLocation& ALoc);
	VExpression* DoResolve(VEmitContext&);
	void RequestAddressOf();
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VFieldAccess
//
//==========================================================================

class VFieldAccess : public VExpression
{
public:
	VExpression*		op;
	VField*				field;
	bool				AddressRequested;

	VFieldAccess(VExpression* AOp, VField* AField, const TLocation& ALoc, int ExtraFlags);
	~VFieldAccess();
	VExpression* DoResolve(VEmitContext&);
	void RequestAddressOf();
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDelegateVal
//
//==========================================================================

class VDelegateVal : public VExpression
{
public:
	VExpression*		op;
	VMethod*			M;

	VDelegateVal(VExpression* AOp, VMethod* AM, const TLocation& ALoc);
	~VDelegateVal();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VInvocation
//
//==========================================================================

class VInvocation : public VExpression
{
public:
	VExpression*	SelfExpr;
	VMethod*		Func;
	VField*			DelegateField;
	bool			HaveSelf;
	bool			BaseCall;
	int				NumArgs;
	VExpression*	Args[MAX_PARAMS + 1];

	VInvocation(VExpression* ASelfExpr, VMethod* AFunc, VField* ADelegateField,
		bool AHaveSelf, bool ABaseCall, const TLocation& ALoc, int ANumArgs,
		VExpression** AArgs);
	~VInvocation();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	void CheckParams();
};

//==========================================================================
//
//	VDelegateToBool
//
//==========================================================================

class VDelegateToBool : public VExpression
{
public:
	VExpression*		op;

	VDelegateToBool(VExpression* AOp);
	~VDelegateToBool();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VStringToBool
//
//==========================================================================

class VStringToBool : public VExpression
{
public:
	VExpression*		op;

	VStringToBool(VExpression* AOp);
	~VStringToBool();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VPointerToBool
//
//==========================================================================

class VPointerToBool : public VExpression
{
public:
	VExpression*		op;

	VPointerToBool(VExpression* AOp);
	~VPointerToBool();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//BEGIN VExpression

//==========================================================================
//
//	VExpression::VExpression
//
//==========================================================================

VExpression::VExpression(const TLocation& ALoc)
: Type(ev_void)
, RealType(ev_void)
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

	switch (e->Type.type)
	{
	case ev_int:
	case ev_float:
	case ev_name:
	case ev_bool:
		break;

	case ev_pointer:
	case ev_reference:
	case ev_classid:
	case ev_state:
		e = new VPointerToBool(e);
		break;

	case ev_string:
		e = new VStringToBool(e);
		break;

	case ev_delegate:
		e = new VDelegateToBool(e);
		break;

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

void VExpression::EmitPushPointedCode(TType type, VEmitContext& ec)
{
	switch (type.type)
	{
	case ev_int:
	case ev_float:
	case ev_name:
	case ev_struct://	Not exactly, but the rest of code will fail without these.
	case ev_array:
		ec.AddStatement(OPC_PushPointed);
		break;

	case ev_pointer:
	case ev_reference:
	case ev_classid:
	case ev_state:
		ec.AddStatement(OPC_PushPointedPtr);
		break;

	case ev_vector:
		ec.AddStatement(OPC_VPushPointed);
		break;

	case ev_bool:
		if (type.bit_mask & 0x000000ff)
			ec.AddStatement(OPC_PushBool0, (int)(type.bit_mask));
		else if (type.bit_mask & 0x0000ff00)
			ec.AddStatement(OPC_PushBool1, (int)(type.bit_mask >> 8));
		else if (type.bit_mask & 0x00ff0000)
			ec.AddStatement(OPC_PushBool2, (int)(type.bit_mask >> 16));
		else
			ec.AddStatement(OPC_PushBool3, (int)(type.bit_mask >> 24));
		break;

	case ev_string:
		ec.AddStatement(OPC_PushPointedStr);
		break;

	case ev_delegate:
		ec.AddStatement(OPC_PushPointedDelegate);
		break;
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
//	VExpression::CreateTypeExprCopy
//
//==========================================================================

VExpression* VExpression::CreateTypeExprCopy()
{
	ParseError(Loc, "Not a type");
	return new VTypeExpr(ev_unknown, Loc);
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

//END

//BEGIN VIntLiteral

//==========================================================================
//
//	VIntLiteral::VIntLiteral
//
//==========================================================================

VIntLiteral::VIntLiteral(vint32 AValue, const TLocation& ALoc)
: VExpression(ALoc)
, Value(AValue)
{
	Type = ev_int;
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

//END

//BEGIN VFloatLiteral

//==========================================================================
//
//	VFloatLiteral::VFloatLiteral
//
//==========================================================================

VFloatLiteral::VFloatLiteral(float AValue, const TLocation& ALoc)
: VExpression(ALoc)
, Value(AValue)
{
	Type = ev_float;
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

//END

//BEGIN VNameLiteral

//==========================================================================
//
//	VNameLiteral::VNameLiteral
//
//==========================================================================

VNameLiteral::VNameLiteral(VName AValue, const TLocation& ALoc)
: VExpression(ALoc)
, Value(AValue)
{
	Type = ev_name;
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

//END

//BEGIN VStringLiteral

//==========================================================================
//
//	VStringLiteral::VStringLiteral
//
//==========================================================================

VStringLiteral::VStringLiteral(vint32 AValue, const TLocation& ALoc)
: VExpression(ALoc)
, Value(AValue)
{
	Type = ev_string;
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

//END

//BEGIN VSelf

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
	Type = TType(ec.SelfClass);
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

//END

//BEGIN VNoneLiteral

//==========================================================================
//
//	VNoneLiteral::VNoneLiteral
//
//==========================================================================

VNoneLiteral::VNoneLiteral(const TLocation& ALoc)
: VExpression(ALoc)
{
	Type = TType((VClass*)NULL);
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

//END

//BEGIN VNullLiteral

//==========================================================================
//
//	VNullLiteral::VNullLiteral
//
//==========================================================================

VNullLiteral::VNullLiteral(const TLocation& ALoc)
: VExpression(ALoc)
{
	Type = TType(ev_void).MakePointerType();
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

//END

//BEGIN VVector

//==========================================================================
//
//	VVector::VVector
//
//==========================================================================

VVector::VVector(VExpression* AOp1, VExpression* AOp2, VExpression* AOp3, const TLocation& ALoc)
: VExpression(ALoc)
, op1(AOp1)
, op2(AOp2)
, op3(AOp3)
{
	if (!op1)
	{
		ParseError(Loc, "Expression expected");
	}
	if (!op2)
	{
		ParseError(Loc, "Expression expected");
	}
	if (!op3)
	{
		ParseError(Loc, "Expression expected");
	}
}

//==========================================================================
//
//	VVector::~VVector
//
//==========================================================================

VVector::~VVector()
{
	if (op1)
		delete op1;
	if (op2)
		delete op2;
	if (op3)
		delete op3;
}

//==========================================================================
//
//	VVector::DoResolve
//
//==========================================================================

VExpression* VVector::DoResolve(VEmitContext& ec)
{
	if (op1)
		op1 = op1->Resolve(ec);
	if (op2)
		op2 = op2->Resolve(ec);
	if (op3)
		op3 = op3->Resolve(ec);
	if (!op1 || !op2 || !op3)
	{
		delete this;
		return NULL;
	}

	if (op1->Type.type != ev_float)
	{
		ParseError(Loc, "Expression type mistmatch, vector param 1 is not a float");
		delete this;
		return NULL;
	}
	if (op2->Type.type != ev_float)
	{
		ParseError(Loc, "Expression type mistmatch, vector param 2 is not a float");
		delete this;
		return NULL;
	}
	if (op3->Type.type != ev_float)
	{
		ParseError(Loc, "Expression type mistmatch, vector param 3 is not a float");
		delete this;
		return NULL;
	}

	Type = ev_vector;
	return this;
}

//==========================================================================
//
//	VVector::Emit
//
//==========================================================================

void VVector::Emit(VEmitContext& ec)
{
	op1->Emit(ec);
	op2->Emit(ec);
	op3->Emit(ec);
}

//END

//BEGIN VSingleName

//==========================================================================
//
//	VSingleName::VSingleName
//
//==========================================================================

VSingleName::VSingleName(VName AName, const TLocation& ALoc)
: VExpression(ALoc)
, Name(AName)
{
}

//==========================================================================
//
//	VSingleName::DoResolve
//
//==========================================================================

VExpression* VSingleName::DoResolve(VEmitContext& ec)
{
	int num = ec.CheckForLocalVar(Name);
	if (num != -1)
	{
		VExpression* e = new VLocalVar(num, Loc);
		delete this;
		return e->Resolve(ec);
	}

	if (ec.SelfClass)
	{
		VConstant* Const = ec.SelfClass->CheckForConstant(Name);
		if (Const)
		{
			VExpression* e = new VConstantValue(Const, Loc);
			delete this;
			return e->Resolve(ec);
		}

		VMethod* M = ec.SelfClass->CheckForMethod(Name);
		if (M)
		{
			VExpression* e = new VDelegateVal((new VSelf(Loc))->Resolve(ec), M, Loc);
			delete this;
			return e->Resolve(ec);
		}

		VField* field = ec.SelfClass->CheckForField(Loc, Name, ec.SelfClass);
		if (field)
		{
			VExpression* e = new VFieldAccess((new VSelf(Loc))->Resolve(ec), field, Loc, 0);
			delete this;
			return e->Resolve(ec);
		}

		VState* State = ec.SelfClass->CheckForState(Name);
		if (State)
		{
			VExpression* e = new VStateConstant(State, Loc);
			delete this;
			return e->Resolve(ec);
		}
	}

	VConstant* Const = ec.Package->CheckForConstant(Name);
	if (Const)
	{
		VExpression* e = new VConstantValue(Const, Loc);
		delete this;
		return e->Resolve(ec);
	}

	VClass* Class = VMemberBase::CheckForClass(Name);
	if (Class)
	{
		VExpression* e = new VClassConstant(Class, Loc);
		delete this;
		return e->Resolve(ec);
	}

	ParseError(Loc, "Illegal expression identifier %s", *Name);
	delete this;
	return NULL;
}

//==========================================================================
//
//	VSingleName::ResolveAsType
//
//==========================================================================

VTypeExpr* VSingleName::ResolveAsType(VEmitContext& ec)
{
	Type = VMemberBase::CheckForType(ec.SelfClass, Name);
	if (Type.type == ev_unknown)
	{
		ParseError(Loc, "Invalid identifier, bad type name %s", *Name);
		delete this;
		return NULL;
	}

	VTypeExpr* e = new VTypeExpr(Type, Loc);
	delete this;
	return e;
}

//==========================================================================
//
//	VSingleName::Emit
//
//==========================================================================

void VSingleName::Emit(VEmitContext&)
{
	ParseError(Loc, "Should not happen");
}

//==========================================================================
//
//	VSingleName::IsValidTypeExpression
//
//==========================================================================

bool VSingleName::IsValidTypeExpression()
{
	return true;
}

//==========================================================================
//
//	VSingleName::CreateTypeExprCopy
//
//==========================================================================

VExpression* VSingleName::CreateTypeExprCopy()
{
	return new VSingleName(Name, Loc);
}

//END

//BEGIN VDoubleName

//==========================================================================
//
//	VDoubleName::VDoubleName
//
//==========================================================================

VDoubleName::VDoubleName(VName AName1, VName AName2, const TLocation& ALoc)
: VExpression(ALoc)
, Name1(AName1)
, Name2(AName2)
{
}

//==========================================================================
//
//	VDoubleName::DoResolve
//
//==========================================================================

VExpression* VDoubleName::DoResolve(VEmitContext& ec)
{
	VClass* Class = VMemberBase::CheckForClass(Name1);
	if (!Class)
	{
		ParseError(Loc, "No such class %s", *Name1);
		delete this;
		return NULL;
	}

	VConstant* Const = Class->CheckForConstant(Name2);
	if (Const)
	{
		VExpression* e = new VConstantValue(Const, Loc);
		delete this;
		return e->Resolve(ec);
	}

	VState* State = Class->CheckForState(Name2);
	if (State)
	{
		VExpression* e = new VStateConstant(State, Loc);
		delete this;
		return e->Resolve(ec);
	}

	ParseError(Loc, "No such constant or state %s", *Name2);
	delete this;
	return NULL;
}

//==========================================================================
//
//	VDoubleName::ResolveAsType
//
//==========================================================================

VTypeExpr* VDoubleName::ResolveAsType(VEmitContext&)
{
	VClass* Class = VMemberBase::CheckForClass(Name1);
	if (!Class)
	{
		ParseError(Loc, "No such class %s", *Name1);
		delete this;
		return NULL;
	}

	Type = VMemberBase::CheckForType(Class, Name2);
	if (Type.type == ev_unknown)
	{
		ParseError(Loc, "Invalid identifier, bad type name %s::%s", *Name1, *Name2);
		delete this;
		return NULL;
	}

	VTypeExpr* e = new VTypeExpr(Type, Loc);
	delete this;
	return e;
}

//==========================================================================
//
//	VDoubleName::Emit
//
//==========================================================================

void VDoubleName::Emit(VEmitContext&)
{
	ParseError(Loc, "Should not happen");
}

//==========================================================================
//
//	VDoubleName::IsValidTypeExpression
//
//==========================================================================

bool VDoubleName::IsValidTypeExpression()
{
	return true;
}

//==========================================================================
//
//	VDoubleName::CreateTypeExprCopy
//
//==========================================================================

VExpression* VDoubleName::CreateTypeExprCopy()
{
	return new VDoubleName(Name1, Name2, Loc);
}

//END

//BEGIN VPointerField

//==========================================================================
//
//	VPointerField::VPointerField
//
//==========================================================================

VPointerField::VPointerField(VExpression* AOp, VName AFieldName, const TLocation& ALoc)
: VExpression(ALoc)
, op(AOp)
, FieldName(AFieldName)
{
}

//==========================================================================
//
//	VPointerField::~VPointerField
//
//==========================================================================

VPointerField::~VPointerField()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VPointerField::DoResolve
//
//==========================================================================

VExpression* VPointerField::DoResolve(VEmitContext& ec)
{
	if (op)
		op = op->Resolve(ec);
	if (!op)
	{
		delete this;
		return NULL;
	}

	if (op->Type.type != ev_pointer)
	{
		ParseError(Loc, "Pointer type required on left side of ->");
		delete this;
		return NULL;
	}
	TType type = op->Type.GetPointerInnerType();
	if (!type.Struct)
	{
		ParseError(Loc, "Not a structure type");
		delete this;
		return NULL;
	}
	VField* field = type.Struct->CheckForField(FieldName);
	if (!field)
	{
		ParseError(Loc, "No such field %s", *FieldName);
		delete this;
		return NULL;
	}
	VExpression* e = new VFieldAccess(op, field, Loc, 0);
	op = NULL;
	delete this;
	return e->Resolve(ec);
}

//==========================================================================
//
//	VPointerField::Emit
//
//==========================================================================

void VPointerField::Emit(VEmitContext&)
{
	ParseError(Loc, "Should not happen");
}

//END

//BEGIN VDotField

//==========================================================================
//
//	VDotField::VDotField
//
//==========================================================================

VDotField::VDotField(VExpression* AOp, VName AFieldName, const TLocation& ALoc)
: VExpression(ALoc)
, op(AOp)
, FieldName(AFieldName)
{
}

//==========================================================================
//
//	VDotField::~VDotField
//
//==========================================================================

VDotField::~VDotField()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VDotField::DoResolve
//
//==========================================================================

VExpression* VDotField::DoResolve(VEmitContext& ec)
{
	if (op)
		op = op->Resolve(ec);
	if (!op)
	{
		delete this;
		return NULL;
	}

	if (op->Type.type == ev_reference)
	{
		VMethod* M = op->Type.Class->CheckForMethod(FieldName);
		if (M)
		{
			VExpression* e = new VDelegateVal(op, M, Loc);
			op = NULL;
			delete this;
			return e->Resolve(ec);
		}
		else
		{
			VField* field = op->Type.Class->CheckForField(Loc, FieldName, ec.SelfClass);
			if (!field)
			{
				ParseError(Loc, "No such field %s", *FieldName);
				delete this;
				return NULL;
			}
			VExpression* e = new VFieldAccess(op, field, Loc, 0);
			op = NULL;
			delete this;
			return e->Resolve(ec);
		}
	}
	else if (op->Type.type == ev_struct || op->Type.type == ev_vector)
	{
		TType type = op->Type;
		int Flags = op->Flags;
		op->Flags &= ~FIELD_ReadOnly;
		op->RequestAddressOf();
		VField* field = type.Struct->CheckForField(FieldName);
		if (!field)
		{
			ParseError(Loc, "No such field %s", *FieldName);
			delete this;
			return NULL;
		}
		VExpression* e = new VFieldAccess(op, field, Loc, Flags & FIELD_ReadOnly);
		op = NULL;
		delete this;
		return e->Resolve(ec);
	}
	ParseError(Loc, "Reference, struc or vector expected on left side of .");
	delete this;
	return NULL;
}

//==========================================================================
//
//	VDotField::Emit
//
//==========================================================================

void VDotField::Emit(VEmitContext&)
{
	ParseError(Loc, "Should not happen");
}

//END

//BEGIN VArrayElement

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
		delete op;
	if (ind)
		delete ind;
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

	if (ind->Type.type != ev_int)
	{
		ParseError(Loc, "Array index must be of integer type");
		delete this;
		return NULL;
	}
	if (op->Type.type == ev_array)
	{
		Flags = op->Flags;
		Type = op->Type.GetArrayInnerType();
		op->Flags &= ~FIELD_ReadOnly;
		op->RequestAddressOf();
	}
	else if (op->Type.type == ev_pointer)
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
	if (Type.type == ev_bool)
	{
		Type = TType(ev_int);
	}
	return this;
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
	ec.AddStatement(OPC_ArrayElement, RealType);
	if (!AddressRequested)
	{
		EmitPushPointedCode(RealType, ec);
	}
}

//END

//BEGIN VBaseInvocation

//==========================================================================
//
//	VBaseInvocation::VBaseInvocation
//
//==========================================================================

VBaseInvocation::VBaseInvocation(VName AName, int ANumArgs, VExpression** AArgs,
	const TLocation& ALoc)
: VExpression(ALoc)
, Name(AName)
, NumArgs(ANumArgs)
{
	for (int i = 0; i < NumArgs; i++)
		Args[i] = AArgs[i];
}

//==========================================================================
//
//	VBaseInvocation::~VBaseInvocation
//
//==========================================================================

VBaseInvocation::~VBaseInvocation()
{
	for (int i = 0; i < NumArgs; i++)
		delete Args[i];
}

//==========================================================================
//
//	VBaseInvocation::DoResolve
//
//==========================================================================

VExpression* VBaseInvocation::DoResolve(VEmitContext& ec)
{
	if (!ec.SelfClass)
	{
		ParseError(Loc, ":: not in method");
		delete this;
		return NULL;
	}
	VMethod* Func = ec.SelfClass->ParentClass->CheckForMethod(Name);
	if (!Func)
	{
		ParseError(Loc, "No such method %s", *Name);
		delete this;
		return NULL;
	}

	VExpression* e = new VInvocation(NULL, Func, NULL, false,
		true, Loc, NumArgs, Args);
	NumArgs = 0;
	delete this;
	return e->Resolve(ec);
}

//==========================================================================
//
//	VBaseInvocation::Emit
//
//==========================================================================

void VBaseInvocation::Emit(VEmitContext&)
{
	ParseError(Loc, "Should not happen");
}

//END

//BEGIN VCastOrInvocation

//==========================================================================
//
//	VCastOrInvocation::VCastOrInvocation
//
//==========================================================================

VCastOrInvocation::VCastOrInvocation(VName AName, const TLocation& ALoc, int ANumArgs,
	VExpression** AArgs)
: VExpression(ALoc)
, Name(AName)
, NumArgs(ANumArgs)
{
	for (int i = 0; i < NumArgs; i++)
		Args[i] = AArgs[i];
}

//==========================================================================
//
//	VCastOrInvocation::~VCastOrInvocation
//
//==========================================================================

VCastOrInvocation::~VCastOrInvocation()
{
	for (int i = 0; i < NumArgs; i++)
		if (Args[i])
			delete Args[i];
}

//==========================================================================
//
//	VCastOrInvocation::DoResolve
//
//==========================================================================

VExpression* VCastOrInvocation::DoResolve(VEmitContext& ec)
{
	VClass* Class = VMemberBase::CheckForClass(Name);
	if (Class)
	{
		if (NumArgs != 1 || !Args[0])
		{
			ParseError(Loc, "Dynamic cast requires 1 argument");
			delete this;
			return NULL;
		}
		VExpression* e = new VDynamicCast(Class, Args[0], Loc);
		NumArgs = 0;
		delete this;
		return e->Resolve(ec);
	}

	if (ec.SelfClass)
	{
		VMethod* M = ec.SelfClass->CheckForMethod(Name);
		if (M)
		{
			VExpression* e = new VInvocation(NULL, M, NULL,
				false, false, Loc, NumArgs, Args);
			NumArgs = 0;
			delete this;
			return e->Resolve(ec);
		}
		VField* field = ec.SelfClass->CheckForField(Loc, Name, ec.SelfClass);
		if (field && field->type.type == ev_delegate)
		{
			VExpression* e = new VInvocation(NULL, field->func, field,
				false, false, Loc, NumArgs, Args);
			NumArgs = 0;
			delete this;
			return e->Resolve(ec);
		}
	}

	ParseError(Loc, "Unknown method %s", *Name);
	delete this;
	return NULL;
}

//==========================================================================
//
//	VCastOrInvocation::Emit
//
//==========================================================================

void VCastOrInvocation::Emit(VEmitContext&)
{
	ParseError(Loc, "Should not happen");
}

//END

//BEGIN VDotInvocation

//==========================================================================
//
//	VDotInvocation::VDotInvocation
//
//==========================================================================

VDotInvocation::VDotInvocation(VExpression* ASelfExpr, VName AMethodName,
	const TLocation& ALoc, int ANumArgs, VExpression** AArgs)
: VExpression(ALoc)
, SelfExpr(ASelfExpr)
, MethodName(AMethodName)
, NumArgs(ANumArgs)
{
	for (int i = 0; i < NumArgs; i++)
		Args[i] = AArgs[i];
}

//==========================================================================
//
//	VDotInvocation::~VDotInvocation
//
//==========================================================================

VDotInvocation::~VDotInvocation()
{
	if (SelfExpr)
		delete SelfExpr;
	for (int i = 0; i < NumArgs; i++)
		delete Args[i];
}

//==========================================================================
//
//	VDotInvocation::DoResolve
//
//==========================================================================

VExpression* VDotInvocation::DoResolve(VEmitContext& ec)
{
	if (SelfExpr)
		SelfExpr = SelfExpr->Resolve(ec);
	if (!SelfExpr)
	{
		delete this;
		return NULL;
	}

	if (SelfExpr->Type.type != ev_reference)
	{
		ParseError(Loc, "Object reference expected left side of .");
		delete this;
		return NULL;
	}

	VMethod* M = SelfExpr->Type.Class->CheckForMethod(MethodName);
	if (M)
	{
		VExpression* e = new VInvocation(SelfExpr, M, NULL, true,
			false, Loc, NumArgs, Args);
		SelfExpr = NULL;
		NumArgs = 0;
		delete this;
		return e->Resolve(ec);
	}

	VField* field = SelfExpr->Type.Class->CheckForField(Loc, MethodName,
		ec.SelfClass);
	if (field && field->type.type == ev_delegate)
	{
		VExpression* e = new VInvocation(SelfExpr, field->func, field, true,
			false, Loc, NumArgs, Args);
		SelfExpr = NULL;
		NumArgs = 0;
		delete this;
		return e->Resolve(ec);
	}

	ParseError(Loc, "No such method %s", *MethodName);
	delete this;
	return NULL;
}

//==========================================================================
//
//	VDotInvocation::Emit
//
//==========================================================================

void VDotInvocation::Emit(VEmitContext&)
{
	ParseError(Loc, "Should not happen");
}

//END

//BEGIN VUnary

//==========================================================================
//
//	VUnary::VUnary
//
//==========================================================================

VUnary::VUnary(VUnary::EUnaryOp AOper, VExpression* AOp, const TLocation& ALoc)
: VExpression(ALoc)
, Oper(AOper)
, op(AOp)
{
	if (!op)
	{
		ParseError(Loc, "Expression expected");
		return;
	}
}

//==========================================================================
//
//	VUnary::~VUnary
//
//==========================================================================

VUnary::~VUnary()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VUnary::DoResolve
//
//==========================================================================

VExpression* VUnary::DoResolve(VEmitContext& ec)
{
	if (op)
	{
		if (Oper == Not)
			op = op->ResolveBoolean(ec);
		else
			op = op->Resolve(ec);
	}
	if (!op)
	{
		delete this;
		return NULL;
	}

	switch (Oper)
	{
	case Plus:
		Type = op->Type;
		if (op->Type.type != ev_int && op->Type.type != ev_float)
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		else
		{
			VExpression* e = op;
			op = NULL;
			delete this;
			return e;
		}

	case Minus:
		if (op->Type.type == ev_int)
		{
			Type = ev_int;
		}
		else if (op->Type.type == ev_float)
		{
			Type = ev_float;
		}
		else if (op->Type.type == ev_vector)
		{
			Type = op->Type;
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		break;

	case Not:
		Type = ev_int;
		break;

	case BitInvert:
		if (op->Type.type != ev_int)
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		Type = ev_int;
		break;

	case TakeAddress:
		if (op->Type.type == ev_reference)
		{
			ParseError(Loc, "Tried to take address of reference");
			delete this;
			return NULL;
		}
		else
		{
			op->RequestAddressOf();
			Type = op->RealType.MakePointerType();
		}
		break;
	}

	//	Optimise integer constants.
	if (op->IsIntConst())
	{
		vint32 Value = op->GetIntConst();
		VExpression* e = NULL;
		switch (Oper)
		{
		case Minus:
			e = new VIntLiteral(-Value, Loc);
			break;

		case Not:
			e = new VIntLiteral(!Value, Loc);
			break;

		case BitInvert:
			e = new VIntLiteral(~Value, Loc);
			break;

		default:
			break;
		}
		if (e)
		{
			delete this;
			return e;
		}
	}

	//	Optimise float constants.
	if (op->IsFloatConst() && Oper == Minus)
	{
		float Value = op->GetFloatConst();
		VExpression* e = new VFloatLiteral(-Value, Loc);
		delete this;
		return e;
	}

	return this;
}

//==========================================================================
//
//	VUnary::Emit
//
//==========================================================================

void VUnary::Emit(VEmitContext& ec)
{
	op->Emit(ec);

	switch (Oper)
	{
	case Plus:
		break;

	case Minus:
		if (op->Type.type == ev_int)
		{
			ec.AddStatement(OPC_UnaryMinus);
		}
		else if (op->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FUnaryMinus);
		}
		else if (op->Type.type == ev_vector)
		{
			ec.AddStatement(OPC_VUnaryMinus);
		}
		break;

	case Not:
		ec.AddStatement(OPC_NegateLogical);
		break;

	case BitInvert:
		ec.AddStatement(OPC_BitInverse);
		break;

	case TakeAddress:
		break;
	}
}

//==========================================================================
//
//	VUnary::EmitBranchable
//
//==========================================================================

void VUnary::EmitBranchable(VEmitContext& ec, VLabel Lbl, bool OnTrue)
{
	if (Oper == Not)
	{
		op->EmitBranchable(ec, Lbl, !OnTrue);
	}
	else
	{
		VExpression::EmitBranchable(ec, Lbl, OnTrue);
	}
}

//END

//BEGIN VUnaryMutator

//==========================================================================
//
//	VUnaryMutator::VUnaryMutator
//
//==========================================================================

VUnaryMutator::VUnaryMutator(EIncDec AOper, VExpression* AOp, const TLocation& ALoc)
: VExpression(ALoc)
, Oper(AOper)
, op(AOp)
{
	if (!op)
	{
		ParseError(Loc, "Expression expected");
		return;
	}
}

//==========================================================================
//
//	VUnaryMutator::~VUnaryMutator
//
//==========================================================================

VUnaryMutator::~VUnaryMutator()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VUnaryMutator::DoResolve
//
//==========================================================================

VExpression* VUnaryMutator::DoResolve(VEmitContext& ec)
{
	if (op)
		op = op->Resolve(ec);
	if (!op)
	{
		delete this;
		return NULL;
	}

	if (op->Type.type != ev_int)
	{
		ParseError(Loc, "Expression type mistmatch");
		delete this;
		return NULL;
	}
	Type = ev_int;
	op->RequestAddressOf();
	return this;
}

//==========================================================================
//
//	VUnaryMutator::Emit
//
//==========================================================================

void VUnaryMutator::Emit(VEmitContext& ec)
{
	op->Emit(ec);
	switch (Oper)
	{
	case PreInc:
		ec.AddStatement(OPC_PreInc);
		break;

	case PreDec:
		ec.AddStatement(OPC_PreDec);
		break;

	case PostInc:
		ec.AddStatement(OPC_PostInc);
		break;

	case PostDec:
		ec.AddStatement(OPC_PostDec);
		break;

	case Inc:
		ec.AddStatement(OPC_IncDrop);
		break;

	case Dec:
		ec.AddStatement(OPC_DecDrop);
		break;
	}
}

//==========================================================================
//
//	VUnaryMutator::AddDropResult
//
//==========================================================================

bool VUnaryMutator::AddDropResult()
{
	switch (Oper)
	{
	case PreInc:
	case PostInc:
		Oper = Inc;
		break;

	case PreDec:
	case PostDec:
		Oper = Dec;
		break;

	case Inc:
	case Dec:
		FatalError("Should not happen");
	}
	Type = ev_void;
	return true;
}

//END

//BEGIN VPushPointed

//==========================================================================
//
//	VPushPointed::VPushPointed
//
//==========================================================================

VPushPointed::VPushPointed(VExpression* AOp)
: VExpression(AOp->Loc)
, op(AOp)
, AddressRequested(false)
{
	if (!op)
	{
		ParseError(Loc, "Expression expected");
		return;
	}
}

//==========================================================================
//
//	VPushPointed::~VPushPointed
//
//==========================================================================

VPushPointed::~VPushPointed()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VPushPointed::DoResolve
//
//==========================================================================

VExpression* VPushPointed::DoResolve(VEmitContext& ec)
{
	if (op)
		op = op->Resolve(ec);
	if (!op)
	{
		delete this;
		return NULL;
	}

	if (op->Type.type != ev_pointer)
	{
		ParseError(Loc, "Expression syntax error");
		delete this;
		return NULL;
	}
	Type = op->Type.GetPointerInnerType();
	RealType = Type;
	if (Type.type == ev_bool)
	{
		Type = TType(ev_int);
	}
	return this;
}

//==========================================================================
//
//	VPushPointed::RequestAddressOf
//
//==========================================================================

void VPushPointed::RequestAddressOf()
{
	if (RealType.type == ev_void)
	{
		ParseError(Loc, "Bad address operation");
		return;
	}
	if (AddressRequested)
		ParseError(Loc, "Multiple address of");
	AddressRequested = true;
}

//==========================================================================
//
//	VPushPointed::Emit
//
//==========================================================================

void VPushPointed::Emit(VEmitContext& ec)
{
	op->Emit(ec);
	if (!AddressRequested)
	{
		EmitPushPointedCode(RealType, ec);
	}
}

//END

//BEGIN VBinary

//==========================================================================
//
//	VBinary::VBinary
//
//==========================================================================

VBinary::VBinary(EBinOp AOper, VExpression* AOp1, VExpression* AOp2, const TLocation& ALoc)
: VExpression(ALoc)
, Oper(AOper)
, op1(AOp1)
, op2(AOp2)
{
	if (!op2)
	{
		ParseError(Loc, "Expression expected");
		return;
	}
}

//==========================================================================
//
//	VBinary::~VBinary
//
//==========================================================================

VBinary::~VBinary()
{
	if (op1)
		delete op1;
	if (op2)
		delete op2;
}

//==========================================================================
//
//	VBinary::DoResolve
//
//==========================================================================

VExpression* VBinary::DoResolve(VEmitContext& ec)
{
	if (op1)
		op1 = op1->Resolve(ec);
	if (op2)
		op2 = op2->Resolve(ec);
	if (!op1 || !op2)
	{
		delete this;
		return NULL;
	}

	switch (Oper)
	{
	case Add:
	case Subtract:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			Type = ev_int;
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			Type = ev_float;
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_vector)
		{
			Type = ev_vector;
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		break;
	case Multiply:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			Type = ev_int;
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			Type = ev_float;
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_float)
		{
			Type = ev_vector;
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_vector)
		{
			Type = ev_vector;
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		break;
	case Divide:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			Type = ev_int;
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			Type = ev_float;
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_float)
		{
			Type = ev_vector;
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		break;
	case Modulus:
	case LShift:
	case RShift:
	case And:
	case XOr:
	case Or:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			Type = ev_int;
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		break;
	case Less:
	case LessEquals:
	case Greater:
	case GreaterEquals:
		if (!(op1->Type.type == ev_int && op2->Type.type == ev_int) &&
			!(op1->Type.type == ev_float && op2->Type.type == ev_float))
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		Type = ev_int;
		break;
	case Equals:
	case NotEquals:
		if (!(op1->Type.type == ev_int && op2->Type.type == ev_int) &&
			!(op1->Type.type == ev_float && op2->Type.type == ev_float) &&
			!(op1->Type.type == ev_name && op2->Type.type == ev_name) &&
			!(op1->Type.type == ev_pointer && op2->Type.type == ev_pointer) &&
			!(op1->Type.type == ev_vector && op2->Type.type == ev_vector) &&
			!(op1->Type.type == ev_classid && op2->Type.type == ev_classid) &&
			!(op1->Type.type == ev_state && op2->Type.type == ev_state) &&
			!(op1->Type.type == ev_reference && op2->Type.type == ev_reference))
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		Type = ev_int;
		break;
	}

	//	Optimise integer constants
	if (op1->IsIntConst() && op2->IsIntConst())
	{
		vint32 Value1 = op1->GetIntConst();
		vint32 Value2 = op2->GetIntConst();
		VExpression* e = NULL;
		switch (Oper)
		{
		case Add:
			e = new VIntLiteral(Value1 + Value2, Loc);
			break;

		case Subtract:
			e = new VIntLiteral(Value1 - Value2, Loc);
			break;

		case Multiply:
			e = new VIntLiteral(Value1 * Value2, Loc);
			break;

		case Divide:
			if (!Value2)
			{
				ParseError(Loc, "Division by 0");
				delete this;
				return NULL;
			}
			e = new VIntLiteral(Value1 / Value2, Loc);
			break;

		case Modulus:
			if (!Value2)
			{
				ParseError(Loc, "Division by 0");
				delete this;
				return NULL;
			}
			e = new VIntLiteral(Value1 % Value2, Loc);
			break;

		case LShift:
			e = new VIntLiteral(Value1 << Value2, Loc);
			break;

		case RShift:
			e = new VIntLiteral(Value1 >> Value2, Loc);
			break;

		case Less:
			e = new VIntLiteral(Value1 < Value2, Loc);
			break;

		case LessEquals:
			e = new VIntLiteral(Value1 <= Value2, Loc);
			break;

		case Greater:
			e = new VIntLiteral(Value1 > Value2, Loc);
			break;

		case GreaterEquals:
			e = new VIntLiteral(Value1 >= Value2, Loc);
			break;

		case Equals:
			e = new VIntLiteral(Value1 == Value2, Loc);
			break;

		case NotEquals:
			e = new VIntLiteral(Value1 != Value2, Loc);
			break;

		case And:
			e = new VIntLiteral(Value1 & Value2, Loc);
			break;

		case XOr:
			e = new VIntLiteral(Value1 ^ Value2, Loc);
			break;

		case Or:
			e = new VIntLiteral(Value1 | Value2, Loc);
			break;

		default:
			break;
		}
		if (e)
		{
			delete this;
			return e;
		}
	}

	//	Optimise float constants.
	if (op1->IsFloatConst() && op2->IsFloatConst())
	{
		float Value1 = op1->GetFloatConst();
		float Value2 = op2->GetFloatConst();
		VExpression* e = NULL;
		switch (Oper)
		{
		case Add:
			e = new VFloatLiteral(Value1 + Value2, Loc);
			break;
	
		case Subtract:
			e = new VFloatLiteral(Value1 - Value2, Loc);
			break;
	
		case Multiply:
			e = new VFloatLiteral(Value1 * Value2, Loc);
			break;
	
		case Divide:
			if (!Value2)
			{
				ParseError(Loc, "Division by 0");
				delete this;
				return NULL;
			}
			e = new VFloatLiteral(Value1 / Value2, Loc);
			break;
	
		default:
			break;
		}
		if (e)
		{
			delete this;
			return e;
		}
	}

	return this;
}

//==========================================================================
//
//	VBinary::Emit
//
//==========================================================================

void VBinary::Emit(VEmitContext& ec)
{
	op1->Emit(ec);
	op2->Emit(ec);

	switch (Oper)
	{
	case Add:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_Add);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FAdd);
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_vector)
		{
			ec.AddStatement(OPC_VAdd);
		}
		break;

	case Subtract:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_Subtract);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FSubtract);
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_vector)
		{
			ec.AddStatement(OPC_VSubtract);
		}
		break;

	case Multiply:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_Multiply);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FMultiply);
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_VPostScale);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_vector)
		{
			ec.AddStatement(OPC_VPreScale);
		}
		break;

	case Divide:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_Divide);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FDivide);
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_VIScale);
		}
		break;

	case Modulus:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_Modulus);
		}
		break;

	case LShift:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_LShift);
		}
		break;

	case RShift:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_RShift);
		}
		break;

	case Less:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_Less);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FLess);
		}
		break;

	case LessEquals:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_LessEquals);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FLessEquals);
		}
		break;

	case Greater:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_Greater);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FGreater);
		}
		break;

	case GreaterEquals:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_GreaterEquals);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FGreaterEquals);
		}
		break;

	case Equals:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_Equals);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FEquals);
		}
		else if (op1->Type.type == ev_name && op2->Type.type == ev_name)
		{
			ec.AddStatement(OPC_Equals);
		}
		else if (op1->Type.type == ev_pointer && op2->Type.type == ev_pointer)
		{
			ec.AddStatement(OPC_PtrEquals);
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_vector)
		{
			ec.AddStatement(OPC_VEquals);
		}
		else if (op1->Type.type == ev_classid && op2->Type.type == ev_classid)
		{
			ec.AddStatement(OPC_PtrEquals);
		}
		else if (op1->Type.type == ev_state && op2->Type.type == ev_state)
		{
			ec.AddStatement(OPC_PtrEquals);
		}
		else if (op1->Type.type == ev_reference && op2->Type.type == ev_reference)
		{
			ec.AddStatement(OPC_PtrEquals);
		}
		break;

	case NotEquals:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_NotEquals);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FNotEquals);
		}
		else if (op1->Type.type == ev_name && op2->Type.type == ev_name)
		{
			ec.AddStatement(OPC_NotEquals);
		}
		else if (op1->Type.type == ev_pointer && op2->Type.type == ev_pointer)
		{
			ec.AddStatement(OPC_PtrNotEquals);
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_vector)
		{
			ec.AddStatement(OPC_VNotEquals);
		}
		else if (op1->Type.type == ev_classid && op2->Type.type == ev_classid)
		{
			ec.AddStatement(OPC_PtrNotEquals);
		}
		else if (op1->Type.type == ev_state && op2->Type.type == ev_state)
		{
			ec.AddStatement(OPC_PtrNotEquals);
		}
		else if (op1->Type.type == ev_reference && op2->Type.type == ev_reference)
		{
			ec.AddStatement(OPC_PtrNotEquals);
		}
		break;

	case And:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_AndBitwise);
		}
		break;

	case XOr:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_XOrBitwise);
		}
		break;

	case Or:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_OrBitwise);
		}
		break;
	}
}

//END

//BEGIN VBinaryLogical

//==========================================================================
//
//	VBinaryLogical::VBinaryLogical
//
//==========================================================================

VBinaryLogical::VBinaryLogical(ELogOp AOper, VExpression* AOp1,
	VExpression* AOp2, const TLocation& ALoc)
: VExpression(ALoc)
, Oper(AOper)
, op1(AOp1)
, op2(AOp2)
{
	if (!op2)
	{
		ParseError(Loc, "Expression expected");
		return;
	}
}

//==========================================================================
//
//	VBinaryLogical::~VBinaryLogical
//
//==========================================================================

VBinaryLogical::~VBinaryLogical()
{
	if (op1)
		delete op1;
	if (op2)
		delete op2;
}

//==========================================================================
//
//	VBinaryLogical::DoResolve
//
//==========================================================================

VExpression* VBinaryLogical::DoResolve(VEmitContext& ec)
{
	if (op1)
		op1 = op1->ResolveBoolean(ec);
	if (op2)
		op2 = op2->ResolveBoolean(ec);
	if (!op1 || !op2)
	{
		delete this;
		return NULL;
	}

	Type = ev_int;

	//	Optimise constant cases.
	if (op1->IsIntConst() && op2->IsIntConst())
	{
		vint32 Value1 = op1->GetIntConst();
		vint32 Value2 = op2->GetIntConst();
		VExpression* e = NULL;
		switch (Oper)
		{
		case And:
			e = new VIntLiteral(Value1 && Value2, Loc);
			break;

		case Or:
			e = new VIntLiteral(Value1 || Value2, Loc);
			break;
		}
		if (e)
		{
			delete this;
			return e;
		}
	}

	return this;
}

//==========================================================================
//
//	VBinaryLogical::Emit
//
//==========================================================================

void VBinaryLogical::Emit(VEmitContext& ec)
{
	VLabel Push01 = ec.DefineLabel();
	VLabel End = ec.DefineLabel();

	op1->EmitBranchable(ec, Push01, Oper == Or);

	op2->Emit(ec);
	ec.AddStatement(OPC_Goto, End);

	ec.MarkLabel(Push01);
	ec.AddStatement(Oper == And ? OPC_PushNumber0 : OPC_PushNumber1);

	ec.MarkLabel(End);
}

//==========================================================================
//
//	VBinaryLogical::EmitBranchable
//
//==========================================================================

void VBinaryLogical::EmitBranchable(VEmitContext& ec, VLabel Lbl, bool OnTrue)
{
	switch (Oper)
	{
	case And:
		if (OnTrue)
		{
			VLabel End = ec.DefineLabel();
			op1->EmitBranchable(ec, End, false);
			op2->EmitBranchable(ec, Lbl, true);
			ec.MarkLabel(End);
		}
		else
		{
			op1->EmitBranchable(ec, Lbl, false);
			op2->EmitBranchable(ec, Lbl, false);
		}
		break;

	case Or:
		if (OnTrue)
		{
			op1->EmitBranchable(ec, Lbl, true);
			op2->EmitBranchable(ec, Lbl, true);
		}
		else
		{
			VLabel End = ec.DefineLabel();
			op1->EmitBranchable(ec, End, true);
			op2->EmitBranchable(ec, Lbl, false);
			ec.MarkLabel(End);
		}
		break;
	}
}

//END

//BEGIN VConditional

//==========================================================================
//
//	VConditional::VConditional
//
//==========================================================================

VConditional::VConditional(VExpression* AOp, VExpression* AOp1, VExpression* AOp2, const TLocation& ALoc)
: VExpression(ALoc)
, op(AOp)
, op1(AOp1)
, op2(AOp2)
{
	if (!op1)
	{
		ParseError(Loc, "Expression expected");
		return;
	}
	if (!op2)
	{
		ParseError(Loc, "Expression expected");
		return;
	}
}

//==========================================================================
//
//	VConditional::~VConditional
//
//==========================================================================

VConditional::~VConditional()
{
	if (op)
		delete op;
	if (op1)
		delete op1;
	if (op2)
		delete op2;
}

//==========================================================================
//
//	VConditional::DoResolve
//
//==========================================================================

VExpression* VConditional::DoResolve(VEmitContext& ec)
{
	if (op)
		op = op->ResolveBoolean(ec);
	if (op1)
		op1 = op1->Resolve(ec);
	if (op2)
		op2 = op2->Resolve(ec);
	if (!op || !op1 || !op2)
	{
		delete this;
		return NULL;
	}

	op1->Type.CheckMatch(Loc, op2->Type);
	if (op1->Type.type == ev_pointer && op1->Type.InnerType == ev_void)
		Type = op2->Type;
	else
		Type = op1->Type;
	return this;
}

//==========================================================================
//
//	VConditional::Emit
//
//==========================================================================

void VConditional::Emit(VEmitContext& ec)
{
	VLabel FalseTarget = ec.DefineLabel();
	VLabel End = ec.DefineLabel();

	op->EmitBranchable(ec, FalseTarget, false);
	op1->Emit(ec);
	ec.AddStatement(OPC_Goto, End);
	ec.MarkLabel(FalseTarget);
	op2->Emit(ec);
	ec.MarkLabel(End);
}

//END

//BEGIN VAssignment

//==========================================================================
//
//	VAssignment::VAssignment
//
//==========================================================================

VAssignment::VAssignment(VAssignment::EAssignOper AOper, VExpression* AOp1,
	VExpression* AOp2, const TLocation& ALoc)
: VExpression(ALoc)
, Oper(AOper)
, op1(AOp1)
, op2(AOp2)
{
	if (!op2)
	{
		ParseError(Loc, "Expression required on the right side of assignment operator");
		return;
	}
}

//==========================================================================
//
//	VAssignment::~VAssignment
//
//==========================================================================

VAssignment::~VAssignment()
{
	if (op1)
		delete op1;
	if (op2)
		delete op2;
}

//==========================================================================
//
//	VAssignment::DoResolve
//
//==========================================================================

VExpression* VAssignment::DoResolve(VEmitContext& ec)
{
	if (op1)
		op1 = op1->Resolve(ec);
	if (op2)
		op2 = op2->Resolve(ec);
	if (!op1 || !op2)
	{
		delete this;
		return NULL;
	}

	op2->Type.CheckMatch(Loc, op1->RealType);
	op1->RequestAddressOf();
	return this;
}

//==========================================================================
//
//	VAssignment::Emit
//
//==========================================================================

void VAssignment::Emit(VEmitContext& ec)
{
	op1->Emit(ec);
	op2->Emit(ec);

	switch (Oper)
	{
	case Assign:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_AssignDrop);
		}
		else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_AssignDrop);
		}
		else if (op1->RealType.type == ev_name && op2->Type.type == ev_name)
		{
			ec.AddStatement(OPC_AssignDrop);
		}
		else if (op1->RealType.type == ev_string && op2->Type.type == ev_string)
		{
			ec.AddStatement(OPC_AssignStrDrop);
		}
		else if (op1->RealType.type == ev_pointer && op2->Type.type == ev_pointer)
		{
			ec.AddStatement(OPC_AssignPtrDrop);
		}
		else if (op1->RealType.type == ev_vector && op2->Type.type == ev_vector)
		{
			ec.AddStatement(OPC_VAssignDrop);
		}
		else if (op1->RealType.type == ev_classid && (op2->Type.type == ev_classid ||
			(op2->Type.type == ev_reference && op2->Type.Class == NULL)))
		{
			ec.AddStatement(OPC_AssignPtrDrop);
		}
		else if (op1->RealType.type == ev_state && (op2->Type.type == ev_state ||
			(op2->Type.type == ev_reference && op2->Type.Class == NULL)))
		{
			ec.AddStatement(OPC_AssignPtrDrop);
		}
		else if (op1->RealType.type == ev_reference && op2->Type.type == ev_reference)
		{
			ec.AddStatement(OPC_AssignPtrDrop);
		}
		else if (op1->RealType.type == ev_bool && op2->Type.type == ev_int)
		{
			if (op1->RealType.bit_mask & 0x000000ff)
				ec.AddStatement(OPC_AssignBool0, (int)op1->RealType.bit_mask);
			else if (op1->RealType.bit_mask & 0x0000ff00)
				ec.AddStatement(OPC_AssignBool1, (int)(op1->RealType.bit_mask >> 8));
			else if (op1->RealType.bit_mask & 0x00ff0000)
				ec.AddStatement(OPC_AssignBool2, (int)(op1->RealType.bit_mask >> 16));
			else
				ec.AddStatement(OPC_AssignBool3, (int)(op1->RealType.bit_mask >> 24));
		}
		else if (op1->RealType.type == ev_delegate && op2->Type.type == ev_delegate)
		{
			ec.AddStatement(OPC_AssignDelegate);
		}
		else if (op1->RealType.type == ev_delegate && op2->Type.type == ev_reference && op2->Type.Class == NULL)
		{
			ec.AddStatement(OPC_PushNull);
			ec.AddStatement(OPC_AssignDelegate);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case AddAssign:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_AddVarDrop);
		}
		else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FAddVarDrop);
		}
		else if (op1->RealType.type == ev_vector && op2->Type.type == ev_vector)
		{
			ec.AddStatement(OPC_VAddVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case MinusAssign:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_SubVarDrop);
		}
		else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FSubVarDrop);
		}
		else if (op1->RealType.type == ev_vector && op2->Type.type == ev_vector)
		{
			ec.AddStatement(OPC_VSubVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case MultiplyAssign:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_MulVarDrop);
		}
		else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FMulVarDrop);
		}
		else if (op1->RealType.type == ev_vector && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_VScaleVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case DivideAssign:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_DivVarDrop);
		}
		else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_FDivVarDrop);
		}
		else if (op1->RealType.type == ev_vector && op2->Type.type == ev_float)
		{
			ec.AddStatement(OPC_VIScaleVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case ModAssign:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_ModVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case AndAssign:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_AndVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case OrAssign:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_OrVarDrop);
		}
//FIXME This is wrong!
		else if (op1->RealType.type == ev_bool && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_OrVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case XOrAssign:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_XOrVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case LShiftAssign:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_LShiftVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case RShiftAssign:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			ec.AddStatement(OPC_RShiftVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;
	}
}

//END

//BEGIN VStateConstant

//==========================================================================
//
//	VStateConstant::VStateConstant
//
//==========================================================================

VStateConstant::VStateConstant(VState* AState, const TLocation& ALoc)
: VExpression(ALoc)
, State(AState)
{
	Type = ev_state;
}

//==========================================================================
//
//	VStateConstant::DoResolve
//
//==========================================================================

VExpression* VStateConstant::DoResolve(VEmitContext&)
{
	return this;
}

//==========================================================================
//
//	VStateConstant::Emit
//
//==========================================================================

void VStateConstant::Emit(VEmitContext& ec)
{
	ec.AddStatement(OPC_PushState, State);
}

//END

//BEGIN VClassConstant

//==========================================================================
//
//	VClassConstant::VClassConstant
//
//==========================================================================

VClassConstant::VClassConstant(VClass* AClass, const TLocation& ALoc)
: VExpression(ALoc)
, Class(AClass)
{
	Type = ev_classid;
}

//==========================================================================
//
//	VClassConstant::DoResolve
//
//==========================================================================

VExpression* VClassConstant::DoResolve(VEmitContext&)
{
	return this;
}

//==========================================================================
//
//	VClassConstant::Emit
//
//==========================================================================

void VClassConstant::Emit(VEmitContext& ec)
{
	ec.AddStatement(OPC_PushClassId, Class);
}

//END

//BEGIN VConstantValue

//==========================================================================
//
//	VConstantValue::VConstantValue
//
//==========================================================================

VConstantValue::VConstantValue(VConstant* AConst, const TLocation& ALoc)
: VExpression(ALoc)
, Const(AConst)
{
}

//==========================================================================
//
//	VConstantValue::DoResolve
//
//==========================================================================

VExpression* VConstantValue::DoResolve(VEmitContext&)
{
	Type = (EType)Const->Type;
	return this;
}

//==========================================================================
//
//	VConstantValue::Emit
//
//==========================================================================

void VConstantValue::Emit(VEmitContext& ec)
{
	ec.EmitPushNumber(Const->Value);
}

//==========================================================================
//
//	VConstantValue::GetIntConst
//
//==========================================================================

vint32 VConstantValue::GetIntConst() const
{
	if (Const->Type == ev_int)
	{
		return Const->Value;
	}
	return VExpression::GetIntConst();
}

//==========================================================================
//
//	VConstantValue::GetFloatConst
//
//==========================================================================

float VConstantValue::GetFloatConst() const
{
	if (Const->Type == ev_float)
	{
		return Const->FloatValue;
	}
	return VExpression::GetFloatConst();
}

//==========================================================================
//
//	VConstantValue::IsIntConst
//
//==========================================================================

bool VConstantValue::IsIntConst() const
{
	return Const->Type == ev_int;
}

//==========================================================================
//
//	VConstantValue::IsFloatConst
//
//==========================================================================

bool VConstantValue::IsFloatConst() const
{
	return Const->Type == ev_float;
}

//END

//BEGIN VDynamicCast

//==========================================================================
//
//	VDynamicCast::VDynamicCast
//
//==========================================================================

VDynamicCast::VDynamicCast(VClass* AClass, VExpression* AOp, const TLocation& ALoc)
: VExpression(ALoc)
, Class(AClass)
, op(AOp)
{
}

//==========================================================================
//
//	VDynamicCast::~VDynamicCast
//
//==========================================================================

VDynamicCast::~VDynamicCast()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VDynamicCast::DoResolve
//
//==========================================================================

VExpression* VDynamicCast::DoResolve(VEmitContext& ec)
{
	if (op)
		op = op->Resolve(ec);
	if (!op)
	{
		delete this;
		return NULL;
	}

	if (op->Type.type != ev_reference)
	{
		ParseError(Loc, "Bad expression, class reference required");
		delete this;
		return NULL;
	}
	Type = TType(Class);
	return this;
}

//==========================================================================
//
//	VDynamicCast::Emit
//
//==========================================================================

void VDynamicCast::Emit(VEmitContext& ec)
{
	op->Emit(ec);
	ec.AddStatement(OPC_DynamicCast, Class);
}

//END

//BEGIN VLocalVar

//==========================================================================
//
//	VLocalVar::VLocalVar
//
//==========================================================================

VLocalVar::VLocalVar(int ANum, const TLocation& ALoc)
: VExpression(ALoc)
, num(ANum)
, AddressRequested(false)
, PushOutParam(false)
{
}

//==========================================================================
//
//	VLocalVar::DoResolve
//
//==========================================================================

VExpression* VLocalVar::DoResolve(VEmitContext& ec)
{
	Type = ec.LocalDefs[num].type;
	RealType = ec.LocalDefs[num].type;
	if (Type.type == ev_bool)
	{
		Type = TType(ev_int);
	}
	PushOutParam = !!(ec.LocalDefs[num].ParamFlags & FPARM_Out);
	return this;
}

//==========================================================================
//
//	VLocalVar::RequestAddressOf
//
//==========================================================================

void VLocalVar::RequestAddressOf()
{
	if (PushOutParam)
	{
		PushOutParam = false;
		return;
	}
	if (AddressRequested)
		ParseError(Loc, "Multiple address of");
	AddressRequested = true;
}

//==========================================================================
//
//	VLocalVar::Emit
//
//==========================================================================

void VLocalVar::Emit(VEmitContext& ec)
{
	if (AddressRequested)
	{
		ec.EmitLocalAddress(ec.LocalDefs[num].ofs);
	}
	else if (ec.LocalDefs[num].ParamFlags & FPARM_Out)
	{
		if (ec.LocalDefs[num].ofs < 256)
		{
			int Ofs = ec.LocalDefs[num].ofs;
			if (Ofs == 0)
				ec.AddStatement(OPC_LocalValue0);
			else if (Ofs == 1)
				ec.AddStatement(OPC_LocalValue1);
			else if (Ofs == 2)
				ec.AddStatement(OPC_LocalValue2);
			else if (Ofs == 3)
				ec.AddStatement(OPC_LocalValue3);
			else if (Ofs == 4)
				ec.AddStatement(OPC_LocalValue4);
			else if (Ofs == 5)
				ec.AddStatement(OPC_LocalValue5);
			else if (Ofs == 6)
				ec.AddStatement(OPC_LocalValue6);
			else if (Ofs == 7)
				ec.AddStatement(OPC_LocalValue7);
			else
				ec.AddStatement(OPC_LocalValueB, Ofs);
		}
		else
		{
			ec.EmitLocalAddress(ec.LocalDefs[num].ofs);
			ec.AddStatement(OPC_PushPointedPtr);
		}
		if (PushOutParam)
		{
			EmitPushPointedCode(ec.LocalDefs[num].type, ec);
		}
	}
	else if (ec.LocalDefs[num].ofs < 256)
	{
		int Ofs = ec.LocalDefs[num].ofs;
		if (ec.LocalDefs[num].type.type == ev_bool &&
			ec.LocalDefs[num].type.bit_mask != 1)
		{
			ParseError(Loc, "Strange local bool mask");
		}
		switch (ec.LocalDefs[num].type.type)
		{
		case ev_int:
		case ev_float:
		case ev_name:
		case ev_pointer:
		case ev_reference:
		case ev_classid:
		case ev_state:
		case ev_bool:
			if (Ofs == 0)
				ec.AddStatement(OPC_LocalValue0);
			else if (Ofs == 1)
				ec.AddStatement(OPC_LocalValue1);
			else if (Ofs == 2)
				ec.AddStatement(OPC_LocalValue2);
			else if (Ofs == 3)
				ec.AddStatement(OPC_LocalValue3);
			else if (Ofs == 4)
				ec.AddStatement(OPC_LocalValue4);
			else if (Ofs == 5)
				ec.AddStatement(OPC_LocalValue5);
			else if (Ofs == 6)
				ec.AddStatement(OPC_LocalValue6);
			else if (Ofs == 7)
				ec.AddStatement(OPC_LocalValue7);
			else
				ec.AddStatement(OPC_LocalValueB, Ofs);
			break;

		case ev_vector:
			ec.AddStatement(OPC_VLocalValueB, Ofs);
			break;

		case ev_string:
			ec.AddStatement(OPC_StrLocalValueB, Ofs);
			break;

		default:
			ParseError(Loc, "Invalid operation of this variable type");
		}
	}
	else
	{
		ec.EmitLocalAddress(ec.LocalDefs[num].ofs);
		EmitPushPointedCode(ec.LocalDefs[num].type, ec);
	}
}

//END

//BEGIN VFieldAccess

//==========================================================================
//
//	VFieldAccess::VFieldAccess
//
//==========================================================================

VFieldAccess::VFieldAccess(VExpression* AOp, VField* AField, const TLocation& ALoc, int ExtraFlags)
: VExpression(ALoc)
, op(AOp)
, field(AField)
, AddressRequested(false)
{
	Flags = field->flags | ExtraFlags;
}

//==========================================================================
//
//	VFieldAccess::~VFieldAccess
//
//==========================================================================

VFieldAccess::~VFieldAccess()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VFieldAccess::DoResolve
//
//==========================================================================

VExpression* VFieldAccess::DoResolve(VEmitContext&)
{
	Type = field->type;
	RealType = field->type;
	if (Type.type == ev_bool)
	{
		Type = TType(ev_int);
	}
	return this;
}

//==========================================================================
//
//	VFieldAccess::RequestAddressOf
//
//==========================================================================

void VFieldAccess::RequestAddressOf()
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
//	VFieldAccess::Emit
//
//==========================================================================

void VFieldAccess::Emit(VEmitContext& ec)
{
	op->Emit(ec);
	if (AddressRequested)
	{
		ec.AddStatement(OPC_Offset, field);
	}
	else
	{
		switch (field->type.type)
		{
		case ev_int:
		case ev_float:
		case ev_name:
			ec.AddStatement(OPC_FieldValue, field);
			break;

		case ev_pointer:
		case ev_reference:
		case ev_classid:
		case ev_state:
			ec.AddStatement(OPC_PtrFieldValue, field);
			break;

		case ev_vector:
			ec.AddStatement(OPC_VFieldValue, field);
			break;

		case ev_bool:
			if (field->type.bit_mask & 0x000000ff)
				ec.AddStatement(OPC_Bool0FieldValue, field, (int)(field->type.bit_mask));
			else if (field->type.bit_mask & 0x0000ff00)
				ec.AddStatement(OPC_Bool1FieldValue, field, (int)(field->type.bit_mask >> 8));
			else if (field->type.bit_mask & 0x00ff0000)
				ec.AddStatement(OPC_Bool2FieldValue, field, (int)(field->type.bit_mask >> 16));
			else
				ec.AddStatement(OPC_Bool3FieldValue, field, (int)(field->type.bit_mask >> 24));
			break;

		case ev_string:
			ec.AddStatement(OPC_StrFieldValue, field);
			break;

		case ev_delegate:
			ec.AddStatement(OPC_Offset, field);
			ec.AddStatement(OPC_PushPointedDelegate);
			break;

		default:
			ParseError(Loc, "Invalid operation on field of this type");
		}
	}
}

//END

//BEGIN VDelegateVal

//==========================================================================
//
//	VDelegateVal::VDelegateVal
//
//==========================================================================

VDelegateVal::VDelegateVal(VExpression* AOp, VMethod* AM, const TLocation& ALoc)
: VExpression(ALoc)
, op(AOp)
, M(AM)
{
}

//==========================================================================
//
//	VDelegateVal::~VDelegateVal
//
//==========================================================================

VDelegateVal::~VDelegateVal()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VDelegateVal::DoResolve
//
//==========================================================================

VExpression* VDelegateVal::DoResolve(VEmitContext&)
{
	Type = ev_delegate;
	Type.Function = M;
	return this;
}

//==========================================================================
//
//	VDelegateVal::Emit
//
//==========================================================================

void VDelegateVal::Emit(VEmitContext& ec)
{
	op->Emit(ec);
	ec.AddStatement(OPC_PushVFunc, M);
}

//END

//BEGIN VInvocation

//==========================================================================
//
//	VInvocation::VInvocation
//
//==========================================================================

VInvocation::VInvocation(VExpression* ASelfExpr, VMethod* AFunc, VField* ADelegateField,
	bool AHaveSelf, bool ABaseCall, const TLocation& ALoc, int ANumArgs,
	VExpression** AArgs)
: VExpression(ALoc)
, SelfExpr(ASelfExpr)
, Func(AFunc)
, DelegateField(ADelegateField)
, HaveSelf(AHaveSelf)
, BaseCall(ABaseCall)
, NumArgs(ANumArgs)
{
	for (int i = 0; i < NumArgs; i++)
		Args[i] = AArgs[i];
}

//==========================================================================
//
//	VInvocation::~VInvocation
//
//==========================================================================

VInvocation::~VInvocation()
{
	if (SelfExpr)
		delete SelfExpr;
	for (int i = 0; i < NumArgs; i++)
		delete Args[i];
}

//==========================================================================
//
//	VInvocation::DoResolve
//
//==========================================================================

VExpression* VInvocation::DoResolve(VEmitContext& ec)
{
	//	Resolve arguments
	bool ArgsOk = true;
	for (int i = 0; i < NumArgs; i++)
	{
		if (Args[i])
		{
			Args[i] = Args[i]->Resolve(ec);
			if (!Args[i])
			{
				ArgsOk = false;
			}
		}
	}
	if (!ArgsOk)
	{
		delete this;
		return NULL;
	}

	CheckParams();

	Type  = Func->ReturnType;
	if (Type.type == ev_bool)
		Type = TType(ev_int);
	return this;
}

//==========================================================================
//
//	VInvocation::Emit
//
//==========================================================================

void VInvocation::Emit(VEmitContext& ec)
{
	if (SelfExpr)
		SelfExpr->Emit(ec);

	bool DirectCall = BaseCall || (Func->Flags & FUNC_Final);

	if (Func->Flags & FUNC_Static)
	{
		if (HaveSelf)
		{
			ParseError(Loc, "Invalid static function call");
		}
	}
	else
	{
		if (!HaveSelf)
		{
			ec.AddStatement(OPC_LocalValue0);
		}
	}

	vint32 SelfOffset = 1;
	for (int i = 0; i < NumArgs; i++)
	{
		if (!Args[i])
		{
			switch (Func->ParamTypes[i].type)
			{
			case ev_int:
			case ev_float:
			case ev_name:
			case ev_bool:
				ec.EmitPushNumber(0);
				SelfOffset++;
				break;

			case ev_string:
			case ev_pointer:
			case ev_reference:
			case ev_classid:
			case ev_state:
				ec.AddStatement(OPC_PushNull);
				SelfOffset++;
				break;

			case ev_vector:
				ec.EmitPushNumber(0);
				ec.EmitPushNumber(0);
				ec.EmitPushNumber(0);
				SelfOffset += 3;
				break;

			default:
				ParseError(Loc, "Bad optional parameter type");
				break;
			}
			ec.EmitPushNumber(0);
			SelfOffset++;
		}
		else
		{
			Args[i]->Emit(ec);
			if (Args[i]->Type.type == ev_vector)
				SelfOffset += 3;
			else
				SelfOffset++;
			if (Func->ParamFlags[i] & FPARM_Optional)
			{
				ec.EmitPushNumber(1);
				SelfOffset++;
			}
		}
	}

	if (DirectCall)
	{
		ec.AddStatement(OPC_Call, Func);
	}
	else if (DelegateField)
	{
		ec.AddStatement(OPC_DelegateCall, DelegateField, SelfOffset);
	}
	else
	{
		ec.AddStatement(OPC_VCall, Func, SelfOffset);
	}
}

//==========================================================================
//
//	VInvocation::CheckParams
//
//==========================================================================

void VInvocation::CheckParams()
{
	//	Determine parameter count.
	int argsize = 0;
	int max_params;
	int num_needed_params = Func->NumParams;
	if (Func->Flags & FUNC_VarArgs)
	{
		max_params = MAX_PARAMS - 1;
	}
	else
	{
		max_params = Func->NumParams;
	}

	for (int i = 0; i < NumArgs; i++)
	{
		if (i < num_needed_params)
		{
			if (!Args[i])
			{
				if (!(Func->ParamFlags[i] & FPARM_Optional))
				{
					ParseError(Loc, "Bad expresion");
				}
				argsize += Func->ParamTypes[i].GetSize();
			}
			else
			{
				Args[i]->Type.CheckMatch(Args[i]->Loc, Func->ParamTypes[i]);
				if (Func->ParamFlags[i] & FPARM_Out)
				{
					Args[i]->RequestAddressOf();
				}
				argsize += Args[i]->Type.GetSize();
			}
		}
		else if (!Args[i])
		{
			ParseError(Loc, "Bad expresion");
		}
		else
		{
			argsize += Args[i]->Type.GetSize();
		}
	}
	if (NumArgs > max_params)
	{
		ParseError(Loc, "Incorrect number of arguments, need %d, got %d.", max_params, NumArgs);
	}
	while (NumArgs < num_needed_params)
	{
		if (Func->ParamFlags[NumArgs] & FPARM_Optional)
		{
			Args[NumArgs] = NULL;
			NumArgs++;
		}
		else
		{
			ParseError(Loc, "Incorrect argument count %d, should be %d",
				NumArgs, num_needed_params);
			break;
		}
	}

	if (Func->Flags & FUNC_VarArgs)
	{
		Args[NumArgs++] = new VIntLiteral(argsize / 4 - num_needed_params, Loc);
	}
}

//END

//BEGIN VDelegateToBool

//==========================================================================
//
//	VDelegateToBool::VDelegateToBool
//
//==========================================================================

VDelegateToBool::VDelegateToBool(VExpression* AOp)
: VExpression(AOp->Loc)
, op(AOp)
{
	Type = ev_int;
	op->RequestAddressOf();
}

//==========================================================================
//
//	VDelegateToBool::~VDelegateToBool
//
//==========================================================================

VDelegateToBool::~VDelegateToBool()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VDelegateToBool::DoResolve
//
//==========================================================================

VExpression* VDelegateToBool::DoResolve(VEmitContext&)
{
	return this;
}

//==========================================================================
//
//	VDelegateToBool::Emit
//
//==========================================================================

void VDelegateToBool::Emit(VEmitContext& ec)
{
	op->Emit(ec);
	ec.AddStatement(OPC_PushPointedPtr);
	ec.AddStatement(OPC_PtrToBool);
}

//END

//BEGIN VStringToBool

//==========================================================================
//
//	VStringToBool::VStringToBool
//
//==========================================================================

VStringToBool::VStringToBool(VExpression* AOp)
: VExpression(AOp->Loc)
, op(AOp)
{
	Type = ev_int;
}

//==========================================================================
//
//	VStringToBool::~VStringToBool
//
//==========================================================================

VStringToBool::~VStringToBool()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VStringToBool::DoResolve
//
//==========================================================================

VExpression* VStringToBool::DoResolve(VEmitContext&)
{
	return this;
}

//==========================================================================
//
//	VStringToBool::Emit
//
//==========================================================================

void VStringToBool::Emit(VEmitContext& ec)
{
	op->Emit(ec);
	ec.AddStatement(OPC_StrToBool);
}

//END

//BEGIN VDelegateToBool

//==========================================================================
//
//	VPointerToBool::VPointerToBool
//
//==========================================================================

VPointerToBool::VPointerToBool(VExpression* AOp)
: VExpression(AOp->Loc)
, op(AOp)
{
	Type = ev_int;
}

//==========================================================================
//
//	VPointerToBool::~VPointerToBool
//
//==========================================================================

VPointerToBool::~VPointerToBool()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VPointerToBool::DoResolve
//
//==========================================================================

VExpression* VPointerToBool::DoResolve(VEmitContext&)
{
	return this;
}

//==========================================================================
//
//	VPointerToBool::Emit
//
//==========================================================================

void VPointerToBool::Emit(VEmitContext& ec)
{
	op->Emit(ec);
	ec.AddStatement(OPC_PtrToBool);
}

//END

//BEGIN VDropResult

//==========================================================================
//
//	VDropResult::VDropResult
//
//==========================================================================

VDropResult::VDropResult(VExpression* AOp)
: VExpression(AOp->Loc)
, op(AOp)
{
}

//==========================================================================
//
//	VDropResult::~VDropResult
//
//==========================================================================

VDropResult::~VDropResult()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VDropResult::DoResolve
//
//==========================================================================

VExpression* VDropResult::DoResolve(VEmitContext& ec)
{
	if (op)
		op = op->Resolve(ec);
	if (!op)
	{
		delete this;
		return NULL;
	}

	if (op->Type.type == ev_delegate)
	{
		ParseError(Loc, "Delegate call parameters are missing");
		delete this;
		return NULL;
	}

	if (op->Type.type != ev_string && op->Type.GetSize() != 4 &&
		op->Type.type != ev_vector && op->Type.type != ev_void)
	{
		ParseError(Loc, "Expression's result type cannot be dropped");
		delete this;
		return NULL;
	}

	if (op->AddDropResult())
	{
		VExpression* e = op;
		op = NULL;
		delete this;
		return e;
	}

	Type = ev_void;
	return this;
}

//==========================================================================
//
//	VDropResult::Emit
//
//==========================================================================

void VDropResult::Emit(VEmitContext& ec)
{
	op->Emit(ec);
	if (op->Type.type == ev_string)
	{
		ec.AddStatement(OPC_DropStr);
	}
	else if (op->Type.type == ev_vector)
	{
		ec.AddStatement(OPC_VDrop);
	}
	else if (op->Type.GetSize() == 4)
	{
		ec.AddStatement(OPC_Drop);
	}
}

//END

//BEGIN VTypeExpr

//==========================================================================
//
//	VTypeExpr::VTypeExpr
//
//==========================================================================

VTypeExpr::VTypeExpr(TType AType, const TLocation& ALoc)
: VExpression(ALoc)
{
	Type = AType;
	Name[0] = 0;
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
	if (Type.type == ev_unknown)
	{
		ParseError(Loc, "Bad type");
		delete this;
		return NULL;
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

const char* VTypeExpr::GetName()
{
	if (!Name[0])
		Type.GetName(Name);
	return Name;
}

//==========================================================================
//
//	VTypeExpr::CreateTypeExprCopy
//
//==========================================================================

VExpression* VTypeExpr::CreateTypeExprCopy()
{
	return new VTypeExpr(Type, Loc);
}

//END

//BEGIN VPointerType

//==========================================================================
//
//	VPointerType::VPointerType
//
//==========================================================================

VPointerType::VPointerType(VExpression* AExpr, const TLocation& ALoc)
: VTypeExpr(ev_unknown, ALoc)
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

//END

//BEGIN VFixedArrayType

//==========================================================================
//
//	VFixedArrayType::VFixedArrayType
//
//==========================================================================

VFixedArrayType::VFixedArrayType(VExpression* AExpr, VExpression* ASizeExpr,
	const TLocation& ALoc)
: VTypeExpr(ev_unknown, ALoc)
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
	}
	if (SizeExpr)
	{
		delete SizeExpr;
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

//END

//BEGIN VLocalDecl

//==========================================================================
//
//	VLocalDecl::VLocalDecl
//
//==========================================================================

VLocalDecl::VLocalDecl(const TLocation& ALoc)
: VExpression(ALoc)
{
}

//==========================================================================
//
//	VLocalDecl::~VLocalDecl
//
//==========================================================================

VLocalDecl::~VLocalDecl()
{
	for (int i = 0; i < Vars.Num(); i++)
	{
		if (Vars[i].TypeExpr)
		{
			delete Vars[i].TypeExpr;
		}
		if (Vars[i].Value)
		{
			delete Vars[i].Value;
		}
	}
}

//==========================================================================
//
//	VLocalDecl::DoResolve
//
//==========================================================================

VExpression* VLocalDecl::DoResolve(VEmitContext& ec)
{
	Declare(ec);
	return this;
}

//==========================================================================
//
//	VLocalDecl::Emit
//
//==========================================================================

void VLocalDecl::Emit(VEmitContext& ec)
{
	EmitInitialisations(ec);
}

//==========================================================================
//
//	VLocalDecl::Declare
//
//==========================================================================

void VLocalDecl::Declare(VEmitContext& ec)
{
	for (int i = 0; i < Vars.Num(); i++)
	{
		VLocalEntry& e = Vars[i];

		if (ec.CheckForLocalVar(e.Name) != -1)
		{
			ParseError(e.Loc, "Redefined identifier %s", *e.Name);
		}

		e.TypeExpr = e.TypeExpr->ResolveAsType(ec);
		if (!e.TypeExpr)
		{
			continue;
		}
		TType Type = e.TypeExpr->Type;
		if (Type.type == ev_void)
		{
			ParseError(e.TypeExpr->Loc, "Bad variable type");
		}

		VLocalVarDef& L = ec.LocalDefs.Alloc();
		L.Name = e.Name;
		L.type = Type;
		L.ofs = ec.localsofs;
		L.Visible = false;
		L.ParamFlags = 0;

		//  Initialisation
		if (e.Value)
		{
			VExpression* op1 = new VLocalVar(ec.LocalDefs.Num() - 1, e.Loc);
			e.Value = new VAssignment(VAssignment::Assign, op1, e.Value, e.Loc);
			e.Value = e.Value->Resolve(ec);
		}

		L.Visible = true;

		ec.localsofs += Type.GetSize() / 4;
		if (ec.localsofs > 1024)
		{
			ParseWarning(e.Loc, "Local vars > 1k");
		}
	}
}

//==========================================================================
//
//	VLocalDecl::EmitInitialisations
//
//==========================================================================

void VLocalDecl::EmitInitialisations(VEmitContext& ec)
{
	for (int i = 0; i < Vars.Num(); i++)
	{
		if (Vars[i].Value)
		{
			Vars[i].Value->Emit(ec);
		}
	}
}

//END
