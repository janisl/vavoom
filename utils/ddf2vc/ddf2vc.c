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
#include "z_zone.h"
#include "vc.h"

const char *chat_macros[10];
const char *destination_keys;
const char *gammamsg[5];

const mapstuff_t *currentmap;

char *ddfdir = "ddf";
char *outdir = "../../progs/ddf";

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

const char *M_GetParm(const char *check)
{
	return NULL;
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

int main(int argc, char *argv[])
{
	// Start memory allocation system at the very start
	Z_Init();

	DDF_MainInit();
	DDF_MainCleanUp();

	VC_WriteMobjs();
	VC_WriteWeapons();

	return 0;
}

