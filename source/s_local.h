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
class VSoundDevice:public VSubsystem
{
	DECLARE_CLASS(VSoundDevice, VSubsystem, 0);
	NO_DEFAULT_CONSTRUCTOR(VSoundDevice);

	//	VSoundDevice interface.
	virtual void Init(void)
	{}
	virtual void Shutdown(void)
	{}
	virtual void PlaySound(int, const TVec &, const TVec &, int, int, float)
	{}
	virtual void PlaySoundTillDone(char *)
	{}
	virtual void StopSound(int, int)
	{}
	virtual void StopAllSound(void)
	{}
	virtual bool IsSoundPlaying(int, int)
	{ return false; }
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

//
//	Data
//
void S_InitScript(void);
bool S_LoadSound(int sound_id);
void S_DoneWithLump(int sound_id);

//
//  MUSIC I/O
//
void S_InitMusic(void);
void S_UpdateMusic(void);
void S_ShutdownMusic(void);

//
//	CD MUSIC
//
void CD_Init(void);
void CD_Update(void);
void CD_Shutdown(void);

//
//	EAX utilites
//
float EAX_CalcEnvSize(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern boolean		UseSndScript;
extern char			ArchivePath[128];

// the complete set of sound effects
extern TArray<sfxinfo_t>	S_sfx;

extern TCvarI		sfx_volume;
extern TCvarI		music_volume;
extern TCvarI		swap_stereo;

#endif

//**************************************************************************
//
//	$Log$
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
