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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************

#ifndef _NET_LOC_H
#define _NET_LOC_H

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

#define	MAX_NET_DRIVERS		8

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

extern int				messagesSent;
extern int				messagesReceived;
extern int				unreliableMessagesSent;
extern int				unreliableMessagesReceived;

#endif
