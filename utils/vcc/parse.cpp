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

static VExpression* ParseExpressionPriority13();
static VExpression* ParseExpression();
static VCompound* ParseCompoundStatement();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static bool				CheckForLocal;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	CheckForTypeKeyword
//
//==========================================================================

static TType CheckForTypeKeyword()
{
	if (TK_Check(KW_VOID))
	{
		return TType(ev_void);
	}
	if (TK_Check(KW_INT))
	{
		return TType(ev_int);
	}
	if (TK_Check(KW_FLOAT))
	{
		return TType(ev_float);
	}
	if (TK_Check(KW_NAME))
	{
		return TType(ev_name);
	}
	if (TK_Check(KW_STRING))
	{
		return TType(ev_string);
	}
	if (TK_Check(KW_CLASSID))
	{
		return TType(ev_classid);
	}
	if (TK_Check(KW_STATE))
	{
		return TType(ev_state);
	}
	if (TK_Check(KW_BOOL))
	{
		TType ret(ev_bool);
		ret.bit_mask = 1;
		return ret;
	}
	return TType(ev_unknown);
}

//==========================================================================
//
//	ParseDotMethodCall
//
//==========================================================================

static VExpression* ParseDotMethodCall(VExpression* SelfExpr,
	VName MethodName, TLocation Loc)
{
	VExpression* Args[MAX_PARAMS + 1];
	int NumArgs = 0;
	if (!TK_Check(PU_RPAREN))
	{
		do
		{
			Args[NumArgs] = ParseExpressionPriority13();
			if (NumArgs == MAX_PARAMS)
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
	VExpression* Args[MAX_PARAMS + 1];
	int NumArgs = 0;
	if (!TK_Check(PU_RPAREN))
	{
		do
		{
			Args[NumArgs] = ParseExpressionPriority13();
			if (NumArgs == MAX_PARAMS)
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
	VExpression* Args[MAX_PARAMS + 1];
	int NumArgs = 0;
	if (!TK_Check(PU_RPAREN))
	{
		do
		{
			Args[NumArgs] = ParseExpressionPriority13();
			if (NumArgs == MAX_PARAMS)
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

static VLocalDecl* ParseLocalVar(VExpression* TypeExpr)
{
	VLocalDecl* Decl = new VLocalDecl(tk_Location);
	do
	{
		VLocalEntry e;

		e.TypeExpr = TypeExpr->CreateTypeExprCopy();
		TLocation l = tk_Location;
		while (TK_Check(PU_ASTERISK))
		{
			e.TypeExpr = new VPointerType(e.TypeExpr, l);
			l = tk_Location;
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
			TLocation SLoc = tk_Location;
			VExpression* SE = ParseExpression();
			TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
			e.TypeExpr = new VFixedArrayType(e.TypeExpr, SE, SLoc);
		}
		//  Initialisation
		else if (TK_Check(PU_ASSIGN))
		{
			e.Value = ParseExpressionPriority13();
		}
		Decl->Vars.Append(e);
	} while (TK_Check(PU_COMMA));
	delete TypeExpr;
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
			return ParseLocalVar(new VSingleName(Name, Loc));
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
		op1 = new VBinaryLogical(PU_AND_LOG, op1, op2, l);
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
		op1 = new VBinaryLogical(PU_OR_LOG, op1, op2, l);
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

static VExpression* ParseExpression()
{
	CheckForLocal = false;
	return ParseExpressionPriority14();
}

//==========================================================================
//
//	ParseStatement
//
//==========================================================================

static VStatement* ParseStatement()
{
	TLocation l = tk_Location;
	switch(tk_Token)
	{
	case TK_EOF:
		ERR_Exit(ERR_UNEXPECTED_EOF, true, NULL);
		return NULL;

	case TK_KEYWORD:
		if (TK_Check(KW_IF))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			VExpression* e = ParseExpression();
			if (!e)
			{
				ParseError(tk_Location, "If expression expected");
			}
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			VStatement* STrue = ParseStatement();
			if (TK_Check(KW_ELSE))
			{
				VStatement* SFalse = ParseStatement();
				return new VIf(e, STrue, SFalse, l);
			}
			else
			{
				return new VIf(e, STrue, l);
			}
		}
		else if (TK_Check(KW_WHILE))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			VExpression* Expr = ParseExpression();
			if (!Expr)
			{
				ParseError(tk_Location, "Wile loop expression expected");
			}
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			VStatement* Statement = ParseStatement();
			return new VWhile(Expr, Statement, l);
		}
		else if (TK_Check(KW_DO))
		{
			VStatement* Statement = ParseStatement();
			TK_Expect(KW_WHILE, ERR_BAD_DO_STATEMENT);
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			VExpression* Expr = ParseExpression();
			if (!Expr)
			{
				ParseError(tk_Location, "Do loop expression expected");
			}
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			return new VDo(Expr, Statement, l);
		}
		else if (TK_Check(KW_FOR))
		{
			VFor* For = new VFor(l);
			For->NumLocalsOnStart = numlocaldefs;
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			do
			{
				VExpression* Expr = ParseExpression();
				if (!Expr)
				{
					break;
				}
				For->InitExpr.Append(new VDropResult(Expr));
			} while (TK_Check(PU_COMMA));
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			For->CondExpr = ParseExpression();
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			do
			{
				VExpression* Expr = ParseExpression();
				if (!Expr)
				{
					break;
				}
				For->LoopExpr.Append(new VDropResult(Expr));
			} while (TK_Check(PU_COMMA));
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			VStatement* Statement = ParseStatement();
			For->Statement = Statement;
			return For;
		}
		else if (TK_Check(KW_BREAK))
		{
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			return new VBreak(l);
		}
		else if (TK_Check(KW_CONTINUE))
		{
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			return new VContinue(l);
		}
		else if (TK_Check(KW_RETURN))
		{
			VExpression* Expr = ParseExpression();
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			return new VReturn(Expr, l);
		}
		else if (TK_Check(KW_SWITCH))
		{
			VSwitch* Switch = new VSwitch(l);
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			Switch->Expr = ParseExpression();
			if (!Switch->Expr)
			{
				ParseError(tk_Location, "Switch expression expected");
			}
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);

			TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
			do
			{
				l = tk_Location;
				if (TK_Check(KW_CASE))
				{
					VExpression* Expr = ParseExpression();
					if (!Expr)
					{
						ParseError(tk_Location, "Case value expected");
					}
					TK_Expect(PU_COLON, ERR_MISSING_COLON);
					Switch->Statements.Append(new VSwitchCase(Switch, Expr, l));
				}
				else if (TK_Check(KW_DEFAULT))
				{
					TK_Expect(PU_COLON, ERR_MISSING_COLON);
					Switch->Statements.Append(new VSwitchDefault(Switch, l));
				}
				else
				{
					VStatement* Statement = ParseStatement();
					Switch->Statements.Append(Statement);
				}
			} while (!TK_Check(PU_RBRACE));
			return Switch;
		}
		else
		{
			TType type = CheckForTypeKeyword();
			if (type.type != ev_unknown)
			{
				VLocalDecl* Decl = ParseLocalVar(new VTypeExpr(type, l));
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				return new VLocalVarStatement(Decl);
			}
			else
			{
				CheckForLocal = true;
				VExpression* Expr = ParseExpressionPriority14();
				if (Expr)
				{
					TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
					return new VExpressionStatement(new VDropResult(Expr));
				}
				else
				{
					ParseError(l, "Misplaced keyword %s", tk_String);
					TK_NextToken();
					return new VEmptyStatement(l);
				}
			}
		}

	case TK_PUNCT:
		if (TK_Check(PU_LBRACE))
		{
			return ParseCompoundStatement();
		}
		//Fall through

	default:
		CheckForLocal = true;
		VExpression* Expr = ParseExpressionPriority14();
		if (!Expr)
		{
			if (!TK_Check(PU_SEMICOLON))
			{
				ParseError(l, "Token %s makes no sense here", tk_String);
				TK_NextToken();
			}
			return new VEmptyStatement(l);
		}
		else if (Expr->IsSingleName() && tk_Token == TK_IDENTIFIER)
		{
			VLocalDecl* Decl = ParseLocalVar(Expr);
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			return new VLocalVarStatement(Decl);
		}
		else
		{
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			return new VExpressionStatement(new VDropResult(Expr));
		}
	}
}

//==========================================================================
//
//	ParseCompoundStatement
//
//==========================================================================

static VCompound* ParseCompoundStatement()
{
	VCompound* Comp = new VCompound(tk_Location);
	while (!TK_Check(PU_RBRACE))
	{
		Comp->Statements.Append(ParseStatement());
	}
	return Comp;
}

//==========================================================================
//
//	ParseType
//
//==========================================================================

static VExpression* ParseType()
{
	TLocation l = tk_Location;
	if (tk_Token == TK_KEYWORD)
	{
		TType t = CheckForTypeKeyword();
		if (t.type != ev_unknown)
		{
			return new VTypeExpr(t, l);
		}
	}
	else if (tk_Token == TK_IDENTIFIER)
	{
		VExpression* e = new VSingleName(tk_Name, l);
		TK_NextToken();
		return e;
	}
	return NULL;
}

//==========================================================================
//
//	ParseMethodDef
//
//==========================================================================

static void ParseMethodDef(VExpression* RetType, VName MName, TLocation MethodLoc,
	VClass* InClass, int Modifiers)
{
	if (CheckForFunction(InClass, MName))
	{
		ERR_Exit(ERR_FUNCTION_REDECLARED, true,
			"Function: %s.%s", *InClass->Name, *MName);
	}

	VMethod* Func = new VMethod(MName, InClass, MethodLoc);
	Func->Modifiers = Modifiers;
	Func->ReturnTypeExpr = RetType;
	InClass->AddMethod(Func);

	do
	{
		if (TK_Check(PU_VARARGS))
		{
			Func->Flags |= FUNC_VarArgs;
			break;
		}

		VMethodParam& P = Func->Params[Func->NumParams];

		P.TypeExpr = ParseType();
		if (!P.TypeExpr && Func->NumParams == 0)
		{
			break;
		}
		TLocation l = tk_Location;
		while (TK_Check(PU_ASTERISK))
		{
			P.TypeExpr = new VPointerType(P.TypeExpr, l);;
			l = tk_Location;
		}
		if (tk_Token == TK_IDENTIFIER)
		{
			P.Name = tk_Name;
			P.Loc = tk_Location;
			TK_NextToken();
		}
		if (Func->NumParams == MAX_PARAMS)
		{
			ParseError(tk_Location, "Method parameters overflow");
			continue;
		}
		Func->NumParams++;
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);

	if (TK_Check(PU_SEMICOLON))
	{
		numbuiltins++;
	}
	else
	{
		TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
		Func->Statement = ParseCompoundStatement();
	}
}

//==========================================================================
//
//	ParseDelegate
//
//==========================================================================

static void ParseDelegate(VExpression* RetType, VField* Delegate)
{
	VMethod* Func = new VMethod(NAME_None, Delegate, Delegate->Loc);
	Func->ReturnTypeExpr = RetType;

	TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
	do
	{
		VMethodParam& P = Func->Params[Func->NumParams];

		P.TypeExpr = ParseType();
		if (!P.TypeExpr && Func->NumParams == 0)
		{
			break;
		}
		TLocation l = tk_Location;
		while (TK_Check(PU_ASTERISK))
		{
			P.TypeExpr = new VPointerType(P.TypeExpr, l);;
			l = tk_Location;
		}
		if (tk_Token == TK_IDENTIFIER)
		{
			P.Name = tk_Name;
			P.Loc = tk_Location;
			TK_NextToken();
		}
		if (Func->NumParams == MAX_PARAMS)
		{
			ParseError(tk_Location, "Method parameters overflow");
			continue;
		}
		Func->NumParams++;
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);

	Delegate->func = Func;
	Delegate->type = TType(ev_delegate);
	Delegate->type.Function = Func;
}

//==========================================================================
//
//	ParseDefaultProperties
//
//==========================================================================

static void ParseDefaultProperties(VClass* InClass)
{
	VMethod* Func = new VMethod(NAME_None, InClass, tk_Location);
	Func->ReturnTypeExpr = new VTypeExpr(ev_void, tk_Location);
	Func->ReturnType = TType(ev_void);
	InClass->DefaultProperties = Func;

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	Func->Statement = ParseCompoundStatement();
}

//==========================================================================
//
//	ParseStruct
//
//==========================================================================

static void ParseStruct(VClass* InClass, bool IsVector)
{
	VName Name = tk_Name;
	TLocation StrLoc = tk_Location;
	if (tk_Token != TK_IDENTIFIER)
	{
		ParseError("Struct name expected");
		Name = NAME_None;
	}
	else
	{
		TK_NextToken();
	}

	//	New struct
	VStruct* Struct = new VStruct(Name, InClass ? (VMemberBase*)InClass :
		(VMemberBase*)CurrentPackage, StrLoc);
	Struct->Defined = false;
	Struct->IsVector = IsVector;
	Struct->Fields = NULL;

	if (!IsVector && TK_Check(PU_COLON))
	{
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError(tk_Location, "Parent class name expected");
		}
		else
		{
			Struct->ParentStructName = tk_Name;
			Struct->ParentStructLoc = tk_Location;
			TK_NextToken();
		}
	}

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		vint32 Modifiers = TModifiers::Parse();

		VExpression* Type = ParseType();
		if (!Type)
		{
			ParseError(tk_Location, "Field type expected.");
			TK_NextToken();
			continue;
		}

		do
		{
			VExpression* FieldType = Type->CreateTypeExprCopy();
			TLocation l = tk_Location;
			while (TK_Check(PU_ASTERISK))
			{
				FieldType = new VPointerType(FieldType, l);
				l = tk_Location;
			}

			VName FieldName(NAME_None);
			TLocation FieldLoc = tk_Location;
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError(tk_Location, "Field name expected");
			}
			else
			{
				FieldName = tk_Name;
			}
			TK_NextToken();
			if (TK_Check(PU_LINDEX))
			{
				TLocation SLoc = tk_Location;
				VExpression* e = ParseExpression();
				TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
				FieldType = new VFixedArrayType(FieldType, e, SLoc);
			}
			VField* fi = new VField(FieldName, Struct, FieldLoc);
			fi->TypeExpr = FieldType;
			fi->Modifiers = Modifiers;
			Struct->AddField(fi);
		} while (TK_Check(PU_COMMA));
		delete Type;
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	}
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);

	if (InClass)
	{
		InClass->Structs.Append(Struct);
	}
	else
	{
		ParsedStructs.Append(Struct);
	}
}

//==========================================================================
//
//	ParseStates
//
//==========================================================================

static void ParseStates(VClass* InClass)
{
	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	while (!TK_Check(PU_RBRACE))
	{
		//	State identifier
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError(tk_Location, "State name expected");
			TK_NextToken();
			continue;
		}
		VState* s = new VState(tk_Name, InClass, tk_Location);
		InClass->AddState(s);
		TK_NextToken();
		TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
		//	Sprite name
		if (tk_Token != TK_NAME)
		{
			ParseError(ERR_NONE, "Sprite name expected");
		}
		if (tk_Name != NAME_None && strlen(*tk_Name) != 4)
		{
			ParseError(ERR_NONE, "Invalid sprite name");
		}
		s->SpriteName = tk_Name;
		TK_NextToken();
		TK_Expect(PU_COMMA, ERR_NONE);
		//  Frame
		s->FrameExpr = ParseExpression();
		TK_Expect(PU_COMMA, ERR_NONE);
		if (tk_Token == TK_NAME)
		{
			//	Model
			s->ModelName = tk_Name;
			TK_NextToken();
			TK_Expect(PU_COMMA, ERR_NONE);
			//  Frame
			s->ModelFrameExpr = ParseExpression();
			if (!s->ModelFrameExpr)
			{
				ParseError(tk_Location, "Model frame number expected");
			}
			TK_Expect(PU_COMMA, ERR_NONE);
		}
		//  Tics
		s->TimeExpr = ParseExpression();
		TK_Expect(PU_COMMA, ERR_NONE);
		//  Next state
		if (tk_Token != TK_IDENTIFIER &&
			(tk_Token != TK_KEYWORD || tk_Keyword != KW_NONE))
		{
			ERR_Exit(ERR_NONE, true, NULL);
		}
		if (tk_Token == TK_IDENTIFIER)
		{
			s->NextStateName = tk_Name;
		}
		TK_NextToken();
		TK_Expect(PU_RPAREN, ERR_NONE);
		//	Code
		s->Function = new VMethod(NAME_None, s, s->Loc);
		s->Function->ReturnTypeExpr = new VTypeExpr(ev_void, tk_Location);
		s->Function->ReturnType = TType(ev_void);
		TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
		s->Function->Statement = ParseCompoundStatement();
	}
}

//==========================================================================
//
//	ParseClass
//
//==========================================================================

static void ParseClass()
{
	if (tk_Token != TK_IDENTIFIER)
	{
		ParseError(tk_Location, "Class name expected");
	}
	//	New class.
	VClass* Class = new VClass(tk_Name, CurrentPackage, tk_Location);
	Class->Parsed = false;
	Class->Defined = false;
	TK_NextToken();

	if (TK_Check(PU_COLON))
	{
		if (tk_Token != TK_IDENTIFIER)
		{
			ParseError(tk_Location, "Parent class name expected");
		}
		else
		{
			Class->ParentClassName = tk_Name;
			Class->ParentClassLoc = tk_Location;
			TK_NextToken();
		}
	}
	else if (Class->Name != NAME_Object)
	{
		ParseError("Parent class expected");
	}

	Class->Parsed = true;

	Class->Modifiers = TModifiers::Parse();
	do
	{
		if (TK_Check(KW_MOBJINFO))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			VExpression* e = ParseExpression();
			if (e)
			{
				Class->MobjInfoExpressions.Append(e);
			}
			else
			{
				ParseError(tk_Location, "Constant expression expected");
			}
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
		}
		else if (TK_Check(KW_SCRIPTID))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			VExpression* e = ParseExpression();
			if (e)
			{
				Class->ScriptIdExpressions.Append(e);
			}
			else
			{
				ParseError(tk_Location, "Constant expression expected");
			}
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
		}
		else
		{
			break;
		}
	} while (1);

	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
	while (!TK_Check(KW_DEFAULTPROPERTIES))
	{
		if (TK_Check(KW_STATES))
		{
			ParseStates(Class);
			continue;
		}

		if (TK_Check(KW_ENUM))
		{
			VConstant* PrevValue = NULL;
			TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
			do
			{
				if (tk_Token != TK_IDENTIFIER)
				{
					ParseError(tk_Location, "Identifier expected");
					TK_NextToken();
					continue;
				}
				if (CheckForConstant(Class, tk_Name))
				{
					ParseError(tk_Location, "Redefined identifier %s", *tk_Name);
				}
				VConstant* cDef = new VConstant(tk_Name, Class, tk_Location);
				cDef->Type = ev_int;
				TK_NextToken();
				if (TK_Check(PU_ASSIGN))
				{
					cDef->ValueExpr = ParseExpression();
				}
				else if (PrevValue)
				{
					cDef->PrevEnumValue = PrevValue;
				}
				else
				{
					cDef->ValueExpr = new VIntLiteral(0, tk_Location);
				}
				PrevValue = cDef;
				Class->AddConstant(cDef);
			} while (TK_Check(PU_COMMA));
			TK_Expect(PU_RBRACE, ERR_MISSING_RBRACE);
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			continue;
		}

		if (TK_Check(KW_CONST))
		{
			TType t = CheckForTypeKeyword();
			do
			{
				if (tk_Token != TK_IDENTIFIER)
				{
					ParseError(tk_Location, "Const name expected");
					TK_NextToken();
					continue;
				}
				if (CheckForConstant(Class, tk_Name))
				{
					ParseError(tk_Location, "Redefined identifier %s", *tk_Name);
				}
				VConstant* cDef = new VConstant(tk_Name, Class, tk_Location);
				cDef->Type = t.type;
				TK_NextToken();
				if (!TK_Check(PU_ASSIGN))
				{
					ParseError(tk_Location, "Assignement operator expected");
				}
				cDef->ValueExpr = ParseExpression();
				Class->AddConstant(cDef);
			} while (TK_Check(PU_COMMA));
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			continue;
		}

		if (TK_Check(KW_STRUCT))
		{
			ParseStruct(Class, false);
			continue;
		}

		if (TK_Check(KW_VECTOR))
		{
			ParseStruct(Class, true);
			continue;
		}

		if (TK_Check(KW_DELEGATE))
		{
			VExpression* Type = ParseType();
			if (!Type)
			{
				ParseError(tk_Location, "Field type expected.");
				continue;
			}
			TLocation l = tk_Location;
			while (TK_Check(PU_ASTERISK))
			{
				Type = new VPointerType(Type, l);
				l = tk_Location;
			}

			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError("Field name expected");
				continue;
			}
			VField* fi = new VField(tk_Name, Class, tk_Location);
			if (CheckForField(tk_Name, Class, false) ||
				CheckForMethod(tk_Name, Class))
			{
				ParseError("Redeclared field");
			}
			TK_NextToken();
			Class->AddField(fi);
			ParseDelegate(Type, fi);
			continue;
		}

		int Modifiers = TModifiers::Parse();

		VExpression* Type = ParseType();
		if (!Type)
		{
			ParseError(tk_Location, "Field type expected.");
			TK_NextToken();
			continue;
		}

		bool need_semicolon = true;
		do
		{
			VExpression* FieldType = Type->CreateTypeExprCopy();
			TLocation l = tk_Location;
			while (TK_Check(PU_ASTERISK))
			{
				FieldType = new VPointerType(FieldType, l);
				l = tk_Location;
			}
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError("Field name expected");
				continue;
			}
			VName FieldName = tk_Name;
			TLocation FieldLoc = tk_Location;
			TK_NextToken();

			if (CheckForField(FieldName, Class, false))
			{
				ParseError("Redeclared field");
				continue;
			}

			if (TK_Check(PU_LPAREN))
			{
				ParseMethodDef(FieldType, FieldName, FieldLoc, Class, Modifiers);
				need_semicolon = false;
				break;
			}

			if (TK_Check(PU_LINDEX))
			{
				TLocation SLoc = tk_Location;
				VExpression* e = ParseExpression();
				TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
				FieldType = new VFixedArrayType(FieldType, e, SLoc);
			}

			VField* fi = new VField(FieldName, Class, FieldLoc);
			fi->TypeExpr = FieldType;
			fi->Modifiers = Modifiers;
			Class->AddField(fi);
		} while (TK_Check(PU_COMMA));
		delete Type;
		if (need_semicolon)
		{
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		}
	}

	ParseDefaultProperties(Class);

	ParsedClasses.Append(Class);
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
				VImportedPackage& I = PackagesToLoad.Alloc();
				I.Name = tk_Name;
				I.Loc = tk_Location;
				TK_NextToken();
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			}
			else if (TK_Check(KW_ENUM))
			{
				VConstant* PrevValue = NULL;
				TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
				do
				{
					if (tk_Token != TK_IDENTIFIER)
					{
						ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
					}
					if (CheckForConstant(NULL, tk_Name))
					{
						ParseError(tk_Location, "Redefined identifier %s", *tk_Name);
					}
					VConstant* cDef = new VConstant(tk_Name, CurrentPackage, tk_Location);
					cDef->Type = ev_int;
					TK_NextToken();
					if (TK_Check(PU_ASSIGN))
					{
						cDef->ValueExpr = ParseExpression();
					}
					else if (PrevValue)
					{
						cDef->PrevEnumValue = PrevValue;
					}
					else
					{
						cDef->ValueExpr = new VIntLiteral(0, tk_Location);
					}
					PrevValue = cDef;
					ParsedConstants.Append(cDef);
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
