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
#include "gamedefs.h"
#include "progdefs.h"
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
//	VStruct::VStruct
//
//==========================================================================

#ifndef IN_VCC
VStruct::VStruct(VName AName)
: VMemberBase(MEMBER_Struct, AName)
, ObjectFlags(0)
#else
VStruct::VStruct(VName InName, VMemberBase* InOuter, TLocation InLoc)
: VMemberBase(MEMBER_Struct, InName, InOuter, InLoc)
#endif
, ParentStruct(0)
#ifndef IN_VCC
, Size(0)
, Alignment(0)
#endif
, IsVector(false)
, Fields(0)
#ifndef IN_VCC
, ReferenceFields(0)
, DestructorFields(0)
#else
, StackSize(0)
, ParentStructName(NAME_None)
, Defined(true)
#endif
{
}

//==========================================================================
//
//	VStruct::Serialise
//
//==========================================================================

void VStruct::Serialise(VStream& Strm)
{
	guard(VStruct::Serialise);
#ifndef IN_VCC
	vint32 StackSize;
#endif
	VMemberBase::Serialise(Strm);
	Strm << ParentStruct
		<< IsVector
		<< STRM_INDEX(StackSize)
		<< Fields;
	unguard;
}

#ifdef IN_VCC

//==========================================================================
//
//	VStruct::AddField
//
//==========================================================================

void VStruct::AddField(VField* f)
{
	for (VField* Check = Fields; Check; Check = Check->Next)
	{
		if (f->Name == Check->Name)
		{
			ParseError(f->Loc, "Redeclared field");
			ParseError(Check->Loc, "Previous declaration here");
		}
	}

	if (!Fields)
		Fields = f;
	else
	{
		VField* Prev = Fields;
		while (Prev->Next)
			Prev = Prev->Next;
		Prev->Next = f;
	}
	f->Next = NULL;
}

//==========================================================================
//
//	VStruct::CheckForField
//
//==========================================================================

VField* VStruct::CheckForField(VName FieldName)
{
	for (VField* fi = Fields; fi; fi = fi->Next)
	{
		if (fi->Name == FieldName)
		{
			return fi;
		}
	}
	if (ParentStruct)
	{
		return ParentStruct->CheckForField(FieldName);
	}
	return NULL;
}

//==========================================================================
//
//	VStruct::NeedsDestructor
//
//==========================================================================

bool VStruct::NeedsDestructor() const
{
	for (VField* F = Fields; F; F = F->Next)
		if (F->NeedsDestructor())
			return true;
	if (ParentStruct)
		return ParentStruct->NeedsDestructor();
	return false;
}

//==========================================================================
//
//	VStruct::Define
//
//==========================================================================

bool VStruct::Define()
{
	if (ParentStructName != NAME_None)
	{
		VFieldType type = CheckForType(Outer->MemberType == MEMBER_Class ?
			(VClass*)Outer : NULL, ParentStructName);
		if (type.Type != TYPE_Struct)
		{
			ParseError(ParentStructLoc, "%s is not a struct type",
				*ParentStructName);
		}
		else
		{
			ParentStruct = type.Struct;
		}
	}

	if (ParentStruct && !ParentStruct->Defined)
	{
		ParseError(ParentStructLoc, "Parent struct must be declared before");
		return false;
	}

	Defined = true;
	return true;
}

//==========================================================================
//
//	VStruct::DefineMembers
//
//==========================================================================

bool VStruct::DefineMembers()
{
	bool Ret = true;

	//	Define fields.
	vint32 size = 0;
	if (ParentStruct)
	{
		size = ParentStruct->StackSize * 4;
	}
	VField* PrevBool = NULL;
	for (VField* fi = Fields; fi; fi = fi->Next)
	{
		if (!fi->Define())
		{
			Ret = false;
		}
		if (fi->Type.Type == TYPE_Bool && PrevBool && PrevBool->Type.BitMask != 0x80000000)
		{
			fi->Type.BitMask = PrevBool->Type.BitMask << 1;
		}
		else
		{
			size += fi->Type.GetSize();
		}
		PrevBool = fi->Type.Type == TYPE_Bool ? fi : NULL;
	}

	//	Validate vector type.
	if (IsVector)
	{
		int fc = 0;
		for (VField* f = Fields; f; f = f->Next)
		{
			if (f->Type.Type != TYPE_Float)
			{
				ParseError(f->Loc, "Vector can have only float fields");
				Ret = false;
			}
			fc++;
		}
		if (fc != 3)
		{
			ParseError(Loc, "Vector must have exactly 3 float fields");
			Ret = false;
		}
	}

	StackSize = (size + 3) / 4;
	return Ret;
}

#else

//==========================================================================
//
//	VStruct::PostLoad
//
//==========================================================================

void VStruct::PostLoad()
{
	if (ObjectFlags & CLASSOF_PostLoaded)
	{
		//	Already done.
		return;
	}

	//	Make sure parent struct has been set up.
	if (ParentStruct)
	{
		ParentStruct->PostLoad();
	}

	//	Calculate field offsets and class size.
	CalcFieldOffsets();

	//	Set up list of reference fields.
	InitReferences();

	//	Set up list of destructor fields.
	InitDestructorFields();

	ObjectFlags |= CLASSOF_PostLoaded;
}

//==========================================================================
//
//	VStruct::CalcFieldOffsets
//
//==========================================================================

void VStruct::CalcFieldOffsets()
{
	guard(VStruct::CalcFieldOffsets);
	int size = ParentStruct ? ParentStruct->Size : 0;
	Alignment = ParentStruct ? ParentStruct->Alignment : 0;
	VField* PrevField = NULL;
	for (VField* fi = Fields; fi; fi = fi->Next)
	{
		if (fi->Type.Type == TYPE_Bool && PrevField &&
			PrevField->Type.Type == TYPE_Bool &&
			PrevField->Type.BitMask != 0x80000000)
		{
			vuint32 bit_mask = PrevField->Type.BitMask << 1;
			if (fi->Type.BitMask != bit_mask)
				Sys_Error("Wrong bit mask");
			fi->Type.BitMask = bit_mask;
			fi->Ofs = PrevField->Ofs;
		}
		else
		{
			if (fi->Type.Type == TYPE_Struct ||
				(fi->Type.Type == TYPE_Array && fi->Type.ArrayInnerType == TYPE_Struct))
			{
				//	Make sure struct size has been calculated.
				fi->Type.Struct->PostLoad();
			}
			//	Align field offset.
			int FldAlign = fi->Type.GetAlignment();
			size = (size + FldAlign - 1) & ~(FldAlign - 1);
			//	Structure itself has the bigest alignment.
			if (Alignment < FldAlign)
			{
				Alignment = FldAlign;
			}
			fi->Ofs = size;
			size += fi->Type.GetSize();
		}
		PrevField = fi;
	}
	Size = (size + Alignment - 1) & ~(Alignment - 1);
	unguard;
}

//==========================================================================
//
//	VStruct::InitReferences
//
//==========================================================================

void VStruct::InitReferences()
{
	guard(VStruct::InitReferences);
	ReferenceFields = NULL;
	if (ParentStruct)
	{
		ReferenceFields = ParentStruct->ReferenceFields;
	}

	for (VField* F = Fields; F; F = F->Next)
	{
		switch (F->Type.Type)
		{
		case TYPE_Reference:
		case TYPE_Delegate:
			F->NextReference = ReferenceFields;
			ReferenceFields = F;
			break;
		
		case TYPE_Struct:
			F->Type.Struct->PostLoad();
			if (F->Type.Struct->ReferenceFields)
			{
				F->NextReference = ReferenceFields;
				ReferenceFields = F;
			}
			break;
		
		case TYPE_Array:
		case TYPE_DynamicArray:
			if (F->Type.ArrayInnerType == TYPE_Reference)
			{
				F->NextReference = ReferenceFields;
				ReferenceFields = F;
			}
			else if (F->Type.ArrayInnerType == TYPE_Struct)
			{
				F->Type.Struct->PostLoad();
				if (F->Type.Struct->ReferenceFields)
				{
					F->NextReference = ReferenceFields;
					ReferenceFields = F;
				}
			}
			break;
		}
	}
	unguard;
}

//==========================================================================
//
//	VStruct::InitDestructorFields
//
//==========================================================================

void VStruct::InitDestructorFields()
{
	guard(VStruct::InitDestructorFields);
	DestructorFields = NULL;
	if (ParentStruct)
	{
		DestructorFields = ParentStruct->DestructorFields;
	}

	for (VField* F = Fields; F; F = F->Next)
	{
		switch (F->Type.Type)
		{
		case TYPE_String:
			F->DestructorLink = DestructorFields;
			DestructorFields = F;
			break;
		
		case TYPE_Struct:
			F->Type.Struct->PostLoad();
			if (F->Type.Struct->DestructorFields)
			{
				F->DestructorLink = DestructorFields;
				DestructorFields = F;
			}
			break;
		
		case TYPE_Array:
			if (F->Type.ArrayInnerType == TYPE_String)
			{
				F->DestructorLink = DestructorFields;
				DestructorFields = F;
			}
			else if (F->Type.ArrayInnerType == TYPE_Struct)
			{
				F->Type.Struct->PostLoad();
				if (F->Type.Struct->DestructorFields)
				{
					F->DestructorLink = DestructorFields;
					DestructorFields = F;
				}
			}
			break;

		case TYPE_DynamicArray:
			F->DestructorLink = DestructorFields;
			DestructorFields = F;
			break;
		}
	}
	unguard;
}

//==========================================================================
//
//	VStruct::CopyObject
//
//==========================================================================

void VStruct::CopyObject(const vuint8* Src, vuint8* Dst)
{
	guard(VStruct::CopyObject);
	//	Copy parent struct's fields.
	if (ParentStruct)
	{
		ParentStruct->CopyObject(Src, Dst);
	}
	//	Copy fields.
	for (VField* F = Fields; F; F = F->Next)
	{
		VField::CopyFieldValue(Src + F->Ofs, Dst + F->Ofs, F->Type);
	}
	unguardf(("(%s)", *Name));
}

//==========================================================================
//
//	VStruct::SerialiseObject
//
//==========================================================================

void VStruct::SerialiseObject(VStream& Strm, vuint8* Data)
{
	guard(VStruct::SerialiseObject);
	//	Serialise parent struct's fields.
	if (ParentStruct)
	{
		ParentStruct->SerialiseObject(Strm, Data);
	}
	//	Serialise fields.
	for (VField* F = Fields; F; F = F->Next)
	{
		//	Skip native and transient fields.
		if (F->Flags & (FIELD_Native | FIELD_Transient))
		{
			continue;
		}
		VField::SerialiseFieldValue(Strm, Data + F->Ofs, F->Type);
	}
	unguardf(("(%s)", *Name));
}

//==========================================================================
//
//	VStruct::CleanObject
//
//==========================================================================

void VStruct::CleanObject(vuint8* Data)
{
	guard(VStruct::CleanObject);
	for (VField* F = ReferenceFields; F; F = F->NextReference)
	{
		VField::CleanField(Data + F->Ofs, F->Type);
	}
	unguardf(("(%s)", *Name));
}

//==========================================================================
//
//	VStruct::DestructObject
//
//==========================================================================

void VStruct::DestructObject(vuint8* Data)
{
	guard(VStruct::DestructObject);
	for (VField* F = DestructorFields; F; F = F->DestructorLink)
	{
		VField::DestructField(Data + F->Ofs, F->Type);
	}
	unguardf(("(%s)", *Name));
}

//==========================================================================
//
//	VStruct::IdenticalObject
//
//==========================================================================

bool VStruct::IdenticalObject(const vuint8* Val1, const vuint8* Val2)
{
	guard(VStruct::IdenticalObject);
	//	Compare parent struct's fields.
	if (ParentStruct)
	{
		if (!ParentStruct->IdenticalObject(Val1, Val2))
		{
			return false;
		}
	}
	//	Compare fields.
	for (VField* F = Fields; F; F = F->Next)
	{
		if (!VField::IdenticalValue(Val1 + F->Ofs, Val2 + F->Ofs, F->Type))
		{
			return false;
		}
	}
	return true;
	unguardf(("(%s)", *Name));
}

//==========================================================================
//
//	VStruct::NetSerialiseObject
//
//==========================================================================

bool VStruct::NetSerialiseObject(VStream& Strm, VNetObjectsMap* Map,
	vuint8* Data)
{
	guard(VStruct::NetSerialiseObject);
	bool Ret = true;
	//	Serialise parent struct's fields.
	if (ParentStruct)
	{
		Ret = ParentStruct->NetSerialiseObject(Strm, Map, Data);
	}
	//	Serialise fields.
	for (VField* F = Fields; F; F = F->Next)
	{
		if (!VField::NetSerialiseValue(Strm, Map, Data + F->Ofs, F->Type))
		{
			Ret = false;
		}
	}
	return Ret;
	unguardf(("(%s)", *Name));
}

#endif
