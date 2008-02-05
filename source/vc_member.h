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

class VExpression;

#define ANY_PACKAGE				((VPackage*)-1)
#define ANY_MEMBER				255

enum
{
	MEMBER_Package,
	MEMBER_Field,
	MEMBER_Property,
	MEMBER_Method,
	MEMBER_State,
	MEMBER_Const,
	MEMBER_Struct,
	MEMBER_Class,
};

//==========================================================================
//
//	VMemberBase
//
//	The base class of all objects.
//
//==========================================================================

class VMemberBase
{
public:
	//	Internal variables.
	vuint8			MemberType;
	vint32			MemberIndex;
	VName			Name;
	VMemberBase*	Outer;
	TLocation		Loc;
	VMemberBase*	HashNext;

	static bool						GObjInitialised;
	static TArray<VMemberBase*>		GMembers;
	static VMemberBase*				GMembersHash[4096];

	static TArray<VStr>				GPackagePath;
	static TArray<VPackage*>		GLoadedPackages;

	static VClass*					GClasses;	// Linked list of all classes.
	static TArray<VClass*>			GNetClassLookup;

	//	Srtuctors.
	VMemberBase(vuint8, VName, VMemberBase*, TLocation);
	virtual ~VMemberBase();

	//	Accessors.
	const char *GetName() const
	{
		return *Name;
	}
	const VName GetVName() const
	{
		return Name;
	}
	VStr GetFullName() const;
	VPackage* GetPackage() const;
	bool IsIn(VMemberBase*) const;

	virtual void Serialise(VStream&);
	virtual void PostLoad();
	virtual void Shutdown();

	static void StaticInit();
	static void StaticExit();
	static void StaticAddPackagePath(const char*);
	static VPackage* StaticLoadPackage(VName, TLocation);
	static VMemberBase* StaticFindMember(VName, VMemberBase*, vuint8);

	//FIXME This looks ugly.
	static VFieldType StaticFindType(VClass*, VName);
	static VClass* StaticFindClass(VName);

	static void StaticSetUpNetClasses();
};
