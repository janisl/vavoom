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

// MACROS ------------------------------------------------------------------

#define	MAX_MSGLEN			8000		// max length of a reliable message
#define	MAX_DATAGRAM		1024		// max length of unreliable message

#define	NET_NAMELEN			64
#define NET_MAXMESSAGE		8192
#define NET_HEADERSIZE		(2 * sizeof(unsigned int))
#define NET_DATAGRAMSIZE	(MAX_DATAGRAM + NET_HEADERSIZE)

// TYPES -------------------------------------------------------------------

struct sockaddr_t
{
    short		sa_family;
    char		sa_data[14];
};

struct qsocket_t
{
	qsocket_t	*next;
	double		connecttime;
	double		lastMessageTime;
	double		lastSendTime;

	boolean		disconnected;
	boolean		canSend;
	boolean		sendNext;
	
	int			driver;
	int			landriver;
	int			socket;
	void		*driverdata;

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

void NET_Init(void);
void NET_Shutdown(void);
void NET_Slist(void);
qsocket_t *NET_Connect(char *host);
qsocket_t *NET_CheckNewConnections(void);
int NET_GetMessage(qsocket_t *sock);
int NET_SendMessage(qsocket_t *sock, TSizeBuf *data);
int NET_SendUnreliableMessage(qsocket_t *sock, TSizeBuf *data);
boolean NET_CanSendMessage(qsocket_t *sock);
void NET_Close(qsocket_t *sock);
void NET_Poll(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern TMessage		net_msg;

//**************************************************************************
//
//	$Log$
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
