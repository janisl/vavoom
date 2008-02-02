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

#include "gamedefs.h"
#include "progdefs.h"
#include "network.h"

//==========================================================================
//
//	operator VStream << FType
//
//==========================================================================

VStream& operator<<(VStream& Strm, VField::FType& T)
{
	guard(operator VStream << FType);
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
	unguard;
}

//==========================================================================
//
//	VField::FType::GetSize
//
//==========================================================================

int VField::FType::GetSize() const
{
	guard(VField::FType::GetSize);
	switch (Type)
	{
	case TYPE_Int:			return sizeof(vint32);
	case TYPE_Byte:			return sizeof(vuint8);
	case TYPE_Bool:			return sizeof(vuint32);
	case TYPE_Float:		return sizeof(float);
	case TYPE_Name:			return sizeof(VName);
	case TYPE_String:		return sizeof(VStr);
	case TYPE_Pointer:		return sizeof(void*);
	case TYPE_Reference:	return sizeof(VObject*);
	case TYPE_Class:		return sizeof(VClass*);
	case TYPE_State:		return sizeof(VState*);
	case TYPE_Delegate:		return sizeof(VObjectDelegate);
	case TYPE_Struct:		return (Struct->Size + 3) & ~3;
	case TYPE_Vector:		return sizeof(TVec);
	case TYPE_Array:		return ArrayDim * GetArrayInnerType().GetSize();
	case TYPE_DynamicArray:	return sizeof(VScriptArray);
	}
	return 0;
	unguard;
}

//==========================================================================
 //
//	VField::FType::GetAlignment
//
//==========================================================================

int VField::FType::GetAlignment() const
{
	guard(VField::FType::GetAlignment);
	switch (Type)
	{
	case TYPE_Int:			return sizeof(vint32);
	case TYPE_Byte:			return sizeof(vuint8);
	case TYPE_Bool:			return sizeof(vuint32);
	case TYPE_Float:		return sizeof(float);
	case TYPE_Name:			return sizeof(VName);
	case TYPE_String:		return sizeof(char*);
	case TYPE_Pointer:		return sizeof(void*);
	case TYPE_Reference:	return sizeof(VObject*);
	case TYPE_Class:		return sizeof(VClass*);
	case TYPE_State:		return sizeof(VState*);
	case TYPE_Delegate:		return sizeof(VObject*);
	case TYPE_Struct:		return Struct->Alignment;
	case TYPE_Vector:		return sizeof(float);
	case TYPE_Array:		return GetArrayInnerType().GetAlignment();
	case TYPE_DynamicArray:	return sizeof(void*);
	}
	return 0;
	unguard;
}

//==========================================================================
//
//	VField::FType::GetArrayInnerType
//
//==========================================================================

VField::FType VField::FType::GetArrayInnerType() const
{
	guard(VField::FType::GetArrayInnerType);
	if (Type != TYPE_Array && Type != TYPE_DynamicArray)
	{
		Sys_Error("Not an array type");
		return *this;
	}
	VField::FType ret = *this;
	ret.Type = ArrayInnerType;
	ret.ArrayInnerType = TYPE_Void;
	ret.ArrayDim = 0;
	return ret;
	unguard;
}

//==========================================================================
//
//  PF_Fixme
//
//==========================================================================

void PF_Fixme()
{
	Sys_Error("unimplemented bulitin");
}

//==========================================================================
//
//	operator VStream << mobjinfo_t
//
//==========================================================================

VStream& operator<<(VStream& Strm, mobjinfo_t& MI)
{
	return Strm << STRM_INDEX(MI.doomednum)
		<< STRM_INDEX(MI.GameFilter)
		<< MI.class_id;
}

//==========================================================================
//
//	VScriptArray::Clear
//
//==========================================================================

void VScriptArray::Clear(VField::FType& Type)
{
	guard(VScriptArray::Clear);
	if (ArrData)
	{
		int InnerSize = Type.GetSize();
		for (int i = 0; i < ArrSize; i++)
		{
			VField::DestructField(ArrData + i * InnerSize, Type);
		}
		delete[] ArrData;
	}
	ArrData = NULL;
	ArrNum = 0;
	ArrSize = 0;
	unguard;
}

//==========================================================================
//
//	VScriptArray::Resize
//
//==========================================================================

void VScriptArray::Resize(int NewSize, VField::FType& Type)
{
	guard(VScriptArray::Resize);
	check(NewSize >= 0);

	if (NewSize <= 0)
	{
		Clear(Type);
		return;
	}

	if (NewSize == ArrSize)
	{
		return;
	}
	vuint8* OldData = ArrData;
	vint32 OldSize = ArrSize;
	ArrSize = NewSize;
	if (ArrNum > NewSize)
	{
		ArrNum = NewSize;
	}

	int InnerSize = Type.GetSize();
	ArrData = new vuint8[ArrSize * InnerSize];
	memset(ArrData, 0, ArrSize * InnerSize);
	for (int i = 0; i < ArrNum; i++)
	{
		VField::CopyFieldValue(OldData + i * InnerSize,
			ArrData + i * InnerSize, Type);
	}

	if (OldData)
	{
		for (int i = 0; i < OldSize; i++)
		{
			VField::DestructField(OldData + i * InnerSize, Type);
		}
		delete[] OldData;
	}
	unguard;
}

//==========================================================================
//
//	VScriptArray::SetNum
//
//==========================================================================

void VScriptArray::SetNum(int NewNum, VField::FType& Type)
{
	guard(VScriptArray::SetNum);
	check(NewNum >= 0);
	if (NewNum == 0)
	{
		//	As a special case setting size to 0 should clear the array.
		Clear(Type);
	}
	else if (NewNum > ArrSize)
	{
		Resize(NewNum + NewNum * 3 / 8 + 32, Type);
	}
	ArrNum = NewNum;
	unguard;
}

//==========================================================================
//
//	VScriptArray
//
//==========================================================================

void VScriptArray::Insert(int Index, int Count, VField::FType& Type)
{
	guard(VScriptArray::Insert);
	check(ArrData != NULL);
	check(Index >= 0);
	check(Index <= ArrNum);

	SetNum(ArrNum + Count, Type);
	int InnerSize = Type.GetSize();
	//	Move value to new location.
	for (int i = ArrNum - 1; i >= Index + Count; i--)
	{
		VField::CopyFieldValue(ArrData + (i - Count) * InnerSize,
			ArrData + i * InnerSize, Type);
	}
	//	Clean inserted elements
	for (int i = Index; i < Index + Count; i++)
	{
		VField::DestructField(ArrData + i * InnerSize, Type);
	}
	memset(ArrData + Index * InnerSize, 0, Count * InnerSize);
	unguard;
}

//==========================================================================
//
//	VScriptArray::Remove
//
//==========================================================================

void VScriptArray::Remove(int Index, int Count, VField::FType& Type)
{
	guard(VScriptArray::Remove);
	check(ArrData != NULL);
	check(Index >= 0);
	check(Index + Count <= ArrNum);

	ArrNum -= Count;
	if (ArrNum == 0)
	{
		//	Array is empty, so just clear it.
		Clear(Type);
	}
	else
	{
		//	Move elements that are after removed ones.
		int InnerSize = Type.GetSize();
		for (int i = Index; i < ArrNum; i++)
		{
			VField::CopyFieldValue(ArrData + (i + Count) * InnerSize,
				ArrData + i * InnerSize, Type);
		}
	}
	unguard;
}
