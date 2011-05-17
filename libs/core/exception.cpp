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
//**	Copyright (C) 1999-2010 Jānis Legzdiņš
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

#include "core.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

#ifdef USE_GUARD_SIGNAL_CONTEXT
jmp_buf			__Context::Env;
const char*		__Context::ErrToThrow;
#endif

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char*	host_error_string;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VavoomError::VavoomError
//
//==========================================================================

VavoomError::VavoomError(const char *text)
{
	VStr::NCpy(message, text, MAX_ERROR_TEXT_SIZE - 1);
	message[MAX_ERROR_TEXT_SIZE - 1] = 0;
}

//==========================================================================
//
//	VavoomError::What
//
//==========================================================================

const char* VavoomError::What() const
{
	return message;
}

//==========================================================================
//
//	Host_CoreDump
//
//==========================================================================

void Host_CoreDump(const char *fmt, ...)
{
	bool first = false;

	if (!host_error_string)
	{
		host_error_string = new char[32];
		VStr::Cpy(host_error_string, "Stack trace: ");
		first = true;
	}

	va_list argptr;
	char string[1024];
	
	va_start(argptr, fmt);
	vsprintf(string, fmt, argptr);
	va_end(argptr);

	GLog.WriteLine("- %s", string);

	char *new_string = new char[VStr::Length(host_error_string) +
		VStr::Length(string) + 6];
	VStr::Cpy(new_string, host_error_string);
	if (first)
		first = false;
	else
		strcat(new_string, " <- ");
	strcat(new_string, string);
	delete[] host_error_string;
	host_error_string = NULL;
	host_error_string = new_string;
}

//==========================================================================
//
//	Host_GetCoreDump
//
//==========================================================================

const char *Host_GetCoreDump()
{
	return host_error_string ? host_error_string : "";
}

//==========================================================================
//
// 	Sys_Error
//
//	Exits game and displays error message.
//
//==========================================================================

void Sys_Error(const char *error, ...)
{
	va_list		argptr;
	char		buf[1024];

	va_start(argptr,error);
	vsprintf(buf, error, argptr);
	va_end(argptr);

	GLog.WriteLine("Sys_Error: %s", buf);
	throw VavoomError(buf);
}
