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

#define DIRECTSOUND_VERSION		0x0700
#include "winlocal.h"
#include <dsound.h>
#include "eax.h"

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VDirectSoundDevice : public VSoundDevice
{
public:
	enum { STRM_LEN = 8 * 1024 };

	struct FBuffer
	{
		bool					Playing;	//	Is buffer taken.
		int						SoundID;	//	Sound loaded in buffer.
		LPDIRECTSOUNDBUFFER		Buffer;		//	DirectSound buffer.
		double					FreeTime;	//	Time buffer was stopped.
	};

	bool					SupportEAX;

	LPDIRECTSOUND			DSound;
	LPDIRECTSOUNDBUFFER		PrimarySoundBuffer;
	LPDIRECTSOUND3DLISTENER	Listener;
	IKsPropertySet*			PropertySet;

	FBuffer*				Buffers;
	int						NumBuffers;		// number of buffers available

	LPDIRECTSOUNDBUFFER		StrmBuffer;
	int						StrmNextUpdatePart;
	void*					StrmLockBuffer1;
	void*					StrmLockBuffer2;
	DWORD					StrmLockSize1;
	DWORD					StrmLockSize2;

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

	const char* DS_Error(HRESULT);

	int CreateBuffer(int);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_SOUND_DEVICE(VDirectSoundDevice, SNDDRV_Default, "Default",
	"DirectSound sound device", NULL);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VCvarF		s3d_distance_unit("s3d_distance_unit", "32.0", CVAR_Archive);
static VCvarF		s3d_doppler_factor("s3d_doppler_factor", "1.0", CVAR_Archive);
static VCvarF		s3d_rolloff_factor("s3d_rolloff_factor", "1.0", CVAR_Archive);
static VCvarF		s3d_min_distance("s3d_min_distance", "64.0", CVAR_Archive);
static VCvarF		s3d_max_distance("s3d_max_distance", "2024.0", CVAR_Archive);
static VCvarI		eax_environment("eax_environment", "0");

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  VDirectSoundDevice::Init
//
// 	Inits sound
//
//==========================================================================

bool VDirectSoundDevice::Init()
{
	guard(VDirectSoundDevice::Init);
	HRESULT			result;
	DSBUFFERDESC	dsbdesc;
	WAVEFORMATEX	wfx;
	DSCAPS			caps;

	Buffers = NULL;
	NumBuffers = 0;
	SupportEAX = false;
	DSound = NULL;
	PrimarySoundBuffer = NULL;
	Listener = NULL;
	PropertySet = NULL;
	StrmBuffer = NULL;
	StrmNextUpdatePart = 0;

	GCon->Log(NAME_Init, "======================================");
	GCon->Log(NAME_Init, "Initialising DirectSound driver.");

	// Create DirectSound object
	result = CoCreateInstance(CLSID_DirectSound, NULL,
		CLSCTX_INPROC_SERVER, IID_IDirectSound, (void**)&DSound);
	if (result != DS_OK)
		Sys_Error("Failed to create DirectSound object");

	result = DSound->Initialize(NULL);
	if (result == DSERR_NODRIVER)
	{
		//	User don't have a sound card
		DSound->Release();
		DSound = NULL;
		GCon->Log(NAME_Init, "Sound driver not found");
		return false;
	}
	if (result != DS_OK)
		Sys_Error("Failed to initialise DirectSound object\n%s", DS_Error(result));

	// Set the cooperative level
	result = DSound->SetCooperativeLevel(hwnd, DSSCL_EXCLUSIVE);
	if (result != DS_OK)
		Sys_Error("Failed to set sound cooperative level\n%s", DS_Error(result));

	//	Check for 3D sound hardware
	memset(&caps, 0, sizeof(caps));
	caps.dwSize = sizeof(caps);
	DSound->GetCaps(&caps);
	if (caps.dwFreeHw3DStaticBuffers && caps.dwFreeHwMixingStaticBuffers && 
		!GArgs.CheckParm("-no3dsound"))
	{
		Sound3D = true;
		GCon->Log(NAME_Init, "3D sound on");
	}

	//	Create primary buffer
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));  
	dsbdesc.dwSize        = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags       = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	dsbdesc.dwBufferBytes = 0;
	dsbdesc.lpwfxFormat   = NULL;
	if (Sound3D)
	{
		dsbdesc.dwFlags |= DSBCAPS_CTRL3D;
	}

	result = DSound->CreateSoundBuffer(&dsbdesc, &PrimarySoundBuffer, NULL);
	if (result != DS_OK)
		Sys_Error("Failed to create primary sound buffer\n%s", DS_Error(result));

	// Set up wave format
	memset(&wfx, 0, sizeof(WAVEFORMATEX));
	wfx.wFormatTag		= WAVE_FORMAT_PCM;
	wfx.wBitsPerSample	= WORD(caps.dwFlags & DSCAPS_PRIMARY16BIT ? 16 : 8);
	wfx.nChannels		= caps.dwFlags & DSCAPS_PRIMARYSTEREO ? 2 : 1;
	//wfx.nSamplesPerSec	= 11025;
	wfx.nSamplesPerSec	= 44100;
	wfx.nBlockAlign		= WORD(wfx.wBitsPerSample / 8 * wfx.nChannels);
	wfx.nAvgBytesPerSec	= wfx.nSamplesPerSec * wfx.nBlockAlign;
	wfx.cbSize			= 0;

	result = PrimarySoundBuffer->SetFormat(&wfx);
	if (result != DS_OK)
		Sys_Error("I_InitSound: Failed to set wave format of primary buffer\n%s", DS_Error(result));

	// Get listener interface
	if (Sound3D)
	{
		result = PrimarySoundBuffer->QueryInterface(IID_IDirectSound3DListener, (LPVOID *)&Listener);
		if (FAILED(result))
		{
			Sys_Error("Failed to get Listener");
		}

		LPDIRECTSOUNDBUFFER		tempBuffer;
		WAVEFORMATEX			pcmwf;

		// Set up wave format structure.
		memset(&pcmwf, 0, sizeof(WAVEFORMATEX));
		pcmwf.wFormatTag      = WAVE_FORMAT_PCM;      
		pcmwf.nChannels       = 1;
		pcmwf.nSamplesPerSec  = 44100;
		pcmwf.wBitsPerSample  = WORD(8);
		pcmwf.nBlockAlign     = WORD(pcmwf.wBitsPerSample / 8 * pcmwf.nChannels);
		pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;

		// Set up DSBUFFERDESC structure.
		memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));  // Zero it out.
		dsbdesc.dwSize        = sizeof(DSBUFFERDESC);
		dsbdesc.dwFlags       = DSBCAPS_CTRLVOLUME | 
			DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC | 
			DSBCAPS_CTRL3D | DSBCAPS_LOCHARDWARE;
		dsbdesc.dwBufferBytes = 44100;
		dsbdesc.lpwfxFormat   = &pcmwf;

		if (SUCCEEDED(DSound->CreateSoundBuffer(&dsbdesc, &tempBuffer, NULL)))
		{
			if (FAILED(tempBuffer->QueryInterface(IID_IKsPropertySet, 
				(void **)&PropertySet)))
			{
				GCon->Log(NAME_Init, "IKsPropertySet failed");
			}
			else
			{
				GCon->Log(NAME_Init, "IKsPropertySet acquired");

				ULONG Support;
				result = PropertySet->QuerySupport(
					DSPROPSETID_EAX_ListenerProperties, 
					DSPROPERTY_EAXLISTENER_ALLPARAMETERS, &Support);
				if (FAILED(result) || 
					(Support & (KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET)) !=
					(KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET))
				{
					GCon->Log(NAME_Init, "EAX 2.0 not supported");
					PropertySet->Release();
					PropertySet = NULL;
				}
				else
				{
					GCon->Log(NAME_Init, "EAX 2.0 supported");
					SupportEAX = true;
				}
			}
			tempBuffer->Release();
		}

		Listener->SetDistanceFactor(1.0 / s3d_distance_unit, DS3D_IMMEDIATE);
		Listener->SetDopplerFactor(s3d_doppler_factor, DS3D_IMMEDIATE);
		Listener->SetRolloffFactor(s3d_rolloff_factor, DS3D_IMMEDIATE);
	}
	return true;
	unguard;
}

//==========================================================================
//
//	VDirectSoundDevice::SetChannels
//
//==========================================================================

int VDirectSoundDevice::SetChannels(int InNumChannels)
{
	guard(VDirectSoundDevice::SetChannels);
	DSCAPS caps;
	memset(&caps, 0, sizeof(caps));
	caps.dwSize = sizeof(caps);
	DSound->GetCaps(&caps);

	if (Sound3D)
		NumBuffers = caps.dwFreeHw3DStaticBuffers;
	else
		NumBuffers = caps.dwFreeHwMixingStaticBuffers;
	if (!NumBuffers)
	{
		GCon->Log(NAME_Init, "No HW channels available");
		NumBuffers = 8;
	}
	Buffers = Z_CNew<FBuffer>(NumBuffers);
	GCon->Logf(NAME_Init, "Using %d sound buffers", NumBuffers);

	int Ret = InNumChannels;
	if (Ret > NumBuffers)
	{
		Ret = NumBuffers;
	}
	return Ret;
	unguard;
}

//==========================================================================
//
//	VDirectSoundDevice::Shutdown
//
//==========================================================================

void VDirectSoundDevice::Shutdown()
{
	guard(VDirectSoundDevice::Shutdown);
	//	Shutdown sound
	if (DSound)
	{
		DSound->Release();
		DSound = NULL;
	}
	if (Buffers)
	{
		Z_Free(Buffers);
		Buffers = NULL;
	}
	unguard;
}

//==========================================================================
//
//  VDirectSoundDevice::Tick
//
//==========================================================================

void VDirectSoundDevice::Tick(float)
{
}

//==========================================================================
//
//	VDirectSoundDevice::DS_Error
//
//==========================================================================

const char* VDirectSoundDevice::DS_Error(HRESULT result)
{
	static char	errmsg[128];

	switch(result)
	{
	case DS_OK:
		return "The request completed successfully.";

	case DSERR_ALLOCATED:
		return "The request failed because resources, such as a priority level, were already in use by another caller.";

	case DSERR_ALREADYINITIALIZED:
		return "The object is already initialised.";

	case DSERR_BADFORMAT:
		return "The specified wave format is not supported.";

	case DSERR_BUFFERLOST:
		return "The buffer memory has been lost and must be restored.";

	case DSERR_CONTROLUNAVAIL:
		return "The control (volume, pan, and so forth) requested by the caller is not available.";

	case DSERR_GENERIC:
		return "An undetermined error occurred inside the DirectSound subsystem.";

	case DSERR_INVALIDCALL:
		return "This function is not valid for the current state of this object.";

	case DSERR_INVALIDPARAM:
		return "An invalid parameter was passed to the returning function.";

	case DSERR_NOAGGREGATION:
		return "The object does not support aggregation.";

	case DSERR_NODRIVER:
		return "No sound driver is available for use.";

	case DSERR_OTHERAPPHASPRIO:
		return "This value is obsolete and is not used.";

	case DSERR_OUTOFMEMORY:
		return "The DirectSound subsystem could not allocate sufficient memory to complete the caller's request.";

	case DSERR_PRIOLEVELNEEDED:
		return "The caller does not have the priority level required for the function to succeed.";

	case DSERR_UNINITIALIZED:
		return "The IDirectSound::Initialise method has not been called or has not been called successfully before other methods were called.";

	case DSERR_UNSUPPORTED:
		return "The function called is not supported at this time.";

	default:
		sprintf(errmsg,"Unknown Error Code: %04X", result);
		return errmsg;
	}
}

//==========================================================================
//
//	VDirectSoundDevice::CreateBuffer
//
//==========================================================================

int VDirectSoundDevice::CreateBuffer(int sound_id)
{
	HRESULT					result;
	LPDIRECTSOUNDBUFFER		dsbuffer;
	DSBUFFERDESC			dsbdesc;
	WAVEFORMATEX			pcmwf;
	void					*buffer;
	void					*buff2;
	DWORD					size1;
	DWORD					size2;
	int						i;

	for (i = 0; i < NumBuffers; i++)
	{
		if (!Buffers[i].Playing && Buffers[i].SoundID == sound_id)
		{
			return i;
		}
	}

	//	Check, that sound lump is loaded
	if (!S_LoadSound(sound_id))
	{
		//	Missing sound.
		return -1;
	}
	sfxinfo_t &sfx = S_sfx[sound_id];

	// Set up wave format structure.
	memset(&pcmwf, 0, sizeof(WAVEFORMATEX));
	pcmwf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.nChannels = 1;
	pcmwf.nSamplesPerSec = sfx.SampleRate;
	pcmwf.wBitsPerSample = WORD(sfx.SampleBits);
	pcmwf.nBlockAlign = WORD(pcmwf.wBitsPerSample / 8 * pcmwf.nChannels);
	pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;

	// Set up DSBUFFERDESC structure.
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));  // Zero it out.
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY |
		DSBCAPS_STATIC;
	dsbdesc.dwBufferBytes = sfx.DataSize;
	dsbdesc.lpwfxFormat = &pcmwf;
	if (Sound3D)
	{
		dsbdesc.dwFlags |= DSBCAPS_CTRL3D | DSBCAPS_LOCHARDWARE;
	}
	else
	{
		dsbdesc.dwFlags |= DSBCAPS_CTRLPAN;
	}

	int Handle;
	for (Handle = 0; Handle < NumBuffers; Handle++)
	{
		if (!Buffers[Handle].SoundID)
			break;
	}
	result = DSound->CreateSoundBuffer(&dsbdesc, &dsbuffer, NULL);
	if (Handle == NumBuffers || result != DS_OK)
	{
		int		best = -1;
		double	least_time = 999999999.0;

		for (i = 0; i < NumBuffers; i++)
		{
			if (Buffers[i].SoundID && !Buffers[i].Playing &&
				Buffers[i].FreeTime < least_time)
			{
				best = i;
				least_time = Buffers[i].FreeTime;
			}
		}
		if (best != -1)
		{
			Buffers[best].Buffer->Release();
			Buffers[best].SoundID = 0;
			if (Handle == NumBuffers)
				Handle = best;
			if (result != DS_OK)
				result = DSound->CreateSoundBuffer(&dsbdesc, &dsbuffer, NULL);
		}
		else
		{
			//	All channels are busy.
			return -1;
		}
	}

	if (result != DS_OK)
	{
		GCon->Log(NAME_Dev, "Failed to create sound buffer");
		GCon->Log(NAME_Dev, DS_Error(result));

		//	We don't need to keep lump static
		S_DoneWithLump(sound_id);

		return -1;
	}

	dsbuffer->Lock(0, sfx.DataSize,
		&buffer, &size1, &buff2, &size2, DSBLOCK_ENTIREBUFFER);
	memcpy(buffer, sfx.Data, sfx.DataSize);
	dsbuffer->Unlock(buffer, sfx.DataSize, buff2, size2);

	//	We don't need to keep lump static
	S_DoneWithLump(sound_id);

	Buffers[Handle].Buffer = dsbuffer;
	Buffers[Handle].SoundID = sound_id;
	return Handle;
}

//==========================================================================
//
//	VDirectSoundDevice::PlaySound
//
// 	This function adds a sound to the list of currently active sounds, which
// is maintained as a given number of internal channels.
//
//==========================================================================

int VDirectSoundDevice::PlaySound(int sound_id, float vol, float sep,
	float pitch, bool Loop)
{
	guard(VDirectSoundDevice::PlaySound);
	HRESULT					result;

	int Handle = CreateBuffer(sound_id);
	if (Handle == -1)
	{
		return -1;
	}
	Buffers[Handle].Buffer->SetFrequency((DWORD)(S_sfx[sound_id].SampleRate * pitch));
	Buffers[Handle].Buffer->SetCurrentPosition(0);

	if (Sound3D)
	{
		LPDIRECTSOUND3DBUFFER	Buf3D; 

		Buffers[Handle].Buffer->SetVolume((LONG)(4096.0 * (vol - 1.0)));

		result = Buffers[Handle].Buffer->QueryInterface(
			IID_IDirectSound3DBuffer, (LPVOID *)&Buf3D); 
		if (FAILED(result))
		{
			Sys_Error("Failed to get 3D buffer");
		}

		Buf3D->SetMode(DS3DMODE_HEADRELATIVE, DS3D_IMMEDIATE);
		Buf3D->SetPosition(0.0, -16.0, 0.0, DS3D_IMMEDIATE);
		Buf3D->SetMinDistance(s3d_min_distance, DS3D_IMMEDIATE);
		Buf3D->SetMaxDistance(s3d_max_distance, DS3D_IMMEDIATE);
		Buf3D->Release();
	}
	else
	{
		Buffers[Handle].Buffer->SetVolume((int)(vol * 3000) - 3000);
		Buffers[Handle].Buffer->SetPan((int)(sep * 2000));
	}

	result = Buffers[Handle].Buffer->Play(0, 0, Loop ? DSBPLAY_LOOPING : 0);
	if (result != DS_OK)
	{
		GCon->Log(NAME_Dev, "Failed to play buffer");
		GCon->Log(NAME_Dev, DS_Error(result));
	}
	Buffers[Handle].Playing = true;
	return Handle;
	unguard;
}

//==========================================================================
//
//	VDirectSoundDevice::PlaySound3D
//
// 	This function adds a sound to the list of currently active sounds, which
// is maintained as a given number of internal channels.
//
//==========================================================================

int VDirectSoundDevice::PlaySound3D(int sound_id, const TVec &origin,
	const TVec &velocity, float volume, float pitch, bool Loop)
{
	guard(VDirectSoundDevice::PlaySound3D);
	HRESULT					result;
	LPDIRECTSOUND3DBUFFER	Buf3D; 

	int Handle = CreateBuffer(sound_id);
	if (Handle == -1)
	{
		return -1;
	}
	Buffers[Handle].Buffer->SetFrequency((DWORD)(S_sfx[sound_id].SampleRate * pitch));
	Buffers[Handle].Buffer->SetCurrentPosition(0);

	Buffers[Handle].Buffer->SetVolume((LONG)(4096.0 * (volume - 1.0)));

	result = Buffers[Handle].Buffer->QueryInterface(
		IID_IDirectSound3DBuffer, (LPVOID *)&Buf3D); 
	if (FAILED(result))
	{
		Sys_Error("Failed to get 3D buffer");
	}

	Buf3D->SetPosition(origin.x, origin.z, origin.y, DS3D_IMMEDIATE);
	Buf3D->SetVelocity(velocity.x, velocity.z, velocity.y, DS3D_IMMEDIATE);
	Buf3D->SetMinDistance(s3d_min_distance, DS3D_IMMEDIATE);
	Buf3D->SetMaxDistance(s3d_max_distance, DS3D_IMMEDIATE);
	Buf3D->Release();

	result = Buffers[Handle].Buffer->Play(0, 0, Loop ? DSBPLAY_LOOPING : 0);
	if (result != DS_OK)
	{
		GCon->Log(NAME_Dev, "Failed to play buffer");
		GCon->Log(NAME_Dev, DS_Error(result));
	}
	Buffers[Handle].Playing = true;
	return Handle;
	unguard;
}

//==========================================================================
//
//  VDirectSoundDevice::UpdateChannel
//
//==========================================================================

void VDirectSoundDevice::UpdateChannel(int Handle, float vol, float sep)
{
	guard(VDirectSoundDevice::UpdateChannel);
	if (Handle == -1)
	{
		return;
	}
	//	Update params
	if (!Sound3D)
	{
		Buffers[Handle].Buffer->SetVolume((int)(vol * 3000) - 3000);
		Buffers[Handle].Buffer->SetPan((int)(sep * 2000));
	}
	unguard;
}

//==========================================================================
//
//	VDirectSoundDevice::UpdateChannel3D
//
//==========================================================================

void VDirectSoundDevice::UpdateChannel3D(int Handle, const TVec& Org,
	const TVec& Vel)
{
	guard(VDirectSoundDevice::PlaySound3D);
	HRESULT					result;
	LPDIRECTSOUND3DBUFFER	Buf3D; 

	if (Handle == -1)
	{
		return;
	}
	result = Buffers[Handle].Buffer->QueryInterface(
		IID_IDirectSound3DBuffer, (LPVOID *)&Buf3D);
	if (FAILED(result))
	{
		return;
	}
	Buf3D->SetPosition(Org.x, Org.z, Org.y, DS3D_DEFERRED);
	Buf3D->SetVelocity(Vel.x, Vel.z, Vel.y, DS3D_DEFERRED);
	Buf3D->Release();
	unguard;
}

//==========================================================================
//
//	VDirectSoundDevice::IsChannelPlaying
//
//==========================================================================

bool VDirectSoundDevice::IsChannelPlaying(int Handle)
{
	guard(VDirectSoundDevice::IsChannelPlaying);
	if (Handle == -1)
	{
		return false;
	}
	if (Buffers[Handle].Buffer)
	{
		DWORD	Status;

		Buffers[Handle].Buffer->GetStatus(&Status);

		if (Status & DSBSTATUS_PLAYING)
		{
			return true;
		}
	}
	return false;
	unguard;
}

//==========================================================================
//
//  VDirectSoundDevice::StopChannel
//
//	Stop the sound. Necessary to prevent runaway chainsaw, and to stop
// rocket launches when an explosion occurs.
//	All sounds MUST be stopped;
//
//==========================================================================

void VDirectSoundDevice::StopChannel(int Handle)
{
	if (Handle == -1)
	{
		return;
	}

	//	Stop buffer
	Buffers[Handle].Buffer->Stop();

	//	Mark buffer as not playing for reuse.
	Buffers[Handle].FreeTime = Sys_Time();
	Buffers[Handle].Playing = false;
}

//==========================================================================
//
//	VDirectSoundDevice::UpdateListener
//
//==========================================================================

void VDirectSoundDevice::UpdateListener(const TVec& org, const TVec& vel,
	const TVec& fwd, const TVec&, const TVec& up)
{
	guard(VDirectSoundDevice::UpdateListener);
	//	Set position, velocity and orientation.
	Listener->SetPosition(org.x, org.z, org.y, DS3D_DEFERRED);
	Listener->SetVelocity(vel.x, vel.z, vel.y, DS3D_DEFERRED);
	Listener->SetOrientation(fwd.x, fwd.z, fwd.y, up.x, up.z, up.y, DS3D_DEFERRED);

	//	Set factor values.
	Listener->SetDistanceFactor(1.0 / s3d_distance_unit, DS3D_DEFERRED);
	Listener->SetDopplerFactor(s3d_doppler_factor, DS3D_DEFERRED);
	Listener->SetRolloffFactor(s3d_rolloff_factor, DS3D_DEFERRED);

	if (SupportEAX)
	{
		//	Set environment properties.
		DWORD envId = eax_environment;
		if (envId < 0 || envId >= EAX_ENVIRONMENT_COUNT)
			envId = EAX_ENVIRONMENT_GENERIC;
		PropertySet->Set(DSPROPSETID_EAX_ListenerProperties,
			DSPROPERTY_EAXLISTENER_ENVIRONMENT |
			DSPROPERTY_EAXLISTENER_DEFERRED, NULL, 0, &envId, sizeof(DWORD));

		float envSize = EAX_CalcEnvSize();
		PropertySet->Set(DSPROPSETID_EAX_ListenerProperties,
			DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE |
			DSPROPERTY_EAXLISTENER_DEFERRED, NULL, 0, &envSize, sizeof(float));
	}

	//	Commit settings.
	Listener->CommitDeferredSettings();
	unguard;
}

//==========================================================================
//
//	VDirectSoundDevice::OpenStream
//
//==========================================================================

bool VDirectSoundDevice::OpenStream(int Rate, int Bits, int Channels)
{
	guard(VDirectSoundDevice::OpenStream);
	HRESULT					result;
	DSBUFFERDESC			dsbdesc;
	WAVEFORMATEX			pcmwf;
	int						i;

	// Set up wave format structure.
	memset(&pcmwf, 0, sizeof(WAVEFORMATEX));
	pcmwf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.nChannels = Channels;
	pcmwf.nSamplesPerSec = Rate;
	pcmwf.wBitsPerSample = WORD(Bits);
	pcmwf.nBlockAlign = WORD(pcmwf.wBitsPerSample / 8 * pcmwf.nChannels);
	pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;

	// Set up DSBUFFERDESC structure.
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));  // Zero it out.
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME |
		DSBCAPS_STATIC;
	dsbdesc.dwBufferBytes = STRM_LEN * 4;
	dsbdesc.lpwfxFormat = &pcmwf;

	result = DSound->CreateSoundBuffer(&dsbdesc, &StrmBuffer, NULL);
	if (result != DS_OK)
	{
		int		best = -1;
		double	least_time = 999999999.0;

		for (i = 0; i < NumBuffers; i++)
		{
			if (Buffers[i].SoundID && !Buffers[i].Playing &&
				Buffers[i].FreeTime < least_time)
			{
				best = i;
				least_time = Buffers[i].FreeTime;
			}
		}
		if (best != -1)
		{
			Buffers[best].Buffer->Release();
			Buffers[best].SoundID = 0;
			result = DSound->CreateSoundBuffer(&dsbdesc, &StrmBuffer, NULL);
		}
	}
	if (result != DS_OK)
	{
		GCon->Log(NAME_Dev, "Failed to create sound buffer");
		GCon->Log(NAME_Dev, DS_Error(result));
		return false;
	}
	return true;
	unguard;
}

//==========================================================================
//
//	VDirectSoundDevice::CloseStream
//
//==========================================================================

void VDirectSoundDevice::CloseStream()
{
	guard(VDirectSoundDevice::CloseStream);
	if (StrmBuffer)
	{
		StrmBuffer->Stop();
		StrmBuffer->Release();
		StrmBuffer = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VDirectSoundDevice::GetStreamAvailable
//
//==========================================================================

int VDirectSoundDevice::GetStreamAvailable()
{
	guard(VDirectSoundDevice::GetStreamAvailable);
	DWORD	Status;
	DWORD	PlayPos;
	DWORD	WritePos;

	StrmBuffer->GetStatus(&Status);
	if (!(Status & DSBSTATUS_PLAYING))
	{
		//	Not playing, lock entire buffer.
		StrmBuffer->Lock(0, STRM_LEN * 4, &StrmLockBuffer1, &StrmLockSize1,
			&StrmLockBuffer2, &StrmLockSize2, DSBLOCK_ENTIREBUFFER);
		return StrmLockSize1 / 4;
	}
	StrmBuffer->GetCurrentPosition(&PlayPos, &WritePos);
	int PlayPart = PlayPos / (STRM_LEN);
	if (PlayPart != StrmNextUpdatePart)
	{
		StrmBuffer->Lock(StrmNextUpdatePart * STRM_LEN, STRM_LEN,
			&StrmLockBuffer1, &StrmLockSize1, &StrmLockBuffer2,
			&StrmLockSize2, 0);
		return StrmLockSize1 / 4;
	}
	return 0;
	unguard;
}

//==========================================================================
//
//	VDirectSoundDevice::SetStreamData
//
//==========================================================================

short* VDirectSoundDevice::GetStreamBuffer()
{
	guard(VDirectSoundDevice::GetStreamBuffer);
	return (short*)StrmLockBuffer1;
	unguard;
}

//==========================================================================
//
//	VDirectSoundDevice::SetStreamData
//
//==========================================================================

void VDirectSoundDevice::SetStreamData(short*, int)
{
	guard(VDirectSoundDevice::SetStreamData);
	DWORD	Status;

	StrmBuffer->Unlock(StrmLockBuffer1, StrmLockSize1, StrmLockBuffer2, StrmLockSize2);
	StrmBuffer->GetStatus(&Status);
	if (!(Status & DSBSTATUS_PLAYING))
	{
		StrmBuffer->SetCurrentPosition(0);
		StrmBuffer->Play(0, 0, DSBPLAY_LOOPING);
	}
	StrmNextUpdatePart = (StrmNextUpdatePart + 1) & 3;
	unguard;
}

//==========================================================================
//
//	VDirectSoundDevice::SetStreamVolume
//
//==========================================================================

void VDirectSoundDevice::SetStreamVolume(float Volume)
{
	guard(VDirectSoundDevice::SetStreamVolume);
	StrmBuffer->SetVolume(int(4000 * (Volume - 1.0)));
	unguard;
}

//==========================================================================
//
//	VDirectSoundDevice::PauseStream
//
//==========================================================================

void VDirectSoundDevice::PauseStream()
{
	guard(VDirectSoundDevice::PauseStream);
	StrmBuffer->Stop();
	unguard;
}

//==========================================================================
//
//	VDirectSoundDevice::ResumeStream
//
//==========================================================================

void VDirectSoundDevice::ResumeStream()
{
	guard(VDirectSoundDevice::ResumeStream);
	StrmBuffer->Play(0, 0, DSBPLAY_LOOPING);
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.35  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
//	Revision 1.34  2006/02/26 14:45:57  dj_jl
//	Fix compilation with newer version DirectX headers.
//	
//	Revision 1.33  2005/11/13 14:36:22  dj_jl
//	Moved common sound functions to main sound module.
//	
//	Revision 1.32  2005/11/07 22:57:09  dj_jl
//	Some M$VC fixes.
//	
//	Revision 1.31  2005/11/06 15:27:09  dj_jl
//	Added support for 16 bit sounds.
//	
//	Revision 1.30  2005/11/05 15:50:07  dj_jl
//	Voices played as normal sounds.
//	
//	Revision 1.29  2005/11/03 22:46:35  dj_jl
//	Support for any bitrate streams.
//	
//	Revision 1.28  2005/10/18 20:53:04  dj_jl
//	Implemented basic support for streamed music.
//	
//	Revision 1.27  2005/09/19 23:00:19  dj_jl
//	Streaming support.
//	
//	Revision 1.26  2005/09/12 19:45:16  dj_jl
//	Created midi device class.
//	
//	Revision 1.25  2004/11/30 07:17:17  dj_jl
//	Made string pointers const.
//	
//	Revision 1.24  2004/08/21 19:10:44  dj_jl
//	Changed sound driver declaration.
//	
//	Revision 1.23  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//	
//	Revision 1.22  2004/08/04 05:36:47  dj_jl
//	Structure alignment fix.
//	
//	Revision 1.21  2004/04/15 07:12:58  dj_jl
//	Restoring sound volume on exit
//	
//	Revision 1.20  2004/01/09 08:17:44  dj_jl
//	Fixed repeating voices
//	
//	Revision 1.19  2003/03/08 12:08:04  dj_jl
//	Beautification.
//	
//	Revision 1.18  2002/07/27 18:10:11  dj_jl
//	Implementing Strife conversations.
//	
//	Revision 1.17  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.16  2002/07/23 13:12:00  dj_jl
//	Some compatibility fixes, beautification.
//	
//	Revision 1.15  2002/07/20 14:49:41  dj_jl
//	Implemented sound drivers.
//	
//	Revision 1.14  2002/05/18 16:57:17  dj_jl
//	Added EAX support.
//	
//	Revision 1.13  2002/02/22 18:09:52  dj_jl
//	Some improvements, beautification.
//	
//	Revision 1.12  2002/01/29 18:17:58  dj_jl
//	Fixed 3D sound.
//	
//	Revision 1.11  2002/01/28 18:43:16  dj_jl
//	Added console variables for DirectSound3D settings
//	
//	Revision 1.10  2002/01/11 08:12:01  dj_jl
//	Added guard macros
//	
//	Revision 1.9  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.8  2001/12/18 19:06:36  dj_jl
//	Made TCvar a pure C++ class
//	
//	Revision 1.7  2001/09/05 12:21:42  dj_jl
//	Release changes
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
