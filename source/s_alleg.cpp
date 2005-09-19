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

#define MAX_SND_DIST			2025
#define PRIORITY_MAX_ADJUST		10

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

	SAMPLE		spl;
	int			voice;
};

class VAllegroSoundDevice : public VSoundDevice
{
public:
	void Tick(float DeltaTime);

	void Init(void);
	void Shutdown(void);
	void PlaySound(int sound_id, const TVec &origin, const TVec &velocity,
		int origin_id, int channel, float volume);
	void PlayVoice(const char *Name);
	void PlaySoundTillDone(const char *sound);
	void StopSound(int origin_id, int channel);
	void StopAllSound(void);
	bool IsSoundPlaying(int origin_id, int sound_id);

	bool OpenStream();
	void CloseStream();
	int GetStreamAvailable();
	short* GetStreamBuffer();
	void SetStreamData(short* Data, int Len);
	void SetStreamVolume(float);
	void PauseStream();
	void ResumeSteam();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void StopChannel(int chan_num);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_SOUND_DEVICE(VAllegroSoundDevice, SNDDRV_Default, "Default",
	"Allegro sound device", NULL);

bool				allegro_sound_initialised;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static float		snd_MaxVolume = -1;      // maximum volume for sound

static channel_t	Channel[DIGI_VOICES];
static int			snd_Channels = 0;   // number of channels available

static byte*		SoundCurve;

static int 			sndcount = 0;

static TVec			listener_forward;
static TVec			listener_right;
static TVec			listener_up;

static AUDIOSTREAM*	Strm;
static void*		StrmBuf;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  VAllegroSoundDevice::Init
//
// 	Inits sound
//
//==========================================================================

void VAllegroSoundDevice::Init(void)
{
	guard(VAllegroSoundDevice::Init);
	int		sound_card;
	int		music_card;
	int		i;

	// Default settings
	sound_card = DIGI_AUTODETECT;
	music_card = MIDI_AUTODETECT;

	// Check parametters
	if (M_CheckParm("-nosound") || M_CheckParm("-nosfx"))
		sound_card = DIGI_NONE;
	if (M_CheckParm("-nosound") || M_CheckParm("-nomusic"))
		music_card = MIDI_NONE;

	// Init sound device
	if (install_sound(sound_card, music_card, NULL) == -1)
	{
		Sys_Error("ALLEGRO SOUND INIT ERROR!!!!\n%s\n", allegro_error);
	}
	allegro_sound_initialised = true;

	snd_Channels = digi_driver->voices;

	// Free all channels for use
	memset(Channel, 0, sizeof(Channel));
	for (i = 0; i < snd_Channels; i++)
	{
		Channel[i].voice = -1;
	}

	SoundCurve = (byte*)W_CacheLumpName("SNDCURVE", PU_STATIC);

	GCon->Logf(NAME_Init, "configured audio device");
	GCon->Logf(NAME_Init, "SFX   : %s", digi_driver->desc);
	GCon->Logf(NAME_Init, "Music : %s", midi_driver->desc);
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
	remove_sound();
	unguard;
}

//==========================================================================
//
//	GetChannel
//
//==========================================================================

static int GetChannel(int sound_id, int origin_id, int channel, int priority)
{
	int 		chan;
	int			i;

	int			lp; //least priority
	int			found;
	int			prior;
	int numchannels = sound_id == VOICE_SOUND_ID ? 1 : S_sfx[sound_id].numchannels;

	if (numchannels != -1)
	{
		lp = -1; //denote the argument sound_id
		found = 0;
		prior = priority;
		for (i = 0; i < snd_Channels; i++)
		{
			if (Channel[i].sound_id == sound_id && Channel[i].voice >= 0)
			{
				found++; //found one.  Now, should we replace it??
				if (prior >= Channel[i].priority)
				{
					// if we're gonna kill one, then this'll be it
					lp = i;
					prior = Channel[i].priority;
				}
			}
		}

		if (found >= numchannels)
		{
			if (lp == -1)
			{// other sounds have greater priority
				return -1; // don't replace any sounds
			}
			StopChannel(lp);
		}
	}

	//	Only one sound per channel
	if (origin_id && channel)
	{
		for (i = 0; i < snd_Channels; i++)
		{
			if (Channel[i].origin_id == origin_id &&
				Channel[i].channel == channel)
			{
				StopChannel(i);
				return i;
			}
		}
	}

	//	Look for a free channel
	for (i = 0; i < snd_Channels; i++)
	{
		if (Channel[i].voice < 0)
		{
			return i;
		}
	}

	//	Look for a lower priority sound to replace.
	sndcount++;
	if (sndcount >= snd_Channels)
	{
		sndcount = 0;
	}

	for (chan = 0; chan < snd_Channels; chan++)
	{
		i = (sndcount + chan) % snd_Channels;
		if (priority >= Channel[i].priority)
		{
			//replace the lower priority sound.
			StopChannel(i);
			return i;
		}
	}

	//	no free channels.
	return -1;
}

//==========================================================================
//
//	CalcDist
//
//==========================================================================

static int CalcDist(const TVec &origin)
{
	return (int)Length(origin - cl.vieworg);
}

//==========================================================================
//
//	CalcPriority
//
//==========================================================================

static int CalcPriority(int sound_id, int dist)
{
	return S_sfx[sound_id].priority *
		(PRIORITY_MAX_ADJUST - PRIORITY_MAX_ADJUST * dist / MAX_SND_DIST);
}

//==========================================================================
//
//	CalcVol
//
//==========================================================================

static int CalcVol(float volume, int dist)
{
	return int(SoundCurve[dist] * 2 * volume);
}

//==========================================================================
//
//	CalcSep
//
//==========================================================================

static int CalcSep(const TVec &origin)
{
	TVec		dir;
	float		dot;
	int			sep;

	dir = origin - cl.vieworg;
	dot = DotProduct(dir, listener_right);
	sep = 127 + (int)(dot * 128.0 / MAX_SND_DIST);

	if (swap_stereo)
	{
		sep = 255 - sep;
	}

	return sep;
}

//==========================================================================
//
//	CalcPitch
//
//==========================================================================

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

//==========================================================================
//
//	VAllegroSoundDevice::PlaySound
//
// 	This function adds a sound to the list of currently active sounds, which
// is maintained as a given number of internal channels.
//
//==========================================================================

void VAllegroSoundDevice::PlaySound(int sound_id, const TVec &origin,
	const TVec &velocity, int origin_id, int channel, float volume)
{
	guard(VAllegroSoundDevice::PlaySound);
	SAMPLE*		spl;
	int 		dist;
	int 		priority;
	int			chan;
	int			voice;
	int 		vol;
	int 		sep;
	int			pitch;

	if (!snd_Channels || !sound_id || !snd_MaxVolume || !volume)
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

	if (Channel[chan].voice >= 0)
	{
		Sys_Error("I_StartSound: Previous sound not stoped");
	}

	if (!S_LoadSound(sound_id))
	{
		//	Missing sound.
		return;
	}

	//	Converts raw 11khz, 8-bit data to a SAMPLE* that allegro uses.
	spl = &Channel[chan].spl;

	spl->bits 		= 8;
	spl->stereo 	= 0;
	spl->freq 		= S_sfx[sound_id].freq;
	spl->priority 	= MID(0, priority / PRIORITY_MAX_ADJUST, 255);
	spl->len 		= S_sfx[sound_id].len;
	spl->loop_start = 0;
	spl->loop_end 	= S_sfx[sound_id].len;
	spl->param 		= 0xffffffff;
	spl->data 		= S_sfx[sound_id].data;

	// Start the sound
	voice = allocate_voice(spl);

	if (voice < 0)
	{
		S_DoneWithLump(sound_id);
		return;
	}

	vol = CalcVol(volume, dist);
	voice_set_volume(voice, vol);
	if (dist)
	{
		sep = CalcSep(origin);
		voice_set_pan(voice, sep);
	}
	pitch = CalcPitch(spl->freq, sound_id);
	voice_set_frequency(voice, pitch);
	voice_set_playmode(voice, PLAYMODE_PLAY);
	voice_start(voice);
	release_voice(voice);

	Channel[chan].origin_id = origin_id;
	Channel[chan].origin = origin;
	Channel[chan].channel = channel;
	Channel[chan].velocity = velocity;
	Channel[chan].sound_id = sound_id;
	Channel[chan].priority = priority;
	Channel[chan].volume = volume;
	Channel[chan].voice = voice;
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::PlayVoice
//
//==========================================================================

void VAllegroSoundDevice::PlayVoice(const char *Name)
{
	guard(VAllegroSoundDevice::PlayVoice);
	SAMPLE*		spl;
	int 		priority;
	int			chan;
	int			voice;

	if (!snd_Channels || !*Name || !snd_MaxVolume)
	{
		return;
	}

	priority = 255 * PRIORITY_MAX_ADJUST;

	chan = GetChannel(VOICE_SOUND_ID, 0, 1, priority);
	if (chan == -1)
	{
		return; //no free channels.
	}

	if (Channel[chan].voice >= 0)
	{
		Sys_Error("I_StartSound: Previous sound not stoped");
	}

	if (!S_LoadSound(VOICE_SOUND_ID, Name))
	{
		//	Missing sound.
		return;
	}

	//	Converts raw 11khz, 8-bit data to a SAMPLE* that allegro uses.
	spl = &Channel[chan].spl;

	spl->bits 		= 8;
	spl->stereo 	= 0;
	spl->freq 		= S_VoiceInfo.freq;
	spl->priority 	= 255;
	spl->len 		= S_VoiceInfo.len;
	spl->loop_start = 0;
	spl->loop_end 	= S_VoiceInfo.len;
	spl->param 		= 0xffffffff;
	spl->data 		= S_VoiceInfo.data;

	// Start the sound
	voice = allocate_voice(spl);

	if (voice < 0)
	{
		S_DoneWithLump(VOICE_SOUND_ID);
		return;
	}

	voice_set_playmode(voice, PLAYMODE_PLAY);
	voice_start(voice);
	release_voice(voice);

	Channel[chan].origin_id = 0;
	Channel[chan].origin = TVec(0 , 0, 0);
	Channel[chan].channel = 1;
	Channel[chan].velocity = TVec(0, 0, 0);
	Channel[chan].sound_id = VOICE_SOUND_ID;
	Channel[chan].priority = priority;
	Channel[chan].volume = 1.0;
	Channel[chan].voice = voice;
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::PlaySoundTillDone
//
//==========================================================================

void VAllegroSoundDevice::PlaySoundTillDone(const char *sound)
{
	guard(VAllegroSoundDevice::PlaySoundTillDone);
	int			sound_id;
	double		start;
	SAMPLE		spl;
	int			voice;

	sound_id = S_GetSoundID(sound);
	if (!snd_Channels || !sound_id || !snd_MaxVolume)
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

	//	Converts raw 11khz, 8-bit data to a SAMPLE* that allegro uses.
	spl.bits 		= 8;
	spl.stereo 		= 0;
	spl.freq 		= S_sfx[sound_id].freq;
	spl.priority 	= 255;
	spl.len 		= S_sfx[sound_id].len;
	spl.loop_start	= 0;
	spl.loop_end 	= S_sfx[sound_id].len;
	spl.param 		= 0xffffffff;
	spl.data 		= S_sfx[sound_id].data;

	// Start the sound
	voice = allocate_voice(&spl);

	if (voice < 0)
	{
		return;
	}

	voice_set_playmode(voice, PLAYMODE_PLAY);
	voice_start(voice);
	release_voice(voice);

	start = Sys_Time();
	while (1)
	{
		if (voice_check(voice) != &spl)
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

	deallocate_voice(voice);
	unguard;
}

//==========================================================================
//
//  VAllegroSoundDevice::Tick
//
// 	Update the sound parameters. Used to control volume, pan, and pitch
// changes such as when a player turns.
//
//==========================================================================

void VAllegroSoundDevice::Tick(float DeltaTime)
{
	guard(VAllegroSoundDevice::Tick);
	int 		i;
	int			dist;
	int			vol;
	int 		sep;

	if (sfx_volume < 0.0)
	{
		sfx_volume = 0.0;
	}
	if (sfx_volume > 1.0)
	{
		sfx_volume = 1.0;
	}

	if (sfx_volume != snd_MaxVolume)
	{
		snd_MaxVolume = sfx_volume;
		set_volume(int(snd_MaxVolume * 255), -1);
		if (!snd_MaxVolume)
		{
			S_StopAllSound();
		}
	}

	if (!snd_MaxVolume)
	{
		//	Silence
		return;
	}

	AngleVectors(cl.viewangles, listener_forward, listener_right, listener_up);

	for (i = 0; i < snd_Channels; i++)
	{
		if (Channel[i].voice < 0)
		{
			//	Nothing on this channel
			continue;
		}

		if (voice_check(Channel[i].voice) != &Channel[i].spl)
		{
			//	Sound playback done
			StopChannel(i);
			continue;
		}

		if (!Channel[i].origin_id)
		{
			//	Nothing to update
			continue;
		}

		if (Channel[i].origin_id == cl.clientnum + 1)
		{
			//	Nothing to update
			continue;
		}

		//	Move sound
		Channel[i].origin += Channel[i].velocity * DeltaTime;

		dist = CalcDist(Channel[i].origin);
		if (dist >= MAX_SND_DIST)
		{
			//	Too far away
			StopChannel(i);
			continue;
		}

		//	Update params
		vol = CalcVol(Channel[i].volume, dist);
		sep = CalcSep(Channel[i].origin);

		voice_set_volume(Channel[i].voice, vol);
		voice_set_pan(Channel[i].voice, sep);

		Channel[i].priority = CalcPriority(Channel[i].sound_id, dist);
	}
	unguard;
}

//==========================================================================
//
//  StopChannel
//
//	Stop the sound. Necessary to prevent runaway chainsaw, and to stop
// rocket launches when an explosion occurs.
//	All sounds MUST be stopped;
//
//==========================================================================

static void StopChannel(int chan_num)
{
	if (Channel[chan_num].voice >= 0)
	{
		deallocate_voice(Channel[chan_num].voice);
		S_DoneWithLump(Channel[chan_num].sound_id);
		Channel[chan_num].voice = -1;
		Channel[chan_num].origin_id = 0;
		Channel[chan_num].sound_id = 0;
	}
}

//==========================================================================
//
//	VAllegroSoundDevice::StopSound
//
//==========================================================================

void VAllegroSoundDevice::StopSound(int origin_id, int channel)
{
	guard(VAllegroSoundDevice::StopSound);
	int i;

	for (i = 0; i < snd_Channels; i++)
	{
		if (Channel[i].origin_id == origin_id &&
			(!channel || Channel[i].channel == channel))
		{
			StopChannel(i);
		}
	}
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::StopAllSound
//
//==========================================================================

void VAllegroSoundDevice::StopAllSound(void)
{
	guard(VAllegroSoundDevice::StopAllSound);
	int i;

	//	stop all sounds
	for (i = 0; i < snd_Channels; i++)
	{
		StopChannel(i);
	}
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::IsSoundPlaying
//
//==========================================================================

bool VAllegroSoundDevice::IsSoundPlaying(int origin_id, int sound_id)
{
	guard(VAllegroSoundDevice::IsSoundPlaying);
	int i;

	for (i = 0; i < snd_Channels; i++)
	{
		if (Channel[i].sound_id == sound_id &&
			Channel[i].origin_id == origin_id &&
			Channel[i].voice >= 0)
		{
			if (voice_check(Channel[i].voice) == &Channel[i].spl)
			{
				return true;
			}
		}
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VAllegroSoundDevice::OpenStream
//
//==========================================================================

#define STRM_LEN		(8 * 1024)

bool VAllegroSoundDevice::OpenStream()
{
	guard(VAllegroSoundDevice::OpenStream);
	Strm = play_audio_stream(STRM_LEN, 16, 1, 44100, 255, 127);
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
//	VAllegroSoundDevice::ResumeSteam
//
//==========================================================================

void VAllegroSoundDevice::ResumeSteam()
{
	guard(VAllegroSoundDevice::ResumeSteam);
	if (Strm)
	{
		voice_start(Strm->voice);
	}
	unguard;
}

//**************************************************************************
//
//	$Log$
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
