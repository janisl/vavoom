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

#ifndef _NET_LOC_H
#define _NET_LOC_H

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

#define	MAX_NET_DRIVERS		8

#define HOSTCACHESIZE		8

#define NET_HEADERSIZE		(2 * sizeof(vuint32) + sizeof(vuint16))
#define NET_DATAGRAMSIZE	(MAX_DATAGRAM + NET_HEADERSIZE)

// TYPES -------------------------------------------------------------------

struct net_driver_t
{
	char*		name;
	bool		initialised;
	int			(*Init)();
	void		(*Listen)(bool state);
	void		(*SearchForHosts)(bool xmit);
	qsocket_t*	(*Connect)(const char* host);
	qsocket_t*	(*CheckNewConnections)();
	int			(*QGetMessage)(qsocket_t* sock);
	int			(*QSendMessage)(qsocket_t* sock, VMessage* data);
	int			(*SendUnreliableMessage)(qsocket_t* sock, VMessage* data);
	bool		(*CanSendMessage)(qsocket_t* sock);
	bool		(*CanSendUnreliableMessage)(qsocket_t* sock);
	void		(*Close)(qsocket_t* sock);
	void		(*Shutdown)();
};

class VNetLanDriver
{
public:
	const char*	name;
	bool		initialised;
	int			controlSock;

	VNetLanDriver(int, const char*);
	virtual ~VNetLanDriver();
	virtual int Init() = 0;
	virtual void Shutdown() = 0;
	virtual void Listen(bool) = 0;
	virtual int OpenSocket(int) = 0;
	virtual int CloseSocket(int) = 0;
	virtual int Connect(int, sockaddr_t*) = 0;
	virtual int CheckNewConnections() = 0;
	virtual int Read(int, vuint8*, int, sockaddr_t*) = 0;
	virtual int Write(int, vuint8*, int, sockaddr_t*) = 0;
	virtual int Broadcast(int, vuint8*, int) = 0;
	virtual char* AddrToString(sockaddr_t*) = 0;
	virtual int StringToAddr(const char*, sockaddr_t*) = 0;
	virtual int GetSocketAddr(int, sockaddr_t*) = 0;
	virtual int GetNameFromAddr(sockaddr_t*, char*) = 0;
	virtual int GetAddrFromName(const char*, sockaddr_t*) = 0;
	virtual int AddrCompare(sockaddr_t*, sockaddr_t*) = 0;
	virtual int GetSocketPort(sockaddr_t* addr) = 0;
	virtual int SetSocketPort(sockaddr_t* addr, int port) = 0;
};

struct PollProcedure
{
	PollProcedure*	next;
	double			nextTime;
	void			(*procedure)(void*);
	void*			arg;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

qsocket_t *NET_NewQSocket();
void NET_FreeQSocket(qsocket_t* sock);
double SetNetTime();
void SchedulePollProcedure(PollProcedure* pp, double timeOffset);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern net_driver_t		net_drivers[MAX_NET_DRIVERS];
extern int				net_numdrivers;

extern VNetLanDriver*	net_landrivers[MAX_NET_DRIVERS];
extern int				net_numlandrivers;

extern int				net_driverlevel;
extern double			net_time;

extern qsocket_t*		net_activeSockets;
extern qsocket_t*		net_freeSockets;

extern bool				slistInProgress;
extern bool				slistSilent;
extern bool				slistLocal;

extern int				hostCacheCount;
extern hostcache_t		hostcache[HOSTCACHESIZE];

extern int				net_hostport;
extern VCvarS			hostname;

extern char				my_ipx_address[NET_NAMELEN];
extern char				my_tcpip_address[NET_NAMELEN];

extern bool				serialAvailable;
extern bool				ipxAvailable;
extern bool				tcpipAvailable;

extern int				messagesSent;
extern int				messagesReceived;
extern int				unreliableMessagesSent;
extern int				unreliableMessagesReceived;

#endif
