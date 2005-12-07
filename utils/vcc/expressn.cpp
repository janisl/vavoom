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

#define MAX_ARG_COUNT		16

// TYPES -------------------------------------------------------------------

class TTree
{
public:
	TTree()
	: type(NULL)
	, RealType(NULL)
	{}
	explicit TTree(TType *InType)
	: type(InType)
	, RealType(NULL)
	{}
	explicit TTree(TType *InType, TType* InRealType)
	: type(InType)
	, RealType(InRealType)
	{}

	TType *type;
	TType *RealType;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static TTree ParseExpressionPriority2();
static TTree ParseExpressionPriority13();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static bool			CheckForLocal;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	GetAddress
//
//==========================================================================

static TTree EmitPushPointed(TType* type)
{
	if (type->type == ev_vector)
	{
		AddStatement(OPC_VPushPointed);
	}
	else if (type->type == ev_bool)
	{
		AddStatement(OPC_PushBool, type->bit_mask);
	}
	else
	{
		AddStatement(OPC_PushPointed);
	}

	TTree op = TTree(type, type);
	if (type->type == ev_bool)
	{
		op.type = &type_int;
	}
	return op;
}

//==========================================================================
//
//	GetAddress
//
//==========================================================================

static TTree GetAddress(TTree op)
{
	if (!op.RealType)
	{
		ParseError("Bad address operation");
		return op;
	}
	int Opc = UndoStatement();
	if (Opc != OPC_VPushPointed && Opc != OPC_PushBool && Opc != OPC_PushPointed)
	{
		ParseError("Bad address operation");
		return op;
	}
	op.type = MakePointerType(op.RealType);
	op.RealType = NULL;
	return op;
}

//==========================================================================
//
//	ParseFunctionCall
//
//==========================================================================

static TTree ParseFunctionCall(int num, bool is_method)
{
	int arg = 0;
	int argsize = 0;
	int max_params;
	int num_needed_params = functions[num].NumParams & PF_COUNT_MASK;
	if (functions[num].NumParams & PF_VARARGS)
	{
		max_params = MAX_ARG_COUNT - 1;
	}
	else
	{
		max_params = functions[num].NumParams;
	}
	if (is_method)
	{
		AddStatement(OPC_LocalAddress, 0);
		AddStatement(OPC_PushPointed);
	}
	if (!TK_Check(PU_RPAREN))
	{
		do
		{
			TTree op = ParseExpressionPriority13();
			if (arg >= max_params)
			{
				ParseError("Incorrect number of arguments, need %d, got %d.", max_params, arg + 1);
			}
			else
			{
				if (arg < num_needed_params)
				{
					TypeCheck3(op.type, functions[num].ParamTypes[arg]);
				}
			}
			arg++;
			argsize += TypeSize(op.type);
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
	if (arg < num_needed_params)
	{
		ParseError("Incorrect argument count %d, should be %d", arg, num_needed_params);
	}
	if (functions[num].NumParams & PF_VARARGS)
	{
		AddStatement(OPC_PushNumber, argsize / 4 - num_needed_params);
	}
	AddStatement(OPC_Call, num);
	TTree fop = TTree(functions[num].ReturnType);
	if (fop.type->type == ev_bool)
		fop.type = &type_int;
	return fop;
}

//==========================================================================
//
//	ParseMethodCall
//
//==========================================================================

static TTree ParseMethodCall(field_t* field)
{
	int num = field->func_num;
	int arg = 0;
	int argsize = 0;
	int max_params;
	int num_needed_params = functions[num].NumParams & PF_COUNT_MASK;
	if (functions[num].NumParams & PF_VARARGS)
	{
		max_params = MAX_ARG_COUNT - 1;
	}
	else
	{
		max_params = functions[num].NumParams;
	}
	AddStatement(OPC_PushVFunc, field->ofs);
	if (!TK_Check(PU_RPAREN))
	{
		do
		{
			TTree op = ParseExpressionPriority13();
			if (arg >= max_params)
			{
				ParseError("Incorrect number of arguments, need %d, got %d.", max_params, arg + 1);
			}
			else
			{
				if (arg < num_needed_params)
				{
					TypeCheck3(op.type, functions[num].ParamTypes[arg]);
				}
				if (op.type->type == ev_vector)
					AddStatement(OPC_Swap3);
				else
					AddStatement(OPC_Swap);
			}
			arg++;
			argsize += TypeSize(op.type);
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
	if (arg < num_needed_params)
	{
		ParseError("Incorrect argument count %d, should be %d", arg, num_needed_params);
	}
	if (functions[num].NumParams & PF_VARARGS)
	{
		AddStatement(OPC_PushNumber, argsize / 4 - num_needed_params);
		AddStatement(OPC_Swap);
	}
	AddStatement(OPC_ICall);
	TTree fop = TTree(functions[num].ReturnType);
	if (fop.type->type == ev_bool)
		fop.type = &type_int;
	return fop;
}

//==========================================================================
//
//	ParseExpressionPriority0
//
//==========================================================================

static TTree ParseExpressionPriority0()
{
	TTree		op;
	TType*		type;
	int			num;
	field_t*	field;
	FName		Name;
	bool		bLocals;

	bLocals = CheckForLocal;
	CheckForLocal = false;
   	switch (tk_Token)
	{
	case TK_INTEGER:
		AddStatement(OPC_PushNumber, tk_Number);
		TK_NextToken();
		return TTree(&type_int);

	case TK_FLOAT:
		AddStatement(OPC_PushNumber, PassFloat(tk_Float));
		TK_NextToken();
		return TTree(&type_float);

	case TK_NAME:
		AddStatement(OPC_PushName, tk_Name.GetIndex());
		TK_NextToken();
		return TTree(&type_name);

	case TK_STRING:
		AddStatement(OPC_PushString, tk_StringI);
		TK_NextToken();
		return TTree(&type_string);

	case TK_PUNCT:
		if (TK_Check(PU_LPAREN))
		{
			op = ParseExpressionPriority13();
			TK_Expect(PU_RPAREN, ERR_BAD_EXPR);
			return op;
		}

		if (TK_Check(PU_DCOLON))
		{
			if (!SelfClass)
			{
				ParseError(":: not in method");
				break;
			}
			field = CheckForField(SelfClass->ParentClass);
			if (!field)
			{
				ParseError("No such method %s", *tk_Name);
				break;
			}
			if (field->type->type != ev_method)
			{
				ParseError("Not a method");
				break;
			}
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			return ParseFunctionCall(field->func_num, true);
		}
		break;

	case TK_KEYWORD:
		if (TK_Check(KW_VECTOR))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			TTree op1 = ParseExpressionPriority13();
			if (op1.type != &type_float)
				ParseError("Expression type mistmatch, vector param 1 is not a float");
			TK_Expect(PU_COMMA, ERR_BAD_EXPR);
			TTree op2 = ParseExpressionPriority13();
			if (op2.type != &type_float)
				ParseError("Expression type mistmatch, vector param 2 is not a float");
			TK_Expect(PU_COMMA, ERR_BAD_EXPR);
			TTree op3 = ParseExpressionPriority13();
			if (op3.type != &type_float)
				ParseError("Expression type mistmatch, vector param 3 is not a float");
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			return TTree(&type_vector);
		}
		if (TK_Check(KW_SELF))
		{
			if (!SelfClass)
			{
				ParseError("self used outside member function\n");
			}
			else
			{
				AddStatement(OPC_LocalAddress, 0);
				AddStatement(OPC_PushPointed);
				op = TTree(SelfType);
				return op;
			}
		}
		if (TK_Check(KW_NONE))
		{
			AddStatement(OPC_PushNumber, 0);
			op = TTree(&type_none_ref);
			return op;
		}
		if (TK_Check(KW_NULL))
		{
			AddStatement(OPC_PushNumber, 0);
			op = TTree(&type_void_ptr);
			return op;
		}
		if (bLocals)
		{
			type = CheckForType();
			if (type)
			{
				ParseLocalVar(type);
				return TTree();
			}
		}
		break;

	case TK_IDENTIFIER:
		Name = tk_Name;
		TK_NextToken();
		if (TK_Check(PU_LPAREN))
		{
			TClass* Class = CheckForClass(Name);
			if (Class)
			{
				op = ParseExpressionPriority13();
				if (op.type->type != ev_reference)
				{
					ParseError(ERR_BAD_EXPR, "Class reference required");
				}
				TK_Expect(PU_RPAREN, ERR_BAD_EXPR);
				AddStatement(OPC_DynamicCast, Class->Index);
				op = TTree(MakeReferenceType(Class));
				return op;
			}

			num = CheckForFunction(NULL, Name);
			if (num)
			{
				return ParseFunctionCall(num, false);
			}

			if (SelfClass)
			{
				field = CheckForField(Name, SelfClass);
				if (field && field->type->type == ev_method)
				{
					AddStatement(OPC_LocalAddress, 0);
					AddStatement(OPC_PushPointed);
					return ParseMethodCall(field);
				}
			}

			ParseError(ERR_ILLEGAL_EXPR_IDENT, "Identifier: %s", *Name);
			break;
		}
		if (bLocals && (tk_Token == TK_IDENTIFIER ||
			(tk_Token == TK_PUNCT && tk_Punct == PU_ASTERISK)))
		{
			type = CheckForType(Name);
			if (type)
			{
				ParseLocalVar(type);
				return TTree();
			}
		}

		num = CheckForLocalVar(Name);
		if (num)
		{
			AddStatement(OPC_LocalAddress, localdefs[num].ofs);
			op = EmitPushPointed(localdefs[num].type);
			return op;
		}

		num = CheckForConstant(Name);
		if (num != -1)
		{
			AddStatement(OPC_PushNumber, Constants[num].value);
			op = TTree(&type_int);
			return op;
		}

		num = CheckForGlobalVar(Name);
		if (num)
		{
			AddStatement(OPC_GlobalAddress, num);
			op = EmitPushPointed(globaldefs[num].type);
			return op;
		}

		if (SelfClass)
		{
			field = CheckForField(Name, SelfClass);
			if (field)
			{
				AddStatement(OPC_LocalAddress, 0);
				AddStatement(OPC_PushPointed);
				op = TTree(SelfType);
				if (field->type->type == ev_method)
				{
					ParseError("Method call expected");
				}
				else
				{
					AddStatement(OPC_PushNumber, field->ofs);
					AddStatement(OPC_Add);
					op = EmitPushPointed(field->type);
				}
				return op;
			}
		}

		TClass* Class = CheckForClass(Name);
		if (Class)
		{
			AddStatement(OPC_PushClassId, Class->Index);
			op = TTree(&type_classid);
			return op;
		}

		ERR_Exit(ERR_ILLEGAL_EXPR_IDENT, true, "Identifier: %s", *Name);
		break;

	default:
		break;
	}

	op = TTree(&type_void);
	return op;
}

//==========================================================================
//
//	ParseExpressionPriority1
//
//==========================================================================

static TTree ParseExpressionPriority1()
{
	bool		done;
	TTree		op;
	TType*		type;
	field_t*	field;

	op = ParseExpressionPriority0();
	done = false;
	do
	{
		if (TK_Check(PU_MINUS_GT))
		{
			TypeCheck1(op.type);
			if (op.type->type != ev_pointer)
			{
				ERR_Exit(ERR_BAD_EXPR, true, NULL);
			}
			type = op.type->aux_type;
			field = ParseStructField(type->Struct);
			if (field)
			{
				if (field->type->type == ev_method)
				{
					ParseError("Pointed method");
				}
				else
				{
					AddStatement(OPC_PushNumber, field->ofs);
					AddStatement(OPC_Add);
					op = EmitPushPointed(field->type);
				}
			}
		}
		else if (TK_Check(PU_DOT))
		{
			if (op.type->type == ev_reference)
			{
				field = ParseClassField(op.type->Class);
				if (field)
				{
					if (field->type->type == ev_method)
					{
						TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
						op = ParseMethodCall(field);
					}
					else
					{
						AddStatement(OPC_PushNumber, field->ofs);
						AddStatement(OPC_Add);
						op = EmitPushPointed(field->type);
					}
				}
			}
			else if (op.type->type == ev_struct || op.type->type == ev_vector)
			{
				type = op.type;
				op = GetAddress(op);
				field = ParseStructField(type->Struct);
				if (field)
				{
					AddStatement(OPC_PushNumber, field->ofs);
					AddStatement(OPC_Add);
					op = EmitPushPointed(field->type);
				}
			}
			else
			{
				ParseError(ERR_BAD_EXPR);
			}
		}
		else if (TK_Check(PU_LINDEX))
		{
			if (op.type->type == ev_array)
			{
				type = op.type->aux_type;
				op = GetAddress(op);
			}
			else if (op.type->type == ev_pointer)
			{
			   	type = op.type->aux_type;
			}
			else
			{
				ERR_Exit(ERR_BAD_ARRAY, true, NULL);
			}
			TTree ind = ParseExpressionPriority13();
			if (ind.type != &type_int)
			{
				ParseError(ERR_EXPR_TYPE_MISTMATCH);
			}
			TK_Expect(PU_RINDEX, ERR_BAD_ARRAY);
			AddStatement(OPC_PushNumber, TypeSize(type));
			AddStatement(OPC_Multiply);
			AddStatement(OPC_Add);
			op = EmitPushPointed(type);
		}
		else if (TK_Check(PU_LPAREN))
		{
			ParseError("Not a method");
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

static TTree ParseExpressionPriority2()
{
	TTree		op;
	TType*		type;

	if (tk_Token == TK_PUNCT)
	{
		if (TK_Check(PU_PLUS))
		{
			op = ParseExpressionPriority2();
			if (op.type != &type_int && op.type != &type_float)
			{
				ParseError("Expression type mistmatch");
			}
			return op;
		}

		if (TK_Check(PU_MINUS))
		{
			op = ParseExpressionPriority2();
			if (op.type == &type_int)
			{
				AddStatement(OPC_UnaryMinus);
				op = TTree(&type_int);
			}
			else if (op.type == &type_float)
			{
				AddStatement(OPC_FUnaryMinus);
				op = TTree(&type_float);
			}
			else if (op.type->type == ev_vector)
			{
				AddStatement(OPC_VUnaryMinus);
				op = TTree(op.type);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
			return op;
		}

		if (TK_Check(PU_NOT))
		{
			op = ParseExpressionPriority2();
			if (op.type == &type_int)
			{
				AddStatement(OPC_NegateLogical);
			}
			else if (op.type == &type_float)
			{
				AddStatement(OPC_NegateLogical);
			}
			else if (op.type == &type_name)
			{
				AddStatement(OPC_NegateLogical);
			}
			else if (op.type == &type_string)
			{
				AddStatement(OPC_NegateLogical);
			}
			else if (op.type->type == ev_pointer)
			{
				AddStatement(OPC_NegateLogical);
			}
			else if (op.type->type == ev_reference)
			{
				AddStatement(OPC_NegateLogical);
			}
			else if (op.type == &type_classid)
			{
				AddStatement(OPC_NegateLogical);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
			op = TTree(&type_int);
			return op;
		}

		if (TK_Check(PU_TILDE))
		{
			op = ParseExpressionPriority2();
			if (op.type == &type_int)
			{
				AddStatement(OPC_BitInverse);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
			op = TTree(&type_int);
			return op;
		}

		if (TK_Check(PU_AND))
		{
			op = ParseExpressionPriority1();
			if (op.type->type == ev_reference)
			{
				ParseError("Tried to take address of reference");
			}
			else
			{
				type = MakePointerType(op.type);
				op = GetAddress(op);
			}
			return op;
		}
		if (TK_Check(PU_ASTERISK))
		{
			op = ParseExpressionPriority2();
			if (op.type->type != ev_pointer)
			{
				ParseError("Expression syntax error");
				return op;
			}
			type = op.type->aux_type;
			return EmitPushPointed(type);
		}

		if (TK_Check(PU_INC))
		{
			op = ParseExpressionPriority2();
			type = op.type;
			op = GetAddress(op);
			if (type == &type_int)
			{
				AddStatement(OPC_PreInc);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
			op = TTree(&type_int);
			return op;
		}

		if (TK_Check(PU_DEC))
		{
			op = ParseExpressionPriority2();
			type = op.type;
			op = GetAddress(op);
			if (type == &type_int)
			{
				AddStatement(OPC_PreDec);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
			op = TTree(&type_int);
			return op;
		}
	}

	op = ParseExpressionPriority1();

	if (TK_Check(PU_INC))
	{
		type = op.type;
		op = GetAddress(op);
		if (type == &type_int)
		{
			AddStatement(OPC_PostInc);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		op = TTree(&type_int);
		return op;
	}

	if (TK_Check(PU_DEC))
	{
		type = op.type;
		op = GetAddress(op);
		if (type == &type_int)
		{
			AddStatement(OPC_PostDec);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		op = TTree(&type_int);
		return op;
	}

	return op;
}

//==========================================================================
//
//	ParseExpressionPriority3
//
//==========================================================================

static TTree ParseExpressionPriority3()
{
	TTree op1 = ParseExpressionPriority2();
	bool done = false;
	do
	{
		if (TK_Check(PU_ASTERISK))
		{
			TTree op2 = ParseExpressionPriority2();
			if (op1.type == &type_int && op2.type == &type_int)
			{
				AddStatement(OPC_Multiply);
				op1 = TTree(&type_int);
			}
			else if (op1.type == &type_float && op2.type == &type_float)
			{
				AddStatement(OPC_FMultiply);
				op1 = TTree(&type_float);
			}
			else if (op1.type->type == ev_vector && op2.type == &type_float)
			{
				AddStatement(OPC_VPostScale);
				op1 = TTree(&type_vector);
			}
			else if (op1.type == &type_float && op2.type->type == ev_vector)
			{
				AddStatement(OPC_VPreScale);
				op1 = TTree(&type_vector);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
		}
		else if (TK_Check(PU_SLASH))
		{
			TTree op2 = ParseExpressionPriority2();
			if (op1.type == &type_int && op2.type == &type_int)
			{
				AddStatement(OPC_Divide);
				op1 = TTree(&type_int);
			}
			else if (op1.type == &type_float && op2.type == &type_float)
			{
				AddStatement(OPC_FDivide);
				op1 = TTree(&type_float);
			}
			else if (op1.type->type == ev_vector && op2.type == &type_float)
			{
				AddStatement(OPC_VIScale);
				op1 = TTree(&type_vector);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
		}
		else if (TK_Check(PU_PERCENT))
		{
			TTree op2 = ParseExpressionPriority2();
			if (op1.type == &type_int && op2.type == &type_int)
			{
				AddStatement(OPC_Modulus);
				op1 = TTree(&type_int);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
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

static TTree ParseExpressionPriority4()
{
	TTree op1 = ParseExpressionPriority3();
	bool done = false;
	do
	{
		if (TK_Check(PU_PLUS))
		{
			TTree op2 = ParseExpressionPriority3();
			if (op1.type == &type_int && op2.type == &type_int)
			{
				AddStatement(OPC_Add);
				op1 = TTree(&type_int);
			}
			else if (op1.type == &type_float && op2.type == &type_float)
			{
				AddStatement(OPC_FAdd);
				op1 = TTree(&type_float);
			}
			else if (op1.type->type == ev_vector && op2.type->type == ev_vector)
			{
				AddStatement(OPC_VAdd);
				op1 = TTree(&type_vector);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
		}
		else if (TK_Check(PU_MINUS))
		{
			TTree op2 = ParseExpressionPriority3();
			if (op1.type == &type_int && op2.type == &type_int)
			{
				AddStatement(OPC_Subtract);
				op1 = TTree(&type_int);
			}
			else if (op1.type == &type_float && op2.type == &type_float)
			{
				AddStatement(OPC_FSubtract);
				op1 = TTree(&type_float);
			}
			else if (op1.type->type == ev_vector && op2.type->type == ev_vector)
			{
				AddStatement(OPC_VSubtract);
				op1 = TTree(&type_vector);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
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

static TTree ParseExpressionPriority5()
{
	TTree op1 = ParseExpressionPriority4();
	bool done = false;
	do
	{
		if (TK_Check(PU_LSHIFT))
		{
			TTree op2 = ParseExpressionPriority4();
			if (op1.type == &type_int && op2.type == &type_int)
			{
				AddStatement(OPC_LShift);
				op1 = TTree(&type_int);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
		}
		else if (TK_Check(PU_RSHIFT))
		{
			TTree op2 = ParseExpressionPriority4();
			if (op1.type == &type_int && op2.type == &type_int)
			{
				AddStatement(OPC_RShift);
				op1 = TTree(&type_int);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
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

static TTree ParseExpressionPriority6()
{
	TTree op1 = ParseExpressionPriority5();
	bool done = false;
	do
	{
		if (TK_Check(PU_LT))
		{
			TTree op2 = ParseExpressionPriority5();
			if (op1.type == &type_int && op2.type == &type_int)
			{
				AddStatement(OPC_Less);
				op1 = TTree(&type_int);
			}
			else if (op1.type == &type_float && op2.type == &type_float)
			{
				AddStatement(OPC_FLess);
				op1 = TTree(&type_int);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
		}
		else if (TK_Check(PU_LE))
		{
			TTree op2 = ParseExpressionPriority5();
			if (op1.type == &type_int && op2.type == &type_int)
			{
				AddStatement(OPC_LessEquals);
				op1 = TTree(&type_int);
			}
			else if (op1.type == &type_float && op2.type == &type_float)
			{
				AddStatement(OPC_FLessEquals);
				op1 = TTree(&type_int);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
		}
		else if (TK_Check(PU_GT))
		{
			TTree op2 = ParseExpressionPriority5();
			if (op1.type == &type_int && op2.type == &type_int)
			{
				AddStatement(OPC_Greater);
				op1 = TTree(&type_int);
			}
			else if (op1.type == &type_float && op2.type == &type_float)
			{
				AddStatement(OPC_FGreater);
				op1 = TTree(&type_int);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
		}
		else if (TK_Check(PU_GE))
		{
			TTree op2 = ParseExpressionPriority5();
			if (op1.type == &type_int && op2.type == &type_int)
			{
				AddStatement(OPC_GreaterEquals);
				op1 = TTree(&type_int);
			}
			else if (op1.type == &type_float && op2.type == &type_float)
			{
				AddStatement(OPC_FGreaterEquals);
				op1 = TTree(&type_int);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
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

static TTree ParseExpressionPriority7()
{
	TTree op1 = ParseExpressionPriority6();
	bool done = false;
	do
	{
		if (TK_Check(PU_EQ))
		{
			TTree op2 = ParseExpressionPriority6();
			if (op1.type == &type_int && op2.type == &type_int)
			{
				AddStatement(OPC_Equals);
			}
			else if (op1.type == &type_float && op2.type == &type_float)
			{
				AddStatement(OPC_FEquals);
			}
			else if (op1.type == &type_name && op2.type == &type_name)
			{
				AddStatement(OPC_Equals);
			}
			else if (op1.type == &type_string && op2.type == &type_string)
			{
				AddStatement(OPC_Equals);
			}
			else if (op1.type->type == ev_pointer && op2.type->type == ev_pointer)
			{
				AddStatement(OPC_Equals);
			}
			else if (op1.type->type == ev_vector && op2.type->type == ev_vector)
			{
				AddStatement(OPC_VEquals);
			}
			else if (op1.type == &type_classid && op2.type == &type_classid)
			{
				AddStatement(OPC_Equals);
			}
			else if (op1.type->type == ev_reference && op2.type->type == ev_reference)
			{
				AddStatement(OPC_Equals);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
			op1 = TTree(&type_int);
		}
		else if (TK_Check(PU_NE))
		{
			TTree op2 = ParseExpressionPriority6();
			if (op1.type == &type_int && op2.type == &type_int)
			{
				AddStatement(OPC_NotEquals);
			}
			else if (op1.type == &type_float && op2.type == &type_float)
			{
				AddStatement(OPC_FNotEquals);
			}
			else if (op1.type == &type_name && op2.type == &type_name)
			{
				AddStatement(OPC_NotEquals);
			}
			else if (op1.type == &type_string && op2.type == &type_string)
			{
				AddStatement(OPC_NotEquals);
			}
			else if (op1.type->type == ev_pointer && op2.type->type == ev_pointer)
			{
				AddStatement(OPC_NotEquals);
			}
			else if (op1.type->type == ev_vector && op2.type->type == ev_vector)
			{
				AddStatement(OPC_VNotEquals);
			}
			else if (op1.type == &type_classid && op2.type == &type_classid)
			{
				AddStatement(OPC_NotEquals);
			}
			else if (op1.type->type == ev_reference && op2.type->type == ev_reference)
			{
				AddStatement(OPC_NotEquals);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
			op1 = TTree(&type_int);
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

static TTree ParseExpressionPriority8()
{
	TTree op1 = ParseExpressionPriority7();
	while (TK_Check(PU_AND))
	{
		TTree op2 = ParseExpressionPriority7();
		if (op1.type == &type_int && op2.type == &type_int)
		{
			AddStatement(OPC_AndBitwise);
			op1 = TTree(&type_int);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority9
//
//==========================================================================

static TTree ParseExpressionPriority9()
{
	TTree op1 = ParseExpressionPriority8();
	while (TK_Check(PU_XOR))
	{
		TTree op2 = ParseExpressionPriority8();
		if (op1.type == &type_int && op2.type == &type_int)
		{
			AddStatement(OPC_XOrBitwise);
			op1 = TTree(&type_int);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority10
//
//==========================================================================

static TTree ParseExpressionPriority10()
{
	TTree op1 = ParseExpressionPriority9();
	while (TK_Check(PU_OR))
	{
		TTree op2 = ParseExpressionPriority9();
		if (op1.type == &type_int && op2.type == &type_int)
		{
			AddStatement(OPC_OrBitwise);
			op1 = TTree(&type_int);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority11
//
//==========================================================================

static TTree ParseExpressionPriority11()
{
	TTree op1 = ParseExpressionPriority10();
	while (TK_Check(PU_AND_LOG))
	{
		TypeCheck1(op1.type);
		int jmppos = AddStatement(OPC_IfNotTopGoto, 0);
		TTree op2 = ParseExpressionPriority10();
		TypeCheck1(op2.type);
		AddStatement(OPC_AndLogical);
		FixupJump(jmppos);
		op1 = TTree(&type_int);
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority12
//
//==========================================================================

static TTree ParseExpressionPriority12()
{
	TTree op1 = ParseExpressionPriority11();
	while (TK_Check(PU_OR_LOG))
	{
		TypeCheck1(op1.type);
		int jmppos = AddStatement(OPC_IfTopGoto, 0);
		TTree op2 = ParseExpressionPriority11();
		TypeCheck1(op2.type);
		AddStatement(OPC_OrLogical);
		FixupJump(jmppos);
		op1 = TTree(&type_int);
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority13
//
//==========================================================================

static TTree ParseExpressionPriority13()
{
	TTree			op;
	TTree			op1;
	TTree			op2;

	op = ParseExpressionPriority12();
   	if (TK_Check(PU_QUEST))
	{
		TypeCheck1(op.type);
		int jumppos1 = AddStatement(OPC_IfNotGoto, 0);
		op1 = ParseExpressionPriority13();
		TK_Expect(PU_COLON, ERR_MISSING_COLON);
		int jumppos2 = AddStatement(OPC_Goto, 0);
		FixupJump(jumppos1);
		op2 = ParseExpressionPriority13();
		FixupJump(jumppos2);
		TypeCheck3(op1.type, op2.type);
		if (op1.type == &type_void_ptr)
			op = TTree(op2.type);
		else
			op = TTree(op1.type);
	}
	return op;
}

//==========================================================================
//
//	ParseExpressionPriority14
//
//==========================================================================

static TTree ParseExpressionPriority14()
{
	TTree op1 = ParseExpressionPriority13();
	if (TK_Check(PU_ASSIGN))
	{
		TType* type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		TypeCheck3(op2.type, type);
		if (type == &type_int && op2.type == &type_int)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type == &type_float && op2.type == &type_float)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type == &type_name && op2.type == &type_name)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type == &type_string && op2.type == &type_string)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type->type == ev_pointer && op2.type->type == ev_pointer)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type->type == ev_vector && op2.type->type == ev_vector)
		{
			AddStatement(OPC_VAssignDrop);
		}
		else if (type == &type_classid && op2.type == &type_classid)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type->type == ev_reference && op2.type->type == ev_reference)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type->type == ev_bool && op2.type == &type_int)
		{
			AddStatement(OPC_AssignBool, type->bit_mask);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(&type_void);
	}
	else if (TK_Check(PU_ADD_ASSIGN))
	{
		TType* type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		TypeCheck3(op2.type, type);
		if (type == &type_int && op2.type == &type_int)
		{
			AddStatement(OPC_AddVarDrop);
		}
		else if (type == &type_float && op2.type == &type_float)
		{
			AddStatement(OPC_FAddVarDrop);
		}
		else if (type->type == ev_vector && op2.type->type == ev_vector)
		{
			AddStatement(OPC_VAddVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		op1 = TTree(&type_void);
		return op1;
	}
	else if (TK_Check(PU_MINUS_ASSIGN))
	{
		TType* type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		TypeCheck3(op2.type, type);
		if (type == &type_int && op2.type == &type_int)
		{
			AddStatement(OPC_SubVarDrop);
		}
		else if (type == &type_float && op2.type == &type_float)
		{
			AddStatement(OPC_FSubVarDrop);
		}
		else if (type->type == ev_vector && op2.type->type == ev_vector)
		{
			AddStatement(OPC_VSubVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(&type_void);
	}
	else if (TK_Check(PU_MULTIPLY_ASSIGN))
	{
		TType* type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		TypeCheck3(op2.type, type);
		if (type == &type_int && op2.type == &type_int)
		{
			AddStatement(OPC_MulVarDrop);
		}
		else if (type == &type_float && op2.type == &type_float)
		{
			AddStatement(OPC_FMulVarDrop);
		}
		else if (type->type == ev_vector && op2.type == &type_float)
		{
			AddStatement(OPC_VScaleVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(&type_void);
	}
	else if (TK_Check(PU_DIVIDE_ASSIGN))
	{
		TType* type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		TypeCheck3(op2.type, type);
		if (type == &type_int && op2.type == &type_int)
		{
			AddStatement(OPC_DivVarDrop);
		}
		else if (type == &type_float && op2.type == &type_float)
		{
			AddStatement(OPC_FDivVarDrop);
		}
		else if (type->type == ev_vector && op2.type == &type_float)
		{
			AddStatement(OPC_VIScaleVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(&type_void);
	}
	else if (TK_Check(PU_MOD_ASSIGN))
	{
		TType* type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		TypeCheck3(op2.type, type);
		if (type == &type_int && op2.type == &type_int)
		{
			AddStatement(OPC_ModVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(&type_void);
	}
	else if (TK_Check(PU_AND_ASSIGN))
	{
		TType* type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		TypeCheck3(op2.type, type);
		if (type == &type_int && op2.type == &type_int)
		{
			AddStatement(OPC_AndVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(&type_void);
	}
	else if (TK_Check(PU_OR_ASSIGN))
	{
		TType* type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		TypeCheck3(op2.type, type);
		if (type == &type_int && op2.type == &type_int)
		{
			AddStatement(OPC_OrVarDrop);
		}
//FIXME This is wrong!
		else if (type->type == ev_bool && op2.type == &type_int)
		{
			AddStatement(OPC_OrVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(&type_void);
	}
	else if (TK_Check(PU_XOR_ASSIGN))
	{
		TType* type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		TypeCheck3(op2.type, type);
		if (type == &type_int && op2.type == &type_int)
		{
			AddStatement(OPC_XOrVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(&type_void);
	}
	else if (TK_Check(PU_LSHIFT_ASSIGN))
	{
		TType* type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		TypeCheck3(op2.type, type);
		if (type == &type_int && op2.type == &type_int)
		{
			AddStatement(OPC_LShiftVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(&type_void);
	}
	else if (TK_Check(PU_RSHIFT_ASSIGN))
	{
		TType* type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		TypeCheck3(op2.type, type);
		if (type == &type_int && op2.type == &type_int)
		{
			AddStatement(OPC_RShiftVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(&type_void);
	}
	return op1;
}

//==========================================================================
//
//	ParseExpression
//
//==========================================================================

TType* ParseExpression(bool bLocals)
{
	CheckForLocal = bLocals;
	TTree op = ParseExpressionPriority14();
	return op.type;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.32  2005/12/07 22:52:55  dj_jl
//	Moved compiler generated data out of globals.
//
//	Revision 1.31  2005/11/30 23:55:05  dj_jl
//	Directly use with-drop statements.
//	
//	Revision 1.30  2005/11/30 13:14:53  dj_jl
//	Implemented instruction buffer.
//	
//	Revision 1.29  2005/11/29 19:31:43  dj_jl
//	Class and struct classes, removed namespaces, beautification.
//	
//	Revision 1.28  2005/11/24 20:40:42  dj_jl
//	Removed building of the tree, opcode renaming.
//	
//	Revision 1.27  2003/03/08 12:47:52  dj_jl
//	Code cleanup.
//	
//	Revision 1.26  2002/09/07 16:36:38  dj_jl
//	Support bool in function args and return type.
//	Removed support for typedefs.
//	
//	Revision 1.25  2002/08/24 14:45:38  dj_jl
//	2 pass compiling.
//	
//	Revision 1.24  2002/06/14 15:33:45  dj_jl
//	Some fixes.
//	
//	Revision 1.23  2002/05/03 17:04:03  dj_jl
//	Mangling of string pointers.
//	
//	Revision 1.22  2002/03/16 17:54:25  dj_jl
//	Added opcode for pushing virtual function.
//	
//	Revision 1.21  2002/02/26 17:52:20  dj_jl
//	Exporting special property info into progs.
//	
//	Revision 1.20  2002/02/16 16:28:36  dj_jl
//	Added support for bool variables
//	
//	Revision 1.19  2002/01/23 17:56:28  dj_jl
//	Removed support for C-style type casting.
//	
//	Revision 1.18  2002/01/21 18:23:09  dj_jl
//	Constructors with no names
//	
//	Revision 1.17  2002/01/12 18:06:34  dj_jl
//	New style of state functions, some other changes
//	
//	Revision 1.16  2002/01/11 08:17:31  dj_jl
//	Added name subsystem, removed support for unsigned ints
//	
//	Revision 1.15  2002/01/07 12:31:36  dj_jl
//	Changed copyright year
//	
//	Revision 1.14  2001/12/18 19:09:41  dj_jl
//	Some extra info in progs and other small changes
//	
//	Revision 1.13  2001/12/12 19:22:22  dj_jl
//	Support for method usage as state functions, dynamic cast
//	Added dynamic arrays
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
//	Revision 1.8  2001/10/22 17:29:58  dj_jl
//	Operators for clasid type
//	
//	Revision 1.7  2001/10/02 17:44:52  dj_jl
//	Some optimizations
//	
//	Revision 1.6  2001/09/25 17:03:50  dj_jl
//	Added calling of parent functions
//	
//	Revision 1.5  2001/09/20 16:09:55  dj_jl
//	Added basic object-oriented support
//	
//	Revision 1.4  2001/09/05 12:19:20  dj_jl
//	Release changes
//	
//	Revision 1.3  2001/08/21 17:52:54  dj_jl
//	Added support for real string pointers, beautification
//	
//	Revision 1.2  2001/07/27 14:27:56  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
