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
	VSocket* Connect(const char*);
	VSocket* CheckNewConnections();
	int GetMessage(VSocket*);
	int SendMessage(VSocket*, VMessage*);
	int SendUnreliableMessage(VSocket*, VMessage*);
	bool CanSendMessage(VSocket*);
	bool CanSendUnreliableMessage(VSocket*);
	void Close(VSocket*);
	void Shutdown();

	static vuint16 NetbufferChecksum(const vuint8*, int);
	void SearchForHosts(VNetLanDriver*, bool);
	VSocket* Connect(VNetLanDriver*, const char*);
	VSocket* CheckNewConnections(VNetLanDriver*);
	int BuildNetPacket(vuint32, vuint32, vuint8*, vuint32);
	int SendMessageNext(VSocket*);
	int ReSendMessage(VSocket*);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

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

	for (int i = 0; i < VNetwork::NumLanDrivers; i++)
	{
		int csock = VNetwork::LanDrivers[i]->Init();
		if (csock == -1)
			continue;
		VNetwork::LanDrivers[i]->initialised = true;
		VNetwork::LanDrivers[i]->controlSock = csock;
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
	for (int i = 0; i < VNetwork::NumLanDrivers; i++)
		if (VNetwork::LanDrivers[i]->initialised)
			VNetwork::LanDrivers[i]->Listen(state);
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

	VMessage& msg = GNet->NetMsg;

	Drv->GetSocketAddr(Drv->controlSock, &myaddr);
	if (xmit)
	{
		msg.Clear();
		// save space for the header, filled in later
		msg << 0
			<< (byte)CCREQ_SERVER_INFO
			<< "VAVOOM"
			<< (byte)NET_PROTOCOL_VERSION;
		*((vint32*)msg.Data) = BigLong(NETFLAG_CTL | (msg.CurSize << 16));
		Drv->Broadcast(Drv->controlSock, msg.Data, msg.CurSize);
		msg.Clear();
	}

	while ((len = Drv->Read(Drv->controlSock, msg.Data, msg.MaxSize, &readaddr)) > 0)
	{
		if (len < (int)sizeof(int))
			continue;
		msg.CurSize = len;

		// don't answer our own query
		if (Drv->AddrCompare(&readaddr, &myaddr) >= 0)
			continue;

		// is the cache full?
		if (GNet->HostCacheCount == HOSTCACHESIZE)
			continue;

		msg.BeginReading();

		msg >> control;
		control = BigLong(*((vint32*)msg.Data));
		if (control == -1)
			continue;
		if ((control & NETFLAG_FLAGS_MASK) != NETFLAG_CTL)
			continue;
		if (((vint32)(control & NETFLAG_LENGTH_MASK) >> 16) != len)
			continue;

		msg >> msgtype;
		if (msgtype != CCREP_SERVER_INFO)
			continue;

		char*			addr;
		const char*		str;

		addr = Drv->AddrToString(&readaddr);

		// search the cache for this server
		for (n = 0; n < GNet->HostCacheCount; n++)
			if (GNet->HostCache[n].CName == addr)
				break;

		// is it already there?
		if (n < GNet->HostCacheCount)
			continue;

		// add it
		GNet->HostCacheCount++;
		msg >> str;
		GNet->HostCache[n].Name = str;
		msg >> str;
		GNet->HostCache[n].Map = str;
		GNet->HostCache[n].Users = msg.ReadByte();
		GNet->HostCache[n].MaxUsers = msg.ReadByte();
		if (msg.ReadByte() != NET_PROTOCOL_VERSION)
		{
			GNet->HostCache[n].Name = VStr("*") + GNet->HostCache[n].Name;
		}
		GNet->HostCache[n].CName = addr;
		i = 0;
		do
		{
			msg >> str;
			GNet->HostCache[n].WadFiles[i++] = str;
		}  while (*str);

		// check for a name conflict
		for (i = 0; i < GNet->HostCacheCount; i++)
		{
			if (i == n)
				continue;
			if (GNet->HostCache[n].Name.ICmp(GNet->HostCache[i].Name) == 0)
			{
				i = GNet->HostCache[n].Name.Length();
				if (i < 15 && GNet->HostCache[n].Name[i - 1] > '8')
				{
					GNet->HostCache[n].Name += '0';
				}
				else
				{
					GNet->HostCache[n].Name[i - 1]++;
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
	for (int i = 0; i < VNetwork::NumLanDrivers; i++)
	{
		if (GNet->HostCacheCount == HOSTCACHESIZE)
			break;
		if (VNetwork::LanDrivers[i]->initialised)
			SearchForHosts(VNetwork::LanDrivers[i], xmit);
	}
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::Connect
//
//==========================================================================

VSocket* VDatagramDriver::Connect(VNetLanDriver* Drv, const char* host)
{
	guard(VDatagramDriver::Connect);
#ifdef CLIENT
	sockaddr_t		sendaddr;
	sockaddr_t		readaddr;
	VSocket*		sock;
	int				newsock;
	double			start_time;
	int				reps;
	int				ret;
	int				control;
	const char*		reason;
	byte			msgtype;
	int				newport;

	VMessage& msg = GNet->NetMsg;

	// see if we can resolve the host name
	if (Drv->GetAddrFromName(host, &sendaddr) == -1)
		return NULL;

	newsock = Drv->OpenSocket(0);
	if (newsock == -1)
		return NULL;

	sock = GNet->NewSocket(this);
	if (sock == NULL)
		goto ErrorReturn2;
	sock->LanSocket = newsock;
	sock->LanDriver = Drv;

	// connect to the host
	if (Drv->Connect(newsock, &sendaddr) == -1)
		goto ErrorReturn;

	// send the connection request
	GCon->Log("trying..."); SCR_Update();
	start_time = GNet->NetTime;

	for (reps = 0; reps < 3; reps++)
	{
		msg.Clear();
		// save space for the header, filled in later
		msg << 0
			<< (byte)CCREQ_CONNECT
			<< "VAVOOM"
			<< (byte)NET_PROTOCOL_VERSION;
		*((vint32*)msg.Data) = BigLong(NETFLAG_CTL | (msg.CurSize << 16));
		Drv->Write(newsock, msg.Data, msg.CurSize, &sendaddr);
		msg.Clear();

		do
		{
			ret = Drv->Read(newsock, msg.Data, msg.MaxSize, &readaddr);
			// if we got something, validate it
			if (ret > 0)
			{
				// is it from the right place?
				if (sock->LanDriver->AddrCompare(&readaddr, &sendaddr) != 0)
				{
					ret = 0;
					continue;
				}

				if (ret < (int)sizeof(vint32))
				{
					ret = 0;
					continue;
				}

				msg.CurSize = ret;
				msg.BeginReading();

				msg >> control;
				control = BigLong(*((vint32*)msg.Data));
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
		while (ret == 0 && (GNet->SetNetTime() - start_time) < 2.5);
		if (ret)
			break;
		GCon->Log("still trying..."); SCR_Update();
		start_time = GNet->SetNetTime();
	}

	if (ret == 0)
	{
		reason = "No Response";
		GCon->Log(reason);
		strcpy(GNet->ReturnReason, reason);
		goto ErrorReturn;
	}

	if (ret == -1)
	{
		reason = "Network Error";
		GCon->Log(reason);
		strcpy(GNet->ReturnReason, reason);
		goto ErrorReturn;
	}

	msg >> msgtype;
	if (msgtype == CCREP_REJECT)
	{
		msg >> reason;
		GCon->Log(reason);
		strncpy(GNet->ReturnReason, reason, 31);
		goto ErrorReturn;
	}

	if (msgtype != CCREP_ACCEPT)
	{
		reason = "Bad Response";
		GCon->Log(reason);
		strcpy(GNet->ReturnReason, reason);
		goto ErrorReturn;
	}

	msg >> newport;

	memcpy(&sock->Addr, &readaddr, sizeof(sockaddr_t));
	Drv->SetSocketPort(&sock->Addr, newport);

	sock->Address = Drv->GetNameFromAddr(&sendaddr);

	GCon->Log("Connection accepted");
	sock->LastMessageTime = GNet->SetNetTime();

	// switch the connection to the specified address
	if (Drv->Connect(newsock, &sock->Addr) == -1)
	{
		reason = "Connect to Game failed";
		GCon->Log(reason);
		strcpy(GNet->ReturnReason, reason);
		goto ErrorReturn;
	}

//	m_return_onerror = false;
	return sock;

ErrorReturn:
	GNet->FreeSocket(sock);
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

VSocket* VDatagramDriver::Connect(const char* host)
{
	guard(Datagram_Connect);
	for (int i = 0; i < VNetwork::NumLanDrivers; i++)
	{
		if (VNetwork::LanDrivers[i]->initialised)
		{
			VSocket* ret = Connect(VNetwork::LanDrivers[i], host);
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

VSocket* VDatagramDriver::CheckNewConnections(VNetLanDriver* Drv)
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
	VSocket*	sock;
	VSocket*	s;
	int			ret;
	const char*	gamename;

	VMessage& msg = GNet->NetMsg;

	acceptsock = Drv->CheckNewConnections();
	if (acceptsock == -1)
		return NULL;

	msg.Clear();

	len = Drv->Read(acceptsock, msg.Data, msg.MaxSize, &clientaddr);
	if (len < (int)sizeof(vint32))
		return NULL;
	msg.CurSize = len;

	msg.BeginReading();
	msg >> control;
	control = BigLong(*((vint32*)msg.Data));
	if (control == -1)
		return NULL;
	if ((control & NETFLAG_FLAGS_MASK) != NETFLAG_CTL)
		return NULL;
	if (((vint32)(control & NETFLAG_LENGTH_MASK) >> 16) != len)
		return NULL;

	msg >> command;
	if (command == CCREQ_SERVER_INFO)
	{
		msg >> gamename;
		if (strcmp(gamename, "VAVOOM") != 0)
			return NULL;

		msg.Clear();
		// save space for the header, filled in later
		msg << 0
			<< (byte)CCREP_SERVER_INFO
			<< (const char*)VNetwork::HostName
			<< *level.MapName
			<< (byte)svs.num_connected
			<< (byte)svs.max_clients
			<< (byte)NET_PROTOCOL_VERSION;
		for (int i = 0; i < wadfiles.Num(); i++)
			msg << *wadfiles[i];
		msg << "";

		*((vint32*)msg.Data) = BigLong(NETFLAG_CTL | (msg.CurSize << 16));
		Drv->Write(acceptsock, msg.Data, msg.CurSize, &clientaddr);
		msg.Clear();
		return NULL;
	}

	if (command != CCREQ_CONNECT)
		return NULL;

	msg >> gamename;
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
	for (s = GNet->ActiveSockets; s; s = s->Next)
	{
		if (s->Driver != this)
			continue;
		ret = Drv->AddrCompare(&clientaddr, &s->Addr);
		if (ret >= 0)
		{
			// is this a duplicate connection reqeust?
			if (ret == 0 && GNet->NetTime - s->ConnectTime < 2.0)
			{
				// yes, so send a duplicate reply
				msg.Clear();
				Drv->GetSocketAddr(s->LanSocket, &newaddr);
				// save space for the header, filled in later
				msg << 0
					<< (byte)CCREP_ACCEPT
					<< (vint32)Drv->GetSocketPort(&newaddr);
				*((vint32*)msg.Data) = BigLong(NETFLAG_CTL | (msg.CurSize << 16));
				Drv->Write(acceptsock, msg.Data, msg.CurSize, &clientaddr);
				msg.Clear();
				return NULL;
			}
			// it's somebody coming back in from a crash/disconnect
			// so close the old socket and let their retry get them back in
			s->Close();
			return NULL;
		}
	}

	// allocate a QSocket
	sock = GNet->NewSocket(this);
	if (sock == NULL)
	{
		// no room; try to let him know
		msg.Clear();
		// save space for the header, filled in later
		msg << 0
			<< (byte)CCREP_REJECT
			<< "Server is full.\n";
		*((vint32*)msg.Data) = BigLong(NETFLAG_CTL | (msg.CurSize << 16));
		Drv->Write(acceptsock, msg.Data, msg.CurSize, &clientaddr);
		msg.Clear();
		return NULL;
	}

	// allocate a network socket
	newsock = Drv->OpenSocket(0);
	if (newsock == -1)
	{
		GNet->FreeSocket(sock);
		return NULL;
	}

	// connect to the client
	if (Drv->Connect(newsock, &clientaddr) == -1)
	{
		Drv->CloseSocket(newsock);
		GNet->FreeSocket(sock);
		return NULL;
	}

	// everything is allocated, just fill in the details	
	sock->LanSocket = newsock;
	sock->LanDriver = Drv;
	sock->Addr = clientaddr;
	sock->Address = Drv->AddrToString(&clientaddr);

	Drv->GetSocketAddr(newsock, &newaddr);

	// send him back the info about the server connection he has been allocated
	msg.Clear();
	// save space for the header, filled in later
	msg << 0
		<< (byte)CCREP_ACCEPT
		<< (vint32)Drv->GetSocketPort(&newaddr);
	*((vint32*)msg.Data) = BigLong(NETFLAG_CTL | (msg.CurSize << 16));
	Drv->Write(acceptsock, msg.Data, msg.CurSize, &clientaddr);
	msg.Clear();

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

VSocket* VDatagramDriver::CheckNewConnections()
{
	guard(VDatagramDriver::CheckNewConnections);
	for (int i = 0; i < VNetwork::NumLanDrivers; i++)
	{
		if (VNetwork::LanDrivers[i]->initialised)
		{
			VSocket* ret = CheckNewConnections(VNetwork::LanDrivers[i]);
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

int VDatagramDriver::GetMessage(VSocket* sock)
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
	if (!sock->CanSend && (GNet->NetTime - sock->LastSendTime) > 1.0)
		ReSendMessage(sock);

	while(1)
	{
		//	Read message.
		length = sock->LanDriver->Read(sock->LanSocket, (vuint8*)&packetBuffer,
			NET_DATAGRAMSIZE, &readaddr);

//		if ((rand() & 255) > 220)
//			continue;

		if (length == 0)
			break;

		if ((int)length == -1)
		{
			GCon->Log(NAME_DevNet, "Read error");
			return -1;
		}

		if (sock->LanDriver->AddrCompare(&readaddr, &sock->Addr) != 0)
		{
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
			if (sequence < sock->UnreliableReceiveSequence)
			{
				GCon->Log(NAME_DevNet, "Got a stale datagram");
				ret = 0;
				break;
			}
			if (sequence != sock->UnreliableReceiveSequence)
			{
				count = sequence - sock->UnreliableReceiveSequence;
				droppedDatagrams += count;
				GCon->Logf(NAME_DevNet, "Dropped %d datagram(s)", count);
			}
			sock->UnreliableReceiveSequence = sequence + 1;

			length -= NET_HEADERSIZE;

			GNet->NetMsg.Clear();
			GNet->NetMsg.Write(packetBuffer.data, length);

			ret = 2;
			break;
		}

		if (flags & NETFLAG_ACK)
		{
			if (sequence != sock->SendSequence - 1)
			{
				GCon->Log(NAME_DevNet, "Stale ACK received");
				continue;
			}
			if (sequence == sock->AckSequence)
			{
				sock->AckSequence++;
				if (sock->AckSequence != sock->SendSequence)
					GCon->Log(NAME_DevNet, "ack sequencing error");
			}
			else
			{
				GCon->Log(NAME_DevNet, "Duplicate ACK received");
				continue;
			}

			sock->SendMessageLength -= MAX_DATAGRAM;
			if (sock->SendMessageLength > 0)
			{
				memcpy(sock->SendMessageData, sock->SendMessageData +
					MAX_DATAGRAM, sock->SendMessageLength);
				sock->SendNext = true;
			}
			else
			{
				sock->SendMessageLength = 0;
				sock->CanSend = true;
			}

			continue;
		}

		if (flags & NETFLAG_DATA)
		{
			packetBuffer.length = BigLong(NETFLAG_ACK | (NET_HEADERSIZE << 16));
			packetBuffer.sequence = BigLong(sequence);
			packetBuffer.crc = 0;
			sock->LanDriver->Write(sock->LanSocket, (vuint8*)&packetBuffer,
				NET_HEADERSIZE, &readaddr);

			if (sequence != sock->ReceiveSequence)
			{
				receivedDuplicateCount++;
				continue;
			}
			sock->ReceiveSequence++;

			length -= NET_HEADERSIZE;

			if (flags & NETFLAG_EOM)
			{
				GNet->NetMsg.Clear();
				GNet->NetMsg.Write(sock->ReceiveMessageData,
					sock->ReceiveMessageLength);
				GNet->NetMsg.Write(packetBuffer.data, length);
				sock->ReceiveMessageLength = 0;

				ret = 1;
				break;
			}

			memcpy(sock->ReceiveMessageData + sock->ReceiveMessageLength,
				packetBuffer.data, length);
			sock->ReceiveMessageLength += length;
			continue;
		}
	}

	if (sock->SendNext)
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

int VDatagramDriver::SendMessage(VSocket* sock, VMessage* data)
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

	if (sock->CanSend == false)
		I_Error("SendMessage: called with canSend == false\n");
#endif

	memcpy(sock->SendMessageData, data->Data, data->CurSize);
	sock->SendMessageLength = data->CurSize;

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
	packetLen = BuildNetPacket(NETFLAG_DATA | eom, sock->SendSequence,
		sock->SendMessageData, dataLen);

	sock->SendSequence++;
	sock->CanSend = false;

	if (sock->LanDriver->Write(sock->LanSocket, (vuint8*)&packetBuffer,
		packetLen, &sock->Addr) == -1)
	{
		return -1;
	}

	sock->LastSendTime = GNet->NetTime;
	packetsSent++;
	return 1;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::SendMessageNext
//
//==========================================================================

int VDatagramDriver::SendMessageNext(VSocket* sock)
{
	guard(VDatagramDriver::SendMessageNext);
	vuint32		packetLen;
	vuint32		dataLen;
	vuint32		eom;

	if (sock->SendMessageLength <= MAX_DATAGRAM)
	{
		dataLen = sock->SendMessageLength;
		eom = NETFLAG_EOM;
	}
	else
	{
		dataLen = MAX_DATAGRAM;
		eom = 0;
	}
	packetLen = BuildNetPacket(NETFLAG_DATA | eom, sock->SendSequence,
		sock->SendMessageData, dataLen);

	sock->SendSequence++;
	sock->SendNext = false;

	if (sock->LanDriver->Write(sock->LanSocket, (vuint8*)&packetBuffer,
		packetLen, &sock->Addr) == -1)
	{
		return -1;
	}

	sock->LastSendTime = GNet->NetTime;
	packetsSent++;
	return 1;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::ReSendMessage
//
//==========================================================================

int VDatagramDriver::ReSendMessage(VSocket* sock)
{
	guard(VDatagramDriver::ReSendMessage);
	vuint32		packetLen;
	vuint32		dataLen;
	vuint32		eom;

	if (sock->SendMessageLength <= MAX_DATAGRAM)
	{
		dataLen = sock->SendMessageLength;
		eom = NETFLAG_EOM;
	}
	else
	{
		dataLen = MAX_DATAGRAM;
		eom = 0;
	}
	packetLen = BuildNetPacket(NETFLAG_DATA | eom, sock->SendSequence - 1,
		sock->SendMessageData, dataLen);

	sock->SendNext = false;

	if (sock->LanDriver->Write(sock->LanSocket, (vuint8*)&packetBuffer,
		packetLen, &sock->Addr) == -1)
	{
		return -1;
	}

	sock->LastSendTime = GNet->NetTime;
	packetsReSent++;
	return 1;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::SendUnreliableMessage
//
//==========================================================================

int VDatagramDriver::SendUnreliableMessage(VSocket* sock, VMessage* data)
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
		sock->UnreliableSendSequence++, data->Data, data->CurSize);

	if (sock->LanDriver->Write(sock->LanSocket, (vuint8*)&packetBuffer,
		packetLen, &sock->Addr) == -1)
	{
		return -1;
	}

	packetsSent++;
	return 1;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::CanSendMessage
//
//==========================================================================

bool VDatagramDriver::CanSendMessage(VSocket* sock)
{
	guard(VDatagramDriver::CanSendMessage);
	if (sock->SendNext)
		SendMessageNext(sock);

	return sock->CanSend;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::CanSendUnreliableMessage
//
//==========================================================================

bool VDatagramDriver::CanSendUnreliableMessage(VSocket*)
{
	return true;
}

//==========================================================================
//
//	VDatagramDriver::Close
//
//==========================================================================

void VDatagramDriver::Close(VSocket* sock)
{
	guard(VDatagramDriver::Close);
	sock->LanDriver->CloseSocket(sock->LanSocket);
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
	for (int i = 0; i < VNetwork::NumLanDrivers; i++)
	{
		if (VNetwork::LanDrivers[i]->initialised)
		{
			VNetwork::LanDrivers[i]->Shutdown();
			VNetwork::LanDrivers[i]->initialised = false;
		}
	}
	unguard;
}

//==========================================================================
//
//	PrintStats
//
//==========================================================================

static void PrintStats(VSocket* s)
{
	GCon->Logf("canSend = %4d", s->CanSend);
	GCon->Logf("sendSeq = %4d", s->SendSequence);
	GCon->Logf("recvSeq = %4d", s->ReceiveSequence);
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
	VSocket	*s;

	if (Args.Num() == 1)
	{
		GCon->Logf("unreliable messages sent   = %d", GNet->UnreliableMessagesSent);
		GCon->Logf("unreliable messages recv   = %d", GNet->UnreliableMessagesReceived);
		GCon->Logf("reliable messages sent     = %d", GNet->MessagesSent);
		GCon->Logf("reliable messages received = %d", GNet->MessagesReceived);
		GCon->Logf("packetsSent                = %d", VDatagramDriver::packetsSent);
		GCon->Logf("packetsReSent              = %d", VDatagramDriver::packetsReSent);
		GCon->Logf("packetsReceived            = %d", VDatagramDriver::packetsReceived);
		GCon->Logf("receivedDuplicateCount     = %d", VDatagramDriver::receivedDuplicateCount);
		GCon->Logf("shortPacketCount           = %d", VDatagramDriver::shortPacketCount);
		GCon->Logf("droppedDatagrams           = %d", VDatagramDriver::droppedDatagrams);
	}
	else if (Args[1] == "*")
	{
		for (s = GNet->ActiveSockets; s; s = s->Next)
			PrintStats(s);
		for (s = GNet->FreeSockets; s; s = s->Next)
			PrintStats(s);
	}
	else
	{
		for (s = GNet->ActiveSockets; s; s = s->Next)
			if (Args[1].ICmp(s->Address) == 0)
				break;
		if (s == NULL)
			for (s = GNet->FreeSockets; s; s = s->Next)
				if (Args[1].ICmp(s->Address) == 0)
					break;
		if (s == NULL)
			return;
		PrintStats(s);
	}
	unguard;
}
