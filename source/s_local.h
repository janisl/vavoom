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

#define MAX_CHANNELS		8

enum { VOICE_SOUND_ID = -1 };

//	Sound device types.
//??? Should Default be replaced with all default drivers?
enum
{
	SNDDRV_Default,
	SNDDRV_OpenAL,
	SNDDRV_Null,

	SNDDRV_MAX
};

//	Midi device types.
enum
{
	MIDIDRV_Default,
	MIDIDRV_Null,

	MIDIDRV_MAX
};

// TYPES -------------------------------------------------------------------

//
// SoundFX struct.
//
struct sfxinfo_t
{
	FName	tagName;		// Name, by whitch sound is recognised in script
	char	lumpname[12];	// Only need 9 bytes, but padded out to be dword aligned
	int		priority;		// Higher priority takes precendence
	int 	numchannels;	// total number of channels a sound type may occupy
	boolean	changePitch;
	void*	snd_ptr;
    int		lumpnum;        // lump number of sfx
    dword	freq;			// from sound lump
	dword	len;
	void*	data;			// points in snd_lump
	int		usecount;
};

//
//	VSoundDevice
//
//	Sound device interface. This class implements dummy driver.
//
class VSoundDevice
{
public:
	void* operator new(size_t Size, int Tag)
	{ return Z_Calloc(Size, Tag, 0); }
	void operator delete(void* Object, size_t)
	{ Z_Free(Object); }
	virtual ~VSoundDevice()
	{}

	//	VSoundDevice interface.
	virtual void Init()
	{}
	virtual void Shutdown()
	{}
	virtual void Tick(float)
	{}
	virtual void PlaySound(int, const TVec &, const TVec &, int, int, float)
	{}
	virtual void PlayVoice(const char *)
	{}
	virtual void PlaySoundTillDone(const char *)
	{}
	virtual void StopSound(int, int)
	{}
	virtual void StopAllSound(void)
	{}
	virtual bool IsSoundPlaying(int, int)
	{ return false; }
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
	bool		Enabled;
	//	Current playing song info.
	bool		CurrLoop;
	FName		CurrSong;

	void* operator new(size_t Size, int Tag)
	{ return Z_Calloc(Size, Tag, 0); }
	void operator delete(void* Object, size_t)
	{ Z_Free(Object); }
	VMidiDevice()
	: Initialised(false)
	, Enabled(false)
	, CurrLoop(false)
	{}
	virtual ~VMidiDevice()
	{}

	//	VMidiDevice interface.
	virtual void Init()
	{}
	virtual void Shutdown()
	{}
	virtual void Tick(float)
	{}
	virtual void Play(void*, int, const char*, bool)
	{}
	virtual void Pause()
	{}
	virtual void Resume()
	{}
	virtual void Stop()
	{}
	virtual bool IsPlaying()
	{ return false; }
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

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

//
//	Data
//
void S_InitScript();
bool S_LoadSound(int sound_id, const char *VoiceName = NULL);
void S_DoneWithLump(int sound_id);

//
//	CD MUSIC
//
void CD_Init();
void CD_Update();
void CD_Shutdown();

//
//	EAX utilites
//
float EAX_CalcEnvSize();

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern boolean				UseSndScript;
extern char					ArchivePath[128];

// the complete set of sound effects
extern TArray<sfxinfo_t>	S_sfx;
extern sfxinfo_t			S_VoiceInfo;

extern TCvarF				sfx_volume;
extern TCvarF				music_volume;
extern TCvarI				swap_stereo;

extern VSoundDevice			*GSoundDevice;
extern VMidiDevice			*GMidiDevice;

//**************************************************************************
//
//  Quick MUS->MID ! by S.Bacquet
//
//**************************************************************************

#define MUSMAGIC     		"MUS\032"
#define MIDIMAGIC    		"MThd"

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
	dword		hdr_size;//size;
	word		type;//file_type;
	word		num_tracks;
	word		divisions;
} MIDheader;//MIDI_Header_t;

#pragma pack()

int qmus2mid(const char *mus, char *mid, int length);

#endif

//**************************************************************************
//
//	$Log$
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
