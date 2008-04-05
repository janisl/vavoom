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
// Flags describing a class.
//
enum EClassFlags
{
	// Base flags.
	CLASS_Native				= 0x0001,
	CLASS_Abstract				= 0x0002,	//	Class is abstract and can't be instantiated directly.
	CLASS_SkipSuperStateLabels	= 0x0004,	//	Don't copy state labels
};

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
	VName					Name;
	VState*					State;
	TArray<VStateLabel>		SubLabels;

	VStateLabel()
	: Name(NAME_None)
	, State(NULL)
	{}

	friend VStream& operator<<(VStream&, VStateLabel&);
};

//==========================================================================
//
//	VStateLabelDef
//
//==========================================================================

struct VStateLabelDef
{
	VStr		Name;
	VState*		State;
	TLocation	Loc;
	VName		GotoLabel;
	vint32		GotoOffset;

	VStateLabelDef()
	: State(NULL)
	, GotoLabel(NAME_None)
	, GotoOffset(0)
	{}
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
//	VDecorateStateAction
//
//==========================================================================

struct VDecorateStateAction
{
	VName			Name;
	VMethod*		Method;
};

//==========================================================================
//
//	VLightEffectDef
//
//==========================================================================

struct VLightEffectDef
{
	VName				Name;
	vuint8				Type;
	vint32				Colour;
	float				Radius;
	float				Radius2;
	float				MinLight;
	TVec				Offset;
};

//==========================================================================
//
//	VParticleEffectDef
//
//==========================================================================

struct VParticleEffectDef
{
	VName				Name;
	vuint8				Type;
	vuint8				Type2;
	vint32				Colour;
	TVec				Offset;
	vint32				Count;
	float				OrgRnd;
	TVec				Velocity;
	float				VelRnd;
	float				Accel;
	float				Grav;
	float				Duration;
	float				Ramp;
};

//==========================================================================
//
//	VSpriteEffect
//
//==========================================================================

struct VSpriteEffect
{
	vint32				SpriteIndex;
	vint8				Frame;
	VLightEffectDef*	LightDef;
	VParticleEffectDef*	PartDef;
};

//==========================================================================
//
//	VClass
//
//==========================================================================

class VClass : public VMemberBase
{
public:
	enum { LOWER_CASE_HASH_SIZE = 8 * 1024 };

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
	TArray<VStateLabelDef>	StateLabelDefs;
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

	TArray<VDecorateStateAction>	DecorateStateActions;

	TArray<VSpriteEffect>	SpriteEffects;

	VName					LowerCaseName;
	VClass*					LowerCaseHashNext;

	static TArray<mobjinfo_t>	GMobjInfos;
	static TArray<mobjinfo_t>	GScriptIds;
	static TArray<VName>		GSpriteNames;
	static VClass*				GLowerCaseHashTable[LOWER_CASE_HASH_SIZE];

	//	Structors.
	VClass(VName, VMemberBase*, TLocation);
	VClass(ENativeConstructor, size_t ASize, vuint32 AClassFlags,
		VClass *AParent, EName AName, void(*ACtor)());
	~VClass();

	// Systemwide functions.
	static VClass* FindClass(const char *);
	static VClass* FindClassNoCase(const char *);
	static VClass* FindClassLowerCase(VName);
	static int FindSprite(VName, bool = true);
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
	VStateLabel* FindStateLabel(VName, VName = NAME_None, bool = false);
	VStateLabel* FindStateLabel(TArray<VName>&, bool);
	VStateLabel* FindStateLabelChecked(VName, VName = NAME_None, bool = false);
	VStateLabel* FindStateLabelChecked(TArray<VName>&, bool);
	VDecorateStateAction* FindDecorateStateAction(VName);

	bool Define();
	bool DefineMembers();
	void Emit();
	void DecorateEmit();
	void DecoratePostLoad();
	void EmitStateLabels();
	VState* ResolveStateLabel(TLocation, VName, int);
	void SetStateLabel(VName, VState*);
	void SetStateLabel(const TArray<VName>&, VState*);

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
	void HashLowerCased();

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
