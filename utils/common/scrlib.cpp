//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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

#include "cmdlib.h"
#include "scrlib.h"

// MACROS ------------------------------------------------------------------

#define MAX_STRING_SIZE		256

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void CheckOpen(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

char 	*sc_String;
int 	sc_Number;
double	sc_Float;
int 	sc_Line;
bool	sc_End;
bool	sc_Crossed;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char 	ScriptName[1024];
static char 	*ScriptBuffer;
static char 	*ScriptPtr;
static char 	*ScriptEndPtr;
static char 	StringBuffer[MAX_STRING_SIZE];
static bool 	ScriptOpen = false;
static int 		ScriptSize;
static bool 	AlreadyGot = false;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	SC_Open
//
//==========================================================================

void SC_Open(const char *name)
{
	SC_Close();

	// File script
	ScriptSize = LoadFile(name, (void**)&ScriptBuffer);
	strcpy(ScriptName, name);

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
//	SC_Close
//
//==========================================================================

void SC_Close(void)
{
	if (ScriptOpen)
	{
		Free(ScriptBuffer);
		ScriptOpen = false;
	}
}

//==========================================================================
//
//	SC_GetString
//
//==========================================================================

bool SC_GetString(void)
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
		if (ScriptPtr[0] == '/' && ScriptPtr[1] == '/')
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
	if (*ScriptPtr == '\"')
	{
		// Quoted string
		ScriptPtr++;
		while (*ScriptPtr != '\"')
		{
			*text++ = *ScriptPtr++;
			if (ScriptPtr == ScriptEndPtr
				|| text == &sc_String[MAX_STRING_SIZE - 1])
			{
				break;
			}
		}
		ScriptPtr++;
	}
	else
	{
		// Normal string
		while ((*ScriptPtr > 32) && (ScriptPtr[0] != '/' || ScriptPtr[1] != '/'))
		{
			*text++ = *ScriptPtr++;
			if (ScriptPtr == ScriptEndPtr
				|| text == &sc_String[MAX_STRING_SIZE - 1])
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
//	SC_MustGetString
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
//	SC_MustGetStringName
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
//	SC_GetNumber
//
//==========================================================================

bool SC_GetNumber(void)
{
	char *stopper;

	CheckOpen();
	if (SC_GetString())
	{
		sc_Number = strtol(sc_String, &stopper, 0);
		if (*stopper != 0)
		{
			Error("SC_GetNumber: Bad numeric constant \"%s\".\n"
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
//	SC_MustGetNumber
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

bool SC_GetFloat(void)
{
	char *stopper;

	CheckOpen();
	if (SC_GetString())
	{
		sc_Float = strtod(sc_String, &stopper);
		if (*stopper != 0)
		{
			Error("SC_GetFloat: Bad floating point constant \"%s\".\n"
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
//	SC_UnGet
//
//	Assumes there is a valid string in sc_String.
//
//==========================================================================

void SC_UnGet(void)
{
	AlreadyGot = true;
}

//==========================================================================
//
//	SC_MatchString
//
//	Returns the index of the first match to sc_String from the passed
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
//	SC_MustMatchString
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
//	SC_Compare
//
//==========================================================================

bool SC_Compare(const char *text)
{
	return !stricmp(text, sc_String);
}

//==========================================================================
//
//	SC_ScriptError
//
//==========================================================================

void SC_ScriptError(const char *message)
{
	if (message == NULL)
	{
		message = "Bad syntax.";
	}
	Error("Script error, \"%s\" line %d: %s", ScriptName, sc_Line, message);
}

//==========================================================================
//
//	CheckOpen
//
//==========================================================================

static void CheckOpen(void)
{
	if (ScriptOpen == false)
	{
		Error("SC_ call before SC_Open().");
	}
}
