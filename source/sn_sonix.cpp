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

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

#define SS_MAX_SCRIPTS			64
#define SS_TEMPBUFFER_SIZE		1024
#define SS_SEQUENCE_NAME_LENGTH	32

#define SS_SCRIPT_NAME			"SNDSEQ"

#define SS_STRING_PLAY			"play"
#define SS_STRING_PLAYUNTILDONE "playuntildone"
#define SS_STRING_PLAYTIME		"playtime"
#define SS_STRING_PLAYREPEAT	"playrepeat"
#define SS_STRING_DELAY			"delay"
#define SS_STRING_DELAYRAND		"delayrand"
#define SS_STRING_VOLUME		"volume"
#define SS_STRING_END			"end"
#define SS_STRING_STOPSOUND		"stopsound"

// TYPES -------------------------------------------------------------------

typedef enum
{
	SS_CMD_NONE,
	SS_CMD_PLAY,
	SS_CMD_WAITUNTILDONE, // used by PLAYUNTILDONE
	SS_CMD_PLAYTIME,
	SS_CMD_PLAYREPEAT,
	SS_CMD_DELAY,
	SS_CMD_DELAYRAND,
	SS_CMD_VOLUME,
	SS_CMD_STOPSOUND,
	SS_CMD_END
} sscmds_t;

typedef struct
{
	char	name[SS_SEQUENCE_NAME_LENGTH];
	int		*data;
	int		stopSound;
} seq_info_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void VerifySequencePtr(int *base, int *ptr);
static int GetSoundOffset(char *name);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int			ActiveSequences;
seqnode_t	*SequenceListHead;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static seq_info_t	SeqInfo[SS_MAX_SCRIPTS];
static int			NumSequences;

// CODE --------------------------------------------------------------------

//==========================================================================
//
// VerifySequencePtr
//
//   Verifies the integrity of the temporary ptr, and ensures that the ptr
// 		isn't exceeding the size of the temporary buffer
//==========================================================================

static void VerifySequencePtr(int *base, int *ptr)
{
	if (ptr-base > SS_TEMPBUFFER_SIZE)
	{
		Sys_Error("VerifySequencePtr:  tempPtr >= %d\n", SS_TEMPBUFFER_SIZE);
	}
}

//==========================================================================
//
// GetSoundOffset
//
//==========================================================================

static int GetSoundOffset(char *name)
{
	int i;
	
	for (i = 0; i < NumSfx; i++)
	{
		if (!stricmp(name, S_sfx[i].tagName))
		{
			return i;
		}
	}
	SC_ScriptError("GetSoundOffset:  Unknown sound name\n");
	return 0;
}

//==========================================================================
//
// SN_InitSequenceScript
//
//==========================================================================

void SN_InitSequenceScript(void)
{
	boolean		inSequence;
	int 		*tempDataStart = NULL;
	int 		*tempDataPtr = NULL;

	inSequence = false;
	ActiveSequences = 0;
	NumSequences = 0;
	memset(SeqInfo, 0, sizeof(SeqInfo));
	SC_Open(SS_SCRIPT_NAME);
	while (SC_GetString())
	{
		if (*sc_String == ':')
		{
			if (inSequence)
			{
				SC_ScriptError("SN_InitSequenceScript:  Nested Script Error");
			}
			if (NumSequences == SS_MAX_SCRIPTS)
			{
				SC_ScriptError("Number of SS Scripts >= SS_MAX_SCRIPTS");
			}
			tempDataStart = (int *)Z_Malloc(SS_TEMPBUFFER_SIZE, 
				PU_STATIC, NULL);
			memset(tempDataStart, 0, SS_TEMPBUFFER_SIZE);
			tempDataPtr = tempDataStart;
            inSequence = true;
            strcpy(SeqInfo[NumSequences].name, sc_String + 1);
			continue; // parse the next command
		}
		if (!inSequence)
		{
        	SC_ScriptError("String outside sequence");
			continue;
		}
		if (SC_Compare(SS_STRING_PLAYUNTILDONE))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			SC_MustGetString();
			*tempDataPtr++ = SS_CMD_PLAY;
			*tempDataPtr++ = GetSoundOffset(sc_String);
			*tempDataPtr++ = SS_CMD_WAITUNTILDONE;		
		}
		else if (SC_Compare(SS_STRING_PLAY))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			SC_MustGetString();
			*tempDataPtr++ = SS_CMD_PLAY;
			*tempDataPtr++ = GetSoundOffset(sc_String);
		}
		else if (SC_Compare(SS_STRING_PLAYTIME))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			SC_MustGetString();
			*tempDataPtr++ = SS_CMD_PLAY;
			*tempDataPtr++ = GetSoundOffset(sc_String);
			SC_MustGetNumber();
			*tempDataPtr++ = SS_CMD_DELAY;	
			*tempDataPtr++ = sc_Number;
		}
		else if (SC_Compare(SS_STRING_PLAYREPEAT))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			SC_MustGetString();
			*tempDataPtr++ = SS_CMD_PLAYREPEAT;
			*tempDataPtr++ = GetSoundOffset(sc_String);
		}
		else if (SC_Compare(SS_STRING_DELAY))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			*tempDataPtr++ = SS_CMD_DELAY;
			SC_MustGetNumber();
			*tempDataPtr++ = sc_Number;
		}
		else if (SC_Compare(SS_STRING_DELAYRAND))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			*tempDataPtr++ = SS_CMD_DELAYRAND;
			SC_MustGetNumber();
			*tempDataPtr++ = sc_Number;
			SC_MustGetNumber();
			*tempDataPtr++ = sc_Number;
		}
		else if (SC_Compare(SS_STRING_VOLUME))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			*tempDataPtr++ = SS_CMD_VOLUME;
			SC_MustGetNumber();
			*tempDataPtr++ = sc_Number;
		}
		else if (SC_Compare(SS_STRING_END))
		{
			int dataSize;

			*tempDataPtr++ = SS_CMD_END;
			dataSize = (tempDataPtr-tempDataStart)*sizeof(int);
			SeqInfo[NumSequences].data = (int *)Z_Malloc(dataSize, PU_STATIC,
				NULL);
			memcpy(SeqInfo[NumSequences].data, tempDataStart, dataSize);
			Z_Free(tempDataStart);
			inSequence = false;
			NumSequences++;
		}
		else if (SC_Compare(SS_STRING_STOPSOUND))
		{
			SC_MustGetString();
			SeqInfo[NumSequences].stopSound = GetSoundOffset(sc_String);
			*tempDataPtr++ = SS_CMD_STOPSOUND;
		}
		else
		{
			SC_ScriptError("SN_InitSequenceScript:  Unknown commmand.\n");
		}
	}
}

//==========================================================================
//
//  SN_StartSequence
//
//==========================================================================

void SN_StartSequence(int origin_id, const TVec &origin, int sequence)
{
	seqnode_t *node;

	SN_StopSequence(origin_id); // Stop any previous sequence
	node = (seqnode_t *)Z_Malloc(sizeof(seqnode_t), PU_STATIC, NULL);
	node->sequencePtr = SeqInfo[sequence].data;
	node->sequence = sequence;
	node->origin_id = origin_id;
	node->origin = origin;
	node->delayTics = 0;
	node->stopSound = SeqInfo[sequence].stopSound;
	node->volume = 127; // Start at max volume

	if(!SequenceListHead)
	{
		SequenceListHead = node;
		node->next = node->prev = NULL;
	}
	else
	{
		SequenceListHead->prev = node;
		node->next = SequenceListHead;
		node->prev = NULL;
		SequenceListHead = node;
	}
	ActiveSequences++;
	return;
}

//==========================================================================
//
//  SN_StartSequenceName
//
//==========================================================================

void SN_StartSequenceName(int origin_id, const TVec &origin, char *name)
{
	int i;

	for (i = 0; i < NumSequences; i++)
	{
		if (!strcmp(name, SeqInfo[i].name))
		{
			SN_StartSequence(origin_id, origin, i);
			return;
		}
	}
}

//==========================================================================
//
//  SN_StopSequence
//
//==========================================================================

void SN_StopSequence(int origin_id)
{
	seqnode_t *node;

	for(node = SequenceListHead; node; node = node->next)
	{
		if(node->origin_id == origin_id)
		{
			S_StopSound(origin_id);
			if(node->stopSound)
			{
				S_StartSound(node->stopSound, node->origin, TVec(0, 0, 0),
					node->origin_id, node->volume);
			}
			if(SequenceListHead == node)
			{
				SequenceListHead = node->next;
			}
			if(node->prev)
			{
				node->prev->next = node->next;
			}
			if(node->next)
			{
				node->next->prev = node->prev;
			}
			Z_Free(node);
			ActiveSequences--;
		}
	}
}

//==========================================================================
//
//  SN_UpdateActiveSequences
//
//==========================================================================

void SN_UpdateActiveSequences(void)
{
	seqnode_t *node;
	boolean sndPlaying;

	if (!ActiveSequences || cl.paused)
	{ // No sequences currently playing/game is paused
		return;
	}
	for(node = SequenceListHead; node; node = node->next)
	{
		if(node->delayTics)
		{
			node->delayTics--;
			continue;
		}
		sndPlaying = S_GetSoundPlayingInfo((int)node->origin_id, node->currentSoundID);
		switch (*node->sequencePtr)
		{
			case SS_CMD_PLAY:
				if(!sndPlaying)
				{
					node->currentSoundID = *(node->sequencePtr+1);
					S_StartSound(node->currentSoundID, node->origin,
						TVec(0, 0, 0), node->origin_id, node->volume);
				}
				node->sequencePtr += 2;
				break;
			case SS_CMD_WAITUNTILDONE:
				if(!sndPlaying)
				{
					node->sequencePtr++;
					node->currentSoundID = 0;
				}
				break;
			case SS_CMD_PLAYREPEAT:
				if(!sndPlaying)
				{
					node->currentSoundID = *(node->sequencePtr+1);
					S_StartSound(node->currentSoundID, node->origin,
						TVec(0, 0, 0), node->origin_id, node->volume);
				}
				break;
			case SS_CMD_DELAY:
				node->delayTics = *(node->sequencePtr+1);
				node->sequencePtr += 2;
				node->currentSoundID = 0;
				break;
			case SS_CMD_DELAYRAND:
				node->delayTics = *(node->sequencePtr + 1) +
					rand() % (*(node->sequencePtr + 2) - *(node->sequencePtr + 1));
				node->sequencePtr += 2;
				node->currentSoundID = 0;
				break;
			case SS_CMD_VOLUME:
				node->volume = (127*(*(node->sequencePtr+1)))/100;
				node->sequencePtr += 2;
				break;
			case SS_CMD_STOPSOUND:
				// Wait until something else stops the sequence
				break;
			case SS_CMD_END:
				SN_StopSequence(node->origin_id);
				break;
			default:	
				break;
		}
	}
}

//==========================================================================
//
//  SN_StopAllSequences
//
//==========================================================================

void SN_StopAllSequences(void)
{
	seqnode_t *node;

	for(node = SequenceListHead; node; node = node->next)
	{
		node->stopSound = 0; // don't play any stop sounds
		SN_StopSequence(node->origin_id);
	}
}
	
//==========================================================================
//
//  SN_GetSequenceOffset
//
//==========================================================================

int SN_GetSequenceOffset(int sequence, int *sequencePtr)
{
	return (sequencePtr - SeqInfo[sequence].data);
}

//==========================================================================
//
//  SN_ChangeNodeData
//
// 	nodeNum zero is the first node
//==========================================================================

void SN_ChangeNodeData(int nodeNum, int seqOffset, int delayTics, int volume,
	int currentSoundID)
{
	int i;
	seqnode_t *node;

	i = 0;
	node = SequenceListHead;
	while(node && i < nodeNum)
	{
		node = node->next;
		i++;
	}
	if(!node)
	{ // reach the end of the list before finding the nodeNum-th node
		return;
	}
	node->delayTics = delayTics;
	node->volume = volume;
	node->sequencePtr += seqOffset;
	node->currentSoundID = currentSoundID;
}

