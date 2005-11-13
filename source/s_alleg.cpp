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
	void UpdateListener(const TVec&, const TVec&, const TVec&, const TVec&, const TVec&);

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
	if (M_CheckParm("-nomusic"))
		music_card = MIDI_NONE;

	// Init sound device
	if (install_sound(sound_card, music_card, NULL) == -1)
	{
		Sys_Error("ALLEGRO SOUND INIT ERROR!!!!\n%s\n", allegro_error);
	}
	allegro_sound_initialised = true;
	Samples = Z_CNew<SAMPLE*>(S_sfx.Num());

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
		for (int i = 0; i < S_sfx.Num(); i++)
		{
			if (Samples[i])
			{
				destroy_sample(Samples[i]);
			}
		}
		Z_Free(Samples);
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

	if (!S_LoadSound(sound_id))
	{
		//	Missing sound.
		return false;
	}

	int SfxSize = S_sfx[sound_id].DataSize;
	if (S_sfx[sound_id].SampleBits == 16)
		SfxSize >>= 1;

	//	Create SAMPLE* that Allegro uses.
	SAMPLE* spl = create_sample(S_sfx[sound_id].SampleBits, 0,
		S_sfx[sound_id].SampleRate, SfxSize);

	if (S_sfx[sound_id].SampleBits == 16)
	{
		//	Convert 16 bit sound to unsigned format.
		short* pSrc = (short*)S_sfx[sound_id].Data;
		short* pDst = (short*)spl->data;
		for (int i = 0; i < SfxSize; i++, pSrc++, pDst++)
			*pDst = *pSrc ^ 0x8000;
	}
	else
	{
		memcpy(spl->data, S_sfx[sound_id].Data, SfxSize);
	}
	Samples[sound_id] = spl;
	S_DoneWithLump(sound_id);
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
	const TVec&, const TVec&, const TVec&)
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

//**************************************************************************
//
//	$Log$
//	Revision 1.24  2005/11/13 14:36:22  dj_jl
//	Moved common sound functions to main sound module.
//
//	Revision 1.23  2005/11/06 15:31:32  dj_jl
//	Fix for 16 bit sound conversion.
//	
//	Revision 1.22  2005/11/06 15:27:09  dj_jl
//	Added support for 16 bit sounds.
//	
//	Revision 1.21  2005/11/05 15:50:07  dj_jl
//	Voices played as normal sounds.
//	
//	Revision 1.20  2005/11/03 22:46:35  dj_jl
//	Support for any bitrate streams.
//	
//	Revision 1.19  2005/10/18 20:53:04  dj_jl
//	Implemented basic support for streamed music.
//	
//	Revision 1.18  2005/09/19 23:00:19  dj_jl
//	Streaming support.
//	
//	Revision 1.17  2005/09/12 19:45:16  dj_jl
//	Created midi device class.
//	
//	Revision 1.16  2004/11/30 07:17:16  dj_jl
//	Made string pointers const.
//	
//	Revision 1.15  2004/08/21 19:10:44  dj_jl
//	Changed sound driver declaration.
//	
//	Revision 1.14  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//	
//	Revision 1.13  2003/03/08 12:08:04  dj_jl
//	Beautification.
//	
//	Revision 1.12  2002/08/05 17:20:32  dj_jl
//	Fixed voices.
//	
//	Revision 1.11  2002/07/27 18:10:11  dj_jl
//	Implementing Strife conversations.
//	
//	Revision 1.10  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.9  2002/07/20 14:49:41  dj_jl
//	Implemented sound drivers.
//	
//	Revision 1.8  2002/01/11 08:12:01  dj_jl
//	Added guard macros
//	
//	Revision 1.7  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.6  2001/08/30 17:41:42  dj_jl
//	Added entity sound channels
//	
//	Revision 1.5  2001/08/29 17:55:42  dj_jl
//	Added sound channels
//	
//	Revision 1.4  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
