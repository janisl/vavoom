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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void CD_GetInfo(void);
static void CD_Play(int track, boolean looping);
static void CD_Pause(void);
static void CD_Resume(void);
static void CD_Stop(void);
static void CD_OpenDoor(void);
static void CD_CloseDoor(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static boolean		cd_started = false;
static boolean		enabled = false;
static boolean		cdValid = false;
static DWORD		CDDevice;
static boolean		playing = false;
static boolean		wasPlaying = false;
static int			playTrack;
static int			maxTrack;
static boolean		playLooping = false;
static int		 	remap[100];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  CD_Init
//
//==========================================================================

void CD_Init(void)
{
	guard(CD_Init);
    MCI_OPEN_PARMS		open;
	MCI_SET_PARMS		set;
	DWORD				result;
	int					i;

	if (M_CheckParm("-nosound") || M_CheckParm("-nocdaudio"))
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

	for (i = 0; i < 100; i++)
		remap[i] = i;
    cd_started = true;
    enabled = true;

    CD_GetInfo();
	if (!cdValid)
	{
		GCon->Log(NAME_Init, "CDAudio_Init: No CD in player.");
	}

	GCon->Log(NAME_Init, "CD Audio Initialized");
	unguard;
}

//==========================================================================
//
//  CD_MessageHandler
//
//==========================================================================

LONG CD_MessageHandler(HWND, UINT, WPARAM wParam, LPARAM lParam)
{
	guard(CD_MessageHandler);
	if (lParam != (LPARAM)CDDevice)
		return 1;

	switch (wParam)
	{
	 case MCI_NOTIFY_SUCCESSFUL:
		if (playing)
		{
			playing = false;
			if (playLooping)
				CD_Play(playTrack, true);
		}
		break;

	 case MCI_NOTIFY_ABORTED:
	 case MCI_NOTIFY_SUPERSEDED:
		break;

	 case MCI_NOTIFY_FAILURE:
		GCon->Log(NAME_Dev, "MCI_NOTIFY_FAILURE");
		CD_Stop();
		cdValid = false;
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
//	CD_Update
//
//==========================================================================

void CD_Update(void)
{
}

//==========================================================================
//
//  CD_Shutdown
//
//==========================================================================

void CD_Shutdown(void)
{
	guard(CD_Shutdown);
	if (!cd_started)
	{
    	return;
	}

	CD_Stop();
	if (mciSendCommand(CDDevice, MCI_CLOSE, MCI_WAIT, (DWORD)NULL))
		GCon->Log(NAME_Dev, "CD_Shutdown: MCI_CLOSE failed");
	cd_started = false;
	unguard;
}

//==========================================================================
//
//	CD_f
//
//==========================================================================

COMMAND(CD)
{
	guard(COMMAND CD);
	char	*command;

	if (!cd_started)
		return;

	if (Argc() < 2)
		return;

	command = Argv(1);

	if (!stricmp(command, "on"))
	{
		enabled = true;
		return;
	}

	if (!stricmp(command, "off"))
	{
		if (playing)
			CD_Stop();
		enabled = false;
		return;
	}

	if (!stricmp(command, "reset"))
	{
		int		n;

		enabled = true;
		if (playing)
			CD_Stop();
		for (n = 0; n < 100; n++)
			remap[n] = n;
		CD_GetInfo();
		return;
	}

	if (!stricmp(command, "remap"))
	{
    	int		n;
        int		ret;

		ret = Argc() - 2;
		if (ret <= 0)
		{
			for (n = 1; n < 100; n++)
				if (remap[n] != n)
					GCon->Logf("%d -> %d", n, remap[n]);
			return;
		}
		for (n = 1; n <= ret; n++)
			remap[n] = atoi(Argv(n + 1));
		return;
	}

	if (!enabled)
    {
    	return;
    }

	if (!stricmp(command, "eject"))
	{
		if (playing)
			CD_Stop();
    	CD_OpenDoor();
		cdValid = false;
		return;
	}

	if (!stricmp(command, "close"))
	{
		CD_CloseDoor();
		return;
	}

	if (!cdValid)
	{
		CD_GetInfo();
		if (!cdValid)
		{
			GCon->Log("No CD in player.");
			return;
		}
	}

	if (!stricmp(command, "play"))
	{
		CD_Play(atoi(Argv(2)), false);
		return;
	}

	if (!stricmp(command, "loop"))
	{
		CD_Play(atoi(Argv(2)), true);
		return;
	}

	if (!stricmp(command, "pause"))
	{
		CD_Pause();
		return;
	}

	if (!stricmp(command, "resume"))
	{
		CD_Resume();
		return;
	}

	if (!stricmp(command, "stop"))
	{
		CD_Stop();
		return;
	}

	if (!stricmp(command, "info"))
	{
		GCon->Logf("%d tracks", maxTrack);
		if (playing || wasPlaying)
        {
			GCon->Logf("%s %s track %d", playing ? "Currently" : "Paused",
				playLooping ? "looping" : "playing", playTrack);
        }
		return;
	}
	unguard;
}

//==========================================================================
//
//	CD_GetInfo
//
//==========================================================================

static void CD_GetInfo(void)
{
	DWORD				result;
	MCI_STATUS_PARMS	parms;

	cdValid = false;

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

	cdValid = true;
	maxTrack = parms.dwReturn;
}

//==========================================================================
//
//	CD_Play
//
//==========================================================================

static void CD_Play(int track, boolean looping)
{
	MCI_STATUS_PARMS	status;
    MCI_PLAY_PARMS		play;
	DWORD				result;

	if (!cdValid)
	{
		CD_GetInfo();
		if (!cdValid)
			return;
	}

	track = remap[track];

	if (track < 1 || track > maxTrack)
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

	if (playing)
	{
		if (playTrack == track)
			return;
		CD_Stop();
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

	playLooping = looping;
	playTrack = track;
	playing = true;
}

//==========================================================================
//
//  CD_Pause
//
//==========================================================================

static void CD_Pause(void)
{
	MCI_GENERIC_PARMS	parms;
	DWORD				result;

	if (!playing)
		return;

	parms.dwCallback = (DWORD)hwnd;

    result = mciSendCommand(CDDevice, MCI_PAUSE, 0, (DWORD)&parms);

    if (result)
		GCon->Log(NAME_Dev, "MCI_PAUSE failed");

	wasPlaying = playing;
	playing = false;
}

//==========================================================================
//
//  CD_Resume
//
//==========================================================================

static void CD_Resume(void)
{
	MCI_GENERIC_PARMS	parms;
	DWORD				result;

	if (!wasPlaying)
		return;
	
	parms.dwCallback = (DWORD)hwnd;
	result = mciSendCommand(CDDevice, MCI_RESUME, 0, (DWORD)&parms);

	if (result)
	{
		GCon->Log(NAME_Dev, "MCI_RESUME failed");
		return;
	}

	playing = true;
}

//==========================================================================
//
//  CD_Stop
//
//==========================================================================

static void CD_Stop(void)
{
	DWORD				result;

	if (!playing)
		return;

	result = mciSendCommand(CDDevice, MCI_STOP, 0, (DWORD)NULL);

    if (result)
		GCon->Log(NAME_Dev, "MCI_STOP failed");

	wasPlaying = false;
	playing = false;
}

//==========================================================================
//
//  CD_OpenDoor
//
//==========================================================================

static void CD_OpenDoor(void)
{
	MCI_SET_PARMS		parms;
	DWORD				result;

	parms.dwCallback = (DWORD)hwnd;

    result = mciSendCommand(CDDevice, MCI_SET, MCI_SET_DOOR_OPEN, (DWORD)&parms);

    if (result)
		GCon->Log(NAME_Dev, "MCI_SET_DOOR_OPEN failed");
}

//==========================================================================
//
//  CD_CloseDoor
//
//==========================================================================

static void CD_CloseDoor(void)
{
	MCI_SET_PARMS		parms;
	DWORD				result;

	parms.dwCallback = (DWORD)hwnd;

	result = mciSendCommand(CDDevice, MCI_SET, MCI_SET_DOOR_CLOSED, (DWORD)&parms);

    if (result)
		GCon->Log(NAME_Dev, "MCI_SET_DOOR_CLOSED failed");
}

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2002/07/23 16:29:55  dj_jl
//	Replaced console streams with output device class.
//
//	Revision 1.5  2002/01/11 08:12:01  dj_jl
//	Added guard macros
//	
//	Revision 1.4  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
