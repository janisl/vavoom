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

// HEADER FILES ------------------------------------------------------------

#ifdef IN_VCC
#include "../utils/vcc/vcc.h"
#else
#include "vc_local.h"
#endif

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class DummyClass1
{
public:
	void*		Pointer;
	vuint8		Byte1;
	virtual ~DummyClass1() {}
	virtual void Dummy() = 0;
};

class DummyClass2 : public DummyClass1
{
public:
	vuint8		Byte2;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

TArray<mobjinfo_t>		VClass::GMobjInfos;
TArray<mobjinfo_t>		VClass::GScriptIds;
TArray<VName>			VClass::GSpriteNames;
VClass*					VClass::GLowerCaseHashTable[VClass::LOWER_CASE_HASH_SIZE];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VClass::VClass
//
//==========================================================================

VClass::VClass(VName AName, VMemberBase* AOuter, TLocation ALoc)
: VMemberBase(MEMBER_Class, AName, AOuter, ALoc)
, ParentClass(NULL)
, Fields(NULL)
, States(NULL)
, DefaultProperties(NULL)
, ParentClassName(NAME_None)
, GameExpr(NULL)
, MobjInfoExpr(NULL)
, ScriptIdExpr(NULL)
, Defined(true)
, ObjectFlags(0)
, LinkNext(NULL)
, ClassSize(0)
, ClassUnalignedSize(0)
, ClassFlags(0)
, ClassVTable(NULL)
, ClassConstructor(NULL)
, ClassNumMethods(0)
, ReferenceFields(NULL)
, DestructorFields(NULL)
, NetFields(NULL)
, NetMethods(NULL)
, Defaults(NULL)
, NetId(-1)
, NetStates(NULL)
, NumNetFields(0)
, Replacement(NULL)
, Replacee(NULL)
{
	guard(VClass::VClass);
	LinkNext = GClasses;
	GClasses = this;
	HashLowerCased();
	unguard;
}

//==========================================================================
//
//	VClass::VClass
//
//==========================================================================

VClass::VClass(ENativeConstructor, size_t ASize, vuint32 AClassFlags, 
	VClass *AParent, EName AName, void(*ACtor)())
: VMemberBase(MEMBER_Class, AName, NULL, TLocation())
, ParentClass(AParent)
, Fields(NULL)
, States(NULL)
, DefaultProperties(NULL)
, ParentClassName(NAME_None)
, GameExpr(NULL)
, MobjInfoExpr(NULL)
, ScriptIdExpr(NULL)
, Defined(true)
, ObjectFlags(CLASSOF_Native)
, LinkNext(NULL)
, ClassSize(ASize)
, ClassUnalignedSize(ASize)
, ClassFlags(AClassFlags)
, ClassVTable(NULL)
, ClassConstructor(ACtor)
, ClassNumMethods(0)
, ReferenceFields(NULL)
, DestructorFields(NULL)
, NetFields(NULL)
, NetMethods(NULL)
, NetId(-1)
, NetStates(NULL)
, NumNetFields(0)
, Defaults(NULL)
, Replacement(NULL)
, Replacee(NULL)
{
	guard(native VClass::VClass);
	LinkNext = GClasses;
	GClasses = this;
	unguard;
}

//==========================================================================
//
//	VClass::~VClass
//
//==========================================================================

VClass::~VClass()
{
	guard(VClass::~VClass);
	if (GameExpr)
	{
		delete GameExpr;
	}
	if (MobjInfoExpr)
	{
		delete MobjInfoExpr;
	}
	if (ScriptIdExpr)
	{
		delete ScriptIdExpr;
	}

	if (ClassVTable)
	{
		delete[] ClassVTable;
	}
#ifndef IN_VCC
	if (Defaults)
	{
		DestructObject((VObject*)Defaults);
		delete[] Defaults;
	}
#endif

	if (!GObjInitialised)
	{
		return;
	}
	//	Unlink from classes list.
	if (GClasses == this)
	{
		GClasses = LinkNext;
	}
	else
	{
		VClass* Prev = GClasses;
		while (Prev && Prev->LinkNext != this)
		{
			Prev = Prev->LinkNext;
		}
		if (Prev)
		{
			Prev->LinkNext = LinkNext;
		}
		else
		{
#ifndef IN_VCC
			GCon->Log(NAME_Dev, "VClass Unlink: Class not in list");
#endif
		}
	}
	unguard;
}

//==========================================================================
//
//	VClass::FindClass
//
//==========================================================================

VClass *VClass::FindClass(const char *AName)
{
	guard(VClass::FindClass);
	VName TempName(AName, VName::Find);
	if (TempName == NAME_None)
	{
		// No such name, no chance to find a class
		return NULL;
	}
	for (VClass* Cls = GClasses; Cls; Cls = Cls->LinkNext)
	{
		if (Cls->GetVName() == TempName && Cls->MemberType == MEMBER_Class)
		{
			return Cls;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VClass::FindClassNoCase
//
//==========================================================================

VClass *VClass::FindClassNoCase(const char *AName)
{
	guard(VClass::FindClassNoCase);
	for (VClass* Cls = GClasses; Cls; Cls = Cls->LinkNext)
	{
		if (Cls->MemberType == MEMBER_Class &&
			!VStr::ICmp(Cls->GetName(), AName))
		{
			return Cls;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VClass::FindSprite
//
//==========================================================================

int VClass::FindSprite(VName Name, bool Append)
{
	guard(VClass::FindSprite);
	for (int i = 0; i < GSpriteNames.Num(); i++)
	{
		if (GSpriteNames[i] == Name)
		{
			return i;
		}
	}
	if (!Append)
	{
		return -1;
	}
	return GSpriteNames.Append(Name);
	unguard;
}

#ifndef IN_VCC

//==========================================================================
//
//	VClass::GetSpriteNames
//
//==========================================================================

void VClass::GetSpriteNames(TArray<FReplacedString>& List)
{
	guard(VClass::GetSpriteNames);
	for (int i = 0; i < GSpriteNames.Num(); i++)
	{
		FReplacedString&R = List.Alloc();
		R.Index = i;
		R.Replaced = false;
		R.Old = VStr(*GSpriteNames[i]).ToUpper();
	}
	unguard;
}

//==========================================================================
//
//	VClass::ReplaceSpriteNames
//
//==========================================================================

void VClass::ReplaceSpriteNames(TArray<FReplacedString>& List)
{
	guard(VClass::ReplaceSpriteNames);
	for (int i = 0; i < List.Num(); i++)
	{
		if (!List[i].Replaced)
		{
			continue;
		}
		GSpriteNames[List[i].Index] = *List[i].New.ToLower();
	}

	//	Update sprite names in states.
	for (int i = 0; i < VMemberBase::GMembers.Num(); i++)
	{
		if (GMembers[i] && GMembers[i]->MemberType == MEMBER_State)
		{
			VState* S = (VState*)GMembers[i];
			S->SpriteName = GSpriteNames[S->SpriteIndex];
		}
	}
	unguard;
}

#endif

//==========================================================================
//
//	VClass::StaticReinitStatesLookup
//
//==========================================================================

void VClass::StaticReinitStatesLookup()
{
	guard(VClass::StaticReinitStatesLookup);
	//	Clear states lookup tables.
	for (VClass* C = GClasses; C; C = C->LinkNext)
	{
		C->StatesLookup.Clear();
	}

	//	Now init states lookup tables again.
	for (VClass* C = GClasses; C; C = C->LinkNext)
	{
		C->InitStatesLookup();
	}
	unguard;
}

//==========================================================================
//
//	VClass::Serialise
//
//==========================================================================

void VClass::Serialise(VStream& Strm)
{
	guard(VClass::Serialise);
	VMemberBase::Serialise(Strm);
#ifndef IN_VCC
	VClass* PrevParent = ParentClass;
#endif
	Strm << ParentClass
		<< Fields
		<< States
		<< DefaultProperties;
#ifndef IN_VCC
	if ((ObjectFlags & CLASSOF_Native) && ParentClass != PrevParent)
	{
		Sys_Error("Bad parent class, class %s, C++ %s, VavoomC %s)",
			GetName(), PrevParent ? PrevParent->GetName() : "(none)",
			ParentClass ? ParentClass->GetName() : "(none)");
	}
	if (Strm.IsLoading())
	{
		NetStates = States;
	}
#endif

	int NumRepInfos = RepInfos.Num();
	Strm << STRM_INDEX(NumRepInfos);
	if (Strm.IsLoading())
	{
		RepInfos.SetNum(NumRepInfos);
	}
	for (int i = 0; i < RepInfos.Num(); i++)
	{
		Strm << RepInfos[i].Cond;
		int NumRepFields = RepInfos[i].RepFields.Num();
		Strm << STRM_INDEX(NumRepFields);
		if (Strm.IsLoading())
		{
			RepInfos[i].RepFields.SetNum(NumRepFields);
		}
		for (int j = 0; j < RepInfos[i].RepFields.Num(); j++)
		{
			Strm << RepInfos[i].RepFields[j].Member;
		}
	}

	int NumStateLabels = StateLabels.Num();
	Strm << STRM_INDEX(NumStateLabels);
	if (Strm.IsLoading())
	{
		StateLabels.SetNum(NumStateLabels);
	}
	for (int i = 0; i < StateLabels.Num(); i++)
	{
		Strm << StateLabels[i];
	}
	unguard;
}

//==========================================================================
//
//	VClass::Shutdown
//
//==========================================================================

void VClass::Shutdown()
{
	guard(VClass::Shutdown);
	if (ClassVTable)
	{
		delete[] ClassVTable;
		ClassVTable = NULL;
	}
#ifndef IN_VCC
	if (Defaults)
	{
		DestructObject((VObject*)Defaults);
		delete[] Defaults;
		Defaults = NULL;
	}
#endif
	StatesLookup.Clear();
	RepInfos.Clear();
	SpriteEffects.Clear();
	unguard;
}

//==========================================================================
//
//	VClass::AddConstant
//
//==========================================================================

void VClass::AddConstant(VConstant* c)
{
	guard(VClass::AddConstant);
	Constants.Append(c);
	unguard;
}

//==========================================================================
//
//	VClass::AddField
//
//==========================================================================

void VClass::AddField(VField* f)
{
	guard(VClass::AddField);
	if (!Fields)
	{
		Fields = f;
	}
	else
	{
		VField* Prev = Fields;
		while (Prev->Next)
		{
			Prev = Prev->Next;
		}
		Prev->Next = f;
	}
	f->Next = NULL;
	unguard;
}

//==========================================================================
//
//	VClass::AddProperty
//
//==========================================================================

void VClass::AddProperty(VProperty* p)
{
	guard(VClass::AddProperty);
	Properties.Append(p);
	unguard;
}

//==========================================================================
//
//	VClass::AddState
//
//==========================================================================

void VClass::AddState(VState* s)
{
	guard(VClass::AddState);
	if (!States)
	{
		States = s;
	}
	else
	{
		VState* Prev = States;
		while (Prev->Next)
		{
			Prev = Prev->Next;
		}
		Prev->Next = s;
	}
	s->Next = NULL;
	unguard;
}

//==========================================================================
//
//	VClass::AddMethod
//
//==========================================================================

void VClass::AddMethod(VMethod* m)
{
	guard(VClass::AddMethod);
	Methods.Append(m);
	unguard;
}

//==========================================================================
//
//	VClass::FindConstant
//
//==========================================================================

VConstant* VClass::FindConstant(VName Name)
{
	guard(VClass::FindConstant);
	VMemberBase* m = StaticFindMember(Name, this, MEMBER_Const);
	if (m)
	{
		return (VConstant*)m;
	}
	if (ParentClass)
	{
		return ParentClass->FindConstant(Name);
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VClass::FindField
//
//==========================================================================

VField* VClass::FindField(VName Name)
{
	guard(VClass::FindField);
	if (Name == NAME_None)
	{
		return NULL;
	}
	for (VField *F = Fields; F; F = F->Next)
	{
		if (Name == F->Name)
		{
			return F;
		}
	}
	if (ParentClass)
	{
		return ParentClass->FindField(Name);
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VClass::FindField
//
//==========================================================================

VField* VClass::FindField(VName Name, TLocation l, VClass* SelfClass)
{
	guard(VClass::FindField);
	VField* F = FindField(Name);
	if (F && (F->Flags & FIELD_Private) && this != SelfClass)
	{
		ParseError(l, "Field %s is private", *F->Name);
	}
	return F;
	unguard;
}

//==========================================================================
//
//	VClass::FindFieldChecked
//
//==========================================================================

VField* VClass::FindFieldChecked(VName AName)
{
	guard(VClass::FindFieldChecked);
	VField* F = FindField(AName);
	if (!F)
	{
		Sys_Error("Field %s not found", *AName);
	}
	return F;
	unguard;
}

//==========================================================================
//
//	VClass::FindProperty
//
//==========================================================================

VProperty* VClass::FindProperty(VName Name)
{
	guard(VClass::FindProperty);
	if (Name == NAME_None)
	{
		return NULL;
	}
	VProperty* P = (VProperty*)StaticFindMember(Name, this, MEMBER_Property);
	if (P)
	{
		return P;
	}
	if (ParentClass)
	{
		return ParentClass->FindProperty(Name);
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VClass::FindMethod
//
//==========================================================================

VMethod* VClass::FindMethod(VName Name, bool bRecursive)
{
	guard(VClass::FindMethod);
	if (Name == NAME_None)
	{
		return NULL;
	}
	VMethod* M = (VMethod*)StaticFindMember(Name, this, MEMBER_Method);
	if (M)
	{
		return M;
	}
	if (bRecursive && ParentClass)
	{
		return ParentClass->FindMethod(Name);
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VClass::FindMethodChecked
//
//==========================================================================

VMethod *VClass::FindMethodChecked(VName AName)
{
	guard(VClass::FindMethodChecked);
	VMethod *func = FindMethod(AName);
	if (!func)
	{
		Sys_Error("Function %s not found", *AName);
	}
	return func;
	unguard;
}

//==========================================================================
//
//	VClass::GetMethodIndex
//
//==========================================================================

int VClass::GetMethodIndex(VName AName)
{
	guard(VClass::GetMethodIndex);
	for (int i = 0; i < ClassNumMethods; i++)
	{
		if (ClassVTable[i]->Name == AName)
		{
			return i;
		}
	}
	return -1;
	unguard;
}

//==========================================================================
//
//	VClass::FindState
//
//==========================================================================

VState* VClass::FindState(VName AName)
{
	guard(VClass::FindState);
	for (VState* s = States; s; s = s->Next)
	{
		if (s->Name == AName)
		{
			return s;
		}
	}
	if (ParentClass)
	{
		return ParentClass->FindState(AName);
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VClass::FindStateChecked
//
//==========================================================================

VState* VClass::FindStateChecked(VName AName)
{
	guard(VClass::FindStateChecked);
	VState* s = FindState(AName);
	if (!s)
	{
		Sys_Error("State %s not found", *AName);
	}
	return s;
	unguard;
}

//==========================================================================
//
//	VClass::FindStateLabel
//
//==========================================================================

VStateLabel* VClass::FindStateLabel(VName AName, VName SubLabel, bool Exact)
{
	guard(VClass::FindStateLabel);
	for (int i = 0; i < StateLabels.Num(); i++)
	{
		if (!VStr::ICmp(*StateLabels[i].Name, *AName))
		{
			if (SubLabel != NAME_None)
			{
				TArray<VStateLabel>& SubList = StateLabels[i].SubLabels;
				for (int j = 0; j < SubList.Num(); j++)
				{
					if (!VStr::ICmp(*SubList[j].Name, *SubLabel))
					{
						return &SubList[j];
					}
				}
				if (Exact)
				{
					return NULL;
				}
			}
			return &StateLabels[i];
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VClass::FindStateLabel
//
//==========================================================================

VStateLabel* VClass::FindStateLabel(TArray<VName>& Names, bool Exact)
{
	guard(VClass::FindStateLabel);
	TArray<VStateLabel>* List = &StateLabels;
	VStateLabel* Best = NULL;
	for (int ni = 0; ni < Names.Num(); ni++)
	{
		VStateLabel* Lbl = NULL;
		for (int i = 0; i < List->Num(); i++)
		{
			if (!VStr::ICmp(*(*List)[i].Name, *Names[ni]))
			{
				Lbl = &(*List)[i];
				break;
			}
		}
		if (!Lbl)
		{
			if (Exact)
			{
				return NULL;
			}
			break;
		}
		else
		{
			Best = Lbl;
			List = &Lbl->SubLabels;
		}
	}
	return Best;
	unguard;
}

//==========================================================================
//
//	VClass::FindStateLabelChecked
//
//==========================================================================

VStateLabel* VClass::FindStateLabelChecked(VName AName, VName SubLabel,
	bool Exact)
{
	guard(VClass::FindStateLabelChecked);
	VStateLabel* Lbl = FindStateLabel(AName, SubLabel, Exact);
	if (!Lbl)
	{
		VStr FullName = *AName;
		if (SubLabel != NAME_None)
		{
			FullName += ".";
			FullName += *SubLabel;
		}
		Sys_Error("State %s not found", *FullName);
	}
	return Lbl;
	unguard;
}

//==========================================================================
//
//	VClass::FindStateLabelChecked
//
//==========================================================================

VStateLabel* VClass::FindStateLabelChecked(TArray<VName>& Names, bool Exact)
{
	guard(VClass::FindStateLabelChecked);
	VStateLabel* Lbl = FindStateLabel(Names, Exact);
	if (!Lbl)
	{
		VStr FullName = *Names[0];
		for (int i = 1; i < Names.Num(); i++)
		{
			FullName += ".";
			FullName += *Names[i];
		}
		Sys_Error("State %s not found", *FullName);
	}
	return Lbl;
	unguard;
}

//==========================================================================
//
//	VClass::FindDecorateStateAction
//
//==========================================================================

VDecorateStateAction* VClass::FindDecorateStateAction(VName ActName)
{
	guard(VClass::FindDecorateStateAction);
	for (int i = 0; i < DecorateStateActions.Num(); i++)
	{
		if (DecorateStateActions[i].Name == ActName)
		{
			return &DecorateStateActions[i];
		}
	}
	if (ParentClass)
	{
		return ParentClass->FindDecorateStateAction(ActName);
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VClass::Define
//
//==========================================================================

bool VClass::Define()
{
	guard(VClass::Define);
	if (ParentClassName != NAME_None)
	{
		ParentClass = StaticFindClass(ParentClassName);
		if (!ParentClass)
		{
			ParseError(ParentClassLoc, "No such class %s", *ParentClassName);
		}
		else if (!ParentClass->Defined)
		{
			ParseError(ParentClassLoc, "Parent class must be defined before");
		}
	}

	for (int i = 0; i < Structs.Num(); i++)
	{
		if (!Structs[i]->Define())
		{
			return false;
		}
	}

	int GameFilter = 0;
	if (GameExpr)
	{
		VEmitContext ec(this);
		GameExpr = GameExpr->Resolve(ec);
		if (!GameExpr)
		{
			return false;
		}
		if (!GameExpr->IsIntConst())
		{
			ParseError(GameExpr->Loc, "Integer constant expected");
			return false;
		}
		GameFilter = GameExpr->GetIntConst();
	}

	if (MobjInfoExpr)
	{
		VEmitContext ec(this);
		MobjInfoExpr = MobjInfoExpr->Resolve(ec);
		if (!MobjInfoExpr)
		{
			return false;
		}
		if (!MobjInfoExpr->IsIntConst())
		{
			ParseError(MobjInfoExpr->Loc, "Integer constant expected");
			return false;
		}
		mobjinfo_t& mi = ec.Package->MobjInfo.Alloc();
		mi.DoomEdNum = MobjInfoExpr->GetIntConst();
		mi.GameFilter = GameFilter;
		mi.Class = this;
	}

	if (ScriptIdExpr)
	{
		VEmitContext ec(this);
		ScriptIdExpr = ScriptIdExpr->Resolve(ec);
		if (!ScriptIdExpr)
		{
			return false;
		}
		if (!ScriptIdExpr->IsIntConst())
		{
			ParseError(ScriptIdExpr->Loc, "Integer constant expected");
			return false;
		}
		mobjinfo_t& mi = ec.Package->ScriptIds.Alloc();
		mi.DoomEdNum = ScriptIdExpr->GetIntConst();
		mi.GameFilter = GameFilter;
		mi.Class = this;
	}

	Defined = true;
	return true;
	unguard;
}

//==========================================================================
//
//	VClass::DefineMembers
//
//==========================================================================

bool VClass::DefineMembers()
{
	guard(VClass::DefineMembers);
	bool Ret = true;

	for (int i = 0; i < Constants.Num(); i++)
	{
		if (!Constants[i]->Define())
		{
			Ret = false;
		}
	}

	for (int i = 0; i < Structs.Num(); i++)
	{
		Structs[i]->DefineMembers();
	}

	VField* PrevBool = NULL;
	for (VField* fi = Fields; fi; fi = fi->Next)
	{
		if (!fi->Define())
		{
			Ret = false;
		}
		if (fi->Type.Type == TYPE_Bool && PrevBool && PrevBool->Type.BitMask != 0x80000000)
		{
			fi->Type.BitMask = PrevBool->Type.BitMask << 1;
		}
		PrevBool = fi->Type.Type == TYPE_Bool ? fi : NULL;
	}

	for (int i = 0; i < Properties.Num(); i++)
	{
		if (!Properties[i]->Define())
		{
			Ret = false;
		}
	}

	for (int i = 0; i < Methods.Num(); i++)
	{
		if (!Methods[i]->Define())
		{
			Ret = false;
		}
	}

	if (!DefaultProperties->Define())
	{
		Ret = false;
	}

	for (VState* s = States; s; s = s->Next)
	{
		if (!s->Define())
		{
			Ret = false;
		}
	}

	for (int ri = 0; ri < RepInfos.Num(); ri++)
	{
		if (!RepInfos[ri].Cond->Define())
		{
			Ret = false;
		}
		TArray<VRepField>& RepFields = RepInfos[ri].RepFields;
		for (int i = 0; i < RepFields.Num(); i++)
		{
			VField* RepField = NULL;
			for (VField* F = Fields; F; F = F->Next)
			{
				if (F->Name == RepFields[i].Name)
				{
					RepField = F;
					break;
				}
			}
			if (RepField)
			{
				if (RepField->Flags & FIELD_Net)
				{
					ParseError(RepFields[i].Loc, "Field %s has multiple replication statements",
						*RepFields[i].Name);
					continue;
				}
				RepField->Flags |= FIELD_Net;
				RepField->ReplCond = RepInfos[ri].Cond;
				RepFields[i].Member = RepField;
				continue;
			}

			VMethod* RepMethod = NULL;
			for (int mi = 0; mi < Methods.Num(); mi++)
			{
				if (Methods[mi]->Name == RepFields[i].Name)
				{
					RepMethod = Methods[mi];
					break;
				}
			}
			if (RepMethod)
			{
				if (RepMethod->SuperMethod)
				{
					ParseError(RepFields[i].Loc, "Method %s is overloaded in this class",
						*RepFields[i].Name);
					continue;
				}
				if (RepMethod->Flags & FUNC_Net)
				{
					ParseError(RepFields[i].Loc, "Method %s has multiple replication statements",
						*RepFields[i].Name);
					continue;
				}
				RepMethod->Flags |= FUNC_Net;
				RepMethod->ReplCond = RepInfos[ri].Cond;
				if (RepInfos[ri].Reliable)
					RepMethod->Flags |= FUNC_NetReliable;
				RepFields[i].Member = RepMethod;
				continue;
			}

			ParseError(RepFields[i].Loc, "No such field or method %s", *RepFields[i].Name);
		}
	}

	return Ret;
	unguard;
}

//==========================================================================
//
//	VClass::Emit
//
//==========================================================================

void VClass::Emit()
{
	guard(VClass::Emit);
	//	Emit method code.
	for (int i = 0; i < Methods.Num(); i++)
	{
		Methods[i]->Emit();
	}

	//	Build list of state labels, resolve jumps.
	EmitStateLabels();

	//	Emit code of the state methods.
	for (VState* s = States; s; s = s->Next)
	{
		s->Emit();
	}

	//	Emit code of the network replication conditions.
	for (int ri = 0; ri < RepInfos.Num(); ri++)
	{
		RepInfos[ri].Cond->Emit();
	}

	DefaultProperties->Emit();
	unguard;
}

//==========================================================================
//
//	VClass::DecorateEmit
//
//==========================================================================

void VClass::DecorateEmit()
{
	guard(VClass::DecorateEmit);
	//	Emit method code.
	for (int i = 0; i < Methods.Num(); i++)
	{
		Methods[i]->Emit();
	}
	unguard;
}

//==========================================================================
//
//	VClass::EmitStateLabels
//
//==========================================================================

void VClass::EmitStateLabels()
{
	guard(VClass::EmitStateLabels);
	if (ParentClass && !(ClassFlags & CLASS_SkipSuperStateLabels))
	{
		StateLabels = ParentClass->StateLabels;
	}

	//	First add all labels.
	for (int i = 0; i < StateLabelDefs.Num(); i++)
	{
		VStateLabelDef& Lbl = StateLabelDefs[i];
		TArray<VName> Names;
		StaticSplitStateLabel(Lbl.Name, Names);
		SetStateLabel(Names, Lbl.State);
	}

	//	Then resolve state labels that do immediate jumps.
	for (int i = 0; i < StateLabelDefs.Num(); i++)
	{
		VStateLabelDef& Lbl = StateLabelDefs[i];
		if (Lbl.GotoLabel != NAME_None)
		{
			Lbl.State = ResolveStateLabel(Lbl.Loc, Lbl.GotoLabel, Lbl.GotoOffset);
			TArray<VName> Names;
			StaticSplitStateLabel(Lbl.Name, Names);
			SetStateLabel(Names, Lbl.State);
		}
	}
	unguard;
}

//==========================================================================
//
//	VClass::ResolveStateLabel
//
//==========================================================================

VState* VClass::ResolveStateLabel(TLocation Loc, VName LabelName, int Offset)
{
	VClass* CheckClass = this;
	VStr CheckName = *LabelName;

	int DCol = CheckName.IndexOf("::");
	if (DCol >= 0)
	{
		VStr ClassNameStr(CheckName, 0, DCol);
		VName ClassName(*ClassNameStr);
		if (ClassName == NAME_Super)
		{
			CheckClass = ParentClass;
		}
		else
		{
			CheckClass = StaticFindClass(ClassName);
			if (!CheckClass)
			{
				ParseError(Loc, "No such class %s", *ClassName);
				return NULL;
			}
		}
		CheckName = VStr(CheckName, DCol + 2, CheckName.Length() - DCol - 2);
	}

	TArray<VName> Names;
	StaticSplitStateLabel(CheckName, Names);
	VStateLabel* Lbl = CheckClass->FindStateLabel(Names, true);
	if (!Lbl)
	{
		ParseError(Loc, "No such state %s", *LabelName);
		return NULL;
	}

	VState* State = Lbl->State;
	int Count = Offset;
	while (Count--)
	{
		if (!State || !State->Next)
		{
			ParseError(Loc, "Bad jump offset");
			return NULL;
		}
		State = State->Next;
	}
	return State;
}

//==========================================================================
//
//	VClass::SetStateLabel
//
//==========================================================================

void VClass::SetStateLabel(VName AName, VState* State)
{
	guard(VClass::SetStateLabel);
	for (int i = 0; i < StateLabels.Num(); i++)
	{
		if (StateLabels[i].Name == AName)
		{
			StateLabels[i].State = State;
			return;
		}
	}
	VStateLabel& L = StateLabels.Alloc();
	L.Name = AName;
	L.State = State;
	unguard;
}

//==========================================================================
//
//	VClass::SetStateLabel
//
//==========================================================================

void VClass::SetStateLabel(const TArray<VName>& Names, VState* State)
{
	guard(VClass::SetStateLabel);
	if (!Names.Num())
	{
		return;
	}
	TArray<VStateLabel>* List = &StateLabels;
	VStateLabel* Lbl = NULL;
	for (int ni = 0; ni < Names.Num(); ni++)
	{
		Lbl = NULL;
		for (int i = 0; i < List->Num(); i++)
		{
			if ((*List)[i].Name == Names[ni])
			{
				Lbl = &(*List)[i];
				break;
			}
		}
		if (!Lbl)
		{
			Lbl = &List->Alloc();
			Lbl->Name = Names[ni];
		}
		List = &Lbl->SubLabels;
	}
	Lbl->State = State;
	unguard;
}

//==========================================================================
//
//	VClass::PostLoad
//
//==========================================================================

void VClass::PostLoad()
{
	if (ObjectFlags & CLASSOF_PostLoaded)
	{
		//	Already set up.
		return;
	}

	//	Make sure parent class has been set up.
	if (GetSuperClass())
	{
		GetSuperClass()->PostLoad();
	}

	//	Calculate field offsets and class size.
	CalcFieldOffsets();

	//	Initialise reference fields.
	InitReferences();

	//	Initialise destructor fields.
	InitDestructorFields();

	//	Initialise net fields.
	InitNetFields();

	//	Create virtual table.
	CreateVTable();

	//	Set up states lookup table.
	InitStatesLookup();

	//	Set state in-class indexes.
	int CurrIndex = 0;
	for (VState* S = NetStates; S; S = S->NetNext)
	{
		S->InClassIndex = CurrIndex++;
	}

	ObjectFlags |= CLASSOF_PostLoaded;
}

//==========================================================================
//
//	VClass::DecoratePostLoad
//
//==========================================================================

void VClass::DecoratePostLoad()
{
	guard(VClass::DecoratePostLoad);
	//	Compile.
	for (int i = 0; i < Methods.Num(); i++)
	{
		Methods[i]->PostLoad();
	}

	//	Calculate indexes of virtual methods.
	CalcFieldOffsets();

	//	Recreate virtual table.
	CreateVTable();
	unguard;
}

//==========================================================================
//
//	VClass::CalcFieldOffsets
//
//==========================================================================

void VClass::CalcFieldOffsets()
{
	guard(VClass::CalcFieldOffsets);
	//	Skip this for C++ only classes.
	if (!Outer && (ObjectFlags & CLASSOF_Native))
	{
		ClassNumMethods = ParentClass ? ParentClass->ClassNumMethods : 0;
		return;
	}

	int numMethods = ParentClass ? ParentClass->ClassNumMethods : 0;
	for (int i = 0; i < GMembers.Num(); i++)
	{
		if (GMembers[i]->MemberType != MEMBER_Method ||
			GMembers[i]->Outer != this)
		{
			continue;
		}
		VMethod* M = (VMethod*)GMembers[i];
		if (M == DefaultProperties)
		{
			M->VTableIndex = -1;
			continue;
		}
		int MOfs = -1;
		if (ParentClass)
		{
			MOfs = ParentClass->GetMethodIndex(M->Name);
		}
		if (MOfs == -1 && !(M->Flags & FUNC_Final))
		{
			MOfs = numMethods++;
		}
		M->VTableIndex = MOfs;
	}

	VField* PrevField = NULL;
	int PrevSize = ClassSize;
	int size = 0;
	if (ParentClass)
	{
		//	GCC has a strange behavior of starting to add fields in subclasses
		// in a class that has virtual methods on unaligned parent size offset.
		// In other cases and in other compilers it starts on aligned parent
		// class size offset.
		if (sizeof(DummyClass1) == sizeof(DummyClass2))
			size = ParentClass->ClassUnalignedSize;
		else
			size = ParentClass->ClassSize;
	}
	for (VField* fi = Fields; fi; fi = fi->Next)
	{
		if (fi->Type.Type == TYPE_Bool && PrevField &&
			PrevField->Type.Type == TYPE_Bool &&
			PrevField->Type.BitMask != 0x80000000)
		{
			vuint32 bit_mask = PrevField->Type.BitMask << 1;
			if (fi->Type.BitMask != bit_mask)
				Sys_Error("Wrong bit mask");
			fi->Type.BitMask = bit_mask;
			fi->Ofs = PrevField->Ofs;
		}
		else
		{
			if (fi->Type.Type == TYPE_Struct ||
				(fi->Type.Type == TYPE_Array && fi->Type.ArrayInnerType == TYPE_Struct))
			{
				//	Make sure struct size has been calculated.
				fi->Type.Struct->PostLoad();
			}
			int FldAlign = fi->Type.GetAlignment();
			size = (size + FldAlign - 1) & ~(FldAlign - 1);
			fi->Ofs = size;
			size += fi->Type.GetSize();
		}
		PrevField = fi;
	}
	ClassUnalignedSize = size;
	size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);
	ClassSize = size;
	ClassNumMethods = numMethods;
	if ((ObjectFlags & CLASSOF_Native) && ClassSize != PrevSize)
	{
		Sys_Error("Bad class size, class %s, C++ %d, VavoomC %d)",
			GetName(), PrevSize, ClassSize);
	}
	unguard;
}

//==========================================================================
//
//	VClass::InitNetFields
//
//==========================================================================

void VClass::InitNetFields()
{
	guard(VClass::InitNetFields);
	if (ParentClass)
	{
		NetFields = ParentClass->NetFields;
		NetMethods = ParentClass->NetMethods;
		NumNetFields = ParentClass->NumNetFields;
	}

	for (VField* fi = Fields; fi; fi = fi->Next)
	{
		if (!(fi->Flags & FIELD_Net))
		{
			continue;
		}
		fi->NetIndex = NumNetFields++;
		fi->NextNetField = NetFields;
		NetFields = fi;
	}

	for (int i = 0; i < GMembers.Num(); i++)
	{
		if (GMembers[i]->MemberType != MEMBER_Method ||
			GMembers[i]->Outer != this)
		{
			continue;
		}
		VMethod* M = (VMethod*)GMembers[i];
		if (!(M->Flags & FUNC_Net))
		{
			continue;
		}
		VMethod* MPrev = NULL;
		if (ParentClass)
		{
			MPrev = ParentClass->FindMethod(M->Name);
		}
		if (MPrev)
		{
			M->NetIndex = MPrev->NetIndex;
		}
		else
		{
			M->NetIndex = NumNetFields++;
		}
		M->NextNetMethod = NetMethods;
		NetMethods = M;
	}
	unguard;
}

//==========================================================================
//
//	VClass::InitReferences
//
//==========================================================================

void VClass::InitReferences()
{
	guard(VClass::InitReferences);
	ReferenceFields = NULL;
	if (GetSuperClass())
	{
		ReferenceFields = GetSuperClass()->ReferenceFields;
	}

	for (VField* F = Fields; F; F = F->Next)
	{
		switch (F->Type.Type)
		{
		case TYPE_Reference:
		case TYPE_Delegate:
			F->NextReference = ReferenceFields;
			ReferenceFields = F;
			break;
		
		case TYPE_Struct:
			F->Type.Struct->PostLoad();
			if (F->Type.Struct->ReferenceFields)
			{
				F->NextReference = ReferenceFields;
				ReferenceFields = F;
			}
			break;

		case TYPE_Array:
		case TYPE_DynamicArray:
			if (F->Type.ArrayInnerType == TYPE_Reference)
			{
				F->NextReference = ReferenceFields;
				ReferenceFields = F;
			}
			else if (F->Type.ArrayInnerType == TYPE_Struct)
			{
				F->Type.Struct->PostLoad();
				if (F->Type.Struct->ReferenceFields)
				{
					F->NextReference = ReferenceFields;
					ReferenceFields = F;
				}
			}
			break;
		}
	}
	unguard;
}

//==========================================================================
//
//	VClass::InitDestructorFields
//
//==========================================================================

void VClass::InitDestructorFields()
{
	guard(VClass::InitDestructorFields);
	DestructorFields = NULL;
	if (GetSuperClass())
	{
		DestructorFields = GetSuperClass()->DestructorFields;
	}

	for (VField* F = Fields; F; F = F->Next)
	{
		switch (F->Type.Type)
		{
		case TYPE_String:
			F->DestructorLink = DestructorFields;
			DestructorFields = F;
			break;

		case TYPE_Struct:
			F->Type.Struct->PostLoad();
			if (F->Type.Struct->DestructorFields)
			{
				F->DestructorLink = DestructorFields;
				DestructorFields = F;
			}
			break;

		case TYPE_Array:
			if (F->Type.ArrayInnerType == TYPE_String)
			{
				F->DestructorLink = DestructorFields;
				DestructorFields = F;
			}
			else if (F->Type.ArrayInnerType == TYPE_Struct)
			{
				F->Type.Struct->PostLoad();
				if (F->Type.Struct->DestructorFields)
				{
					F->DestructorLink = DestructorFields;
					DestructorFields = F;
				}
			}
			break;

		case TYPE_DynamicArray:
			F->DestructorLink = DestructorFields;
			DestructorFields = F;
			break;
		}
	}
	unguard;
}

//==========================================================================
//
//	VClass::CreateVTable
//
//==========================================================================

void VClass::CreateVTable()
{
	guard(VClass::CreateVTable);
	if (ClassVTable)
	{
		delete[] ClassVTable;
	}
	ClassVTable = new VMethod*[ClassNumMethods];
	memset(ClassVTable, 0, ClassNumMethods * sizeof(VMethod*));
	if (ParentClass)
	{
		memcpy(ClassVTable, ParentClass->ClassVTable,
			ParentClass->ClassNumMethods * sizeof(VMethod*));
	}
	for (int i = 0; i < GMembers.Num(); i++)
	{
		if (GMembers[i]->MemberType != MEMBER_Method ||
			GMembers[i]->Outer != this)
		{
			continue;
		}
		VMethod* M = (VMethod*)GMembers[i];
		if (M->VTableIndex == -1)
		{
			continue;
		}
		ClassVTable[M->VTableIndex] = M;
	}
	unguard;
}

//==========================================================================
//
//	VClass::InitStatesLookup
//
//==========================================================================

void VClass::InitStatesLookup()
{
	guard(VClass::InitStatesLookup);
	//	This is also called from dehacked parser, so we must do this check.
	if (StatesLookup.Num())
	{
		return;
	}

	//	Create states lookup table.
	if (GetSuperClass())
	{
		GetSuperClass()->InitStatesLookup();
		for (int i = 0; i < GetSuperClass()->StatesLookup.Num(); i++)
		{
			StatesLookup.Append(GetSuperClass()->StatesLookup[i]);
		}
	}
	for (VState* S = NetStates; S; S = S->NetNext)
	{
		S->NetId = StatesLookup.Num();
		StatesLookup.Append(S);
	}
	unguard;
}

#ifndef IN_VCC

//==========================================================================
//
//	VClass::CreateDefaults
//
//==========================================================================

void VClass::CreateDefaults()
{
	guard(VClass::CreateDefaults);
	if (Defaults)
	{
		return;
	}

	if (ParentClass && !ParentClass->Defaults)
	{
		ParentClass->CreateDefaults();
	}

	//	Allocate memory.
	Defaults = new vuint8[ClassSize];
	memset(Defaults, 0, ClassSize);

	//	Copy default properties from the parent class.
	if (ParentClass)
	{
		ParentClass->CopyObject(ParentClass->Defaults, Defaults);
	}

	//	Call default properties method.
	if (DefaultProperties)
	{
		P_PASS_REF((VObject*)Defaults);
		VObject::ExecuteFunction(DefaultProperties);
	}
	unguard;
}

//==========================================================================
//
//	VClass::CopyObject
//
//==========================================================================

void VClass::CopyObject(const vuint8* Src, vuint8* Dst)
{
	guard(VClass::CopyObject);
	//	Copy parent class fields.
	if (GetSuperClass())
	{
		GetSuperClass()->CopyObject(Src, Dst);
	}
	//	Copy fields.
	for (VField* F = Fields; F; F = F->Next)
	{
		VField::CopyFieldValue(Src + F->Ofs, Dst + F->Ofs, F->Type);
	}
	unguardf(("(%s)", GetName()));
}

//==========================================================================
//
//	VClass::SerialiseObject
//
//==========================================================================

void VClass::SerialiseObject(VStream& Strm, VObject* Obj)
{
	guard(SerialiseObject);
	//	Serialise parent class fields.
	if (GetSuperClass())
	{
		GetSuperClass()->SerialiseObject(Strm, Obj);
	}
	//	Serialise fields.
	for (VField* F = Fields; F; F = F->Next)
	{
		//	Skip native and transient fields.
		if (F->Flags & (FIELD_Native | FIELD_Transient))
		{
			continue;
		}
		VField::SerialiseFieldValue(Strm, (vuint8*)Obj + F->Ofs, F->Type);
	}
	unguardf(("(%s)", GetName()));
}

//==========================================================================
//
//	VClass::CleanObject
//
//==========================================================================

void VClass::CleanObject(VObject* Obj)
{
	guard(VClass::CleanObject);
	for (VField* F = ReferenceFields; F; F = F->NextReference)
	{
		VField::CleanField((vuint8*)Obj + F->Ofs, F->Type);
	}
	unguardf(("(%s)", GetName()));
}

//==========================================================================
//
//	VClass::DestructObject
//
//==========================================================================

void VClass::DestructObject(VObject* Obj)
{
	guard(VClass::DestructObject);
	for (VField* F = DestructorFields; F; F = F->DestructorLink)
	{
		VField::DestructField((vuint8*)Obj + F->Ofs, F->Type);
	}
	unguardf(("(%s)", GetName()));
}

//==========================================================================
//
//	VClass::CreateDerivedClass
//
//==========================================================================

VClass* VClass::CreateDerivedClass(VName AName, VMemberBase* AOuter,
	TLocation ALoc)
{
	guard(VClass::CreateDerivedClass);
	VClass* NewClass = NULL;
	for (int i = 0; i < GDecorateClassImports.Num(); i++)
	{
		if (GDecorateClassImports[i]->Name == AName)
		{
			//	This class implements a decorate import class.
			NewClass = GDecorateClassImports[i];
			NewClass->MemberType = MEMBER_Class;
			NewClass->Outer = AOuter;
			NewClass->Loc = ALoc;
			//	Make sure parent class is correct.
			VClass* Check = FindClass(*NewClass->ParentClassName);
			if (!Check)
			{
				Sys_Error("No such class %s", *NewClass->ParentClassName);
			}
			if (!IsChildOf(Check))
			{
				Sys_Error("%s must be a child of %s", *AName, *Check->Name);
			}
			GDecorateClassImports.RemoveIndex(i);
			break;
		}
	}
	if (!NewClass)
	{
		NewClass = new VClass(AName, AOuter, ALoc);
	}
	NewClass->ParentClass = this;
	NewClass->PostLoad();
	NewClass->CreateDefaults();
	return NewClass;
	unguard;
}

#endif

//==========================================================================
//
//	VClass::GetReplacement
//
//==========================================================================

VClass* VClass::GetReplacement()
{
	guard(VClass::GetReplacement);
	if (!Replacement)
	{
		return this;
	}
	//	Avoid looping recursion by temporarely NULL-ing the field
	VClass* Temp = Replacement;
	Replacement = NULL;
	VClass* Ret = Temp->GetReplacement();
	Replacement = Temp;
	return Ret;
	unguard;
}

//==========================================================================
//
//	VClass::GetReplacee
//
//==========================================================================

VClass* VClass::GetReplacee()
{
	guard(VClass::GetReplacee);
	if (!Replacee)
	{
		return this;
	}
	//	Avoid looping recursion by temporarely NULL-ing the field
	VClass* Temp = Replacee;
	Replacee = NULL;
	VClass* Ret = Temp->GetReplacee();
	Replacee = Temp;
	return Ret;
	unguard;
}

//==========================================================================
//
//	VClass::HashLowerCased
//
//==========================================================================

void VClass::HashLowerCased()
{
	guard(VClass::HashLowerCased);
	LowerCaseName = *VStr(*Name).ToLower();
	int HashIndex = GetTypeHash(LowerCaseName) & (LOWER_CASE_HASH_SIZE - 1);
	LowerCaseHashNext = GLowerCaseHashTable[HashIndex];
	GLowerCaseHashTable[HashIndex] = this;
	unguard;
}

//==========================================================================
//
//	VClass::FindClassLowerCase
//
//==========================================================================

VClass* VClass::FindClassLowerCase(VName AName)
{
	guard(VClass::FindClassLowerCase);
	int HashIndex = GetTypeHash(AName) & (LOWER_CASE_HASH_SIZE - 1);
	for (VClass* Probe = GLowerCaseHashTable[HashIndex]; Probe;
		Probe = Probe->LowerCaseHashNext)
	{
		if (Probe->LowerCaseName == AName)
		{
			return Probe;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	operator<<
//
//==========================================================================

VStream& operator<<(VStream& Strm, VStateLabel& Lbl)
{
	Strm << Lbl.Name << Lbl.State;
	int NumSub = Lbl.SubLabels.Num();
	Strm << STRM_INDEX(NumSub);
	if (Strm.IsLoading())
	{
		Lbl.SubLabels.SetNum(NumSub);
	}
	for (int i = 0; i < NumSub; i++)
	{
		Strm << Lbl.SubLabels[i];
	}
	return Strm;
}
