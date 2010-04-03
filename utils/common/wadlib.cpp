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

#include "cmdlib.h"
#include "wadlib.h"

namespace VavoomUtils {

#include "fwaddefs.h"

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
//	TIWadFile::Open
//
//==========================================================================

void TIWadFile::Open(const char* filename)
{
	wadinfo_t		header;
	lumpinfo_t*		lump_p;
	int				i;
	filelump_t*		fileinfo;
	filelump_t*		fi_p;

	handle = fopen(filename, "rb");
	if (!handle)
	{
		throw WadLibError(va("couldn't open %s", filename));
	}

	fread(&header, 1, sizeof(header), handle);
	if (strncmp(header.identification, "IWAD", 4))
	{
		// Homebrew levels?
		if (strncmp(header.identification, "PWAD", 4))
		{
			throw WadLibError(va("Wad file %s doesn't have IWAD or PWAD id",
				filename));
		}
	}
	strcpy(wadid, header.identification);

	numlumps = LittleLong(header.numlumps);
	lumpinfo = new lumpinfo_t[numlumps];
	fileinfo = new filelump_t[numlumps];
	fseek(handle, LittleLong(header.infotableofs), SEEK_SET);
	fread(fileinfo, 1, numlumps * sizeof(filelump_t), handle);

	// Fill in lumpinfo
	lump_p = lumpinfo;
	fi_p = fileinfo;
	for (i = 0; i < numlumps; i++, lump_p++, fi_p++)
	{
		CleanupName(fi_p->name, lump_p->name);
		lump_p->position = LittleLong(fi_p->filepos);
		lump_p->size = LittleLong(fi_p->size);
	}
	
	delete fileinfo;
}

//==========================================================================
//
//	TIWadFile::LumpNumForName
//
//==========================================================================

int TIWadFile::LumpNumForName(const char* name)
{
	int			i;
	char		buf[12];

	CleanupName(name, buf);
	for (i = numlumps - 1; i >= 0; i--)
	{
		if (!strcmp(buf, lumpinfo[i].name))
			return i;
	}
	throw WadLibError(va("W_GetNumForName: %s not found!", name));
}

//==========================================================================
//
//	TIWadFile::GetLump
//
//==========================================================================

void* TIWadFile::GetLump(int lump)
{
	void*		ptr;
	lumpinfo_t*	l;
	
	l = lumpinfo + lump;

	ptr = Z_Malloc(l->size);
	fseek(handle, l->position, SEEK_SET);
	fread(ptr, 1, l->size, handle);
	return ptr;
}

//==========================================================================
//
//	TIWadFile::Close
//
//==========================================================================

void TIWadFile::Close()
{
	fclose(handle);
	handle = NULL;
	delete lumpinfo;
}

//==========================================================================
//
//	TOWadFile::Open
//
//==========================================================================

void TOWadFile::Open(const char *filename, const char *Awadid)
{
	handle = fopen(filename, "wb");
	wadinfo_t	header;
	memset(&header, 0, sizeof(header));
	fwrite(&header, 1, sizeof(header), handle);
	lumpinfo = new lumpinfo_t[8 * 1024];
	strncpy(wadid, Awadid, 4);
	numlumps = 0;
}

//==========================================================================
//
//	TOWadFile::AddLump
//
//==========================================================================

void TOWadFile::AddLump(const char *name, const void *data, int size)
{
	CleanupName(name, lumpinfo[numlumps].name);
	lumpinfo[numlumps].size = size;
	lumpinfo[numlumps].position = ftell(handle);
	if (size)
	{
		fwrite(data, 1, size, handle);
	}
	numlumps++;
}

//==========================================================================
//
//	TOWadFile::Close
//
//==========================================================================

void TOWadFile::Close()
{
	wadinfo_t	header;
	strcpy(header.identification, wadid);
	header.numlumps = LittleLong(numlumps);
	header.infotableofs = LittleLong(ftell(handle));
	for (int i = 0; i < numlumps; i++)
	{
		filelump_t	fileinfo;
		strncpy(fileinfo.name, lumpinfo[i].name, 8);
		fileinfo.size = LittleLong(lumpinfo[i].size);
		fileinfo.filepos = LittleLong(lumpinfo[i].position);
		fwrite(&fileinfo, 1, sizeof(fileinfo), handle);
	}
	fseek(handle, 0, SEEK_SET);
	fwrite(&header, 1, sizeof(header), handle);
	fclose(handle);
	handle = NULL;
	delete lumpinfo;
}

} // namespace VavoomUtils
