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

class VExpression
{
public:
	TType		Type;
	TType		RealType;
	int			Flags;
	TLocation	Loc;
	bool		bDidResolve;	//FIXME

	VExpression(const TLocation& ALoc)
	: Type(ev_void)
	, RealType(ev_void)
	, Flags(0)
	, Loc(ALoc)
	, bDidResolve(false)
	{
	}
	virtual ~VExpression()
	{
	}
	virtual void EmitCode() = 0;
	virtual VExpression* DoResolve() = 0;
	VExpression* Resolve()
	{
		if (bDidResolve)
			return this;
		VExpression* e = DoResolve();
		if (e)
			e->bDidResolve = true;
		return e;
	}
	VExpression* ResolveTopLevel();
	virtual void RequestAddressOf()
	{
		ParseError(Loc, "Bad address operation");
	}
	void EmitPushPointedCode(TType type)
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
	}
	virtual bool IsSingleName() { return false; }
};

class VIntLiteral : public VExpression
{
public:
	vint32		Value;

	VIntLiteral(vint32 AValue, const TLocation& ALoc)
	: VExpression(ALoc)
	, Value(AValue)
	{
	}
	VExpression* DoResolve()
	{
		Type = ev_int;
		return this;
	}
	void EmitCode()
	{
		EmitPushNumber(Value);
	}
};

class VFloatLiteral : public VExpression
{
public:
	float		Value;

	VFloatLiteral(float AValue, const TLocation& ALoc)
	: VExpression(ALoc)
	, Value(AValue)
	{
	}
	VExpression* DoResolve()
	{
		Type = ev_float;
		return this;
	}
	void EmitCode()
	{
		AddStatement(OPC_PushNumber, Value);
	}
};

class VNameLiteral : public VExpression
{
public:
	VName		Value;

	VNameLiteral(VName AValue, const TLocation& ALoc)
	: VExpression(ALoc)
	, Value(AValue)
	{
	}
	VExpression* DoResolve()
	{
		Type = ev_name;
		return this;
	}
	void EmitCode()
	{
		AddStatement(OPC_PushName, Value);
	}
};

class VStringLiteral : public VExpression
{
public:
	vint32		Value;

	VStringLiteral(vint32 AValue, const TLocation& ALoc)
	: VExpression(ALoc)
	, Value(AValue)
	{
	}
	VExpression* DoResolve()
	{
		Type = ev_string;
		return this;
	}
	void EmitCode()
	{
		AddStatement(OPC_PushString, Value);
	}
};

class VStateConstant : public VExpression
{
public:
	VState*		State;

	VStateConstant(VState* AState, const TLocation& ALoc)
	: VExpression(ALoc)
	, State(AState)
	{
	}
	VExpression* DoResolve()
	{
		Type = ev_state;
		return this;
	}
	void EmitCode()
	{
		AddStatement(OPC_PushState, State);
	}
};

class VClassConstant : public VExpression
{
public:
	VClass*		Class;

	VClassConstant(VClass* AClass, const TLocation& ALoc)
	: VExpression(ALoc)
	, Class(AClass)
	{
	}
	VExpression* DoResolve()
	{
		Type = ev_classid;
		return this;
	}
	void EmitCode()
	{
		AddStatement(OPC_PushClassId, Class);
	}
};

class VSelf : public VExpression
{
public:
	VSelf(const TLocation& ALoc)
	: VExpression(ALoc)
	{
	}
	VExpression* DoResolve()
	{
		Type = SelfType;
		if (!SelfClass)
		{
			ParseError(Loc, "self used outside member function\n");
		}
		return this;
	}
	void EmitCode()
	{
		AddStatement(OPC_LocalAddress0);
		AddStatement(OPC_PushPointedPtr);
	}
};

class VNoneLiteral : public VExpression
{
public:
	VNoneLiteral(const TLocation& ALoc)
	: VExpression(ALoc)
	{
	}
	VExpression* DoResolve()
	{
		Type = TType((VClass*)NULL);
		return this;
	}
	void EmitCode()
	{
		AddStatement(OPC_PushNull);
	}
};

class VNullLiteral : public VExpression
{
public:
	VNullLiteral(const TLocation& ALoc)
	: VExpression(ALoc)
	{
	}
	VExpression* DoResolve()
	{
		Type = MakePointerType(TType(ev_void));
		return this;
	}
	void EmitCode()
	{
		AddStatement(OPC_PushNull);
	}
};

class VConstantValue : public VExpression
{
public:
	VConstant*		Const;

	VConstantValue(VConstant* AConst, const TLocation& ALoc)
	: VExpression(ALoc)
	, Const(AConst)
	{
	}
	VExpression* DoResolve()
	{
		Type = (EType)Const->Type;
		return this;
	}
	void EmitCode()
	{
		EmitPushNumber(Const->value);
	}
};

class VPushPointed : public VExpression
{
public:
	VExpression*	op;		//	Value
	bool			AddressRequested;

	VPushPointed(VExpression* AOp)
	: VExpression(AOp->Loc)
	, op(AOp)
	, AddressRequested(false)
	{
		if (!op)
		{
			ParseError(Loc, "Expression expected");
			return;
		}
	}
	~VPushPointed()
	{
		if (op)
			delete op;
	}
	VExpression* DoResolve()
	{
		if (op)
			op = op->Resolve();
		if (!op)
		{
			delete this;
			return NULL;
		}

		if (op->Type.type != ev_pointer)
		{
			ParseError(Loc, "Expression syntax error");
			delete this;
			return NULL;
		}
		Type = op->Type.GetPointerInnerType();
		RealType = Type;
		if (Type.type == ev_bool)
		{
			Type = TType(ev_int);
		}
		return this;
	}
	void RequestAddressOf()
	{
		if (RealType.type == ev_void)
		{
			ParseError(Loc, "Bad address operation");
			return;
		}
		if (AddressRequested)
			ParseError(Loc, "Multiple address of");
		AddressRequested = true;
	}
	void EmitCode()
	{
		op->EmitCode();
		if (!AddressRequested)
		{
			EmitPushPointedCode(RealType);
		}
	}
};

class VVector : public VExpression
{
public:
	VExpression*	op1;
	VExpression*	op2;
	VExpression*	op3;

	VVector(VExpression* AOp1, VExpression* AOp2, VExpression* AOp3, const TLocation& ALoc)
	: VExpression(ALoc)
	, op1(AOp1)
	, op2(AOp2)
	, op3(AOp3)
	{
		if (!op1)
		{
			ParseError(Loc, "Expression expected");
		}
		if (!op2)
		{
			ParseError(Loc, "Expression expected");
		}
		if (!op3)
		{
			ParseError(Loc, "Expression expected");
		}
	}
	~VVector()
	{
		if (op1)
			delete op1;
		if (op2)
			delete op2;
		if (op3)
			delete op3;
	}
	VExpression* DoResolve()
	{
		if (op1)
			op1 = op1->Resolve();
		if (op2)
			op2 = op2->Resolve();
		if (op3)
			op3 = op3->Resolve();
		if (!op1 || !op2 || !op3)
		{
			delete this;
			return NULL;
		}

		if (op1->Type.type != ev_float)
		{
			ParseError(Loc, "Expression type mistmatch, vector param 1 is not a float");
			delete this;
			return NULL;
		}
		if (op2->Type.type != ev_float)
		{
			ParseError(Loc, "Expression type mistmatch, vector param 2 is not a float");
			delete this;
			return NULL;
		}
		if (op3->Type.type != ev_float)
		{
			ParseError(Loc, "Expression type mistmatch, vector param 3 is not a float");
			delete this;
			return NULL;
		}

		Type = ev_vector;
		return this;
	}
	void EmitCode()
	{
		op1->EmitCode();
		op2->EmitCode();
		op3->EmitCode();
	}
};

class VDynamicCast : public VExpression
{
public:
	VClass*				Class;
	VExpression*		op;

	VDynamicCast(VClass* AClass, VExpression* AOp, const TLocation& ALoc)
	: VExpression(ALoc)
	, Class(AClass)
	, op(AOp)
	{
	}
	~VDynamicCast()
	{
		if (op)
			delete op;
	}
	VExpression* DoResolve()
	{
		if (op)
			op = op->Resolve();
		if (!op)
		{
			delete this;
			return NULL;
		}

		if (op->Type.type != ev_reference)
		{
			ParseError(Loc, "Bad expression, class reference required");
			delete this;
			return NULL;
		}
		Type = TType(Class);
		return this;
	}
	void EmitCode()
	{
		op->EmitCode();
		AddStatement(OPC_DynamicCast, Class);
	}
};

class VLocalVar : public VExpression
{
public:
	int				num;
	bool			AddressRequested;

	VLocalVar(int ANum, const TLocation& ALoc)
	: VExpression(ALoc)
	, num(ANum)
	, AddressRequested(false)
	{
	}
	VExpression* DoResolve()
	{
		Type = localdefs[num].type;
		RealType = localdefs[num].type;
		if (Type.type == ev_bool)
		{
			Type = TType(ev_int);
		}
		return this;
	}
	void RequestAddressOf()
	{
		if (AddressRequested)
			ParseError(Loc, "Multiple address of");
		AddressRequested = true;
	}
	void EmitCode()
	{
		EmitLocalAddress(localdefs[num].ofs);
		if (!AddressRequested)
		{
			EmitPushPointedCode(localdefs[num].type);
		}
	}
};

class VFieldAccess : public VExpression
{
public:
	VExpression*		op;
	VField*				field;
	bool				AddressRequested;

	VFieldAccess(VExpression* AOp, VField* AField, const TLocation& ALoc, int ExtraFlags)
	: VExpression(ALoc)
	, op(AOp)
	, field(AField)
	, AddressRequested(false)
	{
		Flags = field->flags | ExtraFlags;
	}
	~VFieldAccess()
	{
		if (op)
			delete op;
	}
	VExpression* DoResolve()
	{
		if (op)
			op = op->Resolve();
		if (!op)
		{
			delete this;
			return NULL;
		}

		Type = field->type;
		RealType = field->type;
		if (Type.type == ev_bool)
		{
			Type = TType(ev_int);
		}
		return this;
	}
	void RequestAddressOf()
	{
		if (Flags & FIELD_ReadOnly)
		{
			ParseError(op->Loc, "Tried to assign to a read-only field");
		}
		if (AddressRequested)
			ParseError(Loc, "Multiple address of");
		AddressRequested = true;
	}
	void EmitCode()
	{
		op->EmitCode();
		AddStatement(OPC_Offset, field);
		if (!AddressRequested)
		{
			EmitPushPointedCode(field->type);
		}
	}
};

class VArrayElement : public VExpression
{
public:
	VExpression*		op;
	VExpression*		ind;
	bool				AddressRequested;

	VArrayElement(VExpression* AOp, VExpression* AInd, const TLocation& ALoc)
	: VExpression(ALoc)
	, op(AOp)
	, ind(AInd)
	, AddressRequested(false)
	{
		if (!ind)
		{
			ParseError(Loc, "Expression expected");
			return;
		}
	}
	~VArrayElement()
	{
		if (op)
			delete op;
		if (ind)
			delete ind;
	}
	VExpression* DoResolve()
	{
		if (op)
			op = op->Resolve();
		if (ind)
			ind = ind->Resolve();
		if (!op || !ind)
		{
			delete this;
			return NULL;
		}

		if (ind->Type.type != ev_int)
		{
			ParseError(Loc, "Array index must be of integer type");
			delete this;
			return NULL;
		}
		if (op->Type.type == ev_array)
		{
			Flags = op->Flags;
			Type = op->Type.GetArrayInnerType();
			op->Flags &= ~FIELD_ReadOnly;
			op->RequestAddressOf();
		}
		else if (op->Type.type == ev_pointer)
		{
			Flags = 0;
			Type = op->Type.GetPointerInnerType();
		}
		else
		{
			ParseError(Loc, "Bad operation with array");
			delete this;
			return NULL;
		}

		RealType = Type;
		if (Type.type == ev_bool)
		{
			Type = TType(ev_int);
		}
		return this;
	}
	void RequestAddressOf()
	{
		if (Flags & FIELD_ReadOnly)
		{
			ParseError(op->Loc, "Tried to assign to a read-only field");
		}
		if (AddressRequested)
			ParseError(Loc, "Multiple address of");
		AddressRequested = true;
	}
	void EmitCode()
	{
		op->EmitCode();
		ind->EmitCode();
		AddStatement(OPC_ArrayElement, RealType);
		if (!AddressRequested)
		{
			EmitPushPointedCode(RealType);
		}
	}
};

class VDelegateVal : public VExpression
{
public:
	VExpression*		op;
	VMethod*			M;

	VDelegateVal(VExpression* AOp, VMethod* AM, const TLocation& ALoc)
	: VExpression(ALoc)
	, op(AOp)
	, M(AM)
	{
	}
	~VDelegateVal()
	{
		if (op)
			delete op;
	}
	VExpression* DoResolve()
	{
		if (op)
			op = op->Resolve();
		if (!op)
		{
			delete this;
			return NULL;
		}

		Type = ev_delegate;
		Type.Function = M;
		return this;
	}
	void EmitCode()
	{
		op->EmitCode();
		AddStatement(OPC_PushVFunc, M);
	}
};

class VUnary : public VExpression
{
public:
	EPunctuation		Oper;	//	Operation
	VExpression*		op;		//	Value

	VUnary(EPunctuation AOper, VExpression* AOp, const TLocation& ALoc)
	: VExpression(ALoc)
	, Oper(AOper)
	, op(AOp)
	{
		if (!op)
		{
			ParseError(Loc, "Expression expected");
			return;
		}
	}
	~VUnary()
	{
		if (op)
			delete op;
	}
	VExpression* DoResolve()
	{
		if (op)
			op = op->Resolve();
		if (!op)
		{
			delete this;
			return NULL;
		}

		switch (Oper)
		{
		case PU_PLUS:
			Type = op->Type;
			if (op->Type.type != ev_int && op->Type.type != ev_float)
			{
				ParseError(Loc, "Expression type mistmatch");
				delete this;
				return NULL;
			}
			break;
		case PU_MINUS:
			if (op->Type.type == ev_int)
			{
				Type = ev_int;
			}
			else if (op->Type.type == ev_float)
			{
				Type = ev_float;
			}
			else if (op->Type.type == ev_vector)
			{
				Type = op->Type;
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
				delete this;
				return NULL;
			}
			break;
		case PU_NOT:
			op->Type.CheckSizeIs4(Loc);
			Type = ev_int;
			break;
		case PU_TILDE:
			if (op->Type.type != ev_int)
			{
				ParseError(Loc, "Expression type mistmatch");
				delete this;
				return NULL;
			}
			Type = ev_int;
			break;
		case PU_AND:
			if (op->Type.type == ev_reference)
			{
				ParseError(Loc, "Tried to take address of reference");
				delete this;
				return NULL;
			}
			else
			{
				op->RequestAddressOf();
				Type = MakePointerType(op->RealType);
			}
			break;
		default:
			break;
		}
		return this;
	}
	void EmitCode()
	{
		op->EmitCode();

		switch (Oper)
		{
		case PU_PLUS:
			break;

		case PU_MINUS:
			if (op->Type.type == ev_int)
			{
				AddStatement(OPC_UnaryMinus);
			}
			else if (op->Type.type == ev_float)
			{
				AddStatement(OPC_FUnaryMinus);
			}
			else if (op->Type.type == ev_vector)
			{
				AddStatement(OPC_VUnaryMinus);
			}
			break;

		case PU_NOT:
			op->Type.EmitToBool();
			AddStatement(OPC_NegateLogical);
			break;

		case PU_TILDE:
			AddStatement(OPC_BitInverse);
			break;

		default:
			break;
		}
	}
};

enum EIncDec
{
	INCDEC_PreInc,
	INCDEC_PreDec,
	INCDEC_PostInc,
	INCDEC_PostDec,
};

class VUnaryMutator : public VExpression
{
public:
	EIncDec			Oper;	//	Operation
	VExpression*	op;		//	Value

	VUnaryMutator(EIncDec AOper, VExpression* AOp, const TLocation& ALoc)
	: VExpression(ALoc)
	, Oper(AOper)
	, op(AOp)
	{
		if (!op)
		{
			ParseError(Loc, "Expression expected");
			return;
		}
	}
	~VUnaryMutator()
	{
		if (op)
			delete op;
	}
	VExpression* DoResolve()
	{
		if (op)
			op = op->Resolve();
		if (!op)
		{
			delete this;
			return NULL;
		}

		if (op->Type.type != ev_int)
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		Type = ev_int;
		op->RequestAddressOf();
		return this;
	}
	void EmitCode()
	{
		op->EmitCode();
		switch (Oper)
		{
		case INCDEC_PreInc:
			AddStatement(OPC_PreInc);
			break;

		case INCDEC_PreDec:
			AddStatement(OPC_PreDec);
			break;

		case INCDEC_PostInc:
			AddStatement(OPC_PostInc);
			break;

		case INCDEC_PostDec:
			AddStatement(OPC_PostDec);
			break;
		}
	}
};

class VBinary : public VExpression
{
public:
	EPunctuation		Oper;	//	Operation
	VExpression*		op1;	//	Variable
	VExpression*		op2;	//	Value

	VBinary(EPunctuation AOper, VExpression* AOp1, VExpression* AOp2, const TLocation& ALoc)
	: VExpression(ALoc)
	, Oper(AOper)
	, op1(AOp1)
	, op2(AOp2)
	{
		if (!op2)
		{
			ParseError(Loc, "Expression expected");
			return;
		}
	}
	~VBinary()
	{
		if (op1)
			delete op1;
		if (op2)
			delete op2;
	}
	VExpression* DoResolve()
	{
		if (op1)
			op1 = op1->Resolve();
		if (op2)
			op2 = op2->Resolve();
		if (!op1 || !op2)
		{
			delete this;
			return NULL;
		}

		switch (Oper)
		{
		case PU_ASTERISK:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				Type = ev_int;
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
			{
				Type = ev_float;
			}
			else if (op1->Type.type == ev_vector && op2->Type.type == ev_float)
			{
				Type = ev_vector;
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_vector)
			{
				Type = ev_vector;
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
				delete this;
				return NULL;
			}
			break;
		case PU_SLASH:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				Type = ev_int;
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
			{
				Type = ev_float;
			}
			else if (op1->Type.type == ev_vector && op2->Type.type == ev_float)
			{
				Type = ev_vector;
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
				delete this;
				return NULL;
			}
			break;
		case PU_PLUS:
		case PU_MINUS:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				Type = ev_int;
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
			{
				Type = ev_float;
			}
			else if (op1->Type.type == ev_vector && op2->Type.type == ev_vector)
			{
				Type = ev_vector;
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
				delete this;
				return NULL;
			}
			break;
		case PU_PERCENT:
		case PU_LSHIFT:
		case PU_RSHIFT:
		case PU_AND:
		case PU_XOR:
		case PU_OR:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				Type = ev_int;
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
				delete this;
				return NULL;
			}
			break;
		case PU_LT:
		case PU_LE:
		case PU_GT:
		case PU_GE:
			if (!(op1->Type.type == ev_int && op2->Type.type == ev_int) &&
				!(op1->Type.type == ev_float && op2->Type.type == ev_float))
			{
				ParseError(Loc, "Expression type mistmatch");
				delete this;
				return NULL;
			}
			Type = ev_int;
			break;
		case PU_EQ:
		case PU_NE:
			if (!(op1->Type.type == ev_int && op2->Type.type == ev_int) &&
				!(op1->Type.type == ev_float && op2->Type.type == ev_float) &&
				!(op1->Type.type == ev_name && op2->Type.type == ev_name) &&
				!(op1->Type.type == ev_pointer && op2->Type.type == ev_pointer) &&
				!(op1->Type.type == ev_vector && op2->Type.type == ev_vector) &&
				!(op1->Type.type == ev_classid && op2->Type.type == ev_classid) &&
				!(op1->Type.type == ev_state && op2->Type.type == ev_state) &&
				!(op1->Type.type == ev_reference && op2->Type.type == ev_reference))
			{
				ParseError(Loc, "Expression type mistmatch");
				delete this;
				return NULL;
			}
			Type = ev_int;
			break;
		case PU_AND_LOG:
		case PU_OR_LOG:
			op1->Type.CheckSizeIs4(Loc);
			op2->Type.CheckSizeIs4(Loc);
			Type = ev_int;
			break;
		default:
			break;
		}
		return this;
	}
	void EmitCode()
	{
		int jmppos = 0;

		op1->EmitCode();

		switch (Oper)
		{
		case PU_AND_LOG:
			op1->Type.EmitToBool();
			jmppos = AddStatement(OPC_IfNotTopGoto, 0);
			break;
		case PU_OR_LOG:
			op1->Type.EmitToBool();
			jmppos = AddStatement(OPC_IfTopGoto, 0);
			break;
		default:
			break;
		}

		op2->EmitCode();

		switch (Oper)
		{
		case PU_ASTERISK:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_Multiply);
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_FMultiply);
			}
			else if (op1->Type.type == ev_vector && op2->Type.type == ev_float)
			{
				AddStatement(OPC_VPostScale);
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_vector)
			{
				AddStatement(OPC_VPreScale);
			}
			break;

		case PU_SLASH:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_Divide);
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_FDivide);
			}
			else if (op1->Type.type == ev_vector && op2->Type.type == ev_float)
			{
				AddStatement(OPC_VIScale);
			}
			break;

		case PU_PERCENT:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_Modulus);
			}
			break;

		case PU_PLUS:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_Add);
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_FAdd);
			}
			else if (op1->Type.type == ev_vector && op2->Type.type == ev_vector)
			{
				AddStatement(OPC_VAdd);
			}
			break;

		case PU_MINUS:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_Subtract);
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_FSubtract);
			}
			else if (op1->Type.type == ev_vector && op2->Type.type == ev_vector)
			{
				AddStatement(OPC_VSubtract);
			}
			break;

		case PU_LSHIFT:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_LShift);
			}
			break;

		case PU_RSHIFT:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_RShift);
			}
			break;

		case PU_LT:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_Less);
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_FLess);
			}
			break;

		case PU_LE:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_LessEquals);
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_FLessEquals);
			}
			break;

		case PU_GT:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_Greater);
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_FGreater);
			}
			break;

		case PU_GE:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_GreaterEquals);
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_FGreaterEquals);
			}
			break;

		case PU_EQ:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_Equals);
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_FEquals);
			}
			else if (op1->Type.type == ev_name && op2->Type.type == ev_name)
			{
				AddStatement(OPC_Equals);
			}
			else if (op1->Type.type == ev_pointer && op2->Type.type == ev_pointer)
			{
				AddStatement(OPC_PtrEquals);
			}
			else if (op1->Type.type == ev_vector && op2->Type.type == ev_vector)
			{
				AddStatement(OPC_VEquals);
			}
			else if (op1->Type.type == ev_classid && op2->Type.type == ev_classid)
			{
				AddStatement(OPC_PtrEquals);
			}
			else if (op1->Type.type == ev_state && op2->Type.type == ev_state)
			{
				AddStatement(OPC_PtrEquals);
			}
			else if (op1->Type.type == ev_reference && op2->Type.type == ev_reference)
			{
				AddStatement(OPC_PtrEquals);
			}
			break;

		case PU_NE:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_NotEquals);
			}
			else if (op1->Type.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_FNotEquals);
			}
			else if (op1->Type.type == ev_name && op2->Type.type == ev_name)
			{
				AddStatement(OPC_NotEquals);
			}
			else if (op1->Type.type == ev_pointer && op2->Type.type == ev_pointer)
			{
				AddStatement(OPC_PtrNotEquals);
			}
			else if (op1->Type.type == ev_vector && op2->Type.type == ev_vector)
			{
				AddStatement(OPC_VNotEquals);
			}
			else if (op1->Type.type == ev_classid && op2->Type.type == ev_classid)
			{
				AddStatement(OPC_PtrNotEquals);
			}
			else if (op1->Type.type == ev_state && op2->Type.type == ev_state)
			{
				AddStatement(OPC_PtrNotEquals);
			}
			else if (op1->Type.type == ev_reference && op2->Type.type == ev_reference)
			{
				AddStatement(OPC_PtrNotEquals);
			}
			break;

		case PU_AND:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_AndBitwise);
			}
			break;

		case PU_XOR:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_XOrBitwise);
			}
			break;

		case PU_OR:
			if (op1->Type.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_OrBitwise);
			}
			break;

		case PU_AND_LOG:
			op2->Type.EmitToBool();
			AddStatement(OPC_AndLogical);
			FixupJump(jmppos);
			break;

		case PU_OR_LOG:
			op2->Type.EmitToBool();
			AddStatement(OPC_OrLogical);
			FixupJump(jmppos);
			break;

		default:
			break;
		}
	}
};

class VConditional : public VExpression
{
public:
	VExpression*		op;		//	Condition
	VExpression*		op1;	//	Value1
	VExpression*		op2;	//	Value2

	VConditional(VExpression* AOp, VExpression* AOp1, VExpression* AOp2, const TLocation& ALoc)
	: VExpression(ALoc)
	, op(AOp)
	, op1(AOp1)
	, op2(AOp2)
	{
		if (!op1)
		{
			ParseError(Loc, "Expression expected");
			return;
		}
		if (!op2)
		{
			ParseError(Loc, "Expression expected");
			return;
		}
	}
	~VConditional()
	{
		if (op)
			delete op;
		if (op1)
			delete op1;
		if (op2)
			delete op2;
	}
	VExpression* DoResolve()
	{
		if (op)
			op = op->Resolve();
		if (op1)
			op1 = op1->Resolve();
		if (op2)
			op2 = op2->Resolve();
		if (!op || !op1 || !op2)
		{
			delete this;
			return NULL;
		}

		op->Type.CheckSizeIs4(Loc);
		op1->Type.CheckMatch(op2->Type);
		if (op1->Type.type == ev_pointer && op1->Type.InnerType == ev_void)
			Type = op2->Type;
		else
			Type = op1->Type;
		return this;
	}
	void EmitCode()
	{
		op->EmitCode();
		op->Type.EmitToBool();
		int jumppos1 = AddStatement(OPC_IfNotGoto, 0);
		op1->EmitCode();
		int jumppos2 = AddStatement(OPC_Goto, 0);
		FixupJump(jumppos1);
		op2->EmitCode();
		FixupJump(jumppos2);
	}
};

class VAssignment : public VExpression
{
public:
	EPunctuation		Oper;	//	Operation
	VExpression*		op1;	//	Variable
	VExpression*		op2;	//	Value

	VAssignment(EPunctuation AOper, VExpression* AOp1, VExpression* AOp2, const TLocation& ALoc)
	: VExpression(ALoc)
	, Oper(AOper)
	, op1(AOp1)
	, op2(AOp2)
	{
		if (!op2)
		{
			ParseError(Loc, "Expression required on the right side of assignment operator");
			return;
		}
	}
	~VAssignment()
	{
		if (op1)
			delete op1;
		if (op2)
			delete op2;
	}
	VExpression* DoResolve()
	{
		if (op1)
			op1 = op1->Resolve();
		if (op2)
			op2 = op2->Resolve();
		if (!op1 || !op2)
		{
			delete this;
			return NULL;
		}

		op2->Type.CheckMatch(op1->RealType);
		op1->RequestAddressOf();
		return this;
	}
	void EmitCode()
	{
		op1->EmitCode();
		op2->EmitCode();

		switch (Oper)
		{
		case PU_ASSIGN:
			if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_AssignDrop);
			}
			else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_AssignDrop);
			}
			else if (op1->RealType.type == ev_name && op2->Type.type == ev_name)
			{
				AddStatement(OPC_AssignDrop);
			}
			else if (op1->RealType.type == ev_string && op2->Type.type == ev_string)
			{
				AddStatement(OPC_AssignStrDrop);
			}
			else if (op1->RealType.type == ev_pointer && op2->Type.type == ev_pointer)
			{
				AddStatement(OPC_AssignPtrDrop);
			}
			else if (op1->RealType.type == ev_vector && op2->Type.type == ev_vector)
			{
				AddStatement(OPC_VAssignDrop);
			}
			else if (op1->RealType.type == ev_classid && (op2->Type.type == ev_classid ||
				(op2->Type.type == ev_reference && op2->Type.Class == NULL)))
			{
				AddStatement(OPC_AssignPtrDrop);
			}
			else if (op1->RealType.type == ev_state && (op2->Type.type == ev_state ||
				(op2->Type.type == ev_reference && op2->Type.Class == NULL)))
			{
				AddStatement(OPC_AssignPtrDrop);
			}
			else if (op1->RealType.type == ev_reference && op2->Type.type == ev_reference)
			{
				AddStatement(OPC_AssignPtrDrop);
			}
			else if (op1->RealType.type == ev_bool && op2->Type.type == ev_int)
			{
				if (op1->RealType.bit_mask & 0x000000ff)
					AddStatement(OPC_AssignBool0, op1->RealType.bit_mask);
				else if (op1->RealType.bit_mask & 0x0000ff00)
					AddStatement(OPC_AssignBool1, op1->RealType.bit_mask >> 8);
				else if (op1->RealType.bit_mask & 0x00ff0000)
					AddStatement(OPC_AssignBool2, op1->RealType.bit_mask >> 16);
				else
					AddStatement(OPC_AssignBool3, op1->RealType.bit_mask >> 24);
			}
			else if (op1->RealType.type == ev_delegate && op2->Type.type == ev_delegate)
			{
				AddStatement(OPC_AssignDelegate);
			}
			else if (op1->RealType.type == ev_delegate && op2->Type.type == ev_reference && op2->Type.Class == NULL)
			{
				AddStatement(OPC_PushNull);
				AddStatement(OPC_AssignDelegate);
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
			}
			break;

		case PU_ADD_ASSIGN:
			if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_AddVarDrop);
			}
			else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_FAddVarDrop);
			}
			else if (op1->RealType.type == ev_vector && op2->Type.type == ev_vector)
			{
				AddStatement(OPC_VAddVarDrop);
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
			}
			break;

		case PU_MINUS_ASSIGN:
			if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_SubVarDrop);
			}
			else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_FSubVarDrop);
			}
			else if (op1->RealType.type == ev_vector && op2->Type.type == ev_vector)
			{
				AddStatement(OPC_VSubVarDrop);
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
			}
			break;

		case PU_MULTIPLY_ASSIGN:
			if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_MulVarDrop);
			}
			else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_FMulVarDrop);
			}
			else if (op1->RealType.type == ev_vector && op2->Type.type == ev_float)
			{
				AddStatement(OPC_VScaleVarDrop);
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
			}
			break;

		case PU_DIVIDE_ASSIGN:
			if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_DivVarDrop);
			}
			else if (op1->RealType.type == ev_float && op2->Type.type == ev_float)
			{
				AddStatement(OPC_FDivVarDrop);
			}
			else if (op1->RealType.type == ev_vector && op2->Type.type == ev_float)
			{
				AddStatement(OPC_VIScaleVarDrop);
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
			}
			break;

		case PU_MOD_ASSIGN:
			if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_ModVarDrop);
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
			}
			break;

		case PU_AND_ASSIGN:
			if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_AndVarDrop);
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
			}
			break;

		case PU_OR_ASSIGN:
			if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_OrVarDrop);
			}
//FIXME This is wrong!
			else if (op1->RealType.type == ev_bool && op2->Type.type == ev_int)
			{
				AddStatement(OPC_OrVarDrop);
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
			}
			break;

		case PU_XOR_ASSIGN:
			if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_XOrVarDrop);
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
			}
			break;

		case PU_LSHIFT_ASSIGN:
			if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_LShiftVarDrop);
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
			}
			break;

		case PU_RSHIFT_ASSIGN:
			if (op1->RealType.type == ev_int && op2->Type.type == ev_int)
			{
				AddStatement(OPC_RShiftVarDrop);
			}
			else
			{
				ParseError(Loc, "Expression type mistmatch");
			}
			break;

		default:
			ParseError(Loc, "Impossible");
		}
	}
};

class VInvocation : public VExpression
{
public:
	VExpression*	SelfExpr;
	VMethod*		Func;
	VField*			DelegateField;
	bool			HaveSelf;
	bool			BaseCall;
	int				NumArgs;
	VExpression*	Args[MAX_ARG_COUNT + 1];

	VInvocation(VExpression* ASelfExpr, VMethod* AFunc, VField* ADelegateField,
		bool AHaveSelf, bool ABaseCall, const TLocation& ALoc, int ANumArgs,
		VExpression** AArgs)
	: VExpression(ALoc)
	, SelfExpr(ASelfExpr)
	, Func(AFunc)
	, DelegateField(ADelegateField)
	, HaveSelf(AHaveSelf)
	, BaseCall(ABaseCall)
	, NumArgs(ANumArgs)
	{
		for (int i = 0; i < NumArgs; i++)
			Args[i] = AArgs[i];
	}
	~VInvocation()
	{
		if (SelfExpr)
			delete SelfExpr;
		for (int i = 0; i < NumArgs; i++)
			delete Args[i];
	}
	VExpression* DoResolve()
	{
		//	Resolve self expression if present.
		if (SelfExpr)
		{
			SelfExpr = SelfExpr->Resolve();
			if (!SelfExpr)
			{
				delete this;
				return NULL;
			}
		}

		//	Resolve arguments
		for (int i = 0; i < NumArgs; i++)
		{
			if (Args[i])
				Args[i] = Args[i]->Resolve();
			if (!Args[i])
			{
				delete this;
				return NULL;
			}
		}

		CheckParams();

		Type  = Func->ReturnType;
		if (Type.type == ev_bool)
			Type = TType(ev_int);
		return this;
	}
	void EmitCode()
	{
		if (SelfExpr)
			SelfExpr->EmitCode();

		bool DirectCall = BaseCall || (Func->Flags & FUNC_Final);

		if (Func->Flags & FUNC_Static)
		{
			if (HaveSelf)
			{
				ParseError(Loc, "Invalid static function call");
			}
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

		for (int i = 0; i < NumArgs; i++)
		{
			Args[i]->EmitCode();
			if (!DirectCall)
			{
				if (Args[i]->Type.type == ev_vector)
					AddStatement(OPC_Swap3);
				else
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
	}
	void CheckParams()
	{
		//	Determine parameter count.
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

		for (int i = 0; i < NumArgs; i++)
		{
			if (i >= max_params)
			{
				ParseError(Loc, "Incorrect number of arguments, need %d, got %d.", max_params, i + 1);
			}
			else
			{
				if (i < num_needed_params)
				{
					Args[i]->Type.CheckMatch(Func->ParamTypes[i]);
				}
			}
			argsize += Args[i]->Type.GetSize();
		}
		if (NumArgs < num_needed_params)
		{
			ParseError(Loc, "Incorrect argument count %d, should be %d",
				NumArgs, num_needed_params);
		}

		if (Func->Flags & FUNC_VarArgs)
		{
			Args[NumArgs++] = new VIntLiteral(argsize / 4 - num_needed_params, Loc);
		}
	}
};

class VDelegateToBool : public VExpression
{
public:
	VExpression*		op;

	VDelegateToBool(VExpression* AOp)
	: VExpression(AOp->Loc)
	, op(AOp)
	{
		Type = ev_int;
		op->RequestAddressOf();
	}
	~VDelegateToBool()
	{
		if (op)
			delete op;
	}
	VExpression* DoResolve()
	{
		return this;
	}
	void EmitCode()
	{
		op->EmitCode();
		AddStatement(OPC_PushPointedPtr);
		AddStatement(OPC_PtrToBool);
	}
};

class VBaseInvocation : public VExpression
{
public:
	VName			Name;
	int				NumArgs;
	VExpression*	Args[MAX_ARG_COUNT + 1];

	VBaseInvocation(VName AName, int ANumArgs, VExpression** AArgs,
		const TLocation& ALoc)
	: VExpression(ALoc)
	, Name(AName)
	, NumArgs(ANumArgs)
	{
		for (int i = 0; i < NumArgs; i++)
			Args[i] = AArgs[i];
	}
	~VBaseInvocation()
	{
		for (int i = 0; i < NumArgs; i++)
			delete Args[i];
	}
	VExpression* DoResolve()
	{
		if (!SelfClass)
		{
			ParseError(Loc, ":: not in method");
			delete this;
			return NULL;
		}
		VMethod* Func = CheckForMethod(Name, SelfClass->ParentClass);
		if (!Func)
		{
			ParseError(Loc, "No such method %s", *Name);
			delete this;
			return NULL;
		}

		VExpression* e = new VInvocation(NULL, Func, NULL, false,
			true, Loc, NumArgs, Args);
		NumArgs = 0;
		delete this;
		return e->Resolve();
	}
	void EmitCode()
	{
		ParseError(Loc, "Should not happen");
	}
};

class VCastOrInvocation : public VExpression
{
public:
	VName			Name;
	int				NumArgs;
	VExpression*	Args[MAX_ARG_COUNT + 1];

	VCastOrInvocation(VName AName, const TLocation& ALoc, int ANumArgs,
		VExpression** AArgs)
	: VExpression(ALoc)
	, Name(AName)
	, NumArgs(ANumArgs)
	{
		for (int i = 0; i < NumArgs; i++)
			Args[i] = AArgs[i];
	}
	~VCastOrInvocation()
	{
		for (int i = 0; i < NumArgs; i++)
			delete Args[i];
	}
	VExpression* DoResolve()
	{
		VClass* Class = CheckForClass(Name);
		if (Class)
		{
			if (NumArgs != 1 || !Args[0])
			{
				ParseError(Loc, "Dynamic cast requires 1 argument");
				delete this;
				return NULL;
			}
			VExpression* e = new VDynamicCast(Class, Args[0], Loc);
			NumArgs = 0;
			delete this;
			return e->Resolve();
		}

		if (SelfClass)
		{
			VMethod* M = CheckForMethod(Name, SelfClass);
			if (M)
			{
				VExpression* e = new VInvocation(NULL, M, NULL,
					false, false, Loc, NumArgs, Args);
				NumArgs = 0;
				delete this;
				return e->Resolve();
			}
			VField* field = CheckForField(Name, SelfClass);
			if (field && field->type.type == ev_delegate)
			{
				VExpression* e = new VInvocation(NULL, field->func, field,
					false, false, Loc, NumArgs, Args);
				NumArgs = 0;
				delete this;
				return e->Resolve();
			}
		}

		ParseError(Loc, "Unknown method %s", *Name);
		delete this;
		return NULL;
	}
	void EmitCode()
	{
		ParseError(Loc, "Should not happen");
	}
};

class VDotInvocation : public VExpression
{
public:
	VExpression*	SelfExpr;
	VName			MethodName;
	int				NumArgs;
	VExpression*	Args[MAX_ARG_COUNT + 1];

	VDotInvocation(VExpression* ASelfExpr, VName AMethodName,
		const TLocation& ALoc, int ANumArgs, VExpression** AArgs)
	: VExpression(ALoc)
	, SelfExpr(ASelfExpr)
	, MethodName(AMethodName)
	, NumArgs(ANumArgs)
	{
		for (int i = 0; i < NumArgs; i++)
			Args[i] = AArgs[i];
	}
	~VDotInvocation()
	{
		if (SelfExpr)
			delete SelfExpr;
		for (int i = 0; i < NumArgs; i++)
			delete Args[i];
	}
	VExpression* DoResolve()
	{
		if (SelfExpr)
			SelfExpr = SelfExpr->Resolve();
		if (!SelfExpr)
		{
			delete this;
			return NULL;
		}

		if (SelfExpr->Type.type != ev_reference)
		{
			ParseError(Loc, "Object reference expected left side of .");
		}

		VMethod* M = CheckForMethod(MethodName, SelfExpr->Type.Class);
		if (M)
		{
			VExpression* e = new VInvocation(SelfExpr, M, NULL, true,
				false, Loc, NumArgs, Args);
			SelfExpr = NULL;
			NumArgs = 0;
			delete this;
			return e->Resolve();
		}

		VField* field = CheckForField(MethodName, SelfExpr->Type.Class);
		if (field && field->type.type == ev_delegate)
		{
			VExpression* e = new VInvocation(SelfExpr, field->func, field, true,
				false, Loc, NumArgs, Args);
			SelfExpr = NULL;
			NumArgs = 0;
			delete this;
			return e->Resolve();
		}

		ParseError(Loc, "No such method %s", *MethodName);
		delete this;
		return NULL;
	}
	void EmitCode()
	{
		ParseError(Loc, "Should not happen");
	}
};

class VSingleName : public VExpression
{
public:
	VName			Name;

	VSingleName(VName AName, const TLocation& ALoc)
	: VExpression(ALoc)
	, Name(AName)
	{
	}
	VExpression* DoResolve()
	{
		int num = CheckForLocalVar(Name);
		if (num)
		{
			VExpression* e = new VLocalVar(num, Loc);
			delete this;
			return e->Resolve();
		}

		VConstant* Const = CheckForConstant(SelfClass, Name);
		if (Const)
		{
			VExpression* e = new VConstantValue(Const, Loc);
			delete this;
			return e->Resolve();
		}

		if (SelfClass)
		{
			VMethod* M = CheckForMethod(Name, SelfClass);
			if (M)
			{
				VExpression* e = new VDelegateVal(new VSelf(Loc), M, Loc);
				delete this;
				return e->Resolve();
			}

			VField* field = CheckForField(Name, SelfClass);
			if (field)
			{
				VExpression* e = new VFieldAccess(new VSelf(Loc), field, Loc, 0);
				delete this;
				return e->Resolve();
			}

			VState* State = CheckForState(Name, SelfClass);
			if (State)
			{
				VExpression* e = new VStateConstant(State, Loc);
				delete this;
				return e->Resolve();
			}
		}

		VClass* Class = CheckForClass(Name);
		if (Class)
		{
			VExpression* e = new VClassConstant(Class, Loc);
			delete this;
			return e->Resolve();
		}

		ParseError(Loc, "Illegal expression identifier %s", *Name);
		delete this;
		return NULL;
	}
	void EmitCode()
	{
		ParseError(Loc, "Should not happen");
	}
	bool IsSingleName() { return true; }
};

class VDoubleName : public VExpression
{
public:
	VName			Name1;
	VName			Name2;

	VDoubleName(VName AName1, VName AName2, const TLocation& ALoc)
	: VExpression(ALoc)
	, Name1(AName1)
	, Name2(AName2)
	{
	}
	VExpression* DoResolve()
	{
		VClass* Class = CheckForClass(Name1);
		if (!Class)
		{
			ParseError("No such class %s", *Name1);
			delete this;
			return NULL;
		}

		VConstant* Const = CheckForConstant(Class, Name2);
		if (Const)
		{
			VExpression* e = new VConstantValue(Const, Loc);
			delete this;
			return e->Resolve();
		}

		VState* State = CheckForState(Name2, Class);
		if (State)
		{
			VExpression* e = new VStateConstant(State, Loc);
			delete this;
			return e->Resolve();
		}

		ParseError(Loc, "No such constant or state %s", *Name2);
		delete this;
		return NULL;
	}
	void EmitCode()
	{
		ParseError(Loc, "Should not happen");
	}
	bool IsSingleName() { return true; }
};

class VPointerField : public VExpression
{
public:
	VExpression*		op;
	VName				FieldName;

	VPointerField(VExpression* AOp, VName AFieldName, const TLocation& ALoc)
	: VExpression(ALoc)
	, op(AOp)
	, FieldName(AFieldName)
	{
	}
	~VPointerField()
	{
		if (op)
			delete op;
	}
	VExpression* DoResolve()
	{
		if (op)
			op = op->Resolve();
		if (!op)
		{
			delete this;
			return NULL;
		}

		if (op->Type.type != ev_pointer)
		{
			ParseError(Loc, "Pointer type required on left side of ->");
			delete this;
			return NULL;
		}
		TType type = op->Type.GetPointerInnerType();
		if (!type.Struct)
		{
			ParseError(Loc, "Not a structure type");
			delete this;
			return NULL;
		}
		VField* field = CheckForStructField(type.Struct, FieldName, Loc);
		if (!field)
		{
			ParseError(Loc, "No such field %s", *FieldName);
			delete this;
			return NULL;
		}
		VExpression* e = new VFieldAccess(op, field, Loc, 0);
		op = NULL;
		delete this;
		return e->Resolve();
	}
	void EmitCode()
	{
		ParseError(Loc, "Should not happen");
	}
};

class VDotField : public VExpression
{
public:
	VExpression*		op;
	VName				FieldName;

	VDotField(VExpression* AOp, VName AFieldName, const TLocation& ALoc)
	: VExpression(ALoc)
	, op(AOp)
	, FieldName(AFieldName)
	{
	}
	~VDotField()
	{
		if (op)
			delete op;
	}
	VExpression* DoResolve()
	{
		if (op)
			op = op->Resolve();
		if (!op)
		{
			delete this;
			return NULL;
		}

		if (op->Type.type == ev_reference)
		{
			VMethod* M = CheckForMethod(FieldName, op->Type.Class);
			if (M)
			{
				VExpression* e = new VDelegateVal(op, M, Loc);
				op = NULL;
				delete this;
				return e->Resolve();
			}
			else
			{
				VField* field = CheckForField(FieldName, op->Type.Class);
				if (!field)
				{
					ParseError(Loc, "No such field %s", *FieldName);
					delete this;
					return NULL;
				}
				VExpression* e = new VFieldAccess(op, field, Loc, 0);
				op = NULL;
				delete this;
				return e->Resolve();
			}
		}
		else if (op->Type.type == ev_struct || op->Type.type == ev_vector)
		{
			TType type = op->Type;
			int Flags = op->Flags;
			op->Flags &= ~FIELD_ReadOnly;
			op->RequestAddressOf();
			VField* field = CheckForStructField(type.Struct, FieldName, Loc);
			if (!field)
			{
				ParseError(Loc, "No such field %s", *FieldName);
				delete this;
				return NULL;
			}
			VExpression* e = new VFieldAccess(op, field, Loc, Flags & FIELD_ReadOnly);
			op = NULL;
			delete this;
			return e->Resolve();
		}
		ParseError(Loc, "Reference, struc or vector expected on left side of .");
		delete this;
		return NULL;
	}
	void EmitCode()
	{
		ParseError(Loc, "Should not happen");
	}
};

class VLocalEntry
{
public:
	VName			Name;
	TLocation		Loc;
	VExpression*	Value;
	int				PointerLevel;
	int				ArraySize;

	VLocalEntry()
	: Name(NAME_None)
	, Value(NULL)
	, PointerLevel(0)
	, ArraySize(-1)
	{}
};

class VLocalDecl : public VExpression
{
public:
	TType				BaseType;
	VName				TypeName;
	TArray<VLocalEntry>	Vars;

	VLocalDecl(const TLocation& ALoc)
	: VExpression(ALoc)
	{
	}
	~VLocalDecl()
	{
		for (int i = 0; i < Vars.Num(); i++)
		{
			if (Vars[i].Value)
			{
				delete Vars[i].Value;
			}
		}
	}

	VExpression* DoResolve()
	{
		Declare();
		return this;
	}
	void EmitCode()
	{
		EmitInitialisations();
	}

	void Declare()
	{
		if (BaseType.type == ev_unknown)
		{
			BaseType = CheckForType(SelfClass, TypeName);
			if (BaseType.type == ev_unknown)
			{
				ParseError(tk_Location, "Invalid identifier, bad type");
				return;
			}
		}

		for (int i = 0; i < Vars.Num(); i++)
		{
			VLocalEntry& e = Vars[i];

			if (CheckForLocalVar(e.Name))
			{
				ParseError(e.Loc, "Redefined identifier %s", *e.Name);
			}

			TType Type = BaseType;
			for (int pl = 0; pl < e.PointerLevel; pl++)
			{
				Type = MakePointerType(Type);
			}
			if (Type.type == ev_void)
			{
				ParseError(e.Loc, "Bad variable type");
			}
			if (e.ArraySize != -1)
			{
				Type = MakeArrayType(Type, e.ArraySize);
			}

			if (numlocaldefs == MAX_LOCAL_DEFS)
			{
				ParseError(e.Loc, "Too many local variables");
				continue;
			}

			localdefs[numlocaldefs].Name = e.Name;
			localdefs[numlocaldefs].type = Type;
			localdefs[numlocaldefs].ofs = localsofs;

			//  Initialisation
			if (e.Value)
			{
				VExpression* op1 = new VLocalVar(numlocaldefs, tk_Location);
				e.Value = new VAssignment(PU_ASSIGN, op1, e.Value, tk_Location);
				e.Value = e.Value->Resolve();
			}

			//  Increase variable count after expression so you can't use
			// the variable in expression.
			numlocaldefs++;
			localsofs += Type.GetSize() / 4;
			if (localsofs > 1024)
			{
				ParseWarning("Local vars > 1k");
			}
		}
	}
	void EmitInitialisations()
	{
		for (int i = 0; i < Vars.Num(); i++)
		{
			if (Vars[i].Value)
			{
				Vars[i].Value->EmitCode();
			}
		}
	}
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static VExpression* ParseExpressionPriority13();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static bool			CheckForLocal;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VExpression::ResolveTopLevel
//
//==========================================================================

VExpression* VExpression::ResolveTopLevel()
{
	VExpression* e = Resolve();
	if (e && e->Type.type == ev_delegate)
	{
		e = new VDelegateToBool(e);
	}
	return e;
}

//==========================================================================
//
//	EmitPushNumber
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
//	EmitLocalAddress
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
//	ParseDotMethodCall
//
//==========================================================================

static VExpression* ParseDotMethodCall(VExpression* SelfExpr,
	VName MethodName, TLocation Loc)
{
	VExpression* Args[MAX_ARG_COUNT + 1];
	int NumArgs = 0;
	if (!TK_Check(PU_RPAREN))
	{
		do
		{
			Args[NumArgs] = ParseExpressionPriority13();
			if (NumArgs == MAX_ARG_COUNT)
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
	VExpression* Args[MAX_ARG_COUNT + 1];
	int NumArgs = 0;
	if (!TK_Check(PU_RPAREN))
	{
		do
		{
			Args[NumArgs] = ParseExpressionPriority13();
			if (NumArgs == MAX_ARG_COUNT)
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
	VExpression* Args[MAX_ARG_COUNT + 1];
	int NumArgs = 0;
	if (!TK_Check(PU_RPAREN))
	{
		do
		{
			Args[NumArgs] = ParseExpressionPriority13();
			if (NumArgs == MAX_ARG_COUNT)
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

static VLocalDecl* ParseLocalVar(const TType& InType, VName TypeName)
{
	VLocalDecl* Decl = new VLocalDecl(tk_Location);
	do
	{
		VLocalEntry e;

		while (TK_Check(PU_ASTERISK))
		{
			e.PointerLevel++;
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
			e.ArraySize = EvalConstExpression(SelfClass, ev_int);
			TK_Expect(PU_RINDEX, ERR_MISSING_RFIGURESCOPE);
		}
		//  Initialisation
		else if (TK_Check(PU_ASSIGN))
		{
			e.Value = ParseExpressionPriority13();
		}
		Decl->Vars.Append(e);
	} while (TK_Check(PU_COMMA));

	Decl->BaseType = InType;
	Decl->TypeName = TypeName;
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
			return ParseLocalVar(ev_unknown, Name);
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
		op1 = new VBinary(PU_AND_LOG, op1, op2, l);
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
		op1 = new VBinary(PU_OR_LOG, op1, op2, l);
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

TType ParseExpression(bool bLocals)
{
	if (bLocals && tk_Token == TK_KEYWORD)
	{
		TType type = CheckForTypeKeyword();
		if (type.type != ev_unknown)
		{
			VLocalDecl* Decl = ParseLocalVar(type, NAME_None);
			Decl->Declare();
			Decl->EmitInitialisations();
			delete Decl;
			return ev_void;
		}
	}

	CheckForLocal = bLocals;
	VExpression* op = ParseExpressionPriority14();
	if (!op)
	{
		return ev_void;
	}

	if (bLocals)
	{
		if (op->IsSingleName() && tk_Token == TK_IDENTIFIER)
		{
			VLocalDecl* Decl = ParseLocalVar(ev_unknown, ((VSingleName*)op)->Name);
			delete op;
			Decl->Declare();
			Decl->EmitInitialisations();
			delete Decl;
			return ev_void;
		}
	}

	if (!NumErrors)
	{
		op = op->ResolveTopLevel();
	}
	if (!op)
	{
		return ev_void;
	}
	if (!NumErrors)
	{
		op->EmitCode();
	}
	TType Ret = op->Type;
	delete op;
	return Ret;
}

//==========================================================================
//
//	SkipExpression
//
//==========================================================================

void SkipExpression(bool bLocals = false)
{
#if 0
	CheckForLocal = bLocals;
	SkipExpressionPriority14();
#else
	if (bLocals && tk_Token == TK_KEYWORD)
	{
		TType type = CheckForTypeKeyword();
		if (type.type != ev_unknown)
		{
			VLocalDecl* Decl = ParseLocalVar(type, NAME_None);
			delete Decl;
			return;
		}
	}

	CheckForLocal = bLocals;
	VExpression* op = ParseExpressionPriority14();
	if (!op)
	{
		return;
	}

	if (bLocals)
	{
		if (op->IsSingleName() && tk_Token == TK_IDENTIFIER)
		{
			VLocalDecl* Decl = ParseLocalVar(ev_unknown, ((VSingleName*)op)->Name);
			delete op;
			delete Decl;
			return;
		}
	}

	delete op;
#endif
}
