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

#include "winlocal.h"
#include <wsipx.h>
#include "gamedefs.h"
#include "net_loc.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VWinIpxDriver : public VNetLanDriver
{
public:
	enum { MAXHOSTNAMELEN = 256 };
	enum { IPXSOCKETS = 18 };

	int			net_acceptsocket;
	int			net_controlsocket;
	sockaddr_t	broadcastaddr;

	int			ipxsocket[IPXSOCKETS];
	int			sequence[IPXSOCKETS];

	char		packetBuffer[NET_DATAGRAMSIZE + 4];

	VWinIpxDriver();
	int Init();
	void Shutdown();
	void Listen(bool);
	int OpenSocket(int);
	int CloseSocket(int);
	int Connect(int, sockaddr_t*);
	int CheckNewConnections();
	int Read(int, vuint8*, int, sockaddr_t*);
	int Write(int, vuint8*, int, sockaddr_t*);
	int Broadcast(int, vuint8*, int);
	char* AddrToString(sockaddr_t*);
	int StringToAddr(const char*, sockaddr_t*);
	int GetSocketAddr(int, sockaddr_t*);
	int GetNameFromAddr(sockaddr_t*, char*);
	int GetAddrFromName(const char*, sockaddr_t*);
	int AddrCompare(sockaddr_t*, sockaddr_t*);
	int GetSocketPort(sockaddr_t*);
	int SetSocketPort(sockaddr_t*, int);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern int			winsock_initialised;
extern WSADATA		winsockdata;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VWinIpxDriver	Impl;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VWinIpxDriver::VWinIpxDriver
//
//==========================================================================

VWinIpxDriver::VWinIpxDriver()
: VNetLanDriver(1, "Winsock IPX")
, net_acceptsocket(-1)
, net_controlsocket(0)
{
	memset(&broadcastaddr, 0, sizeof(broadcastaddr));
	memset(ipxsocket, 0, sizeof(ipxsocket));
	memset(sequence, 0, sizeof(sequence));
}

//==========================================================================
//
//  VWinIpxDriver::Init
//
//==========================================================================

int VWinIpxDriver::Init()
{
	guard(VWinIpxDriver::Init);
	int			i;
	char		buff[MAXHOSTNAMELEN];
	sockaddr_t	addr;
	char		*p;
	int			r;

	if (GArgs.CheckParm("-noipx"))
		return -1;

	if (winsock_initialised == 0)
	{
		r = WSAStartup(MAKEWORD(1, 1), &winsockdata);

		if (r)
		{
			GCon->Log(NAME_Init, "Winsock initialisation failed.");
			return -1;
		}
	}
	winsock_initialised++;

	for (i = 0; i < IPXSOCKETS; i++)
		ipxsocket[i] = 0;

	// determine my name & address
	if (gethostname(buff, MAXHOSTNAMELEN) == 0)
	{
		// if the Vavoom hostname isn't set, set it to the machine name
		if (strcmp(hostname, "UNNAMED") == 0)
		{
			// see if it's a text IP address (well, close enough)
			for (p = buff; *p; p++)
				if ((*p < '0' || *p > '9') && *p != '.')
					break;

			// if it is a real name, strip off the domain; we only want the host
			if (*p)
			{
				for (i = 0; i < 15; i++)
					if (buff[i] == '.')
						break;
				buff[i] = 0;
			}
			hostname = buff;
		}
	}

	net_controlsocket = OpenSocket(0);
	if (net_controlsocket == -1)
	{
		GCon->Log(NAME_Init, "WIPX_Init: Unable to open control socket");
		if (--winsock_initialised == 0)
			WSACleanup();
		return -1;
	}

	((sockaddr_ipx *)&broadcastaddr)->sa_family = AF_IPX;
	memset(((sockaddr_ipx *)&broadcastaddr)->sa_netnum, 0, 4);
	memset(((sockaddr_ipx *)&broadcastaddr)->sa_nodenum, 0xff, 6);
	((sockaddr_ipx *)&broadcastaddr)->sa_socket = htons((word)net_hostport);

	GetSocketAddr(net_controlsocket, &addr);
	strcpy(my_ipx_address, AddrToString(&addr));
	p = strrchr(my_ipx_address, ':');
	if (p)
		*p = 0;

	GCon->Log(NAME_Init, "Winsock IPX Initialised");
	ipxAvailable = true;

	return net_controlsocket;
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::Shutdown
//
//==========================================================================

void VWinIpxDriver::Shutdown()
{
	guard(WIPX_Shutdown);
	Listen(false);
	CloseSocket(net_controlsocket);
	if (--winsock_initialised == 0)
		WSACleanup();
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::Listen
//
//==========================================================================

void VWinIpxDriver::Listen(bool state)
{
	guard(VWinIpxDriver::Listen);
	// enable listening
	if (state)
	{
		if (net_acceptsocket == -1)
		{
			net_acceptsocket = OpenSocket(net_hostport);
			if (net_acceptsocket == -1)
				Sys_Error("WIPX_Listen: Unable to open accept socket\n");
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
//  VWinIpxDriver::OpenSocket
//
//==========================================================================

int VWinIpxDriver::OpenSocket(int port)
{
	guard(VWinIpxDriver::OpenSocket);
	int				handle;
	int				newsocket;
	sockaddr_ipx	address;
	dword			trueval = 1;

	for (handle = 0; handle < IPXSOCKETS; handle++)
		if (ipxsocket[handle] == 0)
			break;
	if (handle == IPXSOCKETS)
		return -1;

    newsocket = socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
	if (newsocket == INVALID_SOCKET)
		return -1;

	if (ioctlsocket(newsocket, FIONBIO, &trueval) == -1)
		goto ErrorReturn;

	if (setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&trueval, sizeof(trueval)) < 0)
		goto ErrorReturn;

	address.sa_family = AF_IPX;
	memset(address.sa_netnum, 0, 4);
	memset(address.sa_nodenum, 0, 6);;
	address.sa_socket = htons((word)port);
	if (bind(newsocket, (sockaddr *)&address, sizeof(address)) == 0)
	{
		ipxsocket[handle] = newsocket;
		sequence[handle] = 0;
		return handle;
	}

	Sys_Error("Winsock IPX bind failed\n");
ErrorReturn:
	closesocket(newsocket);
	return -1;
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::CloseSocket
//
//==========================================================================

int VWinIpxDriver::CloseSocket(int handle)
{
	guard(VWinIpxDriver::CloseSocket);
	int socket = ipxsocket[handle];
	int ret;

	ret =  closesocket(socket);
	ipxsocket[handle] = 0;
	return ret;
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::Connect
//
//==========================================================================

int VWinIpxDriver::Connect(int, sockaddr_t*)
{
	return 0;
}

//==========================================================================
//
//  VWinIpxDriver::CheckNewConnections
//
//==========================================================================

int VWinIpxDriver::CheckNewConnections()
{
	guard(VWinIpxDriver::CheckNewConnections);
	dword		available;

	if (net_acceptsocket == -1)
		return -1;

	if (ioctlsocket(ipxsocket[net_acceptsocket], FIONREAD, &available) == -1)
		Sys_Error("WIPX: ioctlsocket (FIONREAD) failed\n");
	if (available)
		return net_acceptsocket;
	return -1;
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::Read
//
//==========================================================================

int VWinIpxDriver::Read(int handle, vuint8* buf, int len, sockaddr_t* addr)
{
	guard(VWinIpxDriver::Read);
	int 	addrlen = sizeof(sockaddr_t);
	int 	socket = ipxsocket[handle];
	int 	ret;

	ret = recvfrom(socket, packetBuffer, len + 4, 0, (struct sockaddr *)addr, &addrlen);
	if (ret == -1)
	{
		int e = WSAGetLastError();

		if (e == WSAEWOULDBLOCK || e == WSAECONNREFUSED)
			return 0;
	}

	if (ret < 4)
		return 0;
	
	// remove sequence number, it's only needed for DOS IPX
	ret -= 4;
	memcpy(buf, packetBuffer + 4, ret);

	return ret;
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::Write
//
//==========================================================================

int VWinIpxDriver::Write(int handle, vuint8* buf, int len, sockaddr_t* addr)
{
	guard(VWinIpxDriver::Write);
	int 	socket = ipxsocket[handle];
	int 	ret;

	// build packet with sequence number
	*(int *)(&packetBuffer[0]) = sequence[handle];
	sequence[handle]++;
	memcpy(&packetBuffer[4], buf, len);
	len += 4;

	ret = sendto(socket, packetBuffer, len, 0, (sockaddr*)addr, sizeof(sockaddr_t));
	if (ret == -1)
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return 0;

	return ret;
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::Broadcast
//
//==========================================================================

int VWinIpxDriver::Broadcast(int handle, vuint8* buf, int len)
{
	guard(WIPX_Broadcast);
	return Write(handle, buf, len, &broadcastaddr);
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::AddrToString
//
//==========================================================================

char* VWinIpxDriver::AddrToString(sockaddr_t* addr)
{
	guard(VWinIpxDriver::AddrToString);
	static char buf[28];

	sprintf(buf, "%02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%u",
		((sockaddr_ipx *)addr)->sa_netnum[0] & 0xff,
		((sockaddr_ipx *)addr)->sa_netnum[1] & 0xff,
		((sockaddr_ipx *)addr)->sa_netnum[2] & 0xff,
		((sockaddr_ipx *)addr)->sa_netnum[3] & 0xff,
		((sockaddr_ipx *)addr)->sa_nodenum[0] & 0xff,
		((sockaddr_ipx *)addr)->sa_nodenum[1] & 0xff,
		((sockaddr_ipx *)addr)->sa_nodenum[2] & 0xff,
		((sockaddr_ipx *)addr)->sa_nodenum[3] & 0xff,
		((sockaddr_ipx *)addr)->sa_nodenum[4] & 0xff,
		((sockaddr_ipx *)addr)->sa_nodenum[5] & 0xff,
		ntohs(((sockaddr_ipx *)addr)->sa_socket)
		);
	return buf;
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::StringToAddr
//
//==========================================================================

int VWinIpxDriver::StringToAddr(const char* string, sockaddr_t* addr)
{
	guard(VWinIpxDriver::StringToAddr);
	int  val;
	char buf[3];

	buf[2] = 0;
	memset(addr, 0, sizeof(sockaddr_t));
	addr->sa_family = AF_IPX;

#define DO(src,dest)	\
	buf[0] = string[src];	\
	buf[1] = string[src + 1];	\
	if (sscanf (buf, "%x", &val) != 1)	\
		return -1;	\
	((struct sockaddr_ipx *)addr)->dest = val

	DO(0, sa_netnum[0]);
	DO(2, sa_netnum[1]);
	DO(4, sa_netnum[2]);
	DO(6, sa_netnum[3]);
	DO(9, sa_nodenum[0]);
	DO(11, sa_nodenum[1]);
	DO(13, sa_nodenum[2]);
	DO(15, sa_nodenum[3]);
	DO(17, sa_nodenum[4]);
	DO(19, sa_nodenum[5]);
#undef DO

	sscanf (&string[22], "%u", &val);
	((sockaddr_ipx *)addr)->sa_socket = htons((word)val);

	return 0;
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::GetSocketAddr
//
//==========================================================================

int VWinIpxDriver::GetSocketAddr(int handle, sockaddr_t* addr)
{
	guard(VWinIpxDriver::GetSocketAddr);
	int 	socket = ipxsocket[handle];
	int 	addrlen = sizeof(sockaddr_t);

	memset(addr, 0, sizeof(sockaddr_t));
	if (getsockname(socket, (sockaddr *)addr, &addrlen) != 0)
	{
		WSAGetLastError();
	}

	return 0;
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::GetNameFromAddr
//
//==========================================================================

int VWinIpxDriver::GetNameFromAddr(sockaddr_t* addr, char* name)
{
	guard(VWinIpxDriver::GetNameFromAddr);
	strcpy(name, AddrToString(addr));
	return 0;
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::GetAddrFromName
//
//==========================================================================

int VWinIpxDriver::GetAddrFromName(const char* name, sockaddr_t* addr)
{
	guard(VWinIpxDriver::GetAddrFromName);
	int		n;
	char	buf[32];

	n = strlen(name);

	if (n == 12)
	{
		sprintf(buf, "00000000:%s:%u", name, net_hostport);
		return StringToAddr(buf, addr);
	}
	if (n == 21)
	{
		sprintf(buf, "%s:%u", name, net_hostport);
		return StringToAddr(buf, addr);
	}
	if (n > 21 && n <= 27)
		return StringToAddr(name, addr);

	return -1;
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::AddrCompare
//
//==========================================================================

int VWinIpxDriver::AddrCompare(sockaddr_t* addr1, sockaddr_t* addr2)
{
	guard(VWinIpxDriver::AddrCompare);
	if (addr1->sa_family != addr2->sa_family)
		return -1;

	if (*((sockaddr_ipx*)addr1)->sa_netnum && *((sockaddr_ipx*)addr2)->sa_netnum)
		if (memcmp(((sockaddr_ipx*)addr1)->sa_netnum, ((sockaddr_ipx*)addr2)->sa_netnum, 4) != 0)
			return -1;
	if (memcmp(((sockaddr_ipx*)addr1)->sa_nodenum, ((sockaddr_ipx*)addr2)->sa_nodenum, 6) != 0)
		return -1;

	if (((sockaddr_ipx*)addr1)->sa_socket != ((sockaddr_ipx*)addr2)->sa_socket)
		return 1;

	return 0;
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::GetSocketPort
//
//==========================================================================

int VWinIpxDriver::GetSocketPort(sockaddr_t* addr)
{
	guard(VWinIpxDriver::GetSocketPort);
	return ntohs(((sockaddr_ipx*)addr)->sa_socket);
	unguard;
}

//==========================================================================
//
//  VWinIpxDriver::SetSocketPort
//
//==========================================================================

int VWinIpxDriver::SetSocketPort(sockaddr_t* addr, int port)
{
	guard(VWinIpxDriver::SetSocketPort);
	((sockaddr_ipx*)addr)->sa_socket = htons((word)port);
	return 0;
	unguard;
}
