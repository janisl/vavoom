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
//**
//**	INFO STRINGS
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "infostr.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	Info_ValueForKey
//
//	Searches the string for the given key and returns the associated value,
// or an empty string.
//
//==========================================================================

char *Info_ValueForKey(const char *s, const char *key)
{
	char		pkey[MAX_INFO_KEY];
	static char	value[2][MAX_INFO_VALUE];	// use two buffers so compares
									// work without stomping on each other
	static int	valueindex = 0;
	char		*o;
	
	if (!s || !key)
	{
		return "";
	}

	if (strlen(s) >= MAX_INFO_STRING)
	{
		Host_Error("Info_ValueForKey: oversize infostring");
	}

	valueindex ^= 1;
	if (*s == '\\')
		s++;		
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			*o++ = *s++;
		}
		*o = 0;

		if (!stricmp(key, pkey))
			return value[valueindex];

		if (!*s)
			return "";
		s++;
	}
}

//==========================================================================
//
//	Info_RemoveKey
//
//==========================================================================

void Info_RemoveKey(char *s, const char *key)
{
	char	*start;
	char	pkey[MAX_INFO_KEY];
	char	value[MAX_INFO_VALUE];
	char	*o;

	if (strlen(s) >= MAX_INFO_STRING)
	{
		Host_Error("Info_RemoveKey: oversize infostring");
	}

	if (strchr(key, '\\'))
	{
		con << "Can't use a key with a \\\n";
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp(key, pkey))
		{
			strcpy(start, s);	// remove this part
			return;
		}

		if (!*s)
			return;
	}

}

//==========================================================================
//
//	Info_SetValueForKey
//
//	Changes or adds a key/value pair
//
//==========================================================================

void Info_SetValueForKey(char *s, const char *key, const char *value)
{
	char	newi[MAX_INFO_STRING];
	char	*v;

	if (strlen(s) >= MAX_INFO_STRING)
	{
		Host_Error("Info_SetValueForKey: oversize infostring");
	}

	if (strchr(key, '\\') || strchr(value, '\\'))
	{
		con << "Can't use keys or values with a \\\n";
		return;
	}

	if (strchr(key, '\"') || strchr(value, '\"'))
	{
		con << "Can't use keys or values with a \"\n";
		return;
	}

	// this next line is kinda trippy
	v = Info_ValueForKey(s, key);
	if (*v)
	{
		//	Key exists, make sure we have enough room for new value, if we
		// don't, don't change it!
		if (strlen(value) - strlen(v) + strlen(s) > MAX_INFO_STRING)
		{
			con << "Info string length exceeded\n";
			return;
		}
	}

	Info_RemoveKey(s, key);
	if (!value || !strlen(value))
		return;

	sprintf(newi, "\\%s\\%s", key, value);

	if (strlen(newi) + strlen(s) > MAX_INFO_STRING)
	{
		con << "Info string length exceeded\n";
		return;
	}

	strcat(s, newi);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
