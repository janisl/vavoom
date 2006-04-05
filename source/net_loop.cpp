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

static bool			localconnectpending = false;
static qsocket_t*	loop_client = NULL;
static qsocket_t*	loop_server = NULL;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	Loop_Init
//
//==========================================================================

int Loop_Init()
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

void Loop_Listen(bool)
{
}

//==========================================================================
//
//	Loop_SearchForHosts
//
//==========================================================================

void Loop_SearchForHosts(bool)
{
	guard(Loop_SearchForHosts);
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
	unguard;
}

//==========================================================================
//
//	Loop_Connect
//
//==========================================================================

qsocket_t* Loop_Connect(const char* host)
{
	guard(Loop_Connect);
	if (strcmp(host,"local") != 0)
		return NULL;
	
	localconnectpending = true;

	if (!loop_client)
	{
		loop_client = NET_NewQSocket();
		if (loop_client == NULL)
		{
			GCon->Log(NAME_DevNet, "Loop_Connect: no qsocket available");
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
			GCon->Log(NAME_DevNet, "Loop_Connect: no qsocket available");
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
	unguard;
}

//==========================================================================
//
//	Loop_CheckNewConnections
//
//==========================================================================

qsocket_t* Loop_CheckNewConnections()
{
	guard(Loop_CheckNewConnections);
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
	unguard;
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

int Loop_GetMessage(qsocket_t* sock)
{
	guard(Loop_GetMessage);
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
	unguard;
}

//==========================================================================
//
//	Loop_SendMessage
//
//==========================================================================

int Loop_SendMessage(qsocket_t* sock, VMessage* data)
{
	guard(Loop_SendMessage);
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
	unguard;
}

//==========================================================================
//
//	Loop_SendUnreliableMessage
//
//==========================================================================

int Loop_SendUnreliableMessage(qsocket_t* sock, VMessage* data)
{
	guard(Loop_SendUnreliableMessage);
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
	unguard;
}

//==========================================================================
//
//	Loop_CanSendMessage
//
//==========================================================================

bool Loop_CanSendMessage(qsocket_t* sock)
{
	guard(Loop_CanSendMessage);
	if (!sock->driverdata)
		return false;
	return sock->canSend;
	unguard;
}

//==========================================================================
//
//	Loop_CanSendUnreliableMessage
//
//==========================================================================

bool Loop_CanSendUnreliableMessage(qsocket_t*)
{
	return true;
}

//==========================================================================
//
//	Loop_Close
//
//==========================================================================

void Loop_Close(qsocket_t* sock)
{
	guard(Loop_Close);
	if (sock->driverdata)
		((qsocket_t *)sock->driverdata)->driverdata = NULL;
	sock->receiveMessageLength = 0;
	sock->sendMessageLength = 0;
	sock->canSend = true;
	if (sock == loop_client)
		loop_client = NULL;
	else
		loop_server = NULL;
	unguard;
}

//==========================================================================
//
//	Loop_Shutdown
//
//==========================================================================

void Loop_Shutdown()
{
}

//**************************************************************************
//
//	$Log$
//	Revision 1.9  2006/04/05 17:20:37  dj_jl
//	Merged size buffer with message class.
//
//	Revision 1.8  2002/08/05 17:20:00  dj_jl
//	Added guarding.
//	
//	Revision 1.7  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.6  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
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
