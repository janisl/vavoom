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
//**
//**	WAD I/O functions.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

//	Boom namespaces.
enum EWadNamespace
{
	WADNS_Global,
	WADNS_Sprites,
	WADNS_Flats,
	WADNS_ColorMaps,
	WADNS_ACSLibrary,
	WADNS_NewTextures,
	WADNS_Voices,
	WADNS_HiResTextures,
	WADNS_Progs,
};

class VSearchPath
{
public:
	virtual ~VSearchPath();
	virtual VStr FindFile(const VStr&) = 0;
	virtual VStream* OpenFileRead(const VStr&) = 0;
	virtual void Close() = 0;
	virtual int CheckNumForName(VName, EWadNamespace) = 0;
	virtual void ReadLump(int, void*) = 0;
	virtual void ReadFromLump(int, void*, int, int) = 0;
	virtual void* CacheLumpNum(int) = 0;
	virtual int LumpLength(int) = 0;
	virtual VName LumpName(int) = 0;
	virtual int IterateNS(int, EWadNamespace) = 0;
	virtual void BuildGLNodes(VSearchPath*) = 0;
	virtual void BuildPVS(VSearchPath*) = 0;
	virtual VStream* CreateLumpReaderNum(int) = 0;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void W_AddFile(const VStr& FileName, const VStr& GwaDir, bool FixVoices);
void W_Shutdown();

void W_OpenAuxiliary(const VStr& FileName);
void W_CloseAuxiliary();

void W_BuildGLNodes(int lump);
void W_BuildPVS(int lump, int gllump);

int W_CheckNumForName(VName Name, EWadNamespace NS = WADNS_Global);
int W_GetNumForName(VName Name, EWadNamespace NS = WADNS_Global);

int W_LumpLength(int lump);
VName W_LumpName(int lump);

void W_ReadLump(int lump, void *dest);
void W_ReadFromLump(int lump, void *dest, int pos, int size);
void* W_CacheLumpNum(int lump);
void* W_CacheLumpName(VName Name, EWadNamespace NS = WADNS_Global);
VStream* W_CreateLumpReaderNum(int lump);
VStream* W_CreateLumpReaderName(VName Name, EWadNamespace NS = WADNS_Global);

int W_IterateNS(int Prev, EWadNamespace NS);

void W_CleanupName(const char *src, char *dst);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern TArray<VSearchPath*>	SearchPaths;
