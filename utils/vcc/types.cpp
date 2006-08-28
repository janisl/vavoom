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

#include "vcc.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VLocalVarDef			localdefs[MAX_LOCAL_DEFS];
int						numlocaldefs = 1;
int						localsofs = 0;

TType					SelfType;
VClass*					SelfClass;

int						maxlocalsofs = 0;
TArray<breakInfo_t>		BreakInfo;
int						BreakLevel;
int						BreakNumLocalsOnStart;
TArray<continueInfo_t> 	ContinueInfo;
int						ContinueLevel;
int						ContinueNumLocalsOnStart;
TType					FuncRetType;

TArray<VImportedPackage>	PackagesToLoad;
TArray<VConstant*>			ParsedConstants;
TArray<VStruct*>			ParsedStructs;
TArray<VClass*>				ParsedClasses;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	InitTypes
//
//==========================================================================

void InitTypes()
{
}

//==========================================================================
//
//	VMemberBase::VMemberBase
//
//==========================================================================

VMemberBase::VMemberBase(vuint8 InType, VName InName, VMemberBase* InOuter,
	TLocation InLoc)
: MemberType(InType)
, MemberIndex(GMembers.Append(this))
, Name(InName)
, Outer(InOuter)
, Loc(InLoc)
{
	int HashIndex = Name.GetIndex() & 4095;
	HashNext = GMembersHash[HashIndex];
	GMembersHash[HashIndex] = this;
}

//==========================================================================
//
//	VMemberBase::IsIn
//
//==========================================================================

bool VMemberBase::IsIn(VMemberBase* SomeOuter) const
{
	for (VMemberBase* Tst = Outer; Tst; Tst = Tst->Outer)
		if (Tst == SomeOuter)
			return true;
	return !SomeOuter;
}

//==========================================================================
//
//	VMemberBase::StaticFindMember
//
//==========================================================================

VMemberBase* VMemberBase::StaticFindMember(VName InName,
	VMemberBase* InOuter, vuint8 InType)
{
	int HashIndex = InName.GetIndex() & 4095;
	for (VMemberBase* m = GMembersHash[HashIndex]; m; m = m->HashNext)
	{
		if (m->Name == InName && (m->Outer == InOuter ||
			(InOuter == ANY_PACKAGE && m->Outer->MemberType == MEMBER_Package)) &&
			(InType == ANY_MEMBER || m->MemberType == InType))
		{
			return m;
		}
	}
	return NULL;
}

//==========================================================================
//
//	TType::TType
//
//==========================================================================

TType::TType(VClass* InClass) :
	type(ev_reference), InnerType(ev_void), ArrayInnerType(ev_void),
	PtrLevel(0), array_dim(0), Class(InClass)
{
}

//==========================================================================
//
//	TType::TType
//
//==========================================================================

TType::TType(VStruct* InStruct) :
	type(InStruct->IsVector ? ev_vector : ev_struct), InnerType(ev_void),
	ArrayInnerType(ev_void), PtrLevel(0), array_dim(0), Struct(InStruct)
{
}

//==========================================================================
//
//	TType::Equals
//
//==========================================================================

bool TType::Equals(const TType& Other) const
{
	if (type != Other.type ||
		InnerType != Other.InnerType ||
		ArrayInnerType != Other.ArrayInnerType ||
		PtrLevel != Other.PtrLevel ||
		array_dim != Other.array_dim ||
		bit_mask != Other.bit_mask)
		return false;
	return true;
}

//==========================================================================
//
//	MakePointerType
//
//==========================================================================

TType MakePointerType(const TType& type)
{
	TType pointer = type;
	if (pointer.type == ev_pointer)
	{
		pointer.PtrLevel++;
	}
	else
	{
		pointer.InnerType = pointer.type;
		pointer.type = ev_pointer;
		pointer.PtrLevel = 1;
	}
	return pointer;
}

//==========================================================================
//
//	TType::GetPointerInnerType
//
//==========================================================================

TType TType::GetPointerInnerType() const
{
	if (type != ev_pointer)
	{
		FatalError("Not a pointer type");
		return *this;
	}
	TType ret = *this;
	ret.PtrLevel--;
	if (ret.PtrLevel <= 0)
	{
		ret.type = InnerType;
		ret.InnerType = ev_void;
	}
	return ret;
}

//==========================================================================
//
//	MakeArrayType
//
//==========================================================================

TType MakeArrayType(const TType& type, int elcount, TLocation l)
{
	if (type.type == ev_array)
	{
		ParseError(l, "Can't have multi-dimensional arrays");
	}
	TType array = type;
	array.ArrayInnerType = type.type;
	array.type = ev_array;
	array.array_dim = elcount;
	return array;
}

//==========================================================================
//
//	TType::GetArrayInnerType
//
//==========================================================================

TType TType::GetArrayInnerType() const
{
	if (type != ev_array)
	{
		FatalError("Not an array type");
		return *this;
	}
	TType ret = *this;
	ret.type = ArrayInnerType;
	ret.ArrayInnerType = ev_void;
	ret.array_dim = 0;
	return ret;
}

//==========================================================================
//
//  CheckForType
//
//==========================================================================

TType CheckForType(VClass* InClass, VName Name)
{
	if (Name == NAME_None)
	{
		return TType(ev_unknown);
	}

	VMemberBase* m = VMemberBase::StaticFindMember(Name, ANY_PACKAGE,
		MEMBER_Class);
	if (m)
	{
		return TType((VClass*)m);
	}
	m = VMemberBase::StaticFindMember(Name, InClass ?
		(VMemberBase*)InClass : (VMemberBase*)ANY_PACKAGE, MEMBER_Struct);
	if (m)
	{
		return TType((VStruct*)m);
	}
	if (InClass)
	{
		return CheckForType(InClass->ParentClass, Name);
	}
	return TType(ev_unknown);
}

//==========================================================================
//
//  CheckForClass
//
//==========================================================================

VClass* CheckForClass(VName Name)
{
	VMemberBase* m = VMemberBase::StaticFindMember(Name, ANY_PACKAGE,
		MEMBER_Class);
	if (m)
	{
		return (VClass*)m;
	}
	return NULL;
}

//==========================================================================
//
//	TType::GetSize
//
//==========================================================================

int TType::GetSize() const
{
	switch (type)
	{
	case ev_int:		return 4;
	case ev_float:		return 4;
	case ev_name:		return 4;
	case ev_string:		return 4;
	case ev_pointer:	return 4;
	case ev_reference:	return 4;
	case ev_array:		return array_dim * GetArrayInnerType().GetSize();
	case ev_struct:		return Struct->StackSize * 4;
	case ev_vector:		return 12;
	case ev_classid:	return 4;
	case ev_state:		return 4;
	case ev_bool:		return 4;
	case ev_delegate:	return 8;
	}
	return 0;
}

//==========================================================================
//
//	CheckForFunction
//
//==========================================================================

VMethod* CheckForFunction(VClass* InClass, VName Name)
{
	if (Name == NAME_None)
	{
		return NULL;
	}
	return (VMethod*)VMemberBase::StaticFindMember(Name, InClass,
		MEMBER_Method);
}

//==========================================================================
//
//	CheckForConstant
//
//==========================================================================

VConstant* CheckForConstant(VClass* InClass, VName Name)
{
	VMemberBase* m = VMemberBase::StaticFindMember(Name, InClass ?
		(VMemberBase*)InClass : (VMemberBase*)ANY_PACKAGE, MEMBER_Const);
	if (m)
	{
		return (VConstant*)m;
	}
	if (InClass)
	{
		return CheckForConstant(InClass->ParentClass, Name);
	}
	return NULL;
}

//==========================================================================
//
//	TType::CheckPassable
//
//	Check, if type can be pushed into the stack
//
//==========================================================================

void TType::CheckPassable(TLocation l) const
{
	if (GetSize() != 4 && type != ev_vector && type != ev_delegate)
	{
		char Name[256];
		GetName(Name);
		ParseError(l, "Type %s is not passable", Name);
	}
}

//==========================================================================
//
//	TType::CheckMatch
//
//	Check, if types are compatible
//
//	t1 - current type
//	t2 - needed type
//
//==========================================================================

void TType::CheckMatch(TLocation l, const TType& Other) const
{
	CheckPassable(l);
	Other.CheckPassable(l);
	if (Equals(Other))
	{
		return;
	}
	if (type == ev_vector && Other.type == ev_vector)
	{
		return;
	}
	if (type == ev_pointer && Other.type == ev_pointer)
	{
		TType it1 = GetPointerInnerType();
		TType it2 = Other.GetPointerInnerType();
		if (it1.Equals(it2))
		{
			return;
		}
		if ((it1.type == ev_void) || (it2.type == ev_void))
		{
			return;
		}
		if (it1.type == ev_struct && it2.type == ev_struct)
		{
			VStruct* s1 = it1.Struct;
			VStruct* s2 = it2.Struct;
			for (VStruct* st1 = s1->ParentStruct; st1; st1 = st1->ParentStruct)
			{
				if (st1 == s2)
				{
					return;
				}
			}
		}
	}
	if (type == ev_reference && Other.type == ev_reference)
	{
		VClass* c1 = Class;
		VClass* c2 = Other.Class;
		if (!c1 || !c2)
		{
			//	none reference can be assigned to any reference.
			return;
		}
		if (c1 == c2)
		{
			return;
		}
		for (VClass* pc1 = c1->ParentClass; pc1; pc1 = pc1->ParentClass)
		{
			if (pc1 == c2)
			{
				return;
			}
		}
	}
	if (type == ev_int && Other.type == ev_bool)
	{
		return;
	}
	//	Allow assigning none to states, classes and delegates
	if (type == ev_reference && Class == NULL && (Other.type == ev_classid ||
		Other.type == ev_state || Other.type == ev_delegate))
	{
		return;
	}
	if (type == ev_delegate && Other.type == ev_delegate)
	{
		VMethod& F1 = *Function;
		VMethod& F2 = *Other.Function;
		if (F1.Flags & FUNC_Static || F2.Flags & FUNC_Static)
		{
			ParseError(l, "Can't assign a static function to delegate");
		}
		if (!F1.ReturnType.Equals(F2.ReturnType))
		{
			ParseError(l, "Delegate has different return type");
		}
		else if (F1.NumParams != F2.NumParams)
		{
			ParseError(l, "Delegate has different number of arguments");
		}
		else for (int i = 0; i < F1.NumParams; i++)
			if (!F1.ParamTypes[i].Equals(F2.ParamTypes[i]))
			{
				ParseError(l, "Delegate argument %d differs", i + 1);
			}
		return;
	}
	char Name1[256];
	char Name2[256];
	GetName(Name1);
	Other.GetName(Name2);
	ParseError(l, "Type mistmatch, types %s and %s are not compatible %d %d",
		Name1, Name2, type, Other.type);
}

//==========================================================================
//
//	TType::GetName
//
//==========================================================================

void TType::GetName(char* Dest) const
{
	switch (type)
	{
	case ev_int:		strcpy(Dest, "int"); break;
	case ev_float:		strcpy(Dest, "float"); break;
	case ev_name:		strcpy(Dest, "name"); break;
	case ev_string:		strcpy(Dest, "string"); break;
	case ev_pointer:	GetPointerInnerType().GetName(Dest); 
		for (int i = 0; i < PtrLevel; i++) strcat(Dest, "*"); break;
	case ev_reference:	strcpy(Dest, Class ? *Class->Name : "none"); break;
	case ev_array:		GetArrayInnerType().GetName(Dest); strcat(Dest, "[]"); break;
	case ev_struct:		strcpy(Dest, *Struct->Name); break;
	case ev_vector:		strcpy(Dest, "vector"); break;
	case ev_classid:	strcpy(Dest, "classid"); break;
	case ev_state:		strcpy(Dest, "state"); break;
	case ev_bool:		strcpy(Dest, "bool"); break;
	default:			strcpy(Dest, "unknown"); break;
	}
}

//==========================================================================
//
//	CheckForStructField
//
//==========================================================================

VField* CheckForStructField(VStruct* InStruct, VName FieldName)
{
	for (VField* fi = InStruct->Fields; fi; fi = fi->Next)
	{
		if (fi->Name == FieldName)
		{
			return fi;
		}
	}
	if (InStruct->ParentStruct)
	{
		return CheckForStructField(InStruct->ParentStruct, FieldName);
	}
	return NULL;
}

//==========================================================================
//
//	CheckForField
//
//==========================================================================

VField* CheckForField(TLocation l, VName Name, VClass* InClass, bool CheckPrivate)
{
	if (!InClass)
	{
		return NULL;
	}
	if (!InClass->Parsed)
	{
		return NULL;
	}
	if (Name == NAME_None)
	{
		return NULL;
	}
	for (VField *fi = InClass->Fields; fi; fi = fi->Next)
	{
		if (Name == fi->Name)
		{
			if (CheckPrivate && fi->flags & FIELD_Private &&
				InClass != SelfClass)
			{
				ParseError(l, "Field %s is private", *fi->Name);
			}
			return fi;
		}
	}
	return CheckForField(l, Name, InClass->ParentClass, CheckPrivate);
}

//==========================================================================
//
//	CheckForMethod
//
//==========================================================================

VMethod* CheckForMethod(VName Name, VClass* InClass)
{
	if (!InClass)
	{
		return NULL;
	}
	if (!InClass->Parsed)
	{
		return NULL;
	}
	if (Name == NAME_None)
	{
		return NULL;
	}
	VMethod* M = (VMethod*)VMemberBase::StaticFindMember(Name, InClass,
		MEMBER_Method);
	if (M)
	{
		return M;
	}
	return CheckForMethod(Name, InClass->ParentClass);
}

//==========================================================================
//
//	CheckForState
//
//==========================================================================

VState* CheckForState(VName StateName, VClass* InClass)
{
	VMemberBase* m = VMemberBase::StaticFindMember(StateName, InClass,
		MEMBER_State);
	if (m)
	{
		return (VState*)m;
	}
	if (InClass->ParentClass)
	{
		return CheckForState(StateName, InClass->ParentClass);
	}
	return NULL;
}

//**************************************************************************
//**
//**
//**
//**************************************************************************

//==========================================================================
//
//	VStruct::AddField
//
//==========================================================================

void VStruct::AddField(VField* f)
{
	for (VField* Check = Fields; Check; Check = Check->Next)
	{
		if (f->Name == Check->Name)
		{
			ParseError(f->Loc, "Redeclared field");
			ParseError(Check->Loc, "Previous declaration here");
		}
	}

	if (!Fields)
		Fields = f;
	else
	{
		VField* Prev = Fields;
		while (Prev->Next)
			Prev = Prev->Next;
		Prev->Next = f;
	}
	f->Next = NULL;
}

//==========================================================================
//
//	VClass::AddConstant
//
//==========================================================================

void VClass::AddConstant(VConstant* c)
{
	Constants.Append(c);
}

//==========================================================================
//
//	VClass::AddField
//
//==========================================================================

void VClass::AddField(VField* f)
{
	if (!Fields)
		Fields = f;
	else
	{
		VField* Prev = Fields;
		while (Prev->Next)
			Prev = Prev->Next;
		Prev->Next = f;
	}
	f->Next = NULL;
}

//==========================================================================
//
//	VClass::AddState
//
//==========================================================================

void VClass::AddState(VState* s)
{
	if (!States)
		States = s;
	else
	{
		VState* Prev = States;
		while (Prev->Next)
			Prev = Prev->Next;
		Prev->Next = s;
	}
	s->Next = NULL;
}

//==========================================================================
//
//	VClass::AddMethod
//
//==========================================================================

void VClass::AddMethod(VMethod* m)
{
	Methods.Append(m);
}

//==========================================================================
//
//	VField::NeedsDestructor
//
//==========================================================================

bool VField::NeedsDestructor() const
{
	if (type.type == ev_string)
		return true;
	if (type.type == ev_array)
	{
		if (type.ArrayInnerType == ev_string)
			return true;
		if (type.ArrayInnerType == ev_struct)
			return type.Struct->NeedsDestructor();
	}
	if (type.type == ev_struct)
		return type.Struct->NeedsDestructor();
	return false;
}

//==========================================================================
//
//	VStruct::NeedsDestructor
//
//==========================================================================

bool VStruct::NeedsDestructor() const
{
	for (VField* F = Fields; F; F = F->Next)
		if (F->NeedsDestructor())
			return true;
	if (ParentStruct)
		return ParentStruct->NeedsDestructor();
	return false;
}

//==========================================================================
//
//	CheckForLocalVar
//
//==========================================================================

int CheckForLocalVar(VName Name)
{
	if (Name == NAME_None)
	{
		return 0;
	}
	for (int i = 1; i < numlocaldefs; i++)
	{
		if (!localdefs[i].Visible)
		{
			continue;
		}
		if (localdefs[i].Name == Name)
		{
			return i;
		}
	}
	return 0;
}

//==========================================================================
//
//	VConstant::Define
//
//==========================================================================

bool VConstant::Define()
{
	if (PrevEnumValue)
	{
		Value = PrevEnumValue->Value + 1;
		return true;
	}

	if (ValueExpr)
	{
		ValueExpr = ValueExpr->Resolve();
	}
	if (!ValueExpr)
	{
		return false;
	}

	switch (Type)
	{
	case ev_int:
		if (!ValueExpr->GetIntConst(Value))
		{
			return false;
		}
		break;

	case ev_float:
		if (!ValueExpr->GetFloatConst(FloatValue))
		{
			return false;
		}
		break;

	default:
		ParseError(Loc, "Unsupported type of constant");
		return false;
	}
	return true;
}

//==========================================================================
//
//	VField::Define
//
//==========================================================================

bool VField::Define()
{
	if (type.type == ev_delegate)
	{
		return func->Define();
	}

	if (TypeExpr)
	{
		TypeExpr = TypeExpr->ResolveAsType();
	}
	if (!TypeExpr)
	{
		return false;
	}

	if (TypeExpr->Type.type == ev_void)
	{
		ParseError(TypeExpr->Loc, "Field cannot have void type.");
		return false;
	}
	type = TypeExpr->Type;

	Modifiers = TModifiers::Check(Modifiers, AllowedModifiers, Loc);
	flags = TModifiers::FieldAttr(Modifiers);
	return true;
}

//==========================================================================
//
//	VMethod::Define
//
//==========================================================================

bool VMethod::Define()
{
	bool Ret = true;

	Modifiers = TModifiers::Check(Modifiers, AllowedModifiers, Loc);
	Flags |= TModifiers::MethodAttr(Modifiers);

	if (Flags & FUNC_Static)
	{
		if (!(Flags & FUNC_Native))
		{
			ParseError(Loc, "Currently only native methods can be static");
			Ret = false;
		}
		if (!(Flags & FUNC_Final))
		{
			ParseError(Loc, "Currently static methods must be final.");
			Ret = false;
		}
	}

	if ((Flags & FUNC_VarArgs) && !(Flags & FUNC_Native))
	{
		ParseError(Loc, "Only native methods can have varargs");
	}

	if (ReturnTypeExpr)
	{
		ReturnTypeExpr = ReturnTypeExpr->ResolveAsType();
	}
	if (ReturnTypeExpr)
	{
		TType t = ReturnTypeExpr->Type;
		if (t.type != ev_void)
		{
			//	Function's return type must be void, vector or with size 4
			t.CheckPassable(ReturnTypeExpr->Loc);
		}
		ReturnType = t;
	}
	else
	{
		Ret = false;
	}

	//	Resolve parameters types.
	ParamsSize = 1;
	for (int i = 0; i < NumParams; i++)
	{
		VMethodParam& P = Params[i];

		if (P.TypeExpr)
		{
			P.TypeExpr = P.TypeExpr->ResolveAsType();
		}
		if (!P.TypeExpr)
		{
			Ret = false;
			continue;
		}
		TType type = P.TypeExpr->Type;

		if (type.type == ev_void)
		{
			ParseError(P.TypeExpr->Loc, "Bad variable type");
			Ret = false;
			continue;
		}
		type.CheckPassable(P.TypeExpr->Loc);

		ParamTypes[i] = type;
		ParamsSize += type.GetSize() / 4;
	}

	//	If this is a overriden method, verify that return type and argument
	// types match.
	VMethod* BaseMethod = NULL;
	if (Outer->MemberType == MEMBER_Class && Name != NAME_None &&
		((VClass*)Outer)->ParentClass)
	{
		BaseMethod = CheckForMethod(Name, ((VClass*)Outer)->ParentClass);
	}
	if (BaseMethod)
	{
		if (BaseMethod->Flags & FUNC_Final)
		{
			ParseError(Loc, "Method already has been declared as final and cannot be overriden.");
			Ret = false;
		}
		if (!BaseMethod->ReturnType.Equals(ReturnType))
		{
			ParseError(Loc, "Method redefined with different return type");
			Ret = false;
		}
		else if (BaseMethod->NumParams != NumParams)
		{
			ParseError(Loc, "Method redefined with different number of arguments");
			Ret = false;
		}
		else for (int i = 0; i < NumParams; i++)
			if (!BaseMethod->ParamTypes[i].Equals(ParamTypes[i]))
			{
				ParseError(Loc, "Type of argument %d differs from base class", i + 1);
				Ret = false;
			}
	}

	return Ret;
}

//==========================================================================
//
//	VState::Define
//
//==========================================================================

bool VState::Define()
{
	bool Ret = true;

	if (!Function->Define())
	{
		Ret = false;
	}

	return Ret;
}

//==========================================================================
//
//	VStruct::Define
//
//==========================================================================

bool VStruct::Define()
{
	if (ParentStructName != NAME_None)
	{
		TType type = CheckForType(Outer->MemberType == MEMBER_Class ?
			(VClass*)Outer : NULL, ParentStructName);
		if (type.type != ev_struct)
		{
			ParseError(ParentStructLoc, "%s is not a struct type",
				*ParentStructName);
		}
		else
		{
			ParentStruct = type.Struct;
		}
	}

	if (ParentStruct && !ParentStruct->Defined)
	{
		ParseError(ParentStructLoc, "Parent struct must be declared before");
		return false;
	}

	Defined = true;
	return true;
}

//==========================================================================
//
//	VClass::Define
//
//==========================================================================

bool VClass::Define()
{
	Modifiers = TModifiers::Check(Modifiers, AllowedModifiers, Loc);
	int ClassAttr = TModifiers::ClassAttr(Modifiers);

	if (ParentClassName != NAME_None)
	{
		ParentClass = CheckForClass(ParentClassName);
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

	for (int i = 0; i < MobjInfoExpressions.Num(); i++)
	{
		MobjInfoExpressions[i] = MobjInfoExpressions[i]->Resolve();
		if (!MobjInfoExpressions[i])
		{
			return false;
		}
		vint32 Id;
		if (!MobjInfoExpressions[i]->GetIntConst(Id))
		{
			return false;
		}
		mobjinfo_t& mi = mobj_info.Alloc();
		mi.doomednum = Id;
		mi.class_id = this;
	}

	for (int i = 0; i < ScriptIdExpressions.Num(); i++)
	{
		ScriptIdExpressions[i] = ScriptIdExpressions[i]->Resolve();
		if (!ScriptIdExpressions[i])
		{
			return false;
		}
		vint32 Id;
		if (!ScriptIdExpressions[i]->GetIntConst(Id))
		{
			return false;
		}
		mobjinfo_t& mi = script_ids.Alloc();
		mi.doomednum = Id;
		mi.class_id = this;
	}

	Defined = true;
	return true;
}

//==========================================================================
//
//	VStruct::DefineMembers
//
//==========================================================================

bool VStruct::DefineMembers()
{
	bool Ret = true;

	//	Define fields.
	vint32 size = 0;
	if (ParentStruct)
	{
		size = ParentStruct->StackSize * 4;
	}
	VField* PrevBool = NULL;
	for (VField* fi = Fields; fi; fi = fi->Next)
	{
		if (!fi->Define())
		{
			Ret = false;
		}
		if (fi->type.type == ev_bool && PrevBool && PrevBool->type.bit_mask != 0x80000000)
		{
			fi->type.bit_mask = PrevBool->type.bit_mask << 1;
		}
		else
		{
			size += fi->type.GetSize();
		}
		PrevBool = fi->type.type == ev_bool ? fi : NULL;
	}

	//	Validate vector type.
	if (IsVector)
	{
		int fc = 0;
		for (VField* f = Fields; f; f = f->Next)
		{
			if (f->type.type != ev_float)
			{
				ParseError(f->Loc, "Vector can have only float fields");
				Ret = false;
			}
			fc++;
		}
		if (fc != 3)
		{
			ParseError(Loc, "Vector must have exactly 3 float fields");
			Ret = false;
		}
	}

	StackSize = (size + 3) / 4;
	return Ret;
}

//==========================================================================
//
//	VClass::DefineMembers
//
//==========================================================================

bool VClass::DefineMembers()
{
	bool Ret = true;

	SelfClass = this;

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
		if (fi->type.type == ev_bool && PrevBool && PrevBool->type.bit_mask != 0x80000000)
		{
			fi->type.bit_mask = PrevBool->type.bit_mask << 1;
		}
		PrevBool = fi->type.type == ev_bool ? fi : NULL;
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

	return Ret;
}

//==========================================================================
//
//	VMethod::Emit
//
//==========================================================================

void VMethod::Emit()
{
	if (Flags & FUNC_Native)
	{
		if (Statement)
		{
			ParseError(Loc, "Native methods can't have a body");
		}
		return;
	}

	if (Outer->MemberType == MEMBER_Field)
	{
		//	Delegate
		return;
	}

	if (!Statement)
	{
		ParseError(Loc, "Method body missing");
		return;
	}

	VMemberBase* C = Outer;
	while (C && C->MemberType != MEMBER_Class)
	{
		C = C->Outer;
	}
	SelfClass = (VClass*)C;
	SelfType = TType(SelfClass);

	numlocaldefs = 1;
	localsofs = 1;

	for (int i = 0; i < NumParams; i++)
	{
		VMethodParam& P = Params[i];
		if (P.Name != NAME_None)
		{
			if (CheckForLocalVar(P.Name))
			{
				ParseError(P.Loc, "Redefined identifier %s", *P.Name);
			}
			localdefs[numlocaldefs].Name = P.Name;
			localdefs[numlocaldefs].type = ParamTypes[i];
			localdefs[numlocaldefs].ofs = localsofs;
			localdefs[numlocaldefs].Visible = true;
			localdefs[numlocaldefs].Cleared = false;
			numlocaldefs++;
		}
		localsofs += ParamTypes[i].GetSize() / 4;
	}
	maxlocalsofs = localsofs;

	BreakLevel = 0;
	ContinueLevel = 0;
	FuncRetType = ReturnType;

	BeginCode(this);
	for (int i = 0; i < numlocaldefs; i++)
	{
		if (localdefs[i].type.type == ev_vector)
		{
			AddStatement(OPC_VFixParam, i);
		}
	}

	if (!NumErrors)
	{
		Statement->Resolve();
	}

	//  Call parent constructor
	if (this == SelfClass->DefaultProperties && SelfClass->ParentClass)
	{
		AddStatement(OPC_LocalAddress0);
		AddStatement(OPC_PushPointedPtr);
		AddStatement(OPC_Call, SelfClass->ParentClass->DefaultProperties);
	}

	if (!NumErrors)
	{
		Statement->Emit();
	}

	if (FuncRetType.type == ev_void)
	{
		EmitClearStrings(0, numlocaldefs);
		AddStatement(OPC_Return);
	}
	NumLocals = maxlocalsofs;
	EndCode(this);
}

//==========================================================================
//
//	VState::Emit
//
//==========================================================================

void VState::Emit()
{
	if (FrameExpr)
		FrameExpr = FrameExpr->Resolve();
	if (ModelFrameExpr)
		ModelFrameExpr = ModelFrameExpr->Resolve();
	if (TimeExpr)
		TimeExpr = TimeExpr->Resolve();

	if (!FrameExpr || !TimeExpr)
		return;
	if (!FrameExpr->GetIntConst(Frame))
		return;
	if (ModelFrameExpr && !ModelFrameExpr->GetIntConst(ModelFrame))
		return;
	if (!TimeExpr->GetFloatConst(Time))
		return;

	if (NextStateName != NAME_None)
	{
		NextState = CheckForState(NextStateName, (VClass*)Outer);
		if (!NextState)
		{
			ParseError(Loc, "No such state %s", *NextStateName);
		}
	}

	Function->Emit();
}

//==========================================================================
//
//	VClass::Emit
//
//==========================================================================

void VClass::Emit()
{
	//	Emit method code.
	for (int i = 0; i < Methods.Num(); i++)
	{
		Methods[i]->Emit();
	}

	//	Emit code of the state methods.
	for (VState* s = States; s; s = s->Next)
	{
		s->Emit();
	}

	DefaultProperties->Emit();
}

//==========================================================================
//
//	EmitCode
//
//==========================================================================

void EmitCode()
{
	for (int i = 0; i < PackagesToLoad.Num(); i++)
	{
		PackagesToLoad[i].Pkg = LoadPackage(PackagesToLoad[i].Name,
			PackagesToLoad[i].Loc);
	}

	if (NumErrors)
	{
		BailOut();
	}

	for (int i = 0; i < ParsedConstants.Num(); i++)
	{
		ParsedConstants[i]->Define();
	}

	for (int i = 0; i < ParsedStructs.Num(); i++)
	{
		ParsedStructs[i]->Define();
	}

	for (int i = 0; i < ParsedClasses.Num(); i++)
	{
		ParsedClasses[i]->Define();
	}

	if (NumErrors)
	{
		BailOut();
	}

	for (int i = 0; i < ParsedStructs.Num(); i++)
	{
		ParsedStructs[i]->DefineMembers();
	}

	for (int i = 0; i < ParsedClasses.Num(); i++)
	{
		ParsedClasses[i]->DefineMembers();
	}

	if (NumErrors)
	{
		BailOut();
	}

	for (int i = 0; i < ParsedClasses.Num(); i++)
	{
		ParsedClasses[i]->Emit();
	}

	if (NumErrors)
	{
		BailOut();
	}
}
