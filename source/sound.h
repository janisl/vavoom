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

struct sfxinfo_t;
struct seq_info_t;
struct VReverbInfo;

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
	bool IsSoundPresent(VName, VName, VName);
	bool LoadSound(int);
	void DoneWithLump(int);
	float GetMusicVolume(VName);
	FAmbientSound* GetAmbientSound(int);

	void SetSeqTrans(VName, int, int);
	VName GetSeqTrans(int, int);
	VName GetSeqSlot(VName);
	int FindSequence(VName);

	void GetSoundLumpNames(TArray<FReplacedString>&);
	void ReplaceSoundLumpNames(TArray<FReplacedString>&);

private:
	struct FPlayerSound
	{
		int		ClassId;
		int		GenderId;
		int		RefId;
		int		SoundId;
	};

	enum { NUM_AMBIENT_SOUNDS = 256 };

	struct VMusicVolume
	{
		VName		SongName;
		float		Volume;
	};

	TArray<VName>			PlayerClasses;
	TArray<VName>			PlayerGenders;
	TArray<FPlayerSound>	PlayerSounds;
	int						NumPlayerReserves;
	float					CurrentChangePitch;
	FAmbientSound*			AmbientSounds[NUM_AMBIENT_SOUNDS];
	TArray<VMusicVolume>	MusicVolumes;
	int						SeqTrans[64 * 3];
	VReverbInfo*			Environments;

	void ParseSndinfo(VScriptParser*);
	int AddSoundLump(VName, int);
	int AddSound(VName, int);
	int FindSound(VName);
	int FindOrAddSound(VName);
	void ParsePlayerSoundCommon(VScriptParser*, int&, int&, int&);
	int AddPlayerClass(VName);
	int FindPlayerClass(VName);
	int AddPlayerGender(VName);
	int FindPlayerGender(VName);
	int FindPlayerSound(int, int, int);
	int LookupPlayerSound(int, int, int);
	int ResolveSound(int, int, int);

	void ParseSequenceScript(VScriptParser*);
	void AssignSeqTranslations(VScriptParser*, int, seqtype_t);

	void ParseReverbs(VScriptParser*);
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
	virtual void PlaySound(int, const TVec&, const TVec&, int, int, float,
		float, bool) = 0;
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
	virtual void StartSequence(int, const TVec&, VName, int) = 0;
	virtual void AddSeqChoice(int, VName) = 0;
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
