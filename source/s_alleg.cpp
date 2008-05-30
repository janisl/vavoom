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
//**	
//**	System interface for sound.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <allegro.h>

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VAllegroSoundDevice : public VSoundDevice
{
public:
	enum { STRM_LEN = 8 * 1024 };

	SAMPLE**		Samples;

	AUDIOSTREAM*	Strm;
	void*			StrmBuf;

	bool Init();
	int SetChannels(int);
	void Shutdown();
	void Tick(float);
	int PlaySound(int, float, float, float, bool);
	int PlaySound3D(int, const TVec&, const TVec&, float, float, bool);
	void UpdateChannel(int, float, float);
	void UpdateChannel3D(int, const TVec&, const TVec&);
	bool IsChannelPlaying(int);
	void StopChannel(int);
	void UpdateListener(const TVec&, const TVec&, const TVec&, const TVec&,
		const TVec&, VReverbInfo*);

	bool OpenStream(int, int, int);
	void CloseStream();
	int GetStreamAvailable();
	short* GetStreamBuffer();
	void SetStreamData(short*, int);
	void SetStreamVolume(float);
	void PauseStream();
	void ResumeStream();

	bool LoadSample(int);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_SOUND_DEVICE(VAllegroSoundDevice, SNDDRV_Default, "Default",
	"Allegro sound device", NULL);

bool				allegro_sound_initialised;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  VAllegroSoundDevice::Init
//
// 	Inits sound
//
//==========================================================================

bool VAllegroSoundDevice::Init()
{
	guard(VAllegroSoundDevice::Init);
	Samples = NULL;
	Strm = NULL;
	StrmBuf = NULL;

	// Default settings
	int sound_card = DIGI_AUTODETECT;
	int music_card = MIDI_AUTODETECT;

	// Check parametters
	if (GArgs.CheckParm("-nomusic"))
		music_card = MIDI_NONE;

	// Init sound device
	if (install_sound(sound_card, music_card, NULL) == -1)
	{
		Sys_Error("ALLEGRO SOUND INIT ERROR!!!!\n%s\n", allegro_error);
	}
	allegro_sound_initialised = true;
	Samples = new SAMPLE*[GSoundManager->S_sfx.Num()];
	memset(Samples, 0, sizeof(SAMPLE*) * GSoundManager->S_sfx.Num());

	GCon->Logf(NAME_Init, "configured audio device");
	GCon->Logf(NAME_Init, "SFX   : %s", digi_driver->desc);
	GCon->Logf(NAME_Init, "Music : %s", midi_driver->desc);
	return true;
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::SetChannels
//
//==========================================================================

int VAllegroSoundDevice::SetChannels(int InNumChannels)
{
	guard(VAllegroSoundDevice::SetChannels);
	int NumVoices = digi_driver->voices;
	if (NumVoices > InNumChannels)
		NumVoices = InNumChannels;
	return NumVoices;
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::Shutdown
//
//==========================================================================

void VAllegroSoundDevice::Shutdown()
{
	guard(VAllegroSoundDevice::Shutdown);
	if (Samples)
	{
		for (int i = 0; i < GSoundManager->S_sfx.Num(); i++)
		{
			if (Samples[i])
			{
				destroy_sample(Samples[i]);
			}
		}
		delete[] Samples;
	}
	remove_sound();
	unguard;
}

//==========================================================================
//
//  VAllegroSoundDevice::Tick
//
//==========================================================================

void VAllegroSoundDevice::Tick(float)
{
}

//==========================================================================
//
//	VAllegroSoundDevice::LoadSample
//
//==========================================================================

bool VAllegroSoundDevice::LoadSample(int sound_id)
{
	guard(VAllegroSoundDevice::LoadSample);
	if (Samples[sound_id])
	{
		return Samples[sound_id];
	}

	if (!GSoundManager->LoadSound(sound_id))
	{
		//	Missing sound.
		return false;
	}

	int SfxSize = GSoundManager->S_sfx[sound_id].DataSize;
	if (GSoundManager->S_sfx[sound_id].SampleBits == 16)
		SfxSize >>= 1;

	//	Create SAMPLE* that Allegro uses.
	SAMPLE* spl = create_sample(GSoundManager->S_sfx[sound_id].SampleBits, 0,
		GSoundManager->S_sfx[sound_id].SampleRate, SfxSize);

	if (GSoundManager->S_sfx[sound_id].SampleBits == 16)
	{
		//	Convert 16 bit sound to unsigned format.
		short* pSrc = (short*)GSoundManager->S_sfx[sound_id].Data;
		short* pDst = (short*)spl->data;
		for (int i = 0; i < SfxSize; i++, pSrc++, pDst++)
			*pDst = *pSrc ^ 0x8000;
	}
	else
	{
		memcpy(spl->data, GSoundManager->S_sfx[sound_id].Data, SfxSize);
	}
	Samples[sound_id] = spl;
	GSoundManager->DoneWithLump(sound_id);
	return true;
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::PlaySound
//
// 	This function adds a sound to the list of currently active sounds, which
// is maintained as a given number of internal channels.
//
//==========================================================================

int VAllegroSoundDevice::PlaySound(int sound_id, float vol, float sep,
	float pitch, bool Loop)
{
	guard(VAllegroSoundDevice::PlaySound);
	if (!LoadSample(sound_id))
	{
		return -1;
	}

	// Start the sound
	SAMPLE* spl = Samples[sound_id];
	int voice = allocate_voice(spl);

	if (voice < 0)
	{
		return -1;
	}

	voice_set_volume(voice, (int)(vol * 255));
	voice_set_pan(voice, 127 + (int)(sep * 127));
	voice_set_frequency(voice, (int)(spl->freq * pitch));
	voice_set_playmode(voice, Loop ? PLAYMODE_LOOP : PLAYMODE_PLAY);
	voice_start(voice);
	return voice;
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::PlaySound3D
//
//==========================================================================

int VAllegroSoundDevice::PlaySound3D(int, const TVec&, const TVec&, float,
	float, bool)
{
	Sys_Error("Allegro driver doesn't support 3D sound");
}

//==========================================================================
//
//  VAllegroSoundDevice::UpdateChannel
//
//==========================================================================

void VAllegroSoundDevice::UpdateChannel(int Voice, float vol, float sep)
{
	guard(VAllegroSoundDevice::UpdateChannel);
	if (Voice == -1)
	{
		return;
	}
	voice_set_volume(Voice, (int)(vol * 255));
	voice_set_pan(Voice, 127 + (int)(sep * 127));
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::UpdateChannel3D
//
//==========================================================================

void VAllegroSoundDevice::UpdateChannel3D(int, const TVec&, const TVec&)
{
}

//==========================================================================
//
//	VAllegroSoundDevice::IsChannelPlaying
//
//==========================================================================

bool VAllegroSoundDevice::IsChannelPlaying(int Voice)
{
	guard(VAllegroSoundDevice::IsChannelPlaying);
	if (Voice == -1)
	{
		return false;
	}
	if (voice_get_position(Voice) != -1)
	{
		return true;
	}
	return false;
	unguard;
}

//==========================================================================
//
//  VAllegroSoundDevice::StopChannel
//
//	Stop the sound. Necessary to prevent runaway chainsaw, and to stop
// rocket launches when an explosion occurs.
//	All sounds MUST be stopped;
//
//==========================================================================

void VAllegroSoundDevice::StopChannel(int Voice)
{
	guard(VAllegroSoundDevice::StopChannel);
	if (Voice == -1)
	{
		return;
	}
	deallocate_voice(Voice);
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::UpdateListener
//
//==========================================================================

void VAllegroSoundDevice::UpdateListener(const TVec&, const TVec&,
	const TVec&, const TVec&, const TVec&, VReverbInfo*)
{
}

//==========================================================================
//
//	VAllegroSoundDevice::OpenStream
//
//==========================================================================

bool VAllegroSoundDevice::OpenStream(int Rate, int Bits, int Channels)
{
	guard(VAllegroSoundDevice::OpenStream);
	Strm = play_audio_stream(STRM_LEN, Bits, Channels == 2, Rate, 255, 127);
	if (!Strm)
	{
		GCon->Log("Can't open stream");
	}
	return !!Strm;
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::CloseStream
//
//==========================================================================

void VAllegroSoundDevice::CloseStream()
{
	guard(VAllegroSoundDevice::CloseStream);
	if (Strm)
	{
		stop_audio_stream(Strm);
		Strm = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::GetStreamAvailable
//
//==========================================================================

int VAllegroSoundDevice::GetStreamAvailable()
{
	guard(VAllegroSoundDevice::GetStreamAvailable);
	if (!Strm)
		return 0;
	StrmBuf = get_audio_stream_buffer(Strm);
	return StrmBuf ? STRM_LEN : 0;
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::SetStreamData
//
//==========================================================================

short* VAllegroSoundDevice::GetStreamBuffer()
{
	guard(VAllegroSoundDevice::GetStreamBuffer);
	return (short*)StrmBuf;
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::SetStreamData
//
//==========================================================================

void VAllegroSoundDevice::SetStreamData(short* Data, int Len)
{
	guard(VAllegroSoundDevice::SetStreamData);
	//	Copy data converting to unsigned format.
	for (int i = 0; i < Len * 2; i++)
	{
		((word*)StrmBuf)[i] = (word)(Data[i] + 0x7fff);
	}
	free_audio_stream_buffer(Strm);
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::SetStreamVolume
//
//==========================================================================

void VAllegroSoundDevice::SetStreamVolume(float Vol)
{
	guard(VAllegroSoundDevice::SetStreamVolume);
	if (Strm)
	{
		voice_set_volume(Strm->voice, int(Vol * 255));
	}
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::PauseStream
//
//==========================================================================

void VAllegroSoundDevice::PauseStream()
{
	guard(VAllegroSoundDevice::PauseStream);
	if (Strm)
	{
		voice_stop(Strm->voice);
	}
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::ResumeStream
//
//==========================================================================

void VAllegroSoundDevice::ResumeStream()
{
	guard(VAllegroSoundDevice::ResumeStream);
	if (Strm)
	{
		voice_start(Strm->voice);
	}
	unguard;
}
