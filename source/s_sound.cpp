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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(VSoundDevice);

TCvarI				sfx_volume("sfx_volume", "8", CVAR_ARCHIVE);
TCvarI				music_volume("music_volume", "8", CVAR_ARCHIVE);
TCvarI				swap_stereo("swap_stereo", "0", CVAR_ARCHIVE);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char			mapSong[12];
static int			mapCDTrack;

static TCvarI		cd_music("use_cd_music", "0", CVAR_ARCHIVE);
static boolean		CDMusic = false;

static VSoundDevice	*GSoundDevice;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	S_Init
//
// 	Initializes sound stuff, including volume
// 	Sets channels, SFX and music volume,
// allocates channel buffer, sets S_sfx lookup.
//
//==========================================================================

void S_Init(void)
{
	guard(S_Init);
	VClass *DeviceClass;
	if (M_CheckParm("-openal"))
	{
		DeviceClass = VClass::FindClass("OpenALDevice");
		if (!DeviceClass)
		{
			Sys_Error("OpenAL driver not available");
		}
	}
	else
	{
		DeviceClass = VClass::FindClass("DefaultSoundDevice");
		if (!DeviceClass)
		{
			//	No sound driver, use no-sound driver.
			DeviceClass = VSoundDevice::StaticClass();
		}
	}
	GSoundDevice = (VSoundDevice *)VObject::StaticSpawnObject(DeviceClass, NULL, PU_STATIC);

	GSoundDevice->Init();
	S_InitMusic();
	CD_Init();

	S_InitScript();
	SN_InitSequenceScript();
	unguard;
}

//==========================================================================
//
//	S_Shutdown
//
//	Shuts down all sound stuff
//
//==========================================================================

void S_Shutdown(void)
{
	guard(S_Shutdown);
	CD_Shutdown();
	S_ShutdownMusic();
	if (GSoundDevice)
	{
		GSoundDevice->Shutdown();
		GSoundDevice->Destroy();
		GSoundDevice = NULL;
	}
	unguard;
}

//==========================================================================
//
//	S_StartSound
//
//==========================================================================

void S_StartSound(int sound_id, const TVec &origin, const TVec &velocity,
	int origin_id, int channel, int volume)
{
	GSoundDevice->PlaySound(sound_id, origin, velocity, origin_id, channel,
		float(volume) / 127.0);
}

//==========================================================================
//
//	S_PlayTillDone
//
//==========================================================================

void S_PlayTillDone(char *sound)
{
	GSoundDevice->PlaySoundTillDone(sound);
}

//==========================================================================
//
//	S_StopSound
//
//==========================================================================

void S_StopSound(int origin_id, int channel)
{
	GSoundDevice->StopSound(origin_id, channel);
}

//==========================================================================
//
//	S_StopAllSound
//
//==========================================================================

void S_StopAllSound(void)
{
	GSoundDevice->StopAllSound();
}

//==========================================================================
//
//	S_GetSoundPlayingInfo
//
//==========================================================================

boolean S_GetSoundPlayingInfo(int origin_id, int sound_id)
{
	return GSoundDevice->IsSoundPlaying(origin_id, sound_id);
}

//==========================================================================
//
//	S_StartSong
//
//==========================================================================

void S_StartSong(char* song, int track, boolean loop)
{
	guard(S_StartSong);
	if (CDMusic)
	{
    	if (loop)
			CmdBuf << "CD Loop " << track << "\n";
		else
			CmdBuf << "CD Play " << track << "\n";
	}
	else
	{
    	if (loop)
			CmdBuf << "Music Loop " << song << "\n";
		else
			CmdBuf << "Music Play " << song << "\n";
	}
	unguard;
}

//==========================================================================
//
//	S_PauseSound
//
//==========================================================================

void S_PauseSound(void)
{
	guard(S_PauseSound);
	if (CDMusic)
	{
    	CmdBuf << "CD Pause\n";
	}
	else
	{
    	CmdBuf << "Music Pause\n";
	}
	unguard;
}

//==========================================================================
//
//	S_ResumeSound
//
//==========================================================================

void S_ResumeSound(void)
{
	guard(S_ResumeSound);
	if (CDMusic)
	{
    	CmdBuf << "CD resume\n";
	}
	else
	{
    	CmdBuf << "Music resume\n";
	}
	unguard;
}

//==========================================================================
//
//  StartMusic
//
//==========================================================================

static void StartMusic(void)
{
	S_StartSong(mapSong, mapCDTrack, true);
}

//==========================================================================
//
//	S_Start
//
//	Per level startup code. Kills playing sounds at start of level,
// determines music if any, changes music.
//
//==========================================================================

void S_Start(const mapInfo_t &info)
{
	guard(S_Start);
	SN_StopAllSequences();
	S_StopAllSound();

	strcpy(mapSong, info.songLump);
	mapCDTrack = info.cdTrack;

	StartMusic();
	unguard;
}	

//==========================================================================
//
// S_UpdateSounds
//
// Updates music & sounds
//
//==========================================================================

void S_UpdateSounds(void)
{
	guard(S_UpdateSounds);
	if (cd_music && !CDMusic)
    {
    	CmdBuf << "Music Stop\n";
		CDMusic = true;
		StartMusic();
    }
	if (!cd_music && CDMusic)
    {
    	CmdBuf << "CD Stop\n";
		CDMusic = false;
		StartMusic();
    }

	// Update any Sequences
	SN_UpdateActiveSequences();

	GSoundDevice->Tick(host_frametime);
	S_UpdateMusic();
	CD_Update();
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2002/07/20 14:49:41  dj_jl
//	Implemented sound drivers.
//
//	Revision 1.5  2002/01/11 08:12:01  dj_jl
//	Added guard macros
//	
//	Revision 1.4  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
