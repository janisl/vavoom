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

#include <allegro.h>
#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

#define MIDIMAGIC    		"MThd"
#define MUSMAGIC     		"MUS\032"

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
	dword		hdr_size;
	word		type;
    word		num_tracks;
	word		divisions;
} MIDheader;

#pragma pack()

struct Track
{
	dword		current;
	char	 	vel;
	long	 	DeltaTime;
	byte		LastEvent;
	byte		*data; 	   /* Primary data */
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void			StartMidiSong(char* song, boolean loop);
static void			StopMidiSong(void);
static boolean		LoadMUS(int len);
static boolean		LoadMIDI(void);
static void			FreeLump(void);
static void			FreeTracks(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static boolean		enabled = false;
static MIDI			mididata;
static boolean		midi_locked = false;

static char			Mus_Song[9] = "";
static boolean		mus_looping = false;
static void*		Mus_SndPtr  = NULL;
static boolean		MusicPaused = false;
static int			MusVolume = -1;

static struct Track	tracks[MIDI_TRACKS];
static word			TrackCnt = 0;
static int	 		MUS2MIDchannel[16];

static byte			track_end[] =
	{0x00, 0xff, 47, 0x00};
static byte			midikey[] =
	{0x00, 0xff, 0x59, 0x02, 0x00, 0x00};   	// C major
static byte			miditempo[] =
	{0x00, 0xff, 0x51, 0x03, 0x09, 0xa3, 0x1a};	// uS/qnote

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	S_InitMusic
//
//==========================================================================

void S_InitMusic(void)
{
	enabled = true;
}

//==========================================================================
//
//  S_UpdateMusic
//
//==========================================================================

void S_UpdateMusic(void)
{
	guard(S_UpdateMusic);
	//	Update volume
    if (music_volume < 0 )
    {
    	music_volume = 0;
    }
    if (music_volume > 15)
    {
    	music_volume = 15;
    }

	if (music_volume != MusVolume)
    {
		if (!MusVolume && (int)music_volume && !MusicPaused)
	    {
		   	midi_resume();
	    }
		if (MusVolume && !(int)music_volume)
	    {
		   	midi_pause();
	    }
	    MusVolume = music_volume;
		set_volume(-1, MusVolume * 17);
    }
	unguard;
}

//==========================================================================
//
//	S_ShutdownMusic
//
//==========================================================================

void S_ShutdownMusic(void)
{
	guard(S_ShutdownMusic);
	StopMidiSong();
	unguard;
}

//==========================================================================
//
//  Music_f
//
//==========================================================================

COMMAND(Music)
{
	guard(COMMAND Music);
	char	*command;

	S_UpdateMusic();

	if (Argc() < 2)
	{
		return;
	}

	command = Argv(1);

	if (!stricmp(command, "on"))
	{
		enabled = true;
		return;
	}

	if (!stricmp(command, "off"))
	{
		StopMidiSong();
		enabled = false;
		return;
	}

	if (!enabled)
    {
    	return;
    }

	if (!stricmp(command, "play"))
    {
		StartMidiSong(Argv(2), false);
		return;
	}

	if (!stricmp(command, "loop"))
    {
		StartMidiSong(Argv(2), true);
		return;
	}

	if (!stricmp(command, "pause"))
    {
	   	midi_pause();
		MusicPaused = true;
		return;
    }

	if (!stricmp(command, "resume"))
    {
    	if (MusVolume)
		   	midi_resume();
		MusicPaused = false;
		return;
    }

	if (!stricmp(command, "stop"))
    {
    	StopMidiSong();
		return;
    }

	if (!stricmp(command, "info"))
	{
	    if (midi_pos >= 0)
        {
        	con << "Currently " << (mus_looping ? "looping" : "playing")
        		<< " \"" << Mus_Song << "\".\n";
        }
        else
        {
        	con << "No song currently playing\n";
        }
        return;
	}
	unguard;
}


//==========================================================================
//
//	StartMidiSong
//
//==========================================================================

static void StartMidiSong(char* song, boolean loop)
{
	int			len;
	boolean		res;

	if (!song || !song[0])
	{
       	return;
	}
	if (!stricmp(song, Mus_Song) && midi_pos >= 0)
	{
       	// don't replay an old song if it is still playing
		return;
	}

	if (UseSndScript)
	{
		char	*name;

		name = va("%s%s.lmp", ArchivePath, song);
	    if (!Sys_FileExists(name))
	    {
			name = va("%s%s.mid", ArchivePath, song);
		    if (!Sys_FileExists(name))
		    {
		    	con << "Can't find song \"" << song << "\"\n";
		        return;
			}
	    }
		StopMidiSong();
		len = M_ReadFile(name, (byte **)&Mus_SndPtr);
	}
	else
	{
		int		lump;

		lump = W_CheckNumForName(song);
    	if (lump < 0)
	    {
	    	con << "Can't find song \"" << song << "\"\n";
    	    return;
	    }
		StopMidiSong();
		Mus_SndPtr = W_CacheLumpNum(lump, PU_MUSIC);
		len = W_LumpLength(lump);
	}

	if (!memcmp(Mus_SndPtr, MIDIMAGIC, 4))
    {
		res = LoadMIDI();
	}
	else if (!memcmp(Mus_SndPtr, MUSMAGIC, 4))
    {
    	res = LoadMUS(len);
	}
    else
    {
    	con << "Not a MUS or MIDI file\n";
        res = false;
    }
	if (!res)
    {
    	FreeLump();
		FreeTracks();
    	return;
    }

   	play_midi(&mididata, loop); // 'true' denotes endless looping.
	if (!MusVolume || MusicPaused)
	{
   		midi_pause();
	}
	strcpy(Mus_Song, song);
    mus_looping = loop;
}

//==========================================================================
//
//	FreeLump
//
//==========================================================================

static void FreeLump(void)
{
	if (Mus_SndPtr)
	{
		if (UseSndScript)
		{
			Z_Free(Mus_SndPtr);
		}
		else
		{
			Z_ChangeTag(Mus_SndPtr, PU_CACHE);
		}
		Mus_SndPtr = NULL;
	}
}

//==========================================================================
//
//  StopMidiSong
//
//==========================================================================

static void StopMidiSong(void)
{
	stop_midi();
	if (midi_locked)
    {
		int		i;

		for (i = 0; i < MIDI_TRACKS; i++)
		{
			if (mididata.track[i].data)
			{
				UNLOCK_DATA(mididata.track[i].data, mididata.track[i].len);
			}
		}
		UNLOCK_DATA(&mididata, sizeof(MIDI));
        midi_locked = false;
    }
	FreeLump();
	FreeTracks();
	Mus_Song[0] = 0;
}

//**************************************************************************
//
//	Loading a MIDI file
//
//**************************************************************************

//==========================================================================
//
//  LoadMIDI
//
//	Convert an in-memory copy of a MIDI format 0 or 1 file to
// an Allegro MIDI structure
//
//==========================================================================

static boolean LoadMIDI(void)
{
	int 		i;
	int 		num_tracks;
    byte		*data;
    MIDheader	*hdr;

	memset(&mididata, 0, sizeof(mididata));

	hdr = (MIDheader*)Mus_SndPtr;

	// MIDI file type
	i = BigShort(hdr->type);
   	if ((i != 0) && (i != 1))
    {
        // only type 0 and 1 are suported
       	cond << "Unsuported MIDI type\n";
     	return false;
    }

    // number of tracks
	num_tracks = BigShort(hdr->num_tracks);
   	if ((num_tracks < 1) || (num_tracks > MIDI_TRACKS))
    {
       	cond << "Invalid MIDI track count\n";
      	return false;
    }

    // beat divisions
   	mididata.divisions = BigShort(hdr->divisions);

    // read each track
	data = (byte*)hdr + sizeof(*hdr);
	for (i = 0; i < num_tracks; i++)
	{
	   	if (memcmp(data, "MTrk", 4))
        {
           	cond << "Bad MIDI track " << i << " header\n";
		 	return false;
        }
      	data += 4;

      	mididata.track[i].len = BigLong(*(int*)data);
      	data += 4;

      	mididata.track[i].data = data;
      	data += mididata.track[i].len;
   	}

	lock_midi(&mididata);
    midi_locked = true;
	return true;
}

//**************************************************************************
//
//	Loading a MUS file and converting it to MIDI format using sources of
// Quick MUS->MID converter by S.Bacquet
//
//**************************************************************************

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
		Sys_Error("qmus2mid: Track buffer full.");
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
//	AllocateChannel
//
//==========================================================================

static void AllocateChannel(int num)
{
	if (tracks[num].data)
    {
    	Z_ChangeTag(tracks[num].data, PU_MUSIC);
    }
    else
    {
		tracks[num].data = (byte *)Z_Malloc(TRACKBUFFERSIZE, PU_MUSIC,
			(void**)&tracks[num].data);
	}
}

//==========================================================================
//
//	LoadMUS
//
//	Convert mus to mid with a wanderfull function
// thanks to S.Bacquet for the source of qmus2mid
//
//==========================================================================

static boolean LoadMUS(int length)
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
	byte				MIDIchan2track[MIDI_TRACKS];
  	boolean				ouch = false;
	MUSheader			*MUSh;

	FreeTracks();	//	Possibly not needed

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
	}

	MUSh = (MUSheader *)Mus_SndPtr;

	if (MUSh->channels > 15)	 /* <=> MUSchannels+drums > 16 */
    {
    	cond << "Too many channels\n";
		return false;
	}

	mus_ptr = (char*)Mus_SndPtr + MUSh->ScoreStart;

	AllocateChannel(0);
	TWriteBuf(0, midikey, 6);
	TWriteBuf(0, miditempo, 7);

	TrackCnt = 1;	//	Music starts here

	event = *(mus_ptr++);
	et = event_type(event);
	MUSchannel = channel(event);
	while ((et != 6) && mus_ptr - (char*)Mus_SndPtr < length && (event != EOF))
    {
		if (MUS2MIDchannel[MUSchannel] == -1)
		{
	  		MIDIchannel = MUS2MIDchannel[MUSchannel] =
	    		(MUSchannel == 15 ? 9 : FirstChannelAvailable());
	  		MIDItrack   = MIDIchan2track[MIDIchannel] = TrackCnt++;
		   	if (TrackCnt > MIDI_TRACKS)
		    {
		       	Sys_Error("qmus2mid: MIDI track count overflow.");
		    }
            AllocateChannel(MIDItrack);
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
			ouch = true;
		}
	}

    for (i=0; i<TrackCnt; i++)
    {
		TWriteBuf(i, track_end, 4);
	}

	if (ouch)
    {
		cond << "WARNING : There are bytes missing at the end of MUS file. "
			"The end of the MIDI file might not fit the original one.\n";
	}

	FreeLump();

	memset(&mididata, 0, sizeof(mididata));
   	mididata.divisions = 89;

	//	Tracks
	for (i = 0; i < (int)TrackCnt; i++)
    {
      	mididata.track[i].len = tracks[i].current;
      	mididata.track[i].data = tracks[i].data;
   	}

	lock_midi(&mididata);
    midi_locked = true;

	return true;
}

//==========================================================================
//
//  FreeTracks
//
//==========================================================================

static void FreeTracks(void)
{
	int i ;

	for (i = 0; i < MIDI_TRACKS; i++)
    {
    	if (tracks[i].data)
        {
      		Z_ChangeTag(tracks[i].data, PU_CACHE);
		}
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2002/01/11 08:12:01  dj_jl
//	Added guard macros
//
//	Revision 1.4  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
