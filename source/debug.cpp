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

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

//#define CLOSEDDF

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static FILE*		df = NULL;
static const char*	debug_file_name;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	OpenDebugFile
//
//==========================================================================

void OpenDebugFile(const char* name)
{
    debug_file_name = name;
#ifdef CLOSEDDF
	df = fopen(debug_file_name, "w");
	fclose(df);
#else
#ifndef DEVELOPER
	if (M_CheckParm("-debug"))
#endif
	{
		if (M_CheckParm("-RHIDE"))
    		df = stderr;
		else
			df = fopen(debug_file_name, "w");
	}
#endif
}

//==========================================================================
//
//	dprintf
//
//==========================================================================

int dprintf(const char *s, ...)
{
	va_list	v;

#ifdef CLOSEDDF
    df = fopen(debug_file_name, "a");
#endif

	va_start(v, s);
	if (df)
	{
		vfprintf(df, s, v);
		fflush(df);
	}
	va_end(v);

#ifdef CLOSEDDF
	fclose(df);
#endif

	return 0;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
