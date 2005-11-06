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

#pragma pack(1)
struct FRiffChunkHeader
{
	char		ID[4];
	dword		Size;
};

struct FWavFormatDesc
{
	word		Format;
	word		Channels;
	dword		Rate;
	dword		BytesPerSec;
	word		BlockAlign;
	word		Bits;
};
#pragma pack()

class VWaveSampleLoader : public VSampleLoader
{
public:
	void Load(sfxinfo_t&, FArchive&);
};

class VWavAudioCodec : public VAudioCodec
{
public:
	FArchive*		Ar;
	int				SamplesLeft;

	int				WavChannels;
	int				WavBits;
	int				BlockAlign;

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

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VWaveSampleLoader		WaveSampleLoader;

IMPLEMENT_AUDIO_CODEC(VWavAudioCodec, "Wav");

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	FindChunk
//
//==========================================================================

static int FindRiffChunk(FArchive& Ar, char* ID)
{
	guard(VWavAudioCodec::FindChunk);
	Ar.Seek(12);
	int EndPos = Ar.TotalSize();
	while (Ar.Tell() + 8 <= EndPos)
	{
		FRiffChunkHeader ChunkHdr;
		Ar.Serialise(&ChunkHdr, 8);
		int ChunkSize = LittleLong(ChunkHdr.Size);
		if (!memcmp(ChunkHdr.ID, ID, 4))
		{
			//	Found chunk.
			return ChunkSize;
		}
		if (Ar.Tell() + ChunkSize > EndPos)
		{
			//	Chunk goes beyound end of file.
			break;
		}
		Ar.Seek(Ar.Tell() + ChunkSize);
	}
	return -1;
	unguard;
}

//==========================================================================
//
//	VWaveSampleLoader::Load
//
//==========================================================================

void VWaveSampleLoader::Load(sfxinfo_t& Sfx, FArchive& Ar)
{
	guard(VWaveSampleLoader::Load);
	//	Check header to see if it's a wave file.
	char Header[12];
	Ar.Seek(0);
	Ar.Serialise(Header, 12);
	if (memcmp(Header, "RIFF", 4) || memcmp(Header + 8, "WAVE", 4))
	{
		//	Not a WAVE.
		return;
	}

	//	Get format settings.
	int FmtSize = FindRiffChunk(Ar, "fmt ");
	if (FmtSize < 16)
	{
		//	Format not found or too small.
		return;
	}
	FWavFormatDesc Fmt;
	Ar.Serialise(&Fmt, 16);
	if (LittleShort(Fmt.Format) != 1)
	{
		//	Not a PCM format.
		return;
	}
	int SampleRate = LittleLong(Fmt.Rate);
	int WavChannels = LittleShort(Fmt.Channels);
	int WavBits = LittleShort(Fmt.Bits);
	int BlockAlign = LittleShort(Fmt.BlockAlign);
	if (WavChannels != 1)
	{
		GCon->Logf("A stereo sample, taking left channel");
	}

	//	Find data chunk.
	int DataSize = FindRiffChunk(Ar, "data");
	if (DataSize == -1)
	{
		//	Data not found
		return;
	}

	//	Fill in sample info and allocate data.
	Sfx.SampleRate = SampleRate;
	Sfx.SampleBits = WavBits;
	Sfx.DataSize = (DataSize / BlockAlign) * (WavBits/ 8);
	Sfx.Data = Z_Malloc(Sfx.DataSize, PU_SOUND, &Sfx.Data);

	//	Read wav data.
	void* WavData = Z_Malloc(DataSize);
	Ar.Serialise(WavData, DataSize);

	//	Copy sample data.
	DataSize /= BlockAlign;
	if (WavBits == 8)
	{
		byte* pSrc = (byte*)WavData;
		byte* pDst = (byte*)Sfx.Data;
		for (int i = 0; i < DataSize; i++, pSrc += BlockAlign, pDst++)
		{
			*pDst = *pSrc;
		}
	}
	else
	{
		byte* pSrc = (byte*)WavData;
		short* pDst = (short*)Sfx.Data;
		for (int i = 0; i < DataSize; i++, pSrc += BlockAlign, pDst++)
		{
			*pDst = LittleShort(*(short*)pSrc);
		}
	}
	Z_Free(WavData);

	unguard;
}

//==========================================================================
//
//	VWavAudioCodec::VWavAudioCodec
//
//==========================================================================

VWavAudioCodec::VWavAudioCodec(FArchive* InAr)
: Ar(InAr)
, SamplesLeft(-1)
{
	guard(VWavAudioCodec::VWavAudioCodec);
	int FmtSize = FindRiffChunk(*Ar, "fmt ");
	if (FmtSize < 16)
	{
		//	Format not found or too small.
		return;
	}
	FWavFormatDesc Fmt;
	Ar->Serialise(&Fmt, 16);
	if (LittleShort(Fmt.Format) != 1)
	{
		//	Not a PCM format.
		return;
	}
	SampleRate = LittleLong(Fmt.Rate);
	WavChannels = LittleShort(Fmt.Channels);
	WavBits = LittleShort(Fmt.Bits);
	BlockAlign = LittleShort(Fmt.BlockAlign);

	SamplesLeft = FindRiffChunk(*Ar, "data");
	if (SamplesLeft == -1)
	{
		//	Data not found
		return;
	}
	SamplesLeft /= BlockAlign;
	unguard;
}

//==========================================================================
//
//	VWavAudioCodec::~VWavAudioCodec
//
//==========================================================================

VWavAudioCodec::~VWavAudioCodec()
{
	guard(VWavAudioCodec::~VWavAudioCodec);
	if (SamplesLeft != -1)
	{
		Ar->Close();
		delete Ar;
		Ar = NULL;
	}
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
	int CurSample = 0;
	byte Buf[1024];
	while (SamplesLeft && CurSample < NumSamples)
	{
		int ReadSamples = 1024 / BlockAlign;
		if (ReadSamples > NumSamples - CurSample)
			ReadSamples = NumSamples - CurSample;
		if (ReadSamples > SamplesLeft)
			ReadSamples = SamplesLeft;
		Ar->Serialise(Buf, ReadSamples * BlockAlign);
		for (int i = 0; i < 2; i++)
		{
			byte* pSrc = Buf;
			if (i && WavChannels > 1)
				pSrc += WavBits / 8;
			short* pDst = Data + CurSample * 2 + i;
			if (WavBits == 8)
			{
				for (int j = 0; j < ReadSamples; j++, pSrc += BlockAlign, pDst += 2)
				{
					*pDst = (*pSrc - 127) << 8;
				}
			}
			else
			{
				for (int j = 0; j < ReadSamples; j++, pSrc += BlockAlign, pDst += 2)
				{
					*pDst = LittleShort(*(short*)pSrc);
				}
			}
		}
		SamplesLeft -= ReadSamples;
		CurSample += ReadSamples;
	}
	return CurSample;
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
	SamplesLeft = FindRiffChunk(*Ar, "data") / BlockAlign;
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
		//	It's a WAVE file.
		VWavAudioCodec* Codec = new VWavAudioCodec(InAr);
		if (Codec->SamplesLeft != -1)
		{
			return Codec;
		}
		//	File seams to be broken.
		delete Codec;
	}
	return NULL;
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2005/11/06 15:27:46  dj_jl
//	Added support for wave format sounds.
//
//	Revision 1.2  2005/11/03 23:59:15  dj_jl
//	Properly implemented wave reading.
//	
//	Revision 1.1  2005/10/18 20:53:04  dj_jl
//	Implemented basic support for streamed music.
//	
//**************************************************************************
