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
extern "C" {
#include "timidity/timidity.h"
#include "timidity/config.h"
#include "timidity/output.h"
#include "timidity/controls.h"
};

#define clogf		GCon->Logf

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VTimidityAudioCodec : public VAudioCodec
{
public:
	FArchive*		Ar;
	MidiSong*		Song;

	VTimidityAudioCodec(FArchive* InAr);
	~VTimidityAudioCodec();
	int Decode(short* Data, int NumSamples);
	bool Finished();
	void Restart();
	static VAudioCodec* Create(FArchive* InAr);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_AUDIO_CODEC(VTimidityAudioCodec, "Timidity");

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TCvarI		s_timidity("s_timidity", "0", CVAR_ARCHIVE);

static bool			timidity_initialised;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	Minimal control mode -- no interaction, just stores messages.
//
//==========================================================================

static void ctl_refresh(void);
static void ctl_total_time(int tt);
static void ctl_master_volume(int mv);
static void ctl_file_name(char *name);
static void ctl_current_time(int ct);
static void ctl_note(int v);
static void ctl_program(int ch, int val);
static void ctl_volume(int channel, int val);
static void ctl_expression(int channel, int val);
static void ctl_panning(int channel, int val);
static void ctl_sustain(int channel, int val);
static void ctl_pitch_bend(int channel, int val);
static void ctl_reset(void);
static int ctl_open(int using_stdin, int using_stdout);
static void ctl_close(void);
static int ctl_read(int32 *valp);
static int cmsg(int type, int verbosity_level, char *fmt, ...);

/**********************************/
/* export the interface functions */

ControlMode vavoom_control_mode = 
{
	"Vavoom interface", 's',
	1,0,0,
	ctl_open,NULL, ctl_close, ctl_read, cmsg,
	ctl_refresh, ctl_reset, ctl_file_name, ctl_total_time, ctl_current_time, 
	ctl_note, 
	ctl_master_volume, ctl_program, ctl_volume, 
	ctl_expression, ctl_panning, ctl_sustain, ctl_pitch_bend
};

static int ctl_open(int, int)
{
  vavoom_control_mode.opened = 1;
  return 0;
}

static void ctl_close()
{ 
  vavoom_control_mode.opened = 0;
}

static int ctl_read(int32*)
{
  return RC_NONE;
}

static int cmsg(int type, int verbosity_level, char *fmt, ...)
{
	va_list ap;
	if ((type == CMSG_TEXT || type == CMSG_INFO || type == CMSG_WARNING) &&
		vavoom_control_mode.verbosity < verbosity_level)
		return 0;
	va_start(ap, fmt);
	vsprintf(timidity_error, fmt, ap);
	GCon->Log(timidity_error);
	va_end(ap);
	return 0;
}

static void ctl_refresh() { }

static void ctl_total_time(int) {}

static void ctl_master_volume(int) {}

static void ctl_file_name(char*) {}

static void ctl_current_time(int) {}

static void ctl_note(int) {}

static void ctl_program(int, int) {}

static void ctl_volume(int, int) {}

static void ctl_expression(int, int) {}

static void ctl_panning(int, int) {}

static void ctl_sustain(int, int) {}

static void ctl_pitch_bend(int, int) {}

static void ctl_reset() {}

//==========================================================================
//
//	Functions to output RIFF WAVE format data to a file or stdout.
//
//==========================================================================

PlayMode vavoom_play_mode =
{
	DEFAULT_RATE, PE_16BIT | PE_SIGNED, "Vavoom audio"
};

//==========================================================================
//
//	VVorbisAudioCodec::VVorbisAudioCodec
//
//==========================================================================

VTimidityAudioCodec::VTimidityAudioCodec(FArchive* InAr)
: Ar(InAr)
{
	guard(VTimidityAudioCodec::VTimidityAudioCodec);
	int Size = Ar->TotalSize();
	void* Data = Z_Malloc(Size);
	Ar->Seek(0);
	Ar->Serialise(Data, Size);
	Song = Timidity_LoadSong(Data, Size);
	Z_Free(Data);
	if (!Song)
		clogf("Failed to load song");
	Timidity_SetVolume(100);
	Timidity_Start(Song);
	clogf("Timidity initialised");
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
	Ar->Close();
	delete Ar;
	Ar = NULL;
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
//	VTimidityAudioCodec::Create
//
//==========================================================================

VAudioCodec* VTimidityAudioCodec::Create(FArchive* InAr)
{
	guard(VTimidityAudioCodec::Create);
	//	Handle only if enabled.
	if (!s_timidity)
		return NULL;

	//	Check if it's a MIDI file.
	char Header[4];
	InAr->Seek(0);
	InAr->Serialise(Header, 4);
	if (memcmp(Header, MIDIMAGIC, 4))
	{
		return NULL;
	}

	play_mode_list[0] = &vavoom_play_mode;
	play_mode = &vavoom_play_mode;
	ctl_list[0] = &vavoom_control_mode;
	ctl = &vavoom_control_mode;

	//	Initialise Timidity.
	if (!timidity_initialised)
	{
		if (Timidity_Init(44100, 16, 2, 2 * 1024))
		{
			clogf("Timidity init failed");
			return NULL;
		}
		timidity_initialised = true;
	}

	//	Create codec.
	return new VTimidityAudioCodec(InAr);
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2005/10/28 17:50:01  dj_jl
//	Added Timidity driver.
//
//**************************************************************************
