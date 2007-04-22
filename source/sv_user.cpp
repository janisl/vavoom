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
#include "progdefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VServerGenChannel : public VChannel
{
public:
	VServerGenChannel(VNetConnection* AConnection, vint32 AIndex, vuint8 AOpenedLocally = true)
	: VChannel(AConnection, CHANNEL_General, AIndex, AOpenedLocally)
	{}

	//	VChannel interface
	void ParsePacket(VMessageIn&);
};

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
//	VNetContext::VNetContext
//
//==========================================================================

VNetContext::VNetContext()
{
}

//==========================================================================
//
//	VNetContext::~VNetContext
//
//==========================================================================

VNetContext::~VNetContext()
{
}

//==========================================================================
//
//	EvalCond
//
//==========================================================================

static bool EvalCond(VObject* Obj, VClass* Class, VMethod* M)
{
	guard(EvalCond);
	for (int i = 0; i < Class->RepInfos.Num(); i++)
	{
		for (int j = 0; j < Class->RepInfos[i].RepMembers.Num(); j++)
		{
			if (Class->RepInfos[i].RepMembers[j]->MemberType != MEMBER_Method)
				continue;
			if (Class->RepInfos[i].RepMembers[j]->Name != M->Name)
				continue;
			P_PASS_REF(Obj);
			return !!VObject::ExecuteFunction(Class->RepInfos[i].Cond).i;
		}
	}
	if (Class->GetSuperClass())
	{
		return EvalCond(Obj, Class->GetSuperClass(), M);
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VBasePlayer::ExecuteNetMethod
//
//==========================================================================

bool VBasePlayer::ExecuteNetMethod(VMethod* Func)
{
	if (!EvalCond(this, GetClass(), Func))
	{
		return false;
	}

	VMessageOut Msg(Net->Channels[CHANIDX_Player]);
	Msg.bReliable = !!(Func->Flags & FUNC_NetReliable);

	Msg.WriteInt(Func->NetIndex, GetClass()->NumNetFields);

	//	Serialise arguments
	guard(SerialiseArguments);
	VStack* Param = pr_stackPtr - Func->ParamsSize + 1;	//	Skip self
	for (int i = 0; i < Func->NumParams; i++)
	{
		switch (Func->ParamTypes[i].Type)
		{
		case ev_int:
		case ev_name:
		case ev_bool:
			VField::NetSerialiseValue(Msg, (vuint8*)&Param->i, Func->ParamTypes[i]);
			Param++;
			break;
		case ev_float:
			VField::NetSerialiseValue(Msg, (vuint8*)&Param->f, Func->ParamTypes[i]);
			Param++;
			break;
		case ev_string:
		case ev_pointer:
		case ev_reference:
		case ev_class:
		case ev_state:
			VField::NetSerialiseValue(Msg, (vuint8*)&Param->p, Func->ParamTypes[i]);
			Param++;
			break;
		case ev_vector:
			{
				TVec Vec;
				Vec.x = Param[0].f;
				Vec.y = Param[1].f;
				Vec.z = Param[2].f;
				VField::NetSerialiseValue(Msg, (vuint8*)&Vec, Func->ParamTypes[i]);
				Param += 3;
			}
			break;
		default:
			Sys_Error("Bad method argument type %d", Func->ParamTypes[i].Type);
		}
	}
	unguard;

	//	Send it.
	Net->Channels[CHANIDX_Player]->SendMessage(&Msg);

	//	Clean up parameters
	guard(CleanUp);
	VStack* Param = pr_stackPtr - Func->ParamsSize + 1;	//	Skip self
	for (int i = 0; i < Func->NumParams; i++)
	{
		switch (Func->ParamTypes[i].Type)
		{
		case ev_int:
		case ev_name:
		case ev_bool:
		case ev_float:
		case ev_pointer:
		case ev_reference:
		case ev_class:
		case ev_state:
			Param++;
			break;
		case ev_string:
			((VStr*)&Param->p)->Clean();
			Param++;
			break;
		case ev_vector:
			Param += 3;
			break;
		default:
			Sys_Error("Bad method argument type %d", Func->ParamTypes[i].Type);
		}
	}
	pr_stackPtr -= Func->ParamsSize;
	unguard;

	//	Push null return value
	guard(RetVal);
	switch (Func->ReturnType.Type)
	{
	case ev_void:
		break;
	case ev_int:
	case ev_name:
	case ev_bool:
		PR_Push(0);
		break;
	case ev_float:
		PR_Pushf(0);
		break;
	case ev_string:
		PR_PushStr(VStr());
		break;
	case ev_pointer:
	case ev_reference:
	case ev_class:
	case ev_state:
		PR_PushPtr(NULL);
		break;
	case ev_vector:
		PR_Pushf(0);
		PR_Pushf(0);
		PR_Pushf(0);
		break;
	default:
		Sys_Error("Bad return value type");
	}
	unguard;

	//	It's been handled here.
	return true;
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
//	VServerNetContext::GetLevel
//
//==========================================================================

VLevel* VServerNetContext::GetLevel()
{
	return GLevel;
}

//==========================================================================
//
//	VServerNetContext::CreateGenChannel
//
//==========================================================================

VChannel* VServerNetContext::CreateGenChannel(VNetConnection* Connection)
{
	return new VServerGenChannel(Connection, 0);
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

	vuint8* Def = Self->GetClass()->Defaults;
	for (VField* F = Self->GetClass()->Fields; F; F = F->Next)
	{
		VField::CopyFieldValue(Def + F->Ofs, (vuint8*)Self + F->Ofs, F->Type);
	}
}
