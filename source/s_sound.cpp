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
#include "cl_local.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VSoundSeqNode
{
public:
	vint32			Sequence;
	vint32*			SequencePtr;
	vint32			OriginId;
	TVec			Origin;
	vint32			CurrentSoundID;
	float			DelayTime;
	float			Volume;
	float			Attenuation;
	vint32			StopSound;
	vuint32			DidDelayOnce;
	TArray<vint32>	SeqChoices;
	vint32			ModeNum;
	VSoundSeqNode*	Prev;
	VSoundSeqNode*	Next;
	VSoundSeqNode*	ParentSeq;
	VSoundSeqNode*	ChildSeq;

	VSoundSeqNode(int, const TVec&, int, int);
	~VSoundSeqNode();
	void Update(float);
	void Serialise(VStream&);
};

//
//	VAudio
//
//	Main audio management class.
//
class VAudio : public VAudioPublic
{
public:
	//	Sound sequence list
	int					ActiveSequences;
	VSoundSeqNode*		SequenceListHead;

	//	Structors.
	VAudio();
	~VAudio();

	//	Top level methods.
	void Init();
	void Shutdown();

	//	Playback of sound effects
	void PlaySound(int, const TVec&, const TVec&, int, int, float, float);
	void StopSound(int, int);
	void StopAllSound();
	bool IsSoundPlaying(int, int);

	//	Music and general sound control
	void StartSong(VName, int, bool);
	void PauseSound();
	void ResumeSound();
	void Start();
	void MusicChanged();
	void UpdateSounds();

	//	Sound sequences
	void StartSequence(int, const TVec&, VName, int);
	void AddSeqChoice(int, VName);
	void StopSequence(int);
	void UpdateActiveSequences(float);
	void StopAllSequences();
	void SerialiseSounds(VStream&);

	//	EAX utilites
	float EAX_CalcEnvSize();

private:
	enum { MAX_CHANNELS = 256 };

	enum { PRIORITY_MAX_ADJUST = 10 };

	//	Info about sounds currently playing.
	struct FChannel
	{
		int			origin_id;
		int			channel;
		TVec		origin;
		TVec		velocity;
		int			sound_id;
		int			priority;
		float		volume;
		float		Attenuation;
		int			handle;
		bool		is3D;
		bool		LocalPlayerSound;
	};

	//	Sound curve
	vuint8*				SoundCurve;
	int 				MaxSoundDist;

	//	Map's music lump and CD track
	VName				MapSong;
	int					MapCDTrack;
	float				MusicVolumeFactor;

	//	Wether we should use CD music
	bool				CDMusic;

	//	Stream music player
	bool				MusicEnabled;
	bool				StreamPlaying;
	VStreamMusicPlayer*	StreamMusicPlayer;

	//	List of currently playing sounds
	FChannel			Channel[MAX_CHANNELS];
	int					NumChannels;
	int 				SndCount;

	// maximum volume for sound
	float				MaxVolume;

	//	Listener orientation
	TVec				ListenerForward;
	TVec				ListenerRight;
	TVec				ListenerUp;

	//	Hardware devices
	VSoundDevice*		SoundDevice;
	VMidiDevice*		MidiDevice;
	VCDAudioDevice*		CDAudioDevice;

	//	Console variables
	static VCvarF		sfx_volume;
	static VCvarF		music_volume;
	static VCvarI		swap_stereo;
	static VCvarI		s_channels;
	static VCvarI		cd_music;
	static VCvarI		s_external_music;
	static VCvarF		eax_distance_unit;

	//	Friends
	friend class TCmdMusic;
	friend class TCmdCD;

	//	Sound effect helpers
	int GetChannel(int, int, int, int);
	void StopChannel(int);
	void UpdateSfx();

	//	Music playback
	void StartMusic();
	void PlaySong(const char*, bool);

	//	Execution of console commands
	void CmdMusic(const TArray<VStr>&);
	void CmdCD(const TArray<VStr>&);

	float CalcDirSize(const TVec&);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VAudioPublic*		GAudio;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

VCvarF				VAudio::sfx_volume("sfx_volume", "0.5", CVAR_Archive);
VCvarF				VAudio::music_volume("music_volume", "0.5", CVAR_Archive);
VCvarI				VAudio::swap_stereo("swap_stereo", "0", CVAR_Archive);
VCvarI				VAudio::s_channels("s_channels", "16", CVAR_Archive);
VCvarI				VAudio::cd_music("use_cd_music", "0", CVAR_Archive);
VCvarI				VAudio::s_external_music("s_external_music", "1", CVAR_Archive);
VCvarF				VAudio::eax_distance_unit("eax_distance_unit", "32.0", CVAR_Archive);

FAudioCodecDesc*	FAudioCodecDesc::List;

static FSoundDeviceDesc*	SoundDeviceList[SNDDRV_MAX];
static FMidiDeviceDesc*		MidiDeviceList[MIDIDRV_MAX];
static FCDAudioDeviceDesc*	CDAudioDeviceList[CDDRV_MAX];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VAudioPublic::Create
//
//==========================================================================

VAudioPublic* VAudioPublic::Create()
{
	return new VAudio();
}

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
{
	NoSoundClipping = false;
	ActiveSequences = 0;
	SequenceListHead = NULL;
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
	const TVec& velocity, int origin_id, int channel, float volume,
	float Attenuation)
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

	//	Check if this sound is emited by the local player.
	bool LocalPlayerSound = false;
	if (GClLevel && GClLevel->LevelInfo)
	{
		for (TThinkerIterator<VEntity> Ent(GClLevel); Ent; ++Ent)
		{
			if (Ent->NetID == origin_id &&
				(Ent->EntityFlags & VEntity::EF_NetLocalPlayer))
			{
				LocalPlayerSound = true;
			}
		}
		NoSoundClipping = !!(GClLevel->LevelInfo->LevelInfoFlags &
			VLevelInfo::LIF_NoSoundClipping);
	}

	// calculate the distance before other stuff so that we can throw out
	// sounds that are beyond the hearing range.
	int dist = 0;
	if (origin_id && !LocalPlayerSound && Attenuation > 0)
		dist = (int)(Length(origin - cl->ViewOrg) * Attenuation);
	if (dist >= MaxSoundDist && !NoSoundClipping)
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
		pitch = 1.0 + (Random() - Random()) *
			GSoundManager->S_sfx[sound_id].ChangePitch;
	}
	int handle;
	bool is3D;
	if (!origin_id || LocalPlayerSound || Attenuation <= 0)
	{
		//	Local sound
		handle = SoundDevice->PlaySound(sound_id, volume, 0, pitch, false);
		is3D = false;
	}
	else if (!SoundDevice->Sound3D)
	{
		float vol = SoundCurve[dist] / 127.0 * volume;
		float sep = DotProduct(origin - cl->ViewOrg, ListenerRight) /
			MaxSoundDist;
		if (swap_stereo)
		{
			sep = -sep;
		}
		handle = SoundDevice->PlaySound(sound_id, vol, sep, pitch, false);
		is3D = false;
	}
	else
	{
		handle = SoundDevice->PlaySound3D(sound_id, origin, velocity,
			volume, pitch, false);
		is3D = true;
	}
	Channel[chan].origin_id = origin_id;
	Channel[chan].channel = channel;
	Channel[chan].origin = origin;
	Channel[chan].velocity = velocity;
	Channel[chan].sound_id = sound_id;
	Channel[chan].priority = priority;
	Channel[chan].volume = volume;
	Channel[chan].Attenuation = Attenuation;
	Channel[chan].handle = handle;
	Channel[chan].is3D = is3D;
	Channel[chan].LocalPlayerSound = LocalPlayerSound;
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
//  VAudio::StartSequence
//
//==========================================================================

void VAudio::StartSequence(int OriginId, const TVec &Origin, VName Name,
	int ModeNum)
{
	guard(VAudio::StartSequence);
	int Idx = GSoundManager->FindSequence(Name);
	if (Idx != -1)
	{
		StopSequence(OriginId); // Stop any previous sequence
		new VSoundSeqNode(OriginId, Origin, Idx, ModeNum);
	}
	unguard;
}

//==========================================================================
//
//  VAudio::AddSeqChoice
//
//==========================================================================

void VAudio::AddSeqChoice(int OriginId, VName Name)
{
	guard(VAudio::AddSeqChoice);
	int Idx = GSoundManager->FindSequence(Name);
	if (Idx == -1)
	{
		return;
	}
	for (VSoundSeqNode* node = SequenceListHead; node; node = node->Next)
	{
		if (node->OriginId == OriginId)
		{
			node->SeqChoices.Append(Idx);
			return;
		}
	}
	unguard;
}

//==========================================================================
//
//  VAudio::StopSequence
//
//==========================================================================

void VAudio::StopSequence(int origin_id)
{
	guard(VAudio::StopSequence);
	for (VSoundSeqNode* node = SequenceListHead; node; node = node->Next)
	{
		if (node->OriginId == origin_id)
		{
			delete node;
		}
	}
	unguard;
}

//==========================================================================
//
//  VAudio::UpdateActiveSequences
//
//==========================================================================

void VAudio::UpdateActiveSequences(float DeltaTime)
{
	guard(VAudio::UpdateActiveSequences);
	if (!ActiveSequences || GClGame->ClientFlags & VClientGameBase::CF_Paused)
	{
		// No sequences currently playing/game is paused
		return;
	}
	for (VSoundSeqNode* node = SequenceListHead; node; node = node->Next)
	{
		node->Update(DeltaTime);
	}
	unguard;
}

//==========================================================================
//
//  VAudio::StopAllSequences
//
//==========================================================================

void VAudio::StopAllSequences()
{
	guard(VAudio::StopAllSequences);
	for (VSoundSeqNode* node = SequenceListHead; node; node = node->Next)
	{
		node->StopSound = 0; // don't play any stop sounds
		delete node;
	}
	unguard;
}

//==========================================================================
//
//	VAudio::SerialiseSounds
//
//==========================================================================

void VAudio::SerialiseSounds(VStream& Strm)
{
	guard(VAudio::SerialiseSounds);
	if (Strm.IsLoading())
	{
		// Reload and restart all sound sequences
		vint32 numSequences = Streamer<vint32>(Strm);
		for (int i = 0; i < numSequences; i++)
		{
			new VSoundSeqNode(0, TVec(0, 0, 0), -1, 0);
		}
		VSoundSeqNode* node = SequenceListHead;
		for (int i = 0; i < numSequences; i++, node = node->Next)
		{
			node->Serialise(Strm);
		}
	}
	else
	{
		// Save the sound sequences
		Strm << ActiveSequences;
		for (VSoundSeqNode* node = SequenceListHead; node; node = node->Next)
		{
			node->Serialise(Strm);
		}
	}
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

	if (cl)
	{
		AngleVectors(cl->ViewAngles, ListenerForward, ListenerRight, ListenerUp);
	}

	if (GClLevel && GClLevel->LevelInfo)
	{
		NoSoundClipping = !!(GClLevel->LevelInfo->LevelInfoFlags &
			VLevelInfo::LIF_NoSoundClipping);
	}

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
		if (!Channel[i].origin_id || Channel[i].Attenuation <= 0)
		{
			//	Full volume sound
			continue;
		}

		if (Channel[i].LocalPlayerSound)
		{
			//	Client sound
			continue;
		}

		//	Move sound
		Channel[i].origin += Channel[i].velocity * host_frametime;

		int dist = (int)(Length(Channel[i].origin - cl->ViewOrg) *
			Channel[i].Attenuation);
		if (dist >= MaxSoundDist && !NoSoundClipping)
		{
			//	Too far away
			StopChannel(i);
			continue;
		}

		//	Update params
		if (!Channel[i].is3D)
		{
			float vol = SoundCurve[dist] / 127.0 * Channel[i].volume;
			float sep = DotProduct(Channel[i].origin - cl->ViewOrg,
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

	if (SoundDevice->Sound3D && cl)
	{
		SoundDevice->UpdateListener(cl->ViewOrg, TVec(0, 0, 0),
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
	MapSong = GClLevel->LevelInfo->SongLump;
	MapCDTrack = GClLevel->LevelInfo->CDTrack;

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
		SoundDevice->SetStreamVolume(music_volume * MusicVolumeFactor);
		StreamMusicPlayer->Tick(host_frametime);
	}
	if (MidiDevice)
	{
		MidiDevice->SetVolume(music_volume * MusicVolumeFactor);
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
	static const char* Exts[] = { "ogg", "mp3", "wav", "mid", "mus", "669",
		"amf", "dsm", "far", "gdm", "imf", "it", "m15", "med", "mod", "mtm",
		"okt", "s3m", "stm", "stx", "ult", "uni", "xm", "flac", NULL };
	static const char* ExtraExts[] = { "ogg", "mp3", NULL };

	if (!Song || !Song[0])
	{
		return;
	}

	if (StreamPlaying)
		StreamMusicPlayer->Stop();
	else if (MidiDevice)
		MidiDevice->Stop();
	StreamPlaying = false;

	//	Get music volume for this song.
	MusicVolumeFactor = GSoundManager->GetMusicVolume(Song);

	//	Find the song.
	int Lump = -1;
	if (s_external_music)
	{
		//	Check external music definition file.
		VStream* XmlStrm = FL_OpenFileRead("extras/music/remap.xml");
		if (XmlStrm)
		{
			VXmlDocument* Doc = new VXmlDocument();
			Doc->Parse(*XmlStrm, "extras/music/remap.xml");
			delete XmlStrm;
			for (VXmlNode* N = Doc->Root.FirstChild; N; N = N->NextSibling)
			{
				if (N->Name != "song")
					continue;
				if (N->GetAttribute("name") != Song)
					continue;
				Lump = W_CheckNumForFileName(N->GetAttribute("file"));
				if (Lump >= 0)
					break;
			}
			delete Doc;
		}
		//	Also try OGG or MP3 directly.
		if (Lump < 0)
		{
			Lump = W_FindLumpByFileNameWithExts(va("extras/music/%s", Song),
				ExtraExts);
		}
	}
	if (Lump < 0)
	{
		int FileIdx = W_FindLumpByFileNameWithExts(va("music/%s", Song), Exts);
		int LumpIdx = W_CheckNumForName(VName(Song, VName::AddLower8), WADNS_Music);
		Lump = MAX(FileIdx, LumpIdx);
	}
	if (Lump < 0)
	{
		GCon->Logf("Can't find song %s", Song);
		return;
	}
	VStream* Strm = W_CreateLumpReaderNum(Lump);

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
			GCon->Log("Music file format is not supported");
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

	VStr command = Args[1].ToLower();

	if (command == "on")
	{
		MusicEnabled = true;
		return;
	}

	if (command == "off")
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

	if (command == "play")
	{
		if (Args.Num() < 3)
		{
			GCon->Log("Please enter name of the song.");
			return;
		}
		PlaySong(*VName(*Args[2], VName::AddLower8), false);
		return;
	}

	if (command == "loop")
	{
		if (Args.Num() < 3)
		{
			GCon->Log("Please enter name of the song.");
			return;
		}
		PlaySong(*VName(*Args[2], VName::AddLower8), true);
		return;
	}

	if (command == "pause")
	{
		if (StreamPlaying)
			StreamMusicPlayer->Pause();
		else if (MidiDevice)
			MidiDevice->Pause();
		return;
	}

	if (command == "resume")
	{
		if (StreamPlaying)
			StreamMusicPlayer->Resume();
		else if (MidiDevice)
			MidiDevice->Resume();
		return;
	}

	if (command == "stop")
	{
		if (StreamPlaying)
			StreamMusicPlayer->Stop();
		else if (MidiDevice)
			MidiDevice->Stop();
		return;
	}

	if (command == "info")
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
	if (!CDAudioDevice)
		return;

	if (Args.Num() < 2)
		return;

	VStr command = Args[1].ToLower();

	if (command == "on")
	{
		CDAudioDevice->Enabled = true;
		return;
	}

	if (command == "off")
	{
		if (CDAudioDevice->Playing)
			CDAudioDevice->Stop();
		CDAudioDevice->Enabled = false;
		return;
	}

	if (command == "reset")
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

	if (command == "remap")
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

	if (command == "eject")
	{
		if (CDAudioDevice->Playing)
			CDAudioDevice->Stop();
		CDAudioDevice->OpenDoor();
		CDAudioDevice->CDValid = false;
		return;
	}

	if (command == "close")
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

	if (command == "play")
	{
		if (Args.Num() < 2)
		{
			GCon->Log("Please enter CD track number");
			return;
		}
		CDAudioDevice->Play(atoi(*Args[2]), false);
		return;
	}

	if (command == "loop")
	{
		if (Args.Num() < 2)
		{
			GCon->Log("Please enter CD track number");
			return;
		}
		CDAudioDevice->Play(atoi(*Args[2]), true);
		return;
	}

	if (command == "pause")
	{
		CDAudioDevice->Pause();
		return;
	}

	if (command == "resume")
	{
		CDAudioDevice->Resume();
		return;
	}

	if (command == "stop")
	{
		CDAudioDevice->Stop();
		return;
	}

	if (command == "info")
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
//	VAudio::EAX_CalcEnvSize
//
//==========================================================================

float VAudio::EAX_CalcEnvSize()
{
	guard(VAudio::EAX_CalcEnvSize);
	if (cls.state != ca_connected)
	{
		return 7.5;
	}

	float len = 0;
	len += CalcDirSize(TVec(3200, 0, 0));
	len += CalcDirSize(TVec(0, 3200, 0));
	len += CalcDirSize(TVec(0, 0, 3200));
	return len / 3.0;
	unguard;
}

//==========================================================================
//
//	VAudio::CalcDirSize
//
//==========================================================================

float VAudio::CalcDirSize(const TVec &dir)
{
	guard(VAudio::CalcDirSize);
	linetrace_t Trace;
	GClLevel->TraceLine(Trace, cl->ViewOrg, cl->ViewOrg + dir, SPF_NOBLOCKSIGHT);
	float len = Length(Trace.LineEnd - cl->ViewOrg);
	GClLevel->TraceLine(Trace, cl->ViewOrg, cl->ViewOrg - dir, SPF_NOBLOCKSIGHT);
	len += Length(Trace.LineEnd - cl->ViewOrg);
	len /= eax_distance_unit;
	if (len > 100)
		len = 100;
	if (len < 1)
		len = 1;
	return len;
	unguard;
}

//==========================================================================
//
//  VSoundSeqNode::VSoundSeqNode
//
//==========================================================================

VSoundSeqNode::VSoundSeqNode(int AOriginId, const TVec& AOrigin,
	int ASequence, int AModeNum)
: Sequence(ASequence)
, OriginId(AOriginId)
, Origin(AOrigin)
, CurrentSoundID(0)
, DelayTime(0.0)
, Volume(1.0) // Start at max volume
, Attenuation(1.0)
, DidDelayOnce(0)
, ModeNum(AModeNum)
, Prev(NULL)
, Next(NULL)
, ParentSeq(NULL)
, ChildSeq(NULL)
{
	if (Sequence >= 0)
	{
		SequencePtr = GSoundManager->SeqInfo[Sequence].Data;
		StopSound = GSoundManager->SeqInfo[Sequence].StopSound;
	}

	//	Add to the list of sound sequences.
	if (!((VAudio*)GAudio)->SequenceListHead)
	{
		((VAudio*)GAudio)->SequenceListHead = this;
	}
	else
	{
		((VAudio*)GAudio)->SequenceListHead->Prev = this;
		Next = ((VAudio*)GAudio)->SequenceListHead;
		((VAudio*)GAudio)->SequenceListHead = this;
	}
	((VAudio*)GAudio)->ActiveSequences++;
}

//==========================================================================
//
//  VSoundSeqNode::~VSoundSeqNode
//
//==========================================================================

VSoundSeqNode::~VSoundSeqNode()
{
	if (ParentSeq && ParentSeq->ChildSeq == this)
	{
		//	Re-activate parent sequence.
		ParentSeq->SequencePtr++;
		ParentSeq->ChildSeq = NULL;
		ParentSeq = NULL;
	}

	if (ChildSeq)
	{
		delete ChildSeq;
	}

	//	Play stop sound.
	if (StopSound >= 0)
	{
		((VAudio*)GAudio)->StopSound(OriginId, 0);
	}
	if (StopSound >= 1)
	{
		((VAudio*)GAudio)->PlaySound(StopSound, Origin, TVec(0, 0, 0),
			OriginId, 1, Volume, Attenuation);
	}

	//	Remove from the list of active sound sequences.
	if (((VAudio*)GAudio)->SequenceListHead == this)
	{
		((VAudio*)GAudio)->SequenceListHead = Next;
	}
	if (Prev)
	{
		Prev->Next = Next;
	}
	if (Next)
	{
		Next->Prev = Prev;
	}
	((VAudio*)GAudio)->ActiveSequences--;
}

//==========================================================================
//
//  VSoundSeqNode::Update
//
//==========================================================================

void VSoundSeqNode::Update(float DeltaTime)
{
	guard(VSoundSeqNode::Update);
	if (DelayTime)
	{
		DelayTime -= DeltaTime;
		if (DelayTime <= 0.0)
		{
			DelayTime = 0.0;
		}
		return;
	}

	bool sndPlaying = GAudio->IsSoundPlaying(OriginId, CurrentSoundID);
	switch (*SequencePtr)
	{
	case SSCMD_None:
		SequencePtr++;
		break;

	case SSCMD_Play:
		if (!sndPlaying)
		{
			CurrentSoundID = SequencePtr[1];
			GAudio->PlaySound(CurrentSoundID, Origin, TVec(0, 0, 0),
				OriginId, 1, Volume, Attenuation);
		}
		SequencePtr += 2;
		break;

	case SSCMD_WaitUntilDone:
		if (!sndPlaying)
		{
			SequencePtr++;
			CurrentSoundID = 0;
		}
		break;

	case SSCMD_PlayRepeat:
		if (!sndPlaying)
		{
			CurrentSoundID = SequencePtr[1];
			GAudio->PlaySound(CurrentSoundID, Origin, TVec(0, 0, 0),
				OriginId, 1, Volume, Attenuation);
		}
		break;

	case SSCMD_PlayLoop:
		CurrentSoundID = SequencePtr[1];
		GAudio->PlaySound(CurrentSoundID, Origin, TVec(0, 0, 0), OriginId, 1,
			Volume, Attenuation);
		DelayTime = SequencePtr[2] / 35.0;
		break;

	case SSCMD_Delay:
		DelayTime = SequencePtr[1] / 35.0;
		SequencePtr += 2;
		CurrentSoundID = 0;
		break;

	case SSCMD_DelayOnce:
		if (!DidDelayOnce & (1 << SequencePtr[2]))
		{
			DidDelayOnce |= 1 << SequencePtr[2];
			DelayTime = SequencePtr[1] / 35.0;
			CurrentSoundID = 0;
		}
		SequencePtr += 3;
		break;

	case SSCMD_DelayRand:
		DelayTime = (SequencePtr[1] + rand() % (SequencePtr[2] -
			SequencePtr[1])) / 35.0;
		SequencePtr += 3;
		CurrentSoundID = 0;
		break;

	case SSCMD_Volume:
		Volume = SequencePtr[1] / 10000.0;
		SequencePtr += 2;
		break;

	case SSCMD_VolumeRel:
		Volume += SequencePtr[1] / 10000.0;
		SequencePtr += 2;
		break;

	case SSCMD_VolumeRand:
		Volume = (SequencePtr[1] + rand() % (SequencePtr[2] -
			SequencePtr[1])) / 10000.0;
		SequencePtr += 3;
		break;

	case SSCMD_Attenuation:
		Attenuation = SequencePtr[1];
		SequencePtr += 2;
		break;

	case SSCMD_RandomSequence:
		if (SeqChoices.Num() == 0)
		{
			SequencePtr++;
		}
		else if (!ChildSeq)
		{
			int Choice = rand() % SeqChoices.Num();
			ChildSeq = new VSoundSeqNode(OriginId, Origin, SeqChoices[Choice],
				ModeNum);
			ChildSeq->ParentSeq = this;
			ChildSeq->Volume = Volume;
			ChildSeq->Attenuation = Attenuation;
			return;
		}
		else
		{
			//	Waiting for child sequence to finish.
			return;
		}
		break;

	case SSCMD_Branch:
		SequencePtr -= SequencePtr[1];
		break;

	case SSCMD_Select:
		{
			//	Transfer sequence to the one matching the ModeNum.
			int NumChoices = SequencePtr[1];
			int i;
			for (i = 0; i < NumChoices; i++)
			{
				if (SequencePtr[2 + i * 2] == ModeNum)
				{
					int Idx = GSoundManager->FindSequence(
						*(VName*)&SequencePtr[3 + i * 2]);
					if (Idx != -1)
					{
						Sequence = Idx;
						SequencePtr = GSoundManager->SeqInfo[Sequence].Data;
						StopSound = GSoundManager->SeqInfo[Sequence].StopSound;
						break;
					}
				}
			}
			if (i == NumChoices)
			{
				//	Not found.
				SequencePtr += 2 + NumChoices;
			}
		}
		break;

	case SSCMD_StopSound:
		// Wait until something else stops the sequence
		break;

	case SSCMD_End:
		delete this;
		break;

	default:	
		break;
	}
	unguard;
}

//==========================================================================
//
//	VSoundSeqNode::Serialise
//
//==========================================================================

void VSoundSeqNode::Serialise(VStream& Strm)
{
	guard(VSoundSeqNode::Serialise);
	Strm << STRM_INDEX(Sequence)
		<< STRM_INDEX(OriginId)
		<< Origin
		<< STRM_INDEX(CurrentSoundID)
		<< DelayTime
		<< STRM_INDEX(DidDelayOnce)
		<< Volume
		<< Attenuation
		<< STRM_INDEX(ModeNum);

	if (Strm.IsLoading())
	{
		vint32 Offset;
		Strm << STRM_INDEX(Offset);
		SequencePtr = GSoundManager->SeqInfo[Sequence].Data + Offset;
		StopSound = GSoundManager->SeqInfo[Sequence].StopSound;

		vint32 Count;
		Strm << STRM_INDEX(Count);
		for (int i = 0; i < Count; i++)
		{
			VName SeqName;
			Strm << SeqName;
			SeqChoices.Append(GSoundManager->FindSequence(SeqName));
		}

		vint32 ParentSeqIdx;
		vint32 ChildSeqIdx;
		Strm << STRM_INDEX(ParentSeqIdx)
			<< STRM_INDEX(ChildSeqIdx);
		if (ParentSeqIdx != -1 || ChildSeqIdx != -1)
		{
			int i = 0;
			for (VSoundSeqNode* n = ((VAudio*)GAudio)->SequenceListHead;
				n; n = n->Next, i++)
			{
				if (ParentSeqIdx == i)
				{
					ParentSeq = n;
				}
				if (ChildSeqIdx == i)
				{
					ChildSeq = n;
				}
			}
		}
	}
	else
	{
		vint32 Offset = SequencePtr - GSoundManager->SeqInfo[Sequence].Data;
		Strm << STRM_INDEX(Offset);

		vint32 Count = SeqChoices.Num();
		Strm << STRM_INDEX(Count);
		for (int i = 0; i < SeqChoices.Num(); i++)
			Strm << GSoundManager->SeqInfo[SeqChoices[i]].Name;

		vint32 ParentSeqIdx = -1;
		vint32 ChildSeqIdx = -1;
		if (ParentSeq || ChildSeq)
		{
			int i = 0;
			for (VSoundSeqNode* n = ((VAudio*)GAudio)->SequenceListHead;
				n; n = n->Next, i++)
			{
				if (ParentSeq == n)
				{
					ParentSeqIdx = i;
				}
				if (ChildSeq == n)
				{
					ChildSeqIdx = i;
				}
			}
		}
		Strm << STRM_INDEX(ParentSeqIdx)
			<< STRM_INDEX(ChildSeqIdx);
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
//  COMMAND Music
//
//==========================================================================

COMMAND(Music)
{
	guard(COMMAND Music);
	((VAudio*)GAudio)->CmdMusic(Args);
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
	((VAudio*)GAudio)->CmdCD(Args);
	unguard;
}
