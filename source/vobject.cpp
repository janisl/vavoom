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

// Register a class at startup time.
VClass VObject::PrivateStaticClass
(
	EC_NativeConstructor,
	sizeof(VObject),
	VObject::StaticClassFlags,
	NULL,
	NAME_Object,
	VObject::InternalConstructor
);
VClass* autoclassVObject = VObject::StaticClass();

bool				VObject::GObjInitialized;
TArray<VObject*>	VObject::GObjObjects;
TArray<int>			VObject::GObjAvailable;
VObject*			VObject::GObjHash[4096];
int					VObject::GNumDeleted;
bool				VObject::GInGarbageCollection;
void*				VObject::GNewObject;

//==========================================================================
//
//	VObject::VObject
//
//==========================================================================

VObject::VObject()
{
}

//==========================================================================
//
//	VObject::~VObject
//
//==========================================================================

VObject::~VObject()
{
	guard(VObject::~VObject);
	ConditionalDestroy();
	GNumDeleted--;
	if (!GObjInitialized)
	{
		return;
	}

	if (!GInGarbageCollection)
	{
		SetFlags(_OF_CleanupRef);
		for (int i = 0; i < GObjObjects.Num(); i++)
		{
			if (!GObjObjects[i] || GObjObjects[i]->GetFlags() & _OF_Destroyed)
			{
				continue;
			}
			GObjObjects[i]->GetClass()->CleanObject(GObjObjects[i]);
		}
	}

	if (Index == GObjObjects.Num() - 1)
	{
		GObjObjects.RemoveIndex(Index);
	}
	else
	{
		GObjObjects[Index] = NULL;
		GObjAvailable.Append(Index);
	}
	unguard;
}

//==========================================================================
//
//	VObject::StaticInit
//
//==========================================================================

#ifdef ZONE_DEBUG_NEW
#undef new
#endif

void* VObject::operator new(size_t)
{
	check(GNewObject);
	return GNewObject;
}

//==========================================================================
//
//	VObject::StaticInit
//
//==========================================================================

void* VObject::operator new(size_t, const char*, int)
{
	check(GNewObject);
	return GNewObject;
}

#ifdef ZONE_DEBUG_NEW
#define new ZONE_DEBUG_NEW
#endif

//==========================================================================
//
//	VObject::StaticInit
//
//==========================================================================

void VObject::operator delete(void* Object)
{
	Z_Free(Object);
}

//==========================================================================
//
//	VObject::StaticInit
//
//==========================================================================

void VObject::operator delete(void* Object, const char*, int)
{
	Z_Free(Object);
}

//==========================================================================
//
//	VObject::StaticInit
//
//==========================================================================

void VObject::StaticInit()
{
	VMemberBase::StaticInit();
	GObjInitialized = true;
}

//==========================================================================
//
//	VObject::StaticExit
//
//==========================================================================

void VObject::StaticExit()
{
	CollectGarbage();
	GObjObjects.Clear();
	GObjAvailable.Clear();
	GObjInitialized = false;
	VMemberBase::StaticExit();
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
	GNewObject = Obj;
	NativeClass->ClassConstructor();
	GNewObject = NULL;
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
		Index = GObjAvailable[GObjAvailable.Num() - 1];
		GObjAvailable.RemoveIndex(GObjAvailable.Num() - 1);
		GObjObjects[Index] = this;
	}
	else
	{
		Index = GObjObjects.Append(this);
	}
	unguard;
}

//==========================================================================
//
//	VObject::ConditionalDestroy
//
//==========================================================================

bool VObject::ConditionalDestroy()
{
	if (!(ObjectFlags & _OF_Destroyed))
	{
		GNumDeleted++;
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

VMethod *VObject::GetVFunction(VName FuncName) const
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

void VObject::CollectGarbage()
{
	guard(VObject::CollectGarbage);
	if (!GNumDeleted)
	{
		return;
	}

	GInGarbageCollection = true;
	//	Mark objects to be cleaned.
	for (int i = 0; i < GObjObjects.Num(); i++)
	{
		if (!GObjObjects[i])
		{
			continue;
		}
		VObject *Obj = GObjObjects[i];
		if (Obj->GetFlags() & _OF_Destroyed)
		{
			Obj->SetFlags(_OF_CleanupRef);
		}
	}

	//	Clean references.
	for (int i = 0; i < GObjObjects.Num(); i++)
	{
		if (!GObjObjects[i] || GObjObjects[i]->GetFlags() & _OF_Destroyed)
		{
			continue;
		}
		GObjObjects[i]->GetClass()->CleanObject(GObjObjects[i]);
	}

	//	Now actually delete the objects.
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
	GInGarbageCollection = false;
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

int VObject::GetObjectsCount()
{
	return GObjObjects.Num();
}

//==========================================================================
//
//	VObject::Serialise
//
//==========================================================================

void VObject::Serialise(VStream& Strm)
{
	guard(VObject::Serialise);
	GetClass()->SerialiseObject(Strm, this);
	unguard;
}

//==========================================================================
//
//	Object.Destroy
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, Destroy)
{
	P_GET_SELF;
	delete Self;
}

//==========================================================================
//
//	Object.IsA
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, IsA)
{
	P_GET_NAME(SomeName);
	P_GET_SELF;
	bool Ret = false;
	for (const VClass *c = Self->Class; c; c = c->GetSuperClass())
	{
		if (c->GetVName() == SomeName)
		{
			Ret = true;
			break;
		}
	}
	RET_BOOL(Ret);
}

//==========================================================================
//
//	Object.IsDestroyed
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, IsDestroyed)
{
	P_GET_SELF;
	RET_BOOL(Self->GetFlags() & _OF_Destroyed);
}
