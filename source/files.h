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

#define MAX_OSPATH		128
#define MAX_VPATH		128

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void FL_Init(void);

void FL_DefaultPath(char *path, const char *basepath);
void FL_DefaultExtension(char *path, const char *extension);
void FL_StripFilename(char *path);
void FL_StripExtension(char *path);
void FL_ExtractFilePath(const char *path, char *dest);
void FL_ExtractFileBase(const char *path, char *dest);
void FL_ExtractFileExtension(const char *path, char *dest);

bool FL_FindFile(const char *fname, char *dest);
int FL_ReadFile(const char* name, void** buffer, int tag);
bool FL_WriteFile(const char* name, const void* source, int length);

FArchive* FL_OpenFileRead(const char *Name);
FArchive* FL_OpenFileWrite(const char *Name);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern bool		fl_devmode;
extern char		fl_basedir[MAX_OSPATH];
extern char		fl_gamedir[MAX_OSPATH];

//**************************************************************************
//
//	$Log$
//	Revision 1.8  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//
//	Revision 1.7  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.6  2001/11/09 14:34:24  dj_jl
//	Added MAX_VPATH
//	
//	Revision 1.5  2001/09/05 12:21:42  dj_jl
//	Release changes
//	
//	Revision 1.4  2001/08/21 17:40:54  dj_jl
//	Added devgame mode
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
