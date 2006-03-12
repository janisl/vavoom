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

	dword	SampleRate;
	int		SampleBits;
	dword	DataSize;
	void*	Data;
};

//
//	VSoundDevice
//
//	Sound device interface. This class implements dummy driver.
//
class VSoundDevice
{
public:
	bool		Sound3D;

	void* operator new(size_t Size, int Tag)
	{ return Z_Calloc(Size, Tag, 0); }
	void operator delete(void* Object, size_t)
	{ Z_Free(Object); }
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
	return new(PU_STATIC) TClass(); \
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

	void* operator new(size_t Size, int Tag)
	{ return Z_Calloc(Size, Tag, 0); }
	void operator delete(void* Object, size_t)
	{ Z_Free(Object); }
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
	return new(PU_STATIC) TClass(); \
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

	void* operator new(size_t Size, int Tag)
	{ return Z_Calloc(Size, Tag, 0); }
	void operator delete(void* Object, size_t)
	{ Z_Free(Object); }
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
	return new(PU_STATIC) TClass(); \
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

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

//
//	Data
//
int S_ResolveSound(int);
bool S_LoadSound(int);
void S_DoneWithLump(int);

//
//	EAX utilites
//
float EAX_CalcEnvSize();

// PUBLIC DATA DECLARATIONS ------------------------------------------------

// the complete set of sound effects
extern TArray<sfxinfo_t>	S_sfx;

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
	dword		hdr_size;
	word		type;
	word		num_tracks;
	word		divisions;
};

#pragma pack()

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.27  2006/03/12 12:54:49  dj_jl
//	Removed use of bitfields for portability reasons.
//
//	Revision 1.26  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//	
//	Revision 1.25  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//	
//	Revision 1.24  2005/11/20 12:38:50  dj_jl
//	Implemented support for sound sequence extensions.
//	
//	Revision 1.23  2005/11/17 18:53:21  dj_jl
//	Implemented support for sndinfo extensions.
//	
//	Revision 1.22  2005/11/13 14:36:22  dj_jl
//	Moved common sound functions to main sound module.
//	
//	Revision 1.21  2005/11/06 15:27:09  dj_jl
//	Added support for 16 bit sounds.
//	
//	Revision 1.20  2005/11/05 15:50:07  dj_jl
//	Voices played as normal sounds.
//	
//	Revision 1.19  2005/11/03 22:46:35  dj_jl
//	Support for any bitrate streams.
//	
//	Revision 1.18  2005/10/28 17:50:01  dj_jl
//	Added Timidity driver.
//	
//	Revision 1.17  2005/10/20 22:31:27  dj_jl
//	Removed Hexen's devsnd support.
//	
//	Revision 1.16  2005/10/18 20:53:04  dj_jl
//	Implemented basic support for streamed music.
//	
//	Revision 1.15  2005/09/19 23:00:19  dj_jl
//	Streaming support.
//	
//	Revision 1.14  2005/09/13 17:32:45  dj_jl
//	Created CD audio device class.
//	
//	Revision 1.13  2005/09/12 19:45:16  dj_jl
//	Created midi device class.
//	
//	Revision 1.12  2004/11/30 07:17:17  dj_jl
//	Made string pointers const.
//	
//	Revision 1.11  2004/08/21 19:10:44  dj_jl
//	Changed sound driver declaration.
//	
//	Revision 1.10  2003/03/08 12:08:04  dj_jl
//	Beautification.
//	
//	Revision 1.9  2002/07/27 18:10:11  dj_jl
//	Implementing Strife conversations.
//	
//	Revision 1.8  2002/07/23 13:12:00  dj_jl
//	Some compatibility fixes, beautification.
//	
//	Revision 1.7  2002/07/20 14:49:41  dj_jl
//	Implemented sound drivers.
//	
//	Revision 1.6  2002/01/11 08:11:05  dj_jl
//	Changes in sound list
//	Added guard macros
//	
//	Revision 1.5  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.4  2001/08/29 17:55:42  dj_jl
//	Added sound channels
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
