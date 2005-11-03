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

#include <vorbis/codec.h>

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VVorbisAudioCodec : public VAudioCodec
{
public:
	int					InitLevel;
	FArchive*			Ar;
	int					BytesLeft;

	ogg_sync_state		oy;
	ogg_stream_state	os;
	vorbis_info			vi;
	vorbis_comment		vc;
	vorbis_dsp_state	vd;
	vorbis_block		vb;

	int					eos;

	VVorbisAudioCodec(FArchive* InAr);
	~VVorbisAudioCodec();
	bool Init();
	void Cleanup();
	int Decode(short* Data, int NumSamples);
	int ReadData();
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

IMPLEMENT_AUDIO_CODEC(VVorbisAudioCodec, "Vorbis");

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VVorbisAudioCodec::VVorbisAudioCodec
//
//==========================================================================

VVorbisAudioCodec::VVorbisAudioCodec(FArchive* InAr)
: Ar(InAr)
{
	guard(VVorbisAudioCodec::VVorbisAudioCodec);
	BytesLeft = Ar->TotalSize();
	Ar->Seek(0);

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

		Ar->Close();
		delete Ar;
		Ar = NULL;
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

	eos = 0;
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

	//	Parsed all three headers. Initialize the Vorbis decoder.
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
			eos = 1;
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
	Ar->Serialise(buffer, bytes);
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
	Ar->Seek(0);
	BytesLeft = Ar->TotalSize();
	Init();
	unguard;
}

//==========================================================================
//
//	VVorbisAudioCodec::Create
//
//==========================================================================

VAudioCodec* VVorbisAudioCodec::Create(FArchive* InAr)
{
	guard(VVorbisAudioCodec::Create);
	VVorbisAudioCodec* Codec = new VVorbisAudioCodec(InAr);
	if (!Codec->Init())
	{
		Codec->Cleanup();
		delete Codec;
		return NULL;
	}
	return Codec;
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2005/11/03 22:46:35  dj_jl
//	Support for any bitrate streams.
//
//	Revision 1.1  2005/10/18 20:53:04  dj_jl
//	Implemented basic support for streamed music.
//	
//**************************************************************************
