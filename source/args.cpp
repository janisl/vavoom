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

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define MAXARGVS        100

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VArgs		GArgs;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VArgs::Init
//
//==========================================================================

void VArgs::Init(int argc, char** argv)
{
	guard(VArgs::Init);
	//	Save args
	Argc = argc;
	Argv = argv;

	FindResponseFile();
	unguard;
}

//==========================================================================
//
//	VArgs::FindResponseFile
//
// 	Find a Response File. We don't do this in DJGPP because it does this
// in startup code.
//
//==========================================================================

void VArgs::FindResponseFile()
{
	for (int i = 1;i < Argc;i++)
	{
		if (Argv[i][0] != '@')
		{
			continue;
		}

		//	Read the response file into memory
		FILE* handle = fopen(&Argv[i][1], "rb");
		if (!handle)
		{
			printf("\nNo such response file %s!", &Argv[i][1]);
			exit(1);
		}
		dprintf("Found response file %s!\n", &Argv[i][1]);
		fseek(handle, 0, SEEK_END);
		int size = ftell(handle);
		fseek(handle, 0, SEEK_SET);
		char *file = (char*)malloc(size + 1);
		fread(file, size, 1, handle);
		fclose(handle);
		file[size] = 0;

		//	Keep all other cmdline args
		char** oldargv = Argv;

		Argv = (char**)malloc(sizeof(char*) * MAXARGVS);
		memset(Argv, 0, sizeof(char*) * MAXARGVS);

		//	Keep args before response file
		int indexinfile;
		for (indexinfile = 0; indexinfile < i; indexinfile++)
		{
			Argv[indexinfile] = oldargv[indexinfile];
		}

		//	Read response file
		char *infile = file;
		int k = 0;
		while (k < size)
		{
			//	Skip whitespace.
			if (infile[k] <= ' ')
			{
				k++;
				continue;
			}

			if (infile[k] == '\"')
			{
				//	Parse quoted string.
				k++;
				Argv[indexinfile++] = infile + k;
				char CurChar;
				char* OutBuf = infile + k;
				do
				{
					CurChar = infile[k];
					if (CurChar == '\\' && infile[k + 1] == '\"')
					{
						CurChar = '\"';
						k++;
					}
					else if (CurChar == '\"')
					{
						CurChar = 0;
					}
					else if (CurChar == 0)
					{
						k--;
					}
					*OutBuf = CurChar;
					k++;
					OutBuf++;
				} while (CurChar);
				*(infile + k) = 0;
			}
			else
			{
				//	Parse unquoted string.
				Argv[indexinfile++] = infile + k;
				while (k < size && infile[k] > ' ' && infile[k] != '\"')
					k++;
				*(infile + k) = 0;
			}
		}

		//	Keep args following response file
		for (k = i + 1; k < Argc; k++)
			Argv[indexinfile++] = oldargv[k];
		Argc = indexinfile;

		//	Display args
	    dprintf("%d command-line args:\n", Argc);
		for (k = 1; k < Argc; k++)
			dprintf("%s\n", Argv[k]);
		i--;
	}
}

//==========================================================================
//
//  VArgs::CheckParm
//
//  Checks for the given parameter in the program's command line arguments.
//  Returns the argument number (1 to argc - 1) or 0 if not present
//
//==========================================================================

int VArgs::CheckParm(const char *check) const
{
	guard(VArgs::CheckParm);
	for (int i = 1; i < Argc; i++)
	{
		if (!stricmp(check, Argv[i]))
		{
			return i;
		}
	}
	return 0;
	unguard;
}

//==========================================================================
//
//  VArgs::CheckValue
//
//==========================================================================

const char* VArgs::CheckValue(const char *check) const
{
	guard(VArgs::CheckValue);
	int a = CheckParm(check);
	if (a && a < Argc - 1 && Argv[a + 1][0] != '-' && Argv[a + 1][0] != '+')
	{
		return Argv[a + 1];
	}
	return NULL;
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2006/04/08 12:14:25  dj_jl
//	Fixed value check.
//
//	Revision 1.1  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//	
//**************************************************************************
