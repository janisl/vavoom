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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum seqtype_t
{
	SEQ_Door,
	SEQ_Platform,
	SEQ_Environment,
};

//
// SoundFX struct.
//
struct sfxinfo_t
{
	VName	TagName;		// Name, by whitch sound is recognised in script
	int		LumpNum;        // lump number of sfx

	int		Priority;		// Higher priority takes precendence
	int 	NumChannels;	// total number of channels a sound type may occupy
	float	ChangePitch;
	int		UseCount;
	int		Link;
	int*	Sounds;			// For random sounds, Link is count.

	bool	bRandomHeader;
	bool	bPlayerReserve;
	bool	bSingular;

	vuint32	SampleRate;
	int		SampleBits;
	vuint32	DataSize;
	void*	Data;
};

enum ESSCmds
{
	SSCMD_None,
	SSCMD_Play,
	SSCMD_WaitUntilDone, // used by PLAYUNTILDONE
	SSCMD_PlayTime,
	SSCMD_PlayRepeat,
	SSCMD_PlayLoop,
	SSCMD_Delay,
	SSCMD_DelayRand,
	SSCMD_Volume,
	SSCMD_StopSound,
	SSCMD_Attenuation,
	SSCMD_End
};

struct seq_info_t
{
	VName		Name;
	vint32*		Data;
	vint32		StopSound;
};

class VSoundManager
{
public:
	//	The complete set of sound effects
	TArray<sfxinfo_t>	S_sfx;
	TArray<seq_info_t>	SeqInfo;

	VSoundManager();
	~VSoundManager();
	void Init();
	int GetSoundID(VName);
	int GetSoundID(const char*);
	int ResolveSound(int);
	int ResolveEntitySound(VName, VName, VName);
	bool LoadSound(int);
	void DoneWithLump(int);

	void SetSeqTrans(VName, int, int);
	VName GetSeqTrans(int, int);

private:
	struct FPlayerSound
	{
		int		ClassId;
		int		GenderId;
		int		RefId;
		int		SoundId;
	};

	enum ESoundType
	{
		SNDTYPE_World = 0,
		SNDTYPE_Point = 1,
		SNDTYPE_Surround =2,

		SNDTYPE_Continuous = 4,
		SNDTYPE_Random = 8,
		SNDTYPE_Periodic = 12,
	};

	struct FAmbientSound
	{
		vuint32		Type;		// type of ambient sound
		float		PeriodMin;	// # of tics between repeats
		float		PeriodMax;	// max # of tics for random ambients
		float		Volume;		// relative volume of sound
		float		Attenuation;
		VName		Sound;		// Logical name of sound to play
	};

	enum { NUM_AMBIENT_SOUNDS = 256 };

	TArray<VName>			PlayerClasses;
	TArray<VName>			PlayerGenders;
	TArray<FPlayerSound>	PlayerSounds;
	int						NumPlayerReserves;
	float					CurrentChangePitch;
	FAmbientSound*			AmbientSounds[NUM_AMBIENT_SOUNDS];
	int						SeqTrans[64 * 3];

	static const char*		Attenuations[];

	void ParseSndinfo();
	int AddSoundLump(VName, int);
	int AddSound(VName, int);
	int FindSound(VName);
	int FindOrAddSound(VName);
	void ParsePlayerSoundCommon(int&, int&, int&);
	int AddPlayerClass(VName);
	int FindPlayerClass(VName);
	int AddPlayerGender(VName);
	int FindPlayerGender(VName);
	int FindPlayerSound(int, int, int);
	int LookupPlayerSound(int, int, int);
	int ResolveSound(int, int, int);

	void ParseSequenceScript();
	void AssignSeqTranslations(int, seqtype_t);
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

//
//	Main
//
void S_Init();
void S_Start();
void S_MusicChanged();
void S_StartSong(VName name, int track, boolean loop);
void S_PauseSound();
void S_ResumeSound();
void S_UpdateSounds();
void S_Shutdown();

//
//	Sound playback
//
void S_StartSound(int, const TVec&, const TVec&, int, int, int);
inline void S_StartSound(int sound_id)
{
	S_StartSound(sound_id, TVec(0, 0, 0), TVec(0, 0, 0), 0, 0, 127);
}
void S_StopSound(int, int);
void S_StopAllSound(void);
boolean S_GetSoundPlayingInfo(int origin_id, int sound_id);

//
//	Sound sequences
//
void SN_StartSequence(int origin_id, const TVec &origin, int sequence);
void SN_StartSequenceName(int origin_id, const TVec &origin, const char *name);
void SN_StopSequence(int origin_id);
void SN_UpdateActiveSequences();
void SN_StopAllSequences();
void SN_SerialiseSounds(VStream& Strm);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern VSoundManager*		GSoundManager;
