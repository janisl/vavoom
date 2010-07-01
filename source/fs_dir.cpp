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

#include "gamedefs.h"
#include "fs_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VFilesDir::CheckNumForFileName
//
//==========================================================================

int VFilesDir::CheckNumForFileName(VStr Name)
{
	guard(VFilesDir::CheckNumForFileName);
	//	Check if the file exists.
	if (!Sys_FileExists(Path + "/" + Name))
	{
		return -1;
	}

	//	Add it to the cached files, if needed.
	for (int i = 0; i < CachedFiles.Num(); i++)
	{
		if (CachedFiles[i] == Name)
		{
			return i;
		}
	}
	return CachedFiles.Append(Name);
	unguard;
}

//==========================================================================
//
//	VFilesDir::FileExists
//
//==========================================================================

bool VFilesDir::FileExists(const VStr& fname)
{
	guard(VFilesDir::FileExists);
	if (Sys_FileExists(Path + "/" + fname))
	{
		return true;
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VFilesDir::OpenFileRead
//
//==========================================================================

VStream* VFilesDir::OpenFileRead(const VStr& Name)
{
	guard(FL_OpenFileRead);
	VStr TmpName = Path + "/" + Name;
	if (!Sys_FileExists(TmpName))
	{
		return NULL;
	}
	FILE *File = fopen(*TmpName, "rb");
	if (!File)
	{
		return NULL;
	}
	return new VStreamFileReader(File, GCon);
	unguard;
}

//==========================================================================
//
//	VFilesDir::ReadFromLump
//
//==========================================================================

void VFilesDir::ReadFromLump(int Lump, void* Dest, int Pos, int Size)
{
	guard(VFilesDir::ReadFromLump);
	check(Lump >= 0);
	check(Lump < CachedFiles.Num());
	VStream* Strm = CreateLumpReaderNum(Lump);
	check(Strm);
	Strm->Seek(Pos);
	Strm->Serialise(Dest, Size);
	delete Strm;
	Strm = NULL;
	unguard;
}

//==========================================================================
//
//	VFilesDir::LumpLength
//
//==========================================================================

int VFilesDir::LumpLength(int Lump)
{
	guard(VFilesDir::LumpLength);
	check(Lump >= 0);
	check(Lump < CachedFiles.Num());
	VStream* Strm = CreateLumpReaderNum(Lump);
	check(Strm);
	int Ret = Strm->TotalSize();
	delete Strm;
	Strm = NULL;
	return Ret;
	unguard;
}

//==========================================================================
//
//	VFilesDir::CreateLumpReaderNum
//
//==========================================================================

VStream* VFilesDir::CreateLumpReaderNum(int LumpNum)
{
	guard(VFilesDir::CreateLumpReaderNum);
	check(LumpNum >= 0);
	check(LumpNum < CachedFiles.Num());
	VStream* Strm = OpenFileRead(CachedFiles[LumpNum]);
	check(Strm);
	return Strm;
	unguard;
}

void VFilesDir::Close()
{
}
int VFilesDir::CheckNumForName(VName, EWadNamespace)
{
	return -1;
}
VName VFilesDir::LumpName(int)
{
	return NAME_None;
}
int VFilesDir::IterateNS(int, EWadNamespace)
{
	return -1;
}
void VFilesDir::BuildGLNodes(VSearchPath*)
{
	Sys_Error("BuildGLNodes on directory");
}
void VFilesDir::BuildPVS(VSearchPath*)
{
	Sys_Error("BuildPVS on directory");
}
void VFilesDir::RenameSprites(const TArray<VSpriteRename>&,
	const TArray<VLumpRename>&)
{
}
