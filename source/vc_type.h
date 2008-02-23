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

//==========================================================================
//
//	EType
//
//==========================================================================

enum EType
{
	TYPE_Void,
	TYPE_Int,
	TYPE_Byte,
	TYPE_Bool,
	TYPE_Float,
	TYPE_Name,
	TYPE_String,
	TYPE_Pointer,
	TYPE_Reference,
	TYPE_Class,
	TYPE_State,
	TYPE_Delegate,
	TYPE_Struct,
	TYPE_Vector,
	TYPE_Array,
	TYPE_DynamicArray,
	TYPE_Unknown,

	NUM_BASIC_TYPES
};

//==========================================================================
//
//	VFieldType
//
//==========================================================================

class VFieldType
{
public:
	vuint8		Type;
	vuint8		InnerType;		//	For pointers
	vuint8		ArrayInnerType;	//	For arrays
	vuint8		PtrLevel;
	vint32		ArrayDim;
	union
	{
		vuint32		BitMask;
		VClass*		Class;			//  Class of the reference
		VStruct*	Struct;			//  Struct data.
		VMethod*	Function;		//  Function of the delegate type.
	};

	VFieldType();
	VFieldType(EType Atype);
	explicit VFieldType(VClass* InClass);
	explicit VFieldType(VStruct* InStruct);

	friend VStream& operator<<(VStream&, VFieldType&);

	bool Equals(const VFieldType&) const;
	VFieldType MakePointerType() const;
	VFieldType GetPointerInnerType() const;
	VFieldType MakeArrayType(int, TLocation) const;
	VFieldType MakeDynamicArrayType(TLocation) const;
	VFieldType GetArrayInnerType() const;
	int GetStackSize() const;
	int GetSize() const;
	int GetAlignment() const;
	void CheckPassable(TLocation) const;
	void CheckMatch(TLocation, const VFieldType&) const;
	VStr GetName() const;
};

//==========================================================================
//
//	VObjectDelegate
//
//==========================================================================

struct VObjectDelegate
{
	VObject*		Obj;
	VMethod*		Func;
};

//==========================================================================
//
//	VScriptArray
//
//==========================================================================

class VScriptArray
{
public:
	int Num() const
	{
		return ArrNum;
	}
	vuint8* Ptr()
	{
		return ArrData;
	}
	void Clear(VFieldType& Type);
	void Resize(int NewSize, VFieldType& Type);
	void SetNum(int NewNum, VFieldType& Type);
	void Insert(int Index, int Count, VFieldType& Type);
	void Remove(int Index, int Count, VFieldType& Type);

private:
	int ArrNum;
	int ArrSize;
	vuint8* ArrData;
};

//==========================================================================
//
//	FReplacedString
//
//==========================================================================

struct FReplacedString
{
	int			Index;
	bool		Replaced;
	VStr		Old;
	VStr		New;
};
