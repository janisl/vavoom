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

class VWavAudioCodec : public VAudioCodec
{
public:
	FArchive*		Ar;
	int				SamplesLeft;

	VWavAudioCodec(FArchive* InAr);
	~VWavAudioCodec();
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

IMPLEMENT_AUDIO_CODEC(VWavAudioCodec, "Wav");

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VVorbisAudioCodec::VVorbisAudioCodec
//
//==========================================================================

VWavAudioCodec::VWavAudioCodec(FArchive* InAr)
: Ar(InAr)
{
	guard(VWavAudioCodec::VWavAudioCodec);
	Ar->Seek(44);
	SamplesLeft = (Ar->TotalSize() - 44) / 4;
	unguard;
}

//==========================================================================
//
//	VVorbisAudioCodec::VVorbisAudioCodec
//
//==========================================================================

VWavAudioCodec::~VWavAudioCodec()
{
	guard(VWavAudioCodec::~VWavAudioCodec);
	Ar->Close();
	delete Ar;
	Ar = NULL;
	unguard;
}

//==========================================================================
//
//	VWavAudioCodec::Decode
//
//==========================================================================

int VWavAudioCodec::Decode(short* Data, int NumSamples)
{
	guard(VWavAudioCodec::Decode);
	if (NumSamples > SamplesLeft)
		NumSamples = SamplesLeft;
	Ar->Serialise(Data, NumSamples * 4);
	SamplesLeft -= NumSamples;
	return NumSamples;
	unguard;
}

//==========================================================================
//
//	VWavAudioCodec::Finished
//
//==========================================================================

bool VWavAudioCodec::Finished()
{
	return !SamplesLeft;
}

//==========================================================================
//
//	VWavAudioCodec::Restart
//
//==========================================================================

void VWavAudioCodec::Restart()
{
	guard(VWavAudioCodec::Restart);
	Ar->Seek(44);
	SamplesLeft = (Ar->TotalSize() - 44) / 4;
	unguard;
}

//==========================================================================
//
//	VWavAudioCodec::Create
//
//==========================================================================

VAudioCodec* VWavAudioCodec::Create(FArchive* InAr)
{
	guard(VWavAudioCodec::Create);
	char Header[12];
	InAr->Seek(0);
	InAr->Serialise(Header, 12);
	if (!memcmp(Header, "RIFF", 4) && !memcmp(Header + 8, "WAVE", 4))
	{
		return new VWavAudioCodec(InAr);
	}
	return NULL;
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2005/10/18 20:53:04  dj_jl
//	Implemented basic support for streamed music.
//
//**************************************************************************
