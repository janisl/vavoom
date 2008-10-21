//**************************************************************************
//**
//**    ##   ##    ##    ##   ##   ####     ####   ###     ###
//**    ##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**     ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**     ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**      ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**       #    ##    ##    #      ####     ####   ##       ##
//**
//**    $Id$
//**
//**    Copyright (C) 1999-2006 Jānis Legzdiņš
//**
//**    This program is free software; you can redistribute it and/or
//**  modify it under the terms of the GNU General Public License
//**  as published by the Free Software Foundation; either version 2
//**  of the License, or (at your option) any later version.
//**
//**    This program is distributed in the hope that it will be useful,
//**  but WITHOUT ANY WARRANTY; without even the implied warranty of
//**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//**  GNU General Public License for more details.
//**
//**************************************************************************
//**	
//**    Handles WAD file header, directory, lump I/O.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "fs_local.h"

// MACROS ------------------------------------------------------------------

#define GET_LUMP_FILE(num)		SearchPaths[num >> 16]
#define FILE_INDEX(num)			(num >> 16)
#define LUMP_INDEX(num)			(num & 0xffff)
#define MAKE_HANDLE(wi, num)	((wi << 16) + num)

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern TArray<VStr>			wadfiles;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int					AuxiliaryIndex;

// CODE --------------------------------------------------------------------

VSearchPath::~VSearchPath()
{
}

//==========================================================================
//
//  W_AddFile
//
//  All files are optional, but at least one file must be found (PWAD, if
// all required lumps are present). Files with a .wad extension are wadlink
// files with multiple lumps. Other files are single lumps with the base
// filename for the lump name.
//
//==========================================================================

void W_AddFile(const VStr& FileName, const VStr& GwaDir, bool FixVoices)
{
	guard(W_AddFile);
	int wadtime;

	wadtime = Sys_FileTime(FileName);
	if (wadtime == -1)
	{
		Sys_Error("Required file %s doesn't exist", *FileName);
	}

	wadfiles.Append(FileName);

	VStr ext = FileName.ExtractFileExtension().ToLower();
	VWadFile* Wad = new VWadFile;
	if (ext != "wad" && ext != "gwa")
	{
		Wad->OpenSingleLump(FileName);
	}
	else
	{
		Wad->Open(FileName, GwaDir, FixVoices, NULL);
	}
	SearchPaths.Append(Wad);

	if (ext == "wad")
	{
		VStr gl_name;

		bool FoundGwa = false;
		if (GwaDir)
		{
			gl_name = GwaDir + "/" +
				FileName.ExtractFileName().StripExtension() + ".gwa";
			if (Sys_FileTime(gl_name) >= wadtime)
			{
				W_AddFile(gl_name, VStr(), false);
				FoundGwa = true;
			}
		}

		if (!FoundGwa)
		{
			gl_name = FileName.StripExtension() + ".gwa";
			if (Sys_FileTime(gl_name) >= wadtime)
			{
				W_AddFile(gl_name, VStr(), false);
			}
			else
			{
				//	Leave empty slot for GWA file
				SearchPaths.Append(new VWadFile);
			}
		}
	}
	unguard;
}

//==========================================================================
//
//  W_AddFileFromZip
//
//==========================================================================

void W_AddFileFromZip(const VStr& WadName, VStream* WadStrm,
	const VStr& GwaName, VStream* GwaStrm)
{
	guard(W_AddFileFromZip);
	//	Add WAD file.
	wadfiles.Append(WadName);
	VWadFile* Wad = new VWadFile;
	Wad->Open(WadName, VStr(), false, WadStrm);
	SearchPaths.Append(Wad);

	if (GwaStrm)
	{
		//	Add GWA file
		wadfiles.Append(GwaName);
		VWadFile* Gwa = new VWadFile;
		Gwa->Open(GwaName, VStr(), false, GwaStrm);
		SearchPaths.Append(Gwa);
	}
	else
	{
		//	Leave empty slot for GWA file
		SearchPaths.Append(new VWadFile);
	}
	unguard;
}

//==========================================================================
//
//  W_OpenAuxiliary
//
//==========================================================================

int W_OpenAuxiliary(const VStr& FileName)
{
	guard(W_OpenAuxiliary);
	W_CloseAuxiliary();

	AuxiliaryIndex = SearchPaths.Num();

	VStr GwaName = FileName.StripExtension() + ".gwa";
	VStream* WadStrm = FL_OpenFileRead(FileName);
	VStream* GwaStrm = FL_OpenFileRead(GwaName);
	W_AddFileFromZip(FileName, WadStrm, GwaName, GwaStrm);
	return MAKE_HANDLE(AuxiliaryIndex, 0);
	unguard;
}

//==========================================================================
//
//  W_CloseAuxiliary
//
//==========================================================================

void W_CloseAuxiliary()
{
	guard(W_CloseAuxiliary);
	if (AuxiliaryIndex)
	{
		SearchPaths[AuxiliaryIndex]->Close();
		SearchPaths[AuxiliaryIndex + 1]->Close();
		delete SearchPaths[AuxiliaryIndex];
		delete SearchPaths[AuxiliaryIndex + 1];
		SearchPaths.SetNum(AuxiliaryIndex);
		AuxiliaryIndex = 0;
	}
	unguard;
}

#ifdef CLIENT

//==========================================================================
//
//	W_BuildGLNodes
//
//==========================================================================

void W_BuildGLNodes(int lump)
{
	guard(W_BuildGLNodes);
	SearchPaths[FILE_INDEX(lump)]->BuildGLNodes(SearchPaths[FILE_INDEX(lump) + 1]);
	unguard;
}

//==========================================================================
//
//	W_BuildPVS
//
//==========================================================================

void W_BuildPVS(int lump, int gllump)
{
	guard(W_BuildPVS);
	SearchPaths[FILE_INDEX(gllump)]->BuildPVS(SearchPaths[FILE_INDEX(lump)]);
	unguard;
}

#endif

//==========================================================================
//
//  W_CheckNumForName
//
//  Returns -1 if name not found.
//
//==========================================================================

int W_CheckNumForName(VName Name, EWadNamespace NS)
{
	guard(W_CheckNumForName);
	for (int wi = SearchPaths.Num() - 1; wi >= 0; wi--)
	{
		int i = SearchPaths[wi]->CheckNumForName(Name, NS);
		if (i >= 0)
		{
			return MAKE_HANDLE(wi, i);
		}
	}

	// Not found.
	return -1;
	unguard;
}

//==========================================================================
//
//  W_GetNumForName
//
//  Calls W_CheckNumForName, but bombs out if not found.
//
//==========================================================================

int W_GetNumForName(VName Name, EWadNamespace NS)
{
	guard(W_GetNumForName);
	int i = W_CheckNumForName(Name, NS);
	if (i == -1)
	{
		Sys_Error("W_GetNumForName: %s not found!", *Name);
	}
	return i;
	unguard;
}

//==========================================================================
//
//  W_CheckNumForNameInFile
//
//  Returns -1 if name not found.
//
//==========================================================================

int W_CheckNumForNameInFile(VName Name, int File, EWadNamespace NS)
{
	guard(W_CheckNumForNameInFile);
	check(File >= 0);
	check(File < SearchPaths.Num());
	int i = SearchPaths[File]->CheckNumForName(Name, NS);
	if (i >= 0)
	{
		return MAKE_HANDLE(File, i);
	}

	// Not found.
	return -1;
	unguard;
}

//==========================================================================
//
//  W_CheckNumForFileName
//
//  Returns -1 if name not found.
//
//==========================================================================

int W_CheckNumForFileName(VStr Name)
{
	guard(W_CheckNumForFileName);
	for (int wi = SearchPaths.Num() - 1; wi >= 0; wi--)
	{
		int i = SearchPaths[wi]->CheckNumForFileName(Name);
		if (i >= 0)
		{
			return MAKE_HANDLE(wi, i);
		}
	}

	// Not found.
	return -1;
	unguard;
}

//==========================================================================
//
//	W_GetNumForFileName
//
//	Calls W_CheckNumForFileName, but bombs out if not found.
//
//==========================================================================

int W_GetNumForFileName(VStr Name)
{
	guard(W_GetNumForFileName);
	int i = W_CheckNumForFileName(Name);
	if (i == -1)
	{
		Sys_Error("W_GetNumForFileName: %s not found!", *Name);
	}
	return i;
	unguard;
}

//==========================================================================
//
//  W_LumpLength
//
//  Returns the buffer size needed to load the given lump.
//
//==========================================================================

int W_LumpLength(int lump)
{
	guard(W_LumpLength);
	if (FILE_INDEX(lump) >= SearchPaths.Num())
	{
		Sys_Error("W_LumpLength: %i >= num_wad_files", FILE_INDEX(lump));
	}
	VSearchPath* w = GET_LUMP_FILE(lump);
	int lumpindex = LUMP_INDEX(lump);
	return w->LumpLength(lumpindex);
	unguard;
}

//==========================================================================
//
//  W_LumpName
//
//==========================================================================

VName W_LumpName(int lump)
{
	guard(W_LumpName);
	if (FILE_INDEX(lump) >= SearchPaths.Num())
	{
		return NAME_None;
	}
	VSearchPath* w = GET_LUMP_FILE(lump);
	int lumpindex = LUMP_INDEX(lump);
	return w->LumpName(lumpindex);
	unguard;
}

//==========================================================================
//
//  W_LumpFile
//
//  Returns file index of the given lump.
//
//==========================================================================

int W_LumpFile(int lump)
{
	return FILE_INDEX(lump);
}

//==========================================================================
//
//  W_ReadFromLump
//
//==========================================================================

void W_ReadFromLump(int lump, void* dest, int pos, int size)
{
	guard(W_ReadFromLump);
	if (FILE_INDEX(lump) >= SearchPaths.Num())
	{
		Sys_Error("W_ReadFromLump: %i >= num_wad_files", FILE_INDEX(lump));
	}

	VSearchPath* w = GET_LUMP_FILE(lump);
	w->ReadFromLump(LUMP_INDEX(lump), dest, pos, size);
	unguard;
}

//==========================================================================
//
//  W_CreateLumpReaderNum
//
//==========================================================================

VStream* W_CreateLumpReaderNum(int lump)
{
	guard(W_CreateLumpReaderNum);
	return GET_LUMP_FILE(lump)->CreateLumpReaderNum(LUMP_INDEX(lump));
	unguard;
}

//==========================================================================
//
//  W_CreateLumpReaderName
//
//==========================================================================

VStream* W_CreateLumpReaderName(VName Name, EWadNamespace NS)
{
	guard(W_CreateLumpReaderName);
	return W_CreateLumpReaderNum(W_GetNumForName(Name, NS));
	unguard;
}

//==========================================================================
//
//  W_IterateNS
//
//==========================================================================

int W_IterateNS(int Prev, EWadNamespace NS)
{
	guard(W_IterateNS);
	int wi = FILE_INDEX((Prev + 1));
	int li = LUMP_INDEX((Prev + 1));
	for (; wi < SearchPaths.Num(); wi++, li = 0)
	{
		li = SearchPaths[wi]->IterateNS(li, NS);
		if (li != -1)
		{
			return MAKE_HANDLE(wi, li);
		}
	}
	return -1;
	unguard;
}

//==========================================================================
//
//  W_IterateFile
//
//==========================================================================

int W_IterateFile(int Prev, const VStr& Name)
{
	guard(W_IterateFile);
	for (int wi = FILE_INDEX(Prev) + 1; wi < SearchPaths.Num(); wi++)
	{
		int li = SearchPaths[wi]->CheckNumForFileName(Name);
		if (li != -1)
		{
			return MAKE_HANDLE(wi, li);
		}
	}
	return -1;
	unguard;
}

//==========================================================================
//
//  W_FindLumpByFileNameWithExts
//
//==========================================================================

int W_FindLumpByFileNameWithExts(VStr BaseName, const char** Exts)
{
	guard(W_FindLumpByFileNameWithExts);
	int Found = -1;
	for (const char** Ext = Exts; *Ext; Ext++)
	{
		VStr Check = BaseName + "." + *Ext;
		int Lump = W_CheckNumForFileName(Check);
		if (Lump <= Found)
		{
			continue;
		}
		//	For files from the same directory the order of extensions defines
		// the priority order.
		if (Found >= 0 && W_LumpFile(Found) == W_LumpFile(Lump))
		{
			continue;
		}
		Found = Lump;
	}
	return Found;
	unguard;
}

//==========================================================================
//
//  W_CreateLumpReaderNum
//
//==========================================================================

void W_LoadLumpIntoArray(VName LumpName, TArray<vuint8>& Array)
{
	int Lump = W_CheckNumForFileName(*LumpName);
	if (Lump < 0)
	{
		Lump = W_GetNumForName(LumpName);
	}
	VStream* Strm = W_CreateLumpReaderNum(Lump);
	check(Strm);
	Array.SetNum(Strm->TotalSize());
	Strm->Serialise(Array.Ptr(), Strm->TotalSize());
	delete Strm;
}

//==========================================================================
//
//  W_Profile
//
//==========================================================================

#if 0
void W_Profile()
{
	static int	info[2500][10];
	static int	profilecount = 0;
	int			i;
	memblock_t*	block;
	void*		ptr;
	char		ch;
	FILE*		f;
	int			j;
	char		name[16];
	
	sprintf(name,"jl/waddump%d.txt", profilecount);
	
	for (i = 0; i < numlumps; i++)
	{	
		ptr = lumpcache[i];
		if (!ptr)
		{
			ch = ' ';
			continue;
		}
		else
		{
			block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
			if (block->tag < PU_PURGELEVEL)
				ch = 'S';
			else
				ch = 'P';
		}
		info[i][profilecount] = ch;
	}
	profilecount++;

	f = fopen(name, "w");
	name[8] = 0;

	for (i=0 ; i<numlumps ; i++)
	{
		memcpy (name,lumpinfo[i].name,8);

		for (j=0 ; j<8 ; j++)
			if (!name[j])
				break;

		for ( ; j<8 ; j++)
			name[j] = ' ';

		fprintf (f,"%i %s %i ", i, name, lumpinfo[i].prev);

//		for (j=0 ; j<profilecount ; j++)
//			fprintf (f,"    %c",info[i][j]);

		fprintf (f,"\n");
	}
	fclose (f);
}
#endif

//==========================================================================
//
//  W_Shutdown
//
//==========================================================================

void W_Shutdown()
{
	guard(W_Shutdown);
	for (int i = 0; i < SearchPaths.Num(); i++)
	{
		delete SearchPaths[i];
	}
	SearchPaths.Clear();
	unguard;
}
