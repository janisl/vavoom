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
//**
//**	WAD I/O functions.
//**
//**************************************************************************

//	Boom namespaces.
enum EWadNamespace
{
	WADNS_Global,
	WADNS_Sprites,
	WADNS_Flats,
	WADNS_ColourMaps,
	WADNS_ACSLibrary,
	WADNS_NewTextures,
	WADNS_Voices,
	WADNS_HiResTextures,

	//	Special namespaces for zip files, in wad file they will be searched
	// in global namespace.
	WADNS_ZipSpecial,
	WADNS_Patches,
	WADNS_Graphics,
	WADNS_Sounds,
	WADNS_Music,
};

void W_AddFile(const VStr& FileName, const VStr& GwaDir, bool FixVoices);
void W_Shutdown();

int W_OpenAuxiliary(const VStr& FileName);
void W_CloseAuxiliary();

void W_BuildGLNodes(int lump);
void W_BuildPVS(int lump, int gllump);

int W_CheckNumForName(VName Name, EWadNamespace NS = WADNS_Global);
int W_GetNumForName(VName Name, EWadNamespace NS = WADNS_Global);
int W_CheckNumForNameInFile(VName Name, int File, EWadNamespace NS = WADNS_Global);

int W_CheckNumForFileName(VStr Name);
int W_GetNumForFileName(VStr Name);
int W_FindLumpByFileNameWithExts(VStr BaseName, const char** Exts);

int W_LumpLength(int lump);
VName W_LumpName(int lump);
int W_LumpFile(int lump);

void W_ReadFromLump(int lump, void *dest, int pos, int size);
void W_LoadLumpIntoArray(int Lump, TArray<vuint8>& Array);
VStream* W_CreateLumpReaderNum(int lump);
VStream* W_CreateLumpReaderName(VName Name, EWadNamespace NS = WADNS_Global);

int W_IterateNS(int Prev, EWadNamespace NS);
int W_IterateFile(int Prev, const VStr& Name);
