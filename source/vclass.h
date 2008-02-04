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
	VField*		Next;
	VField*		NextReference;	//	Linked list of reference fields.
	VField*		DestructorLink;
	VField*		NextNetField;
	vint32		Ofs;
	VFieldType	Type;
	VMethod*	Func;
	vint32		Flags;
	vint32		NetIndex;
	VMethod*	ReplCond;

	VField(VName, VMemberBase*, TLocation);

	void Serialise(VStream&);

	bool NeedsDestructor() const;

	static void CopyFieldValue(const vuint8*, vuint8*, const VFieldType&);
	static void SerialiseFieldValue(VStream&, vuint8*, const VFieldType&);
	static void CleanField(vuint8*, const VFieldType&);
	static void DestructField(vuint8*, const VFieldType&);
	static bool IdenticalValue(const vuint8*, const vuint8*, const VFieldType&);
	static bool NetSerialiseValue(VStream&, VNetObjectsMap*, vuint8*, const VFieldType&);

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
	VFieldType		Type;
	VMethod*		GetFunc;
	VMethod*		SetFunc;
	VField*			DefaultField;
	vuint32			Flags;

	VProperty(VName, VMemberBase*, TLocation);

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
	VFieldType		TypeArg;
};

class VMethod : public VMemberBase
{
public:
	//FIXME avoid copying
	enum { MAX_PARAMS		= 16 };

	vint32					NumLocals;
	vint32					Flags;
	VFieldType				ReturnType;
	vint32					NumParams;
	vint32					ParamsSize;
	VFieldType				ParamTypes[MAX_PARAMS];
	vuint8					ParamFlags[MAX_PARAMS];
	TArray<FInstruction>	Instructions;
	VMethod*				SuperMethod;
	VMethod*				ReplCond;

	vuint32					Profile1;
	vuint32					Profile2;
	TArray<vuint8>			Statements;
	builtin_t				NativeFunc;
	vint16					VTableIndex;
	vint32					NetIndex;
	VMethod*				NextNetMethod;

	VMethod(VName, VMemberBase*, TLocation);
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

	//	Compile time variables
	VName		GotoLabel;
	vint32		GotoOffset;

	VState(VName, VMemberBase*, TLocation);

	void Serialise(VStream&);

	bool IsInRange(VState*, VState*, int);
	bool IsInSequence(VState*);
	VState* GetPlus(int, bool);

	friend inline VStream& operator<<(VStream& Strm, VState*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

//==========================================================================
//
//	VStateLabel
//
//==========================================================================

struct VStateLabel
{
	VName		Name;
	VState*		State;

	VName		GotoLabel;
	vint32		GotoOffset;

	friend VStream& operator<<(VStream& Strm, VStateLabel& Lbl)
	{
		return Strm << Lbl.Name << Lbl.State;
	}
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

	VConstant(VName, VMemberBase*, TLocation);

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
	//	Size in stack units when used as local variable.
	vint32			StackSize;
	VField*			Fields;
	VField*			ReferenceFields;
	VField*			DestructorFields;

	VStruct(VName, VMemberBase*, TLocation);

	void Serialise(VStream&);
	void PostLoad();

	bool NeedsDestructor() const;
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
	TArray<VStateLabel>	StateLabels;

	VClass*			Replacement;
	VClass*			Replacee;

	static TArray<mobjinfo_t>	GMobjInfos;
	static TArray<mobjinfo_t>	GScriptIds;
	static TArray<VName>		GSpriteNames;

	// Constructors.
	VClass(VName, VMemberBase*, TLocation);
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

	VField* FindField(VName);
	VField* FindFieldChecked(VName);
	VMethod* FindFunction(VName);
	VMethod* FindFunctionChecked(VName);
	int GetFunctionIndex(VName);
	VState* FindState(VName);
	VState* FindStateChecked(VName);
	VState* FindStateLabel(VName);
	VState* FindStateLabelChecked(VName);
	void SetStateLabel(VName, VState*);
	void CopyObject(const vuint8*, vuint8*);
	void SerialiseObject(VStream&, VObject*);
	void CleanObject(VObject*);
	void DestructObject(VObject*);
	VClass* CreateDerivedClass(VName, VMemberBase*, TLocation);
	VClass* GetReplacement();
	VClass* GetReplacee();

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
