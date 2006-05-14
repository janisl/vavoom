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
	static VClass* StaticClass() \
		{ return &PrivateStaticClass; }

// Declare a concrete class.
#define DECLARE_CLASS(TClass, TSuperClass, TStaticFlags) \
	DECLARE_BASE_CLASS(TClass, TSuperClass, TStaticFlags) \
	virtual ~TClass() \
		{ ConditionalDestroy(); } \
	static void InternalConstructor() \
		{ new TClass(); }

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
		Pre##TClass::InternalConstructor \
	); \
	VClass* autoclass##Pre##TClass = Pre##TClass::StaticClass();

#define DECLARE_FUNCTION(func) \
	static FBuiltinInfo funcinfo##func; \
	static void exec##func();

#define IMPLEMENT_FUNCTION(TClass, Func) \
	FBuiltinInfo TClass::funcinfo##Func(#Func, TClass::StaticClass(), \
		TClass::exec##Func); \
	void TClass::exec##Func()

// ENUMERATIONS ------------------------------------------------------------

//
// Flags describing a class.
//
enum EClassFlags
{
	// Base flags.
	CLASS_Native	= 0x0001,
	CLASS_Abstract	= 0x0002,  // Class is abstract and can't be instantiated directly.
};

//
// Flags describing an object instance.
//
enum EObjectFlags
{
	_OF_Destroyed		= 0x00000001,	// Object Destroy has already been called.
	_OF_DelayedDestroy	= 0x00000002,
	_OF_CleanupRef		= 0x00000004,
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
	virtual ~VVirtualObjectBase() { }
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
	VMethod**				vtable;
	vint32					Index;				// Index of object into table.
	vuint32					ObjectFlags;		// Private EObjectFlags used by object manager.
	VClass*					Class;	  			// Class the object belongs to.

	// Private systemwide variables.
	static bool				GObjInitialized;
	static TArray<VObject*>	GObjObjects;		// List of all objects.
	static TArray<int>		GObjAvailable;		// Available object indices.
	static VObject*			GObjHash[4096];		// Object hash.
	static int				GNumDeleted;
	static bool				GInGarbageCollection;
	static void*			GNewObject;			// For internal constructors.

public:
	// Constructors.
	VObject();
	static void InternalConstructor()
		{ new VObject(); }

	// Destructors.
	virtual ~VObject();

#ifdef ZONE_DEBUG_NEW
#undef new
#endif
	void* operator new(size_t);
	void* operator new(size_t, const char*, int);
	void operator delete(void*);
	void operator delete(void*, const char*, int);
#ifdef ZONE_DEBUG_NEW
#define new ZONE_DEBUG_NEW
#endif

	// VObject interface.
	virtual void Register();
	virtual void Destroy();
	virtual void Serialise(VStream&);

	// Systemwide functions.
	static void StaticInit();
	static void StaticExit();
	static VObject* StaticSpawnObject(VClass*);
	static void CollectGarbage();
	static VObject* GetIndexObject(int Index);
	static int GetObjectsCount();

	// Functions.
	bool ConditionalDestroy();
	bool IsA(VClass *SomeBaseClass) const;

	// Accessors.
	VClass* GetClass() const
	{
		return Class;
	}
	vuint32 GetFlags() const
	{
		return ObjectFlags;
	}
	void SetFlags(vuint32 NewFlags)
	{
		ObjectFlags |= NewFlags;
	}
	void ClearFlags(vuint32 NewFlags)
	{
		ObjectFlags &= ~NewFlags;
	}
	vuint32 GetIndex() const
	{
		return Index;
	}
	VMethod *GetVFunction(int InIndex) const
	{
		return vtable[InIndex];
	}
	VMethod *GetVFunction(VName FuncName) const;

	int ProcessEvent(VName FuncName)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this);
	}
	int ProcessEvent(VName FuncName, int Parm1)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1);
	}
	int ProcessEvent(VName FuncName, int Parm1, int Parm2)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1, Parm2);
	}
	int ProcessEvent(VName FuncName, int Parm1, int Parm2, int Parm3)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1, Parm2,
			Parm3);
	}
	int ProcessEvent(VName FuncName, int Parm1, int Parm2, int Parm3,
		int Parm4)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1, Parm2,
			Parm3, Parm4);
	}
	int ProcessEvent(VName FuncName, int Parm1, int Parm2, int Parm3,
		int Parm4, int Parm5)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1, Parm2,
			Parm3, Parm4, Parm5);
	}
	int ProcessEvent(VName FuncName, int Parm1, int Parm2, int Parm3,
		int Parm4, int Parm5, int Parm6)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1, Parm2,
			Parm3, Parm4, Parm5, Parm6);
	}
	int ProcessEvent(VName FuncName, int Parm1, int Parm2, int Parm3,
		int Parm4, int Parm5, int Parm6, int Parm7)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1, Parm2,
			Parm3, Parm4, Parm5, Parm6, Parm7);
	}
	int ProcessEvent(VName FuncName, int Parm1, int Parm2, int Parm3,
		int Parm4, int Parm5, int Parm6, int Parm7, int Parm8)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1, Parm2,
			Parm3, Parm4, Parm5, Parm6, Parm7, Parm8);
	}
	int ProcessEvent(VName FuncName, int Parm1, int Parm2, int Parm3,
		int Parm4, int Parm5, int Parm6, int Parm7, int Parm8, int Parm9)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1, Parm2,
			Parm3, Parm4, Parm5, Parm6, Parm7, Parm8, Parm9);
	}
	int ProcessEvent(VName FuncName, int Parm1, int Parm2, int Parm3,
		int Parm4, int Parm5, int Parm6, int Parm7, int Parm8, int Parm9,
		int Parm10)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1, Parm2,
			Parm3, Parm4, Parm5, Parm6, Parm7, Parm8, Parm9, Parm10);
	}
	int ProcessEvent(VName FuncName, int Parm1, int Parm2, int Parm3,
		int Parm4, int Parm5, int Parm6, int Parm7, int Parm8, int Parm9,
		int Parm10, int Parm11)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1, Parm2,
			Parm3, Parm4, Parm5, Parm6, Parm7, Parm8, Parm9, Parm10, Parm11);
	}
	int ProcessEvent(VName FuncName, int Parm1, int Parm2, int Parm3,
		int Parm4, int Parm5, int Parm6, int Parm7, int Parm8, int Parm9,
		int Parm10, int Parm11, int Parm12)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1, Parm2,
			Parm3, Parm4, Parm5, Parm6, Parm7, Parm8, Parm9, Parm10, Parm11,
			Parm12);
	}
	int ProcessEvent(VName FuncName, int Parm1, int Parm2, int Parm3,
		int Parm4, int Parm5, int Parm6, int Parm7, int Parm8, int Parm9,
		int Parm10, int Parm11, int Parm12, int Parm13)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1, Parm2,
			Parm3, Parm4, Parm5, Parm6, Parm7, Parm8, Parm9, Parm10, Parm11,
			Parm12, Parm13);
	}
	int ProcessEvent(VName FuncName, int Parm1, int Parm2, int Parm3,
		int Parm4, int Parm5, int Parm6, int Parm7, int Parm8, int Parm9,
		int Parm10, int Parm11, int Parm12, int Parm13, int Parm14)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1, Parm2,
			Parm3, Parm4, Parm5, Parm6, Parm7, Parm8, Parm9, Parm10, Parm11,
			Parm12, Parm13, Parm14);
	}
	int ProcessEvent(VName FuncName, int Parm1, int Parm2, int Parm3,
		int Parm4, int Parm5, int Parm6, int Parm7, int Parm8, int Parm9,
		int Parm10, int Parm11, int Parm12, int Parm13, int Parm14,
		int Parm15)
	{
		return TProgs::Exec(GetVFunction(FuncName), (int)this, Parm1, Parm2,
			Parm3, Parm4, Parm5, Parm6, Parm7, Parm8, Parm9, Parm10, Parm11,
			Parm12, Parm13, Parm14, Parm15);
	}

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

//
//	Helper macros for implementing native VavoomC functions and calls to the
// VavoomC methods. This will make it simpler to port it to 64 bit platforms.
//

//
//	Macros for passign arguments to VavoomC methods.
//
#define P_PASS_INT(v)		PR_Push(v)
#define P_PASS_FLOAT(v)		PR_Pushf(v)
#define P_PASS_BOOL(v)		PR_Push(v)
#define P_PASS_NAME(v)		PR_PushName(v)
#define P_PASS_VEC(v)		PR_Pushv(v)
#define P_PASS_AVEC(v)		PR_Pushv(*(TVec*)&(v))
#define P_PASS_REF(v)		PR_Push((int)v)
#define P_PASS_PTR(v)		PR_Push((int)v)
#define P_PASS_SELF			PR_Push((int)this)

//
//	Macros for calling VavoomC methods with different return types.
//
#define EV_RET_VOID(v)		TProgs::ExecuteFunction(GetVFunction(v))
#define EV_RET_INT(v)		return TProgs::ExecuteFunction(GetVFunction(v))
#define EV_RET_FLOAT(v)		int ret = TProgs::ExecuteFunction(GetVFunction(v)); return *(float*)&ret
#define EV_RET_BOOL(v)		return !!TProgs::ExecuteFunction(GetVFunction(v))
#define EV_RET_NAME(v)		int ret = TProgs::ExecuteFunction(GetVFunction(v)); return *(VName*)&ret
#define EV_RET_VEC(v)		Sys_Error("Not implemented")//TProgs::ExecuteFunction(GetVFunction(v))
#define EV_RET_AVEC(v)		Sys_Error("Not implemented")//TProgs::ExecuteFunction(GetVFunction(v))
#define EV_RET_REF(t, v)	return (t*)TProgs::ExecuteFunction(GetVFunction(v))
#define EV_RET_PTR(t, v)	return (t*)TProgs::ExecuteFunction(GetVFunction(v))

//
//	Parameter get macros. Parameters must be retrieved in backwards order.
//
#define P_GET_INT(v)		vint32 v = PR_Pop()
#define P_GET_FLOAT(v)		float v = PR_Popf()
#define P_GET_BOOL(v)		bool v = !!PR_Pop()
#define P_GET_NAME(v)		VName v = PR_PopName()
#define P_GET_VEC(v)		TVec v = PR_Popv()
#define P_GET_AVEC(v)		TAVec v; v.roll = PR_Popf(); v.yaw = PR_Popf(); v.pitch = PR_Popf()
#define P_GET_REF(c, v)		c* v = (c*)PR_Pop()
#define P_GET_PTR(t, v)		t* v = (t*)PR_Pop()
#define P_GET_SELF			ThisClass* Self = (ThisClass*)PR_Pop()

//
//	Method return macros.
//
#define RET_INT(v)			PR_Push(v)
#define RET_FLOAT(v)		PR_Pushf(v)
#define RET_BOOL(v)			PR_Push(v)
#define RET_NAME(v)			PR_PushName(v)
#define RET_VEC(v)			PR_Pushv(v)
#define RET_AVEC(v)			PR_Pushv(*(TVec*)&(v))
#define RET_REF(v)			PR_Push((int)v)
#define RET_PTR(v)			PR_Push((int)v)
