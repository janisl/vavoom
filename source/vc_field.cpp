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
#ifndef IN_VCC
#include "network.h"
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
//	VField::VField
//
//==========================================================================

VField::VField(VName AName, VMemberBase* AOuter, TLocation ALoc)
: VMemberBase(MEMBER_Field, AName, AOuter, ALoc)
, Next(NULL)
, Type(TYPE_Void)
, Func(NULL)
, Flags(0)
, ReplCond(NULL)
, TypeExpr(NULL)
, NextReference(0)
, DestructorLink(0)
, NextNetField(0)
, Ofs(0)
, NetIndex(-1)
{
}

//==========================================================================
//
//	VField::~VField
//
//==========================================================================

VField::~VField()
{
	if (TypeExpr)
		delete TypeExpr;
}

//==========================================================================
//
//	VField::Serialise
//
//==========================================================================

void VField::Serialise(VStream& Strm)
{
	guard(VField::Serialise);
	VMemberBase::Serialise(Strm);
	Strm << Next
		<< Type
		<< Func
		<< STRM_INDEX(Flags)
		<< ReplCond;
	unguard;
}

//==========================================================================
//
//	VField::NeedsDestructor
//
//==========================================================================

bool VField::NeedsDestructor() const
{
	if (Type.Type == TYPE_String)
		return true;
	if (Type.Type == TYPE_DynamicArray)
		return true;
	if (Type.Type == TYPE_Array)
	{
		if (Type.ArrayInnerType == TYPE_String)
			return true;
		if (Type.ArrayInnerType == TYPE_Struct)
			return Type.Struct->NeedsDestructor();
	}
	if (Type.Type == TYPE_Struct)
		return Type.Struct->NeedsDestructor();
	return false;
}

//==========================================================================
//
//	VField::Define
//
//==========================================================================

bool VField::Define()
{
	if (Type.Type == TYPE_Delegate)
	{
		return Func->Define();
	}

	if (TypeExpr)
	{
		VEmitContext ec(this);
		TypeExpr = TypeExpr->ResolveAsType(ec);
	}
	if (!TypeExpr)
	{
		return false;
	}

	if (TypeExpr->Type.Type == TYPE_Void)
	{
		ParseError(TypeExpr->Loc, "Field cannot have void type.");
		return false;
	}
	Type = TypeExpr->Type;
	return true;
}

#ifndef IN_VCC

//==========================================================================
//
//	VField::CopyFieldValue
//
//==========================================================================

void VField::CopyFieldValue(const vuint8* Src, vuint8* Dst,
	const VFieldType& Type)
{
	guardSlow(VField::CopyFieldValue);
	switch (Type.Type)
	{
	case TYPE_Int:
		*(vint32*)Dst = *(const vint32*)Src;
		break;

	case TYPE_Byte:
		*(vuint8*)Dst = *(const vuint8*)Src;
		break;

	case TYPE_Bool:
		if (*(const vuint32*)Src & Type.BitMask)
			*(vuint32*)Dst |= Type.BitMask;
		else
			*(vuint32*)Dst &= ~Type.BitMask;
		break;

	case TYPE_Float:
		*(float*)Dst = *(const float*)Src;
		break;

	case TYPE_Vector:
		*(TVec*)Dst = *(const TVec*)Src;
		break;

	case TYPE_Name:
		*(VName*)Dst = *(const VName*)Src;
		break;

	case TYPE_String:
		*(VStr*)Dst = *(const VStr*)Src;
		break;

	case TYPE_Pointer:
		*(void**)Dst = *(void*const*)Src;
		break;

	case TYPE_Reference:
		*(VObject**)Dst = *(VObject*const*)Src;
		break;

	case TYPE_Class:
		*(VClass**)Dst = *(VClass*const*)Src;
		break;

	case TYPE_State:
		*(VState**)Dst = *(VState*const*)Src;
		break;

	case TYPE_Delegate:
		*(VObjectDelegate*)Dst = *(const VObjectDelegate*)Src;
		break;

	case TYPE_Struct:
		Type.Struct->CopyObject(Src, Dst);
		break;

	case TYPE_Array:
	{
		VFieldType IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		int InnerSize = IntType.GetSize();
		for (int i = 0; i < Type.ArrayDim; i++)
		{
			CopyFieldValue(Src + i * InnerSize, Dst + i * InnerSize, IntType);
		}
	}
		break;

	case TYPE_DynamicArray:
	{
		VScriptArray& ASrc = *(VScriptArray*)Src;
		VScriptArray& ADst = *(VScriptArray*)Dst;
		VFieldType IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		int InnerSize = IntType.GetSize();
		ADst.SetNum(ASrc.Num(), IntType);
		for (int i = 0; i < ASrc.Num(); i++)
		{
			CopyFieldValue(ASrc.Ptr() + i * InnerSize,
				ADst.Ptr() + i * InnerSize, IntType);
		}
	}
		break;
	}
	unguardSlow;
}

//==========================================================================
//
//	VField::SerialiseFieldValue
//
//==========================================================================

void VField::SerialiseFieldValue(VStream& Strm, vuint8* Data, const VFieldType& Type)
{
	guard(VField::SerialiseFieldValue);
	VFieldType IntType;
	int InnerSize;
	switch (Type.Type)
	{
	case TYPE_Int:
		Strm << *(vint32*)Data;
		break;

	case TYPE_Byte:
		Strm << *Data;
		break;

	case TYPE_Bool:
		if (Strm.IsLoading())
		{
			vuint8 Val;
			Strm << Val;
			if (Val)
				*(int*)Data |= Type.BitMask;
			else
				*(int*)Data &= ~Type.BitMask;
		}
		else
		{
			vuint8 Val = !!(*(int*)Data & Type.BitMask);
			Strm << Val;
		}
		break;

	case TYPE_Float:
		Strm << *(float*)Data;
		break;

	case TYPE_Vector:
		Strm << *(TVec*)Data;
		break;

	case TYPE_Name:
		Strm << *(VName*)Data;
		break;

	case TYPE_String:
		Strm << *(VStr*)Data;
		break;

	case TYPE_Pointer:
		if (Type.InnerType == TYPE_Struct)
			Strm.SerialiseStructPointer(*(void**)Data, Type.Struct);
		else
		{
			dprintf("Don't know how to serialise pointer type %d\n", Type.InnerType);
			Strm << *(int*)Data;
		}
		break;

	case TYPE_Reference:
		Strm << *(VObject**)Data;
		break;

	case TYPE_Class:
		if (Strm.IsLoading())
		{
			VName CName;
			Strm << CName;
			if (CName != NAME_None)
			{
				*(VClass**)Data = VClass::FindClass(*CName);
			}
			else
			{
				*(VClass**)Data = NULL;
			}
		}
		else
		{
			VName CName = NAME_None;
			if (*(VClass**)Data)
			{
				CName = (*(VClass**)Data)->GetVName();
			}
			Strm << CName;
		}
		break;

	case TYPE_State:
		if (Strm.IsLoading())
		{
			VName CName;
			VName SName;
			Strm << CName << SName;
			if (SName != NAME_None)
			{
				*(VState**)Data = VClass::FindClass(*CName)->FindStateChecked(SName);
			}
			else
			{
				*(VState**)Data = NULL;
			}
		}
		else
		{
			VName CName = NAME_None;
			VName SName = NAME_None;
			if (*(VState**)Data)
			{
				CName = (*(VState**)Data)->Outer->GetVName();
				SName = (*(VState**)Data)->Name;
			}
			Strm << CName << SName;
		}
		break;

	case TYPE_Delegate:
		Strm << ((VObjectDelegate*)Data)->Obj;
		if (Strm.IsLoading())
		{
			VName FuncName;
			Strm << FuncName;
			if (((VObjectDelegate*)Data)->Obj)
				((VObjectDelegate*)Data)->Func = ((VObjectDelegate*)Data)->Obj->GetVFunction(FuncName);
		}
		else
		{
			VName FuncName = NAME_None;
			if (((VObjectDelegate*)Data)->Obj)
				FuncName = ((VObjectDelegate*)Data)->Func->Name;
			Strm << FuncName;
		}
		break;

	case TYPE_Struct:
		Type.Struct->SerialiseObject(Strm, Data);
		break;

	case TYPE_Array:
		IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		InnerSize = IntType.GetSize();
		for (int i = 0; i < Type.ArrayDim; i++)
		{
			SerialiseFieldValue(Strm, Data + i * InnerSize, IntType);
		}
		break;

	case TYPE_DynamicArray:
	{
		VScriptArray& A = *(VScriptArray*)Data;
		IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		InnerSize = IntType.GetSize();
		vint32 ArrNum = A.Num();
		Strm << STRM_INDEX(ArrNum);
		if (Strm.IsLoading())
		{
			A.SetNum(ArrNum, IntType);
		}
		for (int i = 0; i < A.Num(); i++)
		{
			SerialiseFieldValue(Strm, A.Ptr() + i * InnerSize, IntType);
		}
	}
		break;
	}
	unguard;
}

//==========================================================================
//
//	VField::CleanField
//
//==========================================================================

void VField::CleanField(vuint8* Data, const VFieldType& Type)
{
	guard(CleanField);
	VFieldType IntType;
	int InnerSize;
	switch (Type.Type)
	{
	case TYPE_Reference:
		if (*(VObject**)Data && (*(VObject**)Data)->GetFlags() & _OF_CleanupRef)
		{
			*(VObject**)Data = NULL;
		}
		break;

	case TYPE_Delegate:
		if (((VObjectDelegate*)Data)->Obj && (((VObjectDelegate*)Data)->Obj->GetFlags() & _OF_CleanupRef))
		{
			((VObjectDelegate*)Data)->Obj = NULL;
			((VObjectDelegate*)Data)->Func = NULL;
		}
		break;

	case TYPE_Struct:
		Type.Struct->CleanObject(Data);
		break;

	case TYPE_Array:
		IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		InnerSize = IntType.GetSize();
		for (int i = 0; i < Type.ArrayDim; i++)
		{
			CleanField(Data + i * InnerSize, IntType);
		}
		break;

	case TYPE_DynamicArray:
	{
		VScriptArray& A = *(VScriptArray*)Data;
		IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		InnerSize = IntType.GetSize();
		for (int i = 0; i < A.Num(); i++)
		{
			CleanField(A.Ptr() + i * InnerSize, IntType);
		}
	}
		break;
	}
	unguard;
}

//==========================================================================
//
//	VField::DestructField
//
//==========================================================================

void VField::DestructField(vuint8* Data, const VFieldType& Type)
{
	guard(DestructField);
	VFieldType IntType;
	int InnerSize;
	switch (Type.Type)
	{
	case TYPE_String:
		((VStr*)Data)->Clean();
		break;

	case TYPE_Struct:
		Type.Struct->DestructObject(Data);
		break;

	case TYPE_Array:
		IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		InnerSize = IntType.GetSize();
		for (int i = 0; i < Type.ArrayDim; i++)
		{
			DestructField(Data + i * InnerSize, IntType);
		}
		break;

	case TYPE_DynamicArray:
	{
		IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		((VScriptArray*)Data)->Clear(IntType);
	}
		break;
	}
	unguard;
}

//==========================================================================
//
//	VField::IdenticalValue
//
//==========================================================================

bool VField::IdenticalValue(const vuint8* Val1, const vuint8* Val2,
	const VFieldType& Type)
{
	guard(VField::IdenticalValue);
	VFieldType IntType;
	int InnerSize;
	switch (Type.Type)
	{
	case TYPE_Int:
		return *(const vint32*)Val1 == *(const vint32*)Val2;

	case TYPE_Byte:
		return *(const vuint8*)Val1 == *(const vuint8*)Val2;

	case TYPE_Bool:
		return (*(const vuint32*)Val1 & Type.BitMask) ==
			(*(const vuint32*)Val2 & Type.BitMask);

	case TYPE_Float:
		return *(const float*)Val1 == *(const float*)Val2;

	case TYPE_Vector:
		return *(const TVec*)Val1 == *(const TVec*)Val2;

	case TYPE_Name:
		return *(const VName*)Val1 == *(const VName*)Val2;

	case TYPE_String:
		return *(const VStr*)Val1 == *(const VStr*)Val2;

	case TYPE_Pointer:
		return *(void*const*)Val1 == *(void*const*)Val2;

	case TYPE_Reference:
		return *(VObject*const*)Val1 == *(VObject*const*)Val2;

	case TYPE_Class:
		return *(VClass*const*)Val1 == *(VClass*const*)Val2;

	case TYPE_State:
		return *(VState*const*)Val1 == *(VState*const*)Val2;

	case TYPE_Delegate:
		return ((const VObjectDelegate*)Val1)->Obj == ((const VObjectDelegate*)Val2)->Obj &&
			((const VObjectDelegate*)Val1)->Func == ((const VObjectDelegate*)Val2)->Func;

	case TYPE_Struct:
		return Type.Struct->IdenticalObject(Val1, Val2);

	case TYPE_Array:
		IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		InnerSize = IntType.GetSize();
		for (int i = 0; i < Type.ArrayDim; i++)
		{
			if (!IdenticalValue(Val1 + i * InnerSize, Val2 + i * InnerSize,
				IntType))
			{
				return false;
			}
		}
		return true;

	case TYPE_DynamicArray:
	{
		VScriptArray& Arr1 = *(VScriptArray*)Val1;
		VScriptArray& Arr2 = *(VScriptArray*)Val2;
		if (Arr1.Num() != Arr2.Num())
		{
			return false;
		}
		IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		InnerSize = IntType.GetSize();
		for (int i = 0; i < Type.ArrayDim; i++)
		{
			if (!IdenticalValue(Arr1.Ptr() + i * InnerSize,
				Arr2.Ptr() + i * InnerSize, IntType))
			{
				return false;
			}
		}
	}
		return true;
	}
	Sys_Error("Bad field type");
	return false;
	unguard;
}

//==========================================================================
//
//	VField::NetSerialiseValue
//
//==========================================================================

bool VField::NetSerialiseValue(VStream& Strm, VNetObjectsMap* Map,
	vuint8* Data, const VFieldType& Type)
{
	guard(VField::NetSerialiseValue);
	VFieldType IntType;
	int InnerSize;
	bool Ret = true;
	switch (Type.Type)
	{
	case TYPE_Int:
		Strm << *(vint32*)Data;
		break;

	case TYPE_Byte:
		Strm << *(vuint8*)Data;
		break;

	case TYPE_Bool:
		if (Strm.IsLoading())
		{
			vuint8 Val;
			Strm.SerialiseBits(&Val, 1);
			if (Val)
				*(vuint32*)Data |= Type.BitMask;
			else
				*(vuint32*)Data &= ~Type.BitMask;
		}
		else
		{
			vuint8 Val = (vuint8)!!(*(vuint32*)Data & Type.BitMask);
			Strm.SerialiseBits(&Val, 1);
		}
		break;

	case TYPE_Float:
		Strm << *(float*)Data;
		break;

	case TYPE_Name:
		Ret = Map->SerialiseName(Strm, *(VName*)Data);
		break;

	case TYPE_Vector:
		if (Type.Struct->Name == NAME_TAVec)
		{
			if (Strm.IsLoading())
			{
				vuint8 ByteYaw;
				vuint8 BytePitch = 0;
				vuint8 ByteRoll = 0;
				vuint8 HavePitchRoll = 0;
				Strm << ByteYaw;
				Strm.SerialiseBits(&HavePitchRoll, 1);
				if (HavePitchRoll)
				{
					Strm << BytePitch << ByteRoll;
				}
				((TAVec*)Data)->yaw = ByteToAngle(ByteYaw);
				((TAVec*)Data)->pitch = ByteToAngle(BytePitch);
				((TAVec*)Data)->roll = ByteToAngle(ByteRoll);
			}
			else
			{
				vuint8 ByteYaw = AngleToByte(((TAVec*)Data)->yaw);
				vuint8 BytePitch = AngleToByte(((TAVec*)Data)->pitch);
				vuint8 ByteRoll = AngleToByte(((TAVec*)Data)->roll);
				vuint8 HavePitchRoll = BytePitch || ByteRoll;
				Strm << ByteYaw;
				Strm.SerialiseBits(&HavePitchRoll, 1);
				if (HavePitchRoll)
				{
					Strm << BytePitch << ByteRoll;
				}
			}
		}
		else
		{
			if (Strm.IsLoading())
			{
				vint16 x, y, z;
				Strm << x << y << z;
				((TVec*)Data)->x = x;
				((TVec*)Data)->y = y;
				((TVec*)Data)->z = z;
			}
			else
			{
				vint16 x = mround(((TVec*)Data)->x);
				vint16 y = mround(((TVec*)Data)->y);
				vint16 z = mround(((TVec*)Data)->z);
				Strm << x << y << z;
			}
		}
		break;

	case TYPE_String:
		Strm << *(VStr*)Data;
		break;

	case TYPE_Class:
		Ret = Map->SerialiseClass(Strm, *(VClass**)Data);
		break;

	case TYPE_State:
		Ret = Map->SerialiseState(Strm, *(VState**)Data);
		break;

	case TYPE_Reference:
		Ret = Map->SerialiseObject(Strm, *(VObject**)Data);
		break;

	case TYPE_Struct:
		Ret = Type.Struct->NetSerialiseObject(Strm, Map, Data);
		break;

	case TYPE_Array:
		IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		InnerSize = IntType.GetSize();
		for (int i = 0; i < Type.ArrayDim; i++)
		{
			if (!NetSerialiseValue(Strm, Map, Data + i * InnerSize, IntType))
			{
				Ret = false;
			}
		}
		break;

	default:
		Sys_Error("Replication of field type %d is not supported", Type.Type);
	}
	return Ret;
	unguard;
}

#endif
