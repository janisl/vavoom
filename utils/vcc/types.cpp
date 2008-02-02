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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//BEGIN TType

//==========================================================================
//
//	TType::TType
//
//==========================================================================

TType::TType(VClass* InClass) :
	Type(TYPE_Reference), InnerType(TYPE_Void), ArrayInnerType(TYPE_Void),
	PtrLevel(0), ArrayDim(0), Class(InClass)
{
}

//==========================================================================
//
//	TType::TType
//
//==========================================================================

TType::TType(VStruct* InStruct) :
	Type(InStruct->IsVector ? TYPE_Vector : TYPE_Struct), InnerType(TYPE_Void),
	ArrayInnerType(TYPE_Void), PtrLevel(0), ArrayDim(0), Struct(InStruct)
{
}

//==========================================================================
//
//	operator VStream << TType
//
//==========================================================================

VStream& operator<<(VStream& Strm, TType& T)
{
	Strm << T.Type;
	vuint8 RealType = T.Type;
	if (RealType == TYPE_Array)
	{
		Strm << T.ArrayInnerType
			<< STRM_INDEX(T.ArrayDim);
		RealType = T.ArrayInnerType;
	}
	else if (RealType == TYPE_DynamicArray)
	{
		Strm << T.ArrayInnerType;
		RealType = T.ArrayInnerType;
	}
	if (RealType == TYPE_Pointer)
	{
		Strm << T.InnerType
			<< T.PtrLevel;
		RealType = T.InnerType;
	}
	if (RealType == TYPE_Reference || RealType == TYPE_Class)
		Strm << T.Class;
	else if (RealType == TYPE_Struct || RealType == TYPE_Vector)
		Strm << T.Struct;
	else if (RealType == TYPE_Delegate)
		Strm << T.Function;
	else if (RealType == TYPE_Bool)
		Strm << T.BitMask;
	return Strm;
}

//==========================================================================
//
//	TType::Equals
//
//==========================================================================

bool TType::Equals(const TType& Other) const
{
	if (Type != Other.Type ||
		InnerType != Other.InnerType ||
		ArrayInnerType != Other.ArrayInnerType ||
		PtrLevel != Other.PtrLevel ||
		ArrayDim != Other.ArrayDim ||
		Class != Other.Class)
		return false;
	return true;
}

//==========================================================================
//
//	TType::MakePointerType
//
//==========================================================================

TType TType::MakePointerType() const
{
	TType pointer = *this;
	if (pointer.Type == TYPE_Pointer)
	{
		pointer.PtrLevel++;
	}
	else
	{
		pointer.InnerType = pointer.Type;
		pointer.Type = TYPE_Pointer;
		pointer.PtrLevel = 1;
	}
	return pointer;
}

//==========================================================================
//
//	TType::GetPointerInnerType
//
//==========================================================================

TType TType::GetPointerInnerType() const
{
	if (Type != TYPE_Pointer)
	{
		FatalError("Not a pointer type");
		return *this;
	}
	TType ret = *this;
	ret.PtrLevel--;
	if (ret.PtrLevel <= 0)
	{
		ret.Type = InnerType;
		ret.InnerType = TYPE_Void;
	}
	return ret;
}

//==========================================================================
//
//	TType::MakeArrayType
//
//==========================================================================

TType TType::MakeArrayType(int elcount, TLocation l) const
{
	if (Type == TYPE_Array || Type == TYPE_DynamicArray)
	{
		ParseError(l, "Can't have multi-dimensional arrays");
	}
	TType array = *this;
	array.ArrayInnerType = Type;
	array.Type = TYPE_Array;
	array.ArrayDim = elcount;
	return array;
}

//==========================================================================
//
//	TType::MakeDynamicArrayType
//
//==========================================================================

TType TType::MakeDynamicArrayType(TLocation l) const
{
	if (Type == TYPE_Array || Type == TYPE_DynamicArray)
	{
		ParseError(l, "Can't have multi-dimensional arrays");
	}
	TType array = *this;
	array.ArrayInnerType = Type;
	array.Type = TYPE_DynamicArray;
	return array;
}

//==========================================================================
//
//	TType::GetArrayInnerType
//
//==========================================================================

TType TType::GetArrayInnerType() const
{
	if (Type != TYPE_Array && Type != TYPE_DynamicArray)
	{
		FatalError("Not an array type");
		return *this;
	}
	TType ret = *this;
	ret.Type = ArrayInnerType;
	ret.ArrayInnerType = TYPE_Void;
	ret.ArrayDim = 0;
	return ret;
}

//==========================================================================
//
//	TType::GetSize
//
//==========================================================================

int TType::GetSize() const
{
	switch (Type)
	{
	case TYPE_Int:			return 4;
	case TYPE_Byte:			return 4;
	case TYPE_Bool:			return 4;
	case TYPE_Float:		return 4;
	case TYPE_Name:			return 4;
	case TYPE_String:		return 4;
	case TYPE_Pointer:		return 4;
	case TYPE_Reference:	return 4;
	case TYPE_Class:		return 4;
	case TYPE_State:		return 4;
	case TYPE_Delegate:		return 8;
	case TYPE_Struct:		return Struct->StackSize * 4;
	case TYPE_Vector:		return 12;
	case TYPE_Array:		return ArrayDim * GetArrayInnerType().GetSize();
	case TYPE_DynamicArray:	return 12;
	}
	return 0;
}

//==========================================================================
//
//	TType::CheckPassable
//
//	Check, if type can be pushed into the stack
//
//==========================================================================

void TType::CheckPassable(TLocation l) const
{
	if (GetSize() != 4 && Type != TYPE_Vector && Type != TYPE_Delegate)
	{
		ParseError(l, "Type %s is not passable", *GetName());
	}
}

//==========================================================================
//
//	TType::CheckMatch
//
//	Check, if types are compatible
//
//	t1 - current type
//	t2 - needed type
//
//==========================================================================

void TType::CheckMatch(TLocation l, const TType& Other) const
{
	CheckPassable(l);
	Other.CheckPassable(l);
	if (Equals(Other))
	{
		return;
	}
	if (Type == TYPE_Vector && Other.Type == TYPE_Vector)
	{
		return;
	}
	if (Type == TYPE_Pointer && Other.Type == TYPE_Pointer)
	{
		TType it1 = GetPointerInnerType();
		TType it2 = Other.GetPointerInnerType();
		if (it1.Equals(it2))
		{
			return;
		}
		if ((it1.Type == TYPE_Void) || (it2.Type == TYPE_Void))
		{
			return;
		}
		if (it1.Type == TYPE_Struct && it2.Type == TYPE_Struct)
		{
			VStruct* s1 = it1.Struct;
			VStruct* s2 = it2.Struct;
			for (VStruct* st1 = s1->ParentStruct; st1; st1 = st1->ParentStruct)
			{
				if (st1 == s2)
				{
					return;
				}
			}
		}
	}
	if (Type == TYPE_Reference && Other.Type == TYPE_Reference)
	{
		VClass* c1 = Class;
		VClass* c2 = Other.Class;
		if (!c1 || !c2)
		{
			//	none reference can be assigned to any reference.
			return;
		}
		if (c1 == c2)
		{
			return;
		}
		for (VClass* pc1 = c1->ParentClass; pc1; pc1 = pc1->ParentClass)
		{
			if (pc1 == c2)
			{
				return;
			}
		}
	}
	if (Type == TYPE_Class && Other.Type == TYPE_Class)
	{
		VClass* c1 = Class;
		VClass* c2 = Other.Class;
		if (!c2)
		{
			//	Can assgn any class type to generic class type.
			return;
		}
		if (c1 == c2)
		{
			return;
		}
		if (c1)
		{
			for (VClass* pc1 = c1->ParentClass; pc1; pc1 = pc1->ParentClass)
			{
				if (pc1 == c2)
				{
					return;
				}
			}
		}
	}
	if (Type == TYPE_Int && Other.Type == TYPE_Byte)
	{
		return;
	}
	if (Type == TYPE_Int && Other.Type == TYPE_Bool)
	{
		return;
	}
	//	Allow assigning none to states, classes and delegates
	if (Type == TYPE_Reference && Class == NULL && (Other.Type == TYPE_Class ||
		Other.Type == TYPE_State || Other.Type == TYPE_Delegate))
	{
		return;
	}
	if (Type == TYPE_Delegate && Other.Type == TYPE_Delegate)
	{
		VMethod& F1 = *Function;
		VMethod& F2 = *Other.Function;
		if (F1.Flags & FUNC_Static || F2.Flags & FUNC_Static)
		{
			ParseError(l, "Can't assign a static function to delegate");
		}
		if (!F1.ReturnType.Equals(F2.ReturnType))
		{
			ParseError(l, "Delegate has different return type");
		}
		else if (F1.NumParams != F2.NumParams)
		{
			ParseError(l, "Delegate has different number of arguments");
		}
		else for (int i = 0; i < F1.NumParams; i++)
			if (!F1.ParamTypes[i].Equals(F2.ParamTypes[i]))
			{
				ParseError(l, "Delegate argument %d differs", i + 1);
			}
		return;
	}
	ParseError(l, "Type mistmatch, types %s and %s are not compatible %d %d",
		*GetName(), *Other.GetName(), Type, Other.Type);
}

//==========================================================================
//
//	TType::GetName
//
//==========================================================================

VStr TType::GetName() const
{
	switch (Type)
	{
	case TYPE_Int:
		return "int";
	case TYPE_Byte:
		return "byte";
	case TYPE_Bool:
		return "bool";
	case TYPE_Float:
		return "float";
	case TYPE_Name:
		return "name";
	case TYPE_String:
		return "string";
	case TYPE_Pointer:
	{
		VStr Ret = GetPointerInnerType().GetName();
		for (int i = 0; i < PtrLevel; i++)
		{
			Ret += "*";
		}
		return Ret;
	}
	case TYPE_Reference:
		return Class ? *Class->Name : "none";
	case TYPE_Class:
	{
		VStr Ret("class");
		if (Class)
		{
			Ret += "<";
			Ret += *Class->Name;
			Ret += ">";
		}
		return Ret;
	}
	case TYPE_State:
		return "state";
	case TYPE_Struct:
		return *Struct->Name;
	case TYPE_Vector:
		return "vector";
	case TYPE_Array:
		return GetArrayInnerType().GetName() + "[]";
	case TYPE_DynamicArray:
		return VStr("array<") + GetArrayInnerType().GetName() + ">";
	default:
		return "unknown";
	}
}

//END

//BEGIN Import / export helper classes

//==========================================================================
//
//	VProgsImport::VProgsImport
//
//==========================================================================

VProgsImport::VProgsImport(VMemberBase* InObj, vint32 InOuterIndex)
: Type(InObj->MemberType)
, Name(InObj->Name)
, OuterIndex(InOuterIndex)
, Obj(InObj)
{
}

//==========================================================================
//
//	VProgsExport::VProgsExport
//
//==========================================================================

VProgsExport::VProgsExport(VMemberBase* InObj)
: Type(InObj->MemberType)
, Name(InObj->Name)
, Obj(InObj)
{
}

//END
