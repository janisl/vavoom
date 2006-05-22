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
//**
//**	Datagram driver, handles all LAN drivers
//**
//**************************************************************************
//
//	This is the network info/connection protocol. It is used to find Vavoom
// servers, get info about them, and connect to them. Once connected, the
// Vavoom game protocol (documented elsewhere) is used.
//
//
// CCREQ_CONNECT
//		string		"VAVOOM"
//		byte		net_protocol_version	NET_PROTOCOL_VERSION
//
// CCREQ_SERVER_INFO
//		string		"VAVOOM"
//		byte		net_protocol_version	NET_PROTOCOL_VERSION
//
//
//
// CCREP_ACCEPT
//		long		port
//
// CCREP_REJECT
//		string		reason
//
// CCREP_SERVER_INFO
//		string		host_name
//		string		level_name
//		byte		current_players
//		byte		max_players
//		byte		protocol_version		NET_PROTOCOL_VERSION
//		string[]	wad_files				empty string terminated
//
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "net_loc.h"
#include "zlib.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VDatagramDriver : public VNetDriver
{
public:
	// I don't think that communication protocol will change, but just in a
	// case
	enum { NET_PROTOCOL_VERSION = 1 };

	enum { IPPORT_USERRESERVED = 26000 };

	//	NetHeader flags
	enum
	{
		NETFLAG_COMPR_LEN_MASK	= 0x000007ff,
		NETFLAG_COMPR_MODE_MASK	= 0x0000f800,
		NETFLAG_LENGTH_MASK		= 0x07ff0000,
		NETFLAG_FLAGS_MASK		= 0xf8000000,
		NETFLAG_COMPR_NONE		= 0x00000000,
		NETFLAG_COMPR_ZIP		= 0x00000800,
		NETFLAG_EOM				= 0x08000000,
		NETFLAG_ACK				= 0x10000000,
		NETFLAG_DATA			= 0x20000000,
		NETFLAG_UNRELIABLE		= 0x40000000,
		NETFLAG_CTL				= 0x80000000
	};

	//	Client request
	enum
	{
		CCREQ_CONNECT			= 1,
		CCREQ_SERVER_INFO		= 2
	};

	//	Server reply
	enum
	{
		CCREP_ACCEPT			= 11,
		CCREP_REJECT			= 12,
		CCREP_SERVER_INFO		= 13
	};

	struct
	{
		vuint32		length;
		vuint32		sequence;
		vuint16		crc;
		vuint8		data[MAX_DATAGRAM];
	} packetBuffer;

	//	Statistic counters.
	static int		packetsSent;
	static int		packetsReSent;
	static int		packetsReceived;
	static int		receivedDuplicateCount;
	static int		shortPacketCount;
	static int		droppedDatagrams;

	VDatagramDriver();
	int Init();
	void Listen(bool);
	void SearchForHosts(bool);
	qsocket_t* Connect(const char*);
	qsocket_t* CheckNewConnections();
	int GetMessage(qsocket_t*);
	int SendMessage(qsocket_t*, VMessage*);
	int SendUnreliableMessage(qsocket_t*, VMessage*);
	bool CanSendMessage(qsocket_t*);
	bool CanSendUnreliableMessage(qsocket_t*);
	void Close(qsocket_t*);
	void Shutdown();

	static vuint16 NetbufferChecksum(const vuint8*, int);
	void SearchForHosts(VNetLanDriver*, bool);
	qsocket_t* Connect(VNetLanDriver*, const char*);
	qsocket_t* CheckNewConnections(VNetLanDriver*);
	int BuildNetPacket(vuint32, vuint32, vuint8*, vuint32);
	int SendMessageNext(qsocket_t*);
	int ReSendMessage(qsocket_t*);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

#ifdef CLIENT
extern char			m_return_reason[32];
#endif

extern int			num_connected;
extern TArray<VStr>	wadfiles;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VDatagramDriver	Impl;

//	Statistic counters.
int		VDatagramDriver::packetsSent = 0;
int		VDatagramDriver::packetsReSent = 0;
int		VDatagramDriver::packetsReceived = 0;
int		VDatagramDriver::receivedDuplicateCount = 0;
int		VDatagramDriver::shortPacketCount = 0;
int		VDatagramDriver::droppedDatagrams;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VDatagramDriver::VDatagramDriver
//
//==========================================================================

VDatagramDriver::VDatagramDriver()
: VNetDriver(2, "Datagram")
{
	memset(&packetBuffer, 0, sizeof(packetBuffer));
}

//==========================================================================
//
//  VDatagramDriver::NetbufferChecksum
//
//==========================================================================

vuint16 VDatagramDriver::NetbufferChecksum(const vuint8* buf, int len)
{
	guardSlow(NetbufferChecksum);
	TCRC	crc;

	crc.Init();

	for (int i = 0; i < len; i++)
	{
		crc + buf[i];
	}

	return crc;
	unguardSlow;
}

//==========================================================================
//
//	VDatagramDriver::Init
//
//==========================================================================

int VDatagramDriver::Init()
{
	guard(Datagram_Init);
	if (GArgs.CheckParm("-nolan"))
		return -1;

	for (int i = 0; i < net_numlandrivers; i++)
	{
		int csock = net_landrivers[i]->Init();
		if (csock == -1)
			continue;
		net_landrivers[i]->initialised = true;
		net_landrivers[i]->controlSock = csock;
	}

	return 0;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::Listen
//
//==========================================================================

void VDatagramDriver::Listen(bool state)
{
	guard(VDatagramDriver::Listen);
	for (int i = 0; i < net_numlandrivers; i++)
		if (net_landrivers[i]->initialised)
			net_landrivers[i]->Listen(state);
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::SearchForHosts
//
//==========================================================================

void VDatagramDriver::SearchForHosts(VNetLanDriver* Drv, bool xmit)
{
	guard(VDatagramDriver::SearchForHosts);
	sockaddr_t	myaddr;
	sockaddr_t	readaddr;
	int			len;
	int			control;
	byte		msgtype;
	int			n;
	int			i;

	Drv->GetSocketAddr(Drv->controlSock, &myaddr);
	if (xmit)
	{
		net_msg.Clear();
		// save space for the header, filled in later
        net_msg << 0
        		<< (byte)CCREQ_SERVER_INFO
				<< "VAVOOM"
				<< (byte)NET_PROTOCOL_VERSION;
		*((int *)net_msg.Data) = BigLong(NETFLAG_CTL | (net_msg.CurSize << 16));
		Drv->Broadcast(Drv->controlSock, net_msg.Data, net_msg.CurSize);
		net_msg.Clear();
	}

	while ((len = Drv->Read(Drv->controlSock, net_msg.Data, net_msg.MaxSize, &readaddr)) > 0)
	{
		if (len < (int)sizeof(int))
			continue;
		net_msg.CurSize = len;

		// don't answer our own query
		if (Drv->AddrCompare(&readaddr, &myaddr) >= 0)
			continue;

		// is the cache full?
		if (hostCacheCount == HOSTCACHESIZE)
			continue;

		net_msg.BeginReading();

		net_msg >> control;
		control = BigLong(*((vint32*)net_msg.Data));
		if (control == -1)
			continue;
		if ((control & NETFLAG_FLAGS_MASK) != NETFLAG_CTL)
			continue;
		if (((vint32)(control & NETFLAG_LENGTH_MASK) >> 16) != len)
			continue;

		net_msg >> msgtype;
		if (msgtype != CCREP_SERVER_INFO)
			continue;

		char*			addr;
		const char*		str;

		addr = Drv->AddrToString(&readaddr);

		// search the cache for this server
		for (n = 0; n < hostCacheCount; n++)
			if (strcmp(addr, hostcache[n].cname) == 0)
				break;

		// is it already there?
		if (n < hostCacheCount)
			continue;

		// add it
		hostCacheCount++;
		net_msg >> str;
		strcpy(hostcache[n].name, str);
		net_msg >> str;
		strncpy(hostcache[n].map, str, 15);
		hostcache[n].users = net_msg.ReadByte();
		hostcache[n].maxusers = net_msg.ReadByte();
		if (net_msg.ReadByte() != NET_PROTOCOL_VERSION)
		{
			strcpy(hostcache[n].cname, hostcache[n].name);
			hostcache[n].cname[14] = 0;
			strcpy(hostcache[n].name, "*");
			strcat(hostcache[n].name, hostcache[n].cname);
		}
		strcpy(hostcache[n].cname, addr);
		i = 0;
		do
		{
			net_msg >> str;
			strncpy(hostcache[n].wadfiles[i++], str, 15);
		}  while (*str);

		// check for a name conflict
		for (i = 0; i < hostCacheCount; i++)
		{
			if (i == n)
				continue;
			if (stricmp(hostcache[n].name, hostcache[i].name) == 0)
			{
				i = strlen(hostcache[n].name);
				if (i < 15 && hostcache[n].name[i - 1] > '8')
				{
					hostcache[n].name[i] = '0';
					hostcache[n].name[i + 1] = 0;
				}
				else
				{
					hostcache[n].name[i - 1]++;
				}
				i = -1;
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::SearchForHosts
//
//==========================================================================

void VDatagramDriver::SearchForHosts(bool xmit)
{
	guard(Datagram_SearchForHosts);
	for (int i = 0; i < net_numlandrivers; i++)
	{
		if (hostCacheCount == HOSTCACHESIZE)
			break;
		if (net_landrivers[i]->initialised)
			SearchForHosts(net_landrivers[i], xmit);
	}
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::Connect
//
//==========================================================================

qsocket_t* VDatagramDriver::Connect(VNetLanDriver* Drv, const char* host)
{
	guard(VDatagramDriver::Connect);
#ifdef CLIENT
	sockaddr_t		sendaddr;
	sockaddr_t		readaddr;
	qsocket_t		*sock;
	int				newsock;
	double			start_time;
	int				reps;
	int				ret;
	int				control;
	const char*		reason;
	byte			msgtype;
	int				newport;

	// see if we can resolve the host name
	if (Drv->GetAddrFromName(host, &sendaddr) == -1)
		return NULL;

	newsock = Drv->OpenSocket(0);
	if (newsock == -1)
		return NULL;

	sock = NET_NewQSocket(this);
	if (sock == NULL)
		goto ErrorReturn2;
	sock->socket = newsock;
	sock->landriver = Drv;

	// connect to the host
	if (Drv->Connect(newsock, &sendaddr) == -1)
		goto ErrorReturn;

	// send the connection request
	GCon->Log("trying..."); SCR_Update();
	start_time = net_time;

	for (reps = 0; reps < 3; reps++)
	{
		net_msg.Clear();
		// save space for the header, filled in later
        net_msg << 0
				<< (byte)CCREQ_CONNECT
				<< "VAVOOM"
				<< (byte)NET_PROTOCOL_VERSION;
		*((int *)net_msg.Data) = BigLong(NETFLAG_CTL | (net_msg.CurSize << 16));
		Drv->Write(newsock, net_msg.Data, net_msg.CurSize, &sendaddr);
		net_msg.Clear();

		do
		{
			ret = Drv->Read(newsock, net_msg.Data, net_msg.MaxSize, &readaddr);
			// if we got something, validate it
			if (ret > 0)
			{
				// is it from the right place?
				if (sock->landriver->AddrCompare(&readaddr, &sendaddr) != 0)
				{
					ret = 0;
					continue;
				}

				if (ret < (int)sizeof(int))
				{
					ret = 0;
					continue;
				}

				net_msg.CurSize = ret;
	            net_msg.BeginReading();

    	        net_msg >> control;
				control = BigLong(*((int *)net_msg.Data));
				if (control == -1)
				{
					ret = 0;
					continue;
				}
				if ((control & NETFLAG_FLAGS_MASK) !=  NETFLAG_CTL)
				{
					ret = 0;
					continue;
				}
				if (((vint32)(control & NETFLAG_LENGTH_MASK) >> 16) != ret)
				{
					ret = 0;
					continue;
				}
			}
		}
		while (ret == 0 && (SetNetTime() - start_time) < 2.5);
		if (ret)
			break;
		GCon->Log("still trying..."); SCR_Update();
		start_time = SetNetTime();
	}

	if (ret == 0)
	{
		reason = "No Response";
		GCon->Log(reason);
		strcpy(m_return_reason, reason);
		goto ErrorReturn;
	}

	if (ret == -1)
	{
		reason = "Network Error";
		GCon->Log(reason);
		strcpy(m_return_reason, reason);
		goto ErrorReturn;
	}

	net_msg >> msgtype;
	if (msgtype == CCREP_REJECT)
	{
		net_msg >> reason;
		GCon->Log(reason);
		strncpy(m_return_reason, reason, 31);
		goto ErrorReturn;
	}

	if (msgtype != CCREP_ACCEPT)
	{
		reason = "Bad Response";
		GCon->Log(reason);
		strcpy(m_return_reason, reason);
		goto ErrorReturn;
	}

	net_msg >> newport;

	memcpy(&sock->addr, &readaddr, sizeof(sockaddr_t));
	Drv->SetSocketPort(&sock->addr, newport);

	Drv->GetNameFromAddr(&sendaddr, sock->address);

	GCon->Log("Connection accepted");
	sock->lastMessageTime = SetNetTime();

	// switch the connection to the specified address
	if (Drv->Connect(newsock, &sock->addr) == -1)
	{
		reason = "Connect to Game failed";
		GCon->Log(reason);
		strcpy(m_return_reason, reason);
		goto ErrorReturn;
	}

//	m_return_onerror = false;
	return sock;

ErrorReturn:
	NET_FreeQSocket(sock);
ErrorReturn2:
	Drv->CloseSocket(newsock);
//	if (m_return_onerror)
//	{
//		key_dest = key_menu;
//		m_state = m_return_state;
//		m_return_onerror = false;
//	}
#endif
	return NULL;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::Connect
//
//==========================================================================

qsocket_t* VDatagramDriver::Connect(const char* host)
{
	guard(Datagram_Connect);
	for (int i = 0; i < net_numlandrivers; i++)
	{
		if (net_landrivers[i]->initialised)
		{
			qsocket_t* ret = Connect(net_landrivers[i], host);
			if (ret)
			{
				return ret;
			}
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::CheckNewConnections
//
//==========================================================================

qsocket_t* VDatagramDriver::CheckNewConnections(VNetLanDriver* Drv)
{
	guard(VDatagramDriver::CheckNewConnections);
#ifdef SERVER
	sockaddr_t	clientaddr;
	sockaddr_t	newaddr;
	int			acceptsock;
	int			newsock;
	int			len;
	int 		control;
	byte		command;
	qsocket_t	*sock;
	qsocket_t	*s;
	int			ret;
	const char*	gamename;

	acceptsock = Drv->CheckNewConnections();
	if (acceptsock == -1)
		return NULL;

	net_msg.Clear();

	len = Drv->Read(acceptsock, net_msg.Data, net_msg.MaxSize, &clientaddr);
	if (len < (int)sizeof(int))
		return NULL;
	net_msg.CurSize = len;

	net_msg.BeginReading();
	net_msg >> control;
	control = BigLong(*((int *)net_msg.Data));
	if (control == -1)
		return NULL;
	if ((control & NETFLAG_FLAGS_MASK) != NETFLAG_CTL)
		return NULL;
	if (((vint32)(control & NETFLAG_LENGTH_MASK) >> 16) != len)
		return NULL;

	net_msg >> command;
	if (command == CCREQ_SERVER_INFO)
	{
		net_msg >> gamename;
		if (strcmp(gamename, "VAVOOM") != 0)
			return NULL;

		net_msg.Clear();
		// save space for the header, filled in later
   		net_msg << 0
				<< (byte)CCREP_SERVER_INFO
				<< (const char*)hostname
				<< level.mapname
				<< (byte)svs.num_connected
            	<< (byte)svs.max_clients
				<< (byte)NET_PROTOCOL_VERSION;
	    for (int i = 0; i < wadfiles.Num(); i++)
			net_msg << *wadfiles[i];
		net_msg << "";

		*((int *)net_msg.Data) = BigLong(NETFLAG_CTL | (net_msg.CurSize << 16));
		Drv->Write(acceptsock, net_msg.Data, net_msg.CurSize, &clientaddr);
		net_msg.Clear();
		return NULL;
	}

	if (command != CCREQ_CONNECT)
		return NULL;

	net_msg >> gamename;
	if (strcmp(gamename, "VAVOOM") != 0)
		return NULL;

/*	if (MSG_ReadByte() != NET_PROTOCOL_VERSION)
	{
		SZ_Clear(&net_message);
		// save space for the header, filled in later
		MSG_WriteLong(&net_message, 0);
		MSG_WriteByte(&net_message, CCREP_REJECT);
		MSG_WriteString(&net_message, "Incompatible version.\n");
		*((int *)net_message.data) = BigLong(NETFLAG_CTL | (net_message.cursize & NETFLAG_LENGTH_MASK));
		Drv.Write (acceptsock, net_message.data, net_message.cursize, &clientaddr);
		SZ_Clear(&net_message);
		return NULL;
	}
*/
	// see if this guy is already connected
	for (s = net_activeSockets; s; s = s->next)
	{
		if (s->driver != this)
			continue;
		ret = Drv->AddrCompare(&clientaddr, &s->addr);
		if (ret >= 0)
		{
			// is this a duplicate connection reqeust?
			if (ret == 0 && net_time - s->connecttime < 2.0)
			{
				// yes, so send a duplicate reply
				net_msg.Clear();
				Drv->GetSocketAddr(s->socket, &newaddr);
				// save space for the header, filled in later
    	   		net_msg << 0
						<< (byte)CCREP_ACCEPT
						<< (int)Drv->GetSocketPort(&newaddr);
				*((int *)net_msg.Data) = BigLong(NETFLAG_CTL | (net_msg.CurSize << 16));
				Drv->Write(acceptsock, net_msg.Data, net_msg.CurSize, &clientaddr);
				net_msg.Clear();
				return NULL;
			}
			// it's somebody coming back in from a crash/disconnect
			// so close the old qsocket and let their retry get them back in
			NET_Close(s);
			return NULL;
		}
	}

	// allocate a QSocket
	sock = NET_NewQSocket(this);
	if (sock == NULL)
	{
		// no room; try to let him know
		net_msg.Clear();
		// save space for the header, filled in later
		net_msg << 0
				<< (byte)CCREP_REJECT
				<< "Server is full.\n";
		*((int *)net_msg.Data) = BigLong(NETFLAG_CTL | (net_msg.CurSize << 16));
		Drv->Write(acceptsock, net_msg.Data, net_msg.CurSize, &clientaddr);
		net_msg.Clear();
		return NULL;
	}

	// allocate a network socket
	newsock = Drv->OpenSocket(0);
	if (newsock == -1)
	{
		NET_FreeQSocket(sock);
		return NULL;
	}

	// connect to the client
	if (Drv->Connect(newsock, &clientaddr) == -1)
	{
		Drv->CloseSocket(newsock);
		NET_FreeQSocket(sock);
		return NULL;
	}

	// everything is allocated, just fill in the details	
	sock->socket = newsock;
	sock->landriver = Drv;
	sock->addr = clientaddr;
	strcpy(sock->address, Drv->AddrToString(&clientaddr));

	Drv->GetSocketAddr(newsock, &newaddr);

	// send him back the info about the server connection he has been allocated
	net_msg.Clear();
	// save space for the header, filled in later
	net_msg << 0
			<< (byte)CCREP_ACCEPT
			<< (int)Drv->GetSocketPort(&newaddr);
	*((int *)net_msg.Data) = BigLong(NETFLAG_CTL | (net_msg.CurSize << 16));
	Drv->Write(acceptsock, net_msg.Data, net_msg.CurSize, &clientaddr);
	net_msg.Clear();

	return sock;
#else
	return NULL;
#endif
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::CheckNewConnections
//
//==========================================================================

qsocket_t* VDatagramDriver::CheckNewConnections()
{
	guard(VDatagramDriver::CheckNewConnections);
	for (int i = 0; i < net_numlandrivers; i++)
	{
		if (net_landrivers[i]->initialised)
		{
			qsocket_t* ret = CheckNewConnections(net_landrivers[i]);
			if (ret != NULL)
				return ret;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::GetMessage
//
//==========================================================================

int VDatagramDriver::GetMessage(qsocket_t* sock)
{
	guard(VDatagramDriver::GetMessage);
	dword		sequence;
	dword		length;
	dword		flags;
	dword		comprLength;
	dword		comprMethod;
	sockaddr_t	readaddr;
	int			ret = 0;
	word		crc;
	dword		count;

	//	Resend message if needed.
	if (!sock->canSend && (net_time - sock->lastSendTime) > 1.0)
		ReSendMessage(sock);

	while(1)
	{
		//	Read message.
		length = sock->landriver->Read(sock->socket, (byte *)&packetBuffer, NET_DATAGRAMSIZE, &readaddr);

//		if ((rand() & 255) > 220)
//			continue;

		if (length == 0)
			break;

		if ((int)length == -1)
		{
			GCon->Log(NAME_DevNet, "Read error");
			return -1;
		}

		if (sock->landriver->AddrCompare(&readaddr, &sock->addr) != 0)
		{
#ifdef DEBUG
			Con_DPrintf("Forged packet received\n");
			Con_DPrintf("Expected: %s\n", StrAddr(&sock->addr));
			Con_DPrintf("Received: %s\n", StrAddr(&readaddr));
#endif
			continue;
		}

		if (length < NET_HEADERSIZE)
		{
			shortPacketCount++;
			continue;
		}

//		if (length != len)
//		{
//			GCon->Log(NAME_DevNet, "Bad len");
//			continue;
//		}

		length = BigLong(packetBuffer.length);
		comprLength = length & NETFLAG_COMPR_LEN_MASK;
		comprMethod = length & NETFLAG_COMPR_MODE_MASK;
		flags = length & NETFLAG_FLAGS_MASK;
		length = (length & NETFLAG_LENGTH_MASK) >> 16;

		if (flags & NETFLAG_CTL)
			continue;

		sequence = BigLong(packetBuffer.sequence);
		crc = BigShort(packetBuffer.crc);
		packetsReceived++;

		if (flags & (NETFLAG_UNRELIABLE | NETFLAG_DATA))
		{
			//	Check if checksum is OK.
			word buf_crc = NetbufferChecksum(packetBuffer.data, length - NET_HEADERSIZE);
			if (buf_crc != crc)
			{
				GCon->Logf(NAME_DevNet, "bad packet checksum %04x %04d", buf_crc, crc);
				continue;
			}

			if (comprMethod == NETFLAG_COMPR_ZIP)
			{
				if (comprLength > MAX_DATAGRAM)
				{
					GCon->Logf(NAME_DevNet, "Bad decompressed length");
					continue;
				}
				byte CompressedData[MAX_DATAGRAM];
				memcpy(CompressedData, packetBuffer.data, length - NET_HEADERSIZE);
				uLongf DecomprLength = comprLength;
				if (uncompress(packetBuffer.data, &DecomprLength,
					CompressedData, length - NET_HEADERSIZE) != Z_OK)
				{
					GCon->Logf(NAME_DevNet, "Decompression failed");
					continue;
				}
				length = comprLength + NET_HEADERSIZE;
			}
		}

		if (flags & NETFLAG_UNRELIABLE)
		{
			if (sequence < sock->unreliableReceiveSequence)
			{
				GCon->Log(NAME_DevNet, "Got a stale datagram");
				ret = 0;
				break;
			}
			if (sequence != sock->unreliableReceiveSequence)
			{
				count = sequence - sock->unreliableReceiveSequence;
				droppedDatagrams += count;
				GCon->Logf(NAME_DevNet, "Dropped %d datagram(s)", count);
			}
			sock->unreliableReceiveSequence = sequence + 1;

			length -= NET_HEADERSIZE;

			net_msg.Clear();
			net_msg.Write(packetBuffer.data, length);

			ret = 2;
			break;
		}

		if (flags & NETFLAG_ACK)
		{
			if (sequence != sock->sendSequence - 1)
			{
				GCon->Log(NAME_DevNet, "Stale ACK received");
				continue;
			}
			if (sequence == sock->ackSequence)
			{
				sock->ackSequence++;
				if (sock->ackSequence != sock->sendSequence)
					GCon->Log(NAME_DevNet, "ack sequencing error");
			}
			else
			{
				GCon->Log(NAME_DevNet, "Duplicate ACK received");
				continue;
			}

			sock->sendMessageLength -= MAX_DATAGRAM;
			if (sock->sendMessageLength > 0)
			{
				memcpy(sock->sendMessage, sock->sendMessage + MAX_DATAGRAM, sock->sendMessageLength);
				sock->sendNext = true;
			}
			else
			{
				sock->sendMessageLength = 0;
				sock->canSend = true;
			}

			continue;
		}

		if (flags & NETFLAG_DATA)
		{
			packetBuffer.length = BigLong(NETFLAG_ACK | (NET_HEADERSIZE << 16));
			packetBuffer.sequence = BigLong(sequence);
			packetBuffer.crc = 0;
			sock->landriver->Write(sock->socket, (byte *)&packetBuffer, NET_HEADERSIZE, &readaddr);

			if (sequence != sock->receiveSequence)
			{
				receivedDuplicateCount++;
				continue;
			}
			sock->receiveSequence++;

			length -= NET_HEADERSIZE;

			if (flags & NETFLAG_EOM)
			{
				net_msg.Clear();
				net_msg.Write(sock->receiveMessage, sock->receiveMessageLength);
				net_msg.Write(packetBuffer.data, length);
				sock->receiveMessageLength = 0;

				ret = 1;
				break;
			}

			memcpy(sock->receiveMessage + sock->receiveMessageLength, packetBuffer.data, length);
			sock->receiveMessageLength += length;
			continue;
		}
	}

	if (sock->sendNext)
		SendMessageNext(sock);

	return ret;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::BuildNetPacket
//
//==========================================================================

int VDatagramDriver::BuildNetPacket(vuint32 Flags, vuint32 Sequence,
	vuint8* Data, vuint32 DataLen)
{
	guard(VDatagramDriver::BuildNetPacket);
	vuint32 ComprMethod = NETFLAG_COMPR_NONE;
	vuint32 ComprLength = DataLen;

	//	Try to compress
	uLongf ZipLen = MAX_DATAGRAM;
	if (compress(packetBuffer.data, &ZipLen, Data, DataLen) == Z_OK)
	{
		if (ZipLen < DataLen)
		{
			ComprMethod = NETFLAG_COMPR_ZIP;
			ComprLength = ZipLen;
		}
	}

	//	Just copy data if it cannot be compressed.
	if (ComprMethod == NETFLAG_COMPR_NONE)
	{
		memcpy(packetBuffer.data, Data, DataLen);
	}

	vuint32 PacketLen = NET_HEADERSIZE + ComprLength;
	vuint16 CRC = NetbufferChecksum(packetBuffer.data, ComprLength);
	packetBuffer.length = BigLong(DataLen | ComprMethod |
		(PacketLen << 16) | Flags);
	packetBuffer.sequence = BigLong(Sequence);
	packetBuffer.crc = BigShort(CRC);
	return PacketLen;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::SendMessage
//
//==========================================================================

int VDatagramDriver::SendMessage(qsocket_t* sock, VMessage* data)
{
	guard(VDatagramDriver::SendMessage);
	vuint32		packetLen;
	vuint32		dataLen;
	vuint32		eom;

#ifdef DEBUG
	if (data->CurSize == 0)
		I_Error("Datagram_SendMessage: zero length message\n");

	if (data->CurSize > NET_MAXMESSAGE)
		I_Error("Datagram_SendMessage: message too big %u\n", data->CurSize);

	if (sock->canSend == false)
		I_Error("SendMessage: called with canSend == false\n");
#endif

    memcpy(sock->sendMessage, data->Data, data->CurSize);
    sock->sendMessageLength = data->CurSize;

	if (data->CurSize <= MAX_DATAGRAM)
	{
		dataLen = data->CurSize;
		eom = NETFLAG_EOM;
	}
	else
	{
		dataLen = MAX_DATAGRAM;
		eom = 0;
	}
	packetLen = BuildNetPacket(NETFLAG_DATA | eom, sock->sendSequence,
		sock->sendMessage, dataLen);

	sock->sendSequence++;
	sock->canSend = false;

	if (sock->landriver->Write(sock->socket, (byte *)&packetBuffer, packetLen, &sock->addr) == -1)
		return -1;

	sock->lastSendTime = net_time;
	packetsSent++;
	return 1;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::SendMessageNext
//
//==========================================================================

int VDatagramDriver::SendMessageNext(qsocket_t* sock)
{
	guard(VDatagramDriver::SendMessageNext);
	vuint32		packetLen;
	vuint32		dataLen;
	vuint32		eom;

	if (sock->sendMessageLength <= MAX_DATAGRAM)
	{
		dataLen = sock->sendMessageLength;
		eom = NETFLAG_EOM;
	}
	else
	{
		dataLen = MAX_DATAGRAM;
		eom = 0;
	}
	packetLen = BuildNetPacket(NETFLAG_DATA | eom, sock->sendSequence,
		sock->sendMessage, dataLen);

	sock->sendSequence++;
	sock->sendNext = false;

	if (sock->landriver->Write(sock->socket, (byte *)&packetBuffer, packetLen, &sock->addr) == -1)
		return -1;

	sock->lastSendTime = net_time;
	packetsSent++;
	return 1;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::ReSendMessage
//
//==========================================================================

int VDatagramDriver::ReSendMessage(qsocket_t* sock)
{
	guard(VDatagramDriver::ReSendMessage);
	vuint32		packetLen;
	vuint32		dataLen;
	vuint32		eom;

	if (sock->sendMessageLength <= MAX_DATAGRAM)
	{
		dataLen = sock->sendMessageLength;
		eom = NETFLAG_EOM;
	}
	else
	{
		dataLen = MAX_DATAGRAM;
		eom = 0;
	}
	packetLen = BuildNetPacket(NETFLAG_DATA | eom, sock->sendSequence - 1,
		sock->sendMessage, dataLen);

	sock->sendNext = false;

	if (sock->landriver->Write(sock->socket, (byte *)&packetBuffer, packetLen, &sock->addr) == -1)
		return -1;

	sock->lastSendTime = net_time;
	packetsReSent++;
	return 1;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::SendUnreliableMessage
//
//==========================================================================

int VDatagramDriver::SendUnreliableMessage(qsocket_t* sock, VMessage* data)
{
	guard(VDatagramDriver::SendUnreliableMessage);
	vuint32		packetLen;

#ifdef PARANOID
	if (data->CurSize == 0)
		Sys_Error("Datagram_SendUnreliableMessage: zero length message\n");

	if (data->CurSize > MAX_DATAGRAM)
		Sys_Error("Datagram_SendUnreliableMessage: message too big %u\n", data->CurSize);
#endif

	packetLen = BuildNetPacket(NETFLAG_UNRELIABLE,
		sock->unreliableSendSequence++, data->Data, data->CurSize);

	if (sock->landriver->Write(sock->socket, (byte *)&packetBuffer, packetLen, &sock->addr) == -1)
		return -1;

	packetsSent++;
	return 1;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::CanSendMessage
//
//==========================================================================

bool VDatagramDriver::CanSendMessage(qsocket_t* sock)
{
	guard(VDatagramDriver::CanSendMessage);
	if (sock->sendNext)
		SendMessageNext(sock);

	return sock->canSend;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::CanSendUnreliableMessage
//
//==========================================================================

bool VDatagramDriver::CanSendUnreliableMessage(qsocket_t*)
{
	return true;
}

//==========================================================================
//
//	VDatagramDriver::Close
//
//==========================================================================

void VDatagramDriver::Close(qsocket_t* sock)
{
	guard(VDatagramDriver::Close);
	sock->landriver->CloseSocket(sock->socket);
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::Shutdown
//
//==========================================================================

void VDatagramDriver::Shutdown()
{
	guard(VDatagramDriver::Shutdown);
	//
	// shutdown the lan drivers
	//
	for (int i = 0; i < net_numlandrivers; i++)
	{
		if (net_landrivers[i]->initialised)
		{
			net_landrivers[i]->Shutdown();
			net_landrivers[i]->initialised = false;
		}
	}
	unguard;
}

//==========================================================================
//
//	PrintStats
//
//==========================================================================

static void PrintStats(qsocket_t* s)
{
	GCon->Logf("canSend = %4d", s->canSend);
	GCon->Logf("sendSeq = %4d", s->sendSequence);
	GCon->Logf("recvSeq = %4d", s->receiveSequence);
	GCon->Logf("");
}

//==========================================================================
//
//	COMMAND NetStats
//
//==========================================================================

COMMAND(NetStats)
{
	guard(COMMAND NetStats);
	qsocket_t	*s;

	if (Args.Num() == 1)
	{
		GCon->Logf("unreliable messages sent   = %d", unreliableMessagesSent);
		GCon->Logf("unreliable messages recv   = %d", unreliableMessagesReceived);
		GCon->Logf("reliable messages sent     = %d", messagesSent);
		GCon->Logf("reliable messages received = %d", messagesReceived);
		GCon->Logf("packetsSent                = %d", VDatagramDriver::packetsSent);
		GCon->Logf("packetsReSent              = %d", VDatagramDriver::packetsReSent);
		GCon->Logf("packetsReceived            = %d", VDatagramDriver::packetsReceived);
		GCon->Logf("receivedDuplicateCount     = %d", VDatagramDriver::receivedDuplicateCount);
		GCon->Logf("shortPacketCount           = %d", VDatagramDriver::shortPacketCount);
		GCon->Logf("droppedDatagrams           = %d", VDatagramDriver::droppedDatagrams);
	}
	else if (Args[1] == "*")
	{
		for (s = net_activeSockets; s; s = s->next)
			PrintStats(s);
		for (s = net_freeSockets; s; s = s->next)
			PrintStats(s);
	}
	else
	{
		for (s = net_activeSockets; s; s = s->next)
			if (Args[1].ICmp(s->address) == 0)
				break;
		if (s == NULL)
			for (s = net_freeSockets; s; s = s->next)
				if (Args[1].ICmp(s->address) == 0)
					break;
		if (s == NULL)
			return;
		PrintStats(s);
	}
	unguard;
}
