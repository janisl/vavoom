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

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define MAX_STRING_SIZE 64
#define ASCII_COMMENT (';')
#define ASCII_QUOTE (34)
#define LUMP_SCRIPT 1
#define FILE_ZONE_SCRIPT 2

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void CheckOpen(void);
static void OpenScript(const char *name, int type);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

char 	*sc_String;
int 	sc_Number;
double 	sc_Float;
int 	sc_Line;
boolean sc_End;
boolean sc_Crossed;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char 	ScriptName[16];
static char 	*ScriptBuffer;
static char 	*ScriptPtr;
static char 	*ScriptEndPtr;
static char 	StringBuffer[MAX_STRING_SIZE];
static boolean 	ScriptOpen = false;
static int 		ScriptSize;
static boolean 	AlreadyGot = false;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	SC_Open
//
//==========================================================================

void SC_Open(const char *name)
{
	char filename[MAX_OSPATH];

	if (fl_devmode && FL_FindFile(va("scripts/%s.txt", name), filename))
	{
		SC_OpenFile(filename);
	}
	else
	{
		SC_OpenLump(name);
	}
}

//==========================================================================
//
// SC_OpenLump
//
// Loads a script (from the WAD files) and prepares it for parsing.
//
//==========================================================================

void SC_OpenLump(const char *name)
{
	OpenScript(name, LUMP_SCRIPT);
}

//==========================================================================
//
// SC_OpenFile
//
// Loads a script (from a file) and prepares it for parsing.  Uses the
// zone memory allocator for memory allocation and de-allocation.
//
//==========================================================================

void SC_OpenFile(const char *name)
{
	OpenScript(name, FILE_ZONE_SCRIPT);
}

//==========================================================================
//
// OpenScript
//
//==========================================================================

static void OpenScript(const char *name, int type)
{
	SC_Close();
	if (type == LUMP_SCRIPT)
	{
		// Lump script
		ScriptBuffer = (char *)W_CacheLumpName(name, PU_HIGH);
		ScriptSize = W_LumpLength(W_GetNumForName(name));
		strcpy(ScriptName, name);
	}
	else if (type == FILE_ZONE_SCRIPT)
	{
		// File script
		ScriptSize = M_ReadFile(name, (byte **)&ScriptBuffer);
		FL_ExtractFileBase(name, ScriptName);
	}
	ScriptPtr = ScriptBuffer;
	ScriptEndPtr = ScriptPtr+ScriptSize;
	sc_Line = 1;
	sc_End = false;
	ScriptOpen = true;
	sc_String = StringBuffer;
	AlreadyGot = false;
}

//==========================================================================
//
// SC_Close
//
//==========================================================================

void SC_Close(void)
{
	if (ScriptOpen)
	{
		Z_Free(ScriptBuffer);
		ScriptOpen = false;
	}
}

//==========================================================================
//
// SC_GetString
//
//==========================================================================

boolean SC_GetString(void)
{
	char 	*text;
	boolean foundToken;

	CheckOpen();
	if (AlreadyGot)
	{
		AlreadyGot = false;
		return true;
	}
	foundToken = false;
	sc_Crossed = false;
	if (ScriptPtr >= ScriptEndPtr)
	{
		sc_End = true;
		return false;
	}
	while (foundToken == false)
	{
		while (*ScriptPtr <= 32)
		{
			if (ScriptPtr >= ScriptEndPtr)
			{
				sc_End = true;
				return false;
			}
			if (*ScriptPtr++ == '\n')
			{
				sc_Line++;
				sc_Crossed = true;
			}
		}
		if (ScriptPtr >= ScriptEndPtr)
		{
			sc_End = true;
			return false;
		}
		if (*ScriptPtr == ASCII_COMMENT ||
			(ScriptPtr[0] == '/' && ScriptPtr[1] == '/'))
		{
			// Skip comment
			while (*ScriptPtr++ != '\n')
			{
				if (ScriptPtr >= ScriptEndPtr)
				{
					sc_End = true;
					return false;
	
				}
			}
			sc_Line++;
			sc_Crossed = true;
		}
		else
		{
			// Found a token
			foundToken = true;
		}
	}
	text = sc_String;
	if (*ScriptPtr == ASCII_QUOTE)
	{ // Quoted string
		ScriptPtr++;
		while (*ScriptPtr != ASCII_QUOTE)
		{
			*text++ = *ScriptPtr++;
			if(ScriptPtr == ScriptEndPtr
				|| text == &sc_String[MAX_STRING_SIZE-1])
			{
				break;
			}
		}
		ScriptPtr++;
	}
	else
	{
		// Normal string
		while ((*ScriptPtr > 32) && !(*ScriptPtr == ASCII_COMMENT ||
			(ScriptPtr[0] == '/' && ScriptPtr[1] == '/')))
		{
			*text++ = *ScriptPtr++;
			if(ScriptPtr == ScriptEndPtr
				|| text == &sc_String[MAX_STRING_SIZE-1])
			{
				break;
			}
		}
	}
	*text = 0;
	return true;
}

//==========================================================================
//
// SC_MustGetString
//
//==========================================================================

void SC_MustGetString(void)
{
	if (SC_GetString() == false)
	{
		SC_ScriptError("Missing string.");
	}
}

//==========================================================================
//
// SC_MustGetStringName
//
//==========================================================================

void SC_MustGetStringName(const char *name)
{
	SC_MustGetString();
	if (SC_Compare(name) == false)
	{
		SC_ScriptError(NULL);
	}
}

//==========================================================================
//
// SC_GetNumber
//
//==========================================================================

boolean SC_GetNumber(void)
{
	char *stopper;

	CheckOpen();
	if (SC_GetString())
	{
		sc_Number = strtol(sc_String, &stopper, 0);
		if (*stopper != 0)
		{
			Sys_Error("SC_GetNumber: Bad numeric constant \"%s\".\n"
				"Script %s, Line %d", sc_String, ScriptName, sc_Line);
		}
		return true;
	}
	else
	{
		return false;
	}
}

//==========================================================================
//
// SC_MustGetNumber
//
//==========================================================================

void SC_MustGetNumber(void)
{
	if (SC_GetNumber() == false)
	{
		SC_ScriptError("Missing integer.");
	}
}

//==========================================================================
//
//	SC_GetFloat
//
//==========================================================================

boolean SC_GetFloat(void)
{
	char *stopper;

	CheckOpen();
	if (SC_GetString())
	{
		sc_Float = strtod(sc_String, &stopper);
		if (*stopper != 0)
		{
			Sys_Error("SC_GetFloat: Bad floating point constant \"%s\".\n"
				"Script %s, Line %d", sc_String, ScriptName, sc_Line);
		}
		return true;
	}
	else
	{
		return false;
	}
}

//==========================================================================
//
//	SC_MustGetFloat
//
//==========================================================================

void SC_MustGetFloat(void)
{
	if (SC_GetFloat() == false)
	{
		SC_ScriptError("Missing float.");
	}
}

//==========================================================================
//
// SC_UnGet
//
// Assumes there is a valid string in sc_String.
//
//==========================================================================

void SC_UnGet(void)
{
	AlreadyGot = true;
}

//==========================================================================
//
// SC_Check
//
// Returns true if another token is on the current line.
//
//==========================================================================

/*
boolean SC_Check(void)
{
	char *text;

	CheckOpen();
	text = ScriptPtr;
	if(text >= ScriptEndPtr)
	{
		return false;
	}
	while(*text <= 32)
	{
		if(*text == '\n')
		{
			return false;
		}
		text++;
		if(text == ScriptEndPtr)
		{
			return false;
		}
	}
	if(*text == ASCII_COMMENT)
	{
		return false;
	}
	return true;
}
*/

//==========================================================================
//
// SC_MatchString
//
// Returns the index of the first match to sc_String from the passed
// array of strings, or -1 if not found.
//
//==========================================================================

int SC_MatchString(const char **strings)
{
	int i;

	for (i = 0; *strings != NULL; i++)
	{
		if (SC_Compare(*strings++))
		{
			return i;
		}
	}
	return -1;
}

//==========================================================================
//
// SC_MustMatchString
//
//==========================================================================

int SC_MustMatchString(const char **strings)
{
	int i;

	i = SC_MatchString(strings);
	if (i == -1)
	{
		SC_ScriptError(NULL);
	}
	return i;
}

//==========================================================================
//
// SC_Compare
//
//==========================================================================

boolean SC_Compare(const char *text)
{
	return !stricmp(text, sc_String);
}

//==========================================================================
//
// SC_ScriptError
//
//==========================================================================

void SC_ScriptError(const char *message)
{
	if (message == NULL)
	{
		message = "Bad syntax.";
	}
	Sys_Error("Script error, \"%s\" line %d: %s", ScriptName,
		sc_Line, message);
}

//==========================================================================
//
// CheckOpen
//
//==========================================================================

static void CheckOpen(void)
{
	if (ScriptOpen == false)
	{
		Sys_Error("SC_ call before SC_Open().");
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//
//	Revision 1.5  2001/10/12 17:31:13  dj_jl
//	no message
//	
//	Revision 1.4  2001/08/04 17:27:39  dj_jl
//	Added consts to script functions
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
