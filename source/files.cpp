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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define MAXWADFILES 	20

// TYPES -------------------------------------------------------------------

struct search_path_t
{
	char			path[MAX_OSPATH];
	search_path_t	*next;
};

struct version_t
{
	Game_t		game;
    boolean		shareware;
    const char	*mainwad;
	const char	*wadfiles[MAXWADFILES];
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern TCvarI	shareware;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

char	fl_basedir[MAX_OSPATH];
char	fl_gamedir[MAX_OSPATH];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static search_path_t	*searchpaths;

const char	**wadfiles;
static version_t	games[] =
{
	{
		Doom,
        true,
        "doom1.wad",
		{
			"doom1.wad",
			"basev/doomdat.wad",
			"basev/jldoom.wad",
			NULL
		}
	},
	{
		Doom,
        false,
        "doom.wad",
		{
			"doom.wad",
			"basev/doomdat.wad",
			"basev/jldoom.wad",
			NULL
		}
	},
	{
		Doom,
        false,
        "doomu.wad",
		{
			"doomu.wad",
			"basev/doomdat.wad",
			"basev/jldoom.wad",
			NULL
		}
	},
	{
		Doom2,
        false,
        "doom2.wad",
		{
			"doom2.wad",
			"basev/doom2dat.wad",
			"basev/jldoom.wad",
		    "basev/d2progs.wad",
			NULL
		}
	},
	{
		Doom2,
        false,
        "doom2f.wad",
		{
			"doom2f.wad",
			"basev/doom2dat.wad",
			"basev/jldoom.wad",
		    "basev/d2progs.wad",
			NULL
		}
	},
	{
		Doom2,
        false,
        "tnt.wad",
		{
			"tnt.wad",
			"basev/tntdat.wad",
			"basev/jldoom.wad",
		    "basev/d2progs.wad",
			NULL
		}
	},
	{
		Doom2,
        false,
        "plutonia.wad",
		{
			"plutonia.wad",
			"basev/plutdat.wad",
			"basev/jldoom.wad",
		    "basev/d2progs.wad",
			NULL
		}
	},
	{
		Heretic,
        true,
        "heretic1.wad",
		{
		    "heretic1.wad",
		    "basev/hticdat.wad",
			NULL
		}
	},
	{
		Heretic,
        false,
        "heretic.wad",
		{
			"heretic.wad",
		    "basev/hticdat.wad",
			NULL
		}
	},
	{
		Hexen,
        false,
        "hexen.wad",
		{
			"hexen.wad",
		    "basev/hexendat.wad",
		    NULL
		}
	},
	{
		Strife,
        true,
        "strife0.wad",
		{
			"strife0.wad",
			"basev/strifdat.wad",
		    NULL
		}
	},
	{
		Strife,
        false,
        "strife1.wad",
		{
			"strife1.wad",
			"basev/strifdat.wad",
		    NULL
		}
	}
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	AddGameDir
//
//==========================================================================

static void AddGameDir(const char *dir)
{
	search_path_t	*info;

	info = (search_path_t*)Z_StrCalloc(sizeof(*info));
	strcpy(info->path, dir);
	info->next = searchpaths;
	searchpaths = info;

	strcpy(fl_gamedir, dir);
}

//==========================================================================
//
//	IdentifyVersion
//	Checks availability of IWAD files by name, to determine whether
// registered/commercial features should be executed (notably loading PWAD's).
//
//==========================================================================

static void IdentifyVersion (void)
{
	int		i;
	int		select_game;

    select_game = -1;
	if (M_CheckParm("-doom"))
    {
    	select_game = Doom;
    }
	if (M_CheckParm("-doom2"))
    {
    	select_game = Doom2;
    }
	if (M_CheckParm("-heretic"))
    {
    	select_game = Heretic;
    }
	if (M_CheckParm("-hexen"))
    {
    	select_game = Hexen;
    }
	if (M_CheckParm("-strife"))
    {
    	select_game = Strife;
    }

    for (i = (sizeof(games) / sizeof(games[0])) - 1; i >= 0; i--)
    {
    	if (select_game != -1 && games[i].game != select_game)
        {
        	continue;
		}
	    if (Sys_FileExists(games[i].mainwad))
	    {
	        Game = games[i].game;
	      	shareware = games[i].shareware;
			wadfiles = games[i].wadfiles;
	      	return;
	    }
    }

	if (select_game != -1)
		Sys_Error("Main wad file not found.");
	else
	    Sys_Error("Game mode indeterminate.");
}

//==========================================================================
//
//	FL_Init
//
//==========================================================================

void FL_Init(void)
{
	AddGameDir("basev");
	int p =	M_CheckParm("-game");
	if (p && p < myargc - 1)
	{
		AddGameDir(myargv[p + 1]);
	}

	IdentifyVersion();
}

//==========================================================================
//
//	FL_FindFile
//
//==========================================================================

bool FL_FindFile(const char *fname, char *dest)
{
	search_path_t	*search;
	char			tmp[MAX_OSPATH];

	for (search = searchpaths; search; search = search->next)
	{
		sprintf(tmp, "%s/%s", search->path, fname);
		if (Sys_FileExists(tmp))
		{
			if (dest)
			{
				strcpy(dest, tmp);
			}
			return true;
		}
	}
	return false;
}

//==========================================================================
//
//	FL_DefaultPath
//
//==========================================================================

void FL_DefaultPath(char *path, const char *basepath)
{
	char    temp[128];

	if (path[0] == '/')
	{
		return;                   // absolute path location
	}
	strcpy(temp, path);
	strcpy(path, basepath);
	strcat(path, temp);
}

//==========================================================================
//
//	FL_DefaultExtension
//
//==========================================================================

void FL_DefaultExtension(char *path, const char *extension)
{
	char    *src;

	//
	// if path doesn't have a .EXT, append extension
	// (extension should include the .)
	//
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
        {
			return;                 // it has an extension
		}
		src--;
	}

	strcat(path, extension);
}

//==========================================================================
//
//	FL_StripFilename
//
//==========================================================================

void FL_StripFilename(char *path)
{
	int             length;

	length = strlen(path)-1;
	while (length > 0 && path[length] != '/')
	{
		length--;
	}
	path[length] = 0;
}

//==========================================================================
//
//	FL_StripExtension
//
//==========================================================================

void FL_StripExtension(char *path)
{
	char *search;

	search = path + strlen(path) - 1;
	while (*search != '/' && search != path)
	{
		if (*search == '.')
		{
			*search = 0;
			return;
		}
		search--;
	}
}

//==========================================================================
//
//	FL_ExtractFilePath
//
//==========================================================================

void FL_ExtractFilePath(const char *path, char *dest)
{
	const char    *src;

	src = path + strlen(path) - 1;

	//
	// back up until a \ or the start
	//
	while (src != path && *(src-1) != '/')
		src--;

	memcpy(dest, path, src - path);
	dest[src - path] = 0;
}

//==========================================================================
//
//	FL_ExtractFileBase
//
//==========================================================================

void FL_ExtractFileBase(const char *path, char *dest)
{
#if 0
	const char    *src;

	src = path + strlen(path) - 1;

	//
	// back up until a \ or the start
	//
	while (src != path && *(src-1) != '/')
		src--;

	while (*src && *src != '.')
	{
		*dest++ = toupper(*src++);
	}
	*dest = 0;
#else
	int		i;
    int		length;

	i = strlen(path) - 1;
    
    // back up until a \ or the start
    while (i && path[i - 1] != '\\' && path[i - 1] != '/')
    {
		i--;
    }
    
    // copy up to eight characters
    memset(dest, 0, 8);
    length = 0;
    
    while (path[i] && path[i] != '.')
    {
		if (++length == 9)
	    	Sys_Error("Filename base of %s >8 chars", path);

		*dest++ = toupper((int)path[i]);
		i++;
    }
#endif
}

//==========================================================================
//
//	FL_ExtractFileExtension
//
//==========================================================================

void FL_ExtractFileExtension(const char *path, char *dest)
{
	const char    *src;

	src = path + strlen(path) - 1;

	//
	// back up until a . or the start
	//
	while (src != path && *(src-1) != '.')
		src--;
	if (src == path)
	{
		*dest = 0;	// no extension
		return;
	}

	strcpy(dest, src);
}

//==========================================================================
//
//  TFile::OpenRead
//
//==========================================================================

bool TFile::OpenRead(const char* filename)
{
	handle = Sys_FileOpenRead(filename);
	return handle != -1;
}

//==========================================================================
//
//  TFile::OpenWrite
//
//==========================================================================

bool TFile::OpenWrite(const char* filename)
{
	handle = Sys_FileOpenWrite(filename);
	return handle != -1;
}

//==========================================================================
//
//	TFile::Read
//
//==========================================================================

int TFile::Read(void* buf, int size)
{
	return Sys_FileRead(handle, buf, size);
}

//==========================================================================
//
//	TFile::Write
//
//==========================================================================

int TFile::Write(const void* buf, int size)
{
	return Sys_FileWrite(handle, buf, size);
}

//==========================================================================
//
//	TFile::Size
//
//==========================================================================

int TFile::Size(void)
{
	return Sys_FileSize(handle);
}

//==========================================================================
//
//	TFile::Seek
//
//==========================================================================

int TFile::Seek(int offset)
{
	return Sys_FileSeek(handle, offset);
}

//==========================================================================
//
//	TFile::Close
//
//==========================================================================

int TFile::Close(void)
{
	return Sys_FileClose(handle);
}


