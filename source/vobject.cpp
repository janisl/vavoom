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
	(void(*)(void*))VObject::InternalConstructor
);
VClass* autoclassVObject = VObject::StaticClass();

bool				VObject::GObjInitialized;
TArray<VObject*>	VObject::GObjObjects;
TArray<int>			VObject::GObjAvailable;
VObject*			VObject::GObjHash[4096];
static int			GNumDeleted;
static bool			GInGarbageCollection;

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

//**************************************************************************
//
//	$Log$
//	Revision 1.22  2006/03/18 16:51:15  dj_jl
//	Renamed type class names, better code serialisation.
//
//	Revision 1.21  2006/03/06 13:02:32  dj_jl
//	Cleaning up references to destroyed objects.
//	
//	Revision 1.20  2006/02/28 18:04:36  dj_jl
//	Added script execution helpers.
//	
//	Revision 1.19  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//	
//	Revision 1.18  2006/02/26 20:52:49  dj_jl
//	Proper serialisation of level and players.
//	
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
