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

static void SkipCompoundStatement();
void SkipExpression(bool bLocals = false);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

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

void ParseMethodDef(const TType& t, VName MName, TLocation MethodLoc,
	VMethod* BaseMethod, VClass* InClass, int Modifiers)
{
	if (t.type != ev_void)
	{
		//	Function's return type must be void, vector or with size 4
		t.CheckPassable();
	}

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
	if (CheckForFunction(InClass, MName))
	{
		ERR_Exit(ERR_FUNCTION_REDECLARED, true,
			"Function: %s.%s", *InClass->Name, *MName);
	}

	VMethod* Func = new VMethod(MName, InClass, MethodLoc);
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
			TK_NextToken();
		}
		Func->ParamTypes[Func->NumParams] = type;
		Func->NumParams++;
		localsofs += type.GetSize() / 4;
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	Func->ParamsSize = localsofs;

	if (BaseMethod)
	{
		if (BaseMethod->Flags & FUNC_Final)
		{
			ParseError("Method already has been declared as final and cannot be overriden.");
		}
		if (!BaseMethod->ReturnType.Equals(Func->ReturnType))
		{
			ParseError("Method redefined with different return type");
		}
		else if (BaseMethod->NumParams != Func->NumParams)
		{
			ParseError("Method redefined with different number of arguments");
		}
		else for (int i = 0; i < Func->NumParams; i++)
			if (!BaseMethod->ParamTypes[i].Equals(Func->ParamTypes[i]))
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

void ParseDelegate(const TType& t, VField* method, VClass* InClass,
	int FuncFlags)
{
	if (t.type != ev_void)
	{
		//	Function's return type must be void, vector or with size 4
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

void ParseDefaultProperties(VClass* InClass)
{
	VMethod* Func = new VMethod(NAME_None, InClass, tk_Location);
	Func->ReturnType = TType(ev_void);
	Func->ParamsSize = 1;
	InClass->DefaultProperties = Func;

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
