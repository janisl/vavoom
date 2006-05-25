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

	static bool						GObjInitialized;
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

	static void StaticInit();
	static void StaticExit();
	static VPackage* StaticLoadPackage(VName);
	static VMemberBase* StaticFindMember(VName, VMemberBase*, vuint8);
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
	vint32*			Statements;
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
			vint32		BitMask;
			VClass*		Class;			//  Class of the reference
			VStruct*	Struct;			//  Struct data.
			VMethod*	Function;		//  Function of the delegate type.
		};

		friend VStream& operator<<(VStream&, FType&);
	};

	VField*		Next;
	VField*		NextReference;	//	Linked list of reference fields.
	vint32		Ofs;
	FType		Type;
	VMethod*	Func;
	vint32		Flags;

	VField(VName);

	void Serialise(VStream&);

	static void SerialiseFieldValue(VStream&, byte*, const VField::FType&);
	static void CleanField(byte*, const VField::FType&);

	friend inline VStream& operator<<(VStream& Strm, VField*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

//==========================================================================
//
//	VMethod
//
//==========================================================================

class VMethod : public VMemberBase
{
public:
	vint32		FirstStatement;
	vint16		NumParms;
	vint16		NumLocals;
	vint16		Type;
	vint16		Flags;
	vuint32		Profile1;
	vuint32		Profile2;

	VMethod(VName);

	void Serialise(VStream&);
	void PostLoad();

	friend inline VStream& operator<<(VStream& Strm, VMethod*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
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
	vint32		frame;
	VName		ModelName;
	vint32		ModelIndex;
	vint32		model_frame;
	float		time;
	VState*		nextstate;
	VMethod*	function;
	VState*		Next;

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
	VField*			Fields;
	VField*			ReferenceFields;

	VStruct(VName);

	void Serialise(VStream&);
	void PostLoad();

	void InitReferences();
	void SerialiseObject(VStream&, byte*);
	void CleanObject(byte*);

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
	VState*			States;

	static TArray<mobjinfo_t>	GMobjInfos;
	static TArray<mobjinfo_t>	GScriptIds;
	static TArray<VName>		GSpriteNames;
	static TArray<VName>		GModelNames;

	// Constructors.
	VClass(VName AName);
	VClass(ENativeConstructor, size_t ASize, dword AClassFlags,
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
	VClass *GetSuperClass() const
	{
		return ParentClass;
	}

	VMethod* FindFunction(VName InName);
	VMethod* FindFunctionChecked(VName InName);
	int GetFunctionIndex(VName InName);
	VState* FindState(VName InName);
	VState* FindStateChecked(VName InName);
	void InitReferences();
	void CreateVTable();
	void SerialiseObject(VStream&, VObject*);
	void CleanObject(VObject*);

	void Serialise(VStream&);
	void PostLoad();

	friend inline VStream& operator<<(VStream& Strm, VClass*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};
