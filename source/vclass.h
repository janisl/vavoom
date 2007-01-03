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
	vint32		Ofs;
	FType		Type;
	VMethod*	Func;
	vint32		Flags;

	VField(VName);

	void Serialise(VStream&);

	static void SerialiseFieldValue(VStream&, byte*, const VField::FType&);
	static void CleanField(byte*, const VField::FType&);
	static void DestructField(byte*, const VField::FType&);

	friend inline VStream& operator<<(VStream& Strm, VField*& Obj)
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
	vint16			NumParms;
	vint16			NumLocals;
	vint16			Type;
	vint16			Flags;
	vuint32			Profile1;
	vuint32			Profile2;
	TArray<vuint8>	Statements;
	builtin_t		NativeFunc;
	vint32			NumInstructions;
	FInstruction*	Instructions;
	vint16			VTableIndex;

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
	VName		ModelName;
	vint32		ModelIndex;
	vint32		ModelFrame;
	float		Time;
	vint32		Misc1;
	vint32		Misc2;
	VState*		NextState;
	VMethod*	Function;
	VState*		Next;

	vint32		InClassIndex;
	vint32		NetId;

	VState(VName);

	void Serialise(VStream&);

	bool IsInRange(VState*, VState*, int);

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
	vint32		Value;

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
	void SerialiseObject(VStream&, byte*);
	void CleanObject(byte*);
	void DestructObject(byte*);

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
	VClass*		class_id;

	friend VStream& operator<<(VStream&, mobjinfo_t&);
};

//==========================================================================
//
//	VClass
//
//==========================================================================

class VClass : public VMemberBase
{
private:
	// Internal per-object variables.
	vuint32			ObjectFlags;		// Private EObjectFlags used by object manager.
	VClass*			LinkNext;			// Next class in linked list

	friend class VMemberBase;
public:
	VClass*			ParentClass;

	vint32			ClassSize;
	vuint32			ClassFlags;
	VMethod**		ClassVTable;
	void			(*ClassConstructor)();

	vint32			ClassNumMethods;

	VField*			Fields;
	VField*			ReferenceFields;
	VField*			DestructorFields;
	VState*			States;
	VMethod*		DefaultProperties;

	vint32			NetId;
	TArray<VState*>	StatesLookup;

	static TArray<mobjinfo_t>	GMobjInfos;
	static TArray<mobjinfo_t>	GScriptIds;
	static TArray<VName>		GSpriteNames;
	static TArray<VName>		GModelNames;

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
	static int FindModel(VName);

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

	VMethod* FindFunction(VName InName);
	VMethod* FindFunctionChecked(VName InName);
	int GetFunctionIndex(VName InName);
	VState* FindState(VName InName);
	VState* FindStateChecked(VName InName);
	void CalcFieldOffsets();
	void InitReferences();
	void InitDestructorFields();
	void CreateVTable();
	void SerialiseObject(VStream&, VObject*);
	void CleanObject(VObject*);
	void DestructObject(VObject*);

	void Serialise(VStream&);
	void PostLoad();
	void Shutdown();

	friend inline VStream& operator<<(VStream& Strm, VClass*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};
