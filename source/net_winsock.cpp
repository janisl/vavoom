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

#include "winlocal.h"
#include <errno.h>
#include "gamedefs.h"
#include "net_local.h"

// MACROS ------------------------------------------------------------------

#define MAXHOSTNAMELEN		256

// TYPES -------------------------------------------------------------------

class VWinSockDriver : public VNetLanDriver
{
public:
	int				net_acceptsocket;	// socket for fielding new connections
	int				net_controlsocket;
	int				net_broadcastsocket;
	sockaddr_t		broadcastaddr;

	vuint32			myAddr;

	int				winsock_initialised;
	WSADATA			winsockdata;

	static double	blocktime;

	VWinSockDriver();
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

	static BOOL PASCAL FAR BlockingHook();
	void GetLocalAddress();

	int PartialIPAddress(const char*, sockaddr_t*);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

double		VWinSockDriver::blocktime;

static VWinSockDriver	Impl;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VWinSockDriver::VWinSockDriver
//
//==========================================================================

VWinSockDriver::VWinSockDriver()
: VNetLanDriver(0, "Winsock TCPIP")
, net_acceptsocket(-1)
, net_controlsocket(0)
, net_broadcastsocket(0)
, myAddr(0)
, winsock_initialised(0)
{
	memset(&broadcastaddr, 0, sizeof(broadcastaddr));
}

//==========================================================================
//
//  VWinSockDriver::Init
//
//==========================================================================

int VWinSockDriver::Init()
{
	guard(VWinSockDriver::Init);
	int		i;
	char	buff[MAXHOSTNAMELEN];
	char	*p;
	int		r;

	if (GArgs.CheckParm("-noudp"))
		return -1;

	if (winsock_initialised == 0)
	{
		//MAKEWORD(2, 2)
		r = WSAStartup(MAKEWORD(1, 1), &winsockdata);

		if (r)
		{
			GCon->Log(NAME_Init, "Winsock initialisation failed.");
			return -1;
		}
	}
	winsock_initialised++;

	// determine my name
	if (gethostname(buff, MAXHOSTNAMELEN) == SOCKET_ERROR)
	{
		GCon->Log(NAME_DevNet, "Winsock TCP/IP Initialisation failed.");
		if (--winsock_initialised == 0)
			WSACleanup();
		return -1;
	}

	// if the Vavoom hostname isn't set, set it to the machine name
	if (VStr::Cmp(Net->HostName, "UNNAMED") == 0)
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
		Net->HostName = buff;
	}

	const char* pp = GArgs.CheckValue("-ip");
	if (pp)
	{
		myAddr = inet_addr(pp);
		if (myAddr == INADDR_NONE)
			Sys_Error("%s is not a valid IP address", pp);
		VStr::Cpy(Net->MyIpAddress, pp);
	}
	else
	{
		myAddr = INADDR_ANY;
		VStr::Cpy(Net->MyIpAddress, "INADDR_ANY");
	}

    net_controlsocket = OpenSocket(0);
	if (net_controlsocket == -1)
	{
		GCon->Log(NAME_Init, "WINS_Init: Unable to open control socket");
		if (--winsock_initialised == 0)
			WSACleanup();
		return -1;
	}

	((sockaddr_in *)&broadcastaddr)->sin_family = AF_INET;
	((sockaddr_in *)&broadcastaddr)->sin_addr.s_addr = INADDR_BROADCAST;
	((sockaddr_in *)&broadcastaddr)->sin_port = htons((vuint16)Net->HostPort);

	GCon->Log(NAME_Init, "Winsock TCP/IP Initialised");
	Net->IpAvailable = true;

	return net_controlsocket;
	unguard;
}

//==========================================================================
//
//  VWinSockDriver::Shutdown
//
//==========================================================================

void VWinSockDriver::Shutdown()
{
	guard(VWinSockDriver::Shutdown);
	Listen(false);
	CloseSocket(net_controlsocket);
	if (--winsock_initialised == 0)
		WSACleanup();
	unguard;
}

//==========================================================================
//
//  VWinSockDriver::BlockingHook
//
//==========================================================================

BOOL PASCAL FAR VWinSockDriver::BlockingHook()
{
	guard(VWinSockDriver::BlockingHook);
	MSG		msg;
	BOOL	ret;

	if ((Sys_Time() - blocktime) > 2.0)
	{
		WSACancelBlockingCall();
		return FALSE;
	}

	// get the next message, if any
	ret = (BOOL)PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

	// if we got one, process it
	if (ret)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// TRUE if we got a message
	return ret;
	unguard;
} 

//==========================================================================
//
//  VWinSockDriver::GetLocalAddress
//
//==========================================================================

void VWinSockDriver::GetLocalAddress()
{
	guard(VWinSockDriver::GetLocalAddress);
	hostent		*local;
	char		buff[MAXHOSTNAMELEN];
	vuint32		addr;

	if (myAddr != INADDR_ANY)
		return;

	if (gethostname(buff, MAXHOSTNAMELEN) == SOCKET_ERROR)
		return;

	blocktime = Sys_Time();
	WSASetBlockingHook(BlockingHook);
	local = gethostbyname(buff);
	WSAUnhookBlockingHook();
	if (local == NULL)
		return;

	myAddr = *(int *)local->h_addr_list[0];

	addr = ntohl(myAddr);
	sprintf(Net->MyIpAddress, "%d.%d.%d.%d", (addr >> 24) & 0xff,
		(addr >> 16) & 0xff, (addr >> 8) & 0xff, addr & 0xff);
	unguard;
}

//==========================================================================
//
//  VWinSockDriver::Listen
//
//==========================================================================

void VWinSockDriver::Listen(bool state)
{
	guard(VWinSockDriver::Listen);
	if (state)
	{
		// enable listening
		if (net_acceptsocket == -1)
		{
			GetLocalAddress();
			net_acceptsocket = OpenSocket(Net->HostPort);
			if (net_acceptsocket == -1)
				Sys_Error("WINS_Listen: Unable to open accept socket\n");
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
//  VWinSockDriver::OpenSocket
//
//==========================================================================

int VWinSockDriver::OpenSocket(int port)
{
	guard(VWinSockDriver::OpenSocket);
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

	Sys_Error("Unable to bind to %s", AddrToString((sockaddr_t *)&address));

ErrorReturn:
	closesocket(newsocket);
	return -1;
	unguard;
}

//==========================================================================
//
//  VWinSockDriver::CloseSocket
//
//==========================================================================

int VWinSockDriver::CloseSocket(int socket)
{
	guard(VWinSockDriver::CloseSocket);
	if (socket == net_broadcastsocket)
		net_broadcastsocket = 0;
	return closesocket(socket);
	unguard;
}

//==========================================================================
//
//  VWinSockDriver::Connect
//
//==========================================================================

int VWinSockDriver::Connect(int , sockaddr_t *)
{
	return 0;
}

//==========================================================================
//
//  VWinSockDriver::CheckNewConnections
//
//==========================================================================

int VWinSockDriver::CheckNewConnections()
{
	guard(VWinSockDriver::CheckNewConnections);
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
//  VWinSockDriver::Read
//
//==========================================================================

int VWinSockDriver::Read(int socket, vuint8* buf, int len, sockaddr_t* addr)
{
	guard(VWinSockDriver::Read);
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
//  VWinSockDriver::Write
//
//==========================================================================

int VWinSockDriver::Write(int socket, vuint8* buf, int len, sockaddr_t* addr)
{
	guard(VWinSockDriver::Write);
	int ret = sendto(socket, (char*)buf, len, 0, (sockaddr*)addr, sizeof(sockaddr_t));
	if (ret == -1)
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return 0;

	return ret;
	unguard;
}

//==========================================================================
//
//  VWinSockDriver::Broadcast
//
//==========================================================================

int VWinSockDriver::Broadcast(int socket, vuint8* buf, int len)
{
	guard(VWinSockDriver::Broadcast);
	int	i = 1;

	if (socket != net_broadcastsocket)
	{
		if (net_broadcastsocket != 0)
			Sys_Error("Attempted to use multiple broadcasts sockets\n");

		GetLocalAddress();

		// make this socket broadcast capable
		if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i)) < 0)
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
//  VWinSockDriver::AddrToString
//
//==========================================================================

char* VWinSockDriver::AddrToString(sockaddr_t* addr)
{
	guard(VWinSockDriver::AddrToString);
	static char buffer[22];
	int haddr;

	haddr = ntohl(((sockaddr_in *)addr)->sin_addr.s_addr);
	sprintf(buffer, "%d.%d.%d.%d:%d", (haddr >> 24) & 0xff,
		(haddr >> 16) & 0xff, (haddr >> 8) & 0xff,
		haddr & 0xff, ntohs(((sockaddr_in *)addr)->sin_port));
	return buffer;
	unguard;
}

//==========================================================================
//
//  VWinSockDriver::StringToAddr
//
//==========================================================================

int VWinSockDriver::StringToAddr(const char* string, sockaddr_t* addr)
{
	guard(VWinSockDriver::StringToAddr);
	int ha1, ha2, ha3, ha4, hp;
	int ipaddr;

	sscanf(string, "%d.%d.%d.%d:%d", &ha1, &ha2, &ha3, &ha4, &hp);
	ipaddr = (ha1 << 24) | (ha2 << 16) | (ha3 << 8) | ha4;

	addr->sa_family = AF_INET;
	((sockaddr_in *)addr)->sin_addr.s_addr = htonl(ipaddr);
	((sockaddr_in *)addr)->sin_port = htons((word)hp);
	return 0;
	unguard;
}

//==========================================================================
//
//  VWinSockDriver::GetSocketAddr
//
//==========================================================================

int VWinSockDriver::GetSocketAddr(int socket, sockaddr_t *addr)
{
	guard(VWinSockDriver::GetSocketAddr);
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
//  VWinSockDriver::GetNameFromAddr
//
//==========================================================================

VStr VWinSockDriver::GetNameFromAddr(sockaddr_t* addr)
{
	guard(VWinSockDriver::GetNameFromAddr);
	hostent* hostentry = gethostbyaddr((char*)&((sockaddr_in*)addr)->sin_addr,
		sizeof(struct in_addr), AF_INET);
	if (hostentry)
	{
		return (char*)hostentry->h_name;
	}

	return AddrToString(addr);
	unguard;
}

//==========================================================================
//
//	VWinSockDriver::PartialIPAddress
//
//	This lets you type only as much of the net address as required, using
// the local network components to fill in the rest
//
//==========================================================================

int VWinSockDriver::PartialIPAddress(const char *in, sockaddr_t *hostaddr)
{
	guard(PartialIPAddress);
	char buff[256];
	char *b;
	int addr;
	int num;
	int mask;
	int run;
	int port;
	
	buff[0] = '.';
	b = buff;
	VStr::Cpy(buff + 1, in);
	if (buff[1] == '.')
		b++;

	addr = 0;
	mask=-1;
	while (*b == '.')
	{
		b++;
		num = 0;
		run = 0;
		while (!( *b < '0' || *b > '9'))
		{
		  num = num*10 + *b++ - '0';
		  if (++run > 3)
		  	return -1;
		}
		if ((*b < '0' || *b > '9') && *b != '.' && *b != ':' && *b != 0)
			return -1;
		if (num < 0 || num > 255)
			return -1;
		mask <<=8;
		addr = (addr << 8) + num;
	}
	
	if (*b++ == ':')
		port = atoi(b);
	else
		port = Net->HostPort;

	hostaddr->sa_family = AF_INET;
	((sockaddr_in *)hostaddr)->sin_port = htons((short)port);	
	((sockaddr_in *)hostaddr)->sin_addr.s_addr = (myAddr & htonl(mask)) | htonl(addr);
	
	return 0;
	unguard;
}

//==========================================================================
//
//  VWinSockDriver::GetAddrFromName
//
//==========================================================================

int VWinSockDriver::GetAddrFromName(const char *name, sockaddr_t *addr)
{
	guard(VWinSockDriver::GetAddrFromName);
	hostent*	hostentry;

	if (name[0] >= '0' && name[0] <= '9')
		return PartialIPAddress(name, addr);
	
	hostentry = gethostbyname(name);
	if (!hostentry)
		return -1;

	addr->sa_family = AF_INET;
	((sockaddr_in *)addr)->sin_port = htons((vuint16)Net->HostPort);
	((sockaddr_in *)addr)->sin_addr.s_addr = *(int *)hostentry->h_addr_list[0];

	return 0;
	unguard;
}

//==========================================================================
//
//  VWinSockDriver::AddrCompare
//
//==========================================================================

int VWinSockDriver::AddrCompare(sockaddr_t* addr1, sockaddr_t* addr2)
{
	guard(VWinSockDriver::AddrCompare);
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
//  VWinSockDriver::GetSocketPort
//
//==========================================================================

int VWinSockDriver::GetSocketPort(sockaddr_t* addr)
{
	guard(VWinSockDriver::GetSocketPort);
	return ntohs(((sockaddr_in*)addr)->sin_port);
	unguard;
}

//==========================================================================
//
//  VWinSockDriver::SetSocketPort
//
//==========================================================================

int VWinSockDriver::SetSocketPort(sockaddr_t* addr, int port)
{
	guard(VWinSockDriver::SetSocketPort);
	((sockaddr_in*)addr)->sin_port = htons((word)port);
	return 0;
	unguard;
}
