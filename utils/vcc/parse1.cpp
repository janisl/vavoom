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
			SkipExpressionPriority13();
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
		if (TK_Check(KW_SELF) || TK_Check(KW_NONE) || TK_Check(KW_NULL) ||
			TK_Check(KW_TRUE) || TK_Check(KW_FALSE))
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
		if (TK_Check(PU_DCOLON))
		{
			if (tk_Token == TK_IDENTIFIER)
			{
				TK_NextToken();
				return;
			}
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
//	ParseMethodDef
//
//==========================================================================

void ParseMethodDef(const TType& t, VField* method, VField* otherfield,
	VClass* InClass, int Modifiers)
{
	if (t.type != ev_void)
	{
		//	Function's rturn type must be void, vector or with size 4
		t.CheckPassable();
	}

	numlocaldefs = 1;
	int localsofs = 1;

	Modifiers = TModifiers::Check(Modifiers, VMethod::AllowedModifiers);
	int FuncFlags = TModifiers::MethodAttr(Modifiers);

	if (FuncFlags & FUNC_Static)
	{
		if (!(FuncFlags & FUNC_Native))
		{
			ParseError("Currently only native methods can be static");
		}
		if (!(FuncFlags & FUNC_Final))
		{
			ParseError("Currently static methods must be final.");
		}
	}
	if (CheckForFunction(InClass, method->Name))
	{
		ERR_Exit(ERR_FUNCTION_REDECLARED, true,
			"Function: %s.%s", *InClass->Name, *method->Name);
	}

	VMethod* Func = new VMethod(method->Name, InClass, method->Loc);
	method->func = Func;
	Func->Flags = FuncFlags;
	Func->ReturnType = t;

	do
	{
		if (TK_Check(PU_VARARGS))
		{
			if (!(FuncFlags & FUNC_Native))
				ParseError("Only native methods can have varargs");
			Func->Flags |= FUNC_VarArgs;
			break;
		}

		TType type = CheckForType(InClass);

		if (type.type == ev_unknown)
		{
			if (Func->NumParams == 0)
			{
				break;
			}
			ERR_Exit(ERR_BAD_VAR_TYPE, true, NULL);
		}
		while (TK_Check(PU_ASTERISK))
		{
			type = MakePointerType(type);
		}
		if (Func->NumParams == 0 && type.type == ev_void)
		{
//ParseWarning("This is ugly %d", un);
			break;
		}
		type.CheckPassable();

		if (Func->NumParams == MAX_PARAMS)
		{
			ERR_Exit(ERR_PARAMS_OVERFLOW, true, NULL);
		}
   		if (tk_Token == TK_IDENTIFIER)
		{
			numlocaldefs++;
			TK_NextToken();
		}
		Func->ParamTypes[Func->NumParams] = type;
		Func->NumParams++;
		localsofs += type.GetSize() / 4;
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	Func->ParamsSize = localsofs;

	method->type = TType(ev_method);
	if (otherfield)
	{
		VMethod* BaseFunc = otherfield->func;
		if (BaseFunc->Flags & FUNC_Final)
		{
			ParseError("Method already has been declared as final and cannot be overriden.");
		}
		if (!BaseFunc->ReturnType.Equals(Func->ReturnType))
		{
			ParseError("Method redefined with different return type");
		}
		else if (BaseFunc->NumParams != Func->NumParams)
		{
			ParseError("Method redefined with different number of arguments");
		}
		else for (int i = 0; i < Func->NumParams; i++)
			if (!BaseFunc->ParamTypes[i].Equals(Func->ParamTypes[i]))
			{
				ParseError("Type of argument %d differs from base class", i + 1);
			}
	}

	if (FuncFlags & FUNC_Native)
	{
		numbuiltins++;
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		return;
	}

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	SkipCompoundStatement();
}

//==========================================================================
//
//	ParseDelegate
//
//==========================================================================

void ParseDelegate(const TType& t, VField* method, VField* otherfield,
	VClass* InClass, int FuncFlags)
{
	if (t.type != ev_void)
	{
		//	Function's rturn type must be void, vector or with size 4
		t.CheckPassable();
	}

	int localsofs = 1;

	VMethod* Func = new VMethod(NAME_None, method, method->Loc);
	method->func = Func;
	Func->Flags = FuncFlags;
	Func->ReturnType = t;

	TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
	do
	{
		TType type = CheckForType(InClass);

		if (type.type == ev_unknown)
		{
			if (Func->NumParams == 0)
			{
				break;
			}
			ERR_Exit(ERR_BAD_VAR_TYPE, true, NULL);
		}
		while (TK_Check(PU_ASTERISK))
		{
			type = MakePointerType(type);
		}
		type.CheckPassable();

		if (Func->NumParams == MAX_PARAMS)
		{
			ERR_Exit(ERR_PARAMS_OVERFLOW, true, NULL);
		}
   		if (tk_Token == TK_IDENTIFIER)
		{
			TK_NextToken();
		}
		Func->ParamTypes[Func->NumParams] = type;
		Func->NumParams++;
		localsofs += type.GetSize() / 4;
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	Func->ParamsSize = localsofs;

	method->type = TType(ev_delegate);
	method->type.Function = Func;

	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
}

//==========================================================================
//
//	ParseStateCode
//
//==========================================================================

VMethod* ParseStateCode(VClass* InClass, VState* InState)
{
	numlocaldefs = 1;

	VMethod* Func = new VMethod(NAME_None, InState, InState->Loc);
	Func->ReturnType = TType(ev_void);
	Func->ParamsSize = 1;

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	SkipCompoundStatement();
	return Func;
}

//==========================================================================
//
//	ParseDefaultProperties
//
//==========================================================================

void ParseDefaultProperties(VField *method, VClass* InClass)
{
	numlocaldefs = 1;

	method->type = TType(ev_method);
	method->Name = NAME_None;

	VMethod* Func = new VMethod(NAME_None, InClass, method->Loc);
	method->func = Func;
	Func->ReturnType = TType(ev_void);
	Func->ParamsSize = 1;

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	SkipCompoundStatement();
}

//==========================================================================
//
//	AddConstant
//
//==========================================================================

void AddConstant(VClass* InClass, VName Name, int type, int value)
{
	if (CurrentPass == 2)
		ParseError("Add constant in pass 2");
	if (CheckForConstant(InClass, Name))
	{
		ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Symbol: %s", *Name);
	}
	VConstant* cDef = new VConstant(Name, InClass ?
		(VMemberBase*)InClass : (VMemberBase*)CurrentPackage, tk_Location);
	cDef->Type = (EType)type;
	cDef->value = value;
}

//==========================================================================
//
//	PA_Parse
//
//==========================================================================

void PA_Parse()
{
	bool		done;

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
			if (TK_Check(KW_IMPORT))
			{
				if (tk_Token != TK_IDENTIFIER)
				{
					ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
				}
				LoadPackage(tk_Name);
				TK_NextToken();
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			}
			else if (TK_Check(KW_ENUM))
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
						val = EvalConstExpression(NULL, ev_int);
					}
					AddConstant(NULL, Name, ev_int, val);
					val++;
				} while (TK_Check(PU_COMMA));
				TK_Expect(PU_RBRACE, ERR_MISSING_RBRACE);
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			}
			else if (TK_Check(KW_STRUCT))
			{
				ParseStruct(NULL, false);
			}
			else if (TK_Check(KW_CLASS))
			{
				ParseClass();
			}
			else if (TK_Check(KW_VECTOR))
			{
				ParseStruct(NULL, true);
			}
			else
			{
				ERR_Exit(ERR_INVALID_DECLARATOR, true, "Symbol \"%s\"", tk_String);
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
