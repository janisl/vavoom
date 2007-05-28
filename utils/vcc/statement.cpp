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
	VLabel FalseTarget = ec.DefineLabel();

	//	Expression.
	Expr->EmitBranchable(ec, FalseTarget, false);

	//	True statement
	TrueStatement->Emit(ec);
	if (FalseStatement)
	{
		//	False statement
		VLabel End = ec.DefineLabel();
		ec.AddStatement(OPC_Goto, End);
		ec.MarkLabel(FalseTarget);
		FalseStatement->Emit(ec);
		ec.MarkLabel(End);
	}
	else
	{
		ec.MarkLabel(FalseTarget);
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
	VLabel OldStart = ec.LoopStart;
	VLabel OldEnd = ec.LoopEnd;

	VLabel Loop = ec.DefineLabel();
	ec.LoopStart = ec.DefineLabel();
	ec.LoopEnd = ec.DefineLabel();

	ec.AddStatement(OPC_Goto, ec.LoopStart);
	ec.MarkLabel(Loop);
	Statement->Emit(ec);
	ec.MarkLabel(ec.LoopStart);
	Expr->EmitBranchable(ec, Loop, true);
	ec.MarkLabel(ec.LoopEnd);

	ec.LoopStart = OldStart;
	ec.LoopEnd = OldEnd;
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
	VLabel OldStart = ec.LoopStart;
	VLabel OldEnd = ec.LoopEnd;

	VLabel Loop = ec.DefineLabel();
	ec.LoopStart = ec.DefineLabel();
	ec.LoopEnd = ec.DefineLabel();

	ec.MarkLabel(Loop);
	Statement->Emit(ec);
	ec.MarkLabel(ec.LoopStart);
	Expr->EmitBranchable(ec, Loop, true);
	ec.MarkLabel(ec.LoopEnd);

	ec.LoopStart = OldStart;
	ec.LoopEnd = OldEnd;
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
	//	Set-up continues and breaks.
	VLabel OldStart = ec.LoopStart;
	VLabel OldEnd = ec.LoopEnd;

	//	Define labels.
	ec.LoopStart = ec.DefineLabel();
	ec.LoopEnd = ec.DefineLabel();
	VLabel Test = ec.DefineLabel();
	VLabel Loop = ec.DefineLabel();

	//	Emit initialisation expressions.
	for (int i = 0; i < InitExpr.Num(); i++)
	{
		InitExpr[i]->Emit(ec);
	}

	//	Jump to test if it's present.
	if (CondExpr)
	{
		ec.AddStatement(OPC_Goto, Test);
	}

	//	Emit embeded statement.
	ec.MarkLabel(Loop);
	Statement->Emit(ec);

	//	Emit per-loop expression statements.
	ec.MarkLabel(ec.LoopStart);
	for (int i = 0; i < LoopExpr.Num(); i++)
	{
		LoopExpr[i]->Emit(ec);
	}

	//	Loop test.
	ec.MarkLabel(Test);
	if (!CondExpr)
	{
		ec.AddStatement(OPC_Goto, Loop);
	}
	else
	{
		CondExpr->EmitBranchable(ec, Loop, true);
	}

	//	End of loop.
	ec.MarkLabel(ec.LoopEnd);

	//	Restore continue and break state.
	ec.LoopStart = OldStart;
	ec.LoopEnd = OldEnd;
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
, HaveDefault(false)
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

	if (Expr)
	{
		Expr = Expr->Resolve(ec);
	}
	if (!Expr || Expr->Type.type != TYPE_Int)
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
	VLabel OldEnd = ec.LoopEnd;

	Expr->Emit(ec);

	ec.LoopEnd = ec.DefineLabel();

	//	Case table.
	for (int i = 0; i < CaseInfo.Num(); i++)
	{
		CaseInfo[i].Address = ec.DefineLabel();
		if (CaseInfo[i].Value >= 0 && CaseInfo[i].Value < 256)
		{
			ec.AddStatement(OPC_CaseGotoB, CaseInfo[i].Value,
				CaseInfo[i].Address);
		}
		else if (CaseInfo[i].Value >= MIN_VINT16 &&
			CaseInfo[i].Value < MAX_VINT16)
		{
			ec.AddStatement(OPC_CaseGotoS, CaseInfo[i].Value,
				CaseInfo[i].Address);
		}
		else
		{
			ec.AddStatement(OPC_CaseGoto, CaseInfo[i].Value,
				CaseInfo[i].Address);
		}
	}
	ec.AddStatement(OPC_Drop);

	//	Go to default case if we have one, otherwise to the end of switch.
	if (HaveDefault)
	{
		DefaultAddress = ec.DefineLabel();
		ec.AddStatement(OPC_Goto, DefaultAddress);
	}
	else
	{
		ec.AddStatement(OPC_Goto, ec.LoopEnd);
	}

	//	Switch statements.
	for (int i = 0; i < Statements.Num(); i++)
	{
		Statements[i]->Emit(ec);
	}

	ec.MarkLabel(ec.LoopEnd);

	ec.LoopEnd = OldEnd;
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
	if (Expr)
	{
		Expr = Expr->Resolve(ec);
	}
	if (!Expr)
	{
		return false;
	}
	if (!Expr->IsIntConst())
	{
		ParseError(Expr->Loc, "Integer constant expected");
		return false;
	}

	Value = Expr->GetIntConst();
	for (int i = 0; i < Switch->CaseInfo.Num(); i++)
	{
		if (Switch->CaseInfo[i].Value == Value)
		{
			ParseError(Loc, "Duplicate case value");
			break;
		}
	}

	Index = Switch->CaseInfo.Num();
	VSwitch::VCaseInfo& C = Switch->CaseInfo.Alloc();
	C.Value = Value;
	return true;
}

//==========================================================================
//
//	VSwitchCase::DoEmit
//
//==========================================================================

void VSwitchCase::DoEmit(VEmitContext& ec)
{
	ec.MarkLabel(Switch->CaseInfo[Index].Address);
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
	if (Switch->HaveDefault)
	{
		ParseError(Loc, "Only 1 DEFAULT per switch allowed.");
		return false;
	}
	Switch->HaveDefault = true;
	return true;
}

//==========================================================================
//
//	VSwitchDefault::DoEmit
//
//==========================================================================

void VSwitchDefault::DoEmit(VEmitContext& ec)
{
	ec.MarkLabel(Switch->DefaultAddress);
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

bool VBreak::Resolve(VEmitContext&)
{
	return true;
}

//==========================================================================
//
//	VBreak::DoEmit
//
//==========================================================================

void VBreak::DoEmit(VEmitContext& ec)
{
	if (!ec.LoopEnd.IsDefined())
	{
		ParseError(Loc, "Misplaced BREAK statement.");
		return;
	}

	ec.AddStatement(OPC_Goto, ec.LoopEnd);
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

bool VContinue::Resolve(VEmitContext&)
{
	return true;
}

//==========================================================================
//
//	VContinue::DoEmit
//
//==========================================================================

void VContinue::DoEmit(VEmitContext& ec)
{
	if (!ec.LoopStart.IsDefined())
	{
		ParseError(Loc, "Misplaced CONTINUE statement.");
		return;
	}

	ec.AddStatement(OPC_Goto, ec.LoopStart);
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
		if (ec.FuncRetType.type == TYPE_Void)
		{
			ParseError(Loc, "viod function cannot return a value.");
			Ret = false;
		}
		else if (Expr)
		{
			Expr->Type.CheckMatch(Expr->Loc, ec.FuncRetType);
		}
		else
		{
			Ret = false;
		}
	}
	else
	{
		if (ec.FuncRetType.type != TYPE_Void)
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
		else if (Expr->Type.type == TYPE_Vector)
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
	int NumLocalsOnStart = ec.LocalDefs.Num();
	for (int i = 0; i < Statements.Num(); i++)
	{
		if (!Statements[i]->Resolve(ec))
		{
			Ret = false;
		}
	}

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
}

//END
