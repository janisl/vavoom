
//**************************************************************************
//**
//** token.c
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#ifdef __NeXT__
#include <libc.h>
#else
#ifndef __linux__
#include <io.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#endif
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "token.h"
#include "error.h"
#include "misc.h"
#include "symbol.h"

// MACROS ------------------------------------------------------------------

#define NON_HEX_DIGIT 255
#define MAX_NESTED_SOURCES 16

// TYPES -------------------------------------------------------------------

typedef enum
{
	CHR_EOF,
	CHR_LETTER,
	CHR_NUMBER,
	CHR_QUOTE,
	CHR_SPECIAL
} chr_t;

typedef struct
{
	char name[MAX_FILE_NAME_LENGTH];
	char *start;
	char *end;
	char *position;
	int line;
	boolean incLineNumber;
	char lastChar;
} nestInfo_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void MakeIncludePath(char *sourceName);
static void PopNestedSource(void);
static void ProcessLetterToken(void);
static void ProcessNumberToken(void);
static void EvalFixedConstant(int whole);
static void EvalHexConstant(void);
static void EvalRadixConstant(void);
static int DigitValue(char digit, int radix);
static void ProcessQuoteToken(void);
static void ProcessSpecialToken(void);
static boolean CheckForKeyword(void);
static boolean CheckForLineSpecial(void);
static boolean CheckForConstant(void);
static void NextChr(void);
static void SkipComment(void);
static void SkipCPPComment(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

tokenType_t tk_Token;
int tk_Line;
int tk_Number;
char *tk_String;
int tk_SpecialValue;
int tk_SpecialArgCount;
char tk_SourceName[MAX_FILE_NAME_LENGTH];
int tk_IncludedLines;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char Chr;
static char *FileStart;
static char *FilePtr;
static char *FileEnd;
static boolean SourceOpen;
static char ASCIIToChrCode[256];
static byte ASCIIToHexDigit[256];
static char TokenStringBuffer[MAX_QUOTED_LENGTH];
static nestInfo_t OpenFiles[MAX_NESTED_SOURCES];
static boolean AlreadyGot;
static int NestDepth;
static boolean IncLineNumber;
static char IncludePath[MAX_FILE_NAME_LENGTH];

static struct
{
	char *name;
	tokenType_t token;
} Keywords[] =
{
	"break", TK_BREAK,
	"case", TK_CASE,
	"const", TK_CONST,
	"continue", TK_CONTINUE,
	"default", TK_DEFAULT,
	"define", TK_DEFINE,
	"do", TK_DO,
	"else", TK_ELSE,
	"for", TK_FOR,
	"goto", TK_GOTO,
	"if", TK_IF,
	"include", TK_INCLUDE,
	"int", TK_INT,
	"open", TK_OPEN,
	"print", TK_PRINT,
	"printbold", TK_PRINTBOLD,
	"restart", TK_RESTART,
	"script", TK_SCRIPT,
	"special", TK_SPECIAL,
	"str", TK_STR,
	"suspend", TK_SUSPEND,
	"switch", TK_SWITCH,
	"terminate", TK_TERMINATE,
	"until", TK_UNTIL,
	"void", TK_VOID,
	"while", TK_WHILE,
	"world", TK_WORLD,
	NULL, -1
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

	for(i = 0; i < 256; i++)
	{
		ASCIIToChrCode[i] = CHR_SPECIAL;
		ASCIIToHexDigit[i] = NON_HEX_DIGIT;
	}
	for(i = '0'; i <= '9'; i++)
	{
		ASCIIToChrCode[i] = CHR_NUMBER;
		ASCIIToHexDigit[i] = i-'0';
	}
	for(i = 'A'; i <= 'F'; i++)
	{
		ASCIIToHexDigit[i] = 10+(i-'A');
	}
	for(i = 'a'; i <= 'f'; i++)
	{
		ASCIIToHexDigit[i] = 10+(i-'a');
	}
	for(i = 'A'; i <= 'Z'; i++)
	{
		ASCIIToChrCode[i] = CHR_LETTER;
	}
	for(i = 'a'; i <= 'z'; i++)
	{
		ASCIIToChrCode[i] = CHR_LETTER;
	}
	ASCIIToChrCode[ASCII_QUOTE] = CHR_QUOTE;
	ASCIIToChrCode[ASCII_UNDERSCORE] = CHR_LETTER;
	ASCIIToChrCode[EOF_CHARACTER] = CHR_EOF;
	tk_String = TokenStringBuffer;
	IncLineNumber = FALSE;
	tk_IncludedLines = 0;
	SourceOpen = FALSE;
}

//==========================================================================
//
// TK_OpenSource
//
//==========================================================================

void TK_OpenSource(char *fileName)
{
	int size;

	TK_CloseSource();
	size = MS_LoadFile(fileName, (void **)&FileStart);
	strcpy(tk_SourceName, fileName);
	MakeIncludePath(fileName);
	SourceOpen = TRUE;
	FileEnd = FileStart+size;
	FilePtr = FileStart;
	tk_Line = 1;
	tk_Token = TK_NONE;
	AlreadyGot = FALSE;
	NestDepth = 0;
	NextChr();
}

//==========================================================================
//
// MakeIncludePath
//
//==========================================================================

static void MakeIncludePath(char *sourceName)
{
	strcpy(IncludePath, sourceName);
	if(MS_StripFilename(IncludePath) == NO)
	{
		IncludePath[0] = 0;
	}
	else
	{ // Add a directory delimiter to the include path
		strcat(IncludePath, DIRECTORY_DELIMITER);
	}
}

//==========================================================================
//
// TK_Include
//
//==========================================================================

void TK_Include(char *fileName)
{
	int size;
	nestInfo_t *info;

	if(NestDepth == MAX_NESTED_SOURCES)
	{
		ERR_Exit(ERR_INCL_NESTING_TOO_DEEP, YES,
			"Unable to include file \"%s\".", fileName);
	}
	info = &OpenFiles[NestDepth++];
	strcpy(info->name, tk_SourceName);
	info->start = FileStart;
	info->end = FileEnd;
	info->position = FilePtr;
	info->line = tk_Line;
	info->incLineNumber = IncLineNumber;
	info->lastChar = Chr;
	strcpy(tk_SourceName, IncludePath);
	strcat(tk_SourceName, fileName);
	size = MS_LoadFile(tk_SourceName, (void **)&FileStart);
	FileEnd = FileStart+size;
	FilePtr = FileStart;
	tk_Line = 1;
	IncLineNumber = FALSE;
	tk_Token = TK_NONE;
	AlreadyGot = FALSE;
	NextChr();
}

//==========================================================================
//
// PopNestedSource
//
//==========================================================================

static void PopNestedSource(void)
{
	nestInfo_t *info;

	free(FileStart);
	tk_IncludedLines += tk_Line;
	info = &OpenFiles[--NestDepth];
	strcpy(tk_SourceName, info->name);
	FileStart = info->start;
	FileEnd = info->end;
	FilePtr = info->position;
	tk_Line = info->line;
	IncLineNumber = info->incLineNumber;
	Chr = info->lastChar;
	tk_Token = TK_NONE;
	AlreadyGot = FALSE;
}

//==========================================================================
//
// TK_CloseSource
//
//==========================================================================

void TK_CloseSource(void)
{
	int i;

	if(SourceOpen)
	{
		free(FileStart);
		for(i = 0; i < NestDepth; i++)
		{
			free(OpenFiles[i].start);
		}
		SourceOpen = FALSE;
	}
}

//==========================================================================
//
// TK_NextToken
//
//==========================================================================

tokenType_t TK_NextToken(void)
{
	boolean validToken;

	if(AlreadyGot == TRUE)
	{
		AlreadyGot = FALSE;
		return tk_Token;
	}
	validToken = NO;
	do
	{
		while(Chr == ASCII_SPACE)
		{
			NextChr();
		}
		switch(ASCIIToChrCode[(byte)Chr])
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
			default:
				ProcessSpecialToken();
				break;
		}
		if(tk_Token == TK_STARTCOMMENT)
		{
			SkipComment();
		}
		else if(tk_Token == TK_CPPCOMMENT)
		{
			SkipCPPComment();
		}
		else if((tk_Token == TK_EOF) && (NestDepth > 0))
		{
			PopNestedSource();
		}
		else
		{
			validToken = YES;
		}
	} while(validToken == NO);
	return tk_Token;
}

//==========================================================================
//
// TK_NextCharacter
//
//==========================================================================

int TK_NextCharacter(void)
{
	int c;

	while(Chr == ASCII_SPACE)
	{
		NextChr();
	}
	c = (int)Chr;
	if(c == EOF_CHARACTER)
	{
		c = -1;
	}
	NextChr();
	return c;
}

//==========================================================================
//
// TK_NextTokenMustBe
//
//==========================================================================

void TK_NextTokenMustBe(tokenType_t token, error_t error)
{
	if(TK_NextToken() != token)
	{
		ERR_Exit(error, YES, NULL);
	}
}

//==========================================================================
//
// TK_TokenMustBe
//
//==========================================================================

void TK_TokenMustBe(tokenType_t token, error_t error)
{
	if(tk_Token != token)
	{
		ERR_Exit(error, YES, NULL);
	}
}

//==========================================================================
//
// TK_Member
//
//==========================================================================

boolean TK_Member(tokenType_t *list)
{
	int i;

	for(i = 0; list[i] != TK_NONE; i++)
	{
		if(tk_Token == list[i])
		{
			return YES;
		}
	}
	return NO;
}

//==========================================================================
//
// TK_Undo
//
//==========================================================================

void TK_Undo(void)
{
	if(tk_Token != TK_NONE)
	{
		AlreadyGot = TRUE;
	}
}

//==========================================================================
//
// ProcessLetterToken
//
//==========================================================================

static void ProcessLetterToken(void)
{
	int i;
	char *text;

	i = 0;
	text = TokenStringBuffer;
	while(ASCIIToChrCode[(byte)Chr] == CHR_LETTER
		|| ASCIIToChrCode[(byte)Chr] == CHR_NUMBER)
	{
		if(++i == MAX_IDENTIFIER_LENGTH)
		{
			ERR_Exit(ERR_IDENTIFIER_TOO_LONG, YES, NULL);
		}
		*text++ = Chr;
		NextChr();
	}
	*text = 0;
	MS_StrLwr(TokenStringBuffer);
	if(CheckForKeyword() == FALSE
		&& CheckForLineSpecial() == FALSE
		&& CheckForConstant() == FALSE)
	{
		tk_Token = TK_IDENTIFIER;
	}
}

//==========================================================================
//
// CheckForKeyword
//
//==========================================================================

static boolean CheckForKeyword(void)
{
	int i;

	for(i = 0; Keywords[i].name != NULL; i++)
	{
		if(strcmp(tk_String, Keywords[i].name) == 0)
		{
			tk_Token = Keywords[i].token;
			return TRUE;
		}
	}
	return FALSE;
}

//==========================================================================
//
// CheckForLineSpecial
//
//==========================================================================

static boolean CheckForLineSpecial(void)
{
	symbolNode_t *sym;

	sym = SY_FindGlobal(tk_String);
	if(sym == NULL)
	{
		return FALSE;
	}
	if(sym->type != SY_SPECIAL)
	{
		return FALSE;
	}
	tk_Token = TK_LINESPECIAL;
	tk_SpecialValue = sym->info.special.value;
	tk_SpecialArgCount = sym->info.special.argCount;
	return TRUE;
}

//==========================================================================
//
// CheckForConstant
//
//==========================================================================

static boolean CheckForConstant(void)
{
	symbolNode_t *sym;

	sym = SY_FindGlobal(tk_String);
	if(sym == NULL)
	{
		return FALSE;
	}
	if(sym->type != SY_CONSTANT)
	{
		return FALSE;
	}
	tk_Token = TK_NUMBER;
	tk_Number = sym->info.constant.value;
	return TRUE;
}

//==========================================================================
//
// ProcessNumberToken
//
//==========================================================================

static void ProcessNumberToken(void)
{
	char c;

	c = Chr;
	NextChr();
	if(c == '0' && (Chr == 'x' || Chr == 'X'))
	{ // Hexadecimal constant
		NextChr();
		EvalHexConstant();
		return;
	}
	tk_Number = c-'0';
	while(ASCIIToChrCode[(byte)Chr] == CHR_NUMBER)
	{
		tk_Number = 10*tk_Number+(Chr-'0');
		NextChr();
	}
	if(Chr == '.')
	{ // Fixed point
		NextChr(); // Skip period
		EvalFixedConstant(tk_Number);
		return;
	}
	if(Chr == ASCII_UNDERSCORE)
	{
		NextChr(); // Skip underscore
		EvalRadixConstant();
		return;
	}
	tk_Token = TK_NUMBER;
}

//==========================================================================
//
// EvalFixedConstant
//
//==========================================================================

static void EvalFixedConstant(int whole)
{
	int frac;
	int divisor;

	frac = 0;
	divisor = 1;
	while(ASCIIToChrCode[(byte)Chr] == CHR_NUMBER)
	{
		frac = 10*frac+(Chr-'0');
		divisor *= 10;
		NextChr();
	}
	tk_Number = (whole<<16)+((frac<<16)/divisor);
	tk_Token = TK_NUMBER;
}

//==========================================================================
//
// EvalHexConstant
//
//==========================================================================

static void EvalHexConstant(void)
{
	tk_Number = 0;
	while(ASCIIToHexDigit[(byte)Chr] != NON_HEX_DIGIT)
	{
		tk_Number = (tk_Number<<4)+ASCIIToHexDigit[(byte)Chr];
		NextChr();
	}
	tk_Token = TK_NUMBER;
}

//==========================================================================
//
// EvalRadixConstant
//
//==========================================================================

static void EvalRadixConstant(void)
{
	int radix;
	int digitVal;

	radix = tk_Number;
	if(radix < 2 || radix > 36)
	{
		ERR_Exit(ERR_BAD_RADIX_CONSTANT, YES, NULL);
	}
	tk_Number = 0;
	while((digitVal = DigitValue(Chr, radix)) != -1)
	{
		tk_Number = radix*tk_Number+digitVal;
		NextChr();
	}
	tk_Token = TK_NUMBER;
}

//==========================================================================
//
// DigitValue
//
// Returns -1 if the digit is not allowed in the specified radix.
//
//==========================================================================

static int DigitValue(char digit, int radix)
{
	digit = toupper(digit);
	if(digit < '0' || (digit > '9' && digit < 'A') || digit > 'Z')
	{
		return -1;
	}
	if(digit > '9')
	{
		digit = 10+digit-'A';
	}
	else
	{
		digit -= '0';
	}
	if(digit >= radix)
	{
		return -1;
	}
	return digit;
}

//==========================================================================
//
// ProcessQuoteToken
//
//==========================================================================

static void ProcessQuoteToken(void)
{
	int i;
	char *text;

	i = 0;
	text = TokenStringBuffer;
	NextChr();
	while(Chr != EOF_CHARACTER)
	{
		if(Chr == ASCII_QUOTE)
		{
			break;
		}
		if(++i > MAX_QUOTED_LENGTH-1)
		{
			ERR_Exit(ERR_STRING_TOO_LONG, YES, NULL);
		}
		*text++ = Chr;
		NextChr();
	}
	*text = 0;
	if(Chr == ASCII_QUOTE)
	{
		NextChr();
	}
	tk_Token = TK_STRING;
}

//==========================================================================
//
// ProcessSpecialToken
//
//==========================================================================

static void ProcessSpecialToken(void)
{
	char c;

	c = Chr;
	NextChr();
	switch(c)
	{
		case '+':
			switch(Chr)
			{
				case '=':
					tk_Token = TK_ADDASSIGN;
					NextChr();
					break;
				case '+':
					tk_Token = TK_INC;
					NextChr();
					break;
				default:
					tk_Token = TK_PLUS;
					break;
			}
			break;
		case '-':
			switch(Chr)
			{
				case '=':
					tk_Token = TK_SUBASSIGN;
					NextChr();
					break;
				case '-':
					tk_Token = TK_DEC;
					NextChr();
					break;
				default:
					tk_Token = TK_MINUS;
					break;
			}
			break;
		case '*':
			switch(Chr)
			{
				case '=':
					tk_Token = TK_MULASSIGN;
					NextChr();
					break;
				case '/':
					tk_Token = TK_ENDCOMMENT;
					NextChr();
					break;
				default:
					tk_Token = TK_ASTERISK;
					break;
			}
			break;
		case '/':
			switch(Chr)
			{
				case '=':
					tk_Token = TK_DIVASSIGN;
					NextChr();
					break;
				case '/':
					tk_Token = TK_CPPCOMMENT;
					break;
				case '*':
					tk_Token = TK_STARTCOMMENT;
					NextChr();
					break;
				default:
					tk_Token = TK_SLASH;
					break;
			}
			break;
		case '%':
			if(Chr == '=')
			{
				tk_Token = TK_MODASSIGN;
				NextChr();
			}
			else
			{
				tk_Token = TK_PERCENT;
			}
			break;
		case '=':
			if(Chr == '=')
			{
				tk_Token = TK_EQ;
				NextChr();
			}
			else
			{
				tk_Token = TK_ASSIGN;
			}
			break;
		case '<':
			if(Chr == '=')
			{
				tk_Token = TK_LE;
				NextChr();
			}
			else if(Chr == '<')
			{
				tk_Token = TK_LSHIFT;
				NextChr();
			}
			else
			{
				tk_Token = TK_LT;
			}
			break;
		case '>':
			if(Chr == '=')
			{
				tk_Token = TK_GE;
				NextChr();
			}
			else if(Chr == '>')
			{
				tk_Token = TK_RSHIFT;
				NextChr();
			}
			else
			{
				tk_Token = TK_GT;
			}
			break;
		case '!':
			if(Chr == '=')
			{
				tk_Token = TK_NE;
				NextChr();
			}
			else
			{
				tk_Token = TK_NOT;
			}
			break;
		case '&':
			if(Chr == '&')
			{
				tk_Token = TK_ANDLOGICAL;
				NextChr();
			}
			else
			{
				tk_Token = TK_ANDBITWISE;
			}
			break;
		case '|':
			if(Chr == '|')
			{
				tk_Token = TK_ORLOGICAL;
				NextChr();
			}
			else
			{
				tk_Token = TK_ORBITWISE;
			}
			break;
		case '(':
			tk_Token = TK_LPAREN;
			break;
		case ')':
			tk_Token = TK_RPAREN;
			break;
		case '{':
			tk_Token = TK_LBRACE;
			break;
		case '}':
			tk_Token = TK_RBRACE;
			break;
		case '[':
			tk_Token = TK_LBRACKET;
			break;
		case ']':
			tk_Token = TK_RBRACKET;
			break;
		case ':':
			tk_Token = TK_COLON;
			break;
		case ';':
			tk_Token = TK_SEMICOLON;
			break;
		case ',':
			tk_Token = TK_COMMA;
			break;
		case '.':
			tk_Token = TK_PERIOD;
			break;
		case '#':
			tk_Token = TK_NUMBERSIGN;
			break;
		case '^':
			tk_Token = TK_EORBITWISE;
			break;
		case '~':
			tk_Token = TK_TILDE;
			break;
		default:
			ERR_Exit(ERR_BAD_CHARACTER, YES, NULL);
			break;
	}
}

//==========================================================================
//
// NextChr
//
//==========================================================================

static void NextChr(void)
{
	if(FilePtr >= FileEnd)
	{
		Chr = EOF_CHARACTER;
		return;
	}
	if(IncLineNumber == TRUE)
	{
		tk_Line++;
		IncLineNumber = FALSE;
	}
	Chr = *FilePtr++;
	if(Chr < ASCII_SPACE)
	{
		if(Chr == '\n')
		{
			IncLineNumber = TRUE;
		}
		Chr = ASCII_SPACE;
	}
}

//==========================================================================
//
// SkipComment
//
//==========================================================================

void SkipComment(void)
{
	boolean first;

	first = FALSE;
	while(Chr != EOF_CHARACTER)
	{
		if(first == TRUE && Chr == '/')
		{
			break;
		}
		first = (Chr == '*');
		NextChr();
	}
	NextChr();
}

//==========================================================================
//
// SkipCPPComment
//
//==========================================================================

void SkipCPPComment(void)
{
	while(FilePtr < FileEnd)
	{
		if(*FilePtr++ == '\n')
		{
			tk_Line++;
			break;
		}
	}
	NextChr();
}
