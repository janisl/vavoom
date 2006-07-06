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
	VName			CurrSong;
	bool			Stopping;
	double			FinishTime;
	VSoundDevice*	SoundDevice;

	VStreamMusicPlayer(VSoundDevice* InSoundDevice)
	: StrmOpened(false)
	, Codec(NULL)
	, CurrLoop(false)
	, Stopping(false)
	, SoundDevice(InSoundDevice)
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

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VAudio*				GAudio;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

VCvarF				VAudio::sfx_volume("sfx_volume", "0.5", CVAR_Archive);
VCvarF				VAudio::music_volume("music_volume", "0.5", CVAR_Archive);
VCvarI				VAudio::swap_stereo("swap_stereo", "0", CVAR_Archive);
VCvarI				VAudio::s_channels("s_channels", "16", CVAR_Archive);
VCvarI				VAudio::cd_music("use_cd_music", "0", CVAR_Archive);

FAudioCodecDesc*	FAudioCodecDesc::List;

static FSoundDeviceDesc*	SoundDeviceList[SNDDRV_MAX];
static FMidiDeviceDesc*		MidiDeviceList[MIDIDRV_MAX];
static FCDAudioDeviceDesc*	CDAudioDeviceList[CDDRV_MAX];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VAudio::VAudio
//
//==========================================================================

VAudio::VAudio()
: SoundCurve(NULL)
, MaxSoundDist(0)
, MapSong(NAME_None)
, MapCDTrack(0)
, CDMusic(false)
, MusicEnabled(true)
, StreamPlaying(false)
, StreamMusicPlayer(NULL)
, NumChannels(0)
, SndCount(0)
, MaxVolume(0)
, SoundDevice(NULL)
, MidiDevice(NULL)
, CDAudioDevice(NULL)
, ActiveSequences(0)
, SequenceListHead(NULL)
{
	memset(Channel, 0, sizeof(Channel));
}

//==========================================================================
//
//	VAudio::~VAudio
//
//==========================================================================

VAudio::~VAudio()
{
	Shutdown();
}

//==========================================================================
//
//	VAudio::Init
//
//	Initialises sound stuff, including volume
//	Sets channels, SFX and music volume, allocates channel buffer.
//
//==========================================================================

void VAudio::Init()
{
	guard(VAudio::Init);
	//	Initialise sound driver.
	int SIdx = -1;
	if (!GArgs.CheckParm("-nosound") && !GArgs.CheckParm("-nosfx"))
	{
		for (int i = 0; i < SNDDRV_MAX; i++)
		{
			if (!SoundDeviceList[i])
				continue;
			//	Default to first available non-null sound device.
			if (SIdx == -1)
				SIdx = i;
			//	Check for user selection.
			if (SoundDeviceList[i]->CmdLineArg &&
				GArgs.CheckParm(SoundDeviceList[i]->CmdLineArg))
				SIdx = i;
		}
	}
	if (SIdx != -1)
	{
		GCon->Logf(NAME_Init, "Selected %s", SoundDeviceList[SIdx]->Description);
		SoundDevice = SoundDeviceList[SIdx]->Creator();
		if (!SoundDevice->Init())
		{
			delete SoundDevice;
			SoundDevice = NULL;
		}
	}

	//	Initialise MIDI driver.
	int MIdx = -1;
	if (!GArgs.CheckParm("-nosound") && !GArgs.CheckParm("-nomusic"))
	{
		for (int i = 0; i < MIDIDRV_MAX; i++)
		{
			if (!MidiDeviceList[i])
				continue;
			//	Default to first available non-null midi device.
			if (MIdx == -1)
				MIdx = i;
			//	Check for user selection.
			if (MidiDeviceList[i]->CmdLineArg &&
				GArgs.CheckParm(MidiDeviceList[i]->CmdLineArg))
				MIdx = i;
		}
	}
	if (MIdx != -1)
	{
		GCon->Logf(NAME_Init, "Selected %s", MidiDeviceList[MIdx]->Description);
		MidiDevice = MidiDeviceList[MIdx]->Creator();
		MidiDevice->Init();
		if (!MidiDevice->Initialised)
		{
			delete MidiDevice;
			MidiDevice = NULL;
		}
	}

	//	Initialise CD audio driver.
	int CDIdx = -1;
	if (!GArgs.CheckParm("-nosound") && !GArgs.CheckParm("-nocdaudio"))
	{
		for (int i = 0; i < CDDRV_MAX; i++)
		{
			if (!CDAudioDeviceList[i])
				continue;
			//	Default to first available non-null CD audio device.
			if (CDIdx == -1)
				CDIdx = i;
			//	Check for user selection.
			if (CDAudioDeviceList[i]->CmdLineArg &&
				GArgs.CheckParm(CDAudioDeviceList[i]->CmdLineArg))
				CDIdx = i;
		}
	}
	if (CDIdx != -1)
	{
		GCon->Logf(NAME_Init, "Selected %s", CDAudioDeviceList[CDIdx]->Description);
		CDAudioDevice = CDAudioDeviceList[CDIdx]->Creator();
		CDAudioDevice->Init();
		if (!CDAudioDevice->Initialised)
		{
			delete CDAudioDevice;
			CDAudioDevice = NULL;
		}
	}

	//	Initialise stream music player.
	if (SoundDevice && !GArgs.CheckParm("-nomusic"))
	{
		StreamMusicPlayer = new VStreamMusicPlayer(SoundDevice);
		StreamMusicPlayer->Init();
	}

	int Lump = W_CheckNumForName(NAME_sndcurve);
	if (Lump >= 0)
	{
		VStream* Strm = W_CreateLumpReaderNum(Lump);
		MaxSoundDist = Strm->TotalSize();
		SoundCurve = new vuint8[MaxSoundDist];
		Strm->Serialise(SoundCurve, MaxSoundDist);
		delete Strm;
	}
	else
	{
		MaxSoundDist = 1200;
		SoundCurve = new vuint8[MaxSoundDist];
		for (int i = 0; i < MaxSoundDist; i++)
		{
			SoundCurve[i] = MIN(127, (MaxSoundDist - i) * 127 /
				(MaxSoundDist - 160));
		}
	}
	MaxVolume = -1;

	//	Free all channels for use.
	memset(Channel, 0, sizeof(Channel));
	NumChannels = SoundDevice ? SoundDevice->SetChannels(s_channels) : 0;
	unguard;
}

//==========================================================================
//
//	VAudio::Shutdown
//
//	Shuts down all sound stuff
//
//==========================================================================

void VAudio::Shutdown()
{
	guard(VAudio::Shutdown);
	//	Stop playback of all sounds.
	StopAllSequences();
	StopAllSound();

	if (StreamMusicPlayer)
	{
		StreamMusicPlayer->Shutdown();
		delete StreamMusicPlayer;
		StreamMusicPlayer = NULL;
	}
	if (CDAudioDevice)
	{
		CDAudioDevice->Shutdown();
		delete CDAudioDevice;
		CDAudioDevice = NULL;
	}
	if (MidiDevice)
	{
		MidiDevice->Shutdown();
		delete MidiDevice;
		MidiDevice = NULL;
	}
	if (SoundDevice)
	{
		SoundDevice->Shutdown();
		delete SoundDevice;
		SoundDevice = NULL;
	}
	if (SoundCurve)
	{
		delete[] SoundCurve;
		SoundCurve = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VAudio::PlaySound
//
// 	This function adds a sound to the list of currently active sounds, which
// is maintained as a given number of internal channels.
//
//==========================================================================

void VAudio::PlaySound(int InSoundId, const TVec& origin,
	const TVec& velocity, int origin_id, int channel, float volume)
{
	guard(VAudio::PlaySound);
	if (!SoundDevice || !InSoundId || !MaxVolume || !volume)
	{
		return;
	}

	//	Find actual sound ID to use.
	int sound_id = GSoundManager->ResolveSound(InSoundId);

	//	Apply sound volume.
	volume *= MaxVolume;

	// calculate the distance before other stuff so that we can throw out
	// sounds that are beyond the hearing range.
	int dist = 0;
	if (origin_id && origin_id != cl->clientnum + 1)
		dist = (int)Length(origin - cl->vieworg);
	if (dist >= MaxSoundDist)
	{
		return; // sound is beyond the hearing range...
	}

	int priority = GSoundManager->S_sfx[sound_id].Priority *
		(PRIORITY_MAX_ADJUST - PRIORITY_MAX_ADJUST * dist / MaxSoundDist);

	int chan = GetChannel(sound_id, origin_id, channel, priority);
	if (chan == -1)
	{
		return; //no free channels.
	}

	float pitch = 1.0;
	if (GSoundManager->S_sfx[sound_id].ChangePitch)
	{
		pitch = 1.0 + (Random() - Random()) * GSoundManager->S_sfx[sound_id].ChangePitch;
	}
	int handle;
	bool is3D;
	if (!origin_id || origin_id == cl->clientnum + 1)
	{
		//	Local sound
		handle = SoundDevice->PlaySound(sound_id, volume, 0, pitch, false);
		is3D = false;
	}
	else if (!SoundDevice->Sound3D)
	{
		float vol = SoundCurve[dist] / 127.0 * volume;
		float sep = DotProduct(origin - cl->vieworg, ListenerRight) / MaxSoundDist;
		if (swap_stereo)
		{
			sep = -sep;
		}
		handle = SoundDevice->PlaySound(sound_id, vol, sep, pitch, false);
		is3D = false;
	}
	else
	{
		handle = SoundDevice->PlaySound3D(sound_id, origin, velocity, volume, pitch, false);
		is3D = true;
	}
	Channel[chan].origin_id = origin_id;
	Channel[chan].channel = channel;
	Channel[chan].origin = origin;
	Channel[chan].velocity = velocity;
	Channel[chan].sound_id = sound_id;
	Channel[chan].priority = priority;
	Channel[chan].volume = volume;
	Channel[chan].handle = handle;
	Channel[chan].is3D = is3D;
	unguard;
}

//==========================================================================
//
//	VAudio::GetChannel
//
//==========================================================================

int VAudio::GetChannel(int sound_id, int origin_id, int channel, int priority)
{
	guard(VAudio::GetChannel);
	int 		chan;
	int			i;
	int			lp; //least priority
	int			found;
	int			prior;
	int numchannels = GSoundManager->S_sfx[sound_id].NumChannels;

	if (numchannels > 0)
	{
		lp = -1; //denote the argument sound_id
		found = 0;
		prior = priority;
		for (i = 0; i < NumChannels; i++)
		{
			if (Channel[i].sound_id == sound_id)
			{
				if (GSoundManager->S_sfx[sound_id].bSingular)
				{
					// This sound is already playing, so don't start it again.
					return -1;
				}
				found++; //found one.  Now, should we replace it??
				if (prior >= Channel[i].priority)
				{
					// if we're gonna kill one, then this'll be it
					lp = i;
					prior = Channel[i].priority;
				}
			}
		}

		if (found >= numchannels)
		{
			if (lp == -1)
			{// other sounds have greater priority
				return -1; // don't replace any sounds
			}
			StopChannel(lp);
		}
	}

	//	Mobjs can have only one sound
	if (origin_id && channel)
    {
		for (i = 0; i < NumChannels; i++)
		{
			if (Channel[i].origin_id == origin_id &&
				Channel[i].channel == channel)
			{
				// only allow other mobjs one sound
				StopChannel(i);
				return i;
			}
		}
	}

	//	Look for a free channel
	for (i = 0; i < NumChannels; i++)
	{
		if (!Channel[i].sound_id)
		{
			return i;
		}
	}

	//	Look for a lower priority sound to replace.
	SndCount++;
	if (SndCount >= NumChannels)
	{
		SndCount = 0;
	}

	for (chan = 0; chan < NumChannels; chan++)
	{
		i = (SndCount + chan) % NumChannels;
		if (priority >= Channel[i].priority)
		{
			//replace the lower priority sound.
			StopChannel(i);
			return i;
		}
	}

    //	no free channels.
	return -1;
	unguard;
}

//==========================================================================
//
//	VAudio::StopChannel
//
//==========================================================================

void VAudio::StopChannel(int chan_num)
{
	guard(VAudio::StopChannel);
	if (Channel[chan_num].sound_id)
	{
		SoundDevice->StopChannel(Channel[chan_num].handle);
		Channel[chan_num].handle = -1;
		Channel[chan_num].origin_id = 0;
		Channel[chan_num].sound_id = 0;
	}
	unguard;
}

//==========================================================================
//
//	VAudio::StopSound
//
//==========================================================================

void VAudio::StopSound(int origin_id, int channel)
{
	guard(VAudio::StopSound);
	for (int i = 0; i < NumChannels; i++)
	{
		if (Channel[i].origin_id == origin_id &&
			(!channel || Channel[i].channel == channel))
		{
			StopChannel(i);
		}
	}
	unguard;
}

//==========================================================================
//
//	VAudio::StopAllSound
//
//==========================================================================

void VAudio::StopAllSound()
{
	guard(VAudio::StopAllSound);
	//	stop all sounds
	for (int i = 0; i < NumChannels; i++)
	{
		StopChannel(i);
	}
	unguard;
}

//==========================================================================
//
//	VAudio::IsSoundPlaying
//
//==========================================================================

bool VAudio::IsSoundPlaying(int origin_id, int InSoundId)
{
	guard(VAudio::IsSoundPlaying);
	int sound_id = GSoundManager->ResolveSound(InSoundId);
	for (int i = 0; i < NumChannels; i++)
	{
		if (Channel[i].sound_id == sound_id &&
			Channel[i].origin_id == origin_id &&
			SoundDevice->IsChannelPlaying(Channel[i].handle))
		{
			return true;
		}
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VAudio::UpdateSfx
//
// 	Update the sound parameters. Used to control volume and pan
// changes such as when a player turns.
//
//==========================================================================

void VAudio::UpdateSfx()
{
	guard(VAudio::UpdateSfx);
	if (!SoundDevice || !NumChannels)
	{
		return;
	}

	if (sfx_volume != MaxVolume)
    {
	    MaxVolume = sfx_volume;
		if (!MaxVolume)
		{
			StopAllSound();
		}
    }

	if (!MaxVolume)
	{
		//	Silence
		return;
	}

	AngleVectors(cl->viewangles, ListenerForward, ListenerRight, ListenerUp);

	for (int i = 0; i < NumChannels; i++)
	{
		if (!Channel[i].sound_id)
		{
			//	Nothing on this channel
			continue;
		}
		if (!SoundDevice->IsChannelPlaying(Channel[i].handle))
		{
			//	Playback done
			StopChannel(i);
			continue;
		}
		if (!Channel[i].origin_id)
		{
			//	Full volume sound
			continue;
		}

		if (Channel[i].origin_id == cl->clientnum + 1)
		{
			//	Client sound
			continue;
		}

		//	Move sound
		Channel[i].origin += Channel[i].velocity * host_frametime;

		int dist = (int)Length(Channel[i].origin - cl->vieworg);
		if (dist >= MaxSoundDist)
		{
			//	Too far away
			StopChannel(i);
			continue;
		}

		//	Update params
		if (!Channel[i].is3D)
		{
			float vol = SoundCurve[dist] / 127.0 * Channel[i].volume;
			float sep = DotProduct(Channel[i].origin - cl->vieworg,
				ListenerRight) / MaxSoundDist;
			if (swap_stereo)
			{
				sep = -sep;
			}
			SoundDevice->UpdateChannel(Channel[i].handle, vol, sep);
		}
		else
		{
			SoundDevice->UpdateChannel3D(Channel[i].handle,
				Channel[i].origin, Channel[i].velocity);
		}
		Channel[i].priority = GSoundManager->S_sfx[Channel[i].sound_id].Priority *
			(PRIORITY_MAX_ADJUST - PRIORITY_MAX_ADJUST * dist / MaxSoundDist);
	}

	if (SoundDevice->Sound3D)
	{
		SoundDevice->UpdateListener(cl->vieworg, TVec(0, 0, 0),
			ListenerForward, ListenerRight, ListenerUp);
	}

	SoundDevice->Tick(host_frametime);
	unguard;
}

//==========================================================================
//
//	VAudio::StartSong
//
//==========================================================================

void VAudio::StartSong(VName song, int track, bool loop)
{
	guard(VAudio::StartSong);
	if (CDMusic)
	{
		if (loop)
			GCmdBuf << "CD Loop " << VStr(track) << "\n";
		else
			GCmdBuf << "CD Play " << VStr(track) << "\n";
	}
	else
	{
		if (loop)
			GCmdBuf << "Music Loop " << *song << "\n";
		else
			GCmdBuf << "Music Play " << *song << "\n";
	}
	unguard;
}

//==========================================================================
//
//	VAudio::PauseSound
//
//==========================================================================

void VAudio::PauseSound()
{
	guard(VAudio::PauseSound);
	if (CDMusic)
	{
		GCmdBuf << "CD Pause\n";
	}
	else
	{
		GCmdBuf << "Music Pause\n";
	}
	unguard;
}

//==========================================================================
//
//	VAudio::ResumeSound
//
//==========================================================================

void VAudio::ResumeSound()
{
	guard(VAudio::ResumeSound);
	if (CDMusic)
	{
		GCmdBuf << "CD resume\n";
	}
	else
	{
		GCmdBuf << "Music resume\n";
	}
	unguard;
}

//==========================================================================
//
//  VAudio::StartMusic
//
//==========================================================================

void VAudio::StartMusic()
{
	StartSong(MapSong, MapCDTrack, true);
}

//==========================================================================
//
//	VAudio::Start
//
//	Per level startup code. Kills playing sounds at start of level,
// determines music if any, changes music.
//
//==========================================================================

void VAudio::Start()
{
	guard(VAudio::Start);
	StopAllSequences();
	StopAllSound();

	MapSong = cl_level.SongLump;
	MapCDTrack = cl_level.cdTrack;

	StartMusic();
	unguard;
}	

//==========================================================================
//
//	VAudio::MusicChanged
//
//==========================================================================

void VAudio::MusicChanged()
{
	guard(VAudio::MusicChanged);
	MapSong = cl_level.SongLump;
	MapCDTrack = cl_level.cdTrack;

	StartMusic();
	unguard;
}

//==========================================================================
//
//	VAudio::UpdateSounds
//
//	Updates music & sounds
//
//==========================================================================

void VAudio::UpdateSounds()
{
	guard(VAudio::UpdateSounds);
	//	Check sound volume.
	if (sfx_volume < 0.0)
	{
		sfx_volume = 0.0;
	}
	if (sfx_volume > 1.0)
	{
		sfx_volume = 1.0;
	}

	//	Check music volume.
	if (music_volume < 0.0)
	{
		music_volume = 0.0;
	}
	if (music_volume > 1.0)
	{
		music_volume = 1.0;
	}

	//	Check for CD music change.
	if (cd_music && !CDMusic)
	{
		GCmdBuf << "Music Stop\n";
		CDMusic = true;
		StartMusic();
	}
	if (!cd_music && CDMusic)
	{
		GCmdBuf << "CD Stop\n";
		CDMusic = false;
		StartMusic();
	}

	// Update any Sequences
	UpdateActiveSequences(host_frametime);

	UpdateSfx();
	if (StreamMusicPlayer)
	{
		SoundDevice->SetStreamVolume(music_volume);
		StreamMusicPlayer->Tick(host_frametime);
	}
	if (MidiDevice)
	{
		MidiDevice->SetVolume(music_volume);
		MidiDevice->Tick(host_frametime);
	}
	if (CDAudioDevice)
	{
		CDAudioDevice->Update();
	}
	unguard;
}

//==========================================================================
//
//	VAudio::PlaySong
//
//==========================================================================

void VAudio::PlaySong(const char* Song, bool Loop)
{
	guard(VAudio::PlaySong);
	if (!Song || !Song[0])
	{
		return;
	}

	if (StreamPlaying)
		StreamMusicPlayer->Stop();
	else if (MidiDevice)
		MidiDevice->Stop();
	StreamPlaying = false;

	//	Find the song.
	VStream* Strm = FL_OpenFileRead(va("music/%s.ogg", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.mp3", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.wav", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.mid", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.mus", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.669", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.amf", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.dsm", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.far", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.gdm", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.imf", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.it", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.m15", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.med", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.mod", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.mtm", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.okt", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.s3m", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.stm", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.stx", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.ult", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.uni", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.xm", Song));
	if (!Strm)
		Strm = FL_OpenFileRead(va("music/%s.flac", Song));
	if (!Strm)
	{
		int Lump = W_CheckNumForName(VName(Song, VName::AddLower8));
		if (Lump < 0)
		{
			GCon->Logf("Can't find song %s", Song);
			return;
		}
		Strm = W_CreateLumpReaderNum(Lump);
	}

	byte Hdr[4];
	Strm->Serialise(Hdr, 4);
	if (!memcmp(Hdr, MUSMAGIC, 4))
	{
		// convert mus to mid with a wanderfull function
		// thanks to S.Bacquet for the source of qmus2mid
		Strm->Seek(0);
		VMemoryStream* MidStrm = new VMemoryStream();
		MidStrm->BeginWrite();
		VQMus2Mid Conv;
		int MidLength = Conv.Run(*Strm, *MidStrm);
		delete Strm;
		if (!MidLength)
		{
			delete MidStrm;
			return;
		}
		MidStrm->Seek(0);
		MidStrm->BeginRead();
		Strm = MidStrm;
	}

	//	Try to create audio codec.
	VAudioCodec* Codec = NULL;
	for (FAudioCodecDesc* Desc = FAudioCodecDesc::List; Desc && !Codec; Desc = Desc->Next)
	{
		Codec = Desc->Creator(Strm);
	}

	if (StreamMusicPlayer && Codec)
	{
		//	Start playing streamed music.
		StreamMusicPlayer->Play(Codec, Song, Loop);
		StreamPlaying = true;
	}
	else if (MidiDevice)
	{
		int Length = Strm->TotalSize();
		void* Data = Z_Malloc(Length);
		Strm->Seek(0);
		Strm->Serialise(Data, Length);
		Strm->Close();
		delete Strm;

		if (!memcmp(Data, MIDIMAGIC, 4))
		{
			MidiDevice->Play(Data, Length, Song, Loop);
		}
		else
		{
			GCon->Log("Not a MUS or MIDI file");
			Z_Free(Data);
		}
	}
	else
	{
		delete Strm;
	}
	unguard;
}

//==========================================================================
//
//  VAudio::CmdMusic
//
//==========================================================================

void VAudio::CmdMusic(const TArray<VStr>& Args)
{
	guard(VAudio::CmdMusic);
	if (!MidiDevice && !StreamMusicPlayer)
	{
		return;
	}

	if (Args.Num() < 2)
	{
		return;
	}

	const char* command = *Args[1];

	if (!stricmp(command, "on"))
	{
		MusicEnabled = true;
		return;
	}

	if (!stricmp(command, "off"))
	{
		if (MidiDevice)
			MidiDevice->Stop();
		if (StreamMusicPlayer)
			StreamMusicPlayer->Stop();
		MusicEnabled = false;
		return;
	}

	if (!MusicEnabled)
	{
		return;
	}

	if (!stricmp(command, "play"))
	{
		if (Args.Num() < 3)
		{
			GCon->Log("Please enter name of the song.");
			return;
		}
		PlaySong(*VName(*Args[2], VName::AddLower8), false);
		return;
	}

	if (!stricmp(command, "loop"))
	{
		if (Args.Num() < 3)
		{
			GCon->Log("Please enter name of the song.");
			return;
		}
		PlaySong(*VName(*Args[2], VName::AddLower8), true);
		return;
	}

	if (!stricmp(command, "pause"))
	{
		if (StreamPlaying)
			StreamMusicPlayer->Pause();
		else if (MidiDevice)
			MidiDevice->Pause();
		return;
	}

	if (!stricmp(command, "resume"))
	{
		if (StreamPlaying)
			StreamMusicPlayer->Resume();
		else if (MidiDevice)
			MidiDevice->Resume();
		return;
	}

	if (!stricmp(command, "stop"))
	{
		if (StreamPlaying)
			StreamMusicPlayer->Stop();
		else if (MidiDevice)
			MidiDevice->Stop();
		return;
	}

	if (!stricmp(command, "info"))
	{
		if (StreamPlaying && StreamMusicPlayer->IsPlaying())
		{
			GCon->Logf("Currently %s %s.", StreamMusicPlayer->CurrLoop ?
				"looping" : "playing", *StreamMusicPlayer->CurrSong);
		}
		else if (MidiDevice && !StreamPlaying && MidiDevice->IsPlaying())
		{
			GCon->Logf("Currently %s %s.", MidiDevice->CurrLoop ?
				"looping" : "playing", *MidiDevice->CurrSong);
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
//	VAudio::CmdCD
//
//==========================================================================

void VAudio::CmdCD(const TArray<VStr>& Args)
{
	guard(VAudio::CmdCD);
	const char*		command;

	if (!CDAudioDevice)
		return;

	if (Args.Num() < 2)
		return;

	command = *Args[1];

	if (!stricmp(command, "on"))
	{
		CDAudioDevice->Enabled = true;
		return;
	}

	if (!stricmp(command, "off"))
	{
		if (CDAudioDevice->Playing)
			CDAudioDevice->Stop();
		CDAudioDevice->Enabled = false;
		return;
	}

	if (!stricmp(command, "reset"))
	{
		int		n;

		CDAudioDevice->Enabled = true;
		if (CDAudioDevice->Playing)
			CDAudioDevice->Stop();
		for (n = 0; n < 100; n++)
			CDAudioDevice->Remap[n] = n;
		CDAudioDevice->GetInfo();
		return;
	}

	if (!stricmp(command, "remap"))
	{
		int		n;
		int		ret;

		ret = Args.Num() - 2;
		if (ret <= 0)
		{
			for (n = 1; n < 100; n++)
				if (CDAudioDevice->Remap[n] != n)
					GCon->Logf("%d -> %d", n, CDAudioDevice->Remap[n]);
			return;
		}
		for (n = 1; n <= ret; n++)
			CDAudioDevice->Remap[n] = atoi(*Args[n + 1]);
		return;
	}

	if (!CDAudioDevice->Enabled)
	{
		return;
	}

	if (!stricmp(command, "eject"))
	{
		if (CDAudioDevice->Playing)
			CDAudioDevice->Stop();
		CDAudioDevice->OpenDoor();
		CDAudioDevice->CDValid = false;
		return;
	}

	if (!stricmp(command, "close"))
	{
		CDAudioDevice->CloseDoor();
		return;
	}

	if (!CDAudioDevice->CDValid)
	{
		CDAudioDevice->GetInfo();
		if (!CDAudioDevice->CDValid)
		{
			GCon->Log("No CD in player.");
			return;
		}
	}

	if (!stricmp(command, "play"))
	{
		if (Args.Num() < 2)
		{
			GCon->Log("Please enter CD track number");
			return;
		}
		CDAudioDevice->Play(atoi(*Args[2]), false);
		return;
	}

	if (!stricmp(command, "loop"))
	{
		if (Args.Num() < 2)
		{
			GCon->Log("Please enter CD track number");
			return;
		}
		CDAudioDevice->Play(atoi(*Args[2]), true);
		return;
	}

	if (!stricmp(command, "pause"))
	{
		CDAudioDevice->Pause();
		return;
	}

	if (!stricmp(command, "resume"))
	{
		CDAudioDevice->Resume();
		return;
	}

	if (!stricmp(command, "stop"))
	{
		CDAudioDevice->Stop();
		return;
	}

	if (!stricmp(command, "info"))
	{
		GCon->Logf("%d tracks", CDAudioDevice->MaxTrack);
		if (CDAudioDevice->Playing || CDAudioDevice->WasPlaying)
		{
			GCon->Logf("%s %s track %d", CDAudioDevice->Playing ?
				"Currently" : "Paused", CDAudioDevice->PlayLooping ?
				"looping" : "playing", CDAudioDevice->PlayTrack);
		}
		return;
	}
	unguard;
}

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
	for (int Len = SoundDevice->GetStreamAvailable(); Len;
		Len = SoundDevice->GetStreamAvailable())
	{
		short* Data = SoundDevice->GetStreamBuffer();
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
		SoundDevice->SetStreamData(Data, Len);
	}
	unguard;
}

//==========================================================================
//
//	VStreamMusicPlayer::Play
//
//==========================================================================

void VStreamMusicPlayer::Play(VAudioCodec* InCodec, const char* InName,
	bool InLoop)
{
	guard(VStreamMusicPlayer::Play);
	StrmOpened = SoundDevice->OpenStream(InCodec->SampleRate,
		InCodec->SampleBits, InCodec->NumChannels);
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
	SoundDevice->PauseStream();
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
	SoundDevice->ResumeStream();
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
	SoundDevice->CloseStream();
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

//==========================================================================
//
//  COMMAND Music
//
//==========================================================================

COMMAND(Music)
{
	guard(COMMAND Music);
	GAudio->CmdMusic(Args);
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
	GAudio->CmdCD(Args);
	unguard;
}
