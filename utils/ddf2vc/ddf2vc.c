//---------------------------------------------------------------------------
//  EDGE Main Init + Program Loop Code
//----------------------------------------------------------------------------
// 
//  Copyright (c) 1999-2001  The EDGE Team.
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//----------------------------------------------------------------------------
//
//  Based on the DOOM source code, released by Id Software under the
//  following copyright:
//
//    Copyright (C) 1993-1996 by id Software, Inc.
//
//----------------------------------------------------------------------------
//
// DESCRIPTION:
//      EDGE main program (E_EDGEMain),
//      game loop (E_EDGELoop) and startup functions.
//
// -MH- 1998/07/02 "shootupdown" --> "true3dgameplay"
// -MH- 1998/08/19 added up/down movement variables
//

#include "i_defs.h"

#include "m_misc.h"
#include "m_argv.h"
#include "z_zone.h"
#include "vc.h"

const char *chat_macros[10];
const char *destination_keys;
const char *gammamsg[5];

const mapstuff_t *currentmap;

char *gamedir;
char *ddfdir;
char *progsdir;
char *scriptsdir;

default_t defaults[] =
{
    {"chatmacro0", (int *)&chat_macros[0], 0},  //(int) HUSTR_CHATMACRO0 },
    {"chatmacro1", (int *)&chat_macros[1], 0},  //(int) HUSTR_CHATMACRO1 },
    {"chatmacro2", (int *)&chat_macros[2], 0},  //(int) HUSTR_CHATMACRO2 },
    {"chatmacro3", (int *)&chat_macros[3], 0},  //(int) HUSTR_CHATMACRO3 },
    {"chatmacro4", (int *)&chat_macros[4], 0},  //(int) HUSTR_CHATMACRO4 },
    {"chatmacro5", (int *)&chat_macros[5], 0},  //(int) HUSTR_CHATMACRO5 },
    {"chatmacro6", (int *)&chat_macros[6], 0},  //(int) HUSTR_CHATMACRO6 },
    {"chatmacro7", (int *)&chat_macros[7], 0},  //(int) HUSTR_CHATMACRO7 },
    {"chatmacro8", (int *)&chat_macros[8], 0},  //(int) HUSTR_CHATMACRO8 },
    {"chatmacro9", (int *)&chat_macros[9], 0}  //(int) HUSTR_CHATMACRO9 }

};

gameflags_t level_flags;

boolean_t external_ddf = true;
boolean_t strict_errors = false;
boolean_t lax_errors = false;
boolean_t no_warnings = false;
boolean_t no_obsoletes = false;

void I_Printf(const char *message,...)
{
	va_list args;

	va_start(args, message);
	vprintf(message, args);
	va_end(args);
}

void I_Error(const char *error,...)
{
	va_list args;

	va_start(args, error);
	printf("ERROR: ");
	vprintf(error, args);
	va_end(args);
}

void I_Warning(const char *warning,...)
{
	va_list args;

	va_start(args, warning);
	printf("WARNING: ");
	vprintf(warning, args);
	va_end(args);
}

int W_CheckNumForName2(const char *name)
{
	return -1;
}

void P_AddPlayer(int pnum)
{
}

float_t M_Tan(angle_t ang)
{
  return (float_t) tan((double)ang * M_PI / (float_t) ANG180);
}

int M_Random(void)
{
	return 0;
}

int R_AddSpriteName(const char *name, int frame)
{
	return (int)Z_StrDup(name);
}

int strncasecmpwild(const char *s1, const char *s2, int n)
{
  int i = 0;

  for (i = 0; s1[i] && s2[i] && i < n; i++)
  {
    if ((toupper(s1[i]) != toupper(s2[i])) && (s1[i] != '?') && (s2[i] != '?'))
      break;
  }
  // -KM- 1999/01/29 If strings are equal return equal.
  if (i == n)
    return 0;

  if (s1[i] == '?' || s2[i] == '?')
    return 0;

  return s1[i] - s2[i];
}

boolean_t I_Access(const char *fname)
{
	FILE *f = fopen(fname, "rb");
	if (f)
	{
		fclose(f);
		return true;
	}
	return false;
}

//
// Prepares the end of the path name, so it will be possible to concatenate
// a DIRSEPARATOR and a file name to it.
// Allocates and returns the new string.
//
char *I_PreparePath(const char *path)
{
  int len = strlen(path);
  char *s;

  if (len == 0)
  {
    // empty string means ".\"
    return Z_StrDup(".");
  }

  if (path[0] >= 'A' && path[0] <= 'z' && path[1] == ':' && len == 2)
  {
    // special case: "c:" turns into "c:."
    s = Z_Malloc(4);
    s[0] = path[0];
    s[1] = path[1];
    s[2] = '.';
    s[3] = 0;
    return s;
  }

  if (path[len - 1] == '\\' || path[len - 1] == '/')
  {
    // cut off the last separator
    s = Z_Malloc(len);
    memcpy(s, path, len - 1);
    s[len - 1] = 0;

    return s;
  }

  return Z_StrDup(path);
}

int main(int argc, const char *argv[])
{
	M_InitArguments(argc, argv);
	// Start memory allocation system at the very start
	Z_Init();

	gamedir = ".";
	if (M_GetParm("-game"))
	{
		gamedir = I_PreparePath(M_GetParm("-game"));
	}
	if (M_GetParm("-progs"))
	{
		progsdir = I_PreparePath(M_GetParm("-progs"));
	}
	else
	{
		char tmp[256];

		sprintf(tmp, "%s/progs", gamedir);
		progsdir = Z_StrDup(tmp);
	}
	if (M_GetParm("-scripts"))
	{
		scriptsdir = I_PreparePath(M_GetParm("-scripts"));
	}
	else
	{
		char tmp[256];

		sprintf(tmp, "%s/scripts", gamedir);
		scriptsdir = Z_StrDup(tmp);
	}

	//
	// Base DDF-s
	//
	if (M_GetParm("-baseddf"))
	{
		ddfdir = I_PreparePath(M_GetParm("-baseddf"));
	}
	else
	{
		char tmp[256];

		sprintf(tmp, "%s/baseddf", gamedir);
		ddfdir = Z_StrDup(tmp);
	}
	DDF_MainInit();

	//
	//	Additional ddf-s
	//
	if (M_GetParm("-ddf"))
	{
		ddfdir = I_PreparePath(M_GetParm("-ddf"));
	}
	else
	{
		char tmp[256];

		sprintf(tmp, "%s/ddf", gamedir);
		ddfdir = Z_StrDup(tmp);
	}
	DDF_ReadLangs(NULL, 0);
	DDF_ReadSFX(NULL, 0);
	DDF_ReadColourMaps(NULL, 0);
	DDF_ReadAtks(NULL, 0);
	DDF_ReadWeapons(NULL, 0);
	DDF_ReadThings(NULL, 0);
	DDF_ReadLines(NULL, 0);
	DDF_ReadSectors(NULL, 0);
	DDF_ReadSW(NULL, 0);
	DDF_ReadAnims(NULL, 0);
	DDF_ReadGames(NULL, 0);
	DDF_ReadLevels(NULL, 0);
	DDF_ReadMusicPlaylist(NULL, 0);

	DDF_MainCleanUp();

	VC_WriteMobjs();
	VC_WriteWeapons();
	WriteSoundScript();

	return 0;
}

