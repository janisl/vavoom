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

// MACROS ------------------------------------------------------------------

#define Random()	((float)(rand() & 0x7fff) / (float)0x8000)

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// Returns the position of the given parameter
// in the arg list (0 if not found).
void M_InitArgs(int argc, char **argv);
int M_CheckParm(const char* check);

//	Endianess handling
void M_InitByteOrder(void);
extern short (*LittleShort)(short);
extern short (*BigShort)(short);
extern int (*LittleLong)(int);
extern int (*BigLong)(int);
extern float (*LittleFloat)(float);
extern float (*BigFloat)(float);

int M_ReadFile(const char* name, byte** buffer);
boolean M_WriteFile(const char* name, const void* source, int length);

boolean M_ValidEpisodeMap(int episode, int map);

int superatoi(const char *s);

char *va(const char *text, ...) __attribute__ ((format(printf, 1, 2)));
int PassFloat(float f);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int		myargc;
extern char**	myargv;

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
