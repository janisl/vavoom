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

#include <mad.h>

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VMp3AudioCodec : public VAudioCodec
{
public:
	enum { INPUT_BUFFER_SIZE = 5 * 8192 };

	VStream*			Strm;
	bool				FreeStream;
	int					BytesLeft;
	bool				Initialised;

	mad_stream			Stream;
	mad_frame			Frame;
	mad_synth			Synth;
	byte				InputBuffer[INPUT_BUFFER_SIZE + MAD_BUFFER_GUARD];
	int					FramePos;
	bool				HaveFrame;

	VMp3AudioCodec(VStream*, bool);
	~VMp3AudioCodec();
	bool Init();
	int Decode(short*, int);
	int ReadData();
	bool Finished();
	void Restart();
	static VAudioCodec* Create(VStream*);
};

class VMp3SampleLoader : public VSampleLoader
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

IMPLEMENT_AUDIO_CODEC(VMp3AudioCodec, "MP3");

VMp3SampleLoader		Mp3SampleLoader;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VMp3AudioCodec::VMp3AudioCodec
//
//==========================================================================

VMp3AudioCodec::VMp3AudioCodec(VStream* AStrm, bool AFreeStream)
: Strm(AStrm)
, FreeStream(AFreeStream)
, Initialised(false)
{
	guard(VMp3AudioCodec::VMp3AudioCodec);
	//	Seek file to the begining
	BytesLeft = Strm->TotalSize();
	Strm->Seek(0);

	//	Initialise structures used by libmad
	mad_stream_init(&Stream);
	mad_frame_init(&Frame);
	mad_synth_init(&Synth);
	unguard;
}

//==========================================================================
//
//	VMp3AudioCodec::~VMp3AudioCodec
//
//==========================================================================

VMp3AudioCodec::~VMp3AudioCodec()
{
	guard(VMp3AudioCodec::~VMp3AudioCodec);
	if (Initialised)
	{
		//	Close file only if decoder has been initialised succesfully.
		if (FreeStream)
		{
			Strm->Close();
			delete Strm;
		}
		Strm = NULL;
	}
	//	Clear structs used by libmad.
	mad_synth_finish(&Synth);
	mad_frame_finish(&Frame);
	mad_stream_finish(&Stream);
	unguard;
}

//==========================================================================
//
//	VMp3AudioCodec::Init
//
//==========================================================================

bool VMp3AudioCodec::Init()
{
	guard(VMp3AudioCodec::Init);

	//	Check for ID3v2 header.
	byte Id3Hdr[10];
	int SavedPos = Strm->Tell();
	Strm->Serialise(Id3Hdr, 10);
	if (Id3Hdr[0] == 'I' && Id3Hdr[1] == 'D' && Id3Hdr[2] == '3')
	{
		//	It's a ID3v3 header, skip it.
		int HdrSize = Id3Hdr[9] + (Id3Hdr[8] << 7) + (Id3Hdr[7] << 14) +
			(Id3Hdr[6] << 21);
		if (HdrSize + 10 > BytesLeft)
			return false;
		Strm->Seek(Strm->Tell() + HdrSize);
		BytesLeft -= 10 + HdrSize;
	}
	else
	{
		//	Not a ID3v3 header, seek back to saved position.
		Strm->Seek(SavedPos);
	}

	//	Read some data.
	ReadData();

	//	Decode first frame. If this fails we assume it's not a MP3 file.
	if (mad_frame_decode(&Frame, &Stream))
	{
		//	Not a valid stream.
		return false;
	}

	//	We are ready to read data.
	mad_synth_frame(&Synth, &Frame);
	FramePos = 0;
	HaveFrame = true;

	//	Everything's OK.
	SampleRate = Frame.header.samplerate;
	Initialised = true;
	return true;
	unguard;
}

//==========================================================================
//
//	VMp3AudioCodec::Decode
//
//==========================================================================

int VMp3AudioCodec::Decode(short* Data, int NumSamples)
{
	guard(VMp3AudioCodec::Decode);
	int CurSample = 0;
	do
	{
		if (HaveFrame)
		{
			//	Convert stream from fixed point to short.
			for (; FramePos < Synth.pcm.length; FramePos++)
			{
				//	Left channel
				short Sample;
				mad_fixed_t Fixed = Synth.pcm.samples[0][FramePos];
				if (Fixed >= MAD_F_ONE)
					Sample = 0x7fff;
				else if (Fixed <= -MAD_F_ONE)
					Sample = -0x7fff;
				else
					Sample = Fixed >> (MAD_F_FRACBITS - 15);
				Data[CurSample * 2] = Sample;

				//	Right channel. If the decoded stream is monophonic then
				// the right output channel is the same as the left one.
				if (MAD_NCHANNELS(&Frame.header) == 2)
				{
					Fixed = Synth.pcm.samples[1][FramePos];
					if (Fixed >= MAD_F_ONE)
						Sample = 0x7fff;
					else if (Fixed <= -MAD_F_ONE)
						Sample = -0x7fff;
					else
						Sample = Fixed >> (MAD_F_FRACBITS - 15);
				}
				Data[CurSample * 2 + 1] = Sample;
				CurSample++;
				//	Check if we already have decoded enough.
				if (CurSample >= NumSamples)
					return CurSample;
			}
			//	We are done with the frame.
			HaveFrame = false;
		}

		//	Fill in input buffer if it becomes empty.
		if (Stream.buffer == NULL || Stream.error == MAD_ERROR_BUFLEN)
		{
			if (!ReadData())
				break;
		}

		//	Decode the next frame. 
		if (mad_frame_decode(&Frame, &Stream))
		{
			if (MAD_RECOVERABLE(Stream.error) ||
				Stream.error==MAD_ERROR_BUFLEN)
			{
				continue;
			}
			else
			{
				break;
			}
		}

		//	Once decoded the frame is synthesized to PCM samples.
		mad_synth_frame(&Synth, &Frame);
		FramePos = 0;
		HaveFrame = true;
	} while(1);
	return CurSample;
	unguard;
}

//==========================================================================
//
//	VMp3AudioCodec::ReadData
//
//==========================================================================

int VMp3AudioCodec::ReadData()
{
	guard(VMp3AudioCodec::ReadData);
	int			ReadSize;
	int			Remaining;
	byte*		ReadStart;

	//	If there are some bytes left, move them to the beginning of the
	// buffer.
	if (Stream.next_frame != NULL)
	{
		Remaining = Stream.bufend - Stream.next_frame;
		memmove(InputBuffer, Stream.next_frame, Remaining);
		ReadStart = InputBuffer + Remaining;
		ReadSize = INPUT_BUFFER_SIZE - Remaining;
	}
	else
	{
		ReadSize = INPUT_BUFFER_SIZE;
		ReadStart = InputBuffer;
		Remaining = 0;
	}
	//	Fill-in the buffer.
	if (ReadSize > BytesLeft)
		ReadSize = BytesLeft;
	if (!ReadSize)
		return 0;
	Strm->Serialise(ReadStart, ReadSize);
	BytesLeft -= ReadSize;

	//	When decoding the last frame of a file, it must be followed by 
	// MAD_BUFFER_GUARD zero bytes if one wants to decode that last frame.
	if (!BytesLeft)
	{
		memset(ReadStart + ReadSize + Remaining, 0, MAD_BUFFER_GUARD);
		ReadSize += MAD_BUFFER_GUARD;
	}

	//	Pipe the new buffer content to libmad's stream decoder facility.
	mad_stream_buffer(&Stream, InputBuffer, ReadSize + Remaining);
	Stream.error = MAD_ERROR_NONE;
	return ReadSize + Remaining;
	unguard;
}

//==========================================================================
//
//	VMp3AudioCodec::Finished
//
//==========================================================================

bool VMp3AudioCodec::Finished()
{
	guard(VMp3AudioCodec::Finished);
	//	We are done if there's no more data and last frame has been decoded.
	return !BytesLeft && !HaveFrame;
	unguard;
}

//==========================================================================
//
//	VMp3AudioCodec::Restart
//
//==========================================================================

void VMp3AudioCodec::Restart()
{
	guard(VMp3AudioCodec::Restart);
	//	Seek to the beginning of the file.
	Strm->Seek(0);
	BytesLeft = Strm->TotalSize();
	unguard;
}

//==========================================================================
//
//	VMp3AudioCodec::Create
//
//==========================================================================

VAudioCodec* VMp3AudioCodec::Create(VStream* InStrm)
{
	guard(VMp3AudioCodec::Create);
	VMp3AudioCodec* Codec = new VMp3AudioCodec(InStrm, true);
	if (!Codec->Init())
	{
		delete Codec;
		return NULL;
	}
	return Codec;
	unguard;
}

//==========================================================================
//
//	VMp3SampleLoader::Create
//
//==========================================================================

void VMp3SampleLoader::Load(sfxinfo_t& Sfx, VStream& Stream)
{
	guard(VMp3SampleLoader::Load);
	VMp3AudioCodec* Codec = new VMp3AudioCodec(&Stream, false);
	if (!Codec->Init())
	{
		delete Codec;
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
			Data.SetNum(Data.Num() + SamplesDecoded);
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
		return;
	}

	//	Copy parameters.
	Sfx.SampleRate = Codec->SampleRate;
	Sfx.SampleBits = Codec->SampleBits;

	//	Copy data.
	Sfx.DataSize = Data.Num() * 2;
	Sfx.Data = Z_Malloc(Data.Num() * 2);
	memcpy(Sfx.Data, &Data[0], Data.Num() * 2);

	delete Codec;
	unguard;
}
