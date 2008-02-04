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
//	VPropertyAssign
//
//==========================================================================

class VPropertyAssign : public VInvocation
{
public:
	VPropertyAssign(VExpression* ASelfExpr, VMethod* AFunc, bool AHaveSelf,
		const TLocation& ALoc);
	bool IsPropertyAssign() const;
};

//==========================================================================
//
//	VDynArrayGetNum
//
//==========================================================================

class VDynArrayGetNum : public VExpression
{
public:
	VExpression*		ArrayExpr;

	VDynArrayGetNum(VExpression*, const TLocation&);
	~VDynArrayGetNum();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDynArraySetNum
//
//==========================================================================

class VDynArraySetNum : public VExpression
{
public:
	VExpression*		ArrayExpr;
	VExpression*		NumExpr;

	VDynArraySetNum(VExpression*, VExpression*, const TLocation&);
	~VDynArraySetNum();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	bool IsDynArraySetNum() const;
};

//==========================================================================
//
//	VDynArrayInsert
//
//==========================================================================

class VDynArrayInsert : public VExpression
{
public:
	VExpression*		ArrayExpr;
	VExpression*		IndexExpr;
	VExpression*		CountExpr;

	VDynArrayInsert(VExpression*, VExpression*, VExpression*,
		const TLocation&);
	~VDynArrayInsert();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VDynArrayRemove
//
//==========================================================================

class VDynArrayRemove : public VExpression
{
public:
	VExpression*		ArrayExpr;
	VExpression*		IndexExpr;
	VExpression*		CountExpr;

	VDynArrayRemove(VExpression*, VExpression*, VExpression*,
		const TLocation&);
	~VDynArrayRemove();
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

	if (op1->Type.Type != TYPE_Float)
	{
		ParseError(Loc, "Expression type mistmatch, vector param 1 is not a float");
		delete this;
		return NULL;
	}
	if (op2->Type.Type != TYPE_Float)
	{
		ParseError(Loc, "Expression type mistmatch, vector param 2 is not a float");
		delete this;
		return NULL;
	}
	if (op3->Type.Type != TYPE_Float)
	{
		ParseError(Loc, "Expression type mistmatch, vector param 3 is not a float");
		delete this;
		return NULL;
	}

	Type = TYPE_Vector;
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
//	VSingleName::IntResolve
//
//==========================================================================

VExpression* VSingleName::IntResolve(VEmitContext& ec, bool AssignTarget)
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

		VProperty* Prop = ec.SelfClass->CheckForProperty(Name);
		if (Prop)
		{
			if (AssignTarget)
			{
				if (ec.InDefaultProperties)
				{
					if (!Prop->DefaultField)
					{
						ParseError(Loc, "Property %s has no default field set", *Name);
						delete this;
						return NULL;
					}
					VExpression* e = new VFieldAccess((new VSelf(Loc))->Resolve(ec),
						Prop->DefaultField, Loc, 0);
					delete this;
					return e->Resolve(ec);
				}
				else
				{
					if (!Prop->SetFunc)
					{
						ParseError(Loc, "Property %s cannot be set", *Name);
						delete this;
						return NULL;
					}
					VExpression* e = new VPropertyAssign(NULL, Prop->SetFunc, false, Loc);
					delete this;
					//	Assignment will call resolve.
					return e;
				}
			}
			else
			{
				if (!Prop->GetFunc)
				{
					ParseError(Loc, "Property %s cannot be read", *Name);
					delete this;
					return NULL;
				}
				VExpression* e = new VInvocation(NULL, Prop->GetFunc, NULL,
					false, false, Loc, 0, NULL);
				delete this;
				return e->Resolve(ec);
			}
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

	Const = (VConstant*)VMemberBase::StaticFindMember(Name, ANY_PACKAGE,
		MEMBER_Const);
	if (Const)
	{
		VExpression* e = new VConstantValue(Const, Loc);
		delete this;
		return e->Resolve(ec);
	}

	ParseError(Loc, "Illegal expression identifier %s", *Name);
	delete this;
	return NULL;
}

//==========================================================================
//
//	VSingleName::DoResolve
//
//==========================================================================

VExpression* VSingleName::DoResolve(VEmitContext& ec)
{
	return IntResolve(ec, false);
}

//==========================================================================
//
//	VSingleName::ResolveAssignmentTarget
//
//==========================================================================

VExpression* VSingleName::ResolveAssignmentTarget(VEmitContext& ec)
{
	return IntResolve(ec, true);
}

//==========================================================================
//
//	VSingleName::ResolveAsType
//
//==========================================================================

VTypeExpr* VSingleName::ResolveAsType(VEmitContext& ec)
{
	Type = VMemberBase::CheckForType(ec.SelfClass, Name);
	if (Type.Type == TYPE_Unknown)
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
	if (Type.Type == TYPE_Unknown)
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

	if (op->Type.Type != TYPE_Pointer)
	{
		ParseError(Loc, "Pointer type required on left side of ->");
		delete this;
		return NULL;
	}
	VFieldType type = op->Type.GetPointerInnerType();
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
//	VDotField::IntResolve
//
//==========================================================================

VExpression* VDotField::IntResolve(VEmitContext& ec, bool AssignTarget)
{
	if (op)
		op = op->Resolve(ec);
	if (!op)
	{
		delete this;
		return NULL;
	}

	if (op->Type.Type == TYPE_Reference)
	{
		VMethod* M = op->Type.Class->CheckForMethod(FieldName);
		if (M)
		{
			VExpression* e = new VDelegateVal(op, M, Loc);
			op = NULL;
			delete this;
			return e->Resolve(ec);
		}

		VField* field = op->Type.Class->CheckForField(Loc, FieldName, ec.SelfClass);
		if (field)
		{
			VExpression* e = new VFieldAccess(op, field, Loc,
				op->IsDefaultObject() ? FIELD_ReadOnly : 0);
			op = NULL;
			delete this;
			return e->Resolve(ec);
		}

		VProperty* Prop = op->Type.Class->CheckForProperty(FieldName);
		if (Prop)
		{
			if (AssignTarget)
			{
				if (!Prop->SetFunc)
				{
					ParseError(Loc, "Property %s cannot be set", *FieldName);
					delete this;
					return NULL;
				}
				VExpression* e = new VPropertyAssign(op, Prop->SetFunc, true, Loc);
				op = NULL;
				delete this;
				//	Assignment will call resolve.
				return e;
			}
			else
			{
				if (op->IsDefaultObject())
				{
					if (!Prop->DefaultField)
					{
						ParseError(Loc, "Property %s has no default field set", *FieldName);
						delete this;
						return NULL;
					}
					VExpression* e = new VFieldAccess(op, Prop->DefaultField,
						Loc, FIELD_ReadOnly);
					op = NULL;
					delete this;
					return e->Resolve(ec);
				}
				else
				{
					if (!Prop->GetFunc)
					{
						ParseError(Loc, "Property %s cannot be read", *FieldName);
						delete this;
						return NULL;
					}
					VExpression* e = new VInvocation(op, Prop->GetFunc, NULL,
						true, false, Loc, 0, NULL);
					op = NULL;
					delete this;
					return e->Resolve(ec);
				}
			}
		}

		ParseError(Loc, "No such field %s", *FieldName);
		delete this;
		return NULL;
	}
	else if (op->Type.Type == TYPE_Struct || op->Type.Type == TYPE_Vector)
	{
		VFieldType type = op->Type;
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
	else if (op->Type.Type == TYPE_DynamicArray)
	{
		VFieldType type = op->Type;
		op->Flags &= ~FIELD_ReadOnly;
		op->RequestAddressOf();
		if (FieldName == NAME_Num)
		{
			if (AssignTarget)
			{
				VExpression* e = new VDynArraySetNum(op, NULL, Loc);
				op = NULL;
				delete this;
				return e->Resolve(ec);
			}
			else
			{
				VExpression* e = new VDynArrayGetNum(op, Loc);
				op = NULL;
				delete this;
				return e->Resolve(ec);
			}
		}
		else
		{
			ParseError(Loc, "No such field %s", *FieldName);
			delete this;
			return NULL;
		}
	}
	ParseError(Loc, "Reference, struct or vector expected on left side of . %d", op->Type.Type);
	delete this;
	return NULL;
}

//==========================================================================
//
//	VDotField::DoResolve
//
//==========================================================================

VExpression* VDotField::DoResolve(VEmitContext& ec)
{
	return IntResolve(ec, false);
}

//==========================================================================
//
//	VDotField::ResolveAssignmentTarget
//
//==========================================================================

VExpression* VDotField::ResolveAssignmentTarget(VEmitContext& ec)
{
	return IntResolve(ec, true);
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

//BEGIN VDefaultObject

//==========================================================================
//
//	VDefaultObject::VDefaultObject
//
//==========================================================================

VDefaultObject::VDefaultObject(VExpression* AOp, const TLocation& ALoc)
: VExpression(ALoc)
, op(AOp)
{
}

//==========================================================================
//
//	VDefaultObject::~VDefaultObject
//
//==========================================================================

VDefaultObject::~VDefaultObject()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VDefaultObject::DoResolve
//
//==========================================================================

VExpression* VDefaultObject::DoResolve(VEmitContext& ec)
{
	if (op)
		op = op->Resolve(ec);
	if (!op)
	{
		delete this;
		return NULL;
	}

	if (op->Type.Type == TYPE_Reference)
	{
		Type = op->Type;
		return this;
	}
	else if (op->Type.Type == TYPE_Class)
	{
		if (!op->Type.Class)
		{
			ParseError(Loc, "A typed class value required");
			delete this;
			return NULL;
		}
		Type = VFieldType(op->Type.Class);
		return this;
	}

	ParseError(Loc, "Reference or class expected on left side of default");
	delete this;
	return NULL;
}

//==========================================================================
//
//	VDefaultObject::Emit
//
//==========================================================================

void VDefaultObject::Emit(VEmitContext& ec)
{
	op->Emit(ec);
	if (op->Type.Type == TYPE_Reference)
	{
		ec.AddStatement(OPC_GetDefaultObj);
	}
	else if (op->Type.Type == TYPE_Class)
	{
		ec.AddStatement(OPC_GetClassDefaultObj);
	}
}

//==========================================================================
//
//	VDefaultObject::IsDefaultObject
//
//==========================================================================

bool VDefaultObject::IsDefaultObject() const
{
	return true;
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
			if (M->Flags & FUNC_Iterator)
			{
				ParseError(Loc, "Iterator methods can only be used in foreach statements");
				delete this;
				return NULL;
			}
			VExpression* e = new VInvocation(NULL, M, NULL,
				false, false, Loc, NumArgs, Args);
			NumArgs = 0;
			delete this;
			return e->Resolve(ec);
		}
		VField* field = ec.SelfClass->CheckForField(Loc, Name, ec.SelfClass);
		if (field && field->Type.Type == TYPE_Delegate)
		{
			VExpression* e = new VInvocation(NULL, field->Func, field,
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
//	VCastOrInvocation::ResolveIterator
//
//==========================================================================

VExpression* VCastOrInvocation::ResolveIterator(VEmitContext& ec)
{
	VMethod* M = ec.SelfClass->CheckForMethod(Name);
	if (!M)
	{
		ParseError(Loc, "Unknown method %s", *Name);
		delete this;
		return NULL;
	}
	if (!(M->Flags & FUNC_Iterator))
	{
		ParseError(Loc, "%s is not an iterator method", *Name);
		delete this;
		return NULL;
	}

	VExpression* e = new VInvocation(NULL, M, NULL, false, false, Loc,
		NumArgs, Args);
	NumArgs = 0;
	delete this;
	return e->Resolve(ec);
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

	if (SelfExpr->Type.Type == TYPE_DynamicArray)
	{
		if (MethodName == NAME_Insert)
		{
			if (NumArgs == 1)
			{
				//	Default count is 1
				Args[1] = new VIntLiteral(1, Loc);
				NumArgs = 2;
			}
			if (NumArgs != 2)
			{
				ParseError(Loc, "Insert requires 1 or 2 arguments");
				delete this;
				return NULL;
			}
			VExpression* e = new VDynArrayInsert(SelfExpr, Args[0], Args[1], Loc);
			SelfExpr = NULL;
			NumArgs = 0;
			delete this;
			return e->Resolve(ec);
		}

		if (MethodName == NAME_Remove)
		{
			if (NumArgs == 1)
			{
				//	Default count is 1
				Args[1] = new VIntLiteral(1, Loc);
				NumArgs = 2;
			}
			if (NumArgs != 2)
			{
				ParseError(Loc, "Insert requires 1 or 2 arguments");
				delete this;
				return NULL;
			}
			VExpression* e = new VDynArrayRemove(SelfExpr, Args[0], Args[1], Loc);
			SelfExpr = NULL;
			NumArgs = 0;
			delete this;
			return e->Resolve(ec);
		}

		ParseError(Loc, "Invalid operation on dynamic array");
		delete this;
		return NULL;
	}

	if (SelfExpr->Type.Type != TYPE_Reference)
	{
		ParseError(Loc, "Object reference expected left side of .");
		delete this;
		return NULL;
	}

	VMethod* M = SelfExpr->Type.Class->CheckForMethod(MethodName);
	if (M)
	{
		if (M->Flags & FUNC_Iterator)
		{
			ParseError(Loc, "Iterator methods can only be used in foreach statements");
			delete this;
			return NULL;
		}
		VExpression* e = new VInvocation(SelfExpr, M, NULL, true,
			false, Loc, NumArgs, Args);
		SelfExpr = NULL;
		NumArgs = 0;
		delete this;
		return e->Resolve(ec);
	}

	VField* field = SelfExpr->Type.Class->CheckForField(Loc, MethodName,
		ec.SelfClass);
	if (field && field->Type.Type == TYPE_Delegate)
	{
		VExpression* e = new VInvocation(SelfExpr, field->Func, field, true,
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
//	VDotInvocation::ResolveIterator
//
//==========================================================================

VExpression* VDotInvocation::ResolveIterator(VEmitContext& ec)
{
	if (SelfExpr)
		SelfExpr = SelfExpr->Resolve(ec);
	if (!SelfExpr)
	{
		delete this;
		return NULL;
	}

	if (SelfExpr->Type.Type != TYPE_Reference)
	{
		ParseError(Loc, "Object reference expected left side of .");
		delete this;
		return NULL;
	}

	VMethod* M = SelfExpr->Type.Class->CheckForMethod(MethodName);
	if (!M)
	{
		ParseError(Loc, "No such method %s", *MethodName);
		delete this;
		return NULL;
	}
	if (!(M->Flags & FUNC_Iterator))
	{
		ParseError(Loc, "%s is not an iterator method", *MethodName);
		delete this;
		return NULL;
	}
	
	VExpression* e = new VInvocation(SelfExpr, M, NULL, true,
		false, Loc, NumArgs, Args);
	SelfExpr = NULL;
	NumArgs = 0;
	delete this;
	return e->Resolve(ec);
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

	if (op->Type.Type != TYPE_Pointer)
	{
		ParseError(Loc, "Expression syntax error");
		delete this;
		return NULL;
	}
	Type = op->Type.GetPointerInnerType();
	RealType = Type;
	if (Type.Type == TYPE_Byte || Type.Type == TYPE_Bool)
	{
		Type = VFieldType(TYPE_Int);
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
	if (RealType.Type == TYPE_Void)
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
	if (op1->Type.Type == TYPE_Pointer && op1->Type.InnerType == TYPE_Void)
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
		op1 = op1->ResolveAssignmentTarget(ec);
	if (op2)
		op2 = op2->Resolve(ec);
	if (!op1 || !op2)
	{
		delete this;
		return NULL;
	}

	if (op1->IsPropertyAssign())
	{
		if (Oper != Assign)
		{
			ParseError(Loc, "Only = can be used to assign to a property");
			delete this;
			return NULL;
		}
		VPropertyAssign* e = (VPropertyAssign*)op1;
		e->NumArgs = 1;
		e->Args[0] = op2;
		op1 = NULL;
		op2 = NULL;
		delete this;
		return e->Resolve(ec);
	}

	if (op1->IsDynArraySetNum())
	{
		if (Oper != Assign)
		{
			ParseError(Loc, "Only = can be used to resize an array");
			delete this;
			return NULL;
		}
		op2->Type.CheckMatch(Loc, VFieldType(TYPE_Int));
		VDynArraySetNum* e = (VDynArraySetNum*)op1;
		e->NumExpr = op2;
		op1 = NULL;
		op2 = NULL;
		delete this;
		return e->Resolve(ec);
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
		if (op1->RealType.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_AssignDrop);
		}
		else if (op1->RealType.Type == TYPE_Byte && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_ByteAssignDrop);
		}
		else if (op1->RealType.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_AssignDrop);
		}
		else if (op1->RealType.Type == TYPE_Name && op2->Type.Type == TYPE_Name)
		{
			ec.AddStatement(OPC_AssignDrop);
		}
		else if (op1->RealType.Type == TYPE_String && op2->Type.Type == TYPE_String)
		{
			ec.AddStatement(OPC_AssignStrDrop);
		}
		else if (op1->RealType.Type == TYPE_Pointer && op2->Type.Type == TYPE_Pointer)
		{
			ec.AddStatement(OPC_AssignPtrDrop);
		}
		else if (op1->RealType.Type == TYPE_Vector && op2->Type.Type == TYPE_Vector)
		{
			ec.AddStatement(OPC_VAssignDrop);
		}
		else if (op1->RealType.Type == TYPE_Class && (op2->Type.Type == TYPE_Class ||
			(op2->Type.Type == TYPE_Reference && op2->Type.Class == NULL)))
		{
			ec.AddStatement(OPC_AssignPtrDrop);
		}
		else if (op1->RealType.Type == TYPE_State && (op2->Type.Type == TYPE_State ||
			(op2->Type.Type == TYPE_Reference && op2->Type.Class == NULL)))
		{
			ec.AddStatement(OPC_AssignPtrDrop);
		}
		else if (op1->RealType.Type == TYPE_Reference && op2->Type.Type == TYPE_Reference)
		{
			ec.AddStatement(OPC_AssignPtrDrop);
		}
		else if (op1->RealType.Type == TYPE_Bool && op2->Type.Type == TYPE_Int)
		{
			if (op1->RealType.BitMask & 0x000000ff)
				ec.AddStatement(OPC_AssignBool0, (int)op1->RealType.BitMask);
			else if (op1->RealType.BitMask & 0x0000ff00)
				ec.AddStatement(OPC_AssignBool1, (int)(op1->RealType.BitMask >> 8));
			else if (op1->RealType.BitMask & 0x00ff0000)
				ec.AddStatement(OPC_AssignBool2, (int)(op1->RealType.BitMask >> 16));
			else
				ec.AddStatement(OPC_AssignBool3, (int)(op1->RealType.BitMask >> 24));
		}
		else if (op1->RealType.Type == TYPE_Delegate && op2->Type.Type == TYPE_Delegate)
		{
			ec.AddStatement(OPC_AssignDelegate);
		}
		else if (op1->RealType.Type == TYPE_Delegate && op2->Type.Type == TYPE_Reference && op2->Type.Class == NULL)
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
		if (op1->RealType.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_AddVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Byte && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_ByteAddVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FAddVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Vector && op2->Type.Type == TYPE_Vector)
		{
			ec.AddStatement(OPC_VAddVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case MinusAssign:
		if (op1->RealType.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_SubVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Byte && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_ByteSubVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FSubVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Vector && op2->Type.Type == TYPE_Vector)
		{
			ec.AddStatement(OPC_VSubVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case MultiplyAssign:
		if (op1->RealType.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_MulVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Byte && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_ByteMulVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FMulVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Vector && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_VScaleVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case DivideAssign:
		if (op1->RealType.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_DivVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Byte && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_ByteDivVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FDivVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Vector && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_VIScaleVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case ModAssign:
		if (op1->RealType.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_ModVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Byte && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_ByteModVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case AndAssign:
		if (op1->RealType.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_AndVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Byte && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_ByteAndVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case OrAssign:
		if (op1->RealType.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_OrVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Byte && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_ByteOrVarDrop);
		}
//FIXME This is wrong!
		else if (op1->RealType.Type == TYPE_Bool && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_OrVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case XOrAssign:
		if (op1->RealType.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_XOrVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Byte && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_ByteXOrVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case LShiftAssign:
		if (op1->RealType.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_LShiftVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Byte && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_ByteLShiftVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;

	case RShiftAssign:
		if (op1->RealType.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_RShiftVarDrop);
		}
		else if (op1->RealType.Type == TYPE_Byte && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_ByteRShiftVarDrop);
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
		}
		break;
	}
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
	Type = TYPE_Class;
	Type.Class = Class;
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
	if (Const->Type == TYPE_Int)
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
	if (Const->Type == TYPE_Float)
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
	return Const->Type == TYPE_Int;
}

//==========================================================================
//
//	VConstantValue::IsFloatConst
//
//==========================================================================

bool VConstantValue::IsFloatConst() const
{
	return Const->Type == TYPE_Float;
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
	Type = ec.LocalDefs[num].Type;
	RealType = ec.LocalDefs[num].Type;
	if (Type.Type == TYPE_Byte || Type.Type == TYPE_Bool)
	{
		Type = VFieldType(TYPE_Int);
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
		ec.EmitLocalAddress(ec.LocalDefs[num].Offset);
	}
	else if (ec.LocalDefs[num].ParamFlags & FPARM_Out)
	{
		if (ec.LocalDefs[num].Offset < 256)
		{
			int Ofs = ec.LocalDefs[num].Offset;
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
			ec.EmitLocalAddress(ec.LocalDefs[num].Offset);
			ec.AddStatement(OPC_PushPointedPtr);
		}
		if (PushOutParam)
		{
			EmitPushPointedCode(ec.LocalDefs[num].Type, ec);
		}
	}
	else if (ec.LocalDefs[num].Offset < 256)
	{
		int Ofs = ec.LocalDefs[num].Offset;
		if (ec.LocalDefs[num].Type.Type == TYPE_Bool &&
			ec.LocalDefs[num].Type.BitMask != 1)
		{
			ParseError(Loc, "Strange local bool mask");
		}
		switch (ec.LocalDefs[num].Type.Type)
		{
		case TYPE_Int:
		case TYPE_Byte:
		case TYPE_Bool:
		case TYPE_Float:
		case TYPE_Name:
		case TYPE_Pointer:
		case TYPE_Reference:
		case TYPE_Class:
		case TYPE_State:
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

		case TYPE_Vector:
			ec.AddStatement(OPC_VLocalValueB, Ofs);
			break;

		case TYPE_String:
			ec.AddStatement(OPC_StrLocalValueB, Ofs);
			break;

		default:
			ParseError(Loc, "Invalid operation of this variable type");
		}
	}
	else
	{
		ec.EmitLocalAddress(ec.LocalDefs[num].Offset);
		EmitPushPointedCode(ec.LocalDefs[num].Type, ec);
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
	Flags = field->Flags | ExtraFlags;
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
	Type = field->Type;
	RealType = field->Type;
	if (Type.Type == TYPE_Byte || Type.Type == TYPE_Bool)
	{
		Type = VFieldType(TYPE_Int);
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
		switch (field->Type.Type)
		{
		case TYPE_Int:
		case TYPE_Float:
		case TYPE_Name:
			ec.AddStatement(OPC_FieldValue, field);
			break;

		case TYPE_Byte:
			ec.AddStatement(OPC_ByteFieldValue, field);
			break;

		case TYPE_Bool:
			if (field->Type.BitMask & 0x000000ff)
				ec.AddStatement(OPC_Bool0FieldValue, field, (int)(field->Type.BitMask));
			else if (field->Type.BitMask & 0x0000ff00)
				ec.AddStatement(OPC_Bool1FieldValue, field, (int)(field->Type.BitMask >> 8));
			else if (field->Type.BitMask & 0x00ff0000)
				ec.AddStatement(OPC_Bool2FieldValue, field, (int)(field->Type.BitMask >> 16));
			else
				ec.AddStatement(OPC_Bool3FieldValue, field, (int)(field->Type.BitMask >> 24));
			break;

		case TYPE_Pointer:
		case TYPE_Reference:
		case TYPE_Class:
		case TYPE_State:
			ec.AddStatement(OPC_PtrFieldValue, field);
			break;

		case TYPE_Vector:
			ec.AddStatement(OPC_VFieldValue, field);
			break;

		case TYPE_String:
			ec.AddStatement(OPC_StrFieldValue, field);
			break;

		case TYPE_Delegate:
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
	Type = TYPE_Delegate;
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
	if (Type.Type == TYPE_Byte || Type.Type == TYPE_Bool)
		Type = VFieldType(TYPE_Int);
	if (Func->Flags & FUNC_Spawner)
		Type.Class = Args[0]->Type.Class;
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
			if (ec.CurrentFunc->Flags & FUNC_Static)
			{
				ParseError(Loc, "An object is required to call non-static methods");
			}
			ec.AddStatement(OPC_LocalValue0);
		}
	}

	vint32 SelfOffset = 1;
	for (int i = 0; i < NumArgs; i++)
	{
		if (!Args[i])
		{
			switch (Func->ParamTypes[i].Type)
			{
			case TYPE_Int:
			case TYPE_Byte:
			case TYPE_Bool:
			case TYPE_Float:
			case TYPE_Name:
				ec.EmitPushNumber(0);
				SelfOffset++;
				break;

			case TYPE_String:
			case TYPE_Pointer:
			case TYPE_Reference:
			case TYPE_Class:
			case TYPE_State:
				ec.AddStatement(OPC_PushNull);
				SelfOffset++;
				break;

			case TYPE_Vector:
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
			if (Args[i]->Type.Type == TYPE_Vector)
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
				argsize += Func->ParamTypes[i].GetStackSize();
			}
			else
			{
				if (Func->ParamFlags[i] & FPARM_Out)
				{
					if (!Args[i]->Type.Equals(Func->ParamTypes[i]))
					{
						//FIXME This should be error.
						Args[i]->Type.CheckMatch(Args[i]->Loc, Func->ParamTypes[i]);
						//ParseError(Args[i]->Loc, "Out parameter types must be equal");
					}
					Args[i]->RequestAddressOf();
				}
				else
				{
					Args[i]->Type.CheckMatch(Args[i]->Loc, Func->ParamTypes[i]);
				}
				argsize += Args[i]->Type.GetStackSize();
			}
		}
		else if (!Args[i])
		{
			ParseError(Loc, "Bad expresion");
		}
		else
		{
			argsize += Args[i]->Type.GetStackSize();
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

//BEGIN VPropertyAssign

//==========================================================================
//
//	VPropertyAssign::VPropertyAssign
//
//==========================================================================

VPropertyAssign::VPropertyAssign(VExpression* ASelfExpr, VMethod* AFunc,
	bool AHaveSelf, const TLocation& ALoc)
: VInvocation(ASelfExpr, AFunc, NULL, AHaveSelf, false, ALoc, 0, NULL)
{
}

//==========================================================================
//
//	VPropertyAssign::IsPropertyAssign
//
//==========================================================================

bool VPropertyAssign::IsPropertyAssign() const
{
	return true;
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

	if (op->Type.Type == TYPE_Delegate)
	{
		ParseError(Loc, "Delegate call parameters are missing");
		delete this;
		return NULL;
	}

	if (op->Type.Type != TYPE_String && op->Type.GetStackSize() != 4 &&
		op->Type.Type != TYPE_Vector && op->Type.Type != TYPE_Void)
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

	Type = TYPE_Void;
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
	if (op->Type.Type == TYPE_String)
	{
		ec.AddStatement(OPC_DropStr);
	}
	else if (op->Type.Type == TYPE_Vector)
	{
		ec.AddStatement(OPC_VDrop);
	}
	else if (op->Type.GetStackSize() == 4)
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
		Type.Class = VMemberBase::CheckForClass(MetaClassName);
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

//END

//BEGIN VPointerType

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

//BEGIN VDynamicArrayType

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

//END

//BEGIN VDynArrayGetNum

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
		delete ArrayExpr;
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

//END

//BEGIN VDynArraySetNum

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
		delete ArrayExpr;
	if (NumExpr)
		delete NumExpr;
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

//END

//BEGIN VDynArrayInsert

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
		delete ArrayExpr;
	if (IndexExpr)
		delete IndexExpr;
	if (CountExpr)
		delete CountExpr;
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

//END

//BEGIN VDynArrayRemove

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
		delete ArrayExpr;
	if (IndexExpr)
		delete IndexExpr;
	if (CountExpr)
		delete CountExpr;
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
		VFieldType Type = e.TypeExpr->Type;
		if (Type.Type == TYPE_Void)
		{
			ParseError(e.TypeExpr->Loc, "Bad variable type");
		}

		VLocalVarDef& L = ec.LocalDefs.Alloc();
		L.Name = e.Name;
		L.Type = Type;
		L.Offset = ec.localsofs;
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

		ec.localsofs += Type.GetStackSize() / 4;
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
