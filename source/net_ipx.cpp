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
//**	IPX driver for DOS. Taken from QUAKE sources.
//**
//**	All low level IPX function calls are in seperate functions.
//**
//**	Removed near pointer usage, added some memory utility functions.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>
#include "gamedefs.h"
#include "net_loc.h"

// MACROS ------------------------------------------------------------------

#define IPX_OPEN					0
#define IPX_CLOSE					1
#define IPX_GETROUTE				2
#define IPX_SEND					3
#define IPX_LISTEN					4
#define IPX_SCHEDULEEVENT			5	//	Not used
#define IPX_CANCEL					6	//	Not used
#define IPX_SCHEDULESPECIALEVENT	7	//	Not used
#define IPX_GETINTERVALMARKER		8	//	Not used
#define IPX_GETADDRESS				9
#define IPX_RELINQUISH				10

#define PTYPE_UNKNOWN				0
#define PTYPE_RIP					1
#define PTYPE_ECHO					2
#define PTYPE_ERROR					3
#define PTYPE_IPX					4
#define PTYPE_SPX					5

#define AF_NETWARE 					64

#define LOWMEMSIZE					(100 * 1024)
#define IPXBUFFERS					(LOWMEMSIZE / sizeof(packet_t))
#define IPXSOCKBUFFERS				5
#define IPXSOCKETS      			(IPXBUFFERS / IPXSOCKBUFFERS)

// TYPES -------------------------------------------------------------------

#pragma pack(1)

struct IPXAddr
{
	vuint8		network[4]; /* high-low */
	vuint8		node[6];    /* high-low */
	vuint16		socket;		/* high-low */
};

struct sockaddr_ipx
{
	vint16		sipx_family;
	IPXAddr		sipx_addr;
	vint8		sipx_zero[2];
};

struct ECB
{
	vuint16		Link[2];                /* offset-segment */
	vuint16		ESRAddress[2];          /* offset-segment */
	vuint8		InUseFlag;
	vuint8		CompletionCode;
	vuint16		ECBSocket;              /* high-low */
	vuint8		IPXWorkspace[4];        /* N/A */
	vuint8		DriverWorkspace[12];    /* N/A */
	vuint8		ImmediateAddress[6];    /* high-low */
	vuint16		FragmentCount;          /* low-high */

	vuint16    	fAddress[2];            /* offset-segment */
	vuint16    	fSize;                  /* low-high */
};

struct IPXPacket
{
	vuint16		PacketCheckSum;         /* high-low */
	vuint16		PacketLength;           /* high-low */
	vuint8		PacketTransportControl;
	vuint8		PacketType;

	IPXAddr		destination;
	IPXAddr		source;
};

#pragma pack()

// time is used by the communication driver to sequence packets returned
// to DOOM when more than one is waiting

struct packet_t
{
	ECB			ecb;
	IPXPacket	ipx;
	long		sequence;
	vuint8		data[NET_DATAGRAMSIZE];
};

class VIpxDriver : public VNetLanDriver
{
public:
	vuint16				ipx_cs;
	vuint16				ipx_ip;

	__dpmi_regs			regs;

	_go32_dpmi_seginfo	packets_info;
	int					packets_offset;

	vuint16				Socket[IPXSOCKETS];
	int					BasePacket[IPXSOCKETS];
	long				Sequence[IPXSOCKETS];	// for time stamp in packets
	int					handlesInUse;

	VNetPollProcedure	pollProcedure;

	int					net_acceptsocket;
	int					net_controlsocket;

	VIpxDriver();
	int Init();
	void Shutdown();
	void Listen(bool);
	int OpenSocket(int);
	int CloseSocket(int);
	int Connect(int, sockaddr_t*);
	int CheckNewConnections();
	int Read(int, vuint8*, int, sockaddr_t*);
	int Write(int, vuint8*, int, sockaddr_t*);
	int Broadcast(int, vuint8*, int);
	char* AddrToString(sockaddr_t*);
	int StringToAddr(const char*, sockaddr_t*);
	int GetSocketAddr(int, sockaddr_t*);
	VStr GetNameFromAddr(sockaddr_t*);
	int GetAddrFromName(const char*, sockaddr_t*);
	int AddrCompare(sockaddr_t*, sockaddr_t*);
	int GetSocketPort(sockaddr_t*);
	int SetSocketPort(sockaddr_t*, int);

	static void IPX_PollProcedure(void*);

	//	Low level IPX functions.
	int IPX_GetFunction();
	int IPX_OpenSocket(vuint16 port);
	void IPX_CloseSocket(vuint16 socket);
	int IPX_GetLocalTarget(IPXAddr *addr, vuint8 *localTarget);
	int IPX_SendPacket(unsigned int offset);
	int IPX_ListenForPacket(unsigned int offset);
	void IPX_GetLocalAddress(IPXAddr *addr);
	void IPX_RelinquishControl();

	//	DOS memory handling.
	int InitDOSMemory();
	int PacketOffset(int index);
	void GetPacket(int index, packet_t *packet);
	void PutPacket(int index, packet_t *packet);
	vuint8 PacketInUse(int index);
	vuint8 PacketCompletionCode(int index);
	void FreeDOSMemory();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VIpxDriver		Impl;

// CODE --------------------------------------------------------------------

//**************************************************************************
//
//	IPX Socket
//
//**************************************************************************

//==========================================================================
//
//	VIpxDriver::VIpxDriver
//
//==========================================================================

VIpxDriver::VIpxDriver()
: VNetLanDriver(1, "IPX")
, ipx_cs(0)
, ipx_ip(0)
, packets_offset(0)
, handlesInUse(0)
, pollProcedure(IPX_PollProcedure, this)
, net_acceptsocket(-1)
, net_controlsocket(0)
{
	memset(&regs, 0, sizeof(regs));
	memset(&packets_info, 0, sizeof(packets_info));
	memset(&Socket, 0, sizeof(Socket));
	memset(&BasePacket, 0, sizeof(BasePacket));
	memset(&Sequence, 0, sizeof(Sequence));
	memset(&pollProcedure, 0, sizeof(pollProcedure));
}

//==========================================================================
//
//  VIpxDriver::Init
//
//==========================================================================

int VIpxDriver::Init()
{
	guard(VIpxDriver::Init);
	size_t		i;
	sockaddr_t	addr;
	char*		colon;

	if (GArgs.CheckParm("-noipx"))
		return -1;

	// find the IPX far call entry point
	if (IPX_GetFunction())
		return -1;

	// grab a chunk of memory down in DOS land
	if (InitDOSMemory())
		return -1;

	// init socket handles
	handlesInUse = 0;
	for (i = 0; i < IPXSOCKETS; i++)
	{
		Socket[i] = 0;
	}

	// allocate a control socket
	net_controlsocket = OpenSocket(0);
	if (net_controlsocket == -1)
	{
		FreeDOSMemory();
		GCon->Log(NAME_DevNet, "IPX_Init: Unable to open control socket");
		return -1;
	}

	Net->SchedulePollProcedure(&pollProcedure, 0.01);

	GetSocketAddr(net_controlsocket, &addr);
	VStr::Cpy(Net->MyIpxAddress, AddrToString(&addr));
	colon = strrchr(Net->MyIpxAddress, ':');
	if (colon)
		*colon = 0;

	Net->IpxAvailable = true;
	GCon->Log(NAME_Init, "IPX initialised");
	return net_controlsocket;
	unguard;
}

//==========================================================================
//
//  VIpxDriver::Shutdown
//
//==========================================================================

void VIpxDriver::Shutdown()
{
	guard(VIpxDriver::Shutdown);
	Listen(false);
	CloseSocket(net_controlsocket);
	FreeDOSMemory();
	unguard;
}

//==========================================================================
//
//  VIpxDriver::IPX_PollProcedure
//
//==========================================================================

void VIpxDriver::IPX_PollProcedure(void* arg)
{
	guard(VIpxDriver::IPX_PollProcedure);
	VIpxDriver* Self = (VIpxDriver*)arg;
	Self->IPX_RelinquishControl();
	Self->Net->SchedulePollProcedure(&Self->pollProcedure, 0.01);
	unguard;
}

//==========================================================================
//
//  VIpxDriver::Listen
//
//==========================================================================

void VIpxDriver::Listen(bool state)
{
	guard(VIpxDriver::Listen);
	if (state)
	{
		// enable listening
		if (net_acceptsocket == -1)
		{
			net_acceptsocket = OpenSocket(Net->HostPort);
			if (net_acceptsocket == -1)
			{
				Sys_Error("IPX_Listen: Unable to open accept socket\n");
			}
		}
	}
	else
	{
		// disable listening
		if (net_acceptsocket != -1)
		{
			CloseSocket(net_acceptsocket);
			net_acceptsocket = -1;
		}
	}
	unguard;
}

//==========================================================================
//
//  VIpxDriver::OpenSocket
//
//==========================================================================

int VIpxDriver::OpenSocket(int port)
{
	guard(VIpxDriver::OpenSocket);
	int		handle;
	int		socket;

	if (handlesInUse == IPXSOCKETS)
		return -1;

	// open the IPX socket
	socket = IPX_OpenSocket(port);
	if (socket < 0)
	{
		return -1;
	}

	// grab a handle; fill in the ECBs, and get them listening
	for (handle = 0; handle < (int)IPXSOCKETS; handle++)
	{
		if (Socket[handle] == 0)
		{
			Socket[handle] = socket;
			BasePacket[handle] = handle * IPXSOCKBUFFERS;
			Sequence[handle] = 0;
			//
			// set up several receiving ECBs
			//
			for (int i = 0; i < IPXSOCKBUFFERS; i++)
			{
				packet_t	packet;

				GetPacket(BasePacket[handle] + i, &packet);
				packet.ecb.ECBSocket = socket;
				packet.ecb.InUseFlag = 0;
				PutPacket(BasePacket[handle] + i, &packet);
				if (i)
					IPX_ListenForPacket(PacketOffset(BasePacket[handle] + i));
			}
			handlesInUse++;
			return handle;
		}
	}

	// "this will NEVER happen"
	Sys_Error("IPX_OpenSocket: handle allocation failed\n");
	unguard;
}

//==========================================================================
//
//  VIpxDriver::CloseSocket
//
//==========================================================================

int VIpxDriver::CloseSocket(int handle)
{
	guard(VIpxDriver::CloseSocket);
	// if there's a send in progress, give it one last chance
	if (PacketInUse(BasePacket[handle]))
	{
		IPX_RelinquishControl();
	}

	// close the socket (all pending sends/received are cancelled)
	IPX_CloseSocket(Socket[handle]);

	Socket[handle] = 0;
	handlesInUse--;

	return 0;
	unguard;
}

//==========================================================================
//
//  VIpxDriver::Connect
//
//==========================================================================

int VIpxDriver::Connect(int handle, sockaddr_t *addr)
{
	guard(VIpxDriver::Connect);
	IPXAddr		ipxaddr;
	packet_t	packet;

	memcpy(&ipxaddr, &((sockaddr_ipx *)addr)->sipx_addr, sizeof(IPXAddr));

	GetPacket(BasePacket[handle], &packet);
	if (IPX_GetLocalTarget(&ipxaddr, packet.ecb.ImmediateAddress))
	{
		GCon->Log(NAME_DevNet, "Get Local Target failed");
		return -1;
	}
	PutPacket(BasePacket[handle], &packet);

	return 0;
	unguard;
}

//==========================================================================
//
//  VIpxDriver::CheckNewConnections
//
//==========================================================================

int VIpxDriver::CheckNewConnections()
{
	guard(VIpxDriver::CheckNewConnections);
	if (net_acceptsocket == -1)
		return -1;

	for (int n = 1; n < IPXSOCKBUFFERS; n++)
		if (PacketInUse(BasePacket[net_acceptsocket] + n) == 0)
			return net_acceptsocket;
	return -1;
	unguard;
}

//==========================================================================
//
//  VIpxDriver::Read
//
//==========================================================================

int VIpxDriver::Read(int handle, vuint8* buf, int len, sockaddr_t* addr)
{
	guard(VIpxDriver::Read);
	int				packetnum;
	long			besttic;
	int				copylen;
	packet_t		packet;
	packet_t		send_packet;

	// if multiple packets are waiting, return them in order by time
tryagain:

		besttic = MAXLONG;
	packetnum = -1;

	for (int i = BasePacket[handle] + 1;
		i < BasePacket[handle] + IPXSOCKBUFFERS; i++)
	{
		if (!PacketInUse(i))
		{
			GetPacket(i, &packet);
			if (packet.sequence < besttic)
			{
				besttic = packet.sequence;
				packetnum = i;
			}
		}
	}

	if (besttic == MAXLONG)
	{
		return 0;                           // no packets
	}

	//
	// got a good packet
	//
	GetPacket(packetnum, &packet);

	if (packet.ecb.CompletionCode)
	{
		GCon->Logf(NAME_Init, "Warning: IPX_Read error %d", packet.ecb.CompletionCode);
		packet.ecb.fSize = sizeof(packet_t) - sizeof(ECB);
		PutPacket(packetnum, &packet);
		IPX_ListenForPacket(PacketOffset(packetnum));
		goto tryagain;
	}

	// copy the data up to the buffer
	copylen = BigShort(packet.ipx.PacketLength) - sizeof(IPXPacket) - sizeof(long);
	if (len < copylen)
		Sys_Error("IPX_Read: buffer too small (%d vs %d)\n", len, copylen);
	memcpy(buf, packet.data, copylen);

	// fill in the addr if they want it
	if (addr)
	{
		((sockaddr_ipx *)addr)->sipx_family = AF_NETWARE;
		memcpy(&((sockaddr_ipx *)addr)->sipx_addr, &packet.ipx.source, sizeof(IPXAddr));
		((sockaddr_ipx *)addr)->sipx_zero[0] = 0;
		((sockaddr_ipx *)addr)->sipx_zero[1] = 0;
	}

	// update the send ecb's immediate address
	GetPacket(BasePacket[handle], &send_packet);
	memcpy(send_packet.ecb.ImmediateAddress, packet.ecb.ImmediateAddress, 6);
	PutPacket(BasePacket[handle], &send_packet);

	// get this ecb listening again
	packet.ecb.fSize = sizeof(packet_t) - sizeof(ECB);
	PutPacket(packetnum, &packet);
	IPX_ListenForPacket(PacketOffset(packetnum));

	return copylen;
	unguard;
}

//==========================================================================
//
//  VIpxDriver::Write
//
//==========================================================================

int VIpxDriver::Write(int handle, vuint8* buf, int len, sockaddr_t* addr)
{
	guard(VIpxDriver::Write);
	packet_t	packet;

	// has the previous send completed?
	while (PacketInUse(BasePacket[handle]))
	{
		IPX_RelinquishControl();
	}

	switch (PacketCompletionCode(BasePacket[handle]))
	{
	case 0x00: // success
	case 0xfc: // request cancelled
		break;

	case 0xfd: // malformed packet
	default:
		GCon->Logf(NAME_DevNet, "IPX driver send failure: %d", 
			(int)PacketCompletionCode(BasePacket[handle]));
		break;

	case 0xfe: // packet undeliverable
	case 0xff: // unable to send packet
		GCon->Log(NAME_DevNet, "IPX lost route, trying to re-establish");

		// look for a new route
		GetPacket(BasePacket[handle], &packet);
		if (IPX_GetLocalTarget(&packet.ipx.destination, packet.ecb.ImmediateAddress))
			return -1;
		PutPacket(BasePacket[handle], &packet);

		// re-send the one that failed
		IPX_SendPacket(PacketOffset(BasePacket[handle]));

		// report that we did not send the current one
		return 0;
	}

	GetPacket(BasePacket[handle], &packet);

	// set the length (ipx + sequence + datalength)
	packet.ecb.fSize = sizeof(IPXPacket) + sizeof(long) + len;

	// ipx header : type
	packet.ipx.PacketType = PTYPE_IPX;

	// ipx header : destination
	// set the address
	memcpy(&packet.ipx.destination, &((sockaddr_ipx*)addr)->sipx_addr, sizeof(IPXAddr));
	memcpy(packet.ecb.ImmediateAddress, ((sockaddr_ipx*)addr)->sipx_addr.node, 6);

	// sequence number
	packet.sequence = Sequence[handle];
	Sequence[handle]++;

	// copy down the data
	memcpy(packet.data, buf, len);

	PutPacket(BasePacket[handle], &packet);

	// send the packet
	IPX_SendPacket(PacketOffset(BasePacket[handle]));

	return len;
	unguard;
}

//==========================================================================
//
//  VIpxDriver::Broadcast
//
//==========================================================================

int VIpxDriver::Broadcast(int handle, vuint8* buf, int len)
{
	guard(VIpxDriver::Broadcast);
	sockaddr_ipx	addr;
	packet_t		packet;

	memset(addr.sipx_addr.network, 0x00, 4);
	memset(addr.sipx_addr.node, 0xff, 6);
	addr.sipx_addr.socket = BigShort(Net->HostPort);

	GetPacket(BasePacket[handle], &packet);
	memset(packet.ecb.ImmediateAddress, 0xff, 6);
	PutPacket(BasePacket[handle], &packet);

	return Write(handle, buf, len, (sockaddr_t *)&addr);
	unguard;
}

//==========================================================================
//
//  VIpxDriver::AddrToString
//
//==========================================================================

char* VIpxDriver::AddrToString(sockaddr_t* addr)
{
	guard(VIpxDriver::AddrToString);
	static char buf[28];

	sprintf(buf, "%02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%u",
		((sockaddr_ipx*)addr)->sipx_addr.network[0],
		((sockaddr_ipx*)addr)->sipx_addr.network[1],
		((sockaddr_ipx*)addr)->sipx_addr.network[2],
		((sockaddr_ipx*)addr)->sipx_addr.network[3],
		((sockaddr_ipx*)addr)->sipx_addr.node[0],
		((sockaddr_ipx*)addr)->sipx_addr.node[1],
		((sockaddr_ipx*)addr)->sipx_addr.node[2],
		((sockaddr_ipx*)addr)->sipx_addr.node[3],
		((sockaddr_ipx*)addr)->sipx_addr.node[4],
		((sockaddr_ipx*)addr)->sipx_addr.node[5],
		(vuint16)BigShort(((sockaddr_ipx*)addr)->sipx_addr.socket)
		);
	return buf;
	unguard;
}

//==========================================================================
//
//  VIpxDriver::StringToAddr
//
//==========================================================================

int VIpxDriver::StringToAddr(const char *string, sockaddr_t *addr)
{
	guard(VIpxDriver::StringToAddr);
	int		val;
	char	buf[3];

	buf[2] = 0;
	memset(addr, 0, sizeof(sockaddr_t));
	addr->sa_family = AF_NETWARE;

#define DO(src,dest)	\
	buf[0] = string[src];	\
	buf[1] = string[src + 1];	\
	if (sscanf(buf, "%x", &val) != 1)	\
		return -1;	\
	((sockaddr_ipx*)addr)->sipx_addr.dest = val

	DO(0, network[0]);
	DO(2, network[1]);
	DO(4, network[2]);
	DO(6, network[3]);
	DO(9, node[0]);
	DO(11, node[1]);
	DO(13, node[2]);
	DO(15, node[3]);
	DO(17, node[4]);
	DO(19, node[5]);
#undef DO

	sscanf(&string[22], "%u", &val);
	((sockaddr_ipx*)addr)->sipx_addr.socket = BigShort(val);

	return 0;
	unguard;
}

//==========================================================================
//
//  VIpxDriver::GetSocketAddr
//
//==========================================================================

int VIpxDriver::GetSocketAddr(int handle, sockaddr_t* addr)
{
	guard(VIpxDriver::GetSocketAddr);
	memset(addr, 0, sizeof(sockaddr_t));
	addr->sa_family = AF_NETWARE;
	IPX_GetLocalAddress(&((sockaddr_ipx*)addr)->sipx_addr);
	((sockaddr_ipx*)addr)->sipx_addr.socket = Socket[handle];
	return 0;
	unguard;
}

//==========================================================================
//
//  VIpxDriver::GetNameFromAddr
//
//==========================================================================

VStr VIpxDriver::GetNameFromAddr(sockaddr_t* addr)
{
	guard(VIpxDriver::GetNameFromAddr);
	return AddrToString(addr);
	unguard;
}

//==========================================================================
//
//  VIpxDriver::GetAddrFromName
//
//==========================================================================

int VIpxDriver::GetAddrFromName(const char* name, sockaddr_t* addr)
{
	guard(VIpxDriver::GetAddrFromName);
	int		n;
	char	buf[32];

	n = VStr::Length(name);

	if (n == 12)
	{
		sprintf(buf, "00000000:%s:%u", name, Net->HostPort);
		return StringToAddr(buf, addr);
	}
	if (n == 21)
	{
		sprintf(buf, "%s:%u", name, Net->HostPort);
		return StringToAddr(buf, addr);
	}
	if (n > 21 && n <= 27)
		return StringToAddr(name, addr);

	return -1;
	unguard;
}

//==========================================================================
//
//  VIpxDriver::AddrCompare
//
//==========================================================================

int VIpxDriver::AddrCompare(sockaddr_t* addr1, sockaddr_t* addr2)
{
	guard(VIpxDriver::AddrCompare);
	if (addr1->sa_family != addr2->sa_family)
	{
		return -1;
	}

	if (memcmp(&((sockaddr_ipx*)addr1)->sipx_addr,
		&((sockaddr_ipx*)addr2)->sipx_addr, 10))
	{
		return -1;
	}

	if (((sockaddr_ipx*)addr1)->sipx_addr.socket !=
		((sockaddr_ipx*)addr2)->sipx_addr.socket)
	{
		return 1;
	}

	return 0;
	unguard;
}

//==========================================================================
//
//  VIpxDriver::GetSocketPort
//
//==========================================================================

int VIpxDriver::GetSocketPort(sockaddr_t* addr)
{
	guard(VIpxDriver::GetSocketPort);
	return (vuint16)BigShort(((sockaddr_ipx*)addr)->sipx_addr.socket);
	unguard;
}

//==========================================================================
//
//  VIpxDriver::SetSocketPort
//
//==========================================================================

int VIpxDriver::SetSocketPort(sockaddr_t* addr, int port)
{
	guard(VIpxDriver::SetSocketPort);
	((sockaddr_ipx*)addr)->sipx_addr.socket = (vuint16)BigShort(port);
	return 0;
	unguard;
}

//**************************************************************************
//
//	Low level IPX functions
//
//**************************************************************************

//==========================================================================
//
//  VIpxDriver::IPX_GetFunction
//
//==========================================================================

int VIpxDriver::IPX_GetFunction()
{
	//	Find the IPX far call entry point
	regs.x.ax = 0x7a00;
	__dpmi_simulate_real_mode_interrupt(0x2f, &regs);
	if (regs.h.al != 0xff)
	{
		GCon->Log(NAME_Init, "IPX not detected");
		return -1;
	}
	ipx_cs = regs.x.es;
	ipx_ip = regs.x.di;
	return 0;
}

//==========================================================================
//
//  VIpxDriver::IPX_OpenSocket
//
//==========================================================================

int VIpxDriver::IPX_OpenSocket(vuint16 port)
{
	// open the IPX socket
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_OPEN;
	regs.h.al = 0;
	regs.x.dx = BigShort(port);
	__dpmi_simulate_real_mode_procedure_retf(&regs);
	if (regs.h.al == 0xfe)
	{
		GCon->Log(NAME_DevNet, "IPX_OpenSocket: all sockets in use");
		return -1;
	}
	if (regs.h.al == 0xff)
	{
		GCon->Log(NAME_DevNet, "IPX_OpenSocket: socket already open");
		return -1;
	}
	if (regs.h.al)
	{
		GCon->Logf(NAME_DevNet, "IPX_OpenSocket: error %02x", regs.h.al);
		return -1;
	}
	return regs.x.dx;
}

//==========================================================================
//
//  VIpxDriver::IPX_CloseSocket
//
//==========================================================================

void VIpxDriver::IPX_CloseSocket(vuint16 socket)
{
	// close the socket (all pending sends/received are cancelled)
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_CLOSE;
	regs.x.dx = socket;
	__dpmi_simulate_real_mode_procedure_retf(&regs);
}

//==========================================================================
//
//  VIpxDriver::IPX_GetLocalTarget
//
//==========================================================================

int VIpxDriver::IPX_GetLocalTarget(IPXAddr *addr, vuint8 *localTarget)
{
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_GETROUTE;
	regs.x.es = __tb >> 4;
	regs.x.si = __tb & 0x0f;
	regs.x.di = (__tb & 0x0f) + sizeof(IPXAddr);
	dosmemput(addr, sizeof(IPXAddr), __tb);
	__dpmi_simulate_real_mode_procedure_retf(&regs);
	if (regs.h.al)
		return -1;
	dosmemget(__tb + sizeof(IPXAddr), 6, localTarget);
	return 0;
}

//==========================================================================
//
//  VIpxDriver::IPX_SendPacket
//
//==========================================================================

int VIpxDriver::IPX_SendPacket(unsigned int offset)
{
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_SEND;
	regs.x.es = offset >> 4;
	regs.x.si = offset & 0xf;
	__dpmi_simulate_real_mode_procedure_retf(&regs);
	if (regs.h.al)
	{
		GCon->Logf(NAME_DevNet, "IPX_SendPacket: 0x%02x", regs.h.al);
		return -1;
	}
	return 0;
}

//==========================================================================
//
//  VIpxDriver::IPX_ListenForPacket
//
//==========================================================================

int VIpxDriver::IPX_ListenForPacket(unsigned int offset)
{
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_LISTEN;
	regs.x.es = offset >> 4;
	regs.x.si = offset & 0xf;
	__dpmi_simulate_real_mode_procedure_retf(&regs);
	if (regs.h.al)
	{
		GCon->Logf(NAME_DevNet, "IPX_ListenForPacket: 0x%02x", regs.h.al);
		return -1;
	}
	return 0;
}

//==========================================================================
//
//  VIpxDriver::IPX_GetLocalAddress
//
//==========================================================================

void VIpxDriver::IPX_GetLocalAddress(IPXAddr *addr)
{
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_GETADDRESS;
	regs.x.es = __tb >> 4;
	regs.x.si = __tb & 0x0f;
	__dpmi_simulate_real_mode_procedure_retf(&regs);
	dosmemget(__tb, 10, addr);
}

//==========================================================================
//
//  VIpxDriver::IPX_RelinquishControl
//
//==========================================================================

void VIpxDriver::IPX_RelinquishControl()
{
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_RELINQUISH;
	__dpmi_simulate_real_mode_procedure_retf(&regs);
}

//**************************************************************************
//
//	DOS memory handling
//
//**************************************************************************

//==========================================================================
//
//  VIpxDriver::InitDOSMemory
//
//==========================================================================

int VIpxDriver::InitDOSMemory()
{
	//	Grab a chunk of memory down in DOS land
	packets_info.size = (LOWMEMSIZE + 15) / 16;
	if (_go32_dpmi_allocate_dos_memory(&packets_info))
	{
		GCon->Log(NAME_Init, "Not enough low memory");
		return -1;
	}
	packets_offset = packets_info.rm_segment << 4;

	//	Set default data in packets
	packet_t	packet;
	memset(&packet, 0, sizeof(packet_t));
	for (size_t i = 0; i < IPXBUFFERS; i++)
	{
		packet.ecb.InUseFlag = 0xff;
		packet.ecb.FragmentCount = 1;
		packet.ecb.fAddress[0] = (PacketOffset(i) + sizeof(ECB)) & 0x0f;
		packet.ecb.fAddress[1] = (PacketOffset(i) + sizeof(ECB)) >> 4;
		packet.ecb.fSize = sizeof(packet_t) - sizeof(ECB);

		PutPacket(i, &packet);
	}

	return 0;
}

//==========================================================================
//
//	VIpxDriver::PacketOffset
//
//==========================================================================

int VIpxDriver::PacketOffset(int index)
{
	return packets_offset + index * sizeof(packet_t);
}

//==========================================================================
//
//	VIpxDriver::GetPacket
//
//==========================================================================

void VIpxDriver::GetPacket(int index, packet_t *packet)
{
	dosmemget(PacketOffset(index), sizeof(packet_t), packet);
}

//==========================================================================
//
//	VIpxDriver::PutPacket
//
//==========================================================================

void VIpxDriver::PutPacket(int index, packet_t *packet)
{
	dosmemput(packet, sizeof(packet_t), PacketOffset(index));
}

//==========================================================================
//
//	VIpxDriver::PacketInUse
//
//==========================================================================

vuint8 VIpxDriver::PacketInUse(int index)
{
	return _farpeekb(_dos_ds, PacketOffset(index) + 8);
}

//==========================================================================
//
//	VIpxDriver::PacketCompletionCode
//
//==========================================================================

vuint8 VIpxDriver::PacketCompletionCode(int index)
{
	return _farpeekb(_dos_ds, PacketOffset(index) + 9);
}

//==========================================================================
//
//  VIpxDriver::FreeDOSMemory
//
//==========================================================================

void VIpxDriver::FreeDOSMemory()
{
	if (packets_offset)
	{
		_go32_dpmi_free_dos_memory(&packets_info);
	}
}
