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

#ifndef _S_LOCAL_H
#define _S_LOCAL_H

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

//	Sound device types.
//??? Should Default be replaced with all default drivers?
enum
{
	SNDDRV_Default,
	SNDDRV_OpenAL,

	SNDDRV_MAX
};

//	Midi device types.
enum
{
	MIDIDRV_Default,

	MIDIDRV_MAX
};

//	CD audio device types.
enum
{
	CDDRV_Default,

	CDDRV_MAX
};

// TYPES -------------------------------------------------------------------

//
//	VSoundDevice
//
//	Sound device interface. This class implements dummy driver.
//
class VSoundDevice
{
public:
	bool		Sound3D;

	VSoundDevice()
	: Sound3D(false)
	{}
	virtual ~VSoundDevice()
	{}

	//	VSoundDevice interface.
	virtual bool Init() = 0;
	virtual int SetChannels(int) = 0;
	virtual void Shutdown() = 0;
	virtual void Tick(float) = 0;
	virtual int PlaySound(int, float, float, float, bool) = 0;
	virtual int PlaySound3D(int, const TVec&, const TVec&, float, float, bool) = 0;
	virtual void UpdateChannel(int, float, float) = 0;
	virtual void UpdateChannel3D(int, const TVec&, const TVec&) = 0;
	virtual bool IsChannelPlaying(int) = 0;
	virtual void StopChannel(int) = 0;
	virtual void UpdateListener(const TVec&, const TVec&, const TVec&,
		const TVec&, const TVec&) = 0;

	virtual bool OpenStream(int, int, int) = 0;
	virtual void CloseStream() = 0;
	virtual int GetStreamAvailable() = 0;
	virtual short* GetStreamBuffer() = 0;
	virtual void SetStreamData(short*, int) = 0;
	virtual void SetStreamVolume(float) = 0;
	virtual void PauseStream() = 0;
	virtual void ResumeStream() = 0;
};

//	Describtion of a sound driver.
struct FSoundDeviceDesc
{
	const char*		Name;
	const char*		Description;
	const char*		CmdLineArg;
	VSoundDevice*	(*Creator)();

	FSoundDeviceDesc(int Type, const char* AName, const char* ADescription,
		const char* ACmdLineArg, VSoundDevice* (*ACreator)());
};

//	Sound device registration helper.
#define IMPLEMENT_SOUND_DEVICE(TClass, Type, Name, Description, CmdLineArg) \
VSoundDevice* Create##TClass() \
{ \
	return new TClass(); \
} \
FSoundDeviceDesc TClass##Desc(Type, Name, Description, CmdLineArg, Create##TClass);

//
//	VMidiDevice
//
//	Midi player device interface. This class implements dummy driver.
//
class VMidiDevice
{
public:
	bool		Initialised;
	//	Current playing song info.
	bool		CurrLoop;
	VName		CurrSong;

	VMidiDevice()
	: Initialised(false)
	, CurrLoop(false)
	{}
	virtual ~VMidiDevice()
	{}

	//	VMidiDevice interface.
	virtual void Init() = 0;
	virtual void Shutdown() = 0;
	virtual void SetVolume(float) = 0;
	virtual void Tick(float) = 0;
	virtual void Play(void*, int, const char*, bool) = 0;
	virtual void Pause() = 0;
	virtual void Resume() = 0;
	virtual void Stop() = 0;
	virtual bool IsPlaying() = 0;
};

//	Describtion of a midi driver.
struct FMidiDeviceDesc
{
	const char*		Name;
	const char*		Description;
	const char*		CmdLineArg;
	VMidiDevice*	(*Creator)();

	FMidiDeviceDesc(int Type, const char* AName, const char* ADescription,
		const char* ACmdLineArg, VMidiDevice* (*ACreator)());
};

//	Midi device registration helper.
#define IMPLEMENT_MIDI_DEVICE(TClass, Type, Name, Description, CmdLineArg) \
VMidiDevice* Create##TClass() \
{ \
	return new TClass(); \
} \
FMidiDeviceDesc TClass##Desc(Type, Name, Description, CmdLineArg, Create##TClass);

//
//	VCDAudioDevice
//
//	CD player device interface. This class implements dummy driver.
//
class VCDAudioDevice
{
public:
	bool		Initialised;
	bool		Enabled;
	bool		CDValid;
	bool		Playing;
	bool		WasPlaying;
	bool		PlayLooping;
	int			PlayTrack;
	int			MaxTrack;
	int		 	Remap[100];

	VCDAudioDevice()
	: Initialised(false)
	, Enabled(false)
	, CDValid(false)
	, Playing(false)
	, WasPlaying(false)
	, PlayLooping(false)
	, PlayTrack(0)
	, MaxTrack(0)
	{}
	virtual ~VCDAudioDevice()
	{}

	//	VCDAudioDevice interface.
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Shutdown() = 0;
	virtual void GetInfo() = 0;
	virtual void Play(int, bool) = 0;
	virtual void Pause() = 0;
	virtual void Resume() = 0;
	virtual void Stop() = 0;
	virtual void OpenDoor() = 0;
	virtual void CloseDoor() = 0;
};

//	Describtion of a CD driver.
struct FCDAudioDeviceDesc
{
	const char*		Name;
	const char*		Description;
	const char*		CmdLineArg;
	VCDAudioDevice*	(*Creator)();

	FCDAudioDeviceDesc(int Type, const char* AName, const char* ADescription,
		const char* ACmdLineArg, VCDAudioDevice* (*ACreator)());
};

//	CD audio device registration helper.
#define IMPLEMENT_CD_AUDIO_DEVICE(TClass, Type, Name, Description, CmdLineArg) \
VCDAudioDevice* Create##TClass() \
{ \
	return new TClass(); \
} \
FCDAudioDeviceDesc TClass##Desc(Type, Name, Description, CmdLineArg, Create##TClass);

//	Loader of sound samples.
class VSampleLoader
{
public:
	VSampleLoader*			Next;

	static VSampleLoader*	List;

	VSampleLoader()
	{
		Next = List;
		List = this;
	}
	virtual ~VSampleLoader()
	{}
	virtual void Load(sfxinfo_t&, VStream&) = 0;
};

//	Streamed audio decoder interface.
class VAudioCodec
{
public:
	int			SampleRate;
	int			SampleBits;
	int			NumChannels;

	VAudioCodec()
	: SampleRate(44100)
	, SampleBits(16)
	, NumChannels(2)
	{}
	virtual ~VAudioCodec()
	{}
	virtual int Decode(short*, int) = 0;
	virtual bool Finished() = 0;
	virtual void Restart() = 0;
};

//	Description of an audio codec.
struct FAudioCodecDesc
{
	const char*					Description;
	VAudioCodec*				(*Creator)(VStream*);
	FAudioCodecDesc*			Next;

	static FAudioCodecDesc*		List;

	FAudioCodecDesc(const char* InDescription, VAudioCodec* (*InCreator)(VStream*))
	: Description(InDescription)
	, Creator(InCreator)
	{
		Next = List;
		List = this;
	}
};

//	Audio codec registration helper.
#define IMPLEMENT_AUDIO_CODEC(TClass, Description) \
FAudioCodecDesc		TClass##Desc(Description, TClass::Create);

//	Quick MUS to MIDI converter.
class VQMus2Mid
{
private:
	struct VTrack
	{
		vint32				DeltaTime;
		vuint8				LastEvent;
		vint8				Vel;
		TArray<vuint8>		Data;	//  Primary data
	};

	VTrack					Tracks[32];
	vuint16					TrackCnt;
	vint32 					Mus2MidChannel[16];

	static const vuint8		Mus2MidControl[15];
	static const vuint8		TrackEnd[];
	static const vuint8		MidiKey[];
	static const vuint8		MidiTempo[];

	int FirstChannelAvailable();
	void TWriteByte(int, vuint8);
	void TWriteBuf(int, const vuint8*, int);
	void TWriteVarLen(int, vuint32);
	vuint32 ReadTime(VStream&);
	bool Convert(VStream&);
	void WriteMIDIFile(VStream&);
	void FreeTracks();

public:
	int Run(VStream&, VStream&);
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//**************************************************************************
//
//  MIDI and MUS file header structures.
//
//**************************************************************************

#define MUSMAGIC     		"MUS\032"
#define MIDIMAGIC    		"MThd"

#pragma pack(1)

struct FMusHeader
{
	char		ID[4];			// identifier "MUS" 0x1A
	word		ScoreSize;
	word		ScoreStart;
	word		NumChannels; 	// count of primary channels
	word		NumSecChannels;	// count of secondary channels (?)
	word		InstrumentCount;
	word		Dummy;
};

struct MIDheader
{
	char		ID[4];
	vuint32		hdr_size;
	word		type;
	word		num_tracks;
	word		divisions;
};

#pragma pack()

#endif
