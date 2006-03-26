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
	CLASSOF_RefsInitialised	= 0x00000002,	// Reference fields have been set up
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

	//	New and delete operators.
	void* operator new(size_t Size, int Tag)
	{ return Z_Calloc(Size, Tag, 0); }
	void operator delete(void* Object, size_t)
	{ Z_Free(Object); }

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

	vuint16			Checksum;
	char*			Strings;
	int*			Statements;
	VMethod**		VTables;
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
		byte		Type;
		byte		InnerType;		//	For pointers
		byte		ArrayInnerType;	//	For arrays
		byte		PtrLevel;
		int			ArrayDim;
		union
		{
			int			BitMask;
			VClass*		Class;			//  Class of the reference
			VStruct*	Struct;			//  Struct data.
			VMethod*	Function;		//  Function of the delegate type.
		};

		friend VStream& operator<<(VStream&, FType&);
	};

	VField*		Next;
	VField*		NextReference;	//	Linked list of reference fields.
	int			Ofs;
	FType		Type;
	int			Flags;

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
	int		FirstStatement;
	short	NumParms;
	short	NumLocals;
    short	Type;
	short	Flags;
	dword	Profile1;
	dword	Profile2;

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
	int			SpriteIndex;
	int			frame;
	VName		ModelName;
	int			ModelIndex;
	int			model_frame;
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
	int				Size;
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
	int			doomednum;
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
	dword			ObjectFlags;		// Private EObjectFlags used by object manager.
	VClass*			LinkNext;			// Next class in linked list

	friend class VMemberBase;
public:
	VClass*			ParentClass;

	int				ClassSize;
	dword			ClassFlags;
	VMethod**		ClassVTable;
	void			(*ClassConstructor)(void*);

	int				ClassNumMethods;
	int				VTableOffset;

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
		VClass *AParent, EName AName, void(*ACtor)(void*));

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
	void SerialiseObject(VStream&, VObject*);
	void CleanObject(VObject*);

	void Serialise(VStream&);
	void PostLoad();

	friend inline VStream& operator<<(VStream& Strm, VClass*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }
};

//**************************************************************************
//
//	$Log$
//	Revision 1.20  2006/03/26 13:06:18  dj_jl
//	Implemented support for modular progs.
//
//	Revision 1.19  2006/03/23 18:31:59  dj_jl
//	Members tree.
//	
//	Revision 1.18  2006/03/18 16:51:15  dj_jl
//	Renamed type class names, better code serialisation.
//	
//	Revision 1.17  2006/03/13 18:32:45  dj_jl
//	Added function to check if a state is in the range.
//	
//	Revision 1.16  2006/03/12 20:06:02  dj_jl
//	States as objects, added state variable type.
//	
//	Revision 1.15  2006/03/10 19:31:25  dj_jl
//	Use serialisation for progs files.
//	
//	Revision 1.14  2006/03/06 13:02:32  dj_jl
//	Cleaning up references to destroyed objects.
//	
//	Revision 1.13  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//	
//	Revision 1.12  2006/02/26 20:52:49  dj_jl
//	Proper serialisation of level and players.
//	
//	Revision 1.11  2006/02/25 17:09:35  dj_jl
//	Import all progs type info.
//	
//	Revision 1.10  2005/11/24 20:09:23  dj_jl
//	Removed unused fields from Object class.
//	
//	Revision 1.9  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//	
//	Revision 1.8  2003/03/08 12:08:05  dj_jl
//	Beautification.
//	
//	Revision 1.7  2002/05/03 17:06:23  dj_jl
//	Mangling of string pointers.
//	
//	Revision 1.6  2002/03/09 18:05:34  dj_jl
//	Added support for defining native functions outside pr_cmds
//	
//	Revision 1.5  2002/02/26 17:54:26  dj_jl
//	Importing special property info from progs and using it in saving.
//	
//	Revision 1.4  2002/02/02 19:20:41  dj_jl
//	VMethod pointers used instead of the function numbers
//	
//	Revision 1.3  2002/01/11 08:15:40  dj_jl
//	Removed VMethod
//	
//	Revision 1.2  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2001/12/27 17:35:42  dj_jl
//	Split VClass in seperate module
//	
//**************************************************************************
