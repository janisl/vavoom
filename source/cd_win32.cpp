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
#include <mmsystem.h>

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VWin32CDAudioDevice : public VCDAudioDevice, public VWinMessageHandler
{
public:
	DWORD		CDDevice;

	void Init();
	void Update();
	void Shutdown();
	void GetInfo();
	void Play(int, bool);
	void Pause();
	void Resume();
	void Stop();
	void OpenDoor();
	void CloseDoor();

	LONG OnMessage(HWND, UINT, WPARAM, LPARAM);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CD_AUDIO_DEVICE(VWin32CDAudioDevice, CDDRV_Default, "Default",
	"Windows CD audio device", NULL);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VWin32CDAudioDevice::Init
//
//==========================================================================

void VWin32CDAudioDevice::Init()
{
	guard(VWin32CDAudioDevice::Init);
	MCI_OPEN_PARMS		open;
	MCI_SET_PARMS		set;
	DWORD				result;
	int					i;

	if (GArgs.CheckParm("-nosound") || GArgs.CheckParm("-nocdaudio"))
	{
		return;
	}

	open.dwCallback       = (DWORD)hwnd;
	open.lpstrDeviceType  = "cdaudio";
	result = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_SHAREABLE, (DWORD)&open);
	if (result)
	{
		GCon->Log(NAME_Init, "CDAudio_Init: MCI_OPEN failed");
		return;
	}
	CDDevice = open.wDeviceID;

	// Set the time format to track/minute/second/frame (TMSF).
	set.dwTimeFormat = MCI_FORMAT_TMSF;
	result = mciSendCommand(CDDevice, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)&set);
	if (result)
	{
		GCon->Log(NAME_Init, "MCI_SET_TIME_FORMAT failed");
		mciSendCommand(CDDevice, MCI_CLOSE, 0, (DWORD)NULL);
		return;
	}

	GCDMsgHandler = this;
	for (i = 0; i < 100; i++)
		Remap[i] = i;
	Initialised = true;
	Enabled = true;

	GetInfo();
	if (!CDValid)
	{
		GCon->Log(NAME_Init, "CDAudio_Init: No CD in player.");
	}

	GCon->Log(NAME_Init, "CD Audio Initialised");
	unguard;
}

//==========================================================================
//
//  VWin32CDAudioDevice::OnMessage
//
//==========================================================================

LONG VWin32CDAudioDevice::OnMessage(HWND, UINT, WPARAM wParam, LPARAM lParam)
{
	guard(VWin32CDAudioDevice::OnMessage);
	if ((DWORD)lParam != CDDevice)
		return 1;

	switch (wParam)
	{
	case MCI_NOTIFY_SUCCESSFUL:
		if (Playing)
		{
			Playing = false;
			if (PlayLooping)
				Play(PlayTrack, true);
		}
		break;

	case MCI_NOTIFY_ABORTED:
	case MCI_NOTIFY_SUPERSEDED:
		break;

	case MCI_NOTIFY_FAILURE:
		GCon->Log(NAME_Dev, "MCI_NOTIFY_FAILURE");
		Stop();
		CDValid = false;
		break;

	default:
		GCon->Logf(NAME_Dev, "Unexpected MM_MCINOTIFY type (%d)", wParam);
		return 1;
	}

	return 0;
	unguard;
}

//==========================================================================
//
//	VWin32CDAudioDevice::Update
//
//==========================================================================

void VWin32CDAudioDevice::Update()
{
}

//==========================================================================
//
//	VWin32CDAudioDevice::Shutdown
//
//==========================================================================

void VWin32CDAudioDevice::Shutdown()
{
	guard(VWin32CDAudioDevice::Shutdown);
	if (!Initialised)
	{
		return;
	}

	Stop();
	if (mciSendCommand(CDDevice, MCI_CLOSE, MCI_WAIT, (DWORD)NULL))
		GCon->Log(NAME_Dev, "CD_Shutdown: MCI_CLOSE failed");
	GCDMsgHandler = NULL;
	Initialised = false;
	unguard;
}

//==========================================================================
//
//	VWin32CDAudioDevice::GetInfo
//
//==========================================================================

void VWin32CDAudioDevice::GetInfo()
{
	guard(VWin32CDAudioDevice::GetInfo);
	DWORD				result;
	MCI_STATUS_PARMS	parms;

	CDValid = false;

	parms.dwCallback = (DWORD)hwnd;
	parms.dwItem = MCI_STATUS_READY;
	result = mciSendCommand(CDDevice, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD)&parms);
	if (result)
	{
		GCon->Log(NAME_Dev, "CDAudio: drive ready test - get status failed");
		return;
	}
	if (!parms.dwReturn)
	{
		GCon->Log(NAME_Dev, "CDAudio: drive not ready");
		return;
	}

	parms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
	result = mciSendCommand(CDDevice, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD)&parms);
	if (result)
	{
		GCon->Log(NAME_Dev, "CDAudio: get tracks - status failed");
		return;
	}
	if (parms.dwReturn < 1)
	{
		GCon->Log(NAME_Dev, "CDAudio: no music tracks");
		return;
	}

	CDValid = true;
	MaxTrack = parms.dwReturn;
	unguard;
}

//==========================================================================
//
//	VWin32CDAudioDevice::Play
//
//==========================================================================

void VWin32CDAudioDevice::Play(int track, bool looping)
{
	guard(VWin32CDAudioDevice::Play);
	MCI_STATUS_PARMS	status;
	MCI_PLAY_PARMS		play;
	DWORD				result;

	if (!CDValid)
	{
		GetInfo();
		if (!CDValid)
			return;
	}

	track = Remap[track];

	if (track < 1 || track > MaxTrack)
	{
		GCon->Logf(NAME_Dev, "CDAudio: Bad track number %d.", track);
		return;
	}

	// don't try to play a non-audio track
	status.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
	status.dwTrack = track;
	result = mciSendCommand(CDDevice, MCI_STATUS, MCI_STATUS_ITEM |
		MCI_TRACK | MCI_WAIT, (DWORD)&status);
	if (result)
	{
		GCon->Log(NAME_Dev, "MCI_STATUS failed");
		return;
	}
	if (status.dwReturn != MCI_CDA_TRACK_AUDIO)
	{
		GCon->Logf("CDAudio: track %d is not audio", track);
		return;
	}

	// get the length of the track to be played
	status.dwItem = MCI_STATUS_LENGTH;
	status.dwTrack = track;
	result = mciSendCommand(CDDevice, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, (DWORD)&status);
	if (result)
	{
		GCon->Log(NAME_Dev, "MCI_STATUS failed");
		return;
	}

	if (Playing)
	{
		if (PlayTrack == track)
			return;
		Stop();
	}

	play.dwCallback = (DWORD)hwnd;
	play.dwFrom = MCI_MAKE_TMSF(track, 0, 0, 0);
	play.dwTo = (status.dwReturn << 8) | track;

	result = mciSendCommand(CDDevice, MCI_PLAY, MCI_NOTIFY | MCI_FROM | MCI_TO, (DWORD)&play);

	if (result)
	{
		GCon->Log(NAME_Dev, "MCI_PLAY failed");
		return;
	}

	PlayLooping = looping;
	PlayTrack = track;
	Playing = true;
	unguard;
}

//==========================================================================
//
//	VWin32CDAudioDevice::Pause
//
//==========================================================================

void VWin32CDAudioDevice::Pause()
{
	guard(VWin32CDAudioDevice::Pause);
	MCI_GENERIC_PARMS	parms;
	DWORD				result;

	if (!Playing)
		return;

	parms.dwCallback = (DWORD)hwnd;

	result = mciSendCommand(CDDevice, MCI_PAUSE, 0, (DWORD)&parms);

	if (result)
		GCon->Log(NAME_Dev, "MCI_PAUSE failed");

	WasPlaying = Playing;
	Playing = false;
	unguard;
}

//==========================================================================
//
//	VWin32CDAudioDevice::Resume
//
//==========================================================================

void VWin32CDAudioDevice::Resume()
{
	guard(VWin32CDAudioDevice::Resume);
	MCI_GENERIC_PARMS	parms;
	DWORD				result;

	if (!WasPlaying)
		return;

	parms.dwCallback = (DWORD)hwnd;
	result = mciSendCommand(CDDevice, MCI_RESUME, 0, (DWORD)&parms);

	if (result)
	{
		GCon->Log(NAME_Dev, "MCI_RESUME failed");
		return;
	}

	Playing = true;
	unguard;
}

//==========================================================================
//
//	VWin32CDAudioDevice::Stop
//
//==========================================================================

void VWin32CDAudioDevice::Stop()
{
	guard(VWin32CDAudioDevice::Stop);
	DWORD				result;

	if (!Playing)
		return;

	result = mciSendCommand(CDDevice, MCI_STOP, 0, (DWORD)NULL);

	if (result)
		GCon->Log(NAME_Dev, "MCI_STOP failed");

	WasPlaying = false;
	Playing = false;
	unguard;
}

//==========================================================================
//
//	VWin32CDAudioDevice::OpenDoor
//
//==========================================================================

void VWin32CDAudioDevice::OpenDoor()
{
	guard(VWin32CDAudioDevice::OpenDoor);
	MCI_SET_PARMS		parms;
	DWORD				result;

	parms.dwCallback = (DWORD)hwnd;

	result = mciSendCommand(CDDevice, MCI_SET, MCI_SET_DOOR_OPEN, (DWORD)&parms);

	if (result)
		GCon->Log(NAME_Dev, "MCI_SET_DOOR_OPEN failed");
	unguard;
}

//==========================================================================
//
//	VWin32CDAudioDevice::CloseDoor
//
//==========================================================================

void VWin32CDAudioDevice::CloseDoor()
{
	guard(VWin32CDAudioDevice::CloseDoor);
	MCI_SET_PARMS		parms;
	DWORD				result;

	parms.dwCallback = (DWORD)hwnd;

	result = mciSendCommand(CDDevice, MCI_SET, MCI_SET_DOOR_CLOSED, (DWORD)&parms);

	if (result)
		GCon->Log(NAME_Dev, "MCI_SET_DOOR_CLOSED failed");
	unguard;
}
