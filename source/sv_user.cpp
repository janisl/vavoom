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

static TCvarI		sv_maxmove("sv_maxmove", "400", CVAR_ARCHIVE);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	SV_ReadMove
//
//==========================================================================

void SV_ReadMove(void)
{
    ticcmd_t	cmd;

	if (sv_player->mo)
	{
		sv_player->mo->angles.yaw = net_msg.ReadByte() << 24;
		sv_player->mo->angles.pitch = net_msg.ReadByte() << 24;
		sv_player->mo->angles.roll = net_msg.ReadByte() << 24;
	}
	else
	{
		cond << "Move with no body\n";
		net_msg.ReadByte();
		net_msg.ReadByte();
		net_msg.ReadByte();
	}
	net_msg >> cmd.forwardmove
			>> cmd.sidemove
			>> cmd.flymove
			>> cmd.buttons
			>> cmd.impulse;

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

	sv_player->forwardmove = cmd.forwardmove;
	sv_player->sidemove = cmd.sidemove;
	sv_player->flymove = cmd.flymove;
	sv_player->buttons = cmd.buttons;
	if (cmd.impulse)
	{
		sv_player->impulse = cmd.impulse;
	}
}

//==========================================================================
//
//	SV_RunClientCommand
//
//==========================================================================

void SV_RunClientCommand(const char *cmd)
{
	Cmd_ExecuteString(cmd, src_client);
}

//==========================================================================
//
//	SV_ReadFromUserInfo
//
//==========================================================================

void SV_ReadFromUserInfo(void)
{
	if (!sv_loading)
	{
		sv_player->baseclass = atoi(Info_ValueForKey(sv_player->userinfo, "class"));
	}
	strcpy(sv_player->name, Info_ValueForKey(sv_player->userinfo, "name"));
	sv_player->color = atoi(Info_ValueForKey(sv_player->userinfo, "color"));
	svpr.Exec("UserinfoChanged", (int)sv_player);
}

//==========================================================================
//
//	SV_ReadClientMessages
//
//==========================================================================

bool SV_ReadClientMessages(int clientnum)
{
	int			ret;
	byte		cmd_type;

	sv_player = &players[clientnum];
	do
	{
		ret = NET_GetMessage(sv_player->netcon);
		if (ret == -1)
		{
			cond << "Bad read\n";
			return false;
		}

		if (ret == 0)
			return true;

		net_msg.BeginReading();

		while (1)
		{
			if (net_msg.badread)
			{
				cond << "Packet corupted";
				return false;
			}

			net_msg >> cmd_type;

			if (net_msg.badread)
				break; // Here this means end of packet

			switch (cmd_type)
			{
			 case clc_nop:
				break;

			 case clc_move:
				SV_ReadMove();
				break;

			 case clc_disconnect:
				return false;
	
			 case clc_player_info:
				strcpy(sv_player->userinfo, net_msg.ReadString());
				SV_ReadFromUserInfo();
				sv_reliable << (byte)svc_userinfo
							<< (byte)(sv_player - players)
							<< sv_player->userinfo;
				break;

			 case clc_stringcmd:
				SV_RunClientCommand(net_msg.ReadString());
				break;

			 default:
				cond << "Invalid command\n";
				return false;
			}
		}
	} while (ret == 1);

	return true;
}

//==========================================================================
//
//	COMMAND SetInfo
//
//==========================================================================

COMMAND(SetInfo)
{
	if (cmd_source != src_client)
	{
		con << "SetInfo is not valid from console\n";
		return;
	}

	if (Argc() != 3)
	{
		return;
	}

	Info_SetValueForKey(sv_player->userinfo, Argv(1), Argv(2));
	sv_reliable << (byte)svc_setinfo
				<< (byte)(sv_player - players)
				<< Argv(1)
				<< Argv(2);
	SV_ReadFromUserInfo();
}

