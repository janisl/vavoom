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
//**	Copyright (C) 1999-2002 J�nis Legzdi��
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>//
#include <errno.h>
#include <unistd.h>//
#include <netdb.h>
#include <sys/ioctl.h>

//	These 2 includes are from Quake sources, not tested
#ifdef __sun__
#include <sys/filio.h>
#endif

#ifdef NeXT
#include <libc.h>
#endif

#include "gamedefs.h"
#include "net_loc.h"
#include "net_udp.h"

// MACROS ------------------------------------------------------------------

#define MAXHOSTNAMELEN		256

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

//int gethostname(char *, int);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int			net_acceptsocket = -1;		// socket for fielding new connections
static int			net_controlsocket;
static int			net_broadcastsocket = 0;
static sockaddr_t	broadcastaddr;

static dword		myAddr;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  UDP_Init
//
//==========================================================================

int UDP_Init(void)
{
	hostent		*local;
	char		buff[MAXHOSTNAMELEN];
	sockaddr_t	addr;
	char		*colon;
	
	if (M_CheckParm("-noudp"))
		return -1;

	//	determine my name & address
	gethostname(buff, MAXHOSTNAMELEN);
	local = gethostbyname(buff);
	myAddr = *(int *)local->h_addr_list[0];

	// if the quake hostname isn't set, set it to the machine name
	if (strcmp(hostname, "UNNAMED") == 0)
	{
		buff[15] = 0;
		hostname = buff;
	}

	if ((net_controlsocket = UDP_OpenSocket(0)) == -1)
		Sys_Error("UDP_Init: Unable to open control socket\n");

	((sockaddr_in *)&broadcastaddr)->sin_family = AF_INET;
	((sockaddr_in *)&broadcastaddr)->sin_addr.s_addr = INADDR_BROADCAST;
	((sockaddr_in *)&broadcastaddr)->sin_port = htons(net_hostport);

	UDP_GetSocketAddr(net_controlsocket, &addr);
	strcpy(my_tcpip_address, UDP_AddrToString(&addr));
	colon = strrchr(my_tcpip_address, ':');
	if (colon)
		*colon = 0;

	con << "UDP Initialized\n";
	tcpipAvailable = true;

	return net_controlsocket;
}

//==========================================================================
//
//  UDP_Shutdown
//
//==========================================================================

void UDP_Shutdown(void)
{
	UDP_Listen(false);
	UDP_CloseSocket(net_controlsocket);
}

//==========================================================================
//
//  UDP_Listen
//
//==========================================================================

void UDP_Listen(boolean state)
{
	if (state)
	{
		// enable listening
		if (net_acceptsocket == -1)
		{
            net_acceptsocket = UDP_OpenSocket(net_hostport);
			if (net_acceptsocket == -1)
				Sys_Error("UDP_Listen: Unable to open accept socket\n");
		}
	}
	else
	{
		// disable listening
		if (net_acceptsocket != -1)
		{
			UDP_CloseSocket(net_acceptsocket);
			net_acceptsocket = -1;
		}
	}
}
//==========================================================================
//
//  UDP_OpenSocket
//
//==========================================================================

int UDP_OpenSocket(int port)
{
	int			newsocket;
	sockaddr_in	address;
	boolean		trueval = true;

    newsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (newsocket == -1)
	{
		return -1;
	}

	if (ioctl(newsocket, FIONBIO, (char *)&trueval) == -1)
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
//  UDP_CloseSocket
//
//==========================================================================

int UDP_CloseSocket(int socket)
{
	if (socket == net_broadcastsocket)
		net_broadcastsocket = 0;
	return close(socket);
}

//==========================================================================
//
//  UDP_Connect
//
//==========================================================================

int UDP_Connect(int , sockaddr_t *)
{
	return 0;
}

//==========================================================================
//
//  UDP_CheckNewConnections
//
//==========================================================================

int UDP_CheckNewConnections(void)
{
	dword		available;

	if (net_acceptsocket == -1)
		return -1;

	if (ioctl(net_acceptsocket, FIONREAD, &available) == -1)
		Sys_Error("UDP: ioctlsocket (FIONREAD) failed\n");
	if (available)
		return net_acceptsocket;
	return -1;
}

//==========================================================================
//
//  UDP_Read
//
//==========================================================================

int UDP_Read(int socket, byte *buf, int len, sockaddr_t *addr)
{
	socklen_t	addrlen = sizeof(sockaddr_t);
	int		ret;

	ret = recvfrom(socket, buf, len, 0, (sockaddr *)addr, &addrlen);
	if (ret == -1 && (errno == EWOULDBLOCK || errno == ECONNREFUSED))
		return 0;
	return ret;
}

//==========================================================================
//
//  UDP_Write
//
//==========================================================================

int UDP_Write(int socket, byte *buf, int len, sockaddr_t *addr)
{
	int ret;

	ret = sendto(socket, buf, len, 0, (sockaddr *)addr, sizeof(sockaddr));
	if (ret == -1 && errno == EWOULDBLOCK)
		return 0;
	return ret;
}

//==========================================================================
//
//  UDP_Broadcast
//
//==========================================================================

int UDP_Broadcast(int socket, byte *buf, int len)
{
	int			i = 1;


	if (socket != net_broadcastsocket)
	{
		if (net_broadcastsocket != 0)
			Sys_Error("Attempted to use multiple broadcasts sockets\n");

		// make this socket broadcast capable
		if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i)) < 0)
		{
			con << "Unable to make socket broadcast capable\n";
			return -1;
		}

		net_broadcastsocket = socket;
	}

	return UDP_Write(socket, buf, len, &broadcastaddr);
}

//==========================================================================
//
//  UDP_AddrToString
//
//==========================================================================

char *UDP_AddrToString(sockaddr_t *addr)
{
	static char buffer[22];
	int haddr;

	haddr = ntohl(((sockaddr_in *)addr)->sin_addr.s_addr);
	sprintf(buffer, "%d.%d.%d.%d:%d", (haddr >> 24) & 0xff,
		(haddr >> 16) & 0xff, (haddr >> 8) & 0xff, haddr & 0xff,
		ntohs(((sockaddr_in *)addr)->sin_port));
	return buffer;
}

//==========================================================================
//
//  UDP_StringToAddr
//
//==========================================================================

int UDP_StringToAddr(char *string, sockaddr_t *addr)
{
	int ha1, ha2, ha3, ha4, hp;
	int ipaddr;

	sscanf(string, "%d.%d.%d.%d:%d", &ha1, &ha2, &ha3, &ha4, &hp);
	ipaddr = (ha1 << 24) | (ha2 << 16) | (ha3 << 8) | ha4;

	addr->sa_family = AF_INET;
	((sockaddr_in *)addr)->sin_addr.s_addr = htonl(ipaddr);
	((sockaddr_in *)addr)->sin_port = htons(hp);
	return 0;
}

//==========================================================================
//
//  UDP_GetSocketAddr
//
//==========================================================================

int UDP_GetSocketAddr(int socket, sockaddr_t *addr)
{
	socklen_t	addrlen = sizeof(sockaddr_t);
	dword	a;

	memset(addr, 0, sizeof(sockaddr_t));
	getsockname(socket, (sockaddr *)addr, &addrlen);
	a = ((sockaddr_in *)addr)->sin_addr.s_addr;
	if (a == 0 || a == inet_addr("127.0.0.1"))
		((sockaddr_in *)addr)->sin_addr.s_addr = myAddr;

	return 0;
}

//==========================================================================
//
//  UDP_GetNameFromAddr
//
//==========================================================================

int UDP_GetNameFromAddr(sockaddr_t *addr, char *name)
{
	hostent		*hostentry;

	hostentry = gethostbyaddr((char *)&((sockaddr_in *)addr)->sin_addr, sizeof(in_addr), AF_INET);
	if (hostentry)
	{
		strncpy(name, (char *)hostentry->h_name, NET_NAMELEN - 1);
		return 0;
	}

	strcpy(name, UDP_AddrToString(addr));
	return 0;
}

//==========================================================================
//
//	PartialIPAddress
//
//	This lets you type only as much of the net address as required, using
// the local network components to fill in the rest
//
//==========================================================================

static int PartialIPAddress (char *in, sockaddr_t *hostaddr)
{
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
		port = net_hostport;

	hostaddr->sa_family = AF_INET;
	((sockaddr_in *)hostaddr)->sin_port = htons((short)port);	
	((sockaddr_in *)hostaddr)->sin_addr.s_addr = (myAddr & htonl(mask)) | htonl(addr);
	
	return 0;
}

//==========================================================================
//
//  UDP_GetAddrFromName
//
//==========================================================================

int UDP_GetAddrFromName(char *name, sockaddr_t *addr)
{
	hostent		*hostentry;

	if (name[0] >= '0' && name[0] <= '9')
		return PartialIPAddress(name, addr);
	
	hostentry = gethostbyname(name);
	if (!hostentry)
		return -1;

	addr->sa_family = AF_INET;
	((sockaddr_in *)addr)->sin_port = htons(net_hostport);	
	((sockaddr_in *)addr)->sin_addr.s_addr = *(int *)hostentry->h_addr_list[0];

	return 0;
}

//==========================================================================
//
//  UDP_AddrCompare
//
//==========================================================================

int UDP_AddrCompare(sockaddr_t *addr1, sockaddr_t *addr2)
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
//  UDP_GetSocketPort
//
//==========================================================================

int UDP_GetSocketPort(sockaddr_t *addr)
{
	return ntohs(((sockaddr_in *)addr)->sin_port);
}

//==========================================================================
//
//  UDP_SetSocketPort
//
//==========================================================================

int UDP_SetSocketPort(sockaddr_t *addr, int port)
{
	((sockaddr_in *)addr)->sin_port = htons(port);
	return 0;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//
//	Revision 1.4  2001/12/18 19:05:03  dj_jl
//	Made TCvar a pure C++ class
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
