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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

#define DEFAULT_ARCHIVEPATH		"o:/sound/archive/"

// TYPES -------------------------------------------------------------------

struct raw_sound_t
{
	word	unkn1;
    word	freq;
	dword	len;
	byte	data[1];
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean				UseSndScript;
char				ArchivePath[128];

sfxinfo_t			*S_sfx;
int					NumSfx;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	S_InitScript
//
//	Loads sound script lump or file, if param -devsnd was specified
//
//==========================================================================

void S_InitScript(void)
{
	int p;
	int i;

    //
    //	Allocate memory for sound info
    //

	S_sfx = (sfxinfo_t*)Z_Malloc(sizeof(sfxinfo_t), PU_STATIC, 0);
	memset(S_sfx, 0, sizeof(sfxinfo_t));
	NumSfx = 1;

    //
    //	Load script SFXINFO
    //

	SC_OpenLump("sfxinfo");

	while (SC_GetString())
	{
		Z_Resize((void**)&S_sfx, (NumSfx + 1) * sizeof(*S_sfx));
		strncpy(S_sfx[NumSfx].tagName, sc_String, 32);

		SC_MustGetString();
		if (*sc_String != '?')
		{
			strcpy(S_sfx[NumSfx].lumpname, sc_String);
		}
        else
        {
			S_sfx[NumSfx].lumpname[0] = 0;
		}

		SC_MustGetNumber();
		S_sfx[NumSfx].priority = sc_Number;

		SC_MustGetNumber();
		S_sfx[NumSfx].numchannels = sc_Number;

		SC_MustGetNumber();
		S_sfx[NumSfx].changePitch = sc_Number;

       	NumSfx++;
	}
	SC_Close();

	//
    //	Load script SNDINFO
    //

	strcpy(ArchivePath, DEFAULT_ARCHIVEPATH);
	p = M_CheckParm("-devsnd");
	if (!p)
	{
		UseSndScript = false;
		SC_OpenLump("sndinfo");
	}
	else
	{
		UseSndScript = true;
		SC_OpenFile(myargv[p + 1]);
	}

	while (SC_GetString())
	{
		if (*sc_String == '$')
		{
			if (!stricmp(sc_String, "$ARCHIVEPATH"))
			{
				SC_MustGetString();
				strcpy(ArchivePath, sc_String);
			}
			else if (!stricmp(sc_String, "$MAP"))
			{
				SC_MustGetNumber();
				SC_MustGetString();
				if(sc_Number)
				{
					P_PutMapSongLump(sc_Number, sc_String);
				}
			}
			continue;
		}
		else
		{
			for (i = 0; i < NumSfx; i++)
			{
				if (!strcmp(S_sfx[i].tagName, sc_String))
				{
					break;
				}
			}
			if (i == NumSfx)
			{
            	//	Not found - add it
            	NumSfx++;
                Z_Resize((void**)&S_sfx, NumSfx * sizeof(*S_sfx));
				strncpy(S_sfx[i].tagName, sc_String, 32);
                //	Default values
                S_sfx[i].priority = 127;
                S_sfx[i].numchannels = -1;
                S_sfx[i].changePitch = 1;
			}

			SC_MustGetString();
			if (*sc_String != '?')
			{
				strcpy(S_sfx[i].lumpname, sc_String);
			}
			else
			{
				strcpy(S_sfx[i].lumpname, "default");
            }
		}
	}
	SC_Close();

	//
    //	Set "default" sound for empty sounds
    //

	for (i = 0; i < NumSfx; i++)
	{
		if (!S_sfx[i].lumpname[0])
		{
			strcpy(S_sfx[i].lumpname, "default");
		}
        S_sfx[i].snd_ptr = NULL;
        S_sfx[i].lumpnum = -1;
	}
}

//==========================================================================
//
// S_GetSoundID
//
//==========================================================================

int S_GetSoundID(char *name)
{
	int i;

	for (i = 0; i < NumSfx; i++)
	{
		if (!strcmp(S_sfx[i].tagName, name))
		{
			return i;
		}
	}
	con << "WARNING! Can't find sound named \"" << name << "\".\n";
	return 0;
}

#ifdef CLIENT

//==========================================================================
//
//	S_LoadSound
//
//==========================================================================

void S_LoadSound(int sound_id)
{
	if (!S_sfx[sound_id].snd_ptr)
	{
		if (UseSndScript)
		{
			M_ReadFile(va("%s%s.lmp", ArchivePath, S_sfx[sound_id].lumpname),
				(byte **)&S_sfx[sound_id].snd_ptr);
		}
		else
		{
		  	// get lumpnum if necessary
			if (S_sfx[sound_id].lumpnum < 0)
			{
				S_sfx[sound_id].lumpnum = W_GetNumForName(S_sfx[sound_id].lumpname);
			}
			S_sfx[sound_id].snd_ptr = W_CacheLumpNum(S_sfx[sound_id].lumpnum,
				PU_SOUND);
		}

		raw_sound_t *rawdata = (raw_sound_t *)S_sfx[sound_id].snd_ptr;
	    S_sfx[sound_id].freq = LittleShort(rawdata->freq);
		S_sfx[sound_id].len = LittleLong(rawdata->len);
		S_sfx[sound_id].data = rawdata->data;
	}
	S_sfx[sound_id].usecount++;
}

//==========================================================================
//
//	S_DoneWithLump
//
//==========================================================================

void S_DoneWithLump(int sound_id)
{
	int i;
	void *ptr;

	if (!S_sfx[sound_id].snd_ptr || !S_sfx[sound_id].usecount)
	{
		Sys_Error("S_DoneWithLump: Empty lump");
	}

	S_sfx[sound_id].usecount--;
	if (S_sfx[sound_id].usecount)
	{
		//	still used
		return;
	}

	//	Mark as not loaded
	ptr = S_sfx[sound_id].snd_ptr;
	S_sfx[sound_id].snd_ptr = NULL;

	for (i = 0; i < NumSfx; i++)
	{
		if (S_sfx[i].snd_ptr == ptr)
		{
			//	Another active sound uses this lump
			return;
		}
	}
	//	Make lump cachable
	Z_ChangeTag(ptr, PU_CACHE);
}

#endif
#ifndef CLIENT
//==========================================================================
//
//	S_Init
//
//==========================================================================

void S_Init(void)
{
	S_InitScript();
}

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
