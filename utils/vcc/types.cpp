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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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

struct typedef_t
{
	char		name[MAX_IDENTIFIER_LENGTH];
	TType		*type;
	typedef_t	*next;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

TType		type_void("void", ev_void, NULL, NULL, 0);
TType		type_int("int", ev_int, &type_void, NULL, 4);
TType		type_uint("uint", ev_uint, &type_int, NULL, 4);
TType		type_float("float", ev_float, &type_uint, NULL, 4);
TType		type_string("string", ev_string, &type_float, NULL, 4);
TType		type_function("function_t", ev_function, &type_string, &type_void, 4);
TType		type_state("state_t", ev_struct, &type_function, NULL, -1);
TType		type_mobjinfo("mobjinfo_t", ev_struct, &type_state, NULL, -1);
TType		type_void_ptr("", ev_pointer, &type_mobjinfo, &type_void, 4);
TType		type_vector("", ev_vector, &type_void_ptr, NULL, 12);
TType		type_classid("classid", ev_classid, &type_vector, NULL, 4);

TType		*types = &type_classid;

TType		**classtypes;

TType		NoneClass("", ev_class, NULL, NULL, 0);

typedef_t	*typedefs;

int			numclasses = 1;

// CODE --------------------------------------------------------------------

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

		for (i = 0; i < type->num_params & PF_COUNT_MASK; i++)
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
//  CheckForType
//
//==========================================================================

TType *CheckForType(void)
{
	TType		*check;
	typedef_t	*tdef;

	for (tdef = typedefs; tdef; tdef = tdef->next)
	{
		if (TK_Check(tdef->name))
		{
			return tdef->type;
		}
	}

	for (check = types; check; check = check->next)
	{
		if (check->name[0] && TK_Check(check->name))
		{
			return check;
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
	if (t != &type_int && t != &type_uint)
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
	if (t1 == &type_uint) t1 = &type_int;
	if (t2 == &type_uint) t2 = &type_int;
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
		if (t1 == &type_uint) t1 = &type_int;
		if (t2 == &type_uint) t2 = &type_int;
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
	ParseError(ERR_EXPR_TYPE_MISTMATCH, " Types %s and %s are not compatible",
		t1->name, t2->name);
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
		strcpy(struct_type->name, tk_String);
		struct_type->type = ev_struct;
		struct_type->next = types;
		types = struct_type;
		TK_NextToken();
		dprintf("Added new struct %s\n", struct_type->name);
	}

	if (TK_Check(";"))
	{
		struct_type->size = -1;
		return;
	}

	num_fields = 0;
	size = 0;

	if (TK_Check(":"))
	{
		dprintf("Structure is inherited from %s\n", tk_String);
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
	TK_Expect("{", ERR_MISSING_LBRACE);
	while (!TK_Check("}"))
	{
		if (TK_Check("addfields"))
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
			TK_Expect(";", ERR_MISSING_SEMICOLON);
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
			while (TK_Check("*"))
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
			strcpy(fi->name, tk_String);
			TK_NextToken();
			fi->ofs = size;
			while (TK_Check("["))
			{
				i = EvalConstExpression(ev_int);
				TK_Expect("]", ERR_MISSING_RFIGURESCOPE);
				t = MakeArrayType(t, i);
			}
		   	size += TypeSize(t);
			fi->type = t;
			dprintf("Field %d %s, ofs %d, type %d.\n",
				num_fields, fi->name, fi->ofs, fi->type);
			num_fields++;
		} while (TK_Check(","));
		TK_Expect(";", ERR_MISSING_SEMICOLON);
	}
	TK_Expect(";", ERR_MISSING_SEMICOLON);

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
	if (struct_type->type != ev_struct && struct_type->type != ev_class)
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
	TK_Expect("{", ERR_MISSING_LBRACE);
	while (!TK_Check("}"))
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
			while (TK_Check("*"))
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
			strcpy(fi->name, tk_String);
			TK_NextToken();
			fi->ofs = ofs;
			while (TK_Check("["))
			{
				i = EvalConstExpression(ev_int);
				TK_Expect("]", ERR_MISSING_RFIGURESCOPE);
				t = MakeArrayType(t, i);
			}
		   	size -= TypeSize(t);
		   	ofs += TypeSize(t);
		   	if (size < 0)
			{
	   			ParseError("Additional fields size overflow.");
			}
			fi->type = t;
			dprintf("Field %d %s, ofs %d, type %d.\n",
				num_fields, fi->name, fi->ofs, fi->type);
			num_fields++;
		} while (TK_Check(","));
		TK_Expect(";", ERR_MISSING_SEMICOLON);
	}
	TK_Expect(";", ERR_MISSING_SEMICOLON);

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
		strcpy(struct_type->name, tk_String);
		struct_type->type = ev_vector;
		struct_type->next = types;
		types = struct_type;
		TK_NextToken();
		dprintf("Added new vector type %s\n", struct_type->name);
	}

	if (TK_Check(";"))
	{
		struct_type->size = -1;
		return;
	}

	num_fields = 0;
	size = 0;

	TK_Expect("{", ERR_MISSING_LBRACE);
	while (!TK_Check("}"))
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
			strcpy(fi->name, tk_String);
			TK_NextToken();
			fi->ofs = size;
		   	size += TypeSize(type);
			fi->type = type;
			dprintf("Field %d %s, ofs %d, type %d.\n",
				num_fields, fi->name, fi->ofs, fi->type);
			num_fields++;
		} while (TK_Check(","));
		TK_Expect(";", ERR_MISSING_SEMICOLON);
	}
	TK_Expect(";", ERR_MISSING_SEMICOLON);
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

#define BASE_CLASS_SIZE			4
#define BASE_NUM_METHODS		2

void ParseClass(void)
{
	field_t		fields[128];
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
		strcpy(class_type->name, tk_String);
		class_type->s_name = FindString(tk_String);
		class_type->type = ev_class;
		class_type->next = types;
		class_type->classid = numclasses++;
		types = class_type;
		TK_NextToken();
		dprintf("Added new class %s\n", class_type->name);
	}

	if (TK_Check(";"))
	{
		class_type->size = -1;
		return;
	}

	class_type->numfields = 0;
	class_type->num_methods = BASE_NUM_METHODS;
	size = BASE_CLASS_SIZE;

	if (TK_Check(":"))
	{
		dprintf("Class is inherited from %s\n", tk_String);
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

   	class_type->available_size = 0;
   	class_type->available_ofs = 0;
	class_type->fields = fields;
	class_type->size = size;
	TK_Expect("{", ERR_MISSING_LBRACE);
	while (!TK_Check("}"))
	{
		if (TK_Check("addfields"))
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
			TK_Expect(";", ERR_MISSING_SEMICOLON);
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
			while (TK_Check("*"))
			{
				t = MakePointerType(t);
			}
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError("Field name expected");
				continue;
			}
			fi = &fields[class_type->numfields];
			strcpy(fi->name, tk_String);
			otherfield = CheckForField(class_type);
			if (!otherfield)
			{
				TK_NextToken();
			}
			if (TK_Check("("))
			{
				TType functype;
				memset(&functype, 0, sizeof(TType));
				functype.type = ev_method;
				functype.aux_type = t;

				do
				{
					if (TK_Check("..."))
					{
						functype.num_params |= PF_VARARGS;
						break;
					}

					type = CheckForType();

					if (!type)
					{
						if (functype.num_params == 0)
						{
							break;
						}
						ERR_Exit(ERR_BAD_VAR_TYPE, true, NULL);
					}
					while (TK_Check("*"))
					{
					   	type = MakePointerType(type);
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
				} while (TK_Check(","));
				TK_Expect(")", ERR_MISSING_RPAREN);

				fi->type = FindType(&functype);
				if (otherfield)
				{
					if (otherfield->type != fi->type)
					{
						ParseError("Method redefined with different type");
						break;
					}
					fi->ofs = otherfield->ofs;
					dprintf("Overrided method\n");
				}
				else
				{
					fi->ofs = class_type->num_methods;
					class_type->num_methods++;
					dprintf("New method\n");
				}
				fi->func_num = 0;
				dprintf("Method %d %s, ofs %d, type %d.\n",
					class_type->numfields, fi->name, fi->ofs, fi->type);
				class_type->numfields++;
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
			fi->ofs = size;
			while (TK_Check("["))
			{
				i = EvalConstExpression(ev_int);
				TK_Expect("]", ERR_MISSING_RFIGURESCOPE);
				t = MakeArrayType(t, i);
			}
		   	size += TypeSize(t);
			fi->type = t;
			dprintf("Field %d %s, ofs %d, type %d.\n",
				class_type->numfields, fi->name, fi->ofs, fi->type);
			class_type->numfields++;
		} while (TK_Check(","));
		TK_Expect(";", ERR_MISSING_SEMICOLON);
	}
	TK_Expect(";", ERR_MISSING_SEMICOLON);

	//	Pievieno pie tipa
	class_type->fields = new field_t[class_type->numfields];
	memcpy(class_type->fields, fields, class_type->numfields * sizeof(*fields));
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
		ParseError(ERR_INVALID_IDENTIFIER, "%s, field name expacted", tk_String);
		return NULL;
	}
	for (i = 0; i < t->numfields; i++)
	{
		if (TK_Check(fi[i].name))
		{
			return &fi[i];
		}
	}
	if (t->aux_type)
	{
		return ParseField(t->aux_type);
	}
	ParseError(ERR_NOT_A_FIELD, "Identifier: %s", tk_String);
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
		if (TK_Check(fi[i].name))
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
	while (TK_Check("*"))
	{
		type = MakePointerType(type);
	}

	if (TK_Check("("))
	{
		//	Function pointer type
		TType		functype;
		char		name[MAX_IDENTIFIER_LENGTH];

		memset(&functype, 0, sizeof(TType));
		functype.type = ev_function;
		functype.size = 4;
		functype.aux_type = type;

		if (!TK_Check("*"))
		{
			ParseError("Missing *");
			return;
		}
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError("New type name expected");
			return;
		}
		strcpy(name, tk_String);
		TK_NextToken();
		TK_Expect(")", ERR_MISSING_RPAREN);

		//	Args
		TK_Expect("(", ERR_MISSING_LPAREN);
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

			while (TK_Check("*"))
			{
		   		type = MakePointerType(type);
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
		} while (TK_Check(","));
		TK_Expect(")", ERR_MISSING_RPAREN);
		TK_Expect(";", ERR_MISSING_SEMICOLON);

		//	Add to typedefs
		tdef = new typedef_t;
		strcpy(tdef->name, name);
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
	strcpy(tdef->name, tk_String);
	tdef->type = type;
	tdef->next = typedefs;
	typedefs = tdef;

	TK_NextToken();
	TK_Expect(";", ERR_MISSING_SEMICOLON);
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
	numglobals += t->num_methods;
	if (t->aux_type)
	{
		AddVTable(t->aux_type);
		memcpy(vtable, globals + t->aux_type->vtable,
			t->aux_type->num_methods * 4);
	}
	vtable[0] = t->classid;
	vtable[1] = t->size;
	for (int i = 0; i < t->numfields; i++)
	{
		field_t &f = t->fields[i];
		if (f.type->type != ev_method)
		{
			continue;
		}
		if (!f.func_num)
		{
			ParseError("Method %s::%s not defined", t->name, f.name);
		}
		vtable[f.ofs] = f.func_num;
	}
}

//==========================================================================
//
//	AddVirtualTables
//
//==========================================================================

void AddVirtualTables(void)
{
	classtypes = new TType*[numclasses];
	NoneClass.num_methods = BASE_NUM_METHODS;
	AddVTable(&NoneClass);
	for (TType *t = types; t; t = t->next)
	{
		if (t->type == ev_class)
		{
			AddVTable(t);
		}
	}
}

//**************************************************************************
//
//	$Log$
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
