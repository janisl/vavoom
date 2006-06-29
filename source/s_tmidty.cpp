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

#include "gamedefs.h"
#include "s_local.h"

#include "timidity/timidity.h"
#include "timidity/config.h"
#include "timidity/output.h"
#include "timidity/controls.h"
#include "timidity/common.h"

using namespace LibTimidity;

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VTimidityAudioCodec : public VAudioCodec
{
public:
	MidiSong*			Song;

	static bool			TimidityInitialised;
	static ControlMode	MyControlMode;
	static PlayMode		MyPlayMode;

	VTimidityAudioCodec(MidiSong* InSong);
	~VTimidityAudioCodec();
	int Decode(short* Data, int NumSamples);
	bool Finished();
	void Restart();

	//	Control mode functions.
	static void ctl_refresh();
	static void ctl_total_time(int);
	static void ctl_master_volume(int);
	static void ctl_file_name(char*);
	static void ctl_current_time(int);
	static void ctl_note(int);
	static void ctl_program(int, int);
	static void ctl_volume(int, int);
	static void ctl_expression(int, int);
	static void ctl_panning(int, int);
	static void ctl_sustain(int, int);
	static void ctl_pitch_bend(int, int);
	static void ctl_reset();
	static int ctl_open(int, int);
	static void ctl_close();
	static int ctl_read(int32*);
	static int ctl_msg(int, int, char*, ...);

	static VAudioCodec* Create(VStream* InStrm);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_AUDIO_CODEC(VTimidityAudioCodec, "Timidity");

bool			VTimidityAudioCodec::TimidityInitialised;
ControlMode		VTimidityAudioCodec::MyControlMode =
{
	"Vavoom interface", 's', 0, 0, 0,
	VTimidityAudioCodec::ctl_open,
	NULL,
	VTimidityAudioCodec::ctl_close,
	VTimidityAudioCodec::ctl_read,
	VTimidityAudioCodec::ctl_msg,
	VTimidityAudioCodec::ctl_refresh,
	VTimidityAudioCodec::ctl_reset,
	VTimidityAudioCodec::ctl_file_name,
	VTimidityAudioCodec::ctl_total_time,
	VTimidityAudioCodec::ctl_current_time,
	VTimidityAudioCodec::ctl_note,
	VTimidityAudioCodec::ctl_master_volume,
	VTimidityAudioCodec::ctl_program,
	VTimidityAudioCodec::ctl_volume,
	VTimidityAudioCodec::ctl_expression,
	VTimidityAudioCodec::ctl_panning,
	VTimidityAudioCodec::ctl_sustain,
	VTimidityAudioCodec::ctl_pitch_bend
};
PlayMode		VTimidityAudioCodec::MyPlayMode =
{
	DEFAULT_RATE, PE_16BIT | PE_SIGNED, "Vavoom audio"
};

#if defined(DJGPP) || defined(_WIN32)
static VCvarI	s_timidity("s_timidity", "0", CVAR_Archive);
static VCvarS	s_timidity_patches("s_timidity_patches", "\\TIMIDITY", CVAR_Archive);
#else
static VCvarI	s_timidity("s_timidity", "1", CVAR_Archive);
static VCvarS	s_timidity_patches("s_timidity_patches", "/usr/share/timidity", CVAR_Archive);
#endif

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VVorbisAudioCodec::VVorbisAudioCodec
//
//==========================================================================

VTimidityAudioCodec::VTimidityAudioCodec(MidiSong* InSong)
: Song(InSong)
{
	guard(VTimidityAudioCodec::VTimidityAudioCodec);
	Timidity_SetVolume(100);
	Timidity_Start(Song);
	unguard;
}

//==========================================================================
//
//	VVorbisAudioCodec::VVorbisAudioCodec
//
//==========================================================================

VTimidityAudioCodec::~VTimidityAudioCodec()
{
	guard(VTimidityAudioCodec::~VTimidityAudioCodec);
	Timidity_Stop();
	Timidity_FreeSong(Song);
	unguard;
}

//==========================================================================
//
//	VTimidityAudioCodec::Decode
//
//==========================================================================

int VTimidityAudioCodec::Decode(short* Data, int NumSamples)
{
	guard(VTimidityAudioCodec::Decode);
	return Timidity_PlaySome(Data, NumSamples);
	unguard;
}

//==========================================================================
//
//	VTimidityAudioCodec::Finished
//
//==========================================================================

bool VTimidityAudioCodec::Finished()
{
	return !Timidity_Active();
}

//==========================================================================
//
//	VTimidityAudioCodec::Restart
//
//==========================================================================

void VTimidityAudioCodec::Restart()
{
	guard(VTimidityAudioCodec::Restart);
	Timidity_Start(Song);
	unguard;
}

//==========================================================================
//
//	Minimal control mode -- no interaction, just stores messages.
//
//==========================================================================

int VTimidityAudioCodec::ctl_open(int, int)
{
  MyControlMode.opened = 1;
  return 0;
}

void VTimidityAudioCodec::ctl_close()
{ 
  MyControlMode.opened = 0;
}

int VTimidityAudioCodec::ctl_read(int32*)
{
  return RC_NONE;
}

int VTimidityAudioCodec::ctl_msg(int type, int verbosity_level, char *fmt, ...)
{
	va_list ap;
	if ((type == CMSG_TEXT || type == CMSG_INFO || type == CMSG_WARNING) &&
		MyControlMode.verbosity < verbosity_level)
		return 0;
	va_start(ap, fmt);
	vsprintf(timidity_error, fmt, ap);
	GCon->Log(timidity_error);
	va_end(ap);
	return 0;
}

void VTimidityAudioCodec::ctl_refresh() {}

void VTimidityAudioCodec::ctl_total_time(int) {}

void VTimidityAudioCodec::ctl_master_volume(int) {}

void VTimidityAudioCodec::ctl_file_name(char*) {}

void VTimidityAudioCodec::ctl_current_time(int) {}

void VTimidityAudioCodec::ctl_note(int) {}

void VTimidityAudioCodec::ctl_program(int, int) {}

void VTimidityAudioCodec::ctl_volume(int, int) {}

void VTimidityAudioCodec::ctl_expression(int, int) {}

void VTimidityAudioCodec::ctl_panning(int, int) {}

void VTimidityAudioCodec::ctl_sustain(int, int) {}

void VTimidityAudioCodec::ctl_pitch_bend(int, int) {}

void VTimidityAudioCodec::ctl_reset() {}

//==========================================================================
//
//	VTimidityAudioCodec::Create
//
//==========================================================================

VAudioCodec* VTimidityAudioCodec::Create(VStream* InStrm)
{
	guard(VTimidityAudioCodec::Create);
	//	Handle only if enabled.
	if (!s_timidity)
		return NULL;

	//	Check if it's a MIDI file.
	char Header[4];
	InStrm->Seek(0);
	InStrm->Serialise(Header, 4);
	if (memcmp(Header, MIDIMAGIC, 4))
	{
		return NULL;
	}

	//	Register our play and control modes.
	play_mode_list[0] = &MyPlayMode;
	play_mode = &MyPlayMode;
	ctl_list[0] = &MyControlMode;
	ctl = &MyControlMode;

	//	Initialise Timidity.
	if (!TimidityInitialised)
	{
		add_to_pathlist(s_timidity_patches);
		if (Timidity_Init(44100, 16, 2, 2 * 1024))
		{
			GCon->Logf("Timidity init failed");
			return NULL;
		}
		TimidityInitialised = true;
	}

	//	Load song.
	int Size = InStrm->TotalSize();
	void* Data = Z_Malloc(Size);
	InStrm->Seek(0);
	InStrm->Serialise(Data, Size);
	MidiSong* Song = Timidity_LoadSongMem(Data, Size);
	Z_Free(Data);
	if (!Song)
	{
		GCon->Logf("Failed to load MIDI song");
		return NULL;
	}
	InStrm->Close();
	delete InStrm;

	//	Create codec.
	return new VTimidityAudioCodec(Song);
	unguard;
}
