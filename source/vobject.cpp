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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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

TArray<VObject*>	VObject::GObjObjects;
TArray<int>			VObject::GObjAvailable;
VObject*			VObject::GObjAutoRegister;
VObject*			VObject::GObjHash[4096];

// -------------------------------------------------------------------------

VObject::VObject(void)
{
	Register();
}

// -------------------------------------------------------------------------

VObject::VObject(ENativeConstructor, VClass* AClass, const char *AName, dword AFlags)
	: ObjectFlags(AFlags), Class(AClass)
{
	*(const char **)&Name = AName;
	HashNext = GObjAutoRegister;
	GObjAutoRegister = this;
}

// -------------------------------------------------------------------------

VObject::~VObject(void)
{
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
}

// -------------------------------------------------------------------------

void VObject::StaticInit(void)
{
	for (VObject *c = GObjAutoRegister; c; c = c->HashNext)
	{
		c->Name = FName(*(char **)&c->Name);
		c->Register();
	}
}

// -------------------------------------------------------------------------

void VObject::StaticExit(void)
{
}

// -------------------------------------------------------------------------

VObject *VObject::StaticSpawnObject(VClass *AClass, VObject *AOuter, int tag)
{
	try
	{
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
		NativeClass->ClassConstructor(Obj);//InternalConstructor(Obj);
		Obj->Class = AClass;
		Obj->Outer = AOuter;
		Obj->vtable = AClass->ClassVTable;
		if (Obj->vtable)
		{
			TProgs::Exec(Obj->vtable[4], (int)Obj);
		}
		return Obj;
	}
	catch (...)
	{
		dprintf("- VObject::StaticSpawnObject\n");
		dprintf("%p - %s\n", AClass, AClass->GetName());
		throw;
	}
}

// -------------------------------------------------------------------------

void VObject::Register(void)
{
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
}

// -------------------------------------------------------------------------

void VObject::HashObject(void)
{
#if 0
	int HashIndex = GetTypeHash(Name) & 4095;
	HashNext = GObjHash[HashIndex];
	GObjHash[HashIndex] = this;
#endif
}

// -------------------------------------------------------------------------

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

// -------------------------------------------------------------------------

void VObject::Destroy(void)
{
	if (IsA(VClass::StaticClass()))
	{
		con << "Destroying class " << ((VClass *)this)->GetName() << endl;
	}
	if (vtable)
	{
		TProgs::Exec(vtable[5], (int)this);
	}
	delete this;
}

// -------------------------------------------------------------------------

void VObject::Serialize(FArchive &Ar)
{
}

// -------------------------------------------------------------------------

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

// -------------------------------------------------------------------------

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

//**************************************************************************
//**************************************************************************

IMPLEMENT_CLASS(VClass)

// -------------------------------------------------------------------------

VClass::VClass(void)
{
}

// -------------------------------------------------------------------------

VClass::VClass(ENativeConstructor, size_t ASize, dword AClassFlags, 
	VClass *AParent, const char *AName, int AFlags, void(*ACtor)(void*))
	: VObject(EC_NativeConstructor, StaticClass(), AName, AFlags), 
	ParentClass(AParent), ClassSize(ASize),
	ClassFlags(AClassFlags), ClassConstructor(ACtor)
{
}

// -------------------------------------------------------------------------

VClass *VClass::FindClass(const char *AName)
{
	FName TempName(AName, FNAME_Find);
	if (TempName == NAME_None)
	{
		// No such name, no chance to find a class
		return NULL;
	}
	for (TObjectIterator<VClass> It; It; ++It)
	{
		if (It->GetFName() == TempName)
		{
			return *It;
		}
	}
	return NULL;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2001/12/18 19:03:17  dj_jl
//	A lots of work on VObject
//
//**************************************************************************
