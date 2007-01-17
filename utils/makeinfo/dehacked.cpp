//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id:$
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
//**
//**	Dehacked patch parsing
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include "info.h"
#include "makeinfo.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void FixupHeights();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern mobjinfo_t			mobjinfo[];

extern bool					Hacked;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char		*Patch;
static char		*PatchPtr;
static char 	*String;
static int 		value;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  GetLine
//
//==========================================================================

static bool GetLine()
{
	do
	{
		if (!*PatchPtr)
		{
			return false;
		}

		String = PatchPtr;

		while (*PatchPtr && *PatchPtr != '\n')
		{
			PatchPtr++;
		}
		if (*PatchPtr == '\n')
		{
			*PatchPtr = 0;
			PatchPtr++;
		}

		if (*String == '#')
		{
			*String = 0;
			continue;
		}

		while (*String && *String <= ' ')
		{
			String++;
		}
	} while (!*String);

	return true;
}

//==========================================================================
//
//  ParseParam
//
//==========================================================================

static bool ParseParam()
{
	char	*val;

	if (!GetLine())
	{
		return false;
	}

	val = strchr(String, '=');

	if (!val)
	{
		return false;
	}

	value = atoi(val + 1);

	do
	{
		*val = 0;
		val--;
	}
	while (val >= String && *val <= ' ');

	return true;
}

//==========================================================================
//
//  ReadThing
//
//==========================================================================

static void ReadThing(int num)
{
	num--; // begin at 0 not 1;
	if (num >= NUMMOBJTYPES || num < 0)
	{
		printf("WARNING! Invalid thing num %d\n", num);
		while (ParseParam());
		return;
	}

	while (ParseParam())
	{
		if (!strcmp(String ,"ID #"))	    			mobjinfo[num].doomednum   =value;
		else if (!strcmp(String, "Initial frame"))		mobjinfo[num].spawnstate  =value;
		else if (!strcmp(String, "Hit points"))	    	mobjinfo[num].spawnhealth =value;
		else if (!strcmp(String, "First moving frame"))	mobjinfo[num].seestate    =value;
		else if (!strcmp(String, "Alert sound"))	    mobjinfo[num].seesound    =value;
		else if (!strcmp(String, "Reaction time"))   	mobjinfo[num].reactiontime=value;
		else if (!strcmp(String, "Attack sound"))	    mobjinfo[num].attacksound =value;
		else if (!strcmp(String, "Injury frame"))	    mobjinfo[num].painstate   =value;
		else if (!strcmp(String, "Pain chance"))     	mobjinfo[num].painchance  =value;
		else if (!strcmp(String, "Pain sound")) 		mobjinfo[num].painsound   =value;
		else if (!strcmp(String, "Close attack frame"))	mobjinfo[num].meleestate  =value;
		else if (!strcmp(String, "Far attack frame"))	mobjinfo[num].missilestate=value;
		else if (!strcmp(String, "Death frame"))	    mobjinfo[num].deathstate  =value;
		else if (!strcmp(String, "Exploding frame"))	mobjinfo[num].xdeathstate =value;
		else if (!strcmp(String, "Death sound")) 		mobjinfo[num].deathsound  =value;
		else if (!strcmp(String, "Speed"))	    		mobjinfo[num].speed       =value;
		else if (!strcmp(String, "Width"))	    		mobjinfo[num].radius      =value;
		else if (!strcmp(String, "Height"))	    		mobjinfo[num].height      =value;
		else if (!strcmp(String, "Mass"))	    		mobjinfo[num].mass	      =value;
		else if (!strcmp(String, "Missile damage"))		mobjinfo[num].damage      =value;
		else if (!strcmp(String, "Action sound"))		mobjinfo[num].activesound =value;
		else if (!strcmp(String, "Bits"))	    		mobjinfo[num].flags       =value;
		else if (!strcmp(String, "Respawn frame"))		mobjinfo[num].raisestate  =value;
		else printf("WARNING! Invalid mobj param %s\n", String);
	}
}

//==========================================================================
//
//  ReadText
//
//==========================================================================

static void ReadText(int oldSize)
{
	char	*lenPtr;
	int		newSize;
	int		len;

	lenPtr = strtok(NULL, " ");
	if (!lenPtr)
	{
		return;
	}
	newSize = atoi(lenPtr);

	len = 0;
	while (*PatchPtr && len < oldSize)
	{
		if (*PatchPtr == '\r')
		{
			PatchPtr++;
			continue;
		}
		PatchPtr++;
		len++;
	}

	len = 0;
	while (*PatchPtr && len < newSize)
	{
		if (*PatchPtr == '\r')
		{
			PatchPtr++;
			continue;
		}
		PatchPtr++;
		len++;
	}

	GetLine();
}

//==========================================================================
//
//  LoadDehackedFile
//
//==========================================================================

static void LoadDehackedFile(char *filename)
{
	char*	Section;
	char*	numStr;
	int		i = 0;

	printf("Hacking %s\n", filename);

	FILE *f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);
	fseek(f, 0, SEEK_SET);
	Patch = (char*)malloc(len + 1);
	fread(Patch, 1, len, f);
	Patch[len] = 0;
	fclose(f);
	PatchPtr = Patch;

	GetLine();
	while (*PatchPtr)
	{
		Section = strtok(String, " ");
		if (!Section)
			continue;

		numStr = strtok(NULL, " ");
		if (numStr)
		{
			i = atoi(numStr);
		}

		if (!strcmp(Section, "Thing"))
		{
			ReadThing(i);
		}
		else if (!strcmp(Section, "Sound"))
		{
			while (ParseParam());
		}
		else if (!strcmp(Section, "Frame"))
		{
			while (ParseParam());
		}
		else if (!strcmp(Section, "Sprite"))
		{
			while (ParseParam());
		}
		else if (!strcmp(Section, "Ammo"))
		{
			while (ParseParam());
		}
		else if (!strcmp(Section, "Weapon"))
		{
			while (ParseParam());
		}
		else if (!strcmp(Section, "Pointer"))
		{
			while (ParseParam());
		}
		else if (!strcmp(Section, "Cheat"))
		{
			//	Old cheat handling is removed
			while (ParseParam());
		}
		else if (!strcmp(Section, "Misc"))
		{
			while (ParseParam());
		}
		else if (!strcmp(Section, "Text"))
		{
			ReadText(i);
		}
		else
		{
			printf("Don't know how to handle \"%s\"\n", String);
			GetLine();
		}
	}
	free(Patch);
}

//==========================================================================
//
//  ProcessDehackedFiles
//
//==========================================================================

void ProcessDehackedFiles(int argc, char **argv)
{
	int		p;

	for (p = 1; p < argc; p++)
		if (!stricmp(argv[p], "-deh"))
			break;
	if (p == argc)
	{
		FixupHeights();
		return;
	}

	Hacked = true;

	while (++p != argc && argv[p][0] != '-')
	{
		LoadDehackedFile(argv[p]);
	}
}
