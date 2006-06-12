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

#define MAX_INFO_STRING		1024

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

VStr Info_ValueForKey(const VStr& s, const VStr& key)
{
	guard(Info_ValueForKey);
	if (!s || !key)
	{
		return VStr();
	}

	if (s.Length() >= MAX_INFO_STRING)
	{
		Host_Error("Info_ValueForKey: oversize infostring");
	}

	int i = 0;
	if (s[i] == '\\')
		i++;		
	while (1)
	{
		int Start = i;
		while (s[i] != '\\')
		{
			if (!s[i])
				return VStr();
			i++;
		}
		VStr pkey(s, Start, i - Start);
		i++;

		Start = i;
		while (s[i] != '\\' && s[i])
		{
			i++;
		}

		if (!key.ICmp(pkey))
			return VStr(s, Start, i - Start);

		if (!s[i])
			return VStr();
		i++;
	}
	unguard;
}

//==========================================================================
//
//	Info_RemoveKey
//
//==========================================================================

void Info_RemoveKey(VStr& s, const VStr& key)
{
	guard(Info_RemoveKey);
	if (!s)
	{
		return;
	}
	if (s.Length() >= MAX_INFO_STRING)
	{
		Host_Error("Info_RemoveKey: oversize infostring");
	}

	if (strchr(*key, '\\'))
	{
		GCon->Log("Can't use a key with a \\");
		return;
	}

	int i = 0;
	while (1)
	{
		int start = i;
		if (s[i] == '\\')
			i++;
		int KeyStart = i;
		while (s[i] != '\\')
		{
			if (!s[i])
				return;
			i++;
		}
		VStr pkey(s, KeyStart, i - KeyStart);
		i++;

		int ValStart = i;
		while (s[i] != '\\' && s[i])
		{
			i++;
		}
		VStr value(s, ValStart, i - ValStart);

		if (!key.Cmp(pkey))
		{
			s = VStr(s, 0, start) + VStr(s, i, s.Length() - i);	// remove this part
			return;
		}

		if (!s[i])
			return;
	}
	unguard;
}

//==========================================================================
//
//	Info_SetValueForKey
//
//	Changes or adds a key/value pair
//
//==========================================================================

void Info_SetValueForKey(VStr& s, const VStr& key, const VStr& value)
{
	guard(Info_SetValueForKey);
	if (s.Length() >= MAX_INFO_STRING)
	{
		Host_Error("Info_SetValueForKey: oversize infostring");
	}

	if (strchr(*key, '\\') || strchr(*value, '\\'))
	{
		GCon->Log("Can't use keys or values with a \\");
		return;
	}

	if (strchr(*key, '\"') || strchr(*value, '\"'))
	{
		GCon->Log("Can't use keys or values with a \"");
		return;
	}

	// this next line is kinda trippy
	VStr v = Info_ValueForKey(s, key);
	if (v)
	{
		//	Key exists, make sure we have enough room for new value, if we
		// don't, don't change it!
		if (value.Length() - v.Length() + s.Length() > MAX_INFO_STRING)
		{
			GCon->Log("Info string length exceeded");
			return;
		}
	}

	Info_RemoveKey(s, key);
	if (!value)
		return;

	VStr newi = VStr("\\") + key + "\\" +  value;

	if (newi.Length() + s.Length() > MAX_INFO_STRING)
	{
		GCon->Log("Info string length exceeded");
		return;
	}

	s = s + newi;
	unguard;
}
