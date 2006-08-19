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

static VExpression* ParseExpressionPriority13();

static void 	ParseCompoundStatement();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VLocalVarDef			localdefs[MAX_LOCAL_DEFS];
int						numlocaldefs = 1;
int						localsofs = 0;

TType					SelfType;
VClass*					SelfClass;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static bool				CheckForLocal;

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
//	EmitPushNumber
//
//==========================================================================

void EmitPushNumber(int Val)
{
	if (Val == 0)
		AddStatement(OPC_PushNumber0);
	else if (Val == 1)
		AddStatement(OPC_PushNumber1);
	else if (Val >= 0 && Val < 256)
		AddStatement(OPC_PushNumberB, Val);
	else if (Val >= MIN_VINT16 && Val <= MAX_VINT16)
		AddStatement(OPC_PushNumberS, Val);
	else
		AddStatement(OPC_PushNumber, Val);
}

//==========================================================================
//
//	EmitLocalAddress
//
//==========================================================================

void EmitLocalAddress(int Ofs)
{
	if (Ofs == 0)
		AddStatement(OPC_LocalAddress0);
	else if (Ofs == 1)
		AddStatement(OPC_LocalAddress1);
	else if (Ofs == 2)
		AddStatement(OPC_LocalAddress2);
	else if (Ofs == 3)
		AddStatement(OPC_LocalAddress3);
	else if (Ofs == 4)
		AddStatement(OPC_LocalAddress4);
	else if (Ofs == 5)
		AddStatement(OPC_LocalAddress5);
	else if (Ofs == 6)
		AddStatement(OPC_LocalAddress6);
	else if (Ofs == 7)
		AddStatement(OPC_LocalAddress7);
	else if (Ofs < 256)
		AddStatement(OPC_LocalAddressB, Ofs);
	else if (Ofs < MAX_VINT16)
		AddStatement(OPC_LocalAddressS, Ofs);
	else
		AddStatement(OPC_LocalAddress, Ofs);
}

//==========================================================================
//
//	ParseDotMethodCall
//
//==========================================================================

static VExpression* ParseDotMethodCall(VExpression* SelfExpr,
	VName MethodName, TLocation Loc)
{
	VExpression* Args[MAX_ARG_COUNT + 1];
	int NumArgs = 0;
	if (!TK_Check(PU_RPAREN))
	{
		do
		{
			Args[NumArgs] = ParseExpressionPriority13();
			if (NumArgs == MAX_ARG_COUNT)
				ParseError(tk_Location, "Too many arguments");
			else
				NumArgs++;
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
	return new VDotInvocation(SelfExpr, MethodName, Loc, NumArgs, Args);
}

//==========================================================================
//
//	ParseBaseMethodCall
//
//==========================================================================

static VExpression* ParseBaseMethodCall(VName Name, TLocation Loc)
{
	VExpression* Args[MAX_ARG_COUNT + 1];
	int NumArgs = 0;
	if (!TK_Check(PU_RPAREN))
	{
		do
		{
			Args[NumArgs] = ParseExpressionPriority13();
			if (NumArgs == MAX_ARG_COUNT)
				ParseError(tk_Location, "Too many arguments");
			else
				NumArgs++;
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
	return new VBaseInvocation(Name, NumArgs, Args, Loc);
}

//==========================================================================
//
//	ParseMethodCallOrCast
//
//==========================================================================

static VExpression* ParseMethodCallOrCast(VName Name, TLocation Loc)
{
	VExpression* Args[MAX_ARG_COUNT + 1];
	int NumArgs = 0;
	if (!TK_Check(PU_RPAREN))
	{
		do
		{
			Args[NumArgs] = ParseExpressionPriority13();
			if (NumArgs == MAX_ARG_COUNT)
				ParseError(tk_Location, "Too many arguments");
			else
				NumArgs++;
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
	return new VCastOrInvocation(Name, Loc, NumArgs, Args);
}

//==========================================================================
//
//	ParseLocalVar
//
//==========================================================================

static VLocalDecl* ParseLocalVar(const TType& InType, VName TypeName)
{
	VLocalDecl* Decl = new VLocalDecl(tk_Location);
	do
	{
		VLocalEntry e;

		while (TK_Check(PU_ASTERISK))
		{
			e.PointerLevel++;
		}
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError(tk_Location, "Invalid identifier, variable name expected");
			continue;
		}
		e.Loc = tk_Location;
		e.Name = tk_Name;
		TK_NextToken();

		if (TK_Check(PU_LINDEX))
		{
			e.ArraySize = EvalConstExpression(SelfClass, ev_int);
			TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
		}
		//  Initialisation
		else if (TK_Check(PU_ASSIGN))
		{
			e.Value = ParseExpressionPriority13();
		}
		Decl->Vars.Append(e);
	} while (TK_Check(PU_COMMA));

	Decl->BaseType = InType;
	Decl->TypeName = TypeName;
	return Decl;
}

//==========================================================================
//
//	ParseExpressionPriority0
//
//==========================================================================

static VExpression* ParseExpressionPriority0()
{
	VName		Name;
	bool		bLocals;
	TLocation	Loc;

	bLocals = CheckForLocal;
	CheckForLocal = false;
	TLocation l = tk_Location;
	switch (tk_Token)
	{
	case TK_INTEGER:
		TK_NextToken();
		return new VIntLiteral(tk_Number, l);

	case TK_FLOAT:
		TK_NextToken();
		return new VFloatLiteral(tk_Float, l);

	case TK_NAME:
		TK_NextToken();
		return new VNameLiteral(tk_Name, l);

	case TK_STRING:
		TK_NextToken();
		return new VStringLiteral(tk_StringI, l);

	case TK_PUNCT:
		if (TK_Check(PU_LPAREN))
		{
			VExpression* op = ParseExpressionPriority13();
			if (!op)
			{
				ParseError(l, "Expression expected");
			}
			TK_Expect(PU_RPAREN, ERR_BAD_EXPR);
			return op;
		}

		if (TK_Check(PU_DCOLON))
		{
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError(l, "Method name expected.");
				break;
			}
			Loc = tk_Location;
			Name = tk_Name;
			TK_NextToken();
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			return ParseBaseMethodCall(Name, Loc);
		}
		break;

	case TK_KEYWORD:
		if (TK_Check(KW_VECTOR))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			VExpression* op1 = ParseExpressionPriority13();
			TK_Expect(PU_COMMA, ERR_BAD_EXPR);
			VExpression* op2 = ParseExpressionPriority13();
			TK_Expect(PU_COMMA, ERR_BAD_EXPR);
			VExpression* op3 = ParseExpressionPriority13();
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			return new VVector(op1, op2, op3, l);
		}
		if (TK_Check(KW_SELF))
		{
			return new VSelf(l);
		}
		if (TK_Check(KW_NONE))
		{
			return new VNoneLiteral(l);
		}
		if (TK_Check(KW_NULL))
		{
			return new VNullLiteral(l);
		}
		if (TK_Check(KW_TRUE))
		{
			return new VIntLiteral(1, l);
		}
		if (TK_Check(KW_FALSE))
		{
			return new VIntLiteral(0, l);
		}
		break;

	case TK_IDENTIFIER:
		Loc = tk_Location;
		Name = tk_Name;
		TK_NextToken();
		if (TK_Check(PU_LPAREN))
		{
			return ParseMethodCallOrCast(Name, Loc);
		}

		if (TK_Check(PU_DCOLON))
		{
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError(ERR_ILLEGAL_EXPR_IDENT, "Identifier: %s", tk_String);
				break;
			}
			VName Name2 = tk_Name;
			TK_NextToken();
			return new VDoubleName(Name, Name2, Loc);
		}

		if (bLocals && tk_Token == TK_PUNCT && tk_Punct == PU_ASTERISK)
		{
			return ParseLocalVar(ev_unknown, Name);
		}

		return new VSingleName(Name, Loc);

	default:
		break;
	}

	return NULL;
}

//==========================================================================
//
//	ParseExpressionPriority1
//
//==========================================================================

static VExpression* ParseExpressionPriority1()
{
	VExpression* op = ParseExpressionPriority0();
	if (!op)
		return NULL;
	bool done = false;
	do
	{
		TLocation l = tk_Location;

		if (TK_Check(PU_MINUS_GT))
		{
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError(tk_Location, "Invalid identifier, field name expacted");
			}
			else
			{
				op = new VPointerField(op, tk_Name, tk_Location);
				TK_NextToken();
			}
		}
		else if (TK_Check(PU_DOT))
		{
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError(tk_Location, "Invalid identifier, field name expacted");
			}
			else
			{
				VName FieldName = tk_Name;
				TLocation Loc = tk_Location;
				TK_NextToken();
				if (TK_Check(PU_LPAREN))
				{
					op = ParseDotMethodCall(op, FieldName, Loc);
				}
				else
				{
					op = new VDotField(op, FieldName, Loc);
				}
			}
		}
		else if (TK_Check(PU_LINDEX))
		{
			VExpression* ind = ParseExpressionPriority13();
			TK_Expect(PU_RINDEX, ERR_BAD_ARRAY);
			op = new VArrayElement(op, ind, l);
		}
		else
		{
			done = true;
		}
	} while (!done);

	return op;
}

//==========================================================================
//
//	ParseExpressionPriority2
//
//==========================================================================

static VExpression* ParseExpressionPriority2()
{
	VExpression*	op;

	if (tk_Token == TK_PUNCT)
	{
		TLocation l = tk_Location;

		if (TK_Check(PU_PLUS))
		{
			op = ParseExpressionPriority2();
			return new VUnary(PU_PLUS, op, l);
		}

		if (TK_Check(PU_MINUS))
		{
			op = ParseExpressionPriority2();
			return new VUnary(PU_MINUS, op, l);
		}

		if (TK_Check(PU_NOT))
		{
			op = ParseExpressionPriority2();
			return new VUnary(PU_NOT, op, l);
		}

		if (TK_Check(PU_TILDE))
		{
			op = ParseExpressionPriority2();
			return new VUnary(PU_TILDE, op, l);
		}

		if (TK_Check(PU_AND))
		{
			op = ParseExpressionPriority1();
			return new VUnary(PU_AND, op, l);
		}

		if (TK_Check(PU_ASTERISK))
		{
			op = ParseExpressionPriority2();
			return new VPushPointed(op);
		}

		if (TK_Check(PU_INC))
		{
			op = ParseExpressionPriority2();
			return new VUnaryMutator(INCDEC_PreInc, op, l);
		}

		if (TK_Check(PU_DEC))
		{
			op = ParseExpressionPriority2();
			return new VUnaryMutator(INCDEC_PreDec, op, l);
		}
	}

	op = ParseExpressionPriority1();
	if (!op)
		return NULL;
	TLocation l = tk_Location;

	if (TK_Check(PU_INC))
	{
		return new VUnaryMutator(INCDEC_PostInc, op, l);
	}

	if (TK_Check(PU_DEC))
	{
		return new VUnaryMutator(INCDEC_PostDec, op, l);
	}

	return op;
}

//==========================================================================
//
//	ParseExpressionPriority3
//
//==========================================================================

static VExpression* ParseExpressionPriority3()
{
	VExpression* op1 = ParseExpressionPriority2();
	if (!op1)
		return NULL;
	bool done = false;
	do
	{
		TLocation l = tk_Location;
		if (TK_Check(PU_ASTERISK))
		{
			VExpression* op2 = ParseExpressionPriority2();
			op1 = new VBinary(PU_ASTERISK, op1, op2, l);
		}
		else if (TK_Check(PU_SLASH))
		{
			VExpression* op2 = ParseExpressionPriority2();
			op1 = new VBinary(PU_SLASH, op1, op2, l);
		}
		else if (TK_Check(PU_PERCENT))
		{
			VExpression* op2 = ParseExpressionPriority2();
			op1 = new VBinary(PU_PERCENT, op1, op2, l);
		}
		else
		{
			done = true;
		}
	}
	while (!done);
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority4
//
//==========================================================================

static VExpression* ParseExpressionPriority4()
{
	VExpression* op1 = ParseExpressionPriority3();
	if (!op1)
		return NULL;
	bool done = false;
	do
	{
		TLocation l = tk_Location;
		if (TK_Check(PU_PLUS))
		{
			VExpression* op2 = ParseExpressionPriority3();
			op1 = new VBinary(PU_PLUS, op1, op2, l);
		}
		else if (TK_Check(PU_MINUS))
		{
			VExpression* op2 = ParseExpressionPriority3();
			op1 = new VBinary(PU_MINUS, op1, op2, l);
		}
		else
		{
			done = true;
		}
	}
	while (!done);
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority5
//
//==========================================================================

static VExpression* ParseExpressionPriority5()
{
	VExpression* op1 = ParseExpressionPriority4();
	if (!op1)
		return NULL;
	bool done = false;
	do
	{
		TLocation l = tk_Location;
		if (TK_Check(PU_LSHIFT))
		{
			VExpression* op2 = ParseExpressionPriority4();
			op1 = new VBinary(PU_LSHIFT, op1, op2, l);
		}
		else if (TK_Check(PU_RSHIFT))
		{
			VExpression* op2 = ParseExpressionPriority4();
			op1 = new VBinary(PU_RSHIFT, op1, op2, l);
		}
		else
		{
			done = true;
		}
	}
	while (!done);
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority6
//
//==========================================================================

static VExpression* ParseExpressionPriority6()
{
	VExpression* op1 = ParseExpressionPriority5();
	if (!op1)
		return NULL;
	bool done = false;
	do
	{
		TLocation l = tk_Location;
		if (TK_Check(PU_LT))
		{
			VExpression* op2 = ParseExpressionPriority5();
			op1 = new VBinary(PU_LT, op1, op2, l);
		}
		else if (TK_Check(PU_LE))
		{
			VExpression* op2 = ParseExpressionPriority5();
			op1 = new VBinary(PU_LE, op1, op2, l);
		}
		else if (TK_Check(PU_GT))
		{
			VExpression* op2 = ParseExpressionPriority5();
			op1 = new VBinary(PU_GT, op1, op2, l);
		}
		else if (TK_Check(PU_GE))
		{
			VExpression* op2 = ParseExpressionPriority5();
			op1 = new VBinary(PU_GE, op1, op2, l);
		}
		else
		{
			done = true;
		}
	}
	while (!done);
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority7
//
//==========================================================================

static VExpression* ParseExpressionPriority7()
{
	VExpression* op1 = ParseExpressionPriority6();
	if (!op1)
		return NULL;
	bool done = false;
	do
	{
		TLocation l = tk_Location;
		if (TK_Check(PU_EQ))
		{
			VExpression* op2 = ParseExpressionPriority6();
			op1 = new VBinary(PU_EQ, op1, op2, l);
		}
		else if (TK_Check(PU_NE))
		{
			VExpression* op2 = ParseExpressionPriority6();
			op1 = new VBinary(PU_NE, op1, op2, l);
		}
		else
		{
			done = true;
		}
	} while (!done);
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority8
//
//==========================================================================

static VExpression* ParseExpressionPriority8()
{
	VExpression* op1 = ParseExpressionPriority7();
	if (!op1)
		return NULL;
	TLocation l = tk_Location;
	while (TK_Check(PU_AND))
	{
		VExpression* op2 = ParseExpressionPriority7();
		op1 = new VBinary(PU_AND, op1, op2, l);
		l = tk_Location;
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority9
//
//==========================================================================

static VExpression* ParseExpressionPriority9()
{
	VExpression* op1 = ParseExpressionPriority8();
	if (!op1)
		return NULL;
	TLocation l = tk_Location;
	while (TK_Check(PU_XOR))
	{
		VExpression* op2 = ParseExpressionPriority8();
		op1 = new VBinary(PU_XOR, op1, op2, l);
		l = tk_Location;
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority10
//
//==========================================================================

static VExpression* ParseExpressionPriority10()
{
	VExpression* op1 = ParseExpressionPriority9();
	if (!op1)
		return NULL;
	TLocation l = tk_Location;
	while (TK_Check(PU_OR))
	{
		VExpression* op2 = ParseExpressionPriority9();
		op1 = new VBinary(PU_OR, op1, op2, l);
		l = tk_Location;
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority11
//
//==========================================================================

static VExpression* ParseExpressionPriority11()
{
	VExpression* op1 = ParseExpressionPriority10();
	if (!op1)
		return NULL;
	TLocation l = tk_Location;
	while (TK_Check(PU_AND_LOG))
	{
		VExpression* op2 = ParseExpressionPriority10();
		op1 = new VBinary(PU_AND_LOG, op1, op2, l);
		l = tk_Location;
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority12
//
//==========================================================================

static VExpression* ParseExpressionPriority12()
{
	VExpression* op1 = ParseExpressionPriority11();
	if (!op1)
		return NULL;
	TLocation l = tk_Location;
	while (TK_Check(PU_OR_LOG))
	{
		VExpression* op2 = ParseExpressionPriority11();
		op1 = new VBinary(PU_OR_LOG, op1, op2, l);
		l = tk_Location;
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority13
//
//==========================================================================

static VExpression* ParseExpressionPriority13()
{
	VExpression* op = ParseExpressionPriority12();
	if (!op)
		return NULL;
	TLocation l = tk_Location;
	if (TK_Check(PU_QUEST))
	{
		VExpression* op1 = ParseExpressionPriority13();
		TK_Expect(PU_COLON, ERR_MISSING_COLON);
		VExpression* op2 = ParseExpressionPriority13();
		op = new VConditional(op, op1, op2, l);
	}
	return op;
}

//==========================================================================
//
//	ParseExpressionPriority14
//
//==========================================================================

static VExpression* ParseExpressionPriority14()
{
	VExpression* op1 = ParseExpressionPriority13();
	if (!op1)
		return NULL;
	TLocation l = tk_Location;
	if (TK_Check(PU_ASSIGN))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(PU_ASSIGN, op1, op2, l);
	}
	else if (TK_Check(PU_ADD_ASSIGN))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(PU_ADD_ASSIGN, op1, op2, l);
	}
	else if (TK_Check(PU_MINUS_ASSIGN))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(PU_MINUS_ASSIGN, op1, op2, l);
	}
	else if (TK_Check(PU_MULTIPLY_ASSIGN))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(PU_MULTIPLY_ASSIGN, op1, op2, l);
	}
	else if (TK_Check(PU_DIVIDE_ASSIGN))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(PU_DIVIDE_ASSIGN, op1, op2, l);
	}
	else if (TK_Check(PU_MOD_ASSIGN))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(PU_MOD_ASSIGN, op1, op2, l);
	}
	else if (TK_Check(PU_AND_ASSIGN))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(PU_AND_ASSIGN, op1, op2, l);
	}
	else if (TK_Check(PU_OR_ASSIGN))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(PU_OR_ASSIGN, op1, op2, l);
	}
	else if (TK_Check(PU_XOR_ASSIGN))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(PU_XOR_ASSIGN, op1, op2, l);
	}
	else if (TK_Check(PU_LSHIFT_ASSIGN))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(PU_LSHIFT_ASSIGN, op1, op2, l);
	}
	else if (TK_Check(PU_RSHIFT_ASSIGN))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(PU_RSHIFT_ASSIGN, op1, op2, l);
	}
	return op1;
}

//==========================================================================
//
//	ParseExpression
//
//==========================================================================

TType ParseExpression(bool bLocals)
{
	if (bLocals && tk_Token == TK_KEYWORD)
	{
		TType type = CheckForTypeKeyword();
		if (type.type != ev_unknown)
		{
			VLocalDecl* Decl = ParseLocalVar(type, NAME_None);
			Decl->Declare();
			Decl->EmitInitialisations();
			delete Decl;
			return ev_void;
		}
	}

	CheckForLocal = bLocals;
	VExpression* op = ParseExpressionPriority14();
	if (!op)
	{
		return ev_void;
	}

	if (bLocals)
	{
		if (op->IsSingleName() && tk_Token == TK_IDENTIFIER)
		{
			VLocalDecl* Decl = ParseLocalVar(ev_unknown, ((VSingleName*)op)->Name);
			delete op;
			Decl->Declare();
			Decl->EmitInitialisations();
			delete Decl;
			return ev_void;
		}
	}

	if (!NumErrors)
	{
		op = op->ResolveTopLevel();
	}
	if (!op)
	{
		return ev_void;
	}
	if (!NumErrors)
	{
		op->Emit();
	}
	TType Ret = op->Type;
	delete op;
	return Ret;
}

//==========================================================================
//
//	SkipExpression
//
//==========================================================================

void SkipExpression(bool bLocals = false)
{
	if (bLocals && tk_Token == TK_KEYWORD)
	{
		TType type = CheckForTypeKeyword();
		if (type.type != ev_unknown)
		{
			VLocalDecl* Decl = ParseLocalVar(type, NAME_None);
			delete Decl;
			return;
		}
	}

	CheckForLocal = bLocals;
	VExpression* op = ParseExpressionPriority14();
	if (!op)
	{
		return;
	}

	if (bLocals)
	{
		if (op->IsSingleName() && tk_Token == TK_IDENTIFIER)
		{
			VLocalDecl* Decl = ParseLocalVar(ev_unknown, ((VSingleName*)op)->Name);
			delete op;
			delete Decl;
			return;
		}
	}

	delete op;
}

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
//	ParseCompoundStatement
//
//==========================================================================

static void ParseCompoundStatement()
{
	int		num_local_defs_on_start;

	num_local_defs_on_start = numlocaldefs;

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
