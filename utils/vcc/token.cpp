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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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

#define EOF_CHARACTER		127
#define NON_HEX_DIGIT		255

// TYPES -------------------------------------------------------------------

enum chr_t
{
	CHR_EOF,
	CHR_LETTER,
	CHR_NUMBER,
	CHR_QUOTE,
	CHR_SINGLE_QUOTE,
	CHR_SPECIAL
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void NextChr(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

char				tk_SourceName[MAX_FILE_NAME_LENGTH];
int 				tk_Line;

tokenType_t 		tk_Token;
char*				tk_String;
int					tk_StringI;
int 				tk_Number;
float				tk_Float;
Keyword				tk_Keyword;
Punctuation			tk_Punct;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char			ASCIIToChrCode[256];
static byte			ASCIIToHexDigit[256];
static char			TokenStringBuffer[MAX_QUOTED_LENGTH];
static boolean		IncLineNumber;
static boolean		NewLine;
static boolean		SourceOpen;
static char*		FileStart;
static char*		FilePtr;
static char*		FileEnd;
static char			Chr;

static char* Keywords[] =
{
	"",
	"__states__",
	"__mobjinfo__",
	"addfields",
	"break",
	"case",
	"class",
	"classid",
	"continue",
	"default",
	"defaultproperties",
	"do",
	"else",
	"enum",
	"float",
	"for",
	"function_t",
	"if",
   	"int",
	"native",
	"none",
	"NULL",
	"return",
	"self",
	"string",
	"struct",
	"switch",
	"this",
	"typedef",
   	"uint",
	"vector",
   	"void",
	"while",
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// TK_Init
//
//==========================================================================

void TK_Init(void)
{
	int i;

	for (i = 0; i < 256; i++)
	{
		ASCIIToChrCode[i] = CHR_SPECIAL;
		ASCIIToHexDigit[i] = NON_HEX_DIGIT;
	}
	for (i = '0'; i <= '9'; i++)
	{
		ASCIIToChrCode[i] = CHR_NUMBER;
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
		ASCIIToChrCode[i] = CHR_LETTER;
	}
	for (i = 'a'; i <= 'z'; i++)
	{
		ASCIIToChrCode[i] = CHR_LETTER;
	}
	ASCIIToChrCode['\"'] = CHR_QUOTE;
	ASCIIToChrCode['\''] = CHR_SINGLE_QUOTE;
	ASCIIToChrCode['_'] = CHR_LETTER;
	ASCIIToChrCode[0] = CHR_EOF;
	ASCIIToChrCode[EOF_CHARACTER] = CHR_EOF;
	tk_String = TokenStringBuffer;
	IncLineNumber = false;
	SourceOpen = false;
}

//==========================================================================
//
// TK_OpenSource
//
//==========================================================================

void TK_OpenSource(void *buf, size_t size)
{
	//	PrincipÆ nav vajadzØgs aizvñrt
	TK_CloseSource();

	//	IelÆdñ failu un sagatavojas kompilÆcijai
	SourceOpen = true;
	FileStart = (char *)buf;
	FileEnd = FileStart + size;
	FilePtr = FileStart;
	tk_Line = 1;
	tk_Token = TK_NONE;
	NewLine = true;
	NextChr();
}

//==========================================================================
//
// TK_CloseSource
//
//==========================================================================

void TK_CloseSource(void)
{
	if (SourceOpen)
	{
		Free(FileStart);
		SourceOpen = false;
	}
}

//==========================================================================
//
// NextChr
//
//==========================================================================

static void NextChr(void)
{
	if (FilePtr >= FileEnd)
	{
		Chr = EOF_CHARACTER;
		return;
	}
	if (IncLineNumber)
	{
		tk_Line++;
		IncLineNumber = false;
	}
	Chr = *FilePtr++;
	if ((byte)Chr < ' ')
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
// ProcessNumberToken
//
//==========================================================================

static void ProcessNumberToken(void)
{
	char c;

	tk_Token = TK_INTEGER;
	c = Chr;
	NextChr();
	tk_Number = c - '0';
	if (c == '0' && (Chr == 'x' || Chr == 'X'))
	{
		//  HeksadecimÆla konstante
		NextChr();
		while (ASCIIToHexDigit[(byte)Chr] != NON_HEX_DIGIT)
		{
			tk_Number = (tk_Number << 4) + ASCIIToHexDigit[(byte)Chr];
			NextChr();
		}
		return;
	}
	while (ASCIIToChrCode[(byte)Chr] == CHR_NUMBER)
	{
		tk_Number = 10 * tk_Number + (Chr - '0');
		NextChr();
	}
	if (Chr == '.')
	{
		tk_Token = TK_FLOAT;
		NextChr(); // Punkts
		tk_Float = tk_Number;
		float	fmul = 0.1;
		while (ASCIIToChrCode[(byte)Chr] == CHR_NUMBER)
		{
			tk_Float += (Chr - '0') * fmul;
			fmul /= 10.0;
			NextChr();
		}
		return;
	}
	if (Chr == '_')
	{
		int radix;
		int digit;

		NextChr(); // PasvØtroýanas zØmi
		radix = tk_Number;
		if (radix < 2 || radix > 36)
		{
			ERR_Exit(ERR_BAD_RADIX_CONSTANT, true, NULL);
		}
		tk_Number = 0;
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
				tk_Number = radix * tk_Number + digit;
				NextChr();
			}
		} while (digit != -1);
	}
}

//==========================================================================
//
//	ProcessChar
//
//==========================================================================

static void ProcessChar(void)
{
	if (Chr == EOF_CHARACTER)
	{
		ERR_Exit(ERR_EOF_IN_STRING, true, NULL);
	}
	if (IncLineNumber)
	{
		ERR_Exit(ERR_NEW_LINE_INSIDE_QUOTE, true, NULL);
	}
	if (Chr == '\\')
	{
		// speciÆlais simbols
		NextChr();
		if (Chr == EOF_CHARACTER)
		{
			ERR_Exit(ERR_EOF_IN_STRING, true, NULL);
		}
		if (IncLineNumber)
	   	{
			ERR_Exit(ERR_NEW_LINE_INSIDE_QUOTE, true, NULL);
		}
		if (Chr == 'n')
			Chr = '\n';
		else if (Chr == '"')
			Chr = '"';
		else if (Chr == 't')
			Chr = '\t';
		else if (Chr == '\\')
			Chr = '\\';
		else
			ERR_Exit(ERR_UNKNOWN_ESC_CHAR, true, NULL);
	}
}

//==========================================================================
//
// ProcessQuoteToken
//
//==========================================================================

static void ProcessQuoteToken(void)
{
	int len;

	tk_Token = TK_STRING;
	len = 0;
	NextChr();
	while (Chr != '\"')
	{
		if (len >= MAX_QUOTED_LENGTH-1)
		{
			ERR_Exit(ERR_STRING_TOO_LONG, true, NULL);
		}
		ProcessChar();
		TokenStringBuffer[len] = Chr;
		NextChr();
		len++;
	}
	TokenStringBuffer[len] = 0;
	NextChr();
	tk_StringI = FindString(tk_String);
}

//==========================================================================
//
// ProcessLetterToken
//
//==========================================================================

static void ProcessLetterToken(void)
{
	int		len;

	tk_Token = TK_IDENTIFIER;
	len = 0;
	while (ASCIIToChrCode[(byte)Chr] == CHR_LETTER
		|| ASCIIToChrCode[(byte)Chr] == CHR_NUMBER)
	{
		if (len == MAX_IDENTIFIER_LENGTH - 1)
		{
			ERR_Exit(ERR_IDENTIFIER_TOO_LONG, true, NULL);
		}
		TokenStringBuffer[len] = Chr;
		len++;
		NextChr();
	}
	TokenStringBuffer[len] = 0;

	switch (TokenStringBuffer[0])
	{
	case '_':
		if (tk_String[1] == '_')
		{
			if (tk_String[2] == 's' && tk_String[3] == 't' &&
				tk_String[4] == 'a' && tk_String[5] == 't' &&
				tk_String[6] == 'e' && tk_String[7] == 's' &&
				tk_String[8] == '_' && tk_String[9] == '_' &&
				tk_String[10] == 0)
			{
				tk_Token = TK_KEYWORD;
				tk_Keyword = KW_STATES;
			}
			else if (tk_String[2] == 'm' && tk_String[3] == 'o' &&
				tk_String[4] == 'b' && tk_String[5] == 'j' &&
				tk_String[6] == 'i' && tk_String[7] == 'n' &&
				tk_String[8] == 'f' && tk_String[9] == 'o' &&
				tk_String[10] == '_' && tk_String[11] == '_' &&
				tk_String[12] == 0)
			{
				tk_Token = TK_KEYWORD;
				tk_Keyword = KW_MOBJINFO;
			}
		}
		break;

	case 'a':
		if (!strcmp(tk_String, "addfields"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_ADDFIELDS;
		}
		break;

	case 'b':
		if (!strcmp(tk_String, "break"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_BREAK;
		}
		break;

	case 'c':
		if (!strcmp(tk_String, "case"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_CASE;
		}
		else if (!strcmp(tk_String, "class"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_CLASS;
		}
		else if (!strcmp(tk_String, "classid"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_CLASSID;
		}
		else if (!strcmp(tk_String, "continue"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_CONTINUE;
		}
		break;

	case 'd':
		if (!strcmp(tk_String, "default"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_DEFAULT;
		}
		else if (!strcmp(tk_String, "defaultproperties"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_DEFAULTPROPERTIES;
		}
		else if (!strcmp(tk_String, "do"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_DO;
		}
		break;

	case 'e':
		if (!strcmp(tk_String, "else"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_ELSE;
		}
		else if (!strcmp(tk_String, "enum"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_ENUM;
		}
		break;

	case 'f':
		if (!strcmp(tk_String, "float"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_FLOAT;
		}
		else if (!strcmp(tk_String, "for"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_FOR;
		}
		else if (!strcmp(tk_String, "function_t"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_FUNCTION;
		}
		break;

	case 'i':
		if (!strcmp(tk_String, "if"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_IF;
		}
		else if (!strcmp(tk_String, "int"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_INT;
		}
		break;

	case 'n':
		if (tk_String[1] == 'o' && tk_String[2] == 'n' &&
			tk_String[3] == 'e' && tk_String[4] == 0)
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_NONE;
		}
		else if (!strcmp(tk_String, "native"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_NATIVE;
		}
		break;

/*	case 'N':
		if (tk_String[1] == 'U' && tk_String[2] == 'L' &&
			tk_String[3] == 'L' && tk_String[4] == 0)
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_NULL;
		}
		break;*/

	case 'r':
		if (!strcmp(tk_String, "return"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_RETURN;
		}
		break;

	case 's':
		if (!strcmp(tk_String, "self"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_SELF;
		}
		else if (!strcmp(tk_String, "string"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_STRING;
		}
		else if (!strcmp(tk_String, "struct"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_STRUCT;
		}
		else if (!strcmp(tk_String, "switch"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_SWITCH;
		}
		break;

	case 't':
		if (!strcmp(tk_String, "this"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_THIS;
		}
		else if (!strcmp(tk_String, "typedef"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_TYPEDEF;
		}
		break;

	case 'u':
		if (!strcmp(tk_String, "uint"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_UINT;
		}
		break;

	case 'v':
		if (!strcmp(tk_String, "vector"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_VECTOR;
		}
		else if (!strcmp(tk_String, "void"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_VOID;
		}
		break;

	case 'w':
		if (!strcmp(tk_String, "while"))
		{
			tk_Token = TK_KEYWORD;
			tk_Keyword = KW_WHILE;
		}
		break;
	}
	if (tk_String[0] == 'N' && tk_String[1] == 'U' && tk_String[2] == 'L' &&
		tk_String[3] == 'L' && tk_String[4] == 0)
	{
		tk_Token = TK_KEYWORD;
		tk_Keyword = KW_NULL;
	}

	if (tk_Token == TK_IDENTIFIER)
	{
		tk_StringI = FindString(tk_String);
	}
}

//==========================================================================
//
//	ProcessSpecialToken
//
//==========================================================================

static void ProcessSpecialToken(void)
{
	char c = Chr;
   	NextChr();
	tk_Token = TK_PUNCT;
	switch (c)
	{
	case '+':
		if (Chr == '=')
		{
			tk_Punct = PU_ADD_ASSIGN;
			NextChr();
		}
		else if (Chr == '+')
		{
			tk_Punct = PU_INC;
			NextChr();
		}
		else
		{
			tk_Punct = PU_PLUS;
		}
		break;

	case '-':
		if (Chr == '=')
		{
			tk_Punct = PU_MINUS_ASSIGN;
			NextChr();
		}
		else if (Chr == '-')
		{
			tk_Punct = PU_DEC;
			NextChr();
		}
		else if (Chr == '>')
		{
			tk_Punct = PU_MINUS_GT;
			NextChr();
		}
		else
		{
			tk_Punct = PU_MINUS;
		}
		break;

	case '*':
		if (Chr == '=')
		{
			tk_Punct = PU_MULTIPLY_ASSIGN;
			NextChr();
		}
		else
		{
			tk_Punct = PU_ASTERISK;
		}
		break;

	case '/':
		if (Chr == '=')
		{
			tk_Punct = PU_DIVIDE_ASSIGN;
			NextChr();
		}
		else
		{
			tk_Punct = PU_SLASH;
		}
		break;

	case '%':
		if (Chr == '=')
		{
			tk_Punct = PU_MOD_ASSIGN;
			NextChr();
		}
		else
		{
			tk_Punct = PU_PERCENT;
		}
		break;

	case '=':
		if (Chr == '=')
		{
			tk_Punct = PU_EQ;
			NextChr();
		}
		else
		{
			tk_Punct = PU_ASSIGN;
		}
		break;

	case '<':
		if (Chr == '<')
		{
			NextChr();
			if (Chr == '=')
			{
				tk_Punct = PU_LSHIFT_ASSIGN;
				NextChr();
			}
			else
			{
				tk_Punct = PU_LSHIFT;
			}
		}
		else if (Chr == '=')
		{
			tk_Punct = PU_LE;
			NextChr();
		}
		else
		{
			tk_Punct = PU_LT;
		}
		break;

	case '>':
		if (Chr == '>')
		{
			NextChr();
			if (Chr == '=')
			{
				tk_Punct = PU_RSHIFT_ASSIGN;
				NextChr();
			}
			else
			{
				tk_Punct = PU_RSHIFT;
			}
		}
		else if (Chr == '=')
		{
			tk_Punct = PU_GE;
			NextChr();
		}
		else
		{
			tk_Punct = PU_GT;
		}
		break;

	case '!':
		if (Chr == '=')
		{
			tk_Punct = PU_NE;
			NextChr();
		}
		else
		{
			tk_Punct = PU_NOT;
		}
		break;

	case '&':
		if (Chr == '=')
		{
			tk_Punct = PU_AND_ASSIGN;
			NextChr();
		}
		else if (Chr == '&')
		{
			tk_Punct = PU_AND_LOG;
			NextChr();
		}
		else
		{
			tk_Punct = PU_AND;
		}
		break;

	case '|':
		if (Chr == '=')
		{
			tk_Punct = PU_OR_ASSIGN;
			NextChr();
		}
		else if (Chr == '|')
		{
			tk_Punct = PU_OR_LOG;
			NextChr();
		}
		else
		{
			tk_Punct = PU_OR;
		}
		break;

	case '^':
		if (Chr == '=')
		{
			tk_Punct = PU_XOR_ASSIGN;
			NextChr();
		}
		else
		{
			tk_Punct = PU_XOR;
		}
		break;

	case '.':
		if (Chr == '.' && FilePtr[0] == '.')
		{
			tk_Punct = PU_VARARGS;
			NextChr();
			NextChr();
		}
		else
		{
			tk_Punct = PU_DOT;
		}
		break;

	case ':':
		if (Chr == ':')
		{
			tk_Punct = PU_DCOLON;
			NextChr();
		}
		else
		{
			tk_Punct = PU_COLON;
		}
		break;

	case '(':
		tk_Punct = PU_LPAREN;
		break;

	case ')':
		tk_Punct = PU_RPAREN;
		break;

	case '?':
		tk_Punct = PU_QUEST;
		break;

	case '~':
		tk_Punct = PU_TILDE;
		break;

	case ',':
		tk_Punct = PU_COMMA;
		break;

	case ';':
		tk_Punct = PU_SEMICOLON;
		break;

	case '[':
		tk_Punct = PU_LINDEX;
		break;

	case ']':
		tk_Punct = PU_RINDEX;
		break;

	case '{':
		tk_Punct = PU_LBRACE;
		break;

	case '}':
		tk_Punct = PU_RBRACE;
		break;

	default:
		ERR_Exit(ERR_BAD_CHARACTER, true, "Unknown punctuation \'%c\'", Chr);
	}
}

//==========================================================================
//
// TK_NextToken
//
//==========================================================================

void TK_NextToken(void)
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

				//	Nolasa lØnijas numuru
				while (Chr == ' ') NextChr();
				if (ASCIIToChrCode[(byte)Chr] != CHR_NUMBER)
				{
					ERR_Exit(ERR_NONE, false, "Bad directive.");
				}
				ProcessNumberToken();
				tk_Line = tk_Number - 1;

				//	Nolasa faila vÆrdu
				while (Chr == ' ') NextChr();
				if (ASCIIToChrCode[(byte)Chr] != CHR_QUOTE)
				{
					ERR_Exit(ERR_NONE, false, "Bad directive.");
				}
				ProcessQuoteToken();
				strcpy(tk_SourceName, tk_String);

				//	Karodzi·us ignorñ
				while (!NewLine)
				{
					NextChr();
				}
				tk_Token = TK_NONE;
				continue;
			}
		}
		switch (ASCIIToChrCode[(byte)Chr])
		{
			case CHR_EOF:
				tk_Token = TK_EOF;
				break;
			case CHR_LETTER:
				ProcessLetterToken();
				break;
			case CHR_NUMBER:
				ProcessNumberToken();
				break;
			case CHR_QUOTE:
				ProcessQuoteToken();
				break;
//			case CHR_SINGLE_QUOTE:
//				ProcessSingleQuoteToken();
//				break;
			default:
				ProcessSpecialToken();
				break;
		}
	} while (tk_Token == TK_NONE);
}

//==========================================================================
//
//  TK_Check
//
//	Atgrie÷ true un pa·em nÆkoýo gabalu ja tekoýais sakrØt at string
//  Atgrie÷ false un neko nedara citos gadØjumos
//
//==========================================================================

bool TK_Check(const char *string)
{
	if (tk_Token != TK_IDENTIFIER && tk_Token != TK_KEYWORD &&
		tk_Token != TK_PUNCT)
	{
		return false;
	}
	if (strcmp(string, TokenStringBuffer))
	{
		return false;
	}

	TK_NextToken();
	return true;
}

//==========================================================================
//
//	TK_Expect
//
//  Izdod kõÝdu, ja tekoýais gabals nesakrØt ar string.
//  Pa·em nÆkoýo gabalu
//
//==========================================================================

void TK_Expect(const char *string, error_t error)
{
	if (tk_Token != TK_IDENTIFIER && tk_Token != TK_KEYWORD &&
		tk_Token != TK_PUNCT)
	{
		ERR_Exit(error, true, "invalid token type");
	}
	if (strcmp(string, TokenStringBuffer))
	{
		ERR_Exit(error, true, "expected %s, found %s", string, TokenStringBuffer);
	}
	TK_NextToken();
}

//==========================================================================
//
//	TK_Expect
//
//  Izdod kõÝdu, ja tekoýais gabals nesakrØt ar string.
//  Pa·em nÆkoýo gabalu
//
//==========================================================================

void TK_Expect(Keyword kwd, error_t error)
{
	if (tk_Token != TK_KEYWORD)
	{
		ERR_Exit(error, true, "invalid token type");
	}
	if (tk_Keyword != kwd)
	{
		ERR_Exit(error, true, "expected %s, found %s", Keywords[kwd], TokenStringBuffer);
	}
	TK_NextToken();
}

//==========================================================================
//
//	TK_Expect
//
//  Izdod kõÝdu, ja tekoýais gabals nesakrØt ar string.
//  Pa·em nÆkoýo gabalu
//
//==========================================================================

void TK_Expect(Punctuation punct, error_t error)
{
	if (tk_Token != TK_PUNCT || tk_Punct != punct)
	{
		ERR_Exit(error, true, NULL);
	}
	TK_NextToken();
}

//**************************************************************************
//
//	$Log$
//	Revision 1.11  2001/12/18 19:09:41  dj_jl
//	Some extra info in progs and other small changes
//
//	Revision 1.10  2001/12/12 19:22:22  dj_jl
//	Support for method usage as state functions, dynamic cast
//	Added dynamic arrays
//	
//	Revision 1.9  2001/12/04 18:19:03  dj_jl
//	Escape character for \ symbol
//	
//	Revision 1.8  2001/12/03 19:25:44  dj_jl
//	Fixed calling of parent function
//	Added defaultproperties
//	Fixed vectors as arguments to methods
//	
//	Revision 1.7  2001/12/01 18:17:09  dj_jl
//	Fixed calling of parent method, speedup
//	
//	Revision 1.6  2001/11/09 14:42:29  dj_jl
//	References, beautification
//	
//	Revision 1.5  2001/10/02 17:44:52  dj_jl
//	Some optimizations
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
