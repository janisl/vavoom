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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************
//**
//**	WAD I/O functions.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void W_InitMultipleFiles(const char** filenames);
void W_InitFile(const char* filename);

void W_OpenAuxiliary(const char *filename);
void W_CloseAuxiliaryFile(void);
void W_CloseAuxiliary(void);
void W_UsePrimary(void);
void W_UseAuxiliary(void);

int	W_CheckNumForName(const char* name);
int	W_GetNumForName(const char* name);

int	W_LumpLength(int lump);
const char *W_LumpName(int lump);

void W_ReadLump(int lump, void *dest);
void* W_CacheLumpNum(int lump, int tag);
void* W_CacheLumpName(const char* name, int tag);

bool W_ForEachLump(bool (*func)(int, const char*, int));

void W_CleanupName(const char *src, char *dst);

// PUBLIC DATA DECLARATIONS ------------------------------------------------


