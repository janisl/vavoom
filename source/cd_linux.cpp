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

class VLinuxCDAudioDevice : public VCDAudioDevice
{
public:
	int			CDFile;

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
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

IMPLEMENT_CD_AUDIO_DEVICE(VLinuxCDAudioDevice, CDDRV_Default, "Default",
	"Linux CD audio device", NULL);

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VLinuxCDAudioDevice::Init
//
//==========================================================================

void VLinuxCDAudioDevice::Init()
{
	guard(VLinuxCDAudioDevice::Init);
	int i;

	if (M_CheckParm("-nosound") || M_CheckParm("-nocdaudio"))
	{
		return;
	}

	char cd_dev[64] = "/dev/cdrom";
	i = M_CheckParm("-cddev");
	if (i && i < myargc - 1)
	{
		strncpy(cd_dev, myargv[i + 1], sizeof(cd_dev));
		cd_dev[sizeof(cd_dev) - 1] = 0;
	}

	CDFile = open(cd_dev, O_RDONLY);
	if (CDFile == -1)
	{
		GCon->Logf(NAME_Init, "CD_Init: open of \"%s\" failed (%d)",
			cd_dev, errno);
		CDFile = -1;
		return;
	}

	for (i = 0; i < 100; i++)
		Remap[i] = i;
	Initialised = true;
	Enabled = true;

	GetInfo();
	if (!CDValid)
	{
		GCon->Log(NAME_Init, "CD_Init: No CD in player.");
	}

	GCon->Log(NAME_Init, "CD Audio Initialised");
	unguard;
}

//==========================================================================
//
//	VLinuxCDAudioDevice::Update
//
//==========================================================================

void VLinuxCDAudioDevice::Update()
{
	guard(VLinuxCDAudioDevice::Update);
	struct cdrom_subchnl	subchnl;
	static time_t			lastchk;

	if (!Initialised)
		return;

	if (!Enabled)
		return;

	if (Playing && lastchk < time(NULL))
	{
		lastchk = time(NULL) + 2; //two seconds between chks
		subchnl.cdsc_format = CDROM_MSF;
		if (ioctl(CDFile, CDROMSUBCHNL, &subchnl) == -1 )
		{
			GCon->Log(NAME_Dev, "ioctl cdromsubchnl failed");
			Playing = false;
			return;
		}
		if (subchnl.cdsc_audiostatus != CDROM_AUDIO_PLAY &&
			subchnl.cdsc_audiostatus != CDROM_AUDIO_PAUSED)
		{
			Playing = false;
			if (PlayLooping)
				Play(PlayTrack, true);
		}
	}
	unguard;
}

//==========================================================================
//
//	VLinuxCDAudioDevice::Shutdown
//
//==========================================================================

void VLinuxCDAudioDevice::Shutdown()
{
	guard(VLinuxCDAudioDevice::Shutdown);
	if (!Initialised)
		return;

	Stop();
	close(CDFile);
	CDFile = -1;
	Initialised = false;
	unguard;
}

//==========================================================================
//
//	VLinuxCDAudioDevice::GetInfo
//
//==========================================================================

void VLinuxCDAudioDevice::GetInfo()
{
	guard(VLinuxCDAudioDevice::GetInfo);
	struct cdrom_tochdr		tochdr;

	CDValid = false;

	if (ioctl(CDFile, CDROMREADTOCHDR, &tochdr) == -1)
	{
		GCon->Log(NAME_Dev, "ioctl cdromreadtochdr failed");
		return;
	}

	if (tochdr.cdth_trk0 < 1)
	{
		GCon->Log(NAME_Dev, "CDAudio: no music tracks");
		return;
	}

	CDValid = true;
	MaxTrack = tochdr.cdth_trk1;
	unguard;
}

//==========================================================================
//
//	VLinuxCDAudioDevice::Play
//
//==========================================================================

void VLinuxCDAudioDevice::Play(int track, bool looping)
{
	guard(VLinuxCDAudioDevice::Play);
	struct cdrom_tocentry	entry;
	struct cdrom_ti			ti;

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
	entry.cdte_track = track;
	entry.cdte_format = CDROM_MSF;
	if (ioctl(CDFile, CDROMREADTOCENTRY, &entry) == -1)
	{
		GCon->Log(NAME_Dev, "ioctl cdromreadtocentry failed");
		return;
	}
	if (entry.cdte_ctrl == CDROM_DATA_TRACK)
	{
		GCon->Logf("CDAudio: track %d is not audio", track);
		return;
	}

	if (Playing)
	{
		if (PlayTrack == track)
			return;
		Stop();
	}

	ti.cdti_trk0 = track;
	ti.cdti_trk1 = track;
	ti.cdti_ind0 = 1;
	ti.cdti_ind1 = 99;

	if (ioctl(CDFile, CDROMPLAYTRKIND, &ti) == -1)
	{
		GCon->Log(NAME_Dev, "ioctl cdromplaytrkind failed");
		return;
	}

	if (ioctl(CDFile, CDROMRESUME) == -1)
		GCon->Log(NAME_Dev, "ioctl cdromresume failed");

	PlayLooping = looping;
	PlayTrack = track;
	Playing = true;
	unguard;
}

//==========================================================================
//
//	VLinuxCDAudioDevice::Pause
//
//==========================================================================

void VLinuxCDAudioDevice::Pause()
{
	guard(VLinuxCDAudioDevice::Pause);
	if (!Playing)
		return;

	if (ioctl(CDFile, CDROMPAUSE) == -1)
		GCon->Log(NAME_Dev, "ioctl cdrompause failed");

	WasPlaying = Playing;
	Playing = false;
	unguard;
}

//==========================================================================
//
//	VLinuxCDAudioDevice::Resume
//
//==========================================================================

void VLinuxCDAudioDevice::Resume()
{
	guard(VLinuxCDAudioDevice::Resume);
	if (!WasPlaying)
		return;

	if (ioctl(CDFile, CDROMRESUME) == -1)
		GCon->Log(NAME_Dev, "ioctl cdromresume failed");

	Playing = true;
	unguard;
}

//==========================================================================
//
//	VLinuxCDAudioDevice::Stop
//
//==========================================================================

void VLinuxCDAudioDevice::Stop()
{
	guard(VLinuxCDAudioDevice::Stop);
	if (!Playing)
		return;

	if (ioctl(CDFile, CDROMSTOP) == -1)
		GCon->Log(NAME_Dev, "ioctl cdromstop failed");

	WasPlaying = false;
	Playing = false;
	unguard;
}

//==========================================================================
//
//	VLinuxCDAudioDevice::OpenDoor
//
//==========================================================================

void VLinuxCDAudioDevice::OpenDoor()
{
	guard(VLinuxCDAudioDevice::OpenDoor);
	if (ioctl(CDFile, CDROMEJECT) == -1)
		GCon->Log(NAME_Dev, "ioctl cdromeject failed");
	unguard;
}

//==========================================================================
//
//	VLinuxCDAudioDevice::CloseDoor
//
//==========================================================================

void VLinuxCDAudioDevice::CloseDoor()
{
	guard(VLinuxCDAudioDevice::CloseDoor);
	if (ioctl(CDFile, CDROMCLOSETRAY) == -1)
		GCon->Log(NAME_Dev, "ioctl cdromclosetray failed");
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.7  2005/09/13 17:32:45  dj_jl
//	Created CD audio device class.
//
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
