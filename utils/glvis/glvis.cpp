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

#include <stdarg.h>
#include <iostream.h>
#include <time.h>
//	When compiling with -ansi isatty() is not declared
#if defined __unix__ && !defined __STRICT_ANSI__
#include <unistd.h>
#endif
#include "glvis.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class TConsoleGLVis:public TGLVis
{
 public:
	void DisplayMessage(const char *text, ...)
		__attribute__((format(printf, 2, 3)));
	void DisplayStartMap(const char *levelname);
	void DisplayBaseVisProgress(int count, int total);
	void DisplayPortalVisProgress(int count, int total);
	void DisplayMapDone(int accepts, int total);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

bool			silent_mode = false;
static bool		show_progress = true;

TConsoleGLVis	GLVis;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static const char	progress_chars[] = {'|', '/', '-', '\\'};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TConsoleGLVis::DisplayMessage
//
//==========================================================================

void TConsoleGLVis::DisplayMessage(const char *text, ...)
{
	va_list		args;

	if (!silent_mode)
	{
		va_start(args, text);
		vfprintf(stderr, text, args);
		va_end(args);
	}
}

//==========================================================================
//
//	TConsoleGLVis::DisplayStartMap
//
//==========================================================================

void TConsoleGLVis::DisplayStartMap(const char *)
{
	if (!silent_mode)
	{
		cerr << "Creating vis data ... ";
	}
}

//==========================================================================
//
//	TConsoleGLVis::DisplayBaseVisProgress
//
//==========================================================================

void TConsoleGLVis::DisplayBaseVisProgress(int count, int)
{
	if (show_progress && !(count & 0x1f))
	{
		fprintf(stderr, "%c\b", progress_chars[(count >> 5) & 3]);
	}
}

//==========================================================================
//
//	TConsoleGLVis::DisplayPortalVisProgress
//
//==========================================================================

void TConsoleGLVis::DisplayPortalVisProgress(int count, int total)
{
	if (show_progress && (!count || (total - count) % 10 == 0))
	{
		fprintf(stderr, "%05d\b\b\b\b\b", total - count);
	}
}

//==========================================================================
//
//	TConsoleGLVis::DisplayMapDone
//
//==========================================================================

void TConsoleGLVis::DisplayMapDone(int accepts, int total)
{
	if (!silent_mode)
	{
		cerr << accepts << " accepts, " << (total - accepts) << " rejects, "
			<< (accepts * 100 / total) << "%\n";
	}
}

//==========================================================================
//
//	ShowUsage
//
//==========================================================================

static void ShowUsage(void)
{
	cerr << "\nGLVIS version 1.3, Copyright (c)2000 JÆnis Legzdi·ý      ("__DATE__" "__TIME__")\n";
	cerr << "Usage: glvis [options] file[.wad]\n";
	cerr << "    -s            silent mode\n";
	cerr << "    -f            fast mode\n";
	cerr << "    -v            verbose mode\n";
	cerr << "    -t#           specify test level\n";
	cerr << "    -m<LEVELNAME> specifies a level to process, can be used multiple times\n";
	cerr << "    -noreject     don't create reject\n";
	exit(1);
}

//==========================================================================
//
//	main
//
//==========================================================================

int main(int argc, char *argv[])
{
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
				GLVis.fastvis = true;
				break;

			 case 'v':
				GLVis.verbose = true;
				break;

			 case 't':
				GLVis.testlevel = arg[2] - '0';
				break;

			 case 'm':
				strcpy(arg + 2, GLVis.specified_maps[GLVis.num_specified_maps++]);
				break;

			 case 'n':
				if (!strcmp(arg, "-noreject"))
				{
					GLVis.no_reject = true;
					break;
				}

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

	try
	{
		GLVis.Build(srcfile);
	}
	catch (GLVisError &e)
	{
		fprintf(stderr, "%s", e.message);
		return 1;
	}

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
//	Revision 1.7  2001/10/18 17:41:47  dj_jl
//	Added reject building
//
//	Revision 1.6  2001/09/20 16:38:05  dj_jl
//	Moved TGLVis out of namespace
//	
//	Revision 1.5  2001/09/12 17:28:38  dj_jl
//	Created glVIS plugin
//	
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
