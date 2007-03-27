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

class VNullNetDriver : public VNetDriver
{
public:
	VNullNetDriver();
	int Init();
	void Listen(bool);
	void SearchForHosts(bool);
	VSocket* Connect(const char*);
	VSocket* CheckNewConnections();
	int GetMessage(VSocket*, TArray<vuint8>&);
	int SendMessage(VSocket*, vuint8*, vuint32);
	void Close(VSocket*);
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
	VSocket* sock = Net->NewSocket(this);
	if (!sock)
	{
		GCon->Log("Server is full");
		return NULL;
	}
	sock->Address = "NULL";
	return sock;
	unguard;
}

//==========================================================================
//
//	VNullNetDriver::GetMessage
//
//==========================================================================

int VNullNetDriver::GetMessage(VSocket*, TArray<vuint8>&)
{
	return 0;
}

//==========================================================================
//
//	VNullNetDriver::SendMessage
//
//==========================================================================

int VNullNetDriver::SendMessage(VSocket*, vuint8*, vuint32)
{
	return 1;
}

//==========================================================================
//
//	VNullNetDriver::Close
//
//==========================================================================

void VNullNetDriver::Close(VSocket*)
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
