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
#include "array.h"
using namespace VavoomUtils;

#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

#define closesocket		close
#endif

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

enum
{
	MAX_MSGLEN				= 1024,
	MASTER_SERVER_PORT		= 26001,
};

// TYPES -------------------------------------------------------------------

struct TSrvItem
{
	sockaddr	Addr;
	time_t		Time;
};

#ifdef _WIN32
class TWinSockHelper
{
	~TWinSockHelper()
	{
		WSACleanup();
	}
};
#endif

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int					AcceptSocket = -1;		// socket for fielding new connections
static TArray<TSrvItem>		SrvList;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  AddrCompare
//
//==========================================================================

static int AddrCompare(sockaddr* addr1, sockaddr* addr2)
{
	if (addr1->sa_family != addr2->sa_family)
	{
		return -1;
	}

	if (((sockaddr_in*)addr1)->sin_addr.s_addr != ((sockaddr_in*)addr2)->sin_addr.s_addr)
	{
		return -1;
	}

	if (((sockaddr_in*)addr1)->sin_port != ((sockaddr_in*)addr2)->sin_port)
	{
		return 1;
	}

	return 0;
}
//==========================================================================
//
//	ReadNet
//
//==========================================================================

static void ReadNet()
{
	vuint8		Buffer[MAX_MSGLEN];

	//	Check if there's any packet waiting.
	if (recvfrom(AcceptSocket, Buffer, MAX_MSGLEN, MSG_PEEK, NULL, NULL) < 0)
	{
		return;
	}

	//	Read packet.
	sockaddr clientaddr;
	socklen_t addrlen = sizeof(sockaddr);
	int len = recvfrom(AcceptSocket, Buffer, MAX_MSGLEN, 0, &clientaddr, &addrlen);
	if (len < 1)
	{
		return;
	}

	switch (Buffer[0])
	{
	case MCREQ_JOIN:
		{
			for (int i = 0; i < SrvList.Num(); i++)
			{
				if (!AddrCompare(&SrvList[i].Addr, &clientaddr))
				{
					SrvList[i].Time = time(0);
					return;
				}
			}
			TSrvItem& I = SrvList.Alloc();
			I.Addr = clientaddr;
			I.Time = time(0);
		}
		break;

	case MCREQ_QUIT:
		for (int i = 0; i < SrvList.Num(); i++)
		{
			if (!AddrCompare(&SrvList[i].Addr, &clientaddr))
			{
				SrvList.RemoveIndex(i);
				break;
			}
		}
		break;

	case MCREQ_LIST:
		{
			Buffer[0] = MCREP_LIST;
			int Len = 1;
			for (int i = 0; i < SrvList.Num() && i < (MAX_MSGLEN - 1) / 6; i++)
			{
				memcpy(&Buffer[Len], SrvList[i].Addr.sa_data + 2, 4);
				memcpy(&Buffer[Len + 4], SrvList[i].Addr.sa_data, 2);
				Len += 6;
			}
			sendto(AcceptSocket, Buffer, Len, 0, &clientaddr, sizeof(sockaddr));
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
	printf("Vavoom master server.\n");

#ifdef _WIN32
	WSADATA winsockdata;
	//MAKEWORD(2, 2)
	int r = WSAStartup(MAKEWORD(1, 1), &winsockdata);
	if (r)
	{
		printf("Winsock initialisation failed.\n");
		return -1;
	}
	TWinSockHelper Helper;
#endif

	//	Open socket for listening for requests.
	AcceptSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (AcceptSocket == -1)
	{
		printf("Unable to open accept socket\n");
		return -1;
	}

	//	Make socket non-blocking
	int trueval = true;
#ifdef _WIN32
	if (ioctlsocket(AcceptSocket, FIONBIO, &trueval) == -1)
#else
	if (ioctl(AcceptSocket, FIONBIO, (char*)&trueval) == -1)
#endif
	{
		closesocket(AcceptSocket);
		printf("Unable to make socket non-blocking\n");
		return -1;
	}

	//	Bind socket to the port.
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(MASTER_SERVER_PORT);
	if (bind(AcceptSocket, (sockaddr*)&address, sizeof(address)) == -1)
	{
		closesocket(AcceptSocket);
		printf("Unable to bind socket to a port\n");
		return -1;
	}

	//	Main loop.
	while (1)
	{
		for (int i = 0; i < 1000; i++)
		{
			ReadNet();
#ifdef _WIN32
			Sleep(1);
#else
			usleep(1);
#endif
		}

		//	Clean up list from old records.
		time_t CurTime = time(0);
		for (size_t i = 0; i < SrvList.Num(); i++)
		{
			if (CurTime - SrvList[i].Time > 15 * 60)
			{
				SrvList.RemoveIndex(i);
				i--;
			}
		}
	}

	//	Close socket.
	closesocket(AcceptSocket);
	return 0;
}
