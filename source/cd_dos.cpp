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

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VDosCDAudioDevice : public VCDAudioDevice
{
public:
	int			CDVolume;

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

static int bcd_open();
static void bcd_close();

static void bcd_open_door();
static void bcd_close_door();

static int bcd_audio_busy();

static int bcd_get_audio_info();
static int bcd_track_is_audio(int trackno);
static void bcd_play_track(int tracknum);
static void bcd_set_volume(int);
static void bcd_stop();
#define bcd_pause bcd_stop
static void bcd_resume();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CD_AUDIO_DEVICE(VDosCDAudioDevice, CDDRV_Default, "Default",
	"DOS CD audio device", NULL);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VCvarI		cd_volume("cd_volume", "255", CVAR_Archive);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VDosCDAudioDevice::Init
//
//==========================================================================

void VDosCDAudioDevice::Init()
{
	guard(VDosCDAudioDevice::Init);
	if (GArgs.CheckParm("-nosound") || GArgs.CheckParm("-nocdaudio"))
	{
		return;
	}

	if (!bcd_open())
	{
		return;
	}

	for (int i = 0; i < 100; i++)
		Remap[i] = i;
	CDVolume = 255;
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
//	VDosCDAudioDevice::Update
//
//==========================================================================

void VDosCDAudioDevice::Update()
{
	guard(VDosCDAudioDevice::Update);
	static double	lastUpdate = 0;
	double			nowTime;

	if (!Initialised || !Enabled)
	{
		return;
	}

	if (cd_volume < 0)
	{
		cd_volume = 0;
	}

	if (cd_volume > 255)
	{
		cd_volume = 255;
	}

	if (cd_volume != CDVolume)
	{
		CDVolume = cd_volume;
		bcd_set_volume(CDVolume);
	}

	nowTime = Sys_Time();
	if (nowTime - lastUpdate < 0.5)
	{
		return;
	}
	lastUpdate = nowTime;

	if (Playing)
	{
		if (!bcd_audio_busy())
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
//	VDosCDAudioDevice::Shutdown
//
//==========================================================================

void VDosCDAudioDevice::Shutdown()
{
	guard(VDosCDAudioDevice::Shutdown);
	if (!Initialised)
		return;

	Stop();
	bcd_close();
	Initialised = false;
	unguard;
}

//==========================================================================
//
//	VDosCDAudioDevice::GetInfo
//
//==========================================================================

void VDosCDAudioDevice::GetInfo()
{
	guard(VDosCDAudioDevice::GetInfo);
	CDValid = false;

	MaxTrack = bcd_get_audio_info();

	if (MaxTrack == 0)
	{
		GCon->Log(NAME_Dev, "CDAudio: no music tracks");
		return;
	}

	CDValid = true;
	unguard;
}

//==========================================================================
//
//	VDosCDAudioDevice::Play
//
//==========================================================================

void VDosCDAudioDevice::Play(int track, bool looping)
{
	guard(VDosCDAudioDevice::Play);
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
	if (!bcd_track_is_audio(track))
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
	else
	{
		if (bcd_audio_busy())
			bcd_stop();
	}

	bcd_play_track(track);

	PlayLooping = looping;
	PlayTrack = track;
	Playing = true;
	unguard;
}

//==========================================================================
//
//	VDosCDAudioDevice::Pause
//
//==========================================================================

void VDosCDAudioDevice::Pause()
{
	guard(VDosCDAudioDevice::Pause);
	if (!Playing)
		return;

	bcd_pause();

	WasPlaying = Playing;
	Playing = false;
	unguard;
}

//==========================================================================
//
//	VDosCDAudioDevice::Resume
//
//==========================================================================

void VDosCDAudioDevice::Resume()
{
	guard(VDosCDAudioDevice::Resume);
	if (!WasPlaying)
		return;

	bcd_resume();

	Playing = true;
	unguard;
}

//==========================================================================
//
//	VDosCDAudioDevice::Stop
//
//==========================================================================

void VDosCDAudioDevice::Stop()
{
	guard(VDosCDAudioDevice::Stop);
	if (!Playing)
		return;

	bcd_stop();

	WasPlaying = false;
	Playing = false;
	unguard;
}

//==========================================================================
//
//	VDosCDAudioDevice::OpenDoor
//
//==========================================================================

void VDosCDAudioDevice::OpenDoor()
{
	guard(VDosCDAudioDevice::OpenDoor);
	bcd_open_door();
	unguard;
}

//==========================================================================
//
//	VDosCDAudioDevice::CloseDoor
//
//==========================================================================

void VDosCDAudioDevice::CloseDoor()
{
	guard(VDosCDAudioDevice::CloseDoor);
	bcd_close_door();
	unguard;
}

//**************************************************************************
//**
//**	bcd.c
//**
//**	Brennan's CD-ROM Audio Playing Library
//**   by Brennan Underwood, http://brennan.home.ml.org/
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <unistd.h>

// MACROS ------------------------------------------------------------------

#define ERROR_BIT		(1 << 15)
#define BUSY_BIT		(1 << 9)
#define BCD_DOOR_OPEN	1

// TYPES -------------------------------------------------------------------

#pragma pack(1)

/* I know 'typedef struct {} bleh' is a bad habit, but... */
typedef struct
{
	unsigned char len;
	unsigned char unit;
	unsigned char command;
	unsigned short status;
	unsigned char reserved[8];
} RequestHeader;

typedef struct
{
	RequestHeader request_header;
	unsigned char descriptor;
	unsigned long address;
	unsigned short len;
	unsigned short secnum;
	unsigned long ptr;
} IOCTLI;

typedef struct
{
	unsigned char control;
	unsigned char lowest;
	unsigned char highest;
	unsigned char total[4];
} DiskInfo;

typedef struct
{
	unsigned char control;
	unsigned char track_number;
	unsigned char start[4];
	unsigned char info;
} TrackInfo;

typedef struct
{
	RequestHeader request;
	unsigned char mode;
	unsigned long start;
	unsigned long len;
} PlayRequest;

typedef struct
{
	RequestHeader request;
} StopRequest;

typedef struct
{
	RequestHeader request;
} ResumeRequest;

typedef struct
{
	unsigned char control;
	unsigned char input0;
	unsigned char volume0;
	unsigned char input1;
	unsigned char volume1;
	unsigned char input2;
	unsigned char volume2;
	unsigned char input3;
	unsigned char volume3;
} VolumeRequest;

typedef struct
{
	unsigned char control;
	unsigned long status;
} StatusRequest;

typedef struct
{
	unsigned char control;
	unsigned char mode;
	unsigned long loc;
} PositionRequest;

#pragma pack()

typedef struct
{
	int		is_audio;
	int		start;
	int		end;
	int		len;
} Track;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int		mscdex_version;
static int		cur_drive;	/* current drive - for use by mscdex functions */
static int		num_tracks;
static int		lowest_track;
static int		highest_track;

static Track	tracks[100];

static int		dos_mem_segment;
static int		dos_mem_selector = -1;

static int		_status;

static const char *errorcodes[] =
{
	"Write-protect violation",
	"Unknown unit",
	"Drive not ready",
	"Unknown command",
	"CRC error",
	"Bad drive request structure length",
	"Seek error",
	"Unknown media",
	"Sector not found",
	"Printer out of paper: world coming to an end",/* I mean really, on a CD? */
	"Write fault",
	"Read fault",
	"General failure",
	"Reserved",
	"Reserved",
	"Invalid disk change"
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	bcd_open
//
//	handles the setup for CD-ROM audio interface
//
//==========================================================================

static int bcd_open()
{
	__dpmi_regs	regs;

	/* disk I/O wouldn't work anyway if you set sizeof tb this low, but... */
	if (_go32_info_block.size_of_transfer_buffer < 4096)
	{
		GCon->Log(NAME_Init, "bcd_open: Transfer buffer too small");
		return 0;
	}

	memset(&regs, 0, sizeof regs);
	regs.x.ax = 0x1500;
	regs.x.bx = 0x0;
	__dpmi_int(0x2f, &regs);
	if (regs.x.bx == 0)
	{	/* abba no longer lives */
		GCon->Log(NAME_Init, "bcd_open: MSCDEX not found");
		return 0;
	}

	cur_drive = regs.x.cx; /* use the first drive */

	/* check for mscdex at least 2.0 */
	memset(&regs, 0, sizeof regs);
	regs.x.ax = 0x150C;
	__dpmi_int(0x2f, &regs);
	if (regs.x.bx == 0)
	{
		GCon->Log(NAME_Init, "bcd_open: MSCDEX version < 2.0");
		return 0;
	}
	mscdex_version = regs.x.bx;

	/* allocate 256 bytes of dos memory for the command blocks */
	if ((dos_mem_segment = __dpmi_allocate_dos_memory(16, &dos_mem_selector))<0)
	{
		GCon->Log(NAME_Init, "bcd_open: Could not allocate 256 bytes of DOS memory");
		return 0;
	}

	GCon->Logf(NAME_Init, "MSCDEX version %d.%d", mscdex_version / 0x100,
		mscdex_version % 0x100);
	return mscdex_version;
}

//==========================================================================
//
//	bcd_close
//
//	Shuts down CD-ROM audio interface
//
//==========================================================================

static void bcd_close()
{
	if (dos_mem_selector != -1)
	{
		__dpmi_free_dos_memory(dos_mem_selector);
		dos_mem_selector = -1;
	}
}

//==========================================================================
//
//	bcd_ioctl
//
//	DOS IOCTL w/ command block
//
//==========================================================================

static int bcd_ioctl(IOCTLI *ioctli, void *command, int len, char *act)
{
	int				ioctli_len = sizeof(IOCTLI);
	unsigned long	command_address = dos_mem_segment << 4;
	__dpmi_regs		regs;
	int				error_code;

	memset(&regs, 0, sizeof regs);
	regs.x.es = (__tb >> 4) & 0xffff;
	regs.x.ax = 0x1510;
	regs.x.bx = __tb & 0xf;
	regs.x.cx = cur_drive;
	ioctli->address = dos_mem_segment << 16;
	ioctli->len = len;
	dosmemput(ioctli, ioctli_len, __tb);		/* put ioctl into dos area */
	dosmemput(command, len, command_address);	/* and command too */
	if (__dpmi_int(0x2f, &regs) == -1)
	{
		Sys_Error("bcd_ioctl: __dpmi_int() failed");
	}
	dosmemget(__tb, ioctli_len, ioctli);		/* retrieve results */
	dosmemget(command_address, len, command);
	_status = ioctli->request_header.status;
	if (_status & ERROR_BIT)
	{
		error_code = _status & 0xff;
		GCon->Logf(NAME_Dev, "CD %s failed (%s)", act,
			(error_code < 0 || error_code > 0xf) ? "Invalid error" :
			errorcodes[error_code]);
		return 1;
	}
	else
	{
		return 0;
	}
}

//==========================================================================
//
//	bcd_ioctl2
//
//	no command block
//
//==========================================================================

static void bcd_ioctl2(void *cmd, int len)
{
	__dpmi_regs		regs;

	memset(&regs, 0, sizeof regs);
	regs.x.es = (__tb >> 4) & 0xffff;
	regs.x.ax = 0x1510;
	regs.x.bx = __tb & 0xf;
	regs.x.cx = cur_drive;
	dosmemput(cmd, len, __tb); /* put ioctl block in dos arena */
	if (__dpmi_int(0x2f, &regs) == -1)
	{
		Sys_Error("bcd_ioctl2: __dpmi_int() failed");
	}
}

//==========================================================================
//
//	red2hsg
//
//==========================================================================

static int red2hsg(byte *r)
{
	return r[0] + r[1] * 75 + r[2] * 4500 - 150;
}

//==========================================================================
//
//	bcd_open_door
//
//==========================================================================

static void bcd_open_door()
{
	IOCTLI	ioctli;
	char	eject = 0;

	memset(&ioctli, 0, sizeof ioctli);
	ioctli.request_header.len = sizeof ioctli;
	ioctli.request_header.command = 12;
	ioctli.len = 1;
	bcd_ioctl(&ioctli, &eject, sizeof eject, "open door");
}

//==========================================================================
//
//	bcd_close_door
//
//==========================================================================

static void bcd_close_door()
{
	IOCTLI	ioctli;
	char	closeit = 5;

	memset(&ioctli, 0, sizeof ioctli);
	ioctli.request_header.len = sizeof ioctli;
	ioctli.request_header.command = 12;
	ioctli.len = 1;
	bcd_ioctl(&ioctli, &closeit, sizeof closeit, "close door");
}

//==========================================================================
//
//	bcd_get_track_info
//
//	Internal function to get track info
//
//==========================================================================

static void bcd_get_track_info(int n, Track *t)
{
	IOCTLI		ioctli;
	TrackInfo	info;

	memset(&ioctli, 0, sizeof ioctli);
	memset(&info, 0, sizeof info);
	ioctli.request_header.len = sizeof ioctli;
	ioctli.request_header.command = 3;
	info.control = 11;
	info.track_number = n;
	bcd_ioctl(&ioctli, &info, sizeof info, "track info");
	t->start = red2hsg(info.start);
	if (info.info & 64)
		t->is_audio = 0;
	else
		t->is_audio = 1;
}

//==========================================================================
//
//  bcd_get_audio_info
//
//==========================================================================

static int bcd_get_audio_info()
{
	IOCTLI		ioctli;
	DiskInfo	disk_info;
	int			i;

	memset(&disk_info, 0, sizeof disk_info);
	memset(&ioctli, 0, sizeof ioctli);

	ioctli.request_header.len = 26;
	ioctli.request_header.command = 3;
	ioctli.len = 7;
	disk_info.control = 10;
	if (bcd_ioctl(&ioctli, &disk_info, sizeof disk_info, "get audio info"))
	{
		return 0;
	}

	lowest_track = disk_info.lowest;
	highest_track = disk_info.highest;
	num_tracks = disk_info.highest - disk_info.lowest + 1;

	/* get track starts */
	for (i = lowest_track; i <= highest_track; i++)
		bcd_get_track_info(i, tracks+i);

	/* figure out track ends */
	for (i = lowest_track; i < highest_track; i++)
		tracks[i].end = tracks[i + 1].start - 1;
	tracks[i].end = red2hsg(disk_info.total);
	for (i = lowest_track; i <= highest_track; i++)
		tracks[i].len = tracks[i].end - tracks[i].start;

	return num_tracks;
}

//==========================================================================
//
//	bcd_track_is_audio
//
//==========================================================================

static int bcd_track_is_audio(int trackno)
{
	return tracks[trackno + lowest_track - 1].is_audio;
}

//==========================================================================
//
//	bcd_play_track
//
//==========================================================================

static void bcd_play_track(int trackno)
{
	PlayRequest	cmd;

	memset(&cmd, 0, sizeof cmd);

	cmd.request.len = sizeof cmd;
	cmd.request.command = 132;
	cmd.start = tracks[trackno + lowest_track - 1].start;
	cmd.len   = tracks[trackno + lowest_track - 1].len;
	bcd_ioctl2(&cmd, sizeof cmd);
}

//==========================================================================
//
//	bcd_stop
//
//==========================================================================

static void bcd_stop()
{
	StopRequest	cmd;

	memset(&cmd, 0, sizeof cmd);
	cmd.request.len = sizeof cmd;
	cmd.request.command = 133;
	bcd_ioctl2(&cmd, sizeof cmd);
}

//==========================================================================
//
//	bcd_resume
//
//==========================================================================

static void bcd_resume()
{
	ResumeRequest	cmd;

	memset(&cmd, 0, sizeof cmd);
	cmd.request.len = sizeof cmd;
	cmd.request.command = 136;
	bcd_ioctl2(&cmd, sizeof cmd);
}

//==========================================================================
//
//	bcd_set_volume
//
//==========================================================================

static void bcd_set_volume(int volume)
{
	IOCTLI			ioctli;
	VolumeRequest	v;

	if (volume > 255) volume = 255;
	else if (volume < 0) volume = 0;
	memset(&ioctli, 0, sizeof ioctli);
	ioctli.request_header.len = sizeof ioctli;
	ioctli.request_header.command = 12;
	ioctli.len = sizeof v;
	v.control = 3;
	v.volume0 = volume;
	v.input0 = 0;
	v.volume1 = volume;
	v.input1 = 1;
	v.volume2 = volume;
	v.input2 = 2;
	v.volume3 = volume;
	v.input3 = 3;

	bcd_ioctl(&ioctli, &v, sizeof v, "set volume");
}

//==========================================================================
//
//	bcd_audio_busy
//
//==========================================================================

static int bcd_audio_busy()
{
	IOCTLI			ioctli;
	DiskInfo		disk_info;
	StatusRequest	req;

	/* If the door is open, then the head is busy, and so the busy bit is
		on. It is not, however, playing audio. */
	memset(&ioctli, 0, sizeof ioctli);
	memset(&req, 0, sizeof req);
	ioctli.request_header.len = sizeof ioctli; // ok
	ioctli.request_header.command = 3;
	ioctli.len = sizeof req;
	req.control = 6;
	bcd_ioctl(&ioctli, &req, sizeof req, "status");
	if (req.status & BCD_DOOR_OPEN)
	{
		return 0;
	}

	/* get cd info as an excuse to get a look at the status word */
	memset(&disk_info, 0, sizeof disk_info);
	memset(&ioctli, 0, sizeof ioctli);

	ioctli.request_header.len = 26;
	ioctli.request_header.command = 3;
	ioctli.len = 7;
	disk_info.control = 10;
	if (bcd_ioctl(&ioctli, &disk_info, sizeof disk_info, "get status word"))
	{
		return 0;
	}
	if (!(_status & BUSY_BIT))
	{
		return 0;
	}

	return 1;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.8  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
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
