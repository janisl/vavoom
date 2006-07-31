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

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define MAX_STRING_SIZE 64
#define ASCII_COMMENT (';')
#define ASCII_QUOTE (34)
#define LUMP_SCRIPT 1
#define FILE_ZONE_SCRIPT 2
#define LUMP_NUM_SCRIPT 3

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void CheckOpen();
static void OpenScript(const char *name, int LumpNum, int type);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

char 	*sc_String;
int 	sc_Number;
double 	sc_Float;
int 	sc_Line;
boolean sc_End;
boolean sc_Crossed;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VStr		ScriptName;
static char		*ScriptBuffer;
static char		*ScriptPtr;
static char		*ScriptEndPtr;
static char		StringBuffer[MAX_STRING_SIZE];
static bool		ScriptOpen = false;
static int		ScriptSize;
static bool		AlreadyGot = false;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	SC_Open
//
//==========================================================================

void SC_Open(const char *name)
{
	guard(SC_Open);
	VStr filename = FL_FindFile(va("scripts/%s.txt", name));
	if (fl_devmode && filename)
	{
		SC_OpenFile(*filename);
	}
	else
	{
		SC_OpenLump(name);
	}
	unguard;
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
	guard(SC_OpenLump);
	OpenScript(name, -1, LUMP_SCRIPT);
	unguard;
}

//==========================================================================
//
// SC_OpenLumpNum
//
// Loads a script (from the WAD files) and prepares it for parsing.
//
//==========================================================================

void SC_OpenLumpNum(int LumpNum)
{
	guard(SC_OpenLumpNum);
	OpenScript(*W_LumpName(LumpNum), LumpNum, LUMP_NUM_SCRIPT);
	unguard;
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
	guard(SC_OpenFile);
	OpenScript(name, -1, FILE_ZONE_SCRIPT);
	unguard;
}

//==========================================================================
//
// OpenScript
//
//==========================================================================

static void OpenScript(const char *name, int LumpNum, int type)
{
	guard(OpenScript);
	SC_Close();
	if (type == LUMP_SCRIPT)
	{
		// Lump script
		VStream* Strm = W_CreateLumpReaderName(VName(name, VName::AddLower8));
		ScriptSize = Strm->TotalSize();
		ScriptBuffer = new char[ScriptSize + 1];
		Strm->Serialise(ScriptBuffer, ScriptSize);
		ScriptBuffer[ScriptSize] = 0;
		delete Strm;
		ScriptName = name;
	}
	else if (type == LUMP_NUM_SCRIPT)
	{
		// Lump num script
		VStream* Strm = W_CreateLumpReaderNum(LumpNum);
		ScriptSize = Strm->TotalSize();
		ScriptBuffer = new char[ScriptSize + 1];
		Strm->Serialise(ScriptBuffer, ScriptSize);
		ScriptBuffer[ScriptSize] = 0;
		delete Strm;
		ScriptName = name;
	}
	else if (type == FILE_ZONE_SCRIPT)
	{
		// File script
		ScriptSize = M_ReadFile(name, (byte **)&ScriptBuffer);
		ScriptName = VStr(name).ExtractFileBase();
	}
	ScriptPtr = ScriptBuffer;
	ScriptEndPtr = ScriptPtr + ScriptSize;
	sc_Line = 1;
	sc_End = false;
	ScriptOpen = true;
	sc_String = StringBuffer;
	AlreadyGot = false;

	//	Skip garbage some editors add in the begining of UTF-8 files.
	if (ScriptPtr[0] == 0xef && ScriptPtr[1] == 0xbb && ScriptPtr[2] == 0xbf)
	{
		ScriptPtr += 3;
	}
	unguard;
}

//==========================================================================
//
// SC_Close
//
//==========================================================================

void SC_Close()
{
	guard(SC_Close);
	if (ScriptOpen)
	{
		Z_Free(ScriptBuffer);
		ScriptOpen = false;
	}
	ScriptName.Clean();
	unguard;
}

//==========================================================================
//
// SC_GetString
//
//==========================================================================

boolean SC_GetString()
{
	guard(SC_GetString);
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
	unguard;
}

//==========================================================================
//
// SC_MustGetString
//
//==========================================================================

void SC_MustGetString()
{
	guard(SC_MustGetString);
	if (SC_GetString() == false)
	{
		SC_ScriptError("Missing string.");
	}
	unguard;
}

//==========================================================================
//
// SC_MustGetStringName
//
//==========================================================================

void SC_MustGetStringName(const char *name)
{
	guard(SC_MustGetStringName);
	SC_MustGetString();
	if (SC_Compare(name) == false)
	{
		SC_ScriptError(NULL);
	}
	unguard;
}

//==========================================================================
//
//	SC_CheckNumber
//
//==========================================================================

boolean SC_CheckNumber()
{
	guard(SC_CheckNumber);
	char *stopper;

	CheckOpen();
	if (SC_GetString())
	{
		sc_Number = strtol(sc_String, &stopper, 0);
		if (*stopper != 0)
		{
			SC_UnGet();
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}
	unguard;
}

//==========================================================================
//
// SC_GetNumber
//
//==========================================================================

boolean SC_GetNumber()
{
	guard(SC_GetNumber);
	char *stopper;

	CheckOpen();
	if (SC_GetString())
	{
		sc_Number = strtol(sc_String, &stopper, 0);
		if (*stopper != 0)
		{
			Sys_Error("SC_GetNumber: Bad numeric constant \"%s\".\n"
				"Script %s, Line %d", sc_String, *ScriptName, sc_Line);
		}
		return true;
	}
	else
	{
		return false;
	}
	unguard;
}

//==========================================================================
//
// SC_MustGetNumber
//
//==========================================================================

void SC_MustGetNumber()
{
	guard(SC_MustGetNumber);
	if (SC_GetNumber() == false)
	{
		SC_ScriptError("Missing integer.");
	}
	unguard;
}

//==========================================================================
//
//	SC_CheckFloat
//
//==========================================================================

boolean SC_CheckFloat()
{
	guard(SC_CheckFloat);
	char *stopper;

	CheckOpen();
	if (SC_GetString())
	{
		sc_Float = strtod(sc_String, &stopper);
		if (*stopper != 0)
		{
			SC_UnGet();
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}
	unguard;
}

//==========================================================================
//
//	SC_GetFloat
//
//==========================================================================

boolean SC_GetFloat()
{
	guard(SC_GetFloat);
	char *stopper;

	CheckOpen();
	if (SC_GetString())
	{
		sc_Float = strtod(sc_String, &stopper);
		if (*stopper != 0)
		{
			Sys_Error("SC_GetFloat: Bad floating point constant \"%s\".\n"
				"Script %s, Line %d", sc_String, *ScriptName, sc_Line);
		}
		return true;
	}
	else
	{
		return false;
	}
	unguard;
}

//==========================================================================
//
//	SC_MustGetFloat
//
//==========================================================================

void SC_MustGetFloat()
{
	guard(SC_MustGetFloat);
	if (SC_GetFloat() == false)
	{
		SC_ScriptError("Missing float.");
	}
	unguard;
}

//==========================================================================
//
// SC_UnGet
//
// Assumes there is a valid string in sc_String.
//
//==========================================================================

void SC_UnGet()
{
	guard(SC_UnGet);
	AlreadyGot = true;
	unguard;
}

//==========================================================================
//
// SC_Check
//
// Returns true if another token is on the current line.
//
//==========================================================================

/*
boolean SC_Check()
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
	guard(SC_MatchString);
	int i;

	for (i = 0; *strings != NULL; i++)
	{
		if (SC_Compare(*strings++))
		{
			return i;
		}
	}
	unguard;
	return -1;
}

//==========================================================================
//
// SC_MustMatchString
//
//==========================================================================

int SC_MustMatchString(const char **strings)
{
	guard(SC_MustMatchString);
	int i;

	i = SC_MatchString(strings);
	if (i == -1)
	{
		SC_ScriptError(NULL);
	}
	return i;
	unguard;
}

//==========================================================================
//
// SC_Compare
//
//==========================================================================

boolean SC_Compare(const char *text)
{
	guard(SC_Compare);
	return !VStr::ICmp(text, sc_String);
	unguard;
}

//==========================================================================
//
// SC_ScriptError
//
//==========================================================================

void SC_ScriptError(const char *message)
{
	guard(SC_ScriptError)
	const char* Msg = message ? message : "Bad syntax.";
	Sys_Error("Script error, \"%s\" line %d: %s", *ScriptName,
		sc_Line, Msg);
	unguard;
}

//==========================================================================
//
// CheckOpen
//
//==========================================================================

static void CheckOpen()
{
	guard(CheckOpen);
	if (ScriptOpen == false)
	{
		Sys_Error("SC_ call before SC_Open().");
	}
	unguard;
}
