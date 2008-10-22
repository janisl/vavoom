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
//**
//**	Vavoom master server.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "net_local.h"

// MACROS ------------------------------------------------------------------

enum
{
	MCREQ_JOIN				= 1,
	MCREQ_QUIT				= 2,
	MCREQ_LIST				= 3,
};

enum
{
	MCREP_LIST				= 1,
};

// TYPES -------------------------------------------------------------------

struct TSrvItem
{
	sockaddr_t	Addr;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VNetLanDriver*		Drv;
static int					CSock;

static struct
{
	vuint8		data[MAX_MSGLEN];
} packetBuffer;

TArray<TSrvItem>			SrvList;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VNetLanDriver::VNetLanDriver
//
//==========================================================================

VNetLanDriver::VNetLanDriver()
: initialised(false)
, net_acceptsocket(-1)
{
	Drv = this;
}

//==========================================================================
//
//	main
//
//==========================================================================

void ReadNet()
{
	sockaddr_t	clientaddr;
	int acceptsock = Drv->CheckNewConnections();
	if (acceptsock == -1)
	{
		return;
	}

	int len = Drv->Read(acceptsock, packetBuffer.data, MAX_MSGLEN, &clientaddr);
	if (len < 1)
	{
		return;
	}

	printf("Received command %d from %d.%d.%d.%d:%d\n", packetBuffer.data[0],
		(vuint8)clientaddr.sa_data[2], (vuint8)clientaddr.sa_data[3], (vuint8)clientaddr.sa_data[4],
		(vuint8)clientaddr.sa_data[5], (vuint8)clientaddr.sa_data[0] * 256 + (vuint8)clientaddr.sa_data[1]);
	switch (packetBuffer.data[0])
	{
	case MCREQ_JOIN:
		{
			for (int i = 0; i < SrvList.Num(); i++)
			{
				if (Drv->AddrCompare(&SrvList[i].Addr, &clientaddr))
				{
					return;
				}
			}
			TSrvItem& I = SrvList.Alloc();
			I.Addr = clientaddr;
		}
		break;

	case MCREQ_QUIT:
		for (int i = 0; i < SrvList.Num(); i++)
		{
			if (Drv->AddrCompare(&SrvList[i].Addr, &clientaddr))
			{
				SrvList.RemoveIndex(i);
				break;
			}
		}
		break;

	case MCREQ_LIST:
		{
			packetBuffer.data[0] = MCREP_LIST;
			int Len = 1;
			for (int i = 0; i < SrvList.Num() && i < (MAX_MSGLEN - 1) / 6; i++)
			{
				memcpy(&packetBuffer.data[Len], SrvList[i].Addr.sa_data, 6);
				Len += 6;
			}
			Drv->Write(acceptsock, packetBuffer.data, Len, &clientaddr);
		}
		break;
	}
}

//==========================================================================
//
//	main
//
//==========================================================================

int main(int argc, const char** argv)
{
	CSock = Drv->Init();
	Drv->Listen(true);
	while (1)
	{
		ReadNet();
		usleep(1);
	}
	Drv->Shutdown();
	return 0;
}
