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

#ifdef _WIN32
#include "winlocal.h"
#endif
#include "gme/src/Music_Emu.h"

#include "gamedefs.h"
#include "snd_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VGMEAudioCodec : public VAudioCodec
{
public:
	Music_Emu*		emu;
	gme_info_t*		info;
	long			length;
	bool			playing;

	VGMEAudioCodec(void* Data, int Size, gme_type_t in_file);
	~VGMEAudioCodec();
	int Decode(short* Data, int NumSamples);
	bool Finished();
	void Restart();

	static VAudioCodec* Create(VStream* InStrm);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_AUDIO_CODEC(VGMEAudioCodec, "Game Music Emu");

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VGMEAudioCodec::VGMEAudioCodec
//
//==========================================================================

VGMEAudioCodec::VGMEAudioCodec(void* Data, int Size, gme_type_t in_file)
: emu(in_file->new_emu()), playing(false)
{
	// Create emulator and set up playback parameters
	if (emu == NULL)
	{
		GCon->Log("Couldn't create Emulator, Out of memory.");
	}
//	emu->set_gain(3.0);
	emu->set_sample_rate(44100);
	
	// Load music file into emulator
	gme_load_data(emu, Data, Size);
	
	// Start track (0 is first track)
	gme_err_t err = gme_start_track(emu, 0);
	if (err)
	{
		Z_Free(Data);
	}

	// Set Play length here
	err = gme_track_info(emu, &info, 0);
	if (!err && !info->length && !info->loop_length)
	{
		// Look for length inside of track info
		if (info->length > 0)
		{
			length = info->length;
		}
		if (info->loop_length > 0)
		{
			length = info->intro_length + info->loop_length * 2;
		}
		if (!length)
		{
			length = 150000;
		}
	}
	else
	{
		// Or set a default length of 2.5 mins
		length = 150000;
	}
	// Fade sound before finishing
	gme_set_fade(emu, length - 2500);
}

//==========================================================================
//
//	VGMEAudioCodec::~VGMEAudioCodec
//
//==========================================================================

VGMEAudioCodec::~VGMEAudioCodec()
{
	guard(VGMEAudioCodec::~VGMEAudioCodec);
	// Delete emulator and track info
	playing = false;

	gme_free_info(info);
	info = NULL;

	gme_delete(emu);
	emu = NULL;
	unguard;
}

//==========================================================================
//
//	VGMEAudioCodec::Decode
//
//==========================================================================

int VGMEAudioCodec::Decode(short* Data, int NumSamples)
{
	guard(VGMEAudioCodec::Decode);
	// Are we done yet?
	if (!gme_play(emu, (NumSamples * 2), Data))
	{
		if (!gme_track_ended(emu))
		{
			playing = true;
		}
		else
		{
			playing = false;
		}
	}
	else
	{
		Z_Free(Data);
	}

	// This function converts current time in msec to
	// number of samples played
	return emu->msec_to_samples(gme_tell(emu));
	unguard;
}

//==========================================================================
//
//	VGMEAudioCodec::Finished
//
//==========================================================================

bool VGMEAudioCodec::Finished()
{
	// Finish if we aren't looping and have finished playing
	return !playing;
}

//==========================================================================
//
//	VGMEAudioCodec::Restart
//
//==========================================================================

void VGMEAudioCodec::Restart()
{
	guard(VGMEAudioCodec::Restart);
	// If music is looping, restart playback by simply
	// starting track 0 again and setting playing state
	gme_start_track(emu, 0);
	playing = true;
	unguard;
}

//==========================================================================
//
//	VGMEAudioCodec::Create
//
//==========================================================================

VAudioCodec* VGMEAudioCodec::Create(VStream* InStrm)
{
	guard(VGMEAudioCodec::Create);
	// Scan file's header to determine it's a playable type
	byte Header[4];
	InStrm->Seek(0);
	InStrm->Serialise(Header, 4);
	gme_type_t file = gme_identify_extension(gme_identify_header(Header));
	if (!file)
	{
		// Incorrect header or incompatible file
		return NULL;
	}

	//	Start playback.
	int Size = InStrm->TotalSize();
	void* Data = Z_Malloc(Size);
	InStrm->Seek(0);
	InStrm->Serialise(Data, Size);

	InStrm->Close();
	delete InStrm;
	InStrm = NULL;

	return new VGMEAudioCodec(Data, Size, file);
	unguard;
}
