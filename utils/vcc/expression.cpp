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

class VStateConstant : public VExpression
{
public:
	VState*		State;

	VStateConstant(VState* AState, const TLocation& ALoc);
	VExpression* DoResolve();
	void Emit();
};

class VClassConstant : public VExpression
{
public:
	VClass*		Class;

	VClassConstant(VClass* AClass, const TLocation& ALoc);
	VExpression* DoResolve();
	void Emit();
};

class VConstantValue : public VExpression
{
public:
	VConstant*		Const;

	VConstantValue(VConstant* AConst, const TLocation& ALoc);
	VExpression* DoResolve();
	void Emit();
	bool GetIntConst(vint32&);
};

class VDynamicCast : public VExpression
{
public:
	VClass*				Class;
	VExpression*		op;

	VDynamicCast(VClass* AClass, VExpression* AOp, const TLocation& ALoc);
	~VDynamicCast();
	VExpression* DoResolve();
	void Emit();
};

class VLocalVar : public VExpression
{
public:
	int				num;
	bool			AddressRequested;

	VLocalVar(int ANum, const TLocation& ALoc);
	VExpression* DoResolve();
	void RequestAddressOf();
	void Emit();
};

class VFieldAccess : public VExpression
{
public:
	VExpression*		op;
	VField*				field;
	bool				AddressRequested;

	VFieldAccess(VExpression* AOp, VField* AField, const TLocation& ALoc, int ExtraFlags);
	~VFieldAccess();
	VExpression* DoResolve();
	void RequestAddressOf();
	void Emit();
};

class VDelegateVal : public VExpression
{
public:
	VExpression*		op;
	VMethod*			M;

	VDelegateVal(VExpression* AOp, VMethod* AM, const TLocation& ALoc);
	~VDelegateVal();
	VExpression* DoResolve();
	void Emit();
};

class VInvocation : public VExpression
{
public:
	VExpression*	SelfExpr;
	VMethod*		Func;
	VField*			DelegateField;
	bool			HaveSelf;
	bool			BaseCall;
	int				NumArgs;
	VExpression*	Args[MAX_ARG_COUNT + 1];

	VInvocation(VExpression* ASelfExpr, VMethod* AFunc, VField* ADelegateField,
		bool AHaveSelf, bool ABaseCall, const TLocation& ALoc, int ANumArgs,
		VExpression** AArgs);
	~VInvocation();
	VExpression* DoResolve();
	void Emit();
	void CheckParams();
};

class VDelegateToBool : public VExpression
{
public:
	VExpression*		op;

	VDelegateToBool(VExpression* AOp);
	~VDelegateToBool();
	VExpression* DoResolve();
	void Emit();
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

VExpression* VExpression::Resolve()
{
	VExpression* e = DoResolve();
	return e;
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
//	VExpression::EmitPushPointedCode
//
//==========================================================================

void VExpression::EmitPushPointedCode(TType type)
{
	switch (type.type)
	{
	case ev_int:
	case ev_float:
	case ev_name:
	case ev_struct://	Not exactly, but the rest of code will fail without these.
	case ev_array:
		AddStatement(OPC_PushPointed);
		break;

	case ev_pointer:
	case ev_reference:
	case ev_classid:
	case ev_state:
		AddStatement(OPC_PushPointedPtr);
		break;

	case ev_vector:
		AddStatement(OPC_VPushPointed);
		break;

	case ev_bool:
		if (type.bit_mask & 0x000000ff)
			AddStatement(OPC_PushBool0, type.bit_mask);
		else if (type.bit_mask & 0x0000ff00)
			AddStatement(OPC_PushBool1, type.bit_mask >> 8);
		else if (type.bit_mask & 0x00ff0000)
			AddStatement(OPC_PushBool2, type.bit_mask >> 16);
		else
			AddStatement(OPC_PushBool3, type.bit_mask >> 24);
		break;

	case ev_string:
		AddStatement(OPC_PushPointedStr);
		break;

	case ev_delegate:
		AddStatement(OPC_PushPointedDelegate);
		break;
	}
}

//==========================================================================
//
//	VExpression::IsSingleName
//
//==========================================================================

bool VExpression::IsSingleName()
{
	return false;
}

//==========================================================================
//
//	VExpression::ResolveTopLevel
//
//==========================================================================

VExpression* VExpression::ResolveTopLevel()
{
	VExpression* e = Resolve();
	if (e && e->Type.type == ev_delegate)
	{
		e = new VDelegateToBool(e);
	}
	return e;
}

//==========================================================================
//
//	VExpression::GetIntConst
//
//==========================================================================

bool VExpression::GetIntConst(vint32& Value)
{
	ParseError(Loc, "Integer constant expected");
	Value = 0;
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
}

//==========================================================================
//
//	VIntLiteral::DoResolve
//
//==========================================================================

VExpression* VIntLiteral::DoResolve()
{
	Type = ev_int;
	return this;
}

//==========================================================================
//
//	VIntLiteral::Emit
//
//==========================================================================

void VIntLiteral::Emit()
{
	EmitPushNumber(Value);
}

//==========================================================================
//
//	VIntLiteral::GetIntConst
//
//==========================================================================

bool VIntLiteral::GetIntConst(vint32& OutValue)
{
	OutValue = Value;
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
}

//==========================================================================
//
//	VFloatLiteral::DoResolve
//
//==========================================================================

VExpression* VFloatLiteral::DoResolve()
{
	Type = ev_float;
	return this;
}

//==========================================================================
//
//	VFloatLiteral::Emit
//
//==========================================================================

void VFloatLiteral::Emit()
{
	AddStatement(OPC_PushNumber, Value);
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
}

//==========================================================================
//
//	VNameLiteral::DoResolve
//
//==========================================================================

VExpression* VNameLiteral::DoResolve()
{
	Type = ev_name;
	return this;
}

//==========================================================================
//
//	VNameLiteral::Emit
//
//==========================================================================

void VNameLiteral::Emit()
{
	AddStatement(OPC_PushName, Value);
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
}

//==========================================================================
//
//	VStringLiteral::DoResolve
//
//==========================================================================

VExpression* VStringLiteral::DoResolve()
{
	Type = ev_string;
	return this;
}

//==========================================================================
//
//	VStringLiteral::Emit
//
//==========================================================================

void VStringLiteral::Emit()
{
	AddStatement(OPC_PushString, Value);
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

VExpression* VSelf::DoResolve()
{
	Type = SelfType;
	if (!SelfClass)
	{
		ParseError(Loc, "self used outside member function\n");
	}
	return this;
}

//==========================================================================
//
//	VSelf::Emit
//
//==========================================================================

void VSelf::Emit()
{
	AddStatement(OPC_LocalAddress0);
	AddStatement(OPC_PushPointedPtr);
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
}

//==========================================================================
//
//	VNoneLiteral::DoResolve
//
//==========================================================================

VExpression* VNoneLiteral::DoResolve()
{
	Type = TType((VClass*)NULL);
	return this;
}

//==========================================================================
//
//	VNoneLiteral::Emit
//
//==========================================================================

void VNoneLiteral::Emit()
{
	AddStatement(OPC_PushNull);
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
}

//==========================================================================
//
//	VNullLiteral::DoResolve
//
//==========================================================================

VExpression* VNullLiteral::DoResolve()
{
	Type = MakePointerType(TType(ev_void));
	return this;
}

//==========================================================================
//
//	VNullLiteral::Emit
//
//==========================================================================

void VNullLiteral::Emit()
{
	AddStatement(OPC_PushNull);
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

VExpression* VVector::DoResolve()
{
	if (op1)
		op1 = op1->Resolve();
	if (op2)
		op2 = op2->Resolve();
	if (op3)
		op3 = op3->Resolve();
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

void VVector::Emit()
{
	op1->Emit();
	op2->Emit();
	op3->Emit();
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

VExpression* VSingleName::DoResolve()
{
	int num = CheckForLocalVar(Name);
	if (num)
	{
		VExpression* e = new VLocalVar(num, Loc);
		delete this;
		return e->Resolve();
	}

	VConstant* Const = CheckForConstant(SelfClass, Name);
	if (Const)
	{
		VExpression* e = new VConstantValue(Const, Loc);
		delete this;
		return e->Resolve();
	}

	if (SelfClass)
	{
		VMethod* M = CheckForMethod(Name, SelfClass);
		if (M)
		{
			VExpression* e = new VDelegateVal((new VSelf(Loc))->Resolve(), M, Loc);
			delete this;
			return e->Resolve();
		}

		VField* field = CheckForField(Name, SelfClass);
		if (field)
		{
			VExpression* e = new VFieldAccess((new VSelf(Loc))->Resolve(), field, Loc, 0);
			delete this;
			return e->Resolve();
		}

		VState* State = CheckForState(Name, SelfClass);
		if (State)
		{
			VExpression* e = new VStateConstant(State, Loc);
			delete this;
			return e->Resolve();
		}
	}

	VClass* Class = CheckForClass(Name);
	if (Class)
	{
		VExpression* e = new VClassConstant(Class, Loc);
		delete this;
		return e->Resolve();
	}

	ParseError(Loc, "Illegal expression identifier %s", *Name);
	delete this;
	return NULL;
}

//==========================================================================
//
//	VSingleName::Emit
//
//==========================================================================

void VSingleName::Emit()
{
	ParseError(Loc, "Should not happen");
}

//==========================================================================
//
//	VSingleName::IsSingleName
//
//==========================================================================

bool VSingleName::IsSingleName()
{
	return true;
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

VExpression* VDoubleName::DoResolve()
{
	VClass* Class = CheckForClass(Name1);
	if (!Class)
	{
		ParseError("No such class %s", *Name1);
		delete this;
		return NULL;
	}

	VConstant* Const = CheckForConstant(Class, Name2);
	if (Const)
	{
		VExpression* e = new VConstantValue(Const, Loc);
		delete this;
		return e->Resolve();
	}

	VState* State = CheckForState(Name2, Class);
	if (State)
	{
		VExpression* e = new VStateConstant(State, Loc);
		delete this;
		return e->Resolve();
	}

	ParseError(Loc, "No such constant or state %s", *Name2);
	delete this;
	return NULL;
}

//==========================================================================
//
//	VDoubleName::Emit
//
//==========================================================================

void VDoubleName::Emit()
{
	ParseError(Loc, "Should not happen");
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

VExpression* VPointerField::DoResolve()
{
	if (op)
		op = op->Resolve();
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
	VField* field = CheckForStructField(type.Struct, FieldName, Loc);
	if (!field)
	{
		ParseError(Loc, "No such field %s", *FieldName);
		delete this;
		return NULL;
	}
	VExpression* e = new VFieldAccess(op, field, Loc, 0);
	op = NULL;
	delete this;
	return e->Resolve();
}

//==========================================================================
//
//	VPointerField::Emit
//
//==========================================================================

void VPointerField::Emit()
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

VExpression* VDotField::DoResolve()
{
	if (op)
		op = op->Resolve();
	if (!op)
	{
		delete this;
		return NULL;
	}

	if (op->Type.type == ev_reference)
	{
		VMethod* M = CheckForMethod(FieldName, op->Type.Class);
		if (M)
		{
			VExpression* e = new VDelegateVal(op, M, Loc);
			op = NULL;
			delete this;
			return e->Resolve();
		}
		else
		{
			VField* field = CheckForField(FieldName, op->Type.Class);
			if (!field)
			{
				ParseError(Loc, "No such field %s", *FieldName);
				delete this;
				return NULL;
			}
			VExpression* e = new VFieldAccess(op, field, Loc, 0);
			op = NULL;
			delete this;
			return e->Resolve();
		}
	}
	else if (op->Type.type == ev_struct || op->Type.type == ev_vector)
	{
		TType type = op->Type;
		int Flags = op->Flags;
		op->Flags &= ~FIELD_ReadOnly;
		op->RequestAddressOf();
		VField* field = CheckForStructField(type.Struct, FieldName, Loc);
		if (!field)
		{
			ParseError(Loc, "No such field %s", *FieldName);
			delete this;
			return NULL;
		}
		VExpression* e = new VFieldAccess(op, field, Loc, Flags & FIELD_ReadOnly);
		op = NULL;
		delete this;
		return e->Resolve();
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

void VDotField::Emit()
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

VExpression* VArrayElement::DoResolve()
{
	if (op)
		op = op->Resolve();
	if (ind)
		ind = ind->Resolve();
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

void VArrayElement::Emit()
{
	op->Emit();
	ind->Emit();
	AddStatement(OPC_ArrayElement, RealType);
	if (!AddressRequested)
	{
		EmitPushPointedCode(RealType);
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

VExpression* VBaseInvocation::DoResolve()
{
	if (!SelfClass)
	{
		ParseError(Loc, ":: not in method");
		delete this;
		return NULL;
	}
	VMethod* Func = CheckForMethod(Name, SelfClass->ParentClass);
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
	return e->Resolve();
}

//==========================================================================
//
//	VBaseInvocation::Emit
//
//==========================================================================

void VBaseInvocation::Emit()
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
		delete Args[i];
}

//==========================================================================
//
//	VCastOrInvocation::DoResolve
//
//==========================================================================

VExpression* VCastOrInvocation::DoResolve()
{
	VClass* Class = CheckForClass(Name);
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
		return e->Resolve();
	}

	if (SelfClass)
	{
		VMethod* M = CheckForMethod(Name, SelfClass);
		if (M)
		{
			VExpression* e = new VInvocation(NULL, M, NULL,
				false, false, Loc, NumArgs, Args);
			NumArgs = 0;
			delete this;
			return e->Resolve();
		}
		VField* field = CheckForField(Name, SelfClass);
		if (field && field->type.type == ev_delegate)
		{
			VExpression* e = new VInvocation(NULL, field->func, field,
				false, false, Loc, NumArgs, Args);
			NumArgs = 0;
			delete this;
			return e->Resolve();
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

void VCastOrInvocation::Emit()
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

VExpression* VDotInvocation::DoResolve()
{
	if (SelfExpr)
		SelfExpr = SelfExpr->Resolve();
	if (!SelfExpr)
	{
		delete this;
		return NULL;
	}

	if (SelfExpr->Type.type != ev_reference)
	{
		ParseError(Loc, "Object reference expected left side of .");
	}

	VMethod* M = CheckForMethod(MethodName, SelfExpr->Type.Class);
	if (M)
	{
		VExpression* e = new VInvocation(SelfExpr, M, NULL, true,
			false, Loc, NumArgs, Args);
		SelfExpr = NULL;
		NumArgs = 0;
		delete this;
		return e->Resolve();
	}

	VField* field = CheckForField(MethodName, SelfExpr->Type.Class);
	if (field && field->type.type == ev_delegate)
	{
		VExpression* e = new VInvocation(SelfExpr, field->func, field, true,
			false, Loc, NumArgs, Args);
		SelfExpr = NULL;
		NumArgs = 0;
		delete this;
		return e->Resolve();
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

void VDotInvocation::Emit()
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

VUnary::VUnary(EPunctuation AOper, VExpression* AOp, const TLocation& ALoc)
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

VExpression* VUnary::DoResolve()
{
	if (op)
		op = op->Resolve();
	if (!op)
	{
		delete this;
		return NULL;
	}

	switch (Oper)
	{
	case PU_PLUS:
		Type = op->Type;
		if (op->Type.type != ev_int && op->Type.type != ev_float)
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		break;
	case PU_MINUS:
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
	case PU_NOT:
		op->Type.CheckSizeIs4(Loc);
		Type = ev_int;
		break;
	case PU_TILDE:
		if (op->Type.type != ev_int)
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		Type = ev_int;
		break;
	case PU_AND:
		if (op->Type.type == ev_reference)
		{
			ParseError(Loc, "Tried to take address of reference");
			delete this;
			return NULL;
		}
		else
		{
			op->RequestAddressOf();
			Type = MakePointerType(op->RealType);
		}
		break;
	default:
		break;
	}
	return this;
}

//==========================================================================
//
//	VUnary::Emit
//
//==========================================================================

void VUnary::Emit()
{
	op->Emit();

	switch (Oper)
	{
	case PU_PLUS:
		break;

	case PU_MINUS:
		if (op->Type.type == ev_int)
		{
			AddStatement(OPC_UnaryMinus);
		}
		else if (op->Type.type == ev_float)
		{
			AddStatement(OPC_FUnaryMinus);
		}
		else if (op->Type.type == ev_vector)
		{
			AddStatement(OPC_VUnaryMinus);
		}
		break;

	case PU_NOT:
		op->Type.EmitToBool();
		AddStatement(OPC_NegateLogical);
		break;

	case PU_TILDE:
		AddStatement(OPC_BitInverse);
		break;

	default:
		break;
	}
}

//==========================================================================
//
//	VUnary::GetIntConst
//
//==========================================================================

bool VUnary::GetIntConst(vint32& OutValue)
{
	if (op->GetIntConst(OutValue))
	{
		switch (Oper)
		{
		case PU_PLUS:
			return true;
	
		case PU_MINUS:
			OutValue = -OutValue;
			return true;
	
		case PU_NOT:
			OutValue = !OutValue;
			return true;
	
		case PU_TILDE:
			OutValue = ~OutValue;
			return true;
	
		default:
			ParseError(Loc, "Integer constant expected");
			break;
		}
	}
	return false;
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

VExpression* VUnaryMutator::DoResolve()
{
	if (op)
		op = op->Resolve();
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

void VUnaryMutator::Emit()
{
	op->Emit();
	switch (Oper)
	{
	case INCDEC_PreInc:
		AddStatement(OPC_PreInc);
		break;

	case INCDEC_PreDec:
		AddStatement(OPC_PreDec);
		break;

	case INCDEC_PostInc:
		AddStatement(OPC_PostInc);
		break;

	case INCDEC_PostDec:
		AddStatement(OPC_PostDec);
		break;
	}
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

VExpression* VPushPointed::DoResolve()
{
	if (op)
		op = op->Resolve();
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

void VPushPointed::Emit()
{
	op->Emit();
	if (!AddressRequested)
	{
		EmitPushPointedCode(RealType);
	}
}

//END

//BEGIN VBinary

//==========================================================================
//
//	VBinary::VBinary
//
//==========================================================================

VBinary::VBinary(EPunctuation AOper, VExpression* AOp1, VExpression* AOp2, const TLocation& ALoc)
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

VExpression* VBinary::DoResolve()
{
	if (op1)
		op1 = op1->Resolve();
	if (op2)
		op2 = op2->Resolve();
	if (!op1 || !op2)
	{
		delete this;
		return NULL;
	}

	switch (Oper)
	{
	case PU_ASTERISK:
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
	case PU_SLASH:
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
	case PU_PLUS:
	case PU_MINUS:
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
	case PU_PERCENT:
	case PU_LSHIFT:
	case PU_RSHIFT:
	case PU_AND:
	case PU_XOR:
	case PU_OR:
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
	case PU_LT:
	case PU_LE:
	case PU_GT:
	case PU_GE:
		if (!(op1->Type.type == ev_int && op2->Type.type == ev_int) &&
			!(op1->Type.type == ev_float && op2->Type.type == ev_float))
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		Type = ev_int;
		break;
	case PU_EQ:
	case PU_NE:
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
	case PU_AND_LOG:
	case PU_OR_LOG:
		op1->Type.CheckSizeIs4(Loc);
		op2->Type.CheckSizeIs4(Loc);
		Type = ev_int;
		break;
	default:
		break;
	}
	return this;
}

//==========================================================================
//
//	VBinary::Emit
//
//==========================================================================

void VBinary::Emit()
{
	int jmppos = 0;

	op1->Emit();

	switch (Oper)
	{
	case PU_AND_LOG:
		op1->Type.EmitToBool();
		jmppos = AddStatement(OPC_IfNotTopGoto, 0);
		break;
	case PU_OR_LOG:
		op1->Type.EmitToBool();
		jmppos = AddStatement(OPC_IfTopGoto, 0);
		break;
	default:
		break;
	}

	op2->Emit();

	switch (Oper)
	{
	case PU_ASTERISK:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_Multiply);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_FMultiply);
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_float)
		{
			AddStatement(OPC_VPostScale);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_vector)
		{
			AddStatement(OPC_VPreScale);
		}
		break;

	case PU_SLASH:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_Divide);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_FDivide);
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_float)
		{
			AddStatement(OPC_VIScale);
		}
		break;

	case PU_PERCENT:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_Modulus);
		}
		break;

	case PU_PLUS:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_Add);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_FAdd);
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_vector)
		{
			AddStatement(OPC_VAdd);
		}
		break;

	case PU_MINUS:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_Subtract);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_FSubtract);
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_vector)
		{
			AddStatement(OPC_VSubtract);
		}
		break;

	case PU_LSHIFT:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_LShift);
		}
		break;

	case PU_RSHIFT:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_RShift);
		}
		break;

	case PU_LT:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_Less);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_FLess);
		}
		break;

	case PU_LE:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_LessEquals);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_FLessEquals);
		}
		break;

	case PU_GT:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_Greater);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_FGreater);
		}
		break;

	case PU_GE:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_GreaterEquals);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_FGreaterEquals);
		}
		break;

	case PU_EQ:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_Equals);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_FEquals);
		}
		else if (op1->Type.type == ev_name && op2->Type.type == ev_name)
		{
			AddStatement(OPC_Equals);
		}
		else if (op1->Type.type == ev_pointer && op2->Type.type == ev_pointer)
		{
			AddStatement(OPC_PtrEquals);
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_vector)
		{
			AddStatement(OPC_VEquals);
		}
		else if (op1->Type.type == ev_classid && op2->Type.type == ev_classid)
		{
			AddStatement(OPC_PtrEquals);
		}
		else if (op1->Type.type == ev_state && op2->Type.type == ev_state)
		{
			AddStatement(OPC_PtrEquals);
		}
		else if (op1->Type.type == ev_reference && op2->Type.type == ev_reference)
		{
			AddStatement(OPC_PtrEquals);
		}
		break;

	case PU_NE:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_NotEquals);
		}
		else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_FNotEquals);
		}
		else if (op1->Type.type == ev_name && op2->Type.type == ev_name)
		{
			AddStatement(OPC_NotEquals);
		}
		else if (op1->Type.type == ev_pointer && op2->Type.type == ev_pointer)
		{
			AddStatement(OPC_PtrNotEquals);
		}
		else if (op1->Type.type == ev_vector && op2->Type.type == ev_vector)
		{
			AddStatement(OPC_VNotEquals);
		}
		else if (op1->Type.type == ev_classid && op2->Type.type == ev_classid)
		{
			AddStatement(OPC_PtrNotEquals);
		}
		else if (op1->Type.type == ev_state && op2->Type.type == ev_state)
		{
			AddStatement(OPC_PtrNotEquals);
		}
		else if (op1->Type.type == ev_reference && op2->Type.type == ev_reference)
		{
			AddStatement(OPC_PtrNotEquals);
		}
		break;

	case PU_AND:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_AndBitwise);
		}
		break;

	case PU_XOR:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_XOrBitwise);
		}
		break;

	case PU_OR:
		if (op1->Type.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_OrBitwise);
		}
		break;

	case PU_AND_LOG:
		op2->Type.EmitToBool();
		AddStatement(OPC_AndLogical);
		FixupJump(jmppos);
		break;

	case PU_OR_LOG:
		op2->Type.EmitToBool();
		AddStatement(OPC_OrLogical);
		FixupJump(jmppos);
		break;

	default:
		break;
	}
}

//==========================================================================
//
//	VBinary::GetIntConst
//
//==========================================================================

bool VBinary::GetIntConst(vint32& OutValue)
{
	vint32 Value1;
	vint32 Value2;
	if (op1->GetIntConst(Value1) && op2->GetIntConst(Value2))
	{
		switch (Oper)
		{
		case PU_ASTERISK:
			OutValue = Value1 * Value2;
			return true;
	
		case PU_SLASH:
			if (!Value2)
			{
				ParseError(Loc, "Division by 0");
				OutValue = 0;
				return false;
			}
			OutValue = Value1 / Value2;
			return true;
	
		case PU_PERCENT:
			if (!Value2)
			{
				ParseError(Loc, "Division by 0");
				OutValue = 0;
				return false;
			}
			OutValue = Value1 % Value2;
			return true;
	
		case PU_PLUS:
			OutValue = Value1 + Value2;
			return true;
	
		case PU_MINUS:
			OutValue = Value1 - Value2;
			return true;
	
		case PU_LSHIFT:
			OutValue = Value1 << Value2;
			return true;
	
		case PU_RSHIFT:
			OutValue = Value1 >> Value2;
			return true;
	
		case PU_LT:
			OutValue = Value1 < Value2;
			return true;
	
		case PU_LE:
			OutValue = Value1 <= Value2;
			return true;
	
		case PU_GT:
			OutValue = Value1 > Value2;
			return true;
	
		case PU_GE:
			OutValue = Value1 >= Value2;
			return true;
	
		case PU_EQ:
			OutValue = Value1 == Value2;
			return true;
	
		case PU_NE:
			OutValue = Value1 != Value2;
			return true;
	
		case PU_AND:
			OutValue = Value1 & Value2;
			return true;
	
		case PU_XOR:
			OutValue = Value1 ^ Value2;
			return true;
	
		case PU_OR:
			OutValue = Value1 | Value2;
			return true;
	
		case PU_AND_LOG:
			OutValue = Value1 && Value2;
			return true;
	
		case PU_OR_LOG:
			OutValue = Value1 || Value2;
			return true;
	
		default:
			break;
		}
	}
	OutValue = 0;
	return false;
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

VExpression* VConditional::DoResolve()
{
	if (op)
		op = op->Resolve();
	if (op1)
		op1 = op1->Resolve();
	if (op2)
		op2 = op2->Resolve();
	if (!op || !op1 || !op2)
	{
		delete this;
		return NULL;
	}

	op->Type.CheckSizeIs4(Loc);
	op1->Type.CheckMatch(op2->Type);
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

void VConditional::Emit()
{
	op->Emit();
	op->Type.EmitToBool();
	int jumppos1 = AddStatement(OPC_IfNotGoto, 0);
	op1->Emit();
	int jumppos2 = AddStatement(OPC_Goto, 0);
	FixupJump(jumppos1);
	op2->Emit();
	FixupJump(jumppos2);
}

//END

//BEGIN VAssignment

//==========================================================================
//
//	VAssignment::VAssignment
//
//==========================================================================

VAssignment::VAssignment(EPunctuation AOper, VExpression* AOp1, VExpression* AOp2, const TLocation& ALoc)
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

VExpression* VAssignment::DoResolve()
{
	if (op1)
		op1 = op1->Resolve();
	if (op2)
		op2 = op2->Resolve();
	if (!op1 || !op2)
	{
		delete this;
		return NULL;
	}

	op2->Type.CheckMatch(op1->RealType);
	op1->RequestAddressOf();
	return this;
}

//==========================================================================
//
//	VAssignment::Emit
//
//==========================================================================

void VAssignment::Emit()
{
	op1->Emit();
	op2->Emit();

	switch (Oper)
	{
	case PU_ASSIGN:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (op1->RealType.type == ev_name && op2->Type.type == ev_name)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (op1->RealType.type == ev_string && op2->Type.type == ev_string)
		{
			AddStatement(OPC_AssignStrDrop);
		}
		else if (op1->RealType.type == ev_pointer && op2->Type.type == ev_pointer)
		{
			AddStatement(OPC_AssignPtrDrop);
		}
		else if (op1->RealType.type == ev_vector && op2->Type.type == ev_vector)
		{
			AddStatement(OPC_VAssignDrop);
		}
		else if (op1->RealType.type == ev_classid && (op2->Type.type == ev_classid ||
			(op2->Type.type == ev_reference && op2->Type.Class == NULL)))
		{
			AddStatement(OPC_AssignPtrDrop);
		}
		else if (op1->RealType.type == ev_state && (op2->Type.type == ev_state ||
			(op2->Type.type == ev_reference && op2->Type.Class == NULL)))
		{
			AddStatement(OPC_AssignPtrDrop);
		}
		else if (op1->RealType.type == ev_reference && op2->Type.type == ev_reference)
		{
			AddStatement(OPC_AssignPtrDrop);
		}
		else if (op1->RealType.type == ev_bool && op2->Type.type == ev_int)
		{
			if (op1->RealType.bit_mask & 0x000000ff)
				AddStatement(OPC_AssignBool0, op1->RealType.bit_mask);
			else if (op1->RealType.bit_mask & 0x0000ff00)
				AddStatement(OPC_AssignBool1, op1->RealType.bit_mask >> 8);
			else if (op1->RealType.bit_mask & 0x00ff0000)
				AddStatement(OPC_AssignBool2, op1->RealType.bit_mask >> 16);
			else
				AddStatement(OPC_AssignBool3, op1->RealType.bit_mask >> 24);
		}
		else if (op1->RealType.type == ev_delegate && op2->Type.type == ev_delegate)
		{
			AddStatement(OPC_AssignDelegate);
		}
		else if (op1->RealType.type == ev_delegate && op2->Type.type == ev_reference && op2->Type.Class == NULL)
		{
			AddStatement(OPC_PushNull);
			AddStatement(OPC_AssignDelegate);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case PU_ADD_ASSIGN:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_AddVarDrop);
		}
		else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_FAddVarDrop);
		}
		else if (op1->RealType.type == ev_vector && op2->Type.type == ev_vector)
		{
			AddStatement(OPC_VAddVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case PU_MINUS_ASSIGN:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_SubVarDrop);
		}
		else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_FSubVarDrop);
		}
		else if (op1->RealType.type == ev_vector && op2->Type.type == ev_vector)
		{
			AddStatement(OPC_VSubVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case PU_MULTIPLY_ASSIGN:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_MulVarDrop);
		}
		else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_FMulVarDrop);
		}
		else if (op1->RealType.type == ev_vector && op2->Type.type == ev_float)
		{
			AddStatement(OPC_VScaleVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case PU_DIVIDE_ASSIGN:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_DivVarDrop);
		}
		else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
		{
			AddStatement(OPC_FDivVarDrop);
		}
		else if (op1->RealType.type == ev_vector && op2->Type.type == ev_float)
		{
			AddStatement(OPC_VIScaleVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case PU_MOD_ASSIGN:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_ModVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case PU_AND_ASSIGN:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_AndVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case PU_OR_ASSIGN:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_OrVarDrop);
		}
//FIXME This is wrong!
		else if (op1->RealType.type == ev_bool && op2->Type.type == ev_int)
		{
			AddStatement(OPC_OrVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case PU_XOR_ASSIGN:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_XOrVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case PU_LSHIFT_ASSIGN:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_LShiftVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case PU_RSHIFT_ASSIGN:
		if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
		{
			AddStatement(OPC_RShiftVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	default:
		ParseError(Loc, "Impossible");
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
}

//==========================================================================
//
//	VStateConstant::DoResolve
//
//==========================================================================

VExpression* VStateConstant::DoResolve()
{
	Type = ev_state;
	return this;
}

//==========================================================================
//
//	VStateConstant::Emit
//
//==========================================================================

void VStateConstant::Emit()
{
	AddStatement(OPC_PushState, State);
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
}

//==========================================================================
//
//	VClassConstant::DoResolve
//
//==========================================================================

VExpression* VClassConstant::DoResolve()
{
	Type = ev_classid;
	return this;
}

//==========================================================================
//
//	VClassConstant::Emit
//
//==========================================================================

void VClassConstant::Emit()
{
	AddStatement(OPC_PushClassId, Class);
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

VExpression* VConstantValue::DoResolve()
{
	Type = (EType)Const->Type;
	return this;
}

//==========================================================================
//
//	VConstantValue::Emit
//
//==========================================================================

void VConstantValue::Emit()
{
	EmitPushNumber(Const->value);
}

//==========================================================================
//
//	VConstantValue::GetIntConst
//
//==========================================================================

bool VConstantValue::GetIntConst(vint32& OutValue)
{
	if (Const->Type == ev_int)
	{
		OutValue = Const->value;
		return true;
	}
	ParseError(Loc, "Integer constant expected");
	OutValue = 0;
	return false;
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

VExpression* VDynamicCast::DoResolve()
{
	if (op)
		op = op->Resolve();
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

void VDynamicCast::Emit()
{
	op->Emit();
	AddStatement(OPC_DynamicCast, Class);
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
{
}

//==========================================================================
//
//	VLocalVar::DoResolve
//
//==========================================================================

VExpression* VLocalVar::DoResolve()
{
	Type = localdefs[num].type;
	RealType = localdefs[num].type;
	if (Type.type == ev_bool)
	{
		Type = TType(ev_int);
	}
	return this;
}

//==========================================================================
//
//	VLocalVar::RequestAddressOf
//
//==========================================================================

void VLocalVar::RequestAddressOf()
{
	if (AddressRequested)
		ParseError(Loc, "Multiple address of");
	AddressRequested = true;
}

//==========================================================================
//
//	VLocalVar::Emit
//
//==========================================================================

void VLocalVar::Emit()
{
	EmitLocalAddress(localdefs[num].ofs);
	if (!AddressRequested)
	{
		EmitPushPointedCode(localdefs[num].type);
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

VExpression* VFieldAccess::DoResolve()
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

void VFieldAccess::Emit()
{
	op->Emit();
	AddStatement(OPC_Offset, field);
	if (!AddressRequested)
	{
		EmitPushPointedCode(field->type);
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

VExpression* VDelegateVal::DoResolve()
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

void VDelegateVal::Emit()
{
	op->Emit();
	AddStatement(OPC_PushVFunc, M);
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

VExpression* VInvocation::DoResolve()
{
	//	Resolve arguments
	for (int i = 0; i < NumArgs; i++)
	{
		if (Args[i])
			Args[i] = Args[i]->Resolve();
		if (!Args[i])
		{
			delete this;
			return NULL;
		}
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

void VInvocation::Emit()
{
	if (SelfExpr)
		SelfExpr->Emit();

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
			AddStatement(OPC_LocalAddress0);
			AddStatement(OPC_PushPointedPtr);
		}
		if (DelegateField)
		{
			AddStatement(OPC_Offset, DelegateField);
			AddStatement(OPC_PushPointedDelegate);
		}
		else if (!DirectCall)
		{
			AddStatement(OPC_PushVFunc, Func);
		}
	}

	for (int i = 0; i < NumArgs; i++)
	{
		Args[i]->Emit();
		if (!DirectCall)
		{
			if (Args[i]->Type.type == ev_vector)
				AddStatement(OPC_Swap3);
			else
				AddStatement(OPC_Swap);
		}
	}

	if (DirectCall)
	{
		AddStatement(OPC_Call, Func);
	}
	else
	{
		AddStatement(OPC_ICall);
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
		max_params = MAX_ARG_COUNT - 1;
	}
	else
	{
		max_params = Func->NumParams;
	}

	for (int i = 0; i < NumArgs; i++)
	{
		if (i >= max_params)
		{
			ParseError(Loc, "Incorrect number of arguments, need %d, got %d.", max_params, i + 1);
		}
		else
		{
			if (i < num_needed_params)
			{
				Args[i]->Type.CheckMatch(Func->ParamTypes[i]);
			}
		}
		argsize += Args[i]->Type.GetSize();
	}
	if (NumArgs < num_needed_params)
	{
		ParseError(Loc, "Incorrect argument count %d, should be %d",
			NumArgs, num_needed_params);
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

VExpression* VDelegateToBool::DoResolve()
{
	return this;
}

//==========================================================================
//
//	VDelegateToBool::Emit
//
//==========================================================================

void VDelegateToBool::Emit()
{
	op->Emit();
	AddStatement(OPC_PushPointedPtr);
	AddStatement(OPC_PtrToBool);
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

VExpression* VLocalDecl::DoResolve()
{
	Declare();
	return this;
}

//==========================================================================
//
//	VLocalDecl::Emit
//
//==========================================================================

void VLocalDecl::Emit()
{
	EmitInitialisations();
}

//==========================================================================
//
//	VLocalDecl::Declare
//
//==========================================================================

void VLocalDecl::Declare()
{
	if (BaseType.type == ev_unknown)
	{
		BaseType = CheckForType(SelfClass, TypeName);
		if (BaseType.type == ev_unknown)
		{
			ParseError(tk_Location, "Invalid identifier, bad type");
			return;
		}
	}

	for (int i = 0; i < Vars.Num(); i++)
	{
		VLocalEntry& e = Vars[i];

		if (CheckForLocalVar(e.Name))
		{
			ParseError(e.Loc, "Redefined identifier %s", *e.Name);
		}

		TType Type = BaseType;
		for (int pl = 0; pl < e.PointerLevel; pl++)
		{
			Type = MakePointerType(Type);
		}
		if (Type.type == ev_void)
		{
			ParseError(e.Loc, "Bad variable type");
		}
		if (e.ArraySize != -1)
		{
			Type = MakeArrayType(Type, e.ArraySize);
		}

		if (numlocaldefs == MAX_LOCAL_DEFS)
		{
			ParseError(e.Loc, "Too many local variables");
			continue;
		}

		localdefs[numlocaldefs].Name = e.Name;
		localdefs[numlocaldefs].type = Type;
		localdefs[numlocaldefs].ofs = localsofs;
		localdefs[numlocaldefs].Visible = true;
		localdefs[numlocaldefs].Cleared = false;

		//  Initialisation
		if (e.Value)
		{
			VExpression* op1 = new VLocalVar(numlocaldefs, tk_Location);
			e.Value = new VAssignment(PU_ASSIGN, op1, e.Value, tk_Location);
			e.Value = e.Value->Resolve();
		}

		//  Increase variable count after expression so you can't use
		// the variable in expression.
		numlocaldefs++;
		localsofs += Type.GetSize() / 4;
		if (localsofs > 1024)
		{
			ParseWarning("Local vars > 1k");
		}
	}
}

//==========================================================================
//
//	VLocalDecl::EmitInitialisations
//
//==========================================================================

void VLocalDecl::EmitInitialisations()
{
	for (int i = 0; i < Vars.Num(); i++)
	{
		if (Vars[i].Value)
		{
			Vars[i].Value->Emit();
		}
	}
}

//END
