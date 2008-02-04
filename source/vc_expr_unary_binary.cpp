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

#ifdef IN_VCC
#include "../utils/vcc/vcc.h"
#else
#include "vc_local.h"
#endif

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
//	VUnary::VUnary
//
//==========================================================================

VUnary::VUnary(VUnary::EUnaryOp AOper, VExpression* AOp, const TLocation& ALoc)
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

//==========================================================================
//
//	VUnary::~VUnary
//
//==========================================================================

VUnary::~VUnary()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VUnary::DoResolve
//
//==========================================================================

VExpression* VUnary::DoResolve(VEmitContext& ec)
{
	if (op)
	{
		if (Oper == Not)
			op = op->ResolveBoolean(ec);
		else
			op = op->Resolve(ec);
	}
	if (!op)
	{
		delete this;
		return NULL;
	}

	switch (Oper)
	{
	case Plus:
		Type = op->Type;
		if (op->Type.Type != TYPE_Int && op->Type.Type != TYPE_Float)
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		else
		{
			VExpression* e = op;
			op = NULL;
			delete this;
			return e;
		}

	case Minus:
		if (op->Type.Type == TYPE_Int)
		{
			Type = TYPE_Int;
		}
		else if (op->Type.Type == TYPE_Float)
		{
			Type = TYPE_Float;
		}
		else if (op->Type.Type == TYPE_Vector)
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

	case Not:
		Type = TYPE_Int;
		break;

	case BitInvert:
		if (op->Type.Type != TYPE_Int)
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		Type = TYPE_Int;
		break;

	case TakeAddress:
		if (op->Type.Type == TYPE_Reference)
		{
			ParseError(Loc, "Tried to take address of reference");
			delete this;
			return NULL;
		}
		else
		{
			op->RequestAddressOf();
			Type = op->RealType.MakePointerType();
		}
		break;
	}

	//	Optimise integer constants.
	if (op->IsIntConst())
	{
		vint32 Value = op->GetIntConst();
		VExpression* e = NULL;
		switch (Oper)
		{
		case Minus:
			e = new VIntLiteral(-Value, Loc);
			break;

		case Not:
			e = new VIntLiteral(!Value, Loc);
			break;

		case BitInvert:
			e = new VIntLiteral(~Value, Loc);
			break;

		default:
			break;
		}
		if (e)
		{
			delete this;
			return e;
		}
	}

	//	Optimise float constants.
	if (op->IsFloatConst() && Oper == Minus)
	{
		float Value = op->GetFloatConst();
		VExpression* e = new VFloatLiteral(-Value, Loc);
		delete this;
		return e;
	}

	return this;
}

//==========================================================================
//
//	VUnary::Emit
//
//==========================================================================

void VUnary::Emit(VEmitContext& ec)
{
	op->Emit(ec);

	switch (Oper)
	{
	case Plus:
		break;

	case Minus:
		if (op->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_UnaryMinus);
		}
		else if (op->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FUnaryMinus);
		}
		else if (op->Type.Type == TYPE_Vector)
		{
			ec.AddStatement(OPC_VUnaryMinus);
		}
		break;

	case Not:
		ec.AddStatement(OPC_NegateLogical);
		break;

	case BitInvert:
		ec.AddStatement(OPC_BitInverse);
		break;

	case TakeAddress:
		break;
	}
}

//==========================================================================
//
//	VUnary::EmitBranchable
//
//==========================================================================

void VUnary::EmitBranchable(VEmitContext& ec, VLabel Lbl, bool OnTrue)
{
	if (Oper == Not)
	{
		op->EmitBranchable(ec, Lbl, !OnTrue);
	}
	else
	{
		VExpression::EmitBranchable(ec, Lbl, OnTrue);
	}
}

//==========================================================================
//
//	VUnaryMutator::VUnaryMutator
//
//==========================================================================

VUnaryMutator::VUnaryMutator(EIncDec AOper, VExpression* AOp, const TLocation& ALoc)
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

//==========================================================================
//
//	VUnaryMutator::~VUnaryMutator
//
//==========================================================================

VUnaryMutator::~VUnaryMutator()
{
	if (op)
		delete op;
}

//==========================================================================
//
//	VUnaryMutator::DoResolve
//
//==========================================================================

VExpression* VUnaryMutator::DoResolve(VEmitContext& ec)
{
	if (op)
		op = op->Resolve(ec);
	if (!op)
	{
		delete this;
		return NULL;
	}

	if (op->Type.Type != TYPE_Int)
	{
		ParseError(Loc, "Expression type mistmatch");
		delete this;
		return NULL;
	}
	Type = TYPE_Int;
	op->RequestAddressOf();
	return this;
}

//==========================================================================
//
//	VUnaryMutator::Emit
//
//==========================================================================

void VUnaryMutator::Emit(VEmitContext& ec)
{
	op->Emit(ec);
	switch (Oper)
	{
	case PreInc:
		ec.AddStatement(OPC_PreInc);
		break;

	case PreDec:
		ec.AddStatement(OPC_PreDec);
		break;

	case PostInc:
		ec.AddStatement(OPC_PostInc);
		break;

	case PostDec:
		ec.AddStatement(OPC_PostDec);
		break;

	case Inc:
		ec.AddStatement(OPC_IncDrop);
		break;

	case Dec:
		ec.AddStatement(OPC_DecDrop);
		break;
	}
}

//==========================================================================
//
//	VUnaryMutator::AddDropResult
//
//==========================================================================

bool VUnaryMutator::AddDropResult()
{
	switch (Oper)
	{
	case PreInc:
	case PostInc:
		Oper = Inc;
		break;

	case PreDec:
	case PostDec:
		Oper = Dec;
		break;

	case Inc:
	case Dec:
		FatalError("Should not happen");
	}
	Type = TYPE_Void;
	return true;
}

//==========================================================================
//
//	VBinary::VBinary
//
//==========================================================================

VBinary::VBinary(EBinOp AOper, VExpression* AOp1, VExpression* AOp2, const TLocation& ALoc)
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

//==========================================================================
//
//	VBinary::~VBinary
//
//==========================================================================

VBinary::~VBinary()
{
	if (op1)
		delete op1;
	if (op2)
		delete op2;
}

//==========================================================================
//
//	VBinary::DoResolve
//
//==========================================================================

VExpression* VBinary::DoResolve(VEmitContext& ec)
{
	if (op1)
		op1 = op1->Resolve(ec);
	if (op2)
		op2 = op2->Resolve(ec);
	if (!op1 || !op2)
	{
		delete this;
		return NULL;
	}

	switch (Oper)
	{
	case Add:
	case Subtract:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			Type = TYPE_Int;
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			Type = TYPE_Float;
		}
		else if (op1->Type.Type == TYPE_Vector && op2->Type.Type == TYPE_Vector)
		{
			Type = TYPE_Vector;
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		break;
	case Multiply:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			Type = TYPE_Int;
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			Type = TYPE_Float;
		}
		else if (op1->Type.Type == TYPE_Vector && op2->Type.Type == TYPE_Float)
		{
			Type = TYPE_Vector;
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Vector)
		{
			Type = TYPE_Vector;
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		break;
	case Divide:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			Type = TYPE_Int;
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			Type = TYPE_Float;
		}
		else if (op1->Type.Type == TYPE_Vector && op2->Type.Type == TYPE_Float)
		{
			Type = TYPE_Vector;
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		break;
	case Modulus:
	case LShift:
	case RShift:
	case And:
	case XOr:
	case Or:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			Type = TYPE_Int;
		}
		else
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		break;
	case Less:
	case LessEquals:
	case Greater:
	case GreaterEquals:
		if (!(op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int) &&
			!(op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float))
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		Type = TYPE_Int;
		break;
	case Equals:
	case NotEquals:
		if (!(op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int) &&
			!(op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float) &&
			!(op1->Type.Type == TYPE_Name && op2->Type.Type == TYPE_Name) &&
			!(op1->Type.Type == TYPE_Pointer && op2->Type.Type == TYPE_Pointer) &&
			!(op1->Type.Type == TYPE_Vector && op2->Type.Type == TYPE_Vector) &&
			!(op1->Type.Type == TYPE_Class && op2->Type.Type == TYPE_Class) &&
			!(op1->Type.Type == TYPE_State && op2->Type.Type == TYPE_State) &&
			!(op1->Type.Type == TYPE_Reference && op2->Type.Type == TYPE_Reference))
		{
			ParseError(Loc, "Expression type mistmatch");
			delete this;
			return NULL;
		}
		Type = TYPE_Int;
		break;
	}

	//	Optimise integer constants
	if (op1->IsIntConst() && op2->IsIntConst())
	{
		vint32 Value1 = op1->GetIntConst();
		vint32 Value2 = op2->GetIntConst();
		VExpression* e = NULL;
		switch (Oper)
		{
		case Add:
			e = new VIntLiteral(Value1 + Value2, Loc);
			break;

		case Subtract:
			e = new VIntLiteral(Value1 - Value2, Loc);
			break;

		case Multiply:
			e = new VIntLiteral(Value1 * Value2, Loc);
			break;

		case Divide:
			if (!Value2)
			{
				ParseError(Loc, "Division by 0");
				delete this;
				return NULL;
			}
			e = new VIntLiteral(Value1 / Value2, Loc);
			break;

		case Modulus:
			if (!Value2)
			{
				ParseError(Loc, "Division by 0");
				delete this;
				return NULL;
			}
			e = new VIntLiteral(Value1 % Value2, Loc);
			break;

		case LShift:
			e = new VIntLiteral(Value1 << Value2, Loc);
			break;

		case RShift:
			e = new VIntLiteral(Value1 >> Value2, Loc);
			break;

		case Less:
			e = new VIntLiteral(Value1 < Value2, Loc);
			break;

		case LessEquals:
			e = new VIntLiteral(Value1 <= Value2, Loc);
			break;

		case Greater:
			e = new VIntLiteral(Value1 > Value2, Loc);
			break;

		case GreaterEquals:
			e = new VIntLiteral(Value1 >= Value2, Loc);
			break;

		case Equals:
			e = new VIntLiteral(Value1 == Value2, Loc);
			break;

		case NotEquals:
			e = new VIntLiteral(Value1 != Value2, Loc);
			break;

		case And:
			e = new VIntLiteral(Value1 & Value2, Loc);
			break;

		case XOr:
			e = new VIntLiteral(Value1 ^ Value2, Loc);
			break;

		case Or:
			e = new VIntLiteral(Value1 | Value2, Loc);
			break;

		default:
			break;
		}
		if (e)
		{
			delete this;
			return e;
		}
	}

	//	Optimise float constants.
	if (op1->IsFloatConst() && op2->IsFloatConst())
	{
		float Value1 = op1->GetFloatConst();
		float Value2 = op2->GetFloatConst();
		VExpression* e = NULL;
		switch (Oper)
		{
		case Add:
			e = new VFloatLiteral(Value1 + Value2, Loc);
			break;
	
		case Subtract:
			e = new VFloatLiteral(Value1 - Value2, Loc);
			break;
	
		case Multiply:
			e = new VFloatLiteral(Value1 * Value2, Loc);
			break;
	
		case Divide:
			if (!Value2)
			{
				ParseError(Loc, "Division by 0");
				delete this;
				return NULL;
			}
			e = new VFloatLiteral(Value1 / Value2, Loc);
			break;
	
		default:
			break;
		}
		if (e)
		{
			delete this;
			return e;
		}
	}

	return this;
}

//==========================================================================
//
//	VBinary::Emit
//
//==========================================================================

void VBinary::Emit(VEmitContext& ec)
{
	op1->Emit(ec);
	op2->Emit(ec);

	switch (Oper)
	{
	case Add:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_Add);
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FAdd);
		}
		else if (op1->Type.Type == TYPE_Vector && op2->Type.Type == TYPE_Vector)
		{
			ec.AddStatement(OPC_VAdd);
		}
		break;

	case Subtract:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_Subtract);
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FSubtract);
		}
		else if (op1->Type.Type == TYPE_Vector && op2->Type.Type == TYPE_Vector)
		{
			ec.AddStatement(OPC_VSubtract);
		}
		break;

	case Multiply:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_Multiply);
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FMultiply);
		}
		else if (op1->Type.Type == TYPE_Vector && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_VPostScale);
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Vector)
		{
			ec.AddStatement(OPC_VPreScale);
		}
		break;

	case Divide:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_Divide);
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FDivide);
		}
		else if (op1->Type.Type == TYPE_Vector && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_VIScale);
		}
		break;

	case Modulus:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_Modulus);
		}
		break;

	case LShift:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_LShift);
		}
		break;

	case RShift:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_RShift);
		}
		break;

	case Less:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_Less);
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FLess);
		}
		break;

	case LessEquals:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_LessEquals);
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FLessEquals);
		}
		break;

	case Greater:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_Greater);
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FGreater);
		}
		break;

	case GreaterEquals:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_GreaterEquals);
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FGreaterEquals);
		}
		break;

	case Equals:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_Equals);
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FEquals);
		}
		else if (op1->Type.Type == TYPE_Name && op2->Type.Type == TYPE_Name)
		{
			ec.AddStatement(OPC_Equals);
		}
		else if (op1->Type.Type == TYPE_Pointer && op2->Type.Type == TYPE_Pointer)
		{
			ec.AddStatement(OPC_PtrEquals);
		}
		else if (op1->Type.Type == TYPE_Vector && op2->Type.Type == TYPE_Vector)
		{
			ec.AddStatement(OPC_VEquals);
		}
		else if (op1->Type.Type == TYPE_Class && op2->Type.Type == TYPE_Class)
		{
			ec.AddStatement(OPC_PtrEquals);
		}
		else if (op1->Type.Type == TYPE_State && op2->Type.Type == TYPE_State)
		{
			ec.AddStatement(OPC_PtrEquals);
		}
		else if (op1->Type.Type == TYPE_Reference && op2->Type.Type == TYPE_Reference)
		{
			ec.AddStatement(OPC_PtrEquals);
		}
		break;

	case NotEquals:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_NotEquals);
		}
		else if (op1->Type.Type == TYPE_Float && op2->Type.Type == TYPE_Float)
		{
			ec.AddStatement(OPC_FNotEquals);
		}
		else if (op1->Type.Type == TYPE_Name && op2->Type.Type == TYPE_Name)
		{
			ec.AddStatement(OPC_NotEquals);
		}
		else if (op1->Type.Type == TYPE_Pointer && op2->Type.Type == TYPE_Pointer)
		{
			ec.AddStatement(OPC_PtrNotEquals);
		}
		else if (op1->Type.Type == TYPE_Vector && op2->Type.Type == TYPE_Vector)
		{
			ec.AddStatement(OPC_VNotEquals);
		}
		else if (op1->Type.Type == TYPE_Class && op2->Type.Type == TYPE_Class)
		{
			ec.AddStatement(OPC_PtrNotEquals);
		}
		else if (op1->Type.Type == TYPE_State && op2->Type.Type == TYPE_State)
		{
			ec.AddStatement(OPC_PtrNotEquals);
		}
		else if (op1->Type.Type == TYPE_Reference && op2->Type.Type == TYPE_Reference)
		{
			ec.AddStatement(OPC_PtrNotEquals);
		}
		break;

	case And:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_AndBitwise);
		}
		break;

	case XOr:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_XOrBitwise);
		}
		break;

	case Or:
		if (op1->Type.Type == TYPE_Int && op2->Type.Type == TYPE_Int)
		{
			ec.AddStatement(OPC_OrBitwise);
		}
		break;
	}
}

//==========================================================================
//
//	VBinaryLogical::VBinaryLogical
//
//==========================================================================

VBinaryLogical::VBinaryLogical(ELogOp AOper, VExpression* AOp1,
	VExpression* AOp2, const TLocation& ALoc)
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

//==========================================================================
//
//	VBinaryLogical::~VBinaryLogical
//
//==========================================================================

VBinaryLogical::~VBinaryLogical()
{
	if (op1)
		delete op1;
	if (op2)
		delete op2;
}

//==========================================================================
//
//	VBinaryLogical::DoResolve
//
//==========================================================================

VExpression* VBinaryLogical::DoResolve(VEmitContext& ec)
{
	if (op1)
		op1 = op1->ResolveBoolean(ec);
	if (op2)
		op2 = op2->ResolveBoolean(ec);
	if (!op1 || !op2)
	{
		delete this;
		return NULL;
	}

	Type = TYPE_Int;

	//	Optimise constant cases.
	if (op1->IsIntConst() && op2->IsIntConst())
	{
		vint32 Value1 = op1->GetIntConst();
		vint32 Value2 = op2->GetIntConst();
		VExpression* e = NULL;
		switch (Oper)
		{
		case And:
			e = new VIntLiteral(Value1 && Value2, Loc);
			break;

		case Or:
			e = new VIntLiteral(Value1 || Value2, Loc);
			break;
		}
		if (e)
		{
			delete this;
			return e;
		}
	}

	return this;
}

//==========================================================================
//
//	VBinaryLogical::Emit
//
//==========================================================================

void VBinaryLogical::Emit(VEmitContext& ec)
{
	VLabel Push01 = ec.DefineLabel();
	VLabel End = ec.DefineLabel();

	op1->EmitBranchable(ec, Push01, Oper == Or);

	op2->Emit(ec);
	ec.AddStatement(OPC_Goto, End);

	ec.MarkLabel(Push01);
	ec.AddStatement(Oper == And ? OPC_PushNumber0 : OPC_PushNumber1);

	ec.MarkLabel(End);
}

//==========================================================================
//
//	VBinaryLogical::EmitBranchable
//
//==========================================================================

void VBinaryLogical::EmitBranchable(VEmitContext& ec, VLabel Lbl, bool OnTrue)
{
	switch (Oper)
	{
	case And:
		if (OnTrue)
		{
			VLabel End = ec.DefineLabel();
			op1->EmitBranchable(ec, End, false);
			op2->EmitBranchable(ec, Lbl, true);
			ec.MarkLabel(End);
		}
		else
		{
			op1->EmitBranchable(ec, Lbl, false);
			op2->EmitBranchable(ec, Lbl, false);
		}
		break;

	case Or:
		if (OnTrue)
		{
			op1->EmitBranchable(ec, Lbl, true);
			op2->EmitBranchable(ec, Lbl, true);
		}
		else
		{
			VLabel End = ec.DefineLabel();
			op1->EmitBranchable(ec, End, true);
			op2->EmitBranchable(ec, Lbl, false);
			ec.MarkLabel(End);
		}
		break;
	}
}
