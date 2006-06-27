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

// MACROS ------------------------------------------------------------------

#define Random()	((float)(rand() & 0x7fff) / (float)0x8000)

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

//	Endianess handling
void M_InitByteOrder();
extern bool GBigEndian;
extern short (*LittleShort)(short);
extern short (*BigShort)(short);
extern int (*LittleLong)(int);
extern int (*BigLong)(int);
extern float (*LittleFloat)(float);
extern float (*BigFloat)(float);

int M_ReadFile(const char* name, byte** buffer);
boolean M_WriteFile(const char* name, const void* source, int length);

int superatoi(const char *s);

char *va(const char *text, ...) __attribute__ ((format(printf, 1, 2)));
int PassFloat(float f);

// PUBLIC DATA DECLARATIONS ------------------------------------------------
