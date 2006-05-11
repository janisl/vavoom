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
#include "s_local.h"

// MACROS ------------------------------------------------------------------

#define SS_MAX_SCRIPTS			64
#define SS_TEMPBUFFER_SIZE		1024
#define SS_SEQUENCE_NAME_LENGTH	32

#define SS_STRING_PLAY			"play"
#define SS_STRING_PLAYUNTILDONE "playuntildone"
#define SS_STRING_PLAYTIME		"playtime"
#define SS_STRING_PLAYREPEAT	"playrepeat"
#define SS_STRING_PLAYLOOP		"playloop"
#define SS_STRING_DELAY			"delay"
#define SS_STRING_DELAYRAND		"delayrand"
#define SS_STRING_VOLUME		"volume"
#define SS_STRING_END			"end"
#define SS_STRING_STOPSOUND		"stopsound"
#define SS_STRING_NO_CUTOFF		"nostopcutoff"
#define SS_STRING_ATTENUATION	"attenuation"
#define SS_STRING_DOOR			"door"
#define SS_STRING_PLATFORM		"platform"
#define SS_STRING_ENVIRONMENT	"environment"

// TYPES -------------------------------------------------------------------

enum sscmds_t
{
	SS_CMD_NONE,
	SS_CMD_PLAY,
	SS_CMD_WAITUNTILDONE, // used by PLAYUNTILDONE
	SS_CMD_PLAYTIME,
	SS_CMD_PLAYREPEAT,
	SS_CMD_PLAYLOOP,
	SS_CMD_DELAY,
	SS_CMD_DELAYRAND,
	SS_CMD_VOLUME,
	SS_CMD_STOPSOUND,
	SS_CMD_ATTENUATION,
	SS_CMD_END
};

struct seq_info_t
{
	char	name[SS_SEQUENCE_NAME_LENGTH];
	int		*data;
	int		stopSound;
};

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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void ParseSequenceScript();
static void VerifySequencePtr(int *base, int *ptr);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static seq_info_t	SeqInfo[SS_MAX_SCRIPTS];
static int			NumSequences;
static int			SeqTrans[64 * 3];

static int			ActiveSequences;
static seqnode_t	*SequenceListHead;

static const char *Attenuations[] =
{
	"none",
	"normal",
	"idle",
	"static",
	"surround",
	NULL
};

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
	guard(VerifySequencePtr);
	if (ptr - base > SS_TEMPBUFFER_SIZE)
	{
		Sys_Error("VerifySequencePtr:  tempPtr >= %d\n", SS_TEMPBUFFER_SIZE);
	}
	unguard;
}

//==========================================================================
//
// GetSoundOffset
//
//==========================================================================

static int GetSoundOffset(const char *name)
{
	guard(GetSoundOffset);
	for (int i = 0; i < S_sfx.Num(); i++)
	{
		if (!stricmp(name, *S_sfx[i].TagName))
		{
			return i;
		}
	}
	SC_ScriptError("GetSoundOffset:  Unknown sound name\n");
	return 0;
	unguard;
}

//==========================================================================
//
// SN_InitSequenceScript
//
//==========================================================================

void SN_InitSequenceScript()
{
	guard(SN_InitSequenceScript);
	ActiveSequences = 0;
	NumSequences = 0;
	memset(SeqTrans, -1, sizeof(SeqTrans));
	memset(SeqInfo, 0, sizeof(SeqInfo));
	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) == NAME_sndseq)
		{
			SC_OpenLumpNum(Lump);
			ParseSequenceScript();
		}
	}
	//	Optionally parse script file.
	VStr filename = FL_FindFile("scripts/sndseq.txt");
	if (fl_devmode && filename)
	{
		SC_OpenFile(*filename);
		ParseSequenceScript();
	}
	unguard;
}

//==========================================================================
//
//	AssignTranslations
//
//==========================================================================

static void AssignTranslations(int SeqId, seqtype_t SeqType)
{
	guard(AssignTranslations);
	sc_Crossed = false;

	while (SC_GetString() && !sc_Crossed)
	{
		char* Stopper;
		int Num = strtol(sc_String, &Stopper, 0);
		if (*Stopper == 0)
		{
			SeqTrans[(Num & 63) + SeqType * 64] = SeqId;
		}
	}

	SC_UnGet();
	unguard;
}

//==========================================================================
//
//	ParseSequenceScript
//
//==========================================================================

static void ParseSequenceScript()
{
	guard(ParseSequenceScript);
	int 		*tempDataStart = NULL;
	int 		*tempDataPtr = NULL;
	bool		inSequence = false;
	int			SeqId = 0;

	while (SC_GetString())
	{
		if (*sc_String == ':')
		{
			if (inSequence)
			{
				SC_ScriptError("SN_InitSequenceScript:  Nested Script Error");
			}
			for (SeqId = 0; SeqId < NumSequences; SeqId++)
			{
				if (!strcmp(SeqInfo[SeqId].name, sc_String))
				{
					Z_Free(SeqInfo[SeqId].data);
					break;
				}
			}
			if (SeqId == NumSequences)
			{
				SeqId = NumSequences;
				NumSequences++;
				if (NumSequences == SS_MAX_SCRIPTS)
				{
					SC_ScriptError("Number of SS Scripts >= SS_MAX_SCRIPTS");
				}
			}
			tempDataStart = (int *)Z_Malloc(SS_TEMPBUFFER_SIZE, 
				PU_STATIC, NULL);
			memset(tempDataStart, 0, SS_TEMPBUFFER_SIZE);
			tempDataPtr = tempDataStart;
            inSequence = true;
            strcpy(SeqInfo[SeqId].name, sc_String + 1);
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
		else if (SC_Compare(SS_STRING_PLAYLOOP))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			SC_MustGetString();
			*tempDataPtr++ = SS_CMD_PLAYLOOP;
			*tempDataPtr++ = GetSoundOffset(sc_String);
			SC_MustGetNumber();
			*tempDataPtr++ = sc_Number;
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
		else if (SC_Compare(SS_STRING_ATTENUATION))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			*tempDataPtr++ = SS_CMD_ATTENUATION;
			SC_MustGetString();
			*tempDataPtr++ = SC_MustMatchString(Attenuations);
		}
		else if (SC_Compare(SS_STRING_STOPSOUND))
		{
			SC_MustGetString();
			SeqInfo[SeqId].stopSound = GetSoundOffset(sc_String);
			*tempDataPtr++ = SS_CMD_STOPSOUND;
		}
		else if (SC_Compare(SS_STRING_NO_CUTOFF))
		{
			SeqInfo[SeqId].stopSound = -1;
			*tempDataPtr++ = SS_CMD_STOPSOUND;
		}
		else if (SC_Compare(SS_STRING_DOOR))
		{
			AssignTranslations(SeqId, SEQ_Door);
		}
		else if (SC_Compare(SS_STRING_PLATFORM))
		{
			AssignTranslations(SeqId, SEQ_Platform);
		}
		else if (SC_Compare(SS_STRING_ENVIRONMENT))
		{
			AssignTranslations(SeqId, SEQ_Environment);
		}
		else if (SC_Compare(SS_STRING_END))
		{
			*tempDataPtr++ = SS_CMD_END;
			int dataSize = (tempDataPtr-tempDataStart)*sizeof(int);
			SeqInfo[SeqId].data = (int *)Z_Malloc(dataSize, PU_STATIC,
				NULL);
			memcpy(SeqInfo[SeqId].data, tempDataStart, dataSize);
			Z_Free(tempDataStart);
			inSequence = false;
		}
		else
		{
			SC_ScriptError("SN_InitSequenceScript:  Unknown commmand.\n");
		}
	}
	SC_Close();
	unguard;
}

//==========================================================================
//
//  SN_SetSeqTrans
//
//==========================================================================

void SN_SetSeqTrans(VName Name, int Num, int SeqType)
{
	guard(SN_SetSeqTrans);
	for (int i = 0; i < NumSequences; i++)
	{
		if (!strcmp(*Name, SeqInfo[i].name))
		{
			SeqTrans[(Num & 63) + SeqType * 64] = i;
			return;
		}
	}
	unguard;
}

//==========================================================================
//
//  SN_GetSeqTrans
//
//==========================================================================

VName SN_GetSeqTrans(int Num, int SeqType)
{
	guard(SN_GetSeqTrans);
	if (SeqTrans[(Num & 63) + SeqType * 64] < 0)
	{
		return NAME_None;
	}
	return SeqInfo[SeqTrans[(Num & 63) + SeqType * 64]].name;
	unguard;
}

#ifdef CLIENT

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
	node = (seqnode_t *)Z_Malloc(sizeof(seqnode_t), PU_STATIC, NULL);
	node->sequencePtr = SeqInfo[sequence].data;
	node->sequence = sequence;
	node->origin_id = origin_id;
	node->origin = origin;
	node->delayTics = 0;
	node->stopSound = SeqInfo[sequence].stopSound;
	node->volume = 127; // Start at max volume
	node->currentSoundID = 0;

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
	for (int i = 0; i < NumSequences; i++)
	{
		if (!strcmp(name, SeqInfo[i].name))
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
			Z_Free(node);
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
			case SS_CMD_PLAY:
				if(!sndPlaying)
				{
					node->currentSoundID = *(node->sequencePtr+1);
					S_StartSound(node->currentSoundID, node->origin,
						TVec(0, 0, 0), node->origin_id, 1, node->volume);
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
						TVec(0, 0, 0), node->origin_id, 1, node->volume);
				}
				break;
			case SS_CMD_PLAYLOOP:
				node->currentSoundID = *(node->sequencePtr + 1);
				S_StartSound(node->currentSoundID, node->origin,
					TVec(0, 0, 0), node->origin_id, 1, node->volume);
				node->delayTics = *(node->sequencePtr + 2);
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
			case SS_CMD_ATTENUATION:
				// Unused for now.
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
#ifdef CLIENT
			SN_StartSequence(objectNum, TVec(x, y, z), sequence);
			SN_ChangeNodeData(i, seqOffset, delayTics, volume, soundID);
#endif
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
			int Offset = node->sequencePtr - SeqInfo[node->sequence].data;
			Strm << Offset;
			Strm << node->currentSoundID;
			Strm << node->origin_id;
			Strm << node->origin.x;
			Strm << node->origin.y;
			Strm << node->origin.z;
		}
	}
}

//==========================================================================
//
//	SN_FreeSequenceData
//
//==========================================================================

void SN_FreeSequenceData()
{
	guard(SN_FreeSequenceData);
	for (int i = 0; i < NumSequences; i++)
	{
		Z_Free(SeqInfo[i].data);
	}
	unguard;
}
