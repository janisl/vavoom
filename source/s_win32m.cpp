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

//#define TEST_MP3

#include "winlocal.h"
#include <dmusici.h>
#ifdef TEST_MP3
#include <fmod.h>
#endif

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VDirectMusicMidiDevice : public VMidiDevice
{
public:
	bool						InitCalled;

	IDirectMusicPerformance*	Performance;
	IDirectMusicLoader*			Loader;
	IDirectMusicSegment*		Segment;
	MUSIC_TIME					StartTime;
	MUSIC_TIME					TimeOffset;
	MUSIC_TIME					SegmentLength;

	void*						Mus_SndPtr;
	bool						MusicPaused;
	float						MusVolume;

#ifdef TEST_MP3
	bool						fmodAvailable;
	FSOUND_STREAM*				fmodStream;
	int							fmodChannel;
#endif

	VDirectMusicMidiDevice();
	void Init();
	void Shutdown();
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

extern LPDIRECTSOUND			DSound;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_MIDI_DEVICE(VDirectMusicMidiDevice, MIDIDRV_Default, "Default",
	"DirectMusic midi device", NULL);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VDirectMusicMidiDevice::VDirectMusicMidiDevice
//
//==========================================================================

VDirectMusicMidiDevice::VDirectMusicMidiDevice()
: InitCalled(false)
, Performance(NULL)
, Loader(NULL)
, Segment(NULL)
, StartTime(0)
, TimeOffset(0)
, SegmentLength(0)
, Mus_SndPtr(NULL)
, MusicPaused(false)
, MusVolume(-1)
#ifdef TEST_MP3
, fmodAvailable(false)
, fmodStream(NULL)
, fmodChannel(0)
#endif
{
}

//==========================================================================
//
//	VDirectMusicMidiDevice::Init
//
//==========================================================================

void VDirectMusicMidiDevice::Init()
{
	guard(VDirectMusicMidiDevice::Init);
	HRESULT			result;

	//	We are also calling Init from DirectSound driver.
	if (InitCalled)
	{
		return;
	}
	InitCalled = true;

#ifdef TEST_MP3
	if (FSOUND_GetVersion() < FMOD_VERSION)
	{
		GCon->Log(NAME_Init, "Wrong fmod.DLL version!");
	}
	else if (!FSOUND_Init(44100, 32, 0))
	{
		GCon->Log(NAME_Init, "fmod init failed");
		FSOUND_Close();
	}
	else
	{
		fmodAvailable = true;
	}
#endif

	if (M_CheckParm("-nosound") || M_CheckParm("-nomusic"))
	{
		return;
	}

	result = CoCreateInstance(CLSID_DirectMusicPerformance, NULL,
		CLSCTX_INPROC, IID_IDirectMusicPerformance2, (void**)&Performance);
	if (result != S_OK)
		Sys_Error("S_InitMusic: Failed to create performance");

	result = Performance->Init(NULL, DSound, NULL);
	if (result != S_OK)
		Sys_Error("S_InitMusic: Failed to initialise performance");

	result = Performance->AddPort(NULL);
	if (result != S_OK)
		Sys_Error("S_InitMusic: Failed to add port to Performance");

	result = CoCreateInstance(CLSID_DirectMusicLoader, NULL,
		CLSCTX_INPROC, IID_IDirectMusicLoader, (void**)&Loader);
	if (result != S_OK)
		Sys_Error("S_InitMusic: Failed to create music loader");

	Initialised = true;
	Enabled = true;
	unguard;
}

//==========================================================================
//
//	VDirectMusicMidiDevice::Shutdown
//
//==========================================================================

void VDirectMusicMidiDevice::Shutdown()
{
	guard(VDirectMusicMidiDevice::Shutdown);
	if (Initialised)
	{
		// Release segment
		if (Segment)
			Segment->Release();
		Segment = NULL;

		// Release the loader object.
		if (Loader)
			Loader->Release();
		Loader = NULL;

		// CloseDown and Release the performance object.
		if (Performance)
		{
			Performance->CloseDown();
			Performance->Release();
		}
		Performance = NULL;
		Initialised = false;
	}
#ifdef TEST_MP3
	if (fmodAvailable)
	{
		if (fmodStream)
		{
			FSOUND_Stream_Close(fmodStream);
			fmodStream = NULL;
		}
		FSOUND_Close();
		fmodAvailable = false;
	}
#endif
	unguard;
}

//==========================================================================
//
//	VDirectMusicMidiDevice::Play
//
//==========================================================================

void VDirectMusicMidiDevice::Play(void* Data, int len, const char* song, bool loop)
{
	guard(VDirectMusicMidiDevice::Play);
	Mus_SndPtr = Data;
	char RealName[MAX_OSPATH];
#ifdef TEST_MP3
	if (FL_FindFile(va("music/%s.mp3", song), RealName) ||
		FL_FindFile(va("music/%s.ogg", song), RealName))
	{
		Stop();
		fmodStream = FSOUND_Stream_OpenFile(RealName, FSOUND_NORMAL, 0);
		if (!fmodStream)
		{
			GCon->Logf("Couldn't open %s", RealName);
		}
		else
		{
			fmodChannel = FSOUND_Stream_Play(FSOUND_FREE, fmodStream);
			if (fmodChannel == -1)
			{
				GCon->Log("fmod play failed");
				Stop();
			}
			else
			{
				return;
			}
		}
	}
#endif
	DMUS_OBJECTDESC			ObjDesc; 
	HRESULT					result;

	ObjDesc.dwSize = sizeof(DMUS_OBJECTDESC);
	ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;
	ObjDesc.guidClass = CLSID_DirectMusicSegment;
	ObjDesc.pbMemData = (byte*)Mus_SndPtr;
	ObjDesc.llMemLength = len;

	result = Loader->GetObject(&ObjDesc, IID_IDirectMusicSegment2, (void**)&Segment);
	if (result != S_OK)
	{
		GCon->Log("PlaySong: Failed to get object");
		return;
	}

	result = Segment->SetParam(GUID_StandardMIDIFile, (DWORD)-1, 0, 0, (void*)Performance);
	if (result != S_OK)
		Sys_Error("PlaySong: Failed to set param");

    result = Segment->SetParam(GUID_Download, (DWORD)-1, 0, 0, (void*)Performance);
	if (result != S_OK)
		Sys_Error("PlaySong: Failed to download segment");

	if (loop)
		Segment->SetRepeats(1000000);

	if (Performance->PlaySegment(Segment, 0, 0, NULL) != S_OK)
		GCon->Log("PlaySegment failed");

	Performance->GetTime(NULL, &StartTime);
	TimeOffset = 0;
	Segment->GetLength(&SegmentLength);

	if (!MusVolume || MusicPaused)
	{
		Pause();
	}
	CurrSong = FName(song, FNAME_AddLower8);
	CurrLoop = loop;
	unguard;
}

//==========================================================================
//
//	VDirectMusicMidiDevice::Pause
//
//==========================================================================

void VDirectMusicMidiDevice::Pause()
{
	guard(VDirectMusicMidiDevice::Pause);
#ifdef TEST_MP3
	if (fmodStream)
	{
		FSOUND_SetPaused(fmodChannel, true);
		MusicPaused = true;
		return;
	}
#endif
	if (!Segment)
		return;

	Performance->Stop(Segment, NULL, 0, 0);

	MUSIC_TIME          NowTime;

	//Find out the current performance time so that we can figure out 
	//where we stopped in the segment.
	Performance->GetTime(NULL, &NowTime);

	// Caculate the offset into the segment in music time (ticks)
	// and add to previous offset in 
	// cause there has been more than one pause in this segment playback
	TimeOffset = ((NowTime - StartTime) + TimeOffset) % SegmentLength;

	// Set restart point
	Segment->SetStartPoint(TimeOffset);

	MusicPaused = true;
	unguard;
}

//==========================================================================
//
//	VDirectMusicMidiDevice::Resume
//
//==========================================================================

void VDirectMusicMidiDevice::Resume()
{
	guard(VDirectMusicMidiDevice::Resume);
#ifdef TEST_MP3
	if (fmodStream)
	{
		FSOUND_SetPaused(fmodChannel, false);
		MusicPaused = false;
		return;
	}
#endif
	if (!Segment)
		return;

	Performance->PlaySegment(Segment, 0, 0, NULL);
	Performance->GetTime(NULL, &StartTime);
	MusicPaused = false;
	unguard;
}

//==========================================================================
//
//	VDirectMusicMidiDevice::Tick
//
//==========================================================================

void VDirectMusicMidiDevice::Tick(float)
{
	guard(VDirectMusicMidiDevice::Tick);
	if (!Initialised)
		return;

	//	Update volume
	if (music_volume < 0.0)
	{
		music_volume = 0.0;
	}
	if (music_volume > 1.0)
	{
		music_volume = 1.0;
	}

	if (music_volume != MusVolume)
    {
		HRESULT		res;
		int			vol;

		if (!MusVolume && (int)music_volume && !MusicPaused)
		{
			Resume();
		}
		if (MusVolume && !(int)music_volume)
		{
			Pause();
		}
		MusVolume = music_volume;

		vol = int((MusVolume - 1) * 5000);
		res = Performance->SetGlobalParam(GUID_PerfMasterVolume, &vol, sizeof(long));
		if (res != S_OK)
		{
			if (res == E_FAIL)
				GCon->Log("Failed to set music volume");
			else if (res == E_POINTER)
				GCon->Log("Invalid pointer");
			else if (res == E_OUTOFMEMORY)
				GCon->Log("Out of memory");
			else
				GCon->Log("Invalid error");
		}
	}
	unguard;
}

//==========================================================================
//
//	VDirectMusicMidiDevice::IsPlaying
//
//	Is the song playing?
//
//==========================================================================

bool VDirectMusicMidiDevice::IsPlaying()
{
	guard(VDirectMusicMidiDevice::IsPlaying);
#ifdef TEST_MP3
	if (fmodStream)
	{
		return true;
	}
#endif
	if (!Segment)
		return false;

	return Performance->IsPlaying(Segment, NULL) == S_OK;
	unguard;
}

//==========================================================================
//
//  VDirectMusicMidiDevice::Stop
//
//==========================================================================

void VDirectMusicMidiDevice::Stop()
{
	guard(VDirectMusicMidiDevice::Stop);
#ifdef TEST_MP3
	if (fmodStream)
	{
		FSOUND_Stream_Close(fmodStream);
		fmodStream = NULL;
	}
#endif
	if (Mus_SndPtr)
	{
		if (MusicPaused)
		{
			Resume();
		}
		if (Segment)
		{
			Performance->Stop(Segment, NULL,0, 0);
			Segment->Release();
			Segment = NULL;
		}
		Z_Free(Mus_SndPtr);
		Mus_SndPtr = NULL;
		CurrSong = NAME_None;
	}
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.12  2005/09/12 19:45:16  dj_jl
//	Created midi device class.
//
//	Revision 1.11  2004/09/24 10:54:11  dj_jl
//	MinGW support.
//	
//	Revision 1.10  2002/11/16 17:14:22  dj_jl
//	Some changes for release.
//	
//	Revision 1.9  2002/08/24 14:49:20  dj_jl
//	Added MP3 test.
//	
//	Revision 1.8  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.7  2002/07/20 14:53:02  dj_jl
//	Got rid of warnings.
//	
//	Revision 1.6  2002/01/11 08:12:01  dj_jl
//	Added guard macros
//	
//	Revision 1.5  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.4  2001/10/08 17:34:57  dj_jl
//	A lots of small changes and cleanups
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
