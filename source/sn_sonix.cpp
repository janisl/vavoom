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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "s_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VSoundSeqNode
{
public:
	vint32			Sequence;
	vint32*			SequencePtr;
	vint32			OriginId;
	TVec			Origin;
	vint32			CurrentSoundID;
	float			DelayTime;
	float			Volume;
	vint32			StopSound;
	VSoundSeqNode*	Prev;
	VSoundSeqNode*	Next;

	VSoundSeqNode(int, const TVec&, int);
	~VSoundSeqNode();
	void Update(float);
	void Serialise(VStream&);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  VSoundSeqNode::VSoundSeqNode
//
//==========================================================================

VSoundSeqNode::VSoundSeqNode(int InOriginId, const TVec& InOrigin,
	int InSequence)
: Sequence(InSequence)
, OriginId(InOriginId)
, Origin(InOrigin)
, CurrentSoundID(0)
, DelayTime(0.0)
, Volume(1.0) // Start at max volume
, Prev(NULL)
, Next(NULL)
{
	if (Sequence >= 0)
	{
		SequencePtr = GSoundManager->SeqInfo[Sequence].Data;
		StopSound = GSoundManager->SeqInfo[Sequence].StopSound;
	}

	//	Add to the list of sound sequences.
	if (!GAudio->SequenceListHead)
	{
		GAudio->SequenceListHead = this;
	}
	else
	{
		GAudio->SequenceListHead->Prev = this;
		Next = GAudio->SequenceListHead;
		GAudio->SequenceListHead = this;
	}
	GAudio->ActiveSequences++;
}

//==========================================================================
//
//  VSoundSeqNode::~VSoundSeqNode
//
//==========================================================================

VSoundSeqNode::~VSoundSeqNode()
{
	//	Play stop sound.
	if (StopSound >= 0)
	{
		GAudio->StopSound(OriginId, 0);
	}
	if (StopSound >= 1)
	{
		GAudio->PlaySound(StopSound, Origin, TVec(0, 0, 0), OriginId, 1,
			Volume);
	}

	//	Remove from the list of active sound sequences.
	if (GAudio->SequenceListHead == this)
	{
		GAudio->SequenceListHead = Next;
	}
	if (Prev)
	{
		Prev->Next = Next;
	}
	if (Next)
	{
		Next->Prev = Prev;
	}
	GAudio->ActiveSequences--;
}

//==========================================================================
//
//  VSoundSeqNode::Update
//
//==========================================================================

void VSoundSeqNode::Update(float DeltaTime)
{
	guard(VSoundSeqNode::Update);
	if (DelayTime)
	{
		DelayTime -= DeltaTime;
		if (DelayTime <= 0.0)
		{
			DelayTime = 0.0;
		}
		return;
	}

	bool sndPlaying = GAudio->IsSoundPlaying(OriginId, CurrentSoundID);
	switch (*SequencePtr)
	{
	case SSCMD_Play:
		if (!sndPlaying)
		{
			CurrentSoundID = SequencePtr[1];
			GAudio->PlaySound(CurrentSoundID, Origin, TVec(0, 0, 0),
				OriginId, 1, Volume);
		}
		SequencePtr += 2;
		break;

	case SSCMD_WaitUntilDone:
		if (!sndPlaying)
		{
			SequencePtr++;
			CurrentSoundID = 0;
		}
		break;

	case SSCMD_PlayRepeat:
		if (!sndPlaying)
		{
			CurrentSoundID = SequencePtr[1];
			GAudio->PlaySound(CurrentSoundID, Origin, TVec(0, 0, 0),
				OriginId, 1, Volume);
		}
		break;

	case SSCMD_PlayLoop:
		CurrentSoundID = SequencePtr[1];
		GAudio->PlaySound(CurrentSoundID, Origin, TVec(0, 0, 0), OriginId, 1,
			Volume);
		DelayTime = SequencePtr[2] / 35.0;
		break;

	case SSCMD_Delay:
		DelayTime = SequencePtr[1] / 35.0;
		SequencePtr += 2;
		CurrentSoundID = 0;
		break;

	case SSCMD_DelayRand:
		DelayTime = (SequencePtr[1] + rand() % (SequencePtr[2] -
			SequencePtr[1])) / 35.0;
		SequencePtr += 2;
		CurrentSoundID = 0;
		break;

	case SSCMD_Volume:
		Volume = SequencePtr[1] / 100.0;
		SequencePtr += 2;
		break;

	case SSCMD_Attenuation:
		// Unused for now.
		SequencePtr += 2;
		break;

	case SSCMD_StopSound:
		// Wait until something else stops the sequence
		break;

	case SSCMD_End:
		GAudio->StopSequence(OriginId);
		break;

	default:	
		break;
	}
	unguard;
}

//==========================================================================
//
//	VSoundSeqNode::Serialise
//
//==========================================================================

void VSoundSeqNode::Serialise(VStream& Strm)
{
	guard(VSoundSeqNode::Serialise);
	Strm << Sequence << OriginId << Origin << CurrentSoundID << DelayTime
		<< Volume;
	if (Strm.IsLoading())
	{
		vint32 Offset;
		Strm << Offset;
		SequencePtr = GSoundManager->SeqInfo[Sequence].Data + Offset;
		StopSound = GSoundManager->SeqInfo[Sequence].StopSound;
	}
	else
	{
		vint32 Offset = SequencePtr - GSoundManager->SeqInfo[Sequence].Data;
		Strm << Offset;
	}
	unguard;
}

//==========================================================================
//
//  VAudio::StartSequenceName
//
//==========================================================================

void VAudio::StartSequenceName(int origin_id, const TVec &origin,
	const char *name)
{
	guard(VAudio::StartSequenceName);
	for (int i = 0; i < GSoundManager->SeqInfo.Num(); i++)
	{
		if (GSoundManager->SeqInfo[i].Name == name)
		{
			StopSequence(origin_id); // Stop any previous sequence
			new VSoundSeqNode(origin_id, origin, i);
			return;
		}
	}
	unguard;
}

//==========================================================================
//
//  VAudio::StopSequence
//
//==========================================================================

void VAudio::StopSequence(int origin_id)
{
	guard(VAudio::StopSequence);
	for (VSoundSeqNode* node = SequenceListHead; node; node = node->Next)
	{
		if (node->OriginId == origin_id)
		{
			delete node;
		}
	}
	unguard;
}

//==========================================================================
//
//  VAudio::UpdateActiveSequences
//
//==========================================================================

void VAudio::UpdateActiveSequences(float DeltaTime)
{
	guard(VAudio::UpdateActiveSequences);
	if (!ActiveSequences || cl->ClientFlags & VClientState::CF_Paused)
	{
		// No sequences currently playing/game is paused
		return;
	}
	for (VSoundSeqNode* node = SequenceListHead; node; node = node->Next)
	{
		node->Update(DeltaTime);
	}
	unguard;
}

//==========================================================================
//
//  VAudio::StopAllSequences
//
//==========================================================================

void VAudio::StopAllSequences()
{
	guard(VAudio::StopAllSequences);
	for (VSoundSeqNode* node = SequenceListHead; node; node = node->Next)
	{
		node->StopSound = 0; // don't play any stop sounds
		delete node;
	}
	unguard;
}

//==========================================================================
//
//	VAudio::SerialiseSounds
//
//==========================================================================

void VAudio::SerialiseSounds(VStream& Strm)
{
	guard(VAudio::SerialiseSounds);
	if (Strm.IsLoading())
	{
		// Reload and restart all sound sequences
		vint32 numSequences = Streamer<vint32>(Strm);
		for (int i = 0; i < numSequences; i++)
		{
			VSoundSeqNode* node = new VSoundSeqNode(0, TVec(0, 0, 0), -1);
			node->Serialise(Strm);
		}
	}
	else
	{
		// Save the sound sequences
		Strm << ActiveSequences;
		for (VSoundSeqNode* node = SequenceListHead; node; node = node->Next)
		{
			node->Serialise(Strm);
		}
	}
	unguard;
}
