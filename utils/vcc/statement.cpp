//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: parse.cpp 1670 2006-08-20 12:39:46Z dj_jl $
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

//BEGIN VStatement

//==========================================================================
//
//	VStatement::VStatement
//
//==========================================================================

VStatement::VStatement(const TLocation& ALoc)
: Loc(ALoc)
{}

//==========================================================================
//
//	VStatement::~VStatement
//
//==========================================================================

VStatement::~VStatement()
{}

//==========================================================================
//
//	VStatement::Emit
//
//==========================================================================

void VStatement::Emit(VEmitContext& ec)
{
	DoEmit(ec);
}

//END

//BEGIN VEmptyStatement

//==========================================================================
//
//	VEmptyStatement::VEmptyStatement
//
//==========================================================================

VEmptyStatement::VEmptyStatement(const TLocation& ALoc)
: VStatement(ALoc)
{}

//==========================================================================
//
//	VEmptyStatement::Resolve
//
//==========================================================================

bool VEmptyStatement::Resolve(VEmitContext&)
{
	return true;
}

//==========================================================================
//
//	VEmptyStatement::DoEmit
//
//==========================================================================

void VEmptyStatement::DoEmit(VEmitContext&)
{
}

//END

//BEGIN VIf

//==========================================================================
//
//	VIf::VIf
//
//==========================================================================

VIf::VIf(VExpression* AExpr, VStatement* ATrueStatement, const TLocation& ALoc)
: VStatement(ALoc)
, Expr(AExpr)
, TrueStatement(ATrueStatement)
, FalseStatement(NULL)
{
}

//==========================================================================
//
//	VIf::VIf
//
//==========================================================================

VIf::VIf(VExpression* AExpr, VStatement* ATrueStatement,
	VStatement* AFalseStatement, const TLocation& ALoc)
: VStatement(ALoc)
, Expr(AExpr)
, TrueStatement(ATrueStatement)
, FalseStatement(AFalseStatement)
{}

//==========================================================================
//
//	VIf::~VIf
//
//==========================================================================

VIf::~VIf()
{
	if (Expr)
		delete Expr;
	if (TrueStatement)
		delete TrueStatement;
	if (FalseStatement)
		delete FalseStatement;
}

//==========================================================================
//
//	VIf::Resolve
//
//==========================================================================

bool VIf::Resolve(VEmitContext& ec)
{
	bool Ret = true;

	if (Expr)
	{
		Expr = Expr->ResolveBoolean(ec);
	}
	if (!Expr)
	{
		Ret = false;
	}

	if (!TrueStatement->Resolve(ec))
	{
		Ret = false;
	}

	if (FalseStatement && !FalseStatement->Resolve(ec))
	{
		Ret = false;
	}

	return Ret;
}

//==========================================================================
//
//	VIf::DoEmit
//
//==========================================================================

void VIf::DoEmit(VEmitContext& ec)
{
	//	Expression.
	Expr->Emit(ec);

	//	True statement
	int jumpAddrPtr1 = ec.AddStatement(OPC_IfNotGoto, 0);
	TrueStatement->Emit(ec);
	if (FalseStatement)
	{
		//	False statement
		int jumpAddrPtr2 = ec.AddStatement(OPC_Goto, 0);
		ec.FixupJump(jumpAddrPtr1);
		FalseStatement->Emit(ec);
		ec.FixupJump(jumpAddrPtr2);
	}
	else
	{
		ec.FixupJump(jumpAddrPtr1);
	}
}

//END

//BEGIN VWhile

//==========================================================================
//
//	VWhile::VWhile
//
//==========================================================================

VWhile::VWhile(VExpression* AExpr, VStatement* AStatement, const TLocation& ALoc)
: VStatement(ALoc)
, Expr(AExpr)
, Statement(AStatement)
{
}

//==========================================================================
//
//	VWhile::~VWhile
//
//==========================================================================

VWhile::~VWhile()
{
	if (Expr)
		delete Expr;
	if (Statement)
		delete Statement;
}

//==========================================================================
//
//	VWhile::Resolve
//
//==========================================================================

bool VWhile::Resolve(VEmitContext& ec)
{
	bool Ret = true;

	NumLocalsOnStart = ec.LocalDefs.Num();
	if (Expr)
	{
		Expr = Expr->ResolveBoolean(ec);
	}
	if (!Expr)
	{
		Ret = false;
	}

	if (!Statement->Resolve(ec))
	{
		Ret = false;
	}

	return Ret;
}

//==========================================================================
//
//	VWhile::DoEmit
//
//==========================================================================

void VWhile::DoEmit(VEmitContext& ec)
{
	int PrevBreakLocalsStart = ec.BreakNumLocalsOnStart;
	int PrevContinueLocalsStart = ec.ContinueNumLocalsOnStart;
	ec.BreakNumLocalsOnStart = NumLocalsOnStart;
	ec.ContinueNumLocalsOnStart = NumLocalsOnStart;
	ec.BreakLevel++;
	ec.ContinueLevel++;
	int topAddr = ec.GetNumInstructions();
	Expr->Emit(ec);
	int outAddrPtr = ec.AddStatement(OPC_IfNotGoto, 0);
	Statement->Emit(ec);
	ec.AddStatement(OPC_Goto, topAddr);
	ec.FixupJump(outAddrPtr);
	ec.WriteContinues(topAddr);
	ec.WriteBreaks();
	ec.BreakNumLocalsOnStart = PrevBreakLocalsStart;
	ec.ContinueNumLocalsOnStart = PrevContinueLocalsStart;
}

//END

//BEGIN VDo

//==========================================================================
//
//	VDo::VDo
//
//==========================================================================

VDo::VDo(VExpression* AExpr, VStatement* AStatement, const TLocation& ALoc)
: VStatement(ALoc)
, Expr(AExpr)
, Statement(AStatement)
{
}

//==========================================================================
//
//	VDo::~VDo
//
//==========================================================================

VDo::~VDo()
{
	if (Expr)
		delete Expr;
	if (Statement)
		delete Statement;
}

//==========================================================================
//
//	VDo::Resolve
//
//==========================================================================

bool VDo::Resolve(VEmitContext& ec)
{
	bool Ret = true;

	NumLocalsOnStart = ec.LocalDefs.Num();
	if (Expr)
	{
		Expr = Expr->ResolveBoolean(ec);
	}
	if (!Expr)
	{
		Ret = false;
	}

	if (!Statement->Resolve(ec))
	{
		Ret = false;
	}

	return Ret;
}

//==========================================================================
//
//	VDo::DoEmit
//
//==========================================================================

void VDo::DoEmit(VEmitContext& ec)
{
	int PrevBreakLocalsStart = ec.BreakNumLocalsOnStart;
	int PrevContinueLocalsStart = ec.ContinueNumLocalsOnStart;
	ec.BreakNumLocalsOnStart = NumLocalsOnStart;
	ec.ContinueNumLocalsOnStart = NumLocalsOnStart;
	ec.BreakLevel++;
	ec.ContinueLevel++;
	int topAddr = ec.GetNumInstructions();
	Statement->Emit(ec);
	int exprAddr = ec.GetNumInstructions();
	Expr->Emit(ec);
	ec.AddStatement(OPC_IfGoto, topAddr);
	ec.WriteContinues(exprAddr);
	ec.WriteBreaks();
	ec.BreakNumLocalsOnStart = PrevBreakLocalsStart;
	ec.ContinueNumLocalsOnStart = PrevContinueLocalsStart;
}

//END

//BEGIN VFor

//==========================================================================
//
//	VFor::VFor
//
//==========================================================================

VFor::VFor(const TLocation& ALoc)
: VStatement(ALoc)
, CondExpr(NULL)
, Statement(NULL)
{
}

//==========================================================================
//
//	VFor::~VFor
//
//==========================================================================

VFor::~VFor()
{
	for (int i = 0; i < InitExpr.Num(); i++)
		if (InitExpr[i])
			delete InitExpr[i];
	if (CondExpr)
		delete CondExpr;
	for (int i = 0; i < LoopExpr.Num(); i++)
		if (LoopExpr[i])
			delete LoopExpr[i];
	if (Statement)
		delete Statement;
}

//==========================================================================
//
//	VFor::Resolve
//
//==========================================================================

bool VFor::Resolve(VEmitContext& ec)
{
	bool Ret = true;

	NumLocalsOnStart = ec.LocalDefs.Num();

	for (int i = 0; i < InitExpr.Num(); i++)
	{
		InitExpr[i] = InitExpr[i]->Resolve(ec);
		if (!InitExpr[i])
		{
			Ret = false;
		}
	}

	if (CondExpr)
	{
		CondExpr = CondExpr->ResolveBoolean(ec);
		if (!CondExpr)
		{
			Ret = false;
		}
	}

	for (int i = 0; i < LoopExpr.Num(); i++)
	{
		LoopExpr[i] = LoopExpr[i]->Resolve(ec);
		if (!LoopExpr[i])
		{
			Ret = false;
		}
	}

	if (!Statement->Resolve(ec))
	{
		Ret = false;
	}

	return Ret;
}

//==========================================================================
//
//	VFor::DoEmit
//
//==========================================================================

void VFor::DoEmit(VEmitContext& ec)
{
	int PrevBreakLocalsStart = ec.BreakNumLocalsOnStart;
	int PrevContinueLocalsStart = ec.ContinueNumLocalsOnStart;
	ec.BreakNumLocalsOnStart = NumLocalsOnStart;
	ec.ContinueNumLocalsOnStart = NumLocalsOnStart;
	ec.BreakLevel++;
	ec.ContinueLevel++;
	for (int i = 0; i < InitExpr.Num(); i++)
	{
		InitExpr[i]->Emit(ec);
	}
	int topAddr = ec.GetNumInstructions();
	if (!CondExpr)
	{
		ec.AddStatement(OPC_PushNumber, 1);
	}
	else
	{
		CondExpr->Emit(ec);
	}
	int jumpAddrPtr1 = ec.AddStatement(OPC_IfGoto, 0);
	int jumpAddrPtr2 = ec.AddStatement(OPC_Goto, 0);
	int contAddr = ec.GetNumInstructions();
	for (int i = 0; i < LoopExpr.Num(); i++)
	{
		LoopExpr[i]->Emit(ec);
	}
	ec.AddStatement(OPC_Goto, topAddr);
	ec.FixupJump(jumpAddrPtr1);
	Statement->Emit(ec);
	ec.AddStatement(OPC_Goto, contAddr);
	ec.FixupJump(jumpAddrPtr2);
	ec.WriteContinues(contAddr);
	ec.WriteBreaks();
	ec.BreakNumLocalsOnStart = PrevBreakLocalsStart;
	ec.ContinueNumLocalsOnStart = PrevContinueLocalsStart;
}

//END

//BEGIN VSwitch

//==========================================================================
//
//	VSwitch::VSwitch
//
//==========================================================================

VSwitch::VSwitch(const TLocation& ALoc)
: VStatement(ALoc)
{
}

//==========================================================================
//
//	VSwitch::~VSwitch
//
//==========================================================================

VSwitch::~VSwitch()
{
	if (Expr)
		delete Expr;
	for (int i = 0; i < Statements.Num(); i++)
		if (Statements[i])
			delete Statements[i];
}

//==========================================================================
//
//	VSwitch::Resolve
//
//==========================================================================

bool VSwitch::Resolve(VEmitContext& ec)
{
	bool Ret = true;

	NumLocalsOnStart = ec.LocalDefs.Num();
	if (Expr)
	{
		Expr = Expr->Resolve(ec);
	}
	if (!Expr || Expr->Type.type != ev_int)
	{
		ParseError(Loc, "Int expression expected");
		Ret = false;
	}

	for (int i = 0; i < Statements.Num(); i++)
	{
		if (!Statements[i]->Resolve(ec))
		{
			Ret = false;
		}
	}

	return Ret;
}

//==========================================================================
//
//	VSwitch::DoEmit
//
//==========================================================================

void VSwitch::DoEmit(VEmitContext& ec)
{
	Expr->Emit(ec);

	int switcherAddrPtr = ec.AddStatement(OPC_Goto, 0);
	defaultAddress = -1;
	int PrevBreakLocalsStart = ec.BreakNumLocalsOnStart;
	ec.BreakNumLocalsOnStart = NumLocalsOnStart;
	ec.BreakLevel++;

	for (int i = 0; i < Statements.Num(); i++)
	{
		Statements[i]->Emit(ec);
	}

	int outAddrPtr = ec.AddStatement(OPC_Goto, 0);

	ec.FixupJump(switcherAddrPtr);
	for (int i = 0; i < CaseInfo.Num(); i++)
	{
		if (CaseInfo[i].value >= 0 && CaseInfo[i].value < 256)
		{
			ec.AddStatement(OPC_CaseGotoB, CaseInfo[i].value,
				CaseInfo[i].address);
		}
		else if (CaseInfo[i].value >= MIN_VINT16 &&
			CaseInfo[i].value < MAX_VINT16)
		{
			ec.AddStatement(OPC_CaseGotoS, CaseInfo[i].value,
				CaseInfo[i].address);
		}
		else
		{
			ec.AddStatement(OPC_CaseGoto, CaseInfo[i].value,
				CaseInfo[i].address);
		}
	}
	ec.AddStatement(OPC_Drop);

	if (defaultAddress != -1)
	{
		ec.AddStatement(OPC_Goto, defaultAddress);
	}

	ec.FixupJump(outAddrPtr);

	ec.WriteBreaks();
	ec.BreakNumLocalsOnStart = PrevBreakLocalsStart;
}

//END

//BEGIN VSwitchCase

//==========================================================================
//
//	VSwitchCase::VSwitchCase
//
//==========================================================================

VSwitchCase::VSwitchCase(VSwitch* ASwitch, VExpression* AExpr, const TLocation& ALoc)
: VStatement(ALoc)
, Switch(ASwitch)
, Expr(AExpr)
{
}

//==========================================================================
//
//	VSwitchCase::Resolve
//
//==========================================================================

bool VSwitchCase::Resolve(VEmitContext& ec)
{
	bool Ret = true;
	if (Expr)
	{
		Expr = Expr->Resolve(ec);
	}
	if (!Expr || !Expr->GetIntConst(Value))
	{
		Ret = false;
	}
	return Ret;
}

//==========================================================================
//
//	VSwitchCase::DoEmit
//
//==========================================================================

void VSwitchCase::DoEmit(VEmitContext& ec)
{
	for (int i = 0; i < Switch->CaseInfo.Num(); i++)
	{
		if (Switch->CaseInfo[i].value == Value)
		{
			ParseError(Loc, "Duplicate case value");
			break;
		}
	}
	VSwitch::VCaseInfo& C = Switch->CaseInfo.Alloc();
	C.value = Value;
	C.address = ec.GetNumInstructions();
}

//END

//BEGIN VSwitchDefault

//==========================================================================
//
//	VSwitchDefault::VSwitchDefault
//
//==========================================================================

VSwitchDefault::VSwitchDefault(VSwitch* ASwitch, const TLocation& ALoc)
: VStatement(ALoc)
, Switch(ASwitch)
{}

//==========================================================================
//
//	VSwitchDefault::Resolve
//
//==========================================================================

bool VSwitchDefault::Resolve(VEmitContext&)
{
	return true;
}

//==========================================================================
//
//	VSwitchDefault::DoEmit
//
//==========================================================================

void VSwitchDefault::DoEmit(VEmitContext& ec)
{
	if (Switch->defaultAddress != -1)
	{
		ParseError(Loc, "Only 1 DEFAULT per switch allowed.");
	}
	Switch->defaultAddress = ec.GetNumInstructions();
}

//END

//BEGIN VBreak

//==========================================================================
//
//	VBreak::VBreak
//
//==========================================================================

VBreak::VBreak(const TLocation& ALoc)
: VStatement(ALoc)
{
}

//==========================================================================
//
//	VBreak::Resolve
//
//==========================================================================

bool VBreak::Resolve(VEmitContext& ec)
{
	NumLocalsEnd = ec.LocalDefs.Num();
	return true;
}

//==========================================================================
//
//	VBreak::DoEmit
//
//==========================================================================

void VBreak::DoEmit(VEmitContext& ec)
{
	if (!ec.BreakLevel)
	{
		ParseError(Loc, "Misplaced BREAK statement.");
	}

	ec.EmitClearStrings(ec.BreakNumLocalsOnStart, NumLocalsEnd);

	breakInfo_t& B = ec.BreakInfo.Alloc();
	B.level = ec.BreakLevel;
	B.addressPtr = ec.AddStatement(OPC_Goto, 0);
}

//END

//BEGIN VContinue

//==========================================================================
//
//	VContinue::VContinue
//
//==========================================================================

VContinue::VContinue(const TLocation& ALoc)
: VStatement(ALoc)
{
}

//==========================================================================
//
//	VContinue::Resolve
//
//==========================================================================

bool VContinue::Resolve(VEmitContext& ec)
{
	NumLocalsEnd = ec.LocalDefs.Num();
	return true;
}

//==========================================================================
//
//	VContinue::DoEmit
//
//==========================================================================

void VContinue::DoEmit(VEmitContext& ec)
{
	if (!ec.ContinueLevel)
	{
		ParseError(Loc, "Misplaced CONTINUE statement.");
	}

	ec.EmitClearStrings(ec.ContinueNumLocalsOnStart, NumLocalsEnd);

	continueInfo_t& C = ec.ContinueInfo.Alloc();
	C.level = ec.ContinueLevel;
	C.addressPtr = ec.AddStatement(OPC_Goto, 0);
}

//END

//BEGIN VReturn

//==========================================================================
//
//	VReturn::VReturn
//
//==========================================================================

VReturn::VReturn(VExpression* AExpr, const TLocation& ALoc)
: VStatement(ALoc)
, Expr(AExpr)
{
}

//==========================================================================
//
//	VReturn::~VReturn
//
//==========================================================================

VReturn::~VReturn()
{
	if (Expr)
		delete Expr;
}

//==========================================================================
//
//	VReturn::Resolve
//
//==========================================================================

bool VReturn::Resolve(VEmitContext& ec)
{
	NumLocalsToClear = ec.LocalDefs.Num();
	bool Ret = true;
	if (Expr)
	{
		Expr = Expr->Resolve(ec);
		if (ec.FuncRetType.type == ev_void)
		{
			ParseError(Loc, "viod function cannot return a value.");
			Ret = false;
		}
		else
		{
			Expr->Type.CheckMatch(Expr->Loc, ec.FuncRetType);
		}
	}
	else
	{
		if (ec.FuncRetType.type != ev_void)
		{
			ParseError(Loc, "Return value expected.");
			Ret = false;
		}
	}
	return Ret;
}

//==========================================================================
//
//	VReturn::DoEmit
//
//==========================================================================

void VReturn::DoEmit(VEmitContext& ec)
{
	if (Expr)
	{
		Expr->Emit(ec);
		ec.EmitClearStrings(0, NumLocalsToClear);
		if (Expr->Type.GetSize() == 4)
		{
			ec.AddStatement(OPC_ReturnL);
		}
		else if (Expr->Type.type == ev_vector)
		{
			ec.AddStatement(OPC_ReturnV);
		}
		else
		{
			ParseError(Loc, "Bad return type");
		}
	}
	else
	{
		ec.EmitClearStrings(0, NumLocalsToClear);
		ec.AddStatement(OPC_Return);
	}
}

//END

//BEGIN VExpressionStatement

//==========================================================================
//
//	VExpressionStatement::VExpressionStatement
//
//==========================================================================

VExpressionStatement::VExpressionStatement(VExpression* AExpr)
: VStatement(AExpr->Loc)
, Expr(AExpr)
{
}

//==========================================================================
//
//	VExpressionStatement::~VExpressionStatement
//
//==========================================================================

VExpressionStatement::~VExpressionStatement()
{
	if (Expr)
		delete Expr;
}

//==========================================================================
//
//	VExpressionStatement::Resolve
//
//==========================================================================

bool VExpressionStatement::Resolve(VEmitContext& ec)
{
	bool Ret = true;
	if (Expr)
		Expr = Expr->Resolve(ec);
	if (!Expr)
		Ret = false;
	return Ret;
}

//==========================================================================
//
//	VExpressionStatement::DoEmit
//
//==========================================================================

void VExpressionStatement::DoEmit(VEmitContext& ec)
{
	Expr->Emit(ec);
}

//END

//BEGIN VLocalVarStatement

//==========================================================================
//
//	VLocalVarStatement::VLocalVarStatement
//
//==========================================================================

VLocalVarStatement::VLocalVarStatement(VLocalDecl* ADecl)
: VStatement(ADecl->Loc)
, Decl(ADecl)
{
}

//==========================================================================
//
//	VLocalVarStatement::~VLocalVarStatement
//
//==========================================================================

VLocalVarStatement::~VLocalVarStatement()
{
	if (Decl)
		delete Decl;
}

//==========================================================================
//
//	VLocalVarStatement::Resolve
//
//==========================================================================

bool VLocalVarStatement::Resolve(VEmitContext& ec)
{
	bool Ret = true;
	Decl->Declare(ec);
	return Ret;
}

//==========================================================================
//
//	VLocalVarStatement::DoEmit
//
//==========================================================================

void VLocalVarStatement::DoEmit(VEmitContext& ec)
{
	Decl->EmitInitialisations(ec);
}

//END

//BEGIN VCompound

//==========================================================================
//
//	VCompound::VCompound
//
//==========================================================================

VCompound::VCompound(const TLocation& ALoc)
: VStatement(ALoc)
{
}

//==========================================================================
//
//	VCompound::~VCompound
//
//==========================================================================

VCompound::~VCompound()
{
	for (int i = 0; i < Statements.Num(); i++)
		if (Statements[i])
			delete Statements[i];
}

//==========================================================================
//
//	VCompound::Resolve
//
//==========================================================================

bool VCompound::Resolve(VEmitContext& ec)
{
	bool Ret = true;
	NumLocalsOnStart = ec.LocalDefs.Num();
	for (int i = 0; i < Statements.Num(); i++)
	{
		if (!Statements[i]->Resolve(ec))
		{
			Ret = false;
		}
	}
	NumLocalsOnEnd = ec.LocalDefs.Num();

	for (int i = NumLocalsOnStart; i < ec.LocalDefs.Num(); i++)
		ec.LocalDefs[i].Visible = false;

	return Ret;
}

//==========================================================================
//
//	VCompound::DoEmit
//
//==========================================================================

void VCompound::DoEmit(VEmitContext& ec)
{
	for (int i = 0; i < Statements.Num(); i++)
	{
		Statements[i]->Emit(ec);
	}
	ec.EmitClearStrings(NumLocalsOnStart, NumLocalsOnEnd);
	for (int i = NumLocalsOnStart; i < NumLocalsOnEnd; i++)
		ec.LocalDefs[i].Cleared = true;
}

//END
