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
	static void InternalConstructor(void* X) \
		{ new((EInternal *)X)TClass(); }

// Declare an abstract class.
#define DECLARE_ABSTRACT_CLASS(TClass, TSuperClass, TStaticFlags) \
	DECLARE_BASE_CLASS(TClass, TSuperClass, TStaticFlags | CLASS_Abstract)

// Register a class at startup time.
#define IMPLEMENT_CLASS(TClass) \
	VClass TClass::PrivateStaticClass \
	( \
		EC_NativeConstructor, \
		sizeof(TClass), \
		TClass::StaticClassFlags, \
		TClass::Super::StaticClass(), \
		#TClass + 1, \
		OF_Public | OF_Standalone | OF_Transient | OF_Native, \
		(void(*)(void*))TClass::InternalConstructor \
	); \
	VClass* autoclass##TClass = TClass::StaticClass();

// ENUMERATIONS ------------------------------------------------------------

//
// Internal enums.
//
enum ENativeConstructor		{EC_NativeConstructor};
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

//
// Flags describing an object instance.
//
enum EObjectFlags
{
	OF_Public			= 0x00000001,	// Object is visible outside its package.
	OF_Transient        = 0x00000002,	// Don't save object.
	OF_Standalone       = 0x00000004,   // Keep object around for editing even if unreferenced.
	OF_Native			= 0x00000008,   // Native (VClass only).
	OF_EliminateObject  = 0x00000010,   // NULL out references to this during garbage collecion.
	OF_Destroyed        = 0x00000020,	// Object Destroy has already been called.
};

// TYPES -------------------------------------------------------------------

class	FArchive;

class	VObject;
class		VClass;

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
class VObject:public VVirtualObjectBase
{
	// Declarations.
	DECLARE_BASE_CLASS(VObject, VObject, CLASS_Abstract)

	// Friends.
	friend class FObjectIterator;
	friend class TProgs;

private:
	// Internal per-object variables.
	int						*vtable;
	int						Index;				// Index of object into table.
	VObject*				HashNext;			// Next object in this hash bin.
	VObject*				Outer;				// Object this object resides in.
	dword					ObjectFlags;		// Private EObjectFlags used by object manager.
	FName					Name;				// Name of the object.
	VClass*					Class;	  			// Class the object belongs to.

	// Private systemwide variables.
	static TArray<VObject*>	GObjObjects;		// List of all objects.
	static TArray<int>		GObjAvailable;		// Available object indices.
	static VObject*			GObjAutoRegister;	// Objects to automatically register.
	static VObject*			GObjHash[4096];		// Object hash.

	// Private functions.
	void HashObject(void);
	void UnhashObject(void);

public:
	// Constructors.
	VObject();
	VObject(ENativeConstructor, VClass* AClass, const char *AName, dword AFlags);
	static void InternalConstructor(void* X)
		{ new((EInternal*)X)VObject(); }

	// Destructors.
	virtual ~VObject();
	void operator delete(void* Object, size_t)
		{ Z_Free(Object); }

	// VObject interface.
	virtual void Register(void);
	virtual void Destroy(void);
	virtual void Serialize(FArchive &Ar);

	// Systemwide functions.
	static void StaticInit(void);
	static void StaticExit(void);
	static VObject *StaticSpawnObject(VClass *, VObject *, int);

	// Functions.
	bool IsA(VClass *SomeBaseClass) const;
	bool IsIn(VObject *SomeOuter) const;

	// Accessors.
	VClass* GetClass(void) const
	{
		return Class;
	}
	dword GetFlags(void) const
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
	const char *GetName(void) const
	{
		return *Name;
	}
	const FName GetFName(void) const
	{
		return Name;
	}
	VObject *GetOuter(void) const
	{
		return Outer;
	}
	dword GetIndex(void) const
	{
		return Index;
	}
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

//==========================================================================
//
//	VClass
//
//==========================================================================

class VClass:public VObject
{
	DECLARE_CLASS(VClass, VObject, 0)

	VClass*		ParentClass;

	size_t		ClassSize;
	dword		ClassFlags;
	int*		ClassVTable;
	void (*ClassConstructor)(void*);

	static VClass *FindClass(const char *);

	VClass(void);
	VClass(ENativeConstructor, size_t ASize, dword AClassFlags,
		VClass *AParent, const char *AName, int AFlags, 
		void(*ACtor)(void*));

	bool IsChildOf(const VClass *SomeBaseClass) const
	{
		for (const VClass *c = this; c; c = c->GetSuperClass())
		{
			if (SomeBaseClass == c)
			{
				return true;
			}
		}
		return false;
	}

	VClass *GetSuperClass(void) const
	{
		return ParentClass;
	}
};

//**************************************************************************
//
//	$Log$
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
