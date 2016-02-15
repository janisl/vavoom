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

#include <vorbis/codec.h>

#include "gamedefs.h"
#include "snd_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VVorbisAudioCodec : public VAudioCodec
{
public:
	int					InitLevel;
	VStream*			Strm;
	bool				FreeStream;
	int					BytesLeft;

	ogg_sync_state		oy;
	ogg_stream_state	os;
	vorbis_info			vi;
	vorbis_comment		vc;
	vorbis_dsp_state	vd;
	vorbis_block		vb;

	bool				eos;

	VVorbisAudioCodec(VStream*, bool);
	~VVorbisAudioCodec();
	bool Init();
	void Cleanup();
	int Decode(short*, int);
	int ReadData();
	bool Finished();
	void Restart();
	static VAudioCodec* Create(VStream*);
};

class VVorbisSampleLoader : public VSampleLoader
{
public:
	void Load(sfxinfo_t&, VStream&);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_AUDIO_CODEC(VVorbisAudioCodec, "Vorbis");

VVorbisSampleLoader		VorbisSampleLoader;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VVorbisAudioCodec::VVorbisAudioCodec
//
//==========================================================================

VVorbisAudioCodec::VVorbisAudioCodec(VStream* AStrm, bool AFreeStream)
: Strm(AStrm)
, FreeStream(AFreeStream)
{
	guard(VVorbisAudioCodec::VVorbisAudioCodec);
	BytesLeft = Strm->TotalSize();
	Strm->Seek(0);

	ogg_sync_init(&oy);
	unguard;
}

//==========================================================================
//
//	VVorbisAudioCodec::~VVorbisAudioCodec
//
//==========================================================================

VVorbisAudioCodec::~VVorbisAudioCodec()
{
	guard(VVorbisAudioCodec::~VVorbisAudioCodec);
	if (InitLevel > 0)
	{
		Cleanup();

		if (FreeStream)
		{
			Strm->Close();
			delete Strm;
		}
		Strm = NULL;
	}
	ogg_sync_clear(&oy);
	unguard;
}

//==========================================================================
//
//	VVorbisAudioCodec::Init
//
//==========================================================================

bool VVorbisAudioCodec::Init()
{
	guard(VVorbisAudioCodec::Init);
	ogg_page			og;
	ogg_packet			op;

	eos = false;
	InitLevel = 0;

	//	Read some data.
	ReadData();

	//	Get the first page.
	if (ogg_sync_pageout(&oy, &og) != 1)
	{
		//	Not a Vorbis file.
		return false;
	}

	ogg_stream_init(&os, ogg_page_serialno(&og));
	vorbis_info_init(&vi);
	vorbis_comment_init(&vc);
	InitLevel = 1;

	if (ogg_stream_pagein(&os, &og) < 0)
	{
		//	Stream version mismatch perhaps.
		return false;
	}

	if (ogg_stream_packetout(&os, &op) != 1)
	{
		//	No page? Must not be vorbis.
		return false;
	}

	if (vorbis_synthesis_headerin(&vi, &vc, &op) < 0)
	{
		//	Not a vorbis header.
		return false;
	}

	//	We need 2 more headers.
	int i = 0;
	while (i < 2)
	{
		int result = ogg_stream_packetout(&os, &op);
		if (result < 0)
		{
			//	Corrupt header.
			return false;
		}
		if (result > 0)
		{
			if (vorbis_synthesis_headerin(&vi, &vc, &op))
			{
				//	Corrupt header.
				return false;
			}
			i++;
		}
		else if (ogg_sync_pageout(&oy, &og) > 0)
		{
			ogg_stream_pagein(&os, &og);
		}
		else if (ReadData() == 0 && i < 2)
		{
			//	Out of data while reading headers.
			return false;
		}
	}

	//	Parsed all three headers. Initialise the Vorbis decoder.
	vorbis_synthesis_init(&vd, &vi);
	vorbis_block_init(&vd, &vb);
	SampleRate = vi.rate;
	InitLevel = 2;
	return true;
	unguard;
}

//==========================================================================
//
//	VVorbisAudioCodec::Cleanup
//
//==========================================================================

void VVorbisAudioCodec::Cleanup()
{
	guard(VVorbisAudioCodec::Cleanup);
	if (InitLevel > 0)
	{
		ogg_stream_clear(&os);
	}
	if (InitLevel > 1)
	{
		vorbis_block_clear(&vb);
		vorbis_dsp_clear(&vd);
	}
	if (InitLevel > 0)
	{
		vorbis_comment_clear(&vc);
		vorbis_info_clear(&vi);
	}
	InitLevel = 0;
	unguard;
}

//==========================================================================
//
//	VVorbisAudioCodec::Decode
//
//==========================================================================

int VVorbisAudioCodec::Decode(short* Data, int NumSamples)
{
	guard(VVorbisAudioCodec::Decode);
	ogg_page			og;
	ogg_packet			op;
	int					CurSample = 0;

	while (!eos)
	{
		//	While we have data ready, read it.
		float** pcm;
		int samples = vorbis_synthesis_pcmout(&vd, &pcm);
		if (samples > 0)
		{
			int bout = (CurSample + samples < NumSamples ? samples : NumSamples - CurSample);
			for (int i = 0; i < 2; i++)
			{
				short* ptr = Data + CurSample * 2 + (vi.channels > 1 ? i : 0);
				float* mono = pcm[vi.channels > 1 ? i : 0];
				for (int j = 0; j < bout; j++)
				{
					int val = int(mono[j] * 32767.f);
					//	clipping
					if (val > 32767)
					{
						val = 32767;
					}
					if (val < -32768)
					{
						val = -32768;
					}
					*ptr = val;
					ptr += 2;
				}
			}
			//	Tell libvorbis how many samples we actually consumed
			vorbis_synthesis_read(&vd, bout);
			CurSample += bout;
			if (CurSample == NumSamples)
				return CurSample;
		}

		if (ogg_stream_packetout(&os, &op) > 0)
		{
			//	We have a packet.  Decode it.
			if (vorbis_synthesis(&vb, &op) == 0)
				vorbis_synthesis_blockin(&vd, &vb);
		}
		else if (ogg_sync_pageout(&oy, &og) > 0)
		{
			ogg_stream_pagein(&os, &og);
		}
		else if (ReadData() == 0)
		{
			eos = true;
		}
	}
	return CurSample;
	unguard;
}

//==========================================================================
//
//	VVorbisAudioCodec::ReadData
//
//==========================================================================

int VVorbisAudioCodec::ReadData()
{
	guard(VVorbisAudioCodec::ReadData);
	if (!BytesLeft)
		return 0;
	char* buffer = ogg_sync_buffer(&oy, 4096);
	int bytes = 4096;
	if (bytes > BytesLeft)
		bytes = BytesLeft;
	Strm->Serialise(buffer, bytes);
	ogg_sync_wrote(&oy, bytes);
	BytesLeft -= bytes;
	return bytes;
	unguard;
}

//==========================================================================
//
//	VVorbisAudioCodec::Finished
//
//==========================================================================

bool VVorbisAudioCodec::Finished()
{
	guard(VVorbisAudioCodec::Finished);
	return eos;
	unguard;
}

//==========================================================================
//
//	VVorbisAudioCodec::Restart
//
//==========================================================================

void VVorbisAudioCodec::Restart()
{
	guard(VVorbisAudioCodec::Restart);
	Cleanup();
	Strm->Seek(0);
	BytesLeft = Strm->TotalSize();
	Init();
	unguard;
}

//==========================================================================
//
//	VVorbisAudioCodec::Create
//
//==========================================================================

VAudioCodec* VVorbisAudioCodec::Create(VStream* InStrm)
{
	guard(VVorbisAudioCodec::Create);
	VVorbisAudioCodec* Codec = new VVorbisAudioCodec(InStrm, true);
	if (!Codec->Init())
	{
		Codec->Cleanup();
		delete Codec;
		Codec = NULL;
		return NULL;
	}
	return Codec;
	unguard;
}

//==========================================================================
//
//	VVorbisAudioCodec::Create
//
//==========================================================================

void VVorbisSampleLoader::Load(sfxinfo_t& Sfx, VStream& Stream)
{
	guard(VVorbisSampleLoader::Load);
	VVorbisAudioCodec* Codec = new VVorbisAudioCodec(&Stream, false);
	if (!Codec->Init())
	{
		Codec->Cleanup();
		delete Codec;
		Codec = NULL;
		return;
	}

	TArray<short> Data;
	do
	{
		short Buf[16 * 2048];
		int SamplesDecoded = Codec->Decode(Buf, 16 * 1024);
		if (SamplesDecoded > 0)
		{
			int OldPos = Data.Num();
			Data.SetNumWithReserve(Data.Num() + SamplesDecoded);
			for (int i = 0; i < SamplesDecoded; i++)
			{
				Data[OldPos + i] = Buf[i * 2];
			}
		}
	}
	while (!Codec->Finished());
	if (!Data.Num())
	{
		delete Codec;
		Codec = NULL;
		return;
	}

	//	Copy parameters.
	Sfx.SampleRate = Codec->SampleRate;
	Sfx.SampleBits = Codec->SampleBits;

	//	Copy data.
	Sfx.DataSize = Data.Num() * 2;
	Sfx.Data = Z_Malloc(Data.Num() * 2);
	memcpy(Sfx.Data, Data.Ptr(), Data.Num() * 2);

	delete Codec;
	Codec = NULL;
	unguard;
}
