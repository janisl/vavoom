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

#include <signal.h>
#include "vcc.h"

// MACROS ------------------------------------------------------------------

#define ERROR_FILE_NAME "vcc.err"

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static char *ErrorFileName(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern char SourceFileName[MAX_FILE_NAME_LENGTH];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int			NumErrors = 0;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char* ErrorNames[NUM_ERRORS] =
{
	"No error.",
	//  Atmi·as kõÝdas
	"Internal Error: Too many strings.",
	"Internal Error: Break stack overflow.",
	"Internal Error: Continue stack overflow.",
	"Internal Error: Case stack overflow.",
	"Internel Error: Too meny params.",
	"Internal Error: Local vars overflow.",
	"Internal Error: Statement overflow.",
	//  Failu kõÝdas
	"Couldn't open file.",
	"Couldn't open debug file.",
	//  SadalØýanas kõÝdas
	"Include nesting too deep.",
	"Radix out of range in integer constant.",
	"String too long.",
	"End of file inside quoted string.",
	"New line inside quoted string.",
	"Unknown escape char.",
	"Identifier too long.",
	"Bad character.",
	"Unterminated comment.",
	"Too meny constants.",
	//  Sintakses kõÝdas
	"Syntax error in constant expression.",
	"Syntax error in expression.",
	"Missing '('.",
	"Missing ')'.",
	"Missing '{'.",
	"Missing '}'.",
	"Missing colon.",
	"Missing semicolon.",
	"Incorrect number of arguments.",
	"Operation with void value.",
	"Parameter type mistmatch.",
	"Illegal identifier in expression.",
	"Bad assignement.",
	"Misplaced BREAK statement.",
	"Misplaced CONTINUE statement.",
	"Unexpected end of file.",
	"Do statement not followed by 'while'.",
	"Return value expected.",
	"viod function cannot return a value.",
	"Only 1 DEFAULT per switch allowed.",
	"Invalid statement.",
	"Invalid identifier.",
	"Void variable type",
	"Redefined identifier.",
	"Type mistmatch with previous function declaration",
	"Invalid variable type.",
	"Function redeclared.",
	"Invalid declarator.",
	"Invalid directive",
	"String literal not found.",
	"Undefined functions",
	"'++' or '--' can be used only on a int variable.",
	"End of non void function.",
	"Not a structure pointer.",
	"Invalid structure field name.",
	"Missing ']'.",
	"Invalid operation with array",
	"Expression type mistmatch",
	"Pointer to pointer is not allowed.",
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	ERR_Exit
//
//==========================================================================

void ERR_Exit(error_t error, boolean info, char *text, ...)
{
	char	workString[256];
	va_list	argPtr;
	FILE*	errFile;

	TK_CloseSource();
	errFile = fopen(ErrorFileName(), "w");
	if (errFile)
	{
		fprintf(errFile, "**** ERROR ****\n");
	}
	if (info)
	{
		sprintf(workString, "%s:%d: ", tk_SourceName, tk_Line);
		fprintf(stderr, workString);
		if (errFile)
		{
			fprintf(errFile, workString);
		}
	}
	if (error != ERR_NONE)
	{
		sprintf(workString, "Error #%d - %s", error, ErrorNames[error]);
		fprintf(stderr, workString);
		if (errFile)
		{
			fprintf(errFile, workString);
		}
	}
	if (text)
	{
		va_start(argPtr, text);
		vsprintf(workString, text, argPtr);
		va_end(argPtr);
		fputs(workString, stderr);
		if(errFile)
		{
			fprintf(errFile, workString);
		}
	}
	fputc('\n', stderr);
	if (errFile)
	{
		fputc('\n', errFile);
		fclose(errFile);
	}
	exit(1);
}

//==========================================================================
//
//	ParseError
//
//==========================================================================

void ParseError(error_t error)
{
	NumErrors++;

	fprintf(stderr, "%s:%d: ", tk_SourceName, tk_Line);
	if (error != ERR_NONE)
	{
		fprintf(stderr, "Error #%d - %s", error, ErrorNames[error]);
	}
	fputc('\n', stderr);

	if (NumErrors >= 64)
	{
		TK_CloseSource();
		exit(1);
	}
}

//==========================================================================
//
//	ParseError
//
//==========================================================================

void ParseError(error_t error, const char *text, ...)
{
	va_list	argPtr;

	NumErrors++;

	fprintf(stderr, "%s:%d: ", tk_SourceName, tk_Line);
	if (error != ERR_NONE)
	{
		fprintf(stderr, "Error #%d - %s", error, ErrorNames[error]);
	}
	va_start(argPtr, text);
	vfprintf(stderr, text, argPtr);
	va_end(argPtr);
	fputc('\n', stderr);

	if (NumErrors >= 64)
	{
		TK_CloseSource();
		exit(1);
	}
}

//==========================================================================
//
//	ParseError
//
//==========================================================================

void ParseError(const char *text, ...)
{
	va_list	argPtr;

	NumErrors++;

	fprintf(stderr, "%s:%d: ", tk_SourceName, tk_Line);
	va_start(argPtr, text);
	vfprintf(stderr, text, argPtr);
	va_end(argPtr);
	fputc('\n', stderr);

	if (NumErrors >= 64)
	{
		TK_CloseSource();
		exit(1);
	}
}

//==========================================================================
//
//	ParseWarning
//
//==========================================================================

void ParseWarning(const char *text, ...)
{
	va_list	argPtr;

	fprintf(stderr, "%s:%d: warning: ", tk_SourceName, tk_Line);
	va_start(argPtr, text);
	vfprintf(stderr, text, argPtr);
	va_end(argPtr);
	fputc('\n', stderr);
}

//==========================================================================
//
//	BailOut
//
//==========================================================================

void BailOut(void)
{
	fprintf(stderr, "%s:%d: Confused by previous errors, bailing out\n", tk_SourceName, tk_Line);
	TK_CloseSource();
	exit(1);
}

//==========================================================================
//
//	ERR_RemoveErrorFile
//
//==========================================================================

void ERR_RemoveErrorFile(void)
{
	remove(ErrorFileName());
}

//==========================================================================
//
//	ErrorFileName
//
//==========================================================================

static char *ErrorFileName(void)
{
	static char errFileName[MAX_FILE_NAME_LENGTH];

	strcpy(errFileName, SourceFileName);
	StripFilename(errFileName);
	strcat(errFileName, "/" ERROR_FILE_NAME);
	return errFileName;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2002/01/07 12:31:36  dj_jl
//	Changed copyright year
//
//	Revision 1.4  2001/09/20 16:09:55  dj_jl
//	Added basic object-oriented support
//	
//	Revision 1.3  2001/08/21 17:52:54  dj_jl
//	Added support for real string pointers, beautification
//	
//	Revision 1.2  2001/07/27 14:27:56  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
