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

#include "vc_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int			NumErrors = 0;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static const char* ErrorNames[NUM_ERRORS] =
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
//	ParseWarning
//
//==========================================================================

void ParseWarning(TLocation l, const char *text, ...)
{
	char		Buffer[2048];
	va_list		argPtr;

	va_start(argPtr, text);
	vsprintf(Buffer, text, argPtr);
	va_end(argPtr);
#ifdef IN_VCC
	fprintf(stderr, "%s:%d: warning: %s\n", *l.GetSource(), l.GetLine(), Buffer);
#else
	GCon->Logf("%s:%d: warning: %s", *l.GetSource(), l.GetLine(), Buffer);
#endif
}

//==========================================================================
//
//	ParseError
//
//==========================================================================

void ParseError(TLocation l, const char *text, ...)
{
	char		Buffer[2048];
	va_list		argPtr;

	NumErrors++;

	va_start(argPtr, text);
	vsprintf(Buffer, text, argPtr);
	va_end(argPtr);
#ifdef IN_VCC
	fprintf(stderr, "%s:%d: %s\n", *l.GetSource(), l.GetLine(), Buffer);
#else
	GCon->Logf("%s:%d: %s", *l.GetSource(), l.GetLine(), Buffer);
#endif

	if (NumErrors >= 64)
	{
		Sys_Error("Too many errors");
	}
}

//==========================================================================
//
//	ParseError
//
//==========================================================================

void ParseError(TLocation l, ECompileError error)
{
	ParseError(l, "Error #%d - %s", error, ErrorNames[error]);
}

//==========================================================================
//
//	ParseError
//
//==========================================================================

void ParseError(TLocation l, ECompileError error, const char *text, ...)
{
	char		Buffer[2048];
	va_list		argPtr;

	va_start(argPtr, text);
	vsprintf(Buffer, text, argPtr);
	va_end(argPtr);
	ParseError(l, "Error #%d - %s, %s", error, ErrorNames[error], Buffer);
}

//==========================================================================
//
//	BailOut
//
//==========================================================================

void BailOut()
{
	Sys_Error("Confused by previous errors, bailing out\n");
}

#ifdef IN_VCC

//==========================================================================
//
//	FatalError
//
//==========================================================================

void FatalError(const char *text, ...)
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

#endif
