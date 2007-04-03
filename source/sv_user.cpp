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
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern bool			sv_loading;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VCvarI		sv_maxmove("sv_maxmove", "400", CVAR_Archive);

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
, InMsg(NULL)
, OutMsg(NULL)
, ReceiveSequence(0)
, SendSequence(0)
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
	if (Msg.bReliable && Msg.Sequence < ReceiveSequence)
	{
		Connection->Driver->receivedDuplicateCount++;
		return;
	}

	if (Msg.bReliable && Msg.Sequence > ReceiveSequence)
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
		ReceiveSequence++;
	}

	ParsePacket(Msg);
	if (Msg.bClose)
	{
		delete this;
		return;
	}

	while (InMsg && InMsg->Sequence == ReceiveSequence)
	{
		VMessageIn* OldMsg = InMsg;
		InMsg = OldMsg->Next;
		ReceiveSequence++;
		ParsePacket(*OldMsg);
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
		Msg->Sequence = SendSequence;

		VMessageOut* Copy = new VMessageOut(*Msg);
		Copy->Next = NULL;
		VMessageOut** pNext = &OutMsg;
		while (*pNext)
		{
			pNext = &(*pNext)->Next;
		}
		*pNext = Copy;
		Msg = Copy;

		SendSequence++;
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
	//	Clean up messages that have been ACK-ed
	for (VMessageOut** pMsg = &OutMsg; *pMsg;)
	{
		if ((*pMsg)->bReceivedAck)
		{
			VMessageOut* Msg = *pMsg;
			*pMsg = Msg->Next;
			delete Msg;
		}
		else
		{
			pMsg = &(*pMsg)->Next;
		}
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
	VMessageOut Msg(this);
	Msg.bReliable = true;
	Msg.bClose = true;
	SendMessage(&Msg);
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

//==========================================================================
//
//	VPlayerChannel::VPlayerChannel
//
//==========================================================================

VPlayerChannel::VPlayerChannel(VNetConnection* AConnection, vint32 AIndex,
	vuint8 AOpenedLocally)
: VChannel(AConnection, CHANNEL_Player, AIndex, AOpenedLocally)
, Plr(NULL)
, OldData(NULL)
, NewObj(false)
, FieldCondValues(NULL)
{
}

//==========================================================================
//
//	VPlayerChannel::~VPlayerChannel
//
//==========================================================================

VPlayerChannel::~VPlayerChannel()
{
	SetPlayer(NULL);
}

//==========================================================================
//
//	VPlayerChannel::SetPlayer
//
//==========================================================================

void VPlayerChannel::SetPlayer(VBasePlayer* APlr)
{
	guard(VPlayerChannel::SetPlayer);
	if (Plr)
	{
		for (VField* F = Plr->GetClass()->NetFields; F; F = F->NextNetField)
		{
			VField::CleanField(OldData + F->Ofs, F->Type);
		}
		if (OldData)
		{
			delete[] OldData;
			OldData = NULL;
		}
		if (FieldCondValues)
		{
			delete[] FieldCondValues;
			FieldCondValues = NULL;
		}
	}

	Plr = APlr;

	if (Plr)
	{
		VBasePlayer* Def = (VBasePlayer*)Plr->GetClass()->Defaults;
		OldData = new vuint8[Plr->GetClass()->ClassSize];
		memset(OldData, 0, Plr->GetClass()->ClassSize);
		for (VField* F = Plr->GetClass()->NetFields; F; F = F->NextNetField)
		{
			VField::CopyFieldValue((vuint8*)Def + F->Ofs, OldData + F->Ofs,
				F->Type);
		}
		FieldCondValues = new vuint8[Plr->GetClass()->NumNetFields];
		NewObj = true;
	}
	unguard;
}

//==========================================================================
//
//	VPlayerChannel::Update
//
//==========================================================================

void VPlayerChannel::Update()
{
	guard(VPlayerChannel::Update);
	EvalCondValues(Plr, Plr->GetClass(), FieldCondValues);

	VMessageOut Msg(this);
	Msg.bReliable = true;
	vuint8* Data = (vuint8*)Plr;
	for (VField* F = Plr->GetClass()->NetFields; F; F = F->NextNetField)
	{
		if (!FieldCondValues[F->NetIndex])
		{
			continue;
		}
		if (!VField::IdenticalValue(Data + F->Ofs, OldData + F->Ofs, F->Type))
		{
			Msg << (vuint8)F->NetIndex;
			VField::NetSerialiseValue(Msg, Data + F->Ofs, F->Type);
			VField::CopyFieldValue(Data + F->Ofs, OldData + F->Ofs, F->Type);
		}
	}

	if (Msg.GetNumBits())
	{
		SendMessage(&Msg);
	}
	unguard;
}

//==========================================================================
//
//	VPlayerChannel::ParsePacket
//
//==========================================================================

void VPlayerChannel::ParsePacket(VMessageIn& Msg)
{
	guard(VPlayerChannel::ParsePacket);
	while (!Msg.AtEnd())
	{
		int FldIdx = Msg.ReadByte();
		VField* F = NULL;
		for (VField* CF = Plr->GetClass()->NetFields; CF; CF = CF->NextNetField)
		{
			if (CF->NetIndex == FldIdx)
			{
				F = CF;
				break;
			}
		}
		if (!F)
		{
			Sys_Error("Bad net field %d", FldIdx);
		}
		VField::NetSerialiseValue(Msg, (vuint8*)Plr + F->Ofs, F->Type);
	}
	unguard;
}

//==========================================================================
//
//	VNetConnection::VNetConnection
//
//==========================================================================

VNetConnection::VNetConnection(VSocketPublic* ANetCon)
: NetCon(ANetCon)
, Driver(GNet)
, State(NETCON_Open)
, Message(OUT_MESSAGE_SIZE)
, LastMessage(0)
, NeedsUpdate(false)
, Out(MAX_MSGLEN * 8)
{
	memset(Channels, 0, sizeof(Channels));
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
	if (Sequence < NetCon->UnreliableReceiveSequence)
	{
		GCon->Log(NAME_DevNet, "Got a stale datagram");
		return;
	}
	if (Sequence != NetCon->UnreliableReceiveSequence)
	{
		int count = Sequence - NetCon->UnreliableReceiveSequence;
		Driver->droppedDatagrams += count;
		GCon->Logf(NAME_DevNet, "Dropped %d datagram(s)", count);
	}
	NetCon->UnreliableReceiveSequence = Sequence + 1;

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
			if (AckSeq == NetCon->AckSequence)
			{
				NetCon->AckSequence++;
			}
			else if (AckSeq > NetCon->AckSequence)
			{
				NetCon->AckSequence = AckSeq + 1;
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
					case CHANNEL_Entity:
						Chan = new VEntityChannel(this, Msg.ChanIndex, false);
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
	Msg.PacketId = NetCon->UnreliableSendSequence;
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
		Out << NetCon->UnreliableSendSequence;
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
	NetCon->UnreliableSendSequence++;

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

	//	Run tick for all of the open channels.
	for (int i = OpenChannels.Num() - 1; i >= 0; i--)
	{
		OpenChannels[i]->Tick();
	}

	Flush();
	unguard;
}

//==========================================================================
//
//	SV_ReadMove
//
//==========================================================================

void SV_ReadMove(VMessageIn& msg)
{
	guard(SV_ReadMove);
    ticcmd_t	cmd;

	sv_player->ViewAngles.yaw = ByteToAngle(msg.ReadByte());
	sv_player->ViewAngles.pitch = ByteToAngle(msg.ReadByte());
	sv_player->ViewAngles.roll = ByteToAngle(msg.ReadByte());
	msg << cmd.forwardmove
		<< cmd.sidemove
		<< cmd.flymove
		<< cmd.buttons
		<< cmd.impulse;

	// Don't move faster than maxmove
	if (cmd.forwardmove > sv_maxmove)
	{
		cmd.forwardmove = sv_maxmove;
	}
	else if (cmd.forwardmove < -sv_maxmove)
	{
		cmd.forwardmove = -sv_maxmove;
	}
	if (cmd.sidemove > sv_maxmove)
	{
		cmd.sidemove = sv_maxmove;
	}
	else if (cmd.sidemove < -sv_maxmove)
	{
		cmd.sidemove = -sv_maxmove;
	}

	sv_player->ForwardMove = cmd.forwardmove;
	sv_player->SideMove = cmd.sidemove;
	sv_player->FlyMove = cmd.flymove;
	sv_player->Buttons = cmd.buttons;
	if (cmd.impulse)
	{
		sv_player->Impulse = cmd.impulse;
	}
	unguard;
}

//==========================================================================
//
//	SV_RunClientCommand
//
//==========================================================================

void SV_RunClientCommand(const VStr& cmd)
{
	guard(SV_RunClientCommand);
	VCommand::ExecuteString(cmd, VCommand::SRC_Client);
	unguard;
}

//==========================================================================
//
//	SV_ReadFromUserInfo
//
//==========================================================================

void SV_ReadFromUserInfo()
{
	guard(SV_ReadFromUserInfo);
	if (!sv_loading)
	{
		sv_player->BaseClass = atoi(*Info_ValueForKey(sv_player->UserInfo, "class"));
	}
	sv_player->PlayerName = Info_ValueForKey(sv_player->UserInfo, "name");
	sv_player->Colour = atoi(*Info_ValueForKey(sv_player->UserInfo, "colour"));
	sv_player->eventUserinfoChanged();
	unguard;
}

//==========================================================================
//
//	SV_SetUserInfo
//
//==========================================================================

void SV_SetUserInfo(const VStr& info)
{
	guard(SV_SetUserInfo);
	if (!sv_loading)
	{
		sv_player->UserInfo = info;
		SV_ReadFromUserInfo();
		*sv_reliable << (byte)svc_userinfo
					<< (byte)SV_GetPlayerNum(sv_player)
					<< sv_player->UserInfo;
	}
	unguard;
}

//==========================================================================
//
//	VServerGenChannel::ParsePacket
//
//==========================================================================

void VServerGenChannel::ParsePacket(VMessageIn& msg)
{
	guard(VServerGenChannel::ParsePacket);
	Connection->NeedsUpdate = true;

	while (1)
	{
		if (msg.IsError())
		{
			GCon->Log(NAME_DevNet, "Packet corupted");
			Connection->State = NETCON_Closed;
			return;
		}

		vuint8 cmd_type;
		msg << cmd_type;

		if (msg.IsError())
			break; // Here this means end of packet

		switch (cmd_type)
		{
		case clc_nop:
			break;

		case clc_move:
			SV_ReadMove(msg);
			break;

		case clc_disconnect:
			Connection->State = NETCON_Closed;
			return;

		case clc_player_info:
			SV_SetUserInfo(msg.ReadString());
			break;

		case clc_stringcmd:
			SV_RunClientCommand(msg.ReadString());
			break;

		default:
			GCon->Log(NAME_DevNet, "Invalid command");
			Connection->State = NETCON_Closed;
			return;
		}
	}
	unguard;
}

//==========================================================================
//
//	VServerPlayerNetInfo::GetLevel
//
//==========================================================================

VLevel* VServerPlayerNetInfo::GetLevel()
{
	return GLevel;
}

//==========================================================================
//
//	SV_ReadClientMessages
//
//==========================================================================

void SV_ReadClientMessages(int clientnum)
{
	guard(SV_ReadClientMessages);
	sv_player = GGameInfo->Players[clientnum];
	sv_player->Net->NeedsUpdate = false;
	sv_player->Net->GetMessages();
	unguard;
}

//==========================================================================
//
//	COMMAND SetInfo
//
//==========================================================================

COMMAND(SetInfo)
{
	guard(COMMAND SetInfo);
	if (Source != SRC_Client)
	{
		GCon->Log("SetInfo is not valid from console");
		return;
	}

	if (Args.Num() != 3)
	{
		return;
	}

	Info_SetValueForKey(sv_player->UserInfo, *Args[1], *Args[2]);
	*sv_reliable << (byte)svc_setinfo
				<< (byte)SV_GetPlayerNum(sv_player)
				<< Args[1]
				<< Args[2];
	SV_ReadFromUserInfo();
	unguard;
}

//==========================================================================
//
//	Natives.
//
//==========================================================================

IMPLEMENT_FUNCTION(VBasePlayer, cprint)
{
	VStr msg = PF_FormatString();
	P_GET_SELF;
	SV_ClientPrintf(Self, *msg);
}

IMPLEMENT_FUNCTION(VBasePlayer, centreprint)
{
	VStr msg = PF_FormatString();
	P_GET_SELF;
	SV_ClientCentrePrintf(Self, *msg);
}

IMPLEMENT_FUNCTION(VBasePlayer, GetPlayerNum)
{
	P_GET_SELF;
	RET_INT(SV_GetPlayerNum(Self));
}

IMPLEMENT_FUNCTION(VBasePlayer, ClearPlayer)
{
	P_GET_SELF;

	Self->PClass = 0;
	Self->ForwardMove = 0;
	Self->SideMove = 0;
	Self->FlyMove = 0;
	Self->Buttons = 0;
	Self->Impulse = 0;
	Self->MO = NULL;
	Self->PlayerState = 0;
	Self->ViewOrg = TVec(0, 0, 0);
	Self->PlayerFlags &= ~VBasePlayer::PF_FixAngle;
	Self->Health = 0;
	Self->Items = 0;
	Self->PlayerFlags &= ~VBasePlayer::PF_AttackDown;
	Self->PlayerFlags &= ~VBasePlayer::PF_UseDown;
	Self->ExtraLight = 0;
	Self->FixedColourmap = 0;
	memset(Self->CShifts, 0, sizeof(Self->CShifts));
	Self->PSpriteSY = 0;
	memset((vuint8*)Self + sizeof(VBasePlayer), 0,
		Self->GetClass()->ClassSize - sizeof(VBasePlayer));
}

IMPLEMENT_FUNCTION(VBasePlayer, SelectClientMsg)
{
	P_GET_INT(msgtype);
	P_GET_SELF;
	switch (msgtype)
	{
	case MSG_SV_CLIENT:
		pr_msg = &Self->Net->Message;
		break;
	}
}

