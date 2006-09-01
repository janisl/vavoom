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

char			SourceFileName[MAX_FILE_NAME_LENGTH];
static char		ObjectFileName[MAX_FILE_NAME_LENGTH];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int		num_dump_asm;
static char*	dump_asm_names[1024];
static bool		DebugMode;
static FILE*	DebugFile;

static VLexer	Lex;
static VParser	Parser(Lex);

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
	Lex.OpenSource(buf, size);
	Parser.Parse();
	int parsetime = time(0);
	dprintf("Compiled in %02d:%02d\n",
		(parsetime - preptime) / 60, (parsetime - preptime) % 60);
	EmitCode();
	int compiletime = time(0);
	dprintf("Compiled in %02d:%02d\n",
		(compiletime - parsetime) / 60, (compiletime - parsetime) % 60);
	CurrentPackage->WriteObject(ObjectFileName);
	DumpAsm();
	VName::StaticExit();
	endtime = time(0);
	dprintf("Wrote in %02d:%02d\n",
		(endtime - compiletime) / 60, (endtime - compiletime) % 60);
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
		FatalError("%s:%d Segmentation Violation", Lex.Location.GetSource(),
			Lex.Location.GetLine());
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
	PC_Init();
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
		FatalError("Can\'t open debug file \"%s\".", name);
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
