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
//**	Handles WAD file header, directory, lump I/O.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "fwaddefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct lumpinfo_t
{
    char	name[12];
    int		handle;
    int		position;
    int		size;
	int		prev;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// Location of each lump on disk.
static lumpinfo_t*		lumpinfo;		
static int				numlumps;
static void**			lumpcache;
static int*				lumpindex;

static lumpinfo_t*		PrimaryLumpInfo;
static int 				PrimaryNumLumps;
static void**			PrimaryLumpCache;
static int*				PrimaryLumpIndex;

static lumpinfo_t*		AuxiliaryLumpInfo;
static int 				AuxiliaryNumLumps;
static void**			AuxiliaryLumpCache;
static int*				AuxiliaryLumpIndex;
static int				AuxiliaryHandle = 0;
static boolean 			AuxiliaryOpened = false;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	W_CleanupName
//
//==========================================================================

void W_CleanupName(const char *src, char *dst)
{
	int i;
	for (i = 0; i < 8 && src[i]; i++)
	{
		dst[i] = toupper(src[i]);
	}
	for (; i < 9; i++)
	{
		dst[i] = 0;
	}
}

//==========================================================================
//
//	W_AddFile
//
//	All files are optional, but at least one file must be found (PWAD, if
// all required lumps are present). Files with a .wad extension are wadlink
// files with multiple lumps. Other files are single lumps with the base
// filename for the lump name.
//
//==========================================================================

static void W_AddFile(const char *filename)
{
    wadinfo_t		header;
    lumpinfo_t*		lump_p;
    int				i;
    int				handle;
    int				length;
    int				startlump;
    filelump_t*		fileinfo;
    filelump_t		singleinfo;
	filelump_t*		fi_p = NULL;
	int				name_len;

    // open the file and add to directory

    if ((handle = Sys_FileOpenRead(filename)) == -1)
    {
		con << "couldn't open " << filename << endl;
		return;
    }

	con << "adding " << filename << endl;
    startlump = numlumps;

	name_len = strlen(filename);
    if (stricmp(filename + name_len - 3, "wad") &&
    	stricmp(filename + name_len - 3, "gwa"))
    {
		// single lump file
		fileinfo = &singleinfo;
		singleinfo.filepos = 0;
		singleinfo.size = LittleLong(Sys_FileSize(handle));
		FL_ExtractFileBase(filename, singleinfo.name);
		numlumps++;
	    Z_Resize((void**)&lumpinfo, numlumps * sizeof(lumpinfo_t));
		fi_p = NULL;
    }
    else 
    {
		// WAD file
		Sys_FileRead(handle, &header, sizeof(header));
		if (strncmp(header.identification, "IWAD", 4))
		{
	    	// Homebrew levels?
	    	if (strncmp(header.identification, "PWAD", 4))
	    	{
				Sys_Error ("Wad file %s doesn't have IWAD "
			 		"or PWAD id\n", filename);
	    	}
		}
		header.numlumps = LittleLong(header.numlumps);
		header.infotableofs = LittleLong(header.infotableofs);
		numlumps += header.numlumps;
        //	Moved here to make static data less fragmented
	    Z_Resize((void**)&lumpinfo, numlumps * sizeof(lumpinfo_t));
		length = header.numlumps * sizeof(filelump_t);
		fi_p = fileinfo = (filelump_t*)Z_Malloc(length, PU_STATIC, 0);
		Sys_FileSeek(handle, header.infotableofs);
		Sys_FileRead(handle, fileinfo, length);
    }

    // Fill in lumpinfo
    lump_p = &lumpinfo[startlump];
	
    for (i = startlump; i < numlumps; i++, lump_p++, fileinfo++)
    {
		W_CleanupName(fileinfo->name, lump_p->name);
		lump_p->handle = handle;
		lump_p->position = LittleLong(fileinfo->filepos);
		lump_p->size = LittleLong(fileinfo->size);
		lump_p->prev = lumpindex[(int)toupper(lump_p->name[0])];
		lumpindex[(int)toupper(lump_p->name[0])] = i;
    }
	
	if (fi_p)
	{
		Z_Free(fi_p);
	}

    if (!stricmp(filename + name_len - 3, "wad"))
	{
		char	gl_name[1024];

		strcpy(gl_name, filename);
		strcpy(gl_name + name_len - 3, "gwa");
		if (Sys_FileExists(gl_name))
		{
			W_AddFile(gl_name);
		}
	}
}

//==========================================================================
//
//	W_InitMultipleFiles
//
//	Pass a null terminated list of files to use. All files are optional,
// but at least one file must be found. Files with a .wad extension are
// idlink files with multiple lumps. Other files are single lumps with the
// base filename for the lump name. Lump names can appear multiple times.
// The name searcher looks backwards, so a later file does override all
// earlier ones.
//
//==========================================================================

void W_InitMultipleFiles(const char** filenames)
{	
    int		i;

    // open all the files, load headers, and count lumps
    numlumps = 0;

    lumpindex = (int*)Z_Malloc(1024, PU_STATIC, 0);
	for (i=0; i<256; i++)
       lumpindex[i] = -1;

    // will be realloced as lumps are added
    lumpinfo = (lumpinfo_t*)Z_Malloc(1, PU_STATIC, 0);

    for ( ; *filenames ; filenames++)
		W_AddFile(*filenames);

    if (!numlumps)
		Sys_Error ("W_InitFiles: no files found");
    
    // set up caching
    lumpcache = (void**)Z_Calloc(numlumps * sizeof(*lumpcache), PU_STATIC, 0);

	PrimaryLumpInfo = lumpinfo;
	PrimaryLumpCache = lumpcache;
	PrimaryNumLumps = numlumps;
    PrimaryLumpIndex = lumpindex;
}

//==========================================================================
//
//	W_InitFile
//
//	Just initialize from a single file.
//
//==========================================================================

void W_InitFile(const char* filename)
{
    const char*	names[2];

    names[0] = filename;
    names[1] = NULL;
    W_InitMultipleFiles(names);
}

//==========================================================================
//
//	W_NumLumps
//
//==========================================================================

int W_NumLumps(void)
{
    return numlumps;
}

//==========================================================================
//
//	W_OpenAuxiliary
//
//==========================================================================

void W_OpenAuxiliary(const char *filename)
{
	int 		i;
	int 		size;
	wadinfo_t 	header;
	int 		handle;
	int 		length;
	filelump_t*	fileinfo;
	filelump_t*	sourceLump;
	lumpinfo_t*	destLump;

	if (AuxiliaryOpened)
	{
		W_CloseAuxiliary();
	}
	if ((handle = Sys_FileOpenRead(filename)) == -1)
	{
		Host_Error("W_OpenAuxiliary: %s not found.", filename);
		return;
	}
	AuxiliaryHandle = handle;
	Sys_FileRead(handle, &header, sizeof(header));
	if (strncmp(header.identification, "IWAD", 4) &&
		strncmp(header.identification, "PWAD", 4))
	{
		// Bad file id
		Host_Error("Wad file %s doesn't have IWAD or PWAD id\n", filename);
	}
	header.numlumps = LittleLong(header.numlumps);
	header.infotableofs = LittleLong(header.infotableofs);
	length = header.numlumps*sizeof(filelump_t);
	fileinfo = (filelump_t*)Z_Malloc(length, PU_STATIC, 0);
	Sys_FileSeek(handle, header.infotableofs);
	Sys_FileRead(handle, fileinfo, length);
	numlumps = header.numlumps;

	// Init the auxiliary lumpinfo array
	lumpinfo = (lumpinfo_t*)Z_Malloc(numlumps*sizeof(lumpinfo_t), PU_STATIC, 0);
    lumpindex = (int*)Z_Malloc(1024, PU_STATIC, 0);

	for (i = 0; i < 256; i++)
       lumpindex[i] = -1;

	sourceLump = fileinfo;
	destLump = lumpinfo;
	for (i = 0; i < numlumps; i++, destLump++, sourceLump++)
	{
		W_CleanupName(sourceLump->name, destLump->name);
		destLump->handle = handle;
		destLump->position = LittleLong(sourceLump->filepos);
		destLump->size = LittleLong(sourceLump->size);
		destLump->prev = lumpindex[(int)toupper(destLump->name[0])];
		lumpindex[(int)toupper(destLump->name[0])] = i;
	}
	Z_Free(fileinfo);

	// Allocate the auxiliary lumpcache array
	size = numlumps * sizeof(*lumpcache);
	lumpcache = (void**)Z_Malloc(size, PU_STATIC, 0);
	memset(lumpcache, 0, size);

	AuxiliaryLumpInfo = lumpinfo;
	AuxiliaryLumpCache = lumpcache;
	AuxiliaryNumLumps = numlumps;
    AuxiliaryLumpIndex = lumpindex;
	AuxiliaryOpened = true;
}

//==========================================================================
//
//	W_CloseAuxiliary
//
//==========================================================================

void W_CloseAuxiliary(void)
{
	int i;

	if (AuxiliaryOpened)
	{
		W_UseAuxiliary();
		for (i = 0; i < numlumps; i++)
		{
			if (lumpcache[i])
			{
				Z_Free(lumpcache[i]);
			}
		}
		Z_Free(AuxiliaryLumpInfo);
		Z_Free(AuxiliaryLumpCache);
        Z_Free(AuxiliaryLumpIndex);
		W_CloseAuxiliaryFile();
		AuxiliaryOpened = false;
	}
	W_UsePrimary();
}

//==========================================================================
//
//	W_CloseAuxiliaryFile
//
//	WARNING: W_CloseAuxiliary() must be called before any further
// auxiliary lump processing.
//
//==========================================================================

void W_CloseAuxiliaryFile(void)
{
	if (AuxiliaryHandle)
	{
		Sys_FileClose(AuxiliaryHandle);
		AuxiliaryHandle = 0;
	}
}

//==========================================================================
//
//	W_UsePrimary
//
//==========================================================================

void W_UsePrimary(void)
{
	lumpinfo = PrimaryLumpInfo;
	numlumps = PrimaryNumLumps;
	lumpcache = PrimaryLumpCache;
    lumpindex = PrimaryLumpIndex;
}

//==========================================================================
//
//	W_UseAuxiliary
//
//==========================================================================

void W_UseAuxiliary(void)
{
	if (AuxiliaryOpened == false)
	{
		Sys_Error("W_UseAuxiliary: WAD not opened.");
	}
	lumpinfo = AuxiliaryLumpInfo;
	numlumps = AuxiliaryNumLumps;
	lumpcache = AuxiliaryLumpCache;
    lumpindex = AuxiliaryLumpIndex;
}

//==========================================================================
//
//	W_CheckNumForName
//
//	Returns -1 if name not found.
//
//==========================================================================

int W_CheckNumForName(const char* name)
{
	int			i;
	char		clean[12];

	W_CleanupName(name, clean);
    for (i = lumpindex[(int)toupper(name[0])]; i >= 0; i = lumpinfo[i].prev)
    {
		if (!strcmp(clean, lumpinfo[i].name))
		{
	    	return i;
		}
	}

    // Not found.
    return -1;
}

//==========================================================================
//
//	W_GetNumForName
//
//	Calls W_CheckNumForName, but bombs out if not found.
//
//==========================================================================

int W_GetNumForName(const char* name)
{
    int	i;

    i = W_CheckNumForName(name);
    
    if (i == -1)
    {
		Sys_Error("W_GetNumForName: %s not found!", name);
	}

    return i;
}

//==========================================================================
//
//	W_LumpLength
//
// 	Returns the buffer size needed to load the given lump.
//
//==========================================================================

int W_LumpLength(int lump)
{
    if ((dword)lump >= (dword)numlumps)
	{
		Sys_Error("W_LumpLength: %i >= numlumps",lump);
	}
    return lumpinfo[lump].size;
}

//==========================================================================
//
//	W_LumpName
//
//==========================================================================

const char *W_LumpName(int lump)
{
	static char empty_string[4] = "";
	if ((dword)lump >= (dword)numlumps)
	{
		return empty_string;
	}
	return lumpinfo[lump].name;
}

//==========================================================================
//
//	W_ReadLump
//
//	Loads the lump into the given buffer, which must be >= W_LumpLength().
//
//==========================================================================

void W_ReadLump(int lump, void* dest)
{
    int			c;
    lumpinfo_t*	l;
	
    if ((dword)lump >= (dword)numlumps)
		Sys_Error("W_ReadLump: %i >= numlumps",lump);

    l = lumpinfo + lump;
	
//	Sys_BeginRead();
    Sys_FileSeek(l->handle, l->position);
    c = Sys_FileRead(l->handle, dest, l->size);

    if (c < l->size)
		Sys_Error("W_ReadLump: only read %i of %i on lump %i",
			c, l->size, lump);

//	Sys_EndRead();
}

//==========================================================================
//
//	W_CacheLumpNum
//
//==========================================================================

void* W_CacheLumpNum(int lump, int tag)
{
    byte*	ptr;

    if ((unsigned)lump >= (unsigned)numlumps)
		Sys_Error("W_CacheLumpNum: %i >= numlumps",lump);
		
    if (!lumpcache[lump])
    {
		// read the lump in
		ptr = (byte*)Z_Malloc(W_LumpLength(lump) + 1, tag, &lumpcache[lump]);
		W_ReadLump(lump, lumpcache[lump]);
		ptr[W_LumpLength(lump)] = 0;
    }
    else
    {
		Z_ChangeTag(lumpcache[lump], tag);
    }
	
    return lumpcache[lump];
}

//==========================================================================
//
//	W_CacheLumpName
//
//==========================================================================

void* W_CacheLumpName(const char* name,int tag)
{
    return W_CacheLumpNum(W_GetNumForName(name), tag);
}

//==========================================================================
//
//	W_ForEachLump
//
//==========================================================================

bool W_ForEachLump(bool (*func)(int, const char*, int))
{
	for (int i = 0; i < numlumps; i++)
    {
		if (!func(i, lumpinfo[i].name, lumpinfo[i].size))
		{
			return false;
		}
	}
	return true;
}

//==========================================================================
//
//	W_Profile
//
//==========================================================================

#if 0
void W_Profile(void)
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
//	    	fprintf (f,"    %c",info[i][j]);

		fprintf (f,"\n");
    }
    fclose (f);
}
#endif
