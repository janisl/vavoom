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

#include "net_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VUdpDriver : public VNetLanDriver
{
public:
	int				winsock_initialised;
	WSADATA			winsockdata;

	VUdpDriver();
	int Init();
	void Shutdown();
	void Listen(bool);
	int OpenSocket(int);
	int CloseSocket(int);
	int CheckNewConnections();
	int Read(int, vuint8*, int, sockaddr_t*);
	int Write(int, vuint8*, int, sockaddr_t*);
	int AddrCompare(sockaddr_t*, sockaddr_t*);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VUdpDriver	Impl;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VUdpDriver::VUdpDriver
//
//==========================================================================

VUdpDriver::VUdpDriver()
: winsock_initialised(0)
{
}

//==========================================================================
//
//  VUdpDriver::Init
//
//==========================================================================

int VUdpDriver::Init()
{
	guard(VUdpDriver::Init);
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
	return 0;
	unguard;
}

//==========================================================================
//
//  VUdpDriver::Shutdown
//
//==========================================================================

void VUdpDriver::Shutdown()
{
	guard(VUdpDriver::Shutdown);
	Listen(false);
	if (--winsock_initialised == 0)
	{
		WSACleanup();
	}
	unguard;
}

//==========================================================================
//
//  VUdpDriver::Listen
//
//==========================================================================

void VUdpDriver::Listen(bool state)
{
	guard(VUdpDriver::Listen);
	if (state)
	{
		// enable listening
		if (net_acceptsocket == -1)
		{
			GetLocalAddress();
			net_acceptsocket = OpenSocket(HostPort);
			if (net_acceptsocket == -1)
				Error("WINS_Listen: Unable to open accept socket\n");
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
	unguard;
}


//==========================================================================
//
//  VUdpDriver::OpenSocket
//
//==========================================================================

int VUdpDriver::OpenSocket(int port)
{
	guard(VUdpDriver::OpenSocket);
	int				newsocket;
	sockaddr_in		address;
	DWORD			trueval = 1;

	newsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (newsocket == -1)
		return -1;

	if (ioctlsocket(newsocket, FIONBIO, &trueval) == -1)
		goto ErrorReturn;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = myAddr;
	address.sin_port = htons((word)port);
	if (bind(newsocket, (sockaddr *)&address, sizeof(address)) == 0)
		return newsocket;

	Error("Unable to bind to %s", AddrToString((sockaddr_t *)&address));

ErrorReturn:
	closesocket(newsocket);
	return -1;
	unguard;
}

//==========================================================================
//
//  VUdpDriver::CloseSocket
//
//==========================================================================

int VUdpDriver::CloseSocket(int socket)
{
	guard(VUdpDriver::CloseSocket);
	return closesocket(socket);
	unguard;
}

//==========================================================================
//
//  VUdpDriver::CheckNewConnections
//
//==========================================================================

int VUdpDriver::CheckNewConnections()
{
	guard(VUdpDriver::CheckNewConnections);
	char	buf[4096];

	if (net_acceptsocket == -1)
		return -1;

	if (recvfrom(net_acceptsocket, buf, sizeof(buf), MSG_PEEK, NULL, NULL) >= 0)
	{
		return net_acceptsocket;
	}
	return -1;
	unguard;
}

//==========================================================================
//
//  VUdpDriver::Read
//
//==========================================================================

int VUdpDriver::Read(int socket, vuint8* buf, int len, sockaddr_t* addr)
{
	guard(VUdpDriver::Read);
	int addrlen = sizeof(sockaddr_t);
	int ret;

	ret = recvfrom(socket, (char*)buf, len, 0, (sockaddr*)addr, &addrlen);
	if (ret == -1)
	{
		int e = WSAGetLastError();

		if (e == WSAEWOULDBLOCK || e == WSAECONNREFUSED)
			return 0;
	}
	return ret;
	unguard;
}

//==========================================================================
//
//  VUdpDriver::Write
//
//==========================================================================

int VUdpDriver::Write(int socket, vuint8* buf, int len, sockaddr_t* addr)
{
	guard(VUdpDriver::Write);
	int ret = sendto(socket, (char*)buf, len, 0, (sockaddr*)addr, sizeof(sockaddr_t));
	if (ret == -1)
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return 0;

	return ret;
	unguard;
}

//==========================================================================
//
//  VUdpDriver::AddrCompare
//
//==========================================================================

int VUdpDriver::AddrCompare(sockaddr_t* addr1, sockaddr_t* addr2)
{
	guard(VUdpDriver::AddrCompare);
	if (addr1->sa_family != addr2->sa_family)
		return -1;

	if (((sockaddr_in *)addr1)->sin_addr.s_addr != ((sockaddr_in *)addr2)->sin_addr.s_addr)
		return -1;

	if (((sockaddr_in *)addr1)->sin_port != ((sockaddr_in *)addr2)->sin_port)
		return 1;

	return 0;
	unguard;
}
