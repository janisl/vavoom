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

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/cdio.h>

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

static char			cd_dev[64] = "/dev/cdrom";
static boolean		cd_started = false;
static boolean		enabled = false;
static boolean		cdValid = false;
static int			cdfile = -1;
static boolean		playing = false;
static boolean		wasPlaying = false;
static int			playTrack;
static int			maxTrack;
static boolean		playLooping = false;
static int		 	remap[100];

// CODE --------------------------------------------------------------------

//**************************************************************************
//
//	Main part
//
//**************************************************************************

//==========================================================================
//
//  CD_Init
//
//==========================================================================

void CD_Init(void)
{
	guard(CD_Init);
	int i;

	if (M_CheckParm("-nosound") || M_CheckParm("-nocdaudio"))
	{
		return;
	}

	i = M_CheckParm("-cddev");
	if (i && i < myargc - 1)
	{
		strncpy(cd_dev, myargv[i + 1], sizeof(cd_dev));
		cd_dev[sizeof(cd_dev) - 1] = 0;
	}

	cdfile = open(cd_dev, O_RDONLY);
	if (cdfile == -1)
	{
		GCon->Logf(NAME_Init, "CD_Init: open of \"%s\" failed (%d)",
			cd_dev, errno);
		cdfile = -1;
		return;
	}

	for (i = 0; i < 100; i++)
		remap[i] = i;
	cd_started = true;
	enabled = true;

	CD_GetInfo();
	if (!cdValid)
	{
		GCon->Log(NAME_Init, "CD_Init: No CD in player.");
	}

	GCon->Log(NAME_Init, "CD Audio Initialized");
	unguard;
}

//==========================================================================
//
//	CD_Update
//
//==========================================================================

void CD_Update(void)
{
	guard(CD_Update);
	struct ioc_read_subchannel	subchnl;
	struct cd_sub_channel_info	data;	
	static time_t				lastchk;

	if (!cd_started)
		return;

	if (!enabled)
		return;

	if (playing && lastchk < time(NULL))
	{
		lastchk = time(NULL) + 2; //two seconds between chks
		subchnl.data = &data;
		subchnl.data_len = sizeof(data);
		subchnl.address_format = CD_MSF_FORMAT;
		subchnl.data_format = CD_CURRENT_POSITION;
		if (ioctl(cdfile, CDIOCREADSUBCHANNEL, (char*) &subchnl) == -1 )
		{
			GCon->Log(NAME_Dev, "ioctl CDIOCREADSUBCHANNEL failed");
			playing = false;
			return;
		}
		if (subchnl.data->header.audio_status != CD_AS_PLAY_IN_PROGRESS &&
			subchnl.data->header.audio_status != CD_AS_PLAY_PAUSED)
		{
			playing = false;
			if (playLooping)
				CD_Play(playTrack, true);
		}
	}
	unguard;
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
		return;

	CD_Stop();
	close(cdfile);
	cdfile = -1;
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

//**************************************************************************
//
//  Command execution
//
//**************************************************************************

//==========================================================================
//
//	CD_GetInfo
//
//==========================================================================

static void CD_GetInfo(void)
{
	struct ioc_toc_header           tochdr;

	cdValid = false;

	if (ioctl(cdfile, CDIOREADTOCHEADER, &tochdr) == -1)
	{
		GCon->Log(NAME_Dev, "ioctl CDIOREADTOCHEADER failed");
		return;
	}

	if (tochdr.starting_track < 1)
	{
		GCon->Log(NAME_Dev, "CDAudio: no music tracks");
		return;
	}

	cdValid = true;
	maxTrack = tochdr.ending_track;
}

//==========================================================================
//
//	CD_Play
//
//==========================================================================

static void CD_Play(int track, boolean looping)
{
	struct ioc_read_toc_single_entry        entry;
	struct ioc_play_track                   ti;

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

/*
	// don't try to play a non-audio track
	entry.cdte_track = track;
	entry.cdte_format = CDROM_MSF;
	if (ioctl(cdfile, CDROMREADTOCENTRY, &entry) == -1)
	{
		GCon->Log(NAME_Dev, "ioctl cdromreadtocentry failed");
		return;
	}
	if (entry.cdte_ctrl == CDROM_DATA_TRACK)
	{
		GCon->Logf("CDAudio: track %d is not audio", track);
		return;
	}
*/
	if (playing)
	{
		if (playTrack == track)
			return;
		CD_Stop();
	}

	ti.start_track = track;
	ti.end_track = track;
	ti.start_index = 1;
	ti.end_index = 99;

	if (ioctl(cdfile, CDIOCPLAYTRACKS, &ti) == -1)
	{
		GCon->Log(NAME_Dev, "ioctl CDIOCPLAYTRACKS failed");
		return;
	}

	if (ioctl(cdfile, CDIOCRESUME) == -1)
		GCon->Log(NAME_Dev, "ioctl CDIOCRESUME failed");

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
	if (!playing)
		return;

	if (ioctl(cdfile, CDIOCPAUSE) == -1)
		GCon->Log(NAME_Dev, "ioctl CDIOCPAUSE failed");

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
	if (!wasPlaying)
		return;

	if (ioctl(cdfile, CDIOCRESUME) == -1)
		GCon->Log(NAME_Dev, "ioctl CDIOCRESUME failed");

	playing = true;
}

//==========================================================================
//
//  CD_Stop
//
//==========================================================================

static void CD_Stop(void)
{
	if (!playing)
		return;

	if (ioctl(cdfile, CDIOCSTOP) == -1)
		GCon->Log(NAME_Dev, "ioctl CDIOCSTOP failed");

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
	if (ioctl(cdfile, CDIOCEJECT) == -1)
		GCon->Log(NAME_Dev, "ioctl CDIOCEJECT failed");
}

//==========================================================================
//
//  CD_CloseDoor
//
//==========================================================================

static void CD_CloseDoor(void)
{
	if (ioctl(cdfile, CDIOCCLOSE) == -1)
		GCon->Log(NAME_Dev, "ioctl CDIOCCLOSE failed");
}

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2004/12/22 07:39:08  dj_jl
//	Applied BSD patches.
//
//**************************************************************************
