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

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

#define MAX_SND_DIST		2025
#define PRIORITY_MAX_ADJUST	10

// TYPES -------------------------------------------------------------------

struct channel_t
{
	int			origin_id;
	int			channel;
	TVec		origin;
	TVec		velocity;
	int			sound_id;
	int			priority;
	float		volume;

	Mix_Chunk	*chunk;
	int			voice;
};

class VSDLSoundDevice : public VSoundDevice
{
public:
	void Tick(float DeltaTime);

	void Init(void);
	void Shutdown(void);
	void PlaySound(int sound_id, const TVec &origin, const TVec &velocity,
		int origin_id, int channel, float volume);
	void PlayVoice(const char *Name);
	void PlaySoundTillDone(char *sound);
	void StopSound(int origin_id, int channel);
	void StopAllSound(void);
	bool IsSoundPlaying(int origin_id, int sound_id);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_SOUND_DEVICE(VSDLSoundDevice, SNDDRV_Default, "Default",
	"SDL sound device", NULL);

static TCvarI mix_frequency		("mix_frequency", "22050", CVAR_ARCHIVE);
static TCvarI mix_bits			("mix_bits",      "16",    CVAR_ARCHIVE);
static TCvarI mix_channels		("mix_channels",  "2",     CVAR_ARCHIVE);

static TCvarI mix_chunksize		("mix_chunksize", "4096",  CVAR_ARCHIVE);
static TCvarI mix_voices		("mix_voices",    "8",     CVAR_ARCHIVE);
static TCvarI mix_swapstereo	("mix_swapstereo","0",     CVAR_ARCHIVE);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int sound;
static byte *SoundCurve;
static int sndcount;
static int snd_MaxVolume;

static int frequencies[] = { 11025, 22050, 44100, 0 };
// see SDL/SDL_audio.h for these...
static int chunksizes[] = {  512,  1024,  2048, 4096, 8192, 0};
static int voices[] = {        4,     8,    16,   32,   64, 0};

static channel_t *channels;

static TVec listener_forward;
static TVec listener_right;
static TVec listener_up;

static Uint16 mix_format;

// CODE --------------------------------------------------------------------

////// PRIVATE /////////////////////////////////////////////////////////////

static int know_value(int val, int *vals)
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

static void StopChannel(int chan_num)
{
	if (channels[chan_num].voice >= 0)
	{
		if (channels[chan_num].chunk != NULL)
		{
			Mix_HaltChannel(channels[chan_num].voice);
			Mix_FreeChunk(channels[chan_num].chunk);
		}
		channels[chan_num].chunk = NULL;
		S_DoneWithLump(channels[chan_num].sound_id);
		channels[chan_num].voice = -1;
		channels[chan_num].origin_id = 0;
		channels[chan_num].sound_id = 0;
	}
}

static int CalcDist(const TVec &origin)
{
	return (int)Length(origin - cl.vieworg);
}

static int CalcPriority(int sound_id, int dist)
{
	return S_sfx[sound_id].priority * (PRIORITY_MAX_ADJUST - PRIORITY_MAX_ADJUST * dist / MAX_SND_DIST);
}

static int GetChannel(int sound_id, int origin_id, int channel, int priority)
{
	int chan;
	int i;

	int lp; // least priority
	int found;
	int prior;
	int numchannels = sound_id == VOICE_SOUND_ID ? 1 : S_sfx[sound_id].numchannels;

	if (numchannels != -1)
	{
		lp = -1; // denote the argument sound_id
		found = 0;
		prior = priority;
		for (i = 0; i < mix_voices; i++)
		{
			if (channels[i].sound_id == sound_id && channels[i].voice >= 0)
			{
				found++; // found one.  Now, should we replace it??
				if (prior >= channels[i].priority)
				{
					// if we're gonna kill one, then this'll be it
					lp = i;
					prior = channels[i].priority;
				}
			}
		}

		if (found >= numchannels)
		{
			if (lp == -1)
			{
				// other sounds have greater priority
				return -1; // don't replace any sounds
			}
			StopChannel(lp);
		}
	}

	// Only one sound per channel
	if (origin_id && channel)
	{
		for (i = 0; i < mix_voices; i++)
		{
			if (channels[i].origin_id == origin_id && channels[i].channel == channel)
			{
				StopChannel(i);
				return i;
			}
		}
	}

	// Look for a free channel
	for (i = 0; i < mix_voices; i++)
	{
		if (channels[i].voice < 0)
		{
			return i;
		}
	}

	// Look for a lower priority sound to replace.
	sndcount++;
	if (sndcount >= mix_voices)
	{
		sndcount = 0;
	}

	for (chan = 0; chan < mix_voices; chan++)
	{
		i = (sndcount + chan) % mix_voices;
		if (priority >= channels[i].priority)
		{
			// replace the lower priority sound.
			StopChannel(i);
			return i;
		}
	}

	// no free channels.
	return -1;
}

static int CalcVol(float volume, int dist)
{
	return int(SoundCurve[dist] * volume);
}

static int CalcSep(const TVec &origin)
{
	TVec  dir;
	float dot;
	int   sep;

	dir = origin - cl.vieworg;
	dot = DotProduct(dir, listener_right);
	sep = 127 + (int)(dot * 128.0 / MAX_SND_DIST);

	if (mix_swapstereo)
	{
		sep = 255 - sep;
	}

	return sep;
}

static int CalcPitch(int freq, int sound_id)
{
	if (S_sfx[sound_id].changePitch)
	{
		return freq + ((freq * ((rand() & 7) - (rand() & 7))) >> 7);
	}
	else
	{
		return freq;
	}
}

/* Load raw data -- this is a hacked version of Mix_LoadWAV_RW (gl) */
static Mix_Chunk * LoadRaw(Uint8 *data, int len, int freq)
{
	Mix_Chunk *chunk;
	SDL_AudioCVT cvt;

	if ( SDL_BuildAudioCVT(&cvt, AUDIO_U8, 1, freq,
			mix_format, mix_channels, mix_frequency) < 0 ) {
		return(NULL);
	}

	cvt.len = len;
	cvt.buf = (Uint8 *)malloc(len * cvt.len_mult);
	memcpy(cvt.buf, data, len);

	/* Run the audio converter */
	if ( SDL_ConvertAudio(&cvt) < 0 ) {
		free(cvt.buf);
		return(NULL);
	}

	chunk = (Mix_Chunk *)malloc(sizeof(Mix_Chunk));
	chunk->allocated = 1;
	chunk->abuf = cvt.buf;
	chunk->alen = cvt.len_cvt;
	chunk->volume = MIX_MAX_VOLUME;
	return(chunk);
}

////// PUBLIC //////////////////////////////////////////////////////////////

//==========================================================================
//
//  VSDLSoundDevice::Init
//
// 	Inits sound
//
//==========================================================================

void VSDLSoundDevice::Init(void)
{
	guard(VSDLSoundDevice::Init);
	int i;
	int    freq;
	Uint16 fmt;
	int    ch;  /* audio */
	int    mch; /* mixer */
	int    cksz;
	char   dname[32];

	if (M_CheckParm("-nosound") ||
		(M_CheckParm("-nosfx") && M_CheckParm("-nomusic")))
	{
		return;
	}

	if ( (i = M_CheckParm("-mix_frequency")) )
		mix_frequency = atoi(myargv[i+1]);

	if (know_value(mix_frequency,frequencies))
		freq = mix_frequency;
	else
		freq = MIX_DEFAULT_FREQUENCY;

	if ( (i = M_CheckParm("-mix_bits")) )
		mix_bits = atoi(myargv[i+1]);

	if (mix_bits == 8)
		fmt = AUDIO_U8;
	else
		fmt = MIX_DEFAULT_FORMAT;

	if ( (i = M_CheckParm("-mix_channels")) )
		mix_channels = atoi(myargv[i+1]);
			
	if (mix_channels == 1 || mix_channels == 2 || mix_channels == 4 || mix_channels == 6)
		ch = mix_channels;
	else
		ch = MIX_DEFAULT_CHANNELS;

	if ( (i = M_CheckParm("-mix_chunksize")) )
		mix_chunksize = atoi(myargv[i+1]);

	if (know_value(mix_chunksize, chunksizes))
		cksz = mix_chunksize;
	else
		cksz = 4096;

	if ( (i = M_CheckParm("-mix_voices")) )
		mix_voices = atoi(myargv[i+1]);

	if (know_value(mix_voices, voices))
		mch = mix_voices;
	else
		mch = MIX_CHANNELS;

	if ( (i = M_CheckParm("-mix_swapstereo")) )
		mix_swapstereo = 1;

	if (Mix_OpenAudio(freq,fmt,ch,cksz) < 0)
	{
			sound = 0;
			return;
	}

	sound = 1;

	Mix_QuerySpec(&freq, &fmt, &ch);

	if (!M_CheckParm("-nosfx"))
	{
		mix_voices = Mix_AllocateChannels(mch);
	}
	else
	{
		mix_voices = 0;
	}

	mix_frequency = freq;
	mix_bits = fmt & 0xFF;
	mix_channels = ch;
	mix_format = fmt;

	channels = Z_CNew<channel_t>(mix_voices);
	for (i = 0; i < mix_voices; i++)
	{
		channels[i].voice = -1;
		channels[i].chunk = NULL;
	}

	SoundCurve = (byte*)W_CacheLumpName("SNDCURVE", PU_STATIC);

	sndcount = 0;
	snd_MaxVolume = -1;

	GCon->Logf(NAME_Init, "Configured audio device for %d channels, format %04X.", (int)mix_channels, mix_format);
	GCon->Logf(NAME_Init, "Driver   : %s", SDL_AudioDriverName(dname, 32));

	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::Shutdown
//
//==========================================================================

void VSDLSoundDevice::Shutdown(void)
{
	guard(VSDLSoundDevice::Shutdown);
	if (sound)
	{
		Mix_CloseAudio();
		if (mix_voices) Z_Free(channels);
		sound = 0;
	}
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::PlaySound
//
// 	This function adds a sound to the list of currently active sounds, which
// is maintained as a given number of internal channels.
//
//==========================================================================

void VSDLSoundDevice::PlaySound(int sound_id, const TVec &origin,
	const TVec &velocity, int origin_id, int channel, float volume)
{
	guard(VSDLSoundDevice::PlaySound);
	Mix_Chunk *chunk;
	int dist;
	int priority;
	int chan;
	int voice;
	int vol;
	int sep;
	int pitch;

	if (!mix_voices || !sound_id || !snd_MaxVolume || !volume)
	{
		return;
	}

	// calculate the distance before other stuff so that we can throw out
	// sounds that are beyond the hearing range.
	dist = 0;
	if (origin_id && origin_id != cl.clientnum + 1)
		dist = CalcDist(origin);
	if (dist >= MAX_SND_DIST)
	{
		return; // sound is beyond the hearing range...
	}

	priority = CalcPriority(sound_id, dist);

	chan = GetChannel(sound_id, origin_id, channel, priority);
	if (chan == -1)
	{
		return; //no free channels.
	}

	if (channels[chan].voice >= 0)
	{
		Sys_Error("I_StartSound: Previous sound not stoped");
	}

	if (!S_LoadSound(sound_id))
	{
		//	Missing sound.
		return;
	}

	pitch = CalcPitch(S_sfx[sound_id].freq, sound_id);

	// copy the lump to a SDL_Mixer chunk...
	chunk = LoadRaw((Uint8 *)S_sfx[sound_id].data, S_sfx[sound_id].len, pitch);

	if (chunk == NULL)
		Sys_Error("LoadRaw() failed!\n");

	vol = CalcVol(volume, dist);

	voice = Mix_PlayChannelTimed(-1, chunk, 0, -1);

	if (voice < 0)
	{
		S_DoneWithLump(sound_id);
		return;
	}

	if (dist)
	{
		sep = CalcSep(origin);
//JL		Mix_SetPanning(voice, 255 - sep, sep);
	}
//	pitch = CalcPitch(S_sfx[sound_id].freq, sound_id);
//#warning how to set the pitch? (CS)

	// ready to go...
	//Mix_Play(voice);

	channels[chan].origin_id = origin_id;
	channels[chan].origin    = origin;
	channels[chan].channel   = channel;
	channels[chan].velocity  = velocity;
	channels[chan].sound_id  = sound_id;
	channels[chan].priority  = priority;
	channels[chan].volume    = volume;
	channels[chan].voice     = voice;
	channels[chan].chunk     = chunk;
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::PlayVoice
//
//==========================================================================

void VSDLSoundDevice::PlayVoice(const char *Name)
{
	guard(VSDLSoundDevice::PlayVoice);
	Mix_Chunk *chunk;
	int priority;
	int chan;
	int voice;

	if (!mix_voices || !*Name || !snd_MaxVolume)
	{
		return;
	}

	priority = 255 * PRIORITY_MAX_ADJUST;

	chan = GetChannel(VOICE_SOUND_ID, 0, 1, priority);
	if (chan == -1)
	{
		return; //no free channels.
	}

	if (channels[chan].voice >= 0)
	{
		Sys_Error("I_StartSound: Previous sound not stoped");
	}

	if (!S_LoadSound(VOICE_SOUND_ID, Name))
	{
		//	Missing sound.
		return;
	}


	// copy the lump to a SDL_Mixer chunk...
	chunk = LoadRaw((Uint8 *)S_VoiceInfo.data, S_VoiceInfo.len, S_VoiceInfo.freq);

	if (chunk == NULL)
		Sys_Error("Mix_LoadRAW_RW() failed!\n");

	voice = Mix_PlayChannelTimed(-1, chunk, 0, -1);

	if (voice < 0)
	{
		S_DoneWithLump(VOICE_SOUND_ID);
		return;
	}

	channels[chan].origin_id = 0;
	channels[chan].origin    = TVec(0, 0, 0);
	channels[chan].channel   = 1;
	channels[chan].velocity  = TVec(0, 0, 0);
	channels[chan].sound_id  = VOICE_SOUND_ID;
	channels[chan].priority  = priority;
	channels[chan].volume    = 1.0;
	channels[chan].voice     = voice;
	channels[chan].chunk     = chunk;
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::PlaySoundTillDone
//
//==========================================================================

void VSDLSoundDevice::PlaySoundTillDone(char *sound)
{
	guard(VSDLSoundDevice::PlaySoundTillDone);
	int    sound_id;
	double start;
	int    voice;
	Mix_Chunk *chunk;

	if (!mix_voices || !snd_MaxVolume)
	{
		return;
	}

	sound_id = S_GetSoundID(sound);

	if (!sound_id)
	{
		return;
	}

	//	All sounds must be stoped
	S_StopAllSound();

	if (!S_LoadSound(sound_id))
	{
		//	Missing sound.
		return;
	}

	chunk = LoadRaw((Uint8 *)S_sfx[sound_id].data, S_sfx[sound_id].len, S_sfx[sound_id].freq);

	if (chunk == NULL)
		Sys_Error("Mix_LoadRAW_RW() failed!\n");

	voice = Mix_PlayChannelTimed(-1, chunk, 0, -1);

	if (voice < 0)
	{
		return;
	}

	start = Sys_Time();
	while (1)
	{
		if (!Mix_Playing(voice))
		{
			//	Sound stoped
			break;
		}

		if (Sys_Time() > start + 10.0)
		{
			//	Don't play longer than 10 seconds
			break;
		}
	}
	Mix_HaltChannel(voice);
	Mix_FreeChunk(chunk);
	unguard;
}

//==========================================================================
//
//  VSDLSoundDevice::Tick
//
// 	Update the sound parameters. Used to control volume and pan
// changes such as when a player turns.
//
//==========================================================================

void VSDLSoundDevice::Tick(float DeltaTime)
{
	guard(VSDLSoundDevice::Tick);
	int		i;
	int		dist;
	int		vol;
	int		sep;

	if (!mix_voices || !snd_MaxVolume)
	{
		return;
	}

	if (sfx_volume < 0)
	{
		sfx_volume = 0;
	}
	if (sfx_volume > 15)
	{
		sfx_volume = 15;
	}

	if (sfx_volume != snd_MaxVolume)
	{
		snd_MaxVolume = sfx_volume;
		// set_volume(snd_MaxVolume * 17, -1);
		if (!snd_MaxVolume)
		{
			S_StopAllSound();
		}
	}

	if (!snd_MaxVolume)
	{
		// Silence
		return;
	}

	AngleVectors(cl.viewangles, listener_forward, listener_right, listener_up);

	for (i = 0; i < mix_voices; i++)
	{
		if (channels[i].voice < 0)
		{
			// Nothing on this channel
			continue;
		}

		if (!Mix_Playing(channels[i].voice))
		{
			// Sound playback done
			StopChannel(i);
			continue;
		}

		if (!channels[i].origin_id)
		{
			// Nothing to update
			continue;
		}

		if (channels[i].origin_id == cl.clientnum + 1)
		{
			// Nothing to update
			continue;
		}

		//	Move sound
		channels[i].origin += channels[i].velocity * DeltaTime;

		dist = CalcDist(channels[i].origin);
		if (dist >= MAX_SND_DIST)
		{
			// Too far away
			StopChannel(i);
			continue;
		}

		// Update params
		vol = CalcVol(channels[i].volume, dist);
		sep = CalcSep(channels[i].origin);

		Mix_Volume(channels[i].voice, vol);
//JLK		Mix_SetPanning(channels[i].voice, 255 - sep, sep);

		channels[i].priority = CalcPriority(channels[i].sound_id, dist);
	}
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::StopSound
//
//==========================================================================

void VSDLSoundDevice::StopSound(int origin_id, int channel)
{
	guard(VSDLSoundDevice::StopSound);
	int i;

	if (!mix_voices || !snd_MaxVolume)
	{
		return;
	}

	for (i = 0; i < mix_voices; i++)
	{
		if (channels[i].origin_id == origin_id &&
			(!channel || channels[i].channel == channel))
		{
			StopChannel(i);
		}
	}
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::StopAllSound
//
//==========================================================================

void VSDLSoundDevice::StopAllSound(void)
{
	guard(VSDLSoundDevice::StopAllSound);
	int i;

	if (!mix_voices || !snd_MaxVolume)
	{
		return;
	}

	//	stop all sounds
	for (i = 0; i < mix_voices; i++)
	{
		StopChannel(i);
	}
	unguard;
}

//==========================================================================
//
//	VSDLSoundDevice::IsSoundPlaying
//
//==========================================================================

bool VSDLSoundDevice::IsSoundPlaying(int origin_id, int sound_id)
{
	guard(VSDLSoundDevice::IsSoundPlaying);
	int i;

	if (!mix_voices || !snd_MaxVolume)
	{
		return false;
	}

	for (i = 0; i < mix_voices; i++)
	{
		if (channels[i].sound_id == sound_id &&
			channels[i].origin_id == origin_id &&
			channels[i].voice >= 0)
		{
			if (Mix_Playing(channels[i].voice))
			{
				return true;
			}
		}
	}
	return false;
	unguard;
}

//**************************************************************************
//
//	$Log$
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
