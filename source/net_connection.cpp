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
//	VNetConnection::VNetConnection
//
//==========================================================================

VNetConnection::VNetConnection(VSocketPublic* ANetCon, VNetContext* AContext)
: NetCon(ANetCon)
, Driver(GNet)
, Context(AContext)
, State(NETCON_Open)
, Message(OUT_MESSAGE_SIZE)
, LastMessage(0)
, NeedsUpdate(false)
, AutoAck(false)
, Out(MAX_MSGLEN * 8)
, AckSequence(0)
, UnreliableSendSequence(0)
, UnreliableReceiveSequence(0)
{
	memset(Channels, 0, sizeof(Channels));
	Context->CreateGenChannel(this);
	new VPlayerChannel(this, 1);
}

//==========================================================================
//
//	VNetConnection::~VNetConnection
//
//==========================================================================

VNetConnection::~VNetConnection()
{
	guard(VNetConnection::~VNetConnection);
	while (OpenChannels.Num())
	{
		delete OpenChannels[OpenChannels.Num() - 1];
	}
	if (NetCon)
	{
		NetCon->Close();
	}
	NetCon = NULL;
	unguard;
}

//==========================================================================
//
//	VNetConnection::GetMessages
//
//==========================================================================

void VNetConnection::GetMessages()
{
	guard(VNetConnection::GetMessages);
	int ret;

	do
	{
		TArray<vuint8> Data;
		ret = GetRawPacket(Data);
		if (ret == -1)
		{
			GCon->Log(NAME_DevNet, "Bad read");
			State = NETCON_Closed;
			return;
		}

		if (ret)
		{
			if (Data.Num() > 0)
			{
				vuint8 LastByte = Data[Data.Num() - 1];
				if (LastByte)
				{
					//	Find out real length by stepping back untill the trailing bit.
					vuint32 Length = Data.Num() * 8 - 1;
					for (vuint8 Mask = 0x80; !(LastByte & Mask); Mask >>= 1)
					{
						Length--;
					}
					VBitStreamReader Packet(Data.Ptr(), Length);
					ReceivedPacket(Packet);
				}
				else
				{
					GCon->Logf(NAME_DevNet, "Packet is missing trailing bit");
				}
			}
			else
			{
				GCon->Logf(NAME_DevNet, "Packet is too small");
				Driver->shortPacketCount++;
			}
		}
	} while (ret > 0 && State != NETCON_Closed);
	unguard;
}

//==========================================================================
//
//	VNetConnection::GetRawMessage
//
//==========================================================================

int VNetConnection::GetRawPacket(TArray<vuint8>& Data)
{
	guard(VNetConnection::GetRawMessage);
	checkSlow(NetCon);
	return NetCon->GetMessage(Data);
	unguard;
}

//==========================================================================
//
//	VNetConnection::ReceivedPacket
//
//==========================================================================

void VNetConnection::ReceivedPacket(VBitStreamReader& Packet)
{
	guard(VNetConnection::ReceivedPacket);
	if (Packet.ReadInt(256) != NETPACKET_DATA)
		return;
	Driver->packetsReceived++;

	vuint32 Sequence;
	Packet << Sequence;
	if (Packet.IsError())
	{
		GCon->Log(NAME_DevNet, "Packet is missing packet ID");
		return;
	}
	if (Sequence < UnreliableReceiveSequence)
	{
		GCon->Log(NAME_DevNet, "Got a stale datagram");
		return;
	}
	if (Sequence != UnreliableReceiveSequence)
	{
		int count = Sequence - UnreliableReceiveSequence;
		Driver->droppedDatagrams += count;
		GCon->Logf(NAME_DevNet, "Dropped %d datagram(s)", count);
	}
	UnreliableReceiveSequence = Sequence + 1;

	bool NeedsAck = false;

	while (!Packet.AtEnd())
	{
		//	Read a flag to see if it's an ACK or a message.
		bool IsAck = Packet.ReadBit();
		if (Packet.IsError())
		{
			GCon->Log(NAME_DevNet, "Packet is missing ACK flag");
			return;
		}

		if (IsAck)
		{
			vuint32 AckSeq;
			Packet << AckSeq;
			if (AckSeq == AckSequence)
			{
				AckSequence++;
			}
			else if (AckSeq > AckSequence)
			{
				AckSequence = AckSeq + 1;
			}
			else
			{
				GCon->Log(NAME_DevNet, "Duplicate ACK received");
			}

			//	Mark corrresponding messages as ACK-ed.
			for (int i = 0; i < OpenChannels.Num(); i++)
			{
				for (VMessageOut* Msg = OpenChannels[i]->OutMsg; Msg;
					Msg = Msg->Next)
				{
					if (Msg->PacketId == AckSeq)
					{
						Msg->bReceivedAck = true;
					}
				}
			}

			//	Notify channels that ACK has been received.
			for (int i = OpenChannels.Num() - 1; i >= 0; i--)
			{
				OpenChannels[i]->ReceivedAck();
			}
		}
		else
		{
			NeedsAck = true;
			VMessageIn Msg;

			//	Read message header
			Msg.ChanIndex = Packet.ReadInt(MAX_CHANNELS);
			Msg.bReliable = Packet.ReadBit();
			Msg.bOpen = Packet.ReadBit();
			Msg.bClose = Packet.ReadBit();
			Msg.Sequence = 0;
			Msg.ChanType = 0;
			if (Msg.bReliable)
			{
				Packet << Msg.Sequence;
			}
			if (Msg.bOpen)
			{
				Msg.ChanType = Packet.ReadInt(CHANNEL_MAX);
			}

			//	Read data
			int Length = Packet.ReadInt(MAX_MSGLEN * 8);
			Msg.SetData(Packet, Length);

			VChannel* Chan = Channels[Msg.ChanIndex];
			if (!Chan)
			{
				if (Msg.bOpen)
				{
					switch (Msg.ChanType)
					{
					case CHANNEL_General:
						Sys_Error("Tried to remotely open general channel");
					case CHANNEL_Player:
						Chan = new VPlayerChannel(this, Msg.ChanIndex, false);
						break;
					case CHANNEL_Thinker:
						Chan = new VThinkerChannel(this, Msg.ChanIndex, false);
						break;
					default:
						GCon->Logf("Unknown channel type %d for channel %d",
							Msg.ChanType, Msg.ChanIndex);
						continue;
					}
				}
				else
				{
					GCon->Logf("Channel %d is not open", Msg.ChanIndex);
					continue;
				}
			}
			Chan->ReceivedRawMessage(Msg);
		}
	}

	if (NeedsAck)
	{
		SendAck(Sequence);
	}
	unguard;
}

//==========================================================================
//
//	VNetConnection::SendRawMessage
//
//==========================================================================

void VNetConnection::SendRawMessage(VMessageOut& Msg)
{
	guard(VNetConnection::SendRawMessage);
	PrepareOut(MAX_MESSAGE_HEADER_BITS + Msg.GetNumBits());

	Out.WriteBit(false);
	Out.WriteInt(Msg.ChanIndex, MAX_CHANNELS);
	Out.WriteBit(Msg.bReliable);
	Out.WriteBit(Msg.bOpen);
	Out.WriteBit(Msg.bClose);
	if (Msg.bReliable)
	{
		Out << Msg.Sequence;
	}
	if (Msg.bOpen)
	{
		Out.WriteInt(Msg.ChanType, CHANNEL_MAX);
	}
	Out.WriteInt(Msg.GetNumBits(), MAX_MSGLEN * 8);
	Out.SerialiseBits(Msg.GetData(), Msg.GetNumBits());

	Msg.Time = Driver->NetTime;
	Msg.PacketId = UnreliableSendSequence;
	unguard;
}

//==========================================================================
//
//	VNetConnection::SendAck
//
//==========================================================================

void VNetConnection::SendAck(vuint32 Sequence)
{
	guard(VNetConnection::SendAck);
	if (AutoAck)
	{
		return;
	}

	PrepareOut(33);

	Out.WriteBit(true);
	Out << Sequence;
	unguard;
}

//==========================================================================
//
//	VNetConnection::PrepareOut
//
//==========================================================================

void VNetConnection::PrepareOut(int Length)
{
	guard(VNetConnection::PrepareOut);
	//	Send current packet if new message doesn't fit.
	if (Out.GetNumBits() + Length + MAX_PACKET_TRAILER_BITS > MAX_MSGLEN * 8)
	{
		Flush();
	}

	if (Out.GetNumBits() == 0)
	{
		Out.WriteInt(NETPACKET_DATA, 256);
		Out << UnreliableSendSequence;
	}
	unguard;
}

//==========================================================================
//
//	VNetConnection::Flush
//
//==========================================================================

void VNetConnection::Flush()
{
	guard(VNetConnection::Flush);
	if (!Out.GetNumBits())
	{
		return;
	}

	//	Add trailing bit so we can find out how many bits the message has.
	Out.WriteBit(true);
	//	Pad it with zero bits untill byte boundary.
	while (Out.GetNumBits() & 7)
	{
		Out.WriteBit(false);
	}

	//	Send the message.
	if (NetCon->SendMessage(Out.GetData(), Out.GetNumBytes()) == -1)
	{
		State = NETCON_Closed;
	}

	Driver->packetsSent++;

	//	Increment outgoing packet counter
	UnreliableSendSequence++;

	//	Clear outgoing packet buffer.
	Out = VBitStreamWriter(MAX_MSGLEN * 8);
	unguard;
}

//==========================================================================
//
//	VNetConnection::IsLocalConnection
//
//==========================================================================

bool VNetConnection::IsLocalConnection()
{
	guard(VNetConnection::IsLocalConnection);
	return NetCon->IsLocalConnection();
	unguard;
}

//==========================================================================
//
//	VNetConnection::Tick
//
//==========================================================================

void VNetConnection::Tick()
{
	guard(VNetConnection::Tick);
	if (State == NETCON_Closed)
	{
		return;
	}

	//	For bots and demo playback there's no other end that will send us
	// the ACK so just mark all outgoing messages as ACK-ed.
	if (AutoAck)
	{
		for (int i = OpenChannels.Num() - 1; i >= 0; i--)
		{
			for (VMessageOut* Msg = OpenChannels[i]->OutMsg; Msg; Msg = Msg->Next)
			{
				Msg->bReceivedAck = true;
			}
			OpenChannels[i]->ReceivedAck();
		}
	}

	//	Run tick for all of the open channels.
	for (int i = OpenChannels.Num() - 1; i >= 0; i--)
	{
		OpenChannels[i]->Tick();
	}

	Flush();
	unguard;
}
