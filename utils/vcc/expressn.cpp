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

enum ETreeType
{
	TREE_Unknown,
	TREE_Constant,
};

class TTree
{
public:
	TTree(ETreeType Aoperation = TREE_Unknown, TTree *Achild1 = NULL, TTree *Achild2 = NULL, TTree *Acond = NULL)
		: operation(Aoperation), child1(Achild1), child2(Achild2),
			cond(Acond), link(NULL), oper(NULL)
	{}
	TTree(TOperator *Aoper, TTree *Achild1 = NULL, TTree *Achild2 = NULL, TTree *Acond = NULL)
		: operation(TREE_Unknown), child1(Achild1), child2(Achild2),
			cond(Acond), link(NULL), oper(Aoper)
	{}
	virtual ~TTree(void)
	{
		if (child1)
			delete child1;
		if (child2)
			delete child2;
		if (cond)
			delete cond;
		if (link)
			delete link;
	}
	virtual TTree *GetAddress(void)
	{
		ParseError("Invalid address operation");
		return this;
	}
	virtual void Code(void)
	{
	}
	void AddToList(TTree *element)
	{
		if (!list)
		{
			list = element;
		}
		else
		{
			TTree *prev = list;
			while (prev->link)
				prev = prev->link;
			prev->link = element;
		}
	}
	void CodeList(void)
	{
		for (TTree *tree = list; tree; tree = tree->link)
		{
			tree->Code();
		}
	}

	ETreeType operation;
	TTree *child1;
	TTree *child2;
	union
	{
		TTree *cond;
		TTree *list;
	};
	TTree *link;
	union
	{
		int vInt;
		float vFloat;
		int vOffs;
	};

	TType *type;
	TOperator *oper;
};

class TOp1 : public	TTree
{
 public:
	TOp1(TTree *Aop, TOperator *Aoper) : TTree(Aoper)
	{
		child1 = Aop;
		if (child1->type->type == ev_vector)
			type = child1->type;
		else
			type = oper->type;
	}
	void Code(void)
	{
		if (child1) child1->Code();
		if (oper && oper->opcode != OPC_DONE) AddStatement(oper->opcode);
	}
};

class TOp2 : public	TTree
{
 public:
	TOp2(TTree *Aop1, TTree *Aop2, TOperator *Aoper) : TTree(Aoper)
	{
		child1 = Aop1;
		child2 = Aop2;
		if ((oper->type == &type_void_ptr && child1->type->type == ev_pointer) ||
			(oper->type == &type_none_ref && child1->type->type == ev_reference))
			type = child1->type;
		else
			type = oper->type;
	}
	TOp2(TTree *Aop1, TTree *Aop2)
	{
		child1 = Aop1;
		child2 = Aop2;
		type = child1->type;
	}
	void Code(void)
	{
		if (child1) child1->Code();
		if (child2) child2->Code();
		if (oper) AddStatement(oper->opcode);
	}
};

class TOpAnd : public TOp2
{
 public:
	TOpAnd(TTree *Aop1, TTree *Aop2) : TOp2(Aop1, Aop2)
	{
		TypeCheck1(child1->type);
		TypeCheck1(child2->type);
		type = &type_int;
	}
	void Code(void)
	{
		int*		jmppos;

		if (child1) child1->Code();
		jmppos = AddStatement(OPC_IFNOTTOPGOTO, 0);
		if (child2) child2->Code();
		AddStatement(OPC_ANDLOGICAL);
		*jmppos = CodeBufferSize;
	}
};

class TOpOr : public TOp2
{
 public:
	TOpOr(TTree *Aop1, TTree *Aop2) : TOp2(Aop1, Aop2)
	{
		TypeCheck1(child1->type);
		TypeCheck1(child2->type);
		type = &type_int;
	}
	void Code(void)
	{
		int*		jmppos;

		if (child1) child1->Code();
		jmppos = AddStatement(OPC_IFTOPGOTO, 0);
		if (child2) child2->Code();
		AddStatement(OPC_ORLOGICAL);
		*jmppos = CodeBufferSize;
	}
};

class TOpCond : public   TTree
{
 public:
	TOpCond(TTree *Aexpr, TTree *Aop1, TTree *Aop2)
	{
		cond = Aexpr;
		child1 = Aop1;
		child2 = Aop2;
		TypeCheck1(cond->type);
		TypeCheck3(child1->type, child2->type);
		if (child1->type == &type_void_ptr)
			type = child2->type;
		else
			type = child1->type;
	}
	void Code(void)
	{
	   	int*	jumppos1;
	   	int*	jumppos2;

		if (cond) cond->Code();
		jumppos1 = AddStatement(OPC_IFNOTGOTO, 0);
		if (child1) child1->Code();
		jumppos2 = AddStatement(OPC_GOTO, 0);
		*jumppos1 = CodeBufferSize;
		if (child2) child2->Code();
		*jumppos2 = CodeBufferSize;
	}
};

class TOpPushPointed : public  TTree
{
 public:
	TOpPushPointed(TTree *Aop)
	{
		child1 = Aop;
		if (child1->type->type != ev_pointer)
		{
			ParseError("Expression syntax error");
			type = child1->type;
		}
		else
		{
			type = child1->type->aux_type;
		}
	}
	TOpPushPointed(TTree *Aop, TType *Atype)
	{
		child1 = Aop;
		type = Atype;
	}
	void Code(void)
	{
		if (child1) child1->Code();
		if (type->type == ev_vector)
		{
			AddStatement(OPC_VPUSHPOINTED);
		}
		else
		{
			AddStatement(OPC_PUSHPOINTED);
		}
	}
	TTree *GetAddress(void)
	{
		TTree *tmp;
		tmp = child1;
		child1 = NULL;
		delete this;
		return tmp;
	}
};

class TOpArrayIndex : public TTree
{
 public:
	TOpArrayIndex(TTree *Aop1, TTree *Aop2, TType *Atype)
	{
		child1 = Aop1;
		child2 = Aop2;
		type = MakePointerType(Atype);
//		type_size = TypeSize(Atype);
	}
	void Code(void)
	{
		if (child1) child1->Code();
		if (child2) child2->Code();
		AddStatement(OPC_PUSHNUMBER, TypeSize(type->aux_type));
		AddStatement(OPC_MULTIPLY);
		AddStatement(OPC_ADD);
	}
};

class TOpField : public	TTree
{
 public:
	TOpField(TTree *Aop, int Aoffs, TType *Atype)
	{
		vOffs = Aoffs;
		child1 = Aop;
		type = MakePointerType(Atype);
	}
	void Code(void)
	{
		if (child1) child1->Code();
		AddStatement(OPC_PUSHNUMBER, vOffs);
		AddStatement(OPC_ADD);
	}
};

class TOpConst : public	TTree
{
 public:
	TOpConst(int Aval, TType *Atype)
	{
		vInt = Aval;
		type = Atype;
	}
	void Code(void)
	{
		if (type->type == ev_string)
			AddStatement(OPC_PUSHSTRING, vInt);
		else
			AddStatement(OPC_PUSHNUMBER, vInt);
	}
};

class TOpLocal : public	TTree
{
 public:
	TOpLocal(int Aoffs, TType *Atype)
	{
		vOffs = Aoffs;
		type = Atype;
	}
	~TOpLocal(void)
	{
	}
	void Code(void)
	{
		AddStatement(OPC_LOCALADDRESS, vOffs);
	}
};

class TOpGlobal : public TTree
{
 public:
	TOpGlobal(int Aoffs, TType *Atype)
	{
		vOffs = Aoffs;
		type = Atype;
	}
	~TOpGlobal(void)
	{
	}
	void Code(void)
	{
		AddStatement(OPC_GLOBALADDRESS, vOffs);
	}
};

class TOpFuncCall : public TTree
{
 public:
	TOpFuncCall(int Afnum)
	{
		vOffs = Afnum;
		type = functions[Afnum].type->aux_type;
	}
	void Code(void)
	{
		CodeList();
		AddStatement(OPC_CALL, vOffs);
	}
};

class TOpIndirectFuncCall : public   TTree
{
 public:
	TOpIndirectFuncCall(TTree *Afnumop, TType *Aftype)
	{
		child1 = Afnumop;
		type = Aftype->aux_type;
	}
	void Code(void)
	{
		if (child1)
			child1->Code();
		for (TTree *tree = list; tree; tree = tree->link)
		{
			tree->Code();
			if (tree->type->type == ev_vector)
				AddStatement(OPC_SWAP3);
			else
				AddStatement(OPC_SWAP);
		}
		AddStatement(OPC_ICALL);
	}
};

class TOpVector : public TTree
{
 public:
	TOpVector(TTree *Aop1, TTree *Aop2, TTree *Aop3)
	{
		if (Aop1->type != &type_float)
			ParseError("Expression type mistmatch, vector param 1 is not a float");
		if (Aop2->type != &type_float)
			ParseError("Expression type mistmatch, vector param 2 is not a float");
		if (Aop3->type != &type_float)
			ParseError("Expression type mistmatch, vector param 3 is not a float");
		type = &type_vector;
		AddToList(Aop1);
		AddToList(Aop2);
		AddToList(Aop3);
	}
	void Code(void)
	{
		CodeList();
	}
};

class TOpPushThis:public TTree
{
 public:
	TOpPushThis(void)
	{
		type = ThisType;
	}
	void Code(void)
	{
		AddStatement(OPC_LOCALADDRESS, 0);
		AddStatement(OPC_PUSHPOINTED);
	}
};

class TOpPushSelf:public TTree
{
 public:
	TOpPushSelf(void)
	{
		type = SelfType;
	}
	void Code(void)
	{
		AddStatement(OPC_LOCALADDRESS, 0);
		AddStatement(OPC_PUSHPOINTED);
	}
};

class TOpPushSelfMethod:public TTree
{
 public:
	TOpPushSelfMethod(TTree *Aop, int Aoffs, TType *Atype)
	{
		vOffs = Aoffs;
		child1 = Aop;
		type = Atype;
	}
	void Code(void)
	{
		if (child1) child1->Code();
		AddStatement(OPC_COPY);
		AddStatement(OPC_PUSHNUMBER, VTABLE_OFFS);
		AddStatement(OPC_ADD);
		AddStatement(OPC_PUSHPOINTED);
		AddStatement(OPC_PUSHNUMBER, vOffs * 4);
		AddStatement(OPC_ADD);
		AddStatement(OPC_PUSHPOINTED);
	}
};

class TOpDynamicCast:public TTree
{
public:
	TOpDynamicCast(TTree *Aop, TType *Atype)
	{
		child1 = Aop;
		vInt = Atype->classid;
		type = MakeReferenceType(Atype);
	}
	void Code(void)
	{
		if (child1)
			child1->Code();
		AddStatement(OPC_DYNAMIC_CAST, vInt);
	}
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static TTree *ParseExpressionPriority2(void);
static TTree* ParseExpressionPriority14(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TOperator	*operators[TOperator::NUM_OPERATORS];

static TOperator	UnaryPlus_int(TOperator::ID_UNARYPLUS, &type_int, &type_int, &type_void, OPC_DONE);
static TOperator	UnaryPlus_uint(TOperator::ID_UNARYPLUS, &type_uint, &type_uint, &type_void, OPC_DONE);
static TOperator	UnaryPlus_float(TOperator::ID_UNARYPLUS, &type_float, &type_float, &type_void, OPC_DONE);

static TOperator	UnaryMinus_int(TOperator::ID_UNARYMINUS, &type_int, &type_int, &type_void, OPC_UNARYMINUS);
static TOperator	UnaryMinus_uint(TOperator::ID_UNARYMINUS, &type_uint, &type_uint, &type_void, OPC_UNARYMINUS);
static TOperator	UnaryMinus_float(TOperator::ID_UNARYMINUS, &type_float, &type_float, &type_void, OPC_FUNARYMINUS);
static TOperator	UnaryMinus_vector(TOperator::ID_UNARYMINUS, &type_vector, &type_vector, &type_void, OPC_VUNARYMINUS);

static TOperator	NotLogical_int(TOperator::ID_NEGATELOGICAL, &type_int, &type_int, &type_void, OPC_NEGATELOGICAL);
static TOperator	NotLogical_uint(TOperator::ID_NEGATELOGICAL, &type_int, &type_uint, &type_void, OPC_NEGATELOGICAL);
static TOperator	NotLogical_float(TOperator::ID_NEGATELOGICAL, &type_int, &type_float, &type_void, OPC_NEGATELOGICAL);
static TOperator	NotLogical_str(TOperator::ID_NEGATELOGICAL, &type_int, &type_string, &type_void, OPC_NEGATELOGICAL);
static TOperator	NotLogical_func(TOperator::ID_NEGATELOGICAL, &type_int, &type_function, &type_void, OPC_NEGATELOGICAL);
static TOperator	NotLogical_ptr(TOperator::ID_NEGATELOGICAL, &type_int, &type_void_ptr, &type_void, OPC_NEGATELOGICAL);
static TOperator	NotLogical_ref(TOperator::ID_NEGATELOGICAL, &type_int, &type_none_ref, &type_void, OPC_NEGATELOGICAL);

static TOperator	BitInverse_int(TOperator::ID_BITINVERSE, &type_int, &type_int, &type_void, OPC_BITINVERSE);
static TOperator	BitInverse_uint(TOperator::ID_BITINVERSE, &type_uint, &type_uint, &type_void, OPC_BITINVERSE);

static TOperator	PreInc_int(TOperator::ID_PREINC, &type_int, &type_int, &type_void, OPC_PREINC);
static TOperator	PreInc_uint(TOperator::ID_PREINC, &type_uint, &type_uint, &type_void, OPC_PREINC);

static TOperator	PreDec_int(TOperator::ID_PREDEC, &type_int, &type_int, &type_void, OPC_PREDEC);
static TOperator	PreDec_uint(TOperator::ID_PREDEC, &type_uint, &type_uint, &type_void, OPC_PREDEC);

static TOperator	PostInc_int(TOperator::ID_POSTINC, &type_int, &type_int, &type_void, OPC_POSTINC);
static TOperator	PostInc_uint(TOperator::ID_POSTINC, &type_uint, &type_uint, &type_void, OPC_POSTINC);

static TOperator	PostDec_int(TOperator::ID_POSTDEC, &type_int, &type_int, &type_void, OPC_POSTDEC);
static TOperator	PostDec_uint(TOperator::ID_POSTDEC, &type_uint, &type_uint, &type_void, OPC_POSTDEC);

static TOperator	Mul_int_int(TOperator::ID_MULTIPLY, &type_int, &type_int, &type_int, OPC_MULTIPLY);
static TOperator	Mul_int_uint(TOperator::ID_MULTIPLY, &type_uint, &type_int, &type_uint, OPC_MULTIPLY);
static TOperator	Mul_uint_int(TOperator::ID_MULTIPLY, &type_uint, &type_uint, &type_int, OPC_MULTIPLY);
static TOperator	Mul_uint_uint(TOperator::ID_MULTIPLY, &type_uint, &type_uint, &type_uint, OPC_MULTIPLY);
static TOperator	Mul_float_float(TOperator::ID_MULTIPLY, &type_float, &type_float, &type_float, OPC_FMULTIPLY);
static TOperator	Mul_vec_float(TOperator::ID_MULTIPLY, &type_vector, &type_vector, &type_float, OPC_VPOSTSCALE);
static TOperator	Mul_float_vec(TOperator::ID_MULTIPLY, &type_vector, &type_float, &type_vector, OPC_VPRESCALE);

static TOperator	Div_int_int(TOperator::ID_DIVIDE, &type_int, &type_int, &type_int, OPC_DIVIDE);
static TOperator	Div_int_uint(TOperator::ID_DIVIDE, &type_uint, &type_int, &type_uint, OPC_UDIVIDE);
static TOperator	Div_uint_int(TOperator::ID_DIVIDE, &type_uint, &type_uint, &type_int, OPC_UDIVIDE);
static TOperator	Div_uint_uint(TOperator::ID_DIVIDE, &type_uint, &type_uint, &type_uint, OPC_UDIVIDE);
static TOperator	Div_float_float(TOperator::ID_DIVIDE, &type_float, &type_float, &type_float, OPC_FDIVIDE);
static TOperator	Div_vec_float(TOperator::ID_DIVIDE, &type_vector, &type_vector, &type_float, OPC_VISCALE);

static TOperator	Mod_int_int(TOperator::ID_MODULUS, &type_int, &type_int, &type_int, OPC_MODULUS);
static TOperator	Mod_int_uint(TOperator::ID_MODULUS, &type_uint, &type_int, &type_uint, OPC_UMODULUS);
static TOperator	Mod_uint_int(TOperator::ID_MODULUS, &type_uint, &type_uint, &type_int, OPC_UMODULUS);
static TOperator	Mod_uint_uint(TOperator::ID_MODULUS, &type_uint, &type_uint, &type_uint, OPC_UMODULUS);

static TOperator	Add_int_int(TOperator::ID_ADD, &type_int, &type_int, &type_int, OPC_ADD);
static TOperator	Add_int_uint(TOperator::ID_ADD, &type_uint, &type_int, &type_uint, OPC_ADD);
static TOperator	Add_uint_int(TOperator::ID_ADD, &type_uint, &type_uint, &type_int, OPC_ADD);
static TOperator	Add_uint_uint(TOperator::ID_ADD, &type_uint, &type_uint, &type_uint, OPC_ADD);
static TOperator	Add_float_float(TOperator::ID_ADD, &type_float, &type_float, &type_float, OPC_FADD);
static TOperator	Add_vec_vec(TOperator::ID_ADD, &type_vector, &type_vector, &type_vector, OPC_VADD);

static TOperator	Sub_int_int(TOperator::ID_SUBTRACT, &type_int, &type_int, &type_int, OPC_SUBTRACT);
static TOperator	Sub_int_uint(TOperator::ID_SUBTRACT, &type_uint, &type_int, &type_uint, OPC_SUBTRACT);
static TOperator	Sub_uint_int(TOperator::ID_SUBTRACT, &type_uint, &type_uint, &type_int, OPC_SUBTRACT);
static TOperator	Sub_uint_uint(TOperator::ID_SUBTRACT, &type_uint, &type_uint, &type_uint, OPC_SUBTRACT);
static TOperator	Sub_float_float(TOperator::ID_SUBTRACT, &type_float, &type_float, &type_float, OPC_FSUBTRACT);
static TOperator	Sub_vec_vec(TOperator::ID_SUBTRACT, &type_vector, &type_vector, &type_vector, OPC_VSUBTRACT);

static TOperator	LShift_int_int(TOperator::ID_LSHIFT, &type_int, &type_int, &type_int, OPC_LSHIFT);
static TOperator	LShift_int_uint(TOperator::ID_LSHIFT, &type_int, &type_int, &type_uint, OPC_LSHIFT);
static TOperator	LShift_uint_int(TOperator::ID_LSHIFT, &type_uint, &type_uint, &type_int, OPC_LSHIFT);
static TOperator	LShift_uint_uint(TOperator::ID_LSHIFT, &type_uint, &type_uint, &type_uint, OPC_LSHIFT);

static TOperator	RShift_int_int(TOperator::ID_RSHIFT, &type_int, &type_int, &type_int, OPC_RSHIFT);
static TOperator	RShift_int_uint(TOperator::ID_RSHIFT, &type_int, &type_int, &type_uint, OPC_RSHIFT);
static TOperator	RShift_uint_int(TOperator::ID_RSHIFT, &type_uint, &type_uint, &type_int, OPC_URSHIFT);
static TOperator	RShift_uint_uint(TOperator::ID_RSHIFT, &type_uint, &type_uint, &type_uint, OPC_URSHIFT);

static TOperator	Lt_int_int(TOperator::ID_LT, &type_int, &type_int, &type_int, OPC_LT);
static TOperator	Lt_int_uint(TOperator::ID_LT, &type_int, &type_int, &type_uint, OPC_ULT);
static TOperator	Lt_uint_int(TOperator::ID_LT, &type_int, &type_uint, &type_int, OPC_ULT);
static TOperator	Lt_uint_uint(TOperator::ID_LT, &type_int, &type_uint, &type_uint, OPC_ULT);
static TOperator	Lt_float_float(TOperator::ID_LT, &type_int, &type_float, &type_float, OPC_FLT);

static TOperator	Le_int_int(TOperator::ID_LE, &type_int, &type_int, &type_int, OPC_LE);
static TOperator	Le_int_uint(TOperator::ID_LE, &type_int, &type_int, &type_uint, OPC_ULE);
static TOperator	Le_uint_int(TOperator::ID_LE, &type_int, &type_uint, &type_int, OPC_ULE);
static TOperator	Le_uint_uint(TOperator::ID_LE, &type_int, &type_uint, &type_uint, OPC_ULE);
static TOperator	Le_float_float(TOperator::ID_LE, &type_int, &type_float, &type_float, OPC_FLE);

static TOperator	Gt_int_int(TOperator::ID_GT, &type_int, &type_int, &type_int, OPC_GT);
static TOperator	Gt_int_uint(TOperator::ID_GT, &type_int, &type_int, &type_uint, OPC_UGT);
static TOperator	Gt_uint_int(TOperator::ID_GT, &type_int, &type_uint, &type_int, OPC_UGT);
static TOperator	Gt_uint_uint(TOperator::ID_GT, &type_int, &type_uint, &type_uint, OPC_UGT);
static TOperator	Gt_float_float(TOperator::ID_GT, &type_int, &type_float, &type_float, OPC_FGT);

static TOperator	Ge_int_int(TOperator::ID_GE, &type_int, &type_int, &type_int, OPC_GE);
static TOperator	Ge_int_uint(TOperator::ID_GE, &type_int, &type_int, &type_uint, OPC_UGE);
static TOperator	Ge_uint_int(TOperator::ID_GE, &type_int, &type_uint, &type_int, OPC_UGE);
static TOperator	Ge_uint_uint(TOperator::ID_GE, &type_int, &type_uint, &type_uint, OPC_UGE);
static TOperator	Ge_float_float(TOperator::ID_GE, &type_int, &type_float, &type_float, OPC_FGE);

static TOperator	Eq_int_int(TOperator::ID_EQ, &type_int, &type_int, &type_int, OPC_EQ);
static TOperator	Eq_int_uint(TOperator::ID_EQ, &type_int, &type_int, &type_uint, OPC_EQ);
static TOperator	Eq_uint_int(TOperator::ID_EQ, &type_int, &type_uint, &type_int, OPC_EQ);
static TOperator	Eq_uint_uint(TOperator::ID_EQ, &type_int, &type_uint, &type_uint, OPC_EQ);
static TOperator	Eq_float_float(TOperator::ID_EQ, &type_int, &type_float, &type_float, OPC_FEQ);
static TOperator	Eq_str_str(TOperator::ID_EQ, &type_int, &type_string, &type_string, OPC_EQ);
static TOperator	Eq_func_func(TOperator::ID_EQ, &type_int, &type_function, &type_function, OPC_EQ);
static TOperator	Eq_ptr_ptr(TOperator::ID_EQ, &type_int, &type_void_ptr, &type_void_ptr, OPC_EQ);
static TOperator	Eq_vec_vec(TOperator::ID_EQ, &type_int, &type_vector, &type_vector, OPC_VEQ);
static TOperator	Eq_cid_cid(TOperator::ID_EQ, &type_int, &type_classid, &type_classid, OPC_EQ);
static TOperator	Eq_ref_ref(TOperator::ID_EQ, &type_int, &type_none_ref, &type_none_ref, OPC_EQ);

static TOperator	Ne_int_int(TOperator::ID_NE, &type_int, &type_int, &type_int, OPC_NE);
static TOperator	Ne_int_uint(TOperator::ID_NE, &type_int, &type_int, &type_uint, OPC_NE);
static TOperator	Ne_uint_int(TOperator::ID_NE, &type_int, &type_uint, &type_int, OPC_NE);
static TOperator	Ne_uint_uint(TOperator::ID_NE, &type_int, &type_uint, &type_uint, OPC_NE);
static TOperator	Ne_float_float(TOperator::ID_NE, &type_int, &type_float, &type_float, OPC_FNE);
static TOperator	Ne_str_str(TOperator::ID_NE, &type_int, &type_string, &type_string, OPC_NE);
static TOperator	Ne_func_func(TOperator::ID_NE, &type_int, &type_function, &type_function, OPC_NE);
static TOperator	Ne_ptr_ptr(TOperator::ID_NE, &type_int, &type_void_ptr, &type_void_ptr, OPC_NE);
static TOperator	Ne_vec_vec(TOperator::ID_NE, &type_int, &type_vector, &type_vector, OPC_VNE);
static TOperator	Ne_cid_cid(TOperator::ID_NE, &type_int, &type_classid, &type_classid, OPC_NE);
static TOperator	Ne_ref_ref(TOperator::ID_NE, &type_int, &type_none_ref, &type_none_ref, OPC_NE);

static TOperator	And_int_int(TOperator::ID_ANDBITWISE, &type_int, &type_int, &type_int, OPC_ANDBITWISE);
static TOperator	And_int_uint(TOperator::ID_ANDBITWISE, &type_uint, &type_int, &type_uint, OPC_ANDBITWISE);
static TOperator	And_uint_int(TOperator::ID_ANDBITWISE, &type_uint, &type_uint, &type_int, OPC_ANDBITWISE);
static TOperator	And_uint_uint(TOperator::ID_ANDBITWISE, &type_uint, &type_uint, &type_uint, OPC_ANDBITWISE);

static TOperator	Xor_int_int(TOperator::ID_XORBITWISE, &type_int, &type_int, &type_int, OPC_XORBITWISE);
static TOperator	Xor_int_uint(TOperator::ID_XORBITWISE, &type_uint, &type_int, &type_uint, OPC_XORBITWISE);
static TOperator	Xor_uint_int(TOperator::ID_XORBITWISE, &type_uint, &type_uint, &type_int, OPC_XORBITWISE);
static TOperator	Xor_uint_uint(TOperator::ID_XORBITWISE, &type_uint, &type_uint, &type_uint, OPC_XORBITWISE);

static TOperator	Or_int_int(TOperator::ID_ORBITWISE, &type_int, &type_int, &type_int, OPC_ORBITWISE);
static TOperator	Or_int_uint(TOperator::ID_ORBITWISE, &type_uint, &type_int, &type_uint, OPC_ORBITWISE);
static TOperator	Or_uint_int(TOperator::ID_ORBITWISE, &type_uint, &type_uint, &type_int, OPC_ORBITWISE);
static TOperator	Or_uint_uint(TOperator::ID_ORBITWISE, &type_uint, &type_uint, &type_uint, OPC_ORBITWISE);

static TOperator	Assign_int_int(TOperator::ID_ASSIGN, &type_int, &type_int, &type_int, OPC_ASSIGN);
static TOperator	Assign_int_uint(TOperator::ID_ASSIGN, &type_uint, &type_int, &type_uint, OPC_ASSIGN);
static TOperator	Assign_uint_int(TOperator::ID_ASSIGN, &type_uint, &type_uint, &type_int, OPC_ASSIGN);
static TOperator	Assign_uint_uint(TOperator::ID_ASSIGN, &type_uint, &type_uint, &type_uint, OPC_ASSIGN);
static TOperator	Assign_float_float(TOperator::ID_ASSIGN, &type_float, &type_float, &type_float, OPC_ASSIGN);
static TOperator	Assign_str_str(TOperator::ID_ASSIGN, &type_string, &type_string, &type_string, OPC_ASSIGN);
static TOperator	Assign_func_func(TOperator::ID_ASSIGN, &type_function, &type_function, &type_function, OPC_ASSIGN);
static TOperator	Assign_ptr_ptr(TOperator::ID_ASSIGN, &type_void_ptr, &type_void_ptr, &type_void_ptr, OPC_ASSIGN);
static TOperator	Assign_vec_vec(TOperator::ID_ASSIGN, &type_vector, &type_vector, &type_vector, OPC_VASSIGN);
static TOperator	Assign_cid_cid(TOperator::ID_ASSIGN, &type_classid, &type_classid, &type_classid, OPC_ASSIGN);
static TOperator	Assign_ref_ref(TOperator::ID_ASSIGN, &type_none_ref, &type_none_ref, &type_none_ref, OPC_ASSIGN);

static TOperator	AddVar_int_int(TOperator::ID_ADDVAR, &type_int, &type_int, &type_int, OPC_ADDVAR);
static TOperator	AddVar_int_uint(TOperator::ID_ADDVAR, &type_uint, &type_int, &type_uint, OPC_ADDVAR);
static TOperator	AddVar_uint_int(TOperator::ID_ADDVAR, &type_uint, &type_uint, &type_int, OPC_ADDVAR);
static TOperator	AddVar_uint_uint(TOperator::ID_ADDVAR, &type_uint, &type_uint, &type_uint, OPC_ADDVAR);
static TOperator	AddVar_float_float(TOperator::ID_ADDVAR, &type_float, &type_float, &type_float, OPC_FADDVAR);
static TOperator	AddVar_vec_vec(TOperator::ID_ADDVAR, &type_vector, &type_vector, &type_vector, OPC_VADDVAR);

static TOperator	SubVar_int_int(TOperator::ID_SUBVAR, &type_int, &type_int, &type_int, OPC_SUBVAR);
static TOperator	SubVar_int_uint(TOperator::ID_SUBVAR, &type_uint, &type_int, &type_uint, OPC_SUBVAR);
static TOperator	SubVar_uint_int(TOperator::ID_SUBVAR, &type_uint, &type_uint, &type_int, OPC_SUBVAR);
static TOperator	SubVar_uint_uint(TOperator::ID_SUBVAR, &type_uint, &type_uint, &type_uint, OPC_SUBVAR);
static TOperator	SubVar_float_float(TOperator::ID_SUBVAR, &type_float, &type_float, &type_float, OPC_FSUBVAR);
static TOperator	SubVar_vec_vec(TOperator::ID_SUBVAR, &type_vector, &type_vector, &type_vector, OPC_VSUBVAR);

static TOperator	MulVar_int_int(TOperator::ID_MULVAR, &type_int, &type_int, &type_int, OPC_MULVAR);
static TOperator	MulVar_int_uint(TOperator::ID_MULVAR, &type_uint, &type_int, &type_uint, OPC_MULVAR);
static TOperator	MulVar_uint_int(TOperator::ID_MULVAR, &type_uint, &type_uint, &type_int, OPC_MULVAR);
static TOperator	MulVar_uint_uint(TOperator::ID_MULVAR, &type_uint, &type_uint, &type_uint, OPC_MULVAR);
static TOperator	MulVar_float_float(TOperator::ID_MULVAR, &type_float, &type_float, &type_float, OPC_FMULVAR);
static TOperator	MulVar_vec_float(TOperator::ID_MULVAR, &type_vector, &type_vector, &type_float, OPC_VSCALEVAR);

static TOperator	DivVar_int_int(TOperator::ID_DIVVAR, &type_int, &type_int, &type_int, OPC_DIVVAR);
static TOperator	DivVar_int_uint(TOperator::ID_DIVVAR, &type_uint, &type_int, &type_uint, OPC_UDIVVAR);
static TOperator	DivVar_uint_int(TOperator::ID_DIVVAR, &type_uint, &type_uint, &type_int, OPC_UDIVVAR);
static TOperator	DivVar_uint_uint(TOperator::ID_DIVVAR, &type_uint, &type_uint, &type_uint, OPC_UDIVVAR);
static TOperator	DivVar_float_float(TOperator::ID_DIVVAR, &type_float, &type_float, &type_float, OPC_FDIVVAR);
static TOperator	DivVar_vec_float(TOperator::ID_DIVVAR, &type_vector, &type_vector, &type_float, OPC_VISCALEVAR);

static TOperator	ModVar_int_int(TOperator::ID_MODVAR, &type_int, &type_int, &type_int, OPC_MODVAR);
static TOperator	ModVar_int_uint(TOperator::ID_MODVAR, &type_uint, &type_int, &type_uint, OPC_UMODVAR);
static TOperator	ModVar_uint_int(TOperator::ID_MODVAR, &type_uint, &type_uint, &type_int, OPC_UMODVAR);
static TOperator	ModVar_uint_uint(TOperator::ID_MODVAR, &type_uint, &type_uint, &type_uint, OPC_UMODVAR);

static TOperator	AndVar_int_int(TOperator::ID_ANDVAR, &type_int, &type_int, &type_int, OPC_ANDVAR);
static TOperator	AndVar_int_uint(TOperator::ID_ANDVAR, &type_uint, &type_int, &type_uint, OPC_ANDVAR);
static TOperator	AndVar_uint_int(TOperator::ID_ANDVAR, &type_uint, &type_uint, &type_int, OPC_ANDVAR);
static TOperator	AndVar_uint_uint(TOperator::ID_ANDVAR, &type_uint, &type_uint, &type_uint, OPC_ANDVAR);

static TOperator	OrVar_int_int(TOperator::ID_ORVAR, &type_int, &type_int, &type_int, OPC_ORVAR);
static TOperator	OrVar_int_uint(TOperator::ID_ORVAR, &type_uint, &type_int, &type_uint, OPC_ORVAR);
static TOperator	OrVar_uint_int(TOperator::ID_ORVAR, &type_uint, &type_uint, &type_int, OPC_ORVAR);
static TOperator	OrVar_uint_uint(TOperator::ID_ORVAR, &type_uint, &type_uint, &type_uint, OPC_ORVAR);

static TOperator	XorVar_int_int(TOperator::ID_XORVAR, &type_int, &type_int, &type_int, OPC_XORVAR);
static TOperator	XorVar_int_uint(TOperator::ID_XORVAR, &type_uint, &type_int, &type_uint, OPC_XORVAR);
static TOperator	XorVar_uint_int(TOperator::ID_XORVAR, &type_uint, &type_uint, &type_int, OPC_XORVAR);
static TOperator	XorVar_uint_uint(TOperator::ID_XORVAR, &type_uint, &type_uint, &type_uint, OPC_XORVAR);

static TOperator	LShiftVar_int_int(TOperator::ID_LSHIFTVAR, &type_int, &type_int, &type_int, OPC_LSHIFTVAR);
static TOperator	LShiftVar_int_uint(TOperator::ID_LSHIFTVAR, &type_uint, &type_int, &type_uint, OPC_LSHIFTVAR);
static TOperator	LShiftVar_uint_int(TOperator::ID_LSHIFTVAR, &type_uint, &type_uint, &type_int, OPC_LSHIFTVAR);
static TOperator	LShiftVar_uint_uint(TOperator::ID_LSHIFTVAR, &type_uint, &type_uint, &type_uint, OPC_LSHIFTVAR);

static TOperator	RShiftVar_int_int(TOperator::ID_RSHIFTVAR, &type_int, &type_int, &type_int, OPC_RSHIFTVAR);
static TOperator	RShiftVar_int_uint(TOperator::ID_RSHIFTVAR, &type_uint, &type_int, &type_uint, OPC_URSHIFTVAR);
static TOperator	RShiftVar_uint_int(TOperator::ID_RSHIFTVAR, &type_uint, &type_uint, &type_int, OPC_URSHIFTVAR);
static TOperator	RShiftVar_uint_uint(TOperator::ID_RSHIFTVAR, &type_uint, &type_uint, &type_uint, OPC_URSHIFTVAR);

static TOperator	NullOp(TOperator::NUM_OPERATORS, &type_void, &type_void, &type_void, OPC_DONE);

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
//	TypeCmp
//
//==========================================================================

bool TypeCmp(TType *type1, TType *type2)
{
	if (type1 == type2)
	{
		return true;
	}
	if ((type1->type == ev_vector) && (type2->type == ev_vector))
	{
		return true;
	}
	if ((type1->type == ev_function) && (type2->type == ev_function))
	{
		ParseWarning("Different function types");
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
		if (oper->type1->type == ev_function &&
			oper->type2->type == ev_function &&
			type1->type == ev_function && type1 == type2)
		{
			return oper;
		}
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

static TTree *ParseFunctionCall(int num, bool is_method)
{
	TOpFuncCall *fop;
	int			arg;
	int			argsize;

	fop = new TOpFuncCall(num);
	arg = 0;
	argsize = 0;
	int max_params;
	int num_needed_params = functions[num].type->num_params & PF_COUNT_MASK;
	if (functions[num].type->num_params & PF_VARARGS)
	{
		max_params = MAX_ARG_COUNT - 1;
	}
	else
	{
		max_params = functions[num].type->num_params;
	}
	if (is_method)
	{
		fop->AddToList(new TOpPushThis());
	}
	if (!TK_Check(PU_RPAREN))
	{
		do
		{
			TTree *op = ParseExpressionPriority14();
			if (arg >= max_params)
			{
				ParseError("Incorrect number of arguments, need %d, got %d.", max_params, arg);
			}
			else
			{
				if (arg < num_needed_params)
				{
					TypeCheck3(op->type, functions[num].type->param_types[arg]);
				}
				fop->AddToList(op);
			}
			arg++;
			argsize += TypeSize(op->type);
		} while (TK_Check(PU_COMMA));
		TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	}
	if (arg < num_needed_params)
	{
		ParseError("Incorrect argument count %d, should be %d", arg, num_needed_params);
	}
	if (functions[num].type->num_params & PF_VARARGS)
	{
		fop->AddToList(new TOpConst(argsize / 4 - num_needed_params, &type_int));
	}
	return fop;
}

//==========================================================================
//
//	ParseExpressionPriority0
//
//==========================================================================

static TTree *ParseExpressionPriority0(void)
{
	TTree		*op;
	TType		*type;
	int			num;
	field_t		*field;

   	switch (tk_Token)
	{
	 case TK_INTEGER:
		op = new TOpConst(tk_Number, &type_int);
		TK_NextToken();
		return op;

	 case TK_FLOAT:
		op = new TOpConst(PassFloat(tk_Float), &type_float);
		TK_NextToken();
		return op;

	 case TK_STRING:
		op = new TOpConst(tk_StringI, &type_string);
		TK_NextToken();
		return op;

	 case TK_PUNCT:
	   	if (TK_Check(PU_LPAREN))
		{
			type = CheckForType();
			if (type)
			{
				while (TK_Check(PU_ASTERISK))
				{
					type = MakePointerType(type);
				}
				TK_Expect(PU_RPAREN, ERR_BAD_EXPR);
				op = ParseExpressionPriority2();
				op->type = type;
				return op;
			}

			op = ParseExpressionPriority14();
			TK_Expect(PU_RPAREN, ERR_BAD_EXPR);
			return op;
		}

		if (TK_Check(PU_DCOLON))
		{
			if (!ThisType)
			{
				ParseError(":: not in method");
				break;
			}
			field = CheckForField(ThisType->aux_type->aux_type);
			if (!field)
			{
				ParseError("No such method %s", tk_String);
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
			TTree *op1 = ParseExpressionPriority14();
			TK_Expect(PU_COMMA, ERR_BAD_EXPR);
			TTree *op2 = ParseExpressionPriority14();
			TK_Expect(PU_COMMA, ERR_BAD_EXPR);
			TTree *op3 = ParseExpressionPriority14();
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			return new TOpVector(op1, op2, op3);
		}
		if (TK_Check(KW_THIS))
		{
			if (!ThisType)
			{
				ParseError("this used outside member function\n");
			}
			else
			{
				op = new TOpPushThis();
				return op;
			}
		}
		if (TK_Check(KW_SELF))
		{
			if (!SelfType)
			{
				ParseError("self used outside member function\n");
			}
			else
			{
				op = new TOpPushSelf();
				return op;
			}
		}
		if (TK_Check(KW_NONE))
		{
		   	return new TOpConst(0, &type_none_ref);
		}
		if (TK_Check(KW_NULL))
		{
		   	return new TOpConst(0, &type_void_ptr);
		}
		break;

	 case TK_IDENTIFIER:
		num = CheckForLocalVar(tk_StringI);
		if (num)
		{
			TK_NextToken();
			op = new TOpLocal(localdefs[num].ofs, MakePointerType(localdefs[num].type));
			op = new TOpPushPointed(op,	localdefs[num].type);
			return op;
		}

		num = CheckForConstant(tk_StringI);
		if (num != -1)
		{
			TK_NextToken();
		   	return new TOpConst(Constants[num].value, &type_int);
		}

		num = CheckForGlobalVar(tk_StringI);
		if (num)
		{
			TK_NextToken();
			op = new TOpGlobal(num, MakePointerType(globaldefs[num].type));
			op = new TOpPushPointed(op,	globaldefs[num].type);
			return op;
		}

		num = CheckForFunction(tk_StringI);
		if (num)
		{
			TK_NextToken();
			if (!TK_Check(PU_LPAREN))
			{
				op = new TOpConst(num, functions[num].type);
				return op;
			}
			return ParseFunctionCall(num, false);
		}

		if (ThisType)
		{
			field = CheckForField(ThisType->aux_type);
			if (field)
			{
				op = new TOpPushThis();
				if (field->type->type == ev_method)
				{
					op = new TOpPushSelfMethod(op, field->ofs, field->type);
				}
				else
				{
					op = new TOpField(op, field->ofs, field->type);
					op = new TOpPushPointed(op,	field->type);
				}
				return op;
			}
		}

		type = CheckForType();
		if (type)
		{
			if (type->type != ev_class)
			{
				ParseError(ERR_ILLEGAL_EXPR_IDENT, "Identifier: %s", tk_String);
				break;
			}
			if (TK_Check(PU_LPAREN))
			{
				op = ParseExpressionPriority14();
				if (op->type->type != ev_reference)
				{
					ParseError(ERR_BAD_EXPR, "Class reference required");
				}
				TK_Expect(PU_RPAREN, ERR_BAD_EXPR);
				return new TOpDynamicCast(op, type);
			}
		   	return new TOpConst(type->classid, &type_classid);
		}

		ERR_Exit(ERR_ILLEGAL_EXPR_IDENT, true, "Identifier: %s", tk_String);
		break;

	 default:
	   	break;
	}

	op = new TTree;
	op->type = &type_void;
	return op;
}

//==========================================================================
//
//	ParseExpressionPriority1
//
//==========================================================================

static TTree *ParseExpressionPriority1(void)
{
	bool		done;
	TTree			*op;
	TType		*type;
	field_t		*field;

   	op = ParseExpressionPriority0();
	done = false;
	do
	{
   		if (TK_Check(PU_MINUS_GT))
   	   	{
		   	TypeCheck1(op->type);
			if (op->type->type != ev_pointer)
			{
   				ERR_Exit(ERR_BAD_EXPR, true, NULL);
		   	}
			type = op->type->aux_type;
			field = ParseField(type);
			if (field)
			{
				if (field->type->type == ev_method)
				{
					op = new TOpPushSelfMethod(op, field->ofs, field->type);
				}
				else
				{
					op = new TOpField(op, field->ofs, field->type);
					op = new TOpPushPointed(op,	field->type);
				}
			}
   		}
   		else if (TK_Check(PU_DOT))
   	   	{
			if (op->type->type == ev_reference)
			{
				type = op->type->aux_type;
				field = ParseField(type);
				if (field)
				{
					if (field->type->type == ev_method)
					{
						op = new TOpPushSelfMethod(op, field->ofs, field->type);
					}
					else
					{
						op = new TOpField(op, field->ofs, field->type);
						op = new TOpPushPointed(op,	field->type);
					}
				}
			}
		   	else if (op->type->type == ev_array || op->type->type == ev_pointer)
			{
			   	ParseError(ERR_BAD_EXPR);
			}
			else
			{
				type = op->type;
			   	op = op->GetAddress();
				field = ParseField(type);
				if (field)
				{
					op = new TOpField(op, field->ofs, field->type);
					op = new TOpPushPointed(op,	field->type);
				}
			}
   		}
		else if (TK_Check(PU_LINDEX))
		{
			TTree *ind;
			if (op->type->type == ev_array)
			{
				type = op->type->aux_type;
				op = op->GetAddress();
			}
			else if (op->type->type == ev_pointer)
			{
			   	type = op->type->aux_type;
			}
			else
			{
				ERR_Exit(ERR_BAD_ARRAY, true, NULL);
		   	}
			ind = ParseExpressionPriority14();
			TK_Expect(PU_RINDEX, ERR_BAD_ARRAY);
			op = new TOpArrayIndex(op, ind, type);
			op = new TOpPushPointed(op, type);
		}
		else if (TK_Check(PU_LPAREN))
		{
			TOpIndirectFuncCall *fop;
			TType *ftype = op->type;

			if (ftype->type != ev_function && ftype->type != ev_method)
			{
				ParseError("Not a function");
				return op;
			}
			fop = new TOpIndirectFuncCall(op, ftype);
			int arg = 0;
			if (!TK_Check(PU_RPAREN))
			{
				do
				{
					op = ParseExpressionPriority14();
					if (arg >= ftype->num_params)
					{
						ParseError("Incorrect number of arguments.");
					}
					else
					{
						TypeCheck3(op->type, ftype->param_types[arg]);
						fop->AddToList(op);
					}
					arg++;
				} while (TK_Check(PU_COMMA));
				TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			}
			if (arg != ftype->num_params)
			{
				ParseError("Incorrect argument count %d, should be %d", arg, ftype->num_params);
			}
			return fop;
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

static TTree *ParseExpressionPriority2(void)
{
	TTree			*op;
	TOperator	*oper;
	TType		*type;

   	if (tk_Token == TK_PUNCT)
	{
		if (TK_Check(PU_PLUS))
		{
		   	op = ParseExpressionPriority2();
			oper = FindOperator(TOperator::ID_UNARYPLUS, op->type, &type_void);
			return new TOp1(op, oper);
		}

	   	if (TK_Check(PU_MINUS))
		{
		   	op = ParseExpressionPriority2();
			oper = FindOperator(TOperator::ID_UNARYMINUS, op->type, &type_void);
			return new TOp1(op, oper);
		}

	   	if (TK_Check(PU_NOT))
		{
		   	op = ParseExpressionPriority2();
			oper = FindOperator(TOperator::ID_NEGATELOGICAL, op->type, &type_void);
			return new TOp1(op, oper);
		}

	   	if (TK_Check(PU_TILDE))
		{
		   	op = ParseExpressionPriority2();
			oper = FindOperator(TOperator::ID_BITINVERSE, op->type, &type_void);
			return new TOp1(op, oper);
		}

		if (TK_Check(PU_AND))
		{
		   	op = ParseExpressionPriority1();
			if (op->type->type == ev_reference)
			{
				op->type = MakePointerType(op->type->aux_type);
			}
			else
			{
				type = MakePointerType(op->type);
				op = op->GetAddress();
				op->type = type;
			}
			return op;
		}
		if (TK_Check(PU_ASTERISK))
		{
		   	op = ParseExpressionPriority2();
			if (op->type->type == ev_pointer &&
				op->type->aux_type->type == ev_class)
			{
				op->type = MakeReferenceType(op->type->aux_type);
				return op;
			}
			if (op->type->type == ev_reference)
			{
				ParseError("* applied on a reference");
				return op;
			}
			return new TOpPushPointed(op);
		}

	   	if (TK_Check(PU_INC))
		{
		   	op = ParseExpressionPriority2();
			type = op->type;
			op = op->GetAddress();
			oper = FindOperator(TOperator::ID_PREINC, type, &type_void);
			return new TOp1(op, oper);
		}

	   	if (TK_Check(PU_DEC))
		{
		   	op = ParseExpressionPriority2();
			type = op->type;
			op = op->GetAddress();
			oper = FindOperator(TOperator::ID_PREDEC, type, &type_void);
			return new TOp1(op, oper);
		}
	}

	op = ParseExpressionPriority1();

	if (TK_Check(PU_INC))
	{
		type = op->type;
		op = op->GetAddress();
		oper = FindOperator(TOperator::ID_POSTINC, type, &type_void);
		return new TOp1(op, oper);
	}

	if (TK_Check(PU_DEC))
	{
		type = op->type;
		op = op->GetAddress();
		oper = FindOperator(TOperator::ID_POSTDEC, type, &type_void);
		return new TOp1(op, oper);
	}

	return op;
}

//==========================================================================
//
//	ParseExpressionPriority3
//
//==========================================================================

static TTree *ParseExpressionPriority3(void)
{
	TTree			*op1;
	TTree			*op2;
	TOperator	*oper;
	bool		done;

	op1 = ParseExpressionPriority2();
	done = false;
	do
	{
   		if (TK_Check(PU_ASTERISK))
		{
   			op2 = ParseExpressionPriority2();
			oper = FindOperator(TOperator::ID_MULTIPLY, op1->type, op2->type);
			op1 = new TOp2(op1, op2, oper);
		}
   		else if (TK_Check(PU_SLASH))
		{
   			op2 = ParseExpressionPriority2();
			oper = FindOperator(TOperator::ID_DIVIDE, op1->type, op2->type);
			op1 = new TOp2(op1, op2, oper);
		}
   		else if (TK_Check(PU_PERCENT))
		{
   			op2 = ParseExpressionPriority2();
			oper = FindOperator(TOperator::ID_MODULUS, op1->type, op2->type);
			op1 = new TOp2(op1, op2, oper);
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

static TTree *ParseExpressionPriority4(void)
{
	TTree			*op1;
	TTree			*op2;
	TOperator	*oper;
	bool		done;

	op1 = ParseExpressionPriority3();
	done = false;
	do
	{
   		if (TK_Check(PU_PLUS))
		{
   			op2 = ParseExpressionPriority3();
			oper = FindOperator(TOperator::ID_ADD, op1->type, op2->type);
			op1 = new TOp2(op1, op2, oper);
		}
   		else if (TK_Check(PU_MINUS))
		{
   			op2 = ParseExpressionPriority3();
			oper = FindOperator(TOperator::ID_SUBTRACT, op1->type, op2->type);
			op1 = new TOp2(op1, op2, oper);
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

static TTree *ParseExpressionPriority5(void)
{
	TTree			*op1;
	TTree			*op2;
	TOperator	*oper;
	bool		done;

	op1 = ParseExpressionPriority4();
	done = false;
	do
	{
   		if (TK_Check(PU_LSHIFT))
		{
   			op2 = ParseExpressionPriority4();
			oper = FindOperator(TOperator::ID_LSHIFT, op1->type, op2->type);
			op1 = new TOp2(op1, op2, oper);
		}
   		else if (TK_Check(PU_RSHIFT))
		{
   			op2 = ParseExpressionPriority4();
			oper = FindOperator(TOperator::ID_RSHIFT, op1->type, op2->type);
			op1 = new TOp2(op1, op2, oper);
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

static TTree* ParseExpressionPriority6(void)
{
	TTree			*op1;
	TTree			*op2;
	TOperator	*oper;
	bool		done;

	op1 = ParseExpressionPriority5();
	done = false;
	do
	{
   		if (TK_Check(PU_LT))
		{
   			op2 = ParseExpressionPriority5();
			oper = FindOperator(TOperator::ID_LT, op1->type, op2->type);
			op1 = new TOp2(op1, op2, oper);
		}
   		else if (TK_Check(PU_LE))
		{
   			op2 = ParseExpressionPriority5();
			oper = FindOperator(TOperator::ID_LE, op1->type, op2->type);
			op1 = new TOp2(op1, op2, oper);
		}
   		else if (TK_Check(PU_GT))
		{
   			op2 = ParseExpressionPriority5();
			oper = FindOperator(TOperator::ID_GT, op1->type, op2->type);
			op1 = new TOp2(op1, op2, oper);
		}
   		else if (TK_Check(PU_GE))
		{
   			op2 = ParseExpressionPriority5();
			oper = FindOperator(TOperator::ID_GE, op1->type, op2->type);
			op1 = new TOp2(op1, op2, oper);
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

static TTree* ParseExpressionPriority7(void)
{
	TTree			*op1;
	TTree			*op2;
	TOperator	*oper;
	bool		done;

	op1 = ParseExpressionPriority6();
	done = false;
	do
	{
   		if (TK_Check(PU_EQ))
		{
   			op2 = ParseExpressionPriority6();
			oper = FindOperator(TOperator::ID_EQ, op1->type, op2->type);
			op1 = new TOp2(op1, op2, oper);
		}
   		else if (TK_Check(PU_NE))
		{
   			op2 = ParseExpressionPriority6();
			oper = FindOperator(TOperator::ID_NE, op1->type, op2->type);
			op1 = new TOp2(op1, op2, oper);
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

static TTree* ParseExpressionPriority8(void)
{
	TTree			*op1;
	TTree			*op2;
	TOperator	*oper;

	op1 = ParseExpressionPriority7();
	while (TK_Check(PU_AND))
	{
		op2 = ParseExpressionPriority7();
		oper = FindOperator(TOperator::ID_ANDBITWISE, op1->type, op2->type);
		op1 = new TOp2(op1, op2, oper);
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority9
//
//==========================================================================

static TTree* ParseExpressionPriority9(void)
{
	TTree			*op1;
	TTree			*op2;
	TOperator	*oper;

	op1 = ParseExpressionPriority8();
	while (TK_Check(PU_XOR))
	{
		op2 = ParseExpressionPriority8();
		oper = FindOperator(TOperator::ID_XORBITWISE, op1->type, op2->type);
		op1 = new TOp2(op1, op2, oper);
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority10
//
//==========================================================================

static TTree* ParseExpressionPriority10(void)
{
	TTree			*op1;
	TTree			*op2;
	TOperator	*oper;

	op1 = ParseExpressionPriority9();
	while (TK_Check(PU_OR))
	{
		op2 = ParseExpressionPriority9();
		oper = FindOperator(TOperator::ID_ORBITWISE, op1->type, op2->type);
		op1 = new TOp2(op1, op2, oper);
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority11
//
//==========================================================================

static TTree* ParseExpressionPriority11(void)
{
	TTree			*op1;
	TTree			*op2;

	op1 = ParseExpressionPriority10();
	while (TK_Check(PU_AND_LOG))
	{
		op2 = ParseExpressionPriority10();
		op1 = new TOpAnd(op1, op2);
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority12
//
//==========================================================================

static TTree* ParseExpressionPriority12(void)
{
	TTree			*op1;
	TTree			*op2;

	op1 = ParseExpressionPriority11();
	while (TK_Check(PU_OR_LOG))
	{
		op2 = ParseExpressionPriority11();
		op1 = new TOpOr(op1, op2);
	}
	return op1;
}

//==========================================================================
//
//	ParseExpressionPriority13
//
//==========================================================================

static TTree* ParseExpressionPriority13(void)
{
	TTree			*op;
	TTree			*op1;
	TTree			*op2;

	op = ParseExpressionPriority12();
   	if (TK_Check(PU_QUEST))
	{
		op1 = ParseExpressionPriority13();
		TK_Expect(PU_COLON, ERR_MISSING_COLON);
		op2 = ParseExpressionPriority13();
		op = new TOpCond(op, op1, op2);
	}
	return op;
}

//==========================================================================
//
//	ParseExpressionPriority14
//
//==========================================================================

static TTree* ParseExpressionPriority14(void)
{
	int			i;
	static const struct
	{
		Punctuation			punct;
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
	TOperator	*oper;
	TTree			*op1;
	TTree			*op2;
	TType		*type;

	op1 = ParseExpressionPriority13();
	for (i = 0; i < 11; i++)
   	{
		if (TK_Check(AssignOps[i].punct))
		{
			type = op1->type;
			op1 = op1->GetAddress();
   			op2 = ParseExpressionPriority14();
			oper = FindOperator(AssignOps[i].opid, type, op2->type);
			TypeCheck3(op2->type, type);
		   	op1 = new TOp2(op1, op2, oper);
			op1->type = type;
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

TType *ParseExpression(void)
{
	TTree *op = ParseExpressionPriority14();
	op->Code();
	TType *t = op->type;
	delete op;
	return t;
}

//**************************************************************************
//
//	$Log$
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
