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

// TYPES -------------------------------------------------------------------

static void WriteBreaks();
static void WriteContinues(int address);
static void EmitClearStrings(int Start, int End);
static void AddDrop(const TType& type);

struct breakInfo_t
{
	int		level;
	int		addressPtr;
};

struct continueInfo_t
{
	int		level;
	int		addressPtr;
};

static TArray<breakInfo_t>		BreakInfo;
static int						BreakLevel;
static int						BreakNumLocalsOnStart;
static TArray<continueInfo_t> 	ContinueInfo;
static int						ContinueLevel;
static int						ContinueNumLocalsOnStart;

class VStatement
{
public:
	TLocation		Loc;

	VStatement()
	{}
	VStatement(const TLocation& ALoc)
	: Loc(ALoc)
	{}
	virtual ~VStatement()
	{}
	virtual void DoEmit() = 0;
	void Emit()
	{
		DoEmit();
	}
};

class VEmptyStatement : public VStatement
{
public:
	VEmptyStatement(const TLocation& ALoc)
	: VStatement(ALoc)
	{}
	void DoEmit()
	{
	}
};

class VIf : public VStatement
{
public:
	VExpression*	Expr;
	VStatement*		TrueStatement;
	VStatement*		FalseStatement;

	VIf(VExpression* AExpr, VStatement* ATrueStatement, const TLocation& ALoc)
	: VStatement(ALoc)
	, Expr(AExpr)
	, TrueStatement(ATrueStatement)
	, FalseStatement(NULL)
	{
	}
	VIf(VExpression* AExpr, VStatement* ATrueStatement,
		VStatement* AFalseStatement, const TLocation& ALoc)
	: VStatement(ALoc)
	, Expr(AExpr)
	, TrueStatement(ATrueStatement)
	, FalseStatement(AFalseStatement)
	{}
	~VIf()
	{
		if (Expr)
			delete Expr;
		if (TrueStatement)
			delete TrueStatement;
		if (FalseStatement)
			delete FalseStatement;
	}
	void DoEmit()
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
};

class VWhile : public VStatement
{
public:
	VExpression*		Expr;
	VStatement*			Statement;
	int					NumLocalsOnStart;

	VWhile(VExpression* AExpr, VStatement* AStatement, int ANumLocalsOnStart, const TLocation& ALoc)
	: VStatement(ALoc)
	, Expr(AExpr)
	, Statement(AStatement)
	, NumLocalsOnStart(ANumLocalsOnStart)
	{
	}
	~VWhile()
	{
		if (Expr)
			delete Expr;
		if (Statement)
			delete Statement;
	}
	void DoEmit()
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
};

class VDo : public VStatement
{
public:
	VExpression*		Expr;
	VStatement*			Statement;
	int					NumLocalsOnStart;

	VDo(VExpression* AExpr, VStatement* AStatement, int ANumLocalsOnStart, const TLocation& ALoc)
	: VStatement(ALoc)
	, Expr(AExpr)
	, Statement(AStatement)
	, NumLocalsOnStart(ANumLocalsOnStart)
	{
	}
	~VDo()
	{
		if (Expr)
			delete Expr;
		if (Statement)
			delete Statement;
	}
	void DoEmit()
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
};

class VFor : public VStatement
{
public:
	TArray<VExpression*>	InitExpr;
	VExpression*			CondExpr;
	TArray<VExpression*>	LoopExpr;
	VStatement*				Statement;
	int						NumLocalsOnStart;

	VFor(const TLocation& ALoc)
	: VStatement(ALoc)
	, CondExpr(NULL)
	, Statement(NULL)
	{
	}
	~VFor()
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
	void DoEmit()
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
};

class VSwitch : public VStatement
{
public:
	struct VCaseInfo
	{
		int		value;
		int		address;
	};

	VExpression*		Expr;
	TArray<VCaseInfo>	CaseInfo;
	int					defaultAddress;
	TArray<VStatement*>	Statements;
	int					NumLocalsOnStart;

	VSwitch(const TLocation& ALoc)
	: VStatement(ALoc)
	{
	}
	~VSwitch()
	{
		if (Expr)
			delete Expr;
		for (int i = 0; i < Statements.Num(); i++)
			if (Statements[i])
				delete Statements[i];
	}
	void DoEmit()
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
};

class VSwitchCase : public VStatement
{
public:
	VSwitch*		Switch;
	vint32			Value;

	VSwitchCase(VSwitch* ASwitch, vint32 AValue, const TLocation& ALoc)
	: VStatement(ALoc)
	, Switch(ASwitch)
	, Value(AValue)
	{}
	void DoEmit()
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
};

class VSwitchDefault : public VStatement
{
public:
	VSwitch*		Switch;

	VSwitchDefault(VSwitch* ASwitch, const TLocation& ALoc)
	: VStatement(ALoc)
	, Switch(ASwitch)
	{}
	void DoEmit()
	{
		if (Switch->defaultAddress != -1)
		{
			ParseError(Loc, "Only 1 DEFAULT per switch allowed.");
		}
		Switch->defaultAddress = GetNumInstructions();
	}
};

class VBreak : public VStatement
{
public:
	int				NumLocalsEnd;

	VBreak(const TLocation& ALoc)
	: VStatement(ALoc)
	{
		NumLocalsEnd = numlocaldefs;
	}
	void DoEmit()
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
};

class VContinue : public VStatement
{
public:
	int				NumLocalsEnd;

	VContinue(const TLocation& ALoc)
	: VStatement(ALoc)
	{
		NumLocalsEnd = numlocaldefs;
	}
	void DoEmit()
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
};

class VReturn : public VStatement
{
public:
	VExpression*		Expr;
	int					NumLocalsToClear;

	VReturn(VExpression* AExpr, const TLocation& ALoc)
	: VStatement(ALoc)
	, Expr(AExpr)
	{
		NumLocalsToClear = numlocaldefs;
	}
	~VReturn()
	{
		if (Expr)
			delete Expr;
	}
	void DoEmit()
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
};

class VExpressionStatement : public VStatement
{
public:
	VExpression*		Expr;

	VExpressionStatement(VExpression* AExpr)
	: VStatement(AExpr->Loc)
	, Expr(AExpr)
	{}
	~VExpressionStatement()
	{
		if (Expr)
			delete Expr;
	}
	void DoEmit()
	{
		Expr->Emit();
		AddDrop(Expr->Type);
	}
};

class VLocalVarStatement : public VStatement
{
public:
	VLocalDecl*		Decl;

	VLocalVarStatement(VLocalDecl* ADecl)
	: VStatement(ADecl->Loc)
	, Decl(ADecl)
	{}
	~VLocalVarStatement()
	{
		if (Decl)
			delete Decl;
	}
	void DoEmit()
	{
		Decl->EmitInitialisations();
	}
};

class VCompound : public VStatement
{
public:
	TArray<VStatement*>		Statements;
	int						NumLocalsOnStart;
	int						NumLocalsOnEnd;

	VCompound(const TLocation& ALoc)
	: VStatement(ALoc)
	{}
	~VCompound()
	{
		for (int i = 0; i < Statements.Num(); i++)
			if (Statements[i])
				delete Statements[i];
	}
	void DoEmit()
	{
		for (int i = 0; i < Statements.Num(); i++)
		{
			Statements[i]->Emit();
		}
		EmitClearStrings(NumLocalsOnStart, NumLocalsOnEnd);
		for (int i = NumLocalsOnStart; i < NumLocalsOnEnd; i++)
			localdefs[i].Cleared = true;
	}
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static VExpression* ParseExpressionPriority13();

static VCompound*	ParseCompoundStatement();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VLocalVarDef			localdefs[MAX_LOCAL_DEFS];
int						numlocaldefs = 1;
int						localsofs = 0;

TType					SelfType;
VClass*					SelfClass;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static bool				CheckForLocal;

static int				maxlocalsofs = 0;
static TType			FuncRetType;

// CODE --------------------------------------------------------------------

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

static VExpression* ParseExpression()
{
	CheckForLocal = false;
	return ParseExpressionPriority14();
}

//==========================================================================
//
//	SkipExpression
//
//==========================================================================

void SkipExpression(bool bLocals = false)
{
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
}

//==========================================================================
//
//	CheckForLocalVar
//
//==========================================================================

int CheckForLocalVar(VName Name)
{
	if (Name == NAME_None)
	{
		return 0;
	}
	for (int i = 1; i < numlocaldefs; i++)
	{
		if (!localdefs[i].Visible)
		{
			continue;
		}
		if (localdefs[i].Name == Name)
		{
			return i;
		}
	}
	return 0;
}

//==========================================================================
//
//	AddDrop
//
//==========================================================================

static void AddDrop(const TType& type)
{
	if (type.type == ev_string)
	{
		AddStatement(OPC_DropStr);
	}
	else if (type.GetSize() == 4)
	{
		AddStatement(OPC_Drop);
	}
	else if (type.type == ev_vector)
	{
		AddStatement(OPC_VDrop);
	}
	else if (type.type != ev_void)
	{
		ParseError("Expression's result type cannot be dropped");
	}
}

//==========================================================================
//
// WriteBreaks
//
//==========================================================================

static void WriteBreaks()
{
	BreakLevel--;
	while (BreakInfo.Num() && BreakInfo[BreakInfo.Num() - 1].level > BreakLevel)
	{
		FixupJump(BreakInfo[BreakInfo.Num() - 1].addressPtr);
		BreakInfo.SetNum(BreakInfo.Num() - 1);
	}
}

//==========================================================================
//
// WriteContinues
//
//==========================================================================

static void WriteContinues(int address)
{
	ContinueLevel--;
	while (ContinueInfo.Num() && ContinueInfo[ContinueInfo.Num() - 1].level > ContinueLevel)
	{
		FixupJump(ContinueInfo[ContinueInfo.Num() - 1].addressPtr, address);
		ContinueInfo.SetNum(ContinueInfo.Num() - 1);
	}
}

//==========================================================================
//
//	EmitClearStrings
//
//==========================================================================

static void EmitClearStrings(int Start, int End)
{
	for (int i = Start; i < End; i++)
	{
		if (localdefs[i].Cleared)
		{
			continue;
		}
		if (localdefs[i].type.type == ev_string)
		{
			EmitLocalAddress(localdefs[i].ofs);
			AddStatement(OPC_ClearPointedStr);
		}
		if (localdefs[i].type.type == ev_struct &&
			localdefs[i].type.Struct->NeedsDestructor())
		{
			EmitLocalAddress(localdefs[i].ofs);
			AddStatement(OPC_ClearPointedStruct, localdefs[i].type.Struct);
		}
		if (localdefs[i].type.type == ev_array)
		{
			if (localdefs[i].type.ArrayInnerType == ev_string)
			{
				for (int j = 0; j < localdefs[i].type.array_dim; j++)
				{
					EmitLocalAddress(localdefs[i].ofs);
					EmitPushNumber(j);
					AddStatement(OPC_ArrayElement, localdefs[i].type.GetArrayInnerType());
					AddStatement(OPC_ClearPointedStr);
				}
			}
			else if (localdefs[i].type.ArrayInnerType == ev_struct &&
				localdefs[i].type.Struct->NeedsDestructor())
			{
				for (int j = 0; j < localdefs[i].type.array_dim; j++)
				{
					EmitLocalAddress(localdefs[i].ofs);
					EmitPushNumber(j);
					AddStatement(OPC_ArrayElement, localdefs[i].type.GetArrayInnerType());
					AddStatement(OPC_ClearPointedStruct, localdefs[i].type.Struct);
				}
			}
		}
	}
}

//==========================================================================
//
//	ParseStatement
//
//==========================================================================

static VStatement* ParseStatement()
{
	TLocation l = tk_Location;
	switch(tk_Token)
	{
	case TK_EOF:
		ERR_Exit(ERR_UNEXPECTED_EOF, true, NULL);
		return NULL;

	case TK_KEYWORD:
		if (TK_Check(KW_IF))
		{
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			VExpression* e = ParseExpression();
			if (!e)
			{
				ParseError(tk_Location, "If expression expected");
			}
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			VStatement* STrue = ParseStatement();
			e = e->ResolveTopLevel();
			e->Type.CheckSizeIs4(l);
			if (TK_Check(KW_ELSE))
			{
				VStatement* SFalse = ParseStatement();
				return new VIf(e, STrue, SFalse, l);
			}
			else
			{
				return new VIf(e, STrue, l);
			}
		}
		else if (TK_Check(KW_WHILE))
		{
			int NumLocalsOnStart = numlocaldefs;
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			VExpression* Expr = ParseExpression();
			if (!Expr)
			{
				ParseError(tk_Location, "Wile loop expression expected");
			}
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			VStatement* Statement = ParseStatement();
			if (!NumErrors)
			{
				Expr = Expr->ResolveTopLevel();
			}
			Expr->Type.CheckSizeIs4(l);
			return new VWhile(Expr, Statement, NumLocalsOnStart, l);
		}
		else if (TK_Check(KW_DO))
		{
			int NumLocalsOnStart = numlocaldefs;
			VStatement* Statement = ParseStatement();
			TK_Expect(KW_WHILE, ERR_BAD_DO_STATEMENT);
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			VExpression* Expr = ParseExpression();
			if (!Expr)
			{
				ParseError(tk_Location, "Do loop expression expected");
			}
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			Expr = Expr->ResolveTopLevel();
			Expr->Type.CheckSizeIs4(l);
			return new VDo(Expr, Statement, NumLocalsOnStart, l);
		}
		else if (TK_Check(KW_FOR))
		{
			VFor* For = new VFor(l);
			For->NumLocalsOnStart = numlocaldefs;
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			do
			{
				VExpression* Expr = ParseExpression();
				if (Expr)
				{
					Expr = Expr->ResolveTopLevel();
					For->InitExpr.Append(Expr);
				}
			} while (TK_Check(PU_COMMA));
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			VExpression* Expr = ParseExpression();
			For->CondExpr = Expr;
			if (For->CondExpr)
			{
				For->CondExpr = For->CondExpr->ResolveTopLevel();
				For->CondExpr->Type.CheckSizeIs4(l);
			}
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			do
			{
				VExpression* Expr = ParseExpression();
				if (Expr)
				{
					Expr = Expr->ResolveTopLevel();
					For->LoopExpr.Append(Expr);
				}
			} while (TK_Check(PU_COMMA));
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
			VStatement* Statement = ParseStatement();
			For->Statement = Statement;
			return For;
		}
		else if (TK_Check(KW_BREAK))
		{
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			return new VBreak(l);
		}
		else if (TK_Check(KW_CONTINUE))
		{
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			return new VContinue(l);
		}
		else if (TK_Check(KW_RETURN))
		{
			VExpression* Expr = NULL;
			if (TK_Check(PU_SEMICOLON))
			{
				if (FuncRetType.type != ev_void)
				{
					ERR_Exit(ERR_NO_RET_VALUE, true, NULL);
				}
			}
			else
			{
				if (FuncRetType.type == ev_void)
				{
					ERR_Exit(ERR_VOID_RET, true, NULL);
				}
				Expr = ParseExpression();
				if (!Expr)
				{
					ParseError(l, "Bad expression");
				}
				else
				{
					Expr = Expr->ResolveTopLevel();
				}
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				Expr->Type.CheckMatch(FuncRetType);
			}
			return new VReturn(Expr, l);
		}
		else if (TK_Check(KW_SWITCH))
		{
			VSwitch* Switch = new VSwitch(l);
			Switch->NumLocalsOnStart = numlocaldefs;
			TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
			VExpression* Expr = ParseExpression();
			if (!Expr)
			{
				ParseError("Switch expression expected");
			}
			else
			{
				Expr = Expr->ResolveTopLevel();
				if (Expr->Type.type != ev_int)
				{
					ParseError(l, "Int expression expected");
				}
			}
			Switch->Expr = Expr;
			TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);

			TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
			do
			{
				l = tk_Location;
				if (TK_Check(KW_CASE))
				{
					vint32 Value = EvalConstExpression(SelfClass, ev_int);
					TK_Expect(PU_COLON, ERR_MISSING_COLON);
					Switch->Statements.Append(new VSwitchCase(Switch, Value, l));
				}
				else if (TK_Check(KW_DEFAULT))
				{
					TK_Expect(PU_COLON, ERR_MISSING_COLON);
					Switch->Statements.Append(new VSwitchDefault(Switch, l));
				}
				else
				{
					VStatement* Statement = ParseStatement();
					Switch->Statements.Append(Statement);
				}
			} while (!TK_Check(PU_RBRACE));
			return Switch;
		}
		else
		{
			TType type = CheckForTypeKeyword();
			if (type.type != ev_unknown)
			{
				VLocalDecl* Decl = ParseLocalVar(type, NAME_None);
				Decl->Declare();
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
				return new VLocalVarStatement(Decl);
			}
			else
			{
				CheckForLocal = true;
				VExpression* Expr = ParseExpressionPriority14();
				if (Expr)
				{
					Expr = Expr->ResolveTopLevel();
					TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
					return new VExpressionStatement(Expr);
				}
				else
				{
					ParseError(l, "Misplaced keyword %s", tk_String);
					TK_NextToken();
					return new VEmptyStatement(l);
				}
			}
		}

	case TK_PUNCT:
		if (TK_Check(PU_LBRACE))
		{
			return ParseCompoundStatement();
		}
		//Fall through

	default:
		CheckForLocal = true;
		VExpression* Expr = ParseExpressionPriority14();
		if (!Expr)
		{
			if (!TK_Check(PU_SEMICOLON))
			{
				ParseError(l, "Token %s makes no sense here", tk_String);
				TK_NextToken();
			}
			return new VEmptyStatement(l);
		}
		else if (Expr->IsSingleName() && tk_Token == TK_IDENTIFIER)
		{
			VLocalDecl* Decl = ParseLocalVar(ev_unknown, ((VSingleName*)Expr)->Name);
			delete Expr;
			Decl->Declare();
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			return new VLocalVarStatement(Decl);
		}
		else
		{
			Expr = Expr->ResolveTopLevel();
			TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			return new VExpressionStatement(Expr);
		}
	}
}

//==========================================================================
//
//	ParseCompoundStatement
//
//==========================================================================

static VCompound* ParseCompoundStatement()
{
	VCompound* Comp = new VCompound(tk_Location);

	Comp->NumLocalsOnStart = numlocaldefs;
	while (!TK_Check(PU_RBRACE))
	{
		VStatement* s = ParseStatement();
		Comp->Statements.Append(s);
	}
	Comp->NumLocalsOnEnd = numlocaldefs;

	if (maxlocalsofs < localsofs)
		maxlocalsofs = localsofs;
	for (int i = Comp->NumLocalsOnStart; i < numlocaldefs; i++)
		localdefs[i].Visible = false;

	return Comp;
}

//==========================================================================
//
//	CompileMethodDef
//
//==========================================================================

void CompileMethodDef(const TType& t, VMethod* Method, VClass* InClass)
{
	numlocaldefs = 1;
	localsofs = 1;

	do
	{
		if (TK_Check(PU_VARARGS))
		{
			break;
		}

		TType type = CheckForType(InClass);

		if (type.type == ev_unknown)
		{
			break;
		}
		while (TK_Check(PU_ASTERISK))
		{
			type = MakePointerType(type);
		}
		if (type.type == ev_void)
		{
static int un = 0;
un++;
//ParseWarning("This is ugly %d", un);
			break;
		}
		type.CheckPassable();

   		if (tk_Token == TK_IDENTIFIER)
		{
			if (CheckForLocalVar(tk_Name))
			{
				ERR_Exit(ERR_REDEFINED_IDENTIFIER, true, "Identifier: %s", *tk_Name);
			}
			localdefs[numlocaldefs].Name = tk_Name;
			localdefs[numlocaldefs].type = type;
			localdefs[numlocaldefs].ofs = localsofs;
			localdefs[numlocaldefs].Visible = true;
			localdefs[numlocaldefs].Cleared = false;
			numlocaldefs++;
			TK_NextToken();
		}
		localsofs += type.GetSize() / 4;
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	maxlocalsofs = localsofs;

	if (Method->Flags & FUNC_Native)
	{
		TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
		return;
	}

	SelfType = TType(InClass);
	SelfClass = InClass;
	BreakLevel = 0;
	ContinueLevel = 0;
	FuncRetType = t;

	BeginCode(Method);
	for (int i = 0; i < numlocaldefs; i++)
	{
		if (localdefs[i].type.type == ev_vector)
		{
			AddStatement(OPC_VFixParam, i);
		}
	}
	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	VStatement* s = ParseCompoundStatement();
	if (!NumErrors)
	{
		s->Emit();
	}
	delete s;

	if (FuncRetType.type == ev_void)
	{
		EmitClearStrings(0, numlocaldefs);
		AddStatement(OPC_Return);
	}
	Method->NumLocals = maxlocalsofs;
	EndCode(Method);
}

//==========================================================================
//
//	SkipDelegate
//
//==========================================================================

void SkipDelegate(VClass* InClass)
{
	TK_NextToken();
	while (TK_Check(PU_ASTERISK));
	TK_NextToken();
	TK_Expect(PU_LPAREN, ERR_MISSING_LPAREN);
	do
	{
		if (TK_Check(PU_VARARGS))
		{
			break;
		}
		TType type = CheckForType(InClass);
		if (type.type == ev_unknown)
		{
			break;
		}
		while (TK_Check(PU_ASTERISK));
		if (tk_Token == TK_IDENTIFIER)
		{
			TK_NextToken();
		}
	} while (TK_Check(PU_COMMA));
	TK_Expect(PU_RPAREN, ERR_MISSING_RPAREN);
	TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
}

//==========================================================================
//
//	CompileStateCode
//
//==========================================================================

void CompileStateCode(VClass* InClass, VMethod* Func)
{
	numlocaldefs = 1;
	localsofs = 1;
	maxlocalsofs = 1;

	BeginCode(Func);

	SelfType = TType(InClass);
	SelfClass = InClass;
	BreakLevel = 0;
	ContinueLevel = 0;
	FuncRetType = TType(ev_void);

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	VStatement* s = ParseCompoundStatement();
	if (!NumErrors)
	{
		s->Emit();
	}
	delete s;
	EmitClearStrings(0, numlocaldefs);
	AddStatement(OPC_Return);
	Func->NumLocals = maxlocalsofs;
	EndCode(Func);
}

//==========================================================================
//
//	CompileDefaultProperties
//
//==========================================================================

void CompileDefaultProperties(VMethod* Method, VClass* InClass)
{
	numlocaldefs = 1;
	localsofs = 1;
	maxlocalsofs = 1;

	SelfType = TType(InClass);
	SelfClass = InClass;
	BreakLevel = 0;
	ContinueLevel = 0;
	FuncRetType = TType(ev_void);

	BeginCode(Method);

	//  Call parent constructor
	if (InClass->ParentClass)
	{
		AddStatement(OPC_LocalAddress0);
		AddStatement(OPC_PushPointedPtr);
		AddStatement(OPC_Call, InClass->ParentClass->DefaultProperties);
	}

	TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
	VStatement* s = ParseCompoundStatement();
	if (!NumErrors)
	{
		s->Emit();
	}
	delete s;
	EmitClearStrings(0, numlocaldefs);
	AddStatement(OPC_Return);
	Method->NumLocals = maxlocalsofs;
	EndCode(Method);
}

//==========================================================================
//
//	PA_Compile
//
//==========================================================================

void PA_Compile()
{
	bool		done;

	dprintf("Compiling pass 2\n");

	TK_NextToken();
	done = false;
	while (!done)
	{
		switch(tk_Token)
		{
		case TK_EOF:
			done = true;
			break;
		case TK_KEYWORD:
			if (TK_Check(KW_IMPORT))
			{
				TK_NextToken();
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			}
			else if (TK_Check(KW_ENUM))
			{
				int val;
				VName Name;

				val = 0;
				TK_Expect(PU_LBRACE, ERR_MISSING_LBRACE);
				do
				{
					if (tk_Token != TK_IDENTIFIER)
					{
						ERR_Exit(ERR_INVALID_IDENTIFIER, true, NULL);
					}
					Name = tk_Name;
					TK_NextToken();
					if (TK_Check(PU_ASSIGN))
					{
						val = EvalConstExpression(NULL, ev_int);
					}
					val++;
				} while (TK_Check(PU_COMMA));
				TK_Expect(PU_RBRACE, ERR_MISSING_RBRACE);
				TK_Expect(PU_SEMICOLON, ERR_MISSING_SEMICOLON);
			}
			else if (TK_Check(KW_STRUCT))
			{
				SkipStruct(NULL);
			}
			else if (TK_Check(KW_CLASS))
			{
				CompileClass();
			}
			else if (TK_Check(KW_VECTOR))
			{
				SkipStruct(NULL);
			}
			else
			{
				ERR_Exit(ERR_INVALID_DECLARATOR, true, "Symbol \"%s\"", tk_String);
			}
			break;

		default:
			ERR_Exit(ERR_INVALID_DECLARATOR, true, "Token type %d, symbol \"%s\"", tk_Token, tk_String);
			break;
	   	}
	}

	if (NumErrors)
	{
		ERR_Exit(ERR_NONE, false, NULL);
	}
}
