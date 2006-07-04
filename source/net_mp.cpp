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

#include <dpmi.h>
#include "mpdosock.h"
#include "gamedefs.h"
#include "net_loc.h"

// MACROS ------------------------------------------------------------------

#define MAXHOSTNAMELEN		256

// TYPES -------------------------------------------------------------------

class VMPathDriver : public VNetLanDriver
{
public:
	int			net_acceptsocket;		// socket for fielding new connections
	int			net_controlsocket;
	int			net_broadcastsocket;
	sockaddr_t	broadcastaddr;

	vuint32		myAddr;

	VMPathDriver();
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
	VStr GetNameFromAddr(sockaddr_t*);
	int GetAddrFromName(const char*, sockaddr_t*);
	int AddrCompare(sockaddr_t*, sockaddr_t*);
	int GetSocketPort(sockaddr_t*);
	int SetSocketPort(sockaddr_t*, int);

	int PartialIPAddress(const char*, sockaddr_t*);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

extern "C" {
int WSAGetLastError();
void sockets_flush();
}

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

short				flat_selector;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VMPathDriver	Impl;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VMPathDriver::VMPathDriver
//
//==========================================================================

VMPathDriver::VMPathDriver()
: VNetLanDriver(2, "Win95 TCP/IP")
, net_acceptsocket(-1)
, net_controlsocket(0)
, net_broadcastsocket(0)
, myAddr(0)
{
	memset(&broadcastaddr, 0, sizeof(broadcastaddr));
}

//==========================================================================
//
//  VMPathDriver::Init
//
//==========================================================================

int VMPathDriver::Init()
{
	guard(VMPathDriver::Init);
	hostent		*local = NULL;
	char		buff[MAXHOSTNAMELEN];
	sockaddr_t	addr;
	char		*colon;
	char		*p;
	int			i;
	
	if (!GArgs.CheckParm("-mpath"))
		return -1;

	flat_selector = __dpmi_allocate_ldt_descriptors(1);
	if (flat_selector == -1)
	{
		GCon->Log(NAME_Init, "MPATH_Init: Can't get flat selector");
		return -1;
	}
	if (__dpmi_set_segment_base_address(flat_selector, 0) == -1)
	{
		GCon->Log(NAME_Init, "MPATH_Init: Can't seg flat base!");
		return -1;
	}
	if (__dpmi_set_segment_limit(flat_selector, 0xffffffff) == -1)
	{
		GCon->Log(NAME_Init, "MPATH_Init: Can't set segment limit");
		return -1;
	}

	// determine my name & address
	if (gethostname(buff, MAXHOSTNAMELEN) == 0)
		local = gethostbyname(buff);
	if (local)
	{
		myAddr = *(int *)local->h_addr_list[0];

		// if the Vavoom hostname isn't set, set it to the machine name
		if (strcmp(VNetwork::HostName, "UNNAMED") == 0)
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
			VNetwork::HostName = buff;
		}
	}

	if ((net_controlsocket = OpenSocket(0)) == -1)
		Sys_Error("MPATH_Init: Unable to open control socket\n");

	((sockaddr_in *)&broadcastaddr)->sin_family = AF_INET;
	((sockaddr_in *)&broadcastaddr)->sin_addr.s_addr = INADDR_BROADCAST;
	((sockaddr_in *)&broadcastaddr)->sin_port = htons(GNet->HostPort);

	GetSocketAddr(net_controlsocket, &addr);
	strcpy(GNet->MyIpAddress, AddrToString(&addr));
	colon = strrchr(GNet->MyIpAddress, ':');
	if (colon)
		*colon = 0;

	GCon->Log(NAME_Init, "MPath Initialised");
	GNet->IpAvailable = true;

	return net_controlsocket;
	unguard;
}

//==========================================================================
//
//  VMPathDriver::Shutdown
//
//==========================================================================

void VMPathDriver::Shutdown()
{
	guard(VMPathDriver::Shutdown);
	Listen(false);
	CloseSocket(net_controlsocket);
	unguard;
}

//==========================================================================
//
//  VMPathDriver::Listen
//
//==========================================================================

void VMPathDriver::Listen(bool state)
{
	guard(VMPathDriver::Listen);
	if (state)
	{
		// enable listening
		if (net_acceptsocket == -1)
		{
			net_acceptsocket = OpenSocket(GNet->HostPort);
			if (net_acceptsocket == -1)
				Sys_Error("MPATH_Listen: Unable to open accept socket\n");
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
//  VMPathDriver::OpenSocket
//
//==========================================================================

int VMPathDriver::OpenSocket(int port)
{
	guard(VMPathDriver::OpenSocket);
	int			newsocket;
	sockaddr_in	address;
	u_long		trueval = true;

	newsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (newsocket == -1)
	{
		return -1;
	}

	if (ioctlsocket(newsocket, FIONBIO, &trueval) == -1)
	{
		closesocket(newsocket);
		return -1;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(newsocket, (sockaddr*)&address, sizeof(address)) == -1)
	{
		closesocket(newsocket);
		return -1;
	}

	return newsocket;
	unguard;
}

//==========================================================================
//
//  VMPathDriver::CloseSocket
//
//==========================================================================

int VMPathDriver::CloseSocket(int socket)
{
	guard(VMPathDriver::CloseSocket);
	if (socket == net_broadcastsocket)
		net_broadcastsocket = 0;
	return closesocket(socket);
	unguard;
}

//==========================================================================
//
//  VMPathDriver::Connect
//
//==========================================================================

int VMPathDriver::Connect(int , sockaddr_t *)
{
	return 0;
}

//==========================================================================
//
//  VMPathDriver::CheckNewConnections
//
//==========================================================================

int VMPathDriver::CheckNewConnections()
{
	guard(VMPathDriver::CheckNewConnections);
	char	buf[4];

	if (net_acceptsocket == -1)
		return -1;

	if (recvfrom(net_acceptsocket, buf, 4, MSG_PEEK, NULL, NULL) >= 0)
		return net_acceptsocket;
	return -1;
	unguard;
}

//==========================================================================
//
//  VMPathDriver::Read
//
//==========================================================================

int VMPathDriver::Read(int socket, vuint8* buf, int len, sockaddr_t* addr)
{
	guard(VMPathDriver::Read);
	int		addrlen = sizeof(sockaddr_t);
	int		ret;

	ret = recvfrom(socket, (char*)buf, len, 0, (sockaddr *)addr, &addrlen);
	if (ret == -1)
	{
		int errno = WSAGetLastError();

		if (errno == WSAEWOULDBLOCK || errno == WSAECONNREFUSED)
			return 0;
	}
	return ret;
	unguard;
}

//==========================================================================
//
//  VMPathDriver::Write
//
//==========================================================================

int VMPathDriver::Write(int socket, vuint8* buf, int len, sockaddr_t* addr)
{
	guard(VMPathDriver::Write);
	int ret;

	ret = sendto(socket, (char*)buf, len, 0, (sockaddr*)addr, sizeof(sockaddr));
	if (ret == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
		return 0;

	sockets_flush();

	return ret;
	unguard;
}

//==========================================================================
//
//  VMPathDriver::Broadcast
//
//==========================================================================

int VMPathDriver::Broadcast(int socket, vuint8* buf, int len)
{
	guard(VMPathDriver::Broadcast);
	int			i = 1;

	if (socket != net_broadcastsocket)
	{
		if (net_broadcastsocket != 0)
			Sys_Error("Attempted to use multiple broadcasts sockets\n");

		// make this socket broadcast capable
		if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (char*)&i, sizeof(i)) < 0)
		{
			GCon->Log(NAME_DevNet, "Unable to make socket broadcast capable");
			return -1;
		}

		net_broadcastsocket = socket;
	}

	return Write(socket, buf, len, &broadcastaddr);
	unguard;
}

//==========================================================================
//
//  VMPathDriver::AddrToString
//
//==========================================================================

char* VMPathDriver::AddrToString(sockaddr_t* addr)
{
	guard(VMPathDriver::AddrToString);
	static char buffer[22];
	int haddr;

	haddr = ntohl(((sockaddr_in *)addr)->sin_addr.s_addr);
	sprintf(buffer, "%d.%d.%d.%d:%d", (haddr >> 24) & 0xff,
		(haddr >> 16) & 0xff, (haddr >> 8) & 0xff, haddr & 0xff,
		ntohs(((sockaddr_in *)addr)->sin_port));
	return buffer;
	unguard;
}

//==========================================================================
//
//  VMPathDriver::StringToAddr
//
//==========================================================================

int VMPathDriver::StringToAddr(const char* string, sockaddr_t* addr)
{
	guard(VMPathDriver::StringToAddr);
	int ha1, ha2, ha3, ha4, hp;
	int ipaddr;

	sscanf(string, "%d.%d.%d.%d:%d", &ha1, &ha2, &ha3, &ha4, &hp);
	ipaddr = (ha1 << 24) | (ha2 << 16) | (ha3 << 8) | ha4;

	addr->sa_family = AF_INET;
	((sockaddr_in *)addr)->sin_addr.s_addr = htonl(ipaddr);
	((sockaddr_in *)addr)->sin_port = htons(hp);
	return 0;
	unguard;
}

//==========================================================================
//
//  VMPathDriver::GetSocketAddr
//
//==========================================================================

int VMPathDriver::GetSocketAddr(int socket, sockaddr_t* addr)
{
	guard(VMPathDriver::GetSocketAddr);
	int		addrlen = sizeof(sockaddr_t);
	vuint32	a;

	memset(addr, 0, sizeof(sockaddr_t));
	getsockname(socket, (sockaddr *)addr, &addrlen);
	a = ((sockaddr_in *)addr)->sin_addr.s_addr;
	if (a == 0 || a == inet_addr("127.0.0.1"))
		((sockaddr_in *)addr)->sin_addr.s_addr = myAddr;

	return 0;
	unguard;
}

//==========================================================================
//
//  VMPathDriver::GetNameFromAddr
//
//==========================================================================

VStr VMPathDriver::GetNameFromAddr(sockaddr_t* addr)
{
	guard(VMPathDriver::GetNameFromAddr);
	hostent* hostentry = gethostbyaddr((char*)&((sockaddr_in*)addr)->sin_addr,
		sizeof(in_addr), AF_INET);
	if (hostentry)
	{
		return (char*)hostentry->h_name;
	}

	return AddrToString(addr);
	unguard;
}

//==========================================================================
//
//	VMPathDriver::PartialIPAddress
//
//	This lets you type only as much of the net address as required, using
// the local network components to fill in the rest
//
//==========================================================================

int VMPathDriver::PartialIPAddress(const char *in, sockaddr_t *hostaddr)
{
	guard(VMPathDriver::PartialIPAddress);
	char buff[256];
	char *b;
	int addr;
	int num;
	int mask;
	int run;
	int port;
	
	buff[0] = '.';
	b = buff;
	strcpy(buff + 1, in);
	if (buff[1] == '.')
		b++;

	addr = 0;
	mask=-1;
	while (*b == '.')
	{
		b++;
		num = 0;
		run = 0;
		while (!(*b < '0' || *b > '9'))
		{
		  num = num*10 + *b++ - '0';
		  if (++run > 3)
		  	return -1;
		}
		if ((*b < '0' || *b > '9') && *b != '.' && *b != ':' && *b != 0)
			return -1;
		if (num < 0 || num > 255)
			return -1;
		mask <<= 8;
		addr = (addr << 8) + num;
	}
	
	if (*b++ == ':')
		port = atoi(b);
	else
		port = GNet->HostPort;

	hostaddr->sa_family = AF_INET;
	((sockaddr_in *)hostaddr)->sin_port = htons((short)port);	
	((sockaddr_in *)hostaddr)->sin_addr.s_addr = (myAddr & htonl(mask)) | htonl(addr);
	
	return 0;
	unguard;
}

//==========================================================================
//
//  VMPathDriver::GetAddrFromName
//
//==========================================================================

int VMPathDriver::GetAddrFromName(const char* name, sockaddr_t* addr)
{
	guard(VMPathDriver::GetAddrFromName);
	hostent*		hostentry;

	if (name[0] >= '0' && name[0] <= '9')
		return PartialIPAddress(name, addr);
	
	hostentry = gethostbyname(name);
	if (!hostentry)
		return -1;

	addr->sa_family = AF_INET;
	((sockaddr_in *)addr)->sin_port = htons(GNet->HostPort);
	((sockaddr_in *)addr)->sin_addr.s_addr = *(int*)hostentry->h_addr_list[0];

	return 0;
	unguard;
}

//==========================================================================
//
//  VMPathDriver::AddrCompare
//
//==========================================================================

int VMPathDriver::AddrCompare(sockaddr_t* addr1, sockaddr_t* addr2)
{
	guard(VMPathDriver::AddrCompare);
	if (addr1->sa_family != addr2->sa_family)
		return -1;

	if (((sockaddr_in *)addr1)->sin_addr.s_addr != ((sockaddr_in *)addr2)->sin_addr.s_addr)
		return -1;

	if (((sockaddr_in *)addr1)->sin_port != ((sockaddr_in *)addr2)->sin_port)
		return 1;

	return 0;
	unguard;
}

//==========================================================================
//
//  VMPathDriver::GetSocketPort
//
//==========================================================================

int VMPathDriver::GetSocketPort(sockaddr_t* addr)
{
	guard(VMPathDriver::GetSocketPort);
	return ntohs(((sockaddr_in *)addr)->sin_port);
	unguard;
}

//==========================================================================
//
//  VMPathDriver::SetSocketPort
//
//==========================================================================

int VMPathDriver::SetSocketPort(sockaddr_t* addr, int port)
{
	guard(VMPathDriver::SetSocketPort);
	((sockaddr_in*)addr)->sin_port = htons(port);
	return 0;
	unguard;
}
