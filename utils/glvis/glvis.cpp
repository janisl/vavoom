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
//**
//**	Potentially Visible Set
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <time.h>
//	When compiling with -ansi isatty() is not declared
#if defined __unix__ && !defined __STRICT_ANSI__
#include <unistd.h>
#endif
#include "glvis.h"

// MACROS ------------------------------------------------------------------

#define TEMP_FILE	"$glvis$$.$$$"

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

TIWadFile		inwad;
TIWadFile		gwa;
TOWadFile		outwad;

TIWadFile		*mainwad;
TIWadFile		*glwad;

bool			silent_mode = false;
bool			show_progress = true;
bool			fastvis = false;
bool			verbose = false;

int				testlevel = 2;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int		num_specified_maps = 0;
static char		specified_maps[100][16];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	IsLevelName
//
//==========================================================================

static bool	IsLevelName(int lump)
{
	if (lump + 4 >= glwad->numlumps)
	{
		return false;
	}

	const char	*name = glwad->LumpName(lump);

	if (name[0] != 'G' || name[1] != 'L' || name[2] != '_')
	{
		return false;
	}

	if (num_specified_maps)
	{
		for (int i = 0; i < num_specified_maps; i++)
		{
			if (!strcmp(specified_maps[i], name + 3))
			{
				return true;
			}
		}
	}
	else
	{
		if (!strcmp(glwad->LumpName(lump + 1), "GL_VERT") &&
			!strcmp(glwad->LumpName(lump + 2), "GL_SEGS") &&
			!strcmp(glwad->LumpName(lump + 3), "GL_SSECT") &&
			!strcmp(glwad->LumpName(lump + 4), "GL_NODES"))
		{
			return true;
		}
	}
	return false;
}

//==========================================================================
//
//	ShowUsage
//
//==========================================================================

static void ShowUsage(void)
{
	cerr << "\nGLVIS version 1.2, Copyright (c)2000 JÆnis Legzdi·ý      ("__DATE__" "__TIME__")\n";
	cerr << "Usage: glvis [options] file[.wad]\n";
	cerr << "    -s            silent mode\n";
	cerr << "    -f            fast mode\n";
	cerr << "    -v            verbose mode\n";
	cerr << "    -t#           specify test level\n";
	cerr << "    -m<LEVELNAME> specifies a level to process, can be used multiple times\n";
	exit(1);
}

//==========================================================================
//
//	main
//
//==========================================================================

int main(int argc, char *argv[])
{
	char filename[1024];
	char destfile[1024];
	char bakext[8];
	char *srcfile = NULL;
	int i;

	int starttime = time(0);

	for (i = 1; i < argc; i++)
	{
		char *arg = argv[i];
		if (*arg == '-')
		{
			switch (arg[1])
			{
			 case 's':
				silent_mode = true;
				break;

			 case 'f':
				fastvis = true;
				break;

			 case 'v':
				verbose = true;
				break;

			 case 't':
				testlevel = arg[2] - '0';
				break;

			 case 'm':
				CleanupName(arg + 2, specified_maps[num_specified_maps++]);
				break;

			 default:
				ShowUsage();
			}
		}
		else
		{
			if (srcfile)
			{
				ShowUsage();
			}
			srcfile = arg;
		}
	}

	if (!srcfile)
	{
		ShowUsage();
	}

	show_progress = !silent_mode;
#if defined __unix__ && !defined __STRICT_ANSI__
	// Unix: no whirling baton if stderr is redirected
	if (!isatty(2))
		show_progress = false;
#endif

	strcpy(filename, srcfile);
	DefaultExtension(filename, ".wad");
	strcpy(destfile, filename);
	inwad.Open(filename);
	mainwad = &inwad;

	StripExtension(filename);
	strcat(filename, ".gwa");
	FILE *ff = fopen(filename, "rb");
	if (ff)
	{
		fclose(ff);
		gwa.Open(filename);
		glwad = &gwa;
		strcpy(destfile, filename);
		strcpy(bakext, ".~gw");
	}
	else
	{
		glwad = &inwad;
		strcpy(bakext, ".~wa");
	}

	outwad.Open(TEMP_FILE, glwad->wadid);

	//	Process lumps
	i = 0;
	while (i < glwad->numlumps)
	{
		void *ptr =	glwad->GetLump(i);
		const char *name = glwad->LumpName(i);
		outwad.AddLump(name, ptr, glwad->LumpSize(i));
		Free(ptr);
		if (IsLevelName(i))
		{
			LoadLevel(mainwad->LumpNumForName(name + 3), i);
			i += 5;
			if (!strcmp("GL_PVS", glwad->LumpName(i)))
			{
				i++;
			}
			BuildPVS();
			FreeLevel();
		}
		else
		{
			i++;
		}
	}

	inwad.Close();
	if (gwa.handle)
	{
		gwa.Close();
	}
	outwad.Close();

	strcpy(filename, destfile);
	StripExtension(filename);
	strcat(filename, bakext);
	remove(filename);
	rename(destfile, filename);
	rename(TEMP_FILE, destfile);

	if (!silent_mode)
	{
		int worktime = time(0) - starttime;
		fprintf(stderr, "Time elapsed: %d:%02d:%02d\n", worktime / 3600, (worktime / 60) % 60, worktime % 60);
	}

	return 0;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2001/08/30 17:47:47  dj_jl
//	Overflow protection
//
//	Revision 1.3  2001/08/24 17:09:22  dj_jl
//	Recognizes maps by checking GL lump names
//	
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
