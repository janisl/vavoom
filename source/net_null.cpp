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
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "net_loc.h"
#include "net_null.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

bool		net_connect_bot = false;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	NetNull_Init
//
//==========================================================================

int NetNull_Init(void)
{
	return 0;
}

//==========================================================================
//
//	NetNull_Listen
//
//==========================================================================

void NetNull_Listen(boolean)
{
}

//==========================================================================
//
//	NetNull_SearchForHosts
//
//==========================================================================

void NetNull_SearchForHosts(boolean)
{
}

//==========================================================================
//
//	NetNull_Connect
//
//==========================================================================

qsocket_t *NetNull_Connect(char *)
{
	return NULL;
}

//==========================================================================
//
//	NetNull_CheckNewConnections
//
//==========================================================================

qsocket_t *NetNull_CheckNewConnections(void)
{
	qsocket_t *sock;

	if (!net_connect_bot)
		return NULL;

	net_connect_bot = false;
	sock = NET_NewQSocket();
	if (!sock)
	{
		con << "Server is full\n";
		return NULL;
	}
	strcpy(sock->address, "NULL");
	return sock;
}

//==========================================================================
//
//	NetNull_GetMessage
//
//==========================================================================

int NetNull_GetMessage(qsocket_t *)
{
	return 0;
}

//==========================================================================
//
//	NetNull_SendMessage
//
//==========================================================================

int NetNull_SendMessage(qsocket_t *, TSizeBuf *)
{
	return 1;
}

//==========================================================================
//
//	NetNull_SendUnreliableMessage
//
//==========================================================================

int NetNull_SendUnreliableMessage(qsocket_t *, TSizeBuf *)
{
	return 1;
}

//==========================================================================
//
//	NetNull_CanSendMessage
//
//==========================================================================

boolean NetNull_CanSendMessage(qsocket_t *)
{
	return true;
}

//==========================================================================
//
//	NetNull_CanSendUnreliableMessage
//
//==========================================================================

boolean NetNull_CanSendUnreliableMessage(qsocket_t *)
{
	return true;
}

//==========================================================================
//
//	NetNull_Close
//
//==========================================================================

void NetNull_Close(qsocket_t *)
{
}

//==========================================================================
//
//	NetNull_Shutdown
//
//==========================================================================

void NetNull_Shutdown(void)
{
}

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2001/12/01 17:40:41  dj_jl
//	Added support for bots
//
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
