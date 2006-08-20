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

void VStatement::Emit()
{
	DoEmit();
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

bool VEmptyStatement::Resolve()
{
	return true;
}

//==========================================================================
//
//	VEmptyStatement::DoEmit
//
//==========================================================================

void VEmptyStatement::DoEmit()
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

bool VIf::Resolve()
{
	bool Ret = true;

	if (Expr)
	{
		Expr = Expr->ResolveTopLevel();
	}
	if (!Expr || !Expr->Type.CheckSizeIs4(Loc))
	{
		Ret = false;
	}

	if (!TrueStatement->Resolve())
	{
		Ret = false;
	}

	if (FalseStatement && !FalseStatement->Resolve())
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

void VIf::DoEmit()
{
	//	Expression.
	Expr->Emit();
	Expr->Type.EmitToBool();

	//	True statement
	int jumpAddrPtr1 = AddStatement(OPC_IfNotGoto, 0);
	TrueStatement->Emit();
	if (FalseStatement)
	{
		//	False statement
		int jumpAddrPtr2 = AddStatement(OPC_Goto, 0);
		FixupJump(jumpAddrPtr1);
		FalseStatement->Emit();
		FixupJump(jumpAddrPtr2);
	}
	else
	{
		FixupJump(jumpAddrPtr1);
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

bool VWhile::Resolve()
{
	bool Ret = true;

	NumLocalsOnStart = numlocaldefs;
	if (Expr)
	{
		Expr = Expr->ResolveTopLevel();
	}
	if (!Expr || !Expr->Type.CheckSizeIs4(Loc))
	{
		Ret = false;
	}

	if (!Statement->Resolve())
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

void VWhile::DoEmit()
{
	int PrevBreakLocalsStart = BreakNumLocalsOnStart;
	int PrevContinueLocalsStart = BreakNumLocalsOnStart;
	BreakNumLocalsOnStart = NumLocalsOnStart;
	ContinueNumLocalsOnStart = NumLocalsOnStart;
	BreakLevel++;
	ContinueLevel++;
	int topAddr = GetNumInstructions();
	Expr->Emit();
	Expr->Type.EmitToBool();
	int outAddrPtr = AddStatement(OPC_IfNotGoto, 0);
	Statement->Emit();
	AddStatement(OPC_Goto, topAddr);
	FixupJump(outAddrPtr);
	WriteContinues(topAddr);
	WriteBreaks();
	BreakNumLocalsOnStart = PrevBreakLocalsStart;
	BreakNumLocalsOnStart = PrevContinueLocalsStart;
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

bool VDo::Resolve()
{
	bool Ret = true;

	NumLocalsOnStart = numlocaldefs;
	if (Expr)
	{
		Expr = Expr->ResolveTopLevel();
	}
	if (!Expr || !Expr->Type.CheckSizeIs4(Loc))
	{
		Ret = false;
	}

	if (!Statement->Resolve())
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

void VDo::DoEmit()
{
	int PrevBreakLocalsStart = BreakNumLocalsOnStart;
	int PrevContinueLocalsStart = BreakNumLocalsOnStart;
	BreakNumLocalsOnStart = NumLocalsOnStart;
	ContinueNumLocalsOnStart = NumLocalsOnStart;
	BreakLevel++;
	ContinueLevel++;
	int topAddr = GetNumInstructions();
	Statement->Emit();
	int exprAddr = GetNumInstructions();
	Expr->Emit();
	Expr->Type.EmitToBool();
	AddStatement(OPC_IfGoto, topAddr);
	WriteContinues(exprAddr);
	WriteBreaks();
	BreakNumLocalsOnStart = PrevBreakLocalsStart;
	BreakNumLocalsOnStart = PrevContinueLocalsStart;
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

bool VFor::Resolve()
{
	bool Ret = true;

	for (int i = 0; i < InitExpr.Num(); i++)
	{
		InitExpr[i] = InitExpr[i]->ResolveTopLevel();
		if (!InitExpr[i])
		{
			Ret = false;
		}
	}

	if (CondExpr)
	{
		CondExpr = CondExpr->ResolveTopLevel();
		if (!CondExpr || !CondExpr->Type.CheckSizeIs4(Loc))
		{
			Ret = false;
		}
	}

	for (int i = 0; i < LoopExpr.Num(); i++)
	{
		LoopExpr[i] = LoopExpr[i]->ResolveTopLevel();
		if (!LoopExpr[i])
		{
			Ret = false;
		}
	}

	if (!Statement->Resolve())
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

void VFor::DoEmit()
{
	int PrevBreakLocalsStart = BreakNumLocalsOnStart;
	int PrevContinueLocalsStart = BreakNumLocalsOnStart;
	BreakNumLocalsOnStart = NumLocalsOnStart;
	ContinueNumLocalsOnStart = NumLocalsOnStart;
	BreakLevel++;
	ContinueLevel++;
	for (int i = 0; i < InitExpr.Num(); i++)
	{
		InitExpr[i]->Emit();
		AddDrop(InitExpr[i]->Type);
	}
	int topAddr = GetNumInstructions();
	if (!CondExpr)
	{
		AddStatement(OPC_PushNumber, 1);
	}
	else
	{
		CondExpr->Emit();
		CondExpr->Type.EmitToBool();
	}
	int jumpAddrPtr1 = AddStatement(OPC_IfGoto, 0);
	int jumpAddrPtr2 = AddStatement(OPC_Goto, 0);
	int contAddr = GetNumInstructions();
	for (int i = 0; i < LoopExpr.Num(); i++)
	{
		LoopExpr[i]->Emit();
		AddDrop(LoopExpr[i]->Type);
	}
	AddStatement(OPC_Goto, topAddr);
	FixupJump(jumpAddrPtr1);
	Statement->Emit();
	AddStatement(OPC_Goto, contAddr);
	FixupJump(jumpAddrPtr2);
	WriteContinues(contAddr);
	WriteBreaks();
	BreakNumLocalsOnStart = PrevBreakLocalsStart;
	BreakNumLocalsOnStart = PrevContinueLocalsStart;
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

bool VSwitch::Resolve()
{
	bool Ret = true;

	NumLocalsOnStart = numlocaldefs;
	if (Expr)
	{
		Expr = Expr->ResolveTopLevel();
	}
	if (!Expr || Expr->Type.type != ev_int)
	{
		ParseError(Loc, "Int expression expected");
		Ret = false;
	}

	for (int i = 0; i < Statements.Num(); i++)
	{
		if (!Statements[i]->Resolve())
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

void VSwitch::DoEmit()
{
	Expr->Emit();

	int switcherAddrPtr = AddStatement(OPC_Goto, 0);
	defaultAddress = -1;
	int PrevBreakLocalsStart = BreakNumLocalsOnStart;
	BreakNumLocalsOnStart = NumLocalsOnStart;
	BreakLevel++;

	for (int i = 0; i < Statements.Num(); i++)
	{
		Statements[i]->Emit();
	}

	int outAddrPtr = AddStatement(OPC_Goto, 0);

	FixupJump(switcherAddrPtr);
	for (int i = 0; i < CaseInfo.Num(); i++)
	{
		if (CaseInfo[i].value >= 0 && CaseInfo[i].value < 256)
		{
			AddStatement(OPC_CaseGotoB, CaseInfo[i].value,
				CaseInfo[i].address);
		}
		else if (CaseInfo[i].value >= MIN_VINT16 &&
			CaseInfo[i].value < MAX_VINT16)
		{
			AddStatement(OPC_CaseGotoS, CaseInfo[i].value,
				CaseInfo[i].address);
		}
		else
		{
			AddStatement(OPC_CaseGoto, CaseInfo[i].value,
				CaseInfo[i].address);
		}
	}
	AddStatement(OPC_Drop);

	if (defaultAddress != -1)
	{
		AddStatement(OPC_Goto, defaultAddress);
	}

	FixupJump(outAddrPtr);

	WriteBreaks();
	BreakNumLocalsOnStart = PrevBreakLocalsStart;
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

bool VSwitchCase::Resolve()
{
	bool Ret = true;
	if (Expr)
	{
		Expr = Expr->Resolve();
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

void VSwitchCase::DoEmit()
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
	C.address = GetNumInstructions();
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

bool VSwitchDefault::Resolve()
{
	return true;
}

//==========================================================================
//
//	VSwitchDefault::DoEmit
//
//==========================================================================

void VSwitchDefault::DoEmit()
{
	if (Switch->defaultAddress != -1)
	{
		ParseError(Loc, "Only 1 DEFAULT per switch allowed.");
	}
	Switch->defaultAddress = GetNumInstructions();
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
	NumLocalsEnd = numlocaldefs;
}

//==========================================================================
//
//	VBreak::Resolve
//
//==========================================================================

bool VBreak::Resolve()
{
	return true;
}

//==========================================================================
//
//	VBreak::DoEmit
//
//==========================================================================

void VBreak::DoEmit()
{
	if (!BreakLevel)
	{
		ParseError(Loc, "Misplaced BREAK statement.");
	}

	EmitClearStrings(BreakNumLocalsOnStart, NumLocalsEnd);

	breakInfo_t& B = BreakInfo.Alloc();
	B.level = BreakLevel;
	B.addressPtr = AddStatement(OPC_Goto, 0);
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
	NumLocalsEnd = numlocaldefs;
}

//==========================================================================
//
//	VContinue::Resolve
//
//==========================================================================

bool VContinue::Resolve()
{
	return true;
}

//==========================================================================
//
//	VContinue::DoEmit
//
//==========================================================================

void VContinue::DoEmit()
{
	if (!ContinueLevel)
	{
		ParseError(Loc, "Misplaced CONTINUE statement.");
	}

	EmitClearStrings(ContinueNumLocalsOnStart, NumLocalsEnd);

	continueInfo_t& C = ContinueInfo.Alloc();
	C.level = ContinueLevel;
	C.addressPtr = AddStatement(OPC_Goto, 0);
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
	NumLocalsToClear = numlocaldefs;
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

bool VReturn::Resolve()
{
	bool Ret = true;
	if (Expr)
	{
		Expr = Expr->ResolveTopLevel();
		if (FuncRetType.type == ev_void)
		{
			ParseError(Loc, "viod function cannot return a value.");
			Ret = false;
		}
		else
		{
			Expr->Type.CheckMatch(FuncRetType);
		}
	}
	else
	{
		if (FuncRetType.type != ev_void)
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

void VReturn::DoEmit()
{
	if (Expr)
	{
		Expr->Emit();
		EmitClearStrings(0, NumLocalsToClear);
		if (Expr->Type.GetSize() == 4)
		{
			AddStatement(OPC_ReturnL);
		}
		else if (Expr->Type.type == ev_vector)
		{
			AddStatement(OPC_ReturnV);
		}
		else
		{
			ParseError(Loc, "Bad return type");
		}
	}
	else
	{
		EmitClearStrings(0, NumLocalsToClear);
		AddStatement(OPC_Return);
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

bool VExpressionStatement::Resolve()
{
	bool Ret = true;
	if (Expr)
		Expr = Expr->ResolveTopLevel();
	if (!Expr)
		Ret = false;
	return Ret;
}

//==========================================================================
//
//	VExpressionStatement::DoEmit
//
//==========================================================================

void VExpressionStatement::DoEmit()
{
	Expr->Emit();
	AddDrop(Expr->Type);
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

bool VLocalVarStatement::Resolve()
{
	bool Ret = true;
	Decl->Declare();
	return Ret;
}

//==========================================================================
//
//	VLocalVarStatement::DoEmit
//
//==========================================================================

void VLocalVarStatement::DoEmit()
{
	Decl->EmitInitialisations();
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

bool VCompound::Resolve()
{
	bool Ret = true;
	NumLocalsOnStart = numlocaldefs;
	for (int i = 0; i < Statements.Num(); i++)
	{
		if (!Statements[i]->Resolve())
		{
			Ret = false;
		}
	}
	NumLocalsOnEnd = numlocaldefs;

	if (maxlocalsofs < localsofs)
		maxlocalsofs = localsofs;
	for (int i = NumLocalsOnStart; i < numlocaldefs; i++)
		localdefs[i].Visible = false;

	return Ret;
}

//==========================================================================
//
//	VCompound::DoEmit
//
//==========================================================================

void VCompound::DoEmit()
{
	for (int i = 0; i < Statements.Num(); i++)
	{
		Statements[i]->Emit();
	}
	EmitClearStrings(NumLocalsOnStart, NumLocalsOnEnd);
	for (int i = NumLocalsOnStart; i < NumLocalsOnEnd; i++)
		localdefs[i].Cleared = true;
}

//END
