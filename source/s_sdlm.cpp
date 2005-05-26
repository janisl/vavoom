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

#include <SDL.h>
#include <SDL_mixer.h>

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

                                       /* Theoretical maximums: */
#define MIDI_VOICES           64       /* actual drivers may not be */
#define MIDI_TRACKS           32       /* able to handle this many */

#define MIDIMAGIC    		"MThd"
#define MUSMAGIC     		"MUS\032"

#define last(e) 			((byte)(e & 0x80))
#define event_type(e)		((byte)((e & 0x7F) >> 4))
#define channel(e)			((byte)(e & 0x0F))

#define TRACKBUFFERSIZE		65536  /* 64 Ko */

// TYPES -------------------------------------------------------------------

#pragma pack(1)


typedef struct MIDI                    /* a midi file */
{
   int divisions;                      /* number of ticks per quarter note */
   struct {
      unsigned char *data;             /* MIDI message stream */
      int len;                         /* length of the track data */
   } track[MIDI_TRACKS];
} MIDI;


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
	byte	 	vel;
	dword	 	DeltaTime;
	int		laststatus;
	byte		LastEvent;
	byte		*data; 	   /* Primary data */
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void			StartMidiSong(char* song, boolean loop);
static void			StopMidiSong(void);
#if 0
static boolean			LoadMUS(int len);
#else
int qmus2mid(char *mus, char *mid, int length);
#endif
static void			FreeLump(void);
static void			FreeTracks(void);
static int			write32bit(int handle, dword data);
static void			write_event(int track, int type, int chan, byte *data, int size);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------
static Mix_Music *music = NULL;

static boolean		enabled = false;
static MIDI			mididata;

static char			Mus_Song[9] = "";
static boolean		mus_looping = false;
static void*		Mus_SndPtr  = NULL;
static boolean		MusicPaused = false;
static int			MusVolume = -1;

static char			*mid_file;

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
    			Mix_ResumeMusic();
		}
		if (MusVolume && !(int)music_volume)
		{
			Mix_PauseMusic();
		}
		MusVolume = music_volume;
		Mix_VolumeMusic(MusVolume * 17);
	}
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

	if (!enabled)
	{
    		return;
	}

	if (!stricmp(command, "off"))
	{
		StopMidiSong();
		enabled = false;
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
		Mix_PauseMusic();
		MusicPaused = true;
		return;
	}

	if (!stricmp(command, "resume"))
	{
    		if (MusVolume) Mix_ResumeMusic();
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
		if (music)
        	{
        		GCon->Logf("Currently %s %s.",
				mus_looping ? "looping" : "playing", Mus_Song);
        	}
        	else
        	{
        		GCon->Log("No song currently playing");
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
	int		len, handle, i;
	boolean		res;
	MIDheader	hdr;

	if (!song || !song[0])
	{
       		return;
	}


	if (!stricmp(song, Mus_Song) && music)
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
		    		GCon->Logf("Can't find song %s", song);
		        	return;
			}
		}
		StopMidiSong();
		music = Mix_LoadMUS(name);
		Mix_FadeInMusic(music, loop, 2000);
		return;
	}
	else
	{
		int		lump;

		lump = W_CheckNumForName(song);

    		if (lump < 0)
		{
	    		GCon->Logf("Can't find song %s", song);
    	    		return;
		}
		StopMidiSong();
		Mus_SndPtr = W_CacheLumpNum(lump, PU_MUSIC);
		len = W_LumpLength(lump);
	}

	if (!memcmp(Mus_SndPtr, MIDIMAGIC, 4))
	{
		if ((handle = Sys_FileOpenWrite("vv_temp.mid")) < 0) return;
		if (Sys_FileWrite(handle, Mus_SndPtr, len) != len) return;
		if (Sys_FileClose(handle) < 0) return;
	}
	else if (!memcmp(Mus_SndPtr, MUSMAGIC, 4))
	{
#if 0
		res = LoadMUS(len);
		if (!res) return;

		if ((handle = Sys_FileOpenWrite("vv_temp.mid")) < 0) return;

		memcpy(hdr.ID, MIDIMAGIC, 4);
		hdr.hdr_size = BigLong(6);
		hdr.num_tracks = BigShort((word)TrackCnt);
		hdr.divisions = BigShort((word)mididata.divisions);
		if (TrackCnt > 1) hdr.type = BigShort(1);
		else hdr.type = 0;

		if (Sys_FileWrite(handle, &hdr, sizeof(hdr)) != sizeof(hdr)) return;

		for (i = 0; i < TrackCnt; i++)
		{
			dword tlen = mididata.track[i].len;
			if (Sys_FileWrite(handle, "MTrk", 4) != 4) return;
			if (write32bit(handle, tlen) < 0) return;
			if (Sys_FileWrite(handle, mididata.track[i].data, tlen) != tlen) return;
		}

		if (Sys_FileClose(handle) < 0) return;

		FreeTracks();	//	Possibly not needed
#else
		void* mid = Z_Malloc(256 * 1024);
		res = qmus2mid((char*)Mus_SndPtr, (char*)mid, len);
		if (!res) return;

		if ((handle = Sys_FileOpenWrite("vv_temp.mid")) < 0) return;
		if (Sys_FileWrite(handle, mid, res) != res) return;
		if (Sys_FileClose(handle) < 0) return;

		FreeTracks();	//	Possibly not needed
#endif
	}
	else
	{
    		GCon->Log("Not a MUS or MIDI file");
		music = NULL;
		return;
	}

	music = Mix_LoadMUS("vv_temp.mid");

	if (!music)
	{
		FreeLump();
		FreeTracks();
		return;
	}

	Mix_FadeInMusic(music, loop, 2000);

	if (!MusVolume || MusicPaused)
	{
		Mix_PauseMusic();
	}
	strcpy(Mus_Song, song);
	mus_looping = loop;
}

//==========================================================================
//
//	write32bit
//
//==========================================================================
static int write32bit(int handle, dword data)
{
	byte 	outb;
	outb = (data >> 24) & 0xff;
	if (Sys_FileWrite(handle, &outb, 1) != 1) return -1;
	outb = (data >> 16) & 0xff;
	if (Sys_FileWrite(handle, &outb, 1) != 1) return -1;
	outb = (data >> 8) & 0xff;
	if (Sys_FileWrite(handle, &outb, 1) != 1) return -1;
	outb = data & 0xff;
	if (Sys_FileWrite(handle, &outb, 1) != 1) return -1;
	return 0;
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
	Mix_HaltMusic();
	Mix_FreeMusic(music);
	music = NULL;

	FreeLump();
	//FreeTracks();
	Mus_Song[0] = 0;
}

//**************************************************************************
//
//	Loading a MUS file and converting it to MIDI format using sources of
// Quick MUS->MID converter by S.Bacquet
//
//**************************************************************************

#if 0
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

static void TWriteByte(int MIDItrack, byte data)
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
	while( (value >>= 7)  > 0)
	{
      		buffer <<= 8 ;
      		buffer |= 0x80 ;
      		buffer += (value & 0x7f) ;
	}

  	while (1)
	{
		TWriteByte(tracknum, buffer & 0xff);
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
	register dword		time = 0;
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
//	write_event
//
//==========================================================================
static int debugcnt = 500;

static void write_event(int track, int type, int chan, byte *data, int size)
{
	int	i;
	byte	c;

	TWriteVarLen(track, tracks[track].DeltaTime);

if (debugcnt < 400 && track == 1) {
	printf("track %d len %d delta %lu\n", track, tracks[track].current, tracks[track].DeltaTime);
}
	if (type == 0xf0 || type == 0xf7)
	{
		c = type;
		tracks[track].laststatus = 0;
	}
	else
		c = type | chan;

if (debugcnt < 400 && track == 1) {
	printf("\ttype %02X laststatus %02X c=%02X\n", type, (int)tracks[track].laststatus, c);
}
	//if (tracks[track].laststatus != c)  Why doesn't this work?
		TWriteByte(track, tracks[track].laststatus = c);

	if (type == 0xf0 || type == 0xf7)
		TWriteVarLen(track, (dword)size);

   	tracks[track].LastEvent = type;

	for (i = 0; i < size; i++) TWriteByte(track, data[i]);
if (debugcnt < 400 && track == 1) {
	printf("\tsize %d data1=%d data2=%d len %d\n", size, data[0], data[1], tracks[track].current);
	debugcnt++;
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
//	int					data;
	byte					data[10];
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
      		tracks[i].laststatus = 0;
	}

	MUSh = (MUSheader *)Mus_SndPtr;

	if (MUSh->channels > 15)	 /* <=> MUSchannels+drums > 16 */
	{
    		GCon->Log(NAME_Dev, "Too many channels");
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
      		//TWriteVarLen(MIDItrack, tracks[MIDItrack].DeltaTime);
      		//tracks[MIDItrack].DeltaTime = 0 ;

      		switch (et)
		{
		 case 0:		/* release note */
  			//NewEvent = 0x90 | MIDIchannel;
  			data[0] = *(mus_ptr++);
			data[1] = 0;
			write_event(MIDItrack, 0x90, MIDIchannel, data, 2);
  			break;
		 case 1:
  			//NewEvent = 0x90 | MIDIchannel;
  			data[0] = *(mus_ptr++);
  			if (data[0] & 0x80) tracks[MIDItrack].vel = *(mus_ptr++);
			data[0] &= 0x7F;
			data[1] = tracks[MIDItrack].vel;
			write_event(MIDItrack, 0x90, MIDIchannel, data, 2);
  			break;
		 case 2:
  			//NewEvent = 0xE0 | MIDIchannel;
  			data[0] = *(mus_ptr++);
			data[1] = data[0] >> 1;
			data[0] &= 1;
			data[0] <<= 6;
  			//TWriteByte(MIDItrack, (data[0] & 1) << 6);
  			//TWriteByte(MIDItrack, data[0] >> 1);

			write_event(MIDItrack, 0xE0, MIDIchannel, data, 2);
  			break;
		 case 3:
  			//NewEvent = 0xB0 | MIDIchannel;
  			data[0] = *(mus_ptr++) ;
  			if (data[0] == 12) data[1] = MUSh->channels + 1;
			else data[1] = 0;
			data[0] = MUS2MIDcontrol[data[0]];

			write_event(MIDItrack, 0xB0, MIDIchannel, data, 2);
  			break;
		 case 4:
  			data[0] = *(mus_ptr++);
  			data[1] = *(mus_ptr++);
  			if (data[0])
    			{
      				//NewEvent = 0xB0 | MIDIchannel;
				data[0] = MUS2MIDcontrol[data[0]];
				write_event(MIDItrack, 0xB0, MIDIchannel, data, 2);
    			}
  			else
    			{
      				//NewEvent = 0xC0 | MIDIchannel;
				write_event(MIDItrack, 0xC0, MIDIchannel, data, 2);
    			}
  			break;
		 case 5:
		 case 7:
			GCon->Log(NAME_Dev, "MUS file corupted");
  			return false;
		 default:
           	break;
		} /* switch */

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
      		else  ouch = true;
	} /* while */

	for (i=0; i<TrackCnt; i++)
	{
		TWriteBuf(i, track_end, 4);
	}

	if (ouch)
	{
		GCon->Log(NAME_Dev, "WARNING : There are bytes missing at the end of MUS file.");
		GCon->Log(NAME_Dev, "The end of the MIDI file might not fit the original one.");
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
#else
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
    	GCon->Log("Not a MUS file");
		return false;
	}

	if (MUSh->channels > 15)	 /* <=> MUSchannels+drums > 16 */
    {
    	GCon->Log(NAME_Dev,"Too meny channels");
		return false;
	}

	mus_ptr = mus + MUSh->ScoreStart;

	tracks[0].data = (byte*)Z_Malloc(TRACKBUFFERSIZE, PU_MUSIC, 0);
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
			tracks[MIDItrack].data = (byte*)Z_Malloc(TRACKBUFFERSIZE, PU_MUSIC, 0);
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
			GCon->Log(NAME_Dev,"MUS file corupted");
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
		GCon->Logf(NAME_Dev, "WARNING : There are bytes missing at the end.");
		GCon->Logf(NAME_Dev, "The end of the MIDI file might not fit the original one.");
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
	MIDheader*		hdr;

	//	Header
	hdr = (MIDheader*)mid_file;
	memcpy(hdr->ID, MIDIMAGIC, 4);
    hdr->hdr_size   = BigLong(6);
    hdr->type       = BigShort(1);
	hdr->num_tracks = BigShort(TrackCnt);
	hdr->divisions  = BigShort(89);
	mid_file += sizeof(*hdr);

	//	Tracks
	for (i = 0; i < (int)TrackCnt; i++)
    {
		size = BigLong(tracks[i].current);
		WriteBuf((void*)"MTrk", 4);
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

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2005/05/26 17:00:46  dj_jl
//	Working MUS to MID conversion
//
//	Revision 1.5  2005/04/28 07:16:15  dj_jl
//	Fixed some warnings, other minor fixes.
//	
//	Revision 1.4  2004/10/11 06:49:57  dj_jl
//	SDL patches.
//	
//	Revision 1.3  2002/01/11 08:13:13  dj_jl
//	Added command Music
//	
//	Revision 1.2  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2002/01/03 18:39:42  dj_jl
//	Added SDL port
//	
//**************************************************************************
