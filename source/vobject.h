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
//**
//**	Vavoom object base class.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// Define private default constructor.
#define NO_DEFAULT_CONSTRUCTOR(cls) \
	protected: cls() {} public:

// Verify the a class definition and C++ definition match up.
#define VERIFY_CLASS_OFFSET(Pre,ClassName,Member)

// Verify that C++ and script code agree on the size of a class.
#define VERIFY_CLASS_SIZE(ClassName)

// Declare the base VObject class.
#define DECLARE_BASE_CLASS( TClass, TSuperClass, TStaticFlags ) \
public: \
	enum {StaticClassFlags = TStaticFlags}; \
	typedef TSuperClass Super; \
	typedef TClass ThisClass; \
	void* operator new(size_t, EInternal* Mem) \
		{ return (void*)Mem; }

// Declare a concrete class.
#define DECLARE_CLASS( TClass, TSuperClass, TStaticFlags ) \
	DECLARE_BASE_CLASS( TClass, TSuperClass, TStaticFlags )

// Declare an abstract class.
#define DECLARE_ABSTRACT_CLASS( TClass, TSuperClass, TStaticFlags ) \
	DECLARE_BASE_CLASS( TClass, TSuperClass, TStaticFlags | CLASS_Abstract )

// Declare that objects of class being defined reside within objects of the specified class.
#define DECLARE_WITHIN( TWithinClass ) \
	typedef TWithinClass WithinClass;

// Register a class at startup time.
#define IMPLEMENT_CLASS(TClass)

// ENUMERATIONS ------------------------------------------------------------

enum EInternal				{EC_Internal};

//
// Flags describing a class.
//
enum EClassFlags
{
	// Base flags.
	CLASS_Abstract          = 0x00001,  // Class is abstract and can't be instantiated directly.
	CLASS_Transient			= 0x00008,	// This object type can't be saved; null it out at save time.

	// Flags to inherit from base class.
	CLASS_Inherit           = CLASS_Transient,
	CLASS_RecompilerClear   = CLASS_Inherit | CLASS_Abstract,
};

// TYPES -------------------------------------------------------------------

//
//  A dummy class, which guaranties that compiler creates virtual function 
// table at offset 0. This is because in GCC by default virtual funvtion
// table is appended at the end. It also guaranties virtual destructors.
//
class VVirtualObjectBase
{
public:
	virtual ~VVirtualObjectBase(void) { }
};

//
//  Base object.
//
class VObject:public VVirtualObjectBase
{
	DECLARE_BASE_CLASS(VObject, VObject, CLASS_Abstract)

	friend class TProgs;

private:
	int *vtable;

public:
	void operator delete(void* Object, size_t)
		{ Z_Free(Object); }

	int GetClassID(void)
		{ return vtable[0]; }
	int GetSize(void)
		{ return vtable[1]; }
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2001/12/12 19:28:49  dj_jl
//	Some little changes, beautification
//
//	Revision 1.1  2001/12/01 17:43:13  dj_jl
//	Renamed ClassBase to VObject
//	
//**************************************************************************
