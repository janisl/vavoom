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

#define	NET_NAMELEN			64

#define	MAX_NET_DRIVERS		8

#define HOSTCACHESIZE		8

#define NET_HEADERSIZE		(2 * sizeof(vuint32) + sizeof(vuint16))
#define NET_DATAGRAMSIZE	(MAX_MSGLEN + NET_HEADERSIZE)

class VNetDriver;
class VNetLanDriver;

struct sockaddr_t
{
	vint16		sa_family;
	vint8		sa_data[14];
};

struct VLoopbackMessage
{
	TArray<vuint8>	Data;
	int				MessageType;
};

class VSocket : public VSocketPublic
{
public:
	VSocket*		Next;
	double			ConnectTime;
	double			LastMessageTime;
	double			LastSendTime;

	bool			Disconnected;
	bool			CanSend;
	
	VNetDriver*		Driver;
	VNetLanDriver*	LanDriver;
	int				LanSocket;
	void*			DriverData;

	vuint32			AckSequence;
	vuint32			SendSequence;
	vuint32			UnreliableSendSequence;
	int				SendMessageLength;
	vuint8			SendMessageData[MAX_MSGLEN];

	vuint32			ReceiveSequence;
	vuint32			UnreliableReceiveSequence;

	TArray<VLoopbackMessage>	LoopbackMessages;

	sockaddr_t		Addr;

	bool IsLocalConnection();
	int GetMessage(TArray<vuint8>&);
	int SendMessage(vuint8*, vuint32);
	int SendUnreliableMessage(vuint8*, vuint32);
	bool CanSendMessage();
	void Close();
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
	double			NetTime;
	
	VSocket*		ActiveSockets;
	VSocket*		FreeSockets;

	int				HostCacheCount;
	hostcache_t		HostCache[HOSTCACHESIZE];

	int				HostPort;
	int				DefaultHostPort;

	char			MyIpxAddress[NET_NAMELEN];
	char			MyIpAddress[NET_NAMELEN];

	bool			IpxAvailable;
	bool			IpAvailable;

	int				MessagesSent;
	int				MessagesReceived;
	int				UnreliableMessagesSent;
	int				UnreliableMessagesReceived;

	char			ReturnReason[32];

	bool			Listening;

	static VNetDriver*		Drivers[MAX_NET_DRIVERS];
	static int				NumDrivers;

	static VNetLanDriver*	LanDrivers[MAX_NET_DRIVERS];
	static int				NumLanDrivers;

	static VCvarS			HostName;
	static VCvarF			MessageTimeOut;

	VNetworkLocal();
	virtual VSocket* NewSocket(VNetDriver*) = 0;
	virtual void FreeSocket(VSocket*) = 0;
	virtual double SetNetTime() = 0;
	virtual void SchedulePollProcedure(VNetPollProcedure*, double) = 0;

	virtual void Slist() = 0;
};

class VNetDriver : public VVirtualObjectBase
{
public:
	const char*		name;
	bool			initialised;
	VNetworkLocal*	Net;

	VNetDriver(int, const char*);
	virtual int Init() = 0;
	virtual void Listen(bool) = 0;
	virtual void SearchForHosts(bool) = 0;
	virtual VSocket* Connect(const char*) = 0;
	virtual VSocket* CheckNewConnections() = 0;
	virtual int GetMessage(VSocket*, TArray<vuint8>&) = 0;
	virtual int SendMessage(VSocket*, vuint8*, vuint32) = 0;
	virtual int SendUnreliableMessage(VSocket*, vuint8*, vuint32) = 0;
	virtual bool CanSendMessage(VSocket*) = 0;
	virtual bool CanSendUnreliableMessage(VSocket*) = 0;
	virtual void Close(VSocket*) = 0;
	virtual void Shutdown() = 0;
};

class VNetLanDriver : public VVirtualObjectBase
{
public:
	const char*		name;
	bool			initialised;
	int				controlSock;
	VNetworkLocal*	Net;

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
	virtual int GetAddrFromName(const char*, sockaddr_t*) = 0;
	virtual int AddrCompare(sockaddr_t*, sockaddr_t*) = 0;
	virtual int GetSocketPort(sockaddr_t* addr) = 0;
	virtual int SetSocketPort(sockaddr_t* addr, int port) = 0;
};

#endif
