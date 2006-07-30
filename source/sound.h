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

//
//	VSoundManager
//
//	Handles list of registered sound and sound sequences.
//
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

//
//	VAudio
//
//	Main audio management class.
//
class VAudioPublic : public VVirtualObjectBase
{
public:
	//	Top level methods.
	virtual void Init() = 0;
	virtual void Shutdown() = 0;

	//	Playback of sound effects
	virtual void PlaySound(int, const TVec&, const TVec&, int, int, float) = 0;
	virtual void StopSound(int, int) = 0;
	virtual void StopAllSound() = 0;
	virtual bool IsSoundPlaying(int, int) = 0;

	//	Music and general sound control
	virtual void StartSong(VName, int, bool) = 0;
	virtual void PauseSound() = 0;
	virtual void ResumeSound() = 0;
	virtual void Start() = 0;
	virtual void MusicChanged() = 0;
	virtual void UpdateSounds() = 0;

	//	Sound sequences
	virtual void StartSequenceName(int, const TVec&, const char*) = 0;
	virtual void StopSequence(int) = 0;
	virtual void UpdateActiveSequences(float) = 0;
	virtual void StopAllSequences() = 0;
	virtual void SerialiseSounds(VStream&) = 0;

	//	EAX utilites
	virtual float EAX_CalcEnvSize() = 0;

	static VAudioPublic* Create();
};

extern VSoundManager*		GSoundManager;
extern VAudioPublic*		GAudio;
