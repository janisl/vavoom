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

#include <SDL.h>
#include <SDL_mixer.h>

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VSDLSoundDevice : public VSoundDevice
{
public:
	enum { STRM_LEN = 16 * 1024 };

	Uint16			CurFormat;
	int				CurChannels;
	int				CurFrequency;

	Mix_Chunk**		Chunks;

	SDL_AudioCVT	StrmCvt;
	byte*			StrmBuffer;
	int				StrmBufferUsed;
	float			StrmVol;

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

	int know_value(int, const int*);
	Mix_Chunk* LoadSound(int);

	static void StrmCallback(void*, Uint8*, int);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_SOUND_DEVICE(VSDLSoundDevice, SNDDRV_Default, "Default",
	"SDL sound device", NULL);

bool							sdl_mixer_initialised;

static VCvarI mix_frequency		("mix_frequency", "44100", CVAR_Archive);
static VCvarI mix_bits			("mix_bits",      "16",    CVAR_Archive);
static VCvarI mix_channels		("mix_channels",  "2",     CVAR_Archive);

static VCvarI mix_chunksize		("mix_chunksize", "4096",  CVAR_Archive);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static const int	frequencies[] = { 11025, 22050, 44100, 0 };
// see SDL/SDL_audio.h for these...
static const int	chunksizes[] = {  512,  1024,  2048, 4096, 8192, 0};
static const int	voices[] = {        4,     8,    16,   32,   64, 0};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  VSDLSoundDevice::Init
//
// 	Inits sound
//
//==========================================================================

bool VSDLSoundDevice::Init()
{
	guard(VSDLSoundDevice::Init);
	const char*	p;
	int    freq;
	Uint16 fmt;
	int    ch;  /* audio */
	int    cksz;
	char   dname[32];

	Chunks = NULL;
	StrmBuffer = NULL;
	StrmBufferUsed = 0;
	StrmVol = 1;

	p = GArgs.CheckValue("-mix_frequency");
	if (p)
		mix_frequency = atoi(p);
	if (know_value(mix_frequency, frequencies))
		freq = mix_frequency;
	else
		freq = 44100;

	p = GArgs.CheckValue("-mix_bits");
	if (p)
		mix_bits = atoi(p);
	if (mix_bits == 8)
		fmt = AUDIO_U8;
	else
		fmt = AUDIO_S16;

	p = GArgs.CheckValue("-mix_channels");
	if (p)
		mix_channels = atoi(p);
	if (mix_channels == 1 || mix_channels == 2 || mix_channels == 4 || mix_channels == 6)
		ch = mix_channels;
	else
		ch = 2;

	p = GArgs.CheckValue("-mix_chunksize");
	if (p)
		mix_chunksize = atoi(p);
	if (know_value(mix_chunksize, chunksizes))
		cksz = mix_chunksize;
	else
		cksz = 4096;

	if (Mix_OpenAudio(freq, fmt, ch, cksz) < 0)
	{
		return false;
	}
	sdl_mixer_initialised = true;

	Mix_QuerySpec(&CurFrequency, &CurFormat, &CurChannels);

	//	Allocate array for chunks.
	Chunks = Z_CNew<Mix_Chunk*>(S_sfx.Num());

	GCon->Logf(NAME_Init, "Configured audio device");
	GCon->Logf(NAME_Init, "Driver: %s", SDL_AudioDriverName(dname, 32));
	GCon->Logf(NAME_Init, "Freqency: %d", CurFrequency);
	GCon->Logf(NAME_Init, "Channels: %d", CurChannels);
	GCon->Logf(NAME_Init, "Format: %04x", CurFormat);
	return true;
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::SetChannels
//
//==========================================================================

int VSDLSoundDevice::SetChannels(int InNumChannels)
{
	guard(VSDLSoundDevice::SetChannels);
	//	Allocate voices.
	int NumVoices = Mix_AllocateChannels(InNumChannels);
	GCon->Logf(NAME_Init, "Using %d voices", NumVoices);
	return NumVoices;
	unguard;
}

//==========================================================================
//
//  VSDLSoundDevice::know_value
//
//==========================================================================

int VSDLSoundDevice::know_value(int val, const int* vals)
{
	int i;

	i = 0;
	while (vals[i])
	{
		if (vals[i] == val)
			return 1;
		i++;
	}
	return 0;
}

//==========================================================================
//
//	VSDLSoundDevice::Shutdown
//
//==========================================================================

void VSDLSoundDevice::Shutdown()
{
	guard(VSDLSoundDevice::Shutdown);
	if (Chunks)
	{
		for (int i = 0; i < S_sfx.Num(); i++)
		{
			if (Chunks[i])
			{
				Mix_FreeChunk(Chunks[i]);
			}
		}
		Z_Free(Chunks);
		Chunks = NULL;
	}
	if (sdl_mixer_initialised)
	{
		Mix_CloseAudio();
		sdl_mixer_initialised = false;
	}
	unguard;
}

//==========================================================================
//
//  VSDLSoundDevice::Tick
//
//==========================================================================

void VSDLSoundDevice::Tick(float)
{
}

//==========================================================================
//
//  VSDLSoundDevice::LoadSound
//
//	Load raw data -- this is a hacked version of Mix_LoadWAV_RW (gl)
//
//==========================================================================

Mix_Chunk* VSDLSoundDevice::LoadSound(int sound_id)
{
	guard(VSDLSoundDevice::LoadSound);
	Mix_Chunk *chunk;
	SDL_AudioCVT cvt;

	if (Chunks[sound_id])
	{
		return Chunks[sound_id];
	}

	if (!S_LoadSound(sound_id))
	{
		//	Missing sound.
		return NULL;
	}

	//	Set up audio converter.
	if (SDL_BuildAudioCVT(&cvt, S_sfx[sound_id].SampleBits == 8 ?
		AUDIO_U8 : AUDIO_S16, 1, S_sfx[sound_id].SampleRate,
		CurFormat, CurChannels, CurFrequency) < 0)
	{
		S_DoneWithLump(sound_id);
		return NULL;
	}

	//	Copy data.
	cvt.len = S_sfx[sound_id].DataSize;
	cvt.buf = (Uint8*)malloc(cvt.len * cvt.len_mult);
	memcpy(cvt.buf, S_sfx[sound_id].Data, cvt.len);
	S_DoneWithLump(sound_id);

	//	Run the audio converter.
	if (SDL_ConvertAudio(&cvt) < 0)
	{
		free(cvt.buf);
		return NULL;
	}

	//	Allocate chunk.
	chunk = (Mix_Chunk*)malloc(sizeof(Mix_Chunk));
	chunk->allocated = 1;
	chunk->abuf = cvt.buf;
	chunk->alen = cvt.len_cvt;
	chunk->volume = MIX_MAX_VOLUME;
	Chunks[sound_id] = chunk;
	return chunk;
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::PlaySound
//
// 	This function adds a sound to the list of currently active sounds, which
// is maintained as a given number of internal Voices.
//
//==========================================================================

int VSDLSoundDevice::PlaySound(int sound_id, float vol, float sep, float,
	bool Loop)
{
	guard(VSDLSoundDevice::PlaySound);
	Mix_Chunk* chunk;
	int voice;

	// copy the lump to a SDL_Mixer chunk...
	chunk = LoadSound(sound_id);
	if (!chunk)
	{
		return -1;
	}

	voice = Mix_PlayChannelTimed(-1, chunk, Loop ? -1 : 0, -1);
	if (voice < 0)
	{
		return -1;
	}

	Mix_Volume(voice, (int)(vol * 127));
	Mix_SetPanning(voice, 128 - (int)(sep * 128), 127 + (int)(sep * 128));
	return voice;
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::PlaySound3D
//
//==========================================================================

int VSDLSoundDevice::PlaySound3D(int, const TVec&, const TVec&, float,
	float, bool)
{
	Sys_Error("SDL driver doesn't support 3D sound");
}

//==========================================================================
//
//  VSDLSoundDevice::UpdateChannel
//
//==========================================================================

void VSDLSoundDevice::UpdateChannel(int Voice, float Vol, float Sep)
{
	guard(VSDLSoundDevice::UpdateChannel);
	if (Voice == -1)
	{
		return;
	}
	Mix_Volume(Voice, (int)(Vol * 127));
	Mix_SetPanning(Voice, 128 - (int)(Sep * 128), 127 + (int)(Sep * 128));
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::UpdateChannel3D
//
//==========================================================================

void VSDLSoundDevice::UpdateChannel3D(int, const TVec&, const TVec&)
{
}

//==========================================================================
//
//	VSDLSoundDevice::IsChannelPlaying
//
//==========================================================================

bool VSDLSoundDevice::IsChannelPlaying(int Voice)
{
	guard(VSDLSoundDevice::IsChannelPlaying);
	if (Voice == -1)
	{
		return false;
	}
	if (Mix_Playing(Voice))
	{
		return true;
	}
	return false;
	unguard;
}

//==========================================================================
//
//  VSDLSoundDevice::StopChannel
//
//==========================================================================

void VSDLSoundDevice::StopChannel(int Voice)
{
	guard(VSDLSoundDevice::StopChannel);
	if (Voice == -1)
	{
		return;
	}
	Mix_HaltChannel(Voice);
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::UpdateListener
//
//==========================================================================

void VSDLSoundDevice::UpdateListener(const TVec&, const TVec&, const TVec&,
	const TVec&, const TVec&)
{
}

//==========================================================================
//
//	VSDLSoundDevice::OpenStream
//
//==========================================================================

bool VSDLSoundDevice::OpenStream(int Rate, int Bits, int Channels)
{
	guard(VSDLSoundDevice::OpenStream);
	//	Build converter struct.
	if (SDL_BuildAudioCVT(&StrmCvt, Bits == 8 ? AUDIO_U8 : AUDIO_S16,
		Channels, Rate, CurFormat, CurChannels, CurFrequency) < 0)
	{
		return false;
	}

	//	Set up buffer.
	StrmBuffer = (byte*)Z_Malloc(STRM_LEN * 4 * StrmCvt.len_mult);
	StrmBufferUsed = 0;

	//	Set up music callback.
	Mix_HookMusic(StrmCallback, this);
	return true;
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::CloseStream
//
//==========================================================================

void VSDLSoundDevice::CloseStream()
{
	guard(VSDLSoundDevice::CloseStream);
	Mix_HookMusic(NULL, NULL);
	if (StrmBuffer)
	{
		Z_Free(StrmBuffer);
		StrmBuffer = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::GetStreamAvailable
//
//==========================================================================

int VSDLSoundDevice::GetStreamAvailable()
{
	guard(VSDLSoundDevice::GetStreamAvailable);
	if (StrmBufferUsed < (STRM_LEN * 4 * StrmCvt.len_mult) * 3 / 4)
		return STRM_LEN / 4;
	return 0;
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::SetStreamData
//
//==========================================================================

short* VSDLSoundDevice::GetStreamBuffer()
{
	guard(VSDLSoundDevice::GetStreamBuffer);
	return (short*)(StrmBuffer + StrmBufferUsed);
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::SetStreamData
//
//==========================================================================

void VSDLSoundDevice::SetStreamData(short* Data, int Len)
{
	guard(VSDLSoundDevice::SetStreamData);
	//	Apply volume.
	for (int i = 0; i < Len * 2; i++)
	{
		Data[i] = short(Data[i] * StrmVol);
	}

	SDL_LockAudio();
	//	Check if data has been used while decoding.
	if (StrmBuffer + StrmBufferUsed != (byte*)Data)
	{
		memmove(StrmBuffer + StrmBufferUsed, Data, Len * 4);
	}

	//	Run the audio converter
	StrmCvt.len = Len * 4;
	StrmCvt.buf = StrmBuffer + StrmBufferUsed;
	SDL_ConvertAudio(&StrmCvt);
	StrmBufferUsed += StrmCvt.len_cvt;
	SDL_UnlockAudio();
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::SetStreamVolume
//
//==========================================================================

void VSDLSoundDevice::SetStreamVolume(float Vol)
{
	guard(VSDLSoundDevice::SetStreamVolume);
	StrmVol = Vol;
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::PauseStream
//
//==========================================================================

void VSDLSoundDevice::PauseStream()
{
	guard(VSDLSoundDevice::PauseStream);
	Mix_PauseMusic();
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::ResumeStream
//
//==========================================================================

void VSDLSoundDevice::ResumeStream()
{
	guard(VSDLSoundDevice::ResumeStream);
	Mix_ResumeMusic();
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::StrmCallback
//
//==========================================================================

void VSDLSoundDevice::StrmCallback(void* ptr, Uint8* stream, int len)
{
	guard(VSDLSoundDevice::StrmCallback);
	VSDLSoundDevice* Self = (VSDLSoundDevice*)ptr;
	if (Self->StrmBufferUsed >= len)
	{
		memcpy(stream, Self->StrmBuffer, len);
		memmove(Self->StrmBuffer, Self->StrmBuffer + len, Self->StrmBufferUsed - len);
		Self->StrmBufferUsed -= len;
	}
	else
	{
		memcpy(stream, Self->StrmBuffer, Self->StrmBufferUsed);
		memset(stream + Self->StrmBufferUsed, 0, len - Self->StrmBufferUsed);
		Self->StrmBufferUsed = 0;
	}
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.23  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
//	Revision 1.22  2005/11/13 14:36:22  dj_jl
//	Moved common sound functions to main sound module.
//	
//	Revision 1.21  2005/11/06 15:27:09  dj_jl
//	Added support for 16 bit sounds.
//	
//	Revision 1.20  2005/11/05 15:50:07  dj_jl
//	Voices played as normal sounds.
//	
//	Revision 1.19  2005/11/03 22:46:35  dj_jl
//	Support for any bitrate streams.
//	
//	Revision 1.18  2005/10/18 20:53:04  dj_jl
//	Implemented basic support for streamed music.
//	
//	Revision 1.17  2005/09/19 23:00:19  dj_jl
//	Streaming support.
//	
//	Revision 1.16  2005/09/12 19:45:16  dj_jl
//	Created midi device class.
//	
//	Revision 1.15  2005/09/04 14:43:45  dj_jl
//	Some fixes.
//	
//	Revision 1.14  2005/05/26 17:00:14  dj_jl
//	Disabled pitching
//	
//	Revision 1.13  2005/04/28 07:16:15  dj_jl
//	Fixed some warnings, other minor fixes.
//	
//	Revision 1.12  2004/11/30 07:17:17  dj_jl
//	Made string pointers const.
//	
//	Revision 1.11  2004/10/18 06:36:45  dj_jl
//	Some fixes.
//	
//	Revision 1.10  2004/10/11 06:49:57  dj_jl
//	SDL patches.
//	
//	Revision 1.9  2004/08/21 19:10:44  dj_jl
//	Changed sound driver declaration.
//	
//	Revision 1.8  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//	
//	Revision 1.7  2003/03/08 12:08:04  dj_jl
//	Beautification.
//	
//	Revision 1.6  2002/08/24 14:50:05  dj_jl
//	Some fixes.
//	
//	Revision 1.5  2002/07/27 18:10:11  dj_jl
//	Implementing Strife conversations.
//	
//	Revision 1.4  2002/07/20 14:49:41  dj_jl
//	Implemented sound drivers.
//	
//	Revision 1.3  2002/01/21 18:27:48  dj_jl
//	Fixed volume
//	
//	Revision 1.2  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2002/01/03 18:39:42  dj_jl
//	Added SDL port
//	
//**************************************************************************
