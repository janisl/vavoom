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
};

struct typedef_t
{
	FName		Name;
	TType		*type;
	typedef_t	*next;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

TType		type_void(ev_void, NULL, NULL, 0);
TType		type_int(ev_int, &type_void, NULL, 4);
TType		type_float(ev_float, &type_int, NULL, 4);
TType		type_name(ev_name, &type_float, NULL, 4);
TType		type_string(ev_string, &type_name, NULL, 4);
TType		type_function(ev_function, &type_string, &type_void, 4);
TType		type_state(ev_struct, &type_function, NULL, -1);
TType		type_mobjinfo(ev_struct, &type_state, NULL, -1);
TType		type_void_ptr(ev_pointer, &type_mobjinfo, &type_void, 4);
TType		type_vector(ev_vector, &type_void_ptr, NULL, 12);
TType		type_classid(ev_classid, &type_vector, NULL, 4);
TType		type_class(ev_class, &type_classid, NULL, -1);
TType		type_none_ref(ev_reference, &type_class, &type_class, 4);
TType		type_bool(ev_bool, &type_none_ref, NULL, 4);

TType		*types = &type_bool;

TType		**classtypes;

typedef_t	*typedefs;

int			numclasses = 1;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	InitTypes
//
//==========================================================================

void InitTypes(void)
{
	type_state.Name = "state_t";
	type_mobjinfo.Name = "mobjinfo_t";
	type_class.Name = "Object";
	type_bool.params_size = 1;
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
	int		i;

	for (check = types; check; check = check->next)
	{
		//	Check main params
		if (type->type != check->type ||
			type->aux_type != check->aux_type ||
			type->size != check->size ||
			type->num_params != check->num_params ||
			type->params_size != check->params_size)
			continue;

		for (i = 0; i < (type->num_params & PF_COUNT_MASK); i++)
		{
			if (type->param_types[i] != check->param_types[i])
				break;
		}
		if (i == (type->num_params & PF_COUNT_MASK))
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
	pointer.size = 4;
	return FindType(&pointer);
}

//==========================================================================
//
//	MakeReferenceType
//
//==========================================================================

TType *MakeReferenceType(TType *type)
{
	TType reference;

	if (type->type == ev_reference)
	{
		ParseError("Can't create reference to reference");
	}
	memset(&reference, 0, sizeof(TType));
	reference.type = ev_reference;
	reference.aux_type = type;
	reference.size = 4;
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
	array.size = type->size * elcount;
	return FindType(&array);
}

//==========================================================================
//
//	CheckForTypeKeyword
//
//==========================================================================

TType *CheckForTypeKeyword(void)
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

TType *CheckForType(void)
{
	TType		*check;
	typedef_t	*tdef;

	if (tk_Token == TK_KEYWORD)
	{
		return CheckForTypeKeyword();
	}

	for (tdef = typedefs; tdef; tdef = tdef->next)
	{
		if (TK_Check(tdef->Name))
		{
			return tdef->type;
		}
	}

	for (check = types; check; check = check->next)
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
	TType		*check;
	typedef_t	*tdef;

	if (Name == NAME_None)
	{
		return NULL;
	}

	for (tdef = typedefs; tdef; tdef = tdef->next)
	{
		if (Name == tdef->Name)
		{
			return tdef->type;
		}
	}

	for (check = types; check; check = check->next)
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
//	TypeSize
//
//==========================================================================

int TypeSize(TType *type)
{
	if (type->size < 0)
	{
		ParseError("Incomplete type");
	}
	return (type->size + 3) & ~3;
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
//  PÆrbauda, vai tipa izmñrs ir 4
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
//	TypeCheck2
//
//  PÆrbauda, vai tips ir veselÆ skaitõa tips
//
//==========================================================================

void TypeCheck2(TType *t)
{
	TypeCheck1(t);
	if (t != &type_int)
	{
		ParseError(ERR_EXPR_TYPE_MISTMATCH);
	}
}

//==========================================================================
//
//	TypeCheck3
//
//	Check, if typea are compatible
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
	if ((t1->type == ev_function) && (t2->type == ev_function))
	{
		ParseWarning("Different function types");
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
		if ((t1->type == ev_struct && t2->type == ev_struct) ||
			(t1->type == ev_class && t2->type == ev_class))
		{
			while (t1->aux_type)
			{
				t1 = t1->aux_type;
				if (t1 == t2)
				{
					return;
				}
			}
		}
	}
	if ((t1->type == ev_reference) && (t2->type == ev_reference))
	{
		t1 = t1->aux_type;
		t2 = t2->aux_type;
		if (t1 == t2)
		{
			return;
		}
		if ((t1 == &type_class) || (t2 == &type_class))
		{
			return;
		}
		if ((t1->type == ev_struct && t2->type == ev_struct) ||
			(t1->type == ev_class && t2->type == ev_class))
		{
			while (t1->aux_type)
			{
				t1 = t1->aux_type;
				if (t1 == t2)
				{
					return;
				}
			}
		}
	}
	ParseError(ERR_EXPR_TYPE_MISTMATCH, " Types %s and %s are not compatible %d %d",
		*t1->Name, *t2->Name, t1->type, t2->type);
}

//==========================================================================
//
//	ParseStruct
//
//==========================================================================

void ParseStruct(void)
{
	field_t		fields[128];
	field_t		*fi;
	int			num_fields;
	int			size;
	int			i;
	TType		*t;
	TType		*type;
	TType		*struct_type;

	struct_type = CheckForType();
	if (struct_type)
	{
		if (struct_type->type != ev_struct)
		{
			ParseError("Not a struct type");
			return;
		}
		if (struct_type->size != -1)
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
		//  Pievieno pie tipiem
		struct_type = new TType;
		memset(struct_type, 0, sizeof(TType));
		struct_type->Name = tk_Name;
		struct_type->type = ev_struct;
		struct_type->next = types;
		types = struct_type;
		TK_NextToken();
	}

	if (TK_Check(PU_SEMICOLON))
	{
		struct_type->size = -1;
		return;
	}

	num_fields = 0;
	size = 0;

	if (TK_Check(PU_COLON))
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
			size = TypeSize(type);
		}
	}

   	struct_type->available_size = 0;
   	struct_type->available_ofs = 0;
	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		if (TK_Check(KW_ADDFIELDS))
		{
	   		if (struct_type->available_size)
			{
				ParseError("Addfields already defined");
			}
			if (tk_Token != TK_INTEGER)
			{
				ParseError("Field count expacted");
			}
	   		struct_type->available_size = tk_Number * 4;
   			struct_type->available_ofs = size;
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
		do
		{
			t = type;
			while (TK_Check(PU_ASTERISK))
			{
				t = MakePointerType(t);
			}
#ifdef REF_CPP
			while (TK_Check(PU_AND))
			{
				t = MakeReferenceType(t);
			}
#endif
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
					(dword)prevbool.type->params_size != 0x80000000)
				{
					TType btype;

					memcpy(&btype, t, sizeof(TType));
					btype.params_size = prevbool.type->params_size << 1;
					fi->type = FindType(&btype);
					fi->ofs = prevbool.ofs;
					num_fields++;
					continue;
				}
			}
			fi->ofs = size;
			if (t->type == ev_class)
			{
				t = MakeReferenceType(t);
			}
			while (TK_Check(PU_LINDEX))
			{
				i = EvalConstExpression(ev_int);
				TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
				t = MakeArrayType(t, i);
			}
		   	size += TypeSize(t);
			fi->type = t;
			num_fields++;
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	}
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);

	//	Pievieno pie tipa
	struct_type->fields = new field_t[num_fields];
	memcpy(struct_type->fields, fields, num_fields * sizeof(*fields));
	struct_type->numfields = num_fields;
   	struct_type->size = size;
}

//==========================================================================
//
//	AddFields
//
//==========================================================================

void AddFields(void)
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

	//  Nolasa tipu, kuram tiks pievienoti jaunie lauki.
	struct_type = CheckForType();
   	if (!struct_type)
	{
	   	ParseError("Parent type expected.");
		return;
	}

	//  PÆrbauda, vai tas ir struktÝras tips
	if (struct_type->type != ev_struct && (
		!ClassAddfields || struct_type->type != ev_class))
	{
	 	ParseError("Parent must be a struct.");
		return;
	}

	//  PÆrbauda, vai tipam ir atmi·a papildu laukiem
	if (!struct_type->available_size)
	{
		ParseError("Parent type don't have available memory for additional fields.");
		return;
	}

	//  Nolasa informÆciju
	num_fields = struct_type->numfields;
	memcpy(fields, struct_type->fields, num_fields * sizeof(*fields));
	delete struct_type->fields;
	size = struct_type->available_size;
	ofs = struct_type->available_ofs;

	//	Pievieno laukus
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
#ifdef REF_CPP
			while (TK_Check(PU_AND))
			{
				t = MakeReferenceType(t);
			}
#endif
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
					(dword)prevbool.type->params_size != 0x80000000)
				{
					TType btype;

					memcpy(&btype, t, sizeof(TType));
					btype.params_size = prevbool.type->params_size << 1;
					fi->type = FindType(&btype);
					fi->ofs = prevbool.ofs;
					num_fields++;
					continue;
				}
			}
			fi->ofs = ofs;
			if (t->type == ev_class)
			{
				t = MakeReferenceType(t);
			}
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

	//	Atjauno TypeInfo
	struct_type->fields = new field_t[num_fields];
	memcpy(struct_type->fields, fields, num_fields * sizeof(*fields));
	struct_type->numfields = num_fields;
   	struct_type->available_size = size;
   	struct_type->available_ofs = ofs;
}

//==========================================================================
//
//	ParseVector
//
//==========================================================================

void ParseVector(void)
{
	field_t		fields[3];
	field_t		*fi;
	int			num_fields;
	int			size;
	TType		*type;
	TType		*struct_type;

	struct_type = CheckForType();
	if (struct_type)
	{
		if (struct_type->type != ev_vector)
		{
			ParseError("Not a vector type");
			return;
		}
		if (struct_type->size != -1)
		{
			ParseError("Vector type already completed");
			return;
		}
	}
	else
	{
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError("Vector type name expected");
		}
		//  Pievieno pie tipiem
		struct_type = new TType;
		memset(struct_type, 0, sizeof(TType));
		struct_type->Name = tk_Name;
		struct_type->type = ev_vector;
		struct_type->next = types;
		types = struct_type;
		TK_NextToken();
	}

	if (TK_Check(PU_SEMICOLON))
	{
		struct_type->size = -1;
		return;
	}

	num_fields = 0;
	size = 0;

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		type = CheckForType();
		if (!type)
		{
			ParseError("Field type expected.");
			continue;
		}
		if (type != &type_float)
		{
			ParseError("Vector can have only float fields");
			continue;
		}
		do
		{
			if (num_fields == 3)
			{
				ParseError("Vector must have exactly 3 float fields");
				continue;
			}
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError("Field name expected");
				continue;
			}
			fi = &fields[num_fields];
			fi->Name = tk_Name;
			TK_NextToken();
			fi->ofs = size;
		   	size += TypeSize(type);
			fi->type = type;
			num_fields++;
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	}
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	if (num_fields != 3)
	{
		ParseError("Vector must have exactly 3 float fields");
	}

	//	Pievieno pie tipa
	struct_type->fields = new field_t[num_fields];
	memcpy(struct_type->fields, fields, num_fields * sizeof(*fields));
	struct_type->numfields = num_fields;
   	struct_type->size = size;
}

//==========================================================================
//
//	ParseClass
//
//==========================================================================

void ParseClass(void)
{
	TArray<field_t>		fields;
	field_t		*fi;
	field_t		*otherfield;
	int			size;
	int			i;
	TType		*t;
	TType		*type;
	TType		*class_type;

	class_type = CheckForType();
	if (class_type)
	{
		if (class_type->type != ev_class)
		{
			ParseError("Not a class type");
			return;
		}
		if (class_type->size != -1)
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
		//  Pievieno pie tipiem
		class_type = new TType;
		memset(class_type, 0, sizeof(TType));
		class_type->Name = tk_Name;
		class_type->type = ev_class;
		class_type->next = types;
		class_type->classid = numclasses++;
		types = class_type;
		TK_NextToken();
	}

	if (TK_Check(PU_SEMICOLON))
	{
		class_type->size = -1;
		return;
	}

	class_type->numfields = 0;
	class_type->num_methods = BASE_NUM_METHODS;
	size = 0;

	if (TK_Check(PU_COLON))
	{
		type = CheckForType();
		if (!type)
		{
			ParseError("Parent class type expected");
		}
		else if (type->type != ev_class)
		{
			ParseError("Parent type must be a class");
		}
		else
		{
			class_type->aux_type = type;
			class_type->num_methods = type->num_methods;
			size = TypeSize(type);
		}
	}

	if (TK_Check(KW_MOBJINFO))
	{
		TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
		AddToMobjInfo(EvalConstExpression(ev_int), class_type->classid);
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}

   	class_type->available_size = 0;
   	class_type->available_ofs = 0;
	class_type->fields = &fields[0];
	class_type->size = size;
	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		if (ClassAddfields && TK_Check(KW_ADDFIELDS))
		{
	   		if (class_type->available_size)
			{
				ParseError("Addfields already defined");
			}
			if (tk_Token != TK_INTEGER)
			{
				ParseError("Field count expacted");
			}
	   		class_type->available_size = tk_Number * 4;
   			class_type->available_ofs = size;
			size += tk_Number * 4;
			TK_NextToken();
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			continue;
		}

		if (TK_Check(KW_STATES))
		{
		   	ParseStates(class_type);
			continue;
		}

		if (TK_Check(KW_DEFAULTPROPERTIES))
		{
			fi = new(fields) field_t;
class_type->fields = &fields[0];
			ParseDefaultProperties(fi, class_type);
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
#ifdef REF_CPP
			while (TK_Check(PU_AND))
			{
				t = MakeReferenceType(t);
			}
#endif
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError("Field name expected");
				continue;
			}
			fi = new(fields) field_t;
class_type->fields = &fields[0];
			fi->Name = tk_Name;
			otherfield = CheckForField(class_type);
			if (!otherfield)
			{
				TK_NextToken();
			}
			if (TK_Check(PU_LPAREN))
			{
				ParseMethodDef(t, fi, otherfield, class_type, Flags);
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
					(dword)prevbool.type->params_size != 0x80000000)
				{
					TType btype;

					memcpy(&btype, t, sizeof(TType));
					btype.params_size = prevbool.type->params_size << 1;
					fi->type = FindType(&btype);
					fi->ofs = prevbool.ofs;
					class_type->numfields++;
					continue;
				}
			}
			fi->ofs = size;
			if (t->type == ev_class)
			{
				t = MakeReferenceType(t);
			}
			while (TK_Check(PU_LINDEX))
			{
				i = EvalConstExpression(ev_int);
				TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
				t = MakeArrayType(t, i);
			}
		   	size += TypeSize(t);
			fi->type = t;
			class_type->numfields++;
		} while (TK_Check(PU_COMMA));
		if (need_semicolon)
		{
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		}
	}

	//  Semikols beigÆs nav nepiecieýams
	TK_Check(PU_SEMICOLON);

	//	Pievieno pie tipa
	class_type->fields = new field_t[class_type->numfields];
	memcpy(class_type->fields, fields.GetData(),
		class_type->numfields * sizeof(field_t));
   	class_type->size = size;
}

//==========================================================================
//
//	ParseField
//
//==========================================================================

field_t* ParseField(TType *t)
{
	field_t		*fi;
	int			i;

	if (t->type != ev_struct && t->type != ev_vector && t->type != ev_class)
	{
	 	ParseError(ERR_NOT_A_STRUCT, "Base type required.");
		return NULL;
	}
	if (t->size == -1)
	{
	 	ParseError("Incomplete type.");
		return NULL;
	}
	fi = t->fields;
	if (tk_Token != TK_IDENTIFIER)
	{
		ParseError(ERR_INVALID_IDENTIFIER, ", field name expacted");
		return NULL;
	}
	for (i = 0; i < t->numfields; i++)
	{
		if (TK_Check(fi[i].Name))
		{
			return &fi[i];
		}
	}
	if (t->aux_type)
	{
		return ParseField(t->aux_type);
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

field_t* CheckForField(TType *t, bool check_aux)
{
	if (!t)
	{
		return NULL;
	}
	if (t->type != ev_class)
	{
		return NULL;
	}
	if (t->size == -1)
	{
		return NULL;
	}
	if (tk_Token != TK_IDENTIFIER)
	{
		return NULL;
	}
	field_t *fi = t->fields;
	for (int i = 0; i < t->numfields; i++)
	{
		if (TK_Check(fi[i].Name))
		{
			return &fi[i];
		}
	}
	if (check_aux)
	{
		return CheckForField(t->aux_type);
	}
	return NULL;
}

//==========================================================================
//
//	CheckForField
//
//==========================================================================

field_t* CheckForField(FName Name, TType *t, bool check_aux)
{
	if (!t)
	{
		return NULL;
	}
	if (t->type != ev_class)
	{
		return NULL;
	}
	if (t->size == -1)
	{
		return NULL;
	}
	if (Name == NAME_None)
	{
		return NULL;
	}
	field_t *fi = t->fields;
	for (int i = 0; i < t->numfields; i++)
	{
		if (Name == fi[i].Name)
		{
			return &fi[i];
		}
	}
	if (check_aux)
	{
		return CheckForField(Name, t->aux_type);
	}
	return NULL;
}

//==========================================================================
//
//	FindConstructor
//
//==========================================================================

field_t* FindConstructor(TType *t)
{
	if (!t)
	{
		return NULL;
	}
	if (t->type != ev_class)
	{
		return NULL;
	}
	if (t->size == -1)
	{
		return NULL;
	}
	field_t *fi = t->fields;
	for (int i = 0; i < t->numfields; i++)
	{
		if (fi[i].type->type == ev_method && fi[i].ofs == 0)
		{
			return &fi[i];
		}
	}
	return FindConstructor(t->aux_type);
}

//==========================================================================
//
//	ParseTypeDef
//
//==========================================================================

void ParseTypeDef(void)
{
	TType		*type;
	typedef_t	*tdef;

	//	Return type
	type = CheckForType();
	if (!type)
	{
		ParseError("Type name expected, found %s", tk_String);
		return;
	}
	while (TK_Check(PU_ASTERISK))
	{
		type = MakePointerType(type);
	}
#ifdef REF_CPP
	while (TK_Check(PU_AND))
	{
		type = MakeReferenceType(type);
	}
#endif

	if (TK_Check(PU_LPAREN))
	{
		//	Function pointer type
		TType	functype;
		FName	Name;

		memset(&functype, 0, sizeof(TType));
		functype.type = ev_function;
		functype.size = 4;
		functype.aux_type = type;

		if (!TK_Check(PU_ASTERISK))
		{
			ParseError("Missing *");
			return;
		}
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError("New type name expected");
			return;
		}
		Name = tk_Name;
		TK_NextToken();
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);

		//	Args
		TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
		do
		{
			type = CheckForType();

			if (!type)
			{
				if (functype.num_params == 0)
				{
					break;
				}
				ParseError(ERR_BAD_VAR_TYPE);
				continue;
			}

			while (TK_Check(PU_ASTERISK))
			{
		   		type = MakePointerType(type);
			}
#ifdef REF_CPP
			while (TK_Check(PU_AND))
			{
		   		type = MakeReferenceType(type);
			}
#endif
			if (type->type == ev_class)
			{
				type = MakeReferenceType(type);
			}
			if (functype.num_params == 0 && type == &type_void)
			{
				break;
			}
			TypeCheckPassable(type);

			if (functype.num_params == MAX_PARAMS)
			{
				ERR_Exit(ERR_PARAMS_OVERFLOW, true, NULL);
			}
	   		if (tk_Token == TK_IDENTIFIER)
			{
				TK_NextToken();
			}

			functype.param_types[functype.num_params] = type;
			functype.num_params++;
			functype.params_size += TypeSize(type) / 4;
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);

		//	Add to typedefs
		tdef = new typedef_t;
		tdef->Name = Name;
		tdef->type = FindType(&functype);
		tdef->next = typedefs;
		typedefs = tdef;
		return;
	}

	//	Ordinary typedef
	if (tk_Token != TK_IDENTIFIER)
	{
		ParseError("New type name expected");
		return;
	}

	//	Add to typedefs
	tdef = new typedef_t;
	tdef->Name = tk_Name;
	tdef->type = type;
	tdef->next = typedefs;
	typedefs = tdef;

	TK_NextToken();
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
}

//==========================================================================
//
//	AddVTable
//
//==========================================================================

static void AddVTable(TType *t)
{
	if (t->vtable)
	{
		return;
	}
	classtypes[t->classid] = t;
	t->vtable = numglobals;
	int *vtable = globals + numglobals;
	memset(globalinfo + numglobals, 2, t->num_methods);
	numglobals += t->num_methods;
	if (t->aux_type)
	{
		AddVTable(t->aux_type);
		memcpy(vtable, globals + t->aux_type->vtable,
			t->aux_type->num_methods * 4);
	}
	for (int i = 0; i < t->numfields; i++)
	{
		field_t &f = t->fields[i];
		if (f.type->type != ev_method)
		{
			continue;
		}
		if (!f.func_num)
		{
			ParseError("Method %s::%s not defined", *t->Name, *f.Name);
		}
		vtable[f.ofs] = f.func_num;
	}
	if (!vtable[0])
	{
		ERR_Exit(ERR_NONE, false, "Missing defaultproperties");
	}
}

//==========================================================================
//
//	WritePropertyInfo
//
//==========================================================================

void WritePropertyInfo(TType *t)
{
	t->ofs_properties = numglobals;
	t->num_properties = 0;
	dfield_t *df = (dfield_t *)(globals + numglobals);
	for (int i = 0; i < t->numfields; i++)
	{
		field_t &F = t->fields[i];
		switch (F.type->type)
		{
		case ev_name:
			df[t->num_properties].type = PROPTYPE_Name;
			df[t->num_properties].ofs = F.ofs;
			t->num_properties++;
			break;
		//	ev_string,
		//	ev_function,
		//	ev_pointer,
		case ev_reference:
			df[t->num_properties].type = PROPTYPE_Reference;
			df[t->num_properties].ofs = F.ofs;
			t->num_properties++;
			break;
		case ev_classid:
			df[t->num_properties].type = PROPTYPE_ClassID;
			df[t->num_properties].ofs = F.ofs;
			t->num_properties++;
			break;
		}
	}
	numglobals += t->num_properties * sizeof(dfield_t) / 4;
}

//==========================================================================
//
//	AddVirtualTables
//
//==========================================================================

void AddVirtualTables(void)
{
	int OldNumGlobals = numglobals;
	classtypes = new TType*[numclasses];
	memset(classtypes, 0, numclasses * 4);
	for (TType *t = types; t; t = t->next)
	{
		if (t->type == ev_class)
		{
			AddVTable(t);
		}
	}
	dprintf("Virtual tables takes %d bytes\n", (numglobals - OldNumGlobals) * 4);
	OldNumGlobals = numglobals;
	for (TType *t = types; t; t = t->next)
	{
		if (t->type == ev_class)
		{
			WritePropertyInfo(t);
		}
	}
	dprintf("Fields info takes %d bytes\n", (numglobals - OldNumGlobals) * 4);
}

//**************************************************************************
//
//	$Log$
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
