//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	Copyright (C) 1999-2000 J∆nis Legzdi∑˝
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

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void Init(void);
static void ProcessArgs(int ArgCount, char **ArgVector);
static void OpenDebugFile(char *name);
static void DumpAsm(void);
static void Preprocess(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

char		SourceFileName[MAX_FILE_NAME_LENGTH];
char		ObjectFileName[MAX_FILE_NAME_LENGTH];
char		TempFileName[MAX_FILE_NAME_LENGTH];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int		num_dump_asm;
static char*	dump_asm_names[1024];
static bool		cpp_borland;
static char		cpp_params[2048];
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
	int		starttime;
	int		endtime;

	starttime = time(0);
	Init();
	ProcessArgs(argc, argv);

	Preprocess();
	TK_OpenSource(TempFileName);
	PA_Parse();
	TK_CloseSource();
	PC_WriteObject(ObjectFileName);
	ERR_RemoveErrorFile();
    DumpAsm();
	endtime = time(0);
	dprintf("Time elapsed: %2d:%2d\n",
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
	char*	tempdir;

	//	Izveido pagaidu faila v∆rdu
	tempdir = getenv("TEMP");
    if (!tempdir)
    {
    	tempdir = getenv("TMP");
		if (!tempdir)
        {
        	tempdir = ".";
		}
	}
	sprintf(TempFileName, "%s/vcc_temp.i", tempdir);

    //	Iznÿcina pagaidu failu, ja t∆ds jau eksistÒ
	remove(TempFileName);

	//	Izveido apstr∆des programmas komandlÿniju
	sprintf(cpp_params, "-DPROG_VERSION=%d", PROG_VERSION);

	DebugMode = false;
	DebugFile = NULL;
    num_dump_asm = 0;
	TK_Init();
    PC_Init();
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
	printf("    -b           Use Borland's preprocessor\n");
	printf("The following params are passed directly to the proprocessor:\n");
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
				case 'b':
					if (*text)
					{
						DisplayUsage();
					}
					cpp_borland = true;
					break;
				case 'D':
				case 'U':
                case 'I':
					strcat(cpp_params, " ");
                    strcat(cpp_params, ArgVector[i]);
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
	dprintf("Main source file: %s\n", SourceFileName);
    dprintf("  Resulting file: %s\n", ObjectFileName);
}

//==========================================================================
//
//	Preprocess
//
//==========================================================================

static void Preprocess(void)
{
	int		ret;
	char	cmd_line[2048];

	//	Izdod pazi∑ojumu
	dprintf("Preprocessing\n");

	//	Pievieno pie komandlÿnijas failu v∆rdus
	if (cpp_borland)
	{
		sprintf(cmd_line, "cpp32 -P- %s -o%s %s", cpp_params, TempFileName, SourceFileName);
	}
	else
	{
		sprintf(cmd_line, "gcc -E -x c++ %s %s -o %s", cpp_params, SourceFileName, TempFileName);
//		sprintf(cmd_line, "cpp %s %s -o %s", cpp_params, SourceFileName, TempFileName);
	}

	//	Izpilda komandu
	ret = system(cmd_line);

	if (ret)
    {
    	if (ret == -1)
        {
        	//	SistÒmas kı›da
        	perror("Preprocessor error");
        }
		exit(1);
    }
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

