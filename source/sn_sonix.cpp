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

#include "gamedefs.h"
#ifdef CLIENT
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct seqnode_t
{
	vint32*		sequencePtr;
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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int					ActiveSequences;
static seqnode_t*			SequenceListHead;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  SN_StartSequence
//
//==========================================================================

void SN_StartSequence(int origin_id, const TVec &origin, int sequence)
{
	guard(SN_StartSequence);
	seqnode_t *node;

	SN_StopSequence(origin_id); // Stop any previous sequence
	node = new seqnode_t;
	node->sequencePtr = GSoundManager->SeqInfo[sequence].Data;
	node->sequence = sequence;
	node->origin_id = origin_id;
	node->origin = origin;
	node->delayTics = 0;
	node->stopSound = GSoundManager->SeqInfo[sequence].StopSound;
	node->volume = 127; // Start at max volume
	node->currentSoundID = 0;

	if (!SequenceListHead)
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
	unguard;
}

//==========================================================================
//
//  SN_StartSequenceName
//
//==========================================================================

void SN_StartSequenceName(int origin_id, const TVec &origin, const char *name)
{
	guard(SN_StartSequenceName);
	for (int i = 0; i < GSoundManager->SeqInfo.Num(); i++)
	{
		if (GSoundManager->SeqInfo[i].Name == name)
		{
			SN_StartSequence(origin_id, origin, i);
			return;
		}
	}
	unguard;
}

//==========================================================================
//
//  SN_StopSequence
//
//==========================================================================

void SN_StopSequence(int origin_id)
{
	guard(SN_StopSequence);
	seqnode_t *node;

	for (node = SequenceListHead; node; node = node->next)
	{
		if (node->origin_id == origin_id)
		{
			if (node->stopSound >= 0)
			{
				S_StopSound(origin_id, 0);
			}
			if (node->stopSound >= 1)
			{
				S_StartSound(node->stopSound, node->origin, TVec(0, 0, 0),
					node->origin_id, 1, node->volume);
			}
			if (SequenceListHead == node)
			{
				SequenceListHead = node->next;
			}
			if (node->prev)
			{
				node->prev->next = node->next;
			}
			if (node->next)
			{
				node->next->prev = node->prev;
			}
			delete node;
			ActiveSequences--;
		}
	}
	unguard;
}

//==========================================================================
//
//  SN_UpdateActiveSequences
//
//==========================================================================

void SN_UpdateActiveSequences()
{
	guard(SN_UpdateActiveSequences);
	seqnode_t *node;
	boolean sndPlaying;

	if (!ActiveSequences || cl->ClientFlags & VClientState::CF_Paused)
	{ // No sequences currently playing/game is paused
		return;
	}
	for (node = SequenceListHead; node; node = node->next)
	{
		if (node->delayTics)
		{
			node->delayTics--;
			continue;
		}
		sndPlaying = S_GetSoundPlayingInfo((int)node->origin_id, node->currentSoundID);
		switch (*node->sequencePtr)
		{
			case SSCMD_Play:
				if(!sndPlaying)
				{
					node->currentSoundID = *(node->sequencePtr+1);
					S_StartSound(node->currentSoundID, node->origin,
						TVec(0, 0, 0), node->origin_id, 1, node->volume);
				}
				node->sequencePtr += 2;
				break;
			case SSCMD_WaitUntilDone:
				if(!sndPlaying)
				{
					node->sequencePtr++;
					node->currentSoundID = 0;
				}
				break;
			case SSCMD_PlayRepeat:
				if(!sndPlaying)
				{
					node->currentSoundID = *(node->sequencePtr+1);
					S_StartSound(node->currentSoundID, node->origin,
						TVec(0, 0, 0), node->origin_id, 1, node->volume);
				}
				break;
			case SSCMD_PlayLoop:
				node->currentSoundID = *(node->sequencePtr + 1);
				S_StartSound(node->currentSoundID, node->origin,
					TVec(0, 0, 0), node->origin_id, 1, node->volume);
				node->delayTics = *(node->sequencePtr + 2);
				break;
			case SSCMD_Delay:
				node->delayTics = *(node->sequencePtr+1);
				node->sequencePtr += 2;
				node->currentSoundID = 0;
				break;
			case SSCMD_DelayRand:
				node->delayTics = *(node->sequencePtr + 1) +
					rand() % (*(node->sequencePtr + 2) - *(node->sequencePtr + 1));
				node->sequencePtr += 2;
				node->currentSoundID = 0;
				break;
			case SSCMD_Volume:
				node->volume = (127*(*(node->sequencePtr+1)))/100;
				node->sequencePtr += 2;
				break;
			case SSCMD_Attenuation:
				// Unused for now.
				node->sequencePtr += 2;
				break;
			case SSCMD_StopSound:
				// Wait until something else stops the sequence
				break;
			case SSCMD_End:
				SN_StopSequence(node->origin_id);
				break;
			default:	
				break;
		}
	}
	unguard;
}

//==========================================================================
//
//  SN_StopAllSequences
//
//==========================================================================

void SN_StopAllSequences()
{
	guard(SN_StopAllSequences);
	seqnode_t *node;

	for(node = SequenceListHead; node; node = node->next)
	{
		node->stopSound = 0; // don't play any stop sounds
		SN_StopSequence(node->origin_id);
	}
	unguard;
}

//==========================================================================
//
//  SN_ChangeNodeData
//
// 	nodeNum zero is the first node
//
//==========================================================================

static void SN_ChangeNodeData(int nodeNum, int seqOffset, int delayTics,
	int volume, int currentSoundID)
{
	guard(SN_ChangeNodeData);
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
	unguard;
}

#endif

//==========================================================================
//
//	SN_SerialiseSounds
//
//==========================================================================

void SN_SerialiseSounds(VStream& Strm)
{
#ifdef CLIENT
	if (Strm.IsLoading())
	{
		// Reload and restart all sound sequences
		int numSequences = Streamer<int>(Strm);
		for (int i = 0; i < numSequences; i++)
		{
			int sequence = Streamer<int>(Strm);
			int delayTics = Streamer<int>(Strm);
			int volume = Streamer<int>(Strm);
			int seqOffset = Streamer<int>(Strm);
			int soundID = Streamer<int>(Strm);
			int objectNum = Streamer<int>(Strm);
			float x = Streamer<float>(Strm);
			float y = Streamer<float>(Strm);
			float z = Streamer<float>(Strm);
			SN_StartSequence(objectNum, TVec(x, y, z), sequence);
			SN_ChangeNodeData(i, seqOffset, delayTics, volume, soundID);
		}
	}
	else
	{
		// Save the sound sequences
		Strm << ActiveSequences;
		for (seqnode_t* node = SequenceListHead; node; node = node->next)
		{
			Strm << node->sequence;
			Strm << node->delayTics;
			Strm << node->volume;
			vint32 Offset = node->sequencePtr -
				GSoundManager->SeqInfo[node->sequence].Data;
			Strm << Offset;
			Strm << node->currentSoundID;
			Strm << node->origin_id;
			Strm << node->origin.x;
			Strm << node->origin.y;
			Strm << node->origin.z;
		}
	}
#else
	vint32 Dummy = 0;
	Strm << Dummy;
#endif
}
