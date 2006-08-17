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
		ParseError("Not a pointer type");
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

TType MakeArrayType(const TType& type, int elcount)
{
	if (type.type == ev_array)
	{
		ParseError("Can't have multi-dimensional arrays");
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
		ParseError("Not a pointer type");
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
//	CheckForTypeKeyword
//
//==========================================================================

static TType CheckForTypeKeyword()
{
	if (TK_Check(KW_VOID))
	{
		return TType(ev_void);
	}
	if (TK_Check(KW_INT))
	{
		return TType(ev_int);
	}
	if (TK_Check(KW_FLOAT))
	{
		return TType(ev_float);
	}
	if (TK_Check(KW_NAME))
	{
		return TType(ev_name);
	}
	if (TK_Check(KW_STRING))
	{
		return TType(ev_string);
	}
	if (TK_Check(KW_CLASSID))
	{
		return TType(ev_classid);
	}
	if (TK_Check(KW_STATE))
	{
		return TType(ev_state);
	}
	if (TK_Check(KW_BOOL))
	{
		TType ret(ev_bool);
		ret.bit_mask = 1;
		return ret;
	}
	return TType(ev_unknown);
}

//==========================================================================
//
//  CheckForType
//
//==========================================================================

TType CheckForType(VClass* InClass)
{
	if (tk_Token == TK_KEYWORD)
	{
		return CheckForTypeKeyword();
	}
	if (tk_Token != TK_IDENTIFIER)
	{
		return TType(ev_unknown);
	}

	VMemberBase* m = VMemberBase::StaticFindMember(tk_Name, ANY_PACKAGE,
		MEMBER_Class);
	if (m)
	{
		TK_NextToken();
		return TType((VClass*)m);
	}
	m = VMemberBase::StaticFindMember(tk_Name, InClass ?
		(VMemberBase*)InClass : (VMemberBase*)ANY_PACKAGE, MEMBER_Struct);
	if (m)
	{
		TK_NextToken();
		return TType((VStruct*)m);
	}
	if (InClass)
	{
		return CheckForType(InClass->ParentClass);
	}
	return TType(ev_unknown);
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

VClass* CheckForClass()
{
	if (tk_Token == TK_KEYWORD)
	{
		return NULL;
	}

	VMemberBase* m = VMemberBase::StaticFindMember(tk_Name, ANY_PACKAGE,
		MEMBER_Class);
	if (m)
	{
		TK_NextToken();
		return (VClass*)m;
	}
	return NULL;
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

void TType::CheckPassable() const
{
	if (GetSize() != 4 && type != ev_vector && type != ev_delegate)
	{
		ParseError(ERR_EXPR_TYPE_MISTMATCH);
	}
}

//==========================================================================
//
//	TType::CheckSizeIs4
//
//  Checks if type size is 4
//
//==========================================================================

void TType::CheckSizeIs4() const
{
	switch (type)
	{
	case ev_int:
	case ev_float:
	case ev_name:
	case ev_bool:
		break;

	case ev_string:
		AddStatement(OPC_StrToBool);
		break;

	case ev_pointer:
	case ev_reference:
	case ev_classid:
	case ev_state:
		AddStatement(OPC_PtrToBool);
		break;

	default:
		ParseError(ERR_EXPR_TYPE_MISTMATCH, "Size is not 4");
	}
}

//==========================================================================
//
//	TType::CheckSizeIs4
//
//  Checks if type size is 4
//
//==========================================================================

void TType::CheckSizeIs4(TLocation Loc) const
{
	switch (type)
	{
	case ev_int:
	case ev_float:
	case ev_name:
	case ev_bool:
	case ev_string:
	case ev_pointer:
	case ev_reference:
	case ev_classid:
	case ev_state:
		break;

	default:
		ParseError(ERR_EXPR_TYPE_MISTMATCH, "Size is not 4");
	}
}

//==========================================================================
//
//	TType::EmitToBool
//
//==========================================================================

void TType::EmitToBool() const
{
	switch (type)
	{
	case ev_string:
		AddStatement(OPC_StrToBool);
		break;

	case ev_pointer:
	case ev_reference:
	case ev_classid:
	case ev_state:
		AddStatement(OPC_PtrToBool);
		break;
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

void TType::CheckMatch(const TType& Other) const
{
	CheckPassable();
	Other.CheckPassable();
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
			ParseError("Can't assign a static function to delegate");
		}
		if (!F1.ReturnType.Equals(F2.ReturnType))
		{
			ParseError("Delegate has different return type");
		}
		else if (F1.NumParams != F2.NumParams)
		{
			ParseError("Delegate has different number of arguments");
		}
		else for (int i = 0; i < F1.NumParams; i++)
			if (!F1.ParamTypes[i].Equals(F2.ParamTypes[i]))
			{
				ParseError("Delegate argument %d differs", i + 1);
			}
		return;
	}
	char Name1[256];
	char Name2[256];
	GetName(Name1);
	Other.GetName(Name2);
	ParseError(ERR_EXPR_TYPE_MISTMATCH, " Types %s and %s are not compatible %d %d",
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
//	SkipStruct
//
//==========================================================================

void SkipStruct(VClass* InClass)
{
	TK_NextToken();
	if (TK_Check(PU_SEMICOLON))
	{
		return;
	}

	if (TK_Check(PU_COLON))
	{
		TK_NextToken();
	}

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		TModifiers::Parse();
		TK_NextToken();
		do
		{
			while (TK_Check(PU_ASTERISK));
			TK_NextToken();
			if (TK_Check(PU_LINDEX))
			{
				EvalConstExpression(InClass, ev_int);
				TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
			}
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	}
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
}

//==========================================================================
//
//	CompileClass
//
//==========================================================================

void CompileClass()
{
	TType		t;
	TType		type;

	VClass* Class = CheckForClass();
	if (!Class)
	{
		ParseError("Not a class type");
		return;
	}

	if (TK_Check(PU_SEMICOLON))
	{
		return;
	}

	if (TK_Check(PU_COLON))
	{
		CheckForClass();
	}

	TModifiers::Parse();
	do
	{
		if (TK_Check(KW_MOBJINFO))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			EvalConstExpression(NULL, ev_int);
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
		}
		else if (TK_Check(KW_SCRIPTID))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			EvalConstExpression(NULL, ev_int);
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
		}
		else
		{
			break;
		}
	} while (1);

	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	while (!TK_Check(KW_DEFAULTPROPERTIES))
	{
		if (TK_Check(KW_STATES))
		{
			SkipStates(Class);
			continue;
		}

		if (TK_Check(KW_ENUM))
		{
			TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
			do
			{
				TK_NextToken();
				if (TK_Check(PU_ASSIGN))
				{
					EvalConstExpression(Class, ev_int);
				}
			} while (TK_Check(PU_COMMA));
			TK_Expect(PU_RBRACE, ERR_MISSING_RBRACE);
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			continue;
		}

		if (TK_Check(KW_CONST))
		{
			t = CheckForType(Class);
			do
			{
				TK_NextToken();
				if (!TK_Check(PU_ASSIGN))
					ParseError("Assignement operator expected");
				EvalConstExpression(Class, t.type);
			} while (TK_Check(PU_COMMA));
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			continue;
		}

		if (TK_Check(KW_STRUCT))
		{
			SkipStruct(Class);
			continue;
		}

		if (TK_Check(KW_VECTOR))
		{
			SkipStruct(Class);
			continue;
		}

		if (TK_Check(KW_DELEGATE))
		{
			SkipDelegate(Class);
			continue;
		}

		TModifiers::Parse();

		type = CheckForType(Class);
		if (type.type == ev_unknown)
		{
			ParseError("Field type expected.");
		}

		bool need_semicolon = true;
		do
		{
			t = type;
			while (TK_Check(PU_ASTERISK))
			{
				t = MakePointerType(t);
			}
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError("Field name expected");
				continue;
			}
			VName FieldName = tk_Name;
			TK_NextToken();

			if (TK_Check(PU_LPAREN))
			{
				VMethod* M = (VMethod*)VMemberBase::StaticFindMember(
					FieldName, Class, MEMBER_Method);
				if (!M)
					ERR_Exit(ERR_NONE, true, "Method Field not found");
				CompileMethodDef(t, M, Class);
				need_semicolon = false;
				break;
			}

			VField* fi = NULL;
			for (fi = Class->Fields; fi; fi = fi->Next)
			{
				if (fi->Name == FieldName)
				{
					break;
				}
			}
			if (!fi)
				ERR_Exit(ERR_NONE, true, "Method Field not found");
			if (TK_Check(PU_LINDEX))
			{
				EvalConstExpression(Class, ev_int);
				TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
			}
		} while (TK_Check(PU_COMMA));
		if (need_semicolon)
		{
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		}
	}

	CompileDefaultProperties(Class->DefaultProperties, Class);
}

//==========================================================================
//
//	ParseStructField
//
//==========================================================================

VField* ParseStructField(VStruct* InStruct)
{
	if (!InStruct)
	{
		ParseError(ERR_NOT_A_STRUCT, "Base type required.");
		return NULL;
	}
	if (!InStruct->Parsed)
	{
		ParseError("Incomplete type.");
		return NULL;
	}
	if (tk_Token != TK_IDENTIFIER)
	{
		ParseError(ERR_INVALID_IDENTIFIER, ", field name expacted");
		return NULL;
	}
	for (VField* fi = InStruct->Fields; fi; fi = fi->Next)
	{
		if (TK_Check(fi->Name))
		{
			return fi;
		}
	}
	if (InStruct->ParentStruct)
	{
		return ParseStructField(InStruct->ParentStruct);
	}
	ParseError(ERR_NOT_A_FIELD, "Identifier: %s", *tk_Name);
	if (tk_Token == TK_IDENTIFIER)
	{
		TK_NextToken();
	}
	return NULL;
}

//==========================================================================
//
//	CheckForField
//
//==========================================================================

VField* CheckForField(VName Name, VClass* InClass, bool CheckPrivate)
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
				ParseError("Field %s is private", *fi->Name);
			}
			return fi;
		}
	}
	return CheckForField(Name, InClass->ParentClass, CheckPrivate);
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

//**************************************************************************
//**
//**
//**
//**************************************************************************

//==========================================================================
//
//	ParsePropArrayDims
//
//==========================================================================

static TType ParsePropArrayDims(VClass* Class, const TType& t)
{
	if (TK_Check(PU_LINDEX))
	{
		int i = EvalConstExpression(Class, ev_int);
		TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
		return MakeArrayType(t, i);
	}
	return t;
}

//==========================================================================
//
//	VStruct::AddField
//
//==========================================================================

void VStruct::AddField(VField* f)
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
//	ParseStruct
//
//==========================================================================

void ParseStruct(VClass* InClass, bool IsVector)
{
	VField*		fi;
	TType		t;
	TType		type;
	TType		struct_type;
	VStruct*	Struct;

	struct_type = CheckForType(InClass);
	if (struct_type.type != ev_unknown)
	{
		if (struct_type.type != (IsVector ? ev_vector : ev_struct))
		{
			ParseError(IsVector ? "Not a vector type" : "Not a struct type");
			return;
		}
		Struct = struct_type.Struct;
		if (Struct->Parsed)
		{
			ParseError("Struct type already completed");
			return;
		}
		Struct->Loc = tk_Location;
	}
	else
	{
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError("Struct name expected");
		}
		//	New struct
		Struct = new VStruct(tk_Name, InClass ? (VMemberBase*)InClass :
			(VMemberBase*)CurrentPackage, tk_Location);
		Struct->Parsed = false;
		Struct->IsVector = IsVector;
		//  Add to types
		TK_NextToken();
	}

	if (TK_Check(PU_SEMICOLON))
	{
		return;
	}

	int size = 0;
	Struct->Fields = NULL;

	if (!IsVector && TK_Check(PU_COLON))
	{
		type = CheckForType(InClass);
		if (type.type == ev_unknown)
		{
			ParseError("Parent type expected");
		}
		else if (type.type != ev_struct)
		{
			ParseError("Parent type must be a struct");
		}
		else
		{
			Struct->ParentStruct = type.Struct;
			size = Struct->ParentStruct->StackSize * 4;
		}
	}

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		int Modifiers = TModifiers::Parse();
		Modifiers = TModifiers::Check(Modifiers, VField::AllowedModifiers);

		type = CheckForType(InClass);
		if (type.type == ev_unknown)
		{
			ParseError("Field type expected.");
		}
		if (IsVector && type.type != ev_float)
		{
			ParseError("Vector can have only float fields");
			continue;
		}
		do
		{
			t = type;
			if (!IsVector)
			{
				while (TK_Check(PU_ASTERISK))
				{
					t = MakePointerType(t);
				}
			}
			if (t.type == ev_void)
			{
				ParseError("Field cannot have void type.");
			}
			if (IsVector)
			{
				int fc = 0;
				for (VField* f = Struct->Fields; f; f = f->Next)
					fc++;
				if (fc == 3)
				{
					ParseError("Vector must have exactly 3 float fields");
					continue;
				}
			}
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError("Field name expected");
			}
			fi = new VField(tk_Name, Struct, tk_Location);
			fi->flags = TModifiers::FieldAttr(Modifiers);
			TK_NextToken();
			if (t.type == ev_bool && Struct->Fields)
			{
				VField* prevbool = Struct->Fields;
				while (prevbool->Next)
					prevbool = prevbool->Next;
				if (prevbool->type.type == ev_bool &&
					(vuint32)prevbool->type.bit_mask != 0x80000000)
				{
					fi->type = t;
					fi->type.bit_mask = prevbool->type.bit_mask << 1;
					Struct->AddField(fi);
					continue;
				}
			}
			if (!IsVector)
			{
				t = ParsePropArrayDims(InClass, t);
			}
			fi->type = t;
			size += t.GetSize();
			Struct->AddField(fi);
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	}
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	if (IsVector)
	{
		int fc = 0;
		for (VField* f = Struct->Fields; f; f = f->Next)
			fc++;
		if (fc != 3)
		{
			ParseError("Vector must have exactly 3 float fields");
		}
	}
	Struct->StackSize = (size + 3) / 4;
	Struct->Parsed = true;
}

//==========================================================================
//
//	ParseClass
//
//==========================================================================

void ParseClass()
{
	TType				t;
	TType				type;

	VClass* Class = CheckForClass();
	if (Class)
	{
		if (Class->Parsed)
		{
			ParseError("Class definition already completed");
			return;
		}
		Class->Loc = tk_Location;
	}
	else
	{
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError("Class name expected");
		}
		//	New class.
		Class = new VClass(tk_Name, CurrentPackage, tk_Location);
		Class->Parsed = false;
		TK_NextToken();
	}

	if (TK_Check(PU_SEMICOLON))
	{
		return;
	}

	Class->Fields = NULL;

	if (TK_Check(PU_COLON))
	{
		VClass* Parent = CheckForClass();
		if (!Parent)
		{
			ParseError("Parent class type expected");
		}
		else if (!Parent->Parsed)
		{
			ParseError("Incomplete parent class");
		}
		else
		{
			Class->ParentClass = Parent;
		}
	}
	else if (Class->Name != NAME_Object)
	{
		ParseError("Parent class expected");
	}

	Class->Parsed = true;

	int ClassModifiers = TModifiers::Parse();
	ClassModifiers = TModifiers::Check(ClassModifiers, VClass::AllowedModifiers);
	int ClassAttr = TModifiers::ClassAttr(ClassModifiers);
	do
	{
		if (TK_Check(KW_MOBJINFO))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			AddToMobjInfo(EvalConstExpression(NULL, ev_int), Class);
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
		}
		else if (TK_Check(KW_SCRIPTID))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			AddToScriptIds(EvalConstExpression(NULL, ev_int), Class);
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
		}
		else
		{
			break;
		}
	} while (1);

	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	while (!TK_Check(KW_DEFAULTPROPERTIES))
	{
		if (TK_Check(KW_STATES))
		{
			ParseStates(Class);
			continue;
		}

		if (TK_Check(KW_ENUM))
		{
			int val;
			VName Name;

			val = 0;
			TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
			do
			{
				if (tk_Token != TK_IDENTIFIER)
				{
					ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
				}
				Name = tk_Name;
				TK_NextToken();
				if (TK_Check(PU_ASSIGN))
				{
					val = EvalConstExpression(Class, ev_int);
				}
				AddConstant(Class, Name, ev_int, val);
				val++;
			} while (TK_Check(PU_COMMA));
			TK_Expect(PU_RBRACE, ERR_MISSING_RBRACE);
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			continue;
		}

		if (TK_Check(KW_CONST))
		{
			t = CheckForType(Class);
			do
			{
				if (tk_Token != TK_IDENTIFIER)
				{
					ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
				}
				if (t.type != ev_int && t.type != ev_float)
				{
					ParseError("Unsupported type of constant");
				}
				VName Name = tk_Name;
				TK_NextToken();
				if (!TK_Check(PU_ASSIGN))
					ParseError("Assignement operator expected");
				int val = EvalConstExpression(Class, t.type);
				AddConstant(Class, Name, t.type, val);
			} while (TK_Check(PU_COMMA));
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			continue;
		}

		if (TK_Check(KW_STRUCT))
		{
			ParseStruct(Class, false);
			continue;
		}

		if (TK_Check(KW_VECTOR))
		{
			ParseStruct(Class, true);
			continue;
		}

		if (TK_Check(KW_DELEGATE))
		{
			int Flags = 0;
	
			t = CheckForType(Class);
			if (t.type == ev_unknown)
			{
				ParseError("Field type expected.");
			}
			while (TK_Check(PU_ASTERISK))
			{
				t = MakePointerType(t);
			}
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError("Field name expected");
				continue;
			}
			VField* fi = new VField(tk_Name, Class, tk_Location);
			if (CheckForField(tk_Name, Class, false) ||
				CheckForMethod(tk_Name, Class))
			{
				ParseError("Redeclared field");
			}
			TK_NextToken();
			Class->AddField(fi);
			ParseDelegate(t, fi, Class, Flags);
			continue;
		}

		int Modifiers = TModifiers::Parse();

		type = CheckForType(Class);
		if (type.type == ev_unknown)
		{
			ParseError("Field type expected.");
		}

		bool need_semicolon = true;
		do
		{
			t = type;
			while (TK_Check(PU_ASTERISK))
			{
				t = MakePointerType(t);
			}
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError("Field name expected");
				continue;
			}
			VName FieldName = tk_Name;
			TLocation FieldLoc = tk_Location;
			TK_NextToken();
			VMethod* BaseMethod = CheckForMethod(FieldName, Class);
			if (CheckForField(FieldName, Class, false))
			{
				ParseError("Redeclared field");
				continue;
			}

			if (TK_Check(PU_LPAREN))
			{
				ParseMethodDef(t, FieldName, FieldLoc, BaseMethod, Class,
					Modifiers);
				need_semicolon = false;
				break;
			}
			VField* fi = new VField(FieldName, Class, FieldLoc);
			if (BaseMethod)
			{
				ParseError("Redeclared identifier");
			}
			if (t.type == ev_void)
			{
				ParseError("Field cannot have void type.");
			}
			Modifiers = TModifiers::Check(Modifiers, VField::AllowedModifiers);
			fi->flags = TModifiers::FieldAttr(Modifiers);
			if (t.type == ev_bool && Class->Fields)
			{
				VField* prevbool = Class->Fields;
				while (prevbool->Next)
					prevbool = prevbool->Next;
				if (prevbool->type.type == ev_bool &&
					(vuint32)prevbool->type.bit_mask != 0x80000000)
				{
					fi->type = t;
					fi->type.bit_mask = prevbool->type.bit_mask << 1;
					Class->AddField(fi);
					continue;
				}
			}
			t = ParsePropArrayDims(Class, t);
			fi->type = t;
			Class->AddField(fi);
		} while (TK_Check(PU_COMMA));
		if (need_semicolon)
		{
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		}
	}

	ParseDefaultProperties(Class);
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
