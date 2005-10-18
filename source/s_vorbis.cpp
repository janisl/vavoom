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

	ogg_sync_state		oy; /* sync and verify incoming physical bitstream */
	ogg_stream_state	os; /* take physical pages, weld into a logical
								stream of packets */
	ogg_page			og; /* one Ogg bitstream page.  Vorbis packets are inside */
	ogg_packet			op; /* one raw packet of data for decode */

	vorbis_info			vi; /* struct that stores all the static vorbis bitstream
								settings */
	vorbis_comment		vc; /* struct that stores all the bitstream user comments */
	vorbis_dsp_state	vd; /* central working state for the packet->PCM decoder */
	vorbis_block		vb; /* local working space for packet->PCM decode */

	int					bytes;

	int					eos;

	int					i;

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

IMPLEMENT_AUDIO_CODEC(VVorbisAudioCodec, "Vorbis");

// PRIVATE DATA DEFINITIONS ------------------------------------------------

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

	ogg_sync_init(&oy); /* Now we can read pages */
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
	/* OK, clean up the framer */
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
	eos = 0;
	InitLevel = 0;
	/********** Decode setup ************/
	/* grab some data at the head of the stream.  We want the first page
		(which is guaranteed to be small and only contain the Vorbis
		stream initial header) We need the first page to get the stream
		serialno. */

	/* submit a 4k block to libvorbis' Ogg layer */
	bytes = ReadData();

	/* Get the first page. */
	if (ogg_sync_pageout(&oy, &og) != 1)
	{
		/* error case.  Must not be Vorbis data */
		return false;
	}

	/* Get the serial number and set up the rest of decode. */
	/* serialno first; use it to set up a logical stream */
	ogg_stream_init(&os, ogg_page_serialno(&og));

	/* extract the initial header from the first page and verify that the
		Ogg bitstream is in fact Vorbis data */

	/* I handle the initial header first instead of just having the code
	read all three Vorbis headers at once because reading the initial
	header is an easy way to identify a Vorbis bitstream and it's
	useful to see that functionality seperated out. */

	vorbis_info_init(&vi);
	vorbis_comment_init(&vc);
	InitLevel = 1;
	if (ogg_stream_pagein(&os, &og) < 0)
	{ 
		/* error; stream version mismatch perhaps */
		return false;
	}

	if (ogg_stream_packetout(&os, &op) != 1)
	{
		/* no page? must not be vorbis */
		return false;
	}

	if (vorbis_synthesis_headerin(&vi, &vc, &op) < 0)
	{
		/* error case; not a vorbis header */
		//fprintf(stderr,"This Ogg bitstream does not contain Vorbis "
		//	"audio data.\n");
		return false;
	}

	/* At this point, we're sure we're Vorbis.  We've set up the logical
		(Ogg) bitstream decoder.  Get the comment and codebook headers and
		set up the Vorbis decoder */

	/* The next two packets in order are the comment and codebook headers.
		They're likely large and may span multiple pages.  Thus we reead
		and submit data until we get our two pacakets, watching that no
		pages are missing.  If a page is missing, error out; losing a
		header page is the only place where missing data is fatal. */

	i = 0;
	while (i < 2)
	{
		while(i < 2)
		{
			int result = ogg_sync_pageout(&oy, &og);
			if (result == 0)
				break; /* Need more data */
			/* Don't complain about missing or corrupt data yet.  We'll
			catch it at the packet output phase */
			if (result == 1)
			{
				ogg_stream_pagein(&os, &og); /* we can ignore any errors here
							as they'll also become apparent
							at packetout */
				while (i < 2)
				{
					result = ogg_stream_packetout(&os, &op);
					if (result == 0)
						break;
					if (result < 0)
					{
						/* Uh oh; data at some point was corrupted or missing!
						We can't tolerate that in a header.  Die. */
						//fprintf(stderr,"Corrupt secondary header.  Exiting.\n");
						return false;
					}
					vorbis_synthesis_headerin(&vi, &vc, &op);
					i++;
				}
			}
		}
		/* no harm in not checking before adding more */
		bytes = ReadData();
		if (bytes == 0 && i < 2)
		{
			//fprintf(stderr,"End of file before finding all Vorbis headers!\n");
			return false;
		}
	}

	/* Throw the comments plus a few lines about the bitstream we're
	decoding */
	{
		char **ptr = vc.user_comments;
		while (*ptr)
		{
			GCon->Log(*ptr);
			++ptr;
		}
		GCon->Logf("Bitstream is %d channel, %ldHz", vi.channels, vi.rate);
		GCon->Logf("Encoded by: %s", vc.vendor);
	}

	/* OK, got and parsed all three headers. Initialize the Vorbis
	packet->PCM decoder. */
	vorbis_synthesis_init(&vd, &vi); /* central decode state */
	vorbis_block_init(&vd, &vb);     /* local state for most of the decode
										so multiple block decodes can
										proceed in parallel.  We could init
										multiple vorbis_block structures
										for vd here */
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
	/* clean up this logical bitstream; before exit we see if we're
	followed by another [chained] */

	if (InitLevel > 0)
	{
		ogg_stream_clear(&os);
	}

	/* ogg_page and ogg_packet structs always point to storage in
	libvorbis.  They're never freed or manipulated directly */

	if (InitLevel > 1)
	{
		vorbis_block_clear(&vb);
		vorbis_dsp_clear(&vd);
	}
	if (InitLevel > 0)
	{
		vorbis_comment_clear(&vc);
		vorbis_info_clear(&vi);  /* must be called last */
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
	int CurSample = 0;
	/* The rest is just a straight decode loop until end of stream */
	while (!eos)
	{
		while (!eos)
		{
			int result = ogg_sync_pageout(&oy, &og);
			if (result == 0)
				break; /* need more data */
			if (result < 0)
			{
				/* missing or corrupt data at this page position */
				//fprintf(stderr,"Corrupt or missing data in bitstream; "
				//	"continuing...\n");
			}
			else
			{
				ogg_stream_pagein(&os, &og); /* can safely ignore errors at
												this point */
				while (1)
				{
					result = ogg_stream_packetout(&os, &op);

					if (result == 0)
						break; /* need more data */
					if (result < 0)
					{
						/* missing or corrupt data at this page position */
						/* no reason to complain; already complained above */
					}
					else
					{
						/* we have a packet.  Decode it */
						float **pcm;
						int samples;

						if (vorbis_synthesis(&vb, &op) == 0) /* test for success! */
							vorbis_synthesis_blockin(&vd, &vb);
						/* 
						
						**pcm is a multichannel float vector.  In stereo, for
						example, pcm[0] is left, and pcm[1] is right.  samples is
						the size of each channel.  Convert the float values
						(-1.<=range<=1.) to whatever PCM format and write it out */

						while ((samples = vorbis_synthesis_pcmout(&vd, &pcm)) > 0)
						{
							int j;
							int clipflag = 0;
							int bout = (CurSample + samples < NumSamples ? samples : NumSamples - CurSample);

							/* convert floats to 16 bit signed ints (host order) and
							interleave */
							for (i = 0; i < 2; i++)
							{
								short* ptr = Data + CurSample * 2 + (vi.channels > 1 ? i : 0);
								float *mono = pcm[vi.channels > 1 ? i : 0];
								for (j = 0; j < bout; j++)
								{
									int val = int(mono[j] * 32767.f);
									/* might as well guard against clipping */
									if (val > 32767)
									{
										val = 32767;
										clipflag = 1;
									}
									if (val < -32768)
									{
										val = -32768;
										clipflag = 1;
									}
									*ptr = val;
									ptr += 2;
								}
							}

							//if (clipflag)
							//	fprintf(stderr,"Clipping in frame %ld\n",(long)(vd.sequence));

							vorbis_synthesis_read(&vd, bout); /* tell libvorbis how
																many samples we
																actually consumed */
							CurSample += bout;
							if (CurSample == NumSamples)
								return CurSample;
						}
					}
				}
				if (ogg_page_eos(&og))
					eos = 1;
			}
		}
		if (!eos)
		{
			bytes = ReadData();
			if (bytes == 0)
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
//	Revision 1.1  2005/10/18 20:53:04  dj_jl
//	Implemented basic support for streamed music.
//
//**************************************************************************
