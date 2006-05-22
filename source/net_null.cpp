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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VNullNetDriver : public VNetDriver
{
public:
	VNullNetDriver();
	int Init();
	void Listen(bool);
	void SearchForHosts(bool);
	qsocket_t* Connect(const char*);
	qsocket_t* CheckNewConnections();
	int GetMessage(qsocket_t*);
	int SendMessage(qsocket_t*, VMessage*);
	int SendUnreliableMessage(qsocket_t*, VMessage*);
	bool CanSendMessage(qsocket_t*);
	bool CanSendUnreliableMessage(qsocket_t*);
	void Close(qsocket_t*);
	void Shutdown();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VNullNetDriver	Impl;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VNullNetDriver::VNullNetDriver
//
//==========================================================================

VNullNetDriver::VNullNetDriver()
: VNetDriver(1, "Null")
{
}

//==========================================================================
//
//	VNullNetDriver::Init
//
//==========================================================================

int VNullNetDriver::Init()
{
	return 0;
}

//==========================================================================
//
//	VNullNetDriver::Listen
//
//==========================================================================

void VNullNetDriver::Listen(bool)
{
}

//==========================================================================
//
//	VNullNetDriver::SearchForHosts
//
//==========================================================================

void VNullNetDriver::SearchForHosts(bool)
{
}

//==========================================================================
//
//	VNullNetDriver::Connect
//
//==========================================================================

qsocket_t* VNullNetDriver::Connect(const char*)
{
	return NULL;
}

//==========================================================================
//
//	VNullNetDriver::CheckNewConnections
//
//==========================================================================

qsocket_t* VNullNetDriver::CheckNewConnections()
{
	guard(VNullNetDriver::CheckNewConnections);
	if (!net_connect_bot)
		return NULL;

	net_connect_bot = false;
	qsocket_t* sock = NET_NewQSocket(this);
	if (!sock)
	{
		GCon->Log("Server is full");
		return NULL;
	}
	strcpy(sock->address, "NULL");
	return sock;
	unguard;
}

//==========================================================================
//
//	VNullNetDriver::GetMessage
//
//==========================================================================

int VNullNetDriver::GetMessage(qsocket_t*)
{
	return 0;
}

//==========================================================================
//
//	VNullNetDriver::SendMessage
//
//==========================================================================

int VNullNetDriver::SendMessage(qsocket_t*, VMessage*)
{
	return 1;
}

//==========================================================================
//
//	VNullNetDriver::SendUnreliableMessage
//
//==========================================================================

int VNullNetDriver::SendUnreliableMessage(qsocket_t*, VMessage*)
{
	return 1;
}

//==========================================================================
//
//	VNullNetDriver::CanSendMessage
//
//==========================================================================

bool VNullNetDriver::CanSendMessage(qsocket_t*)
{
	return true;
}

//==========================================================================
//
//	VNullNetDriver::CanSendUnreliableMessage
//
//==========================================================================

bool VNullNetDriver::CanSendUnreliableMessage(qsocket_t*)
{
	return true;
}

//==========================================================================
//
//	VNullNetDriver::Close
//
//==========================================================================

void VNullNetDriver::Close(qsocket_t*)
{
}

//==========================================================================
//
//	VNullNetDriver::Shutdown
//
//==========================================================================

void VNullNetDriver::Shutdown()
{
}
