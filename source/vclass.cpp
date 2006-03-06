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
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
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

bool			VClass::GObjInitialized;
VClass*			VClass::GClasses;

//==========================================================================
//
//	VField::SerialiseFieldValue
//
//==========================================================================

void VField::SerialiseFieldValue(VStream& Strm, byte* Data, const VField::FType& Type)
{
	guard(SerialiseFieldValue);
	VField::FType IntType;
	int InnerSize;
	switch (Type.Type)
	{
	case ev_int:
		Strm << *(int*)Data;
		break;

	case ev_float:
		Strm << *(float*)Data;
		break;

	case ev_bool:
		if (Type.BitMask == 1)
			Strm << *(int*)Data;
		break;

	case ev_vector:
		Strm << *(TVec*)Data;
		break;

	case ev_name:
		Strm << *(VName*)Data;
		break;

	case ev_string:
		if (Strm.IsLoading())
		{
			int TmpIdx;
			Strm << TmpIdx;
			if (TmpIdx)
			{
				*(int*)Data = (int)svpr.StrAtOffs(TmpIdx);
			}
			else
			{
				*(int*)Data = 0;
			}
		}
		else
		{
			int TmpIdx = 0;
			if (*(int*)Data)
			{
				TmpIdx = svpr.GetStringOffs(*(char**)Data);
			}
			Strm << TmpIdx;
		}
		break;

	case ev_pointer:
		if (Type.InnerType == ev_struct)
			Strm.SerialiseStructPointer(*(void**)Data, Type.Struct);
		else
		{
			dprintf("Don't know how to serialise pointer type %d\n", Type.InnerType);
			Strm << *(int*)Data;
		}
		break;

	case ev_reference:
		Strm.SerialiseReference(*(VObject**)Data, Type.Class);
		break;

	case ev_classid:
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

	case ev_delegate:
		Strm.SerialiseReference(*(VObject**)Data, Type.Class);
		if (Strm.IsLoading())
		{
			VName FuncName;
			Strm << FuncName;
			if (*(VObject**)Data)
				((FFunction**)Data)[1] = (*(VObject**)Data)->GetVFunction(FuncName);
		}
		else
		{
			VName FuncName = NAME_None;
			if (*(VObject**)Data)
				FuncName = ((FFunction**)Data)[1]->Name;
			Strm << FuncName;
		}
		break;

	case ev_struct:
		Type.Struct->SerialiseObject(Strm, Data);
		break;

	case ev_array:
		IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		InnerSize = IntType.Type == ev_vector ? 12 : IntType.Type == ev_struct ? IntType.Struct->Size : 4;
		for (int i = 0; i < Type.ArrayDim; i++)
		{
			SerialiseFieldValue(Strm, Data + i * InnerSize, IntType);
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

void VField::CleanField(byte* Data, const VField::FType& Type)
{
	guard(CleanField);
	VField::FType IntType;
	int InnerSize;
	switch (Type.Type)
	{
	case ev_reference:
		if (*(VObject**)Data && (*(VObject**)Data)->GetFlags() & _OF_CleanupRef)
		{
			*(VObject**)Data = NULL;
		}
		break;

	case ev_delegate:
		if (*(VObject**)Data && (*(VObject**)Data)->GetFlags() & _OF_CleanupRef)
		{
			*(VObject**)Data = NULL;
			((FFunction**)Data)[1] = NULL;
		}
		break;

	case ev_struct:
		Type.Struct->CleanObject(Data);
		break;

	case ev_array:
		IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		InnerSize = IntType.Type == ev_struct ? IntType.Struct->Size : 4;
		for (int i = 0; i < Type.ArrayDim; i++)
		{
			CleanField(Data + i * InnerSize, IntType);
		}
		break;
	}
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
	if (ObjectFlags & CLASSOF_RefsInitialised)
	{
		return;
	}

	ReferenceFields = NULL;
	if (ParentStruct)
	{
		ParentStruct->InitReferences();
		ReferenceFields = ParentStruct->ReferenceFields;
	}

	for (VField* F = Fields; F; F = F->Next)
	{
		switch (F->Type.Type)
		{
		case ev_reference:
		case ev_delegate:
			F->NextReference = ReferenceFields;
			ReferenceFields = F;
			break;
		
		case ev_struct:
			F->Type.Struct->InitReferences();
			if (F->Type.Struct->ReferenceFields)
			{
				F->NextReference = ReferenceFields;
				ReferenceFields = F;
			}
			break;
		
		case ev_array:
			if (F->Type.ArrayInnerType == ev_reference)
			{
				F->NextReference = ReferenceFields;
				ReferenceFields = F;
			}
			else if (F->Type.ArrayInnerType == ev_struct)
			{
				F->Type.Struct->InitReferences();
				if (F->Type.Struct->ReferenceFields)
				{
					F->NextReference = ReferenceFields;
					ReferenceFields = F;
				}
			}
			break;
		}
	}

	ObjectFlags |= CLASSOF_RefsInitialised;
	unguard;
}

//==========================================================================
//
//	VStruct::SerialiseObject
//
//==========================================================================

void VStruct::SerialiseObject(VStream& Strm, byte* Data)
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
		//	Skip fields with native serialisation.
		if (F->Flags & FIELD_Native)
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

void VStruct::CleanObject(byte* Data)
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
//	VClass::VClass
//
//==========================================================================

VClass::VClass(VName AName, int ASize)
: Name(AName)
, ClassSize(ASize)
{
	guard(VClass::VClass);
	LinkNext = GClasses;
	GClasses = this;
	unguard;
}

//==========================================================================
//
//	VClass::VClass
//
//==========================================================================

VClass::VClass(ENativeConstructor, size_t ASize, dword AClassFlags, 
	VClass *AParent, EName AName, void(*ACtor)(void*))
: ObjectFlags(CLASSOF_Native)
, Name(AName)
, ParentClass(AParent)
, ClassSize(ASize)
, ClassFlags(AClassFlags)
, ClassConstructor(ACtor)
{
	guard(native VClass::VClass);
	LinkNext = GClasses;
	GClasses = this;
	unguard;
}

//==========================================================================
//
//	VClass::~VClass
//
//==========================================================================

VClass::~VClass()
{
	guard(VClass::~VClass);
	if (!GObjInitialized)
	{
		return;
	}
	if (GClasses == this)
	{
		GClasses = LinkNext;
	}
	else
	{
		VClass* Prev = GClasses;
		while (Prev && Prev->LinkNext != this)
		{
			Prev = Prev->LinkNext;
		}
		if (Prev)
		{
			Prev->LinkNext = LinkNext;
		}
		else
		{
			GCon->Log(NAME_Dev, "VClass Unlink: Class not in list");
		}
	}
	unguard;
}

//==========================================================================
//
//	VClass::StaticInit
//
//==========================================================================

void VClass::StaticInit()
{
	GObjInitialized = true;
}

//==========================================================================
//
//	VClass::StaticExit
//
//==========================================================================

void VClass::StaticExit()
{
	GObjInitialized = false;
}

//==========================================================================
//
//	VClass::FindClass
//
//==========================================================================

VClass *VClass::FindClass(const char *AName)
{
	VName TempName(AName, VName::Find);
	if (TempName == NAME_None)
	{
		// No such name, no chance to find a class
		return NULL;
	}
	for (VClass* Cls = GClasses; Cls; Cls = Cls->LinkNext)
	{
		if (Cls->GetVName() == TempName)
		{
			return Cls;
		}
	}
	return NULL;
}

//==========================================================================
//
//	VClass::FindFunction
//
//==========================================================================

FFunction *VClass::FindFunction(VName InName)
{
	guard(VClass::FindFunction);
	for (int i = 0; i < ClassNumMethods; i++)
	{
		if (ClassVTable[i]->Name == InName)
		{
			return ClassVTable[i];
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VClass::FindFunctionChecked
//
//==========================================================================

FFunction *VClass::FindFunctionChecked(VName InName)
{
	guard(VClass::FindFunctionChecked);
	FFunction *func = FindFunction(InName);
	if (!func)
	{
		Sys_Error("Function %s not found", *InName);
	}
	return func;
	unguard;
}

//==========================================================================
//
//	VClass::GetFunctionIndex
//
//==========================================================================

int VClass::GetFunctionIndex(VName InName)
{
	guard(VClass::GetFunctionIndex);
	for (int i = 0; i < ClassNumMethods; i++)
	{
		if (ClassVTable[i]->Name == InName)
		{
			return i;
		}
	}
	return -1;
	unguard;
}

//==========================================================================
//
//	VClass::InitReferences
//
//==========================================================================

void VClass::InitReferences()
{
	guard(VClass::InitReferences);
	if (ObjectFlags & CLASSOF_RefsInitialised)
	{
		return;
	}

	ReferenceFields = NULL;
	if (GetSuperClass())
	{
		GetSuperClass()->InitReferences();
		ReferenceFields = GetSuperClass()->ReferenceFields;
	}

	for (VField* F = Fields; F; F = F->Next)
	{
		switch (F->Type.Type)
		{
		case ev_reference:
		case ev_delegate:
			F->NextReference = ReferenceFields;
			ReferenceFields = F;
			break;
		
		case ev_struct:
			F->Type.Struct->InitReferences();
			if (F->Type.Struct->ReferenceFields)
			{
				F->NextReference = ReferenceFields;
				ReferenceFields = F;
			}
			break;
		
		case ev_array:
			if (F->Type.ArrayInnerType == ev_reference)
			{
				F->NextReference = ReferenceFields;
				ReferenceFields = F;
			}
			else if (F->Type.ArrayInnerType == ev_struct)
			{
				F->Type.Struct->InitReferences();
				if (F->Type.Struct->ReferenceFields)
				{
					F->NextReference = ReferenceFields;
					ReferenceFields = F;
				}
			}
			break;
		}
	}

	ObjectFlags |= CLASSOF_RefsInitialised;
	unguard;
}

//==========================================================================
//
//	VClass::SerialiseObject
//
//==========================================================================

void VClass::SerialiseObject(VStream& Strm, VObject* Obj)
{
	guard(SerialiseObject);
	//	Serialise parent class fields.
	if (GetSuperClass())
	{
		GetSuperClass()->SerialiseObject(Strm, Obj);
	}
	//	Serialise fields.
	for (VField* F = Fields; F; F = F->Next)
	{
		//	Skip fields with native serialisation.
		if (F->Flags & FIELD_Native)
		{
			continue;
		}
		guard(Field);
		VField::SerialiseFieldValue(Strm, (byte*)Obj + F->Ofs, F->Type);
		unguardf(("(%s)", *F->Name));
	}
	unguardf(("(%s)", GetName()));
}

//==========================================================================
//
//	VClass::CleanObject
//
//==========================================================================

void VClass::CleanObject(VObject* Obj)
{
	guard(CleanObject);
	for (VField* F = ReferenceFields; F; F = F->NextReference)
	{
		VField::CleanField((byte*)Obj + F->Ofs, F->Type);
	}
	unguardf(("(%s)", GetName()));
}

//**************************************************************************
//
//	$Log$
//	Revision 1.8  2006/03/06 13:02:32  dj_jl
//	Cleaning up references to destroyed objects.
//
//	Revision 1.7  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//	
//	Revision 1.6  2006/02/26 20:52:48  dj_jl
//	Proper serialisation of level and players.
//	
//	Revision 1.5  2005/11/24 20:09:23  dj_jl
//	Removed unused fields from Object class.
//	
//	Revision 1.4  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//	
//	Revision 1.3  2002/03/09 18:05:34  dj_jl
//	Added support for defining native functions outside pr_cmds
//	
//	Revision 1.2  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2001/12/27 17:35:41  dj_jl
//	Split VClass in seperate module
//	
//	Revision 1.1  2001/12/18 19:03:17  dj_jl
//	A lots of work on VObject
//	
//**************************************************************************
