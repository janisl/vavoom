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
	guard(SV_ReadMove);
    ticcmd_t	cmd;

	sv_player->ViewAngles.yaw = ByteToAngle(net_msg.ReadByte());
	sv_player->ViewAngles.pitch = ByteToAngle(net_msg.ReadByte());
	sv_player->ViewAngles.roll = ByteToAngle(net_msg.ReadByte());
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

void SV_RunClientCommand(const char *cmd)
{
	guard(SV_RunClientCommand);
	Cmd_ExecuteString(cmd, src_client);
	unguard;
}

//==========================================================================
//
//	SV_ReadFromUserInfo
//
//==========================================================================

void SV_ReadFromUserInfo(void)
{
	guard(SV_ReadFromUserInfo);
	if (!sv_loading)
	{
		sv_player->BaseClass = atoi(Info_ValueForKey(sv_player->UserInfo, "class"));
	}
	strcpy(sv_player->PlayerName, Info_ValueForKey(sv_player->UserInfo, "name"));
	sv_player->Color = atoi(Info_ValueForKey(sv_player->UserInfo, "color"));
	sv_player->eventUserinfoChanged();
	unguard;
}

//==========================================================================
//
//	SV_SetUserInfo
//
//==========================================================================

void SV_SetUserInfo(const char *info)
{
	guard(SV_SetUserInfo);
	if (!sv_loading)
	{
		strcpy(sv_player->UserInfo, info);
		SV_ReadFromUserInfo();
		sv_reliable << (byte)svc_userinfo
					<< (byte)SV_GetPlayerNum(sv_player)
					<< sv_player->UserInfo;
	}
	unguard;
}

//==========================================================================
//
//	SV_ReadClientMessages
//
//==========================================================================

bool SV_ReadClientMessages(int clientnum)
{
	guard(SV_ReadClientMessages);
	int			ret;
	byte		cmd_type;

	sv_player = GGameInfo->Players[clientnum];
	sv_player->bNeedsUpdate = false;
	do
	{
		ret = NET_GetMessage(sv_player->NetCon);
		if (ret == -1)
		{
			GCon->Log(NAME_DevNet, "Bad read");
			return false;
		}

		if (ret == 0)
			return true;

		sv_player->bNeedsUpdate = true;

		net_msg.BeginReading();

		while (1)
		{
			if (net_msg.badread)
			{
				GCon->Log(NAME_DevNet, "Packet corupted");
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
				SV_SetUserInfo(net_msg.ReadString());
				break;

			 case clc_stringcmd:
				SV_RunClientCommand(net_msg.ReadString());
				break;

			 default:
				GCon->Log(NAME_DevNet, "Invalid command");
				return false;
			}
		}
	} while (ret == 1);

	return true;
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
	if (cmd_source != src_client)
	{
		GCon->Log("SetInfo is not valid from console");
		return;
	}

	if (Argc() != 3)
	{
		return;
	}

	Info_SetValueForKey(sv_player->UserInfo, Argv(1), Argv(2));
	sv_reliable << (byte)svc_setinfo
				<< (byte)SV_GetPlayerNum(sv_player)
				<< Argv(1)
				<< Argv(2);
	SV_ReadFromUserInfo();
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.16  2006/02/15 23:28:18  dj_jl
//	Moved all server progs global variables to classes.
//
//	Revision 1.15  2004/12/27 12:23:17  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.14  2003/11/12 16:47:40  dj_jl
//	Changed player structure into a class
//	
//	Revision 1.13  2003/07/11 16:45:20  dj_jl
//	Made array of players with pointers
//	
//	Revision 1.12  2003/03/08 16:02:53  dj_jl
//	A little multiplayer fix.
//	
//	Revision 1.11  2002/07/23 16:29:56  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.10  2002/07/13 07:50:58  dj_jl
//	Added guarding.
//	
//	Revision 1.9  2002/02/15 19:12:04  dj_jl
//	Property namig style change
//	
//	Revision 1.8  2002/01/17 18:21:40  dj_jl
//	Fixed Hexen class bug
//	
//	Revision 1.7  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.6  2001/12/01 17:40:41  dj_jl
//	Added support for bots
//	
//	Revision 1.5  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.4  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
