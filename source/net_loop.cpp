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
#include "net_loop.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static boolean		localconnectpending = false;
static qsocket_t	*loop_client = NULL;
static qsocket_t	*loop_server = NULL;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	Loop_Init
//
//==========================================================================

int Loop_Init(void)
{
#ifdef CLIENT
	if (cls.state == ca_dedicated)
		return -1;
	return 0;
#else
	return -1;
#endif
}

//==========================================================================
//
//	Loop_Listen
//
//==========================================================================

void Loop_Listen(boolean)
{
}

//==========================================================================
//
//	Loop_SearchForHosts
//
//==========================================================================

void Loop_SearchForHosts(boolean)
{
#ifdef SERVER
	if (!sv.active)
		return;

	hostCacheCount = 1;
	if (strcmp(hostname, "UNNAMED") == 0)
		strcpy(hostcache[0].name, "local");
	else
		strcpy(hostcache[0].name, hostname);
	strncpy(hostcache[0].map, level.mapname, 15);
	hostcache[0].users = svs.num_connected;
	hostcache[0].maxusers = svs.max_clients;
	strcpy(hostcache[0].cname, "local");
#endif
}

//==========================================================================
//
//	Loop_Connect
//
//==========================================================================

qsocket_t *Loop_Connect(char *host)
{
	if (strcmp(host,"local") != 0)
		return NULL;
	
	localconnectpending = true;

	if (!loop_client)
	{
		loop_client = NET_NewQSocket();
		if (loop_client == NULL)
		{
			con << "Loop_Connect: no qsocket available\n";
			return NULL;
		}
		strcpy(loop_client->address, "localhost");
	}
	loop_client->receiveMessageLength = 0;
	loop_client->sendMessageLength = 0;
	loop_client->canSend = true;

	if (!loop_server)
	{
		loop_server = NET_NewQSocket();
		if (loop_server == NULL)
		{
			con << "Loop_Connect: no qsocket available\n";
			return NULL;
		}
		strcpy(loop_server->address, "LOCAL");
	}
	loop_server->receiveMessageLength = 0;
	loop_server->sendMessageLength = 0;
	loop_server->canSend = true;

	loop_client->driverdata = (void *)loop_server;
	loop_server->driverdata = (void *)loop_client;
	
	return loop_client;	
}

//==========================================================================
//
//	Loop_CheckNewConnections
//
//==========================================================================

qsocket_t *Loop_CheckNewConnections(void)
{
	if (!localconnectpending)
		return NULL;

	localconnectpending = false;
	loop_server->sendMessageLength = 0;
	loop_server->receiveMessageLength = 0;
	loop_server->canSend = true;
	loop_client->sendMessageLength = 0;
	loop_client->receiveMessageLength = 0;
	loop_client->canSend = true;
	return loop_server;
}

//==========================================================================
//
//	IntAlign
//
//==========================================================================

static int IntAlign(int value)
{
	return (value + (sizeof(int) - 1)) & (~(sizeof(int) - 1));
}

//==========================================================================
//
//	Loop_GetMessage
//
//==========================================================================

int Loop_GetMessage(qsocket_t *sock)
{
	int		ret;
	int		length;

	if (sock->receiveMessageLength == 0)
		return 0;

	ret = sock->receiveMessage[0];
	length = sock->receiveMessage[1] + (sock->receiveMessage[2] << 8);
	// alignment byte skipped here
	net_msg.Clear();
    net_msg.Write(sock->receiveMessage + 4,	length);

	length = IntAlign(length + 4);
	sock->receiveMessageLength -= length;

	if (sock->receiveMessageLength)
		memcpy(sock->receiveMessage, &sock->receiveMessage[length], sock->receiveMessageLength);

	if (sock->driverdata && ret == 1)
		((qsocket_t *)sock->driverdata)->canSend = true;

	return ret;
}

//==========================================================================
//
//	Loop_SendMessage
//
//==========================================================================

int Loop_SendMessage(qsocket_t *sock, TSizeBuf *data)
{
	byte	*buffer;
	int		*bufferLength;

	if (!sock->driverdata)
		return -1;

	bufferLength = &((qsocket_t *)sock->driverdata)->receiveMessageLength;

	if ((*bufferLength + data->CurSize + 4) > NET_MAXMESSAGE)
		Sys_Error("Loop_SendMessage: overflow\n");

	buffer = ((qsocket_t *)sock->driverdata)->receiveMessage + *bufferLength;

	// message type
	*buffer++ = 1;

	// length
    *buffer++ = data->CurSize & 0xff;
	*buffer++ = data->CurSize >> 8;

	// align
	buffer++;

	// message
    memcpy(buffer, data->Data, data->CurSize);
	*bufferLength = IntAlign(*bufferLength + data->CurSize + 4);

	sock->canSend = false;
	return 1;
}

//==========================================================================
//
//	Loop_SendUnreliableMessage
//
//==========================================================================

int Loop_SendUnreliableMessage(qsocket_t *sock, TSizeBuf *data)
{
	byte	*buffer;
	int		*bufferLength;

	if (!sock->driverdata)
		return -1;

	bufferLength = &((qsocket_t *)sock->driverdata)->receiveMessageLength;

	if ((*bufferLength + data->CurSize + sizeof(byte) + sizeof(short)) > NET_MAXMESSAGE)
		return 0;

	buffer = ((qsocket_t *)sock->driverdata)->receiveMessage + *bufferLength;

	// message type
	*buffer++ = 2;

	// length
	*buffer++ = data->CurSize & 0xff;
	*buffer++ = data->CurSize >> 8;

	// align
	buffer++;

	// message
	memcpy(buffer, data->Data, data->CurSize);
	*bufferLength = IntAlign(*bufferLength + data->CurSize + 4);
	return 1;
}

//==========================================================================
//
//	Loop_CanSendMessage
//
//==========================================================================

boolean Loop_CanSendMessage(qsocket_t *sock)
{
	if (!sock->driverdata)
		return false;
	return sock->canSend;
}

//==========================================================================
//
//	Loop_CanSendUnreliableMessage
//
//==========================================================================

boolean Loop_CanSendUnreliableMessage(qsocket_t *)
{
	return true;
}

//==========================================================================
//
//	Loop_Close
//
//==========================================================================

void Loop_Close(qsocket_t *sock)
{
	if (sock->driverdata)
		((qsocket_t *)sock->driverdata)->driverdata = NULL;
	sock->receiveMessageLength = 0;
	sock->sendMessageLength = 0;
	sock->canSend = true;
	if (sock == loop_client)
		loop_client = NULL;
	else
		loop_server = NULL;
}

//==========================================================================
//
//	Loop_Shutdown
//
//==========================================================================

void Loop_Shutdown(void)
{
}

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2001/12/18 19:05:03  dj_jl
//	Made TCvar a pure C++ class
//
//	Revision 1.4  2001/12/01 17:40:41  dj_jl
//	Added support for bots
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
