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

#ifndef _SCRIPTS_H
#define _SCRIPTS_H

namespace VavoomUtils {

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void SC_Open(const char *name);
void SC_Close(void);
bool SC_GetString(void);
void SC_MustGetString(void);
void SC_MustGetStringName(const char *name);
bool SC_GetNumber(void);
void SC_MustGetNumber(void);
bool SC_GetFloat(void);
void SC_MustGetFloat(void);
void SC_UnGet(void);
bool SC_Compare(const char *text);
int SC_MatchString(const char **strings);
int SC_MustMatchString(const char **strings);
void SC_ScriptError(const char *message);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern char 	*sc_String;
extern int 		sc_Number;
extern double	sc_Float;
extern int 		sc_Line;
extern bool 	sc_End;
extern bool 	sc_Crossed;

} // namespace

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/09/24 17:28:45  dj_jl
//	Beautification
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
