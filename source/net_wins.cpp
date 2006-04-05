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
#include <errno.h>
#include "gamedefs.h"
#include "net_loc.h"
#include "net_wins.h"

// MACROS ------------------------------------------------------------------

#define MAXHOSTNAMELEN		256

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int			winsock_initialized = 0;
WSADATA		winsockdata;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int			net_acceptsocket = -1;		// socket for fielding new connections
static int			net_controlsocket;
static int			net_broadcastsocket = 0;
static sockaddr_t	broadcastaddr;

static dword		myAddr;

static double		blocktime;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  WINS_Init
//
//==========================================================================

int WINS_Init()
{
	guard(WINS_Init);
	int		i;
	char	buff[MAXHOSTNAMELEN];
	char	*p;
	int		r;

	if (GArgs.CheckParm("-noudp"))
		return -1;

	if (winsock_initialized == 0)
	{
		//MAKEWORD(2, 2)
		r = WSAStartup(MAKEWORD(1, 1), &winsockdata);

		if (r)
		{
			GCon->Log(NAME_Init, "Winsock initialization failed.");
			return -1;
		}
	}
	winsock_initialized++;

	// determine my name
	if (gethostname(buff, MAXHOSTNAMELEN) == SOCKET_ERROR)
	{
		GCon->Log(NAME_DevNet, "Winsock TCP/IP Initialization failed.");
		if (--winsock_initialized == 0)
			WSACleanup();
		return -1;
	}

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

	const char* pp = GArgs.CheckValue("-ip");
	if (pp)
	{
		myAddr = inet_addr(pp);
		if (myAddr == INADDR_NONE)
			Sys_Error("%s is not a valid IP address", pp);
		strcpy(my_tcpip_address, pp);
	}
	else
	{
		myAddr = INADDR_ANY;
		strcpy(my_tcpip_address, "INADDR_ANY");
	}

    net_controlsocket = WINS_OpenSocket(0);
	if (net_controlsocket == -1)
	{
		GCon->Log(NAME_Init, "WINS_Init: Unable to open control socket");
		if (--winsock_initialized == 0)
			WSACleanup();
		return -1;
	}

	((sockaddr_in *)&broadcastaddr)->sin_family = AF_INET;
	((sockaddr_in *)&broadcastaddr)->sin_addr.s_addr = INADDR_BROADCAST;
	((sockaddr_in *)&broadcastaddr)->sin_port = htons((word)net_hostport);

	GCon->Log(NAME_Init, "Winsock TCP/IP Initialized");
	tcpipAvailable = true;

	return net_controlsocket;
	unguard;
}

//==========================================================================
//
//  WINS_Shutdown
//
//==========================================================================

void WINS_Shutdown()
{
	guard(WINS_Shutdown);
	WINS_Listen(false);
	WINS_CloseSocket(net_controlsocket);
	if (--winsock_initialized == 0)
		WSACleanup();
	unguard;
}

//==========================================================================
//
//  BlockingHook
//
//==========================================================================

static BOOL PASCAL FAR BlockingHook()
{
	guard(BlockingHook);
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
//  WINS_GetLocalAddress
//
//==========================================================================

static void WINS_GetLocalAddress()
{
	guard(WINS_GetLocalAddress);
	hostent		*local;
	char		buff[MAXHOSTNAMELEN];
	dword		addr;

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
	sprintf(my_tcpip_address, "%d.%d.%d.%d", (addr >> 24) & 0xff, (addr >> 16) & 0xff, (addr >> 8) & 0xff, addr & 0xff);
	unguard;
}

//==========================================================================
//
//  WINS_Listen
//
//==========================================================================

void WINS_Listen(bool state)
{
	guard(WINS_Listen);
	if (state)
	{
		// enable listening
		if (net_acceptsocket == -1)
		{
			WINS_GetLocalAddress();
            net_acceptsocket = WINS_OpenSocket(net_hostport);
			if (net_acceptsocket == -1)
				Sys_Error("WINS_Listen: Unable to open accept socket\n");
		}
	}
	else
    {
		// disable listening
		if (net_acceptsocket != -1)
		{
			WINS_CloseSocket(net_acceptsocket);
			net_acceptsocket = -1;
		}
	}
	unguard;
}


//==========================================================================
//
//  WINS_OpenSocket
//
//==========================================================================

int WINS_OpenSocket(int port)
{
	guard(WINS_OpenSocket);
	int				newsocket;
	sockaddr_in		address;
	dword			trueval = 1;

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

	Sys_Error("Unable to bind to %s", WINS_AddrToString((sockaddr_t *)&address));

ErrorReturn:
	closesocket(newsocket);
	return -1;
	unguard;
}

//==========================================================================
//
//  WINS_CloseSocket
//
//==========================================================================

int WINS_CloseSocket(int socket)
{
	guard(WINS_CloseSocket);
	if (socket == net_broadcastsocket)
		net_broadcastsocket = 0;
	return closesocket(socket);
	unguard;
}

//==========================================================================
//
//  WINS_Connect
//
//==========================================================================

int WINS_Connect(int , sockaddr_t *)
{
	return 0;
}

//==========================================================================
//
//  WINS_CheckNewConnections
//
//==========================================================================

int WINS_CheckNewConnections()
{
	guard(WINS_CheckNewConnections);
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
//  WINS_Read
//
//==========================================================================

int WINS_Read(int socket, byte *buf, int len, sockaddr_t *addr)
{
	guard(WINS_Read);
	int addrlen = sizeof(sockaddr_t);
	int ret;

	ret = recvfrom(socket, (char*)buf, len, 0, (sockaddr *)addr, &addrlen);
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
//  WINS_Write
//
//==========================================================================

int WINS_Write(int socket, byte *buf, int len, sockaddr_t *addr)
{
	guard(WINS_Write);
	int ret;

	ret = sendto(socket, (char*)buf, len, 0, (sockaddr *)addr, sizeof(sockaddr_t));
	if (ret == -1)
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return 0;

	return ret;
	unguard;
}

//==========================================================================
//
//  WINS_Broadcast
//
//==========================================================================

int WINS_Broadcast(int socket, byte *buf, int len)
{
	guard(WINS_Broadcast);
	int	i = 1;

	if (socket != net_broadcastsocket)
	{
		if (net_broadcastsocket != 0)
			Sys_Error("Attempted to use multiple broadcasts sockets\n");

		WINS_GetLocalAddress();

		// make this socket broadcast capable
		if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i)) < 0)
		{
			GCon->Log(NAME_DevNet, "Unable to make socket broadcast capable");
			return -1;
		}
	net_broadcastsocket = socket;
	}

	return WINS_Write(socket, buf, len, &broadcastaddr);
	unguard;
}

//==========================================================================
//
//  WINS_AddrToString
//
//==========================================================================

char *WINS_AddrToString(sockaddr_t *addr)
{
	guard(WINS_AddrToString);
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
//  WINS_StringToAddr
//
//==========================================================================

int WINS_StringToAddr(const char *string, sockaddr_t *addr)
{
	guard(WINS_StringToAddr);
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
//  WINS_GetSocketAddr
//
//==========================================================================

int WINS_GetSocketAddr(int socket, sockaddr_t *addr)
{
	guard(WINS_GetSocketAddr);
	int		addrlen = sizeof(sockaddr_t);
	dword	a;

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
//  WINS_GetNameFromAddr
//
//==========================================================================

int WINS_GetNameFromAddr(sockaddr_t *addr, char *name)
{
	guard(WINS_GetNameFromAddr);
	hostent *hostentry;

	hostentry = gethostbyaddr((char *)&((sockaddr_in *)addr)->sin_addr, sizeof(struct in_addr), AF_INET);
	if (hostentry)
	{
		strncpy(name, (char *)hostentry->h_name, NET_NAMELEN - 1);
		return 0;
	}

	strcpy(name, WINS_AddrToString(addr));
	return 0;
	unguard;
}

//==========================================================================
//
//	PartialIPAddress
//
//	This lets you type only as much of the net address as required, using
// the local network components to fill in the rest
//
//==========================================================================

static int PartialIPAddress(const char *in, sockaddr_t *hostaddr)
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
		port = net_hostport;

	hostaddr->sa_family = AF_INET;
	((sockaddr_in *)hostaddr)->sin_port = htons((short)port);	
	((sockaddr_in *)hostaddr)->sin_addr.s_addr = (myAddr & htonl(mask)) | htonl(addr);
	
	return 0;
	unguard;
}

//==========================================================================
//
//  WINS_GetAddrFromName
//
//==========================================================================

int WINS_GetAddrFromName(const char *name, sockaddr_t *addr)
{
	guard(WINS_GetAddrFromName);
	hostent *hostentry;

	if (name[0] >= '0' && name[0] <= '9')
		return PartialIPAddress(name, addr);
	
	hostentry = gethostbyname(name);
	if (!hostentry)
		return -1;

	addr->sa_family = AF_INET;
	((sockaddr_in *)addr)->sin_port = htons((word)net_hostport);
	((sockaddr_in *)addr)->sin_addr.s_addr = *(int *)hostentry->h_addr_list[0];

	return 0;
	unguard;
}

//==========================================================================
//
//  WINS_AddrCompare
//
//==========================================================================

int WINS_AddrCompare(sockaddr_t *addr1, sockaddr_t *addr2)
{
	guard(WINS_AddrCompare);
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
//  WINS_GetSocketPort
//
//==========================================================================

int WINS_GetSocketPort(sockaddr_t *addr)
{
	guard(WINS_GetSocketPort);
	return ntohs(((sockaddr_in *)addr)->sin_port);
	unguard;
}

//==========================================================================
//
//  WINS_SetSocketPort
//
//==========================================================================

int WINS_SetSocketPort(sockaddr_t *addr, int port)
{
	guard(WINS_SetSocketPort);
	((sockaddr_in *)addr)->sin_port = htons((word)port);
	return 0;
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.11  2006/04/05 17:20:37  dj_jl
//	Merged size buffer with message class.
//
//	Revision 1.10  2006/03/20 20:02:21  dj_jl
//	Accept zero length packets.
//	
//	Revision 1.9  2002/08/05 17:20:00  dj_jl
//	Added guarding.
//	
//	Revision 1.8  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.7  2002/01/11 08:12:49  dj_jl
//	Changes for MinGW
//	
//	Revision 1.6  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.5  2001/12/18 19:05:03  dj_jl
//	Made TCvar a pure C++ class
//	
//	Revision 1.4  2001/10/04 17:23:29  dj_jl
//	Got rid of some warnings
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
