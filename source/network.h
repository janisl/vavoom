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

struct sockaddr_t
{
    short		sa_family;
    char		sa_data[14];
};

struct qsocket_t
{
	qsocket_t*	next;
	double		connecttime;
	double		lastMessageTime;
	double		lastSendTime;

	bool		disconnected;
	bool		canSend;
	bool		sendNext;
	
	int			driver;
	int			landriver;
	int			socket;
	void*		driverdata;

	dword		ackSequence;
	dword		sendSequence;
	dword		unreliableSendSequence;
	int			sendMessageLength;
	byte		sendMessage[NET_MAXMESSAGE];

	dword		receiveSequence;
	dword		unreliableReceiveSequence;
	int			receiveMessageLength;
	byte		receiveMessage[NET_MAXMESSAGE];

	sockaddr_t	addr;
	char		address[NET_NAMELEN];
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
qsocket_t* NET_Connect(const char* host);
qsocket_t* NET_CheckNewConnections();
int NET_GetMessage(qsocket_t* sock);
int NET_SendMessage(qsocket_t* sock, VMessage* data);
int NET_SendUnreliableMessage(qsocket_t* sock, VMessage* data);
bool NET_CanSendMessage(qsocket_t* sock);
void NET_Close(qsocket_t* sock);
void NET_Poll();

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern VMessage		net_msg;

//**************************************************************************
//
//	$Log$
//	Revision 1.7  2006/04/05 17:20:37  dj_jl
//	Merged size buffer with message class.
//
//	Revision 1.6  2005/08/29 19:29:37  dj_jl
//	Implemented network packet compression.
//	
//	Revision 1.5  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.4  2001/10/08 17:27:53  dj_jl
//	Moved slist menu builtins here
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
