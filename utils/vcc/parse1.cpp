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

#ifdef USE_2_PASSES

namespace Pass1 {

// MACROS ------------------------------------------------------------------

#define MAX_BREAK		128
#define MAX_CONTINUE	128
#define MAX_CASE		128

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void ParseCompoundStatement(void);

static void ParseExpressionPriority2(void);
static void ParseExpressionPriority13(void);
static void ParseExpressionPriority14(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int			numlocaldefs = 1;

static bool			CheckForLocal;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	CheckForGlobalVar
//
//==========================================================================

int CheckForGlobalVar(FName Name)
{
	if (Name == NAME_None)
	{
		return 0;
	}
	for (int i = 1; i < numglobaldefs; i++)
	{
		if (globaldefs[i].Name == Name)
		{
			return i;
		}
	}
	return 0;
}

//==========================================================================
//
//	CheckForFunction
//
//==========================================================================

int CheckForFunction(TType *InClass, FName Name)
{
	if (Name == NAME_None)
	{
		return 0;
	}
	for (int i = 1; i < numfunctions; i++)
	{
		if (functions[i].OuterClass == InClass && functions[i].Name == Name)
		{
			return i;
		}
	}
	return 0;
}

//==========================================================================
//
//	CheckForConstant
//
//==========================================================================

int CheckForConstant(FName Name)
{
#if 1
	for (constant_t *C = ConstantsHash[GetTypeHash(Name) & 255];
		C; C = C->HashNext)
	{
		if (C->Name == Name)
		{
			return C - Constants;
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
//	ParseFunctionCall
//
//==========================================================================

static void ParseFunctionCall(void)
{
	if (!TK_Check(PU_RPAREN))
	{
		do
		{
			ParseExpressionPriority14();
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
}

//==========================================================================
//
//	ParseExpressionPriority0
//
//==========================================================================

static void ParseExpressionPriority0(void)
{
	bool		bLocals;

	bLocals = CheckForLocal;
	CheckForLocal = false;
   	switch (tk_Token)
	{
	case TK_INTEGER:
		TK_NextToken();
		return;

	case TK_FLOAT:
		TK_NextToken();
		return;

	case TK_NAME:
		TK_NextToken();
		return;

	case TK_STRING:
		TK_NextToken();
		return;

	case TK_PUNCT:
	   	if (TK_Check(PU_LPAREN))
		{
			ParseExpressionPriority14();
			TK_Expect(PU_RPAREN, ERR_BAD_EXPR);
			return;
		}

		if (TK_Check(PU_DCOLON))
		{
			if (tk_Token != TK_IDENTIFIER)
				ParseError("Identifier expected");
			TK_NextToken();
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			ParseFunctionCall();
			return;
		}
		break;

	case TK_KEYWORD:
		if (TK_Check(KW_VECTOR))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			ParseExpressionPriority14();
			TK_Expect(PU_COMMA, ERR_BAD_EXPR);
			ParseExpressionPriority14();
			TK_Expect(PU_COMMA, ERR_BAD_EXPR);
			ParseExpressionPriority14();
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			return;
		}
		if (TK_Check(KW_THIS))
		{
			return;
		}
		if (TK_Check(KW_SELF))
		{
			return;
		}
		if (TK_Check(KW_NONE))
		{
		   	return;
		}
		if (TK_Check(KW_NULL))
		{
		   	return;
		}
		if (bLocals)
		{
			TK_NextToken();
			ParseLocalVar();
			return;
		}
		break;

	case TK_IDENTIFIER:
		TK_NextToken();
		if (TK_Check(PU_LPAREN))
		{
			ParseFunctionCall();
			return;
		}
		if (bLocals && (tk_Token == TK_IDENTIFIER ||
			(tk_Token == TK_PUNCT && tk_Punct == PU_ASTERISK)))
		{
			ParseLocalVar();
			return;
		}
		break;

	default:
	   	break;
	}
}

//==========================================================================
//
//	ParseExpressionPriority1
//
//==========================================================================

static void ParseExpressionPriority1(void)
{
	bool		done;

   	ParseExpressionPriority0();
	done = false;
	do
	{
   		if (TK_Check(PU_MINUS_GT))
   	   	{
			ParseField();
   		}
   		else if (TK_Check(PU_DOT))
   	   	{
			ParseField();
   		}
		else if (TK_Check(PU_LINDEX))
		{
			ParseExpressionPriority14();
			TK_Expect(PU_RINDEX, ERR_BAD_ARRAY);
		}
		else if (TK_Check(PU_LPAREN))
		{
			ParseFunctionCall();
		}
		else
		{
			done = true;
		}
	} while (!done);
}

//==========================================================================
//
//	ParseExpressionPriority2
//
//==========================================================================

static void ParseExpressionPriority2(void)
{
   	if (tk_Token == TK_PUNCT)
	{
		if (TK_Check(PU_PLUS))
		{
		   	ParseExpressionPriority2();
			return;
		}

	   	if (TK_Check(PU_MINUS))
		{
		   	ParseExpressionPriority2();
			return;
		}

	   	if (TK_Check(PU_NOT))
		{
		   	ParseExpressionPriority2();
			return;
		}

	   	if (TK_Check(PU_TILDE))
		{
		   	ParseExpressionPriority2();
			return;
		}

		if (TK_Check(PU_AND))
		{
		   	ParseExpressionPriority1();
			return;
		}
		if (TK_Check(PU_ASTERISK))
		{
		   	ParseExpressionPriority2();
			return;
		}

	   	if (TK_Check(PU_INC))
		{
		   	ParseExpressionPriority2();
			return;
		}

	   	if (TK_Check(PU_DEC))
		{
		   	ParseExpressionPriority2();
			return;
		}
	}

	ParseExpressionPriority1();

	if (TK_Check(PU_INC))
	{
		return;
	}

	if (TK_Check(PU_DEC))
	{
		return;
	}
}

//==========================================================================
//
//	ParseExpressionPriority3
//
//==========================================================================

static void ParseExpressionPriority3(void)
{
	ParseExpressionPriority2();
	while (TK_Check(PU_ASTERISK) || TK_Check(PU_SLASH) ||
		TK_Check(PU_PERCENT))
	{
		ParseExpressionPriority2();
	}
}

//==========================================================================
//
//	ParseExpressionPriority4
//
//==========================================================================

static void ParseExpressionPriority4(void)
{
	ParseExpressionPriority3();
	while (TK_Check(PU_PLUS) || TK_Check(PU_MINUS))
	{
		ParseExpressionPriority3();
	}
}

//==========================================================================
//
//	ParseExpressionPriority5
//
//==========================================================================

static void ParseExpressionPriority5(void)
{
	ParseExpressionPriority4();
	while (TK_Check(PU_LSHIFT) || TK_Check(PU_RSHIFT))
	{
		ParseExpressionPriority4();
	}
}

//==========================================================================
//
//	ParseExpressionPriority6
//
//==========================================================================

static void ParseExpressionPriority6(void)
{
	ParseExpressionPriority5();
	while (TK_Check(PU_LT) || TK_Check(PU_LE) ||
		TK_Check(PU_GT) || TK_Check(PU_GE))
	{
		ParseExpressionPriority5();
	}
}

//==========================================================================
//
//	ParseExpressionPriority7
//
//==========================================================================

static void ParseExpressionPriority7(void)
{
	ParseExpressionPriority6();
	while (TK_Check(PU_EQ) || TK_Check(PU_NE))
	{
		ParseExpressionPriority6();
	}
}

//==========================================================================
//
//	ParseExpressionPriority8
//
//==========================================================================

static void ParseExpressionPriority8(void)
{
	ParseExpressionPriority7();
	while (TK_Check(PU_AND))
	{
		ParseExpressionPriority7();
	}
}

//==========================================================================
//
//	ParseExpressionPriority9
//
//==========================================================================

static void ParseExpressionPriority9(void)
{
	ParseExpressionPriority8();
	while (TK_Check(PU_XOR))
	{
		ParseExpressionPriority8();
	}
}

//==========================================================================
//
//	ParseExpressionPriority10
//
//==========================================================================

static void ParseExpressionPriority10(void)
{
	ParseExpressionPriority9();
	while (TK_Check(PU_OR))
	{
		ParseExpressionPriority9();
	}
}

//==========================================================================
//
//	ParseExpressionPriority11
//
//==========================================================================

static void ParseExpressionPriority11(void)
{
	ParseExpressionPriority10();
	while (TK_Check(PU_AND_LOG))
	{
		ParseExpressionPriority10();
	}
}

//==========================================================================
//
//	ParseExpressionPriority12
//
//==========================================================================

static void ParseExpressionPriority12(void)
{
	ParseExpressionPriority11();
	while (TK_Check(PU_OR_LOG))
	{
		ParseExpressionPriority11();
	}
}

//==========================================================================
//
//	ParseExpressionPriority13
//
//==========================================================================

static void ParseExpressionPriority13(void)
{
	ParseExpressionPriority12();
   	if (TK_Check(PU_QUEST))
	{
		ParseExpressionPriority13();
		TK_Expect(PU_COLON, ERR_MISSING_COLON);
		ParseExpressionPriority13();
	}
}

//==========================================================================
//
//	ParseExpressionPriority14
//
//==========================================================================

static void ParseExpressionPriority14(void)
{
	static const Punctuation AssignOps[] =
	{
		PU_ASSIGN,
		PU_ADD_ASSIGN,
		PU_MINUS_ASSIGN,
		PU_MULTIPLY_ASSIGN,
		PU_DIVIDE_ASSIGN,
		PU_MOD_ASSIGN,
		PU_AND_ASSIGN,
		PU_OR_ASSIGN,
		PU_XOR_ASSIGN,
		PU_LSHIFT_ASSIGN,
		PU_RSHIFT_ASSIGN
	};

	ParseExpressionPriority13();
	for (int i = 0; i < 11; i++)
   	{
		if (TK_Check(AssignOps[i]))
		{
			ParseExpressionPriority14();
			return;
   		}
	}
}

//==========================================================================
//
//	ParseExpression
//
//==========================================================================

void ParseExpression(bool bLocals)
{
	CheckForLocal = bLocals;
	ParseExpressionPriority14();
}

//==========================================================================
//
// ProcessStatement
//
//==========================================================================

static void ParseStatement(void)
{
	switch (tk_Token)
	{
		case TK_EOF:
			ERR_Exit(ERR_UNEXPECTED_EOF, true, NULL);
			break;
		case TK_KEYWORD:
			if (TK_Check(KW_IF))
			{
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				ParseExpression();
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				ParseStatement();
				if (TK_Check(KW_ELSE))
				{
					ParseStatement();
				}
			}
			else if (TK_Check(KW_WHILE))
			{
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				ParseExpression();
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				ParseStatement();
			}
			else if (TK_Check(KW_DO))
			{
				ParseStatement();
				TK_Expect(KW_WHILE, ERR_BAD_DO_STATEMENT);
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				ParseExpression();
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			}
			else if (TK_Check(KW_FOR))
			{
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				do
				{
					ParseExpression();
				} while (TK_Check(PU_COMMA));
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				ParseExpression();
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				do
				{
					ParseExpression();
				} while (TK_Check(PU_COMMA));
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				ParseStatement();
			}
			else if (TK_Check(KW_BREAK))
			{
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			}
			else if (TK_Check(KW_CONTINUE))
			{
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			}
			else if (TK_Check(KW_RETURN))
			{
//				if (!TK_Check(PU_SEMICOLON))
				{
					ParseExpression();
					TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				}
			}
			else if (TK_Check(KW_SWITCH))
			{
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				ParseExpression();
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);

				TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
				do
				{
					if (TK_Check(KW_CASE))
					{
						//FIXME
						ParseExpression(false);
						TK_Expect(PU_COLON, ERR_MISSING_COLON);
						continue;
					}
					if (TK_Check(KW_DEFAULT))
					{
						TK_Expect(PU_COLON, ERR_MISSING_COLON);
						continue;
					}
					ParseStatement();
				} while (!TK_Check(PU_RBRACE));
			}
			else
			{
				ParseExpression(true);
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			}
			break;
		case TK_PUNCT:
			if (TK_Check(PU_LBRACE))
			{
				ParseCompoundStatement();
				break;
			}
		default:
			ParseExpression(true);
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			break;
	}
}

//==========================================================================
//
//	ParseLocalVar
//
//==========================================================================

void ParseLocalVar(void)
{
	do
	{
		while (TK_Check(PU_ASTERISK));
#ifdef REF_CPP
		while (TK_Check(PU_AND));
#endif
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError(ERR_INVALID_IDENTIFIER, "variable name expected");
			continue;
		}
		TK_NextToken();

		if (TK_Check(PU_LINDEX))
		{
			EvalConstExpression(ev_int);
			TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
		}
		//  inicializÆcija
		else if (TK_Check(PU_ASSIGN))
		{
			ParseExpression();
		}
	} while (TK_Check(PU_COMMA));
}

//==========================================================================
//
//	ParseCompoundStatement
//
//==========================================================================

static void ParseCompoundStatement(void)
{
	while (!TK_Check(PU_RBRACE))
	{
		ParseStatement();
	}
}

//==========================================================================
//
//	ParseGlobalData
//
//==========================================================================

static TType* ParseGlobalData(TType *type, int *dst);

static bool ParseFields(TType *type, int *dst)
{
	if (type->aux_type)
	{
		if (!ParseFields(type->aux_type, dst))
		{
			return false;
		}
	}
	for (int i = 0; i < type->numfields; i++)
	{
		field_t *field = &type->fields[i];
		ParseGlobalData(field->type, (int*)((byte*)dst + field->ofs));
		if (!TK_Check(PU_COMMA))
		{
			TK_Expect(PU_RBRACE, ERR_MISSING_RBRACE);
			return false;
		}
		if (TK_Check(PU_RBRACE))
		{
			return false;
		}
	}
	return true;
}

static TType* ParseGlobalData(TType *type, int *dst)
{
	int numinitialisers;

	switch (type->type)
	{
	 case ev_array:
		numinitialisers = 0;
		TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
		do
		{
			ParseGlobalData(type->aux_type,
				(int*)((byte*)dst + numinitialisers * type->aux_type->size));
			numinitialisers++;
			if (!TK_Check(PU_COMMA))
			{
				TK_Expect(PU_RBRACE, ERR_MISSING_RBRACE);
				break;
			}
		} while (!TK_Check(PU_RBRACE));
		if (!type->size)
		{
			type = MakeArrayType(type->aux_type, numinitialisers);
		}
		else
		{
			if (numinitialisers > type->size / type->aux_type->size)
			{
				ERR_Exit(ERR_NONE, true, "Too many initialisers.");
			}
		}
		break;

	 case ev_struct:
		TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
		if (ParseFields(type, dst))
		{
			TK_Expect(PU_RBRACE, ERR_MISSING_RBRACE);
		}
		break;

	 case ev_vector:
		TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
		if (ParseFields(type, dst))
		{
			TK_Expect(PU_RBRACE, ERR_MISSING_RBRACE);
		}
		break;

	 default:
		*dst = EvalConstExpression(type->type);
		if (type->type == ev_string)
		{
			globalinfo[dst - globals] = 1;
		}
		else if (type->type == ev_function)
		{
			globalinfo[dst - globals] = 2;
		}
		else if (type->type == ev_classid)
		{
			globalinfo[dst - globals] = 3;
		}
		else if (type->type == ev_name)
		{
			globalinfo[dst - globals] = 4;
		}
	}
	return type;
}

//==========================================================================
//
//	ParseArrayDimensions
//
//==========================================================================

static TType *ParseArrayDimensions(TType *type)
{
	if (TK_Check(PU_LINDEX))
	{
		int		size;

		if (TK_Check(PU_RINDEX))
		{
			size = 0;
		}
		else
		{
			size = EvalConstExpression(ev_int);
			TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
		}
		type = ParseArrayDimensions(type);
		if (!type->size)
		{
			ParseError("Empty sub-array type");
		}
		type = MakeArrayType(type, size);
	}
	return type;
}

//==========================================================================
//
//	ParseDef
//
//==========================================================================

static void ParseDef(TType *type, bool IsNative)
{
	FName		Name;
	int			num;
	TType		*t;
	field_t		*method = NULL;

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
		ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
	}

	numlocaldefs = 1;
	int localsofs = 0;

	TType *ctype = CheckForType();
	if (ctype)
	{
		TK_Expect(PU_DCOLON, ERR_NONE);
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError("Method name expected");
		}
		Name = tk_Name;
		method = CheckForField(ctype, false);
		if (!method || method->type->type != ev_method)
		{
			ParseError("No such method");
		}
		if (!method)
		{
			TK_NextToken();
		}
		localsofs = 1;
	}
	else
	{
		Name = tk_Name;
		TK_NextToken();
	}

	if (method)
	{
		TK_Expect(PU_LPAREN, ERR_NONE);
	}
	else if (!TK_Check(PU_LPAREN))
	{
		if (IsNative)
		{
			ERR_Exit(ERR_MISSING_LPAREN, true, NULL);
		}
		do
		{
			if (Name == NAME_None)
			{
				t = type;
				if (TK_Check(PU_ASTERISK))
				{
					t = MakePointerType(t);
				}
#ifdef REF_CPP
				if (TK_Check(PU_AND))
				{
					t = MakeReferenceType(t);
				}
#endif
				if (tk_Token != TK_IDENTIFIER)
				{
					ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
				}
				Name = tk_Name;
			}
		   	if (t == &type_void)
			{
				ERR_Exit(ERR_VOID_VAR, true, NULL);
			}
			if (t->type == ev_class)
			{
				t = MakeReferenceType(t);
			}
			if (CheckForGlobalVar(Name) ||
				CheckForFunction(NULL, Name) ||
				CheckForConstant(Name) != -1)
			{
				ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Symbol: %s",
					*Name);
			}
			t = ParseArrayDimensions(t);
			// inicializÆcija
			if (TK_Check(PU_ASSIGN))
			{
				t = ParseGlobalData(t, globals + numglobals);
			}
			if (!t->size)
			{
				ERR_Exit(ERR_NONE, true, "Size of type = 0.");
			}
			globaldefs[numglobaldefs].Name = Name;
			globaldefs[numglobaldefs].type = t;
			globaldefs[numglobaldefs].ofs = numglobals;
			numglobals += TypeSize(t) / 4;
			numglobaldefs++;
			Name = NAME_None;
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		return;
	}

	if (t->type == ev_class)
	{
		t = MakeReferenceType(t);
	}

	TType functype;
	memset(&functype, 0, sizeof(TType));
	functype.type = ev_function;
	functype.size = 4;
	functype.aux_type = t;

	if (!ctype)
	{
		if (CheckForGlobalVar(Name))
		{
			ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Symbol: %s", *Name);
		}
		if (CheckForConstant(Name) != -1)
		{
			ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Symbol: %s", *Name);
		}
	}

	do
	{
		if (TK_Check(PU_VARARGS))
		{
			functype.num_params |= PF_VARARGS;
			break;
		}

		type = CheckForType();

		if (!type)
		{
			if (numlocaldefs == 1)
			{
				break;
			}
			ERR_Exit(ERR_BAD_VAR_TYPE, true, NULL);
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
		if (numlocaldefs == 1 && type == &type_void)
		{
			break;
		}
		TypeCheckPassable(type);

		if (numlocaldefs - 1 == MAX_PARAMS)
		{
			ERR_Exit(ERR_PARAMS_OVERFLOW, true, NULL);
		}
   		if (tk_Token == TK_IDENTIFIER)
		{
			numlocaldefs++;
			TK_NextToken();
		}
		functype.param_types[functype.num_params] = type;
		functype.num_params++;
		localsofs += TypeSize(type) / 4;
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	functype.params_size = localsofs;

	num = CheckForFunction(ctype, Name);
	if (num)
	{
		if (IsNative && functions[num].first_statement > 0)
		{
	   		ERR_Exit(ERR_FUNCTION_REDECLARED, true,
	   				 "Declared function defined as native.");
		}
		if (functions[num].type != FindType(&functype))
		{
	   		ERR_Exit(ERR_TYPE_MISTMATCH, true, NULL);
		}
	}
	else
	{
		num = numfunctions;
		functions[num].Name = Name;
		functions[num].OuterClass = ctype;
		functions[num].first_statement = 0;
		functions[num].type = FindType(&functype);
		numfunctions++;
	}
	if (IsNative)
	{
		functions[num].first_statement = -numbuiltins;
		functions[num].flags |= FUNC_Native;
		numbuiltins++;
	}
	if (method)
	{
		TType methodtype;
		memcpy(&methodtype, &functype, sizeof(TType));
		methodtype.type = ev_method;
		if (method->type != FindType(&methodtype))
		{
	   		ERR_Exit(ERR_TYPE_MISTMATCH, true, NULL);
		}
		method->func_num = num;
	}

	if (TK_Check(PU_LBRACE))
	{
	   	ParseCompoundStatement();
	}
	else
	{
		//  Funkcijas prototips
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		return;
	}
}

//==========================================================================
//
//	ParseMethodDef
//
//==========================================================================

void ParseMethodDef(TType *t, field_t *method, field_t *otherfield,
	TType *class_type, int FuncFlags)
{
	if (t->type == ev_class)
	{
		t = MakeReferenceType(t);
	}
	if (t != &type_void)
	{
		//	Funkcijas atgri÷amajam tipam jÆbÝt void vai arØ ar izmñru 4
		TypeCheckPassable(t);
	}

	numlocaldefs = 1;
	int localsofs = 1;

	TType functype;
	memset(&functype, 0, sizeof(TType));
	functype.type = ev_function;
	functype.size = 4;
	functype.aux_type = t;

	do
	{
		if (TK_Check(PU_VARARGS))
		{
			functype.num_params |= PF_VARARGS;
			break;
		}

		TType *type = CheckForType();

		if (!type)
		{
			if (functype.num_params == 0)
			{
				break;
			}
			ERR_Exit(ERR_BAD_VAR_TYPE, true, NULL);
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
			numlocaldefs++;
			TK_NextToken();
		}
		functype.param_types[functype.num_params] = type;
		functype.num_params++;
		localsofs += TypeSize(type) / 4;
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	functype.params_size = localsofs;

	TType methodtype;
	memcpy(&methodtype, &functype, sizeof(TType));
	methodtype.type = ev_method;
	method->type = FindType(&methodtype);
	if (otherfield)
	{
		if (otherfield->type != method->type)
		{
			ParseError("Method redefined with different type");
		}
		method->ofs = otherfield->ofs;
	}
	else
	{
		method->ofs = class_type->num_methods;
		class_type->num_methods++;
	}
	class_type->numfields++;

	if (CheckForFunction(class_type, method->Name))
	{
		ERR_Exit(ERR_FUNCTION_REDECLARED, true,
			 "Function: %s::%s", *class_type->Name, *method->Name);
	}

	int num = numfunctions;
	numfunctions++;
	method->func_num = num;
	functions[num].Name = method->Name;
	functions[num].OuterClass = class_type;
	functions[num].type = FindType(&functype);
	functions[num].first_statement = 0;
	functions[num].flags = FuncFlags;

	if (FuncFlags & FUNC_Native)
	{
		functions[num].first_statement = -numbuiltins;
		numbuiltins++;
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		return;
	}

	if (TK_Check(PU_LBRACE))
	{
	   	ParseCompoundStatement();
	}
	else
	{
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	}
}

//==========================================================================
//
//	ParseStateCode
//
//==========================================================================

int ParseStateCode(TType *class_type)
{
	numlocaldefs = 1;

	TType functype;
	memset(&functype, 0, sizeof(TType));
	functype.type = ev_function;
	functype.size = 4;
	functype.aux_type = &type_void;
	functype.params_size = 1;

	int num = numfunctions;
	numfunctions++;
	functions[num].Name = NAME_None;
	functions[num].OuterClass = class_type;
	functions[num].type = FindType(&functype);

	if (TK_Check(PU_LBRACE))
	{
	   	ParseCompoundStatement();
	}
	else
	{
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	}
	return num;
}

//==========================================================================
//
//	ParseDefaultProperties
//
//==========================================================================

void ParseDefaultProperties(field_t *method, TType *class_type)
{
	numlocaldefs = 1;

	TType functype;
	memset(&functype, 0, sizeof(TType));
	functype.type = ev_function;
	functype.size = 4;
	functype.aux_type = &type_void;
	functype.params_size = 1;

	TType methodtype;
	memcpy(&methodtype, &functype, sizeof(TType));
	methodtype.type = ev_method;
	method->type = FindType(&methodtype);
	method->ofs = 0;
	method->Name = NAME_None;
	class_type->numfields++;

	if (CheckForFunction(class_type, class_type->Name))
	{
		ERR_Exit(ERR_FUNCTION_REDECLARED, true,
			 "Function: %s", *class_type->Name, *class_type->Name);
	}

	int num = numfunctions;
	numfunctions++;
	method->func_num = num;
	functions[num].OuterClass = class_type;
	functions[num].type = FindType(&functype);
	functions[num].first_statement = 0;

	if (TK_Check(PU_LBRACE))
	{
	   	ParseCompoundStatement();
	}
	else
	{
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	}
}

//==========================================================================
//
//	AddConstant
//
//==========================================================================

void AddConstant(FName Name, int value)
{
	if (CheckForGlobalVar(Name) || CheckForFunction(NULL, Name) ||
		CheckForConstant(Name) != -1)
	{
		ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Symbol: %s", *Name);
	}
	if (numconstants == MAX_CONSTANTS)
	{
		ERR_Exit(ERR_TOO_MENY_CONSTANTS, true, NULL);
	}
	Constants[numconstants].Name = Name;
	Constants[numconstants].value = value;
	int hash = GetTypeHash(Name) & 255;
	Constants[numconstants].HashNext = ConstantsHash[hash];
	ConstantsHash[hash] = &Constants[numconstants];
	numconstants++;
}

//==========================================================================
//
//	PA_Parse
//
//==========================================================================

void PA_Parse(void)
{
	boolean		done;
	TType		*type;

	dprintf("Compiling pass 1\n");

	numconstants = 0;

	//  Add empty function for default constructors
	TType functype;
	memset(&functype, 0, sizeof(TType));
	functype.type = ev_function;
	functype.size = 4;
	functype.aux_type = &type_void;
	functype.params_size = 1; // this pointer

	functions[numfunctions].Name = NAME_None;
	functions[numfunctions].OuterClass = NULL;
	functions[numfunctions].type = FindType(&functype);
	functions[numfunctions].num_locals = 1;
	numfunctions++;

	TK_NextToken();
	done = false;
	while (!done)
	{
		switch(tk_Token)
		{
			case TK_EOF:
				done = true;
				break;
			case TK_KEYWORD:
				type = CheckForType();
				if (type)
				{
					ParseDef(type, false);
				}
				else if (TK_Check(KW_NATIVE))
				{
					type = CheckForType();
					if (type)
					{
						ParseDef(type, true);
					}
					else
					{
						ERR_Exit(ERR_INVALID_DECLARATOR, true, "Symbol \"%s\"", tk_String);
					}
				}
				else if (TK_Check(KW_ENUM))
				{
					int val;
					FName Name;

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
							val = EvalConstExpression(ev_int);
						}
						AddConstant(Name, val);
						val++;
					} while (TK_Check(PU_COMMA));
					TK_Expect(PU_RBRACE, ERR_MISSING_RBRACE);
					TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				}
				else if (TK_Check(KW_STRUCT))
				{
					ParseStruct();
				}
				else if (TK_Check(KW_CLASS))
				{
					ParseClass();
				}
				else if (TK_Check(KW_ADDFIELDS))
				{
					AddFields();
				}
				else if (TK_Check(KW_VECTOR))
				{
					ParseVector();
				}
				else if (TK_Check(KW_TYPEDEF))
				{
					ParseTypeDef();
				}
				else if (TK_Check(KW_STATES))
				{
				   	ParseStates(NULL);
				}
				else
				{
					ERR_Exit(ERR_INVALID_DECLARATOR, true, "Symbol \"%s\"", tk_String);
				}
				break;

			case TK_IDENTIFIER:
				type = CheckForType();
				if (type)
				{
					ParseDef(type, false);
				}
				else
				{
					ERR_Exit(ERR_INVALID_DECLARATOR, true, "Identifier \"%s\"", tk_String);
				}
				break;

			default:
				ERR_Exit(ERR_INVALID_DECLARATOR, true, "Token type %d, symbol \"%s\"", tk_Token, tk_String);
				break;
	   	}
	}

	if (NumErrors)
	{
		ERR_Exit(ERR_NONE, false, NULL);
	}
}

} // namespace Pass1

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2002/08/24 14:45:38  dj_jl
//	2 pass compiling.
//
//**************************************************************************
