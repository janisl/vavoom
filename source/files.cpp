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
    const char	*mainwad;
	const char	*gamedir;
	const char	*checkparm;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void SetupGameDir(const char *dirname);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

#ifdef DEVELOPER
bool	fl_devmode = true;
#else
bool	fl_devmode = false;
#endif
char	fl_basedir[MAX_OSPATH];
char	fl_gamedir[MAX_OSPATH];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static search_path_t	*searchpaths;

const char				*wadfiles[MAXWADFILES];
static version_t		games[] =
{
	{
		Doom,
        "doom1.wad",
		"basev/doom1",
		"-doom"
	},
	{
		Doom,
        "doom.wad",
		"basev/doom1",
		"-doom"
	},
	{
		Doom,
        "doomu.wad",
		"basev/doom1",
		"-doom"
	},
	{
		Doom2,
        "doom2.wad",
		"basev/doom2",
		"-doom2"
	},
	{
		Doom2,
        "doom2f.wad",
		"basev/doom2",
		"-doom2"
	},
	{
		Doom2,
        "tnt.wad",
		"basev/tnt",
		"-tnt"
	},
	{
		Doom2,
        "plutonia.wad",
		"basev/plutonia",
		"-plutonia"
	},
	{
		Heretic,
        "heretic1.wad",
		"basev/heretic",
		"-heretic"
	},
	{
		Heretic,
        "heretic.wad",
		"basev/heretic",
		"-heretic"
	},
	{
		Hexen,
        "hexen.wad",
		"basev/hexen",
		"-hexen"
	},
	{
		Strife,
        "strife0.wad",
		"basev/strife",
		"-strife"
	},
	{
		Strife,
        "strife1.wad",
		"basev/strife",
		"-strife"
	}
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	FL_AddFile
//
//==========================================================================

void FL_AddFile(const char *file)
{
    int     i;
    char    *newfile;

    i = 0;
    while (wadfiles[i])
	{
    	i++;
	}
    newfile = (char*)Z_Malloc(strlen(file) + 1, PU_STATIC, 0);
    strcpy(newfile, file);
    wadfiles[i] = newfile;
}

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

	for (int i = 0; i < 1024; i++)
	{
		char	buf[128];

		sprintf(buf, "%s/wad%d.wad", dir, i);
		if (!Sys_FileExists(buf))
			break;
		FL_AddFile(buf);
	}

	strcpy(fl_gamedir, dir);
}

//==========================================================================
//
//	ParseBase
//
//==========================================================================

static void ParseBase(const char *name)
{
	char		tmp[256];

	if (!Sys_FileExists(name))
	{
		return;
	}

	SC_OpenFile(name);
	SC_MustGetStringName("base");
	SC_MustGetString();
	strcpy(tmp, sc_String);
	SC_MustGetStringName("end");
	SC_Close();
	SetupGameDir(tmp);
}

//==========================================================================
//
//	SetupGameDir
//
//==========================================================================

static void SetupGameDir(const char *dirname)
{
	char		tmp[256];

	sprintf(tmp, "%s/base.txt", dirname);
	ParseBase(tmp);
	AddGameDir(dirname);
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
			FL_AddFile(games[i].mainwad);
			SetupGameDir(games[i].gamedir);
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

	IdentifyVersion();

	int p =	M_CheckParm("-game");
	if (p && p < myargc - 1)
	{
		SetupGameDir(myargv[p + 1]);
	}

	p =	M_CheckParm("-devgame");
	if (p && p < myargc - 1)
	{
		fl_devmode = true;
		SetupGameDir(myargv[p + 1]);
	}

	p = M_CheckParm("-file");
	if (p)
	{
		while (++p != myargc && myargv[p][0] != '-' && myargv[p][0] != '+')
		{
			FL_AddFile(myargv[p]);
		}
	}
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

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2001/08/21 17:40:54  dj_jl
//	Added devgame mode
//
//	Revision 1.4  2001/08/04 17:26:59  dj_jl
//	Removed shareware / ExtendedWAD from engine
//	Added support for script base.txt in game directory
//	
//	Revision 1.3  2001/07/31 17:08:37  dj_jl
//	Reworking filesystem
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
