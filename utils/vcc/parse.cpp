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

#define MAX_BREAK		256
#define MAX_CONTINUE	256
#define MAX_CASE		256

// TYPES -------------------------------------------------------------------

struct breakInfo_t
{
	int		level;
	int		addressPtr;
};

struct continueInfo_t
{
	int		level;
	int		addressPtr;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void 	ParseCompoundStatement();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VLocalVarDef			localdefs[MAX_LOCAL_DEFS];

TType					SelfType;
VClass*					SelfClass;

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
static TType			FuncRetType;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	CheckForLocalVar
//
//==========================================================================

int CheckForLocalVar(VName Name)
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
//	AddDrop
//
//==========================================================================

static void AddDrop(const TType& type)
{
	if (type.type == ev_string)
	{
		AddStatement(OPC_DropStr);
	}
	else if (type.GetSize() == 4)
	{
		AddStatement(OPC_Drop);
	}
	else if (type.type == ev_vector)
	{
		AddStatement(OPC_VDrop);
	}
	else if (type.type != ev_void)
	{
		ParseError("Expression's result type cannot be dropped");
	}
}

//==========================================================================
//
//	AddBreak
//
//==========================================================================

static void AddBreak()
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
	BreakInfo[BreakIndex].addressPtr = AddStatement(OPC_Goto, 0);
	BreakIndex++;
}

//==========================================================================
//
// WriteBreaks
//
//==========================================================================

static void WriteBreaks()
{
	BreakLevel--;
	while (BreakIndex && BreakInfo[BreakIndex-1].level > BreakLevel)
	{
		FixupJump(BreakInfo[--BreakIndex].addressPtr);
	}
}

//==========================================================================
//
//	AddContinue
//
//==========================================================================

static void AddContinue()
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
	ContinueInfo[ContinueIndex].addressPtr = AddStatement(OPC_Goto, 0);
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
		FixupJump(ContinueInfo[--ContinueIndex].addressPtr, address);
	}
}

//==========================================================================
//
//	EmitClearStrings
//
//==========================================================================

static void EmitClearStrings(int Start = 0)
{
	for (int i = Start; i < numlocaldefs; i++)
	{
		if (localdefs[i].type.type == ev_string)
		{
			EmitLocalAddress(localdefs[i].ofs);
			AddStatement(OPC_ClearPointedStr);
		}
		if (localdefs[i].type.type == ev_struct &&
			localdefs[i].type.Struct->NeedsDestructor())
		{
			EmitLocalAddress(localdefs[i].ofs);
			AddStatement(OPC_ClearPointedStruct, localdefs[i].type.Struct);
		}
		if (localdefs[i].type.type == ev_array)
		{
			if (localdefs[i].type.ArrayInnerType == ev_string)
			{
				for (int j = 0; j < localdefs[i].type.array_dim; j++)
				{
					EmitLocalAddress(localdefs[i].ofs);
					EmitPushNumber(j);
					AddStatement(OPC_ArrayElement, localdefs[i].type.GetArrayInnerType());
					AddStatement(OPC_ClearPointedStr);
				}
			}
			else if (localdefs[i].type.ArrayInnerType == ev_struct &&
				localdefs[i].type.Struct->NeedsDestructor())
			{
				for (int j = 0; j < localdefs[i].type.array_dim; j++)
				{
					EmitLocalAddress(localdefs[i].ofs);
					EmitPushNumber(j);
					AddStatement(OPC_ArrayElement, localdefs[i].type.GetArrayInnerType());
					AddStatement(OPC_ClearPointedStruct, localdefs[i].type.Struct);
				}
			}
		}
	}
}

//==========================================================================
//
//	ParseStatement
//
//==========================================================================

static void ParseStatement()
{
	TType		t;

	switch(tk_Token)
	{
		case TK_EOF:
			ERR_Exit(ERR_UNEXPECTED_EOF, true, NULL);
			break;
		case TK_KEYWORD:
			if (TK_Check(KW_IF))
			{
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				ParseExpression().CheckSizeIs4();
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				int jumpAddrPtr1 = AddStatement(OPC_IfNotGoto, 0);
				ParseStatement();
				if (TK_Check(KW_ELSE))
				{
					int jumpAddrPtr2 = AddStatement(OPC_Goto, 0);
					FixupJump(jumpAddrPtr1);
					ParseStatement();
					FixupJump(jumpAddrPtr2);
				}
				else
				{
					FixupJump(jumpAddrPtr1);
				}
			}
			else if (TK_Check(KW_WHILE))
			{
				int			topAddr;

				BreakLevel++;
				ContinueLevel++;
				topAddr = GetNumInstructions();
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				ParseExpression().CheckSizeIs4();
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				int outAddrPtr = AddStatement(OPC_IfNotGoto, 0);
				ParseStatement();
				AddStatement(OPC_Goto, topAddr);

				FixupJump(outAddrPtr);
				WriteContinues(topAddr);
				WriteBreaks();
			}
			else if (TK_Check(KW_DO))
			{
				BreakLevel++;
				ContinueLevel++;
				int topAddr = GetNumInstructions();
				ParseStatement();
				TK_Expect(KW_WHILE, ERR_BAD_DO_STATEMENT);
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				int exprAddr = GetNumInstructions();
				ParseExpression().CheckSizeIs4();
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				AddStatement(OPC_IfGoto, topAddr);
				WriteContinues(exprAddr);
				WriteBreaks();
			}
			else if (TK_Check(KW_FOR))
			{
				BreakLevel++;
				ContinueLevel++;
				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				do
				{
					t = ParseExpression();
					AddDrop(t);
				} while (TK_Check(PU_COMMA));
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				int topAddr = GetNumInstructions();
				t = ParseExpression();
				if (t.type == ev_void)
				{
					AddStatement(OPC_PushNumber, 1);
				}
				else
				{
					t.CheckSizeIs4();
				}
				int jumpAddrPtr1 = AddStatement(OPC_IfGoto, 0);
				int jumpAddrPtr2 = AddStatement(OPC_Goto, 0);
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				int contAddr = GetNumInstructions();
				do
				{
					t = ParseExpression();
					AddDrop(t);
				} while (TK_Check(PU_COMMA));
				AddStatement(OPC_Goto, topAddr);
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
				FixupJump(jumpAddrPtr1);
				ParseStatement();
				AddStatement(OPC_Goto, contAddr);
				FixupJump(jumpAddrPtr2);
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
					if (FuncRetType.type != ev_void)
					{
						ERR_Exit(ERR_NO_RET_VALUE, true, NULL);
					}
					EmitClearStrings();
					AddStatement(OPC_Return);
				}
				else
				{
					if (FuncRetType.type == ev_void)
					{
						ERR_Exit(ERR_VOID_RET, true, NULL);
					}
					t = ParseExpression();
					TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
					t.CheckMatch(FuncRetType);
					EmitClearStrings();
					if (t.GetSize() == 4)
					{
						AddStatement(OPC_ReturnL);
					}
					else if (t.type == ev_vector)
					{
						AddStatement(OPC_ReturnV);
					}
					else
					{
						ERR_Exit(ERR_NONE, true, "Bad return type");
					}
				}
			}
			else if (TK_Check(KW_SWITCH))
			{
				int			switcherAddrPtr;
				int			outAddrPtr;
				int			numcases;
				int			defaultAddress;
				int			i;
				struct
				{
					int value;
					int address;
				} CaseInfo[MAX_CASE];

				TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
				TType etype = ParseExpression();
				if (etype.type != ev_int)
				{
					ParseError("Int expression expected");
				}
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);

				switcherAddrPtr = AddStatement(OPC_Goto, 0);
				defaultAddress = -1;
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
						CaseInfo[numcases].value = EvalConstExpression(SelfClass, etype.type);
						CaseInfo[numcases].address = GetNumInstructions();
						numcases++;
						TK_Expect(PU_COLON, ERR_MISSING_COLON);
						continue;
					}
					if (TK_Check(KW_DEFAULT))
					{
						if (defaultAddress != -1)
						{
							ERR_Exit(ERR_MULTIPLE_DEFAULT, true, NULL);
						}
						defaultAddress = GetNumInstructions();
						TK_Expect(PU_COLON, ERR_MISSING_COLON);
						continue;
					}
					ParseStatement();
				} while (!TK_Check(PU_RBRACE));

				outAddrPtr = AddStatement(OPC_Goto, 0);

				FixupJump(switcherAddrPtr);
				for (i = 0; i < numcases; i++)
				{
					if (CaseInfo[i].value >= 0 && CaseInfo[i].value < 256)
					{
						AddStatement(OPC_CaseGotoB, CaseInfo[i].value,
							CaseInfo[i].address);
					}
					else if (CaseInfo[i].value >= MIN_VINT16 &&
						CaseInfo[i].value < MAX_VINT16)
					{
						AddStatement(OPC_CaseGotoS, CaseInfo[i].value,
							CaseInfo[i].address);
					}
					else
					{
						AddStatement(OPC_CaseGoto, CaseInfo[i].value,
							CaseInfo[i].address);
					}
				}
				AddStatement(OPC_Drop);

				if (defaultAddress != -1)
				{
					AddStatement(OPC_Goto, defaultAddress);
				}

				FixupJump(outAddrPtr);

				WriteBreaks();
			}
			else
			{
				t = ParseExpression(true);
				AddDrop(t);
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
			AddDrop(t);
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			break;
	}
}

//==========================================================================
//
//	ParseLocalVar
//
//==========================================================================

void ParseLocalVar(const TType& type)
{
	TType	t;
	int		size;

	if (type.type == ev_unknown)
	{
		ParseError(ERR_INVALID_IDENTIFIER, "Bad type");
		return;
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
			ParseError(ERR_BAD_VAR_TYPE);
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
			size = EvalConstExpression(SelfClass, ev_int);
			t = MakeArrayType(t, size);
			TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
		}
		//  Initialisation
		else if (TK_Check(PU_ASSIGN))
		{
			EmitLocalAddress(localsofs);
			TType t1 = ParseExpression();
			t1.CheckMatch(t);
			if (t1.type == ev_vector)
				AddStatement(OPC_VAssignDrop);
			else if (t1.type == ev_pointer || t1.type == ev_reference ||
				t1.type == ev_classid || t1.type == ev_state)
				AddStatement(OPC_AssignPtrDrop);
			else if (t1.type == ev_string)
				AddStatement(OPC_AssignStrDrop);
			else
				AddStatement(OPC_AssignDrop);
		}
		localdefs[numlocaldefs].type = t;
		localdefs[numlocaldefs].ofs = localsofs;
		//  Increase variable count after expression so you can't use
		// the variable in expression.
		numlocaldefs++;
		localsofs += t.GetSize() / 4;
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

static void ParseCompoundStatement()
{
	int		num_local_defs_on_start;

	num_local_defs_on_start = numlocaldefs;
	/*do
	{
		TType *type = CheckForType_();
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
	EmitClearStrings(num_local_defs_on_start);
	numlocaldefs = num_local_defs_on_start;
}

//==========================================================================
//
//	CompileMethodDef
//
//==========================================================================

void CompileMethodDef(const TType& t, VMethod* Method, VClass* InClass)
{
	numlocaldefs = 1;
	localsofs = 1;

	do
	{
		if (TK_Check(PU_VARARGS))
		{
			break;
		}

		TType type = CheckForType(InClass);

		if (type.type == ev_unknown)
		{
			break;
		}
		while (TK_Check(PU_ASTERISK))
		{
			type = MakePointerType(type);
		}
		if (type.type == ev_void)
		{
static int un = 0;
un++;
//ParseWarning("This is ugly %d", un);
			break;
		}
		type.CheckPassable();

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
		localsofs += type.GetSize() / 4;
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	maxlocalsofs = localsofs;

	if (Method->Flags & FUNC_Native)
	{
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		return;
	}

	SelfType = TType(InClass);
	SelfClass = InClass;
	BreakLevel = 0;
	ContinueLevel = 0;
	FuncRetType = t;

	BeginCode(Method);
	for (int i = 0; i < numlocaldefs; i++)
	{
		if (localdefs[i].type.type == ev_vector)
		{
			AddStatement(OPC_VFixParam, i);
		}
	}
	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	ParseCompoundStatement();

	if (FuncRetType.type == ev_void)
	{
		EmitClearStrings();
		AddStatement(OPC_Return);
	}
	Method->NumLocals = maxlocalsofs;
	EndCode(Method);
}

//==========================================================================
//
//	SkipDelegate
//
//==========================================================================

void SkipDelegate(VClass* InClass)
{
	TK_NextToken();
	while (TK_Check(PU_ASTERISK));
	TK_NextToken();
	TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
	do
	{
		if (TK_Check(PU_VARARGS))
		{
			break;
		}
		TType type = CheckForType(InClass);
		if (type.type == ev_unknown)
		{
			break;
		}
		while (TK_Check(PU_ASTERISK));
		if (tk_Token == TK_IDENTIFIER)
		{
			TK_NextToken();
		}
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
}

//==========================================================================
//
//	CompileStateCode
//
//==========================================================================

void CompileStateCode(VClass* InClass, VMethod* Func)
{
	numlocaldefs = 1;
	localsofs = 1;
	maxlocalsofs = 1;

	BeginCode(Func);

	SelfType = TType(InClass);
	SelfClass = InClass;
	BreakLevel = 0;
	ContinueLevel = 0;
	FuncRetType = TType(ev_void);

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	ParseCompoundStatement();
	EmitClearStrings();
	AddStatement(OPC_Return);
	Func->NumLocals = maxlocalsofs;
	EndCode(Func);
}

//==========================================================================
//
//	CompileDefaultProperties
//
//==========================================================================

void CompileDefaultProperties(VMethod* Method, VClass* InClass)
{
	numlocaldefs = 1;
	localsofs = 1;
	maxlocalsofs = 1;

	SelfType = TType(InClass);
	SelfClass = InClass;
	BreakLevel = 0;
	ContinueLevel = 0;
	FuncRetType = TType(ev_void);

	BeginCode(Method);

	//  Call parent constructor
	if (InClass->ParentClass)
	{
		AddStatement(OPC_LocalAddress0);
		AddStatement(OPC_PushPointedPtr);
		AddStatement(OPC_Call, InClass->ParentClass->DefaultProperties);
	}

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	ParseCompoundStatement();
	EmitClearStrings();
	AddStatement(OPC_Return);
	Method->NumLocals = maxlocalsofs;
	EndCode(Method);
}

//==========================================================================
//
//	PA_Compile
//
//==========================================================================

void PA_Compile()
{
	bool		done;

	dprintf("Compiling pass 2\n");

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
					val++;
				} while (TK_Check(PU_COMMA));
				TK_Expect(PU_RBRACE, ERR_MISSING_RBRACE);
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			}
			else if (TK_Check(KW_STRUCT))
			{
				SkipStruct(NULL);
			}
			else if (TK_Check(KW_CLASS))
			{
				CompileClass();
			}
			else if (TK_Check(KW_VECTOR))
			{
				SkipStruct(NULL);
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
