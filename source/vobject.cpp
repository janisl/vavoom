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

// Register a class at startup time.
VClass VObject::PrivateStaticClass
(
	EC_NativeConstructor,
	sizeof(VObject),
	VObject::StaticClassFlags,
	NULL,
	NAME_Object,
	(void(*)(void*))VObject::InternalConstructor
);
VClass* autoclassVObject = VObject::StaticClass();

bool				VObject::GObjInitialized;
TArray<VObject*>	VObject::GObjObjects;
TArray<int>			VObject::GObjAvailable;
VObject*			VObject::GObjHash[4096];

//==========================================================================
//
//	VObject::VObject
//
//==========================================================================

VObject::VObject(void)
{
}

//==========================================================================
//
//	VObject::~VObject
//
//==========================================================================

VObject::~VObject(void)
{
	guard(VObject::~VObject);
	ConditionalDestroy();
	if (!GObjInitialized)
	{
		return;
	}
	if (Index == GObjObjects.Num() - 1)
	{
		GObjObjects.Pop();
	}
	else
	{
		GObjObjects[Index] = NULL;
		GObjAvailable.AddItem(Index);
	}
	unguard;
}

//==========================================================================
//
//	VObject::StaticInit
//
//==========================================================================

void VObject::StaticInit(void)
{
	VClass::StaticInit();
	GObjInitialized = true;
}

//==========================================================================
//
//	VObject::StaticExit
//
//==========================================================================

void VObject::StaticExit(void)
{
	GObjInitialized = false;
	VClass::StaticExit();
}

//==========================================================================
//
//	VObject::StaticSpawnObject
//
//==========================================================================

VObject *VObject::StaticSpawnObject(VClass *AClass, int tag)
{
	guard(VObject::StaticSpawnObject);
	VObject *Obj = (VObject *)Z_Calloc(AClass->ClassSize, tag, 0);
	VClass *NativeClass = AClass;
	while (NativeClass && !(NativeClass->GetFlags() & CLASSOF_Native))
	{
		NativeClass = NativeClass->GetSuperClass();
	}
	if (!NativeClass)
	{
		Sys_Error("No native base class");
	}
	NativeClass->ClassConstructor(Obj);
	Obj->Class = AClass;
	Obj->vtable = AClass->ClassVTable;
	Obj->Register();
	if (Obj->vtable)
	{
		TProgs::Exec(Obj->vtable[0], (int)Obj);
	}
	return Obj;
	unguardf(("%s", AClass->GetName()));
}

//==========================================================================
//
//	VObject::Register
//
//==========================================================================

void VObject::Register()
{
	guard(VObject::Register);
	if (GObjAvailable.Num())
	{
		Index = GObjAvailable.Pop();
		GObjObjects[Index] = this;
	}
	else
	{
		Index = GObjObjects.AddItem(this);
	}
	unguard;
}

//==========================================================================
//
//	VObject::ConditionalDestroy
//
//==========================================================================

bool VObject::ConditionalDestroy(void)
{
	if (!(ObjectFlags & _OF_Destroyed))
	{
		SetFlags(_OF_Destroyed);
		Destroy();
	}
	return true;
}

//==========================================================================
//
//	VObject::Destroy
//
//==========================================================================

void VObject::Destroy()
{
}

//==========================================================================
//
//	VObject::IsA
//
//==========================================================================

bool VObject::IsA(VClass *SomeBaseClass) const
{
	for (const VClass *c = Class; c; c = c->GetSuperClass())
	{
		if (SomeBaseClass == c)
		{
			return true;
		}
	}
	return false;
}

//==========================================================================
//
//	VObject::GetVFunction
//
//==========================================================================

FFunction *VObject::GetVFunction(FName FuncName) const
{
	guardSlow(VObject::GetVFunction);
	return vtable[Class->GetFunctionIndex(FuncName)];
	unguardSlow;
}

//==========================================================================
//
//	VObject::CollectGarbage
//
//==========================================================================

void VObject::CollectGarbage(void)
{
	guard(VObject::CollectGarbage);
	for (int i = 0; i < GObjObjects.Num(); i++)
	{
		if (!GObjObjects[i])
		{
			continue;
		}
		VObject *Obj = GObjObjects[i];
		if (Obj->GetFlags() & _OF_Destroyed)
		{
			delete Obj;
		}
	}
	unguard;
}

//==========================================================================
//
//	VObject::GetIndexObject
//
//==========================================================================

VObject *VObject::GetIndexObject(int Index)
{
	return GObjObjects[Index];
}

//==========================================================================
//
//	VObject::GetObjectsCount
//
//==========================================================================

int VObject::GetObjectsCount(void)
{
	return GObjObjects.Num();
}

//==========================================================================
//
//	SerialiseFieldValue
//
//==========================================================================

static void SerialiseFieldValue(VStream& Strm, byte* Data, const VField::FType& Type, bool InNative)
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
		Strm << *(FName*)Data;
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
			FName CName;
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
			FName CName = NAME_None;
			if (*(VClass**)Data)
			{
				CName = (*(VClass**)Data)->GetFName();
			}
			Strm << CName;
		}
		break;

	case ev_delegate:
		Strm.SerialiseReference(*(VObject**)Data, Type.Class);
		if (Strm.IsLoading())
		{
			FName FuncName;
			Strm << FuncName;
			if (*(VObject**)Data)
				((FFunction**)Data)[1] = (*(VObject**)Data)->GetVFunction(FuncName);
		}
		else
		{
			FName FuncName = NAME_None;
			if (*(VObject**)Data)
				FuncName = ((FFunction**)Data)[1]->Name;
			Strm << FuncName;
		}
		break;

	case ev_struct:
		for (VField* SF = Type.Struct->Fields; SF; SF = SF->Next)
		{
			SerialiseFieldValue(Strm, Data + SF->Ofs, SF->Type, InNative);
		}
		break;

	case ev_array:
		IntType = Type;
		IntType.Type = Type.ArrayInnerType;
		InnerSize = IntType.Type == ev_vector ? 12 : IntType.Type == ev_struct ? IntType.Struct->Size : 4;
		for (int i = 0; i < Type.ArrayDim; i++)
		{
			SerialiseFieldValue(Strm, Data + i * InnerSize, IntType, InNative);
		}
		break;
	}
	unguard;
}

//==========================================================================
//
//	SerialiseObject
//
//==========================================================================

static void SerialiseObject(VStream& Strm, VObject* Obj, VClass* InClass)
{
	guard(SerialiseObject);
	//	Serialise parent class fields.
	if (InClass->GetSuperClass())
	{
		SerialiseObject(Strm, Obj, InClass->GetSuperClass());
	}
	//	Serialise fields.
	bool InNative = !!(InClass->GetFlags() & CLASSOF_Native);
	for (VField* F = InClass->Fields; F; F = F->Next)
	{
		//	Skip fields with native serialisation.
		if (F->Flags & FIELD_Native)
		{
			continue;
		}
		SerialiseFieldValue(Strm, (byte*)Obj + F->Ofs, F->Type, InNative);
	}
	unguard;
}

//==========================================================================
//
//	VObject::Serialise
//
//==========================================================================

void VObject::Serialise(VStream& Strm)
{
	guard(VObject::Serialise);
	SerialiseObject(Strm, this, GetClass());
	unguard;
}

//==========================================================================
//
//	Object.Destroy
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, Destroy)
{
	VObject *ptr;

	ptr = (VObject *)PR_Pop();
	delete ptr;
}

//==========================================================================
//
//	Object.IsA
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, IsA)
{
	VObject *ptr;
	FName SomeName;

	SomeName = PR_PopName();
	ptr = (VObject *)PR_Pop();
	for (const VClass *c = ptr->Class; c; c = c->GetSuperClass())
	{
		if (c->GetFName() == SomeName)
		{
			PR_Push(true);
			return;
		}
	}
	PR_Push(false);
}

//==========================================================================
//
//	Object.IsDestroyed
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, IsDestroyed)
{
	VObject *ptr;

	ptr = (VObject *)PR_Pop();
	PR_Push(ptr->GetFlags() & _OF_Destroyed);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.17  2006/02/25 17:14:19  dj_jl
//	Implemented proper serialisation of the objects.
//
//	Revision 1.16  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//	
//	Revision 1.15  2005/11/24 20:09:23  dj_jl
//	Removed unused fields from Object class.
//	
//	Revision 1.14  2004/12/03 16:15:47  dj_jl
//	Implemented support for extended ACS format scripts, functions, libraries and more.
//	
//	Revision 1.13  2004/08/21 19:10:44  dj_jl
//	Changed sound driver declaration.
//	
//	Revision 1.12  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//	
//	Revision 1.11  2003/03/08 11:36:03  dj_jl
//	API fixes.
//	
//	Revision 1.10  2002/08/28 16:43:13  dj_jl
//	Fixed object registration.
//	
//	Revision 1.9  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.8  2002/07/15 17:51:09  dj_jl
//	Made VSubsystem global.
//	
//	Revision 1.7  2002/05/29 16:53:52  dj_jl
//	Added GetVFunction.
//	
//	Revision 1.6  2002/03/28 18:02:11  dj_jl
//	Changed native IsA to take name as argument.
//	
//	Revision 1.5  2002/03/09 18:05:34  dj_jl
//	Added support for defining native functions outside pr_cmds
//	
//	Revision 1.4  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2002/01/03 18:36:40  dj_jl
//	Added GObjInitialized
//	
//	Revision 1.2  2001/12/27 17:35:42  dj_jl
//	Split VClass in seperate module
//	
//	Revision 1.1  2001/12/18 19:03:17  dj_jl
//	A lots of work on VObject
//	
//**************************************************************************
