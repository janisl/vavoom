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
#include <linux/cdrom.h>

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
	struct cdrom_subchnl	subchnl;
	static time_t			lastchk;

	if (!cd_started)
		return;

	if (!enabled)
		return;

	if (playing && lastchk < time(NULL))
	{
		lastchk = time(NULL) + 2; //two seconds between chks
		subchnl.cdsc_format = CDROM_MSF;
		if (ioctl(cdfile, CDROMSUBCHNL, &subchnl) == -1 )
		{
			GCon->Log(NAME_Dev, "ioctl cdromsubchnl failed");
			playing = false;
			return;
		}
		if (subchnl.cdsc_audiostatus != CDROM_AUDIO_PLAY &&
			subchnl.cdsc_audiostatus != CDROM_AUDIO_PAUSED)
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
	struct cdrom_tochdr		tochdr;

	cdValid = false;

	if (ioctl(cdfile, CDROMREADTOCHDR, &tochdr) == -1)
    {
		GCon->Log(NAME_Dev, "ioctl cdromreadtochdr failed");
		return;
    }

	if (tochdr.cdth_trk0 < 1)
	{
		GCon->Log(NAME_Dev, "CDAudio: no music tracks");
		return;
	}

	cdValid = true;
	maxTrack = tochdr.cdth_trk1;
}

//==========================================================================
//
//	CD_Play
//
//==========================================================================

static void CD_Play(int track, boolean looping)
{
	struct cdrom_tocentry	entry;
	struct cdrom_ti			ti;

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

	if (playing)
	{
		if (playTrack == track)
			return;
		CD_Stop();
	}

	ti.cdti_trk0 = track;
	ti.cdti_trk1 = track;
	ti.cdti_ind0 = 1;
	ti.cdti_ind1 = 99;

	if (ioctl(cdfile, CDROMPLAYTRKIND, &ti) == -1)
    {
		GCon->Log(NAME_Dev, "ioctl cdromplaytrkind failed");
		return;
    }

	if (ioctl(cdfile, CDROMRESUME) == -1)
		GCon->Log(NAME_Dev, "ioctl cdromresume failed");

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

	if (ioctl(cdfile, CDROMPAUSE) == -1)
		GCon->Log(NAME_Dev, "ioctl cdrompause failed");

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
	
	if (ioctl(cdfile, CDROMRESUME) == -1)
		GCon->Log(NAME_Dev, "ioctl cdromresume failed");

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

	if (ioctl(cdfile, CDROMSTOP) == -1)
		GCon->Log(NAME_Dev, "ioctl cdromstop failed");

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
	if (ioctl(cdfile, CDROMEJECT) == -1)
		GCon->Log(NAME_Dev, "ioctl cdromeject failed");
}

//==========================================================================
//
//  CD_CloseDoor
//
//==========================================================================

static void CD_CloseDoor(void)
{
	if (ioctl(cdfile, CDROMCLOSETRAY) == -1)
		GCon->Log(NAME_Dev, "ioctl cdromclosetray failed");
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
