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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void SC_Open(const char *name);
void SC_OpenLump(const char *name);
void SC_OpenFile(const char *name);
void SC_Close(void);
boolean SC_GetString(void);
void SC_MustGetString(void);
void SC_MustGetStringName(const char *name);
boolean SC_GetNumber(void);
void SC_MustGetNumber(void);
boolean SC_GetFloat(void);
void SC_MustGetFloat(void);
void SC_UnGet(void);
//boolean SC_Check(void);
boolean SC_Compare(const char *text);
int SC_MatchString(const char **strings);
int SC_MustMatchString(const char **strings);
void SC_ScriptError(const char *message);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern char 	*sc_String;
extern int 		sc_Number;
extern double 	sc_Float;
extern int 		sc_Line;
extern boolean 	sc_End;
extern boolean 	sc_Crossed;
extern boolean 	sc_FileScripts;
extern char 	*sc_ScriptsDir;

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
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
