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

static VCvarF		net_test_loss("net_test_loss", "0");

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VNetConnection::VNetConnection
//
//==========================================================================

VNetConnection::VNetConnection(VSocketPublic* ANetCon, VNetContext* AContext,
	VBasePlayer* AOwner)
: NetCon(ANetCon)
, Driver(GNet)
, Context(AContext)
, Owner(AOwner)
, State(NETCON_Open)
, LastSendTime(0)
, NeedsUpdate(false)
, AutoAck(false)
, Out(MAX_MSGLEN * 8)
, AckSequence(0)
, UnreliableSendSequence(0)
, UnreliableReceiveSequence(0)
, UpdatePvs(NULL)
, UpdatePvsSize(0)
, LeafPvs(NULL)
{
	memset(Channels, 0, sizeof(Channels));
	memset(InSequence, 0, sizeof(InSequence));
	memset(OutSequence, 0, sizeof(OutSequence));

	ObjMap = new VNetObjectsMap(this);

	CreateChannel(CHANNEL_Control, CHANIDX_General);
	CreateChannel(CHANNEL_Player, CHANIDX_Player);
	CreateChannel(CHANNEL_Level, CHANIDX_Level);
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
		delete NetCon;
	}
	NetCon = NULL;
	if (Context->ServerConnection)
	{
		checkSlow(Context->ServerConnection == this);
		Context->ServerConnection = NULL;
	}
	else
	{
		Context->ClientConnections.Remove(this);
	}
	if (UpdatePvs)
	{
		delete[] UpdatePvs;
	}
	if (ObjMap)
	{
		delete ObjMap;
	}
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

	Driver->SetNetTime();

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
			if (!IsLocalConnection())
			{
				NetCon->LastMessageTime = Driver->NetTime;
				if (ret == 1)
					Driver->MessagesReceived++;
				else if (ret == 2)
					Driver->UnreliableMessagesReceived++;
			}

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

	NeedsUpdate = true;

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
						if (Msg->bOpen)
						{
							OpenChannels[i]->OpenAcked = true;
						}
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
			if (Packet.IsError())
			{
				GCon->Logf(NAME_DevNet, "Packet is missing message header");
				break;
			}

			//	Read data
			int Length = Packet.ReadInt(MAX_MSGLEN * 8);
			Msg.SetData(Packet, Length);
			if (Packet.IsError())
			{
				GCon->Logf(NAME_DevNet, "Packet is missing message data");
				break;
			}

			VChannel* Chan = Channels[Msg.ChanIndex];
			if (!Chan)
			{
				if (Msg.bOpen)
				{
					Chan = CreateChannel(Msg.ChanType, Msg.ChanIndex, false);
					Chan->OpenAcked = true;
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
//	VNetConnection::CreateChannel
//
//==========================================================================

VChannel* VNetConnection::CreateChannel(vuint8 Type, vint32 AIndex,
	vuint8 OpenedLocally)
{
	guard(VNetConnection::CreateChannel);
	//	If channel index is -1, find a free channel slot.
	vint32 Index = AIndex;
	if (Index == -1)
	{
		Index = CHANIDX_ThinkersStart;
		while (Index < MAX_CHANNELS && Channels[Index])
		{
			Index++;
		}
		if (Index == MAX_CHANNELS)
		{
			return NULL;
		}
	}

	switch (Type)
	{
	case CHANNEL_Control:
		return new VControlChannel(this, Index, OpenedLocally);
	case CHANNEL_Level:
		return new VLevelChannel(this, Index, OpenedLocally);
	case CHANNEL_Player:
		return new VPlayerChannel(this, Index, OpenedLocally);
	case CHANNEL_Thinker:
		return new VThinkerChannel(this, Index, OpenedLocally);
	default:
		GCon->Logf("Unknown channel type %d for channel %d", Type, Index);
		return NULL;
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
	Driver->SetNetTime();
	if (!Out.GetNumBits() && Driver->NetTime - LastSendTime < 5.0)
	{
		return;
	}

	//	Prepare out for keepalive messages
	if (!Out.GetNumBits())
	{
		PrepareOut(0);
	}

	//	Add trailing bit so we can find out how many bits the message has.
	Out.WriteBit(true);
	//	Pad it with zero bits untill byte boundary.
	while (Out.GetNumBits() & 7)
	{
		Out.WriteBit(false);
	}

	//	Send the message.
	if (net_test_loss == 0 || Random() * 100.0 <= net_test_loss)
	{
		if (NetCon->SendMessage(Out.GetData(), Out.GetNumBytes()) == -1)
		{
			State = NETCON_Closed;
		}
	}
	LastSendTime = Driver->NetTime;

	if (!IsLocalConnection())
		Driver->MessagesSent++;
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
	//	For demo playback NetCon can be NULL.
	return NetCon ? NetCon->IsLocalConnection() : true;
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
			OpenChannels[i]->OpenAcked = true;
			OpenChannels[i]->ReceivedAck();
		}
	}

	// see if this connection has timed out
	if (!IsLocalConnection() &&
		Driver->NetTime - NetCon->LastMessageTime > VNetworkPublic::MessageTimeOut)
	{
		if (State != NETCON_Closed)
		{
			GCon->Logf("Channel timed out");
		}
		State = NETCON_Closed;
	}
	else
	{
		//	Run tick for all of the open channels.
		for (int i = OpenChannels.Num() - 1; i >= 0; i--)
		{
			OpenChannels[i]->Tick();
		}
		//	If general channel has been closed, then this connection is closed
		if (!Channels[CHANIDX_General])
		{
			State = NETCON_Closed;
		}
	}

	//	Flush any remaining data or send keepalive.
	Flush();
	unguard;
}

//==========================================================================
//
//	VNetConnection::SendCommand
//
//==========================================================================

void VNetConnection::SendCommand(VStr Str)
{
	guard(VNetConnection::SendCommand);
	VMessageOut		Msg(Channels[CHANIDX_General]);
	Msg.bReliable = true;
	Msg << Str;
	Channels[CHANIDX_General]->SendMessage(&Msg);
	unguard;
}

//==========================================================================
//
//	VNetConnection::SetUpFatPVS
//
//==========================================================================

void VNetConnection::SetUpFatPVS()
{
	guard(VNetConnection::SetUpFatPVS);
	float	dummy_bbox[6] = {-99999, -99999, -99999, 99999, 99999, 99999};
	VLevel*	Level = Context->GetLevel();

	LeafPvs = Level->LeafPVS(Owner->MO->SubSector);

	//	Re-allocate PVS buffer if needed.
	if (UpdatePvsSize != (Level->NumSubsectors + 7) / 8)
	{
		if (UpdatePvs)
		{
			delete[] UpdatePvs;
		}
		UpdatePvsSize = (Level->NumSubsectors + 7) / 8;
		UpdatePvs = new vuint8[UpdatePvsSize];
	}

	//	Build view PVS using view clipper.
	memset(UpdatePvs, 0, UpdatePvsSize);
	Clipper.ClearClipNodes(Owner->ViewOrg, Level);
	SetUpPvsNode(Level->NumNodes - 1, dummy_bbox);
	unguard;
}

//==========================================================================
//
//	VNetConnection::SetUpPvsNode
//
//==========================================================================

void VNetConnection::SetUpPvsNode(int BspNum, float* BBox)
{
	guard(VNetConnection::SetUpPvsNode);
	VLevel* Level = Context->GetLevel();
	if (Clipper.ClipIsFull())
	{
		return;
	}
	if (!Clipper.ClipIsBBoxVisible(BBox))
	{
		return;
	}

	// Found a subsector?
	if (BspNum & NF_SUBSECTOR)
	{
		int SubNum = BspNum == -1 ? 0 : BspNum & ~NF_SUBSECTOR;
		subsector_t* Sub = &Level->Subsectors[SubNum];
		if (!Sub->sector->linecount)
		{
			//	Skip sectors containing original polyobjs
			return;
		}
		if (!(LeafPvs[SubNum >> 3] & (1 << (SubNum & 7))))
		{
			return;
		}

		if (!Clipper.ClipCheckSubsector(Sub))
		{
			return;
		}
		Clipper.ClipAddSubsectorSegs(Sub);
		UpdatePvs[SubNum >> 3] |= 1 << (SubNum & 7);
		return;
	}

	node_t* Bsp = &Level->Nodes[BspNum];

	// Decide which side the view point is on.
	int Side = Bsp->PointOnSide(Owner->ViewOrg);

	// Recursively divide front space.
	SetUpPvsNode(Bsp->children[Side], Bsp->bbox[Side]);

	// Divide back space.
	SetUpPvsNode(Bsp->children[Side ^ 1], Bsp->bbox[Side ^ 1]);
	unguard;
}

//==========================================================================
//
//	VNetConnection::CheckFatPVS
//
//==========================================================================

int VNetConnection::CheckFatPVS(subsector_t* Subsector)
{
	guardSlow(VNetConnection::CheckFatPVS);
	int ss = Subsector - Context->GetLevel()->Subsectors;
	return UpdatePvs[ss / 8] & (1 << (ss & 7));
	unguardSlow;
}

//==========================================================================
//
//	VNetConnection::SecCheckFatPVS
//
//==========================================================================

bool VNetConnection::SecCheckFatPVS(sector_t* Sec)
{
	guardSlow(VNetConnection::SecCheckFatPVS);
	for (subsector_t* Sub = Sec->subsectors; Sub; Sub = Sub->seclink)
	{
		if (CheckFatPVS(Sub))
		{
			return true;
		}
	}
	return false;
	unguardSlow;
}
