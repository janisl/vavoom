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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  VNetLanDriver::Init
//
//==========================================================================

int VNetLanDriver::Init()
{
	guard(VNetLanDriver::Init);
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
//  VNetLanDriver::Shutdown
//
//==========================================================================

void VNetLanDriver::Shutdown()
{
	guard(VNetLanDriver::Shutdown);
	Listen(false);
	if (--winsock_initialised == 0)
	{
		WSACleanup();
	}
	unguard;
}

//==========================================================================
//
//  VNetLanDriver::Listen
//
//==========================================================================

void VNetLanDriver::Listen(bool state)
{
	guard(VNetLanDriver::Listen);
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
//  VNetLanDriver::OpenSocket
//
//==========================================================================

int VNetLanDriver::OpenSocket(int port)
{
	guard(VNetLanDriver::OpenSocket);
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
//  VNetLanDriver::CloseSocket
//
//==========================================================================

int VNetLanDriver::CloseSocket(int socket)
{
	guard(VNetLanDriver::CloseSocket);
	return closesocket(socket);
	unguard;
}

//==========================================================================
//
//  VNetLanDriver::CheckNewConnections
//
//==========================================================================

int VNetLanDriver::CheckNewConnections()
{
	guard(VNetLanDriver::CheckNewConnections);
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
//  VNetLanDriver::Read
//
//==========================================================================

int VNetLanDriver::Read(int socket, vuint8* buf, int len, sockaddr_t* addr)
{
	guard(VNetLanDriver::Read);
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
//  VNetLanDriver::Write
//
//==========================================================================

int VNetLanDriver::Write(int socket, vuint8* buf, int len, sockaddr_t* addr)
{
	guard(VNetLanDriver::Write);
	int ret = sendto(socket, (char*)buf, len, 0, (sockaddr*)addr, sizeof(sockaddr_t));
	if (ret == -1)
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return 0;

	return ret;
	unguard;
}

//==========================================================================
//
//  VNetLanDriver::AddrCompare
//
//==========================================================================

int VNetLanDriver::AddrCompare(sockaddr_t* addr1, sockaddr_t* addr2)
{
	guard(VNetLanDriver::AddrCompare);
	if (addr1->sa_family != addr2->sa_family)
		return -1;

	if (((sockaddr_in *)addr1)->sin_addr.s_addr != ((sockaddr_in *)addr2)->sin_addr.s_addr)
		return -1;

	if (((sockaddr_in *)addr1)->sin_port != ((sockaddr_in *)addr2)->sin_port)
		return 1;

	return 0;
	unguard;
}
