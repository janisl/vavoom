//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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
/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// Quake is a trademark of Id Software, Inc., (c) 1996 Id Software, Inc. All
// rights reserved.

//**************************************************************************
//**
//** 	cd_linux.c
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
		con << "CD_Init: open of \"" << cd_dev << "\" failed (" << errno << ")\n";
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
		con << "CD_Init: No CD in player.\n";
	}

	con << "CD Audio Initialized\n";
}

//==========================================================================
//
//	CD_Update
//
//==========================================================================

void CD_Update(void)
{
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
			cond << "ioctl cdromsubchnl failed\n";
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
}

//==========================================================================
//
//  CD_Shutdown
//
//==========================================================================

void CD_Shutdown(void)
{
	if (!cd_started)
		return;

	CD_Stop();
	close(cdfile);
	cdfile = -1;
	cd_started = false;
}

//==========================================================================
//
//	CD_f
//
//==========================================================================

COMMAND(CD)
{
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
					con << n << " -> " << remap[n] << endl;
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
			con << "No CD in player.\n";
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
		con << maxTrack << " tracks\n";
		if (playing || wasPlaying)
        {
			con << (playing ? "Currently " : "Paused ")
				<< (playLooping ? "looping" : "playing")
				<< " track " << playTrack << endl;
        }
		return;
	}
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
		cond << "ioctl cdromreadtochdr failed\n";
		return;
    }

	if (tochdr.cdth_trk0 < 1)
	{
		cond << "CDAudio: no music tracks\n";
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
		cond << "CDAudio: Bad track number " << track << ".\n";
		return;
	}

	// don't try to play a non-audio track
	entry.cdte_track = track;
	entry.cdte_format = CDROM_MSF;
    if (ioctl(cdfile, CDROMREADTOCENTRY, &entry) == -1)
	{
		cond << "ioctl cdromreadtocentry failed\n";
		return;
	}
	if (entry.cdte_ctrl == CDROM_DATA_TRACK)
	{
		con << "CDAudio: track " << track << " is not audio\n";
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
		cond << "ioctl cdromplaytrkind failed\n";
		return;
    }

	if (ioctl(cdfile, CDROMRESUME) == -1)
		cond << "ioctl cdromresume failed\n";

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
		cond << "ioctl cdrompause failed\n";

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
		cond << "ioctl cdromresume failed\n";

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
		cond << "ioctl cdromstop failed\n";

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
		cond << "ioctl cdromeject failed\n";
}

//==========================================================================
//
//  CD_CloseDoor
//
//==========================================================================

static void CD_CloseDoor(void)
{
	if (ioctl(cdfile, CDROMCLOSETRAY) == -1)
		cond << "ioctl cdromclosetray failed\n";
}

