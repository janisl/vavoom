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

// HEADER FILES ------------------------------------------------------------

#include "vcc.h"

// MACROS ------------------------------------------------------------------

#define BASE_NUM_METHODS		1

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
	case ev_struct:		if (Struct->Size < 0) { ParseError("Incomplete type"); }
						return (Struct->Size + 3) & ~3;
	case ev_vector:		return 12;
	case ev_classid:	return 4;
	case ev_state:		return 4;
	case ev_bool:		return 4;
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
	if (type == ev_void)
	{
		ParseError(ERR_VOID_VALUE);
	}
	if (GetSize() != 4)
	{
		ParseError(ERR_EXPR_TYPE_MISTMATCH, "Size is not 4");
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
		if (TK_Check(KW_ADDFIELDS))
		{
			TK_NextToken();
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			continue;
		}
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
//	SkipAddFields
//
//==========================================================================

void SkipAddFields(VClass* InClass)
{
	TK_NextToken();

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
	VField*	fi = NULL;
	VField*	otherfield;
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

		if (TK_Check(KW_ADDFIELDS))
		{
			SkipAddFields(Class);
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
			for (fi = Class->Fields; fi; fi = fi->Next)
			{
				if (fi->Name == tk_Name)
				{
					break;
				}
			}
			if (!fi)
				ERR_Exit(ERR_NONE, true, "Method Field not found");
			otherfield = CheckForField(Class, false);
			if (!otherfield)
			{
				TK_NextToken();
			}
			if (TK_Check(PU_LPAREN))
			{
				CompileMethodDef(t, fi, otherfield, Class);
				need_semicolon = false;
				break;
			}
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

	for (fi = Class->Fields; fi; fi = fi->Next)
	{
		if (fi->type.type == ev_method &&
			fi->Name == NAME_None && fi->ofs == 0)
		{
			break;
		}
	}
	if (!fi)
		ERR_Exit(ERR_NONE, true, "DP Field not found");
	CompileDefaultProperties(fi, Class);
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
	if (InStruct->Size == -1)
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
//	ParseClassField
//
//==========================================================================

VField* ParseClassField(VClass* InClass)
{
	if (InClass->Size == -1)
	{
		ParseError("Incomplete type.");
		return NULL;
	}
	if (tk_Token != TK_IDENTIFIER)
	{
		ParseError(ERR_INVALID_IDENTIFIER, ", field name expacted");
		return NULL;
	}
	for (VField* fi = InClass->Fields; fi; fi = fi->Next)
	{
		if (TK_Check(fi->Name))
		{
			if (fi->flags & FIELD_Private && InClass != SelfClass)
			{
				ParseError("Field %s is private", *fi->Name);
			}
			return fi;
		}
	}
	if (InClass->ParentClass)
	{
		return ParseClassField(InClass->ParentClass);
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

VField* CheckForField(VClass* InClass, bool CheckPrivate)
{
	if (!InClass)
	{
		return NULL;
	}
	if (InClass->Size == -1)
	{
		return NULL;
	}
	if (tk_Token != TK_IDENTIFIER)
	{
		return NULL;
	}
	for (VField *fi = InClass->Fields; fi; fi = fi->Next)
	{
		if (TK_Check(fi->Name))
		{
			if (CheckPrivate && fi->flags & FIELD_Private &&
				InClass != SelfClass)
			{
				ParseError("Field %s is private", *fi->Name);
			}
			return fi;
		}
	}
	return CheckForField(InClass->ParentClass, CheckPrivate);
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
	if (InClass->Size == -1)
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
//	FindConstructor
//
//==========================================================================

VField* FindConstructor(VClass* InClass)
{
	if (!InClass)
	{
		return NULL;
	}
	if (InClass->Size == -1)
	{
		return NULL;
	}
	for (VField *fi = InClass->Fields; fi; fi = fi->Next)
	{
		if (fi->type.type == ev_method && fi->ofs == 0)
		{
			return fi;
		}
	}
	return FindConstructor(InClass->ParentClass);
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
	VField*	fi;
	int			size;
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
		if (Struct->Size != -1)
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
		Struct->IsVector = IsVector;
		//  Add to types
		TK_NextToken();
	}

	if (TK_Check(PU_SEMICOLON))
	{
		Struct->Size = -1;
		return;
	}

	Struct->Fields = NULL;
	size = 0;

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
			size = type.GetSize();
		}
	}

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		if (!IsVector && TK_Check(KW_ADDFIELDS))
		{
			if (Struct->AvailableSize)
			{
				ParseError("Addfields already defined");
			}
			if (tk_Token != TK_INTEGER)
			{
				ParseError("Field count expacted");
			}
			Struct->AvailableSize = tk_Number * 4;
			Struct->AvailableOfs = size;
			size += tk_Number * 4;
			TK_NextToken();
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			continue;
		}

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
					(dword)prevbool->type.bit_mask != 0x80000000)
				{
					fi->type = t;
					fi->type.bit_mask = prevbool->type.bit_mask << 1;
					fi->ofs = prevbool->ofs;
					Struct->AddField(fi);
					continue;
				}
			}
			fi->ofs = size;
			if (!IsVector)
			{
				t = ParsePropArrayDims(InClass, t);
			}
			size += t.GetSize();
			fi->type = t;
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

	Struct->Size = size;
}

//==========================================================================
//
//	AddFields
//
//==========================================================================

void AddFields(VClass* InClass)
{
	TType			struct_type;
	TType			type;
	VField*		fi;
	int				size;
	int				ofs;
	TType			t;

	//  Read type, to which fields will be added to.
	struct_type = CheckForType(InClass);
	if (struct_type.type == ev_unknown)
	{
		ParseError("Parent type expected.");
		return;
	}

	//  Check if it's a structure type
	if (struct_type.type != ev_struct)
	{
		ParseError("Parent must be a struct.");
		return;
	}

	VStruct* Struct = struct_type.Struct;

	//  Check if type has reserved memory for additional fields
	if (!Struct->AvailableSize)
	{
		ParseError("Parent type don't have available memory for additional fields.");
		return;
	}

	//  Read info
	size = Struct->AvailableSize;
	ofs = Struct->AvailableOfs;

	//	Add fields
	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		int Modifiers = TModifiers::Parse();
		Modifiers = TModifiers::Check(Modifiers, VField::AllowedModifiers);

		type = CheckForType(InClass);
		if (type.type == ev_unknown)
		{
			ParseError("Field type expected.");
			continue;
		}
		do
		{
			t = type;
			while (TK_Check(PU_ASTERISK))
			{
				t = MakePointerType(t);
			}
			if (t.type == ev_void)
			{
				ParseError("Field cannot have void type.");
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
					(dword)prevbool->type.bit_mask != 0x80000000)
				{
					fi->type = t;
					fi->type.bit_mask = prevbool->type.bit_mask << 1;
					fi->ofs = prevbool->ofs;
					Struct->AddField(fi);
					continue;
				}
			}
			fi->ofs = ofs;
			t = ParsePropArrayDims(InClass, t);
			size -= t.GetSize();
			ofs += t.GetSize();
			if (size < 0)
			{
				ParseError("Additional fields size overflow.");
			}
			fi->type = t;
			Struct->AddField(fi);
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	}
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);

	//	Renew TypeInfo
	Struct->AvailableSize = size;
	Struct->AvailableOfs = ofs;
}

//==========================================================================
//
//	ParseClass
//
//==========================================================================

void ParseClass()
{
	VField*			fi;
	VField*			otherfield;
	int					size;
	TType				t;
	TType				type;

	VClass* Class = CheckForClass();
	if (Class)
	{
		if (Class->Size != -1)
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
		TK_NextToken();
	}

	if (TK_Check(PU_SEMICOLON))
	{
		Class->Size = -1;
		return;
	}

	Class->Fields = NULL;
	Class->NumMethods = BASE_NUM_METHODS;
	size = 0;

	if (TK_Check(PU_COLON))
	{
		VClass* Parent = CheckForClass();
		if (!Parent)
		{
			ParseError("Parent class type expected");
		}
		else if (Parent->Size == -1)
		{
			ParseError("Incomplete parent class");
		}
		else
		{
			Class->ParentClass = Parent;
			Class->NumMethods = Parent->NumMethods;
			size = Parent->Size;
		}
	}
	else if (Class->Name != NAME_Object)
	{
		ParseError("Parent class expected");
	}

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

	Class->Size = size;
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

		if (TK_Check(KW_ADDFIELDS))
		{
			AddFields(Class);
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
			fi = new VField(tk_Name, Class, tk_Location);
			otherfield = CheckForField(Class, false);
			if (otherfield)
			{
				ParseError("Redeclared field");
			}
			else
			{
				TK_NextToken();
			}
			fi->ofs = size;
			size += 8;
			Class->AddField(fi);
			ParseDelegate(t, fi, otherfield, Class, Flags);
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
			fi = new VField(tk_Name, Class, tk_Location);
			otherfield = CheckForField(Class, false);
			if (!otherfield)
			{
				TK_NextToken();
			}
			if (TK_Check(PU_LPAREN))
			{
				ParseMethodDef(t, fi, otherfield, Class, Modifiers);
				Class->AddField(fi);
				need_semicolon = false;
				break;
			}
			if (otherfield)
			{
				ParseError("Redeclared field");
				continue;
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
					(dword)prevbool->type.bit_mask != 0x80000000)
				{
					fi->type = t;
					fi->type.bit_mask = prevbool->type.bit_mask << 1;
					fi->ofs = prevbool->ofs;
					Class->AddField(fi);
					continue;
				}
			}
			fi->ofs = size;
			t = ParsePropArrayDims(Class, t);
			size += t.GetSize();
			fi->type = t;
			Class->AddField(fi);
		} while (TK_Check(PU_COMMA));
		if (need_semicolon)
		{
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		}
	}

	fi = new VField(NAME_None, Class, tk_Location);
	ParseDefaultProperties(fi, Class);
	Class->AddField(fi);

	Class->Size = size;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.52  2006/03/26 13:06:49  dj_jl
//	Implemented support for modular progs.
//
//	Revision 1.51  2006/03/23 22:22:02  dj_jl
//	Hashing of members for faster search.
//	
//	Revision 1.50  2006/03/23 18:30:54  dj_jl
//	Use single list of all members, members tree.
//	
//	Revision 1.49  2006/03/13 21:24:21  dj_jl
//	Added support for read-only, private and transient fields.
//	
//	Revision 1.48  2006/03/12 20:04:50  dj_jl
//	States as objects, added state variable type.
//	
//	Revision 1.47  2006/03/10 19:31:55  dj_jl
//	Use serialisation for progs files.
//	
//	Revision 1.46  2006/02/28 19:17:20  dj_jl
//	Added support for constants.
//	
//	Revision 1.45  2006/02/27 21:23:55  dj_jl
//	Rewrote names class.
//	
//	Revision 1.44  2006/02/25 17:07:57  dj_jl
//	Linked list of fields, export all type info.
//	
//	Revision 1.43  2006/02/20 19:34:32  dj_jl
//	Created modifiers class.
//	
//	Revision 1.42  2006/02/19 20:37:02  dj_jl
//	Implemented support for delegates.
//	
//	Revision 1.41  2006/02/19 14:37:36  dj_jl
//	Changed type handling.
//	
//	Revision 1.40  2006/02/17 19:25:00  dj_jl
//	Removed support for progs global variables and functions.
//	
//	Revision 1.39  2006/02/15 23:27:07  dj_jl
//	Added script ID class attribute.
//	
//	Revision 1.38  2006/02/11 14:48:33  dj_jl
//	Fixed arrays also for structs.
//	
//	Revision 1.37  2006/02/11 14:44:35  dj_jl
//	Fixed multi-dimentional arrays.
//	
//	Revision 1.36  2005/12/14 20:53:23  dj_jl
//	State names belong to a class.
//	Structs and enums defined in a class.
//	
//	Revision 1.35  2005/12/12 20:58:47  dj_jl
//	Removed compiler limitations.
//	
//	Revision 1.34  2005/12/07 22:52:55  dj_jl
//	Moved compiler generated data out of globals.
//	
//	Revision 1.33  2005/11/29 19:31:43  dj_jl
//	Class and struct classes, removed namespaces, beautification.
//	
//	Revision 1.32  2005/11/24 20:42:05  dj_jl
//	Renamed opcodes, cleanup and improvements.
//	
//	Revision 1.31  2005/04/28 07:14:03  dj_jl
//	Fixed some warnings.
//	
//	Revision 1.30  2003/03/08 12:47:52  dj_jl
//	Code cleanup.
//	
//	Revision 1.29  2002/11/02 17:11:13  dj_jl
//	New style classes.
//	
//	Revision 1.28  2002/09/07 16:36:38  dj_jl
//	Support bool in function args and return type.
//	Removed support for typedefs.
//	
//	Revision 1.27  2002/08/24 14:45:38  dj_jl
//	2 pass compiling.
//	
//	Revision 1.26  2002/06/14 15:33:45  dj_jl
//	Some fixes.
//	
//	Revision 1.25  2002/05/03 17:04:03  dj_jl
//	Mangling of string pointers.
//	
//	Revision 1.24  2002/03/12 19:17:30  dj_jl
//	Added keyword abstract
//	
//	Revision 1.23  2002/02/26 17:52:20  dj_jl
//	Exporting special property info into progs.
//	
//	Revision 1.22  2002/02/16 16:28:36  dj_jl
//	Added support for bool variables
//	
//	Revision 1.21  2002/02/02 19:23:02  dj_jl
//	Natives declared inside class declarations.
//	
//	Revision 1.20  2002/01/21 18:23:09  dj_jl
//	Constructors with no names
//	
//	Revision 1.19  2002/01/17 18:19:52  dj_jl
//	New style of adding to mobjinfo, some fixes
//	
//	Revision 1.18  2002/01/15 18:29:36  dj_jl
//	no message
//	
//	Revision 1.17  2002/01/11 08:17:31  dj_jl
//	Added name subsystem, removed support for unsigned ints
//	
//	Revision 1.16  2002/01/07 12:31:36  dj_jl
//	Changed copyright year
//	
//	Revision 1.15  2001/12/27 17:44:02  dj_jl
//	Removed support for C++ style constructors and destructors, some fixes
//	
//	Revision 1.14  2001/12/18 19:09:41  dj_jl
//	Some extra info in progs and other small changes
//	
//	Revision 1.13  2001/12/12 19:22:22  dj_jl
//	Support for method usage as state functions, dynamic cast
//	Added dynamic arrays
//	
//	Revision 1.12  2001/12/03 19:25:44  dj_jl
//	Fixed calling of parent function
//	Added defaultproperties
//	Fixed vectors as arguments to methods
//	
//	Revision 1.11  2001/12/01 18:17:09  dj_jl
//	Fixed calling of parent method, speedup
//	
//	Revision 1.10  2001/11/09 14:42:29  dj_jl
//	References, beautification
//	
//	Revision 1.9  2001/10/27 07:54:59  dj_jl
//	Added support for constructors and destructors
//	
//	Revision 1.8  2001/10/09 17:31:55  dj_jl
//	Addfields to class disabled by default
//	
//	Revision 1.7  2001/10/02 17:40:48  dj_jl
//	Possibility to declare function's code inside class declaration
//	
//	Revision 1.6  2001/09/25 17:03:50  dj_jl
//	Added calling of parent functions
//	
//	Revision 1.5  2001/09/24 17:31:38  dj_jl
//	Some fixes
//	
//	Revision 1.4  2001/09/20 16:09:55  dj_jl
//	Added basic object-oriented support
//	
//	Revision 1.3  2001/08/21 17:52:54  dj_jl
//	Added support for real string pointers, beautification
//	
//	Revision 1.2  2001/07/27 14:27:56  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
