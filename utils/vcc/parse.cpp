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

#define MAX_BREAK		128
#define MAX_CONTINUE	128
#define MAX_CASE		128

// TYPES -------------------------------------------------------------------

struct breakInfo_t
{
	int		level;
	int		*addressPtr;
};

struct continueInfo_t
{
	int		level;
	int		*addressPtr;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void ParseMobjInfo(void);
void ParseStates(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void 	ParseCompoundStatement(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

char*					strings;
int						strofs;
localvardef_t			localdefs[MAX_LOCAL_DEFS];

constant_t				Constants[MAX_CONSTANTS];
int						numconstants;
int						ConstLookup[256];

TType					*ThisType;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int				numlocaldefs = 1;
static int				localsofs = 0;
static int				maxlocalsofs = 0;
static breakInfo_t		BreakInfo[MAX_BREAK];
static int 				BreakIndex;
static int				BreakLevel;
static continueInfo_t 	ContinueInfo[MAX_CONTINUE];
static int 				ContinueIndex;
static int				ContinueLevel;
static TType			*FuncRetType;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	CheckForGlobalVar
//
//==========================================================================

int CheckForGlobalVar(char* name)
{
	int		i;

	for (i=1; i<numglobaldefs; i++)
	{
		if (!strcmp(strings + globaldefs[i].s_name, name))
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

int CheckForFunction(char* name)
{
	int		i;

	for (i=1; i<numfunctions; i++)
	{
		if (!strcmp(strings + functions[i].s_name, name))
		{
			return i;
		}
	}
	return 0;
}

//==========================================================================
//
//	CheckForLocalVar
//
//==========================================================================

int CheckForLocalVar(char* name)
{
	int		i;

	for (i=1; i<numlocaldefs; i++)
	{
		if (!strcmp(localdefs[i].name, name))
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

int CheckForConstant(const char *name)
{
	for (int i = ConstLookup[(byte)name[0]]; i != -1; i = Constants[i].next)
	{
		if (!strcmp(name, Constants[i].name))
		{
			return i;
		}
	}
	return -1;
}

//==========================================================================
//
//	AddDrop
//
//==========================================================================

void AddDrop(TType *type)
{
	if (TypeSize(type) == 4)
	{
		AddStatement(OPC_DROP);
	}
	else if (type->type == ev_vector)
	{
		AddStatement(OPC_VDROP);
	}
	else if (type != &type_void)
	{
		ParseError("Expression's result type cannot be dropped");
	}
}

//==========================================================================
//
//	AddBreak
//
//==========================================================================

static void AddBreak(void)
{
	if (!BreakLevel)
	{
		ERR_Exit(ERR_MISPLACED_BREAK, true, NULL);
	}
	TK_Expect(";", ERR_MISSING_SEMICOLON);
	if (BreakIndex == MAX_BREAK)
	{
		ERR_Exit(ERR_BREAK_OVERFLOW, true, NULL);
	}
	BreakInfo[BreakIndex].level = BreakLevel;
	BreakInfo[BreakIndex].addressPtr = AddStatement(OPC_GOTO, 0);
	BreakIndex++;
}

//==========================================================================
//
// WriteBreaks
//
//==========================================================================

static void WriteBreaks(void)
{
	BreakLevel--;
	while (BreakIndex && BreakInfo[BreakIndex-1].level > BreakLevel)
	{
		*BreakInfo[--BreakIndex].addressPtr = CodeBufferSize;
	}
}

//==========================================================================
//
//	AddContinue
//
//==========================================================================

static void AddContinue(void)
{
	if (!ContinueLevel)
	{
		ERR_Exit(ERR_MISPLACED_CONTINUE, true, NULL);
	}
	TK_Expect(";", ERR_MISSING_SEMICOLON);
	if(ContinueIndex == MAX_CONTINUE)
	{
		ERR_Exit(ERR_CONTINUE_OVERFLOW, true, NULL);
	}
	ContinueInfo[ContinueIndex].level = ContinueLevel;
	ContinueInfo[ContinueIndex].addressPtr = AddStatement(OPC_GOTO, 0);
	ContinueIndex++;
}

//==========================================================================
//
// WriteContinues
//
//==========================================================================

static void WriteContinues(int address)
{
	ContinueLevel--;
	while (ContinueIndex && ContinueInfo[ContinueIndex-1].level > ContinueLevel)
	{
		*ContinueInfo[--ContinueIndex].addressPtr = address;
	}
}

//==========================================================================
//
// ProcessStatement
//
//==========================================================================

static void ParseStatement(void)
{
	TType		*t;

	switch(tk_Token)
	{
		case TK_EOF:
			ERR_Exit(ERR_UNEXPECTED_EOF, true, NULL);
			break;
		case TK_KEYWORD:
			if (TK_Check("if"))
			{
				int*		jumpAddrPtr1;
				int*		jumpAddrPtr2;

				TK_Expect("(", ERR_MISSING_LPAREN);
				TypeCheck1(ParseExpression());
				TK_Expect(")", ERR_MISSING_RPAREN);
				jumpAddrPtr1 = AddStatement(OPC_IFNOTGOTO, 0);
				ParseStatement();
				if (TK_Check("else"))
				{
					jumpAddrPtr2 = AddStatement(OPC_GOTO, 0);
					*jumpAddrPtr1 = CodeBufferSize;
					ParseStatement();
					*jumpAddrPtr2 = CodeBufferSize;
				}
				else
				{
					*jumpAddrPtr1 = CodeBufferSize;
				}
			}
			else if (TK_Check("while"))
			{
				int			topAddr;
				int*		outAddrPtr;

				BreakLevel++;
				ContinueLevel++;
				topAddr = CodeBufferSize;
				TK_Expect("(", ERR_MISSING_LPAREN);
				TypeCheck1(ParseExpression());
				TK_Expect(")", ERR_MISSING_RPAREN);
				outAddrPtr = AddStatement(OPC_IFNOTGOTO, 0);
				ParseStatement();
				AddStatement(OPC_GOTO, topAddr);

				*outAddrPtr = CodeBufferSize;
				WriteContinues(topAddr);
				WriteBreaks();
			}
			else if (TK_Check("do"))
			{
				int			topAddr;
				int			exprAddr;

				BreakLevel++;
				ContinueLevel++;
				topAddr = CodeBufferSize;
				ParseStatement();
				TK_Expect("while", ERR_BAD_DO_STATEMENT);
				TK_Expect("(", ERR_MISSING_LPAREN);
				exprAddr = CodeBufferSize;
				TypeCheck1(ParseExpression());
				TK_Expect(")", ERR_MISSING_RPAREN);
				TK_Expect(";", ERR_MISSING_SEMICOLON);
				AddStatement(OPC_IFGOTO, topAddr);
				WriteContinues(exprAddr);
				WriteBreaks();
			}
			else if (TK_Check("for"))
			{
				int*		jumpAddrPtr1;
				int*		jumpAddrPtr2;
				int			topAddr;
				int			contAddr;

				BreakLevel++;
				ContinueLevel++;
				TK_Expect("(", ERR_MISSING_LPAREN);
				do
				{
					t = ParseExpression();
				   	AddDrop(t);
				} while (TK_Check(","));
				TK_Expect(";", ERR_MISSING_SEMICOLON);
				topAddr = CodeBufferSize;
				t = ParseExpression();
				if (t == &type_void)
				{
					AddStatement(OPC_PUSHNUMBER, 1);
				}
				else
				{
					TypeCheck1(t);
				}
				jumpAddrPtr1 = AddStatement(OPC_IFGOTO, 0);
				jumpAddrPtr2 = AddStatement(OPC_GOTO, 0);
				TK_Expect(";", ERR_MISSING_SEMICOLON);
				contAddr = CodeBufferSize;
				do
				{
					t = ParseExpression();
				   	AddDrop(t);
				} while (TK_Check(","));
				AddStatement(OPC_GOTO, topAddr);
				TK_Expect(")", ERR_MISSING_RPAREN);
				*jumpAddrPtr1 = CodeBufferSize;
				ParseStatement();
				AddStatement(OPC_GOTO, contAddr);
				*jumpAddrPtr2 = CodeBufferSize;
				WriteContinues(contAddr);
				WriteBreaks();
			}
			else if (TK_Check("break"))
			{
				AddBreak();
			}
			else if (TK_Check("continue"))
			{
				AddContinue();
			}
			else if (TK_Check("return"))
			{
				if (TK_Check (";"))
				{
					if (FuncRetType != &type_void)
					{
						ERR_Exit(ERR_NO_RET_VALUE, true, NULL);
					}
					AddStatement(OPC_RETURN);
				}
				else
				{
					if (FuncRetType == &type_void)
					{
						ERR_Exit(ERR_VOID_RET, true, NULL);
					}
					t = ParseExpression();
					TK_Expect(";", ERR_MISSING_SEMICOLON);
					TypeCheck3(t, FuncRetType);
					if (TypeSize(t) == 4)
					{
						AddStatement(OPC_RETURNL);
					}
					else if (t->type == ev_vector)
					{
						AddStatement(OPC_RETURNV);
					}
					else
					{
						ERR_Exit(ERR_NONE, true, "Bad return type");
					}
				}
			}
			else if (TK_Check("switch"))
			{
				int*		switcherAddrPtr;
				int*		outAddrPtr;
				int			numcases;
				int			defaultAddress;
				int			i;
				struct
				{
					int value;
					int address;
				} CaseInfo[MAX_CASE];

				TK_Expect("(", ERR_MISSING_LPAREN);
				TypeCheck1(ParseExpression());
				TK_Expect(")", ERR_MISSING_RPAREN);

				switcherAddrPtr = AddStatement(OPC_GOTO, 0);
				defaultAddress = 0;
				numcases = 0;
				BreakLevel++;

				TK_Expect("{", ERR_MISSING_LBRACE);
				do
				{
					if (TK_Check("case"))
					{
						if (numcases == MAX_CASE)
						{
							ERR_Exit(ERR_CASE_OVERFLOW, true, NULL);
						}
						CaseInfo[numcases].value = EvalConstExpression(ev_int);
						CaseInfo[numcases].address = CodeBufferSize;
						numcases++;
						TK_Expect(":", ERR_MISSING_COLON);
						continue;
					}
					if (TK_Check("default"))
					{
						if (defaultAddress)
						{
							ERR_Exit(ERR_MULTIPLE_DEFAULT, true, NULL);
						}
						defaultAddress = CodeBufferSize;
						TK_Expect(":", ERR_MISSING_COLON);
						continue;
					}
					ParseStatement();
				} while (!TK_Check("}"));

				outAddrPtr = AddStatement(OPC_GOTO, 0);

				*switcherAddrPtr = CodeBufferSize;
				for (i = 0; i < numcases; i++)
				{
					AddStatement(OPC_CASEGOTO, CaseInfo[i].value,
									CaseInfo[i].address);
				}
				AddDrop(&type_int);

				if (defaultAddress)
				{
					AddStatement(OPC_GOTO, defaultAddress);
				}

				*outAddrPtr = CodeBufferSize;

				WriteBreaks();
			}
			else if (!strcmp(tk_String, "this"))
			{
				t = ParseExpression();
				AddDrop(t);
				TK_Expect(";", ERR_MISSING_SEMICOLON);
			}
			else
			{
				ERR_Exit(ERR_INVALID_STATEMENT, true, "Symbol: %s", tk_String);
			}
			break;
		case TK_PUNCT:
			if (TK_Check("{"))
			{
				ParseCompoundStatement();
				break;
			}
		default:
			t = ParseExpression();
			AddDrop(t);
			TK_Expect(";", ERR_MISSING_SEMICOLON);
			break;
	}
}

//==========================================================================
//
//	ParseCompoundStatement
//
//==========================================================================

static void ParseCompoundStatement(void)
{
	TType	*type;
	TType	*t;
	int		size;
	int		num_local_defs_on_start;
	int		num_locals_ofs_on_start;

	num_local_defs_on_start = numlocaldefs;
	num_locals_ofs_on_start = localsofs;
	do
	{
		type = CheckForType();
		if (type)
		{
			do
			{
				t = type;
				while (TK_Check("*"))
				{
					t = MakePointerType(t);
				}
				if (t == &type_void)
				{
					ParseError(ERR_BAD_VAR_TYPE);
				}
				if (t->type == ev_class)
				{
					ParseWarning("Class local variable");
				}
				if (tk_Token != TK_IDENTIFIER)
				{
					ParseError(ERR_INVALID_IDENTIFIER, "%s, variable name expected", tk_String);
					continue;
				}
				if (numlocaldefs == MAX_LOCAL_DEFS)
				{
					ParseError(ERR_LOCALS_OVERFLOW);
					continue;
				}
				strcpy(localdefs[numlocaldefs].name, tk_String);

				if (CheckForLocalVar(tk_String))
				{
					ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Identifier: %s", tk_String);
				}
				TK_NextToken();

				size = 1;
				if (TK_Check("["))
				{
					size = EvalConstExpression(ev_int);
					t = MakeArrayType(t, size);
					TK_Expect("]", ERR_MISSING_RFIGURESCOPE);
				}
				//  inicializÆcija
				else if (TK_Check("="))
				{
					AddStatement(OPC_LOCALADDRESS, localsofs);
					TType *t1 = ParseExpression();
					TypeCheck3(t, t1);
					AddStatement(OPC_ASSIGN);
					AddDrop(t1);
				}
				localdefs[numlocaldefs].type = t;
				localdefs[numlocaldefs].ofs = localsofs;
				//  MainØgo skaitu palielina pñc izteiksmes, lai ýo mainØgo
				// nebÝtu iespñjams izmantot izteiksmñ
				numlocaldefs++;
				localsofs += TypeSize(t) / 4;
				if (localsofs > 1024)
				{
					ParseWarning("Local vars > 1k");
				}
			} while (TK_Check(","));
			TK_Expect(";", ERR_MISSING_SEMICOLON);
		}
	} while (type);

	while (!TK_Check("}"))
	{
		ParseStatement();
	}

	if (maxlocalsofs < localsofs)
		maxlocalsofs = localsofs;
	numlocaldefs = num_local_defs_on_start;
	localsofs	= num_locals_ofs_on_start;
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
		if (!TK_Check(","))
		{
			TK_Expect("}", ERR_MISSING_RBRACE);
			return false;
		}
		if (TK_Check("}"))
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
		TK_Expect("{", ERR_MISSING_LBRACE);
		do
		{
			ParseGlobalData(type->aux_type,
				(int*)((byte*)dst + numinitialisers * type->aux_type->size));
			numinitialisers++;
			if (!TK_Check(","))
			{
				TK_Expect("}", ERR_MISSING_RBRACE);
				break;
			}
		} while (!TK_Check("}"));
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
		TK_Expect("{", ERR_MISSING_LBRACE);
		if (ParseFields(type, dst))
		{
			TK_Expect("}", ERR_MISSING_RBRACE);
		}
		break;

	 case ev_class: // FIXME allowed?
		TK_Expect("{", ERR_MISSING_LBRACE);
		if (ParseFields(type, dst))
		{
			TK_Expect("}", ERR_MISSING_RBRACE);
		}
		break;

	 case ev_vector:
		TK_Expect("{", ERR_MISSING_LBRACE);
		if (ParseFields(type, dst))
		{
			TK_Expect("}", ERR_MISSING_RBRACE);
		}
		break;

	 default:
		*dst = EvalConstExpression(type->type);
		if (type->type == ev_string)
		{
			globalinfo[dst - globals] = 1;
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
	if (TK_Check("["))
	{
		int		size;

		if (TK_Check("]"))
		{
			size = 0;
		}
		else
		{
			size = EvalConstExpression(ev_int);
			TK_Expect("]", ERR_MISSING_RFIGURESCOPE);
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

static void ParseDef(TType *type, boolean builtin)
{
	int			s_name = 0;
	int			num;
	TType		*t;
	field_t		*method = NULL;

	t = type;
	while (TK_Check("*"))
	{
		t = MakePointerType(t);
	}
	if (tk_Token != TK_IDENTIFIER)
	{
		ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
	}

	numlocaldefs = 1;
	localsofs = 0;
	maxlocalsofs = 0;
	ThisType = NULL;

	TType *ctype = CheckForType();
	if (ctype)
	{
		TK_Expect("::", ERR_NONE);
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError("Method name expected");
		}
		s_name = FindString(va("%s::%s", ctype->name, tk_String));
		method = CheckForField(ctype, false);
		if (!method || method->type->type != ev_method)
		{
			ParseError("No such method");
		}
		if (!method)
		{
			TK_NextToken();
		}
		ThisType = MakePointerType(ctype);
		localsofs = 1;
	}
	else
	{
		s_name = FindString(tk_String);
		TK_NextToken();
	}

	if (ThisType)
	{
		TK_Expect("(", ERR_NONE);
	}
	else if (!TK_Check("("))
	{
		if (builtin)
		{
			ERR_Exit(ERR_MISSING_LPAREN, true, NULL);
		}
		do
		{
			if (!s_name)
			{
				t = type;
				if (TK_Check("*"))
				{
					t = MakePointerType(t);
				}
				if (tk_Token != TK_IDENTIFIER)
				{
					ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
				}
				s_name = FindString(tk_String);
			}
		   	if (t == &type_void)
			{
				ERR_Exit(ERR_VOID_VAR, true, NULL);
			}
			if (t->type == ev_class)
			{
				ParseWarning("Class variable");
			}
			if (CheckForGlobalVar(strings + s_name) ||
				CheckForFunction(strings + s_name) ||
				CheckForConstant(strings + s_name) != -1)
			{
				ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Symbol: %s",
					strings + s_name);
			}
			t = ParseArrayDimensions(t);
			// inicializÆcija
			if (TK_Check ("="))
			{
				t = ParseGlobalData(t, globals + numglobals);
			}
			if (!t->size)
			{
				ERR_Exit(ERR_NONE, true, "Size of type = 0.");
			}
			globaldefs[numglobaldefs].s_name = s_name;
			globaldefs[numglobaldefs].type = t;
			globaldefs[numglobaldefs].ofs = numglobals;
			numglobals += TypeSize(t) / 4;
			numglobaldefs++;
			s_name = 0;
		} while (TK_Check (","));
		TK_Expect(";", ERR_MISSING_SEMICOLON);
		return;
	}

	BreakLevel = 0;
	ContinueLevel = 0;
	FuncRetType = t;
	if (t != &type_void)
	{
		//	Funkcijas atgri÷amajam tipam jÆbÝt void vai arØ ar izmñru 4
		TypeCheckPassable(t);
	}

	TType functype;
	memset(&functype, 0, sizeof(TType));
	functype.type = ev_function;
	functype.size = 4;
	functype.aux_type = t;

	if (CheckForGlobalVar(strings + s_name))
	{
		ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Symbol: %s", strings + s_name);
	}
	if (CheckForConstant(strings + s_name) != -1)
	{
		ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Symbol: %s", strings + s_name);
	}

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
			if (numlocaldefs == 1)
			{
				break;
			}
			ERR_Exit(ERR_BAD_VAR_TYPE, true, NULL);
		}
		while (TK_Check("*"))
		{
		   	type = MakePointerType(type);
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
			if (CheckForLocalVar(tk_String))
			{
				ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Identifier: %s", tk_String);
			}
			strcpy(localdefs[numlocaldefs].name, tk_String);
			localdefs[numlocaldefs].type = type;
			localdefs[numlocaldefs].ofs = localsofs;
			numlocaldefs++;
			TK_NextToken();
		}
		else
		{
			localdefs[numlocaldefs].name[0] = 0;
		}
		functype.param_types[functype.num_params] = type;
		functype.num_params++;
		localsofs += TypeSize(type) / 4;
	} while (TK_Check(","));
	TK_Expect(")", ERR_MISSING_RPAREN);
	functype.params_size = localsofs;
	maxlocalsofs = localsofs;

	num = CheckForFunction(strings + s_name);
	if (num)
	{
		if (builtin && functions[num].first_statement >= 0)
		{
	   		ERR_Exit(ERR_FUNCTION_REDECLARED, true,
	   				 "Declared function defined as builtin.");
		}
		if (functions[num].type != FindType(&functype))
		{
	   		ERR_Exit(ERR_TYPE_MISTMATCH, true, NULL);
		}
	}
	else
	{
		num = numfunctions;
		functions[num].s_name = s_name;
		if (builtin)
		{
			functions[num].first_statement = -numbuiltins;
			numbuiltins++;
		}
		else
		{
			functions[num].first_statement = 0;
		}
		functions[num].type = FindType(&functype);
		numfunctions++;
	}
	if (method)
	{
		method->func_num = num;
	}

	if (TK_Check("{"))
	{
		if (functions[num].first_statement)
		{
			ERR_Exit(ERR_FUNCTION_REDECLARED, true,
					 "Function: %s", strings + s_name);
		}
		functions[num].first_statement = CodeBufferSize;

	   	ParseCompoundStatement();

		if (FuncRetType == &type_void)
		{
			AddStatement(OPC_RETURN);
		}
		functions[num].num_locals = maxlocalsofs;
	}
	else
	{
		//  Funkcijas prototips
		TK_Expect(";", ERR_MISSING_SEMICOLON);
		return;
	}
}

//==========================================================================
//
//	TK_AddConstant
//
//==========================================================================

void TK_AddConstant(char* name, int value)
{
	if (CheckForGlobalVar(name) ||
		CheckForFunction(name) ||
		CheckForConstant(name) != -1)
	{
		ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Symbol: %s", name);
	}
	if (numconstants == MAX_CONSTANTS)
	{
		ERR_Exit(ERR_TOO_MENY_CONSTANTS, true, NULL);
	}
	strcpy(Constants[numconstants].name, name);
	Constants[numconstants].value = value;
	Constants[numconstants].next = ConstLookup[(byte)name[0]];
	ConstLookup[(byte)name[0]] = numconstants;
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
	int			i;
	TType		*type;

	dprintf("Compiling\n");
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
				else if (TK_Check("extern"))
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
				else if (TK_Check("enum"))
				{
					int		val;
					char	name[MAX_IDENTIFIER_LENGTH];

					val = 0;
					TK_Expect("{", ERR_MISSING_LBRACE);
					do
					{
						if (tk_Token != TK_IDENTIFIER)
						{
							ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
						}
						strcpy(name, tk_String);
						TK_NextToken();
						if (TK_Check("="))
						{
							val = EvalConstExpression(ev_int);
						}
						TK_AddConstant(name, val);
						val++;
					} while (TK_Check(","));
					TK_Expect("}", ERR_MISSING_RBRACE);
					TK_Expect(";", ERR_MISSING_SEMICOLON);
				}
				else if (TK_Check("struct"))
				{
					ParseStruct();
				}
				else if (TK_Check("class"))
				{
					ParseClass();
				}
				else if (TK_Check("addfields"))
				{
					AddFields();
				}
				else if (TK_Check("vector"))
				{
					ParseVector();
				}
				else if (TK_Check("typedef"))
				{
					ParseTypeDef();
				}
				else if (TK_Check("__states__"))
				{
				   	ParseStates();
				}
				else if (TK_Check("__mobjinfo__"))
				{
				   	ParseMobjInfo();
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

	AddInfoTables();
	AddVirtualTables();
	// check to make sure all functions prototyped have code
	done = true;
	for (i = 1 ; i < numfunctions ; i++)
	{
		if (!functions[i].first_statement)
		{
			fprintf(stderr, "Function %s was not defined\n",
					strings + functions[i].s_name);
			done = false;
		}
	}
	if (!done)
	{
		ERR_Exit(ERR_UNDEFINED_FUNCTIONS, false, NULL);
	}
	if (NumErrors)
	{
		ERR_Exit(ERR_NONE, false, NULL);
	}
}

//**************************************************************************
//
//	$Log$
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
