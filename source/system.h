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

int Sys_FileOpenRead(const VStr&);
int Sys_FileOpenWrite(const VStr&);
int Sys_FileRead(int, void*, int);
int Sys_FileWrite(int, const void*, int);
int Sys_FileSize(int);
int Sys_FileSeek(int, int);
int Sys_FileClose(int);
int Sys_FileExists(const VStr&);
int Sys_FileTime(const VStr&);

int Sys_CreateDirectory(const VStr&);
int Sys_OpenDir(const VStr&);
VStr Sys_ReadDir();
void Sys_CloseDir();
bool Sys_DirExists(const VStr&);

void* Sys_ZoneBase(int*);

void __attribute__((noreturn, format(printf, 1, 2))) __declspec(noreturn) 
	Sys_Error(const char*, ...);
void __attribute__((noreturn)) __declspec(noreturn) Sys_Quit();
void Sys_Shutdown();

double Sys_Time();

char *Sys_ConsoleInput();

extern "C" {

void Sys_LowFPPrecision();
void Sys_HighFPPrecision();

}

//
// memory protection
//
void Sys_MakeCodeWriteable(unsigned long, unsigned long);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//**************************************************************************
//
//	$Log$
//	Revision 1.9  2006/03/04 16:01:34  dj_jl
//	File system API now uses strings.
//
//	Revision 1.8  2002/04/11 16:40:32  dj_jl
//	Added __declspec modifiers.
//	
//	Revision 1.7  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.6  2001/11/09 14:19:42  dj_jl
//	Functions for directory listing
//	
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
