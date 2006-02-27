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
//**
//**	Vavoom object base class.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// Define private default constructor.
#define NO_DEFAULT_CONSTRUCTOR(cls) \
	protected: cls() {} public:

// Declare the base VObject class.
#define DECLARE_BASE_CLASS(TClass, TSuperClass, TStaticFlags) \
public: \
	/* Identification */ \
	enum {StaticClassFlags = TStaticFlags}; \
	private: static VClass PrivateStaticClass; public: \
	typedef TSuperClass Super;\
	typedef TClass ThisClass;\
	static VClass* StaticClass(void) \
		{ return &PrivateStaticClass; } \
	void* operator new(size_t, EInternal* Mem) \
		{ return (void *)Mem; }

// Declare a concrete class.
#define DECLARE_CLASS(TClass, TSuperClass, TStaticFlags) \
	DECLARE_BASE_CLASS(TClass, TSuperClass, TStaticFlags) \
	virtual ~TClass() \
		{ ConditionalDestroy(); } \
	static void InternalConstructor(void* X) \
		{ new((EInternal *)X)TClass(); }

// Declare an abstract class.
#define DECLARE_ABSTRACT_CLASS(TClass, TSuperClass, TStaticFlags) \
	DECLARE_BASE_CLASS(TClass, TSuperClass, TStaticFlags | CLASS_Abstract) \
	virtual ~TClass() \
		{ ConditionalDestroy(); }

// Register a class at startup time.
#define IMPLEMENT_CLASS(Pre, TClass) \
	VClass Pre##TClass::PrivateStaticClass \
	( \
		EC_NativeConstructor, \
		sizeof(Pre##TClass), \
		Pre##TClass::StaticClassFlags, \
		Pre##TClass::Super::StaticClass(), \
		NAME_##TClass, \
		(void(*)(void*))Pre##TClass::InternalConstructor \
	); \
	VClass* autoclass##Pre##TClass = Pre##TClass::StaticClass();

#define DECLARE_FUNCTION(func) \
	static FBuiltinInfo funcinfo##func; \
	static void exec##func(void);

#define IMPLEMENT_FUNCTION(TClass, Func) \
	FBuiltinInfo TClass::funcinfo##Func(#Func, TClass::StaticClass(), \
		TClass::exec##Func); \
	void TClass::exec##Func(void)

// ENUMERATIONS ------------------------------------------------------------

//
// Internal enums.
//
enum EInternal				{EC_Internal};

//
// Flags describing a class.
//
enum EClassFlags
{
	// Base flags.
	CLASS_Abstract		= 0x00001,  // Class is abstract and can't be instantiated directly.
};

//
// Flags describing an object instance.
//
enum EObjectFlags
{
	_OF_Destroyed		= 0x00000001,	// Object Destroy has already been called.
};

// TYPES -------------------------------------------------------------------

//==========================================================================
//
//	VObject
//
//==========================================================================

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
// The base class of all objects.
//
class VObject : public VVirtualObjectBase
{
	// Declarations.
	DECLARE_BASE_CLASS(VObject, VObject, CLASS_Abstract)

	// Friends.
	friend class FObjectIterator;
	friend class TProgs;

private:
	// Internal per-object variables.
	FFunction**				vtable;
	int						Index;				// Index of object into table.
	dword					ObjectFlags;		// Private EObjectFlags used by object manager.
	VClass*					Class;	  			// Class the object belongs to.

	// Private systemwide variables.
	static bool				GObjInitialized;
	static TArray<VObject*>	GObjObjects;		// List of all objects.
	static TArray<int>		GObjAvailable;		// Available object indices.
	static VObject*			GObjHash[4096];		// Object hash.

public:
	// Constructors.
	VObject();
	static void InternalConstructor(void* X)
		{ new((EInternal*)X)VObject(); }

	// Destructors.
	virtual ~VObject();
	void operator delete(void* Object, size_t)
		{ Z_Free(Object); }

	// VObject interface.
	virtual void Register();
	virtual void Destroy();
	virtual void Serialise(VStream&);

	// Systemwide functions.
	static void StaticInit();
	static void StaticExit();
	static VObject *StaticSpawnObject(VClass*, int);
	static void CollectGarbage();
	static VObject *GetIndexObject(int Index);
	static int GetObjectsCount();

	// Functions.
	bool ConditionalDestroy();
	bool IsA(VClass *SomeBaseClass) const;

	// Accessors.
	VClass* GetClass() const
	{
		return Class;
	}
	dword GetFlags() const
	{
		return ObjectFlags;
	}
	void SetFlags(dword NewFlags)
	{
		ObjectFlags |= NewFlags;
	}
	void ClearFlags(dword NewFlags)
	{
		ObjectFlags &= ~NewFlags;
	}
	dword GetIndex() const
	{
		return Index;
	}
	FFunction *GetVFunction(int InIndex) const
	{
		return vtable[InIndex];
	}
	FFunction *GetVFunction(VName FuncName) const;

	DECLARE_FUNCTION(Destroy)
	DECLARE_FUNCTION(IsA)
	DECLARE_FUNCTION(IsDestroyed)
};

// Dynamically cast an object type-safely.
template<class T> T* Cast(VObject* Src)
{
	return Src && Src->IsA(T::StaticClass()) ? (T*)Src : NULL;
}
template<class T, class U> T* CastChecked(U* Src)
{
	if (!Src || !Src->IsA(T::StaticClass()))
		Sys_Error("Cast to %s failed", T::StaticClass()->GetName());
	return (T*)Src;
}

/*----------------------------------------------------------------------------
	Object iterators.
----------------------------------------------------------------------------*/

//
// Class for iterating through all objects.
//
class FObjectIterator
{
public:
	FObjectIterator(VClass* InClass = VObject::StaticClass())
	:	Class(InClass), Index(-1)
	{
		++*this;
	}
	void operator++()
	{
		while (++Index < VObject::GObjObjects.Num() && 
			(!VObject::GObjObjects[Index] || 
				!VObject::GObjObjects[Index]->IsA(Class)));
	}
	VObject* operator*()
	{
		return VObject::GObjObjects[Index];
	}
	VObject* operator->()
	{
		return VObject::GObjObjects[Index];
	}
	operator bool()
	{
		return Index < VObject::GObjObjects.Num();
	}
protected:
	VClass* Class;
	int Index;
};

//
// Class for iterating through all objects which inherit from a
// specified base class.
//
template<class T> class TObjectIterator : public FObjectIterator
{
public:
	TObjectIterator()
	:	FObjectIterator(T::StaticClass())
	{}
	T* operator*()
	{
		return (T*)FObjectIterator::operator*();
	}
	T* operator->()
	{
		return (T*)FObjectIterator::operator->();
	}
};

//**************************************************************************
//
//	$Log$
//	Revision 1.17  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//
//	Revision 1.16  2006/02/25 17:14:19  dj_jl
//	Implemented proper serialisation of the objects.
//	
//	Revision 1.15  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//	
//	Revision 1.14  2005/11/24 20:09:23  dj_jl
//	Removed unused fields from Object class.
//	
//	Revision 1.13  2004/12/03 16:15:47  dj_jl
//	Implemented support for extended ACS format scripts, functions, libraries and more.
//	
//	Revision 1.12  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//	
//	Revision 1.11  2003/03/08 11:36:03  dj_jl
//	API fixes.
//	
//	Revision 1.10  2002/05/29 16:53:52  dj_jl
//	Added GetVFunction.
//	
//	Revision 1.9  2002/05/18 16:56:35  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.8  2002/03/09 18:05:34  dj_jl
//	Added support for defining native functions outside pr_cmds
//	
//	Revision 1.7  2002/02/02 19:20:41  dj_jl
//	FFunction pointers used instead of the function numbers
//	
//	Revision 1.6  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.5  2002/01/03 18:36:40  dj_jl
//	Added GObjInitialized
//	
//	Revision 1.4  2001/12/27 17:35:42  dj_jl
//	Split VClass in seperate module
//	
//	Revision 1.3  2001/12/18 19:03:17  dj_jl
//	A lots of work on VObject
//	
//	Revision 1.2  2001/12/12 19:28:49  dj_jl
//	Some little changes, beautification
//	
//	Revision 1.1  2001/12/01 17:43:13  dj_jl
//	Renamed ClassBase to VObject
//	
//**************************************************************************
