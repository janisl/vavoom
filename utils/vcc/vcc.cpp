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
//**	Copyright (C) 1999-2001 J∆nis Legzdi∑˝
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

#include <time.h>
#include "vcc.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void cpp_init(void);
size_t cpp_main(char *, void **);
void cpp_add_include(char *);
void cpp_add_define(int, char *);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void Init(void);
static void ProcessArgs(int ArgCount, char **ArgVector);
static void OpenDebugFile(char *name);
static void DumpAsm(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

bool			ClassAddfields = false;
char			SourceFileName[MAX_FILE_NAME_LENGTH];
static char		ObjectFileName[MAX_FILE_NAME_LENGTH];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int		num_dump_asm;
static char*	dump_asm_names[1024];
static boolean 	DebugMode;
static FILE*	DebugFile;
static bool		dumpPreprocessed = false;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	main
//
//==========================================================================

int main(int argc, char **argv)
{
	int starttime;
	int endtime;
	void *buf;
	size_t size;

	starttime = time(0);
	cpp_init();
	Init();
	ProcessArgs(argc, argv);

	dprintf("Preprocessing\n");
	size = cpp_main(SourceFileName, &buf);
	if (dumpPreprocessed)
	{
		FILE *f = fopen("vcc.i", "w");
		fwrite(buf, 1, size, f);
		fclose(f);
	}
	TK_OpenSource(buf, size);
	PA_Parse();
	TK_CloseSource();
	PC_WriteObject(ObjectFileName);
	ERR_RemoveErrorFile();
	DumpAsm();
	endtime = time(0);
	dprintf("Time elapsed: %02d:%02d\n",
		(endtime - starttime) / 60, (endtime - starttime) % 60);
	return 0;
}

//==========================================================================
//
// 	Init
//
//==========================================================================

static void Init(void)
{
	char		pvbuf[32];

	//	Izveido apstr∆des programmas komandlÿniju
	memset(pvbuf, 0, 32);
	sprintf(pvbuf, "PROG_VERSION=%d", PROG_VERSION);
	cpp_add_define('D', pvbuf);

	DebugMode = false;
	DebugFile = NULL;
	num_dump_asm = 0;
	TK_Init();
	PC_Init();
	InitTypes();
	InitInfoTables();
}

//==========================================================================
//
//	DisplayUsage
//
//==========================================================================

static void DisplayUsage(void)
{
	//	Druk∆ virsrakstu
	printf("\n");
	printf("VCC Version 1.%d. Copyright (c)2000-2001 by JL.          ("__DATE__" "__TIME__")\n", PROG_VERSION);
	printf("Usage: vcc [options] source[.c] [object[.dat]]\n");
	printf("    -d<file>     Output debugging information into specified file\n");
	printf("    -a<function> Output function's ASM statements into debug file\n");
	printf("    -D<name>[=<value>] Define macro\n");
	printf("    -U<name>           Unefine macro\n");
	printf("    -I<directory>      Include files directory\n");
	exit(1);
}

//==========================================================================
//
// 	ProcessArgs
//
//==========================================================================

static void ProcessArgs(int ArgCount, char **ArgVector)
{
	int		i;
	int		count;
	char*	text;
	char	option;

	count = 0;
	for (i = 1; i < ArgCount; i++)
	{
		text = ArgVector[i];
		if (*text == '-')
		{
			text++;
			if (*text == 0)
			{
				DisplayUsage();
			}
			option = *text++;
			switch (option)
			{
				case 'i':
					dumpPreprocessed = true;
					break;
				case 'c':
					if (*text == 'a' && !text[1])
					{
						ClassAddfields = true;
					}
					break;
				case 'd':
					DebugMode = true;
					if (*text)
					{
						OpenDebugFile(text);
					}
					break;
				case 'a':
					if (!*text)
					{
						DisplayUsage();
					}
					dump_asm_names[num_dump_asm++] = text;
					break;
				case 'I':
					cpp_add_include(text);
					break;
				case 'D':
				case 'U':
					cpp_add_define(option, text);
					break;
				default:
					DisplayUsage();
					break;
			}
			continue;
		}
		count++;
		switch(count)
		{
			case 1:
				strcpy(SourceFileName, text);
				DefaultExtension(SourceFileName, ".vc");
				break;
			case 2:
				strcpy(ObjectFileName, text);
				DefaultExtension(ObjectFileName, ".dat");
				break;
			default:
				DisplayUsage();
				break;
		}
	}
	if (count == 0)
	{
		DisplayUsage();
	}
	if (count == 1)
	{
		strcpy(ObjectFileName, SourceFileName);
		StripExtension(ObjectFileName);
		DefaultExtension(ObjectFileName, ".dat");
	}
	FixFileSlashes(SourceFileName);
	FixFileSlashes(ObjectFileName);
	dprintf("Main source file: %s\n", SourceFileName);
	dprintf("  Resulting file: %s\n", ObjectFileName);
}

//==========================================================================
//
// 	OpenDebugFile
//
//==========================================================================

static void OpenDebugFile(char *name)
{
	if (!(DebugFile = fopen(name, "w")))
	{
		ERR_Exit(ERR_CANT_OPEN_DBGFILE, false, "File: \"%s\".", name);
	}
}

//==========================================================================
//
//	DumpAsm
//
//==========================================================================

static void DumpAsm(void)
{
	int		i;

	for (i=0; i<num_dump_asm; i++)
	{
		PC_DumpAsm(dump_asm_names[i]);
	}
}

//==========================================================================
//
//	dprintf
//
//==========================================================================

int dprintf(const char *text, ...)
{
	FILE*		fp;
	va_list		argPtr;
	int			ret;

	if (!DebugMode)
	{
		return 0;
	}
	fp = DebugFile? DebugFile : stdout;
	va_start(argPtr, text);
	ret = vfprintf(fp, text, argPtr);
	va_end(argPtr);
	fflush(fp);
	return ret;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.10  2001/12/27 17:44:02  dj_jl
//	Removed support for C++ style constructors and destructors, some fixes
//
//	Revision 1.9  2001/12/12 19:22:22  dj_jl
//	Support for method usage as state functions, dynamic cast
//	Added dynamic arrays
//	
//	Revision 1.8  2001/12/01 18:17:09  dj_jl
//	Fixed calling of parent method, speedup
//	
//	Revision 1.7  2001/10/09 17:31:55  dj_jl
//	Addfields to class disabled by default
//	
//	Revision 1.6  2001/10/02 17:46:04  dj_jl
//	Preprocessing into a memory buffer
//	
//	Revision 1.5  2001/09/24 17:31:38  dj_jl
//	Some fixes
//	
//	Revision 1.4  2001/09/20 16:09:55  dj_jl
//	Added basic object-oriented support
//	
//	Revision 1.3  2001/08/21 17:52:54  dj_jl
//	Added support for real string pointers, beautification
//	
//	Revision 1.2  2001/07/27 14:27:56  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
