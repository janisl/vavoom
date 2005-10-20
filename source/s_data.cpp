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
#include "s_local.h"

// MACROS ------------------------------------------------------------------

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

TArray<sfxinfo_t>	S_sfx;
sfxinfo_t			S_VoiceInfo;

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
	guard(S_InitScript);
	int p;
	int i;

	//
	//	Allocate memory for sound info
	//

	S_sfx.AddZeroed();

	//
	//	Load script SFXINFO
	//

	SC_Open("sfxinfo");

	while (SC_GetString())
	{
		i =	S_sfx.AddZeroed();
		S_sfx[i].tagName = sc_String;

		SC_MustGetString();
		if (*sc_String != '?')
		{
			strcpy(S_sfx[i].lumpname, sc_String);
		}
		else
		{
			S_sfx[i].lumpname[0] = 0;
		}

		SC_MustGetNumber();
		S_sfx[i].priority = sc_Number;

		SC_MustGetNumber();
		S_sfx[i].numchannels = sc_Number;

		SC_MustGetNumber();
		S_sfx[i].changePitch = sc_Number;
	}
	SC_Close();

	//
	//	Load script SNDINFO
	//

	SC_Open("sndinfo");

	while (SC_GetString())
	{
		if (*sc_String == '$')
		{
			if (!stricmp(sc_String, "$ARCHIVEPATH"))
			{
				//	Ignored.
				SC_MustGetString();
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
			i = 0;
			for (TArray<sfxinfo_t>::TIterator It(S_sfx); It; ++It)
			{
				if (!strcmp(*It->tagName, sc_String))
				{
					i = It.GetIndex();
					break;
				}
			}
			if (!i)
			{
				//	Not found - add it
				i = S_sfx.AddZeroed();
				S_sfx[i].tagName = sc_String;
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

	S_sfx.Shrink();

	//
	//	Set "default" sound for empty sounds
	//
	for (TArray<sfxinfo_t>::TIterator It(S_sfx); It; ++It)
	{
		if (!It->lumpname[0])
		{
			strcpy(It->lumpname, "default");
		}
		It->snd_ptr = NULL;
		It->lumpnum = -1;
	}

	//
	//	Prepare slot for voices
	//
	S_VoiceInfo.priority = 255;
	S_VoiceInfo.numchannels = -1;
	S_VoiceInfo.changePitch = false;
	unguard;
}

//==========================================================================
//
//	S_GetSoundID
//
//==========================================================================

int S_GetSoundID(FName Name)
{
	guard(S_GetSoundID);
	for (TArray<sfxinfo_t>::TIterator It(S_sfx); It; ++It)
	{
		if (It->tagName == Name)
		{
			return It.GetIndex();
		}
	}
	GCon->Logf("WARNING! Can't find sound %s", *Name);
	return 0;
	unguard;
}

//==========================================================================
//
// S_GetSoundID
//
//==========================================================================

int S_GetSoundID(const char *name)
{
	guard(S_GetSoundID);
	for (TArray<sfxinfo_t>::TIterator It(S_sfx); It; ++It)
	{
//FIXME really case sensitive? What about ACS?
		if (!strcmp(*It->tagName, name))
		{
			return It.GetIndex();
		}
	}
	GCon->Logf("WARNING! Can't find sound named %s", name);
	return 0;
	unguard;
}

#ifdef CLIENT

//==========================================================================
//
//	S_LoadSound
//
//==========================================================================

bool S_LoadSound(int sound_id, const char *VoiceName)
{
	guard(S_LoadSound);
	if (VoiceName)
	{
		//	Load voice.
		if (S_VoiceInfo.snd_ptr)
		{
			GCon->Log(NAME_Dev, "WARNING! Voice is still used");
		}

		S_VoiceInfo.lumpnum = W_CheckNumForName(VoiceName, WADNS_Voices);
		if (S_VoiceInfo.lumpnum < 0)
		{
			GCon->Logf(NAME_Dev, "Voice %s not found", VoiceName);
			return false;
		}
		S_VoiceInfo.snd_ptr = W_CacheLumpNum(S_VoiceInfo.lumpnum, PU_SOUND);

		raw_sound_t *rawdata = (raw_sound_t *)S_VoiceInfo.snd_ptr;
	    S_VoiceInfo.freq = LittleShort(rawdata->freq);
		S_VoiceInfo.len = LittleLong(rawdata->len);
		S_VoiceInfo.data = rawdata->data;
		S_VoiceInfo.usecount++;
		return true;
	}

	if (!S_sfx[sound_id].snd_ptr)
	{
		// get lumpnum if necessary
		if (S_sfx[sound_id].lumpnum < 0)
		{
			S_sfx[sound_id].lumpnum = W_CheckNumForName(
				S_sfx[sound_id].lumpname);
			//FIXME Strife quit sounds are voices.
			if (S_sfx[sound_id].lumpnum < 0)
				S_sfx[sound_id].lumpnum = W_CheckNumForName(
					S_sfx[sound_id].lumpname, WADNS_Voices);
			if (S_sfx[sound_id].lumpnum < 0)
			{
				GCon->Logf(NAME_Dev, "Sound lump %s not found",
					S_sfx[sound_id].lumpname);
				return false;
			}
		}
		S_sfx[sound_id].snd_ptr = W_CacheLumpNum(S_sfx[sound_id].lumpnum,
			PU_SOUND);

		raw_sound_t *rawdata = (raw_sound_t *)S_sfx[sound_id].snd_ptr;
	    S_sfx[sound_id].freq = LittleShort(rawdata->freq);
		S_sfx[sound_id].len = LittleLong(rawdata->len);
		S_sfx[sound_id].data = rawdata->data;
	}
	S_sfx[sound_id].usecount++;
	return true;
	unguard;
}

//==========================================================================
//
//	S_DoneWithLump
//
//==========================================================================

void S_DoneWithLump(int sound_id)
{
	guard(S_DoneWithLump);
	void *ptr;

	sfxinfo_t &sfx = sound_id == VOICE_SOUND_ID ? S_VoiceInfo : S_sfx[sound_id];
	if (!sfx.snd_ptr || !sfx.usecount)
	{
		Sys_Error("S_DoneWithLump: Empty lump");
	}

	sfx.usecount--;
	if (sfx.usecount)
	{
		//	still used
		return;
	}

	//	Mark as not loaded
	ptr = sfx.snd_ptr;
	sfx.snd_ptr = NULL;

	for (TArray<sfxinfo_t>::TIterator It(S_sfx); It; ++It)
	{
		if (It->snd_ptr == ptr)
		{
			//	Another active sound uses this lump
			return;
		}
	}
	//	Make lump cachable
	Z_ChangeTag(ptr, PU_CACHE);
	unguard;
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
//	Revision 1.13  2005/10/20 22:31:27  dj_jl
//	Removed Hexen's devsnd support.
//
//	Revision 1.12  2004/11/30 07:17:17  dj_jl
//	Made string pointers const.
//	
//	Revision 1.11  2004/11/23 12:43:10  dj_jl
//	Wad file lump namespaces.
//	
//	Revision 1.10  2002/07/27 18:10:11  dj_jl
//	Implementing Strife conversations.
//	
//	Revision 1.9  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.8  2002/07/20 14:50:24  dj_jl
//	Missing sound data will not crash game anymore.
//	
//	Revision 1.7  2002/01/12 18:05:00  dj_jl
//	Beautification
//	
//	Revision 1.6  2002/01/11 08:11:05  dj_jl
//	Changes in sound list
//	Added guard macros
//	
//	Revision 1.5  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.4  2001/10/12 17:31:13  dj_jl
//	no message
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
