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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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
//**
//**    Quick MUS->MID ! by S.Bacquet
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

#define MUSMAGIC     		"MUS\032"
#define MIDIMAGIC    		"MThd"

#define last(e) 			((byte)(e & 0x80))
#define event_type(e)		((byte)((e & 0x7F) >> 4))
#define channel(e)			((byte)(e & 0x0F))

#define TRACKBUFFERSIZE		65536  /* 64 Ko */

// TYPES -------------------------------------------------------------------

#pragma pack(1)

typedef struct
{
	char		ID[4];			/* identifier "MUS" 0x1A */
	word		ScoreLength;
	word		ScoreStart;
	word		channels; 		/* count of primary channels */
	word		SecChannels;	/* count of secondary channels (?) */
	word		InstrCnt;
	word		dummy;
} MUSheader;

typedef struct
{
	char		ID[4];
    dword		size;
    word		file_type;
	word		num_tracks;
	word		divisions;
} MIDI_Header_t;

#pragma pack()

struct Track
{
	dword		current;
	char	 	vel;
	long	 	DeltaTime;
	byte		LastEvent;
	char		*data; 	   /* Primary data */
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char				*mid_file;

static struct Track		tracks[32];
static word				TrackCnt = 0;
static int	 			MUS2MIDchannel[16];

static byte				track_end[] =
	{0x00, 0xff, 47, 0x00};
static byte				midikey[] =
	{0x00, 0xff, 0x59, 0x02, 0x00, 0x00};   	// C major
static byte				miditempo[] =
	{0x00, 0xff, 0x51, 0x03, 0x09, 0xa3, 0x1a};	// uS/qnote

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  FirstChannelAvailable
//
//==========================================================================

static int FirstChannelAvailable(void)
{
	int 	i;
	int 	old15 = MUS2MIDchannel[15];
	int		max = -1;

	MUS2MIDchannel[15] = -1;
	for (i = 0; i < 16; i++)
    	if (MUS2MIDchannel[i] > max)
    		max = MUS2MIDchannel[i];
	MUS2MIDchannel[15] = old15 ;

	return (max == 8 ? 10 : max + 1);
}

//==========================================================================
//
//  TWriteByte
//
//==========================================================================

static void TWriteByte(int MIDItrack, char data)
{
	if (tracks[MIDItrack].current < TRACKBUFFERSIZE)
    {
    	tracks[MIDItrack].data[tracks[MIDItrack].current] = data;
	}
	else
    {
		I_Error("qmus2mid: Track buffer full.");
    }
	tracks[MIDItrack].current++;
}

//==========================================================================
//
//	TWriteBuf
//
//==========================================================================

static void TWriteBuf(int MIDItrack, byte *buf, int size)
{
	int		i;

	for (i = 0; i < size; i++)
    {
    	TWriteByte(MIDItrack, buf[i]);
    }
}

//==========================================================================
//
//  TWriteVarLen
//
//==========================================================================

static void TWriteVarLen(int tracknum, register dword value)
{
	register dword	buffer ;

	buffer = value & 0x7f ;
	while( (value >>= 7) )
    {
      	buffer <<= 8 ;
      	buffer |= 0x80 ;
      	buffer += (value & 0x7f) ;
    }
  	while (1)
    {
		TWriteByte(tracknum, buffer);
      	if (buffer & 0x80)
			buffer >>= 8 ;
      	else
			break;
    }
}

//==========================================================================
//
//  ReadTime
//
//==========================================================================

static dword ReadTime(char **file)
{
	register dword 	time = 0 ;
	int 			data;

	do
    {
    	data = *(*file)++;
		if (data != EOF)
			time = (time << 7) + (data & 0x7F);
    } while ((data != EOF) && (data & 0x80));

	return time;
}

//==========================================================================
//
//  convert
//
//==========================================================================

static boolean convert(char *mus, int length)
{
	char				*mus_ptr;
	byte				et;
	int					MUSchannel;
	int					MIDIchannel;
	int					MIDItrack = 0;
	int					NewEvent;
	int 				i;
	int					event;
	int					data;
	dword				DeltaTime;
	dword				TotalTime = 0;
	byte				MUS2MIDcontrol[15] =
	{
	    0,				/* Program change - not a MIDI control change */
	    0x00,			/* Bank select */
	    0x01,			/* Modulation pot */
	    0x07,			/* Volume */
	    0x0A,			/* Pan pot */
	    0x0B,			/* Expression pot */
	    0x5B,			/* Reverb depth */
	    0x5D,			/* Chorus depth */
	    0x40,			/* Sustain pedal */
	    0x43,			/* Soft pedal */
	    0x78,			/* All sounds off */
	    0x7B,			/* All notes off */
	    0x7E,			/* Mono */
	    0x7F,			/* Poly */
	    0x79			/* Reset all controllers */
	};
	byte				MIDIchan2track[16];
  	char 				ouch = 0;
	MUSheader			*MUSh;

	for (i = 0 ; i < 16 ; i++)
	{
		MUS2MIDchannel[i] = -1;
	}
	for (i = 0 ; i < 32 ; i++)
	{
      	tracks[i].current = 0;
      	tracks[i].vel = 64;
      	tracks[i].DeltaTime = 0;
      	tracks[i].LastEvent = 0;
      	tracks[i].data = NULL;
	}

	MUSh = (MUSheader *)mus;
	if (strncmp(MUSh->ID, MUSMAGIC, 4))
    {
    	con << "Not a MUS file\n";
		return false;
	}

	if (MUSh->channels > 15)	 /* <=> MUSchannels+drums > 16 */
    {
    	cond << "Too meny channels\n";
		return false;
	}

	mus_ptr = mus + MUSh->ScoreStart;

	tracks[0].data = (char *)Z_Malloc(TRACKBUFFERSIZE, PU_MUSIC, 0);
	TWriteBuf(0, midikey, 6);
	TWriteBuf(0, miditempo, 7);

	TrackCnt = 1;	//	Music starts here

	event = *(mus_ptr++);
	et = event_type(event);
	MUSchannel = channel(event);
	while ((et != 6) && mus_ptr - mus < length && (event != EOF))
    {
		if (MUS2MIDchannel[MUSchannel] == -1)
		{
	  		MIDIchannel = MUS2MIDchannel[MUSchannel] =
	    		(MUSchannel == 15 ? 9 : FirstChannelAvailable());
	  		MIDItrack   = MIDIchan2track[MIDIchannel] = TrackCnt++;
			tracks[MIDItrack].data = (char *)Z_Malloc(TRACKBUFFERSIZE, PU_MUSIC, 0);
		}
      	else
		{
	  		MIDIchannel = MUS2MIDchannel[MUSchannel];
	  		MIDItrack   = MIDIchan2track[MIDIchannel];
		}
      	TWriteVarLen(MIDItrack, tracks[MIDItrack].DeltaTime);
      	tracks[MIDItrack].DeltaTime = 0 ;
      	switch (et)
		{
		 case 0:		/* release note */
  			NewEvent = 0x90 | MIDIchannel;
   			TWriteByte(MIDItrack, NewEvent);
   			tracks[MIDItrack].LastEvent = NewEvent;
  			data = *(mus_ptr++);
  			TWriteByte(MIDItrack, data);
  			TWriteByte(MIDItrack, 0);
  			break;
		 case 1:
  			NewEvent = 0x90 | MIDIchannel;
   			TWriteByte(MIDItrack, NewEvent);
   			tracks[MIDItrack].LastEvent = NewEvent;
  			data = *(mus_ptr++);
  			TWriteByte(MIDItrack, data & 0x7F);
  			if (data & 0x80)
    			tracks[MIDItrack].vel = *(mus_ptr++);
  			TWriteByte(MIDItrack, tracks[MIDItrack].vel);
  			break;
		 case 2:
  			NewEvent = 0xE0 | MIDIchannel;
   			TWriteByte(MIDItrack, NewEvent);
   			tracks[MIDItrack].LastEvent = NewEvent;
  			data = *(mus_ptr++);
  			TWriteByte(MIDItrack, (data & 1) << 6);
  			TWriteByte(MIDItrack, data >> 1);
  			break;
		 case 3:
  			NewEvent = 0xB0 | MIDIchannel;
   			TWriteByte(MIDItrack, NewEvent);
   			tracks[MIDItrack].LastEvent = NewEvent;
  			data = *(mus_ptr++) ;
  			TWriteByte(MIDItrack, MUS2MIDcontrol[data]);
  			if (data == 12)
    			TWriteByte(MIDItrack, MUSh->channels + 1);
  			else
    			TWriteByte(MIDItrack, 0);
  			break;
		 case 4:
  			data = *(mus_ptr++);
  			if (data)
    		{
      			NewEvent = 0xB0 | MIDIchannel;
  				TWriteByte(MIDItrack, NewEvent);
  				tracks[MIDItrack].LastEvent = NewEvent;
      			TWriteByte(MIDItrack, MUS2MIDcontrol[data]);
    		}
  			else
    		{
      			NewEvent = 0xC0 | MIDIchannel;
  				TWriteByte(MIDItrack, NewEvent);
  				tracks[MIDItrack].LastEvent = NewEvent;
    		}
  			data = *(mus_ptr++);
  			TWriteByte(MIDItrack, data);
  			break;
		 case 5:
		 case 7:
			cond << "MUS file corupted\n";
  			return false;
		 default:
           	break;
		}
      	if (last(event))
		{
	  		DeltaTime = ReadTime(&mus_ptr);
	  		TotalTime += DeltaTime;
	  		for (i = 0; i < (int)TrackCnt; i++)
	    		tracks[i].DeltaTime += DeltaTime;
		}
      	event = *(mus_ptr++);
      	if (event != EOF)
      	{
	  		et = event_type(event);
	  		MUSchannel = channel(event);
		}
      	else
        {
			ouch = 1;
		}
	}

    for (i=0; i<TrackCnt; i++)
    {
		TWriteBuf(i, track_end, 4);
	}

	if (ouch)
    {
		cond << "WARNING : There are bytes missing at the end.\n"
		"The end of the MIDI file might not fit the original one.\n";
	}

	return true;
}

//==========================================================================
//
//  WriteBuf
//
//==========================================================================

static void WriteBuf(void *p, int size)
{
	memcpy(mid_file, p, size);
	mid_file += size;
}

//==========================================================================
//
//	WriteMIDIFile
//
//==========================================================================

static void WriteMIDIFile(void)
{
	int				i;
	dword			size;
	MIDI_Header_t	*hdr;

	//	Header
	hdr = (MIDI_Header_t*)mid_file;
	memcpy(hdr->ID, MIDIMAGIC, 4);
    hdr->size       = BigLong(6);
    hdr->file_type  = BigShort(1);
	hdr->num_tracks = BigShort(TrackCnt);
	hdr->divisions  = BigShort(89);
	mid_file += sizeof(*hdr);

	//	Tracks
	for (i = 0; i < (int)TrackCnt; i++)
    {
		size = BigLong(tracks[i].current);
		WriteBuf("MTrk", 4);
		WriteBuf(&size, 4);
		WriteBuf(tracks[i].data, tracks[i].current);
	}
}

//==========================================================================
//
//  FreeTracks
//
//==========================================================================

static void FreeTracks(void)
{
	int i ;

	for (i = 0; i < 16; i++)
    {
    	if (tracks[i].data)
        {
      		Z_Free(tracks[i].data);
            tracks[i].data = NULL;
		}
	}
}

//==========================================================================
//
//  qmus2mid
//
//==========================================================================

int qmus2mid(char *mus, char *mid, int length)
{
	mid_file = mid;

	if (convert(mus, length))
    {
	    WriteMIDIFile();
	}
	FreeTracks();

    return mid_file - mid;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
