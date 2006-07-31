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
#include "net_loc.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VLoopbackDriver : public VNetDriver
{
public:
	bool		localconnectpending;
	VSocket*	loop_client;
	VSocket*	loop_server;

	VLoopbackDriver();
	int Init();
	void Listen(bool);
	void SearchForHosts(bool);
	VSocket* Connect(const char*);
	VSocket* CheckNewConnections();
	int GetMessage(VSocket*);
	int SendMessage(VSocket*, VMessage*);
	int SendUnreliableMessage(VSocket*, VMessage*);
	bool CanSendMessage(VSocket*);
	bool CanSendUnreliableMessage(VSocket*);
	void Close(VSocket*);
	void Shutdown();

	static int IntAlign(int);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VLoopbackDriver	Impl;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VLoopbackDriver::VLoopbackDriver
//
//==========================================================================

VLoopbackDriver::VLoopbackDriver()
: VNetDriver(0, "Loopback")
, localconnectpending(false)
, loop_client(NULL)
, loop_server(NULL)
{
}

//==========================================================================
//
//	VLoopbackDriver::Init
//
//==========================================================================

int VLoopbackDriver::Init()
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
//	VLoopbackDriver::Listen
//
//==========================================================================

void VLoopbackDriver::Listen(bool)
{
}

//==========================================================================
//
//	VLoopbackDriver::SearchForHosts
//
//==========================================================================

void VLoopbackDriver::SearchForHosts(bool)
{
	guard(VLoopbackDriver::SearchForHosts);
#ifdef SERVER
	if (!sv.active)
		return;

	Net->HostCacheCount = 1;
	if (VStr::Cmp(Net->HostName, "UNNAMED") == 0)
		Net->HostCache[0].Name = "local";
	else
		Net->HostCache[0].Name = Net->HostName;
	Net->HostCache[0].Map = VStr(level.MapName);
	Net->HostCache[0].Users = svs.num_connected;
	Net->HostCache[0].MaxUsers = svs.max_clients;
	Net->HostCache[0].CName = "local";
#endif
	unguard;
}

//==========================================================================
//
//	VLoopbackDriver::Connect
//
//==========================================================================

VSocket* VLoopbackDriver::Connect(const char* host)
{
	guard(VLoopbackDriver::Connect);
	if (VStr::Cmp(host, "local") != 0)
		return NULL;
	
	localconnectpending = true;

	if (!loop_client)
	{
		loop_client = Net->NewSocket(this);
		if (loop_client == NULL)
		{
			GCon->Log(NAME_DevNet, "Loop_Connect: no qsocket available");
			return NULL;
		}
		loop_client->Address = "localhost";
	}
	loop_client->ReceiveMessageLength = 0;
	loop_client->SendMessageLength = 0;
	loop_client->CanSend = true;

	if (!loop_server)
	{
		loop_server = Net->NewSocket(this);
		if (loop_server == NULL)
		{
			GCon->Log(NAME_DevNet, "Loop_Connect: no qsocket available");
			return NULL;
		}
		loop_server->Address = "LOCAL";
	}
	loop_server->ReceiveMessageLength = 0;
	loop_server->SendMessageLength = 0;
	loop_server->CanSend = true;

	loop_client->DriverData = (void*)loop_server;
	loop_server->DriverData = (void*)loop_client;
	
	return loop_client;	
	unguard;
}

//==========================================================================
//
//	VLoopbackDriver::CheckNewConnections
//
//==========================================================================

VSocket* VLoopbackDriver::CheckNewConnections()
{
	guard(VLoopbackDriver::CheckNewConnections);
	if (!localconnectpending)
		return NULL;

	localconnectpending = false;
	loop_server->SendMessageLength = 0;
	loop_server->ReceiveMessageLength = 0;
	loop_server->CanSend = true;
	loop_client->SendMessageLength = 0;
	loop_client->ReceiveMessageLength = 0;
	loop_client->CanSend = true;
	return loop_server;
	unguard;
}

//==========================================================================
//
//	VLoopbackDriver::IntAlign
//
//==========================================================================

int VLoopbackDriver::IntAlign(int value)
{
	return (value + (sizeof(int) - 1)) & (~(sizeof(int) - 1));
}

//==========================================================================
//
//	VLoopbackDriver::GetMessage
//
//==========================================================================

int VLoopbackDriver::GetMessage(VSocket* sock)
{
	guard(VLoopbackDriver::GetMessage);
	int		ret;
	int		length;

	if (sock->ReceiveMessageLength == 0)
		return 0;

	ret = sock->ReceiveMessageData[0];
	length = sock->ReceiveMessageData[1] + (sock->ReceiveMessageData[2] << 8);
	// alignment byte skipped here
	Net->NetMsg.Clear();
	Net->NetMsg.Write(sock->ReceiveMessageData + 4,	length);

	length = IntAlign(length + 4);
	sock->ReceiveMessageLength -= length;

	if (sock->ReceiveMessageLength)
		memcpy(sock->ReceiveMessageData, &sock->ReceiveMessageData[length],
			sock->ReceiveMessageLength);

	if (sock->DriverData && ret == 1)
		((VSocket*)sock->DriverData)->CanSend = true;

	return ret;
	unguard;
}

//==========================================================================
//
//	VLoopbackDriver::SendMessage
//
//==========================================================================

int VLoopbackDriver::SendMessage(VSocket* sock, VMessage* data)
{
	guard(VLoopbackDriver::SendMessage);
	vuint8*		buffer;
	int*		bufferLength;

	if (!sock->DriverData)
		return -1;

	bufferLength = &((VSocket*)sock->DriverData)->ReceiveMessageLength;

	if ((*bufferLength + data->CurSize + 4) > NET_MAXMESSAGE)
		Sys_Error("Loop_SendMessage: overflow\n");

	buffer = ((VSocket*)sock->DriverData)->ReceiveMessageData + *bufferLength;

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

	sock->CanSend = false;
	return 1;
	unguard;
}

//==========================================================================
//
//	VLoopbackDriver::SendUnreliableMessage
//
//==========================================================================

int VLoopbackDriver::SendUnreliableMessage(VSocket* sock, VMessage* data)
{
	guard(VLoopbackDriver::SendUnreliableMessage);
	vuint8*		buffer;
	int*		bufferLength;

	if (!sock->DriverData)
		return -1;

	bufferLength = &((VSocket*)sock->DriverData)->ReceiveMessageLength;

	if ((*bufferLength + data->CurSize + sizeof(byte) + sizeof(short)) > NET_MAXMESSAGE)
		return 0;

	buffer = ((VSocket*)sock->DriverData)->ReceiveMessageData + *bufferLength;

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
//	VLoopbackDriver::CanSendMessage
//
//==========================================================================

bool VLoopbackDriver::CanSendMessage(VSocket* sock)
{
	guard(VLoopbackDriver::CanSendMessage);
	if (!sock->DriverData)
		return false;
	return sock->CanSend;
	unguard;
}

//==========================================================================
//
//	VLoopbackDriver::CanSendUnreliableMessage
//
//==========================================================================

bool VLoopbackDriver::CanSendUnreliableMessage(VSocket*)
{
	return true;
}

//==========================================================================
//
//	VLoopbackDriver::Close
//
//==========================================================================

void VLoopbackDriver::Close(VSocket* sock)
{
	guard(VLoopbackDriver::Close);
	if (sock->DriverData)
		((VSocket*)sock->DriverData)->DriverData = NULL;
	sock->ReceiveMessageLength = 0;
	sock->SendMessageLength = 0;
	sock->CanSend = true;
	if (sock == loop_client)
		loop_client = NULL;
	else
		loop_server = NULL;
	unguard;
}

//==========================================================================
//
//	VLoopbackDriver::Shutdown
//
//==========================================================================

void VLoopbackDriver::Shutdown()
{
}
