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

TType					*ThisType;
TType					*SelfType;

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
//	CheckForLocalVar
//
//==========================================================================

int CheckForLocalVar(FName Name)
{
	if (Name == NAME_None)
	{
		return 0;
	}
	for (int i = 1; i < numlocaldefs; i++)
	{
		if (localdefs[i].Name == Name)
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
	for (int i = 0; i < numconstants; i++)
	{
		if (Constants[i].Name == Name)
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
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
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
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
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
			if (TK_Check(KW_IF))
			{
				int*		jumpAddrPtr1;
				int*		jumpAddrPtr2;

				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				TypeCheck1(ParseExpression());
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				jumpAddrPtr1 = AddStatement(OPC_IFNOTGOTO, 0);
				ParseStatement();
				if (TK_Check(KW_ELSE))
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
			else if (TK_Check(KW_WHILE))
			{
				int			topAddr;
				int*		outAddrPtr;

				BreakLevel++;
				ContinueLevel++;
				topAddr = CodeBufferSize;
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				TypeCheck1(ParseExpression());
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				outAddrPtr = AddStatement(OPC_IFNOTGOTO, 0);
				ParseStatement();
				AddStatement(OPC_GOTO, topAddr);

				*outAddrPtr = CodeBufferSize;
				WriteContinues(topAddr);
				WriteBreaks();
			}
			else if (TK_Check(KW_DO))
			{
				int			topAddr;
				int			exprAddr;

				BreakLevel++;
				ContinueLevel++;
				topAddr = CodeBufferSize;
				ParseStatement();
				TK_Expect(KW_WHILE, ERR_BAD_DO_STATEMENT);
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				exprAddr = CodeBufferSize;
				TypeCheck1(ParseExpression());
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				AddStatement(OPC_IFGOTO, topAddr);
				WriteContinues(exprAddr);
				WriteBreaks();
			}
			else if (TK_Check(KW_FOR))
			{
				int*		jumpAddrPtr1;
				int*		jumpAddrPtr2;
				int			topAddr;
				int			contAddr;

				BreakLevel++;
				ContinueLevel++;
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				do
				{
					t = ParseExpression();
				   	AddDrop(t);
				} while (TK_Check(PU_COMMA));
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
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
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				contAddr = CodeBufferSize;
				do
				{
					t = ParseExpression();
				   	AddDrop(t);
				} while (TK_Check(PU_COMMA));
				AddStatement(OPC_GOTO, topAddr);
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				*jumpAddrPtr1 = CodeBufferSize;
				ParseStatement();
				AddStatement(OPC_GOTO, contAddr);
				*jumpAddrPtr2 = CodeBufferSize;
				WriteContinues(contAddr);
				WriteBreaks();
			}
			else if (TK_Check(KW_BREAK))
			{
				AddBreak();
			}
			else if (TK_Check(KW_CONTINUE))
			{
				AddContinue();
			}
			else if (TK_Check(KW_RETURN))
			{
				if (TK_Check(PU_SEMICOLON))
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
					TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
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
			else if (TK_Check(KW_SWITCH))
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

				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				TType *etype = ParseExpression();
				TypeCheck1(etype);
//				if (etype->type != ev_int && etype->type != ev_uint)
//				{
//					ParseWarning("Int expression expected");
//				}
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);

				switcherAddrPtr = AddStatement(OPC_GOTO, 0);
				defaultAddress = 0;
				numcases = 0;
				BreakLevel++;

				TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
				do
				{
					if (TK_Check(KW_CASE))
					{
						if (numcases == MAX_CASE)
						{
							ERR_Exit(ERR_CASE_OVERFLOW, true, NULL);
						}
						CaseInfo[numcases].value = EvalConstExpression(etype->type);
						CaseInfo[numcases].address = CodeBufferSize;
						numcases++;
						TK_Expect(PU_COLON, ERR_MISSING_COLON);
						continue;
					}
					if (TK_Check(KW_DEFAULT))
					{
						if (defaultAddress)
						{
							ERR_Exit(ERR_MULTIPLE_DEFAULT, true, NULL);
						}
						defaultAddress = CodeBufferSize;
						TK_Expect(PU_COLON, ERR_MISSING_COLON);
						continue;
					}
					ParseStatement();
				} while (!TK_Check(PU_RBRACE));

				outAddrPtr = AddStatement(OPC_GOTO, 0);

				*switcherAddrPtr = CodeBufferSize;
				for (i = 0; i < numcases; i++)
				{
					if (etype->type == ev_classid)
						AddStatement(OPC_CASE_GOTO_CLASSID,
							CaseInfo[i].value, CaseInfo[i].address);
					else if (etype->type == ev_name)
						AddStatement(OPC_CASE_GOTO_NAME,
							CaseInfo[i].value, CaseInfo[i].address);
					else
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
			else
			{
				t = ParseExpression(true);
				if (t)
				{
					AddDrop(t);
				}
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
			t = ParseExpression(true);
			if (t)
			{
				AddDrop(t);
			}
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			break;
	}
}

//==========================================================================
//
//	ParseLocalVar
//
//==========================================================================

void ParseLocalVar(TType *type)
{
	TType	*t;
	int		size;

	if (!type)
	{
		ERR_Exit(ERR_INVALID_IDENTIFIER, true, "Bad type");
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
			ParseError(ERR_BAD_VAR_TYPE);
		}
		if (t->type == ev_class)
		{
			t = MakeReferenceType(t);
		}
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError(ERR_INVALID_IDENTIFIER, "variable name expected");
			continue;
		}
		if (numlocaldefs == MAX_LOCAL_DEFS)
		{
			ParseError(ERR_LOCALS_OVERFLOW);
			continue;
		}
		localdefs[numlocaldefs].Name = tk_Name;

		if (CheckForLocalVar(tk_Name))
		{
			ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Identifier: %s", *tk_Name);
		}
		TK_NextToken();

		size = 1;
		if (TK_Check(PU_LINDEX))
		{
			size = EvalConstExpression(ev_int);
			t = MakeArrayType(t, size);
			TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
		}
		//  inicializÆcija
		else if (TK_Check(PU_ASSIGN))
		{
			AddStatement(OPC_LOCALADDRESS, localsofs);
			TType *t1 = ParseExpression();
			TypeCheck3(t, t1);
			if (t1->type == ev_vector)
				AddStatement(OPC_VASSIGN);
			else
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
	} while (TK_Check(PU_COMMA));
//	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
}

//==========================================================================
//
//	ParseCompoundStatement
//
//==========================================================================

static void ParseCompoundStatement(void)
{
	int		num_local_defs_on_start;
	int		num_locals_ofs_on_start;

	num_local_defs_on_start = numlocaldefs;
	num_locals_ofs_on_start = localsofs;
	/*do
	{
		TType *type = CheckForType();
		if (type)
		{
			ParseLocalVar(type);
		}
	} while (type);*/

	while (!TK_Check(PU_RBRACE))
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
	localsofs = 0;
	maxlocalsofs = 0;
	ThisType = NULL;
	SelfType = NULL;

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
		ThisType = MakePointerType(ctype);
		SelfType = MakeReferenceType(ctype);
		localsofs = 1;
	}
	else
	{
		Name = tk_Name;
		TK_NextToken();
	}

	if (ThisType)
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
			if (CheckForLocalVar(tk_Name))
			{
				ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Identifier: %s", *tk_Name);
			}
			localdefs[numlocaldefs].Name = tk_Name;
			localdefs[numlocaldefs].type = type;
			localdefs[numlocaldefs].ofs = localsofs;
			numlocaldefs++;
			TK_NextToken();
		}
		functype.param_types[functype.num_params] = type;
		functype.num_params++;
		localsofs += TypeSize(type) / 4;
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	functype.params_size = localsofs;
	maxlocalsofs = localsofs;

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
		if (functions[num].first_statement)
		{
			ERR_Exit(ERR_FUNCTION_REDECLARED, true, "Function: %s", *Name);
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
	TType *class_type)
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
	localsofs = 1;

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
			if (CheckForLocalVar(tk_Name))
			{
				ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Identifier: %s", *tk_Name);
			}
			localdefs[numlocaldefs].Name = tk_Name;
			localdefs[numlocaldefs].type = type;
			localdefs[numlocaldefs].ofs = localsofs;
			numlocaldefs++;
			TK_NextToken();
		}
		functype.param_types[functype.num_params] = type;
		functype.num_params++;
		localsofs += TypeSize(type) / 4;
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	functype.params_size = localsofs;
	maxlocalsofs = localsofs;

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

	if (TK_Check(PU_LBRACE))
	{
		ThisType = MakePointerType(class_type);
		SelfType = MakeReferenceType(class_type);
		BreakLevel = 0;
		ContinueLevel = 0;
		FuncRetType = t;

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
	localsofs = 1;

	TType functype;
	memset(&functype, 0, sizeof(TType));
	functype.type = ev_function;
	functype.size = 4;
	functype.aux_type = &type_void;
	functype.params_size = 1;
	maxlocalsofs = 1;

	int num = numfunctions;
	numfunctions++;
	functions[num].Name = NAME_None;
	functions[num].OuterClass = class_type;
	functions[num].type = FindType(&functype);
	functions[num].first_statement = CodeBufferSize;

	ThisType = MakePointerType(class_type);
	SelfType = MakeReferenceType(class_type);
	BreakLevel = 0;
	ContinueLevel = 0;
	FuncRetType = &type_void;

	if (TK_Check(PU_LBRACE))
	{
	   	ParseCompoundStatement();
		AddStatement(OPC_RETURN);
	}
	else
	{
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	}
	functions[num].num_locals = maxlocalsofs;
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
	localsofs = 1;

	TType functype;
	memset(&functype, 0, sizeof(TType));
	functype.type = ev_function;
	functype.size = 4;
	functype.aux_type = &type_void;
	functype.params_size = 1;
	maxlocalsofs = 1;

	TType methodtype;
	memcpy(&methodtype, &functype, sizeof(TType));
	methodtype.type = ev_method;
	method->type = FindType(&methodtype);
	method->ofs = 0;
	method->Name = class_type->Name;
	class_type->numfields++;

	if (CheckForFunction(class_type, class_type->Name))
	{
		ERR_Exit(ERR_FUNCTION_REDECLARED, true,
			 "Function: %s", *class_type->Name, *class_type->Name);
	}

	int num = numfunctions;
	numfunctions++;
	method->func_num = num;
	functions[num].Name = class_type->Name;
	functions[num].OuterClass = class_type;
	functions[num].type = FindType(&functype);
	functions[num].first_statement = 0;

	if (TK_Check(PU_LBRACE))
	{
		ThisType = MakePointerType(class_type);
		SelfType = MakeReferenceType(class_type);
		BreakLevel = 0;
		ContinueLevel = 0;
		FuncRetType = &type_void;

		functions[num].first_statement = CodeBufferSize;

		//  Call parent constructor
		field_t *pcon = FindConstructor(class_type->aux_type);
		if (pcon)
		{
			AddStatement(OPC_LOCALADDRESS, 0);
			AddStatement(OPC_PUSHPOINTED);
			AddStatement(OPC_CALL, pcon->func_num);
		}

	   	ParseCompoundStatement();
		AddStatement(OPC_RETURN);
		functions[num].num_locals = maxlocalsofs;
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
	functions[numfunctions].first_statement = CodeBufferSize;
	functions[numfunctions].num_locals = 1;
	numfunctions++;

	AddStatement(OPC_RETURN);

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
				else if (TK_Check(KW_MOBJINFO))
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
			fprintf(stderr, "Function %s::%s was not defined\n",
				functions[i].OuterClass ? *functions[i].OuterClass->Name :
				"none", *functions[i].Name);
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
//	Revision 1.19  2002/01/12 18:06:34  dj_jl
//	New style of state functions, some other changes
//
//	Revision 1.18  2002/01/11 08:17:31  dj_jl
//	Added name subsystem, removed support for unsigned ints
//	
//	Revision 1.17  2002/01/07 12:31:36  dj_jl
//	Changed copyright year
//	
//	Revision 1.16  2001/12/27 17:44:00  dj_jl
//	Removed support for C++ style constructors and destructors, some fixes
//	
//	Revision 1.15  2001/12/18 19:09:41  dj_jl
//	Some extra info in progs and other small changes
//	
//	Revision 1.14  2001/12/12 19:22:22  dj_jl
//	Support for method usage as state functions, dynamic cast
//	Added dynamic arrays
//	
//	Revision 1.13  2001/12/04 18:19:55  dj_jl
//	Fixed vector assignement in declaration
//	
//	Revision 1.12  2001/12/03 19:25:44  dj_jl
//	Fixed calling of parent function
//	Added defaultproperties
//	Fixed vectors as arguments to methods
//	
//	Revision 1.11  2001/12/01 18:17:09  dj_jl
//	Fixed calling of parent method, speedup
//	
//	Revision 1.10  2001/11/09 14:42:28  dj_jl
//	References, beautification
//	
//	Revision 1.9  2001/10/27 07:54:59  dj_jl
//	Added support for constructors and destructors
//	
//	Revision 1.8  2001/10/22 17:31:34  dj_jl
//	Posibility to use classid constants in switch statement
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
