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

#include "vc_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

bool					VMemberBase::GObjInitialised;
TArray<VMemberBase*>	VMemberBase::GMembers;
VMemberBase*			VMemberBase::GMembersHash[4096];

TArray<VStr>			VMemberBase::GPackagePath;
TArray<VPackage*>		VMemberBase::GLoadedPackages;
TArray<VClass*>			VMemberBase::GDecorateClassImports;

VClass*					VMemberBase::GClasses;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

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

//==========================================================================
//
//	VMemberBase::VMemberBase
//
//==========================================================================

VMemberBase::VMemberBase(vuint8 AMemberType, VName AName, VMemberBase* AOuter,
	TLocation ALoc)
: MemberType(AMemberType)
, Name(AName)
, Outer(AOuter)
, Loc(ALoc)
{
	if (GObjInitialised)
	{
		MemberIndex = GMembers.Append(this);
		int HashIndex = Name.GetIndex() & 4095;
		HashNext = GMembersHash[HashIndex];
		GMembersHash[HashIndex] = this;
	}
}

//==========================================================================
//
//	VMemberBase::~VMemberBase
//
//==========================================================================

VMemberBase::~VMemberBase()
{
}

//==========================================================================
//
//	VMemberBase::GetFullName
//
//==========================================================================

VStr VMemberBase::GetFullName() const
{
	guardSlow(VMemberBase::GetFullName);
	if (Outer)
		return Outer->GetFullName() + "." + Name;
	return VStr(Name);
	unguardSlow;
}

//==========================================================================
//
//	VMemberBase::GetPackage
//
//==========================================================================

VPackage* VMemberBase::GetPackage() const
{
	guard(VMemberBase::GetPackage);
	for (const VMemberBase* p = this; p; p = p->Outer)
		if (p->MemberType == MEMBER_Package)
			return (VPackage*)p;
	Sys_Error("Member object %s not in a package", *GetFullName());
	return NULL;
	unguard;
}

//==========================================================================
//
//	VMemberBase::IsIn
//
//==========================================================================

bool VMemberBase::IsIn(VMemberBase* SomeOuter) const
{
	guardSlow(VMemberBase::IsIn);
	for (VMemberBase* Tst = Outer; Tst; Tst = Tst->Outer)
		if (Tst == SomeOuter)
			return true;
	return !SomeOuter;
	unguardSlow;
}

//==========================================================================
//
//	VMemberBase::Serialise
//
//==========================================================================

void VMemberBase::Serialise(VStream& Strm)
{
	Strm << Outer;
}

//==========================================================================
//
//	VMemberBase::PostLoad
//
//==========================================================================

void VMemberBase::PostLoad()
{
}

//==========================================================================
//
//	VMemberBase::Shutdown
//
//==========================================================================

void VMemberBase::Shutdown()
{
}

//==========================================================================
//
//	VMemberBase::StaticInit
//
//==========================================================================

void VMemberBase::StaticInit()
{
	guard(VMemberBase::StaticInit);
	//	Add native classes to the list.
	for (VClass* C = GClasses; C; C = C->LinkNext)
	{
		C->MemberIndex = GMembers.Append(C);
		int HashIndex = C->Name.GetIndex() & 4095;
		C->HashNext = GMembersHash[HashIndex];
		GMembersHash[HashIndex] = C;
		C->HashLowerCased();
	}

	//	Sprite TNT1 is always 0, ---- is always 1
	VClass::GSpriteNames.Append("tnt1");
	VClass::GSpriteNames.Append("----");

	GObjInitialised = true;
	unguard;
}

//==========================================================================
//
//	VMemberBase::StaticExit
//
//==========================================================================

void VMemberBase::StaticExit()
{
	for (int i = 0; i < GMembers.Num(); i++)
	{
		if (GMembers[i]->MemberType != MEMBER_Class ||
			!(((VClass*)GMembers[i])->ObjectFlags & CLASSOF_Native))
		{
			delete GMembers[i];
			GMembers[i] = NULL;
		}
		else
		{
			GMembers[i]->Shutdown();
		}
	}
	GMembers.Clear();
	GPackagePath.Clear();
	GLoadedPackages.Clear();
	GDecorateClassImports.Clear();
	VClass::GMobjInfos.Clear();
	VClass::GScriptIds.Clear();
	VClass::GSpriteNames.Clear();
	GObjInitialised = false;
}

//==========================================================================
//
//	VMemberBase::StaticAddPackagePath
//
//==========================================================================

void VMemberBase::StaticAddPackagePath(const char* Path)
{
	GPackagePath.Append(Path);
}

//==========================================================================
//
//	VMemberBase::StaticLoadPackage
//
//==========================================================================

VPackage* VMemberBase::StaticLoadPackage(VName AName, TLocation l)
{
	guard(VMemberBase::StaticLoadPackage);
	//	Check if already loaded.
	for (int i = 0; i < GLoadedPackages.Num(); i++)
	{
		if (GLoadedPackages[i]->Name == AName)
		{
			return GLoadedPackages[i];
		}
	}

	VPackage* Pkg = new VPackage(AName);
	GLoadedPackages.Append(Pkg);
	Pkg->LoadObject(l);
	return Pkg;
	unguard;
}

//==========================================================================
//
//	VMemberBase::StaticFindMember
//
//==========================================================================

VMemberBase* VMemberBase::StaticFindMember(VName AName, VMemberBase* AOuter,
	vuint8 AType)
{
	guard(VMemberBase::StaticFindMember);
	int HashIndex = AName.GetIndex() & 4095;
	for (VMemberBase* m = GMembersHash[HashIndex]; m; m = m->HashNext)
	{
		if (m->Name == AName && (m->Outer == AOuter ||
			(AOuter == ANY_PACKAGE && m->Outer && m->Outer->MemberType == MEMBER_Package)) &&
			(AType == ANY_MEMBER || m->MemberType == AType))
		{
			return m;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VMemberBase::StaticFindType
//
//==========================================================================

VFieldType VMemberBase::StaticFindType(VClass* AClass, VName Name)
{
	guard(VMemberBase::StaticFindType);
	if (Name == NAME_None)
	{
		return VFieldType(TYPE_Unknown);
	}

	VMemberBase* m = StaticFindMember(Name, ANY_PACKAGE, MEMBER_Class);
	if (m)
	{
		return VFieldType((VClass*)m);
	}
	m = StaticFindMember(Name, AClass ? (VMemberBase*)AClass :
		(VMemberBase*)ANY_PACKAGE, MEMBER_Struct);
	if (m)
	{
		return VFieldType((VStruct*)m);
	}
	if (AClass)
	{
		return StaticFindType(AClass->ParentClass, Name);
	}
	return VFieldType(TYPE_Unknown);
	unguard;
}

//==========================================================================
//
//	VMemberBase::StaticFindClass
//
//==========================================================================

VClass* VMemberBase::StaticFindClass(VName Name)
{
	guard(VMemberBase::StaticFindClass);
	VMemberBase* m = StaticFindMember(Name, ANY_PACKAGE, MEMBER_Class);
	if (m)
	{
		return (VClass*)m;
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VMemberBase::StaticSplitStateLabel
//
//==========================================================================

void VMemberBase::StaticSplitStateLabel(const VStr& LabelName,
	TArray<VName>& Parts)
{
	guard(VMemberBase::StaticSplitStateLabel);
	TArray<VStr> StrParts;
	LabelName.Split(".", StrParts);
	Parts.Clear();
	//	Remap old death state labels to proper names.
	if (StrParts[0] == "XDeath")
	{
		Parts.Append("Death");
		Parts.Append("Extreme");
	}
	else if (StrParts[0] == "Burn")
	{
		Parts.Append("Death");
		Parts.Append("Fire");
	}
	else if (StrParts[0] == "Ice")
	{
		Parts.Append("Death");
		Parts.Append("Ice");
	}
	else if (StrParts[0] == "Disintegrate")
	{
		Parts.Append("Death");
		Parts.Append("Disintegrate");
	}
	else
	{
		Parts.Append(*StrParts[0]);
	}
	for (int i = 1; i < StrParts.Num(); i++)
	{
		Parts.Append(*StrParts[i]);
	}
	unguard;
}
