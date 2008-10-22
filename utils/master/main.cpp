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

#define check(a)

#include "cmdlib.h"
using namespace VavoomUtils;
#ifdef _WIN32
#include <windows.h>
#include <errno.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#endif
#include "array.h"

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

enum
{
	MAX_MSGLEN			= 1024,		// max length of a message
	NET_DATAGRAMSIZE	= MAX_MSGLEN,
	HostPort			= 26001,
};

struct sockaddr_t
{
	vint16		sa_family;
	vint8		sa_data[14];
};

class VNetLanDriver
{
public:
	bool			initialised;
	int				net_acceptsocket;		// socket for fielding new connections
#ifdef _WIN32
	int				winsock_initialised;
	WSADATA			winsockdata;
#endif

	VNetLanDriver();
	int Init();
	void Shutdown();
	int OpenSocket(int);
	int CloseSocket(int);
	void Listen(bool);
	int CheckNewConnections();
	int Read(int, vuint8*, int, sockaddr_t*);
	int Write(int, vuint8*, int, sockaddr_t*);
	int AddrCompare(sockaddr_t*, sockaddr_t*);
};

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
static VNetLanDriver		Impl;
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
#ifdef _WIN32
, winsock_initialised(0)
#endif
{
	Drv = this;
}

//==========================================================================
//
//  VNetLanDriver::Init
//
//==========================================================================

int VNetLanDriver::Init()
{
#ifdef _WIN32
	if (winsock_initialised == 0)
	{
		//MAKEWORD(2, 2)
		int r = WSAStartup(MAKEWORD(1, 1), &winsockdata);
		if (r)
		{
			printf("Winsock initialisation failed.\n");
			return -1;
		}
	}
	winsock_initialised++;
#endif
	return 0;
}

//==========================================================================
//
//  VNetLanDriver::Shutdown
//
//==========================================================================

void VNetLanDriver::Shutdown()
{
	Listen(false);
#ifdef _WIN32
	if (--winsock_initialised == 0)
	{
		WSACleanup();
	}
#endif
}

//==========================================================================
//
//  VNetLanDriver::OpenSocket
//
//==========================================================================

int VNetLanDriver::OpenSocket(int port)
{
	int			newsocket;
	sockaddr_in	address;
	int			trueval = true;

	newsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (newsocket == -1)
	{
		return -1;
	}

#ifdef _WIN32
	if (ioctlsocket(newsocket, FIONBIO, &trueval) == -1)
#else
	if (ioctl(newsocket, FIONBIO, (char*)&trueval) == -1)
#endif
	{
		close(newsocket);
		return -1;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(newsocket, (sockaddr*)&address, sizeof(address)) == -1)
	{
		close(newsocket);
		return -1;
	}

	return newsocket;
}

//==========================================================================
//
//  VNetLanDriver::CloseSocket
//
//==========================================================================

int VNetLanDriver::CloseSocket(int socket)
{
#ifdef _WIN32
	return closesocket(socket);
#else
	return close(socket);
#endif
}

//==========================================================================
//
//  VNetLanDriver::Listen
//
//==========================================================================

void VNetLanDriver::Listen(bool state)
{
	if (state)
	{
		// enable listening
		if (net_acceptsocket == -1)
		{
			net_acceptsocket = OpenSocket(HostPort);
			if (net_acceptsocket == -1)
				Error("UDP_Listen: Unable to open accept socket\n");
		}
	}
	else
	{
		// disable listening
		if (net_acceptsocket != -1)
		{
			CloseSocket(net_acceptsocket);
			net_acceptsocket = -1;
		}
	}
}

//==========================================================================
//
//  VNetLanDriver::CheckNewConnections
//
//==========================================================================

int VNetLanDriver::CheckNewConnections()
{
	char	buf[4096];

	if (net_acceptsocket == -1)
		return -1;

	if (recvfrom(net_acceptsocket, buf, sizeof(buf), MSG_PEEK, NULL, NULL) >= 0)
	{
		return net_acceptsocket;
	}
	return -1;
}

//==========================================================================
//
//  VNetLanDriver::Read
//
//==========================================================================

int VNetLanDriver::Read(int socket, vuint8* buf, int len, sockaddr_t* addr)
{
	socklen_t	addrlen = sizeof(sockaddr_t);
	int		ret;

	ret = recvfrom(socket, buf, len, 0, (sockaddr*)addr, &addrlen);
	if (ret == -1)
	{
#ifdef _WIN32
		int e = WSAGetLastError();
		if (e == WSAEWOULDBLOCK || e == WSAECONNREFUSED)
#else
		if (errno == EWOULDBLOCK || errno == ECONNREFUSED)
#endif
		{
			return 0;
		}
	}
	return ret;
}

//==========================================================================
//
//  VNetLanDriver::Write
//
//==========================================================================

int VNetLanDriver::Write(int socket, vuint8* buf, int len, sockaddr_t* addr)
{
	int ret = sendto(socket, buf, len, 0, (sockaddr *)addr, sizeof(sockaddr));
	if (ret == -1)
	{
#ifdef _WIN32
		if (WSAGetLastError() == WSAEWOULDBLOCK)
#else
		if (errno == EWOULDBLOCK)
#endif
		{
			return 0;
		}
	}
	return ret;
}

//==========================================================================
//
//  VNetLanDriver::AddrCompare
//
//==========================================================================

int VNetLanDriver::AddrCompare(sockaddr_t* addr1, sockaddr_t* addr2)
{
	if (addr1->sa_family != addr2->sa_family)
		return -1;

	if (((sockaddr_in *)addr1)->sin_addr.s_addr != ((sockaddr_in *)addr2)->sin_addr.s_addr)
		return -1;

	if (((sockaddr_in *)addr1)->sin_port != ((sockaddr_in *)addr2)->sin_port)
		return 1;

	return 0;
}
//==========================================================================
//
//	ReadNet
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
