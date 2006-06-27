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
	switch (type.type)
	{
	case ev_int:
	case ev_float:
	case ev_name:
	case ev_struct://	Not exactly, but the rest of code will fail without these.
	case ev_array:
		AddStatement(OPC_PushPointed);
		break;

	case ev_pointer:
	case ev_reference:
	case ev_classid:
	case ev_state:
		AddStatement(OPC_PushPointedPtr);
		break;

	case ev_vector:
		AddStatement(OPC_VPushPointed);
		break;

	case ev_bool:
		if (type.bit_mask & 0x000000ff)
			AddStatement(OPC_PushBool0, type.bit_mask);
		else if (type.bit_mask & 0x0000ff00)
			AddStatement(OPC_PushBool1, type.bit_mask >> 8);
		else if (type.bit_mask & 0x00ff0000)
			AddStatement(OPC_PushBool2, type.bit_mask >> 16);
		else
			AddStatement(OPC_PushBool3, type.bit_mask >> 24);
		break;

	case ev_string:
		AddStatement(OPC_PushPointedStr);
		break;

	case ev_delegate:
		AddStatement(OPC_PushPointedDelegate);
		break;
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
	if (Opc != OPC_VPushPointed && Opc != OPC_PushBool0 &&
		Opc != OPC_PushBool1 && Opc != OPC_PushBool2 &&
		Opc != OPC_PushBool3 && Opc != OPC_PushPointed &&
		Opc != OPC_PushPointedPtr && Opc != OPC_PushPointedStr &&
		Opc != OPC_PushPointedDelegate)
	{
		ParseError("Bad address operation %d", Opc);
		return op;
	}
	op.Type = MakePointerType(op.RealType);
	op.RealType = TType(ev_void);
	return op;
}

//==========================================================================
//
//	ParseMethodCall
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
//	ParseMethodCall
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
//	ParseMethodCall
//
//==========================================================================

static TTree ParseMethodCall(VMethod* Func, VField* DelegateField,
	bool HaveSelf, bool BaseCall)
{
	bool DirectCall = BaseCall || (Func->Flags & FUNC_Final);

	//	Determine parameter count.
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
			AddStatement(OPC_LocalAddress0);
			AddStatement(OPC_PushPointedPtr);
		}
		if (DelegateField)
		{
			AddStatement(OPC_Offset, DelegateField);
			AddStatement(OPC_PushPointedDelegate);
		}
		else if (!DirectCall)
		{
			AddStatement(OPC_PushVFunc, Func);
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
				if (!DirectCall)
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
		EmitPushNumber(argsize / 4 - num_needed_params);
		if (!DirectCall)
		{
			AddStatement(OPC_Swap);
		}
	}
	if (DirectCall)
	{
		AddStatement(OPC_Call, Func);
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
		EmitPushNumber(tk_Number);
		TK_NextToken();
		return TTree(ev_int);

	case TK_FLOAT:
		AddStatement(OPC_PushNumber, tk_Float);
		TK_NextToken();
		return TTree(ev_float);

	case TK_NAME:
		AddStatement(OPC_PushName, tk_Name);
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
			if (tk_Token != TK_IDENTIFIER)
			{
				ParseError("Method name expected.");
				break;
			}
			VMethod* M = CheckForMethod(tk_Name, SelfClass->ParentClass);
			if (!M)
			{
				ParseError("No such method %s", *tk_Name);
				break;
			}
			TK_NextToken();
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			return ParseMethodCall(M, NULL, false, true);
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
				AddStatement(OPC_LocalAddress0);
				AddStatement(OPC_PushPointedPtr);
				op = TTree(SelfType);
				return op;
			}
		}
		if (TK_Check(KW_NONE))
		{
			AddStatement(OPC_PushNull);
			op = TTree(TType((VClass*)NULL));
			return op;
		}
		if (TK_Check(KW_NULL))
		{
			AddStatement(OPC_PushNull);
			op = TTree(MakePointerType(TType(ev_void)));
			return op;
		}
		if (TK_Check(KW_TRUE))
		{
			AddStatement(OPC_PushNumber1);
			op = TTree(ev_int);
			return op;
		}
		if (TK_Check(KW_FALSE))
		{
			AddStatement(OPC_PushNumber0);
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
				AddStatement(OPC_DynamicCast, Class);
				op = TTree(TType(Class));
				return op;
			}

			if (SelfClass)
			{
				VMethod* M = CheckForMethod(Name, SelfClass);
				if (M)
				{
					return ParseMethodCall(M, NULL, false, false);
				}
				field = CheckForField(Name, SelfClass);
				if (field && field->type.type == ev_delegate)
				{
					return ParseMethodCall(field->func, field, false, false);
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
					EmitPushNumber(Const->value);
					op = TTree((EType)Const->Type);
					return op;
				}

				State = CheckForState(tk_Name, Class);
				if (State)
				{
					TK_NextToken();
					AddStatement(OPC_PushState, State);
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
			EmitLocalAddress(localdefs[num].ofs);
			op = EmitPushPointed(localdefs[num].type);
			return op;
		}

		Const = CheckForConstant(SelfClass, Name);
		if (Const)
		{
			EmitPushNumber(Const->value);
			op = TTree((EType)Const->Type);
			return op;
		}

		if (SelfClass)
		{
			VMethod* M = CheckForMethod(Name, SelfClass);
			if (M)
			{
				AddStatement(OPC_LocalAddress0);
				AddStatement(OPC_PushPointedPtr);
				AddStatement(OPC_PushVFunc, M);
				op = TTree(ev_delegate);
				op.Type.Function = M;
				return op;
			}

			field = CheckForField(Name, SelfClass);
			if (field)
			{
				AddStatement(OPC_LocalAddress0);
				AddStatement(OPC_PushPointedPtr);
				AddStatement(OPC_Offset, field);
				op = EmitPushPointed(field->type);
				op.Flags = field->flags;
				return op;
			}

			State = CheckForState(Name, SelfClass);
			if (State)
			{
				AddStatement(OPC_PushState, State);
				op = TTree(ev_state);
				return op;
			}
		}

		Class = CheckForClass(Name);
		if (Class)
		{
			AddStatement(OPC_PushClassId, Class);
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
			if (op.Type.type != ev_pointer)
			{
				ERR_Exit(ERR_BAD_EXPR, true, NULL);
			}
			type = op.Type.GetPointerInnerType();
			field = ParseStructField(type.Struct);
			if (field)
			{
				AddStatement(OPC_Offset, field);
				op = EmitPushPointed(field->type);
				op.Flags = field->flags;
			}
		}
		else if (TK_Check(PU_DOT))
		{
			if (op.Type.type == ev_reference)
			{
				if (tk_Token != TK_IDENTIFIER)
				{
					ParseError(ERR_INVALID_IDENTIFIER, ", field name expacted");
				}
				else
				{
					VMethod* M = CheckForMethod(tk_Name, op.Type.Class);
					if (M)
					{
						TK_NextToken();
						if (TK_Check(PU_LPAREN))
						{
							op = ParseMethodCall(M, NULL, true, false);
						}
						else
						{
							AddStatement(OPC_PushVFunc, M);
							op = TTree(ev_delegate);
							op.Type.Function = M;
						}
					}
					else
					{
						field = CheckForField(tk_Name, op.Type.Class);
						if (!field)
						{
							ParseError(ERR_NOT_A_FIELD, "Identifier: %s", *tk_Name);
						}
						TK_NextToken();
						if (field)
						{
							if (field->type.type == ev_delegate)
							{
								if (TK_Check(PU_LPAREN))
								{
									op = ParseMethodCall(field->func, field, true, false);
								}
								else
								{
									AddStatement(OPC_Offset, field);
									op = EmitPushPointed(field->type);
								}
							}
							else
							{
								AddStatement(OPC_Offset, field);
								op = EmitPushPointed(field->type);
								op.Flags = field->flags;
							}
						}
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
					AddStatement(OPC_Offset, field);
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
			AddStatement(OPC_ArrayElement, type);
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
			op.Type.CheckSizeIs4();
			AddStatement(OPC_NegateLogical);
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
			else if (op1.Type.type == ev_pointer && op2.Type.type == ev_pointer)
			{
				AddStatement(OPC_PtrEquals);
			}
			else if (op1.Type.type == ev_vector && op2.Type.type == ev_vector)
			{
				AddStatement(OPC_VEquals);
			}
			else if (op1.Type.type == ev_classid && op2.Type.type == ev_classid)
			{
				AddStatement(OPC_PtrEquals);
			}
			else if (op1.Type.type == ev_state && op2.Type.type == ev_state)
			{
				AddStatement(OPC_PtrEquals);
			}
			else if (op1.Type.type == ev_reference && op2.Type.type == ev_reference)
			{
				AddStatement(OPC_PtrEquals);
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
			else if (op1.Type.type == ev_pointer && op2.Type.type == ev_pointer)
			{
				AddStatement(OPC_PtrNotEquals);
			}
			else if (op1.Type.type == ev_vector && op2.Type.type == ev_vector)
			{
				AddStatement(OPC_VNotEquals);
			}
			else if (op1.Type.type == ev_classid && op2.Type.type == ev_classid)
			{
				AddStatement(OPC_PtrNotEquals);
			}
			else if (op1.Type.type == ev_state && op2.Type.type == ev_state)
			{
				AddStatement(OPC_PtrNotEquals);
			}
			else if (op1.Type.type == ev_reference && op2.Type.type == ev_reference)
			{
				AddStatement(OPC_PtrNotEquals);
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
			AddStatement(OPC_AssignStrDrop);
		}
		else if (type.type == ev_pointer && op2.Type.type == ev_pointer)
		{
			AddStatement(OPC_AssignPtrDrop);
		}
		else if (type.type == ev_vector && op2.Type.type == ev_vector)
		{
			AddStatement(OPC_VAssignDrop);
		}
		else if (type.type == ev_classid && (op2.Type.type == ev_classid ||
			(op2.Type.type == ev_reference && op2.Type.Class == NULL)))
		{
			AddStatement(OPC_AssignPtrDrop);
		}
		else if (type.type == ev_state && (op2.Type.type == ev_state ||
			(op2.Type.type == ev_reference && op2.Type.Class == NULL)))
		{
			AddStatement(OPC_AssignPtrDrop);
		}
		else if (type.type == ev_reference && op2.Type.type == ev_reference)
		{
			AddStatement(OPC_AssignPtrDrop);
		}
		else if (type.type == ev_bool && op2.Type.type == ev_int)
		{
			if (type.bit_mask & 0x000000ff)
				AddStatement(OPC_AssignBool0, type.bit_mask);
			else if (type.bit_mask & 0x0000ff00)
				AddStatement(OPC_AssignBool1, type.bit_mask >> 8);
			else if (type.bit_mask & 0x00ff0000)
				AddStatement(OPC_AssignBool2, type.bit_mask >> 16);
			else
				AddStatement(OPC_AssignBool3, type.bit_mask >> 24);
		}
		else if (type.type == ev_delegate && op2.Type.type == ev_delegate)
		{
			AddStatement(OPC_AssignDelegate);
		}
		else if (type.type == ev_delegate && op2.Type.type == ev_reference && op2.Type.Class == NULL)
		{
			AddStatement(OPC_PushNull);
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
