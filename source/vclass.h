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

//
// Flags describing an class instance.
//
enum EClassObjectFlags
{
	CLASSOF_Native			= 0x00000001,   // Native
	CLASSOF_PostLoaded		= 0x00000002,	// PostLoad has been called
};

enum ENativeConstructor		{EC_NativeConstructor};

struct VObjectDelegate
{
	VObject*		Obj;
	VMethod*		Func;
};

//==========================================================================
//
//	VMemberBase
//
//==========================================================================

class VMemberBase
{
public:
	//	Internal variables.
	vuint8			MemberType;
	VMemberBase*	Outer;
	VName			Name;

	static bool						GObjInitialised;
	static VClass*					GClasses;	// Linked list of all classes.
	static TArray<VMemberBase*>		GMembers;
	static TArray<VPackage*>		GLoadedPackages;
	static TArray<VClass*>			GNetClassLookup;

	//	Srtuctors.
	VMemberBase(vuint8, VName);
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

	virtual void Serialise(VStream&);
	virtual void PostLoad();
	virtual void Shutdown();

	static void StaticInit();
	static void StaticExit();
	static VPackage* StaticLoadPackage(VName);
	static VMemberBase* StaticFindMember(VName, VMemberBase*, vuint8);
	static void SetUpNetClasses();
};

//==========================================================================
//
//	VPackage
//
//==========================================================================

class VPackage : public VMemberBase
{
public:
	VPackage(VName);
	~VPackage();

	vuint16			Checksum;
	char*			Strings;
	VProgsReader*	Reader;

	void Serialise(VStream&);

	friend inline VStream& operator<<(VStream& Strm, VPackage*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

//==========================================================================
//
//	VField
//
//==========================================================================

class VField : public VMemberBase
{
public:
	struct FType
	{
		vuint8		Type;
		vuint8		InnerType;		//	For pointers
		vuint8		ArrayInnerType;	//	For arrays
		vuint8		PtrLevel;
		vint32		ArrayDim;
		union
		{
			vuint32		BitMask;
			VClass*		Class;			//  Class of the reference
			VStruct*	Struct;			//  Struct data.
			VMethod*	Function;		//  Function of the delegate type.
		};

		friend VStream& operator<<(VStream&, FType&);

		int GetSize() const;
		int GetAlignment() const;
		FType GetArrayInnerType() const;
	};

	VField*		Next;
	VField*		NextReference;	//	Linked list of reference fields.
	VField*		DestructorLink;
	VField*		NextNetField;
	vint32		Ofs;
	FType		Type;
	VMethod*	Func;
	vint32		Flags;
	vint32		NetIndex;
	VMethod*	ReplCond;

	VField(VName);

	void Serialise(VStream&);

	static void CopyFieldValue(const vuint8*, vuint8*, const FType&);
	static void SerialiseFieldValue(VStream&, vuint8*, const FType&);
	static void CleanField(vuint8*, const FType&);
	static void DestructField(vuint8*, const FType&);
	static bool IdenticalValue(const vuint8*, const vuint8*, const FType&);
	static bool NetSerialiseValue(VStream&, VNetObjectsMap*, vuint8*, const FType&);

	friend inline VStream& operator<<(VStream& Strm, VField*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

//==========================================================================
//
//	VProperty
//
//==========================================================================

class VProperty : public VMemberBase
{
public:
	VField::FType	Type;
	VMethod*		GetFunc;
	VMethod*		SetFunc;
	VField*			DefaultField;
	vuint32			Flags;

	VProperty(VName);

	void Serialise(VStream&);

	friend VStream& operator<<(VStream& Strm, VProperty*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

//==========================================================================
//
//	VMethod
//
//==========================================================================

struct FInstruction
{
	vint32			Address;
	vint32			Opcode;
	vint32			Arg1;
	vint32			Arg2;
	VMemberBase*	Member;
	VName			NameArg;
	VField::FType	TypeArg;
};

class VMethod : public VMemberBase
{
public:
	//FIXME avoid copying
	enum { MAX_PARAMS		= 16 };

	vint32			NumLocals;
	vint32			Flags;
	VField::FType	ReturnType;
	vint32			NumParams;
	vint32			ParamsSize;
	VField::FType	ParamTypes[MAX_PARAMS];
	vuint8			ParamFlags[MAX_PARAMS];
	vint32			NumInstructions;
	FInstruction*	Instructions;
	VMethod*		SuperMethod;
	VMethod*		ReplCond;

	vuint32			Profile1;
	vuint32			Profile2;
	TArray<vuint8>	Statements;
	builtin_t		NativeFunc;
	vint16			VTableIndex;
	vint32			NetIndex;
	VMethod*		NextNetMethod;

	VMethod(VName);
	~VMethod();

	void Serialise(VStream&);
	void PostLoad();

	friend inline VStream& operator<<(VStream& Strm, VMethod*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }

private:
	void CompileCode();
	void OptimiseInstructions();
};

//==========================================================================
//
//	VState
//
//==========================================================================

class VState : public VMemberBase
{
public:
	VName		SpriteName;
	vint32		SpriteIndex;
	vint32		Frame;
	float		Time;
	vint32		Misc1;
	vint32		Misc2;
	VState*		NextState;
	VMethod*	Function;
	VState*		Next;

	vint32		InClassIndex;
	vint32		NetId;
	VState*		NetNext;

	VState(VName);

	void Serialise(VStream&);

	bool IsInRange(VState*, VState*, int);
	bool IsInSequence(VState*);

	friend inline VStream& operator<<(VStream& Strm, VState*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

//==========================================================================
//
//	VConstant
//
//==========================================================================

class VConstant : public VMemberBase
{
public:
	vuint8		Type;
	union
	{
		vint32	Value;
		float	FloatValue;
	};

	VConstant(VName);

	void Serialise(VStream&);

	friend inline VStream& operator<<(VStream& Strm, VConstant*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

//==========================================================================
//
//	VStruct
//
//==========================================================================

class VStruct : public VMemberBase
{
public:
	vint32			ObjectFlags;
	VStruct*		ParentStruct;
	vint32			Size;
	vuint8			Alignment;
	vuint8			IsVector;
	VField*			Fields;
	VField*			ReferenceFields;
	VField*			DestructorFields;

	VStruct(VName);

	void Serialise(VStream&);
	void PostLoad();

	void CalcFieldOffsets();
	void InitReferences();
	void InitDestructorFields();
	void CopyObject(const vuint8*, vuint8*);
	void SerialiseObject(VStream&, vuint8*);
	void CleanObject(vuint8*);
	void DestructObject(vuint8*);
	bool IdenticalObject(const vuint8*, const vuint8*);
	bool NetSerialiseObject(VStream&, VNetObjectsMap*, vuint8*);

	friend inline VStream& operator<<(VStream& Strm, VStruct*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

//==========================================================================
//
//	mobjinfo_t
//
//==========================================================================

struct mobjinfo_t
{
	vint32		doomednum;
	vint32		GameFilter;
	VClass*		class_id;

	friend VStream& operator<<(VStream&, mobjinfo_t&);
};

//==========================================================================
//
//	VClass
//
//==========================================================================

struct VRepInfo
{
	VMethod*				Cond;
	TArray<VMemberBase*>	RepMembers;
};

class VClass : public VMemberBase
{
private:
	// Internal per-object variables.
	vuint32			ObjectFlags;		// Private EObjectFlags used by object manager.

	friend class VMemberBase;
public:
	VClass*			LinkNext;			// Next class in linked list
	VClass*			ParentClass;

	vint32			ClassSize;
	vint32			ClassUnalignedSize;
	vuint32			ClassFlags;
	VMethod**		ClassVTable;
	void			(*ClassConstructor)();

	vint32			ClassNumMethods;

	VField*			Fields;
	VField*			ReferenceFields;
	VField*			DestructorFields;
	VField*			NetFields;
	VMethod*		NetMethods;
	VState*			States;
	VMethod*		DefaultProperties;

	vuint8*			Defaults;

	vint32				NetId;
	VState*				NetStates;
	TArray<VState*>		StatesLookup;
	vint32				NumNetFields;
	TArray<VRepInfo>	RepInfos;

	static TArray<mobjinfo_t>	GMobjInfos;
	static TArray<mobjinfo_t>	GScriptIds;
	static TArray<VName>		GSpriteNames;

	// Constructors.
	VClass(VName AName);
	VClass(ENativeConstructor, size_t ASize, vuint32 AClassFlags,
		VClass *AParent, EName AName, void(*ACtor)());

	// Destructors.
	~VClass();

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

	// Systemwide functions.
	static VClass *FindClass(const char *);
	static int FindSprite(VName);
	static void GetSpriteNames(TArray<FReplacedString>&);
	static void ReplaceSpriteNames(TArray<FReplacedString>&);
	static void StaticReinitStatesLookup();

	// Accessors.
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
	VClass *GetSuperClass() const
	{
		return ParentClass;
	}

	VField* FindField(VName InName);
	VField* FindFieldChecked(VName InName);
	VMethod* FindFunction(VName InName);
	VMethod* FindFunctionChecked(VName InName);
	int GetFunctionIndex(VName InName);
	VState* FindState(VName InName);
	VState* FindStateChecked(VName InName);
	void CopyObject(const vuint8*, vuint8*);
	void SerialiseObject(VStream&, VObject*);
	void CleanObject(VObject*);
	void DestructObject(VObject*);

	void Serialise(VStream&);
	void PostLoad();
	void Shutdown();

private:
	void CalcFieldOffsets();
	void InitNetFields();
	void InitReferences();
	void InitDestructorFields();
	void CreateVTable();
	void InitStatesLookup();
	void CreateDefaults();

public:
	friend inline VStream& operator<<(VStream& Strm, VClass*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

//==========================================================================
//
//	VScriptArray
//
//==========================================================================

class VScriptArray
{
public:
	int Num() const
	{
		return ArrNum;
	}
	vuint8* Ptr()
	{
		return ArrData;
	}
	void Clear(VField::FType& Type);
	void Resize(int NewSize, VField::FType& Type);
	void SetNum(int NewNum, VField::FType& Type);
	void Insert(int Index, int Count, VField::FType& Type);
	void Remove(int Index, int Count, VField::FType& Type);

private:
	int ArrNum;
	int ArrSize;
	vuint8* ArrData;
};
