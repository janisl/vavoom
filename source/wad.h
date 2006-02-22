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

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void W_AddFile(const char *filename, const char* gwadir, bool FixVoices);

void W_OpenAuxiliary(const char *filename);
void W_CloseAuxiliaryFile(void);
void W_CloseAuxiliary(void);

void W_BuildGLNodes(int lump);
void W_BuildPVS(int lump, int gllump);

int	W_CheckNumForName(const char* name, EWadNamespace NS = WADNS_Global);
int	W_GetNumForName(const char* name, EWadNamespace NS = WADNS_Global);

int	W_LumpLength(int lump);
const char *W_LumpName(int lump);

void W_ReadLump(int lump, void *dest);
void W_ReadFromLump(int lump, void *dest, int pos, int size);
void* W_CacheLumpNum(int lump, int tag);
void* W_CacheLumpName(const char* name, int tag,
	EWadNamespace NS = WADNS_Global);
VStream* W_CreateLumpReader(int lump);
VStream* W_CreateLumpReader(const char* name, EWadNamespace NS = WADNS_Global);

bool W_ForEachLump(bool (*func)(int, const char*, int, EWadNamespace));
int W_IterateNS(int Prev, EWadNamespace NS);

void W_CleanupName(const char *src, char *dst);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//**************************************************************************
//
//	$Log$
//	Revision 1.13  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//
//	Revision 1.12  2006/01/29 20:41:30  dj_jl
//	On Unix systems use ~/.vavoom for generated files.
//	
//	Revision 1.11  2005/11/24 20:07:36  dj_jl
//	Aded namespace for progs.
//	
//	Revision 1.10  2005/11/06 15:28:40  dj_jl
//	Some cleanup.
//	
//	Revision 1.9  2005/11/05 14:57:36  dj_jl
//	Putting Strife shareware voices in correct namespace.
//	
//	Revision 1.8  2005/05/26 16:55:43  dj_jl
//	New lump namespace iterator
//	
//	Revision 1.7  2004/11/23 12:43:11  dj_jl
//	Wad file lump namespaces.
//	
//	Revision 1.6  2002/05/18 16:56:35  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.5  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.4  2001/09/14 16:52:14  dj_jl
//	Added dynamic build of GWA file
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
