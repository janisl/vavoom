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

// MACROS ------------------------------------------------------------------

#define	MAX_MSGLEN			8000		// max length of a reliable message
#define	MAX_DATAGRAM		1024		// max length of unreliable message

#define	NET_NAMELEN			64
#define NET_MAXMESSAGE		8192

// TYPES -------------------------------------------------------------------

class VNetDriver;
class VNetLanDriver;

struct sockaddr_t
{
	vint16		sa_family;
	vint8		sa_data[14];
};

struct qsocket_t
{
	qsocket_t*		next;
	double			connecttime;
	double			lastMessageTime;
	double			lastSendTime;

	bool			disconnected;
	bool			canSend;
	bool			sendNext;
	
	VNetDriver*		driver;
	VNetLanDriver*	landriver;
	int				socket;
	void*			driverdata;

	vuint32			ackSequence;
	vuint32			sendSequence;
	vuint32			unreliableSendSequence;
	int				sendMessageLength;
	vuint8			sendMessage[NET_MAXMESSAGE];

	vuint32			receiveSequence;
	vuint32			unreliableReceiveSequence;
	int				receiveMessageLength;
	vuint8			receiveMessage[NET_MAXMESSAGE];

	sockaddr_t		addr;
	char			address[NET_NAMELEN];
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

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void NET_Init();
void NET_Shutdown();
void NET_Slist();
qsocket_t* NET_Connect(const char*);
qsocket_t* NET_CheckNewConnections();
bool NET_IsLocalConnection(qsocket_t*);
int NET_GetMessage(qsocket_t*);
int NET_SendMessage(qsocket_t*, VMessage*);
int NET_SendUnreliableMessage(qsocket_t*, VMessage*);
bool NET_CanSendMessage(qsocket_t*);
void NET_Close(qsocket_t*);
void NET_Poll();

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern VMessage		net_msg;
extern bool			net_connect_bot;
