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
#include "snd_local.h"

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

	static ControlMode	MyControlMode;

	VTimidityAudioCodec(MidiSong* InSong);
	~VTimidityAudioCodec();
	int Decode(short* Data, int NumSamples);
	bool Finished();
	void Restart();

	//	Control mode functions.
	static int ctl_msg(int, int, const char*, ...);

	static VAudioCodec* Create(VStream* InStrm);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_AUDIO_CODEC(VTimidityAudioCodec, "Timidity");

ControlMode		VTimidityAudioCodec::MyControlMode =
{
	VTimidityAudioCodec::ctl_msg,
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
//	VTimidityAudioCodec::VTimidityAudioCodec
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
//	VTimidityAudioCodec::~VTimidityAudioCodec
//
//==========================================================================

VTimidityAudioCodec::~VTimidityAudioCodec()
{
	guard(VTimidityAudioCodec::~VTimidityAudioCodec);
	Timidity_Stop();
	Timidity_FreeSong(Song);
	Timidity_Close();
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

int VTimidityAudioCodec::ctl_msg(int type, int verbosity_level, const char *fmt, ...)
{
	char Buf[1024];
	va_list ap;
	if ((type == CMSG_TEXT || type == CMSG_INFO || type == CMSG_WARNING) &&
//		MyControlMode.verbosity < verbosity_level)
		0 < verbosity_level)
		return 0;
	va_start(ap, fmt);
	vsprintf(Buf, fmt, ap);
	GCon->Log(Buf);
	va_end(ap);
	return 0;
}

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

	//	Register our control mode.
	ctl = &MyControlMode;

	//	Initialise Timidity.
	add_to_pathlist(s_timidity_patches);
	if (Timidity_Init())
	{
		GCon->Logf("Timidity init failed");
		return NULL;
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
		Timidity_Close();
		return NULL;
	}
	InStrm->Close();
	delete InStrm;

	//	Create codec.
	return new VTimidityAudioCodec(Song);
	unguard;
}
