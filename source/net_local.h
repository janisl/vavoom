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

#ifndef _NET_LOC_H
#define _NET_LOC_H

#include "network.h"

#define	NET_NAMELEN			64

#define	MAX_NET_DRIVERS		8

#define HOSTCACHESIZE		8

#define NET_DATAGRAMSIZE	MAX_MSGLEN

class VNetDriver;
class VNetLanDriver;

struct sockaddr_t
{
	vint16		sa_family;
	vint8		sa_data[14];
};

class VSocket : public VSocketPublic
{
public:
	VSocket*		Next;

	VNetDriver*		Driver;

	VSocket(VNetDriver*);
	~VSocket();
};

struct VNetPollProcedure
{
	VNetPollProcedure*	next;
	double				nextTime;
	void				(*procedure)(void*);
	void*				arg;

	VNetPollProcedure()
	: next(NULL)
	, nextTime(0.0)
	, procedure(NULL)
	, arg(NULL)
	{}
	VNetPollProcedure(void (*aProcedure)(void*), void* aArg)
	: next(NULL)
	, nextTime(0.0)
	, procedure(aProcedure)
	, arg(aArg)
	{}
};

class VNetworkLocal : public VNetworkPublic
{
public:
	VSocket*		ActiveSockets;

	int				HostCacheCount;
	hostcache_t		HostCache[HOSTCACHESIZE];

	int				HostPort;
	int				DefaultHostPort;

	char			MyIpxAddress[NET_NAMELEN];
	char			MyIpAddress[NET_NAMELEN];

	bool			IpxAvailable;
	bool			IpAvailable;

	char			ReturnReason[32];

	bool			Listening;

	static VNetDriver*		Drivers[MAX_NET_DRIVERS];
	static int				NumDrivers;

	static VNetLanDriver*	LanDrivers[MAX_NET_DRIVERS];
	static int				NumLanDrivers;

	static VCvarS			HostName;

	VNetworkLocal();
	virtual void SchedulePollProcedure(VNetPollProcedure*, double) = 0;

	virtual void Slist() = 0;
};

class VNetDriver : public VInterface
{
public:
	const char*		name;
	bool			initialised;
	VNetworkLocal*	Net;

	VNetDriver(int, const char*);
	virtual int Init() = 0;
	virtual void Listen(bool) = 0;
	virtual void SearchForHosts(bool, bool) = 0;
	virtual VSocket* Connect(const char*) = 0;
	virtual VSocket* CheckNewConnections() = 0;
	virtual void UpdateMaster() = 0;
	virtual void QuitMaster() = 0;
	virtual bool QueryMaster(bool) = 0;
	virtual void EndQueryMaster() = 0;
	virtual void Shutdown() = 0;
};

class VNetLanDriver : public VInterface
{
public:
	const char*		name;
	bool			initialised;
	int				controlSock;
	int				MasterQuerySocket;
	VNetworkLocal*	Net;

	int				net_acceptsocket;	// socket for fielding new connections
	int				net_controlsocket;
	int				net_broadcastsocket;
	sockaddr_t		broadcastaddr;

	vuint32			myAddr;

	VNetLanDriver(int, const char*);
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
	virtual VStr GetNameFromAddr(sockaddr_t*) = 0;
	virtual int GetAddrFromName(const char*, sockaddr_t*, int) = 0;
	virtual int AddrCompare(sockaddr_t*, sockaddr_t*) = 0;
	virtual int GetSocketPort(sockaddr_t* addr) = 0;
	virtual int SetSocketPort(sockaddr_t* addr, int port) = 0;
};

#endif
