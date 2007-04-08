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
#include "network.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VChannel::VChannel
//
//==========================================================================

VChannel::VChannel(VNetConnection* AConnection, EChannelType AType,
	vint32 AIndex, vuint8 AOpenedLocally)
: Connection(AConnection)
, Index(AIndex)
, Type(AType)
, OpenedLocally(AOpenedLocally)
, Closing(false)
, InMsg(NULL)
, OutMsg(NULL)
{
	if (Index == -1)
	{
		Index = 2;
		while (Index < MAX_CHANNELS && Connection->Channels[Index])
		{
			Index++;
		}
		check(Index < MAX_CHANNELS);
	}
	checkSlow(!Connection->Channels[Index]);
	Connection->Channels[Index] = this;
	Connection->OpenChannels.Append(this);
}

//==========================================================================
//
//	VChannel::~VChannel
//
//==========================================================================

VChannel::~VChannel()
{
	guard(VChannel::~VChannel);
	for (VMessageIn* Msg = InMsg; Msg; )
	{
		VMessageIn* Next = Msg->Next;
		delete Msg;
		Msg = Next;
	}
	for (VMessageOut* Msg = OutMsg; Msg; )
	{
		VMessageOut* Next = Msg->Next;
		delete Msg;
		Msg = Next;
	}
	if (Index != -1 && Connection->Channels[Index] == this)
	{
		Connection->Channels[Index] = NULL;
	}
	Connection->OpenChannels.Remove(this);
	unguard;
}

//==========================================================================
//
//	VChannel::ReceivedRawMessage
//
//==========================================================================

void VChannel::ReceivedRawMessage(VMessageIn& Msg)
{
	guard(VChannel::ReceivedRawMessage);
	//	Drop outdated messages
	if (Msg.bReliable && Msg.Sequence < Connection->InSequence[Index])
	{
		Connection->Driver->receivedDuplicateCount++;
		return;
	}

	if (Msg.bReliable && Msg.Sequence > Connection->InSequence[Index])
	{
		VMessageIn** pNext = &InMsg;
		while (*pNext && (*pNext)->Sequence <= Msg.Sequence)
		{
			if ((*pNext)->Sequence == Msg.Sequence)
			{
				Connection->Driver->receivedDuplicateCount++;
				return;
			}
		}
		VMessageIn* Copy = new VMessageIn(Msg);
		Copy->Next = *pNext;
		*pNext = Copy;
		return;
	}
	if (Msg.bReliable)
	{
		Connection->InSequence[Index]++;
	}

	if (!Closing)
	{
		ParsePacket(Msg);
	}
	if (Msg.bClose)
	{
		delete this;
		return;
	}

	while (InMsg && InMsg->Sequence == Connection->InSequence[Index])
	{
		VMessageIn* OldMsg = InMsg;
		InMsg = OldMsg->Next;
		Connection->InSequence[Index]++;
		if (!Closing)
		{
			ParsePacket(*OldMsg);
		}
		bool Closed = false;
		if (OldMsg->bClose)
		{
			delete this;
			Closed = true;
		}
		delete OldMsg;
		if (Closed)
		{
			return;
		}
	}
	unguard;
}

//==========================================================================
//
//	VChannel::SendMessage
//
//==========================================================================

void VChannel::SendMessage(VMessageOut* AMsg)
{
	guard(VChannel::SendMessage);
	VMessageOut* Msg = AMsg;
	if (Msg->bReliable)
	{
		Msg->Sequence = Connection->OutSequence[Index];

		VMessageOut* Copy = new VMessageOut(*Msg);
		Copy->Next = NULL;
		VMessageOut** pNext = &OutMsg;
		while (*pNext)
		{
			pNext = &(*pNext)->Next;
		}
		*pNext = Copy;
		Msg = Copy;

		Connection->OutSequence[Index]++;
	}

	Connection->SendRawMessage(*Msg);
	unguard;
}

//==========================================================================
//
//	VChannel::ReceivedAck
//
//==========================================================================

void VChannel::ReceivedAck()
{
	guard(VChannel::ReceivedAck);
	//	Clean up messages that have been ACK-ed. Only the first ones are
	// deleted so that close message doesn't get handled while there's
	// still messages that are not ACK-ed.
	bool CloseAcked = false;
	while (OutMsg && OutMsg->bReceivedAck)
	{
		VMessageOut* Msg = OutMsg;
		OutMsg = Msg->Next;
		if (Msg->bClose)
		{
			CloseAcked = true;
		}
		delete Msg;
	}

	//	If we received ACK for close message then delete this channel.
	if (CloseAcked)
	{
		delete this;
	}
	unguard;
}

//==========================================================================
//
//	VChannel::Close
//
//==========================================================================

void VChannel::Close()
{
	guard(VChannel::Close);
	if (Closing)
	{
		//	Already in closing state.
		return;
	}

	//	Send close message.
	VMessageOut Msg(this);
	Msg.bReliable = true;
	Msg.bClose = true;
	SendMessage(&Msg);

	//	Enter closing state.
	Closing = true;
	unguard;
}

//==========================================================================
//
//	VChannel::Tick
//
//==========================================================================

void VChannel::Tick()
{
	guard(VChannel::Tick);
	//	Resend timed out messages.
	for (VMessageOut* Msg = OutMsg; Msg; Msg = Msg->Next)
	{
		if (!Msg->bReceivedAck && Connection->Driver->NetTime - Msg->Time > 1.0)
		{
			Connection->SendRawMessage(*Msg);
			Connection->Driver->packetsReSent++;
		}
	}
	unguard;
}
