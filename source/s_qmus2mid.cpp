//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: template.cpp,v 1.4 2002/05/18 16:56:35 dj_jl Exp $
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
//
//  Quick MUS->MID ! by S.Bacquet
//
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

const vuint8		VQMus2Mid::Mus2MidControl[15] =
{
	0,				//	Program change - not a MIDI control change
	0x00,			//	Bank select
	0x01,			//	Modulation pot
	0x07,			//	Volume
	0x0A,			//	Pan pot
	0x0B,			//	Expression pot
	0x5B,			//	Reverb depth
	0x5D,			//	Chorus depth
	0x40,			//	Sustain pedal
	0x43,			//	Soft pedal
	0x78,			//	All sounds off
	0x7B,			//	All notes off
	0x7E,			//	Mono
	0x7F,			//	Poly
	0x79			//	Reset all controllers
};
const vuint8		VQMus2Mid::TrackEnd[] =
{
	0x00, 0xff, 47, 0x00
};
const vuint8		VQMus2Mid::MidiKey[] =
{
	0x00, 0xff, 0x59, 0x02, 0x00, 0x00   		// C major
};
const vuint8		VQMus2Mid::MidiTempo[] =
{
	0x00, 0xff, 0x51, 0x03, 0x09, 0xa3, 0x1a	// uS/qnote
};

//==========================================================================
//
//	VQMus2Mid::FirstChannelAvailable
//
//==========================================================================

int VQMus2Mid::FirstChannelAvailable()
{
	guard(VQMus2Mid::FirstChannelAvailable);
	int 	old15 = Mus2MidChannel[15];
	int		max = -1;

	Mus2MidChannel[15] = -1;
	for (int i = 0; i < 16; i++)
	{
		if (Mus2MidChannel[i] > max)
		{
			max = Mus2MidChannel[i];
		}
	}
	Mus2MidChannel[15] = old15;

	return (max == 8 ? 10 : max + 1);
	unguard;
}

//==========================================================================
//
//	VQMus2Mid::TWriteByte
//
//==========================================================================

void VQMus2Mid::TWriteByte(int MIDItrack, vuint8 data)
{
	guard(VQMus2Mid::TWriteByte);
	Tracks[MIDItrack].Data.Append(data);
	unguard;
}

//==========================================================================
//
//	VQMus2Mid::TWriteBuf
//
//==========================================================================

void VQMus2Mid::TWriteBuf(int MIDItrack, const vuint8* buf, int size)
{
	guard(VQMus2Mid::TWriteBuf);
	for (int i = 0; i < size; i++)
	{
		TWriteByte(MIDItrack, buf[i]);
	}
	unguard;
}

//==========================================================================
//
//	VQMus2Mid::TWriteVarLen
//
//==========================================================================

void VQMus2Mid::TWriteVarLen(int tracknum, vuint32 value)
{
	guard(VQMus2Mid::TWriteVarLen);
	vuint32 buffer = value & 0x7f;
	while ((value >>= 7))
	{
		buffer <<= 8;
		buffer |= 0x80;
		buffer += (value & 0x7f);
	}
	while (1)
	{
		TWriteByte(tracknum, buffer);
		if (buffer & 0x80)
			buffer >>= 8;
		else
			break;
	}
	unguard;
}

//==========================================================================
//
//	VQMus2Mid::ReadTime
//
//==========================================================================

vuint32 VQMus2Mid::ReadTime(VStream& Strm)
{
	guard(VQMus2Mid::ReadTime);
	vuint32		time = 0;
	vuint8		data;

	if (Strm.AtEnd())
		return 0;
	do
	{
		Strm << data;
		time = (time << 7) + (data & 0x7F);
	} while (!Strm.AtEnd() && (data & 0x80));

	return time;
	unguard;
}

//==========================================================================
//
//  VQMus2Mid::Convert
//
//==========================================================================

bool VQMus2Mid::Convert(VStream& Strm)
{
	guard(VQMus2Mid::Convert);
	vuint8				et;
	int					MUSchannel;
	int					MIDIchannel;
	int					MIDItrack = 0;
	int					NewEvent;
	int 				i;
	vuint8				event;
	vuint8				data;
	vuint32				DeltaTime;
	vuint8				MIDIchan2track[16];
	bool 				ouch = false;
	FMusHeader			MUSh;

	for (i = 0; i < 16; i++)
	{
		Mus2MidChannel[i] = -1;
	}
	for (i = 0; i < 32; i++)
	{
		Tracks[i].DeltaTime = 0;
		Tracks[i].LastEvent = 0;
		Tracks[i].Vel = 64;
		Tracks[i].Data.Clear();
	}

	Strm.Serialise(&MUSh, sizeof(FMusHeader));
	if (VStr::NCmp(MUSh.ID, MUSMAGIC, 4))
	{
		GCon->Log("Not a MUS file");
		return false;
	}

	if ((vuint16)LittleShort(MUSh.NumChannels) > 15)	 /* <=> MUSchannels+drums > 16 */
	{
		GCon->Log(NAME_Dev,"Too many channels");
		return false;
	}

	Strm.Seek((vuint16)LittleShort(MUSh.ScoreStart));

	TWriteBuf(0, MidiKey, 6);
	TWriteBuf(0, MidiTempo, 7);

	TrackCnt = 1;	//	Music starts here

	Strm << event;
	et = (event & 0x70) >> 4;
	MUSchannel = event & 0x0f;
	while ((et != 6) && !Strm.AtEnd())
	{
		if (Mus2MidChannel[MUSchannel] == -1)
		{
			MIDIchannel = Mus2MidChannel[MUSchannel] =
				(MUSchannel == 15 ? 9 : FirstChannelAvailable());
			MIDItrack = MIDIchan2track[MIDIchannel] = TrackCnt++;
		}
		else
		{
			MIDIchannel = Mus2MidChannel[MUSchannel];
			MIDItrack = MIDIchan2track[MIDIchannel];
		}
		TWriteVarLen(MIDItrack, Tracks[MIDItrack].DeltaTime);
		Tracks[MIDItrack].DeltaTime = 0;

		switch (et)
		{
		//	Release note
		case 0:
			//NewEvent = 0x90 | MIDIchannel;
			NewEvent = 0x80 | MIDIchannel;
			TWriteByte(MIDItrack, NewEvent);
			Tracks[MIDItrack].LastEvent = NewEvent;
			Strm << data;
			TWriteByte(MIDItrack, data);
			//TWriteByte(MIDItrack, 0);
			TWriteByte(MIDItrack, 64);
			break;

		//	Note on
		case 1:
			NewEvent = 0x90 | MIDIchannel;
			TWriteByte(MIDItrack, NewEvent);
			Tracks[MIDItrack].LastEvent = NewEvent;
			Strm << data;
			TWriteByte(MIDItrack, data & 0x7F);
			if (data & 0x80)
			{
				Strm << data;
				Tracks[MIDItrack].Vel = data;
			}
			TWriteByte(MIDItrack, Tracks[MIDItrack].Vel);
			break;

		//	Pitch wheel
		case 2:
			NewEvent = 0xE0 | MIDIchannel;
			TWriteByte(MIDItrack, NewEvent);
			Tracks[MIDItrack].LastEvent = NewEvent;
			Strm << data;
			TWriteByte(MIDItrack, (data & 1) << 6);
			TWriteByte(MIDItrack, data >> 1);
			break;

		//	Control change
		case 3:
			NewEvent = 0xB0 | MIDIchannel;
			TWriteByte(MIDItrack, NewEvent);
			Tracks[MIDItrack].LastEvent = NewEvent;
			Strm << data;
			check(data < 15);
			TWriteByte(MIDItrack, Mus2MidControl[data]);
			if (data == 12)
				//TWriteByte(MIDItrack, LittleShort(MUSh.NumChannels) + 1);
				TWriteByte(MIDItrack, LittleShort(MUSh.NumChannels));
			else
				TWriteByte(MIDItrack, 0);
			break;

		//	Control or program change
		case 4:
			Strm << data;
			if (data)
			{
				NewEvent = 0xB0 | MIDIchannel;
				TWriteByte(MIDItrack, NewEvent);
				Tracks[MIDItrack].LastEvent = NewEvent;
				check(data < 15);
				TWriteByte(MIDItrack, Mus2MidControl[data]);
			}
			else
			{
				NewEvent = 0xC0 | MIDIchannel;
				TWriteByte(MIDItrack, NewEvent);
				Tracks[MIDItrack].LastEvent = NewEvent;
			}
			Strm << data;
			TWriteByte(MIDItrack, data);
			break;

		case 5:
		case 7:
			GCon->Log(NAME_Dev,"MUS file corupted");
			return false;
		default:
			break;
		}

		if (event & 0x80)
		{
			DeltaTime = ReadTime(Strm);
			for (i = 0; i < 16; i++)
				Tracks[i].DeltaTime += DeltaTime;
		}

		if (!Strm.AtEnd())
		{
			Strm << event;
			et = (event & 0x70) >> 4;
			MUSchannel = event & 0x0f;
		}
		else
		{
			ouch = true;
		}
	}

	for (i = 0; i < TrackCnt; i++)
	{
		TWriteBuf(i, TrackEnd, 4);
	}

	if (ouch)
	{
		GCon->Logf(NAME_Dev, "WARNING : There are bytes missing at the end.");
		GCon->Logf(NAME_Dev, "The end of the MIDI file might not fit the original one.");
	}

	return true;
	unguard;
}

//==========================================================================
//
//	VQMus2Mid::WriteMIDIFile
//
//==========================================================================

void VQMus2Mid::WriteMIDIFile(VStream& Strm)
{
	guard(VQMus2Mid::WriteMIDIFile);
	//	Header
	char HdrId[4] = { 'M', 'T', 'h', 'd' };
	vuint32 HdrSize = 6;
	vuint16 HdrType = 1;
	vuint16 HdrNumTracks = TrackCnt;
	vuint16 HdrDivisions = 89;

	Strm.Serialise(HdrId, 4);
	Strm.SerialiseBigEndian(&HdrSize, 4);
	Strm.SerialiseBigEndian(&HdrType, 2);
	Strm.SerialiseBigEndian(&HdrNumTracks, 2);
	Strm.SerialiseBigEndian(&HdrDivisions, 2);

	//	Tracks
	for (int i = 0; i < (int)TrackCnt; i++)
	{
		//	Identifier.
		char TrackId[4] = { 'M', 'T', 'r', 'k' };
		Strm.Serialise(TrackId, 4);

		//	Data size.
		vuint32 TrackSize = Tracks[i].Data.Num();
		Strm.SerialiseBigEndian(&TrackSize, 4);

		//	Data.
		Strm.Serialise(Tracks[i].Data.Ptr(), Tracks[i].Data.Num());
	}
	unguard;
}

//==========================================================================
//
//  VQMus2Mid::Run
//
//==========================================================================

int VQMus2Mid::Run(VStream& InStrm, VStream& OutStrm)
{
	guard(VQMus2Mid::Run);
	if (Convert(InStrm))
	{
		WriteMIDIFile(OutStrm);
	}
	return OutStrm.TotalSize();
	unguard;
}
