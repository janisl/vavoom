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

#define NET_HEADERSIZE		(2 * sizeof(unsigned int) + sizeof(unsigned short))
#define NET_DATAGRAMSIZE	(MAX_DATAGRAM + NET_HEADERSIZE)

// TYPES -------------------------------------------------------------------

struct net_driver_t
{
	char		*name;
	boolean		initialized;
	int			(*Init)(void);
	void		(*Listen)(boolean state);
	void		(*SearchForHosts)(boolean xmit);
	qsocket_t	*(*Connect)(char *host);
	qsocket_t 	*(*CheckNewConnections)(void);
	int			(*QGetMessage)(qsocket_t *sock);
	int			(*QSendMessage)(qsocket_t *sock, TSizeBuf *data);
	int			(*SendUnreliableMessage)(qsocket_t *sock, TSizeBuf *data);
	boolean		(*CanSendMessage)(qsocket_t *sock);
	boolean		(*CanSendUnreliableMessage)(qsocket_t *sock);
	void		(*Close)(qsocket_t *sock);
	void		(*Shutdown)(void);
//	int			controlSock;
};

struct net_landriver_t
{
	char		*name;
	boolean		initialized;
	int			controlSock;
	int			(*Init) (void);
	void		(*Shutdown) (void);
	void		(*Listen) (boolean state);
	int 		(*OpenSocket) (int port);
	int 		(*CloseSocket) (int socket);
	int 		(*Connect) (int socket, sockaddr_t *addr);
	int 		(*CheckNewConnections) (void);
	int 		(*Read) (int socket, byte *buf, int len, sockaddr_t *addr);
	int 		(*Write) (int socket, byte *buf, int len, sockaddr_t *addr);
	int 		(*Broadcast) (int socket, byte *buf, int len);
	char *		(*AddrToString) (sockaddr_t *addr);
	int 		(*StringToAddr) (char *string, sockaddr_t *addr);
	int 		(*GetSocketAddr) (int socket, sockaddr_t *addr);
	int 		(*GetNameFromAddr) (sockaddr_t *addr, char *name);
	int 		(*GetAddrFromName) (char *name, sockaddr_t *addr);
	int			(*AddrCompare) (sockaddr_t *addr1, sockaddr_t *addr2);
	int			(*GetSocketPort) (sockaddr_t *addr);
	int			(*SetSocketPort) (sockaddr_t *addr, int port);
};

struct PollProcedure
{
	PollProcedure	*next;
	double			nextTime;
	void			(*procedure)(void *);
	void			*arg;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

qsocket_t *NET_NewQSocket(void);
void NET_FreeQSocket(qsocket_t *sock);
double SetNetTime(void);
void SchedulePollProcedure(PollProcedure *pp, double timeOffset);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern net_driver_t		net_drivers[MAX_NET_DRIVERS];
extern int				net_numdrivers;

extern net_landriver_t	net_landrivers[MAX_NET_DRIVERS];
extern int				net_numlandrivers;

extern int				net_driverlevel;
extern double			net_time;

extern qsocket_t		*net_activeSockets;
extern qsocket_t		*net_freeSockets;

extern boolean			slistInProgress;
extern boolean			slistSilent;
extern boolean			slistLocal;

extern int				hostCacheCount;
extern hostcache_t		hostcache[HOSTCACHESIZE];

extern int				net_hostport;
extern TCvarS			hostname;

extern char				my_ipx_address[NET_NAMELEN];
extern char				my_tcpip_address[NET_NAMELEN];

extern boolean			serialAvailable;
extern boolean			ipxAvailable;
extern boolean			tcpipAvailable;

extern int				messagesSent;
extern int				messagesReceived;
extern int				unreliableMessagesSent;
extern int				unreliableMessagesReceived;

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2005/08/29 19:29:37  dj_jl
//	Implemented network packet compression.
//
//	Revision 1.5  2002/01/07 12:16:42  dj_jl
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
