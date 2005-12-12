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

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void SkipCompoundStatement();

static void SkipExpressionPriority2();
static void SkipExpressionPriority13();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int			numlocaldefs = 1;

static bool			CheckForLocal;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	SkipLocalVar
//
//==========================================================================

static void SkipLocalVar()
{
	do
	{
		while (TK_Check(PU_ASTERISK));
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
			SkipExpressionPriority13();
		}
	} while (TK_Check(PU_COMMA));
}

//==========================================================================
//
//	SkipFunctionCall
//
//==========================================================================

static void SkipFunctionCall()
{
	if (!TK_Check(PU_RPAREN))
	{
		do
		{
			SkipExpressionPriority13();
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
}

//==========================================================================
//
//	SkipExpressionPriority0
//
//==========================================================================

static void SkipExpressionPriority0()
{
	bool		bLocals;

	bLocals = CheckForLocal;
	CheckForLocal = false;
   	switch (tk_Token)
	{
	case TK_INTEGER:
	case TK_FLOAT:
	case TK_NAME:
	case TK_STRING:
		TK_NextToken();
		return;

	case TK_PUNCT:
	   	if (TK_Check(PU_LPAREN))
		{
			SkipExpressionPriority13();
			TK_Expect(PU_RPAREN, ERR_BAD_EXPR);
			return;
		}

		if (TK_Check(PU_DCOLON))
		{
			if (tk_Token != TK_IDENTIFIER)
				ParseError("Identifier expected");
			TK_NextToken();
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			SkipFunctionCall();
			return;
		}
		break;

	case TK_KEYWORD:
		if (TK_Check(KW_VECTOR))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			SkipExpressionPriority13();
			TK_Expect(PU_COMMA, ERR_BAD_EXPR);
			SkipExpressionPriority13();
			TK_Expect(PU_COMMA, ERR_BAD_EXPR);
			SkipExpressionPriority13();
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			return;
		}
		if (TK_Check(KW_SELF) || TK_Check(KW_NONE) || TK_Check(KW_NULL))
		{
			return;
		}
		if (bLocals)
		{
			TK_NextToken();
			SkipLocalVar();
			return;
		}
		break;

	case TK_IDENTIFIER:
		TK_NextToken();
		if (TK_Check(PU_LPAREN))
		{
			SkipFunctionCall();
			return;
		}
		if (bLocals && (tk_Token == TK_IDENTIFIER ||
			(tk_Token == TK_PUNCT && tk_Punct == PU_ASTERISK)))
		{
			SkipLocalVar();
			return;
		}
		break;

	default:
	   	break;
	}
}

//==========================================================================
//
//	SkipExpressionPriority1
//
//==========================================================================

static void SkipExpressionPriority1()
{
	SkipExpressionPriority0();
	bool done = false;
	do
	{
		if (TK_Check(PU_MINUS_GT) || TK_Check(PU_DOT))
		{
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError(ERR_INVALID_IDENTIFIER);
			}
			else
			{
				TK_NextToken();
			}
		}
		else if (TK_Check(PU_LINDEX))
		{
			SkipExpressionPriority13();
			TK_Expect(PU_RINDEX, ERR_BAD_ARRAY);
		}
		else if (TK_Check(PU_LPAREN))
		{
			SkipFunctionCall();
		}
		else
		{
			done = true;
		}
	} while (!done);
}

//==========================================================================
//
//	SkipExpressionPriority2
//
//==========================================================================

static void SkipExpressionPriority2()
{
   	if (tk_Token == TK_PUNCT)
	{
		if (TK_Check(PU_PLUS) || TK_Check(PU_MINUS) ||
			TK_Check(PU_NOT) || TK_Check(PU_TILDE) ||
			TK_Check(PU_AND) || TK_Check(PU_ASTERISK) ||
			TK_Check(PU_INC) || TK_Check(PU_DEC))
		{
		   	SkipExpressionPriority2();
			return;
		}
	}

	SkipExpressionPriority1();
	TK_Check(PU_INC) || TK_Check(PU_DEC);
}

//==========================================================================
//
//	SkipExpressionPriority12
//
//==========================================================================

static void SkipExpressionPriority12()
{
	SkipExpressionPriority2();
	while (TK_Check(PU_OR_LOG) || TK_Check(PU_AND_LOG) ||
		TK_Check(PU_OR) || TK_Check(PU_XOR) || TK_Check(PU_AND) ||
		TK_Check(PU_EQ) || TK_Check(PU_NE) ||
		TK_Check(PU_LT) || TK_Check(PU_LE) ||
		TK_Check(PU_GT) || TK_Check(PU_GE) ||
		TK_Check(PU_LSHIFT) || TK_Check(PU_RSHIFT) ||
		TK_Check(PU_PLUS) || TK_Check(PU_MINUS) ||
		TK_Check(PU_ASTERISK) || TK_Check(PU_SLASH) || TK_Check(PU_PERCENT))
	{
		SkipExpressionPriority2();
	}
}

//==========================================================================
//
//	SkipExpressionPriority13
//
//==========================================================================

static void SkipExpressionPriority13()
{
	SkipExpressionPriority12();
   	if (TK_Check(PU_QUEST))
	{
		SkipExpressionPriority13();
		TK_Expect(PU_COLON, ERR_MISSING_COLON);
		SkipExpressionPriority13();
	}
}

//==========================================================================
//
//	SkipExpressionPriority14
//
//==========================================================================

static void SkipExpressionPriority14()
{
	SkipExpressionPriority13();
	if (TK_Check(PU_ASSIGN) ||
		TK_Check(PU_ADD_ASSIGN) ||
		TK_Check(PU_MINUS_ASSIGN) ||
		TK_Check(PU_MULTIPLY_ASSIGN) ||
		TK_Check(PU_DIVIDE_ASSIGN) ||
		TK_Check(PU_MOD_ASSIGN) ||
		TK_Check(PU_AND_ASSIGN) ||
		TK_Check(PU_OR_ASSIGN) ||
		TK_Check(PU_XOR_ASSIGN) ||
		TK_Check(PU_LSHIFT_ASSIGN) ||
		TK_Check(PU_RSHIFT_ASSIGN))
	{
		SkipExpressionPriority13();
	}
}

//==========================================================================
//
//	SkipExpression
//
//==========================================================================

static void SkipExpression(bool bLocals = false)
{
	CheckForLocal = bLocals;
	SkipExpressionPriority14();
}

//==========================================================================
//
//	SkipStatement
//
//==========================================================================

static void SkipStatement()
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
				SkipExpression();
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				SkipStatement();
				if (TK_Check(KW_ELSE))
				{
					SkipStatement();
				}
			}
			else if (TK_Check(KW_WHILE))
			{
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				SkipExpression();
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				SkipStatement();
			}
			else if (TK_Check(KW_DO))
			{
				SkipStatement();
				TK_Expect(KW_WHILE, ERR_BAD_DO_STATEMENT);
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				SkipExpression();
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			}
			else if (TK_Check(KW_FOR))
			{
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				do
				{
					SkipExpression();
				} while (TK_Check(PU_COMMA));
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				SkipExpression();
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				do
				{
					SkipExpression();
				} while (TK_Check(PU_COMMA));
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				SkipStatement();
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
					SkipExpression();
					TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				}
			}
			else if (TK_Check(KW_SWITCH))
			{
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				SkipExpression();
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);

				TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
				do
				{
					if (TK_Check(KW_CASE))
					{
						//FIXME
						SkipExpression(false);
						TK_Expect(PU_COLON, ERR_MISSING_COLON);
						continue;
					}
					if (TK_Check(KW_DEFAULT))
					{
						TK_Expect(PU_COLON, ERR_MISSING_COLON);
						continue;
					}
					SkipStatement();
				} while (!TK_Check(PU_RBRACE));
			}
			else
			{
				SkipExpression(true);
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			}
			break;
		case TK_PUNCT:
			if (TK_Check(PU_LBRACE))
			{
				SkipCompoundStatement();
				break;
			}
		default:
			SkipExpression(true);
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			break;
	}
}

//==========================================================================
//
//	SkipCompoundStatement
//
//==========================================================================

static void SkipCompoundStatement()
{
	while (!TK_Check(PU_RBRACE))
	{
		SkipStatement();
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
	for (int i = 0; i < type->Struct->NumFields; i++)
	{
		field_t* field = &type->Struct->Fields[i];
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
				(int*)((byte*)dst + numinitialisers * TypeSize(type->aux_type)));
			numinitialisers++;
			if (!TK_Check(PU_COMMA))
			{
				TK_Expect(PU_RBRACE, ERR_MISSING_RBRACE);
				break;
			}
		} while (!TK_Check(PU_RBRACE));
		if (!type->array_dim)
		{
			type = MakeArrayType(type->aux_type, numinitialisers);
		}
		else
		{
			if (numinitialisers > type->array_dim)
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
			globalinfo[dst - &globals[0]] = GLOBALTYPE_String;
		}
		else if (type->type == ev_classid)
		{
			globalinfo[dst - &globals[0]] = GLOBALTYPE_Class;
		}
		else if (type->type == ev_name)
		{
			globalinfo[dst - &globals[0]] = GLOBALTYPE_Name;
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
		if (!TypeSize(type))
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

	t = type;
	while (TK_Check(PU_ASTERISK))
	{
		t = MakePointerType(t);
	}
	if (tk_Token != TK_IDENTIFIER)
	{
		ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
	}

	numlocaldefs = 1;
	int localsofs = 0;
	Name = tk_Name;
	TK_NextToken();

	if (!TK_Check(PU_LPAREN))
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
			if (CheckForGlobalVar(Name) != -1 ||
				CheckForFunction(NULL, Name) != -1 ||
				CheckForConstant(Name) != -1)
			{
				ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Symbol: %s",
					*Name);
			}
			t = ParseArrayDimensions(t);
			// inicializÆcija
			if (TK_Check(PU_ASSIGN))
			{
				t = ParseGlobalData(t, &globals[globals.Num()]);
			}
			if (!TypeSize(t))
			{
				ERR_Exit(ERR_NONE, true, "Size of type = 0.");
			}
			TGlobalDef* GlobalDef = new(globaldefs) TGlobalDef;
			GlobalDef->Name = Name;
			GlobalDef->type = t;
			GlobalDef->ofs = globals.Num();
			globals.Add(TypeSize(t) / 4);
			dprintf("Added global %s, %d bytes\n", *Name, TypeSize(t));
			Name = NAME_None;
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		return;
	}

	if (CheckForGlobalVar(Name) != -1)
	{
		ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Symbol: %s", *Name);
	}
	if (CheckForConstant(Name) != -1)
	{
		ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Symbol: %s", *Name);
	}
	if (CheckForFunction(NULL, Name) != -1)
	{
		ERR_Exit(ERR_FUNCTION_REDECLARED, true, "Function: %s", *Name);
	}

	num = functions.Num();
	new(functions) TFunction;
	functions[num].Name = Name;
	functions[num].OuterClass = NULL;
	functions[num].ReturnType = t;

	do
	{
		if (TK_Check(PU_VARARGS))
		{
			functions[num].NumParams |= PF_VARARGS;
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
		if (functions[num].NumParams == 0 && type == &type_void)
		{
			break;
		}
		TypeCheckPassable(type);

		if (functions[num].NumParams == MAX_PARAMS)
		{
			ERR_Exit(ERR_PARAMS_OVERFLOW, true, NULL);
		}
   		if (tk_Token == TK_IDENTIFIER)
		{
			numlocaldefs++;
			TK_NextToken();
		}
		functions[num].ParamTypes[functions[num].NumParams] = type;
		functions[num].NumParams++;
		localsofs += TypeSize(type) / 4;
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	functions[num].ParamsSize = localsofs;

	if (IsNative)
	{
		functions[num].FirstStatement = -numbuiltins;
		functions[num].Flags |= FUNC_Native;
		numbuiltins++;
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		return;
	}

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
   	SkipCompoundStatement();
}

//==========================================================================
//
//	ParseMethodDef
//
//==========================================================================

void ParseMethodDef(TType* t, field_t* method, field_t* otherfield,
	TClass* InClass, int FuncFlags)
{
	if (t != &type_void)
	{
		//	Function's rturn type must be void, vector or with size 4
		TypeCheckPassable(t);
	}

	numlocaldefs = 1;
	int localsofs = 1;

	if (CheckForFunction(InClass, method->Name) != -1)
	{
		ERR_Exit(ERR_FUNCTION_REDECLARED, true,
			"Function: %s.%s", *InClass->Name, *method->Name);
	}

	int num = functions.Num();
	method->func_num = num;
	TFunction& Func = *new(functions) TFunction;
	Func.Name = method->Name;
	Func.OuterClass = InClass;
	Func.Flags = FuncFlags;
	Func.ReturnType = t;

	TType methodtype;
	memset(&methodtype, 0, sizeof(TType));
	methodtype.type = ev_method;
	methodtype.aux_type = t;

	do
	{
		if (TK_Check(PU_VARARGS))
		{
			Func.NumParams |= PF_VARARGS;
			break;
		}

		TType *type = CheckForType();

		if (!type)
		{
			if (Func.NumParams == 0)
			{
				break;
			}
			ERR_Exit(ERR_BAD_VAR_TYPE, true, NULL);
		}
		while (TK_Check(PU_ASTERISK))
		{
			type = MakePointerType(type);
		}
		if (Func.NumParams == 0 && type == &type_void)
		{
			break;
		}
		TypeCheckPassable(type);

		if (Func.NumParams == MAX_PARAMS)
		{
			ERR_Exit(ERR_PARAMS_OVERFLOW, true, NULL);
		}
   		if (tk_Token == TK_IDENTIFIER)
		{
			numlocaldefs++;
			TK_NextToken();
		}
		Func.ParamTypes[Func.NumParams] = type;
		Func.NumParams++;
		localsofs += TypeSize(type) / 4;
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	Func.ParamsSize = localsofs;

	method->type = FindType(&methodtype);
	if (otherfield)
	{
		TFunction& BaseFunc = functions[otherfield->func_num];
		if (BaseFunc.ReturnType != Func.ReturnType)
		{
			ParseError("Method redefined with different return type");
		}
		else if (BaseFunc.NumParams != Func.NumParams)
		{
			ParseError("Method redefined with different number of arguments");
		}
		else for (int i = 0; i < Func.NumParams; i++)
			if (BaseFunc.ParamTypes[i] != Func.ParamTypes[i])
			{
				ParseError("Type of argument %d differs from base class", i + 1);
			}
		method->ofs = otherfield->ofs;
	}
	else
	{
		method->ofs = InClass->NumMethods;
		InClass->NumMethods++;
	}
	InClass->NumFields++;

	if (FuncFlags & FUNC_Native)
	{
		Func.FirstStatement = -numbuiltins;
		numbuiltins++;
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		return;
	}

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	SkipCompoundStatement();
}

//==========================================================================
//
//	ParseStateCode
//
//==========================================================================

int ParseStateCode(TClass* InClass)
{
	numlocaldefs = 1;

	int num = functions.Num();
	new(functions) TFunction;
	functions[num].Name = NAME_None;
	functions[num].OuterClass = InClass;
	functions[num].ReturnType = &type_void;
	functions[num].ParamsSize = 1;

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	SkipCompoundStatement();
	return num;
}

//==========================================================================
//
//	ParseDefaultProperties
//
//==========================================================================

void ParseDefaultProperties(field_t *method, TClass* InClass)
{
	numlocaldefs = 1;


	TType methodtype;
	memset(&methodtype, 0, sizeof(TType));
	methodtype.type = ev_method;
	methodtype.aux_type = &type_void;

	method->type = FindType(&methodtype);
	method->ofs = 0;
	method->Name = NAME_None;
	InClass->NumFields++;

	if (CheckForFunction(InClass, InClass->Name) != -1)
	{
		ERR_Exit(ERR_FUNCTION_REDECLARED, true,
			 "Function: %s.%s", *InClass->Name, *InClass->Name);
	}

	int num = functions.Num();
	new(functions) TFunction;
	method->func_num = num;
	functions[num].OuterClass = InClass;
	functions[num].ReturnType = &type_void;
	functions[num].ParamsSize = 1;

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	SkipCompoundStatement();
}

//==========================================================================
//
//	AddConstant
//
//==========================================================================

void AddConstant(FName Name, int value)
{
	if (CurrentPass == 2)
		ParseError("Add constant in pass 2");
	if (CheckForGlobalVar(Name) != -1 || CheckForFunction(NULL, Name) != -1 ||
		CheckForConstant(Name) != -1)
	{
		ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Symbol: %s", *Name);
	}
	constant_t* cDef = new(Constants) constant_t;
	cDef->Name = Name;
	cDef->value = value;
	int hash = GetTypeHash(Name) & 255;
	cDef->HashNext = ConstantsHash[hash];
	ConstantsHash[hash] = Constants.Num() - 1;
}

//==========================================================================
//
//	PA_Parse
//
//==========================================================================

void PA_Parse()
{
	boolean		done;
	TType		*type;

	dprintf("Compiling pass 1\n");

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
				ParseStruct(false);
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
				ParseStruct(true);
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

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2005/12/12 20:58:47  dj_jl
//	Removed compiler limitations.
//
//	Revision 1.5  2005/11/29 19:31:43  dj_jl
//	Class and struct classes, removed namespaces, beautification.
//	
//	Revision 1.4  2005/11/24 20:42:05  dj_jl
//	Renamed opcodes, cleanup and improvements.
//	
//	Revision 1.3  2003/03/08 12:47:52  dj_jl
//	Code cleanup.
//	
//	Revision 1.2  2002/09/07 16:36:38  dj_jl
//	Support bool in function args and return type.
//	Removed support for typedefs.
//	
//	Revision 1.1  2002/08/24 14:45:38  dj_jl
//	2 pass compiling.
//	
//**************************************************************************
