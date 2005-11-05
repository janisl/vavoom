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
void S_Init();
void S_Start();
void S_MusicChanged();
void S_StartSong(const char* name, int track, boolean loop);
void S_PauseSound();
void S_ResumeSound();
void S_UpdateSounds();
int S_GetSoundID(FName Name);
int S_GetSoundID(const char *name);
void S_Shutdown();

//
//	Sound playback
//
void S_StartSound(int, const TVec&, const TVec&, int, int, int);
inline void S_StartSound(int sound_id)
{
	S_StartSound(sound_id, TVec(0, 0, 0), TVec(0, 0, 0), 0, 0, 127);
}
void S_PlayTillDone(const char *);
void S_StopSound(int, int);
void S_StopAllSound(void);
boolean S_GetSoundPlayingInfo(int origin_id, int sound_id);

//
//	Sound sequences
//
void SN_InitSequenceScript(void);
void SN_StartSequence(int origin_id, const TVec &origin, int sequence);
void SN_StartSequenceName(int origin_id, const TVec &origin, const char *name);
void SN_StopSequence(int origin_id);
void SN_UpdateActiveSequences(void);
void SN_StopAllSequences(void);
int SN_GetSequenceOffset(int sequence, int *sequencePtr);
void SN_ChangeNodeData(int nodeNum, int seqOffset, int delayTics, int volume,
	int currentSoundID);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int			ActiveSequences;
extern seqnode_t	*SequenceListHead;

//**************************************************************************
//
//	$Log$
//	Revision 1.10  2005/11/05 15:50:07  dj_jl
//	Voices played as normal sounds.
//
//	Revision 1.9  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.8  2004/11/30 07:17:17  dj_jl
//	Made string pointers const.
//	
//	Revision 1.7  2002/07/27 18:10:11  dj_jl
//	Implementing Strife conversations.
//	
//	Revision 1.6  2002/01/11 08:15:06  dj_jl
//	Sound index retrieval by FName
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
