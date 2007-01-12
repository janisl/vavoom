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

// PUBLIC DATA DEFINITIONS -------------------------------------------------

TArray<char*>		TLocation::SourceFiles;

const char*			VLexer::TokenNames[] =
{
	"",
	"END OF FILE",
	"IDENTIFIER",
	"NAME LITERAL",
	"STRING LITERAL",
	"INTEGER LITERAL",
	"FLOAT LITERAL",
	//	Keywords
	"abstract",
	"bool",
	"break",
	"case",
	"class",
	"const",
	"continue",
	"default",
	"defaultproperties",
	"delegate",
	"do",
	"else",
	"enum",
	"false",
	"final",
	"float",
	"for",
	"get",
	"if",
	"import",
	"int",
	"name",
	"native",
	"none",
	"NULL",
	"optional",
	"out",
	"private",
	"readonly",
	"return",
	"self",
	"set",
	"state",
	"static",
	"string",
	"struct",
	"switch",
	"transient",
	"true",
	"vector",
	"void",
	"while",
	"__states__",
	"__mobjinfo__",
	"__scriptid__",
	//	Punctuation
	"...",
	"<<=",
	">>=",
	"+=",
	"-=",
	"*=",
	"/=",
	"%=",
	"&=",
	"|=",
	"^=",
	"==",
	"!=",
	"<=",
	">=",
	"&&",
	"||",
	"<<",
	">>",
	"++",
	"--",
	"->",
	"::",
	"<",
	">",
	"?",
	"&",
	"|",
	"^",
	"~",
	"!",
	"+",
	"-",
	"*",
	"/",
	"%",
	"(",
	")",
	".",
	",",
	";",
	":",
	"=",
	"[",
	"]",
	"{",
	"}",
};

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VLexer::VLexer
//
//==========================================================================

VLexer::VLexer()
: IncLineNumber(false)
, NewLine(false)
, SourceOpen(false)
, FileStart(NULL)
, FilePtr(NULL)
, FileEnd(NULL)
, SourceIdx(0)
, Line(1)
, Token(TK_NoToken)
, Number(0)
, Float(0)
, Name(NAME_None)
{
	int i;

	memset(TokenStringBuffer, 0, sizeof(TokenStringBuffer));

	for (i = 0; i < 256; i++)
	{
		ASCIIToChrCode[i] = CHR_Special;
		ASCIIToHexDigit[i] = NON_HEX_DIGIT;
	}
	for (i = '0'; i <= '9'; i++)
	{
		ASCIIToChrCode[i] = CHR_Number;
		ASCIIToHexDigit[i] = i-'0';
	}
	for (i = 'A'; i <= 'F'; i++)
	{
		ASCIIToHexDigit[i] = 10+(i-'A');
	}
	for (i = 'a'; i <= 'f'; i++)
	{
		ASCIIToHexDigit[i] = 10+(i-'a');
	}
	for (i = 'A'; i <= 'Z'; i++)
	{
		ASCIIToChrCode[i] = CHR_Letter;
	}
	for (i = 'a'; i <= 'z'; i++)
	{
		ASCIIToChrCode[i] = CHR_Letter;
	}
	ASCIIToChrCode[(int)'\"'] = CHR_Quote;
	ASCIIToChrCode[(int)'\''] = CHR_SingleQuote;
	ASCIIToChrCode[(int)'_'] = CHR_Letter;
	ASCIIToChrCode[0] = CHR_EOF;
	ASCIIToChrCode[EOF_CHARACTER] = CHR_EOF;
	String = TokenStringBuffer;
}

//==========================================================================
//
//	VLexer::OpenSource
//
//==========================================================================

void VLexer::OpenSource(void* buf, size_t size)
{
	//	Read file and prepare for compilation.
	SourceOpen = true;
	FileStart = (char*)buf;
	FileEnd = FileStart + size;
	FilePtr = FileStart;
	SourceIdx = 0;
	Line = 1;
	Location = TLocation(SourceIdx, Line);
	Token = TK_NoToken;
	NewLine = true;
	NextChr();
}

//==========================================================================
//
//	VLexer::~VLexer
//
//==========================================================================

VLexer::~VLexer()
{
	if (SourceOpen)
	{
		Free(FileStart);
		SourceOpen = false;
	}
}

//==========================================================================
//
//	VLexer::NextChr
//
//==========================================================================

void VLexer::NextChr()
{
	if (FilePtr >= FileEnd)
	{
		Chr = EOF_CHARACTER;
		return;
	}
	if (IncLineNumber)
	{
		Line++;
		Location = TLocation(SourceIdx, Line);
		IncLineNumber = false;
	}
	Chr = *FilePtr++;
	if ((vuint8)Chr < ' ')
	{
		if (Chr == '\n')
		{
			IncLineNumber = true;
			NewLine = true;
		}
		Chr = ' ';
	}
}

//==========================================================================
//
//	VLexer::ProcessNumberToken
//
//==========================================================================

void VLexer::ProcessNumberToken()
{
	char c;

	Token = TK_IntLiteral;
	c = Chr;
	NextChr();
	Number = c - '0';
	if (c == '0' && (Chr == 'x' || Chr == 'X'))
	{
		//  Hexadecimal constant.
		NextChr();
		while (ASCIIToHexDigit[(vuint8)Chr] != NON_HEX_DIGIT)
		{
			Number = (Number << 4) + ASCIIToHexDigit[(vuint8)Chr];
			NextChr();
		}
		return;
	}
	while (ASCIIToChrCode[(vuint8)Chr] == CHR_Number)
	{
		Number = 10 * Number + (Chr - '0');
		NextChr();
	}
	if (Chr == '.')
	{
		Token = TK_FloatLiteral;
		NextChr(); // Point
		Float = Number;
		float	fmul = 0.1;
		while (ASCIIToChrCode[(vuint8)Chr] == CHR_Number)
		{
			Float += (Chr - '0') * fmul;
			fmul /= 10.0;
			NextChr();
		}
		return;
	}
	if (Chr == '_')
	{
		int radix;
		int digit;

		NextChr(); // Underscore
		radix = Number;
		if (radix < 2 || radix > 36)
		{
			ParseError(Location, ERR_BAD_RADIX_CONSTANT);
			radix = 2;
		}
		Number = 0;
		do
		{
			digit = toupper(Chr);
			if (digit < '0' || (digit > '9' && digit < 'A') || digit > 'Z')
			{
				digit = -1;
			}
			else if(digit > '9')
			{
				digit = 10 + digit - 'A';
			}
			else
			{
				digit -= '0';
			}
			if (digit >= radix)
			{
				digit = -1;
			}
			if (digit != -1)
			{
				Number = radix * Number + digit;
				NextChr();
			}
		} while (digit != -1);
	}
}

//==========================================================================
//
//	VLexer::ProcessChar
//
//==========================================================================

void VLexer::ProcessChar()
{
	if (Chr == EOF_CHARACTER)
	{
		ParseError(Location, ERR_EOF_IN_STRING);
		BailOut();
	}
	if (IncLineNumber)
	{
		ParseError(Location, ERR_NEW_LINE_INSIDE_QUOTE);
	}
	if (Chr == '\\')
	{
		//	Special symbol
		NextChr();
		if (Chr == EOF_CHARACTER)
		{
			ParseError(Location, ERR_EOF_IN_STRING);
			BailOut();
		}
		if (IncLineNumber)
		{
			ParseError(Location, ERR_NEW_LINE_INSIDE_QUOTE);
		}
		if (Chr == 'n')
			Chr = '\n';
		else if (Chr == '\'')
			Chr = '\'';
		else if (Chr == '"')
			Chr = '"';
		else if (Chr == 't')
			Chr = '\t';
		else if (Chr == '\\')
			Chr = '\\';
		else
			ParseError(Location, ERR_UNKNOWN_ESC_CHAR);
	}
}

//==========================================================================
//
//	VLexer::ProcessQuoteToken
//
//==========================================================================

void VLexer::ProcessQuoteToken()
{
	int len;

	Token = TK_StringLiteral;
	len = 0;
	NextChr();
	while (Chr != '\"')
	{
		if (len >= MAX_QUOTED_LENGTH - 1)
		{
			ParseError(Location, ERR_STRING_TOO_LONG);
			NextChr();
			continue;
		}
		ProcessChar();
		TokenStringBuffer[len] = Chr;
		NextChr();
		len++;
	}
	TokenStringBuffer[len] = 0;
	NextChr();
}

//==========================================================================
//
//	VLexer::ProcessSingleQuoteToken
//
//==========================================================================

void VLexer::ProcessSingleQuoteToken()
{
	int len;

	Token = TK_NameLiteral;
	len = 0;
	NextChr();
	while (Chr != '\'')
	{
		if (len >= MAX_IDENTIFIER_LENGTH - 1)
		{
			ParseError(Location, ERR_STRING_TOO_LONG);
			NextChr();
			continue;
		}
		ProcessChar();
		TokenStringBuffer[len] = Chr;
		NextChr();
		len++;
	}
	TokenStringBuffer[len] = 0;
	NextChr();
	Name = TokenStringBuffer;
}

//==========================================================================
//
//	VLexer::ProcessLetterToken
//
//==========================================================================

void VLexer::ProcessLetterToken()
{
	int		len;

	Token = TK_Identifier;
	len = 0;
	while (ASCIIToChrCode[(vuint8)Chr] == CHR_Letter
		|| ASCIIToChrCode[(vuint8)Chr] == CHR_Number)
	{
		if (len == MAX_IDENTIFIER_LENGTH - 1)
		{
			ParseError(Location, ERR_IDENTIFIER_TOO_LONG);
			NextChr();
			continue;
		}
		TokenStringBuffer[len] = Chr;
		len++;
		NextChr();
	}
	TokenStringBuffer[len] = 0;

	register const char* s = TokenStringBuffer;
	switch (s[0])
	{
	case '_':
		if (s[1] == '_')
		{
			if (s[2] == 'm' && s[3] == 'o' && s[4] == 'b' && s[5] == 'j' &&
				s[6] == 'i' && s[7] == 'n' && s[8] == 'f' && s[9] == 'o' &&
				s[10] == '_' && s[11] == '_' && s[12] == 0)
			{
				Token = TK_MobjInfo;
			}
			else if (s[2] == 's')
			{
				if (s[3] == 't' && s[4] == 'a' && s[5] == 't' && s[6] == 'e' &&
					s[7] == 's' && s[8] == '_' && s[9] == '_' && s[10] == 0)
				{
					Token = TK_States;
				}
				else if (s[3] == 'c' && s[4] == 'r' && s[5] == 'i' &&
					s[6] == 'p' && s[7] == 't' && s[8] == 'i' && s[9] == 'd' &&
					s[10] == '_' && s[11] == '_' && s[12] == 0)
				{
					Token = TK_ScriptId;
				}
			}
		}
		break;

	case 'a':
		if (s[1] == 'b' && s[2] == 's' && s[3] == 't' && s[4] == 'r' &&
			s[5] == 'a' && s[6] == 'c' && s[7] == 't' && s[8] == 0)
		{
			Token = TK_Abstract;
		}
		break;

	case 'b':
		if (s[1] == 'o' && s[2] == 'o' && s[3] == 'l' && s[4] == 0)
		{
			Token = TK_Bool;
		}
		else if (s[1] == 'r' && s[2] == 'e' && s[3] == 'a' && s[4] == 'k' &&
			s[5] == 0)
		{
			Token = TK_Break;
		}
		break;

	case 'c':
		if (s[1] == 'a' && s[2] == 's' && s[3] == 'e' && s[4] == 0)
		{
			Token = TK_Case;
		}
		else if (s[1] == 'l' && s[2] == 'a' && s[3] == 's' && s[4] == 's' &&
			s[5] == 0)
		{
			Token = TK_Class;
		}
		else if (s[1] == 'o' && s[2] == 'n')
		{
			if (s[3] == 's' && s[4] == 't' && s[5] == 0)
			{
				Token = TK_Const;
			}
			else if (s[3] == 't' && s[4] == 'i' && s[5] == 'n' &&
				s[6] == 'u' && s[7] == 'e' && s[8] == 0)
			{
				Token = TK_Continue;
			}
		}
		break;

	case 'd':
		if (s[1] == 'e')
		{
			if (s[2] == 'f' && s[3] == 'a' && s[4] == 'u' && s[5] == 'l' &&
				s[6] == 't')
			{
				if (s[7] == 0)
				{
					Token = TK_Default;
				}
				else if (s[7] == 'p' && s[8] == 'r' && s[9] == 'o' &&
					s[10] == 'p' && s[11] == 'e' && s[12] == 'r' &&
					s[13] == 't' && s[14] == 'i' && s[15] == 'e' &&
					s[16] == 's' && s[17] == 0)
				{
					Token = TK_DefaultProperties;
				}
			}
			else if (s[2] == 'l' && s[3] == 'e' && s[4] == 'g' &&
				s[5] == 'a' && s[6] == 't' && s[7] == 'e' && s[8] == 0)
			{
				Token = TK_Delegate;
			}
		}
		else if (s[1] == 'o' && s[2] == 0)
		{
			Token = TK_Do;
		}
		break;

	case 'e':
		if (s[1] == 'l' && s[2] == 's' && s[3] == 'e' && s[4] == 0)
		{
			Token = TK_Else;
		}
		else if (s[1] == 'n' && s[2] == 'u' && s[3] == 'm' && s[4] == 0)
		{
			Token = TK_Enum;
		}
		break;

	case 'f':
		if (s[1] == 'a' && s[2] == 'l' && s[3] == 's' && s[4] == 'e' &&
			s[5] == 0)
		{
			Token = TK_False;
		}
		else if (s[1] == 'i' && s[2] == 'n' && s[3] == 'a' && s[4] == 'l' &&
			s[5] == 0)
		{
			Token = TK_Final;
		}
		else if (s[1] == 'l' && s[2] == 'o' && s[3] == 'a' && s[4] == 't' &&
			s[5] == 0)
		{
			Token = TK_Float;
		}
		else if (s[1] == 'o' && s[2] == 'r' && s[3] == 0)
		{
			Token = TK_For;
		}
		break;

	case 'g':
		if (s[1] == 'e' && s[2] == 't' && s[3] == 0)
		{
			Token = TK_Get;
		}

	case 'i':
		if (s[1] == 'f' && s[2] == 0)
		{
			Token = TK_If;
		}
		else if (s[1] == 'm' && s[2] == 'p' && s[3] == 'o' && s[4] == 'r' &&
			s[5] == 't' && s[6] == 0)
		{
			Token = TK_Import;
		}
		else if (s[1] == 'n' && s[2] == 't' && s[3] == 0)
		{
			Token = TK_Int;
		}
		break;

	case 'n':
		if (s[1] == 'a' && s[2] == 'm' && s[3] == 'e' && s[4] == 0)
		{
			Token = TK_Name;
		}
		else if (s[1] == 'o' && s[2] == 'n' && s[3] == 'e' && s[4] == 0)
		{
			Token = TK_None;
		}
		else if (s[1] == 'a' && s[2] == 't' && s[3] == 'i' && s[4] == 'v' &&
			s[5] == 'e' && s[6] == 0)
		{
			Token = TK_Native;
		}
		break;

/*	case 'N':
		if (s[1] == 'U' && s[2] == 'L' &&
			s[3] == 'L' && s[4] == 0)
		{
			Token = TK_KEYWORD;
			tk_Keyword = KW_NULL;
		}
		break;*/

	case 'o':
		if (s[1] == 'p' && s[2] == 't' && s[3] == 'i' && s[4] == 'o' &&
			s[5] == 'n' && s[6] == 'a' && s[7] == 'l' && s[8] == 0)
		{
			Token = TK_Optional;
		}
		else if (s[1] == 'u' && s[2] == 't' && s[3] == 0)
		{
			Token = TK_Out;
		}
		break;

	case 'p':
		if (s[1] == 'r' && s[2] == 'i' && s[3] == 'v' && s[4] == 'a' &&
			s[5] == 't' && s[6] == 'e' && s[7] == 0)
		{
			Token = TK_Private;
		}
		break;

	case 'r':
		if (s[1] == 'e')
		{
			if (s[2] == 'a' && s[3] == 'd' && s[4] == 'o' && s[5] == 'n' &&
				s[6] == 'l' && s[7] == 'y' && s[8] == 0)
			{
				Token = TK_ReadOnly;
			}
			else if (s[2] == 't' && s[3] == 'u' && s[4] == 'r' &&
				s[5] == 'n' && s[6] == 0)
			{
				Token = TK_Return;
			}
		}
		break;

	case 's':
		if (s[1] == 'e')
		{
			if (s[2] == 'l' && s[3] == 'f' && s[4] == 0)
			{
				Token = TK_Self;
			}
			else if (s[2] == 't' && s[3] == 0)
			{
				Token = TK_Set;
			}
		}
		else if (s[1] == 't')
		{
			if (s[2] == 'a' && s[3] == 't')
			{
				if (s[4] == 'e' && s[5] == 0)
				{
					Token = TK_State;
				}
				else if (s[4] == 'i' && s[5] == 'c' && s[6] == 0)
				{
					Token = TK_Static;
				}
			}
			else if (s[2] == 'r')
			{
				if (s[3] == 'i' && s[4] == 'n' && s[5] == 'g' && s[6] == 0)
				{
					Token = TK_String;
				}
				else if (s[3] == 'u' && s[4] == 'c' && s[5] == 't' &&
					s[6] == 0)
				{
					Token = TK_Struct;
				}
			}
		}
		else if (s[1] == 'w' && s[2] == 'i' && s[3] == 't' && s[4] == 'c' &&
			s[5] == 'h' && s[6] == 0)
		{
			Token = TK_Switch;
		}
		break;

	case 't':
		if (s[1] == 'r')
		{
			if (s[2] == 'a' && s[3] == 'n' && s[4] == 's' && s[5] == 'i' &&
				s[6] == 'e' && s[7] == 'n' && s[8] == 't' && s[9] == 0)
			{
				Token = TK_Transient;
			}
			else if (s[2] == 'u' && s[3] == 'e' && s[4] == 0)
			{
				Token = TK_True;
			}
		}
		break;

	case 'v':
		if (s[1] == 'e' && s[2] == 'c' && s[3] == 't' && s[4] == 'o' &&
			s[5] == 'r' && s[6] == 0)
		{
			Token = TK_Vector;
		}
		else if (s[1] == 'o' && s[2] == 'i' && s[3] == 'd' && s[4] == 0)
		{
			Token = TK_Void;
		}
		break;

	case 'w':
		if (s[1] == 'h' && s[2] == 'i' && s[3] == 'l' && s[4] == 'e' &&
			s[5] == 0)
		{
			Token = TK_While;
		}
		break;
	}
	if (s[0] == 'N' && s[1] == 'U' && s[2] == 'L' && s[3] == 'L' && s[4] == 0)
	{
		Token = TK_Null;
	}

	if (Token == TK_Identifier)
	{
		Name = TokenStringBuffer;
	}
}

//==========================================================================
//
//	VLexer::ProcessSpecialToken
//
//==========================================================================

void VLexer::ProcessSpecialToken()
{
	char c = Chr;
	NextChr();
	switch (c)
	{
	case '+':
		if (Chr == '=')
		{
			Token = TK_AddAssign;
			NextChr();
		}
		else if (Chr == '+')
		{
			Token = TK_Inc;
			NextChr();
		}
		else
		{
			Token = TK_Plus;
		}
		break;

	case '-':
		if (Chr == '=')
		{
			Token = TK_MinusAssign;
			NextChr();
		}
		else if (Chr == '-')
		{
			Token = TK_Dec;
			NextChr();
		}
		else if (Chr == '>')
		{
			Token = TK_Arrow;
			NextChr();
		}
		else
		{
			Token = TK_Minus;
		}
		break;

	case '*':
		if (Chr == '=')
		{
			Token = TK_MultiplyAssign;
			NextChr();
		}
		else
		{
			Token = TK_Asterisk;
		}
		break;

	case '/':
		if (Chr == '=')
		{
			Token = TK_DivideAssign;
			NextChr();
		}
		else
		{
			Token = TK_Slash;
		}
		break;

	case '%':
		if (Chr == '=')
		{
			Token = TK_ModAssign;
			NextChr();
		}
		else
		{
			Token = TK_Percent;
		}
		break;

	case '=':
		if (Chr == '=')
		{
			Token = TK_Equals;
			NextChr();
		}
		else
		{
			Token = TK_Assign;
		}
		break;

	case '<':
		if (Chr == '<')
		{
			NextChr();
			if (Chr == '=')
			{
				Token = TK_LShiftAssign;
				NextChr();
			}
			else
			{
				Token = TK_LShift;
			}
		}
		else if (Chr == '=')
		{
			Token = TK_LessEquals;
			NextChr();
		}
		else
		{
			Token = TK_Less;
		}
		break;

	case '>':
		if (Chr == '>')
		{
			NextChr();
			if (Chr == '=')
			{
				Token = TK_RShiftAssign;
				NextChr();
			}
			else
			{
				Token = TK_RShift;
			}
		}
		else if (Chr == '=')
		{
			Token = TK_GreaterEquals;
			NextChr();
		}
		else
		{
			Token = TK_Greater;
		}
		break;

	case '!':
		if (Chr == '=')
		{
			Token = TK_NotEquals;
			NextChr();
		}
		else
		{
			Token = TK_Not;
		}
		break;

	case '&':
		if (Chr == '=')
		{
			Token = TK_AndAssign;
			NextChr();
		}
		else if (Chr == '&')
		{
			Token = TK_AndLog;
			NextChr();
		}
		else
		{
			Token = TK_And;
		}
		break;

	case '|':
		if (Chr == '=')
		{
			Token = TK_OrAssign;
			NextChr();
		}
		else if (Chr == '|')
		{
			Token = TK_OrLog;
			NextChr();
		}
		else
		{
			Token = TK_Or;
		}
		break;

	case '^':
		if (Chr == '=')
		{
			Token = TK_XOrAssign;
			NextChr();
		}
		else
		{
			Token = TK_XOr;
		}
		break;

	case '.':
		if (Chr == '.' && FilePtr[0] == '.')
		{
			Token = TK_VarArgs;
			NextChr();
			NextChr();
		}
		else
		{
			Token = TK_Dot;
		}
		break;

	case ':':
		if (Chr == ':')
		{
			Token = TK_DColon;
			NextChr();
		}
		else
		{
			Token = TK_Colon;
		}
		break;

	case '(':
		Token = TK_LParen;
		break;

	case ')':
		Token = TK_RParen;
		break;

	case '?':
		Token = TK_Quest;
		break;

	case '~':
		Token = TK_Tilde;
		break;

	case ',':
		Token = TK_Comma;
		break;

	case ';':
		Token = TK_Semicolon;
		break;

	case '[':
		Token = TK_LBracket;
		break;

	case ']':
		Token = TK_RBracket;
		break;

	case '{':
		Token = TK_LBrace;
		break;

	case '}':
		Token = TK_RBrace;
		break;

	default:
		ParseError(Location, ERR_BAD_CHARACTER, "Unknown punctuation \'%c\'", Chr);
		Token = TK_NoToken;
	}
}

//==========================================================================
//
//	VLexer::ProcessFileName
//
//==========================================================================

void VLexer::ProcessFileName()
{
	int len = 0;
	NextChr();
	while (Chr != '\"')
	{
		if (len >= MAX_QUOTED_LENGTH - 1)
		{
			ParseError(Location, ERR_STRING_TOO_LONG);
			NextChr();
			continue;
		}
		if (Chr == EOF_CHARACTER)
		{
			ParseError(Location, ERR_EOF_IN_STRING);
			break;
		}
		if (IncLineNumber)
		{
			ParseError(Location, ERR_NEW_LINE_INSIDE_QUOTE);
		}
		TokenStringBuffer[len] = Chr;
		NextChr();
		len++;
	}
	TokenStringBuffer[len] = 0;
	NextChr();
}

//==========================================================================
//
//	VLexer::AddSourceFile
//
//==========================================================================

int VLexer::AddSourceFile(const char* SName)
{
	//	Find it.
	for (int i = 0; i < TLocation::SourceFiles.Num(); i++)
		if (!strcmp(SName, TLocation::SourceFiles[i]))
			return i;

	//	Not found, add it.
	char* NewName = new char[strlen(SName) + 1];
	strcpy(NewName, SName);
	return TLocation::SourceFiles.Append(NewName);
}

//==========================================================================
//
//	VLexer::NextToken
//
//==========================================================================

void VLexer::NextToken()
{
	do
	{
		TokenStringBuffer[0] = 0;
		while (Chr == ' ') NextChr();
		if (NewLine)
		{
			NewLine = false;
			if (Chr == '#')
			{
				if (!strncmp(FilePtr, "line", 4))
				{
					NextChr();
					NextChr();
					NextChr();
					NextChr();
				}
				NextChr();

				//	Read line number
				while (Chr == ' ') NextChr();
				if (ASCIIToChrCode[(vuint8)Chr] != CHR_Number)
				{
					ParseError(Location, "Bad directive.");
				}
				ProcessNumberToken();
				Line = Number - 1;

				//	Read file name
				while (Chr == ' ') NextChr();
				if (ASCIIToChrCode[(vuint8)Chr] != CHR_Quote)
				{
					ParseError(Location, "Bad directive.");
				}
				ProcessFileName();
				SourceIdx = AddSourceFile(String);
				Location = TLocation(SourceIdx, Line);

				//	Ignore flags
				while (!NewLine)
				{
					NextChr();
				}
				Token = TK_NoToken;
				continue;
			}
		}
		switch (ASCIIToChrCode[(vuint8)Chr])
		{
			case CHR_EOF:
				Token = TK_EOF;
				break;
			case CHR_Letter:
				ProcessLetterToken();
				break;
			case CHR_Number:
				ProcessNumberToken();
				break;
			case CHR_Quote:
				ProcessQuoteToken();
				break;
			case CHR_SingleQuote:
				ProcessSingleQuoteToken();
				break;
			default:
				ProcessSpecialToken();
				break;
		}
	} while (Token == TK_NoToken);
}

//==========================================================================
//
//	VLexer::Check
//
//==========================================================================

bool VLexer::Check(EToken tk)
{
	if (Token == tk)
	{
		NextToken();
		return true;
	}
	return false;
}

//==========================================================================
//
//	VLexer::Expect
//
//	Report error, if current token is not equals to tk.
//	Take next token.
//
//==========================================================================

void VLexer::Expect(EToken tk)
{
	if (Token != tk)
	{
		ParseError(Location, "expected %s, found %s", TokenNames[tk],
			TokenNames[Token]);
	}
	NextToken();
}

//==========================================================================
//
//	VLexer::Expect
//
//	Report error, if current token is not equals to tk.
//	Take next token.
//
//==========================================================================

void VLexer::Expect(EToken tk, ECompileError error)
{
	if (Token != tk)
	{
		ParseError(Location, error, "expected %s, found %s", TokenNames[tk],
			TokenNames[Token]);
	}
	NextToken();
}

//==========================================================================
//
//	TLocation::GetSource
//
//==========================================================================

const char* TLocation::GetSource() const
{
	if (!Loc)
		return "(external)";
	return SourceFiles[Loc >> 16];
}
