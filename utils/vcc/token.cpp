//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	Copyright (C) 1999-2000 J∆nis Legzdi∑˝
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

typedef enum
{
	CHR_EOF,
	CHR_LETTER,
	CHR_NUMBER,
	CHR_QUOTE,
	CHR_SINGLE_QUOTE,
	CHR_SPECIAL
} chr_t;

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
int 				tk_Number;
float				tk_Float;

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
	"__states__",
    "__mobjinfo__",
	"addfields",
	"break",
	"case",
	"continue",
	"default",
	"do",
	"else",
    "enum",
	"extern",
    "float",
	"for",
	"function_t",
	"if",
   	"int",
	"return",
	"string",
    "struct",
	"switch",
	"typedef",
   	"uint",
	"vector",
   	"void",
	"while",
};

static char* Punctuations[] =
{
    //  Gar∆kiem simboliem j∆but pirms ÿs∆kiem
    "...", "<<=", ">>=",

    "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
    "==", "!=", "<=", ">=",
    "&&", "||",
    "<<", ">>",
    "++", "--",
    "->",

    "<", ">" , "?",
    "&" , "|", "^", "~", "!",
    "+", "-", "*", "/", "%", "(", ")",
    ".", ",", ";", ":",
    "=", "[", "]", "{", "}"
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
	for (i = 0; i < 256; i++)
    {
    	ConstLookup[i] = -1;
	}
	numconstants = 0;
}

//==========================================================================
//
// TK_OpenSource
//
//==========================================================================

void TK_OpenSource(char *fileName)
{
	int 	size;

	//	Princip∆ nav vajadzÿgs aizvÒrt
	TK_CloseSource();

    //	Iel∆dÒ failu un sagatavojas kompil∆cijai
	size = LoadFile(fileName, (void **)&FileStart);
	SourceOpen = true;
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
		//  Heksadecim∆la konstante
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

		NextChr(); // Pasvÿtro˝anas zÿmi
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
		// speci∆lais simbols
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
}

//==========================================================================
//
//	ProcessSingleQuoteToken
//
//==========================================================================

static void ProcessSingleQuoteToken(void)
{
	tk_Token = TK_INTEGER;
	NextChr();

	ProcessChar();
	tk_Number = Chr;
	NextChr();

	if (Chr != '\'')
	{
		ParseError("Bad char constant.");
	}
	NextChr();
}

//==========================================================================
//
// ProcessLetterToken
//
//==========================================================================

static void ProcessLetterToken(void)
{
	int 	i;
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

	for (i=0; i < (int)(sizeof(Keywords) / sizeof(Keywords[0])); i++)
    {
    	if (!strcmp(tk_String, Keywords[i]))
		{
        	tk_Token = TK_KEYWORD;
            return;
		}
    }
}

//==========================================================================
//
//	ProcessSpecialToken
//
//==========================================================================

static void ProcessSpecialToken(void)
{
	int		i;
	int		len;

	tk_Token = TK_PUNCT;
	FilePtr--;
	for (i=0 ; i < (int)(sizeof(Punctuations) / sizeof(Punctuations[1])) ; i++)
	{
		len = strlen(Punctuations[i]);
		if (!strncmp(Punctuations[i], FilePtr, len))
		{
			strcpy(TokenStringBuffer, Punctuations[i]);
			FilePtr += len;
		   	NextChr();
			return;
		}
	}
   	FilePtr++;

	ERR_Exit(ERR_BAD_CHARACTER, true, "Unknown punctuation \'%c\'", Chr);
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

                //	Nolasa lÿnijas numuru
				while (Chr == ' ') NextChr();
            	if (ASCIIToChrCode[(byte)Chr] != CHR_NUMBER)
                {
                	ERR_Exit(ERR_NONE, false, "Bad directive.");
                }
                ProcessNumberToken();
                tk_Line = tk_Number - 1;

				//	Nolasa faila v∆rdu
				while (Chr == ' ') NextChr();
            	if (ASCIIToChrCode[(byte)Chr] != CHR_QUOTE)
                {
                	ERR_Exit(ERR_NONE, false, "Bad directive.");
                }
                ProcessQuoteToken();
                strcpy(tk_SourceName, tk_String);

                //	Karodzi∑us ignorÒ
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
			case CHR_SINGLE_QUOTE:
				ProcessSingleQuoteToken();
				break;
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
//	Atgrie˜ true un pa∑em n∆ko˝o gabalu ja teko˝ais sakrÿt at string
//  Atgrie˜ false un neko nedara citos gadÿjumos
//
//==========================================================================

boolean TK_Check(char *string)
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
//  Izdod kı›du, ja teko˝ais gabals nesakrÿt ar string.
//  Pa∑em n∆ko˝o gabalu
//
//==========================================================================

void TK_Expect(char *string, error_t error)
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

