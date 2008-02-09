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

#ifdef IN_VCC
#include "../utils/vcc/vcc.h"
#else
#include "vc_local.h"
#endif

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

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
	VField* field = type.Struct->FindField(FieldName);
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
		VMethod* M = op->Type.Class->FindMethod(FieldName);
		if (M)
		{
			VExpression* e = new VDelegateVal(op, M, Loc);
			op = NULL;
			delete this;
			return e->Resolve(ec);
		}

		VField* field = op->Type.Class->FindField(FieldName, Loc, ec.SelfClass);
		if (field)
		{
			VExpression* e = new VFieldAccess(op, field, Loc,
				op->IsDefaultObject() ? FIELD_ReadOnly : 0);
			op = NULL;
			delete this;
			return e->Resolve(ec);
		}

		VProperty* Prop = op->Type.Class->FindProperty(FieldName);
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
		VField* field = type.Struct->FindField(FieldName);
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
