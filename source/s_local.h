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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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

// TYPES -------------------------------------------------------------------

//
// SoundFX struct.
//
struct sfxinfo_t
{
	char	tagName[32];	// Name, by whitch sound is recognised in script
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

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

//
//	Data
//
void S_InitScript(void);
void S_LoadSound(int sound_id);
void S_DoneWithLump(int sound_id);

//
//  SFX I/O
//
void S_InitSfx(void);
void S_UpdateSfx(void);
void S_ShutdownSfx(void);

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

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern boolean		UseSndScript;
extern char			ArchivePath[128];

// the complete set of sound effects
extern int			NumSfx;
extern sfxinfo_t*	S_sfx;

extern TCvarI		sfx_volume;
extern TCvarI		music_volume;
extern TCvarI		swap_stereo;

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
