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

namespace Pass2 {

// MACROS ------------------------------------------------------------------

#define MAX_ARG_COUNT		16

// TYPES -------------------------------------------------------------------

class TOperator
{
 public:
	enum id_t 
	{
		ID_UNARYPLUS,
		ID_UNARYMINUS,
		ID_NEGATELOGICAL,
		ID_BITINVERSE,
		ID_PREINC,
		ID_PREDEC,
		ID_POSTINC,
		ID_POSTDEC,
		ID_MULTIPLY,
		ID_DIVIDE,
		ID_MODULUS,
		ID_ADD,
		ID_SUBTRACT,
		ID_LSHIFT,
		ID_RSHIFT,
		ID_LT,
		ID_LE,
		ID_GT,
		ID_GE,
		ID_EQ,
		ID_NE,
		ID_ANDBITWISE,
		ID_XORBITWISE,
		ID_ORBITWISE,

		ID_ASSIGN,
		ID_ADDVAR,
		ID_SUBVAR,
		ID_MULVAR,
		ID_DIVVAR,
		ID_MODVAR,
		ID_ANDVAR,
		ID_ORVAR,
		ID_XORVAR,
		ID_LSHIFTVAR,
		ID_RSHIFTVAR,

		NUM_OPERATORS
	};

	TOperator(id_t Aopid, TType* Atype, TType* Atype1, TType* Atype2, int Aopcode);

	TOperator	*next;
	id_t		opid;
	TType		*type;
	TType		*type1;
	TType		*type2;
	int			opcode;
};

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

static TOperator	*operators[TOperator::NUM_OPERATORS];

static TOperator	UnaryPlus_int(TOperator::ID_UNARYPLUS, &type_int, &type_int, &type_void, OPC_Done);
static TOperator	UnaryPlus_float(TOperator::ID_UNARYPLUS, &type_float, &type_float, &type_void, OPC_Done);

static TOperator	UnaryMinus_int(TOperator::ID_UNARYMINUS, &type_int, &type_int, &type_void, OPC_UnaryMinus);
static TOperator	UnaryMinus_float(TOperator::ID_UNARYMINUS, &type_float, &type_float, &type_void, OPC_FUnaryMinus);
static TOperator	UnaryMinus_vector(TOperator::ID_UNARYMINUS, &type_vector, &type_vector, &type_void, OPC_VUnaryMinus);

static TOperator	NotLogical_int(TOperator::ID_NEGATELOGICAL, &type_int, &type_int, &type_void, OPC_NegateLogical);
static TOperator	NotLogical_float(TOperator::ID_NEGATELOGICAL, &type_int, &type_float, &type_void, OPC_NegateLogical);
static TOperator	NotLogical_name(TOperator::ID_NEGATELOGICAL, &type_int, &type_name, &type_void, OPC_NegateLogical);
static TOperator	NotLogical_str(TOperator::ID_NEGATELOGICAL, &type_int, &type_string, &type_void, OPC_NegateLogical);
static TOperator	NotLogical_ptr(TOperator::ID_NEGATELOGICAL, &type_int, &type_void_ptr, &type_void, OPC_NegateLogical);
static TOperator	NotLogical_ref(TOperator::ID_NEGATELOGICAL, &type_int, &type_none_ref, &type_void, OPC_NegateLogical);
static TOperator	NotLogical_cid(TOperator::ID_NEGATELOGICAL, &type_int, &type_classid, &type_void, OPC_NegateLogical);

static TOperator	BitInverse_int(TOperator::ID_BITINVERSE, &type_int, &type_int, &type_void, OPC_BitInverse);

static TOperator	PreInc_int(TOperator::ID_PREINC, &type_int, &type_int, &type_void, OPC_PreInc);

static TOperator	PreDec_int(TOperator::ID_PREDEC, &type_int, &type_int, &type_void, OPC_PreDec);

static TOperator	PostInc_int(TOperator::ID_POSTINC, &type_int, &type_int, &type_void, OPC_PostInc);

static TOperator	PostDec_int(TOperator::ID_POSTDEC, &type_int, &type_int, &type_void, OPC_PostDec);

static TOperator	Mul_int_int(TOperator::ID_MULTIPLY, &type_int, &type_int, &type_int, OPC_Multiply);
static TOperator	Mul_float_float(TOperator::ID_MULTIPLY, &type_float, &type_float, &type_float, OPC_FMultiply);
static TOperator	Mul_vec_float(TOperator::ID_MULTIPLY, &type_vector, &type_vector, &type_float, OPC_VPostScale);
static TOperator	Mul_float_vec(TOperator::ID_MULTIPLY, &type_vector, &type_float, &type_vector, OPC_VPreScale);

static TOperator	Div_int_int(TOperator::ID_DIVIDE, &type_int, &type_int, &type_int, OPC_Divide);
static TOperator	Div_float_float(TOperator::ID_DIVIDE, &type_float, &type_float, &type_float, OPC_FDivide);
static TOperator	Div_vec_float(TOperator::ID_DIVIDE, &type_vector, &type_vector, &type_float, OPC_VIScale);

static TOperator	Mod_int_int(TOperator::ID_MODULUS, &type_int, &type_int, &type_int, OPC_Modulus);

static TOperator	Add_int_int(TOperator::ID_ADD, &type_int, &type_int, &type_int, OPC_Add);
static TOperator	Add_float_float(TOperator::ID_ADD, &type_float, &type_float, &type_float, OPC_FAdd);
static TOperator	Add_vec_vec(TOperator::ID_ADD, &type_vector, &type_vector, &type_vector, OPC_VAdd);

static TOperator	Sub_int_int(TOperator::ID_SUBTRACT, &type_int, &type_int, &type_int, OPC_Subtract);
static TOperator	Sub_float_float(TOperator::ID_SUBTRACT, &type_float, &type_float, &type_float, OPC_FSubtract);
static TOperator	Sub_vec_vec(TOperator::ID_SUBTRACT, &type_vector, &type_vector, &type_vector, OPC_VSubtract);

static TOperator	LShift_int_int(TOperator::ID_LSHIFT, &type_int, &type_int, &type_int, OPC_LShift);

static TOperator	RShift_int_int(TOperator::ID_RSHIFT, &type_int, &type_int, &type_int, OPC_RShift);

static TOperator	Lt_int_int(TOperator::ID_LT, &type_int, &type_int, &type_int, OPC_Less);
static TOperator	Lt_float_float(TOperator::ID_LT, &type_int, &type_float, &type_float, OPC_FLess);

static TOperator	Le_int_int(TOperator::ID_LE, &type_int, &type_int, &type_int, OPC_LessEquals);
static TOperator	Le_float_float(TOperator::ID_LE, &type_int, &type_float, &type_float, OPC_FLessEquals);

static TOperator	Gt_int_int(TOperator::ID_GT, &type_int, &type_int, &type_int, OPC_Greater);
static TOperator	Gt_float_float(TOperator::ID_GT, &type_int, &type_float, &type_float, OPC_FGreater);

static TOperator	Ge_int_int(TOperator::ID_GE, &type_int, &type_int, &type_int, OPC_GreaterEquals);
static TOperator	Ge_float_float(TOperator::ID_GE, &type_int, &type_float, &type_float, OPC_FGreaterEquals);

static TOperator	Eq_int_int(TOperator::ID_EQ, &type_int, &type_int, &type_int, OPC_Equals);
static TOperator	Eq_float_float(TOperator::ID_EQ, &type_int, &type_float, &type_float, OPC_FEquals);
static TOperator	Eq_name_name(TOperator::ID_EQ, &type_int, &type_name, &type_name, OPC_Equals);
static TOperator	Eq_str_str(TOperator::ID_EQ, &type_int, &type_string, &type_string, OPC_Equals);
static TOperator	Eq_ptr_ptr(TOperator::ID_EQ, &type_int, &type_void_ptr, &type_void_ptr, OPC_Equals);
static TOperator	Eq_vec_vec(TOperator::ID_EQ, &type_int, &type_vector, &type_vector, OPC_VEquals);
static TOperator	Eq_cid_cid(TOperator::ID_EQ, &type_int, &type_classid, &type_classid, OPC_Equals);
static TOperator	Eq_ref_ref(TOperator::ID_EQ, &type_int, &type_none_ref, &type_none_ref, OPC_Equals);

static TOperator	Ne_int_int(TOperator::ID_NE, &type_int, &type_int, &type_int, OPC_NotEquals);
static TOperator	Ne_float_float(TOperator::ID_NE, &type_int, &type_float, &type_float, OPC_FNotEquals);
static TOperator	Ne_name_name(TOperator::ID_NE, &type_int, &type_name, &type_name, OPC_NotEquals);
static TOperator	Ne_str_str(TOperator::ID_NE, &type_int, &type_string, &type_string, OPC_NotEquals);
static TOperator	Ne_ptr_ptr(TOperator::ID_NE, &type_int, &type_void_ptr, &type_void_ptr, OPC_NotEquals);
static TOperator	Ne_vec_vec(TOperator::ID_NE, &type_int, &type_vector, &type_vector, OPC_VNotEquals);
static TOperator	Ne_cid_cid(TOperator::ID_NE, &type_int, &type_classid, &type_classid, OPC_NotEquals);
static TOperator	Ne_ref_ref(TOperator::ID_NE, &type_int, &type_none_ref, &type_none_ref, OPC_NotEquals);

static TOperator	And_int_int(TOperator::ID_ANDBITWISE, &type_int, &type_int, &type_int, OPC_AndBitwise);

static TOperator	Xor_int_int(TOperator::ID_XORBITWISE, &type_int, &type_int, &type_int, OPC_XOrBitwise);

static TOperator	Or_int_int(TOperator::ID_ORBITWISE, &type_int, &type_int, &type_int, OPC_OrBitwise);

static TOperator	Assign_int_int(TOperator::ID_ASSIGN, &type_int, &type_int, &type_int, OPC_Assign);
static TOperator	Assign_float_float(TOperator::ID_ASSIGN, &type_float, &type_float, &type_float, OPC_Assign);
static TOperator	Assign_name_name(TOperator::ID_ASSIGN, &type_name, &type_name, &type_name, OPC_Assign);
static TOperator	Assign_str_str(TOperator::ID_ASSIGN, &type_string, &type_string, &type_string, OPC_Assign);
static TOperator	Assign_ptr_ptr(TOperator::ID_ASSIGN, &type_void_ptr, &type_void_ptr, &type_void_ptr, OPC_Assign);
static TOperator	Assign_vec_vec(TOperator::ID_ASSIGN, &type_vector, &type_vector, &type_vector, OPC_VAssign);
static TOperator	Assign_cid_cid(TOperator::ID_ASSIGN, &type_classid, &type_classid, &type_classid, OPC_Assign);
static TOperator	Assign_ref_ref(TOperator::ID_ASSIGN, &type_none_ref, &type_none_ref, &type_none_ref, OPC_Assign);
static TOperator	Assign_bool_int(TOperator::ID_ASSIGN, &type_bool, &type_bool, &type_int, OPC_AssignBool);

static TOperator	AddVar_int_int(TOperator::ID_ADDVAR, &type_int, &type_int, &type_int, OPC_AddVar);
static TOperator	AddVar_float_float(TOperator::ID_ADDVAR, &type_float, &type_float, &type_float, OPC_FAddVar);
static TOperator	AddVar_vec_vec(TOperator::ID_ADDVAR, &type_vector, &type_vector, &type_vector, OPC_VAddVar);

static TOperator	SubVar_int_int(TOperator::ID_SUBVAR, &type_int, &type_int, &type_int, OPC_SubVar);
static TOperator	SubVar_float_float(TOperator::ID_SUBVAR, &type_float, &type_float, &type_float, OPC_FSubVar);
static TOperator	SubVar_vec_vec(TOperator::ID_SUBVAR, &type_vector, &type_vector, &type_vector, OPC_VSubVar);

static TOperator	MulVar_int_int(TOperator::ID_MULVAR, &type_int, &type_int, &type_int, OPC_MulVar);
static TOperator	MulVar_float_float(TOperator::ID_MULVAR, &type_float, &type_float, &type_float, OPC_FMulVar);
static TOperator	MulVar_vec_float(TOperator::ID_MULVAR, &type_vector, &type_vector, &type_float, OPC_VScaleVar);

static TOperator	DivVar_int_int(TOperator::ID_DIVVAR, &type_int, &type_int, &type_int, OPC_DivVar);
static TOperator	DivVar_float_float(TOperator::ID_DIVVAR, &type_float, &type_float, &type_float, OPC_FDivVar);
static TOperator	DivVar_vec_float(TOperator::ID_DIVVAR, &type_vector, &type_vector, &type_float, OPC_VIScaleVar);

static TOperator	ModVar_int_int(TOperator::ID_MODVAR, &type_int, &type_int, &type_int, OPC_ModVar);

static TOperator	AndVar_int_int(TOperator::ID_ANDVAR, &type_int, &type_int, &type_int, OPC_AndVar);

static TOperator	OrVar_int_int(TOperator::ID_ORVAR, &type_int, &type_int, &type_int, OPC_OrVar);
static TOperator	OrVar_bool_int(TOperator::ID_ORVAR, &type_bool, &type_bool, &type_int, OPC_OrVar);

static TOperator	XorVar_int_int(TOperator::ID_XORVAR, &type_int, &type_int, &type_int, OPC_XOrVar);

static TOperator	LShiftVar_int_int(TOperator::ID_LSHIFTVAR, &type_int, &type_int, &type_int, OPC_LShiftVar);

static TOperator	RShiftVar_int_int(TOperator::ID_RSHIFTVAR, &type_int, &type_int, &type_int, OPC_RShiftVar);

static TOperator	NullOp(TOperator::NUM_OPERATORS, &type_void, &type_void, &type_void, OPC_Done);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TOperator::TOperator
//
//==========================================================================

TOperator::TOperator(id_t Aopid, TType* Atype, TType* Atype1, TType* Atype2, int Aopcode) :
	opid(Aopid), type(Atype), type1(Atype1), type2(Atype2), opcode(Aopcode)
{
	next = operators[opid];
	operators[opid] = this;
}

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
	UndoStatement();
	op.type = MakePointerType(op.RealType);
	op.RealType = NULL;
	return op;
}

//==========================================================================
//
//	TypeCmp
//
//==========================================================================

bool TypeCmp(TType *type1, TType *type2)
{
	if (type1 == type2)
	{
		return true;
	}
	if (type1->type == ev_bool && type2->type == ev_bool)
	{
		return true;
	}
	if ((type1->type == ev_vector) && (type2->type == ev_vector))
	{
		return true;
	}
	if ((type1->type == ev_pointer) && (type2->type == ev_pointer))
	{
		if (type1 == &type_void_ptr || type2 == &type_void_ptr)
		{
			return true;
		}
	}
	if ((type1->type == ev_reference) && (type2->type == ev_reference))
	{
		if (type1 == &type_none_ref || type2 == &type_none_ref)
		{
			return true;
		}
	}
	return false;
}

//==========================================================================
//
//	FindOperator
//
//==========================================================================

TOperator *FindOperator(TOperator::id_t opid, TType *type1, TType *type2)
{
	TOperator	*oper;

	for (oper = operators[opid]; oper; oper = oper->next)
	{
		if (TypeCmp(oper->type1, type1) && TypeCmp(oper->type2, type2))
		{
			return oper;
		}
	}
	ParseError("Expression type mistmatch");
	return &NullOp;
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
			if (!SelfType)
			{
				ParseError(":: not in method");
				break;
			}
			field = CheckForField(SelfType->aux_type->aux_type);
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
			if (!SelfType)
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
			type = CheckForType(Name);
			if (type)
			{
				if (type->type != ev_class)
				{
					ParseError(ERR_ILLEGAL_EXPR_IDENT, "Identifier: %s", *Name);
					break;
				}
				op = ParseExpressionPriority13();
				if (op.type->type != ev_reference)
				{
					ParseError(ERR_BAD_EXPR, "Class reference required");
				}
				TK_Expect(PU_RPAREN, ERR_BAD_EXPR);
				AddStatement(OPC_DynamicCast, type->classid);
				op = TTree(MakeReferenceType(type));
				return op;
			}

			num = CheckForFunction(NULL, Name);
			if (num)
			{
				return ParseFunctionCall(num, false);
			}

			if (SelfType)
			{
				field = CheckForField(Name, SelfType->aux_type);
				if (field && field->type->type == ev_method)
				{
					AddStatement(OPC_LocalAddress, 0);
					AddStatement(OPC_PushPointed);
					AddStatement(OPC_PushVFunc, field->ofs);
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

		if (SelfType)
		{
			field = CheckForField(Name, SelfType->aux_type);
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

		type = CheckForType(Name);
		if (type)
		{
			if (type->type != ev_class)
			{
				ParseError(ERR_ILLEGAL_EXPR_IDENT, "Identifier: %s", *Name);
				break;
			}
			AddStatement(OPC_PushClassId, type->classid);
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
			field = ParseField(type);
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
				type = op.type->aux_type;
				field = ParseField(type);
				if (field)
				{
					if (field->type->type == ev_method)
					{
						TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
						AddStatement(OPC_PushVFunc, field->ofs);
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
		   	else if (op.type->type == ev_array || op.type->type == ev_pointer)
			{
			   	ParseError(ERR_BAD_EXPR);
			}
			else
			{
				type = op.type;
			   	op = GetAddress(op);
				field = ParseField(type);
				if (field)
				{
					AddStatement(OPC_PushNumber, field->ofs);
					AddStatement(OPC_Add);
					op = EmitPushPointed(field->type);
				}
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
			TypeCheck2(ind.type);
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
	TOperator*	oper;
	TType*		type;

	if (tk_Token == TK_PUNCT)
	{
		if (TK_Check(PU_PLUS))
		{
			op = ParseExpressionPriority2();
			oper = FindOperator(TOperator::ID_UNARYPLUS, op.type, &type_void);
			op = TTree(oper->type);
			return op;
		}

		if (TK_Check(PU_MINUS))
		{
			op = ParseExpressionPriority2();
			oper = FindOperator(TOperator::ID_UNARYMINUS, op.type, &type_void);
			if (oper)
				AddStatement(oper->opcode);
			if (op.type->type == ev_vector)
				type = op.type;
			else
				type = oper->type;
			op = TTree(type);
			return op;
		}

		if (TK_Check(PU_NOT))
		{
			op = ParseExpressionPriority2();
			oper = FindOperator(TOperator::ID_NEGATELOGICAL, op.type, &type_void);
			if (oper)
				AddStatement(oper->opcode);
			op = TTree(oper->type);
			return op;
		}

		if (TK_Check(PU_TILDE))
		{
			op = ParseExpressionPriority2();
			oper = FindOperator(TOperator::ID_BITINVERSE, op.type, &type_void);
			if (oper)
				AddStatement(oper->opcode);
			op = TTree(oper->type);
			return op;
		}

		if (TK_Check(PU_AND))
		{
			op = ParseExpressionPriority1();
			if (op.type->type == ev_reference)
			{
				ParseError("Tried to take address of reference");
				op = TTree(MakePointerType(op.type->aux_type));
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
			if (op.type->type == ev_pointer &&
				op.type->aux_type->type == ev_class)
			{
				op = TTree(MakeReferenceType(op.type->aux_type));
				return op;
			}
			if (op.type->type == ev_reference)
			{
				ParseError("* applied on a reference");
				return op;
			}
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
			oper = FindOperator(TOperator::ID_PREINC, type, &type_void);
			if (oper)
				AddStatement(oper->opcode);
			op = TTree(oper->type);
			return op;
		}

		if (TK_Check(PU_DEC))
		{
			op = ParseExpressionPriority2();
			type = op.type;
			op = GetAddress(op);
			oper = FindOperator(TOperator::ID_PREDEC, type, &type_void);
			if (oper)
				AddStatement(oper->opcode);
			op = TTree(oper->type);
			return op;
		}
	}

	op = ParseExpressionPriority1();

	if (TK_Check(PU_INC))
	{
		type = op.type;
		op = GetAddress(op);
		oper = FindOperator(TOperator::ID_POSTINC, type, &type_void);
		if (oper)
			AddStatement(oper->opcode);
		op = TTree(oper->type);
		return op;
	}

	if (TK_Check(PU_DEC))
	{
		type = op.type;
		op = GetAddress(op);
		oper = FindOperator(TOperator::ID_POSTDEC, type, &type_void);
		if (oper)
			AddStatement(oper->opcode);
		op = TTree(oper->type);
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
	TTree		op1;
	TTree		op2;
	TOperator*	oper;
	bool		done;

	op1 = ParseExpressionPriority2();
	done = false;
	do
	{
		if (TK_Check(PU_ASTERISK))
		{
			op2 = ParseExpressionPriority2();
			oper = FindOperator(TOperator::ID_MULTIPLY, op1.type, op2.type);
			if (oper)
			{
				AddStatement(oper->opcode);
				op1 = TTree(oper->type);
			}
		}
		else if (TK_Check(PU_SLASH))
		{
			op2 = ParseExpressionPriority2();
			oper = FindOperator(TOperator::ID_DIVIDE, op1.type, op2.type);
			if (oper)
			{
				AddStatement(oper->opcode);
				op1 = TTree(oper->type);
			}
		}
		else if (TK_Check(PU_PERCENT))
		{
			op2 = ParseExpressionPriority2();
			oper = FindOperator(TOperator::ID_MODULUS, op1.type, op2.type);
			if (oper)
			{
				AddStatement(oper->opcode);
				op1 = TTree(oper->type);
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
	TTree		op1;
	TTree		op2;
	TOperator*	oper;
	bool		done;

	op1 = ParseExpressionPriority3();
	done = false;
	do
	{
		if (TK_Check(PU_PLUS))
		{
			op2 = ParseExpressionPriority3();
			oper = FindOperator(TOperator::ID_ADD, op1.type, op2.type);
			if (oper)
			{
				AddStatement(oper->opcode);
				op1 = TTree(oper->type);
			}
		}
		else if (TK_Check(PU_MINUS))
		{
			op2 = ParseExpressionPriority3();
			oper = FindOperator(TOperator::ID_SUBTRACT, op1.type, op2.type);
			if (oper)
			{
				AddStatement(oper->opcode);
				op1 = TTree(oper->type);
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
	TTree		op1;
	TTree		op2;
	TOperator*	oper;
	bool		done;

	op1 = ParseExpressionPriority4();
	done = false;
	do
	{
		if (TK_Check(PU_LSHIFT))
		{
			op2 = ParseExpressionPriority4();
			oper = FindOperator(TOperator::ID_LSHIFT, op1.type, op2.type);
			if (oper)
			{
				AddStatement(oper->opcode);
				op1 = TTree(oper->type);
			}
		}
		else if (TK_Check(PU_RSHIFT))
		{
			op2 = ParseExpressionPriority4();
			oper = FindOperator(TOperator::ID_RSHIFT, op1.type, op2.type);
			if (oper)
			{
				AddStatement(oper->opcode);
				op1 = TTree(oper->type);
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
	TTree		op1;
	TTree		op2;
	TOperator*	oper;
	bool		done;

	op1 = ParseExpressionPriority5();
	done = false;
	do
	{
		if (TK_Check(PU_LT))
		{
			op2 = ParseExpressionPriority5();
			oper = FindOperator(TOperator::ID_LT, op1.type, op2.type);
			if (oper)
			{
				AddStatement(oper->opcode);
				op1 = TTree(oper->type);
			}
		}
		else if (TK_Check(PU_LE))
		{
			op2 = ParseExpressionPriority5();
			oper = FindOperator(TOperator::ID_LE, op1.type, op2.type);
			if (oper)
			{
				AddStatement(oper->opcode);
				op1 = TTree(oper->type);
			}
		}
		else if (TK_Check(PU_GT))
		{
			op2 = ParseExpressionPriority5();
			oper = FindOperator(TOperator::ID_GT, op1.type, op2.type);
			if (oper)
			{
				AddStatement(oper->opcode);
				op1 = TTree(oper->type);
			}
		}
		else if (TK_Check(PU_GE))
		{
			op2 = ParseExpressionPriority5();
			oper = FindOperator(TOperator::ID_GE, op1.type, op2.type);
			if (oper)
			{
				AddStatement(oper->opcode);
				op1 = TTree(oper->type);
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
	TTree		op1;
	TTree		op2;
	TOperator*	oper;
	bool		done;

	op1 = ParseExpressionPriority6();
	done = false;
	do
	{
   		if (TK_Check(PU_EQ))
		{
   			op2 = ParseExpressionPriority6();
			oper = FindOperator(TOperator::ID_EQ, op1.type, op2.type);
			if (oper)
			{
				AddStatement(oper->opcode);
				op1 = TTree(oper->type);
			}
		}
   		else if (TK_Check(PU_NE))
		{
   			op2 = ParseExpressionPriority6();
			oper = FindOperator(TOperator::ID_NE, op1.type, op2.type);
			if (oper)
			{
				AddStatement(oper->opcode);
				op1 = TTree(oper->type);
			}
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
	TTree		op1;
	TTree		op2;
	TOperator*	oper;

	op1 = ParseExpressionPriority7();
	while (TK_Check(PU_AND))
	{
		op2 = ParseExpressionPriority7();
		oper = FindOperator(TOperator::ID_ANDBITWISE, op1.type, op2.type);
		if (oper)
 		{
 			AddStatement(oper->opcode);
			op1 = TTree(oper->type);
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
	TTree		op1;
	TTree		op2;
	TOperator*	oper;

	op1 = ParseExpressionPriority8();
	while (TK_Check(PU_XOR))
	{
		op2 = ParseExpressionPriority8();
		oper = FindOperator(TOperator::ID_XORBITWISE, op1.type, op2.type);
		if (oper)
 		{
 			AddStatement(oper->opcode);
			op1 = TTree(oper->type);
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
	TTree		op1;
	TTree		op2;
	TOperator*	oper;

	op1 = ParseExpressionPriority9();
	while (TK_Check(PU_OR))
	{
		op2 = ParseExpressionPriority9();
		oper = FindOperator(TOperator::ID_ORBITWISE, op1.type, op2.type);
		if (oper)
 		{
 			AddStatement(oper->opcode);
			op1 = TTree(oper->type);
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
	TTree			op1;
	TTree			op2;

	op1 = ParseExpressionPriority10();
	while (TK_Check(PU_AND_LOG))
	{
		int*		jmppos;

		TypeCheck1(op1.type);
		jmppos = AddStatement(OPC_IfNotTopGoto, 0);
		op2 = ParseExpressionPriority10();
		TypeCheck1(op2.type);
		AddStatement(OPC_AndLogical);
		*jmppos = CodeBufferSize;
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
	TTree			op1;
	TTree			op2;

	op1 = ParseExpressionPriority11();
	while (TK_Check(PU_OR_LOG))
	{
		int*		jmppos;

		TypeCheck1(op1.type);
		jmppos = AddStatement(OPC_IfTopGoto, 0);
		op2 = ParseExpressionPriority11();
		TypeCheck1(op2.type);
		AddStatement(OPC_OrLogical);
		*jmppos = CodeBufferSize;
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
	   	int*	jumppos1;
	   	int*	jumppos2;

		TypeCheck1(op.type);
		jumppos1 = AddStatement(OPC_IfNotGoto, 0);
		op1 = ParseExpressionPriority13();
		TK_Expect(PU_COLON, ERR_MISSING_COLON);
		jumppos2 = AddStatement(OPC_Goto, 0);
		*jumppos1 = CodeBufferSize;
		op2 = ParseExpressionPriority13();
		*jumppos2 = CodeBufferSize;
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
	int			i;
	static const struct
	{
		EPunctuation		punct;
		TOperator::id_t		opid;
	} AssignOps[] =
	{
		{PU_ASSIGN,				TOperator::ID_ASSIGN},
		{PU_ADD_ASSIGN,			TOperator::ID_ADDVAR},
		{PU_MINUS_ASSIGN,		TOperator::ID_SUBVAR},
		{PU_MULTIPLY_ASSIGN,	TOperator::ID_MULVAR},
		{PU_DIVIDE_ASSIGN,		TOperator::ID_DIVVAR},
		{PU_MOD_ASSIGN,			TOperator::ID_MODVAR},
		{PU_AND_ASSIGN,			TOperator::ID_ANDVAR},
		{PU_OR_ASSIGN,			TOperator::ID_ORVAR},
		{PU_XOR_ASSIGN,			TOperator::ID_XORVAR},
		{PU_LSHIFT_ASSIGN,		TOperator::ID_LSHIFTVAR},
		{PU_RSHIFT_ASSIGN,		TOperator::ID_RSHIFTVAR}
	};
	TOperator*	oper;
	TTree		op1;
	TTree		op2;
	TType*		type;

	op1 = ParseExpressionPriority13();
	for (i = 0; i < 11; i++)
	{
		if (TK_Check(AssignOps[i].punct))
		{
			type = op1.RealType;
			op1 = GetAddress(op1);
			op2 = ParseExpressionPriority13();
			oper = FindOperator(AssignOps[i].opid, type, op2.type);
			TypeCheck3(op2.type, type);
			if (oper)
			{
				if (oper->opcode == OPC_AssignBool)
					AddStatement(oper->opcode, type->bit_mask);
				else
					AddStatement(oper->opcode);
			}
			op1 = TTree(type);
			return op1;
		}
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

} // namespace Pass2

//**************************************************************************
//
//	$Log$
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
