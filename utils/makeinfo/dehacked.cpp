//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	Copyright (C) 1999-2000 JÆnis Legzdi·ý
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
#include "makeinfo.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern char*				sprnames[];
//extern char* 				statename[];
//extern char*				mt_names[];
extern char*				mobj_names[];
//extern char*				snd_names[];
//extern char* 				flagnames1[32];
//extern char* 				flagnames2[32];
//extern char*				ammo_names[];
//extern state_action_info_t	StateActionInfo[];
extern int					numstates;
extern int					nummobjtypes;
//extern int					numweapons;
extern int					num_sfx;
extern state_t				states[];
extern mobjinfo_t			mobjinfo[];
extern weaponinfo_t			weaponinfo[];
extern sfxinfo_t			sfx[];
extern string_def_t			strings[];
extern string_def_t			txtlumps[];
extern map_info_t			map_info[];
extern int					maxammo[];
extern int					perammo[];
extern int					numammo;
extern int					initial_health;
extern int					initial_ammo;
extern int					bfg_cells;
extern int					soulsphere_max;
extern int					soulsphere_health;
extern int					megasphere_health;
extern int					god_health;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

//static int		maxammo;
//static int		clipammo;
//static int		num_states;
//static int		nummobjtypes;
static int*		functions;

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

static bool GetLine(void)
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

static bool ParseParam(void)
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
	if (num >= nummobjtypes || num < 0)
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
//  ReadSound
//
//==========================================================================

static void ReadSound(int num)
{
	while (ParseParam())
    {
    	if (!strcmp(String, "Offset"));				//Lump name offset - can't handle
    	else if (!strcmp(String, "Zero/One"));		//Singularity - removed
    	else if (!strcmp(String, "Value"))			sfx[num].priority = value;
    	else if (!strcmp(String, "Zero 1"));        //Lump num - can't be set
    	else if (!strcmp(String, "Zero 2"));        //Data pointer - can't be set
    	else if (!strcmp(String, "Zero 3"));		//Usefulness - removed
    	else if (!strcmp(String, "Zero 4"));        //Link - removed
    	else if (!strcmp(String, "Neg. One 1"));    //Link pitch - removed
    	else if (!strcmp(String, "Neg. One 2"));    //Link volume - removed
		else printf("WARNING! Invalid sound param %s\n", String);
    }
}

//==========================================================================
//
//  ReadState
//
//==========================================================================

static void ReadState(int num)
{
	if (num >= numstates || num < 0)
    {
    	printf("WARNING! Invalid state num %d\n", num);
		while (ParseParam());
        return;
    }

   	while (ParseParam())
	{
   		if (!strcmp(String, "Sprite number"))     		states[num].sprite    = value;
		else if (!strcmp(String, "Sprite subnumber"))	states[num].frame	  = value;
   		else if (!strcmp(String, "Duration"))    		states[num].tics	  = value;
   		else if (!strcmp(String, "Next frame"))    		states[num].nextstate = value;
   		else if (!strcmp(String, "Unknown 1"))    		states[num].misc1 	  = value;
   		else if (!strcmp(String, "Unknown 2"))    		states[num].misc2 	  = value;
   		else if (!strcmp(String, "Action pointer"))     printf("WARNING! Tried to set action pointer.\n");
		else printf("WARNING! Invalid state param %s\n", String);
	}
}

//==========================================================================
//
//  ReadSpriteName
//
//==========================================================================

static void ReadSpriteName(int)
{
    while (ParseParam())
	{
		if (!stricmp(String, "Offset"));	//	Can't handle
		else printf("WARNING! Invalid sprite name param %s\n", String);
	}
}

//==========================================================================
//
//  ReadAmmo
//
//==========================================================================

static void ReadAmmo(int num)
{
	while (ParseParam())
	{
   		if (!stricmp(String, "Max ammo"))		maxammo[num] = value;
   		else if (!stricmp(String, "Per ammo"))	perammo[num] = value;
		else printf("WARNING! Invalid ammo param %s\n", String);
	}
}

//==========================================================================
//
//  ReadWeapon
//
//==========================================================================

static void ReadWeapon(int num)
{
	while (ParseParam())
	{
   		if (!stricmp(String, "Ammo type"))				weaponinfo[num].ammo 		= value;
   		else if (!stricmp(String, "Deselect frame"))	weaponinfo[num].upstate		= value;
   		else if (!stricmp(String, "Select frame"))  	weaponinfo[num].downstate 	= value;
   		else if (!stricmp(String, "Bobbing frame")) 	weaponinfo[num].readystate	= value;
   		else if (!stricmp(String, "Shooting frame"))	weaponinfo[num].atkstate 	= value;
   		else if (!stricmp(String, "Firing frame"))  	weaponinfo[num].flashstate	= value;
		else printf("WARNING! Invalid weapon param %s\n", String);
	}
}

//==========================================================================
//
//  ReadPointer
//
//==========================================================================

static void ReadPointer(int num)
{
	int		statenum = -1;
    int		i;
    int		j;

	for (i=0, j=0; i < numstates; i++)
	{
		if (functions[i])
		{
			if (j == num)
			{
				statenum = i;
				break;
			}
			j++;
		}
	}

	if (statenum == -1)
	{
    	printf("WARNING! Invalid pointer\n");
		while (ParseParam());
		return;
	}

    while (ParseParam())
	{
		if (!stricmp(String, "Codep Frame"))	states[statenum].action_num = functions[value];
		else printf("WARNING! Invalid pointer param %s\n", String);
	}
}

//==========================================================================
//
//  ReadCheats
//
//==========================================================================

static void ReadCheats(int)
{
	//	Old cheat handling is removed
    while (ParseParam());
}

//==========================================================================
//
//  ReadMisc
//
//==========================================================================

static void ReadMisc(int)
{
	//	Not handled yet
    while (ParseParam())
	{
		if (!stricmp(String, "Initial Health"))			initial_health = value;
   		else if (!stricmp(String, "Initial Bullets"))	initial_ammo = value;
   		else if (!stricmp(String, "Max Health"));
   		else if (!stricmp(String, "Max Armor"));
   		else if (!stricmp(String, "Green Armor Class"));
   		else if (!stricmp(String, "Blue Armor Class"));
   		else if (!stricmp(String, "Max Soulsphere"))	soulsphere_max = value;
   		else if (!stricmp(String, "Soulsphere Health"))	soulsphere_health = value;
   		else if (!stricmp(String, "Megasphere Health"))	megasphere_health = value;
   		else if (!stricmp(String, "God Mode Health"))   god_health = value;
   		else if (!stricmp(String, "IDFA Armor"));		//	Cheat removed
   		else if (!stricmp(String, "IDFA Armor Class"));	//	Cheat removed
   		else if (!stricmp(String, "IDKFA Armor"));		//	Cheat removed
   		else if (!stricmp(String, "IDKFA Armor Class"));//	Cheat removed
   		else if (!stricmp(String, "BFG Cells/Shot"))	bfg_cells = value;
   		else if (!stricmp(String, "Monsters Infight"));	//	What's that?
		else printf("WARNING! Invalid misc %s\n", String);
	}
}

//==========================================================================
//
//	FindString
//
//==========================================================================

static void FindString(char *oldStr, char *newStr)
{
	bool	SoundFound = false;
	int		i;

	for (i = 0; i < num_sfx; i++)
	{
		if (!strcmp(sfx[i].lumpname + 2, oldStr))
        {
//			printf("Sound name, old \"%s\" new \"%s\"\n", oldStr, newStr);
			strcpy(sfx[i].lumpname + 2, newStr);
            SoundFound = true;	//Continue, because other sounds can use the same sound
        }
    }
	if (SoundFound)
    {
    	return;
    }

	for (i = 0; mobj_names[i]; i++)
    {
    	if (!strcmp(mobj_names[i], oldStr))
        {
//			printf("Mobj name, old \"%s\" new \"%s\"\n", oldStr, newStr);
    		strcpy(mobj_names[i], newStr);
            return;
        }
    }

	for (i = 0; sprnames[i]; i++)
    {
    	if (!strcmp(sprnames[i], oldStr))
        {
//			printf("Sprite name, old \"%s\" new \"%s\"\n", oldStr, newStr);
    		strcpy(sprnames[i], newStr);
            return;
        }
    }

    for (i = 0; strings[i].macro; i++)
    {
    	if (!strcmp(strings[i].def_val, oldStr))
        {
//			printf("String %s, old \"%s\" new \"%s\"\n", strings[i].macro, oldStr, newStr);
			if (strings[i].new_val)
            	free(strings[i].new_val);
            strings[i].new_val = (char*)malloc(strlen(newStr) + 1);
    		strcpy(strings[i].new_val, newStr);
			return;
        }
    }

    for (i = 0; txtlumps[i].macro; i++)
    {
    	if (!strcmp(txtlumps[i].def_val, oldStr))
        {
//			printf("Lump %s, old \"%s\" new \"%s\"\n", txtlumps[i].macro, oldStr, newStr);
			if (txtlumps[i].new_val)
            	free(txtlumps[i].new_val);
            txtlumps[i].new_val = (char*)malloc(strlen(newStr) + 1);
    		strcpy(txtlumps[i].new_val, newStr);
			return;
        }
    }

#ifdef DOOM2
    if (map_info[0].name[0] && !strncmp(oldStr, "level", 5))
    {
    	for (i = 0; i < 32; i++)
        {
        	if (!strcmp(map_info[i].name, oldStr + 9) ||
        		!strcmp(map_info[i].name, oldStr + 10))
	        {
//				printf("Map name, old \"%s\" new \"%s\"\n", oldStr, newStr);
				while (newStr[0] && newStr[strlen(newStr) - 1] == ' ')
					newStr[strlen(newStr) - 1] = 0;
				if (oldStr[8] == ':' && !strnicmp(oldStr, newStr, 9))
				{
		        	newStr += 9;
				}
				else if (oldStr[7] == ':' && !strnicmp(oldStr, newStr, 8))
				{
		        	newStr += 8;
				}
				while (newStr[0] == ' ')
					newStr++;
	        	strcpy(map_info[i].name, newStr);
	            return;
    	    }
        }
    }
#else
    if (map_info[0].name[0] && oldStr[0] == 'E' && oldStr[2] == 'M' && oldStr[4] == ':')
    {
    	for (i = 0; i < 32; i++)
        {
        	if (!strcmp(map_info[i].name, oldStr + 6))
	        {
				while (newStr[0] && newStr[strlen(newStr) - 1] == ' ')
					newStr[strlen(newStr) - 1] = 0;
				if (oldStr[4] == ':' && !strnicmp(oldStr, newStr, 5))
				{
		        	newStr += 5;
				}
				while (newStr[0] == ' ')
					newStr++;
	        	strcpy(map_info[i].name, newStr);
	            return;
    	    }
        }
    }
#endif

    if (map_info[0].name[0])
    {
    	for (i = 0; i < 32; i++)
        {
        	if (!strcmp(map_info[i].song + 2, oldStr))
	        {
//				printf("Song name, old \"%s\" new \"%s\"\n", oldStr, newStr);
	        	strcpy(map_info[i].song + 2, newStr);
	            return;
    	    }
        }
    }

	printf("Not found old \"%s\" new \"%s\"\n", oldStr, newStr);
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
	char	*oldStr;
    char	*newStr;
	int		len;

	lenPtr = strtok(NULL, " ");
	if (!lenPtr)
    {
    	return;
    }
	newSize = atoi(lenPtr);

	oldStr = (char*)calloc(oldSize + 1, 1);
	newStr = (char*)calloc(newSize + 1, 1);

	len = 0;
	while (*PatchPtr && len < oldSize)
    {
    	if (*PatchPtr == '\r')
        {
        	PatchPtr++;
        	continue;
		}
		oldStr[len] = *PatchPtr;
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
		newStr[len] = *PatchPtr;
       	PatchPtr++;
        len++;
    }

	FindString(oldStr, newStr);

	free(oldStr);
    free(newStr);

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
           	ReadSound(i);
		}
		else if (!strcmp(Section, "Frame"))
		{
           	ReadState(i);
		}
		else if (!strcmp(Section, "Sprite"))
		{
           	ReadSpriteName(i);
		}
		else if (!strcmp(Section, "Ammo"))
		{
			ReadAmmo(i);
		}
		else if (!strcmp(Section, "Weapon"))
		{
			ReadWeapon(i);
		}
		else if (!strcmp(Section, "Pointer"))
		{
           	ReadPointer(i);
		}
		else if (!strcmp(Section, "Cheat"))
		{
           	ReadCheats(i);
		}
		else if (!strcmp(Section, "Misc"))
		{
           	ReadMisc(i);
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
	int		i;

	for (p = 1; p < argc; p++)
    	if (!stricmp(argv[p], "-deh"))
        	break;
	if (p == argc)
    {
    	return;
    }

    functions = (int*)malloc(numstates * 4);
	for (i = 0; i < numstates; i++)
		functions[i] = states[i].action_num;

	while (++p != argc && argv[p][0] != '-')
    {
    	LoadDehackedFile(argv[p]);
	}

	free(functions);
}
