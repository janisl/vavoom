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
//		vuint8		net_protocol_version	NET_PROTOCOL_VERSION
//
// CCREQ_SERVER_INFO
//		string		"VAVOOM"
//		vuint8		net_protocol_version	NET_PROTOCOL_VERSION
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
//		vuint8		current_players
//		vuint8		max_players
//		vuint8		protocol_version		NET_PROTOCOL_VERSION
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
		vuint8		data[MAX_MSGLEN];
	} packetBuffer;

	VDatagramDriver();
	int Init();
	void Listen(bool);
	void SearchForHosts(bool);
	VSocket* Connect(const char*);
	VSocket* CheckNewConnections();
	int GetMessage(VSocket*, TArray<vuint8>&);
	int SendMessage(VSocket*, vuint8*, vuint32);
	void Close(VSocket*);
	void Shutdown();

	void SearchForHosts(VNetLanDriver*, bool);
	VSocket* Connect(VNetLanDriver*, const char*);
	VSocket* CheckNewConnections(VNetLanDriver*);
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
//	VDatagramDriver::Init
//
//==========================================================================

int VDatagramDriver::Init()
{
	guard(Datagram_Init);
	if (GArgs.CheckParm("-nolan"))
		return -1;

	for (int i = 0; i < VNetworkLocal::NumLanDrivers; i++)
	{
		VNetworkLocal::LanDrivers[i]->Net = Net;
		int csock = VNetworkLocal::LanDrivers[i]->Init();
		if (csock == -1)
			continue;
		VNetworkLocal::LanDrivers[i]->initialised = true;
		VNetworkLocal::LanDrivers[i]->controlSock = csock;
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
	for (int i = 0; i < VNetworkLocal::NumLanDrivers; i++)
		if (VNetworkLocal::LanDrivers[i]->initialised)
			VNetworkLocal::LanDrivers[i]->Listen(state);
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
	vuint8		control;
	vuint8		msgtype;
	int			n;
	int			i;

	Drv->GetSocketAddr(Drv->controlSock, &myaddr);
	if (xmit)
	{
		VMessageOut Reply(256 << 3);
		Reply << (vuint8)NETPACKET_CTL
			<< (vuint8)CCREQ_SERVER_INFO
			<< "VAVOOM"
			<< (vuint8)NET_PROTOCOL_VERSION;
		Drv->Broadcast(Drv->controlSock, Reply.GetData(), Reply.GetNumBytes());
	}

	while ((len = Drv->Read(Drv->controlSock, packetBuffer.data, MAX_MSGLEN, &readaddr)) > 0)
	{
		if (len < (int)sizeof(int))
			continue;

		// don't answer our own query
		if (Drv->AddrCompare(&readaddr, &myaddr) >= 0)
			continue;

		// is the cache full?
		if (Net->HostCacheCount == HOSTCACHESIZE)
			continue;

		VMessageIn msg(packetBuffer.data, len << 3);
		msg << control;
		if (control != NETPACKET_CTL)
			continue;

		msg << msgtype;
		if (msgtype != CCREP_SERVER_INFO)
			continue;

		char*			addr;
		VStr			str;

		addr = Drv->AddrToString(&readaddr);

		// search the cache for this server
		for (n = 0; n < Net->HostCacheCount; n++)
			if (Net->HostCache[n].CName == addr)
				break;

		// is it already there?
		if (n < Net->HostCacheCount)
			continue;

		// add it
		Net->HostCacheCount++;
		msg << str;
		Net->HostCache[n].Name = str;
		msg << str;
		Net->HostCache[n].Map = str;
		Net->HostCache[n].Users = msg.ReadByte();
		Net->HostCache[n].MaxUsers = msg.ReadByte();
		if (msg.ReadByte() != NET_PROTOCOL_VERSION)
		{
			Net->HostCache[n].Name = VStr("*") + Net->HostCache[n].Name;
		}
		Net->HostCache[n].CName = addr;
		i = 0;
		do
		{
			msg << str;
			Net->HostCache[n].WadFiles[i++] = str;
		}  while (*str);

		// check for a name conflict
		for (i = 0; i < Net->HostCacheCount; i++)
		{
			if (i == n)
				continue;
			if (Net->HostCache[n].Name.ICmp(Net->HostCache[i].Name) == 0)
			{
				i = Net->HostCache[n].Name.Length();
				if (i < 15 && Net->HostCache[n].Name[i - 1] > '8')
				{
					Net->HostCache[n].Name += '0';
				}
				else
				{
					Net->HostCache[n].Name[i - 1]++;
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
	for (int i = 0; i < VNetworkLocal::NumLanDrivers; i++)
	{
		if (Net->HostCacheCount == HOSTCACHESIZE)
			break;
		if (VNetworkLocal::LanDrivers[i]->initialised)
			SearchForHosts(VNetworkLocal::LanDrivers[i], xmit);
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
	vuint8			control;
	VStr			reason;
	vuint8			msgtype;
	int				newport;
	VMessageIn*		msg = NULL;

	// see if we can resolve the host name
	if (Drv->GetAddrFromName(host, &sendaddr) == -1)
		return NULL;

	newsock = Drv->OpenSocket(0);
	if (newsock == -1)
		return NULL;

	sock = Net->NewSocket(this);
	if (sock == NULL)
		goto ErrorReturn2;
	sock->LanSocket = newsock;
	sock->LanDriver = Drv;

	// connect to the host
	if (Drv->Connect(newsock, &sendaddr) == -1)
		goto ErrorReturn;

	// send the connection request
	GCon->Log("trying..."); SCR_Update();
	start_time = Net->NetTime;

	for (reps = 0; reps < 3; reps++)
	{
		VMessageOut MsgOut(256 << 3);
		// save space for the header, filled in later
		MsgOut << (vuint8)NETPACKET_CTL
			<< (vuint8)CCREQ_CONNECT
			<< "VAVOOM"
			<< (vuint8)NET_PROTOCOL_VERSION;
		Drv->Write(newsock, MsgOut.GetData(), MsgOut.GetNumBytes(), &sendaddr);

		do
		{
			ret = Drv->Read(newsock, packetBuffer.data, MAX_MSGLEN, &readaddr);
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

				msg = new VMessageIn(packetBuffer.data, ret << 3);

				*msg << control;
				if (control !=  NETPACKET_CTL)
				{
					ret = 0;
					delete msg;
					continue;
				}
			}
		}
		while (ret == 0 && (Net->SetNetTime() - start_time) < 2.5);
		if (ret)
			break;
		GCon->Log("still trying..."); SCR_Update();
		start_time = Net->SetNetTime();
	}

	if (ret == 0)
	{
		reason = "No Response";
		GCon->Log(reason);
		VStr::Cpy(Net->ReturnReason, *reason);
		goto ErrorReturn;
	}

	if (ret == -1)
	{
		reason = "Network Error";
		GCon->Log(reason);
		VStr::Cpy(Net->ReturnReason, *reason);
		goto ErrorReturn;
	}

	*msg << msgtype;
	if (msgtype == CCREP_REJECT)
	{
		*msg << reason;
		GCon->Log(reason);
		VStr::NCpy(Net->ReturnReason, *reason, 31);
		goto ErrorReturn;
	}

	if (msgtype != CCREP_ACCEPT)
	{
		reason = "Bad Response";
		GCon->Log(reason);
		VStr::Cpy(Net->ReturnReason, *reason);
		goto ErrorReturn;
	}

	*msg << newport;

	memcpy(&sock->Addr, &readaddr, sizeof(sockaddr_t));
	Drv->SetSocketPort(&sock->Addr, newport);

	sock->Address = Drv->GetNameFromAddr(&sendaddr);

	GCon->Log("Connection accepted");
	sock->LastMessageTime = Net->SetNetTime();

	// switch the connection to the specified address
	if (Drv->Connect(newsock, &sock->Addr) == -1)
	{
		reason = "Connect to Game failed";
		GCon->Log(reason);
		VStr::Cpy(Net->ReturnReason, *reason);
		goto ErrorReturn;
	}

	delete msg;
//	m_return_onerror = false;
	return sock;

ErrorReturn:
	Net->FreeSocket(sock);
ErrorReturn2:
	Drv->CloseSocket(newsock);
	if (msg)
		delete msg;
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
	for (int i = 0; i < VNetworkLocal::NumLanDrivers; i++)
	{
		if (VNetworkLocal::LanDrivers[i]->initialised)
		{
			VSocket* ret = Connect(VNetworkLocal::LanDrivers[i], host);
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
	vuint8		control;
	vuint8		command;
	VSocket*	sock;
	VSocket*	s;
	int			ret;
	VStr		gamename;

	acceptsock = Drv->CheckNewConnections();
	if (acceptsock == -1)
		return NULL;

	len = Drv->Read(acceptsock, packetBuffer.data, MAX_MSGLEN, &clientaddr);
	if (len < (int)sizeof(vint32))
		return NULL;
	VMessageIn msg(packetBuffer.data, len << 3);

	msg << control;
	if (control != NETPACKET_CTL)
		return NULL;

	msg << command;
	if (command == CCREQ_SERVER_INFO)
	{
		msg << gamename;
		if (gamename != "VAVOOM")
			return NULL;

		VMessageOut MsgOut(MAX_MSGLEN << 3);
		MsgOut << (vuint8)NETPACKET_CTL
			<< (vuint8)CCREP_SERVER_INFO
			<< (const char*)VNetworkLocal::HostName
			<< *level.MapName
			<< (vuint8)svs.num_connected
			<< (vuint8)svs.max_clients
			<< (vuint8)NET_PROTOCOL_VERSION;
		for (int i = 0; i < wadfiles.Num(); i++)
			MsgOut << *wadfiles[i];
		MsgOut << "";

		Drv->Write(acceptsock, MsgOut.GetData(), MsgOut.GetNumBytes(), &clientaddr);
		return NULL;
	}

	if (command != CCREQ_CONNECT)
		return NULL;

	msg << gamename;
	if (gamename != "VAVOOM")
		return NULL;

/*	if (MSG_ReadByte() != NET_PROTOCOL_VERSION)
	{
		SZ_Clear(&net_message);
		// save space for the header, filled in later
		MSG_WriteLong(&net_message, 0);
		MSG_WriteByte(&net_message, CCREP_REJECT);
		MSG_WriteString(&net_message, "Incompatible version.\n");
		*((int *)net_message.data) = BigLong(NETPACKET_CTL | (net_message.cursize & NETFLAG_LENGTH_MASK));
		Drv.Write (acceptsock, net_message.data, net_message.cursize, &clientaddr);
		SZ_Clear(&net_message);
		return NULL;
	}
*/
	// see if this guy is already connected
	for (s = Net->ActiveSockets; s; s = s->Next)
	{
		if (s->Driver != this)
			continue;
		ret = Drv->AddrCompare(&clientaddr, &s->Addr);
		if (ret >= 0)
		{
			// is this a duplicate connection reqeust?
			if (ret == 0 && Net->NetTime - s->ConnectTime < 2.0)
			{
				// yes, so send a duplicate reply
				VMessageOut MsgOut(32 << 3);
				Drv->GetSocketAddr(s->LanSocket, &newaddr);
				// save space for the header, filled in later
				MsgOut << (vuint8)NETPACKET_CTL
					<< (vuint8)CCREP_ACCEPT
					<< (vint32)Drv->GetSocketPort(&newaddr);
				Drv->Write(acceptsock, MsgOut.GetData(), MsgOut.GetNumBytes(), &clientaddr);
				return NULL;
			}
			// it's somebody coming back in from a crash/disconnect
			// so close the old socket and let their retry get them back in
			s->Close();
			return NULL;
		}
	}

	// allocate a QSocket
	sock = Net->NewSocket(this);
	if (sock == NULL)
	{
		// no room; try to let him know
		VMessageOut MsgOut(256 << 3);
		MsgOut << (vuint8)NETPACKET_CTL
			<< (vuint8)CCREP_REJECT
			<< "Server is full.\n";
		Drv->Write(acceptsock, MsgOut.GetData(), MsgOut.GetNumBytes(), &clientaddr);
		return NULL;
	}

	// allocate a network socket
	newsock = Drv->OpenSocket(0);
	if (newsock == -1)
	{
		Net->FreeSocket(sock);
		return NULL;
	}

	// connect to the client
	if (Drv->Connect(newsock, &clientaddr) == -1)
	{
		Drv->CloseSocket(newsock);
		Net->FreeSocket(sock);
		return NULL;
	}

	// everything is allocated, just fill in the details	
	sock->LanSocket = newsock;
	sock->LanDriver = Drv;
	sock->Addr = clientaddr;
	sock->Address = Drv->AddrToString(&clientaddr);

	Drv->GetSocketAddr(newsock, &newaddr);

	// send him back the info about the server connection he has been allocated
	VMessageOut MsgOut(32 << 3);
	MsgOut << (vuint8)NETPACKET_CTL
		<< (vuint8)CCREP_ACCEPT
		<< (vint32)Drv->GetSocketPort(&newaddr);
	Drv->Write(acceptsock, MsgOut.GetData(), MsgOut.GetNumBytes(), &clientaddr);

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
	for (int i = 0; i < Net->NumLanDrivers; i++)
	{
		if (Net->LanDrivers[i]->initialised)
		{
			VSocket* ret = CheckNewConnections(Net->LanDrivers[i]);
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

int VDatagramDriver::GetMessage(VSocket* Sock, TArray<vuint8>& Data)
{
	guard(VDatagramDriver::GetMessage);
	vuint32		length;
	sockaddr_t	readaddr;
	int			ret = 0;

	while(1)
	{
		//	Read message.
		length = Sock->LanDriver->Read(Sock->LanSocket, (vuint8*)&packetBuffer,
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

		if (Sock->LanDriver->AddrCompare(&readaddr, &Sock->Addr) != 0)
		{
			continue;
		}

		Data.SetNum(length);
		memcpy(Data.Ptr(), packetBuffer.data, length);

		ret = 1;
		break;
	}

	return ret;
	unguard;
}

//==========================================================================
//
//	VDatagramDriver::SendMessage
//
//==========================================================================

int VDatagramDriver::SendMessage(VSocket* Sock, vuint8* Data, vuint32 Length)
{
	guard(VDatagramDriver::SendMessage);
	checkSlow(Length > 0);
	checkSlow(Length <= MAX_MSGLEN);
	return Sock->LanDriver->Write(Sock->LanSocket, Data, Length,
		&Sock->Addr) == -1 ? -1 : 1;
	unguard;
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
	for (int i = 0; i < Net->NumLanDrivers; i++)
	{
		if (Net->LanDrivers[i]->initialised)
		{
			Net->LanDrivers[i]->Shutdown();
			Net->LanDrivers[i]->initialised = false;
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

	VNetworkLocal* Net = (VNetworkLocal*)GNet;
	if (Args.Num() == 1)
	{
		GCon->Logf("unreliable messages sent   = %d", Net->UnreliableMessagesSent);
		GCon->Logf("unreliable messages recv   = %d", Net->UnreliableMessagesReceived);
		GCon->Logf("reliable messages sent     = %d", Net->MessagesSent);
		GCon->Logf("reliable messages received = %d", Net->MessagesReceived);
		GCon->Logf("packetsSent                = %d", Net->packetsSent);
		GCon->Logf("packetsReSent              = %d", Net->packetsReSent);
		GCon->Logf("packetsReceived            = %d", Net->packetsReceived);
		GCon->Logf("receivedDuplicateCount     = %d", Net->receivedDuplicateCount);
		GCon->Logf("shortPacketCount           = %d", Net->shortPacketCount);
		GCon->Logf("droppedDatagrams           = %d", Net->droppedDatagrams);
	}
	else if (Args[1] == "*")
	{
		for (s = Net->ActiveSockets; s; s = s->Next)
			PrintStats(s);
		for (s = Net->FreeSockets; s; s = s->Next)
			PrintStats(s);
	}
	else
	{
		for (s = Net->ActiveSockets; s; s = s->Next)
			if (Args[1].ICmp(s->Address) == 0)
				break;
		if (s == NULL)
			for (s = Net->FreeSockets; s; s = s->Next)
				if (Args[1].ICmp(s->Address) == 0)
					break;
		if (s == NULL)
			return;
		PrintStats(s);
	}
	unguard;
}
