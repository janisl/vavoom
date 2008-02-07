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

enum ENativeConstructor		{EC_NativeConstructor};

//
// Flags describing an class instance.
//
enum EClassObjectFlags
{
	CLASSOF_Native			= 0x00000001,   // Native
	CLASSOF_PostLoaded		= 0x00000002,	// PostLoad has been called
};

//==========================================================================
//
//	VStateLabel
//
//==========================================================================

struct VStateLabel
{
	//	Persistent fields
	VName		Name;
	VState*		State;

	//	Compiler fields
	TLocation	Loc;
	VName		GotoLabel;
	vint32		GotoOffset;

	VStateLabel()
	: Name(NAME_None)
	, State(NULL)
	, GotoLabel(NAME_None)
	, GotoOffset(0)
	{}

	friend inline VStream& operator<<(VStream& Strm, VStateLabel& Lbl)
	{
		return Strm << Lbl.Name << Lbl.State;
	}
};

//==========================================================================
//
//	VRepField
//
//==========================================================================

struct VRepField
{
	VName				Name;
	TLocation			Loc;
	VMemberBase*		Member;
};

//==========================================================================
//
//	VRepInfo
//
//==========================================================================

struct VRepInfo
{
	bool				Reliable;
	VMethod*			Cond;
	TArray<VRepField>	RepFields;
};

//==========================================================================
//
//	VClass
//
//==========================================================================

class VClass : public VMemberBase
{
public:
	//	Persistent fields
	VClass*					ParentClass;
	VField*					Fields;
	VState*					States;
	VMethod*				DefaultProperties;
	TArray<VRepInfo>		RepInfos;
	TArray<VStateLabel>		StateLabels;

	//	Compiler fields
	VName					ParentClassName;
	TLocation				ParentClassLoc;
	VExpression*			GameExpr;
	VExpression*			MobjInfoExpr;
	VExpression*			ScriptIdExpr;
	TArray<VStruct*>		Structs;
	TArray<VConstant*>		Constants;
	TArray<VProperty*>		Properties;
	TArray<VMethod*>		Methods;
	bool					Defined;

	// Internal per-object variables.
	vuint32					ObjectFlags;		// Private EObjectFlags used by object manager.
	VClass*					LinkNext;			// Next class in linked list

	vint32					ClassSize;
	vint32					ClassUnalignedSize;
	vuint32					ClassFlags;
	VMethod**				ClassVTable;
	void					(*ClassConstructor)();

	vint32					ClassNumMethods;

	VField*					ReferenceFields;
	VField*					DestructorFields;
	VField*					NetFields;
	VMethod*				NetMethods;
	vint32					NetId;
	VState*					NetStates;
	TArray<VState*>			StatesLookup;
	vint32					NumNetFields;

	vuint8*					Defaults;

	VClass*					Replacement;
	VClass*					Replacee;

	static TArray<mobjinfo_t>	GMobjInfos;
	static TArray<mobjinfo_t>	GScriptIds;
	static TArray<VName>		GSpriteNames;

	//	Structors.
	VClass(VName, VMemberBase*, TLocation);
	VClass(ENativeConstructor, size_t ASize, vuint32 AClassFlags,
		VClass *AParent, EName AName, void(*ACtor)());
	~VClass();

	// Systemwide functions.
	static VClass *FindClass(const char *);
	static int FindSprite(VName);
#ifndef IN_VCC
	static void GetSpriteNames(TArray<FReplacedString>&);
	static void ReplaceSpriteNames(TArray<FReplacedString>&);
#endif
	static void StaticReinitStatesLookup();

	void Serialise(VStream&);
	void PostLoad();
	void Shutdown();

	void AddConstant(VConstant*);
	void AddField(VField*);
	void AddProperty(VProperty*);
	void AddState(VState*);
	void AddMethod(VMethod*);

	VConstant* FindConstant(VName);
	VField* FindField(VName);
	VField* FindField(VName, TLocation, VClass*);
	VField* FindFieldChecked(VName);
	VProperty* FindProperty(VName);
	VMethod* FindMethod(VName, bool = true);
	VMethod* FindMethodChecked(VName);
	int GetMethodIndex(VName);
	VState* FindState(VName);
	VState* FindStateChecked(VName);
	VStateLabel* FindStateLabel(VName, bool = true);
	VStateLabel* FindStateLabelChecked(VName);

	bool Define();
	bool DefineMembers();
	void Emit();
	VState* ResolveStateLabel(TLocation, VName, int);
	void SetStateLabel(VName, VState*);

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

	// Accessors.
	VClass *GetSuperClass() const
	{
		return ParentClass;
	}

#ifndef IN_VCC
	void CopyObject(const vuint8*, vuint8*);
	void SerialiseObject(VStream&, VObject*);
	void CleanObject(VObject*);
	void DestructObject(VObject*);
	VClass* CreateDerivedClass(VName, VMemberBase*, TLocation);
#endif
	VClass* GetReplacement();
	VClass* GetReplacee();

private:
	void CalcFieldOffsets();
	void InitNetFields();
	void InitReferences();
	void InitDestructorFields();
	void CreateVTable();
	void InitStatesLookup();
#ifndef IN_VCC
	void CreateDefaults();
#endif

public:
	friend inline VStream& operator<<(VStream& Strm, VClass*& Obj)
	{ return Strm << *(VMemberBase**)&Obj; }

	friend class VMemberBase;
};
