
//**************************************************************************
//**
//** error.c
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "common.h"
#include "error.h"
#include "token.h"
#include "misc.h"

// MACROS ------------------------------------------------------------------

#define ERROR_FILE_NAME "acs.err"

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static char *ErrorFileName(void);
static char *ErrorText(error_t error);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern char acs_SourceFileName[MAX_FILE_NAME_LENGTH];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static struct
{
	error_t number;
	char *name;
} ErrorNames[] =
{
	ERR_MISSING_SEMICOLON,
	"Missing semicolon.",
	ERR_MISSING_LPAREN,
	"Missing '('.",
	ERR_MISSING_RPAREN,
	"Missing ')'.",
	ERR_MISSING_SCRIPT_NUMBER,
	"Missing script number.",
	ERR_IDENTIFIER_TOO_LONG,
	"Identifier too long.",
	ERR_STRING_TOO_LONG,
	"String too long.",
	ERR_FILE_NAME_TOO_LONG,
	"File name too long.",
	ERR_BAD_CHARACTER,
	"Bad character in script text.",
	ERR_ALLOC_PCODE_BUFFER,
	"Failed to allocate PCODE buffer.",
	ERR_PCODE_BUFFER_OVERFLOW,
	"PCODE buffer overflow.",
	ERR_TOO_MANY_SCRIPTS,
	"Too many scripts.",
	ERR_SAVE_OBJECT_FAILED,
	"Couldn't save object file.",
	ERR_MISSING_LPAREN_SCR,
	"Missing '(' in script definition.",
	ERR_INVALID_IDENTIFIER,
	"Invalid identifier.",
	ERR_REDEFINED_IDENTIFIER,
	"Redefined identifier.",
	ERR_MISSING_COMMA,
	"Missing comma.",
	ERR_BAD_VAR_TYPE,
	"Invalid variable type.",
	ERR_TOO_MANY_SCRIPT_ARGS,
	"Too many script arguments.",
	ERR_MISSING_LBRACE_SCR,
	"Missing opening '{' in script definition.",
	ERR_MISSING_RBRACE_SCR,
	"Missing closing '}' in script definition.",
	ERR_TOO_MANY_MAP_VARS,
	"Too many map variables.",
	ERR_TOO_MANY_SCRIPT_VARS,
	"Too many script variables.",
	ERR_MISSING_WVAR_INDEX,
	"Missing index in world variable declaration.",
	ERR_BAD_WVAR_INDEX,
	"World variable index out of range.",
	ERR_MISSING_WVAR_COLON,
	"Missing colon in world variable declaration.",
	ERR_MISSING_SPEC_VAL,
	"Missing value in special declaration.",
	ERR_MISSING_SPEC_COLON,
	"Missing colon in special declaration.",
	ERR_MISSING_SPEC_ARGC,
	"Missing argument count in special declaration.",
	ERR_CANT_READ_FILE,
	"Couldn't read file.",
	ERR_CANT_OPEN_FILE,
	"Couldn't open file.",
	ERR_CANT_OPEN_DBGFILE,
	"Couldn't open debug file.",
	ERR_INVALID_DIRECTIVE,
	"Invalid directive.",
	ERR_BAD_DEFINE,
	"Non-numeric constant found in #define.",
	ERR_INCL_NESTING_TOO_DEEP,
	"Include nesting too deep.",
	ERR_STRING_LIT_NOT_FOUND,
	"String literal not found.",
	ERR_INVALID_DECLARATOR,
	"Invalid declarator.",
	ERR_BAD_LSPEC_ARG_COUNT,
	"Incorrect number of special arguments.",
	ERR_BAD_ARG_COUNT,
	"Incorrect number of arguments.",
	ERR_UNKNOWN_IDENTIFIER,
	"Identifier has not been declared.",
	ERR_MISSING_COLON,
	"Missing colon.",
	ERR_BAD_EXPR,
	"Syntax error in expression.",
	ERR_BAD_CONST_EXPR,
	"Syntax error in constant expression.",
	ERR_NO_DIRECT_VER,
	"Internal function has no direct version.",
	ERR_ILLEGAL_EXPR_IDENT,
	"Illegal identifier in expression.",
	ERR_EXPR_FUNC_NO_RET_VAL,
	"Function call in expression has no return value.",
	ERR_MISSING_ASSIGN_OP,
	"Missing assignment operator.",
	ERR_INCDEC_OP_ON_NON_VAR,
	"'++' or '--' used on a non-variable.",
	ERR_MISSING_RBRACE,
	"Missing '}' at end of compound statement.",
	ERR_INVALID_STATEMENT,
	"Invalid statement.",
	ERR_BAD_DO_STATEMENT,
	"Do statement not followed by 'while' or 'until'.",
	ERR_BAD_SCRIPT_DECL,
	"Bad script declaration.",
	ERR_CASE_OVERFLOW,
	"Internal Error: Case stack overflow.",
	ERR_BREAK_OVERFLOW,
	"Internal Error: Break stack overflow.",
	ERR_CONTINUE_OVERFLOW,
	"Internal Error: Continue stack overflow.",
	ERR_STATEMENT_OVERFLOW,
	"Internal Error: Statement overflow.",
	ERR_MISPLACED_BREAK,
	"Misplaced BREAK statement.",
	ERR_MISPLACED_CONTINUE,
	"Misplaced CONTINUE statement.",
	ERR_CASE_NOT_IN_SWITCH,
	"CASE must appear in switch statement.",
	ERR_DEFAULT_NOT_IN_SWITCH,
	"DEFAULT must appear in switch statement.",
	ERR_MULTIPLE_DEFAULT,
	"Only 1 DEFAULT per switch allowed.",
	ERR_EXPR_STACK_OVERFLOW,
	"Expression stack overflow.",
	ERR_EXPR_STACK_EMPTY,
	"Tried to POP empty expression stack.",
	ERR_UNKNOWN_CONST_EXPR_PCD,
	"Unknown PCD in constant expression.",
	ERR_BAD_RADIX_CONSTANT,
	"Radix out of range in integer constant.",
	ERR_BAD_ASSIGNMENT,
	"Syntax error in multiple assignment statement.",
	ERR_OUT_OF_MEMORY,
	"Out of memory.",
	ERR_TOO_MANY_STRINGS,
	"Too many strings.",
	ERR_UNKNOWN_PRTYPE,
	"Unknown cast type in print statement.",
	ERR_NONE, NULL
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// ERR_Exit
//
//==========================================================================

void ERR_Exit(error_t error, boolean info, char *text, ...)
{
	char workString[256];
	va_list argPtr;
	FILE *errFile;

	errFile = fopen(ErrorFileName(), "w");
	fprintf(stderr, "**** ERROR ****\n");
	if(info == YES)
	{
		sprintf(workString, "Line %d in file \"%s\" ...\n", tk_Line,
			tk_SourceName);
		fprintf(stderr, workString);
		if(errFile)
		{
			fprintf(errFile, workString);
		}
	}
	if(error != ERR_NONE)
	{
		if(ErrorText(error) != NULL)
		{
			sprintf(workString, "Error #%d: %s\n", error,
				ErrorText(error));
			fprintf(stderr, workString);
			if(errFile)
			{
				fprintf(errFile, workString);
			}
		}
	}
	if(text)
	{
		va_start(argPtr, text);
		vsprintf(workString, text, argPtr);
		va_end(argPtr);
		fputs(workString, stderr);
		fputc('\n', stderr);
		if(errFile)
		{
			fprintf(errFile, workString);
		}
	}
	if(errFile)
	{
		fclose(errFile);
	}
	exit(1);
}

//==========================================================================
//
// ERR_RemoveErrorFile
//
//==========================================================================

void ERR_RemoveErrorFile(void)
{
	remove(ErrorFileName());
}

//==========================================================================
//
// ErrorFileName
//
//==========================================================================

static char *ErrorFileName(void)
{
	static char errFileName[MAX_FILE_NAME_LENGTH];

	strcpy(errFileName, acs_SourceFileName);
	if(MS_StripFilename(errFileName) == NO)
	{
		strcpy(errFileName, ERROR_FILE_NAME);
	}
	else
	{
		strcat(errFileName, DIRECTORY_DELIMITER ERROR_FILE_NAME);
	}
	return errFileName;
}

//==========================================================================
//
// ErrorText
//
//==========================================================================

static char *ErrorText(error_t error)
{
	int i;

	for(i = 0; ErrorNames[i].number != ERR_NONE; i++)
	{
		if(error == ErrorNames[i].number)
		{
			return ErrorNames[i].name;
		}
	}
	return NULL;
}
