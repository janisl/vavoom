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

// Verify the a class definition and C++ definition match up.
#define VERIFY_CLASS_OFFSET(Pre,ClassName,Member) /*\
	{for( TFieldIterator<UProperty> It( FindObjectChecked<UClass>( Pre##ClassName::StaticClass()->GetOuter(), TEXT(#ClassName) ) ); It; ++It ) \
		if( appStricmp(It->GetName(),TEXT(#Member))==0 ) \
			if( It->Offset != STRUCT_OFFSET(Pre##ClassName,Member) ) \
				appErrorf(TEXT("Class %s Member %s problem: Script=%i C++=%i"), TEXT(#ClassName), TEXT(#Member), It->Offset, STRUCT_OFFSET(Pre##ClassName,Member) );}*/

// Verify that C++ and script code agree on the size of a class.
#define VERIFY_CLASS_SIZE(ClassName) /*\
	check(sizeof(ClassName)==ClassName::StaticClass()->GetPropertiesSize());*/

// Register a class at startup time.
VClass VObject::PrivateStaticClass
(
	EC_NativeConstructor,
	sizeof(VObject),
	VObject::StaticClassFlags,
	NULL,
	"Object",
	OF_Public | OF_Standalone | OF_Transient | OF_Native,
	(void(*)(void*))VObject::InternalConstructor
);
VClass* autoclassVObject = VObject::StaticClass();

bool				VObject::GObjInitialized;
TArray<VObject*>	VObject::GObjObjects;
TArray<int>			VObject::GObjAvailable;
VObject*			VObject::GObjAutoRegister;
VObject*			VObject::GObjHash[4096];

//==========================================================================
//
//	VObject::VObject
//
//==========================================================================

VObject::VObject(void)
{
	Register();
}

//==========================================================================
//
//	VObject::VObject
//
//==========================================================================

VObject::VObject(ENativeConstructor, VClass* AClass, const char *AName, dword AFlags)
	: ObjectFlags(AFlags), Class(AClass)
{
	*(const char **)&Name = AName;
	HashNext = GObjAutoRegister;
	GObjAutoRegister = this;
}

//==========================================================================
//
//	VObject::~VObject
//
//==========================================================================

VObject::~VObject(void)
{
	guard(VObject::~VObject);
	if (!GObjInitialized)
	{
		return;
	}
	UnhashObject();
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
	guard(VObject::StaticInit);
	for (VObject *c = GObjAutoRegister; c; c = c->HashNext)
	{
		c->Name = FName(*(char **)&c->Name);
		c->Register();
	}
	GObjInitialized = true;
	unguard;
}

//==========================================================================
//
//	VObject::StaticExit
//
//==========================================================================

void VObject::StaticExit(void)
{
	GObjInitialized = false;
}

//==========================================================================
//
//	VObject::StaticSpawnObject
//
//==========================================================================

VObject *VObject::StaticSpawnObject(VClass *AClass, VObject *AOuter, int tag)
{
	guard(VObject::StaticSpawnObject);
	VObject *Obj = (VObject *)Z_Calloc(AClass->ClassSize, tag, 0);
	VClass *NativeClass = AClass;
	while (NativeClass && !(NativeClass->GetFlags() & OF_Native))
	{
		NativeClass = NativeClass->GetSuperClass();
	}
	if (!NativeClass)
	{
		Sys_Error("No native base class");
	}
	NativeClass->ClassConstructor(Obj);
	Obj->Class = AClass;
	Obj->Outer = AOuter;
	Obj->vtable = AClass->ClassVTable;
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

void VObject::Register(void)
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
	HashObject();
	unguard;
}

//==========================================================================
//
//	VObject::HashObject
//
//==========================================================================

void VObject::HashObject(void)
{
#if 0
	int HashIndex = GetTypeHash(Name) & 4095;
	HashNext = GObjHash[HashIndex];
	GObjHash[HashIndex] = this;
#endif
}

//==========================================================================
//
//	VObject::UnhashObject
//
//==========================================================================

void VObject::UnhashObject(void)
{
#if 0
	int HashIndex = GetTypeHash(Name) & 4095;
	if (GObjHash[HashIndex] == this)
	{
		GObjHash[HashIndex] = HashNext;
	}
	else
	{
		VObject *TempHash = GObjHash[HashIndex];
		while (TempHash && TempHash->HashNext != this)
		{
			TempHash = TempHash->HashNext;
		}
		if (TempHash)
		{
			TempHash->HashNext = HashNext;
		}
		else
		{
			con << "VObject::UnhashObject: Not in hash\n";
		}
	}
#endif
}

//==========================================================================
//
//	VObject::Destroy
//
//==========================================================================

void VObject::Destroy(void)
{
	SetFlags(OF_Destroyed);
}

//==========================================================================
//
//	VObject::Serialize
//
//==========================================================================

void VObject::Serialize(FArchive &Ar)
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
//	VObject::IsIn
//
//==========================================================================

bool VObject::IsIn(VObject *SomeOuter) const
{
	for (const VObject *o = Outer; o; o = o->GetOuter())
	{
		if (SomeOuter == o)
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
		if (Obj->GetFlags() & OF_Destroyed)
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
//	Object.Destroy
//
//==========================================================================

IMPLEMENT_FUNCTION(VObject, Destroy)
{
	VObject *ptr;

	ptr = (VObject *)PR_Pop();
	ptr->Destroy();
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
	PR_Push(ptr->GetFlags() & OF_Destroyed);
}

//**************************************************************************
//
//	$Log$
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
