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

// State updates, number of tics / second.
#define TICRATE		35

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

int Sys_FileOpenRead(const char*);
int Sys_FileOpenWrite(const char*);
int Sys_FileRead(int, void*, int);
int Sys_FileWrite(int, const void*, int);
int Sys_FileSize(int);
int Sys_FileSeek(int, int);
int Sys_FileClose(int);
int Sys_FileExists(const char*);
int Sys_FileTime(const char*);

int Sys_CreateDirectory(const char*);
void Sys_GetBaseDir(char*);

void* Sys_ZoneBase(int*);

void Sys_Error(const char*, ...) __attribute__ ((noreturn, format(printf, 1, 2)));
void Sys_Quit(void) __attribute__ ((noreturn));
void Sys_Shutdown(void);

double Sys_Time(void);

char *Sys_ConsoleInput(void);

extern "C" {

void Sys_LowFPPrecision(void);
void Sys_HighFPPrecision(void);

}

//
// memory protection
//
void Sys_MakeCodeWriteable(unsigned long, unsigned long);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2001/08/31 17:26:48  dj_jl
//	Attribute syntax change
//
//	Revision 1.4  2001/08/29 17:49:36  dj_jl
//	Added file time functions
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
