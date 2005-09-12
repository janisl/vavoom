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

#ifdef _WIN32
#include "winlocal.h"
#else
#define INITGUID
#endif
#include <AL/al.h>
#include <AL/alc.h>
//	Linux headers doesn't define this
#ifndef OPENAL
#define OPENAL
#endif
#include "eax.h"

#include "gamedefs.h"
#include "s_local.h"

struct FALChannel
{
	int			origin_id;
	int			channel;
	TVec		origin;
	TVec		velocity;
	int			sound_id;
	int			priority;
	float		volume;

	ALuint		source;
};

class VOpenALDevice : public VSoundDevice
{
private:
	enum { MAX_VOICES = 256 };

	enum { MAX_SND_DIST = 2025 };
	enum { PRIORITY_MAX_ADJUST = 10 };
	enum { DIST_ADJUST = MAX_SND_DIST / PRIORITY_MAX_ADJUST };

	ALCdevice*	Device;
	ALCcontext*	Context;
	ALuint*		Buffers;
	ALuint		VoiceBuffer;

	int			snd_MaxVolume;	// maximum volume for sound

	FALChannel	Channel[MAX_VOICES];
	int			snd_Channels;	// number of channels available

	int 		sndcount;
	bool		supportEAX;

	EAXGet		pEAXGet;
	EAXSet		pEAXSet;

	TVec		listener_forward;
	TVec		listener_right;
	TVec		listener_up;

	static TCvarF		doppler_factor;
	static TCvarF		doppler_velocity;
	static TCvarF		rolloff_factor;
	static TCvarF		reference_distance;
	static TCvarF		max_distance;
	static TCvarI		eax_environment;

public:
	//	VSubsystem interface.
	void Tick(float DeltaTime);

	//	VSoundDevice interface.
	void Init(void);
	void Shutdown(void);
	void PlaySound(int sound_id, const TVec &origin, const TVec &velocity,
		int origin_id, int channel, float volume);
	void PlayVoice(const char *Name);
	void PlaySoundTillDone(const char *sound);
	void StopSound(int origin_id, int channel);
	void StopAllSound(void);
	bool IsSoundPlaying(int origin_id, int sound_id);

private:
	int GetChannel(int sound_id, int origin_id, int channel, int priority);
	void StopChannel(int chan_num);

	int CalcDist(const TVec &origin)
	{
		return (int)Length(origin - cl.vieworg);
	}

	int CalcPriority(int sound_id, int dist)
	{
		return S_sfx[sound_id].priority *
			(PRIORITY_MAX_ADJUST - (dist / DIST_ADJUST));
	}
};

IMPLEMENT_SOUND_DEVICE(VOpenALDevice, SNDDRV_OpenAL, "OpenAL",
	"OpenAL sound device", "-openal");

TCvarF VOpenALDevice::doppler_factor("al_doppler_factor", "1.0", CVAR_ARCHIVE);
TCvarF VOpenALDevice::doppler_velocity("al_doppler_velocity", "10000.0", CVAR_ARCHIVE);
TCvarF VOpenALDevice::rolloff_factor("al_rolloff_factor", "1.0", CVAR_ARCHIVE);
TCvarF VOpenALDevice::reference_distance("al_reference_distance", "64.0", CVAR_ARCHIVE);
TCvarF VOpenALDevice::max_distance("al_max_distance", "2024.0", CVAR_ARCHIVE);
TCvarI VOpenALDevice::eax_environment("al_eax_environment", "0");

//==========================================================================
//
//  VOpenALDevice::Init
//
// 	Inits sound
//
//==========================================================================

void VOpenALDevice::Init(void)
{
	guard(VOpenALDevice::Init);
	ALenum E;

	if (M_CheckParm("-nosound") ||
		(M_CheckParm("-nosfx") && M_CheckParm("-nomusic")))
	{
		return;
	}

	//	Setup sound
	if (!M_CheckParm("-nosfx"))
	{
		//	Connect to a device.
		Device = alcOpenDevice(NULL);
		if (!Device)
		{
			GCon->Log(NAME_Init, "Couldn't open OpenAL device");
			return;
		}
		//	In Linux it's not implemented.
#ifdef ALC_DEVICE_SPECIFIER
		GCon->Logf(NAME_Init, "Opened OpenAL device %s", alcGetString(Device, ALC_DEVICE_SPECIFIER));
#endif

		//	Create a context and make it current.
		Context = alcCreateContext(Device, NULL);
		if (!Context)
		{
			Sys_Error("Failed to create OpenAL context");
		}
		alcMakeContextCurrent(Context);
		E = alGetError();
		if (E != AL_NO_ERROR)
		{
			Sys_Error("OpenAL error: %s", alGetString(E));
		}

		//	Print some information.
		GCon->Logf(NAME_Init, "AL_VENDOR: %s", alGetString(AL_VENDOR));
		GCon->Logf(NAME_Init, "AL_RENDERER: %s", alGetString(AL_RENDERER));
		GCon->Logf(NAME_Init, "AL_VERSION: %s", alGetString(AL_VERSION));
		GCon->Log(NAME_Init, "AL_EXTENSIONS:");
		char *sbuf = Z_StrDup((char*)alGetString(AL_EXTENSIONS));
		for (char *s = strtok(sbuf, " "); s; s = strtok(NULL, " "))
		{
			GCon->Logf(NAME_Init, "- %s", s);
		}
		Z_Free(sbuf);

		if (alIsExtensionPresent((ALubyte *)"EAX"))
		{
			GCon->Log(NAME_Init, "EAX 2.0 supported");
			pEAXSet = (EAXSet)alGetProcAddress((ALubyte *)"EAXSet");
			pEAXGet = (EAXGet)alGetProcAddress((ALubyte *)"EAXGet");
			supportEAX = true;
		}

		//	Allocate array for buffers.
		Buffers = Z_CNew<ALuint>(S_sfx.Num());

		//	Free all channels for use.
		snd_Channels = MAX_VOICES;
		memset(Channel, 0, sizeof(Channel));

		snd_MaxVolume = -1;
	}
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::Shutdown
//
//==========================================================================

void VOpenALDevice::Shutdown(void)
{
	guard(VOpenALDevice::Shutdown);
	//	Stop playback of all sounds.
	StopAllSound();
	//	Delete buffers.
	if (Buffers)
	{
		alDeleteBuffers(S_sfx.Num(), Buffers);
		Z_Free(Buffers);
		Buffers = NULL;
	}
	if (VoiceBuffer)
	{
		alDeleteBuffers(1, &VoiceBuffer);
		VoiceBuffer = 0;
	}
	//	Destroy context.
	if (Context)
	{
#ifndef __linux__
		// This causes a freeze in Linux
		alcMakeContextCurrent(NULL);
#endif
		alcDestroyContext(Context);
		Context = NULL;
	}
	//	Disconnect from a device.
	if (Device)
	{
		alcCloseDevice(Device);
		Device = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::GetChannel
//
//==========================================================================

int VOpenALDevice::GetChannel(int sound_id, int origin_id, int channel, int priority)
{
	guard(VOpenALDevice::GetChannel);
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
		for (i=0; i<snd_Channels; i++)
		{
			if (Channel[i].sound_id == sound_id)
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

	//	Mobjs can have only one sound
	if (origin_id && channel)
    {
		for (i = 0; i < snd_Channels; i++)
		{
			if (Channel[i].origin_id == origin_id &&
				Channel[i].channel == channel)
			{
				// only allow other mobjs one sound
				StopChannel(i);
				return i;
			}
		}
	}

	//	Look for a free channel
	for (i = 0; i < snd_Channels; i++)
	{
		if (!Channel[i].sound_id)
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
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::PlaySound
//
// 	This function adds a sound to the list of currently active sounds, which
// is maintained as a given number of internal channels.
//
//==========================================================================

void VOpenALDevice::PlaySound(int sound_id, const TVec &origin,
	const TVec &velocity, int origin_id, int channel, float volume)
{
	guard(VOpenALDevice::PlaySound);
	if (!snd_Channels || !sound_id || !snd_MaxVolume || !volume)
	{
		return;
	}

	// calculate the distance before other stuff so that we can throw out
	// sounds that are beyond the hearing range.
	int dist = 0;
	if (origin_id && origin_id != cl.clientnum + 1)
		dist = CalcDist(origin);
	if (dist >= MAX_SND_DIST)
	{
		return; // sound is beyond the hearing range...
	}

	int priority = CalcPriority(sound_id, dist);

	int chan = GetChannel(sound_id, origin_id, channel, priority);
	if (chan == -1)
	{
		return; //no free channels.
	}

	if (!Buffers[sound_id])
	{
		//	Check, that sound lump is loaded
		if (!S_LoadSound(sound_id))
		{
			//	Missing sound.
			return;
		}

		alGetError();	//	Clear error code.
		alGenBuffers(1, &Buffers[sound_id]);
		if (alGetError() != AL_NO_ERROR)
		{
			GCon->Log(NAME_Dev, "Failed to gen buffer");
			S_DoneWithLump(sound_id);
			return;
		}
		alBufferData(Buffers[sound_id], AL_FORMAT_MONO8,
			S_sfx[sound_id].data, S_sfx[sound_id].len, S_sfx[sound_id].freq);
		if (alGetError() != AL_NO_ERROR)
		{
			GCon->Log(NAME_Dev, "Failed to load buffer data");
			S_DoneWithLump(sound_id);
			return;
		}

		//	We don't need to keep lump static
		S_DoneWithLump(sound_id);
	}

	ALuint src;
	alGetError();	//	Clear error code.
	alGenSources(1, &src);
	if (alGetError() != AL_NO_ERROR)
	{
		GCon->Log(NAME_Dev, "Failed to gen source");
		return;
	}

	alSourcei(src, AL_BUFFER, Buffers[sound_id]);

    alSourcef(src, AL_GAIN, volume);
	alSourcef(src, AL_ROLLOFF_FACTOR, rolloff_factor);
	if (!origin_id)
	{
		alSourcei(src, AL_SOURCE_RELATIVE, AL_TRUE);
	}
	else if (origin_id == cl.clientnum + 1)
	{
		alSourcei(src, AL_SOURCE_RELATIVE, AL_TRUE);
		alSource3f(src, AL_POSITION, 0.0, 0.0, -16.0);
		alSourcef(src, AL_REFERENCE_DISTANCE, reference_distance);
		alSourcef(src, AL_MAX_DISTANCE, max_distance);
	}
	else
	{
		alSource3f(src, AL_POSITION,
			origin.x, origin.y, origin.z);
		alSource3f(src, AL_VELOCITY,
			velocity.x, velocity.y, velocity.z);
		alSourcef(src, AL_REFERENCE_DISTANCE, reference_distance);
		alSourcef(src, AL_MAX_DISTANCE, max_distance);
	} 
	if (S_sfx[sound_id].changePitch)
	{
		alSourcef(src, AL_PITCH, 1.0 + (Random() - Random()) / 16.0);
	}
	alSourcePlay(src);

	Channel[chan].origin_id = origin_id;
	Channel[chan].channel = channel;
	Channel[chan].origin = origin;
	Channel[chan].velocity = velocity;
	Channel[chan].sound_id = sound_id;
	Channel[chan].priority = priority;
	Channel[chan].volume = volume;
	Channel[chan].source = src;
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::PlayVoice
//
//==========================================================================

void VOpenALDevice::PlayVoice(const char *Name)
{
	guard(VOpenALDevice::PlayVoice);
	if (!snd_Channels || !*Name || !snd_MaxVolume)
	{
		return;
	}

	int priority = 255 * PRIORITY_MAX_ADJUST;

	int chan = GetChannel(VOICE_SOUND_ID, 0, 1, priority);
	if (chan == -1)
	{
		return; //no free channels.
	}

	//	Check, that sound lump is loaded
	if (!S_LoadSound(VOICE_SOUND_ID, Name))
	{
		//	Missing sound.
		return;
	}

	alGetError();	//	Clear error code.
	if (!VoiceBuffer)
	{
		alGenBuffers(1, &VoiceBuffer);
		if (alGetError() != AL_NO_ERROR)
		{
			GCon->Log(NAME_Dev, "Failed to gen buffer");
			S_DoneWithLump(VOICE_SOUND_ID);
			return;
		}
	}
	alBufferData(VoiceBuffer, AL_FORMAT_MONO8,
		S_VoiceInfo.data, S_VoiceInfo.len, S_VoiceInfo.freq);
	if (alGetError() != AL_NO_ERROR)
	{
		GCon->Log(NAME_Dev, "Failed to load buffer data");
		S_DoneWithLump(VOICE_SOUND_ID);
		return;
	}

	//	We don't need to keep lump static
	S_DoneWithLump(VOICE_SOUND_ID);

	ALuint src;
	alGenSources(1, &src);
	if (alGetError() != AL_NO_ERROR)
	{
		GCon->Log(NAME_Dev, "Failed to gen source");
		return;
	}

	alSourcei(src, AL_BUFFER, VoiceBuffer);
	alSourcei(src, AL_SOURCE_RELATIVE, AL_TRUE);
	alSourcePlay(src);

	Channel[chan].origin_id = 0;
	Channel[chan].channel = 1;
	Channel[chan].origin = TVec(0, 0, 0);
	Channel[chan].velocity = TVec(0, 0, 0);
	Channel[chan].sound_id = VOICE_SOUND_ID;
	Channel[chan].priority = priority;
	Channel[chan].volume = 1.0;
	Channel[chan].source = src;
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::PlaySoundTillDone
//
//==========================================================================

void VOpenALDevice::PlaySoundTillDone(const char *sound)
{
	guard(VOpenALDevice::PlaySoundTillDone);
	//	Get sound ID
	int sound_id = S_GetSoundID(sound);

	//	Maybe don't play it?
	if (!snd_Channels || !sound_id || !snd_MaxVolume)
	{
		return;
	}

	//	Silence please
	S_StopAllSound();

	//	Create buffer
	if (!Buffers[sound_id])
	{
		//	Check, that sound lump is loaded
		if (!S_LoadSound(sound_id))
		{
			//	Missing sound.
			return;
		}

		alGenBuffers(1, &Buffers[sound_id]);
		alBufferData(Buffers[sound_id], AL_FORMAT_MONO8,
			S_sfx[sound_id].data, S_sfx[sound_id].len, S_sfx[sound_id].freq);

		//	We don't need to keep lump static
		S_DoneWithLump(sound_id);
	}

	//	Create source.
	ALuint src;
	alGetError();	//	Clear error code.
	alGenSources(1, &src);
	if (alGetError() != AL_NO_ERROR)
	{
		GCon->Log(NAME_Dev, "Failed to gen source");
		return;
	}

	alSourcei(src, AL_BUFFER, Buffers[sound_id]);
	alSourcei(src, AL_SOURCE_RELATIVE, AL_TRUE);

	//	Play it
    alSourcePlay(src);

	//	Start wait
	double start = Sys_Time();
	while (1)
    {
	    ALint Status;

		alGetSourcei(src, AL_SOURCE_STATE, &Status);
		if (Status == AL_STOPPED)
		{
			//	Playback done
			break;
		}

		if (Sys_Time() - start > 10.0)
		{
			//	Time out
			break;
		}
    }

	//	Stop and release buffer
	alSourceStop(src);
	alDeleteSources(1, &src);
	unguard;
}

//==========================================================================
//
//  VOpenALDevice::Tick
//
// 	Update the sound parameters. Used to control volume and pan
// changes such as when a player turns.
//
//==========================================================================

void VOpenALDevice::Tick(float DeltaTime)
{
	guard(VOpenALDevice::Tick);
	int 		i;

	if (!snd_Channels)
	{
		return;
	}

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
		alListenerf(AL_GAIN, snd_MaxVolume);
		if (!snd_MaxVolume)
		{
			StopAllSound();
		}
    }

	if (!snd_MaxVolume)
	{
		return;
	}

	AngleVectors(cl.viewangles, listener_forward, listener_right, listener_up);

	for (i = 0; i < snd_Channels; i++)
	{
		if (!Channel[i].sound_id)
		{
			//	Nothing on this channel
			continue;
		}
		ALint State;
	    alGetSourcei(Channel[i].source, AL_SOURCE_STATE, &State);
		if (State == AL_STOPPED)
		{
			//	Playback done
        	StopChannel(i);
			continue;
		}

		if (!Channel[i].origin_id)
		{
			//	Full volume sound
			continue;
		}

		if (Channel[i].origin_id == cl.clientnum + 1)
		{
			//	Client sound
			continue;
		}

		//	Move sound
		Channel[i].origin += Channel[i].velocity * DeltaTime;

		int dist = CalcDist(Channel[i].origin);
		if (dist >= MAX_SND_DIST)
		{
			//	Too far away
			StopChannel(i);
			continue;
		}

		Channel[i].priority = CalcPriority(Channel[i].sound_id, dist);
	}

	alListener3f(AL_POSITION, cl.vieworg.x, cl.vieworg.y, cl.vieworg.z);

//	alListener3f(AL_VELOCITY,
//		(float)listener->mo->momx,
//		(float)listener->mo->momy,
//		(float)listener->mo->momz);

	ALfloat orient[6] = {
		listener_forward.x,
		listener_forward.y,
		listener_forward.z,
		listener_up.x,
		listener_up.y,
		listener_up.z};
	alListenerfv(AL_ORIENTATION, orient);

	alDopplerFactor(doppler_factor);
	alDopplerVelocity(doppler_velocity);

	if (supportEAX)
	{
		int envId = eax_environment;
		if (envId < 0 || envId >= EAX_ENVIRONMENT_COUNT)
			envId = EAX_ENVIRONMENT_GENERIC;
		pEAXSet(&DSPROPSETID_EAX_ListenerProperties,
			DSPROPERTY_EAXLISTENER_ENVIRONMENT, 0, &envId, sizeof(int));

		float envSize = EAX_CalcEnvSize();
		pEAXSet(&DSPROPSETID_EAX_ListenerProperties,
			DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE, 0, &envSize, sizeof(float));
	}
	unguard;
}

//==========================================================================
//
//  VOpenALDevice::StopChannel
//
//	Stop the sound. Necessary to prevent runaway chainsaw, and to stop
// rocket launches when an explosion occurs.
//	All sounds MUST be stopped;
//
//==========================================================================

void VOpenALDevice::StopChannel(int chan_num)
{
	if (Channel[chan_num].sound_id)
    {
		//	Stop buffer
		alSourceStop(Channel[chan_num].source);
		alDeleteSources(1, &Channel[chan_num].source);

		//	Clear channel data
		Channel[chan_num].source = 0;
		Channel[chan_num].origin_id = 0;
        Channel[chan_num].sound_id = 0;
	}
}

//==========================================================================
//
//	VOpenALDevice::StopSound
//
//==========================================================================

void VOpenALDevice::StopSound(int origin_id, int channel)
{
	guard(VOpenALDevice::StopSound);
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
//	VOpenALDevice::StopAllSound
//
//==========================================================================

void VOpenALDevice::StopAllSound(void)
{
	guard(VOpenALDevice::StopAllSound);
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
//	VOpenALDevice::IsSoundPlaying
//
//==========================================================================

bool VOpenALDevice::IsSoundPlaying(int origin_id, int sound_id)
{
	guard(VOpenALDevice::IsSoundPlaying);
	int i;

	for (i = 0; i < snd_Channels; i++)
	{
		if (Channel[i].sound_id == sound_id &&
			Channel[i].origin_id == origin_id)
		{
		    ALint State;

		    alGetSourcei(Channel[i].source, AL_SOURCE_STATE, &State);

			if (State == AL_PLAYING)
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
//	Revision 1.10  2005/09/12 19:45:16  dj_jl
//	Created midi device class.
//
//	Revision 1.9  2005/04/28 07:16:15  dj_jl
//	Fixed some warnings, other minor fixes.
//	
//	Revision 1.8  2004/11/30 07:17:16  dj_jl
//	Made string pointers const.
//	
//	Revision 1.7  2004/08/21 19:10:44  dj_jl
//	Changed sound driver declaration.
//	
//	Revision 1.6  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//	
//	Revision 1.5  2003/03/08 12:08:04  dj_jl
//	Beautification.
//	
//	Revision 1.4  2002/08/08 18:05:20  dj_jl
//	Release fixes.
//	
//	Revision 1.3  2002/07/27 18:10:11  dj_jl
//	Implementing Strife conversations.
//	
//	Revision 1.2  2002/07/23 13:12:00  dj_jl
//	Some compatibility fixes, beautification.
//	
//	Revision 1.1  2002/07/20 14:50:47  dj_jl
//	Added OpenAL driver.
//	
//**************************************************************************
