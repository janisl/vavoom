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

VChannel::VChannel()
{
}

//==========================================================================
//
//	VChannel::~VChannel
//
//==========================================================================

VChannel::~VChannel()
{
}

//==========================================================================
//
//	VPlayerChannel::VPlayerChannel
//
//==========================================================================

VPlayerChannel::VPlayerChannel()
: Plr(NULL)
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
	PlayerNet = NULL;

	if (Plr)
	{
		PlayerNet = Plr->Net;

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
	VMessageOut& Msg = sv_player->Net->Message;
	vuint8* Data = (vuint8*)Plr;
	for (VField* F = Plr->GetClass()->NetFields; F; F = F->NextNetField)
	{
		if (!FieldCondValues[F->NetIndex])
		{
			continue;
		}
		if (!VField::IdenticalValue(Data + F->Ofs, OldData + F->Ofs, F->Type))
		{
			Msg << (vuint8)svc_set_player_prop;
			Msg << (vuint8)F->NetIndex;
			VField::NetSerialiseValue(Msg, Data + F->Ofs, F->Type);
			VField::CopyFieldValue(Data + F->Ofs, OldData + F->Ofs, F->Type);
		}
	}
	unguard;
}

//==========================================================================
//
//	VPlayerNetInfo::VPlayerNetInfo
//
//==========================================================================

VPlayerNetInfo::VPlayerNetInfo()
: NetCon(NULL)
, Message(OUT_MESSAGE_SIZE)
, MobjUpdateStart(0)
, LastMessage(0)
, NeedsUpdate(false)
, EntChan(NULL)
, Messages(NULL)
{
	EntChan = new VEntityChannel[GMaxEntities];
}

//==========================================================================
//
//	VPlayerNetInfo::~VPlayerNetInfo
//
//==========================================================================

VPlayerNetInfo::~VPlayerNetInfo()
{
	delete[] EntChan;
	Chan.SetPlayer(NULL);
}

//==========================================================================
//
//	VPlayerNetInfo::SetNetCon
//
//==========================================================================

void VPlayerNetInfo::SetNetCon(VSocketPublic* ANetCon)
{
	guard(VPlayerNetInfo::SetNetCon);
	NetCon = ANetCon;
	unguard;
}

//==========================================================================
//
//	VPlayerNetInfo::GetMessages
//
//==========================================================================

bool VPlayerNetInfo::GetMessages()
{
	guard(VPlayerNetInfo::GetMessages);
	int ret;

	do
	{
		VMessageIn* Msg = NULL;
		ret = GetRawMessage(Msg);
		if (ret == -1)
		{
			GCon->Log(NAME_DevNet, "Bad read");
			return false;
		}

		if (ret)
		{
			if (ret == 1)
			{
				int Tmp;
				*Msg << Tmp;
			}
			if (!ParsePacket(*Msg))
			{
				delete Msg;
				return false;
			}
			delete Msg;
		}

		//	This is for client connection which closes the connection on
		// disconnect command.
		if (!NetCon)
		{
			return true;
		}
	} while (ret > 0);

	return true;
	unguard;
}

//==========================================================================
//
//	VPlayerNetInfo::GetRawMessage
//
//==========================================================================

int VPlayerNetInfo::GetRawMessage(VMessageIn*& Msg)
{
	guard(VPlayerNetInfo::GetRawMessage);
	checkSlow(NetCon);
	TArray<vuint8> Data;
	int Ret = NetCon->GetMessage(Data);
	if (Ret > 0)
	{
		check(Data.Num());
		vuint8 LastByte = Data[Data.Num() - 1];
		if (LastByte == 0)
		{
			GCon->Logf(NAME_DevNet, "Packet is missing trailing bit");
			return 0;
		}
		//	Find out real length by stepping back untill the trailing bit.
		vuint32 Length = Data.Num() * 8 - 1;
		for (vuint8 Mask = 0x80; !(LastByte & Mask); Mask >>= 1)
		{
			Length--;
		}
		Msg = new VMessageIn(Data.Ptr(), Length);
		Msg->MessageType = Ret;
	}
	return Ret;
	unguard;
}

//==========================================================================
//
//	VPlayerNetInfo::SendMessage
//
//==========================================================================

int VPlayerNetInfo::SendMessage(VMessageOut* Msg, bool Reliable)
{
	guard(VPlayerNetInfo::SendMessage);
	VBitStreamWriter	Out(MAX_MSGLEN * 8);

	if (Reliable)
	{
		int Tmp = 0;
		Out << Tmp;
	}
	Out.SerialiseBits(Msg->GetData(), Msg->GetNumBits());
	//	Add trailing bit so we can find out how many bits the message has.
	Out.WriteBit(true);
	//	Pad it with zero bits untill byte boundary.
	while (Out.GetNumBits() & 7)
	{
		Out.WriteBit(false);
	}

	if (Reliable)
	{
		return NetCon->SendMessage(Out.GetData(), Out.GetNumBytes());
	}
	else
	{
		return NetCon->SendUnreliableMessage(Out.GetData(), Out.GetNumBytes());
	}
	unguard;
}

//==========================================================================
//
//	VPlayerNetInfo::CanSendMessage
//
//==========================================================================

bool VPlayerNetInfo::CanSendMessage()
{
	guard(VPlayerNetInfo::CanSendMessage);
	return NetCon->CanSendMessage();
	unguard;
}

//==========================================================================
//
//	VPlayerNetInfo::IsLocalConnection
//
//==========================================================================

bool VPlayerNetInfo::IsLocalConnection()
{
	guard(VPlayerNetInfo::IsLocalConnection);
	return NetCon->IsLocalConnection();
	unguard;
}

//==========================================================================
//
//	VPlayerNetInfo::CloseSocket
//
//==========================================================================

void VPlayerNetInfo::CloseSocket()
{
	guard(VPlayerNetInfo::CloseSocket);
	if (NetCon)
	{
		NetCon->Close();
	}
	NetCon = NULL;
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
//	VServerPlayerNetInfo::ParsePacket
//
//==========================================================================

bool VServerPlayerNetInfo::ParsePacket(VMessageIn& msg)
{
	NeedsUpdate = true;

	while (1)
	{
		if (msg.IsError())
		{
			GCon->Log(NAME_DevNet, "Packet corupted");
			return false;
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
			return false;

		case clc_player_info:
			SV_SetUserInfo(msg.ReadString());
			break;

		case clc_stringcmd:
			SV_RunClientCommand(msg.ReadString());
			break;

		default:
			GCon->Log(NAME_DevNet, "Invalid command");
			return false;
		}
	}
	return true;
}

//==========================================================================
//
//	SV_ReadClientMessages
//
//==========================================================================

bool SV_ReadClientMessages(int clientnum)
{
	guard(SV_ReadClientMessages);
	sv_player = GGameInfo->Players[clientnum];
	sv_player->Net->NeedsUpdate = false;
	return sv_player->Net->GetMessages();
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

