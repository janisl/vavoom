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
//	EvalConstExpression
//
//==========================================================================

static int ConstExprFactor(void)
{
	int		num;
	int		ret = 0;

	switch(tk_Token)
	{
		case TK_INTEGER:
			ret = tk_Number;
			TK_NextToken();
			break;

		case TK_PUNCT:
			if (TK_Check(PU_LPAREN))
			{
				ret = EvalConstExpression(ev_int);
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
			num = CheckForConstant(tk_StringI);
			if (num != -1)
			{
				TK_NextToken();
			   	ret = Constants[num].value;
			}
			else
			{
				ERR_Exit(ERR_BAD_CONST_EXPR, true, "Invalid token %d %s", tk_Token, tk_String);
			}
			break;

		default:
			ERR_Exit(ERR_BAD_CONST_EXPR, true, "Invalid token %d %s", tk_Token, tk_String);
			break;
	}
	return ret;
}

// Operatori: * / %
static int CExprLevJ(void)
{
	boolean	unaryMinus;
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
static int CExprLevI(void)
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
static int CExprLevH(void)
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
static int CExprLevG(void)
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
static int CExprLevF(void)
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
static int CExprLevE(void)
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
static int CExprLevD(void)
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
static int CExprLevC(void)
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
static int CExprLevB(void)
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
static int CExprLevA(void)
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

static float FConstExprFactor(void)
{
	float		ret = 0.0;

	switch (tk_Token)
	{
		case TK_FLOAT:
			ret = tk_Float;
			TK_NextToken();
			break;
		case TK_PUNCT:
			if (TK_Check(PU_LPAREN))
			{
				ret = EvalConstExpression(ev_float);
				TK_Expect(PU_RPAREN, ERR_BAD_CONST_EXPR);
			}
			else
			{
				ERR_Exit(ERR_BAD_CONST_EXPR, true, "Invalid punct");
			}
			break;
		default:
			ERR_Exit(ERR_BAD_CONST_EXPR, true, "Invalid token %d %s", tk_Token, tk_String);
			break;
	}
	return ret;
}

// Operatori: * / %
static float FCExprLevJ(void)
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
static float FCExprLevI(void)
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

int EvalConstExpression(int type)
{
	int		ret;
	TType	*t;

 	switch (type)
	{
	 case ev_int:
	 case ev_uint:
		return CExprLevA();

	 case ev_float:
		return PassFloat(FCExprLevI());

	 case ev_string:
	 	if (tk_Token != TK_STRING)
		{
			ERR_Exit(ERR_BAD_CONST_EXPR, true, "String expected");
		}
		ret = tk_StringI;
		TK_NextToken();
		return ret;

	 case ev_function:
		if (TK_Check(KW_NULL))
		{
			return 0;
		}
		else if (tk_Token == TK_IDENTIFIER)
		{
			ret = CheckForFunction(tk_StringI);
			if (!ret)
			{
				ERR_Exit(ERR_NONE, true, "%s is not a function", tk_String);
			}
			TK_NextToken();
			return ret;
		}
		else
		{
			ERR_Exit(ERR_NONE, true, "Function name expected");
		}
		break;

	 case ev_classid:
		t = CheckForType();
		if (t && t->type == ev_class)
		{
			return t->classid;
		}
		else
		{
			ERR_Exit(ERR_NONE, true, "Bad classid");
		}

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

float ConstFloatExpression(void)
{
   	return FCExprLevI();
}

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2001/12/01 18:17:09  dj_jl
//	Fixed calling of parent method, speedup
//
//	Revision 1.5  2001/10/02 17:44:52  dj_jl
//	Some optimizations
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
