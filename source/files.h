//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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

#define MAX_OSPATH		128

// TYPES -------------------------------------------------------------------

class TFile
{
 public:
	bool OpenRead(const char *);
	bool OpenWrite(const char *);
	int Read(void* buf, int size);
	int Write(const void* buf, int size);
	int Size();
	int Seek(int);
	int Close();

 protected:
	int handle;
};

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
int  FL_ReadFile(const char* name, byte** buffer, int tag);
bool FL_WriteFile(const char* name, const void* source, int length);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern char		fl_basedir[MAX_OSPATH];
extern char		fl_gamedir[MAX_OSPATH];

