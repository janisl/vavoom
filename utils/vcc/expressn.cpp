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
	: Type(ev_void)
	, Flags(0)
	, RealType(ev_void)
	{}
	explicit TTree(EType InType)
	: Type(InType)
	, Flags(0)
	, RealType(ev_void)
	{}
	explicit TTree(const TType& InType)
	: Type(InType)
	, Flags(0)
	, RealType(ev_void)
	{}
	explicit TTree(const TType& InType, int InFlags)
	: Type(InType)
	, Flags(InFlags)
	, RealType(ev_void)
	{}
	explicit TTree(const TType& InType, const TType& InRealType)
	: Type(InType)
	, Flags(0)
	, RealType(InRealType)
	{}

	TType	Type;
	int		Flags;
	TType	RealType;
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

static TTree EmitPushPointed(const TType& type)
{
	if (type.type == ev_vector)
	{
		AddStatement(OPC_VPushPointed);
	}
	else if (type.type == ev_bool)
	{
		AddStatement(OPC_PushBool, type.bit_mask);
	}
	else if (type.type == ev_delegate)
	{
		AddStatement(OPC_PushPointedDelegate);
	}
	else
	{
		AddStatement(OPC_PushPointed);
	}

	TTree op = TTree(type, type);
	if (type.type == ev_bool)
	{
		op.Type = TType(ev_int);
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
	if (op.RealType.type == ev_void)
	{
		ParseError("Bad address operation");
		return op;
	}
	if (op.Flags & FIELD_ReadOnly)
	{
		ParseError("Tried to assign to a read-only field");
	}
	int Opc = UndoStatement();
	if (Opc != OPC_VPushPointed && Opc != OPC_PushBool &&
		Opc != OPC_PushPointed && Opc != OPC_PushPointedDelegate)
	{
		ParseError("Bad address operation");
		return op;
	}
	op.Type = MakePointerType(op.RealType);
	op.RealType = TType(ev_void);
	return op;
}

//==========================================================================
//
//	ParseFunctionCall
//
//==========================================================================

static TTree ParseFunctionCall(VMethod* Func, bool is_method)
{
	int arg = 0;
	int argsize = 0;
	int max_params;
	int num_needed_params = Func->NumParams;
	if (Func->Flags & FUNC_VarArgs)
	{
		max_params = MAX_ARG_COUNT - 1;
	}
	else
	{
		max_params = Func->NumParams;
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
					op.Type.CheckMatch(Func->ParamTypes[arg]);
				}
			}
			arg++;
			argsize += op.Type.GetSize();
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
	if (arg < num_needed_params)
	{
		ParseError("Incorrect argument count %d, should be %d", arg, num_needed_params);
	}
	if (Func->Flags & FUNC_VarArgs)
	{
		AddStatement(OPC_PushNumber, argsize / 4 - num_needed_params);
	}
	AddStatement(OPC_Call, Func->MemberIndex);
	TTree fop = TTree(Func->ReturnType);
	if (fop.Type.type == ev_bool)
		fop.Type = TType(ev_int);
	return fop;
}

//==========================================================================
//
//	ParseMethodCall
//
//==========================================================================

static TTree ParseMethodCall(VField* field, bool HaveSelf)
{
	VMethod* Func = field->func;
	int arg = 0;
	int argsize = 0;
	int max_params;
	int num_needed_params = Func->NumParams;
	if (Func->Flags & FUNC_VarArgs)
	{
		max_params = MAX_ARG_COUNT - 1;
	}
	else
	{
		max_params = Func->NumParams;
	}
	if (Func->Flags & FUNC_Static)
	{
		if (HaveSelf)
			ParseError("Invalid static function call");
	}
	else
	{
		if (!HaveSelf)
		{
			AddStatement(OPC_LocalAddress, 0);
			AddStatement(OPC_PushPointed);
		}
		if (field->type.type == ev_delegate)
		{
			AddStatement(OPC_Offset, field->ofs);
			AddStatement(OPC_PushPointedDelegate);
		}
		else
		{
			AddStatement(OPC_PushVFunc, field->ofs);
		}
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
					op.Type.CheckMatch(Func->ParamTypes[arg]);
				}
				if (!(Func->Flags & FUNC_Static))
				{
					if (op.Type.type == ev_vector)
						AddStatement(OPC_Swap3);
					else
						AddStatement(OPC_Swap);
				}
			}
			arg++;
			argsize += op.Type.GetSize();
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
	if (arg < num_needed_params)
	{
		ParseError("Incorrect argument count %d, should be %d", arg, num_needed_params);
	}
	if (Func->Flags & FUNC_VarArgs)
	{
		AddStatement(OPC_PushNumber, argsize / 4 - num_needed_params);
		if (!(Func->Flags & FUNC_Static))
		{
			AddStatement(OPC_Swap);
		}
	}
	if (Func->Flags & FUNC_Static)
	{
		AddStatement(OPC_Call, Func->MemberIndex);
	}
	else
	{
		AddStatement(OPC_ICall);
	}
	TTree fop = TTree(Func->ReturnType);
	if (fop.Type.type == ev_bool)
		fop.Type = TType(ev_int);
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
	int			num;
	VField*		field;
	VName		Name;
	bool		bLocals;
	VClass*		Class;
	VConstant*	Const;
	VState*		State;

	bLocals = CheckForLocal;
	CheckForLocal = false;
   	switch (tk_Token)
	{
	case TK_INTEGER:
		AddStatement(OPC_PushNumber, tk_Number);
		TK_NextToken();
		return TTree(ev_int);

	case TK_FLOAT:
		AddStatement(OPC_PushNumber, PassFloat(tk_Float));
		TK_NextToken();
		return TTree(ev_float);

	case TK_NAME:
		AddStatement(OPC_PushName, tk_Name.GetIndex());
		TK_NextToken();
		return TTree(ev_name);

	case TK_STRING:
		AddStatement(OPC_PushString, tk_StringI);
		TK_NextToken();
		return TTree(ev_string);

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
			if (field->type.type != ev_method)
			{
				ParseError("Not a method");
				break;
			}
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			return ParseFunctionCall(field->func, true);
		}
		break;

	case TK_KEYWORD:
		if (TK_Check(KW_VECTOR))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			TTree op1 = ParseExpressionPriority13();
			if (op1.Type.type != ev_float)
				ParseError("Expression type mistmatch, vector param 1 is not a float");
			TK_Expect(PU_COMMA, ERR_BAD_EXPR);
			TTree op2 = ParseExpressionPriority13();
			if (op2.Type.type != ev_float)
				ParseError("Expression type mistmatch, vector param 2 is not a float");
			TK_Expect(PU_COMMA, ERR_BAD_EXPR);
			TTree op3 = ParseExpressionPriority13();
			if (op3.Type.type != ev_float)
				ParseError("Expression type mistmatch, vector param 3 is not a float");
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			return TTree(ev_vector);
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
			op = TTree(TType((VClass*)NULL));
			return op;
		}
		if (TK_Check(KW_NULL))
		{
			AddStatement(OPC_PushNumber, 0);
			op = TTree(MakePointerType(TType(ev_void)));
			return op;
		}
		if (TK_Check(KW_TRUE))
		{
			AddStatement(OPC_PushNumber, 1);
			op = TTree(ev_int);
			return op;
		}
		if (TK_Check(KW_FALSE))
		{
			AddStatement(OPC_PushNumber, 0);
			op = TTree(ev_int);
			return op;
		}
		if (bLocals)
		{
			TType type = CheckForType(SelfClass);
			if (type.type != ev_unknown)
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
			VClass* Class = CheckForClass(Name);
			if (Class)
			{
				op = ParseExpressionPriority13();
				if (op.Type.type != ev_reference)
				{
					ParseError(ERR_BAD_EXPR, "Class reference required");
				}
				TK_Expect(PU_RPAREN, ERR_BAD_EXPR);
				AddStatement(OPC_DynamicCast, Class->MemberIndex);
				op = TTree(TType(Class));
				return op;
			}

			if (SelfClass)
			{
				field = CheckForField(Name, SelfClass);
				if (field && (field->type.type == ev_method || field->type.type == ev_delegate))
				{
					return ParseMethodCall(field, false);
				}
			}

			ParseError(ERR_ILLEGAL_EXPR_IDENT, "Identifier: %s", *Name);
			break;
		}

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
					TK_NextToken();
					AddStatement(OPC_PushNumber, Const->value);
					op = TTree((EType)Const->Type);
					return op;
				}

				State = CheckForState(tk_Name, Class);
				if (State)
				{
					TK_NextToken();
					AddStatement(OPC_PushState, State->MemberIndex);
					op = TTree(ev_state);
					return op;
				}
			}

			ParseError(ERR_ILLEGAL_EXPR_IDENT, "Identifier: %s", tk_String);
			break;
		}

		if (bLocals && (tk_Token == TK_IDENTIFIER ||
			(tk_Token == TK_PUNCT && tk_Punct == PU_ASTERISK)))
		{
			TType type = CheckForType(SelfClass, Name);
			if (type.type != ev_unknown)
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

		Const = CheckForConstant(SelfClass, Name);
		if (Const)
		{
			AddStatement(OPC_PushNumber, Const->value);
			op = TTree((EType)Const->Type);
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
				if (field->type.type == ev_method)
				{
					AddStatement(OPC_PushVFunc, field->ofs);
					op = TTree(ev_delegate);
					op.Type.Function = field->func;
				}
				else
				{
					AddStatement(OPC_Offset, field->ofs);
					op = EmitPushPointed(field->type);
					op.Flags = field->flags;
				}
				return op;
			}

			State = CheckForState(Name, SelfClass);
			if (State)
			{
				AddStatement(OPC_PushState, State->MemberIndex);
				op = TTree(ev_state);
				return op;
			}
		}

		Class = CheckForClass(Name);
		if (Class)
		{
			AddStatement(OPC_PushClassId, Class->MemberIndex);
			op = TTree(ev_classid);
			return op;
		}

		ERR_Exit(ERR_ILLEGAL_EXPR_IDENT, true, "Identifier: %s", *Name);
		break;

	default:
		break;
	}

	op = TTree(ev_void);
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
	TType		type;
	VField*	field;

	op = ParseExpressionPriority0();
	done = false;
	do
	{
		if (TK_Check(PU_MINUS_GT))
		{
			op.Type.CheckSizeIs4();
			if (op.Type.type != ev_pointer)
			{
				ERR_Exit(ERR_BAD_EXPR, true, NULL);
			}
			type = op.Type.GetPointerInnerType();
			field = ParseStructField(type.Struct);
			if (field)
			{
				if (field->type.type == ev_method)
				{
					ParseError("Pointed method");
				}
				else
				{
					AddStatement(OPC_Offset, field->ofs);
					op = EmitPushPointed(field->type);
					op.Flags = field->flags;
				}
			}
		}
		else if (TK_Check(PU_DOT))
		{
			if (op.Type.type == ev_reference)
			{
				field = ParseClassField(op.Type.Class);
				if (field)
				{
					if (field->type.type == ev_method)
					{
						if (TK_Check(PU_LPAREN))
						{
							op = ParseMethodCall(field, true);
						}
						else
						{
							AddStatement(OPC_PushVFunc, field->ofs);
							op = TTree(ev_delegate);
							op.Type.Function = field->func;
						}
					}
					else if (field->type.type == ev_delegate)
					{
						if (TK_Check(PU_LPAREN))
						{
							op = ParseMethodCall(field, true);
						}
						else
						{
							AddStatement(OPC_Offset, field->ofs);
							op = EmitPushPointed(field->type);
						}
					}
					else
					{
						AddStatement(OPC_Offset, field->ofs);
						op = EmitPushPointed(field->type);
						op.Flags = field->flags;
					}
				}
			}
			else if (op.Type.type == ev_struct || op.Type.type == ev_vector)
			{
				type = op.Type;
				int Flags = op.Flags;
				op.Flags &= ~FIELD_ReadOnly;
				op = GetAddress(op);
				field = ParseStructField(type.Struct);
				if (field)
				{
					AddStatement(OPC_Offset, field->ofs);
					op = EmitPushPointed(field->type);
					op.Flags |= Flags & FIELD_ReadOnly;
				}
			}
			else
			{
				ParseError(ERR_BAD_EXPR);
			}
		}
		else if (TK_Check(PU_LINDEX))
		{
			if (op.Type.type == ev_array)
			{
				type = op.Type.GetArrayInnerType();
				int Flags = op.Flags;
				op.Flags &= ~FIELD_ReadOnly;
				op = GetAddress(op);
				op.Flags = Flags;
			}
			else if (op.Type.type == ev_pointer)
			{
				type = op.Type.GetPointerInnerType();
			}
			else
			{
				ERR_Exit(ERR_BAD_ARRAY, true, NULL);
			}
			TTree ind = ParseExpressionPriority13();
			if (ind.Type.type != ev_int)
			{
				ParseError(ERR_EXPR_TYPE_MISTMATCH);
			}
			TK_Expect(PU_RINDEX, ERR_BAD_ARRAY);
			AddStatement(OPC_PushNumber, type.GetSize());
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
	TType		type;

	if (tk_Token == TK_PUNCT)
	{
		if (TK_Check(PU_PLUS))
		{
			op = ParseExpressionPriority2();
			if (op.Type.type != ev_int && op.Type.type != ev_float)
			{
				ParseError("Expression type mistmatch");
			}
			return op;
		}

		if (TK_Check(PU_MINUS))
		{
			op = ParseExpressionPriority2();
			if (op.Type.type == ev_int)
			{
				AddStatement(OPC_UnaryMinus);
				op = TTree(ev_int);
			}
			else if (op.Type.type == ev_float)
			{
				AddStatement(OPC_FUnaryMinus);
				op = TTree(ev_float);
			}
			else if (op.Type.type == ev_vector)
			{
				AddStatement(OPC_VUnaryMinus);
				op = TTree(op.Type);
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
			if (op.Type.type == ev_int ||
				op.Type.type == ev_float ||
				op.Type.type == ev_name ||
				op.Type.type == ev_string ||
				op.Type.type == ev_pointer ||
				op.Type.type == ev_reference ||
				op.Type.type == ev_classid ||
				op.Type.type == ev_state)
			{
				AddStatement(OPC_NegateLogical);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
			op = TTree(ev_int);
			return op;
		}

		if (TK_Check(PU_TILDE))
		{
			op = ParseExpressionPriority2();
			if (op.Type.type == ev_int)
			{
				AddStatement(OPC_BitInverse);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
			op = TTree(ev_int);
			return op;
		}

		if (TK_Check(PU_AND))
		{
			op = ParseExpressionPriority1();
			if (op.Type.type == ev_reference)
			{
				ParseError("Tried to take address of reference");
			}
			else
			{
				type = MakePointerType(op.Type);
				op = GetAddress(op);
			}
			return op;
		}
		if (TK_Check(PU_ASTERISK))
		{
			op = ParseExpressionPriority2();
			if (op.Type.type != ev_pointer)
			{
				ParseError("Expression syntax error");
				return op;
			}
			type = op.Type.GetPointerInnerType();
			return EmitPushPointed(type);
		}

		if (TK_Check(PU_INC))
		{
			op = ParseExpressionPriority2();
			type = op.Type;
			op = GetAddress(op);
			if (type.type == ev_int)
			{
				AddStatement(OPC_PreInc);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
			op = TTree(ev_int);
			return op;
		}

		if (TK_Check(PU_DEC))
		{
			op = ParseExpressionPriority2();
			type = op.Type;
			op = GetAddress(op);
			if (type.type == ev_int)
			{
				AddStatement(OPC_PreDec);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
			op = TTree(ev_int);
			return op;
		}
	}

	op = ParseExpressionPriority1();

	if (TK_Check(PU_INC))
	{
		type = op.Type;
		op = GetAddress(op);
		if (type.type == ev_int)
		{
			AddStatement(OPC_PostInc);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		op = TTree(ev_int);
		return op;
	}

	if (TK_Check(PU_DEC))
	{
		type = op.Type;
		op = GetAddress(op);
		if (type.type == ev_int)
		{
			AddStatement(OPC_PostDec);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		op = TTree(ev_int);
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
			if (op1.Type.type == ev_int && op2.Type.type == ev_int)
			{
				AddStatement(OPC_Multiply);
				op1 = TTree(ev_int);
			}
			else if (op1.Type.type == ev_float && op2.Type.type == ev_float)
			{
				AddStatement(OPC_FMultiply);
				op1 = TTree(ev_float);
			}
			else if (op1.Type.type == ev_vector && op2.Type.type == ev_float)
			{
				AddStatement(OPC_VPostScale);
				op1 = TTree(ev_vector);
			}
			else if (op1.Type.type == ev_float && op2.Type.type == ev_vector)
			{
				AddStatement(OPC_VPreScale);
				op1 = TTree(ev_vector);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
		}
		else if (TK_Check(PU_SLASH))
		{
			TTree op2 = ParseExpressionPriority2();
			if (op1.Type.type == ev_int && op2.Type.type == ev_int)
			{
				AddStatement(OPC_Divide);
				op1 = TTree(ev_int);
			}
			else if (op1.Type.type == ev_float && op2.Type.type == ev_float)
			{
				AddStatement(OPC_FDivide);
				op1 = TTree(ev_float);
			}
			else if (op1.Type.type == ev_vector && op2.Type.type == ev_float)
			{
				AddStatement(OPC_VIScale);
				op1 = TTree(ev_vector);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
		}
		else if (TK_Check(PU_PERCENT))
		{
			TTree op2 = ParseExpressionPriority2();
			if (op1.Type.type == ev_int && op2.Type.type == ev_int)
			{
				AddStatement(OPC_Modulus);
				op1 = TTree(ev_int);
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
			if (op1.Type.type == ev_int && op2.Type.type == ev_int)
			{
				AddStatement(OPC_Add);
				op1 = TTree(ev_int);
			}
			else if (op1.Type.type == ev_float && op2.Type.type == ev_float)
			{
				AddStatement(OPC_FAdd);
				op1 = TTree(ev_float);
			}
			else if (op1.Type.type == ev_vector && op2.Type.type == ev_vector)
			{
				AddStatement(OPC_VAdd);
				op1 = TTree(ev_vector);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
		}
		else if (TK_Check(PU_MINUS))
		{
			TTree op2 = ParseExpressionPriority3();
			if (op1.Type.type == ev_int && op2.Type.type == ev_int)
			{
				AddStatement(OPC_Subtract);
				op1 = TTree(ev_int);
			}
			else if (op1.Type.type == ev_float && op2.Type.type == ev_float)
			{
				AddStatement(OPC_FSubtract);
				op1 = TTree(ev_float);
			}
			else if (op1.Type.type == ev_vector && op2.Type.type == ev_vector)
			{
				AddStatement(OPC_VSubtract);
				op1 = TTree(ev_vector);
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
			if (op1.Type.type == ev_int && op2.Type.type == ev_int)
			{
				AddStatement(OPC_LShift);
				op1 = TTree(ev_int);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
		}
		else if (TK_Check(PU_RSHIFT))
		{
			TTree op2 = ParseExpressionPriority4();
			if (op1.Type.type == ev_int && op2.Type.type == ev_int)
			{
				AddStatement(OPC_RShift);
				op1 = TTree(ev_int);
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
			if (op1.Type.type == ev_int && op2.Type.type == ev_int)
			{
				AddStatement(OPC_Less);
				op1 = TTree(ev_int);
			}
			else if (op1.Type.type == ev_float && op2.Type.type == ev_float)
			{
				AddStatement(OPC_FLess);
				op1 = TTree(ev_int);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
		}
		else if (TK_Check(PU_LE))
		{
			TTree op2 = ParseExpressionPriority5();
			if (op1.Type.type == ev_int && op2.Type.type == ev_int)
			{
				AddStatement(OPC_LessEquals);
				op1 = TTree(ev_int);
			}
			else if (op1.Type.type == ev_float && op2.Type.type == ev_float)
			{
				AddStatement(OPC_FLessEquals);
				op1 = TTree(ev_int);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
		}
		else if (TK_Check(PU_GT))
		{
			TTree op2 = ParseExpressionPriority5();
			if (op1.Type.type == ev_int && op2.Type.type == ev_int)
			{
				AddStatement(OPC_Greater);
				op1 = TTree(ev_int);
			}
			else if (op1.Type.type == ev_float && op2.Type.type == ev_float)
			{
				AddStatement(OPC_FGreater);
				op1 = TTree(ev_int);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
		}
		else if (TK_Check(PU_GE))
		{
			TTree op2 = ParseExpressionPriority5();
			if (op1.Type.type == ev_int && op2.Type.type == ev_int)
			{
				AddStatement(OPC_GreaterEquals);
				op1 = TTree(ev_int);
			}
			else if (op1.Type.type == ev_float && op2.Type.type == ev_float)
			{
				AddStatement(OPC_FGreaterEquals);
				op1 = TTree(ev_int);
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
			if (op1.Type.type == ev_int && op2.Type.type == ev_int)
			{
				AddStatement(OPC_Equals);
			}
			else if (op1.Type.type == ev_float && op2.Type.type == ev_float)
			{
				AddStatement(OPC_FEquals);
			}
			else if (op1.Type.type == ev_name && op2.Type.type == ev_name)
			{
				AddStatement(OPC_Equals);
			}
			else if (op1.Type.type == ev_string && op2.Type.type == ev_string)
			{
				AddStatement(OPC_Equals);
			}
			else if (op1.Type.type == ev_pointer && op2.Type.type == ev_pointer)
			{
				AddStatement(OPC_Equals);
			}
			else if (op1.Type.type == ev_vector && op2.Type.type == ev_vector)
			{
				AddStatement(OPC_VEquals);
			}
			else if (op1.Type.type == ev_classid && op2.Type.type == ev_classid)
			{
				AddStatement(OPC_Equals);
			}
			else if (op1.Type.type == ev_state && op2.Type.type == ev_state)
			{
				AddStatement(OPC_Equals);
			}
			else if (op1.Type.type == ev_reference && op2.Type.type == ev_reference)
			{
				AddStatement(OPC_Equals);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
			op1 = TTree(ev_int);
		}
		else if (TK_Check(PU_NE))
		{
			TTree op2 = ParseExpressionPriority6();
			if (op1.Type.type == ev_int && op2.Type.type == ev_int)
			{
				AddStatement(OPC_NotEquals);
			}
			else if (op1.Type.type == ev_float && op2.Type.type == ev_float)
			{
				AddStatement(OPC_FNotEquals);
			}
			else if (op1.Type.type == ev_name && op2.Type.type == ev_name)
			{
				AddStatement(OPC_NotEquals);
			}
			else if (op1.Type.type == ev_string && op2.Type.type == ev_string)
			{
				AddStatement(OPC_NotEquals);
			}
			else if (op1.Type.type == ev_pointer && op2.Type.type == ev_pointer)
			{
				AddStatement(OPC_NotEquals);
			}
			else if (op1.Type.type == ev_vector && op2.Type.type == ev_vector)
			{
				AddStatement(OPC_VNotEquals);
			}
			else if (op1.Type.type == ev_classid && op2.Type.type == ev_classid)
			{
				AddStatement(OPC_NotEquals);
			}
			else if (op1.Type.type == ev_state && op2.Type.type == ev_state)
			{
				AddStatement(OPC_NotEquals);
			}
			else if (op1.Type.type == ev_reference && op2.Type.type == ev_reference)
			{
				AddStatement(OPC_NotEquals);
			}
			else
			{
				ParseError("Expression type mistmatch");
			}
			op1 = TTree(ev_int);
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
		if (op1.Type.type == ev_int && op2.Type.type == ev_int)
		{
			AddStatement(OPC_AndBitwise);
			op1 = TTree(ev_int);
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
		if (op1.Type.type == ev_int && op2.Type.type == ev_int)
		{
			AddStatement(OPC_XOrBitwise);
			op1 = TTree(ev_int);
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
		if (op1.Type.type == ev_int && op2.Type.type == ev_int)
		{
			AddStatement(OPC_OrBitwise);
			op1 = TTree(ev_int);
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
		op1.Type.CheckSizeIs4();
		int jmppos = AddStatement(OPC_IfNotTopGoto, 0);
		TTree op2 = ParseExpressionPriority10();
		op2.Type.CheckSizeIs4();
		AddStatement(OPC_AndLogical);
		FixupJump(jmppos);
		op1 = TTree(ev_int);
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
		op1.Type.CheckSizeIs4();
		int jmppos = AddStatement(OPC_IfTopGoto, 0);
		TTree op2 = ParseExpressionPriority11();
		op2.Type.CheckSizeIs4();
		AddStatement(OPC_OrLogical);
		FixupJump(jmppos);
		op1 = TTree(ev_int);
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
		op.Type.CheckSizeIs4();
		int jumppos1 = AddStatement(OPC_IfNotGoto, 0);
		op1 = ParseExpressionPriority13();
		TK_Expect(PU_COLON, ERR_MISSING_COLON);
		int jumppos2 = AddStatement(OPC_Goto, 0);
		FixupJump(jumppos1);
		op2 = ParseExpressionPriority13();
		FixupJump(jumppos2);
		op1.Type.CheckMatch(op2.Type);
		if (op1.Type.type == ev_pointer && op1.Type.InnerType == ev_void)
			op = TTree(op2.Type);
		else
			op = TTree(op1.Type);
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
		TType type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		op2.Type.CheckMatch(type);
		if (type.type == ev_int && op2.Type.type == ev_int)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type.type == ev_float && op2.Type.type == ev_float)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type.type == ev_name && op2.Type.type == ev_name)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type.type == ev_string && op2.Type.type == ev_string)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type.type == ev_pointer && op2.Type.type == ev_pointer)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type.type == ev_vector && op2.Type.type == ev_vector)
		{
			AddStatement(OPC_VAssignDrop);
		}
		else if (type.type == ev_classid && (op2.Type.type == ev_classid ||
			(op2.Type.type == ev_reference && op2.Type.Class == NULL)))
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type.type == ev_state && (op2.Type.type == ev_state ||
			(op2.Type.type == ev_reference && op2.Type.Class == NULL)))
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type.type == ev_reference && op2.Type.type == ev_reference)
		{
			AddStatement(OPC_AssignDrop);
		}
		else if (type.type == ev_bool && op2.Type.type == ev_int)
		{
			AddStatement(OPC_AssignBool, type.bit_mask);
		}
		else if (type.type == ev_delegate && op2.Type.type == ev_delegate)
		{
			AddStatement(OPC_AssignDelegate);
		}
		else if (type.type == ev_delegate && op2.Type.type == ev_reference && op2.Type.Class == NULL)
		{
			AddStatement(OPC_PushNumber, 0);
			AddStatement(OPC_AssignDelegate);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(ev_void);
	}
	else if (TK_Check(PU_ADD_ASSIGN))
	{
		TType type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		op2.Type.CheckMatch(type);
		if (type.type == ev_int && op2.Type.type == ev_int)
		{
			AddStatement(OPC_AddVarDrop);
		}
		else if (type.type == ev_float && op2.Type.type == ev_float)
		{
			AddStatement(OPC_FAddVarDrop);
		}
		else if (type.type == ev_vector && op2.Type.type == ev_vector)
		{
			AddStatement(OPC_VAddVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		op1 = TTree(ev_void);
		return op1;
	}
	else if (TK_Check(PU_MINUS_ASSIGN))
	{
		TType type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		op2.Type.CheckMatch(type);
		if (type.type == ev_int && op2.Type.type == ev_int)
		{
			AddStatement(OPC_SubVarDrop);
		}
		else if (type.type == ev_float && op2.Type.type == ev_float)
		{
			AddStatement(OPC_FSubVarDrop);
		}
		else if (type.type == ev_vector && op2.Type.type == ev_vector)
		{
			AddStatement(OPC_VSubVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(ev_void);
	}
	else if (TK_Check(PU_MULTIPLY_ASSIGN))
	{
		TType type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		op2.Type.CheckMatch(type);
		if (type.type == ev_int && op2.Type.type == ev_int)
		{
			AddStatement(OPC_MulVarDrop);
		}
		else if (type.type == ev_float && op2.Type.type == ev_float)
		{
			AddStatement(OPC_FMulVarDrop);
		}
		else if (type.type == ev_vector && op2.Type.type == ev_float)
		{
			AddStatement(OPC_VScaleVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(ev_void);
	}
	else if (TK_Check(PU_DIVIDE_ASSIGN))
	{
		TType type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		op2.Type.CheckMatch(type);
		if (type.type == ev_int && op2.Type.type == ev_int)
		{
			AddStatement(OPC_DivVarDrop);
		}
		else if (type.type == ev_float && op2.Type.type == ev_float)
		{
			AddStatement(OPC_FDivVarDrop);
		}
		else if (type.type == ev_vector && op2.Type.type == ev_float)
		{
			AddStatement(OPC_VIScaleVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(ev_void);
	}
	else if (TK_Check(PU_MOD_ASSIGN))
	{
		TType type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		op2.Type.CheckMatch(type);
		if (type.type == ev_int && op2.Type.type == ev_int)
		{
			AddStatement(OPC_ModVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(ev_void);
	}
	else if (TK_Check(PU_AND_ASSIGN))
	{
		TType type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		op2.Type.CheckMatch(type);
		if (type.type == ev_int && op2.Type.type == ev_int)
		{
			AddStatement(OPC_AndVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(ev_void);
	}
	else if (TK_Check(PU_OR_ASSIGN))
	{
		TType type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		op2.Type.CheckMatch(type);
		if (type.type == ev_int && op2.Type.type == ev_int)
		{
			AddStatement(OPC_OrVarDrop);
		}
//FIXME This is wrong!
		else if (type.type == ev_bool && op2.Type.type == ev_int)
		{
			AddStatement(OPC_OrVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(ev_void);
	}
	else if (TK_Check(PU_XOR_ASSIGN))
	{
		TType type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		op2.Type.CheckMatch(type);
		if (type.type == ev_int && op2.Type.type == ev_int)
		{
			AddStatement(OPC_XOrVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(ev_void);
	}
	else if (TK_Check(PU_LSHIFT_ASSIGN))
	{
		TType type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		op2.Type.CheckMatch(type);
		if (type.type == ev_int && op2.Type.type == ev_int)
		{
			AddStatement(OPC_LShiftVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(ev_void);
	}
	else if (TK_Check(PU_RSHIFT_ASSIGN))
	{
		TType type = op1.RealType;
		op1 = GetAddress(op1);
		TTree op2 = ParseExpressionPriority13();
		op2.Type.CheckMatch(type);
		if (type.type == ev_int && op2.Type.type == ev_int)
		{
			AddStatement(OPC_RShiftVarDrop);
		}
		else
		{
			ParseError("Expression type mistmatch");
		}
		return TTree(ev_void);
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
	CheckForLocal = bLocals;
	TTree op = ParseExpressionPriority14();
	if (op.Type.type == ev_delegate)
	{
		op = GetAddress(op);
		AddStatement(OPC_PushPointed);
		op = TTree(ev_int);
	}
	return op.Type;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.46  2006/03/23 18:30:53  dj_jl
//	Use single list of all members, members tree.
//
//	Revision 1.45  2006/03/18 16:53:24  dj_jl
//	Use offset opcode.
//	
//	Revision 1.44  2006/03/13 21:24:21  dj_jl
//	Added support for read-only, private and transient fields.
//	
//	Revision 1.43  2006/03/12 20:04:50  dj_jl
//	States as objects, added state variable type.
//	
//	Revision 1.42  2006/03/10 19:31:55  dj_jl
//	Use serialisation for progs files.
//	
//	Revision 1.41  2006/02/28 19:17:20  dj_jl
//	Added support for constants.
//	
//	Revision 1.40  2006/02/27 21:23:54  dj_jl
//	Rewrote names class.
//	
//	Revision 1.39  2006/02/19 20:37:01  dj_jl
//	Implemented support for delegates.
//	
//	Revision 1.38  2006/02/19 14:37:36  dj_jl
//	Changed type handling.
//	
//	Revision 1.37  2006/02/17 19:25:00  dj_jl
//	Removed support for progs global variables and functions.
//	
//	Revision 1.36  2006/02/10 22:15:21  dj_jl
//	Temporary fix for big-endian systems.
//	
//	Revision 1.35  2005/12/29 19:10:40  dj_jl
//	Fixed compiler problem.
//	
//	Revision 1.34  2005/12/14 20:53:23  dj_jl
//	State names belong to a class.
//	Structs and enums defined in a class.
//	
//	Revision 1.33  2005/12/12 20:58:47  dj_jl
//	Removed compiler limitations.
//	
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
