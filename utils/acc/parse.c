
//**************************************************************************
//**
//** parse.c
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "common.h"
#include "parse.h"
#include "symbol.h"
#include "pcode.h"
#include "token.h"
#include "error.h"
#include "misc.h"
#include "strlist.h"

// MACROS ------------------------------------------------------------------

#define MAX_STATEMENT_DEPTH 128
#define MAX_BREAK 128
#define MAX_CONTINUE 128
#define MAX_CASE 128
#define EXPR_STACK_DEPTH 64

// TYPES -------------------------------------------------------------------

typedef enum
{
	STMT_SCRIPT,
	STMT_IF,
	STMT_ELSE,
	STMT_DO,
	STMT_WHILEUNTIL,
	STMT_SWITCH,
	STMT_FOR
} statement_t;

typedef struct
{
	int level;
	int addressPtr;
} breakInfo_t;

typedef struct
{
	int level;
	int addressPtr;
} continueInfo_t;

typedef struct
{
	int level;
	int value;
	boolean isDefault;
	int address;
} caseInfo_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void Outside(void);
static void OuterScript(void);
static void OuterMapVar(void);
static void OuterWorldVar(void);
static void OuterSpecialDef(void);
static void OuterDefine(void);
static void OuterInclude(void);
static boolean ProcessStatement(statement_t owner);
static void LeadingCompoundStatement(statement_t owner);
static void LeadingVarDeclare(void);
static void LeadingLineSpecial(void);
static void LeadingIdentifier(void);
static void LeadingPrint(void);
static void LeadingVarAssign(symbolNode_t *sym);
static pcd_t GetAssignPCD(tokenType_t token, symbolType_t symbol);
static void LeadingInternFunc(symbolNode_t *sym);
static void LeadingSuspend(void);
static void LeadingTerminate(void);
static void LeadingRestart(void);
static void LeadingIf(void);
static void LeadingFor(void);
static void LeadingWhileUntil(void);
static void LeadingDo(void);
static void LeadingSwitch(void);
static void LeadingCase(void);
static void LeadingDefault(void);
static void LeadingBreak(void);
static void LeadingContinue(void);
static void PushCase(int value, boolean isDefault);
static caseInfo_t *GetCaseInfo(void);
static boolean DefaultInCurrent(void);
static void PushBreak(void);
static void WriteBreaks(void);
static boolean BreakAncestor(void);
static void PushContinue(void);
static void WriteContinues(int address);
static boolean ContinueAncestor(void);
static void ProcessInternFunc(symbolNode_t *sym);
static void EvalExpression(void);
static void ExprLevA(void);
static void ExprLevB(void);
static void ExprLevC(void);
static void ExprLevD(void);
static void ExprLevE(void);
static void ExprLevF(void);
static void ExprLevG(void);
static void ExprLevH(void);
static void ExprLevI(void);
static void ExprLevJ(void);
static void ExprFactor(void);
static void ConstExprFactor(void);
static void SendExprCommand(pcd_t pcd);
static void PushExStk(int value);
static int PopExStk(void);
static pcd_t TokenToPCD(tokenType_t token);
static pcd_t GetPushVarPCD(symbolType_t symType);
static pcd_t GetIncDecPCD(tokenType_t token, symbolType_t symbol);
static int EvalConstExpression(void);
static symbolNode_t *DemandSymbol(char *name);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int pa_ScriptCount;
int pa_OpenScriptCount;
int pa_MapVarCount;
int pa_WorldVarCount;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int ScriptVarCount;
static statement_t StatementHistory[MAX_STATEMENT_DEPTH];
static int StatementIndex;
static breakInfo_t BreakInfo[MAX_BREAK];
static int BreakIndex;
static continueInfo_t ContinueInfo[MAX_CONTINUE];
static int ContinueIndex;
static caseInfo_t CaseInfo[MAX_CASE];
static int CaseIndex;
static int StatementLevel;
static int ExprStack[EXPR_STACK_DEPTH];
static int ExprStackIndex;
static boolean ConstantExpression;

static int AdjustStmtLevel[] =
{
	0,		// STMT_SCRIPT
	0,		// STMT_IF
	0,		// STMT_ELSE
	1,		// STMT_DO
	1,		// STMT_WHILEUNTIL
	1,		// STMT_SWITCH
	1		// STMT_FOR
};

static boolean IsBreakRoot[] =
{
	NO,		// STMT_SCRIPT
	NO,		// STMT_IF
	NO,		// STMT_ELSE
	YES,	// STMT_DO
	YES,	// STMT_WHILEUNTIL
	YES,	// STMT_SWITCH
	YES		// STMT_FOR
};

static boolean IsContinueRoot[] =
{
	NO,		// STMT_SCRIPT
	NO,		// STMT_IF
	NO,		// STMT_ELSE
	YES,	// STMT_DO
	YES,	// STMT_WHILEUNTIL
	NO,		// STMT_SWITCH
	YES		// STMT_FOR
};

static tokenType_t LevFOps[] =
{
	TK_EQ,
	TK_NE,
	TK_NONE
};

static tokenType_t LevGOps[] =
{
	TK_LT,
	TK_LE,
	TK_GT,
	TK_GE,
	TK_NONE
};

static tokenType_t LevHOps[] =
{
	TK_LSHIFT,
	TK_RSHIFT,
	TK_NONE
};

static tokenType_t LevIOps[] =
{
	TK_PLUS,
	TK_MINUS,
	TK_NONE
};

static tokenType_t LevJOps[] =
{
	TK_ASTERISK,
	TK_SLASH,
	TK_PERCENT,
	TK_NONE
};

static tokenType_t AssignOps[] =
{
	TK_ASSIGN,
	TK_ADDASSIGN,
	TK_SUBASSIGN,
	TK_MULASSIGN,
	TK_DIVASSIGN,
	TK_MODASSIGN,
	TK_NONE
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// PA_Parse
//
//==========================================================================

void PA_Parse(void)
{
	pa_ScriptCount = 0;
	pa_OpenScriptCount = 0;
	pa_MapVarCount = 0;
	pa_WorldVarCount = 0;
	TK_NextToken();
	Outside();
}

//==========================================================================
//
// Outside
//
//==========================================================================

static void Outside(void)
{
	boolean done;

	done = NO;
	while(done == NO)
	{
		switch(tk_Token)
		{
			case TK_EOF:
				done = YES;
				break;
			case TK_SCRIPT:
				OuterScript();
				break;
			case TK_INT:
			case TK_STR:
				OuterMapVar();
				break;
			case TK_WORLD:
				OuterWorldVar();
				break;
			case TK_SPECIAL:
				OuterSpecialDef();
				break;
			case TK_NUMBERSIGN:
				TK_NextToken();
				switch(tk_Token)
				{
					case TK_DEFINE:
						OuterDefine();
						break;
					case TK_INCLUDE:
						OuterInclude();
						break;
					default:
						ERR_Exit(ERR_INVALID_DIRECTIVE, YES, NULL);
						break;
				}
				break;
			default:
				ERR_Exit(ERR_INVALID_DECLARATOR, YES, NULL);
				break;
	   	}
	}
}

//==========================================================================
//
// OuterScript
//
//==========================================================================

static void OuterScript(void)
{
	int scriptNumber;
	symbolNode_t *sym;

	MS_Message(MSG_DEBUG, "---- OuterScript ----\n");
	BreakIndex = 0;
	CaseIndex = 0;
	StatementLevel = 0;
	ScriptVarCount = 0;
	SY_FreeLocals();
	TK_NextToken();
	scriptNumber = EvalConstExpression();
	MS_Message(MSG_DEBUG, "Script number: %d\n", scriptNumber);
	if(tk_Token == TK_LPAREN)
	{
		if(TK_NextToken() == TK_VOID)
		{
			TK_NextTokenMustBe(TK_RPAREN, ERR_MISSING_RPAREN);
		}
		else
		{
			TK_Undo();
			do
			{
				TK_NextTokenMustBe(TK_INT, ERR_BAD_VAR_TYPE);
				TK_NextTokenMustBe(TK_IDENTIFIER, ERR_INVALID_IDENTIFIER);
				if(SY_FindLocal(tk_String) != NULL)
				{ // Redefined
					ERR_Exit(ERR_REDEFINED_IDENTIFIER, YES,
						"Identifier: %s", tk_String);
				}
				sym = SY_InsertLocal(tk_String, SY_SCRIPTVAR);
				sym->info.var.index = ScriptVarCount;
				ScriptVarCount++;
				TK_NextToken();
			} while(tk_Token == TK_COMMA);
			TK_TokenMustBe(TK_RPAREN, ERR_MISSING_RPAREN);
			if(ScriptVarCount > 3)
			{
				ERR_Exit(ERR_TOO_MANY_SCRIPT_ARGS, YES, NULL);
			}
		}
		MS_Message(MSG_DEBUG, "Script type: CLOSED (%d %s)\n",
			ScriptVarCount, ScriptVarCount == 1 ? "arg" : "args");
	}
	else if(tk_Token == TK_OPEN)
	{
		MS_Message(MSG_DEBUG, "Script type: OPEN\n");
		scriptNumber += OPEN_SCRIPTS_BASE;
		pa_OpenScriptCount++;
	}
	else
	{
		ERR_Exit(ERR_BAD_SCRIPT_DECL, YES, NULL);
	}
	PC_AddScript(scriptNumber, ScriptVarCount);
	TK_NextToken();
	if(ProcessStatement(STMT_SCRIPT) == NO)
	{
		ERR_Exit(ERR_INVALID_STATEMENT, YES, NULL);
	}
	PC_AppendCmd(PCD_TERMINATE);
	pa_ScriptCount++;
}

//==========================================================================
//
// OuterMapVar
//
//==========================================================================

static void OuterMapVar(void)
{
	symbolNode_t *sym;

	MS_Message(MSG_DEBUG, "---- OuterMapVar ----\n");
	do
	{
		if(pa_MapVarCount == MAX_MAP_VARIABLES)
		{
			ERR_Exit(ERR_TOO_MANY_MAP_VARS, YES, NULL);
		}
		TK_NextTokenMustBe(TK_IDENTIFIER, ERR_INVALID_IDENTIFIER);
		if(SY_FindGlobal(tk_String) != NULL)
		{ // Redefined
			ERR_Exit(ERR_REDEFINED_IDENTIFIER, YES,
				"Identifier: %s", tk_String);
		}
		sym = SY_InsertGlobal(tk_String, SY_MAPVAR);
		sym->info.var.index = pa_MapVarCount;
		pa_MapVarCount++;
		TK_NextToken();
	} while(tk_Token == TK_COMMA);
	TK_TokenMustBe(TK_SEMICOLON, ERR_MISSING_SEMICOLON);
	TK_NextToken();
}

//==========================================================================
//
// OuterWorldVar
//
//==========================================================================

static void OuterWorldVar(void)
{
	int index;
	symbolNode_t *sym;

	MS_Message(MSG_DEBUG, "---- OuterWorldVar ----\n");
	if(TK_NextToken() != TK_INT)
	{
		TK_TokenMustBe(TK_STR, ERR_BAD_VAR_TYPE);
	}
	do
	{
		TK_NextTokenMustBe(TK_NUMBER, ERR_MISSING_WVAR_INDEX);
		if(tk_Number >= MAX_WORLD_VARIABLES)
		{
			ERR_Exit(ERR_BAD_WVAR_INDEX, YES, NULL);
		}
		index = tk_Number;
		TK_NextTokenMustBe(TK_COLON, ERR_MISSING_WVAR_COLON);
		TK_NextTokenMustBe(TK_IDENTIFIER, ERR_INVALID_IDENTIFIER);
		if(SY_FindGlobal(tk_String) != NULL)
		{ // Redefined
			ERR_Exit(ERR_REDEFINED_IDENTIFIER, YES,
				"Identifier: %s", tk_String);
		}
		sym = SY_InsertGlobal(tk_String, SY_WORLDVAR);
		sym->info.var.index = index;
		TK_NextToken();
		pa_WorldVarCount++;
	} while(tk_Token == TK_COMMA);
	TK_TokenMustBe(TK_SEMICOLON, ERR_MISSING_SEMICOLON);
	TK_NextToken();
}

//==========================================================================
//
// OuterSpecialDef
//
//==========================================================================

static void OuterSpecialDef(void)
{
	int special;
	symbolNode_t *sym;

	MS_Message(MSG_DEBUG, "---- OuterSpecialDef ----\n");
	do
	{
		TK_NextTokenMustBe(TK_NUMBER, ERR_MISSING_SPEC_VAL);
		special = tk_Number;
		TK_NextTokenMustBe(TK_COLON, ERR_MISSING_SPEC_COLON);
		TK_NextTokenMustBe(TK_IDENTIFIER, ERR_INVALID_IDENTIFIER);
		if(SY_FindGlobal(tk_String) != NULL)
		{ // Redefined
			ERR_Exit(ERR_REDEFINED_IDENTIFIER, YES,
				"Identifier: %s", tk_String);
		}
		sym = SY_InsertGlobal(tk_String, SY_SPECIAL);
		TK_NextTokenMustBe(TK_LPAREN, ERR_MISSING_LPAREN);
		TK_NextTokenMustBe(TK_NUMBER, ERR_MISSING_SPEC_ARGC);
		sym->info.special.value = special;
		sym->info.special.argCount = tk_Number;
		TK_NextTokenMustBe(TK_RPAREN, ERR_MISSING_RPAREN);
		TK_NextToken();
	} while(tk_Token == TK_COMMA);
	TK_TokenMustBe(TK_SEMICOLON, ERR_MISSING_SEMICOLON);
	TK_NextToken();
}

//==========================================================================
//
// OuterDefine
//
//==========================================================================

static void OuterDefine(void)
{
	int value;
	symbolNode_t *sym;

	MS_Message(MSG_DEBUG, "---- OuterDefine ----\n");
	TK_NextTokenMustBe(TK_IDENTIFIER, ERR_INVALID_IDENTIFIER);
	if(SY_FindGlobal(tk_String) != NULL)
	{ // Redefined
		ERR_Exit(ERR_REDEFINED_IDENTIFIER, YES,
			"Identifier: %s", tk_String);
	}
	sym = SY_InsertGlobal(tk_String, SY_CONSTANT);
	TK_NextToken();
	value = EvalConstExpression();
	MS_Message(MSG_DEBUG, "Constant value: %d\n", value);
	sym->info.constant.value = value;
}

//==========================================================================
//
// OuterInclude
//
//==========================================================================

static void OuterInclude(void)
{
	MS_Message(MSG_DEBUG, "---- OuterInclude ----\n");
	TK_NextTokenMustBe(TK_STRING, ERR_STRING_LIT_NOT_FOUND);
	TK_Include(tk_String);
	TK_NextToken();
}

//==========================================================================
//
// ProcessStatement
//
//==========================================================================

static boolean ProcessStatement(statement_t owner)
{
	if(StatementIndex == MAX_STATEMENT_DEPTH)
	{
		ERR_Exit(ERR_STATEMENT_OVERFLOW, YES, NULL);
	}
	StatementHistory[StatementIndex++] = owner;
	switch(tk_Token)
	{
		case TK_INT:
		case TK_STR:
			LeadingVarDeclare();
			break;
		case TK_LINESPECIAL:
			LeadingLineSpecial();
			break;
		case TK_RESTART:
			LeadingRestart();
			break;
		case TK_SUSPEND:
			LeadingSuspend();
			break;
		case TK_TERMINATE:
			LeadingTerminate();
			break;
		case TK_IDENTIFIER:
			LeadingIdentifier();
			break;
		case TK_PRINT:
		case TK_PRINTBOLD:
			LeadingPrint();
			break;
		case TK_IF:
			LeadingIf();
			break;
		case TK_FOR:
			LeadingFor();
			break;
		case TK_WHILE:
		case TK_UNTIL:
			LeadingWhileUntil();
			break;
		case TK_DO:
			LeadingDo();
			break;
		case TK_SWITCH:
			LeadingSwitch();
			break;
		case TK_CASE:
			if(owner != STMT_SWITCH)
			{
				ERR_Exit(ERR_CASE_NOT_IN_SWITCH, YES, NULL);
			}
			LeadingCase();
			break;
		case TK_DEFAULT:
			if(owner != STMT_SWITCH)
			{
				ERR_Exit(ERR_DEFAULT_NOT_IN_SWITCH, YES, NULL);
			}
			if(DefaultInCurrent() == YES)
			{
				ERR_Exit(ERR_MULTIPLE_DEFAULT, YES, NULL);
			}
			LeadingDefault();
			break;
		case TK_BREAK:
			if(BreakAncestor() == NO)
			{
				ERR_Exit(ERR_MISPLACED_BREAK, YES, NULL);
			}
			LeadingBreak();
			break;
		case TK_CONTINUE:
			if(ContinueAncestor() == NO)
			{
				ERR_Exit(ERR_MISPLACED_CONTINUE, YES, NULL);
			}
			LeadingContinue();
			break;
		case TK_LBRACE:
			LeadingCompoundStatement(owner);
			break;
		case TK_SEMICOLON:
			TK_NextToken();
			break;
		default:
			StatementIndex--;
			return NO;
			break;
	}
	StatementIndex--;
	return YES;
}

//==========================================================================
//
// LeadingCompoundStatement
//
//==========================================================================

static void LeadingCompoundStatement(statement_t owner)
{
	StatementLevel += AdjustStmtLevel[owner];
	TK_NextToken(); // Eat the TK_LBRACE
	do ; while(ProcessStatement(owner) == YES);
	TK_TokenMustBe(TK_RBRACE, ERR_INVALID_STATEMENT);
	TK_NextToken();
	StatementLevel -= AdjustStmtLevel[owner];
}

//==========================================================================
//
// LeadingVarDeclare
//
//==========================================================================

static void LeadingVarDeclare(void)
{
	symbolNode_t *sym;

	MS_Message(MSG_DEBUG, "---- LeadingVarDeclare ----\n");
	do
	{
		if(ScriptVarCount == MAX_SCRIPT_VARIABLES)
		{
			ERR_Exit(ERR_TOO_MANY_SCRIPT_VARS, YES, NULL);
		}
		TK_NextTokenMustBe(TK_IDENTIFIER, ERR_INVALID_IDENTIFIER);
		if(SY_FindLocal(tk_String) != NULL)
		{ // Redefined
			ERR_Exit(ERR_REDEFINED_IDENTIFIER, YES,
				"Identifier: %s", tk_String);
		}
		sym = SY_InsertLocal(tk_String, SY_SCRIPTVAR);
		sym->info.var.index = ScriptVarCount;
		ScriptVarCount++;
		TK_NextToken();
	} while(tk_Token == TK_COMMA);
	TK_TokenMustBe(TK_SEMICOLON, ERR_MISSING_SEMICOLON);
	TK_NextToken();
}

//==========================================================================
//
// LeadingLineSpecial
//
//==========================================================================

static void LeadingLineSpecial(void)
{
	int i;
	int argCount;
	int specialValue;
	boolean direct;

	MS_Message(MSG_DEBUG, "---- LeadingLineSpecial ----\n");
	argCount = tk_SpecialArgCount;
	specialValue = tk_SpecialValue;
	TK_NextTokenMustBe(TK_LPAREN, ERR_MISSING_LPAREN);
	if(TK_NextToken() == TK_CONST)
	{
		TK_NextTokenMustBe(TK_COLON, ERR_MISSING_COLON);
		PC_AppendCmd(PCD_LSPEC1DIRECT+(argCount-1));
		PC_AppendLong(specialValue);
		direct = YES;
	}
	else
	{
		TK_Undo();
		direct = NO;
	}
	i = 0;
	do
	{
		if(i == argCount)
		{
			ERR_Exit(ERR_BAD_LSPEC_ARG_COUNT, YES, NULL);
		}
		TK_NextToken();
		if(direct == YES)
		{
			PC_AppendLong(EvalConstExpression());
		}
		else
		{
			EvalExpression();
		}
		i++;
	} while(tk_Token == TK_COMMA);
	if(i != argCount)
	{
		ERR_Exit(ERR_BAD_LSPEC_ARG_COUNT, YES, NULL);
	}
	TK_TokenMustBe(TK_RPAREN, ERR_MISSING_RPAREN);
	TK_NextTokenMustBe(TK_SEMICOLON, ERR_MISSING_SEMICOLON);
	if(direct == NO)
	{
		PC_AppendCmd(PCD_LSPEC1+(argCount-1));
		PC_AppendLong(specialValue);
	}
	TK_NextToken();
}

//==========================================================================
//
// LeadingIdentifier
//
//==========================================================================

static void LeadingIdentifier(void)
{
	symbolNode_t *sym;

	sym = DemandSymbol(tk_String);
	switch(sym->type)
	{
		case SY_SCRIPTVAR:
		case SY_MAPVAR:
		case SY_WORLDVAR:
			LeadingVarAssign(sym);
			break;
		case SY_INTERNFUNC:
			LeadingInternFunc(sym);
			break;
		default:
			break;
	}
}

//==========================================================================
//
// LeadingInternFunc
//
//==========================================================================

static void LeadingInternFunc(symbolNode_t *sym)
{
	ProcessInternFunc(sym);
	if(sym->info.internFunc.hasReturnValue == YES)
	{
		PC_AppendCmd(PCD_DROP);
	}
	TK_TokenMustBe(TK_SEMICOLON, ERR_MISSING_SEMICOLON);
	TK_NextToken();
}

//==========================================================================
//
// ProcessInternFunc
//
//==========================================================================

static void ProcessInternFunc(symbolNode_t *sym)
{
	int i;
	int argCount;
	boolean direct;

	MS_Message(MSG_DEBUG, "---- ProcessInternFunc ----\n");
	argCount = sym->info.internFunc.argCount;
	TK_NextTokenMustBe(TK_LPAREN, ERR_MISSING_LPAREN);
	if(TK_NextToken() == TK_CONST)
	{
		TK_NextTokenMustBe(TK_COLON, ERR_MISSING_COLON);
		if(sym->info.internFunc.directCommand == PCD_NOP)
		{
			ERR_Exit(ERR_NO_DIRECT_VER, YES, NULL);
		}
		PC_AppendCmd(sym->info.internFunc.directCommand);
		direct = YES;
		TK_NextToken();
	}
	else
	{
		direct = NO;
	}
	i = 0;
	if(argCount > 0)
	{
		TK_Undo(); // Adjust for first expression
		do
		{
			if(i == argCount)
			{
				ERR_Exit(ERR_BAD_ARG_COUNT, YES, NULL);
			}
			TK_NextToken();
			if(direct == YES)
			{
				PC_AppendLong(EvalConstExpression());
			}
			else
			{
				EvalExpression();
			}
			i++;
		} while(tk_Token == TK_COMMA);
	}
	if(i != argCount)
	{
		ERR_Exit(ERR_BAD_ARG_COUNT, YES, NULL);
	}
	TK_TokenMustBe(TK_RPAREN, ERR_MISSING_RPAREN);
	if(direct == NO)
	{
		PC_AppendCmd(sym->info.internFunc.stackCommand);
	}
	TK_NextToken();
}

//==========================================================================
//
// LeadingPrint
//
//==========================================================================

static void LeadingPrint(void)
{
	pcd_t printCmd = 0;
	tokenType_t stmtToken;

	MS_Message(MSG_DEBUG, "---- LeadingPrint ----\n");
	stmtToken = tk_Token; // Will be TK_PRINT or TK_PRINTBOLD
	PC_AppendCmd(PCD_BEGINPRINT);
	TK_NextTokenMustBe(TK_LPAREN, ERR_MISSING_LPAREN);
	do
	{
		switch(TK_NextCharacter())
		{
			case 's': // string
				printCmd = PCD_PRINTSTRING;
				break;
			case 'i': // integer
			case 'd': // decimal
				printCmd = PCD_PRINTNUMBER;
				break;
			case 'c': // character
				printCmd = PCD_PRINTCHARACTER;
				break;
			default:
				ERR_Exit(ERR_UNKNOWN_PRTYPE, YES, NULL);
				break;
		}
		TK_NextTokenMustBe(TK_COLON, ERR_MISSING_COLON);
		TK_NextToken();
		EvalExpression();
		PC_AppendCmd(printCmd);
	} while(tk_Token == TK_COMMA);
	TK_TokenMustBe(TK_RPAREN, ERR_MISSING_RPAREN);
	if(stmtToken == TK_PRINT)
	{
		PC_AppendCmd(PCD_ENDPRINT);
	}
	else
	{
		PC_AppendCmd(PCD_ENDPRINTBOLD);
	}
	TK_NextToken();
}

//==========================================================================
//
// LeadingIf
//
//==========================================================================

static void LeadingIf(void)
{
	int jumpAddrPtr1;
	int jumpAddrPtr2;

	MS_Message(MSG_DEBUG, "---- LeadingIf ----\n");
	TK_NextTokenMustBe(TK_LPAREN, ERR_MISSING_LPAREN);
	TK_NextToken();
	EvalExpression();
	TK_TokenMustBe(TK_RPAREN, ERR_MISSING_RPAREN);
	PC_AppendCmd(PCD_IFNOTGOTO);
	jumpAddrPtr1 = pc_Address;
	PC_SkipLong();
	TK_NextToken();
	if(ProcessStatement(STMT_IF) == NO)
	{
		ERR_Exit(ERR_INVALID_STATEMENT, YES, NULL);
	}
	if(tk_Token == TK_ELSE)
	{
		PC_AppendCmd(PCD_GOTO);
		jumpAddrPtr2 = pc_Address;
		PC_SkipLong();
		PC_WriteLong(pc_Address, jumpAddrPtr1);
		TK_NextToken();
		if(ProcessStatement(STMT_ELSE) == NO)
		{
			ERR_Exit(ERR_INVALID_STATEMENT, YES, NULL);
		}
		PC_WriteLong(pc_Address, jumpAddrPtr2);
	}
	else
	{
		PC_WriteLong(pc_Address, jumpAddrPtr1);
	}
}

//==========================================================================
//
// LeadingFor
//
//==========================================================================

static void LeadingFor(void)
{
}

//==========================================================================
//
// LeadingWhileUntil
//
//==========================================================================

static void LeadingWhileUntil(void)
{
	tokenType_t stmtToken;
	int topAddr;
	int outAddrPtr;

	MS_Message(MSG_DEBUG, "---- LeadingWhileUntil ----\n");
	stmtToken = tk_Token;
	topAddr = pc_Address;
	TK_NextTokenMustBe(TK_LPAREN, ERR_MISSING_LPAREN);
	TK_NextToken();
	EvalExpression();
	TK_TokenMustBe(TK_RPAREN, ERR_MISSING_RPAREN);
	PC_AppendCmd(stmtToken == TK_WHILE ? PCD_IFNOTGOTO : PCD_IFGOTO);
	outAddrPtr = pc_Address;
	PC_SkipLong();
	TK_NextToken();
	if(ProcessStatement(STMT_WHILEUNTIL) == NO)
	{
		ERR_Exit(ERR_INVALID_STATEMENT, YES, NULL);
	}
	PC_AppendCmd(PCD_GOTO);
	PC_AppendLong(topAddr);

	PC_WriteLong(pc_Address, outAddrPtr);

	WriteContinues(topAddr);
	WriteBreaks();
}

//==========================================================================
//
// LeadingDo
//
//==========================================================================

static void LeadingDo(void)
{
	int topAddr;
	int exprAddr;
	tokenType_t stmtToken;

	MS_Message(MSG_DEBUG, "---- LeadingDo ----\n");
	topAddr = pc_Address;
	TK_NextToken();
	if(ProcessStatement(STMT_DO) == NO)
	{
		ERR_Exit(ERR_INVALID_STATEMENT, YES, NULL);
	}
	if(tk_Token != TK_WHILE && tk_Token != TK_UNTIL)
	{
		ERR_Exit(ERR_BAD_DO_STATEMENT, YES, NULL);
	}
	stmtToken = tk_Token;
	TK_NextTokenMustBe(TK_LPAREN, ERR_MISSING_LPAREN);
	exprAddr = pc_Address;
	TK_NextToken();
	EvalExpression();
	TK_TokenMustBe(TK_RPAREN, ERR_MISSING_RPAREN);
	TK_NextTokenMustBe(TK_SEMICOLON, ERR_MISSING_SEMICOLON);
	PC_AppendCmd(stmtToken == TK_WHILE ? PCD_IFGOTO : PCD_IFNOTGOTO);
	PC_AppendLong(topAddr);
	WriteContinues(exprAddr);
	WriteBreaks();
	TK_NextToken();
}

//==========================================================================
//
// LeadingSwitch
//
//==========================================================================

static void LeadingSwitch(void)
{
	int switcherAddrPtr;
	int outAddrPtr;
	caseInfo_t *cInfo;
	int defaultAddress;

	MS_Message(MSG_DEBUG, "---- LeadingSwitch ----\n");

	TK_NextTokenMustBe(TK_LPAREN, ERR_MISSING_LPAREN);
	TK_NextToken();
	EvalExpression();
	TK_TokenMustBe(TK_RPAREN, ERR_MISSING_RPAREN);

	PC_AppendCmd(PCD_GOTO);
	switcherAddrPtr = pc_Address;
	PC_SkipLong();

	TK_NextToken();
	if(ProcessStatement(STMT_SWITCH) == NO)
	{
		ERR_Exit(ERR_INVALID_STATEMENT, YES, NULL);
	}

	PC_AppendCmd(PCD_GOTO);
	outAddrPtr = pc_Address;
	PC_SkipLong();

	PC_WriteLong(pc_Address, switcherAddrPtr);
	defaultAddress = 0;
	while((cInfo = GetCaseInfo()) != NULL)
	{
		if(cInfo->isDefault == YES)
		{
			defaultAddress = cInfo->address;
			continue;
		}
		PC_AppendCmd(PCD_CASEGOTO);
		PC_AppendLong(cInfo->value);
		PC_AppendLong(cInfo->address);
	}
	PC_AppendCmd(PCD_DROP);

	if(defaultAddress != 0)
	{
		PC_AppendCmd(PCD_GOTO);
		PC_AppendLong(defaultAddress);
	}

	PC_WriteLong(pc_Address, outAddrPtr);

	WriteBreaks();
}

//==========================================================================
//
// LeadingCase
//
//==========================================================================

static void LeadingCase(void)
{
	MS_Message(MSG_DEBUG, "---- LeadingCase ----\n");
	TK_NextToken();
	PushCase(EvalConstExpression(), NO);
	TK_TokenMustBe(TK_COLON, ERR_MISSING_COLON);
	TK_NextToken();
}

//==========================================================================
//
// LeadingDefault
//
//==========================================================================

static void LeadingDefault(void)
{
	MS_Message(MSG_DEBUG, "---- LeadingDefault ----\n");
	TK_NextTokenMustBe(TK_COLON, ERR_MISSING_COLON);
	PushCase(0, YES);
	TK_NextToken();
}

//==========================================================================
//
// PushCase
//
//==========================================================================

static void PushCase(int value, boolean isDefault)
{
	if(CaseIndex == MAX_CASE)
	{
		ERR_Exit(ERR_CASE_OVERFLOW, YES, NULL);
	}
	CaseInfo[CaseIndex].level = StatementLevel;
	CaseInfo[CaseIndex].value = value;
	CaseInfo[CaseIndex].isDefault = isDefault;
	CaseInfo[CaseIndex].address = pc_Address;
	CaseIndex++;
}

//==========================================================================
//
// GetCaseInfo
//
//==========================================================================

static caseInfo_t *GetCaseInfo(void)
{
	if(CaseIndex == 0)
	{
		return NULL;
	}
	if(CaseInfo[CaseIndex-1].level > StatementLevel)
	{
		return &CaseInfo[--CaseIndex];
	}
	return NULL;
}

//==========================================================================
//
// DefaultInCurrent
//
//==========================================================================

static boolean DefaultInCurrent(void)
{
	int i;

	for(i = 0; i < CaseIndex; i++)
	{
		if(CaseInfo[i].isDefault == YES
			&& CaseInfo[i].level == StatementLevel)
		{
			return YES;
		}
	}
	return NO;
}

//==========================================================================
//
// LeadingBreak
//
//==========================================================================

static void LeadingBreak(void)
{
	MS_Message(MSG_DEBUG, "---- LeadingBreak ----\n");
	TK_NextTokenMustBe(TK_SEMICOLON, ERR_MISSING_SEMICOLON);
	PC_AppendCmd(PCD_GOTO);
	PushBreak();
	PC_SkipLong();
	TK_NextToken();
}

//==========================================================================
//
// PushBreak
//
//==========================================================================

static void PushBreak(void)
{
	if(BreakIndex == MAX_CASE)
	{
		ERR_Exit(ERR_BREAK_OVERFLOW, YES, NULL);
	}
	BreakInfo[BreakIndex].level = StatementLevel;
	BreakInfo[BreakIndex].addressPtr = pc_Address;
	BreakIndex++;
}

//==========================================================================
//
// WriteBreaks
//
//==========================================================================

static void WriteBreaks(void)
{
	if(BreakIndex == 0)
	{
		return;
	}
	while(BreakIndex && BreakInfo[BreakIndex-1].level > StatementLevel)
	{
		PC_WriteLong(pc_Address, BreakInfo[--BreakIndex].addressPtr);
	}
}

//==========================================================================
//
// BreakAncestor
//
// Returns YES if the current statement history contains a break root
// statement.
//
//==========================================================================

static boolean BreakAncestor(void)
{
	int i;

	for(i = 0; i < StatementIndex; i++)
	{
		if(IsBreakRoot[StatementHistory[i]])
		{
			return YES;
		}
	}
	return NO;
}

//==========================================================================
//
// LeadingContinue
//
//==========================================================================

static void LeadingContinue(void)
{
	MS_Message(MSG_DEBUG, "---- LeadingContinue ----\n");
	TK_NextTokenMustBe(TK_SEMICOLON, ERR_MISSING_SEMICOLON);
	PC_AppendCmd(PCD_GOTO);
	PushContinue();
	PC_SkipLong();
	TK_NextToken();
}

//==========================================================================
//
// PushContinue
//
//==========================================================================

static void PushContinue(void)
{
	if(ContinueIndex == MAX_CONTINUE)
	{
		ERR_Exit(ERR_CONTINUE_OVERFLOW, YES, NULL);
	}
	ContinueInfo[ContinueIndex].level = StatementLevel;
	ContinueInfo[ContinueIndex].addressPtr = pc_Address;
	ContinueIndex++;
}

//==========================================================================
//
// WriteContinues
//
//==========================================================================

static void WriteContinues(int address)
{
	if(ContinueIndex == 0)
	{
		return;
	}
	while(ContinueInfo[ContinueIndex-1].level > StatementLevel)
	{
		PC_WriteLong(address, ContinueInfo[--ContinueIndex].addressPtr);
	}
}

//==========================================================================
//
// ContinueAncestor
//
//==========================================================================

static boolean ContinueAncestor(void)
{
	int i;

	for(i = 0; i < StatementIndex; i++)
	{
		if(IsContinueRoot[StatementHistory[i]])
		{
			return YES;
		}
	}
	return NO;
}

//==========================================================================
//
// LeadingVarAssign
//
//==========================================================================

static void LeadingVarAssign(symbolNode_t *sym)
{
	boolean done;
	tokenType_t assignToken;

	MS_Message(MSG_DEBUG, "---- LeadingVarAssign ----\n");
	done = NO;
	do
	{
		TK_NextToken(); // Fetch assignment operator
		if(tk_Token == TK_INC || tk_Token == TK_DEC)
		{ // Postfix increment or decrement
			PC_AppendCmd(GetIncDecPCD(tk_Token, sym->type));
			PC_AppendLong(sym->info.var.index);
			TK_NextToken();
		}
		else
		{ // Normal operator
			if(TK_Member(AssignOps) == NO)
			{
				ERR_Exit(ERR_MISSING_ASSIGN_OP, YES, NULL);
			}
			assignToken = tk_Token;
			TK_NextToken();
			EvalExpression();
			PC_AppendCmd(GetAssignPCD(assignToken, sym->type));
			PC_AppendLong(sym->info.var.index);
		}
		if(tk_Token == TK_COMMA)
		{
			TK_NextTokenMustBe(TK_IDENTIFIER, ERR_BAD_ASSIGNMENT);
			sym = DemandSymbol(tk_String);
			if(sym->type != SY_SCRIPTVAR && sym->type != SY_MAPVAR
				&& sym->type != SY_WORLDVAR)
			{
				ERR_Exit(ERR_BAD_ASSIGNMENT, YES, NULL);
			}
		}
		else
		{
			TK_TokenMustBe(TK_SEMICOLON, ERR_MISSING_SEMICOLON);
			TK_NextToken();
			done = YES;
		}
	} while(done == NO);
}

//==========================================================================
//
// GetAssignPCD
//
//==========================================================================

static pcd_t GetAssignPCD(tokenType_t token, symbolType_t symbol)
{
	int i;
	static struct
	{
		tokenType_t token;
		symbolType_t symbol;
		pcd_t pcd;
	}  assignmentLookup[] =
	{
		TK_ASSIGN, SY_SCRIPTVAR, PCD_ASSIGNSCRIPTVAR,
		TK_ASSIGN, SY_MAPVAR, PCD_ASSIGNMAPVAR,
		TK_ASSIGN, SY_WORLDVAR, PCD_ASSIGNWORLDVAR,
		TK_ADDASSIGN, SY_SCRIPTVAR, PCD_ADDSCRIPTVAR,
		TK_ADDASSIGN, SY_MAPVAR, PCD_ADDMAPVAR,
		TK_ADDASSIGN, SY_WORLDVAR, PCD_ADDWORLDVAR,
		TK_SUBASSIGN, SY_SCRIPTVAR, PCD_SUBSCRIPTVAR,
		TK_SUBASSIGN, SY_MAPVAR, PCD_SUBMAPVAR,
		TK_SUBASSIGN, SY_WORLDVAR, PCD_SUBWORLDVAR,
		TK_MULASSIGN, SY_SCRIPTVAR, PCD_MULSCRIPTVAR,
		TK_MULASSIGN, SY_MAPVAR, PCD_MULMAPVAR,
		TK_MULASSIGN, SY_WORLDVAR, PCD_MULWORLDVAR,
		TK_DIVASSIGN, SY_SCRIPTVAR, PCD_DIVSCRIPTVAR,
		TK_DIVASSIGN, SY_MAPVAR, PCD_DIVMAPVAR,
		TK_DIVASSIGN, SY_WORLDVAR, PCD_DIVWORLDVAR,
		TK_MODASSIGN, SY_SCRIPTVAR, PCD_MODSCRIPTVAR,
		TK_MODASSIGN, SY_MAPVAR, PCD_MODMAPVAR,
		TK_MODASSIGN, SY_WORLDVAR, PCD_MODWORLDVAR,
		TK_NONE
	};

	for(i = 0; assignmentLookup[i].token != TK_NONE; i++)
	{
		if(assignmentLookup[i].token == token
			&& assignmentLookup[i].symbol == symbol)
		{
			return assignmentLookup[i].pcd;
		}
	}
	return PCD_NOP;
}

//==========================================================================
//
// LeadingSuspend
//
//==========================================================================

static void LeadingSuspend(void)
{
	MS_Message(MSG_DEBUG, "---- LeadingSuspend ----\n");
	TK_NextTokenMustBe(TK_SEMICOLON, ERR_MISSING_SEMICOLON);
	PC_AppendCmd(PCD_SUSPEND);
	TK_NextToken();
}

//==========================================================================
//
// LeadingTerminate
//
//==========================================================================

static void LeadingTerminate(void)
{
	MS_Message(MSG_DEBUG, "---- LeadingTerminate ----\n");
	TK_NextTokenMustBe(TK_SEMICOLON, ERR_MISSING_SEMICOLON);
	PC_AppendCmd(PCD_TERMINATE);
	TK_NextToken();
}

//==========================================================================
//
// LeadingRestart
//
//==========================================================================

static void LeadingRestart(void)
{
	MS_Message(MSG_DEBUG, "---- LeadingRestart ----\n");
	TK_NextTokenMustBe(TK_SEMICOLON, ERR_MISSING_SEMICOLON);
	PC_AppendCmd(PCD_RESTART);
	TK_NextToken();
}

//==========================================================================
//
// EvalConstExpression
//
//==========================================================================

static int EvalConstExpression(void)
{
	ExprStackIndex = 0;
	ConstantExpression = YES;
	ExprLevA();
	if(ExprStackIndex != 1)
	{
		ERR_Exit(ERR_BAD_CONST_EXPR, YES, NULL);
	}
	return PopExStk();
}

//==========================================================================
//
// EvalExpression
//
//==========================================================================

static void EvalExpression(void)
{
	ConstantExpression = NO;
	ExprLevA();
}

// Operator: ||
static void ExprLevA(void)
{
	ExprLevB();
	while(tk_Token == TK_ORLOGICAL)
	{
		TK_NextToken();
		ExprLevB();
		SendExprCommand(PCD_ORLOGICAL);
	}
}

// Operator: &&
static void ExprLevB(void)
{
	ExprLevC();
	while(tk_Token == TK_ANDLOGICAL)
	{
		TK_NextToken();
		ExprLevC();
		SendExprCommand(PCD_ANDLOGICAL);
	}
}

// Operator: |
static void ExprLevC(void)
{
	ExprLevD();
	while(tk_Token == TK_ORBITWISE)
	{
		TK_NextToken();
		ExprLevD();
		SendExprCommand(PCD_ORBITWISE);
	}
}

// Operator: ^
static void ExprLevD(void)
{
	ExprLevE();
	while(tk_Token == TK_EORBITWISE)
	{
		TK_NextToken();
		ExprLevE();
		SendExprCommand(PCD_EORBITWISE);
	}
}

// Operator: &
static void ExprLevE(void)
{
	ExprLevF();
	while(tk_Token == TK_ANDBITWISE)
	{
		TK_NextToken();
		ExprLevF();
		SendExprCommand(PCD_ANDBITWISE);
	}
}

// Operators: == !=
static void ExprLevF(void)
{
	tokenType_t token;

	ExprLevG();
	while(TK_Member(LevFOps))
	{
		token = tk_Token;
		TK_NextToken();
		ExprLevG();
		SendExprCommand(TokenToPCD(token));
	}
}

// Operators: < <= > >=
static void ExprLevG(void)
{
	tokenType_t token;

	ExprLevH();
	while(TK_Member(LevGOps))
	{
		token = tk_Token;
		TK_NextToken();
		ExprLevH();
		SendExprCommand(TokenToPCD(token));
	}
}

// Operators: << >>
static void ExprLevH(void)
{
	tokenType_t token;

	ExprLevI();
	while(TK_Member(LevHOps))
	{
		token = tk_Token;
		TK_NextToken();
		ExprLevI();
		SendExprCommand(TokenToPCD(token));
	}
}

// Operators: + -
static void ExprLevI(void)
{
	tokenType_t token;

	ExprLevJ();
	while(TK_Member(LevIOps))
	{
		token = tk_Token;
		TK_NextToken();
		ExprLevJ();
		SendExprCommand(TokenToPCD(token));
	}
}

// Operators: * / %
static void ExprLevJ(void)
{
	tokenType_t token;
	boolean unaryMinus;

	unaryMinus = FALSE;
	if(tk_Token == TK_MINUS)
	{
		unaryMinus = TRUE;
		TK_NextToken();
	}
	if(ConstantExpression == YES)
	{
		ConstExprFactor();
	}
	else
	{
		ExprFactor();
	}
	if(unaryMinus == TRUE)
	{
		SendExprCommand(PCD_UNARYMINUS);
	}
	while(TK_Member(LevJOps))
	{
		token = tk_Token;
		TK_NextToken();
		if(ConstantExpression == YES)
		{
			ConstExprFactor();
		}
		else
		{
			ExprFactor();
		}
		SendExprCommand(TokenToPCD(token));
	}
}

static void ExprFactor(void)
{
	symbolNode_t *sym;
	tokenType_t opToken;

	switch(tk_Token)
	{
		case TK_STRING:
			PC_AppendCmd(PCD_PUSHNUMBER);
			PC_AppendLong(STR_Find(tk_String));
			TK_NextToken();
			break;
		case TK_NUMBER:
			PC_AppendCmd(PCD_PUSHNUMBER);
			PC_AppendLong(tk_Number);
			TK_NextToken();
			break;
		case TK_LPAREN:
			TK_NextToken();
			ExprLevA();
			if(tk_Token != TK_RPAREN)
			{
				ERR_Exit(ERR_BAD_EXPR, YES, NULL);
			}
			TK_NextToken();
			break;
		case TK_NOT:
			TK_NextToken();
			ExprFactor();
			PC_AppendCmd(PCD_NEGATELOGICAL);
			break;
		case TK_INC:
		case TK_DEC:
			opToken = tk_Token;
			TK_NextTokenMustBe(TK_IDENTIFIER, ERR_INCDEC_OP_ON_NON_VAR);
			sym = DemandSymbol(tk_String);
			if(sym->type != SY_SCRIPTVAR && sym->type != SY_MAPVAR
				&& sym->type != SY_WORLDVAR)
			{
				ERR_Exit(ERR_INCDEC_OP_ON_NON_VAR, YES, NULL);
			}
			PC_AppendCmd(GetIncDecPCD(opToken, sym->type));
			PC_AppendLong(sym->info.var.index);
			PC_AppendCmd(GetPushVarPCD(sym->type));
			PC_AppendLong(sym->info.var.index);
			TK_NextToken();
			break;
		case TK_IDENTIFIER:
			sym = DemandSymbol(tk_String);
			switch(sym->type)
			{
				case SY_SCRIPTVAR:
				case SY_MAPVAR:
				case SY_WORLDVAR:
					PC_AppendCmd(GetPushVarPCD(sym->type));
					PC_AppendLong(sym->info.var.index);
					TK_NextToken();
					if(tk_Token == TK_INC || tk_Token == TK_DEC)
					{
						PC_AppendCmd(GetIncDecPCD(tk_Token, sym->type));
						PC_AppendLong(sym->info.var.index);
						TK_NextToken();
					}
					break;
				case SY_INTERNFUNC:
					if(sym->info.internFunc.hasReturnValue == NO)
					{
						ERR_Exit(ERR_EXPR_FUNC_NO_RET_VAL, YES, NULL);
					}
					ProcessInternFunc(sym);
					break;
				default:
					ERR_Exit(ERR_ILLEGAL_EXPR_IDENT, YES,
						"Identifier: %s", tk_String);
					break;
			}
			break;
		default:
			ERR_Exit(ERR_BAD_EXPR, YES, NULL);
			break;
	}
}

static void ConstExprFactor(void)
{
	switch(tk_Token)
	{
		case TK_STRING:
			PushExStk(STR_Find(tk_String));
			TK_NextToken();
			break;
		case TK_NUMBER:
			PushExStk(tk_Number);
			TK_NextToken();
			break;
		case TK_LPAREN:
			TK_NextToken();
			ExprLevA();
			if(tk_Token != TK_RPAREN)
			{
				ERR_Exit(ERR_BAD_CONST_EXPR, YES, NULL);
			}
			TK_NextToken();
			break;
		case TK_NOT:
			TK_NextToken();
			ConstExprFactor();
			SendExprCommand(PCD_NEGATELOGICAL);
			break;
		default:
			ERR_Exit(ERR_BAD_CONST_EXPR, YES, NULL);
			break;
	}
}

//==========================================================================
//
// SendExprCommand
//
//==========================================================================

static void SendExprCommand(pcd_t pcd)
{
	int operand2;

	if(ConstantExpression == NO)
	{
		PC_AppendCmd(pcd);
		return;
	}
	switch(pcd)
	{
		case PCD_ADD:
			PushExStk(PopExStk()+PopExStk());
			break;
		case PCD_SUBTRACT:
			operand2 = PopExStk();
			PushExStk(PopExStk()-operand2);
			break;
		case PCD_MULTIPLY:
			PushExStk(PopExStk()*PopExStk());
			break;
		case PCD_DIVIDE:
			operand2 = PopExStk();
			PushExStk(PopExStk()/operand2);
			break;
		case PCD_MODULUS:
			operand2 = PopExStk();
			PushExStk(PopExStk()%operand2);
			break;
		case PCD_EQ:
			PushExStk(PopExStk() == PopExStk());
			break;
		case PCD_NE:
			PushExStk(PopExStk() != PopExStk());
			break;
		case PCD_LT:
			PushExStk(PopExStk() >= PopExStk());
			break;
		case PCD_GT:
			PushExStk(PopExStk() <= PopExStk());
			break;
		case PCD_LE:
			PushExStk(PopExStk() > PopExStk());
			break;
		case PCD_GE:
			PushExStk(PopExStk() < PopExStk());
			break;
		case PCD_ANDLOGICAL:
			PushExStk(PopExStk() && PopExStk());
			break;
		case PCD_ORLOGICAL:
			PushExStk(PopExStk() || PopExStk());
			break;
		case PCD_ANDBITWISE:
			PushExStk(PopExStk()&PopExStk());
			break;
		case PCD_ORBITWISE:
			PushExStk(PopExStk()|PopExStk());
			break;
		case PCD_EORBITWISE:
			PushExStk(PopExStk()^PopExStk());
			break;
		case PCD_NEGATELOGICAL:
			PushExStk(!PopExStk());
			break;
		case PCD_LSHIFT:
			operand2 = PopExStk();
			PushExStk(PopExStk()>>operand2);
			break;
		case PCD_RSHIFT:
			operand2 = PopExStk();
			PushExStk(PopExStk()<<operand2);
			break;
		case PCD_UNARYMINUS:
			PushExStk(-PopExStk());
			break;
		default:
			ERR_Exit(ERR_UNKNOWN_CONST_EXPR_PCD, YES, NULL);
			break;
	}
}

//==========================================================================
//
// PushExStk
//
//==========================================================================

static void PushExStk(int value)
{
	if(ExprStackIndex == EXPR_STACK_DEPTH)
	{
		ERR_Exit(ERR_EXPR_STACK_OVERFLOW, YES, NULL);
	}
	ExprStack[ExprStackIndex++] = value;
}

//==========================================================================
//
// PopExStk
//
//==========================================================================

static int PopExStk(void)
{
	if(ExprStackIndex < 1)
	{
		ERR_Exit(ERR_EXPR_STACK_EMPTY, YES, NULL);
	}
	return ExprStack[--ExprStackIndex];
}

//==========================================================================
//
// TokenToPCD
//
//==========================================================================

static pcd_t TokenToPCD(tokenType_t token)
{
	int i;
	static struct
	{
		tokenType_t token;
		pcd_t pcd;
	}  operatorLookup[] =
	{
		TK_EQ, PCD_EQ,
		TK_NE, PCD_NE,
		TK_LT, PCD_LT,
		TK_LE, PCD_LE,
		TK_GT, PCD_GT,
		TK_GE, PCD_GE,
		TK_LSHIFT, PCD_LSHIFT,
		TK_RSHIFT, PCD_RSHIFT,
		TK_PLUS, PCD_ADD,
		TK_MINUS, PCD_SUBTRACT,
		TK_ASTERISK, PCD_MULTIPLY,
		TK_SLASH, PCD_DIVIDE,
		TK_PERCENT, PCD_MODULUS,
		TK_NONE
	};

	for(i = 0; operatorLookup[i].token != TK_NONE; i++)
	{
		if(operatorLookup[i].token == token)
		{
			return operatorLookup[i].pcd;
		}
	}
	return PCD_NOP;
}

//==========================================================================
//
// GetPushVarPCD
//
//==========================================================================

static pcd_t GetPushVarPCD(symbolType_t symType)
{
	switch(symType)
	{
		case SY_SCRIPTVAR:
			return PCD_PUSHSCRIPTVAR;
		case SY_MAPVAR:
			return PCD_PUSHMAPVAR;
		case SY_WORLDVAR:
			return PCD_PUSHWORLDVAR;
		default:
			break;
	}
	return PCD_NOP;
}

//==========================================================================
//
// GetIncDecPCD
//
//==========================================================================

static pcd_t GetIncDecPCD(tokenType_t token, symbolType_t symbol)
{
	int i;
	static struct
	{
		tokenType_t token;
		symbolType_t symbol;
		pcd_t pcd;
	}  incDecLookup[] =
	{
		TK_INC, SY_SCRIPTVAR, PCD_INCSCRIPTVAR,
		TK_INC, SY_MAPVAR, PCD_INCMAPVAR,
		TK_INC, SY_WORLDVAR, PCD_INCWORLDVAR,
		TK_DEC, SY_SCRIPTVAR, PCD_DECSCRIPTVAR,
		TK_DEC, SY_MAPVAR, PCD_DECMAPVAR,
		TK_DEC, SY_WORLDVAR, PCD_DECWORLDVAR,
		TK_NONE
	};

	for(i = 0; incDecLookup[i].token != TK_NONE; i++)
	{
		if(incDecLookup[i].token == token
			&& incDecLookup[i].symbol == symbol)
		{
			return incDecLookup[i].pcd;
		}
	}
	return PCD_NOP;
}

//==========================================================================
//
// DemandSymbol
//
//==========================================================================

static symbolNode_t *DemandSymbol(char *name)
{
	symbolNode_t *sym;

	if((sym = SY_Find(name)) == NULL)
	{
		ERR_Exit(ERR_UNKNOWN_IDENTIFIER, YES,
			"Identifier: %s", name);
	}
	return sym;
}
