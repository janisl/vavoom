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

#include "winlocal.h"
#include <dsound.h>

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

#define	MAX_VOICES				256

#define MAX_SND_DIST			2025
#define PRIORITY_MAX_ADJUST		10
#define DIST_ADJUST 			(MAX_SND_DIST/PRIORITY_MAX_ADJUST)

// TYPES -------------------------------------------------------------------

struct channel_t
{
	int			origin_id;
	int			channel;
	TVec		origin;
	TVec		velocity;
	int			sound_id;
	int			priority;
	int			volume;

	LPDIRECTSOUNDBUFFER		buf;
};

struct free_buf_t
{
	LPDIRECTSOUNDBUFFER		buf;
	int						sound_id;
	double					free_time;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void S_InitDirectMusic(LPDIRECTSOUND DSound);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static char* DS_Error(HRESULT result);

static void StopChannel(int chan_num);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int			snd_MaxVolume = -1;      // maximum volume for sound

static channel_t	Channel[MAX_VOICES];
static int			snd_Channels = 0;   // number of channels available
static free_buf_t	free_buffers[MAX_VOICES];

static byte*		SoundCurve;

static int 			sndcount = 0;
static boolean		sound3D = false;

static LPDIRECTSOUND			DSound = NULL;
static LPDIRECTSOUNDBUFFER		PrimarySoundBuffer = NULL;
static LPDIRECTSOUND3DLISTENER	Listener;

static TVec			listener_forward;
static TVec			listener_right;
static TVec			listener_up;

static TCvarF		s3d_distance_unit("s3d_distance_unit", "72.0", CVAR_ARCHIVE);
static TCvarF		s3d_doppler_factor("s3d_doppler_factor", "2.0", CVAR_ARCHIVE);
static TCvarF		s3d_rolloff_factor("s3d_rolloff_factor", "1.0", CVAR_ARCHIVE);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  S_InitSfx
//
// 	Inits sound
//
//==========================================================================

void S_InitSfx(void)
{
	guard(S_InitSfx);
	HRESULT			result;
	DSBUFFERDESC	dsbdesc;
	WAVEFORMATEX	wfx;
	DSCAPS			caps;

	if (M_CheckParm("-nosound") ||
		(M_CheckParm("-nosfx") && M_CheckParm("-nomusic")))
	{
		return;
	}

	//	Setup sound
	if (!M_CheckParm("-nosfx"))
	{
	    // Create DirectSound object
		result = CoCreateInstance(CLSID_DirectSound, NULL,
			CLSCTX_INPROC_SERVER, IID_IDirectSound, (void**)&DSound);
		if (result != DS_OK)
			Sys_Error("I_InitSound: Failed to create DirectSound object");

		result = DSound->Initialize(NULL);
		if (result == DSERR_NODRIVER)
		{
			//	User don't have a sound card
			DSound->Release();
			DSound = NULL;
			cond << "I_StartupSound: Sound driver not found\n";
			return;
		}
		if (result != DS_OK)
			Sys_Error("I_InitSound: Failed to initialize DirectSound object\n%s", DS_Error(result));

		// Set the cooperative level
		result = DSound->SetCooperativeLevel(hwnd, DSSCL_EXCLUSIVE);
		if (result != DS_OK)
			Sys_Error("Failed to set sound cooperative level\n%s", DS_Error(result));

		//	Check for 3D sound hardware
		memset(&caps, 0, sizeof(caps));
		caps.dwSize = sizeof(caps);
		DSound->GetCaps(&caps);
		if (caps.dwFreeHw3DStaticBuffers && caps.dwFreeHwMixingStaticBuffers && 
			!M_CheckParm("-no3dsound"))
		{
			sound3D = true;
			con << "3D sound on\n";
		}

		//	Create primary buffer
		memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));  
		dsbdesc.dwSize        = sizeof(DSBUFFERDESC);
		dsbdesc.dwFlags       = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
		dsbdesc.dwBufferBytes = 0;
		dsbdesc.lpwfxFormat   = NULL;
		if (sound3D)
		{
			dsbdesc.dwFlags |= DSBCAPS_CTRL3D;
		}

		result = DSound->CreateSoundBuffer(&dsbdesc, &PrimarySoundBuffer, NULL);
		if (result != DS_OK)
			Sys_Error("I_InitSound: Failed to create primary sound buffer\n%s", DS_Error(result));

		// Set up wave format
		memset(&wfx, 0, sizeof(WAVEFORMATEX));
		wfx.wFormatTag		= WAVE_FORMAT_PCM;
		wfx.wBitsPerSample	= WORD(caps.dwFlags & DSCAPS_PRIMARY16BIT ? 16 : 8);
		wfx.nChannels		= caps.dwFlags & DSCAPS_PRIMARYSTEREO ? 2 : 1;
//		wfx.nSamplesPerSec	= 11025;
		wfx.nSamplesPerSec	= 44100;
		wfx.nBlockAlign		= WORD(wfx.wBitsPerSample / 8 * wfx.nChannels);
		wfx.nAvgBytesPerSec	= wfx.nSamplesPerSec * wfx.nBlockAlign;
		wfx.cbSize			= 0;

		result = PrimarySoundBuffer->SetFormat(&wfx);
		if (result != DS_OK)
			Sys_Error("I_InitSound: Failed to set wave format of primary buffer\n%s", DS_Error(result));

		// Get listener interface
		if (sound3D)
		{
			result = PrimarySoundBuffer->QueryInterface(IID_IDirectSound3DListener, (LPVOID *)&Listener);
			if FAILED(result)
			{
				Sys_Error("Failed to get Listener");
			}

			Listener->SetDistanceFactor(1.0 / s3d_distance_unit, DS3D_IMMEDIATE);
			Listener->SetDopplerFactor(s3d_doppler_factor, DS3D_IMMEDIATE);
			Listener->SetRolloffFactor(s3d_rolloff_factor, DS3D_IMMEDIATE);
		}
	}

	//	Init music
	S_InitDirectMusic(DSound);

	if (DSound)
	{
		//	Get amout of free buffers after initializing music.
		memset(&caps, 0, sizeof(caps));
		caps.dwSize = sizeof(caps);
		DSound->GetCaps(&caps);

		if (sound3D)
			snd_Channels = caps.dwFreeHw3DStaticBuffers;
		else
			snd_Channels = caps.dwFreeHwMixingStaticBuffers;
		if (!snd_Channels)
		{
			con << "No HW channels available\n";
			snd_Channels = 8;
		}
		if (snd_Channels > MAX_VOICES)
			snd_Channels = MAX_VOICES;

		// Free all channels for use
		memset(Channel, 0, sizeof(Channel));
		memset(free_buffers, 0, sizeof(free_buffers));

		SoundCurve = (byte*)W_CacheLumpName("SNDCURVE", PU_STATIC);

		cond << "Using " << snd_Channels << " sound buffers\n";
	}
	unguard;
}

//==========================================================================
//
//	S_ShutdownSfx
//
//==========================================================================

void S_ShutdownSfx(void)
{
	guard(S_ShutdownSfx);
	//	Shutdown sound
	if (DSound)
	{
		DSound->Release();
		DSound = NULL;
	}
	unguard;
}

//==========================================================================
//
//	DS_Error
//
//==========================================================================

static char* DS_Error(HRESULT result)
{
    static char	errmsg[128];

	switch(result)
       {
        case DS_OK:
             return "The request completed successfully.";

        case DSERR_ALLOCATED:
             return "The request failed because resources, such as a priority level, were already in use by another caller.";

        case DSERR_ALREADYINITIALIZED:
             return "The object is already initialized.";

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
             return "The IDirectSound::Initialize method has not been called or has not been called successfully before other methods were called.";

        case DSERR_UNSUPPORTED:
             return "The function called is not supported at this time.";

        default:
             sprintf(errmsg,"Unknown Error Code: %04X", result);
			 return errmsg;
       }
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

	if (S_sfx[sound_id].numchannels != -1)
	{
		lp = -1; //denote the argument sound_id
		found = 0;
		prior = priority;
		for (i=0; i<snd_Channels; i++)
		{
			if (Channel[i].sound_id == sound_id && Channel[i].buf)
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

		if (found >= S_sfx[sound_id].numchannels)
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
		if (!Channel[i].buf)
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
		(PRIORITY_MAX_ADJUST - (dist / DIST_ADJUST));
}

//==========================================================================
//
//	CalcVol
//
//==========================================================================

static int CalcVol(int volume, int dist)
{
	return SoundCurve[dist] * volume / 5 - 3225;
}

//==========================================================================
//
//	CalcSep
//
//==========================================================================

static int CalcSep(const TVec &origin)
{
	int sep = (int)DotProduct(origin - cl.vieworg, listener_right);
	if (swap_stereo)
	{
		sep = -sep;
	}
	return sep;
}

//==========================================================================
//
//	CreateBuffer
//
//==========================================================================

static LPDIRECTSOUNDBUFFER CreateBuffer(int sound_id)
{
    HRESULT					result;
	LPDIRECTSOUNDBUFFER		dsbuffer;
    DSBUFFERDESC			dsbdesc;
    PCMWAVEFORMAT			pcmwf;
    void					*buffer;
	void					*buff2;
    DWORD					size1;
	DWORD					size2;
	int						i;

	for (i = 0; i < MAX_VOICES; i++)
	{
		if (free_buffers[i].sound_id == sound_id)
		{
			dsbuffer = free_buffers[i].buf;
			free_buffers[i].sound_id = 0;

			if (S_sfx[sound_id].changePitch)
			{
				int			pitch;

				pitch = S_sfx[sound_id].freq +
					S_sfx[sound_id].freq * (rand() & 7 - rand() & 7) / 128;
			    dsbuffer->SetFrequency(pitch);
			}

		    dsbuffer->SetCurrentPosition(0);

			return dsbuffer;
		}
	}

	//	Check, that sound lump is loaded
	S_LoadSound(sound_id);

    // Set up wave format structure.
	memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
    pcmwf.wf.wFormatTag         = WAVE_FORMAT_PCM;      
    pcmwf.wf.nChannels          = 1;
	pcmwf.wf.nSamplesPerSec     = S_sfx[sound_id].freq;
    pcmwf.wBitsPerSample        = WORD(8);
    pcmwf.wf.nBlockAlign        = WORD(pcmwf.wBitsPerSample / 8 * pcmwf.wf.nChannels);
    pcmwf.wf.nAvgBytesPerSec    = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;

    // Set up DSBUFFERDESC structure.
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));  // Zero it out.
    dsbdesc.dwSize              = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags             = 
		DSBCAPS_CTRLVOLUME | 
		DSBCAPS_CTRLFREQUENCY |
		DSBCAPS_STATIC;
    dsbdesc.dwBufferBytes       = S_sfx[sound_id].len;
    dsbdesc.lpwfxFormat         = (LPWAVEFORMATEX)&pcmwf;
	if (sound3D)
	{
		dsbdesc.dwFlags |= DSBCAPS_CTRL3D | DSBCAPS_LOCHARDWARE;
	}
	else
	{
	    dsbdesc.dwFlags |= DSBCAPS_CTRLPAN;
	}

	result = DSound->CreateSoundBuffer(&dsbdesc, &dsbuffer, NULL);
    if (result != DS_OK)
	{
		int		best = -1;
		double	least_time = 999999999.0;

		for (i = 0; i < MAX_VOICES; i++)
		{
			if (free_buffers[i].sound_id && 
				free_buffers[i].free_time < least_time)
			{
				best = i;
				least_time = free_buffers[i].free_time;
			}
		}
		if (best != -1)
		{
			free_buffers[best].buf->Release();
			free_buffers[best].sound_id = 0;
			result = DSound->CreateSoundBuffer(&dsbdesc, &dsbuffer, NULL);
		}
	}

    if (result != DS_OK)
	{
		cond << "Failed to create sound buffer\n" << DS_Error(result) << endl;

		//	We don't need to keep lump static
		S_DoneWithLump(sound_id);

		return NULL;
	}

    dsbuffer->Lock(0, S_sfx[sound_id].len,
		&buffer, &size1, &buff2, &size2, DSBLOCK_ENTIREBUFFER);
	memcpy(buffer, S_sfx[sound_id].data, S_sfx[sound_id].len);
	dsbuffer->Unlock(buffer, S_sfx[sound_id].len, buff2, size2);

	if (S_sfx[sound_id].changePitch)
	{
		dsbuffer->SetFrequency(S_sfx[sound_id].freq +
			S_sfx[sound_id].freq * (rand() & 7 - rand() & 7) / 128);
	}

    dsbuffer->SetCurrentPosition(0);

	//	We don't need to keep lump static
	S_DoneWithLump(sound_id);

	return dsbuffer;
}

//==========================================================================
//
//	S_StartSound
//
// 	This function adds a sound to the list of currently active sounds, which
// is maintained as a given number of internal channels.
//
//==========================================================================

void S_StartSound(int sound_id, const TVec &origin, const TVec &velocity,
	int origin_id, int channel, int volume)
{
	guard(S_StartSound);
	int 					dist;
	int 					priority;
	int						chan;
	HRESULT					result;
	LPDIRECTSOUNDBUFFER		dsbuffer;

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

	dsbuffer = CreateBuffer(sound_id);
	if (!dsbuffer)
	{
		return;
	}

	Channel[chan].origin_id = origin_id;
	Channel[chan].channel = channel;
	Channel[chan].origin = origin;
	Channel[chan].velocity = velocity;
	Channel[chan].sound_id = sound_id;
	Channel[chan].priority = priority;
	Channel[chan].volume = volume;
	Channel[chan].buf = dsbuffer;

	if (sound3D)
	{
		LPDIRECTSOUND3DBUFFER	Buf3D; 

	    Channel[chan].buf->SetVolume((Channel[chan].volume - 127) << 5);

		result = Channel[chan].buf->QueryInterface(
			IID_IDirectSound3DBuffer, (LPVOID *)&Buf3D); 
		if FAILED(result)
		{
			Sys_Error("Failed to get 3D buffer");
		}

		if (!Channel[chan].origin_id)
		{
			Buf3D->SetMode(DS3DMODE_DISABLE, DS3D_IMMEDIATE);
		}
		else if (Channel[chan].origin_id == cl.clientnum + 1)
		{
			Buf3D->SetMode(DS3DMODE_HEADRELATIVE, DS3D_IMMEDIATE);
			Buf3D->SetPosition(0.0, -16.0, 0.0, DS3D_IMMEDIATE);
		}
		else
		{
//			Buf3D->SetMinDistance(8.0, DS3D_DEFERRED);
			Buf3D->SetPosition(
				Channel[chan].origin.x,
				Channel[chan].origin.z,
				Channel[chan].origin.y,
				DS3D_IMMEDIATE);
			Buf3D->SetVelocity(
				Channel[chan].velocity.x,
				Channel[chan].velocity.z,
				Channel[chan].velocity.y,
				DS3D_IMMEDIATE);
		} 
		Buf3D->Release();
	}
	else
	{
		if (dist)
		{
			int 					vol;
			int 					sep;

			vol = CalcVol(Channel[chan].volume, dist);
			sep = CalcSep(Channel[chan].origin);

			Channel[chan].buf->SetVolume(vol);
		    Channel[chan].buf->SetPan(sep);
		}
	}

	result = dsbuffer->Play(0, 0, 0);
	if (result != DS_OK)
	{
		cond << "Failed to play channel\n" << DS_Error(result) << endl;
		StopChannel(chan);
	}
	unguard;
}

//==========================================================================
//
//	S_PlayTillDone
//
//==========================================================================

void S_PlayTillDone(char *sound)
{
	guard(S_PlayTillDone);
    int						sound_id;
	double					start;
    HRESULT					result;
	LPDIRECTSOUNDBUFFER		dsbuffer;

	//	Get sound ID
	sound_id = S_GetSoundID(sound);

	//	Maybe don't play it?
	if (!snd_Channels || !sound_id || !snd_MaxVolume)
	{
		return;
	}

	//	Silence please
	S_StopAllSound();

	//	Create buffer
	dsbuffer = CreateBuffer(sound_id);
	if (!dsbuffer)
	{
		return;
	}

	//	Set mode for 3D sound
	if (sound3D)
	{
		LPDIRECTSOUND3DBUFFER	Buf3D; 

		result = dsbuffer->QueryInterface(IID_IDirectSound3DBuffer, (LPVOID *)&Buf3D);
		if FAILED(result)
		{
			Sys_Error("Failed to get 3D buffer");
		}
		Buf3D->SetMode(DS3DMODE_DISABLE, DS3D_IMMEDIATE);
		Buf3D->Release();
	}

	//	Play it
    result = dsbuffer->Play(0, 0, 0);
    if (result != DS_OK)
		Sys_Error("Failed to play channel\n%s", DS_Error(result));

	//	Start wait
	start = Sys_Time();
	while (1)
    {
	    DWORD	Status;

	    dsbuffer->GetStatus(&Status);
		if (!(Status & DSBSTATUS_PLAYING))
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
	dsbuffer->Stop();
	dsbuffer->Release();
	unguard;
}

//==========================================================================
//
//  S_UpdateSfx
//
// 	Update the sound parameters. Used to control volume and pan
// changes such as when a player turns.
//
//==========================================================================

void S_UpdateSfx(void)
{
	guard(S_UpdateSfx);
	int 		i;
	int			dist;
    DWORD		Status;

	if (!snd_Channels)
	{
		return;
	}

	if (sfx_volume != snd_MaxVolume)
    {
	    if (sfx_volume < 0)
	    {
			sfx_volume = 0;
		}
	    if (sfx_volume > 15)
	    {
			sfx_volume = 15;
		}
	    snd_MaxVolume = sfx_volume;
		PrimarySoundBuffer->SetVolume((snd_MaxVolume - 15) * 300);
		if (!snd_MaxVolume)
		{
			S_StopAllSound();
		}
    }

	if (!snd_MaxVolume)
	{
		return;
	}

	AngleVectors(cl.viewangles, listener_forward, listener_right, listener_up);

	for (i = 0; i < snd_Channels; i++)
	{
		if (!Channel[i].buf)
		{
			//	Nothing on this channel
			continue;
		}

	    Channel[i].buf->GetStatus(&Status);
		if (!(Status & DSBSTATUS_PLAYING))
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

		dist = CalcDist(Channel[i].origin);
		if (dist >= MAX_SND_DIST)
		{
			//	Too far away
			StopChannel(i);
			continue;
		}

		//	Update params
		if (!sound3D)
		{
			int 					vol;
			int 					sep;

			vol = CalcVol(Channel[i].volume, dist);
			sep = CalcSep(Channel[i].origin);

			Channel[i].buf->SetVolume(vol);
		    Channel[i].buf->SetPan(sep);
		}

		Channel[i].priority = CalcPriority(Channel[i].sound_id, dist);
	}

	if (sound3D)
	{
		Listener->SetPosition(
			cl.vieworg.x,
			cl.vieworg.z,
			cl.vieworg.y,
			DS3D_DEFERRED);

//		Listener->SetVelocity(
//			(float)listener->mo->momx,
//			(float)listener->mo->momz,
//			(float)listener->mo->momy,
//			DS3D_DEFERRED);

		Listener->SetOrientation(
			listener_forward.x,
			listener_forward.z,
			listener_forward.y,
			listener_up.x,
			listener_up.z,
			listener_up.y,
			DS3D_DEFERRED);

		Listener->SetDistanceFactor(1.0 / s3d_distance_unit, DS3D_DEFERRED);
		Listener->SetDopplerFactor(s3d_doppler_factor, DS3D_DEFERRED);
		Listener->SetRolloffFactor(s3d_rolloff_factor, DS3D_DEFERRED);

		Listener->CommitDeferredSettings();
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
	LPDIRECTSOUNDBUFFER		dsbuffer;
	int						i;

	if (Channel[chan_num].buf)
    {
		dsbuffer = Channel[chan_num].buf;
		//	Stop buffer
		dsbuffer->Stop();

		for (i = 0; i < MAX_VOICES; i++)
		{
			if (!free_buffers[i].sound_id)
			{
				free_buffers[i].buf = dsbuffer;
				free_buffers[i].sound_id = Channel[chan_num].sound_id;
				free_buffers[i].free_time = Sys_Time();
				break;
			}
		}
		if (i == MAX_VOICES)
		{
			dsbuffer->Release();
		}

		//	Clear channel data
		Channel[chan_num].buf = NULL;
		Channel[chan_num].origin_id = 0;
        Channel[chan_num].sound_id = 0;
	}
}

//==========================================================================
//
//	S_StopSound
//
//==========================================================================

void S_StopSound(int origin_id, int channel)
{
	guard(S_StopSound);
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
// S_StopAllSound
//
//==========================================================================

void S_StopAllSound(void)
{
	guard(S_StopAllSound);
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
// S_GetSoundPlayingInfo
//
//==========================================================================

boolean S_GetSoundPlayingInfo(int origin_id, int sound_id)
{
	guard(S_GetSoundPlayingInfo);
	int i;

	for (i = 0; i < snd_Channels; i++)
	{
		if (Channel[i].buf && Channel[i].sound_id == sound_id && 
			Channel[i].origin_id == origin_id)
		{
		    DWORD	Status;

		    Channel[i].buf->GetStatus(&Status);

			if (Status & DSBSTATUS_PLAYING)
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
