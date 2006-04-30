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

#include "gamedefs.h"
#include "net_loc.h"

// MACROS ------------------------------------------------------------------

// these two macros are to make the code more readable
#define sfunc	net_drivers[sock->driver]
#define dfunc	net_drivers[net_driverlevel]

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

#ifdef CLIENT
static void Slist_Send(void*);
static void Slist_Poll(void*);
#endif

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern int		num_connected;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int				net_hostport;
int				DEFAULTnet_hostport = 26000;

char    		my_ipx_address[NET_NAMELEN];
char    		my_tcpip_address[NET_NAMELEN];

bool			serialAvailable;
bool			ipxAvailable;
bool			tcpipAvailable;

int				net_driverlevel;

double			net_time;

VMessage		net_msg;

int				hostCacheCount;
hostcache_t		hostcache[HOSTCACHESIZE];

qsocket_t*		net_activeSockets = NULL;
qsocket_t*		net_freeSockets = NULL;

VCvarS			hostname("hostname", "UNNAMED");
VCvarF			net_messagetimeout("net_messagetimeout", "300");

int messagesSent = 0;
int messagesReceived = 0;
int unreliableMessagesSent = 0;
int unreliableMessagesReceived = 0;

#ifdef CLIENT
bool			slistInProgress = false;
bool			slistSilent = false;
bool			slistLocal = true;
bool			slistSorted = true;
static double	slistStartTime;
static int		slistLastShown;

PollProcedure	slistSendProcedure = {NULL, 0.0, Slist_Send, NULL};
PollProcedure	slistPollProcedure = {NULL, 0.0, Slist_Poll, NULL};
#endif

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static PollProcedure*	pollProcedureList = NULL;

static boolean			listening = false;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	NET_Init
//
//==========================================================================

void NET_Init()
{
	guard(NET_Init);
	int			i;
	qsocket_t*	s;

	const char* p = GArgs.CheckValue("-port");
	if (p)
	{
		DEFAULTnet_hostport = atoi(p);
	}
	net_hostport = DEFAULTnet_hostport;

#ifdef CLIENT
/*	if (GArgs.CheckParm("-listen") || cls.state == ca_dedicated)
		listening = true;
	net_numsockets = svs.maxclientslimit;
	if (cls.state != ca_dedicated)
		net_numsockets++;
*/
#else
	listening = true;
#endif
	SetNetTime();

/*	for (i = 0; i < net_numsockets; i++)*/
	for (i = 0; i < MAXPLAYERS + 1; i++)
	{
		s = (qsocket_t *)Z_Malloc(sizeof(qsocket_t), PU_STATIC, 0);
		s->next = net_freeSockets;
		net_freeSockets = s;
		s->disconnected = true;
	}

	// allocate space for network message buffer
	net_msg.Alloc(NET_MAXMESSAGE);

	// initialize all the drivers
	for (net_driverlevel = 0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].Init() != -1)
		{
			net_drivers[net_driverlevel].initialised = true;
			if (listening)
				net_drivers[net_driverlevel].Listen(true);
		}
	}

	if (*my_ipx_address)
		GCon->Logf(NAME_DevNet, "IPX address %s", my_ipx_address);
	if (*my_tcpip_address)
		GCon->Logf(NAME_DevNet, "TCP/IP address %s", my_tcpip_address);
	unguard;
}

//==========================================================================
//
//	NET_Shutdown
//
//==========================================================================

void NET_Shutdown()
{
	guard(NET_Shutdown);
	qsocket_t	*sock;

	SetNetTime();

	for (sock = net_activeSockets; sock; sock = sock->next)
		NET_Close(sock);

	//
	// shutdown the drivers
	//
	for (net_driverlevel = 0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialised)
		{
			net_drivers[net_driverlevel].Shutdown();
			net_drivers[net_driverlevel].initialised = false;
		}
	}
	unguard;
}

//==========================================================================
//
//	SetNetTime
//
//==========================================================================

double SetNetTime()
{
	guard(SetNetTime);
	net_time = Sys_Time();
	return net_time;
	unguard;
}

//==========================================================================
//
//	NET_Poll
//
//==========================================================================

void NET_Poll()
{
	guard(NET_Poll);
	SetNetTime();

	for (PollProcedure *pp = pollProcedureList; pp; pp = pp->next)
	{
		if (pp->nextTime > net_time)
			break;
		pollProcedureList = pp->next;
		pp->procedure(pp->arg);
	}
	unguard;
}

//==========================================================================
//
//	SchedulePollProcedure
//
//==========================================================================

void SchedulePollProcedure(PollProcedure* proc, double timeOffset)
{
	guard(SchedulePollProcedure);
	PollProcedure *pp, *prev;

	proc->nextTime = Sys_Time() + timeOffset;
	for (pp = pollProcedureList, prev = NULL; pp; pp = pp->next)
	{
		if (pp->nextTime >= proc->nextTime)
			break;
		prev = pp;
	}

	if (prev == NULL)
	{
		proc->next = pollProcedureList;
		pollProcedureList = proc;
	}
	else
	{
		proc->next = pp;
		prev->next = proc;
	}
	unguard;
}

//==========================================================================
//
//	NET_NewQSocket
//
//	Called by drivers when a new communications endpoint is required
//	The sequence and buffer fields will be filled in properly
//
//==========================================================================

qsocket_t* NET_NewQSocket()
{
	guard(NET_NewQSocket);
	qsocket_t*	sock;

	if (net_freeSockets == NULL)
		return NULL;

#ifdef SERVER
	if (svs.num_connected >= svs.max_clients)
		return NULL;
#endif

	// get one from free list
	sock = net_freeSockets;
	net_freeSockets = sock->next;

	// add it to active list
	sock->next = net_activeSockets;
	net_activeSockets = sock;

	sock->disconnected = false;
	sock->connecttime = net_time;
	strcpy(sock->address, "UNSET ADDRESS");
	sock->driver = net_driverlevel;
	sock->socket = 0;
	sock->driverdata = NULL;
	sock->canSend = true;
	sock->sendNext = false;
	sock->lastMessageTime = net_time;
	sock->ackSequence = 0;
	sock->sendSequence = 0;
	sock->unreliableSendSequence = 0;
	sock->sendMessageLength = 0;
	sock->receiveSequence = 0;
	sock->unreliableReceiveSequence = 0;
	sock->receiveMessageLength = 0;

	return sock;
	unguard;
}

//==========================================================================
//
//	NET_FreeQSocket
//
//==========================================================================

void NET_FreeQSocket(qsocket_t* sock)
{
	guard(NET_FreeQSocket);
	qsocket_t	*s;

	// remove it from active list
	if (sock == net_activeSockets)
	{
		net_activeSockets = net_activeSockets->next;
	}
	else
	{
		for (s = net_activeSockets; s; s = s->next)
		{
			if (s->next == sock)
			{
				s->next = sock->next;
				break;
			}
		}
		if (!s)
		{
			Sys_Error("NET_FreeQSocket: not active\n");
		}
	}

	// add it to free list
	sock->next = net_freeSockets;
	net_freeSockets = sock;
	sock->disconnected = true;
	unguard;
}

#if defined CLIENT && defined SERVER // I think like this

//==========================================================================
//
//	COMMAND Listen
//
//==========================================================================

COMMAND(Listen)
{
	guard(COMMAND Listen);
	if (Args.Num() != 2)
	{
		GCon->Logf("\"listen\" is \"%d\"", listening ? 1 : 0);
		return;
	}

	listening = atoi(*Args[1]) ? true : false;

	for (net_driverlevel=0 ; net_driverlevel<net_numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialised == false)
			continue;
		dfunc.Listen(listening);
	}
	unguard;
}

#endif

//==========================================================================
//
//	COMMAND Port
//
//==========================================================================

COMMAND(Port)
{
	guard(COMMAND Port);
	int 	n;

	if (Args.Num() != 2)
	{
		GCon->Logf("\"port\" is \"%d\"", net_hostport);
		return;
	}

	n = atoi(*Args[1]);
	if (n < 1 || n > 65534)
	{
		GCon->Log("Bad value, must be between 1 and 65534");
		return;
	}

	DEFAULTnet_hostport = n;
	net_hostport = n;

	if (listening)
	{
		// force a change to the new port
		GCmdBuf << "listen 0\n";
		GCmdBuf << "listen 1\n";
	}
	unguard;
}

#ifdef CLIENT

//==========================================================================
//
//	PrintSlistHeader
//
//==========================================================================

static void PrintSlistHeader()
{
	GCon->Log("Server          Map             Users");
	GCon->Log("--------------- --------------- -----");
	slistLastShown = 0;
}

//==========================================================================
//
//	PrintSlist
//
//==========================================================================

static void PrintSlist()
{
	int n;

	for (n = slistLastShown; n < hostCacheCount; n++)
	{
		if (hostcache[n].maxusers)
			GCon->Logf("%-15s %-15s %2d/%2d", hostcache[n].name, 
				hostcache[n].map, hostcache[n].users, hostcache[n].maxusers);
		else
			GCon->Logf("%-15s %-15s", hostcache[n].name, hostcache[n].map);
	}
	slistLastShown = n;
}

//==========================================================================
//
//	PrintSlistTrailer
//
//==========================================================================

static void PrintSlistTrailer()
{
	if (hostCacheCount)
		GCon->Log("== end list ==");
	else
		GCon->Log("No Vavoom servers found.");
	GCon->Log("");
}

//==========================================================================
//
//	Slist_Send
//
//==========================================================================

static void Slist_Send(void*)
{
	guard(Slist_Send);
	for (net_driverlevel = 0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (!slistLocal && net_driverlevel == 0)
			continue;
		if (net_drivers[net_driverlevel].initialised == false)
			continue;
		dfunc.SearchForHosts(true);
	}

	if ((Sys_Time() - slistStartTime) < 0.5)
		SchedulePollProcedure(&slistSendProcedure, 0.75);
	unguard;
}

//==========================================================================
//
//	Slist_Poll
//
//==========================================================================

static void Slist_Poll(void*)
{
	guard(Slist_Poll);
	for (net_driverlevel=0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (!slistLocal && net_driverlevel == 0)
			continue;
		if (net_drivers[net_driverlevel].initialised == false)
			continue;
		dfunc.SearchForHosts (false);
	}

	if (! slistSilent)
		PrintSlist();

	if ((Sys_Time() - slistStartTime) < 1.5)
	{
		SchedulePollProcedure(&slistPollProcedure, 0.1);
		return;
	}

	if (!slistSilent)
		PrintSlistTrailer();
	slistInProgress = false;
	slistSilent = false;
	slistLocal = true;
	slistSorted = false;
	unguard;
}

//==========================================================================
//
//	NET_Slist
//
//==========================================================================

void NET_Slist()
{
	guard(NET_Slist);
	if (slistInProgress)
		return;

	if (!slistSilent)
	{
		GCon->Log("Looking for Vavoom servers...");
		PrintSlistHeader();
	}

	slistInProgress = true;
	slistStartTime = Sys_Time();

	SchedulePollProcedure(&slistSendProcedure, 0.0);
	SchedulePollProcedure(&slistPollProcedure, 0.1);

	hostCacheCount = 0;
	unguard;
}

//==========================================================================
//
//	COMMAND Slist
//
//==========================================================================

COMMAND(Slist)
{
	guard(COMMAND Slist);
	NET_Slist();
	unguard;
}

//==========================================================================
//
//	NET_Connect
//
//==========================================================================

qsocket_t* NET_Connect(const char* InHost)
{
	guard(NET_Connect);
	const char* host = InHost;
	qsocket_t	*ret;
	int			numdrivers = net_numdrivers;
	int			n;

	SetNetTime();

	if (host && *host == 0)
		host = NULL;

	if (host)
	{
		if (stricmp(host, "local") == 0)
		{
			numdrivers = 1;
			goto JustDoIt;
		}

		if (hostCacheCount)
		{
			for (n = 0; n < hostCacheCount; n++)
			{
				if (stricmp(host, hostcache[n].name) == 0)
				{
					host = hostcache[n].cname;
					break;
				}
			}
			if (n < hostCacheCount)
				goto JustDoIt;
		}
	}

	slistSilent = host ? true : false;
	NET_Slist();

	while (slistInProgress)
		NET_Poll();

	if (host == NULL)
	{
		if (hostCacheCount != 1)
			return NULL;
		host = hostcache[0].cname;
		GCon->Log("Connecting to...");
		GCon->Logf("%s @ %s", hostcache[0].name, host);
		GCon->Log("");
	}

	if (hostCacheCount)
	{
		for (n = 0; n < hostCacheCount; n++)
		{
			if (stricmp(host, hostcache[n].name) == 0)
			{
				host = hostcache[n].cname;
				break;
			}
		}
	}

JustDoIt:
	for (net_driverlevel = 0; net_driverlevel < numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialised == false)
			continue;
		ret = dfunc.Connect(host);
		if (ret)
		{
			return ret;
		}
	}

	if (host)
	{
		PrintSlistHeader();
		PrintSlist();
		PrintSlistTrailer();
	}
	
	return NULL;
	unguard;
}

#endif
#ifdef SERVER

//==========================================================================
//
//	NET_CheckNewConnections
//
//==========================================================================

qsocket_t* NET_CheckNewConnections()
{
	guard(NET_CheckNewConnections);
	qsocket_t	*ret;

	SetNetTime();

	for (net_driverlevel = 0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialised == false)
			continue;
		if (net_driverlevel && listening == false)
			continue;
		ret = dfunc.CheckNewConnections();
		if (ret)
		{
			return ret;
		}
	}
	
	return NULL;
	unguard;
}

#endif

//==========================================================================
//
//	NET_Close
//
//==========================================================================

void NET_Close(qsocket_t* sock)
{
	guard(NET_Close);
	if (!sock)
		return;

	if (sock->disconnected)
		return;

	SetNetTime();

	// call the driver_Close function
	sfunc.Close(sock);

	NET_FreeQSocket(sock);
	unguard;
}

//==========================================================================
//
//	NET_GetMessage
//
//	If there is a complete message, return it in net_message
//
//	returns 0 if no data is waiting
//	returns 1 if a reliable message was received
//	returns 2 if a unreliable message was received
//	returns -1 if connection is invalid
//
//==========================================================================

int	NET_GetMessage(qsocket_t* sock)
{
	guard(NET_GetMessage);
	int			ret;

	if (!sock)
		return -1;

	if (sock->disconnected)
	{
		GCon->Log(NAME_DevNet, "NET_GetMessage: disconnected socket");
		return -1;
	}

	SetNetTime();

	ret = sfunc.QGetMessage(sock);

	// see if this connection has timed out
	if (ret == 0 && sock->driver > 1)
	{
		if (net_time - sock->lastMessageTime > net_messagetimeout)
		{
			NET_Close(sock);
			return -1;
		}
	}

	if (ret > 0)
	{
		if (sock->driver)
		{
			sock->lastMessageTime = net_time;
			if (ret == 1)
				messagesReceived++;
			else if (ret == 2)
				unreliableMessagesReceived++;
		}
	}

	return ret;
	unguard;
}

//==========================================================================
//
//	NET_SendMessage
//
//	Try to send a complete length+message unit over the reliable stream.
//	returns 0 if the message cannot be delivered reliably, but the connection
// is still considered valid
//	returns 1 if the message was sent properly
//	returns -1 if the connection died
//
//==========================================================================

int NET_SendMessage(qsocket_t* sock, VMessage* data)
{
	guard(NET_SendMessage);
	int		r;
	
	if (!sock)
		return -1;

	if (sock->disconnected)
	{
		GCon->Log(NAME_DevNet, "NET_SendMessage: disconnected socket");
		return -1;
	}

	SetNetTime();
	r = sfunc.QSendMessage(sock, data);
	if (r == 1 && sock->driver)
		messagesSent++;

	return r;
	unguard;
}

//==========================================================================
//
//	NET_SendUnreliableMessage
//
//==========================================================================

int NET_SendUnreliableMessage(qsocket_t* sock, VMessage* data)
{
	guard(NET_SendUnreliableMessage);
	int		r;
	
	if (!sock)
		return -1;

	if (sock->disconnected)
	{
		GCon->Log(NAME_DevNet, "NET_SendMessage: disconnected socket");
		return -1;
	}

	SetNetTime();
	r = sfunc.SendUnreliableMessage(sock, data);
	if (r == 1 && sock->driver)
		unreliableMessagesSent++;

	return r;
	unguard;
}

//==========================================================================
//
//	NET_CanSendMessage
//
//	Returns true or false if the given qsocket can currently accept a
// message to be transmitted.
//
//==========================================================================

bool NET_CanSendMessage(qsocket_t* sock)
{
	guard(NET_CanSendMessage);
	int		r;
	
	if (!sock)
		return false;

	if (sock->disconnected)
		return false;

	SetNetTime();

	r = sfunc.CanSendMessage(sock);
	
	return r;
	unguard;
}

//==========================================================================
//
//	Server list menu
//
//==========================================================================

#ifdef CLIENT

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

slist_t slist;

char			m_return_reason[32];

void StartSearch()
{
	slistSilent = true;
	slistLocal = false;
	NET_Slist();
}

slist_t* GetSlist()
{
	guard(GetSlist);
	int		i, j;

	if (!slistSorted)
	{
		if (hostCacheCount > 1)
		{
			hostcache_t temp;
			for (i = 0; i < hostCacheCount; i++)
				for (j = i + 1; j < hostCacheCount; j++)
					if (strcmp(hostcache[j].name, hostcache[i].name) < 0)
					{
						memcpy(&temp, &hostcache[j], sizeof(hostcache_t));
						memcpy(&hostcache[j], &hostcache[i], sizeof(hostcache_t));
						memcpy(&hostcache[i], &temp, sizeof(hostcache_t));
					}
		}
		slistSorted = true;
		memset(m_return_reason, 0, sizeof(m_return_reason));
	}

	if (slistInProgress)
		slist.Flags |= slist_t::SF_InProgress;
	else
		slist.Flags &= ~slist_t::SF_InProgress;
	slist.count = hostCacheCount;
	memcpy(slist.cache, hostcache, sizeof(hostcache));
	strcpy(slist.return_reason, m_return_reason);
	return &slist;
	unguard;
}

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.15  2006/04/05 17:20:37  dj_jl
//	Merged size buffer with message class.
//
//	Revision 1.14  2006/03/12 12:54:48  dj_jl
//	Removed use of bitfields for portability reasons.
//	
//	Revision 1.13  2005/04/28 07:16:15  dj_jl
//	Fixed some warnings, other minor fixes.
//	
//	Revision 1.12  2002/09/07 16:31:51  dj_jl
//	Added Level class.
//	
//	Revision 1.11  2002/08/05 17:20:00  dj_jl
//	Added guarding.
//	
//	Revision 1.10  2002/06/14 15:41:35  dj_jl
//	Got rid of a warning.
//	
//	Revision 1.9  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.8  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.7  2001/12/01 17:40:41  dj_jl
//	Added support for bots
//	
//	Revision 1.6  2001/10/09 17:25:02  dj_jl
//	Finished slist moving stuff
//	
//	Revision 1.5  2001/10/08 17:27:53  dj_jl
//	Moved slist menu builtins here
//	
//	Revision 1.4  2001/10/04 17:23:29  dj_jl
//	Got rid of some warnings
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
