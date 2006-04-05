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

class VOpenALDevice : public VSoundDevice
{
private:
	enum { MAX_VOICES = 256 };

	enum { NUM_STRM_BUFFERS = 8 };
	enum { STRM_BUFFER_SIZE = 1024 };

	ALCdevice*	Device;
	ALCcontext*	Context;
	ALuint*		Buffers;

	bool		supportEAX;
	EAXGet		pEAXGet;
	EAXSet		pEAXSet;

	ALuint		StrmSampleRate;
	ALuint		StrmFormat;
	ALuint		StrmBuffers[NUM_STRM_BUFFERS];
	ALuint		StrmAvailableBuffers[NUM_STRM_BUFFERS];
	int			StrmNumAvailableBuffers;
	ALuint		StrmSource;
	short		StrmDataBuffer[STRM_BUFFER_SIZE * 2];

	static VCvarF		doppler_factor;
	static VCvarF		doppler_velocity;
	static VCvarF		rolloff_factor;
	static VCvarF		reference_distance;
	static VCvarF		max_distance;
	static VCvarI		eax_environment;

public:
	//	VSoundDevice interface.
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

	bool LoadSound(int);
};

IMPLEMENT_SOUND_DEVICE(VOpenALDevice, SNDDRV_OpenAL, "OpenAL",
	"OpenAL sound device", "-openal");

VCvarF VOpenALDevice::doppler_factor("al_doppler_factor", "1.0", CVAR_Archive);
VCvarF VOpenALDevice::doppler_velocity("al_doppler_velocity", "10000.0", CVAR_Archive);
VCvarF VOpenALDevice::rolloff_factor("al_rolloff_factor", "1.0", CVAR_Archive);
VCvarF VOpenALDevice::reference_distance("al_reference_distance", "64.0", CVAR_Archive);
VCvarF VOpenALDevice::max_distance("al_max_distance", "2024.0", CVAR_Archive);
VCvarI VOpenALDevice::eax_environment("al_eax_environment", "0");

//==========================================================================
//
//  VOpenALDevice::Init
//
// 	Inits sound
//
//==========================================================================

bool VOpenALDevice::Init()
{
	guard(VOpenALDevice::Init);
	ALenum E;

	//	Connect to a device.
	Device = alcOpenDevice(NULL);
	if (!Device)
	{
		GCon->Log(NAME_Init, "Couldn't open OpenAL device");
		return false;
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
	Sound3D = true;
	return true;
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::SetChannels
//
//==========================================================================

int VOpenALDevice::SetChannels(int InNumChannels)
{
	guard(VOpenALDevice::SetChannels);
	int NumChannels = MAX_VOICES;
	if (NumChannels > InNumChannels)
		NumChannels = InNumChannels;
	return NumChannels;
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::Shutdown
//
//==========================================================================

void VOpenALDevice::Shutdown()
{
	guard(VOpenALDevice::Shutdown);
	//	Delete buffers.
	if (Buffers)
	{
		alDeleteBuffers(S_sfx.Num(), Buffers);
		Z_Free(Buffers);
		Buffers = NULL;
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
//  VOpenALDevice::Tick
//
//==========================================================================

void VOpenALDevice::Tick(float)
{
}

//==========================================================================
//
//	VOpenALDevice::LoadSound
//
//==========================================================================

bool VOpenALDevice::LoadSound(int sound_id)
{
	guard(VOpenALDevice::LoadSound);
	if (Buffers[sound_id])
	{
		return true;
	}

	//	Check, that sound lump is loaded
	if (!S_LoadSound(sound_id))
	{
		//	Missing sound.
		return false;
	}

	//	Clear error code.
	alGetError();

	//	Create buffer.
	alGenBuffers(1, &Buffers[sound_id]);
	if (alGetError() != AL_NO_ERROR)
	{
		GCon->Log(NAME_Dev, "Failed to gen buffer");
		S_DoneWithLump(sound_id);
		return false;
	}

	//	Load buffer data.
	alBufferData(Buffers[sound_id], S_sfx[sound_id].SampleBits == 8 ?
		AL_FORMAT_MONO8 : AL_FORMAT_MONO16, S_sfx[sound_id].Data,
		S_sfx[sound_id].DataSize, S_sfx[sound_id].SampleRate);
	if (alGetError() != AL_NO_ERROR)
	{
		GCon->Log(NAME_Dev, "Failed to load buffer data");
		S_DoneWithLump(sound_id);
		return false;
	}

	//	We don't need to keep lump static
	S_DoneWithLump(sound_id);
	return true;
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

int VOpenALDevice::PlaySound(int sound_id, float volume, float, float pitch,
	bool Loop)
{
	guard(VOpenALDevice::PlaySound);
	if (!LoadSound(sound_id))
	{
		return -1;
	}

	ALuint src;
	alGetError();	//	Clear error code.
	alGenSources(1, &src);
	if (alGetError() != AL_NO_ERROR)
	{
		GCon->Log(NAME_Dev, "Failed to gen source");
		return -1;
	}

	alSourcei(src, AL_BUFFER, Buffers[sound_id]);

    alSourcef(src, AL_GAIN, volume);
	alSourcef(src, AL_ROLLOFF_FACTOR, rolloff_factor);
	alSourcei(src, AL_SOURCE_RELATIVE, AL_TRUE);
	alSource3f(src, AL_POSITION, 0.0, 0.0, -16.0);
	alSourcef(src, AL_REFERENCE_DISTANCE, reference_distance);
	alSourcef(src, AL_MAX_DISTANCE, max_distance);
	alSourcef(src, AL_PITCH, pitch);
	if (Loop)
		alSourcei(src, AL_LOOPING, AL_TRUE);
	alSourcePlay(src);
	return src;
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::PlaySound3D
//
//==========================================================================

int VOpenALDevice::PlaySound3D(int sound_id, const TVec &origin,
	const TVec &velocity, float volume, float pitch, bool Loop)
{
	guard(VOpenALDevice::PlaySound3D);
	if (!LoadSound(sound_id))
	{
		return -1;
	}

	ALuint src;
	alGetError();	//	Clear error code.
	alGenSources(1, &src);
	if (alGetError() != AL_NO_ERROR)
	{
		GCon->Log(NAME_Dev, "Failed to gen source");
		return -1;
	}

	alSourcei(src, AL_BUFFER, Buffers[sound_id]);

    alSourcef(src, AL_GAIN, volume);
	alSourcef(src, AL_ROLLOFF_FACTOR, rolloff_factor);
	alSource3f(src, AL_POSITION, origin.x, origin.y, origin.z);
	alSource3f(src, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
	alSourcef(src, AL_REFERENCE_DISTANCE, reference_distance);
	alSourcef(src, AL_MAX_DISTANCE, max_distance);
	alSourcef(src, AL_PITCH, pitch);
	if (Loop)
		alSourcei(src, AL_LOOPING, AL_TRUE);
	alSourcePlay(src);
	return src;
	unguard;
}

//==========================================================================
//
//  VOpenALDevice::UpdateChannel
//
//==========================================================================

void VOpenALDevice::UpdateChannel(int, float, float)
{
}

//==========================================================================
//
//	VOpenALDevice::UpdateChannel3D
//
//==========================================================================

void VOpenALDevice::UpdateChannel3D(int Handle, const TVec& Org,
	const TVec& Vel)
{
	guard(VOpenALDevice::UpdateChannel3D);
	if (Handle == -1)
	{
		return;
	}
	alSource3f(Handle, AL_POSITION, Org.x, Org.y, Org.z);
	alSource3f(Handle, AL_VELOCITY, Vel.x, Vel.y, Vel.z);
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::IsChannelPlaying
//
//==========================================================================

bool VOpenALDevice::IsChannelPlaying(int Handle)
{
	guard(VOpenALDevice::IsChannelPlaying);
	if (Handle == -1)
	{
		return false;
	}
	ALint State;
	alGetSourcei(Handle, AL_SOURCE_STATE, &State);
	return State == AL_PLAYING;
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

void VOpenALDevice::StopChannel(int Handle)
{
	guard(VOpenALDevice::StopChannel);
	if (Handle == -1)
	{
		return;
	}
	//	Stop buffer
	alSourceStop(Handle);
	alDeleteSources(1, (ALuint*)&Handle);
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::UpdateListener
//
//==========================================================================

void VOpenALDevice::UpdateListener(const TVec& org, const TVec& vel,
	const TVec& fwd, const TVec&, const TVec& up)
{
	guard(VOpenALDevice::UpdateListener);
	alListener3f(AL_POSITION, org.x, org.y, org.z);
	alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);

	ALfloat orient[6] = { fwd.x, fwd.y, fwd.z, up.x, up.y, up.z};
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
//	VOpenALDevice::OpenStream
//
//==========================================================================

bool VOpenALDevice::OpenStream(int Rate, int Bits, int Channels)
{
	guard(VOpenALDevice::OpenStream);
	StrmSampleRate = Rate;
	StrmFormat = Channels == 2 ?
		Bits == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16 :
		Bits == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;

	alGetError();	//	Clear error code.
	alGenSources(1, &StrmSource);
	if (alGetError() != AL_NO_ERROR)
	{
		GCon->Log(NAME_Dev, "Failed to gen source");
		return false;
	}
	alSourcei(StrmSource, AL_SOURCE_RELATIVE, AL_TRUE);
	alGenBuffers(NUM_STRM_BUFFERS, StrmBuffers);
	alSourceQueueBuffers(StrmSource, NUM_STRM_BUFFERS, StrmBuffers);
	alSourcePlay(StrmSource);
	StrmNumAvailableBuffers = 0;
	return true;
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::CloseStream
//
//==========================================================================

void VOpenALDevice::CloseStream()
{
	guard(VOpenALDevice::CloseStream);
	if (StrmSource)
	{
		alDeleteBuffers(NUM_STRM_BUFFERS, StrmBuffers);
		alDeleteSources(1, &StrmSource);
		StrmSource = 0;
	}
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::GetStreamAvailable
//
//==========================================================================

int VOpenALDevice::GetStreamAvailable()
{
	guard(VOpenALDevice::GetStreamAvailable);
	if (!StrmSource)
		return 0;

	ALint NumProc;
	alGetSourcei(StrmSource, AL_BUFFERS_PROCESSED, &NumProc);
	if (NumProc > 0)
	{
		alSourceUnqueueBuffers(StrmSource, NumProc,
			StrmAvailableBuffers + StrmNumAvailableBuffers);
		StrmNumAvailableBuffers += NumProc;
	}
	return StrmNumAvailableBuffers > 0 ? STRM_BUFFER_SIZE : 0;
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::SetStreamData
//
//==========================================================================

short* VOpenALDevice::GetStreamBuffer()
{
	guard(VOpenALDevice::GetStreamBuffer);
	return StrmDataBuffer;
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::SetStreamData
//
//==========================================================================

void VOpenALDevice::SetStreamData(short* Data, int Len)
{
	guard(VOpenALDevice::SetStreamData);
	ALuint Buf;
	ALint State;

	Buf = StrmAvailableBuffers[StrmNumAvailableBuffers - 1];
	StrmNumAvailableBuffers--;
	alBufferData(Buf, StrmFormat, Data, Len * 4, StrmSampleRate);
	alSourceQueueBuffers(StrmSource, 1, &Buf);
	alGetSourcei(StrmSource, AL_SOURCE_STATE, &State);
	if (State != AL_PLAYING)
		alSourcePlay(StrmSource);
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::SetStreamVolume
//
//==========================================================================

void VOpenALDevice::SetStreamVolume(float Vol)
{
	guard(VOpenALDevice::SetStreamVolume);
	if (StrmSource)
	{
	    alSourcef(StrmSource, AL_GAIN, Vol);
	}
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::PauseStream
//
//==========================================================================

void VOpenALDevice::PauseStream()
{
	guard(VOpenALDevice::PauseStream);
	if (StrmSource)
	{
		alSourcePause(StrmSource);
	}
	unguard;
}

//==========================================================================
//
//	VOpenALDevice::ResumeStream
//
//==========================================================================

void VOpenALDevice::ResumeStream()
{
	guard(VOpenALDevice::ResumeStream);
	if (StrmSource)
	{
		alSourcePlay(StrmSource);
	}
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.17  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
//	Revision 1.16  2005/11/13 14:36:22  dj_jl
//	Moved common sound functions to main sound module.
//	
//	Revision 1.15  2005/11/06 15:27:09  dj_jl
//	Added support for 16 bit sounds.
//	
//	Revision 1.14  2005/11/05 15:50:07  dj_jl
//	Voices played as normal sounds.
//	
//	Revision 1.13  2005/11/03 22:46:35  dj_jl
//	Support for any bitrate streams.
//	
//	Revision 1.12  2005/10/18 20:53:04  dj_jl
//	Implemented basic support for streamed music.
//	
//	Revision 1.11  2005/09/19 23:00:19  dj_jl
//	Streaming support.
//	
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
