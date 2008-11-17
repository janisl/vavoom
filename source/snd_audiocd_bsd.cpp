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
//**	Copyright (C) 1999-2006 Jānis Legzdiņš
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

//  This is not compatible with Solaris
#ifndef __sun__

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/cdio.h>

#include "gamedefs.h"
#include "snd_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VBsdCDAudioDevice : public VCDAudioDevice
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

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CD_AUDIO_DEVICE(VBsdCDAudioDevice, CDDRV_Default, "Default",
	"BSD CD audio device", NULL);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VBsdCDAudioDevice::Init
//
//==========================================================================

void VBsdCDAudioDevice::Init()
{
	guard(VBsdCDAudioDevice::Init);
	int i;

	if (GArgs.CheckParm("-nosound") || GArgs.CheckParm("-nocdaudio"))
	{
		return;
	}

	VStr cd_dev = "/dev/cdrom";
	const char* p = GArgs.CheckValue("-cddev");
	if (p)
	{
		cd_dev = p;
	}

	CDFile = open(*cd_dev, O_RDONLY);
	if (CDFile == -1)
	{
		GCon->Logf(NAME_Init, "CD_Init: open of \"%s\" failed (%d)",
			*cd_dev, errno);
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
//	VBsdCDAudioDevice::Update
//
//==========================================================================

void VBsdCDAudioDevice::Update()
{
	guard(VBsdCDAudioDevice::Update);
	struct ioc_read_subchannel	subchnl;
	struct cd_sub_channel_info	data;	
	static time_t				lastchk;

	if (!Initialised)
		return;

	if (!Enabled)
		return;

	if (Playing && lastchk < time(NULL))
	{
		lastchk = time(NULL) + 2; //two seconds between chks
		subchnl.data = &data;
		subchnl.data_len = sizeof(data);
		subchnl.address_format = CD_MSF_FORMAT;
		subchnl.data_format = CD_CURRENT_POSITION;
		if (ioctl(CDFile, CDIOCREADSUBCHANNEL, (char*) &subchnl) == -1 )
		{
			GCon->Log(NAME_Dev, "ioctl CDIOCREADSUBCHANNEL failed");
			Playing = false;
			return;
		}
		if (subchnl.data->header.audio_status != CD_AS_PLAY_IN_PROGRESS &&
			subchnl.data->header.audio_status != CD_AS_PLAY_PAUSED)
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
//	VBsdCDAudioDevice::Shutdown
//
//==========================================================================

void VBsdCDAudioDevice::Shutdown()
{
	guard(VBsdCDAudioDevice::Shutdown);
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
//	VBsdCDAudioDevice::GetInfo
//
//==========================================================================

void VBsdCDAudioDevice::GetInfo()
{
	guard(VBsdCDAudioDevice::GetInfo);
	struct ioc_toc_header           tochdr;

	CDValid = false;

	if (ioctl(CDFile, CDIOREADTOCHEADER, &tochdr) == -1)
	{
		GCon->Log(NAME_Dev, "ioctl CDIOREADTOCHEADER failed");
		return;
	}

	if (tochdr.starting_track < 1)
	{
		GCon->Log(NAME_Dev, "CDAudio: no music tracks");
		return;
	}

	CDValid = true;
	MaxTrack = tochdr.ending_track;
	unguard;
}

//==========================================================================
//
//	VBsdCDAudioDevice::Play
//
//==========================================================================

void VBsdCDAudioDevice::Play(int track, bool looping)
{
	guard(VBsdCDAudioDevice::Play);
	struct ioc_play_track                   ti;

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

/*
	// don't try to play a non-audio track
	struct ioc_read_toc_single_entry        entry;
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
*/
	if (Playing)
	{
		if (PlayTrack == track)
			return;
		Stop();
	}

	ti.start_track = track;
	ti.end_track = track;
	ti.start_index = 1;
	ti.end_index = 99;

	if (ioctl(CDFile, CDIOCPLAYTRACKS, &ti) == -1)
	{
		GCon->Log(NAME_Dev, "ioctl CDIOCPLAYTRACKS failed");
		return;
	}

	if (ioctl(CDFile, CDIOCRESUME) == -1)
		GCon->Log(NAME_Dev, "ioctl CDIOCRESUME failed");

	PlayLooping = looping;
	PlayTrack = track;
	Playing = true;
	unguard;
}

//==========================================================================
//
//	VBsdCDAudioDevice::Pause
//
//==========================================================================

void VBsdCDAudioDevice::Pause()
{
	guard(VBsdCDAudioDevice::Pause);
	if (!Playing)
		return;

	if (ioctl(CDFile, CDIOCPAUSE) == -1)
		GCon->Log(NAME_Dev, "ioctl CDIOCPAUSE failed");

	WasPlaying = Playing;
	Playing = false;
	unguard;
}

//==========================================================================
//
//	VBsdCDAudioDevice::Resume
//
//==========================================================================

void VBsdCDAudioDevice::Resume()
{
	guard(VBsdCDAudioDevice::Resume);
	if (!WasPlaying)
		return;

	if (ioctl(CDFile, CDIOCRESUME) == -1)
		GCon->Log(NAME_Dev, "ioctl CDIOCRESUME failed");

	Playing = true;
	unguard;
}

//==========================================================================
//
//	VBsdCDAudioDevice::Stop
//
//==========================================================================

void VBsdCDAudioDevice::Stop()
{
	guard(VBsdCDAudioDevice::Stop);
	if (!Playing)
		return;

	if (ioctl(CDFile, CDIOCSTOP) == -1)
		GCon->Log(NAME_Dev, "ioctl CDIOCSTOP failed");

	WasPlaying = false;
	Playing = false;
	unguard;
}

//==========================================================================
//
//	VBsdCDAudioDevice::OpenDoor
//
//==========================================================================

void VBsdCDAudioDevice::OpenDoor()
{
	guard(VBsdCDAudioDevice::OpenDoor);
	if (ioctl(CDFile, CDIOCEJECT) == -1)
		GCon->Log(NAME_Dev, "ioctl CDIOCEJECT failed");
	unguard;
}

//==========================================================================
//
//	VBsdCDAudioDevice::CloseDoor
//
//==========================================================================

void VBsdCDAudioDevice::CloseDoor()
{
	guard(VBsdCDAudioDevice::CloseDoor);
	if (ioctl(CDFile, CDIOCCLOSE) == -1)
		GCon->Log(NAME_Dev, "ioctl CDIOCCLOSE failed");
	unguard;
}

#endif
