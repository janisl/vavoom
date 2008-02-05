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
	bool bLocals = CheckForLocal;
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
	{
		Lex.NextToken();
		if (Lex.Token != TK_Identifier)
		{
			ParseError(l, "Method name expected.");
			break;
		}
		l = Lex.Location;
		VName Name = Lex.Name;
		Lex.NextToken();
		Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
		return ParseBaseMethodCall(Name, l);
	}

	case TK_Identifier:
	{
		VName Name = Lex.Name;
		Lex.NextToken();
		if (Lex.Check(TK_LParen))
		{
			return ParseMethodCallOrCast(Name, l);
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
				return ParseLocalVar(new VDoubleName(Name, Name2, l));
			}
			return new VDoubleName(Name, Name2, l);
		}

		if (bLocals && Lex.Token == TK_Asterisk)
		{
			return ParseLocalVar(new VSingleName(Name, l));
		}

		return new VSingleName(Name, l);
	}

	case TK_Default:
	{
		VExpression* Expr = new VDefaultObject(new VSelf(l), l);
		Lex.NextToken();
		Lex.Expect(TK_Dot);
		if (Lex.Token != TK_Identifier)
		{
			ParseError(Lex.Location, "Invalid identifier, field name expacted");
		}
		VName FieldName = Lex.Name;
		TLocation Loc = Lex.Location;
		Lex.NextToken();
		if (Lex.Check(TK_LParen))
		{
			ParseError(Lex.Location, "Tried to call method on a default object");
		}
		return new VDotField(Expr, FieldName, Loc);
	}

	case TK_Class:
	{
		Lex.NextToken();
		Lex.Expect(TK_Less);
		if (Lex.Token != TK_Identifier)
		{
			ParseError(Lex.Location, "Identifier expected");
			break;
		}
		VName ClassName = Lex.Name;
		Lex.NextToken();
		Lex.Expect(TK_Greater);
		Lex.Expect(TK_LParen);
		VExpression* Expr = ParseExpressionPriority13();
		if (!Expr)
		{
			ParseError(Lex.Location, "Expression expected");
		}
		Lex.Expect(TK_RParen);
		return new VDynamicClassCast(ClassName, Expr, l);
	}

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
			if (Lex.Check(TK_Default))
			{
				Lex.Expect(TK_Dot);
				op = new VDefaultObject(op, l);
			}
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
					if (op->IsDefaultObject())
					{
						ParseError(Lex.Location, "Tried to call method on a default object");
					}
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

	case TK_Foreach:
	{
		Lex.NextToken();
		VExpression* Expr = ParseExpression();
		if (!Expr)
		{
			ParseError(Lex.Location, "Iterator expression expected");
		}
		VStatement* Statement = ParseStatement();
		return new VForeach(Expr, Statement, l);
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
	case TK_Byte:
	case TK_Class:
	case TK_Float:
	case TK_Int:
	case TK_Name:
	case TK_State:
	case TK_String:
	case TK_Void:
	case TK_Array:
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
		return new VTypeExpr(TYPE_Void, l);

	case TK_Int:
		Lex.NextToken();
		return new VTypeExpr(TYPE_Int, l);

	case TK_Byte:
		Lex.NextToken();
		return new VTypeExpr(TYPE_Byte, l);

	case TK_Bool:
	{
		Lex.NextToken();
		VFieldType ret(TYPE_Bool);
		ret.BitMask = 1;
		return new VTypeExpr(ret, l);
	}

	case TK_Float:
		Lex.NextToken();
		return new VTypeExpr(TYPE_Float, l);

	case TK_Name:
		Lex.NextToken();
		return new VTypeExpr(TYPE_Name, l);

	case TK_String:
		Lex.NextToken();
		return new VTypeExpr(TYPE_String, l);

	case TK_Class:
	{
		Lex.NextToken();
		VName MetaClassName = NAME_None;
		if (Lex.Check(TK_Less))
		{
			if (Lex.Token != TK_Identifier)
			{
				ParseError(Lex.Location, "Invalid identifier, class name expected");
			}
			else
			{
				MetaClassName = Lex.Name;
				Lex.NextToken();
			}
			Lex.Expect(TK_Greater);
		}
		return new VTypeExpr(TYPE_Class, l, MetaClassName);
	}

	case TK_State:
		Lex.NextToken();
		return new VTypeExpr(TYPE_State, l);

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

	case TK_Array:
	{
		Lex.NextToken();
		Lex.Expect(TK_Less);
		VExpression* Inner = ParseType();
		if (!Inner)
		{
			ParseError(Lex.Location, "Inner type declaration expected");
		}
		while (Lex.Check(TK_Asterisk))
		{
			Inner = new VPointerType(Inner, Lex.Location);
		}
		Lex.Expect(TK_Greater);
		return new VDynamicArrayType(Inner, l);
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
	TLocation MethodLoc, VClass* InClass, vint32 Modifiers, bool Iterator)
{
	if (InClass->CheckForFunction(MName))
	{
		ParseError(MethodLoc, "Redeclared method %s.%s", *InClass->Name, *MName);
	}

	VMethod* Func = new VMethod(MName, InClass, MethodLoc);
	Func->Modifiers = Modifiers;
	Func->ReturnTypeExpr = RetType;
	if (Iterator)
	{
		Func->Flags |= FUNC_Iterator;
	}
	InClass->AddMethod(Func);

	do
	{
		if (Lex.Check(TK_VarArgs))
		{
			Func->Flags |= FUNC_VarArgs;
			break;
		}

		VMethodParam& P = Func->Params[Func->NumParams];

		P.Modifiers = TModifiers::Parse(Lex);

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
		Package->NumBuiltins++;
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

	Delegate->Func = Func;
	Delegate->Type = VFieldType(TYPE_Delegate);
	Delegate->Type.Function = Func;
}

//==========================================================================
//
//	VParser::ParseDefaultProperties
//
//==========================================================================

void VParser::ParseDefaultProperties(VClass* InClass)
{
	VMethod* Func = new VMethod(NAME_None, InClass, Lex.Location);
	Func->ReturnTypeExpr = new VTypeExpr(TYPE_Void, Lex.Location);
	Func->ReturnType = VFieldType(TYPE_Void);
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
			fi->Flags = TModifiers::FieldAttr(TModifiers::Check(Modifiers,
				TModifiers::Native | TModifiers::Private |
				TModifiers::ReadOnly | TModifiers::Transient, FieldLoc));
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
//	VParser::ParseStateString
//
//==========================================================================

VName VParser::ParseStateString()
{
	char		StateStr[256];

	if (Lex.Token != TK_Identifier)
	{
		ParseError(Lex.Location, "Identifier expected");
		return NAME_None;
	}
	VStr::Cpy(StateStr, *Lex.Name);
	Lex.NextToken();

	if (Lex.Check(TK_DColon))
	{
		if (Lex.Token != TK_Identifier)
		{
			ParseError(Lex.Location, "Identifier expected");
			return NAME_None;
		}
		strcat(StateStr, "::");
		strcat(StateStr, *Lex.Name);
		Lex.NextToken();
	}

	if (Lex.Check(TK_Dot))
	{
		if (Lex.Token != TK_Identifier)
		{
			ParseError(Lex.Location, "Identifier expected");
			return NAME_None;
		}
		strcat(StateStr, ".");
		strcat(StateStr, *Lex.Name);
		Lex.NextToken();
	}

	return StateStr;
}

//==========================================================================
//
//	VParser::ParseStates
//
//==========================================================================

void VParser::ParseStates(VClass* InClass)
{
	Lex.Expect(TK_LBrace, ERR_MISSING_LBRACE);
	int StateIdx = 0;
	VState* PrevState = NULL;
	VState* LoopStart = NULL;
	int NewLabelsStart = InClass->StateLabels.Num();
	while (!Lex.Check(TK_RBrace))
	{
		TLocation TmpLoc = Lex.Location;
		VName TmpName = ParseStateString();

		//	Goto command.
		if (TmpName == NAME_Goto)
		{
			VName GotoLabel = ParseStateString();
			int GotoOffset = 0;
			if (Lex.Check(TK_Plus))
			{
				if (Lex.Token != TK_IntLiteral)
				{
					ParseError(Lex.Location, "Number expected");
					continue;
				}
				GotoOffset = Lex.Number;
				Lex.NextToken();
			}

			if (!PrevState && NewLabelsStart == InClass->StateLabels.Num())
			{
				ParseError(Lex.Location, "Goto before first state");
				continue;
			}
			if (PrevState)
			{
				PrevState->GotoLabel = GotoLabel;
				PrevState->GotoOffset = GotoOffset;
			}
			for (int i = NewLabelsStart; i < InClass->StateLabels.Num(); i++)
			{
				InClass->StateLabels[i].GotoLabel = GotoLabel;
				InClass->StateLabels[i].GotoOffset = GotoOffset;
			}
			NewLabelsStart = InClass->StateLabels.Num();
			PrevState = NULL;
			continue;
		}

		//	Stop command.
		if (TmpName == NAME_Stop)
		{
			if (!PrevState && NewLabelsStart == InClass->StateLabels.Num())
			{
				ParseError(Lex.Location, "Stop before first state");
				continue;
			}
			if (PrevState)
			{
				PrevState->NextState = NULL;
			}
			for (int i = NewLabelsStart; i < InClass->StateLabels.Num(); i++)
			{
				InClass->StateLabels[i].State = NULL;
			}
			NewLabelsStart = InClass->StateLabels.Num();
			PrevState = NULL;
			continue;
		}

		//	Wait command.
		if (TmpName == NAME_Wait || TmpName == NAME_Fail)
		{
			if (!PrevState)
			{
				ParseError(Lex.Location, "%s before first state", *TmpName);
				continue;
			}
			PrevState->NextState = PrevState;
			PrevState = NULL;
			continue;
		}

		//	Loop command.
		if (TmpName == NAME_Loop)
		{
			if (!PrevState)
			{
				ParseError(Lex.Location, "Loop before first state");
				continue;
			}
			PrevState->NextState = LoopStart;
			PrevState = NULL;
			continue;
		}

		//	Check for label.
		if (Lex.Check(TK_Colon))
		{
			VStateLabel& Lbl = InClass->StateLabels.Alloc();
			Lbl.Loc = TmpLoc;
			Lbl.Name = TmpName;
			continue;
		}

		char StateName[16];
		sprintf(StateName, "S_%d", StateIdx);
		VState* s = new VState(StateName, InClass, TmpLoc);
		InClass->AddState(s);

		//	Sprite name
		char SprName[8];
		SprName[0] = 0;
		if (VStr::Length(*TmpName) != 4)
		{
			ParseError(Lex.Location, "Invalid sprite name");
		}
		else
		{
			SprName[0] = VStr::ToLower((*TmpName)[0]);
			SprName[1] = VStr::ToLower((*TmpName)[1]);
			SprName[2] = VStr::ToLower((*TmpName)[2]);
			SprName[3] = VStr::ToLower((*TmpName)[3]);
			SprName[4] = 0;
		}
		s->SpriteName = SprName;

		//  Frame
		VName FramesString(NAME_None);
		TLocation FramesLoc;
		if (Lex.Token != TK_Identifier && Lex.Token != TK_StringLiteral)
		{
			ParseError(Lex.Location, "Identifier expected");
		}
		char FChar = VStr::ToUpper(Lex.String[0]);
		if (FChar < 'A' || FChar > ']')
		{
			ParseError(Lex.Location, "Frames must be A-Z, [, \\ or ]");
		}
		s->Frame = FChar - 'A';
		FramesString = Lex.String;
		FramesLoc = Lex.Location;
		Lex.NextToken();

		//  Tics
		bool Neg = Lex.Check(TK_Minus);
		if (Lex.Token == TK_IntLiteral)
		{
			if (Neg)
			{
				s->Time = -Lex.Number;
			}
			else
			{
				s->Time = float(Lex.Number) / 35.0;
			}
			Lex.NextToken();
		}
		else if (Lex.Token == TK_FloatLiteral)
		{
			if (Neg)
			{
				s->Time = -Lex.Float;
			}
			else
			{
				s->Time = float(Lex.Float) / 35.0;
			}
			Lex.NextToken();
		}
		else
		{
			ParseError(Lex.Location, "State duration expected");
		}

		while (Lex.Token == TK_Identifier && !Lex.NewLine)
		{
			if (Lex.Name == NAME_Bright)
			{
				s->Frame |= VState::FF_FULLBRIGHT;
				Lex.NextToken();
				continue;
			}
			if (Lex.Name == NAME_Offset)
			{
				Lex.NextToken();
				Lex.Expect(TK_LParen);
				Neg = Lex.Check(TK_Minus);
				if (Lex.Token != TK_IntLiteral)
				{
					ParseError(Lex.Location, "Integer expected");
				}
				s->Misc1 = Lex.Number * (Neg ? -1 : 1);
				Lex.NextToken();
				Lex.Expect(TK_Comma);
				Neg = Lex.Check(TK_Minus);
				if (Lex.Token != TK_IntLiteral)
				{
					ParseError(Lex.Location, "Integer expected");
				}
				s->Misc2 = Lex.Number * (Neg ? -1 : 1);
				Lex.NextToken();
				Lex.Expect(TK_RParen);
				continue;
			}
			break;
		}

		//	Code
		if (Lex.Check(TK_LBrace))
		{
			if (VStr::Length(*FramesString) > 1)
			{
				ParseError(Lex.Location, "Only states with single frame can have code block");
			}
			s->Function = new VMethod(NAME_None, s, s->Loc);
			s->Function->ReturnTypeExpr = new VTypeExpr(TYPE_Void, Lex.Location);
			s->Function->ReturnType = VFieldType(TYPE_Void);
			s->Function->Statement = ParseCompoundStatement();
		}
		else if (!Lex.NewLine)
		{
			if (Lex.Token != TK_Identifier)
			{
				ParseError(Lex.Location, "State method name expected");
			}
			else
			{
				s->FunctionName = Lex.Name;
				Lex.NextToken();
			}
		}

		//	Link previous state.
		if (PrevState)
		{
			PrevState->NextState = s;
		}

		//	Assign state to the labels.
		for (int i = NewLabelsStart; i < InClass->StateLabels.Num(); i++)
		{
			InClass->StateLabels[i].State = s;
			LoopStart = s;
		}
		NewLabelsStart = InClass->StateLabels.Num();
		PrevState = s;
		StateIdx++;

		for (size_t i = 1; i < VStr::Length(*FramesString); i++)
		{
			char FChar = VStr::ToUpper((*FramesString)[i]);
			if (FChar < 'A' || FChar > ']')
			{
				ParseError(Lex.Location, "Frames must be A-Z, [, \\ or ]");
			}

			//	Create a new state.
			sprintf(StateName, "S_%d", StateIdx);
			VState* s2 = new VState(StateName, InClass, TmpLoc);
			InClass->AddState(s2);
			s2->SpriteName = s->SpriteName;
			s2->Frame = (s->Frame & VState::FF_FULLBRIGHT) | (FChar - 'A');
			s2->Time = s->Time;
			s2->Misc1 = s->Misc1;
			s2->Misc2 = s->Misc2;
			s2->FunctionName = s->FunctionName;

			//	Link previous state.
			PrevState->NextState = s2;
			PrevState = s2;
			StateIdx++;
		}
	}

	//	Make sure all state labels have corresponding states.
	if (NewLabelsStart != InClass->StateLabels.Num())
	{
		ParseError(Lex.Location, "State label at the end of state block");
	}
	if (PrevState)
	{
		ParseError(Lex.Location, "State block not ended");
	}
}

//==========================================================================
//
//	VParser::ParseReplication
//
//==========================================================================

void VParser::ParseReplication(VClass* Class)
{
	Lex.Expect(TK_LBrace);
	while (!Lex.Check(TK_RBrace))
	{
		VRepInfo& RI = Class->RepInfos.Alloc();

		//	Reliable or unreliable flag, currently unused.
		if (Lex.Check(TK_Reliable))
		{
			RI.Reliable = true;
		}
		else if (Lex.Check(TK_Unreliable))
		{
			RI.Reliable = false;
		}
		else
		{
			ParseError(Lex.Location, "Expected reliable or unreliable");
		}

		//	Replication condition.
		RI.Cond = new VMethod(NAME_None, Class, Lex.Location);
		RI.Cond->ReturnType = VFieldType(TYPE_Bool);
		RI.Cond->ReturnType.BitMask = 1;
		RI.Cond->ReturnTypeExpr = new VTypeExpr(RI.Cond->ReturnType,
			Lex.Location);
		Lex.Expect(TK_If);
		Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
		VExpression* e = ParseExpression();
		if (!e)
		{
			ParseError(Lex.Location, "If expression expected");
		}
		Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
		RI.Cond->Statement = new VReturn(e, RI.Cond->Loc);

		//	Fields
		do
		{
			if (Lex.Token != TK_Identifier)
			{
				ParseError(Lex.Location, "Field name expected");
			}
			else
			{
				VRepField& F = RI.RepFields.Alloc();
				F.Name = Lex.Name;
				F.Loc = Lex.Location;
				F.Member = NULL;
				Lex.NextToken();
			}
		}
		while (Lex.Check(TK_Comma));
		Lex.Expect(TK_Semicolon);
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
	else if (VMemberBase::StaticFindClass(Lex.Name))
	{
		ParseError(Lex.Location, "Class %s already has been declared",
			*Lex.Name);
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
			if (!e)
			{
				ParseError(Lex.Location, "Constant expression expected");
			}
			else if (Class->MobjInfoExpr)
			{
				ParseError(Lex.Location, "Only one Editor ID allowed");
			}
			else
			{
				Class->MobjInfoExpr = e;
			}
			Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
		}
		else if (Lex.Check(TK_ScriptId))
		{
			Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
			VExpression* e = ParseExpression();
			if (!e)
			{
				ParseError(Lex.Location, "Constant expression expected");
			}
			else if (Class->ScriptIdExpr)
			{
				ParseError(Lex.Location, "Only one script ID allowed");
			}
			else
			{
				Class->ScriptIdExpr = e;
			}
			Lex.Expect(TK_RParen, ERR_MISSING_RPAREN);
		}
		else if (Lex.Check(TK_Game))
		{
			Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
			VExpression* e = ParseExpression();
			if (!e)
			{
				ParseError(Lex.Location, "Constant expression expected");
			}
			else if (Class->GameExpr)
			{
				ParseError(Lex.Location, "Only one game expression allowed");
			}
			else
			{
				Class->GameExpr = e;
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
				cDef->Type = TYPE_Int;
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
			int Type = TYPE_Unknown;
			if (Lex.Check(TK_Int))
			{
				Type = TYPE_Int;
			}
			else if (Lex.Check(TK_Float))
			{
				Type = TYPE_Float;
			}
			else if (Lex.Check(TK_Name))
			{
				Type = TYPE_Name;
			}
			else if (Lex.Check(TK_String))
			{
				Type = TYPE_String;
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

		if (Lex.Check(TK_Replication))
		{
			ParseReplication(Class);
			continue;
		}

		int Modifiers = TModifiers::Parse(Lex);

		if (Lex.Check(TK_Iterator))
		{
			if (Lex.Token != TK_Identifier)
			{
				ParseError(Lex.Location, "Method name expected");
			}
			VName FieldName = Lex.Name;
			TLocation FieldLoc = Lex.Location;
			Lex.NextToken();
			Lex.Expect(TK_LParen, ERR_MISSING_LPAREN);
			ParseMethodDef(new VTypeExpr(VFieldType(TYPE_Void).MakePointerType(),
				Lex.Location), FieldName, FieldLoc, Class, Modifiers, true);
			continue;
		}

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

			if (Lex.Check(TK_LBrace))
			{
				VProperty* Prop = new VProperty(FieldName, Class, FieldLoc);
				Prop->TypeExpr = FieldType;
				Prop->Flags = TModifiers::PropAttr(TModifiers::Check(Modifiers,
					TModifiers::Native | TModifiers::Final, FieldLoc));
				do
				{
					if (Lex.Check(TK_Get))
					{
						char TmpName[NAME_SIZE];
						sprintf(TmpName, "get_%s", *FieldName);
						VMethod* Func = new VMethod(TmpName, Class, Lex.Location);
						Func->Modifiers = Modifiers;
						Func->ReturnTypeExpr = FieldType->CreateTypeExprCopy();

						if (Modifiers & TModifiers::Native)
						{
							Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
							Package->NumBuiltins++;
						}
						else
						{
							Lex.Expect(TK_LBrace, ERR_MISSING_LBRACE);
							Func->Statement = ParseCompoundStatement();
						}

						if (Prop->GetFunc)
						{
							ParseError(FieldLoc, "Property already has a get method");
							ParseError(Prop->GetFunc->Loc, "Previous get method here");
						}
						Prop->GetFunc = Func;
						Class->AddMethod(Func);
					}
					else if (Lex.Check(TK_Set))
					{
						char TmpName[NAME_SIZE];
						sprintf(TmpName, "set_%s", *FieldName);
						VMethod* Func = new VMethod(TmpName, Class, Lex.Location);
						Func->Modifiers = Modifiers;
						Func->ReturnTypeExpr = new VTypeExpr(TYPE_Void, Lex.Location);

						VMethodParam& P = Func->Params[Func->NumParams];
						P.Modifiers = 0;
						P.TypeExpr = FieldType->CreateTypeExprCopy();
						P.Name = "value";
						P.Loc = Lex.Location;
						Func->NumParams++;

						if (Modifiers & TModifiers::Native)
						{
							Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
							Package->NumBuiltins++;
						}
						else
						{
							Lex.Expect(TK_LBrace, ERR_MISSING_LBRACE);
							Func->Statement = ParseCompoundStatement();
						}

						if (Prop->SetFunc)
						{
							ParseError(FieldLoc, "Property already has a set method");
							ParseError(Prop->SetFunc->Loc, "Previous set method here");
						}
						Prop->SetFunc = Func;
						Class->AddMethod(Func);
					}
					else if (Lex.Check(TK_Default))
					{
						if (Lex.Token != TK_Identifier)
						{
							ParseError(Lex.Location, "Default field name expected");
						}
						else
						{
							if (Prop->DefaultFieldName != NAME_None)
							{
								ParseError(Lex.Location, "Property already has default field defined");
							}
							Prop->DefaultFieldName = Lex.Name;
							Lex.NextToken();
						}
						Lex.Expect(TK_Semicolon, ERR_MISSING_SEMICOLON);
					}
					else
					{
						ParseError(Lex.Location, "Invalid declaration");
						Lex.NextToken();
					}
				}
				while (!Lex.Check(TK_RBrace));
				Class->AddProperty(Prop);
				need_semicolon = false;
				break;
			}

			if (Lex.Check(TK_LParen))
			{
				ParseMethodDef(FieldType, FieldName, FieldLoc, Class,
					Modifiers, false);
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
			fi->Flags = TModifiers::FieldAttr(TModifiers::Check(Modifiers,
				TModifiers::Native | TModifiers::Private |
				TModifiers::ReadOnly | TModifiers::Transient, FieldLoc));
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

	dprintf("Parsing\n");

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
				cDef->Type = TYPE_Int;
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
