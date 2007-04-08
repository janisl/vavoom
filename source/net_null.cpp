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

class VNullSocket : public VSocket
{
public:
	VNullSocket(VNetDriver* Drv)
	: VSocket(Drv)
	{}

	int GetMessage(TArray<vuint8>&);
	int SendMessage(vuint8*, vuint32);
	bool IsLocalConnection();
};

class VNullNetDriver : public VNetDriver
{
public:
	VNullNetDriver();
	int Init();
	void Listen(bool);
	void SearchForHosts(bool);
	VSocket* Connect(const char*);
	VSocket* CheckNewConnections();
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

VSocket* VNullNetDriver::Connect(const char*)
{
	return NULL;
}

//==========================================================================
//
//	VNullNetDriver::CheckNewConnections
//
//==========================================================================

VSocket* VNullNetDriver::CheckNewConnections()
{
	guard(VNullNetDriver::CheckNewConnections);
	if (!Net->ConnectBot)
		return NULL;

	Net->ConnectBot = false;
	VSocket* sock = new VNullSocket(this);
	sock->Address = "NULL";
	return sock;
	unguard;
}

//==========================================================================
//
//	VNullNetDriver::Shutdown
//
//==========================================================================

void VNullNetDriver::Shutdown()
{
}

//==========================================================================
//
//	VNullSocket::GetMessage
//
//==========================================================================

int VNullSocket::GetMessage(TArray<vuint8>&)
{
	return 0;
}

//==========================================================================
//
//	VNullSocket::SendMessage
//
//==========================================================================

int VNullSocket::SendMessage(vuint8*, vuint32)
{
	return 1;
}

//==========================================================================
//
//	VNullSocket::IsLocalConnection
//
//==========================================================================

bool VNullSocket::IsLocalConnection()
{
	return true;
}
