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

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VParser::ParseDotMethodCall
//
//==========================================================================

VExpression* VParser::ParseDotMethodCall(VExpression* SelfExpr,
	VName MethodName, TLocation Loc)
{
	VExpression* Args[MAX_PARAMS + 1];
	int NumArgs = 0;
	if (!Lex.Check(TK_RParen))
	{
		do
		{
			Args[NumArgs] = ParseExpressionPriority13();
			if (NumArgs == MAX_PARAMS)
				ParseError(Lex.Location, "Too many arguments");
			else
				NumArgs++;
		} while (Lex.Check(TK_Comma));
		Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
	}
	return new VDotInvocation(SelfExpr, MethodName, Loc, NumArgs, Args);
}

//==========================================================================
//
//	VParser::ParseBaseMethodCall
//
//==========================================================================

VExpression* VParser::ParseBaseMethodCall(VName Name, TLocation Loc)
{
	VExpression* Args[MAX_PARAMS + 1];
	int NumArgs = 0;
	if (!Lex.Check(TK_RParen))
	{
		do
		{
			Args[NumArgs] = ParseExpressionPriority13();
			if (NumArgs == MAX_PARAMS)
				ParseError(Lex.Location, "Too many arguments");
			else
				NumArgs++;
		} while (Lex.Check(TK_Comma));
		Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
	}
	return new VBaseInvocation(Name, NumArgs, Args, Loc);
}

//==========================================================================
//
//	VParser::ParseMethodCallOrCast
//
//==========================================================================

VExpression* VParser::ParseMethodCallOrCast(VName Name, TLocation Loc)
{
	VExpression* Args[MAX_PARAMS + 1];
	int NumArgs = 0;
	if (!Lex.Check(TK_RParen))
	{
		do
		{
			Args[NumArgs] = ParseExpressionPriority13();
			if (NumArgs == MAX_PARAMS)
				ParseError(Lex.Location, "Too many arguments");
			else
				NumArgs++;
		} while (Lex.Check(TK_Comma));
		Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
	}
	return new VCastOrInvocation(Name, Loc, NumArgs, Args);
}

//==========================================================================
//
//	VParser::ParseLocalVar
//
//==========================================================================

VLocalDecl* VParser::ParseLocalVar(VExpression* TypeExpr)
{
	VLocalDecl* Decl = new VLocalDecl(Lex.Location);
	do
	{
		VLocalEntry e;

		e.TypeExpr = TypeExpr->CreateTypeExprCopy();
		TLocation l = Lex.Location;
		while (Lex.Check(TK_Asterisk))
		{
			e.TypeExpr = new VPointerType(e.TypeExpr, l);
			l = Lex.Location;
		}
		if (Lex.Token != TK_Identifier)
		{
			ParseError(Lex.Location, "Invalid identifier, variable name expected");
			continue;
		}
		e.Loc = Lex.Location;
		e.Name = Lex.Name;
		Lex.NextToken();

		if (Lex.Check(TK_LBracket))
		{
			TLocation SLoc = Lex.Location;
			VExpression* SE = ParseExpression();
			Lex.Expect(TK_RBracket, ERR_MISSING_RFIGURESCOPE);
			e.TypeExpr = new VFixedArrayType(e.TypeExpr, SE, SLoc);
		}
		//  Initialisation
		else if (Lex.Check(TK_Assign))
		{
			e.Value = ParseExpressionPriority13();
		}
		Decl->Vars.Append(e);
	} while (Lex.Check(TK_Comma));
	delete TypeExpr;
	return Decl;
}

//==========================================================================
//
//	VParser::ParseExpressionPriority0
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority0()
{
	VName		Name;
	bool		bLocals;
	TLocation	Loc;

	bLocals = CheckForLocal;
	CheckForLocal = false;
	TLocation l = Lex.Location;
	switch (Lex.Token)
	{
	case TK_IntLiteral:
	{
		vint32 Val = Lex.Number;
		Lex.NextToken();
		return new VIntLiteral(Val, l);
	}

	case TK_FloatLiteral:
	{
		float Val = Lex.Float;
		Lex.NextToken();
		return new VFloatLiteral(Val, l);
	}

	case TK_NameLiteral:
	{
		VName Val = Lex.Name;
		Lex.NextToken();
		return new VNameLiteral(Val, l);
	}

	case TK_StringLiteral:
	{
		int Val = Package->FindString(Lex.String);
		Lex.NextToken();
		return new VStringLiteral(Val, l);
	}

	case TK_Self:
		Lex.NextToken();
		return new VSelf(l);

	case TK_None:
		Lex.NextToken();
		return new VNoneLiteral(l);
	
	case TK_Null:
		Lex.NextToken();
		return new VNullLiteral(l);

	case TK_False:
		Lex.NextToken();
		return new VIntLiteral(0, l);

	case TK_True:
		Lex.NextToken();
		return new VIntLiteral(1, l);

	case TK_Vector:
	{
		Lex.NextToken();
		Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
		VExpression* op1 = ParseExpressionPriority13();
		Lex.Expect(TK_Comma);
		VExpression* op2 = ParseExpressionPriority13();
		Lex.Expect(TK_Comma);
		VExpression* op3 = ParseExpressionPriority13();
		Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
		return new VVector(op1, op2, op3, l);
	}

	case TK_LParen:
	{
		Lex.NextToken();
		VExpression* op = ParseExpressionPriority13();
		if (!op)
		{
			ParseError(l, "Expression expected");
		}
		Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
		return op;
	}

	case TK_DColon:
		Lex.NextToken();
		if (Lex.Token != TK_Identifier)
		{
			ParseError(l, "Method name expected.");
			break;
		}
		Loc = Lex.Location;
		Name = Lex.Name;
		Lex.NextToken();
		Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
		return ParseBaseMethodCall(Name, Loc);

	case TK_Identifier:
		Loc = Lex.Location;
		Name = Lex.Name;
		Lex.NextToken();
		if (Lex.Check(TK_LParen))
		{
			return ParseMethodCallOrCast(Name, Loc);
		}

		if (Lex.Check(TK_DColon))
		{
			if (Lex.Token != TK_Identifier)
			{
				ParseError(Lex.Location, "Identifier expected");
				break;
			}
			VName Name2 = Lex.Name;
			Lex.NextToken();
			if (bLocals && Lex.Token == TK_Asterisk)
			{
				return ParseLocalVar(new VDoubleName(Name, Name2, Loc));
			}
			return new VDoubleName(Name, Name2, Loc);
		}

		if (bLocals && Lex.Token == TK_Asterisk)
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
//	VParser::ParseExpressionPriority1
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority1()
{
	VExpression* op = ParseExpressionPriority0();
	if (!op)
		return NULL;
	bool done = false;
	do
	{
		TLocation l = Lex.Location;

		if (Lex.Check(TK_Arrow))
		{
			if (Lex.Token != TK_Identifier)
			{
				ParseError(Lex.Location, "Invalid identifier, field name expacted");
			}
			else
			{
				op = new VPointerField(op, Lex.Name, Lex.Location);
				Lex.NextToken();
			}
		}
		else if (Lex.Check(TK_Dot))
		{
			if (Lex.Token != TK_Identifier)
			{
				ParseError(Lex.Location, "Invalid identifier, field name expacted");
			}
			else
			{
				VName FieldName = Lex.Name;
				TLocation Loc = Lex.Location;
				Lex.NextToken();
				if (Lex.Check(TK_LParen))
				{
					op = ParseDotMethodCall(op, FieldName, Loc);
				}
				else
				{
					op = new VDotField(op, FieldName, Loc);
				}
			}
		}
		else if (Lex.Check(TK_LBracket))
		{
			VExpression* ind = ParseExpressionPriority13();
			Lex.Expect(TK_RBracket, ERR_BAD_ARRAY);
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
//	VParser::ParseExpressionPriority2
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority2()
{
	VExpression*	op;

	TLocation l = Lex.Location;

	if (Lex.Check(TK_Plus))
	{
		op = ParseExpressionPriority2();
		return new VUnary(VUnary::Plus, op, l);
	}

	if (Lex.Check(TK_Minus))
	{
		op = ParseExpressionPriority2();
		return new VUnary(VUnary::Minus, op, l);
	}

	if (Lex.Check(TK_Not))
	{
		op = ParseExpressionPriority2();
		return new VUnary(VUnary::Not, op, l);
	}

	if (Lex.Check(TK_Tilde))
	{
		op = ParseExpressionPriority2();
		return new VUnary(VUnary::BitInvert, op, l);
	}

	if (Lex.Check(TK_And))
	{
		op = ParseExpressionPriority1();
		return new VUnary(VUnary::TakeAddress, op, l);
	}

	if (Lex.Check(TK_Asterisk))
	{
		op = ParseExpressionPriority2();
		return new VPushPointed(op);
	}

	if (Lex.Check(TK_Inc))
	{
		op = ParseExpressionPriority2();
		return new VUnaryMutator(VUnaryMutator::PreInc, op, l);
	}

	if (Lex.Check(TK_Dec))
	{
		op = ParseExpressionPriority2();
		return new VUnaryMutator(VUnaryMutator::PreDec, op, l);
	}

	op = ParseExpressionPriority1();
	if (!op)
		return NULL;
	l = Lex.Location;

	if (Lex.Check(TK_Inc))
	{
		return new VUnaryMutator(VUnaryMutator::PostInc, op, l);
	}

	if (Lex.Check(TK_Dec))
	{
		return new VUnaryMutator(VUnaryMutator::PostDec, op, l);
	}

	return op;
}

//==========================================================================
//
//	VParser::ParseExpressionPriority3
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority3()
{
	VExpression* op1 = ParseExpressionPriority2();
	if (!op1)
		return NULL;
	bool done = false;
	do
	{
		TLocation l = Lex.Location;
		if (Lex.Check(TK_Asterisk))
		{
			VExpression* op2 = ParseExpressionPriority2();
			op1 = new VBinary(VBinary::Multiply, op1, op2, l);
		}
		else if (Lex.Check(TK_Slash))
		{
			VExpression* op2 = ParseExpressionPriority2();
			op1 = new VBinary(VBinary::Divide, op1, op2, l);
		}
		else if (Lex.Check(TK_Percent))
		{
			VExpression* op2 = ParseExpressionPriority2();
			op1 = new VBinary(VBinary::Modulus, op1, op2, l);
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
//	VParser::ParseExpressionPriority4
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority4()
{
	VExpression* op1 = ParseExpressionPriority3();
	if (!op1)
		return NULL;
	bool done = false;
	do
	{
		TLocation l = Lex.Location;
		if (Lex.Check(TK_Plus))
		{
			VExpression* op2 = ParseExpressionPriority3();
			op1 = new VBinary(VBinary::Add, op1, op2, l);
		}
		else if (Lex.Check(TK_Minus))
		{
			VExpression* op2 = ParseExpressionPriority3();
			op1 = new VBinary(VBinary::Subtract, op1, op2, l);
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
//	VParser::ParseExpressionPriority5
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority5()
{
	VExpression* op1 = ParseExpressionPriority4();
	if (!op1)
		return NULL;
	bool done = false;
	do
	{
		TLocation l = Lex.Location;
		if (Lex.Check(TK_LShift))
		{
			VExpression* op2 = ParseExpressionPriority4();
			op1 = new VBinary(VBinary::LShift, op1, op2, l);
		}
		else if (Lex.Check(TK_RShift))
		{
			VExpression* op2 = ParseExpressionPriority4();
			op1 = new VBinary(VBinary::RShift, op1, op2, l);
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
//	VParser::ParseExpressionPriority6
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority6()
{
	VExpression* op1 = ParseExpressionPriority5();
	if (!op1)
		return NULL;
	bool done = false;
	do
	{
		TLocation l = Lex.Location;
		if (Lex.Check(TK_Less))
		{
			VExpression* op2 = ParseExpressionPriority5();
			op1 = new VBinary(VBinary::Less, op1, op2, l);
		}
		else if (Lex.Check(TK_LessEquals))
		{
			VExpression* op2 = ParseExpressionPriority5();
			op1 = new VBinary(VBinary::LessEquals, op1, op2, l);
		}
		else if (Lex.Check(TK_Greater))
		{
			VExpression* op2 = ParseExpressionPriority5();
			op1 = new VBinary(VBinary::Greater, op1, op2, l);
		}
		else if (Lex.Check(TK_GreaterEquals))
		{
			VExpression* op2 = ParseExpressionPriority5();
			op1 = new VBinary(VBinary::GreaterEquals, op1, op2, l);
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
//	VParser::ParseExpressionPriority7
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority7()
{
	VExpression* op1 = ParseExpressionPriority6();
	if (!op1)
		return NULL;
	bool done = false;
	do
	{
		TLocation l = Lex.Location;
		if (Lex.Check(TK_Equals))
		{
			VExpression* op2 = ParseExpressionPriority6();
			op1 = new VBinary(VBinary::Equals, op1, op2, l);
		}
		else if (Lex.Check(TK_NotEquals))
		{
			VExpression* op2 = ParseExpressionPriority6();
			op1 = new VBinary(VBinary::NotEquals, op1, op2, l);
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
//	VParser::ParseExpressionPriority8
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority8()
{
	VExpression* op1 = ParseExpressionPriority7();
	if (!op1)
		return NULL;
	TLocation l = Lex.Location;
	while (Lex.Check(TK_And))
	{
		VExpression* op2 = ParseExpressionPriority7();
		op1 = new VBinary(VBinary::And, op1, op2, l);
		l = Lex.Location;
	}
	return op1;
}

//==========================================================================
//
//	VParser::ParseExpressionPriority9
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority9()
{
	VExpression* op1 = ParseExpressionPriority8();
	if (!op1)
		return NULL;
	TLocation l = Lex.Location;
	while (Lex.Check(TK_XOr))
	{
		VExpression* op2 = ParseExpressionPriority8();
		op1 = new VBinary(VBinary::XOr, op1, op2, l);
		l = Lex.Location;
	}
	return op1;
}

//==========================================================================
//
//	VParser::ParseExpressionPriority10
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority10()
{
	VExpression* op1 = ParseExpressionPriority9();
	if (!op1)
		return NULL;
	TLocation l = Lex.Location;
	while (Lex.Check(TK_Or))
	{
		VExpression* op2 = ParseExpressionPriority9();
		op1 = new VBinary(VBinary::Or, op1, op2, l);
		l = Lex.Location;
	}
	return op1;
}

//==========================================================================
//
//	VParser::ParseExpressionPriority11
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority11()
{
	VExpression* op1 = ParseExpressionPriority10();
	if (!op1)
		return NULL;
	TLocation l = Lex.Location;
	while (Lex.Check(TK_AndLog))
	{
		VExpression* op2 = ParseExpressionPriority10();
		op1 = new VBinaryLogical(VBinaryLogical::And, op1, op2, l);
		l = Lex.Location;
	}
	return op1;
}

//==========================================================================
//
//	VParser::ParseExpressionPriority12
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority12()
{
	VExpression* op1 = ParseExpressionPriority11();
	if (!op1)
		return NULL;
	TLocation l = Lex.Location;
	while (Lex.Check(TK_OrLog))
	{
		VExpression* op2 = ParseExpressionPriority11();
		op1 = new VBinaryLogical(VBinaryLogical::Or, op1, op2, l);
		l = Lex.Location;
	}
	return op1;
}

//==========================================================================
//
//	VParser::ParseExpressionPriority13
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority13()
{
	VExpression* op = ParseExpressionPriority12();
	if (!op)
		return NULL;
	TLocation l = Lex.Location;
	if (Lex.Check(TK_Quest))
	{
		VExpression* op1 = ParseExpressionPriority13();
		Lex.Expect(TK_Colon, ERR_MISSING_COLON);
		VExpression* op2 = ParseExpressionPriority13();
		op = new VConditional(op, op1, op2, l);
	}
	return op;
}

//==========================================================================
//
//	VParser::ParseExpressionPriority14
//
//==========================================================================

VExpression* VParser::ParseExpressionPriority14()
{
	VExpression* op1 = ParseExpressionPriority13();
	if (!op1)
		return NULL;
	TLocation l = Lex.Location;
	if (Lex.Check(TK_Assign))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(VAssignment::Assign, op1, op2, l);
	}
	else if (Lex.Check(TK_AddAssign))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(VAssignment::AddAssign, op1, op2, l);
	}
	else if (Lex.Check(TK_MinusAssign))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(VAssignment::MinusAssign, op1, op2, l);
	}
	else if (Lex.Check(TK_MultiplyAssign))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(VAssignment::MultiplyAssign, op1, op2, l);
	}
	else if (Lex.Check(TK_DivideAssign))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(VAssignment::DivideAssign, op1, op2, l);
	}
	else if (Lex.Check(TK_ModAssign))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(VAssignment::ModAssign, op1, op2, l);
	}
	else if (Lex.Check(TK_AndAssign))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(VAssignment::AndAssign, op1, op2, l);
	}
	else if (Lex.Check(TK_OrAssign))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(VAssignment::OrAssign, op1, op2, l);
	}
	else if (Lex.Check(TK_XOrAssign))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(VAssignment::XOrAssign, op1, op2, l);
	}
	else if (Lex.Check(TK_LShiftAssign))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(VAssignment::LShiftAssign, op1, op2, l);
	}
	else if (Lex.Check(TK_RShiftAssign))
	{
		VExpression* op2 = ParseExpressionPriority13();
		return new VAssignment(VAssignment::RShiftAssign, op1, op2, l);
	}
	return op1;
}

//==========================================================================
//
//	VParser::ParseExpression
//
//==========================================================================

VExpression* VParser::ParseExpression()
{
	CheckForLocal = false;
	return ParseExpressionPriority14();
}

//==========================================================================
//
//	VParser::ParseStatement
//
//==========================================================================

VStatement* VParser::ParseStatement()
{
	TLocation l = Lex.Location;
	switch(Lex.Token)
	{
	case TK_EOF:
		ParseError(Lex.Location, ERR_UNEXPECTED_EOF);
		return NULL;

	case TK_If:
	{
		Lex.NextToken();
		Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
		VExpression* e = ParseExpression();
		if (!e)
		{
			ParseError(Lex.Location, "If expression expected");
		}
		Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
		VStatement* STrue = ParseStatement();
		if (Lex.Check(TK_Else))
		{
			VStatement* SFalse = ParseStatement();
			return new VIf(e, STrue, SFalse, l);
		}
		else
		{
			return new VIf(e, STrue, l);
		}
	}

	case TK_While:
	{
		Lex.NextToken();
		Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
		VExpression* Expr = ParseExpression();
		if (!Expr)
		{
			ParseError(Lex.Location, "Wile loop expression expected");
		}
		Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
		VStatement* Statement = ParseStatement();
		return new VWhile(Expr, Statement, l);
	}

	case TK_Do:
	{
		Lex.NextToken();
		VStatement* Statement = ParseStatement();
		Lex.Expect(TK_While, ERR_BAD_DO_STATEMENT);
		Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
		VExpression* Expr = ParseExpression();
		if (!Expr)
		{
			ParseError(Lex.Location, "Do loop expression expected");
		}
		Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
		Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
		return new VDo(Expr, Statement, l);
	}

	case TK_For:
	{
		Lex.NextToken();
		VFor* For = new VFor(l);
		Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
		do
		{
			VExpression* Expr = ParseExpression();
			if (!Expr)
			{
				break;
			}
			For->InitExpr.Append(new VDropResult(Expr));
		} while (Lex.Check(TK_Comma));
		Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
		For->CondExpr = ParseExpression();
		Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
		do
		{
			VExpression* Expr = ParseExpression();
			if (!Expr)
			{
				break;
			}
			For->LoopExpr.Append(new VDropResult(Expr));
		} while (Lex.Check(TK_Comma));
		Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
		VStatement* Statement = ParseStatement();
		For->Statement = Statement;
		return For;
	}

	case TK_Break:
		Lex.NextToken();
		Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
		return new VBreak(l);

	case TK_Continue:
		Lex.NextToken();
		Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
		return new VContinue(l);

	case TK_Return:
	{
		Lex.NextToken();
		VExpression* Expr = ParseExpression();
		Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
		return new VReturn(Expr, l);
	}

	case TK_Switch:
	{
		Lex.NextToken();
		VSwitch* Switch = new VSwitch(l);
		Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
		Switch->Expr = ParseExpression();
		if (!Switch->Expr)
		{
			ParseError(Lex.Location, "Switch expression expected");
		}
		Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);

		Lex.Expect(TK_LBrace, ERR_MISSING_LBRACE);
		do
		{
			l = Lex.Location;
			if (Lex.Check(TK_Case))
			{
				VExpression* Expr = ParseExpression();
				if (!Expr)
				{
					ParseError(Lex.Location, "Case value expected");
				}
				Lex.Expect(TK_Colon, ERR_MISSING_COLON);
				Switch->Statements.Append(new VSwitchCase(Switch, Expr, l));
			}
			else if (Lex.Check(TK_Default))
			{
				Lex.Expect(TK_Colon, ERR_MISSING_COLON);
				Switch->Statements.Append(new VSwitchDefault(Switch, l));
			}
			else
			{
				VStatement* Statement = ParseStatement();
				Switch->Statements.Append(Statement);
			}
		} while (!Lex.Check(TK_RBrace));
		return Switch;
	}

	case TK_LBrace:
		Lex.NextToken();
		return ParseCompoundStatement();

	case TK_Bool:
	case TK_ClassId:
	case TK_Float:
	case TK_Int:
	case TK_Name:
	case TK_State:
	case TK_String:
	case TK_Void:
	{
		VExpression* TypeExpr = ParseType();
		VLocalDecl* Decl = ParseLocalVar(TypeExpr);
		Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
		return new VLocalVarStatement(Decl);
	}

	default:
		CheckForLocal = true;
		VExpression* Expr = ParseExpressionPriority14();
		if (!Expr)
		{
			if (!Lex.Check(TK_Semicolon))
			{
				ParseError(l, "Token %s makes no sense here", VLexer::TokenNames[Lex.Token]);
				Lex.NextToken();
			}
			return new VEmptyStatement(l);
		}
		else if (Expr->IsValidTypeExpression() && Lex.Token == TK_Identifier)
		{
			VLocalDecl* Decl = ParseLocalVar(Expr);
			Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
			return new VLocalVarStatement(Decl);
		}
		else
		{
			Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
			return new VExpressionStatement(new VDropResult(Expr));
		}
	}
}

//==========================================================================
//
//	VParser::ParseCompoundStatement
//
//==========================================================================

VCompound* VParser::ParseCompoundStatement()
{
	VCompound* Comp = new VCompound(Lex.Location);
	while (!Lex.Check(TK_RBrace))
	{
		Comp->Statements.Append(ParseStatement());
	}
	return Comp;
}

//==========================================================================
//
//	VParser::ParseType
//
//==========================================================================

VExpression* VParser::ParseType()
{
	TLocation l = Lex.Location;
	switch (Lex.Token)
	{
	case TK_Void:
		Lex.NextToken();
		return new VTypeExpr(ev_void, l);

	case TK_Int:
		Lex.NextToken();
		return new VTypeExpr(ev_int, l);

	case TK_Float:
		Lex.NextToken();
		return new VTypeExpr(ev_float, l);

	case TK_Name:
		Lex.NextToken();
		return new VTypeExpr(ev_name, l);

	case TK_String:
		Lex.NextToken();
		return new VTypeExpr(ev_string, l);

	case TK_ClassId:
		Lex.NextToken();
		return new VTypeExpr(ev_classid, l);

	case TK_State:
		Lex.NextToken();
		return new VTypeExpr(ev_state, l);

	case TK_Bool:
	{
		Lex.NextToken();
		TType ret(ev_bool);
		ret.bit_mask = 1;
		return new VTypeExpr(ret, l);
	}

	case TK_Identifier:
	{
		VName Name = Lex.Name;
		Lex.NextToken();
		if (Lex.Check(TK_DColon))
		{
			if (Lex.Token != TK_Identifier)
			{
				ParseError(Lex.Location, "Identifier expected");
				return new VSingleName(Name, l);
			}
			VName Name2 = Lex.Name;
			Lex.NextToken();
			return new VDoubleName(Name, Name2, l);
		}
		return new VSingleName(Name, l);
	}

	default:
		return NULL;
	}
}

//==========================================================================
//
//	VParser::ParseMethodDef
//
//==========================================================================

void VParser::ParseMethodDef(VExpression* RetType, VName MName,
	TLocation MethodLoc, VClass* InClass, vint32 Modifiers)
{
	if (InClass->CheckForFunction(MName))
	{
		ParseError(MethodLoc, "Redeclared method %s.%s", *InClass->Name, *MName);
	}

	VMethod* Func = new VMethod(MName, InClass, MethodLoc);
	Func->Modifiers = Modifiers;
	Func->ReturnTypeExpr = RetType;
	InClass->AddMethod(Func);

	do
	{
		if (Lex.Check(TK_VarArgs))
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
		TLocation l = Lex.Location;
		while (Lex.Check(TK_Asterisk))
		{
			P.TypeExpr = new VPointerType(P.TypeExpr, l);;
			l = Lex.Location;
		}
		if (Lex.Token == TK_Identifier)
		{
			P.Name = Lex.Name;
			P.Loc = Lex.Location;
			Lex.NextToken();
		}
		if (Func->NumParams == MAX_PARAMS)
		{
			ParseError(Lex.Location, "Method parameters overflow");
			continue;
		}
		Func->NumParams++;
	} while (Lex.Check(TK_Comma));
	Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);

	if (Lex.Check(TK_Semicolon))
	{
		Package->numbuiltins++;
	}
	else
	{
		Lex.Expect(TK_LBrace, ERR_MISSING_LBRACE);
		Func->Statement = ParseCompoundStatement();
	}
}

//==========================================================================
//
//	VParser::ParseDelegate
//
//==========================================================================

void VParser::ParseDelegate(VExpression* RetType, VField* Delegate)
{
	VMethod* Func = new VMethod(NAME_None, Delegate, Delegate->Loc);
	Func->ReturnTypeExpr = RetType;

	Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
	do
	{
		VMethodParam& P = Func->Params[Func->NumParams];

		P.TypeExpr = ParseType();
		if (!P.TypeExpr && Func->NumParams == 0)
		{
			break;
		}
		TLocation l = Lex.Location;
		while (Lex.Check(TK_Asterisk))
		{
			P.TypeExpr = new VPointerType(P.TypeExpr, l);;
			l = Lex.Location;
		}
		if (Lex.Token == TK_Identifier)
		{
			P.Name = Lex.Name;
			P.Loc = Lex.Location;
			Lex.NextToken();
		}
		if (Func->NumParams == MAX_PARAMS)
		{
			ParseError(Lex.Location, "Method parameters overflow");
			continue;
		}
		Func->NumParams++;
	} while (Lex.Check(TK_Comma));
	Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
	Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);

	Delegate->func = Func;
	Delegate->type = TType(ev_delegate);
	Delegate->type.Function = Func;
}

//==========================================================================
//
//	VParser::ParseDefaultProperties
//
//==========================================================================

void VParser::ParseDefaultProperties(VClass* InClass)
{
	VMethod* Func = new VMethod(NAME_None, InClass, Lex.Location);
	Func->ReturnTypeExpr = new VTypeExpr(ev_void, Lex.Location);
	Func->ReturnType = TType(ev_void);
	InClass->DefaultProperties = Func;

	Lex.Expect(TK_LBrace, ERR_MISSING_LBRACE);
	Func->Statement = ParseCompoundStatement();
}

//==========================================================================
//
//	VParser::ParseStruct
//
//==========================================================================

void VParser::ParseStruct(VClass* InClass, bool IsVector)
{
	VName Name = Lex.Name;
	TLocation StrLoc = Lex.Location;
	if (Lex.Token != TK_Identifier)
	{
		ParseError(Lex.Location, "Struct name expected");
		Name = NAME_None;
	}
	else
	{
		Lex.NextToken();
	}

	//	New struct
	VStruct* Struct = new VStruct(Name, InClass ? (VMemberBase*)InClass :
		(VMemberBase*)Package, StrLoc);
	Struct->Defined = false;
	Struct->IsVector = IsVector;
	Struct->Fields = NULL;

	if (!IsVector && Lex.Check(TK_Colon))
	{
		if (Lex.Token != TK_Identifier)
		{
			ParseError(Lex.Location, "Parent class name expected");
		}
		else
		{
			Struct->ParentStructName = Lex.Name;
			Struct->ParentStructLoc = Lex.Location;
			Lex.NextToken();
		}
	}

	Lex.Expect(TK_LBrace, ERR_MISSING_LBRACE);
	while (!Lex.Check(TK_RBrace))
	{
		vint32 Modifiers = TModifiers::Parse(Lex);

		VExpression* Type = ParseType();
		if (!Type)
		{
			ParseError(Lex.Location, "Field type expected.");
			Lex.NextToken();
			continue;
		}

		do
		{
			VExpression* FieldType = Type->CreateTypeExprCopy();
			TLocation l = Lex.Location;
			while (Lex.Check(TK_Asterisk))
			{
				FieldType = new VPointerType(FieldType, l);
				l = Lex.Location;
			}

			VName FieldName(NAME_None);
			TLocation FieldLoc = Lex.Location;
			if (Lex.Token != TK_Identifier)
			{
				ParseError(Lex.Location, "Field name expected");
			}
			else
			{
				FieldName = Lex.Name;
			}
			Lex.NextToken();
			if (Lex.Check(TK_LBracket))
			{
				TLocation SLoc = Lex.Location;
				VExpression* e = ParseExpression();
				Lex.Expect(TK_RBracket, ERR_MISSING_RFIGURESCOPE);
				FieldType = new VFixedArrayType(FieldType, e, SLoc);
			}
			VField* fi = new VField(FieldName, Struct, FieldLoc);
			fi->TypeExpr = FieldType;
			fi->Modifiers = Modifiers;
			Struct->AddField(fi);
		} while (Lex.Check(TK_Comma));
		delete Type;
		Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
	}
	Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);

	if (InClass)
	{
		InClass->Structs.Append(Struct);
	}
	else
	{
		Package->ParsedStructs.Append(Struct);
	}
}

//==========================================================================
//
//	VParser::ParseStates
//
//==========================================================================

void VParser::ParseStates(VClass* InClass)
{
	Lex.Expect(TK_LBrace, ERR_MISSING_LBRACE);
	while (!Lex.Check(TK_RBrace))
	{
		//	State identifier
		if (Lex.Token != TK_Identifier)
		{
			ParseError(Lex.Location, "State name expected");
			Lex.NextToken();
			continue;
		}
		VState* s = new VState(Lex.Name, InClass, Lex.Location);
		InClass->AddState(s);
		Lex.NextToken();
		Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
		//	Sprite name
		if (Lex.Token != TK_NameLiteral)
		{
			ParseError(Lex.Location, "Sprite name expected");
		}
		if (Lex.Name != NAME_None && strlen(*Lex.Name) != 4)
		{
			ParseError(Lex.Location, "Invalid sprite name");
		}
		s->SpriteName = Lex.Name;
		Lex.NextToken();
		Lex.Expect(TK_Comma);
		//  Frame
		s->FrameExpr = ParseExpression();
		Lex.Expect(TK_Comma);
		if (Lex.Token == TK_NameLiteral)
		{
			//	Model
			s->ModelName = Lex.Name;
			Lex.NextToken();
			Lex.Expect(TK_Comma);
			//  Frame
			s->ModelFrameExpr = ParseExpression();
			if (!s->ModelFrameExpr)
			{
				ParseError(Lex.Location, "Model frame number expected");
			}
			Lex.Expect(TK_Comma);
		}
		//  Tics
		s->TimeExpr = ParseExpression();
		Lex.Expect(TK_Comma);
		//  Next state
		if (Lex.Token != TK_Identifier && Lex.Token != TK_None)
		{
			ParseError(Lex.Location, "Next state name expectred");
		}
		if (Lex.Token == TK_Identifier)
		{
			s->NextStateName = Lex.Name;
		}
		Lex.NextToken();
		Lex.Expect(TK_RParen, ERR_NONE);
		//	Code
		s->Function = new VMethod(NAME_None, s, s->Loc);
		s->Function->ReturnTypeExpr = new VTypeExpr(ev_void, Lex.Location);
		s->Function->ReturnType = TType(ev_void);
		Lex.Expect(TK_LBrace, ERR_MISSING_LBRACE);
		s->Function->Statement = ParseCompoundStatement();
	}
}

//==========================================================================
//
//	VParser::ParseClass
//
//==========================================================================

void VParser::ParseClass()
{
	if (Lex.Token != TK_Identifier)
	{
		ParseError(Lex.Location, "Class name expected");
	}
	//	New class.
	VClass* Class = new VClass(Lex.Name, Package, Lex.Location);
	Class->Defined = false;
	Lex.NextToken();

	if (Lex.Check(TK_Colon))
	{
		if (Lex.Token != TK_Identifier)
		{
			ParseError(Lex.Location, "Parent class name expected");
		}
		else
		{
			Class->ParentClassName = Lex.Name;
			Class->ParentClassLoc = Lex.Location;
			Lex.NextToken();
		}
	}
	else if (Class->Name != NAME_Object)
	{
		ParseError(Lex.Location, "Parent class expected");
	}

	Class->Modifiers = TModifiers::Parse(Lex);
	do
	{
		if (Lex.Check(TK_MobjInfo))
		{
			Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
			VExpression* e = ParseExpression();
			if (e)
			{
				Class->MobjInfoExpressions.Append(e);
			}
			else
			{
				ParseError(Lex.Location, "Constant expression expected");
			}
			Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
		}
		else if (Lex.Check(TK_ScriptId))
		{
			Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
			VExpression* e = ParseExpression();
			if (e)
			{
				Class->ScriptIdExpressions.Append(e);
			}
			else
			{
				ParseError(Lex.Location, "Constant expression expected");
			}
			Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
		}
		else
		{
			break;
		}
	} while (1);

	Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
	while (!Lex.Check(TK_DefaultProperties))
	{
		if (Lex.Check(TK_States))
		{
			ParseStates(Class);
			continue;
		}

		if (Lex.Check(TK_Enum))
		{
			VConstant* PrevValue = NULL;
			Lex.Expect(TK_LBrace, ERR_MISSING_LBRACE);
			do
			{
				if (Lex.Token != TK_Identifier)
				{
					ParseError(Lex.Location, "Identifier expected");
					Lex.NextToken();
					continue;
				}
				if (Class->CheckForConstant(Lex.Name))
				{
					ParseError(Lex.Location, "Redefined identifier %s", *Lex.Name);
				}
				VConstant* cDef = new VConstant(Lex.Name, Class, Lex.Location);
				cDef->Type = ev_int;
				Lex.NextToken();
				if (Lex.Check(TK_Assign))
				{
					cDef->ValueExpr = ParseExpression();
				}
				else if (PrevValue)
				{
					cDef->PrevEnumValue = PrevValue;
				}
				else
				{
					cDef->ValueExpr = new VIntLiteral(0, Lex.Location);
				}
				PrevValue = cDef;
				Class->AddConstant(cDef);
			} while (Lex.Check(TK_Comma));
			Lex.Expect(TK_RBrace, ERR_MISSING_RBRACE);
			Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
			continue;
		}

		if (Lex.Check(TK_Const))
		{
			int Type = ev_unknown;
			if (Lex.Check(TK_Int))
			{
				Type = ev_int;
			}
			else if (Lex.Check(TK_Float))
			{
				Type = ev_float;
			}
			else if (Lex.Check(TK_Name))
			{
				Type = ev_name;
			}
			else if (Lex.Check(TK_String))
			{
				Type = ev_string;
			}
			else
			{
				ParseError(Lex.Location, "Bad constant type");
				Lex.NextToken();
			}
			do
			{
				if (Lex.Token != TK_Identifier)
				{
					ParseError(Lex.Location, "Const name expected");
					Lex.NextToken();
					continue;
				}
				if (Class->CheckForConstant(Lex.Name))
				{
					ParseError(Lex.Location, "Redefined identifier %s", *Lex.Name);
				}
				VConstant* cDef = new VConstant(Lex.Name, Class, Lex.Location);
				cDef->Type = Type;
				Lex.NextToken();
				if (!Lex.Check(TK_Assign))
				{
					ParseError(Lex.Location, "Assignement operator expected");
				}
				cDef->ValueExpr = ParseExpression();
				Class->AddConstant(cDef);
			} while (Lex.Check(TK_Comma));
			Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
			continue;
		}

		if (Lex.Check(TK_Struct))
		{
			ParseStruct(Class, false);
			continue;
		}

		if (Lex.Check(TK_Vector))
		{
			ParseStruct(Class, true);
			continue;
		}

		if (Lex.Check(TK_Delegate))
		{
			VExpression* Type = ParseType();
			if (!Type)
			{
				ParseError(Lex.Location, "Field type expected.");
				continue;
			}
			TLocation l = Lex.Location;
			while (Lex.Check(TK_Asterisk))
			{
				Type = new VPointerType(Type, l);
				l = Lex.Location;
			}

			if (Lex.Token != TK_Identifier)
			{
				ParseError(Lex.Location, "Field name expected");
				continue;
			}
			VField* fi = new VField(Lex.Name, Class, Lex.Location);
			if (Class->CheckForField(Lex.Location, Lex.Name, false) ||
				Class->CheckForMethod(Lex.Name))
			{
				ParseError(Lex.Location, "Redeclared field");
			}
			Lex.NextToken();
			Class->AddField(fi);
			ParseDelegate(Type, fi);
			continue;
		}

		int Modifiers = TModifiers::Parse(Lex);

		VExpression* Type = ParseType();
		if (!Type)
		{
			ParseError(Lex.Location, "Field type expected.");
			Lex.NextToken();
			continue;
		}

		bool need_semicolon = true;
		do
		{
			VExpression* FieldType = Type->CreateTypeExprCopy();
			TLocation l = Lex.Location;
			while (Lex.Check(TK_Asterisk))
			{
				FieldType = new VPointerType(FieldType, l);
				l = Lex.Location;
			}
			if (Lex.Token != TK_Identifier)
			{
				ParseError(Lex.Location, "Field name expected");
				continue;
			}
			VName FieldName = Lex.Name;
			TLocation FieldLoc = Lex.Location;
			Lex.NextToken();

			if (Class->CheckForField(FieldLoc, FieldName, false))
			{
				ParseError(Lex.Location, "Redeclared field");
				continue;
			}

			if (Lex.Check(TK_LParen))
			{
				ParseMethodDef(FieldType, FieldName, FieldLoc, Class, Modifiers);
				need_semicolon = false;
				break;
			}

			if (Lex.Check(TK_LBracket))
			{
				TLocation SLoc = Lex.Location;
				VExpression* e = ParseExpression();
				Lex.Expect(TK_RBracket, ERR_MISSING_RFIGURESCOPE);
				FieldType = new VFixedArrayType(FieldType, e, SLoc);
			}

			VField* fi = new VField(FieldName, Class, FieldLoc);
			fi->TypeExpr = FieldType;
			fi->Modifiers = Modifiers;
			Class->AddField(fi);
		} while (Lex.Check(TK_Comma));
		delete Type;
		if (need_semicolon)
		{
			Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
		}
	}

	ParseDefaultProperties(Class);

	Package->ParsedClasses.Append(Class);
}

//==========================================================================
//
//	VParser::Parse
//
//==========================================================================

void VParser::Parse()
{
	bool		done;

	dprintf("Compiling pass 1\n");

	Lex.NextToken();
	done = false;
	while (!done)
	{
		switch(Lex.Token)
		{
		case TK_EOF:
			done = true;
			break;

		case TK_Import:
		{
			Lex.NextToken();
			if (Lex.Token != TK_Identifier)
			{
				ParseError(Lex.Location, "Package name expected");
			}
			VImportedPackage& I = Package->PackagesToLoad.Alloc();
			I.Name = Lex.Name;
			I.Loc = Lex.Location;
			Lex.NextToken();
			Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
			break;
		}

		case TK_Enum:
		{
			Lex.NextToken();
			VConstant* PrevValue = NULL;
			Lex.Expect(TK_LBrace, ERR_MISSING_LBRACE);
			do
			{
				if (Lex.Token != TK_Identifier)
				{
					ParseError(Lex.Location, "Expected IDENTIFIER");
				}
				if (Package->CheckForConstant(Lex.Name))
				{
					ParseError(Lex.Location, "Redefined identifier %s", *Lex.Name);
				}
				VConstant* cDef = new VConstant(Lex.Name, Package, Lex.Location);
				cDef->Type = ev_int;
				Lex.NextToken();
				if (Lex.Check(TK_Assign))
				{
					cDef->ValueExpr = ParseExpression();
				}
				else if (PrevValue)
				{
					cDef->PrevEnumValue = PrevValue;
				}
				else
				{
					cDef->ValueExpr = new VIntLiteral(0, Lex.Location);
				}
				PrevValue = cDef;
				Package->ParsedConstants.Append(cDef);
			} while (Lex.Check(TK_Comma));
			Lex.Expect(TK_RBrace, ERR_MISSING_RBRACE);
			Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
			break;
		}

		case TK_Struct:
			Lex.NextToken();
			ParseStruct(NULL, false);
			break;

		case TK_Vector:
			Lex.NextToken();
			ParseStruct(NULL, true);
			break;

		case TK_Class:
			Lex.NextToken();
			ParseClass();
			break;

		default:
			ParseError(Lex.Location, "Invalid token \"%s\"",
				VLexer::TokenNames[Lex.Token]);
			Lex.NextToken();
			break;
		}
	}

	if (NumErrors)
	{
		BailOut();
	}
}
