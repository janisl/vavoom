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

class VStreamMusicPlayer
{
public:
	bool			StrmOpened;
	VAudioCodec*	Codec;
	//	Current playing song info.
	bool			CurrLoop;
	FName			CurrSong;
	bool			Stopping;
	double			FinishTime;

	VStreamMusicPlayer()
	: StrmOpened(false)
	, Codec(NULL)
	, CurrLoop(false)
	, Stopping(false)
	{}
	~VStreamMusicPlayer()
	{}

	void Init();
	void Shutdown();
	void Tick(float);
	void Play(VAudioCodec* InCodec, const char* InName, bool InLoop);
	void Pause();
	void Resume();
	void Stop();
	bool IsPlaying();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static int qmus2mid(FArchive& Ar, byte* mid);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_SOUND_DEVICE(VSoundDevice, SNDDRV_Null, "Null",
	"Null sound device", "-nosfx");

IMPLEMENT_MIDI_DEVICE(VMidiDevice, MIDIDRV_Null, "Null",
	"Null midi device", "-nomusic");

IMPLEMENT_CD_AUDIO_DEVICE(VCDAudioDevice, CDDRV_Null, "Null",
	"Null CD audio device", "-nocdaudio");

TCvarF					sfx_volume("sfx_volume", "0.5", CVAR_ARCHIVE);
TCvarF					music_volume("music_volume", "0.5", CVAR_ARCHIVE);
TCvarI					swap_stereo("swap_stereo", "0", CVAR_ARCHIVE);

VSoundDevice*			GSoundDevice;
VMidiDevice*			GMidiDevice;
VCDAudioDevice*			GCDAudioDevice;
FAudioCodecDesc*		FAudioCodecDesc::List;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char					mapSong[12];
static int					mapCDTrack;

static TCvarI				cd_music("use_cd_music", "0", CVAR_ARCHIVE);
static boolean				CDMusic = false;

static FSoundDeviceDesc*	SoundDeviceList[SNDDRV_MAX];
static FMidiDeviceDesc*		MidiDeviceList[MIDIDRV_MAX];
static FCDAudioDeviceDesc*	CDAudioDeviceList[CDDRV_MAX];

static bool					StreamPlaying;
static VStreamMusicPlayer	GStreamMusicPlayer;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	FSoundDeviceDesc::FSoundDeviceDesc
//
//==========================================================================

FSoundDeviceDesc::FSoundDeviceDesc(int Type, const char* AName,
	const char* ADescription, const char* ACmdLineArg,
	VSoundDevice* (*ACreator)())
: Name(AName)
, Description(ADescription)
, CmdLineArg(ACmdLineArg)
, Creator(ACreator)
{
	SoundDeviceList[Type] = this;
}

//==========================================================================
//
//	FMidiDeviceDesc::FMidiDeviceDesc
//
//==========================================================================

FMidiDeviceDesc::FMidiDeviceDesc(int Type, const char* AName,
	const char* ADescription, const char* ACmdLineArg,
	VMidiDevice* (*ACreator)())
: Name(AName)
, Description(ADescription)
, CmdLineArg(ACmdLineArg)
, Creator(ACreator)
{
	MidiDeviceList[Type] = this;
}

//==========================================================================
//
//	FCDAudioDeviceDesc::FCDAudioDeviceDesc
//
//==========================================================================

FCDAudioDeviceDesc::FCDAudioDeviceDesc(int Type, const char* AName,
	const char* ADescription, const char* ACmdLineArg,
	VCDAudioDevice* (*ACreator)())
: Name(AName)
, Description(ADescription)
, CmdLineArg(ACmdLineArg)
, Creator(ACreator)
{
	CDAudioDeviceList[Type] = this;
}

//==========================================================================
//
//	S_Init
//
// 	Initializes sound stuff, including volume
// 	Sets channels, SFX and music volume,
// allocates channel buffer, sets S_sfx lookup.
//
//==========================================================================

void S_Init()
{
	guard(S_Init);
	int i;

	//	Parse scripts BEFORE initializing device, because it may need to know
	// total number of sounds.
	S_InitScript();
	SN_InitSequenceScript();

	//	Find sound driver to use.
	int SIdx = -1;
	for (i = 0; i < SNDDRV_MAX; i++)
	{
		if (!SoundDeviceList[i])
			continue;
		//	Default to first available non-null sound device.
		if (SIdx == -1)
			SIdx = i;
		//	Check for user selection.
		if (SoundDeviceList[i]->CmdLineArg &&
			M_CheckParm(SoundDeviceList[i]->CmdLineArg))
			SIdx = i;
	}
	GCon->Logf(NAME_Init, "Selected %s", SoundDeviceList[SIdx]->Description);
	GSoundDevice = SoundDeviceList[SIdx]->Creator();

	//	Find MIDI driver to use.
	int MIdx = -1;
	for (i = 0; i < MIDIDRV_MAX; i++)
	{
		if (!MidiDeviceList[i])
			continue;
		//	Default to first available non-null midi device.
		if (MIdx == -1)
			MIdx = i;
		//	Check for user selection.
		if (MidiDeviceList[i]->CmdLineArg &&
			M_CheckParm(MidiDeviceList[i]->CmdLineArg))
			MIdx = i;
	}
	GCon->Logf(NAME_Init, "Selected %s", MidiDeviceList[MIdx]->Description);
	GMidiDevice = MidiDeviceList[MIdx]->Creator();

	//	Find CD audio driver to use.
	int CDIdx = -1;
	for (i = 0; i < CDDRV_MAX; i++)
	{
		if (!CDAudioDeviceList[i])
			continue;
		//	Default to first available non-null CD audio device.
		if (CDIdx == -1)
			CDIdx = i;
		//	Check for user selection.
		if (CDAudioDeviceList[i]->CmdLineArg &&
			M_CheckParm(CDAudioDeviceList[i]->CmdLineArg))
			CDIdx = i;
	}
	GCon->Logf(NAME_Init, "Selected %s", CDAudioDeviceList[CDIdx]->Description);
	GCDAudioDevice = CDAudioDeviceList[CDIdx]->Creator();

	//	Initialise devices.
	GSoundDevice->Init();
	GMidiDevice->Init();
	GCDAudioDevice->Init();
	GStreamMusicPlayer.Init();
	unguard;
}

//==========================================================================
//
//	S_Shutdown
//
//	Shuts down all sound stuff
//
//==========================================================================

void S_Shutdown()
{
	guard(S_Shutdown);
	GStreamMusicPlayer.Shutdown();
	if (GCDAudioDevice)
	{
		GCDAudioDevice->Shutdown();
		delete GCDAudioDevice;
		GCDAudioDevice = NULL;
	}
	if (GMidiDevice)
	{
		GMidiDevice->Shutdown();
		delete GMidiDevice;
		GMidiDevice = NULL;
	}
	if (GSoundDevice)
	{
		GSoundDevice->Shutdown();
		delete GSoundDevice;
		GSoundDevice = NULL;
	}
	unguard;
}

//==========================================================================
//
//	S_StartSound
//
//==========================================================================

void S_StartSound(int sound_id, const TVec &origin, const TVec &velocity,
	int origin_id, int channel, int volume)
{
	GSoundDevice->PlaySound(sound_id, origin, velocity, origin_id, channel,
		float(volume) / 127.0);
}

//==========================================================================
//
//	S_PlayVoice
//
//==========================================================================

void S_PlayVoice(const char *Name)
{
	GSoundDevice->PlayVoice(Name);
}

//==========================================================================
//
//	S_PlayTillDone
//
//==========================================================================

void S_PlayTillDone(const char *sound)
{
	GSoundDevice->PlaySoundTillDone(sound);
}

//==========================================================================
//
//	S_StopSound
//
//==========================================================================

void S_StopSound(int origin_id, int channel)
{
	GSoundDevice->StopSound(origin_id, channel);
}

//==========================================================================
//
//	S_StopAllSound
//
//==========================================================================

void S_StopAllSound(void)
{
	GSoundDevice->StopAllSound();
}

//==========================================================================
//
//	S_GetSoundPlayingInfo
//
//==========================================================================

boolean S_GetSoundPlayingInfo(int origin_id, int sound_id)
{
	return GSoundDevice->IsSoundPlaying(origin_id, sound_id);
}

//==========================================================================
//
//	S_StartSong
//
//==========================================================================

void S_StartSong(const char* song, int track, boolean loop)
{
	guard(S_StartSong);
	if (CDMusic)
	{
		if (loop)
			CmdBuf << "CD Loop " << track << "\n";
		else
			CmdBuf << "CD Play " << track << "\n";
	}
	else
	{
		if (loop)
			CmdBuf << "Music Loop " << song << "\n";
		else
			CmdBuf << "Music Play " << song << "\n";
	}
	unguard;
}

//==========================================================================
//
//	S_PauseSound
//
//==========================================================================

void S_PauseSound(void)
{
	guard(S_PauseSound);
	if (CDMusic)
	{
		CmdBuf << "CD Pause\n";
	}
	else
	{
		CmdBuf << "Music Pause\n";
	}
	unguard;
}

//==========================================================================
//
//	S_ResumeSound
//
//==========================================================================

void S_ResumeSound(void)
{
	guard(S_ResumeSound);
	if (CDMusic)
	{
		CmdBuf << "CD resume\n";
	}
	else
	{
		CmdBuf << "Music resume\n";
	}
	unguard;
}

//==========================================================================
//
//  StartMusic
//
//==========================================================================

static void StartMusic()
{
	S_StartSong(mapSong, mapCDTrack, true);
}

//==========================================================================
//
//	S_Start
//
//	Per level startup code. Kills playing sounds at start of level,
// determines music if any, changes music.
//
//==========================================================================

void S_Start()
{
	guard(S_Start);
	SN_StopAllSequences();
	S_StopAllSound();

	strcpy(mapSong, cl_level.songLump);
	mapCDTrack = cl_level.cdTrack;

	StartMusic();
	unguard;
}	

//==========================================================================
//
//	S_MusicChanged
//
//==========================================================================

void S_MusicChanged()
{
	guard(S_MusicChanged);
	strcpy(mapSong, cl_level.songLump);
	mapCDTrack = cl_level.cdTrack;

	StartMusic();
	unguard;
}

//==========================================================================
//
// S_UpdateSounds
//
// Updates music & sounds
//
//==========================================================================

void S_UpdateSounds(void)
{
	guard(S_UpdateSounds);
	if (cd_music && !CDMusic)
	{
		CmdBuf << "Music Stop\n";
		CDMusic = true;
		StartMusic();
	}
	if (!cd_music && CDMusic)
	{
		CmdBuf << "CD Stop\n";
		CDMusic = false;
		StartMusic();
	}

	// Update any Sequences
	SN_UpdateActiveSequences();

	GStreamMusicPlayer.Tick(host_frametime);
	GSoundDevice->Tick(host_frametime);
	GMidiDevice->Tick(host_frametime);
	GCDAudioDevice->Update();
	unguard;
}

//==========================================================================
//
//	FArchiveMemoryReader
//
//==========================================================================

class FArchiveMemoryReader : public FArchive
{
public:
	FArchiveMemoryReader(byte* InData, int InSize)
		: Data(InData), Pos(0), Size(InSize)
	{
		ArIsLoading = true;
		ArIsPersistent = true;
	}
	~FArchiveMemoryReader()
	{
		if (Data)
			Close();
	}
	void Serialise(void* V, int Length)
	{
		if (Length > Size - Pos)
		{
			ArIsError = true;
		}
		memcpy(V, Data + Pos, Length);
		Pos += Length;
	}
	int Tell()
	{
		return Pos;
	}
	int TotalSize()
	{
		return Size;
	}
	bool AtEnd()
	{
		return Pos >= Size;
	}
	void Seek(int InPos)
	{
		Pos = InPos;
	}
	bool Close()
	{
		Z_Free(Data);
		Data = NULL;
		return !ArIsError;
	}

protected:
	byte *Data;
	int Pos;
	int Size;
};

//==========================================================================
//
//	PlaySong
//
//==========================================================================

static void PlaySong(const char* Song, bool Loop)
{
	guard(PlaySong);
	if (!Song || !Song[0])
	{
		return;
	}
	if ((StreamPlaying && !stricmp(Song, *GStreamMusicPlayer.CurrSong) &&
		GStreamMusicPlayer.IsPlaying()) || (!StreamPlaying && !stricmp(Song,
		*GMidiDevice->CurrSong) && GMidiDevice->IsPlaying()))
	{
		// don't replay an old song if it is still playing
		return;
	}

	if (StreamPlaying)
		GStreamMusicPlayer.Stop();
	else
		GMidiDevice->Stop();
	StreamPlaying = false;

	//	Find the song.
	FArchive* StrmAr = FL_OpenFileRead(va("music/%s.ogg", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.mp3", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.wav", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.mid", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.mus", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.669", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.amf", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.dsm", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.far", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.gdm", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.imf", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.it", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.m15", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.med", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.mod", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.mtm", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.okt", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.s3m", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.stm", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.stx", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.ult", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.uni", Song));
	if (!StrmAr)
		StrmAr = FL_OpenFileRead(va("music/%s.xm", Song));
	if (!StrmAr)
	{
		if (W_CheckNumForName(Song) < 0)
		{
			GCon->Logf("Can't find song %s", Song);
			return;
		}
		StrmAr = W_CreateLumpReader(Song);
	}

	byte Hdr[4];
	StrmAr->Serialise(Hdr, 4);
	if (!memcmp(Hdr, MUSMAGIC, 4))
	{
		// convert mus to mid with a wanderfull function
		// thanks to S.Bacquet for the source of qmus2mid
		StrmAr->Seek(0);
		byte* Buf = (byte*)Z_Malloc(256 * 1024, PU_STATIC, 0);
		int MidLength = qmus2mid(*StrmAr, Buf);
		StrmAr->Close();
		delete StrmAr;
		if (!MidLength)
		{
			Z_Free(Buf);
			return;
		}
		StrmAr = new FArchiveMemoryReader(Buf, MidLength);
	}

	//	Try to create audio codec.
	VAudioCodec* Codec = NULL;
	for (FAudioCodecDesc* Desc = FAudioCodecDesc::List; Desc && !Codec; Desc = Desc->Next)
	{
		Codec = Desc->Creator(StrmAr);
	}

	if (Codec)
	{
		//	Start playing streamed music.
		GStreamMusicPlayer.Play(Codec, Song, Loop);
		StreamPlaying = true;
	}
	else
	{
		int Length = StrmAr->TotalSize();
		void* Data = Z_Malloc(Length, PU_MUSIC, NULL);
		StrmAr->Seek(0);
		StrmAr->Serialise(Data, Length);
		StrmAr->Close();
		delete StrmAr;

		if (!memcmp(Data, MIDIMAGIC, 4))
		{
			GMidiDevice->Play(Data, Length, Song, Loop);
		}
		else
		{
			GCon->Log("Not a MUS or MIDI file");
			Z_Free(Data);
		}
	}
	unguard;
}

//==========================================================================
//
//  COMMAND Music
//
//==========================================================================

COMMAND(Music)
{
	guard(COMMAND Music);
	if (!GMidiDevice->Initialised)
	{
		return;
	}

	if (Argc() < 2)
	{
		return;
	}

	const char* command = Argv(1);

	if (!stricmp(command, "on"))
	{
		GMidiDevice->Enabled = true;
		return;
	}

	if (!stricmp(command, "off"))
	{
		GMidiDevice->Stop();
		GStreamMusicPlayer.Stop();
		GMidiDevice->Enabled = false;
		return;
	}

	if (!GMidiDevice->Enabled)
	{
		return;
	}

	if (!stricmp(command, "play"))
	{
		PlaySong(*FName(Argv(2), FNAME_AddLower8), false);
		return;
	}

	if (!stricmp(command, "loop"))
	{
		PlaySong(*FName(Argv(2), FNAME_AddLower8), true);
		return;
	}

	if (!stricmp(command, "pause"))
	{
		if (StreamPlaying)
			GStreamMusicPlayer.Pause();
		else
			GMidiDevice->Pause();
		return;
	}

	if (!stricmp(command, "resume"))
	{
		if (StreamPlaying)
			GStreamMusicPlayer.Resume();
		else
			GMidiDevice->Resume();
		return;
	}

	if (!stricmp(command, "stop"))
	{
		if (StreamPlaying)
			GStreamMusicPlayer.Stop();
		else
			GMidiDevice->Stop();
		return;
	}

	if (!stricmp(command, "info"))
	{
		if (StreamPlaying && GStreamMusicPlayer.IsPlaying())
		{
			GCon->Logf("Currently %s %s.", GStreamMusicPlayer.CurrLoop ?
				"looping" : "playing", *GStreamMusicPlayer.CurrSong);
		}
		else if (!StreamPlaying && GMidiDevice->IsPlaying())
		{
			GCon->Logf("Currently %s %s.", GMidiDevice->CurrLoop ?
				"looping" : "playing", *GMidiDevice->CurrSong);
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
//	COMMAND CD
//
//==========================================================================

COMMAND(CD)
{
	guard(COMMAND CD);
	char	*command;

	if (!GCDAudioDevice->Initialised)
		return;

	if (Argc() < 2)
		return;

	command = Argv(1);

	if (!stricmp(command, "on"))
	{
		GCDAudioDevice->Enabled = true;
		return;
	}

	if (!stricmp(command, "off"))
	{
		if (GCDAudioDevice->Playing)
			GCDAudioDevice->Stop();
		GCDAudioDevice->Enabled = false;
		return;
	}

	if (!stricmp(command, "reset"))
	{
		int		n;

		GCDAudioDevice->Enabled = true;
		if (GCDAudioDevice->Playing)
			GCDAudioDevice->Stop();
		for (n = 0; n < 100; n++)
			GCDAudioDevice->Remap[n] = n;
		GCDAudioDevice->GetInfo();
		return;
	}

	if (!stricmp(command, "remap"))
	{
		int		n;
		int		ret;

		ret = Argc() - 2;
		if (ret <= 0)
		{
			for (n = 1; n < 100; n++)
				if (GCDAudioDevice->Remap[n] != n)
					GCon->Logf("%d -> %d", n, GCDAudioDevice->Remap[n]);
			return;
		}
		for (n = 1; n <= ret; n++)
			GCDAudioDevice->Remap[n] = atoi(Argv(n + 1));
		return;
	}

	if (!GCDAudioDevice->Enabled)
	{
		return;
	}

	if (!stricmp(command, "eject"))
	{
		if (GCDAudioDevice->Playing)
			GCDAudioDevice->Stop();
		GCDAudioDevice->OpenDoor();
		GCDAudioDevice->CDValid = false;
		return;
	}

	if (!stricmp(command, "close"))
	{
		GCDAudioDevice->CloseDoor();
		return;
	}

	if (!GCDAudioDevice->CDValid)
	{
		GCDAudioDevice->GetInfo();
		if (!GCDAudioDevice->CDValid)
		{
			GCon->Log("No CD in player.");
			return;
		}
	}

	if (!stricmp(command, "play"))
	{
		GCDAudioDevice->Play(atoi(Argv(2)), false);
		return;
	}

	if (!stricmp(command, "loop"))
	{
		GCDAudioDevice->Play(atoi(Argv(2)), true);
		return;
	}

	if (!stricmp(command, "pause"))
	{
		GCDAudioDevice->Pause();
		return;
	}

	if (!stricmp(command, "resume"))
	{
		GCDAudioDevice->Resume();
		return;
	}

	if (!stricmp(command, "stop"))
	{
		GCDAudioDevice->Stop();
		return;
	}

	if (!stricmp(command, "info"))
	{
		GCon->Logf("%d tracks", GCDAudioDevice->MaxTrack);
		if (GCDAudioDevice->Playing || GCDAudioDevice->WasPlaying)
		{
			GCon->Logf("%s %s track %d", GCDAudioDevice->Playing ?
				"Currently" : "Paused", GCDAudioDevice->PlayLooping ?
				"looping" : "playing", GCDAudioDevice->PlayTrack);
		}
		return;
	}
	unguard;
}

//**************************************************************************
//
//  Quick MUS->MID ! by S.Bacquet
//
//**************************************************************************

#define last(e) 			((byte)(e & 0x80))
#define event_type(e)		((byte)((e & 0x7F) >> 4))
#define channel(e)			((byte)(e & 0x0F))

#define TRACKBUFFERSIZE		65536  /* 64 Ko */

struct Track
{
	dword		current;
	char		vel;
	long		DeltaTime;
	byte		LastEvent;
	char*		data; 	   /* Primary data */
};

static byte*			mid_file;

static struct Track		tracks[32];
static word				TrackCnt = 0;
static int	 			MUS2MIDchannel[16];

static const byte		MUS2MIDcontrol[15] =
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
static const byte		track_end[] =
{
	0x00, 0xff, 47, 0x00
};
static const byte		midikey[] =
{
	0x00, 0xff, 0x59, 0x02, 0x00, 0x00   		// C major
};
static const byte		miditempo[] =
{
	0x00, 0xff, 0x51, 0x03, 0x09, 0xa3, 0x1a	// uS/qnote
};

//==========================================================================
//
//	FirstChannelAvailable
//
//==========================================================================

static int FirstChannelAvailable()
{
	int 	old15 = MUS2MIDchannel[15];
	int		max = -1;

	MUS2MIDchannel[15] = -1;
	for (int i = 0; i < 16; i++)
	{
		if (MUS2MIDchannel[i] > max)
		{
			max = MUS2MIDchannel[i];
		}
	}
	MUS2MIDchannel[15] = old15;

	return (max == 8 ? 10 : max + 1);
}

//==========================================================================
//
//	TWriteByte
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

static void TWriteBuf(int MIDItrack, const byte* buf, int size)
{
	for (int i = 0; i < size; i++)
	{
		TWriteByte(MIDItrack, buf[i]);
	}
}

//==========================================================================
//
//	TWriteVarLen
//
//==========================================================================

static void TWriteVarLen(int tracknum, dword value)
{
	dword buffer = value & 0x7f;
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
}

//==========================================================================
//
//	ReadTime
//
//==========================================================================

static dword ReadTime(FArchive& Ar)
{
	dword 		time = 0;
	byte		data;

	if (Ar.AtEnd())
		return 0;
	do
	{
		Ar << data;
		time = (time << 7) + (data & 0x7F);
	} while (!Ar.AtEnd() && (data & 0x80));

	return time;
}

//==========================================================================
//
//  convert
//
//==========================================================================

static bool convert(FArchive& Ar)
{
	byte				et;
	int					MUSchannel;
	int					MIDIchannel;
	int					MIDItrack = 0;
	int					NewEvent;
	int 				i;
	byte				event;
	byte				data;
	dword				DeltaTime;
	byte				MIDIchan2track[16];
	bool 				ouch = false;
	FMusHeader			MUSh;

	for (i = 0; i < 16; i++)
	{
		MUS2MIDchannel[i] = -1;
	}
	for (i = 0; i < 32; i++)
	{
		tracks[i].current = 0;
		tracks[i].vel = 64;
		tracks[i].DeltaTime = 0;
		tracks[i].LastEvent = 0;
		tracks[i].data = NULL;
	}

	Ar.Serialise(&MUSh, sizeof(FMusHeader));
	if (strncmp(MUSh.ID, MUSMAGIC, 4))
	{
		GCon->Log("Not a MUS file");
		return false;
	}

	if ((word)LittleShort(MUSh.NumChannels) > 15)	 /* <=> MUSchannels+drums > 16 */
	{
		GCon->Log(NAME_Dev,"Too many channels");
		return false;
	}

	Ar.Seek((word)LittleShort(MUSh.ScoreStart));

	tracks[0].data = (char*)Z_Malloc(TRACKBUFFERSIZE, PU_MUSIC, 0);
	TWriteBuf(0, midikey, 6);
	TWriteBuf(0, miditempo, 7);

	TrackCnt = 1;	//	Music starts here

	Ar << event;
	et = event_type(event);
	MUSchannel = channel(event);
	while ((et != 6) && !Ar.AtEnd())
	{
		if (MUS2MIDchannel[MUSchannel] == -1)
		{
			MIDIchannel = MUS2MIDchannel[MUSchannel] =
				(MUSchannel == 15 ? 9 : FirstChannelAvailable());
			MIDItrack = MIDIchan2track[MIDIchannel] = TrackCnt++;
			tracks[MIDItrack].data = (char*)Z_Malloc(TRACKBUFFERSIZE, PU_MUSIC, 0);
		}
		else
		{
			MIDIchannel = MUS2MIDchannel[MUSchannel];
			MIDItrack = MIDIchan2track[MIDIchannel];
		}
		TWriteVarLen(MIDItrack, tracks[MIDItrack].DeltaTime);
		tracks[MIDItrack].DeltaTime = 0;
		switch (et)
		{
		case 0:		/* release note */
			NewEvent = 0x90 | MIDIchannel;
			TWriteByte(MIDItrack, NewEvent);
			tracks[MIDItrack].LastEvent = NewEvent;
			Ar << data;
			TWriteByte(MIDItrack, data);
			TWriteByte(MIDItrack, 0);
			break;
		case 1:
			NewEvent = 0x90 | MIDIchannel;
			TWriteByte(MIDItrack, NewEvent);
			tracks[MIDItrack].LastEvent = NewEvent;
			Ar << data;
			TWriteByte(MIDItrack, data & 0x7F);
			if (data & 0x80)
			{
				Ar << data;
				tracks[MIDItrack].vel = data;
			}
			TWriteByte(MIDItrack, tracks[MIDItrack].vel);
			break;
		case 2:
			NewEvent = 0xE0 | MIDIchannel;
			TWriteByte(MIDItrack, NewEvent);
			tracks[MIDItrack].LastEvent = NewEvent;
			Ar << data;
			TWriteByte(MIDItrack, (data & 1) << 6);
			TWriteByte(MIDItrack, data >> 1);
			break;
		case 3:
			NewEvent = 0xB0 | MIDIchannel;
			TWriteByte(MIDItrack, NewEvent);
			tracks[MIDItrack].LastEvent = NewEvent;
			Ar << data;
			TWriteByte(MIDItrack, MUS2MIDcontrol[data]);
			if (data == 12)
				TWriteByte(MIDItrack, LittleShort(MUSh.NumChannels) + 1);
			else
				TWriteByte(MIDItrack, 0);
			break;
		case 4:
			Ar << data;
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
			Ar << data;
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
			DeltaTime = ReadTime(Ar);
			for (i = 0; i < (int)TrackCnt; i++)
				tracks[i].DeltaTime += DeltaTime;
		}
		if (!Ar.AtEnd())
		{
			Ar << event;
			et = event_type(event);
			MUSchannel = channel(event);
		}
		else
		{
			ouch = true;
		}
	}

	for (i = 0; i < TrackCnt; i++)
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

static void WriteBuf(const void* p, int size)
{
	memcpy(mid_file, p, size);
	mid_file += size;
}

//==========================================================================
//
//	WriteMIDIFile
//
//==========================================================================

static void WriteMIDIFile()
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

static void FreeTracks()
{
	for (int i = 0; i < 16; i++)
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

static int qmus2mid(FArchive& Ar, byte* mid)
{
	mid_file = mid;

	if (convert(Ar))
	{
		WriteMIDIFile();
	}
	FreeTracks();

	return mid_file - mid;
}

//**************************************************************************
//**************************************************************************

//==========================================================================
//
//	VStreamMusicPlayer::Init
//
//==========================================================================

void VStreamMusicPlayer::Init()
{
}

//==========================================================================
//
//	VStreamMusicPlayer::Shutdown
//
//==========================================================================

void VStreamMusicPlayer::Shutdown()
{
	guard(VStreamMusicPlayer::Shutdown);
	Stop();
	unguard;
}

//==========================================================================
//
//	VStreamMusicPlayer::Tick
//
//==========================================================================

void VStreamMusicPlayer::Tick(float)
{
	guard(VStreamMusicPlayer::Tick);
	if (!StrmOpened)
		return;
	if (Stopping && FinishTime + 1.0 < Sys_Time())
	{
		//	Finish playback.
		Stop();
		return;
	}
	for (int Len = GSoundDevice->GetStreamAvailable(); Len;
		Len = GSoundDevice->GetStreamAvailable())
	{
		short* Data = GSoundDevice->GetStreamBuffer();
		int StartPos = 0;
		while (!Stopping && StartPos < Len)
		{
			int SamplesDecoded = Codec->Decode(Data + StartPos * 2, Len - StartPos);
			StartPos += SamplesDecoded;
			if (Codec->Finished())
			{
				//	Stream ended.
				if (CurrLoop)
				{
					//	Restart stream.
					Codec->Restart();
				}
				else
				{
					//	We'll wait for 1 second to finish playing.
					Stopping = true;
					FinishTime = Sys_Time();
				}
			}
			else if (StartPos < Len)
			{
				//	Should never happen.
				GCon->Log("Stream decoded less but is not finished");
				Stopping = true;
				FinishTime = Sys_Time();
			}
		}
		if (Stopping)
		{
			memset(Data + StartPos * 2, 0, (Len - StartPos) * 4);
		}
		GSoundDevice->SetStreamData(Data, Len);
	}
	GSoundDevice->SetStreamVolume(music_volume);
	unguard;
}

//==========================================================================
//
//	VStreamMusicPlayer::Play
//
//==========================================================================

void VStreamMusicPlayer::Play(VAudioCodec* InCodec, const char* InName, bool InLoop)
{
	guard(VStreamMusicPlayer::Play);
	StrmOpened = GSoundDevice->OpenStream();
	if (!StrmOpened)
		return;
	Codec = InCodec;
	CurrSong = InName;
	CurrLoop = InLoop;
	Stopping = false;
	unguard;
}

//==========================================================================
//
//	VStreamMusicPlayer::Pause
//
//==========================================================================

void VStreamMusicPlayer::Pause()
{
	guard(VStreamMusicPlayer::Pause);
	if (!StrmOpened)
		return;
	GSoundDevice->PauseStream();
	unguard;
}

//==========================================================================
//
//	VStreamMusicPlayer::Resume
//
//==========================================================================

void VStreamMusicPlayer::Resume()
{
	guard(VStreamMusicPlayer::Resume);
	if (!StrmOpened)
		return;
	GSoundDevice->ResumeStream();
	unguard;
}

//==========================================================================
//
//	VStreamMusicPlayer::Stop
//
//==========================================================================

void VStreamMusicPlayer::Stop()
{
	guard(VStreamMusicPlayer::Stop);
	if (!StrmOpened)
		return;
	delete Codec;
	Codec = NULL;
	GSoundDevice->CloseStream();
	StrmOpened = false;
	unguard;
}

//==========================================================================
//
//	VStreamMusicPlayer::IsPlaying
//
//==========================================================================

bool VStreamMusicPlayer::IsPlaying()
{
	guard(VStreamMusicPlayer::IsPlaying);
	if (!StrmOpened)
		return false;
	return false;
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.19  2005/10/29 15:05:13  dj_jl
//	Added MikMod driver.
//
//	Revision 1.18  2005/10/28 17:50:01  dj_jl
//	Added Timidity driver.
//	
//	Revision 1.17  2005/10/22 11:30:07  dj_jl
//	Fixed looping of streams.
//	
//	Revision 1.16  2005/10/18 20:53:04  dj_jl
//	Implemented basic support for streamed music.
//	
//	Revision 1.15  2005/09/13 17:32:45  dj_jl
//	Created CD audio device class.
//	
//	Revision 1.14  2005/09/12 19:45:16  dj_jl
//	Created midi device class.
//	
//	Revision 1.13  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.12  2004/11/30 07:17:17  dj_jl
//	Made string pointers const.
//	
//	Revision 1.11  2004/08/21 19:10:44  dj_jl
//	Changed sound driver declaration.
//	
//	Revision 1.10  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//	
//	Revision 1.9  2003/03/08 12:10:13  dj_jl
//	API fixes.
//	
//	Revision 1.8  2002/07/27 18:10:11  dj_jl
//	Implementing Strife conversations.
//	
//	Revision 1.7  2002/07/23 13:12:00  dj_jl
//	Some compatibility fixes, beautification.
//	
//	Revision 1.6  2002/07/20 14:49:41  dj_jl
//	Implemented sound drivers.
//	
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
