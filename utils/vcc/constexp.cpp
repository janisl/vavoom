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

static VClass*			ConstExprClass;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	EvalConstExpression
//
//==========================================================================

static int ConstExprFactor()
{
	VConstant*	Const;
	int			ret = 0;
	VName		Name;

	switch(tk_Token)
	{
	case TK_INTEGER:
		ret = tk_Number;
		TK_NextToken();
		break;

	case TK_PUNCT:
		if (TK_Check(PU_LPAREN))
		{
			ret = EvalConstExpression(ConstExprClass, ev_int);
			TK_Expect(PU_RPAREN, ERR_BAD_CONST_EXPR);
		}
		else if (TK_Check(PU_NOT))
		{
			ret = !ConstExprFactor();
		}
		else
		{
			ERR_Exit(ERR_BAD_CONST_EXPR, true, "Invalid punct");
		}
		break;

	case TK_IDENTIFIER:
		Name = tk_Name;
		TK_NextToken();
		if (TK_Check(PU_DCOLON))
		{
			VClass* Class = CheckForClass(Name);
			if (!Class)
			{
				ParseError("Class name expected");
				break;
			}

			if (tk_Token == TK_IDENTIFIER)
			{
				Const = CheckForConstant(Class, tk_Name);
				if (Const)
				{
					if (Const->Type != ev_int)
						ParseError(ERR_EXPR_TYPE_MISTMATCH);
					TK_NextToken();
					ret = Const->value;
					break;
				}
			}

			ParseError(ERR_ILLEGAL_EXPR_IDENT, "Identifier: %s", tk_String);
			break;
		}

		Const = CheckForConstant(ConstExprClass, Name);
		if (Const)
		{
			if (Const->Type != ev_int)
				ParseError(ERR_EXPR_TYPE_MISTMATCH);
			ret = Const->value;
		}
		else
		{
			ERR_Exit(ERR_BAD_CONST_EXPR, true, "Invalid identifier %s", *Name);
		}
		break;

	case TK_KEYWORD:
		if (TK_Check(KW_TRUE))
		{
			ret = true;
			break;
		}
		if (TK_Check(KW_FALSE))
		{
			ret = false;
			break;
		}
		ERR_Exit(ERR_BAD_CONST_EXPR, true, "Invalid token %d %s", tk_Token, tk_String);
		break;

	default:
		ERR_Exit(ERR_BAD_CONST_EXPR, true, "Invalid token %d %s", tk_Token, tk_String);
		break;
	}
	return ret;
}

// Operatori: * / %
static int CExprLevJ()
{
	bool	unaryMinus;
	int		ret;

	unaryMinus = false;
	if (TK_Check(PU_MINUS))
	{
		unaryMinus = true;
	}
	ret = ConstExprFactor();
	if (unaryMinus)
	{
		ret = -ret;
	}
	do
	{
		if (TK_Check(PU_ASTERISK))
		{
			ret *= ConstExprFactor();
		}
		else if (TK_Check(PU_SLASH))
		{
			ret /= ConstExprFactor();
		}
		else if (TK_Check(PU_PERCENT))
		{
			ret %= ConstExprFactor();
		}
		else
		{
			return ret;
		}
	} while (1);
}

// Operatori: + -
static int CExprLevI()
{
	int		ret;

	ret = CExprLevJ();
	do
	{
		if (TK_Check(PU_PLUS))
		{
			ret += CExprLevJ();
		}
		else if (TK_Check(PU_MINUS))
		{
			ret -= CExprLevJ();
		}
		else
		{
			return ret;
		}
	} while (1);
}

// Operatori: << >>
static int CExprLevH()
{
	int		ret;

	ret = CExprLevI();
	do
	{
		if (TK_Check(PU_LSHIFT))
		{
			ret <<= CExprLevI();
		}
		else if (TK_Check(PU_RSHIFT))
		{
			ret >>= CExprLevI();
		}
		else
		{
			return ret;
		}
	} while (1);
}

// Operatori: < <= > >=
static int CExprLevG()
{
	int		ret;

	ret = CExprLevH();
	do
	{
		if (TK_Check(PU_LT))
		{
			ret = ret < CExprLevH();
		}
		else if (TK_Check(PU_LE))
		{
			ret = ret <= CExprLevH();
		}
		else if (TK_Check(PU_GT))
		{
			ret = ret > CExprLevH();
		}
		else if (TK_Check(PU_GE))
		{
			ret = ret >= CExprLevH();
		}
		else
		{
			return ret;
		}
	} while (1);
}

// Operatori: == !=
static int CExprLevF()
{
	int		ret;

	ret = CExprLevG();
	do
	{
		if (TK_Check(PU_EQ))
		{
			ret = ret == CExprLevG();
		}
		else if (TK_Check(PU_NE))
		{
			ret = ret != CExprLevG();
		}
		else
		{
			return ret;
		}
	} while (1);
}

// Operators: &
static int CExprLevE()
{
	int		ret;

	ret = CExprLevF();
	while (TK_Check(PU_AND))
	{
		ret &= CExprLevF();
	}
	return ret;
}

// Operators: ^
static int CExprLevD()
{
	int		ret;

	ret = CExprLevE();
	while (TK_Check(PU_XOR))
	{
		ret ^= CExprLevE();
	}
	return ret;
}

// Operators: |
static int CExprLevC()
{
	int		ret;

	ret = CExprLevD();
	while (TK_Check(PU_OR))
	{
		ret |= CExprLevD();
	}
	return ret;
}

// Operators: &&
static int CExprLevB()
{
	int		ret;

	ret = CExprLevC();
	while (TK_Check(PU_AND_LOG))
	{
		ret = ret && CExprLevC();
	}
	return ret;
}

// Operators: ||
static int CExprLevA()
{
	int		ret;

	ret = CExprLevB();
	while (TK_Check(PU_OR_LOG))
	{
		ret = ret || CExprLevB();
	}
	return ret;
}

//**************************************************************************
//
//	Floating point constant
//
//**************************************************************************

static float FConstExprFactor()
{
	VConstant*	Const;
	float		ret = 0.0;
	VName		Name;

	switch (tk_Token)
	{
	case TK_FLOAT:
		ret = tk_Float;
		TK_NextToken();
		break;
	case TK_PUNCT:
		if (TK_Check(PU_LPAREN))
		{
			ret = ConstFloatExpression();
			TK_Expect(PU_RPAREN, ERR_BAD_CONST_EXPR);
		}
		else
		{
			ERR_Exit(ERR_BAD_CONST_EXPR, true, "Invalid punct");
		}
		break;
	case TK_IDENTIFIER:
		Name = tk_Name;
		TK_NextToken();
		if (TK_Check(PU_DCOLON))
		{
			VClass* Class = CheckForClass(Name);
			if (!Class)
			{
				ParseError("Class name expected");
				break;
			}

			if (tk_Token == TK_IDENTIFIER)
			{
				Const = CheckForConstant(Class, tk_Name);
				if (Const)
				{
					if (Const->Type != ev_float)
						ParseError(ERR_EXPR_TYPE_MISTMATCH);
					TK_NextToken();
					ret = Const->value;
					break;
				}
			}

			ParseError(ERR_ILLEGAL_EXPR_IDENT, "Identifier: %s", tk_String);
			break;
		}

		Const = CheckForConstant(ConstExprClass, Name);
		if (Const)
		{
			if (Const->Type != ev_float)
				ParseError(ERR_EXPR_TYPE_MISTMATCH);
			ret = Const->value;
		}
		else
		{
			ERR_Exit(ERR_BAD_CONST_EXPR, true, "Invalid identifier %s", *Name);
		}
		break;
	default:
		ERR_Exit(ERR_BAD_CONST_EXPR, true, "Invalid token %d %s", tk_Token, tk_String);
		break;
	}
	return ret;
}

// Operatori: * / %
static float FCExprLevJ()
{
	bool	unaryMinus;
	float	ret;

	unaryMinus = false;
	if (TK_Check(PU_MINUS))
	{
		unaryMinus = true;
	}
	ret = FConstExprFactor();
	if (unaryMinus)
	{
		ret = -ret;
	}
	do
	{
		if (TK_Check(PU_ASTERISK))
		{
			ret *= FConstExprFactor();
		}
		else if (TK_Check(PU_SLASH))
		{
			ret /= FConstExprFactor();
		}
		else
		{
			return ret;
		}
	} while (1);
}

// Operatori: + -
static float FCExprLevI()
{
	float		ret;

	ret = FCExprLevJ();
	do
	{
		if (TK_Check(PU_PLUS))
		{
			ret += FCExprLevJ();
		}
		else if (TK_Check(PU_MINUS))
		{
			ret -= FCExprLevJ();
		}
		else
		{
			return ret;
		}
	} while (1);
}

//=====================

int EvalConstExpression(VClass*InClass, int type)
{
	int		ret;
	VClass*	c;

	ConstExprClass = InClass;
	switch (type)
	{
	 case ev_int:
		return CExprLevA();

	 case ev_float:
		return PassFloat(FCExprLevI());

	 case ev_name:
	 	if (tk_Token != TK_NAME)
		{
			ERR_Exit(ERR_BAD_CONST_EXPR, true, "Name expected");
		}
		ret = tk_Name.GetIndex();
		TK_NextToken();
		return ret;

	case ev_classid:
		c = CheckForClass();
		if (c)
		{
			return c->MemberIndex;
		}
		else
		{
			ERR_Exit(ERR_NONE, true, "Bad classid");
		}

	 case ev_bool:
		return !!CExprLevA();

	default:
		ERR_Exit(ERR_NONE, true, "Constant value of this variable type cannot be defined.");
	}

	return 0;
}

//==========================================================================
//
//  ConstFloatExpression
//
//==========================================================================

float ConstFloatExpression()
{
   	return FCExprLevI();
}
