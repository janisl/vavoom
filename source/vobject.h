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
	friend inline VStream& operator<<(VStream& Strm, TClass*& Obj) \
		{ return Strm << *(VObject**)&Obj; } \
	static void InternalConstructor() \
		{ new TClass(); }

// Declare an abstract class.
#define DECLARE_ABSTRACT_CLASS(TClass, TSuperClass, TStaticFlags) \
	DECLARE_BASE_CLASS(TClass, TSuperClass, TStaticFlags | CLASS_Abstract) \
	virtual ~TClass() \
		{ ConditionalDestroy(); } \
	friend inline VStream& operator<<(VStream& Strm, TClass*& Obj) \
		{ return Strm << *(VObject**)&Obj; }

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
	virtual void ClearReferences();

	// Systemwide functions.
	static void StaticInit();
	static void StaticExit();
	static VObject* StaticSpawnObject(VClass*);
	static void CollectGarbage();
	static VObject* GetIndexObject(int Index);
	static int GetObjectsCount();

	static VStack ExecuteFunction(VMethod *func);
	static void DumpProfile();

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

	DECLARE_FUNCTION(Destroy)
	DECLARE_FUNCTION(IsA)
	DECLARE_FUNCTION(IsDestroyed)

	//	Error functions
	DECLARE_FUNCTION(Error)
	DECLARE_FUNCTION(FatalError)

	//	Cvar functions
	DECLARE_FUNCTION(CreateCvar)
	DECLARE_FUNCTION(GetCvar)
	DECLARE_FUNCTION(SetCvar)
	DECLARE_FUNCTION(GetCvarF)
	DECLARE_FUNCTION(SetCvarF)
	DECLARE_FUNCTION(GetCvarS)
	DECLARE_FUNCTION(SetCvarS)

	//	Math functions
	DECLARE_FUNCTION(AngleMod360)
	DECLARE_FUNCTION(AngleMod180)
	DECLARE_FUNCTION(abs)
	DECLARE_FUNCTION(fabs)
	DECLARE_FUNCTION(sin)
	DECLARE_FUNCTION(cos)
	DECLARE_FUNCTION(tan)
	DECLARE_FUNCTION(atan)
	DECLARE_FUNCTION(atan2)
	DECLARE_FUNCTION(Normalise)
	DECLARE_FUNCTION(Length)
	DECLARE_FUNCTION(DotProduct)
	DECLARE_FUNCTION(CrossProduct)
	DECLARE_FUNCTION(AngleVectors)
	DECLARE_FUNCTION(AngleVector)
	DECLARE_FUNCTION(VectorAngles)
	DECLARE_FUNCTION(GetPlanePointZ)
	DECLARE_FUNCTION(PointOnPlaneSide)
	DECLARE_FUNCTION(RotateDirectionVector)
	DECLARE_FUNCTION(VectorRotateAroundZ)

	//	String functions
	DECLARE_FUNCTION(strlen)
	DECLARE_FUNCTION(strcmp)
	DECLARE_FUNCTION(stricmp)
	DECLARE_FUNCTION(strcat)
	DECLARE_FUNCTION(strlwr)
	DECLARE_FUNCTION(strupr)
	DECLARE_FUNCTION(substr)
	DECLARE_FUNCTION(va)
	DECLARE_FUNCTION(atoi)
	DECLARE_FUNCTION(atof)

	//	Random numbers
	DECLARE_FUNCTION(Random)
	DECLARE_FUNCTION(P_Random)

	//	Textures
	DECLARE_FUNCTION(CheckTextureNumForName)
	DECLARE_FUNCTION(TextureNumForName)
	DECLARE_FUNCTION(CheckFlatNumForName)
	DECLARE_FUNCTION(FlatNumForName)
	DECLARE_FUNCTION(TextureHeight)

	//	Message IO functions
	DECLARE_FUNCTION(MSG_Select)
	DECLARE_FUNCTION(MSG_CheckSpace)
	DECLARE_FUNCTION(MSG_WriteByte)
	DECLARE_FUNCTION(MSG_WriteShort)
	DECLARE_FUNCTION(MSG_WriteLong)
	DECLARE_FUNCTION(MSG_ReadChar)
	DECLARE_FUNCTION(MSG_ReadByte)
	DECLARE_FUNCTION(MSG_ReadShort)
	DECLARE_FUNCTION(MSG_ReadWord)
	DECLARE_FUNCTION(MSG_ReadLong)

	//	Printing in console
	DECLARE_FUNCTION(print)
	DECLARE_FUNCTION(dprint)

	//	Type conversions
	DECLARE_FUNCTION(itof)
	DECLARE_FUNCTION(ftoi)
	DECLARE_FUNCTION(StrToName)

	//	Console command functions
	DECLARE_FUNCTION(Cmd_CheckParm)
	DECLARE_FUNCTION(CmdBuf_AddText)

	//	Misc
	DECLARE_FUNCTION(Info_ValueForKey)
	DECLARE_FUNCTION(WadLumpPresent)
	DECLARE_FUNCTION(SpawnObject)
	DECLARE_FUNCTION(FindClass)
	DECLARE_FUNCTION(StateIsInRange)

#ifdef CLIENT
	DECLARE_FUNCTION(P_GetMapName)
	DECLARE_FUNCTION(P_GetMapLumpName)
	DECLARE_FUNCTION(P_TranslateMap)
	DECLARE_FUNCTION(KeyNameForNum)
	DECLARE_FUNCTION(IN_GetBindingKeys)
	DECLARE_FUNCTION(IN_SetBinding)
	DECLARE_FUNCTION(SV_GetSaveString)
	DECLARE_FUNCTION(StartSearch)
	DECLARE_FUNCTION(GetSlist)

	DECLARE_FUNCTION(LoadTextLump)
	DECLARE_FUNCTION(AllocDlight)
	DECLARE_FUNCTION(NewParticle)

	//	Graphics
	DECLARE_FUNCTION(SetVirtualScreen)
	DECLARE_FUNCTION(R_RegisterPic)
	DECLARE_FUNCTION(R_RegisterPicPal)
	DECLARE_FUNCTION(R_GetPicInfo)
	DECLARE_FUNCTION(R_DrawPic)
	DECLARE_FUNCTION(R_DrawPic2)
	DECLARE_FUNCTION(R_DrawShadowedPic)
	DECLARE_FUNCTION(R_InstallSprite)
	DECLARE_FUNCTION(R_DrawSpritePatch)
	DECLARE_FUNCTION(InstallModel)
	DECLARE_FUNCTION(R_DrawModelFrame)
	DECLARE_FUNCTION(R_FillRectWithFlat)
	DECLARE_FUNCTION(R_ShadeRect)
	DECLARE_FUNCTION(R_FillRect)

	//	Text
	DECLARE_FUNCTION(T_SetFont)
	DECLARE_FUNCTION(T_SetAlign)
	DECLARE_FUNCTION(T_SetDist)
	DECLARE_FUNCTION(T_SetShadow)
	DECLARE_FUNCTION(T_TextWidth)
	DECLARE_FUNCTION(T_TextHeight)
	DECLARE_FUNCTION(T_DrawText)
	DECLARE_FUNCTION(T_DrawTextW)
	DECLARE_FUNCTION(T_DrawCursor)

	//	Client side sound
	DECLARE_FUNCTION(LocalSound)
	DECLARE_FUNCTION(IsLocalSoundPlaying)
	DECLARE_FUNCTION(StopLocalSounds)

	DECLARE_FUNCTION(TranslateKey)
#endif
#ifdef SERVER
	//	Print functions
	DECLARE_FUNCTION(bprint)

	//	Map utilites
	DECLARE_FUNCTION(LineOpenings)
	DECLARE_FUNCTION(P_BoxOnLineSide)
	DECLARE_FUNCTION(P_BlockThingsIterator)
	DECLARE_FUNCTION(P_PathTraverse)
	DECLARE_FUNCTION(FindThingGap)
	DECLARE_FUNCTION(FindOpening)
	DECLARE_FUNCTION(PointInRegion)
	DECLARE_FUNCTION(AddExtraFloor)
	DECLARE_FUNCTION(SwapPlanes)
	DECLARE_FUNCTION(MapBlock)
	DECLARE_FUNCTION(P_ChangeSector)

	//	Mobj utilites
	DECLARE_FUNCTION(NewMobjThinker)
	DECLARE_FUNCTION(NextMobj)

	//	Special thinker utilites
	DECLARE_FUNCTION(NewSpecialThinker)
	DECLARE_FUNCTION(RemoveSpecialThinker)
	DECLARE_FUNCTION(P_ChangeSwitchTexture)
	DECLARE_FUNCTION(NextThinker)

	//	ACS functions
	DECLARE_FUNCTION(StartACS)
	DECLARE_FUNCTION(SuspendACS)
	DECLARE_FUNCTION(TerminateACS)

	//	Sound functions
	DECLARE_FUNCTION(StartSoundAtVolume)
	DECLARE_FUNCTION(SectorStartSound)
	DECLARE_FUNCTION(SectorStopSound)
	DECLARE_FUNCTION(GetSoundPlayingInfo)
	DECLARE_FUNCTION(GetSoundID)
	DECLARE_FUNCTION(SetSeqTrans)
	DECLARE_FUNCTION(GetSeqTrans)
	DECLARE_FUNCTION(SectorStartSequence)
	DECLARE_FUNCTION(SectorStopSequence)
	DECLARE_FUNCTION(PolyobjStartSequence)
	DECLARE_FUNCTION(PolyobjStopSequence)

	DECLARE_FUNCTION(G_ExitLevel)
	DECLARE_FUNCTION(G_SecretExitLevel)
	DECLARE_FUNCTION(G_Completed)
	DECLARE_FUNCTION(SB_Start)
	DECLARE_FUNCTION(TerrainType)
	DECLARE_FUNCTION(P_ForceLightning)
	DECLARE_FUNCTION(SetFloorPic)
	DECLARE_FUNCTION(SetCeilPic)
	DECLARE_FUNCTION(SetLineTexture)
	DECLARE_FUNCTION(SetLineTransluc)
	DECLARE_FUNCTION(SendFloorSlope)
	DECLARE_FUNCTION(SendCeilingSlope)
	DECLARE_FUNCTION(SetSecLightColor)
	DECLARE_FUNCTION(SetFloorLightSector)
	DECLARE_FUNCTION(SetCeilingLightSector)
	DECLARE_FUNCTION(SetHeightSector)
	DECLARE_FUNCTION(FindModel)
	DECLARE_FUNCTION(GetModelIndex)
	DECLARE_FUNCTION(FindSkin)
	DECLARE_FUNCTION(FindClassFromEditorId)
	DECLARE_FUNCTION(FindClassFromScriptId)
	DECLARE_FUNCTION(ChangeMusic)
	DECLARE_FUNCTION(FindSectorFromTag)
#endif
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
//	Object creation template
//
template<class T> T* Spawn()
{
	return (T*)VObject::StaticSpawnObject(T::StaticClass());
}

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
#define P_PASS_STR(v)		PR_PushStr(v)
#define P_PASS_VEC(v)		PR_Pushv(v)
#define P_PASS_AVEC(v)		PR_Pushv(*(TVec*)&(v))
#define P_PASS_REF(v)		PR_PushPtr(v)
#define P_PASS_PTR(v)		PR_PushPtr(v)
#define P_PASS_SELF			PR_PushPtr(this)

//
//	Macros for calling VavoomC methods with different return types.
//
#define EV_RET_VOID(v)		ExecuteFunction(GetVFunction(v))
#define EV_RET_INT(v)		return ExecuteFunction(GetVFunction(v)).i
#define EV_RET_FLOAT(v)		return ExecuteFunction(GetVFunction(v)).f
#define EV_RET_BOOL(v)		return !!ExecuteFunction(GetVFunction(v)).i
#define EV_RET_NAME(v)		vint32 ret = ExecuteFunction(GetVFunction(v)).i; return *(VName*)&ret
#define EV_RET_STR(v)		VStack Ret = ExecuteFunction(GetVFunction(v)); PR_PushPtr(Ret.p); return PR_PopStr()
#define EV_RET_VEC(v)		Sys_Error("Not implemented")//ExecuteFunction(GetVFunction(v))
#define EV_RET_AVEC(v)		Sys_Error("Not implemented")//ExecuteFunction(GetVFunction(v))
#define EV_RET_REF(t, v)	return (t*)ExecuteFunction(GetVFunction(v)).p
#define EV_RET_PTR(t, v)	return (t*)ExecuteFunction(GetVFunction(v)).p

//
//	Parameter get macros. Parameters must be retrieved in backwards order.
//
#define P_GET_INT(v)		vint32 v = PR_Pop()
#define P_GET_FLOAT(v)		float v = PR_Popf()
#define P_GET_BOOL(v)		bool v = !!PR_Pop()
#define P_GET_NAME(v)		VName v = PR_PopName()
#define P_GET_STR(v)		VStr v = PR_PopStr()
#define P_GET_VEC(v)		TVec v = PR_Popv()
#define P_GET_AVEC(v)		TAVec v; v.roll = PR_Popf(); v.yaw = PR_Popf(); v.pitch = PR_Popf()
#define P_GET_REF(c, v)		c* v = (c*)PR_PopPtr()
#define P_GET_PTR(t, v)		t* v = (t*)PR_PopPtr()
#define P_GET_SELF			ThisClass* Self = (ThisClass*)PR_PopPtr()

//
//	Method return macros.
//
#define RET_INT(v)			PR_Push(v)
#define RET_FLOAT(v)		PR_Pushf(v)
#define RET_BOOL(v)			PR_Push(v)
#define RET_NAME(v)			PR_PushName(v)
#define RET_STR(v)			PR_PushStr(v)
#define RET_VEC(v)			PR_Pushv(v)
#define RET_AVEC(v)			PR_Pushv(*(TVec*)&(v))
#define RET_REF(v)			PR_PushPtr(v)
#define RET_PTR(v)			PR_PushPtr(v)
