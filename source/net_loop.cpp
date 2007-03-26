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
	int GetMessage(VSocket*, TArray<vuint8>&);
	int SendMessage(VSocket*, vuint8*, vuint32);
	bool CanSendMessage(VSocket*);
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
	loop_server->CanSend = true;
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

int VLoopbackDriver::GetMessage(VSocket* Sock, TArray<vuint8>& Data)
{
	guard(VLoopbackDriver::GetMessage);
	if (!Sock->LoopbackMessages.Num())
		return 0;

	Data = Sock->LoopbackMessages[0].Data;
	Sock->LoopbackMessages.RemoveIndex(0);

	if (Sock->DriverData)
		((VSocket*)Sock->DriverData)->CanSend = true;

	return 1;
	unguard;
}

//==========================================================================
//
//	VLoopbackDriver::SendMessage
//
//==========================================================================

int VLoopbackDriver::SendMessage(VSocket* Sock, vuint8* Data, vuint32 Length)
{
	guard(VLoopbackDriver::SendMessage);
	if (!Sock->DriverData)
		return -1;

	VLoopbackMessage& Msg = ((VSocket*)Sock->DriverData)->LoopbackMessages.Alloc();
	Msg.Data.SetNum(Length);
	memcpy(Msg.Data.Ptr(), Data, Length);

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
//	VLoopbackDriver::Close
//
//==========================================================================

void VLoopbackDriver::Close(VSocket* sock)
{
	guard(VLoopbackDriver::Close);
	if (sock->DriverData)
		((VSocket*)sock->DriverData)->DriverData = NULL;
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
