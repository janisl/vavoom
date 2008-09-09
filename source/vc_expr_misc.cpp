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
		VConstant* Const = ec.SelfClass->FindConstant(Name);
		if (Const)
		{
			VExpression* e = new VConstantValue(Const, Loc);
			delete this;
			return e->Resolve(ec);
		}

		VMethod* M = ec.SelfClass->FindMethod(Name);
		if (M)
		{
			VExpression* e = new VDelegateVal((new VSelf(Loc))->Resolve(ec), M, Loc);
			delete this;
			return e->Resolve(ec);
		}

		VField* field = ec.SelfClass->FindField(Name, Loc, ec.SelfClass);
		if (field)
		{
			VExpression* e = new VFieldAccess((new VSelf(Loc))->Resolve(ec), field, Loc, 0);
			delete this;
			return e->Resolve(ec);
		}

		VProperty* Prop = ec.SelfClass->FindProperty(Name);
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

	VConstant* Const = ec.Package->FindConstant(Name);
	if (Const)
	{
		VExpression* e = new VConstantValue(Const, Loc);
		delete this;
		return e->Resolve(ec);
	}

	VClass* Class = VMemberBase::StaticFindClass(Name);
	if (Class)
	{
		VExpression* e = new VClassConstant(Class, Loc);
		delete this;
		return e->Resolve(ec);
	}

	Class = ec.Package->FindDecorateImportClass(Name);
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
	Type = VMemberBase::StaticFindType(ec.SelfClass, Name);
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
	VClass* Class = VMemberBase::StaticFindClass(Name1);
	if (!Class)
	{
		ParseError(Loc, "No such class %s", *Name1);
		delete this;
		return NULL;
	}

	VConstant* Const = Class->FindConstant(Name2);
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
	VClass* Class = VMemberBase::StaticFindClass(Name1);
	if (!Class)
	{
		ParseError(Loc, "No such class %s", *Name1);
		delete this;
		return NULL;
	}

	Type = VMemberBase::StaticFindType(Class, Name2);
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

//BEGIN VClassConstant

//==========================================================================
//
//	VStateConstant::VStateConstant
//
//==========================================================================

VStateConstant::VStateConstant(VState* AState, const TLocation& ALoc)
: VExpression(ALoc)
, State(AState)
{
	Type = TYPE_State;
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
