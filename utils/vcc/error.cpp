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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern char SourceFileName[MAX_FILE_NAME_LENGTH];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int			NumErrors = 0;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char* ErrorNames[NUM_ERRORS] =
{
	"No error.",
	//  File errors
	"Couldn't open file.",
	"Couldn't open debug file.",
	//  Tokenizer errors
	"Radix out of range in integer constant.",
	"String too long.",
	"End of file inside quoted string.",
	"New line inside quoted string.",
	"Unknown escape char.",
	"Identifier too long.",
	"Bad character.",
	//  Syntactic errors
	"Missing '('.",
	"Missing ')'.",
	"Missing '{'.",
	"Missing '}'.",
	"Missing colon.",
	"Missing semicolon.",
	"Unexpected end of file.",
	"Do statement not followed by 'while'.",
	"Invalid identifier.",
	"Function redeclared.",
	"Missing ']'.",
	"Invalid operation with array",
	"Expression type mistmatch",
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	FatalError
//
//==========================================================================

void FatalError(char *text, ...)
{
	char	workString[256];
	va_list	argPtr;

	va_start(argPtr, text);
	vsprintf(workString, text, argPtr);
	va_end(argPtr);
	fputs(workString, stderr);
	fputc('\n', stderr);
	exit(1);
}

//==========================================================================
//
//	ParseError
//
//==========================================================================

void ParseError(TLocation l, ECompileError error)
{
	NumErrors++;

	fprintf(stderr, "%s:%d: ", l.GetSource(), l.GetLine());
	if (error != ERR_NONE)
	{
		fprintf(stderr, "Error #%d - %s", error, ErrorNames[error]);
	}
	fputc('\n', stderr);

	if (NumErrors >= 64)
	{
		exit(1);
	}
}

//==========================================================================
//
//	ParseError
//
//==========================================================================

void ParseError(TLocation l, ECompileError error, const char *text, ...)
{
	va_list	argPtr;

	NumErrors++;

	fprintf(stderr, "%s:%d: ", l.GetSource(), l.GetLine());
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
		exit(1);
	}
}

//==========================================================================
//
//	ParseError
//
//==========================================================================

void ParseError(TLocation l, const char *text, ...)
{
	va_list	argPtr;

	NumErrors++;

	fprintf(stderr, "%s:%d: ", l.GetSource(), l.GetLine());
	va_start(argPtr, text);
	vfprintf(stderr, text, argPtr);
	va_end(argPtr);
	fputc('\n', stderr);

	if (NumErrors >= 64)
	{
		exit(1);
	}
}

//==========================================================================
//
//	ParseWarning
//
//==========================================================================

void ParseWarning(TLocation l, const char *text, ...)
{
	va_list	argPtr;

	fprintf(stderr, "%s:%d: warning: ", l.GetSource(), l.GetLine());
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

void BailOut()
{
	fprintf(stderr, "Confused by previous errors, bailing out\n");
	exit(1);
}
