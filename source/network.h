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

#define	MAX_MSGLEN			8000		// max length of a reliable message
#define	MAX_DATAGRAM		1024		// max length of unreliable message

#define	NET_NAMELEN			64
#define NET_MAXMESSAGE		8192

#define HOSTCACHESIZE		8

#define	MAX_NET_DRIVERS		8

class VNetDriver;
class VNetLanDriver;

struct sockaddr_t
{
	vint16		sa_family;
	vint8		sa_data[14];
};

class VSocket
{
public:
	VSocket*		Next;
	double			ConnectTime;
	double			LastMessageTime;
	double			LastSendTime;

	bool			Disconnected;
	bool			CanSend;
	bool			SendNext;
	
	VNetDriver*		Driver;
	VNetLanDriver*	LanDriver;
	int				LanSocket;
	void*			DriverData;

	vuint32			AckSequence;
	vuint32			SendSequence;
	vuint32			UnreliableSendSequence;
	int				SendMessageLength;
	vuint8			SendMessageData[NET_MAXMESSAGE];

	vuint32			ReceiveSequence;
	vuint32			UnreliableReceiveSequence;
	int				ReceiveMessageLength;
	vuint8			ReceiveMessageData[NET_MAXMESSAGE];

	sockaddr_t		Addr;
	VStr			Address;

	bool IsLocalConnection();
	int GetMessage();
	int SendMessage(VMessage*);
	int SendUnreliableMessage(VMessage*);
	bool CanSendMessage();
	void Close();
};

struct hostcache_t
{
	char		name[16];
	char		map[16];
	char		cname[32];
	char		wadfiles[20][16];
	int			users;
	int			maxusers;
};

//	Structure returned to progs.
struct slist_t
{
	enum
	{
		SF_InProgress	= 0x01,
	};
	vuint32		Flags;
	int			count;
	hostcache_t	cache[HOSTCACHESIZE];
	char		return_reason[32];
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

class VNetwork
{
public:
	//	Public API
	VMessage		NetMsg;

	bool			ConnectBot;

	VNetwork();
	~VNetwork();
	void Init();
	void Shutdown();
	VSocket* Connect(const char*);
	VSocket* CheckNewConnections();
	void Poll();
	void StartSearch();
	slist_t* GetSlist();

	//	API only for network drivers!
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

	VSocket* NewSocket(VNetDriver*);
	void FreeSocket(VSocket*);
	double SetNetTime();
	void SchedulePollProcedure(VNetPollProcedure*, double);

	void Slist();

private:
	VNetPollProcedure	SlistSendProcedure;
	VNetPollProcedure	SlistPollProcedure;

	bool				SlistInProgress;
	bool				SlistSilent;
	bool				SlistLocal;
	bool				SlistSorted;
	double				SlistStartTime;
	int					SlistLastShown;

	slist_t				slist;

	VNetPollProcedure*	PollProcedureList;

	static void Slist_Send(void*);
	static void Slist_Poll(void*);
	void Slist_Send();
	void Slist_Poll();
	void PrintSlistHeader();
	void PrintSlist();
	void PrintSlistTrailer();
};

extern VNetwork*	GNet;
