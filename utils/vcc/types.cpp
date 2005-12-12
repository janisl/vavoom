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

enum
{
	PROPTYPE_Reference,
	PROPTYPE_ClassID,
	PROPTYPE_Name,
	PROPTYPE_String,
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

TType		type_void(ev_void, NULL, NULL);
TType		type_int(ev_int, &type_void, NULL);
TType		type_float(ev_float, &type_int, NULL);
TType		type_name(ev_name, &type_float, NULL);
TType		type_string(ev_string, &type_name, NULL);
TType		type_void_ptr(ev_pointer, &type_string, &type_void);
TType		type_vector(ev_vector, &type_void_ptr, NULL);
TType		type_classid(ev_classid, &type_vector, NULL);
TType		type_none_ref(ev_reference, &type_classid, NULL);
TType		type_bool(ev_bool, &type_none_ref, NULL);

TType		*types = &type_bool;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	InitTypes
//
//==========================================================================

void InitTypes()
{
	type_bool.bit_mask = 1;
}

//==========================================================================
//
//	FindType
//
//	Searches given type in type list. If not found, creates a new one.
//
//==========================================================================

TType *FindType(TType *type)
{
	TType	*check;
	TType	*newtype;

	for (check = types; check; check = check->next)
	{
		//	Check main params
		if (type->type != check->type ||
			type->aux_type != check->aux_type ||
			type->array_dim != check->array_dim ||
			type->bit_mask != check->bit_mask ||
			type->Class != check->Class)
			continue;
		return check;
	}

	//	Not found, create a new one
	newtype = new TType;
	*newtype = *type;
	newtype->next = types;
	types = newtype;
	return newtype;
}

//==========================================================================
//
//	MakePointerType
//
//==========================================================================

TType *MakePointerType(TType *type)
{
	TType	pointer;

	memset(&pointer, 0, sizeof(TType));
	pointer.type = ev_pointer;
	pointer.aux_type = type;
	return FindType(&pointer);
}

//==========================================================================
//
//	MakeReferenceType
//
//==========================================================================

TType *MakeReferenceType(TClass* InClass)
{
	TType reference;

	memset(&reference, 0, sizeof(TType));
	reference.type = ev_reference;
	reference.Class = InClass;
	return FindType(&reference);
}

//==========================================================================
//
//	MakeArrayType
//
//==========================================================================

TType *MakeArrayType(TType *type, int elcount)
{
	TType	array;

	memset(&array, 0, sizeof(TType));
	array.type = ev_array;
	array.aux_type = type;
	array.array_dim = elcount;
	return FindType(&array);
}

//==========================================================================
//
//	CheckForTypeKeyword
//
//==========================================================================

static TType *CheckForTypeKeyword(void)
{
	if (TK_Check(KW_VOID))
	{
		return &type_void;
	}
	if (TK_Check(KW_INT))
	{
		return &type_int;
	}
	if (TK_Check(KW_FLOAT))
	{
		return &type_float;
	}
	if (TK_Check(KW_NAME))
	{
		return &type_name;
	}
	if (TK_Check(KW_STRING))
	{
		return &type_string;
	}
	if (TK_Check(KW_CLASSID))
	{
		return &type_classid;
	}
	if (TK_Check(KW_BOOL))
	{
		return &type_bool;
	}
	return NULL;
}

//==========================================================================
//
//  CheckForType
//
//==========================================================================

TType *CheckForType()
{
	if (tk_Token == TK_KEYWORD)
	{
		return CheckForTypeKeyword();
	}

	for (int i = 0; i < classtypes.Num(); i++)
	{
		if (TK_Check(classtypes[i]->Name))
		{
			return MakeReferenceType(classtypes[i]);
		}
	}
	for (TType* check = types; check; check = check->next)
	{
		if (check->Name != NAME_None)
		{
			if (TK_Check(check->Name))
			{
				return check;
			}
		}
	}
	return NULL;
}

//==========================================================================
//
//  CheckForType
//
//==========================================================================

TType *CheckForType(FName Name)
{
	if (Name == NAME_None)
	{
		return NULL;
	}

	for (int i = 0; i < classtypes.Num(); i++)
	{
		if (Name == classtypes[i]->Name)
		{
			return MakeReferenceType(classtypes[i]);
		}
	}
	for (TType* check = types; check; check = check->next)
	{
		if (check->Name != NAME_None)
		{
			if (Name == check->Name)
			{
				return check;
			}
		}
	}
	return NULL;
}

//==========================================================================
//
//  CheckForClass
//
//==========================================================================

TClass* CheckForClass()
{
	if (tk_Token == TK_KEYWORD)
	{
		return NULL;
	}

	for (int i = 0; i < classtypes.Num(); i++)
	{
		if (TK_Check(classtypes[i]->Name))
		{
			return classtypes[i];
		}
	}
	return NULL;
}

//==========================================================================
//
//  CheckForClass
//
//==========================================================================

TClass* CheckForClass(FName Name)
{
	for (int i = 0; i < classtypes.Num(); i++)
	{
		if (classtypes[i]->Name == Name)
		{
			return classtypes[i];
		}
	}
	return NULL;
}

//==========================================================================
//
//	TypeSize
//
//==========================================================================

int TypeSize(TType *type)
{
	switch (type->type)
	{
	case ev_int:		return 4;
	case ev_float:		return 4;
	case ev_name:		return 4;
	case ev_string:		return 4;
	case ev_pointer:	return 4;
	case ev_reference:	return 4;
	case ev_array:		return type->array_dim * TypeSize(type->aux_type);
	case ev_struct:		if (type->Struct->Size < 0) { ParseError("Incomplete type"); }
						return (type->Struct->Size + 3) & ~3;
	case ev_vector:		return 12;
	case ev_classid:	return 4;
	case ev_bool:		return 4;
	}
	return 0;
}

//==========================================================================
//
//	CheckForGlobalVar
//
//==========================================================================

int CheckForGlobalVar(FName Name)
{
	if (Name == NAME_None)
	{
		return -1;
	}
	for (int i = 0; i < globaldefs.Num(); i++)
	{
		if (globaldefs[i].Name == Name)
		{
			return i;
		}
	}
	return -1;
}

//==========================================================================
//
//	CheckForFunction
//
//==========================================================================

int CheckForFunction(TClass* InClass, FName Name)
{
	if (Name == NAME_None)
	{
		return -1;
	}
	for (int i = 0; i < functions.Num(); i++)
	{
		if (functions[i].OuterClass == InClass && functions[i].Name == Name)
		{
			return i;
		}
	}
	return -1;
}

//==========================================================================
//
//	CheckForConstant
//
//==========================================================================

int CheckForConstant(FName Name)
{
#if 1
	for (int C = ConstantsHash[GetTypeHash(Name) & 255];
		C != -1; C = Constants[C].HashNext)
	{
		if (Constants[C].Name == Name)
		{
			return C;
		}
	}
#else
	for (int i = 0; i < numconstants; i++)
	{
		if (Constants[i].Name == Name)
		{
			return i;
		}
	}
#endif
	return -1;
}

//==========================================================================
//
//	TypeCheckPassable
//
//	Check, if type can be pushed into the stack
//
//==========================================================================

void TypeCheckPassable(TType *type)
{
	if (TypeSize(type) != 4 && type->type != ev_vector)
	{
		ParseError(ERR_EXPR_TYPE_MISTMATCH);
	}
}

//==========================================================================
//
//	TypeCheck1
//
//  Checks if type size is 4
//
//==========================================================================

void TypeCheck1(TType *t)
{
	if (t == &type_void)
	{
		ParseError(ERR_VOID_VALUE);
	}
	if (TypeSize(t) != 4)
	{
		ParseError(ERR_EXPR_TYPE_MISTMATCH, "Size is not 4");
	}
}

//==========================================================================
//
//	TypeCheck3
//
//	Check, if types are compatible
//
//	t1 - current type
//	t2 - needed type
//
//==========================================================================

void TypeCheck3(TType *t1, TType *t2)
{
	TypeCheckPassable(t1);
	TypeCheckPassable(t2);
	if (t1 == t2)
	{
		return;
	}
	if ((t1->type == ev_vector) && (t2->type == ev_vector))
	{
		return;
	}
	if ((t1->type == ev_pointer) && (t2->type == ev_pointer))
	{
		t1 = t1->aux_type;
		t2 = t2->aux_type;
		if (t1 == t2)
		{
			return;
		}
		if ((t1 == &type_void) || (t2 == &type_void))
		{
			return;
		}
		if (t1->type == ev_struct && t2->type == ev_struct)
		{
			TStruct* s1 = t1->Struct;
			TStruct* s2 = t2->Struct;
			for (TStruct* st1 = s1->ParentStruct; st1; st1 = st1->ParentStruct)
			{
				if (st1 == s2)
				{
					return;
				}
			}
		}
	}
	if ((t1->type == ev_reference) && (t2->type == ev_reference))
	{
		TClass* c1 = t1->Class;
		TClass* c2 = t2->Class;
		if (!c1 || !c2)
		{
			//	none reference can be assigned to any reference.
			return;
		}
		if (c1 == c2)
		{
			return;
		}
		if ((c1 == classtypes[0]) || (c2 == classtypes[0]))
		{
			return;
		}
		for (TClass* pc1 = c1->ParentClass; pc1; pc1 = pc1->ParentClass)
		{
			if (pc1 == c2)
			{
				return;
			}
		}
	}
	if ((t1->type == ev_int && t2->type == ev_bool))
	{
		return;
	}
	ParseError(ERR_EXPR_TYPE_MISTMATCH, " Types %s and %s are not compatible %d %d",
		*t1->Name, *t2->Name, t1->type, t2->type);
}

//==========================================================================
//
//	SkipStruct
//
//==========================================================================

void SkipStruct()
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
		TK_NextToken();
		do
		{
			while (TK_Check(PU_ASTERISK));
			TK_NextToken();
			while (TK_Check(PU_LINDEX))
			{
				EvalConstExpression(ev_int);
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

void SkipAddFields()
{
	TK_NextToken();

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		TK_NextToken();
		do
		{
			while (TK_Check(PU_ASTERISK));
			TK_NextToken();
			while (TK_Check(PU_LINDEX))
			{
				EvalConstExpression(ev_int);
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
	field_t		*fi = NULL;
	field_t		*otherfield;
	int			i;
	TType		*t;
	TType		*type;

	TClass* Class = CheckForClass();
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
		CheckForType();
	}

	do
	{
		if (TK_Check(KW_MOBJINFO))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			EvalConstExpression(ev_int);
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
		}
		else if (TK_Check(KW_NATIVE))
		{
		}
		else if (TK_Check(KW_ABSTRACT))
		{
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

		int Flags = 0;
		bool flags_done = false;
		do
		{
			if (TK_Check(KW_NATIVE))
			{
				Flags |= FUNC_Native;
			}
			else
			{
				flags_done = true;
			}
		} while (!flags_done);

		type = CheckForType();
		if (!type)
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
			for (i = 0; i < Class->NumFields; i++)
			{
				fi = &Class->Fields[i];
				if (fi->Name == tk_Name)
				{
					break;
				}
			}
			if (i == Class->NumFields)
				ERR_Exit(ERR_NONE, true, "Method Field not found");
			otherfield = CheckForField(Class);
			if (!otherfield)
			{
				TK_NextToken();
			}
			if (TK_Check(PU_LPAREN))
			{
				CompileMethodDef(t, fi, otherfield, Class, Flags);
				need_semicolon = false;
				break;
			}
			while (TK_Check(PU_LINDEX))
			{
				EvalConstExpression(ev_int);
				TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
			}
		} while (TK_Check(PU_COMMA));
		if (need_semicolon)
		{
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		}
	}

	for (i = 0; i < Class->NumFields; i++)
	{
		fi = &Class->Fields[i];
		if (fi->type->type == ev_method &&
			fi->Name == NAME_None && fi->ofs == 0)
		{
			break;
		}
	}
	if (i == Class->NumFields)
		ERR_Exit(ERR_NONE, true, "DP Field not found");
	CompileDefaultProperties(fi, Class);
}

//==========================================================================
//
//	ParseStructField
//
//==========================================================================

field_t* ParseStructField(TStruct* InStruct)
{
	field_t*	fi;
	int			numfields;
	int			i;

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
	numfields = InStruct->NumFields;
	fi = InStruct->Fields;
	for (i = 0; i < numfields; i++)
	{
		if (TK_Check(fi[i].Name))
		{
			return &fi[i];
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

field_t* ParseClassField(TClass* InClass)
{
	field_t*	fi;
	int			numfields;
	int			i;

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
	numfields = InClass->NumFields;
	fi = InClass->Fields;
	for (i = 0; i < numfields; i++)
	{
		if (TK_Check(fi[i].Name))
		{
			return &fi[i];
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

field_t* CheckForField(TClass* InClass, bool check_aux)
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
	field_t *fi = InClass->Fields;
	for (int i = 0; i < InClass->NumFields; i++)
	{
		if (TK_Check(fi[i].Name))
		{
			return &fi[i];
		}
	}
	if (check_aux)
	{
		return CheckForField(InClass->ParentClass);
	}
	return NULL;
}

//==========================================================================
//
//	CheckForField
//
//==========================================================================

field_t* CheckForField(FName Name, TClass* InClass, bool check_aux)
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
	field_t *fi = InClass->Fields;
	for (int i = 0; i < InClass->NumFields; i++)
	{
		if (Name == fi[i].Name)
		{
			return &fi[i];
		}
	}
	if (check_aux)
	{
		return CheckForField(Name, InClass->ParentClass);
	}
	return NULL;
}

//==========================================================================
//
//	FindConstructor
//
//==========================================================================

field_t* FindConstructor(TClass* InClass)
{
	if (!InClass)
	{
		return NULL;
	}
	if (InClass->Size == -1)
	{
		return NULL;
	}
	field_t *fi = InClass->Fields;
	for (int i = 0; i < InClass->NumFields; i++)
	{
		if (fi[i].type->type == ev_method && fi[i].ofs == 0)
		{
			return &fi[i];
		}
	}
	return FindConstructor(InClass->ParentClass);
}

//==========================================================================
//
//	AddVTable
//
//==========================================================================

static void AddVTable(TClass* InClass)
{
	if (InClass->VTable)
	{
		return;
	}
	if (InClass->ParentClass)
	{
		AddVTable(InClass->ParentClass);
	}
	InClass->VTable = vtables.Num();
	int *vtable = &vtables[vtables.Add(InClass->NumMethods)];
	memset(vtable, 0, InClass->NumMethods * 4);
	if (InClass->ParentClass)
	{
		memcpy(vtable, &vtables[InClass->ParentClass->VTable],
			InClass->ParentClass->NumMethods * 4);
	}
	for (int i = 0; i < InClass->NumFields; i++)
	{
		field_t &f = InClass->Fields[i];
		if (f.type->type != ev_method)
		{
			continue;
		}
		if (f.func_num == -1)
		{
			ParseError("Method %s.%s not defined", *InClass->Name, *f.Name);
		}
		vtable[f.ofs] = f.func_num;
	}
	if (!vtable[0])
	{
		ERR_Exit(ERR_NONE, false, "Missing defaultproperties for %s", *InClass->Name);
	}
}

//==========================================================================
//
//	WritePropertyField
//
//==========================================================================

static void WritePropertyField(TClass* InClass, TType *type, int ofs)
{
	int i;

	switch (type->type)
	{
	case ev_void:
	case ev_int:
	case ev_float:
	case ev_bool:
		break;
	case ev_name:
		propinfos.AddItem(TPropInfo(PROPTYPE_Name, ofs));
		InClass->NumProperties++;
		break;
	case ev_string:
		propinfos.AddItem(TPropInfo(PROPTYPE_String, ofs));
		InClass->NumProperties++;
		break;
	case ev_pointer:	// FIXME
		break;
	case ev_reference:
		propinfos.AddItem(TPropInfo(PROPTYPE_Reference, ofs));
		InClass->NumProperties++;
		break;
	case ev_array:
		for (i = 0; i < TypeSize(type) / TypeSize(type->aux_type); i++)
		{
			WritePropertyField(InClass, type->aux_type,
				ofs + i * TypeSize(type->aux_type));
		}
		break;
	case ev_struct:
	case ev_vector:
		for (i = 0; i < type->Struct->NumFields; i++)
		{
			WritePropertyField(InClass, type->Struct->Fields[i].type,
				ofs + type->Struct->Fields[i].ofs);
		}
	case ev_method:		// Properties are not methods
		break;
	case ev_classid:
		propinfos.AddItem(TPropInfo(PROPTYPE_ClassID, ofs));
		InClass->NumProperties++;
		break;
	}
}

//==========================================================================
//
//	WritePropertyInfo
//
//==========================================================================

static void WritePropertyInfo(TClass* InClass)
{
	InClass->OfsProperties = propinfos.Num();
	InClass->NumProperties = 0;
	for (int i = 0; i < InClass->NumFields; i++)
	{
		WritePropertyField(InClass, InClass->Fields[i].type, InClass->Fields[i].ofs);
	}
}

//==========================================================================
//
//	AddVirtualTables
//
//==========================================================================

void AddVirtualTables()
{
	dprintf("Adding virtual tables\n");
	int i;
	for (i = 0; i < classtypes.Num(); i++)
	{
		AddVTable(classtypes[i]);
	}
	for (i = 0; i < classtypes.Num(); i++)
	{
		WritePropertyInfo(classtypes[i]);
	}
}

//**************************************************************************
//**
//**
//**
//**************************************************************************

//==========================================================================
//
//	ParseStruct
//
//==========================================================================

void ParseStruct(bool IsVector)
{
	field_t		fields[128];
	field_t		*fi;
	int			num_fields;
	int			size;
	int			i;
	TType		*t;
	TType		*type;
	TType		*struct_type;
	TStruct*	Struct;

	struct_type = CheckForType();
	if (struct_type)
	{
		if (struct_type->type != (IsVector ? ev_vector : ev_struct))
		{
			ParseError(IsVector ? "Not a vector type" : "Not a struct type");
			return;
		}
		Struct = struct_type->Struct;
		if (Struct->Size != -1)
		{
			ParseError("Struct type already completed");
			return;
		}
	}
	else
	{
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError("Struct name expected");
		}
		//	New struct
		Struct = new TStruct;
		structtypes.AddItem(Struct);
		Struct->Name = tk_Name;
		//  Add to types
		struct_type = new TType;
		memset(struct_type, 0, sizeof(TType));
		struct_type->Name = tk_Name;
		struct_type->type = IsVector ? ev_vector : ev_struct;
		struct_type->next = types;
		struct_type->Struct = Struct;
		Struct->Type = struct_type;
		types = struct_type;
		TK_NextToken();
	}

	if (TK_Check(PU_SEMICOLON))
	{
		Struct->Size = -1;
		return;
	}

	num_fields = 0;
	size = 0;

	if (!IsVector && TK_Check(PU_COLON))
	{
		type = CheckForType();
		if (!type)
		{
			ParseError("Parent type expected");
		}
		else if (type->type != ev_struct)
		{
			ParseError("Parent type must be a struct");
		}
		else
		{
			struct_type->aux_type = type;
			Struct->ParentStruct = type->Struct;
			size = TypeSize(type);
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
		type = CheckForType();
		if (!type)
		{
			ParseError("Field type expected.");
		}
		if (IsVector && type != &type_float)
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
			if (t == &type_void)
			{
				ParseError("Field cannot have void type.");
			}
			if (IsVector && num_fields == 3)
			{
				ParseError("Vector must have exactly 3 float fields");
				continue;
			}
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError("Field name expected");
			}
			fi = &fields[num_fields];
			fi->Name = tk_Name;
			TK_NextToken();
			if (t->type == ev_bool && num_fields)
			{
				field_t &prevbool = fields[num_fields - 1];
				if (prevbool.type->type == ev_bool &&
					(dword)prevbool.type->bit_mask != 0x80000000)
				{
					TType btype;

					memcpy(&btype, t, sizeof(TType));
					btype.bit_mask = prevbool.type->bit_mask << 1;
					fi->type = FindType(&btype);
					fi->ofs = prevbool.ofs;
					num_fields++;
					continue;
				}
			}
			fi->ofs = size;
			if (!IsVector)
			{
				while (TK_Check(PU_LINDEX))
				{
					i = EvalConstExpression(ev_int);
					TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
					t = MakeArrayType(t, i);
				}
			}
			size += TypeSize(t);
			fi->type = t;
			num_fields++;
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	}
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	if (IsVector && num_fields != 3)
	{
		ParseError("Vector must have exactly 3 float fields");
	}

	//	Add to the type
	Struct->Fields = new field_t[num_fields];
	memcpy(Struct->Fields, fields, num_fields * sizeof(*fields));
	Struct->NumFields = num_fields;
	Struct->Size = size;
}

//==========================================================================
//
//	AddFields
//
//==========================================================================

void AddFields()
{
	TType			*struct_type;
	TType			*type;
	field_t			*fi;
	int				num_fields;
	field_t			fields[128];
	int				size;
	int				ofs;
	int				i;
	TType			*t;

	//  Read type, to which fields will be added to.
	struct_type = CheckForType();
	if (!struct_type)
	{
		ParseError("Parent type expected.");
		return;
	}

	//  Check if it's a structure type
	if (struct_type->type != ev_struct)
	{
		ParseError("Parent must be a struct.");
		return;
	}

	TStruct* Struct = struct_type->Struct;

	//  Check if type has reserved memory for additional fields
	if (!Struct->AvailableSize)
	{
		ParseError("Parent type don't have available memory for additional fields.");
		return;
	}

	//  Read info
	num_fields = Struct->NumFields;
	memcpy(fields, Struct->Fields, num_fields * sizeof(*fields));
	delete Struct->Fields;
	size = Struct->AvailableSize;
	ofs = Struct->AvailableOfs;

	//	Add fields
	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		type = CheckForType();
		if (!type)
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
			if (t == &type_void)
			{
				ParseError("Field cannot have void type.");
			}
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError("Field name expected");
			}
			fi = &fields[num_fields];
			fi->Name = tk_Name;
			TK_NextToken();
			if (t->type == ev_bool && num_fields)
			{
				field_t &prevbool = fields[num_fields - 1];
				if (prevbool.type->type == ev_bool &&
					(dword)prevbool.type->bit_mask != 0x80000000)
				{
					TType btype;

					memcpy(&btype, t, sizeof(TType));
					btype.bit_mask = prevbool.type->bit_mask << 1;
					fi->type = FindType(&btype);
					fi->ofs = prevbool.ofs;
					num_fields++;
					continue;
				}
			}
			fi->ofs = ofs;
			while (TK_Check(PU_LINDEX))
			{
				i = EvalConstExpression(ev_int);
				TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
				t = MakeArrayType(t, i);
			}
			size -= TypeSize(t);
			ofs += TypeSize(t);
			if (size < 0)
			{
				ParseError("Additional fields size overflow.");
			}
			fi->type = t;
			num_fields++;
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	}
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);

	//	Renew TypeInfo
	Struct->Fields = new field_t[num_fields];
	memcpy(Struct->Fields, fields, num_fields * sizeof(*fields));
	Struct->NumFields = num_fields;
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
	TArray<field_t>		fields;
	field_t*			fi;
	field_t*			otherfield;
	int					size;
	int					i;
	TType*				t;
	TType*				type;

	TClass* Class = CheckForClass();
	if (Class)
	{
		if (Class->Size != -1)
		{
			ParseError("Class definition already completed");
			return;
		}
	}
	else
	{
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError("Class name expected");
		}
		//	New class.
		Class = new TClass;
		Class->Index = classtypes.AddItem(Class);
		Class->Name = tk_Name;
		TK_NextToken();
	}

	if (TK_Check(PU_SEMICOLON))
	{
		Class->Size = -1;
		return;
	}

	Class->NumFields = 0;
	Class->NumMethods = BASE_NUM_METHODS;
	size = 0;

	if (TK_Check(PU_COLON))
	{
		TClass* Parent = CheckForClass();
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

	do
	{
		if (TK_Check(KW_MOBJINFO))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			AddToMobjInfo(EvalConstExpression(ev_int), Class->Index);
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
		}
		else if (TK_Check(KW_NATIVE))
		{
		}
		else if (TK_Check(KW_ABSTRACT))
		{
		}
		else
		{
			break;
		}
	} while (1);

	Class->Fields = &fields[0];
	Class->Size = size;
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	while (!TK_Check(KW_DEFAULTPROPERTIES))
	{
		if (TK_Check(KW_STATES))
		{
		   	ParseStates(Class);
			continue;
		}

		int Flags = 0;
		bool flags_done = false;
		do
		{
			if (TK_Check(KW_NATIVE))
			{
				Flags |= FUNC_Native;
			}
			else
			{
				flags_done = true;
			}
		} while (!flags_done);

		FName TypeName = NAME_None;
		type = CheckForType();
		if (!type)
		{
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError("Field type expected.");
			}
			else
			{
				TypeName = tk_Name;
			}
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
			fi = new(fields) field_t;
Class->Fields = &fields[0];
			fi->Name = tk_Name;
			otherfield = CheckForField(Class);
			if (!otherfield)
			{
				TK_NextToken();
			}
			if (TK_Check(PU_LPAREN))
			{
				ParseMethodDef(t, fi, otherfield, Class, Flags);
				need_semicolon = false;
				break;
			}
			if (otherfield)
			{
				ParseError("Redeclared field");
				continue;
			}
			if (t == &type_void)
			{
				ParseError("Field cannot have void type.");
			}
			if (t->type == ev_bool && fields.Num() > 1)
			{
				field_t &prevbool = fields[fields.Num() - 2];
				if (prevbool.type->type == ev_bool &&
					(dword)prevbool.type->bit_mask != 0x80000000)
				{
					TType btype;

					memcpy(&btype, t, sizeof(TType));
					btype.bit_mask = prevbool.type->bit_mask << 1;
					fi->type = FindType(&btype);
					fi->ofs = prevbool.ofs;
					Class->NumFields++;
					continue;
				}
			}
			fi->ofs = size;
			while (TK_Check(PU_LINDEX))
			{
				i = EvalConstExpression(ev_int);
				TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
				t = MakeArrayType(t, i);
			}
		   	size += TypeSize(t);
			fi->type = t;
			Class->NumFields++;
		} while (TK_Check(PU_COMMA));
		if (need_semicolon)
		{
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		}
	}

	fi = new(fields) field_t;
Class->Fields = &fields[0];
	ParseDefaultProperties(fi, Class);

	//	Add to the type
	Class->Fields = new field_t[Class->NumFields];
	memcpy(Class->Fields, fields.GetData(),
		Class->NumFields * sizeof(field_t));
	Class->Size = size;
}

//**************************************************************************
//
//	$Log$
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
