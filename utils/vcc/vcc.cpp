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

// HEADER FILES ------------------------------------------------------------

#include <time.h>
#include <signal.h>
#include "vcc.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void cpp_init();
size_t cpp_main(char*, void**);
void cpp_add_include(char*);
void cpp_add_define(int, char*);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void SignalHandler(int s);
static void Init();
static void ProcessArgs(int ArgCount, char **ArgVector);
static void OpenDebugFile(char *name);
static void DumpAsm();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int				CurrentPass;

char			SourceFileName[MAX_FILE_NAME_LENGTH];
static char		ObjectFileName[MAX_FILE_NAME_LENGTH];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int		num_dump_asm;
static char*	dump_asm_names[1024];
static boolean 	DebugMode;
static FILE*	DebugFile;

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

	signal(SIGSEGV, SignalHandler);

	starttime = time(0);
	cpp_init();
	Init();
	ProcessArgs(argc, argv);

	dprintf("Preprocessing\n");
	size = cpp_main(SourceFileName, &buf);
	int preptime = time(0);
	dprintf("Preprocessing in %02d:%02d\n",
		(preptime - starttime) / 60, (preptime - starttime) % 60);
	TK_OpenSource(buf, size);
	CurrentPass = 1;
	PA_Parse();
	TK_Restart();
	int pass1time = time(0);
	dprintf("Pass 1 in %02d:%02d\n",
		(pass1time - preptime) / 60, (pass1time - preptime) % 60);
	CurrentPass = 2;
	PA_Compile();
	int pass2time = time(0);
	dprintf("Pass 2 in %02d:%02d\n",
		(pass2time - pass1time) / 60, (pass2time - pass1time) % 60);
	TK_CloseSource();
	PC_WriteObject(ObjectFileName);
	ERR_RemoveErrorFile();
	DumpAsm();
	VName::StaticExit();
	endtime = time(0);
	dprintf("Wrote in %02d:%02d\n",
		(endtime - pass2time) / 60, (endtime - pass2time) % 60);
	dprintf("Time elapsed: %02d:%02d\n",
		(endtime - starttime) / 60, (endtime - starttime) % 60);
	return 0;
}

//==========================================================================
//
// 	signal_handler
//
// 	Shuts down system, on error signal
//
//==========================================================================

static void SignalHandler(int s)
{
	switch (s)
	{
	case SIGSEGV:
		ERR_Exit(ERR_NONE, true, "Segmentation Violation");
	}
}

//==========================================================================
//
// 	Init
//
//==========================================================================

static void Init()
{
	char		pvbuf[32];

	//	Add define of the progs version.
	memset(pvbuf, 0, 32);
	sprintf(pvbuf, "PROG_VERSION=%d", PROG_VERSION);
	cpp_add_define('D', pvbuf);

	DebugMode = false;
	DebugFile = NULL;
	num_dump_asm = 0;
	VName::StaticInit();
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

static void DisplayUsage()
{
	//	Print usage.
	printf("\n");
	printf("VCC Version 1.%d. Copyright (c)2000-2001 by JL.          ("__DATE__" "__TIME__")\n", PROG_VERSION);
	printf("Usage: vcc [options] source[.c] [object[.dat]]\n");
	printf("    -d<file>     Output debugging information into specified file\n");
	printf("    -a<function> Output function's ASM statements into debug file\n");
	printf("    -D<name>[=<value>] Define macro\n");
	printf("    -U<name>           Unefine macro\n");
	printf("    -I<directory>      Include files directory\n");
	printf("    -P<directory>      Package import files directory\n");
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
			case 'P':
				AddPackagePath(text);
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
	if (!DebugFile)
	{
		char DbgFileName[MAX_FILE_NAME_LENGTH];
		strcpy(DbgFileName, ObjectFileName);
		StripExtension(DbgFileName);
		DefaultExtension(DbgFileName, ".txt");
		OpenDebugFile(DbgFileName);
		DebugMode = true;
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

static void DumpAsm()
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
//	Revision 1.19  2006/03/26 13:06:49  dj_jl
//	Implemented support for modular progs.
//
//	Revision 1.18  2006/02/27 21:23:55  dj_jl
//	Rewrote names class.
//	
//	Revision 1.17  2005/11/29 19:31:43  dj_jl
//	Class and struct classes, removed namespaces, beautification.
//	
//	Revision 1.16  2005/11/24 20:42:05  dj_jl
//	Renamed opcodes, cleanup and improvements.
//	
//	Revision 1.15  2003/03/08 12:47:52  dj_jl
//	Code cleanup.
//	
//	Revision 1.14  2002/11/02 17:11:13  dj_jl
//	New style classes.
//	
//	Revision 1.13  2002/08/24 14:45:38  dj_jl
//	2 pass compiling.
//	
//	Revision 1.12  2002/01/11 08:17:31  dj_jl
//	Added name subsystem, removed support for unsigned ints
//	
//	Revision 1.11  2002/01/07 12:31:36  dj_jl
//	Changed copyright year
//	
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
