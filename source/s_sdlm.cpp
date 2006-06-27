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

#include <SDL.h>
#include <SDL_mixer.h>

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VSDLMidiDevice : public VMidiDevice
{
public:
	bool		DidInitMixer;
	Mix_Music*	music;

	void*		Mus_SndPtr;
	bool		MusicPaused;
	float		MusVolume;

	VSDLMidiDevice();
	void Init();
	void Shutdown();
	void SetVolume(float);
	void Tick(float);
	void Play(void*, int, const char*, bool);
	void Pause();
	void Resume();
	void Stop();
	bool IsPlaying();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern bool					sdl_mixer_initialised;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_MIDI_DEVICE(VSDLMidiDevice, MIDIDRV_Default, "Default",
	"SDL midi device", NULL);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VSDLMidiDevice::VSDLMidiDevice
//
//==========================================================================

VSDLMidiDevice::VSDLMidiDevice()
: DidInitMixer(false)
, music(NULL)
, Mus_SndPtr(NULL)
, MusicPaused(false)
, MusVolume(-1)
{
}

//==========================================================================
//
//	VSDLMidiDevice::Init
//
//==========================================================================

void VSDLMidiDevice::Init()
{
	guard(VSDLMidiDevice::Init);
	if (!sdl_mixer_initialised)
	{
		//	Currently I failed to make OpenAL work with SDL music.
#if 1
		return;
#else
		if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT,
			MIX_DEFAULT_CHANNELS, 4096) < 0)
		{
			GCon->Logf(NAME_Init, "Failed to intialise SDL mixer");
			return;
		}
		DidInitMixer = true;
#endif
	}
	Initialised = true;
	unguard;
}

//==========================================================================
//
//	VSDLMidiDevice::Shutdown
//
//==========================================================================

void VSDLMidiDevice::Shutdown()
{
	guard(VSDLMidiDevice::Shutdown);
	if (Initialised)
	{
		Stop();
		if (DidInitMixer)
		{
			Mix_CloseAudio();
		}
	}
	unguard;
}

//==========================================================================
//
//	VSDLMidiDevice::SetVolume
//
//==========================================================================

void VSDLMidiDevice::SetVolume(float Volume)
{
	guard(VSDLMidiDevice::SetVolume);
	if (Volume != MusVolume)
	{
		MusVolume = Volume;
		Mix_VolumeMusic(int(MusVolume * 255));
	}
	unguard;
}

//==========================================================================
//
//  VSDLMidiDevice::Tick
//
//==========================================================================

void VSDLMidiDevice::Tick(float)
{
}

//==========================================================================
//
//	VSDLMidiDevice::Play
//
//==========================================================================

void VSDLMidiDevice::Play(void* Data, int len, const char* song, bool loop)
{
	guard(VSDLMidiDevice::Play);
	int		handle;

	Mus_SndPtr = Data;
	if ((handle = Sys_FileOpenWrite("vv_temp.mid")) < 0) return;
	if (Sys_FileWrite(handle, Mus_SndPtr, len) != len) return;
	if (Sys_FileClose(handle) < 0) return;

	music = Mix_LoadMUS("vv_temp.mid");
	remove("vv_temp.mid");

	if (!music)
	{
		Z_Free(Mus_SndPtr);
		Mus_SndPtr = NULL;
		return;
	}

	Mix_FadeInMusic(music, loop, 2000);

	if (!MusVolume || MusicPaused)
	{
		Mix_PauseMusic();
	}
	CurrSong = VName(song, VName::AddLower8);
	CurrLoop = loop;
	unguard;
}

//==========================================================================
//
//  VSDLMidiDevice::Pause
//
//==========================================================================

void VSDLMidiDevice::Pause()
{
	guard(VSDLMidiDevice::Pause);
	Mix_PauseMusic();
	MusicPaused = true;
	unguard;
}

//==========================================================================
//
//  VSDLMidiDevice::Resume
//
//==========================================================================

void VSDLMidiDevice::Resume()
{
	guard(VSDLMidiDevice::Resume);
	if (MusVolume)
		Mix_ResumeMusic();
	MusicPaused = false;
	unguard;
}

//==========================================================================
//
//  VSDLMidiDevice::Stop
//
//==========================================================================

void VSDLMidiDevice::Stop()
{
	guard(VSDLMidiDevice::Stop);
	if (music)
	{
		Mix_HaltMusic();
		Mix_FreeMusic(music);
		music = NULL;
	}
	if (Mus_SndPtr)
	{
		Z_Free(Mus_SndPtr);
		Mus_SndPtr = NULL;
	}
	CurrSong = NAME_None;
	unguard;
}

//==========================================================================
//
//  VSDLMidiDevice::IsPlaying
//
//==========================================================================

bool VSDLMidiDevice::IsPlaying()
{
	guard(VSDLMidiDevice::IsPlaying);
	return !!music;
	unguard;
}
