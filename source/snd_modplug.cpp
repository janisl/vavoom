//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: snd_modplug.cpp 4297 2010-06-03 22:49:00Z firebrand_kh $
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
#include <modplug.h>
#else
#include <libmodplug/modplug.h>
#endif

#include "gamedefs.h"
#include "snd_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VModPlugAudioCodec : public VAudioCodec
{
public:
	ModPlugFile*		file;
	bool			playing;

	//	VAudioCodec interface.
	VModPlugAudioCodec(ModPlugFile* InFile);
	~VModPlugAudioCodec();
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

IMPLEMENT_AUDIO_CODEC(VModPlugAudioCodec, "ModPlug");

static VCvarI	s_modplug_hqmixer("snd_modplug_hqmixer", "0", CVAR_Archive);
static VCvarI	s_modplug_oversampling("snd_modplug_oversampling", "1", CVAR_Archive);
static VCvarI	s_modplug_noise_reduction("snd_modplug_noise_reduction", "0", CVAR_Archive);
static VCvarI	s_modplug_reverb("snd_modplug_reverb", "0", CVAR_Archive);
static VCvarI	s_modplug_megabass("snd_modplug_megabass", "0", CVAR_Archive);
static VCvarI	s_modplug_surround("snd_modplug_surround", "1", CVAR_Archive);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VModPlugAudioCodec::VModPlugAudioCodec
//
//==========================================================================

VModPlugAudioCodec::VModPlugAudioCodec(ModPlugFile* InFile)
: file(InFile), playing(true)
{
}

//==========================================================================
//
//	VModPlugAudioCodec::~VModPlugAudioCodec
//
//==========================================================================

VModPlugAudioCodec::~VModPlugAudioCodec()
{
	guard(VModPlugAudioCodec::~VModPlugAudioCodec);
	ModPlug_Unload(file);
	unguard;
}

//==========================================================================
//
//	VModPlugAudioCodec::Decode
//
//==========================================================================

int VModPlugAudioCodec::Decode(short* Data, int NumSamples)
{
	guard(VModPlugAudioCodec::Decode);
	int count = ModPlug_Read(file, Data, NumSamples * 4);
	if (count < NumSamples * 4)
	{
		memset(Data+count, 0, NumSamples*4-count);
		playing = false;
	}
	return count/4;
	unguard;
}

//==========================================================================
//
//	VModPlugAudioCodec::Finished
//
//==========================================================================

bool VModPlugAudioCodec::Finished()
{
	guard(VModPlugAudioCodec::Finished);
	return !playing;
	unguard;
}

//==========================================================================
//
//	VModPlugAudioCodec::Restart
//
//==========================================================================

void VModPlugAudioCodec::Restart()
{
	guard(VModPlugAudioCodec::Restart);
	ModPlug_Seek(file, 0);
	playing = true;
	unguard;
}

//==========================================================================
//
//	VModPlugAudioCodec::Create
//
//==========================================================================

VAudioCodec* VModPlugAudioCodec::Create(VStream* InStrm)
{
	guard(VModPlugAudioCodec::Create);
	if (snd_mod_player != 1)
	{
		return NULL;
	}
	//	Set up playback parameters.
	ModPlug_Settings settings;
	ModPlug_GetSettings(&settings);

	//  Check desired user settings and set them using flags
	if (s_modplug_oversampling)
	{
		settings.mFlags |= MODPLUG_ENABLE_OVERSAMPLING;
	}
	else
	{
		settings.mFlags &= ~MODPLUG_ENABLE_OVERSAMPLING;
	}
	if (s_modplug_noise_reduction)
	{
		settings.mFlags |= MODPLUG_ENABLE_NOISE_REDUCTION;
	}
	else
	{
		settings.mFlags &= ~MODPLUG_ENABLE_NOISE_REDUCTION;
	}
	if (s_modplug_reverb)
	{
		settings.mFlags |= MODPLUG_ENABLE_REVERB;
	}
	else
	{
		settings.mFlags &= ~MODPLUG_ENABLE_REVERB;
	}
	if (s_modplug_megabass)
	{
		settings.mFlags |= MODPLUG_ENABLE_MEGABASS;
	}
	else
	{
		settings.mFlags &= ~MODPLUG_ENABLE_MEGABASS;
	}
	if (s_modplug_surround)
	{
		settings.mFlags |= MODPLUG_ENABLE_SURROUND;
	}
	else
	{
		settings.mFlags &= ~MODPLUG_ENABLE_SURROUND;
	}
	if (s_modplug_hqmixer == 3)
	{
	    settings.mResamplingMode = MODPLUG_RESAMPLE_FIR;
	}
	else if (s_modplug_hqmixer == 2)
	{
	    settings.mResamplingMode = MODPLUG_RESAMPLE_SPLINE;
	}
	else if (s_modplug_hqmixer == 1)
	{
	    settings.mResamplingMode = MODPLUG_RESAMPLE_LINEAR;
	}
	else
	{
	    settings.mResamplingMode = MODPLUG_RESAMPLE_NEAREST;
	}

	ModPlug_SetSettings(&settings);

	//	Start playback.
	int Size = InStrm->TotalSize();
	void* Data = Z_Malloc(Size);
	InStrm->Seek(0);
	InStrm->Serialise(Data, Size);
	ModPlugFile* file = ModPlug_Load(Data, Size);
	Z_Free(Data);
	if (!file)
	{
		return NULL;
	}

	// Check the file type, we don't want to use ModPlug for
	// MIDI files, so if we have a MIDI file or an invalid
	//  file, we'll reject the Codec
	if (ModPlug_GetModuleType(file) == 0x00 /*MOD_TYPE_NONE*/ ||
		ModPlug_GetModuleType(file) == 0x10000 /*MOD_TYPE_MID*/)
	{
		return NULL;
	}

	if (ModPlug_GetModuleType(file) == 0x01 /*MOD_TYPE_MOD*/)
	{
		// The file is a MOD file, I've seen problems with some
		// GME type files being detected as MOD files by ModPlug,
		// so we have to make sure that it is really a MOD_TYPE
		// file by checking some of it's internal data here

		byte Hdr[4];
		InStrm->Seek(0);
		InStrm->Serialise(Hdr, 4);

		// Ignore files that aren't MOD_TYPE files
		if (memcmp(Hdr, "ZXAY", 4) || memcmp(Hdr, "GBS ", 4) ||
			memcmp(Hdr, "GYMX", 4) || memcmp(Hdr, "HESM", 4) ||
			memcmp(Hdr, "KSCC", 4) || memcmp(Hdr, "KSSX", 4) ||
			memcmp(Hdr, "NESM", 4) || memcmp(Hdr, "NSFE", 4) ||
			memcmp(Hdr, "SAP ", 4) || memcmp(Hdr, "SGC ", 4) ||
			memcmp(Hdr, "SNES", 4) || memcmp(Hdr, "Vgm ", 4))
		{
			// Incompatible file
			file = NULL;
			return NULL;
		}
	}

	//  Set master volume here, we'll set it to the maximum
	//  value (or near to it), it will be adjusted by the
	//  music volume settings
	ModPlug_SetMasterVolume(file, 512);
	InStrm->Close();
	delete InStrm;
	InStrm = NULL;

	return new VModPlugAudioCodec(file);
	unguard;
}
