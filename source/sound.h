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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

#define S_StartSoundName(name)	S_StartSound(S_GetSoundID(name))

// TYPES -------------------------------------------------------------------

struct seqnode_t
{
	int			*sequencePtr;
	int			sequence;
	int			origin_id;
	TVec		origin;
	int			currentSoundID;
	int			delayTics;
	int			volume;
	int			stopSound;
	seqnode_t	*prev;
	seqnode_t	*next;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

//
//	Main
//
void S_Init(void);
void S_Start(const mapInfo_t &info);
void S_StartSong(char* name, int track, boolean loop);
void S_PauseSound(void);
void S_ResumeSound(void);
void S_UpdateSounds(void);
int S_GetSoundID(char *name);
void S_Shutdown(void);

//
//	Sound playback
//
void S_StartSound(int sound_id, const TVec &origin = TVec(0, 0, 0),
	const TVec &velocity = TVec(0, 0, 0), int origin_id = 0, int volume = 127);
void S_PlayTillDone(char *sound);
void S_StopSound(int origin_id);
void S_StopAllSound(void);
boolean S_GetSoundPlayingInfo(int origin_id, int sound_id);

//
//	Sound sequences
//
void SN_InitSequenceScript(void);
void SN_StartSequence(int origin_id, const TVec &origin, int sequence);
void SN_StartSequenceName(int origin_id, const TVec &origin, char *name);
void SN_StopSequence(int origin_id);
void SN_UpdateActiveSequences(void);
void SN_StopAllSequences(void);
int SN_GetSequenceOffset(int sequence, int *sequencePtr);
void SN_ChangeNodeData(int nodeNum, int seqOffset, int delayTics, int volume,
	int currentSoundID);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int			ActiveSequences;
extern seqnode_t	*SequenceListHead;

