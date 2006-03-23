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
	if (type.GetSize() == 4)
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
				etype.CheckSizeIs4();
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
					AddStatement(OPC_CaseGoto, CaseInfo[i].value,
						CaseInfo[i].address);
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
			AddStatement(OPC_LocalAddress, localsofs);
			TType t1 = ParseExpression();
			t1.CheckMatch(t);
			if (t1.type == ev_vector)
				AddStatement(OPC_VAssignDrop);
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
	numlocaldefs = num_local_defs_on_start;
}

//==========================================================================
//
//	CompileMethodDef
//
//==========================================================================

void CompileMethodDef(const TType& t, VField* method, VField* otherfield,
	VClass* InClass)
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

	VMethod* Func = CheckForFunction(InClass, method->Name);
	if (method->func != Func)
		ERR_Exit(ERR_NONE, true, "Found wrong function");

	if (Func->Flags & FUNC_Native)
	{
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		return;
	}

	SelfType = TType(InClass);
	SelfClass = InClass;
	BreakLevel = 0;
	ContinueLevel = 0;
	FuncRetType = t;

	BeginCode(Func);
	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	ParseCompoundStatement();

	if (FuncRetType.type == ev_void)
	{
		AddStatement(OPC_Return);
	}
	Func->NumLocals = maxlocalsofs;
	EndCode(Func);
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
	AddStatement(OPC_Return);
	Func->NumLocals = maxlocalsofs;
	EndCode(Func);
}

//==========================================================================
//
//	CompileDefaultProperties
//
//==========================================================================

void CompileDefaultProperties(VField *method, VClass* InClass)
{
	numlocaldefs = 1;
	localsofs = 1;
	maxlocalsofs = 1;

	SelfType = TType(InClass);
	SelfClass = InClass;
	BreakLevel = 0;
	ContinueLevel = 0;
	FuncRetType = TType(ev_void);

	BeginCode(method->func);

	//  Call parent constructor
	VField *pcon = FindConstructor(InClass->ParentClass);
	if (pcon)
	{
		AddStatement(OPC_LocalAddress, 0);
		AddStatement(OPC_PushPointed);
		AddStatement(OPC_Call, pcon->func->MemberIndex);
	}

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	ParseCompoundStatement();
	AddStatement(OPC_Return);
	method->func->NumLocals = maxlocalsofs;
	EndCode(method->func);
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
			if (TK_Check(KW_ENUM))
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
			else if (TK_Check(KW_ADDFIELDS))
			{
				SkipAddFields(NULL);
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

	AddVirtualTables();
	if (NumErrors)
	{
		ERR_Exit(ERR_NONE, false, NULL);
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.43  2006/03/23 18:30:54  dj_jl
//	Use single list of all members, members tree.
//
//	Revision 1.42  2006/03/13 19:30:46  dj_jl
//	Clean function local variables.
//	
//	Revision 1.41  2006/03/12 13:03:22  dj_jl
//	Removed use of bitfields for portability reasons.
//	
//	Revision 1.40  2006/03/10 19:31:55  dj_jl
//	Use serialisation for progs files.
//	
//	Revision 1.39  2006/02/27 21:23:55  dj_jl
//	Rewrote names class.
//	
//	Revision 1.38  2006/02/20 19:34:32  dj_jl
//	Created modifiers class.
//	
//	Revision 1.37  2006/02/19 20:37:02  dj_jl
//	Implemented support for delegates.
//	
//	Revision 1.36  2006/02/19 14:37:36  dj_jl
//	Changed type handling.
//	
//	Revision 1.35  2006/02/17 19:25:00  dj_jl
//	Removed support for progs global variables and functions.
//	
//	Revision 1.34  2006/02/10 22:15:21  dj_jl
//	Temporary fix for big-endian systems.
//	
//	Revision 1.33  2005/12/14 20:53:23  dj_jl
//	State names belong to a class.
//	Structs and enums defined in a class.
//	
//	Revision 1.32  2005/12/12 20:58:47  dj_jl
//	Removed compiler limitations.
//	
//	Revision 1.31  2005/11/30 23:54:27  dj_jl
//	Switch statement requires integer.
//	
//	Revision 1.30  2005/11/30 13:14:53  dj_jl
//	Implemented instruction buffer.
//	
//	Revision 1.29  2005/11/29 19:31:43  dj_jl
//	Class and struct classes, removed namespaces, beautification.
//	
//	Revision 1.28  2005/11/24 20:42:05  dj_jl
//	Renamed opcodes, cleanup and improvements.
//	
//	Revision 1.27  2005/03/16 14:41:34  dj_jl
//	Increased limits.
//	
//	Revision 1.26  2003/03/08 12:47:52  dj_jl
//	Code cleanup.
//	
//	Revision 1.25  2002/11/02 17:11:13  dj_jl
//	New style classes.
//	
//	Revision 1.24  2002/09/07 16:36:38  dj_jl
//	Support bool in function args and return type.
//	Removed support for typedefs.
//	
//	Revision 1.23  2002/08/24 14:45:38  dj_jl
//	2 pass compiling.
//	
//	Revision 1.22  2002/02/02 19:23:02  dj_jl
//	Natives declared inside class declarations.
//	
//	Revision 1.21  2002/01/21 18:23:09  dj_jl
//	Constructors with no names
//	
//	Revision 1.20  2002/01/17 18:19:52  dj_jl
//	New style of adding to mobjinfo, some fixes
//	
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
