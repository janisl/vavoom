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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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
#include "net_wipx.h"

// MACROS ------------------------------------------------------------------

#define MAXHOSTNAMELEN		256
#define IPXSOCKETS			18

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern boolean		winsock_initialized;
extern WSADATA		winsockdata;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int			net_acceptsocket = -1;		// socket for fielding new connections
static int			net_controlsocket;
static sockaddr_t	broadcastaddr;

static int			ipxsocket[IPXSOCKETS];
static int			sequence[IPXSOCKETS];

static char			packetBuffer[NET_DATAGRAMSIZE + 4];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  WIPX_Init
//
//==========================================================================

int WIPX_Init(void)
{
	int			i;
	char		buff[MAXHOSTNAMELEN];
	sockaddr_t	addr;
	char		*p;
	int			r;

	if (M_CheckParm("-noipx"))
		return -1;

	if (winsock_initialized == 0)
	{
		r = WSAStartup(MAKEWORD(1, 1), &winsockdata);

		if (r)
		{
			con << "Winsock initialization failed.\n";
			return -1;
		}
	}
	winsock_initialized++;

	for (i = 0; i < IPXSOCKETS; i++)
		ipxsocket[i] = 0;

	// determine my name & address
	if (gethostname(buff, MAXHOSTNAMELEN) == 0)
	{
		// if the quake hostname isn't set, set it to the machine name
		if (strcmp(hostname.string, "UNNAMED") == 0)
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

    net_controlsocket = WIPX_OpenSocket(0);
	if (net_controlsocket == -1)
	{
		con << "WIPX_Init: Unable to open control socket\n";
		if (--winsock_initialized == 0)
			WSACleanup();
		return -1;
	}

	((sockaddr_ipx *)&broadcastaddr)->sa_family = AF_IPX;
	memset(((sockaddr_ipx *)&broadcastaddr)->sa_netnum, 0, 4);
	memset(((sockaddr_ipx *)&broadcastaddr)->sa_nodenum, 0xff, 6);
	((sockaddr_ipx *)&broadcastaddr)->sa_socket = htons((word)net_hostport);

	WIPX_GetSocketAddr(net_controlsocket, &addr);
	strcpy(my_ipx_address, WIPX_AddrToString(&addr));
	p = strrchr(my_ipx_address, ':');
	if (p)
		*p = 0;

	con << "Winsock IPX Initialized\n";
	ipxAvailable = true;

	return net_controlsocket;
}

//==========================================================================
//
//  WIPX_Shutdown
//
//==========================================================================

void WIPX_Shutdown(void)
{
	WIPX_Listen(false);
	WIPX_CloseSocket(net_controlsocket);
	if (--winsock_initialized == 0)
		WSACleanup();
}

//==========================================================================
//
//  WIPX_Listen
//
//==========================================================================

void WIPX_Listen(boolean state)
{
	// enable listening
	if (state)
	{
		if (net_acceptsocket == -1)
		{
	        net_acceptsocket = WIPX_OpenSocket(net_hostport);
			if (net_acceptsocket == -1)
				Sys_Error("WIPX_Listen: Unable to open accept socket\n");
		}
	}
	else
    {
		// disable listening
		if (net_acceptsocket != -1)
		{
			WIPX_CloseSocket (net_acceptsocket);
			net_acceptsocket = -1;
		}
	}
}

//==========================================================================
//
//  WIPX_OpenSocket
//
//==========================================================================

int WIPX_OpenSocket(int port)
{
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
}

//==========================================================================
//
//  WIPX_CloseSocket
//
//==========================================================================

int WIPX_CloseSocket(int handle)
{
	int socket = ipxsocket[handle];
	int ret;

	ret =  closesocket(socket);
	ipxsocket[handle] = 0;
	return ret;
}

//==========================================================================
//
//  WIPX_Connect
//
//==========================================================================

int WIPX_Connect(int, sockaddr_t*)
{
	return 0;
}

//==========================================================================
//
//  WIPX_CheckNewConnections
//
//==========================================================================

int WIPX_CheckNewConnections(void)
{
	dword		available;

	if (net_acceptsocket == -1)
		return -1;

	if (ioctlsocket(ipxsocket[net_acceptsocket], FIONREAD, &available) == -1)
		Sys_Error("WIPX: ioctlsocket (FIONREAD) failed\n");
	if (available)
		return net_acceptsocket;
	return -1;
}

//==========================================================================
//
//  WIPX_Read
//
//==========================================================================

int WIPX_Read(int handle, byte *buf, int len, sockaddr_t *addr)
{
	int 	addrlen = sizeof(sockaddr_t);
	int 	socket = ipxsocket[handle];
	int 	ret;

	ret = recvfrom(socket, packetBuffer, len + 4, 0, (struct sockaddr *)addr, &addrlen);
	if (ret == -1)
	{
		int errno = WSAGetLastError();

		if (errno == WSAEWOULDBLOCK || errno == WSAECONNREFUSED)
			return 0;
	}

	if (ret < 4)
		return 0;
	
	// remove sequence number, it's only needed for DOS IPX
	ret -= 4;
	memcpy(buf, packetBuffer + 4, ret);

	return ret;
}

//==========================================================================
//
//  WIPX_Write
//
//==========================================================================

int WIPX_Write(int handle, byte *buf, int len, sockaddr_t *addr)
{
	int 	socket = ipxsocket[handle];
	int 	ret;

	// build packet with sequence number
	*(int *)(&packetBuffer[0]) = sequence[handle];
	sequence[handle]++;
	memcpy(&packetBuffer[4], buf, len);
	len += 4;

	ret = sendto(socket, packetBuffer, len, 0, (sockaddr *)addr, sizeof(sockaddr_t));
	if (ret == -1)
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return 0;

	return ret;
}

//==========================================================================
//
//  WIPX_Broadcast
//
//==========================================================================

int WIPX_Broadcast(int handle, byte *buf, int len)
{
	return WIPX_Write(handle, buf, len, &broadcastaddr);
}

//==========================================================================
//
//  WIPX_AddrToString
//
//==========================================================================

char *WIPX_AddrToString(sockaddr_t *addr)
{
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
}

//==========================================================================
//
//  WIPX_StringToAddr
//
//==========================================================================

int WIPX_StringToAddr(char *string, sockaddr_t *addr)
{
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
}

//==========================================================================
//
//  WIPX_GetSocketAddr
//
//==========================================================================

int WIPX_GetSocketAddr(int handle, sockaddr_t *addr)
{
	int 	socket = ipxsocket[handle];
	int 	addrlen = sizeof(sockaddr_t);

	memset(addr, 0, sizeof(sockaddr_t));
	if (getsockname(socket, (sockaddr *)addr, &addrlen) != 0)
	{
		WSAGetLastError();
	}

	return 0;
}

//==========================================================================
//
//  WIPX_GetNameFromAddr
//
//==========================================================================

int WIPX_GetNameFromAddr(sockaddr_t *addr, char *name)
{
	strcpy(name, WIPX_AddrToString(addr));
	return 0;
}

//==========================================================================
//
//  WIPX_GetAddrFromName
//
//==========================================================================

int WIPX_GetAddrFromName(char *name, sockaddr_t *addr)
{
	int		n;
	char	buf[32];

	n = strlen(name);

	if (n == 12)
	{
		sprintf(buf, "00000000:%s:%u", name, net_hostport);
		return WIPX_StringToAddr(buf, addr);
	}
	if (n == 21)
	{
		sprintf(buf, "%s:%u", name, net_hostport);
		return WIPX_StringToAddr(buf, addr);
	}
	if (n > 21 && n <= 27)
		return WIPX_StringToAddr(name, addr);

	return -1;
}

//==========================================================================
//
//  WIPX_AddrCompare
//
//==========================================================================

int WIPX_AddrCompare(sockaddr_t *addr1, sockaddr_t *addr2)
{
	if (addr1->sa_family != addr2->sa_family)
		return -1;

	if (*((sockaddr_ipx *)addr1)->sa_netnum && *((sockaddr_ipx *)addr2)->sa_netnum)
		if (memcmp(((sockaddr_ipx *)addr1)->sa_netnum, ((sockaddr_ipx *)addr2)->sa_netnum, 4) != 0)
			return -1;
	if (memcmp(((sockaddr_ipx *)addr1)->sa_nodenum, ((sockaddr_ipx *)addr2)->sa_nodenum, 6) != 0)
		return -1;

	if (((sockaddr_ipx *)addr1)->sa_socket != ((sockaddr_ipx *)addr2)->sa_socket)
		return 1;

	return 0;
}

//==========================================================================
//
//  WIPX_GetSocketPort
//
//==========================================================================

int WIPX_GetSocketPort(sockaddr_t *addr)
{
	return ntohs(((sockaddr_ipx *)addr)->sa_socket);
}

//==========================================================================
//
//  WIPX_SetSocketPort
//
//==========================================================================

int WIPX_SetSocketPort(sockaddr_t *addr, int port)
{
	((sockaddr_ipx *)addr)->sa_socket = htons((word)port);
	return 0;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2001/10/09 17:28:12  dj_jl
//	no message
//
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
