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
#include "net_ipx.h"

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
	byte	network[4]; /* high-low */
	byte	node[6];    /* high-low */
	word	socket;		/* high-low */
};

struct sockaddr_ipx
{
    short		sipx_family;
	IPXAddr		sipx_addr;
    char		sipx_zero[2];
};

struct ECB
{
	word		Link[2];                /* offset-segment */
	word		ESRAddress[2];          /* offset-segment */
	byte		InUseFlag;
	byte		CompletionCode;
	word		ECBSocket;              /* high-low */
	byte		IPXWorkspace[4];        /* N/A */
	byte		DriverWorkspace[12];    /* N/A */
	byte		ImmediateAddress[6];    /* high-low */
	word		FragmentCount;          /* low-high */

	word    	fAddress[2];            /* offset-segment */
	word    	fSize;                  /* low-high */
};

struct IPXPacket
{
	word		PacketCheckSum;         /* high-low */
	word		PacketLength;           /* high-low */
	byte		PacketTransportControl;
	byte		PacketType;

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
	byte		data[NET_DATAGRAMSIZE];
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static int _PacketOffset(int index);
static void _PutPacket(int index, packet_t *packet);
static void IPX_PollProcedure(void*);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static word				ipx_cs;
static word				ipx_ip;

static __dpmi_regs		regs;

static _go32_dpmi_seginfo		packets_info;
static int						packets_offset = 0;

static word				Socket[IPXSOCKETS];
static int				BasePacket[IPXSOCKETS];
static long				Sequence[IPXSOCKETS];	// for time stamp in packets
static int          	handlesInUse;

static PollProcedure	pollProcedure = {NULL, 0.0, IPX_PollProcedure, NULL};

static int				net_acceptsocket = -1;
static int				net_controlsocket;

// CODE --------------------------------------------------------------------

//**************************************************************************
//
//	Low level IPX functions
//
//**************************************************************************

//==========================================================================
//
//  _IPX_GetFunction
//
//==========================================================================

static int _IPX_GetFunction(void)
{
	//	Find the IPX far call entry point
	regs.x.ax = 0x7a00;
	__dpmi_simulate_real_mode_interrupt(0x2f, &regs);
	if (regs.h.al != 0xff)
	{
		con << "IPX not detected\n";
		return -1;
	}
	ipx_cs = regs.x.es;
	ipx_ip = regs.x.di;
	return 0;
}

//==========================================================================
//
//  _IPX_OpenSocket
//
//==========================================================================

static int _IPX_OpenSocket(word port)
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
		cond << "_IPX_OpenSocket: all sockets in use\n";
		return -1;
	}
	if (regs.h.al == 0xff)
	{
		cond << "_IPX_OpenSocket: socket already open\n";
		return -1;
	}
	if (regs.h.al)
	{
		cond << "_IPX_OpenSocket: error " << hex << (int)regs.h.al << endl;
		return -1;
	}
    return regs.x.dx;
}

//==========================================================================
//
//  _IPX_CloseSocket
//
//==========================================================================

static void _IPX_CloseSocket(word socket)
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
//  _IPX_GetLocalTarget
//
//==========================================================================

static int _IPX_GetLocalTarget(IPXAddr *addr, byte *localTarget)
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
//  _IPX_SendPacket
//
//==========================================================================

static int _IPX_SendPacket(unsigned int offset)
{
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_SEND;
	regs.x.es = offset >> 4;
	regs.x.si = offset & 0xf;
	__dpmi_simulate_real_mode_procedure_retf(&regs);
	if (regs.h.al)
	{
		cond << "_IPX_SendPacket: 0x" << hex << (int)regs.h.al << endl;
		return -1;
	}
	return 0;
}

//==========================================================================
//
//  _IPX_ListenForPacket
//
//==========================================================================

static int _IPX_ListenForPacket(unsigned int offset)
{
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_LISTEN;
	regs.x.es = offset >> 4;
	regs.x.si = offset & 0xf;
	__dpmi_simulate_real_mode_procedure_retf(&regs);
	if (regs.h.al)
	{
		cond << "_IPX_ListenForPacket: 0x" << hex << (int)regs.h.al << endl;
        return -1;
	}
    return 0;
}

//==========================================================================
//
//  _IPX_GetLocalAddress
//
//==========================================================================

static void _IPX_GetLocalAddress(IPXAddr *addr)
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
//  _IPX_RelinquishControl
//
//==========================================================================

static void _IPX_RelinquishControl(void)
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
//  _InitDOSMemory
//
//==========================================================================

static int _InitDOSMemory()
{
	//	Grab a chunk of memory down in DOS land
	packets_info.size = (LOWMEMSIZE + 15) / 16;
	if (_go32_dpmi_allocate_dos_memory(&packets_info))
	{
		con << "Not enough low memory\n";
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
		packet.ecb.fAddress[0] = (_PacketOffset(i) + sizeof(ECB)) & 0x0f;
		packet.ecb.fAddress[1] = (_PacketOffset(i) + sizeof(ECB)) >> 4;
		packet.ecb.fSize = sizeof(packet_t) - sizeof(ECB);

		_PutPacket(i, &packet);
	}

	return 0;
}

//==========================================================================
//
//	_PacketOffset
//
//==========================================================================

static int _PacketOffset(int index)
{
	return packets_offset + index * sizeof(packet_t);
}

//==========================================================================
//
//	_GetPacket
//
//==========================================================================

static void _GetPacket(int index, packet_t *packet)
{
	dosmemget(_PacketOffset(index), sizeof(packet_t), packet);
}

//==========================================================================
//
//	_PutPacket
//
//==========================================================================

static void _PutPacket(int index, packet_t *packet)
{
	dosmemput(packet, sizeof(packet_t), _PacketOffset(index));
}

//==========================================================================
//
//	_PacketInUse
//
//==========================================================================

static byte _PacketInUse(int index)
{
	return _farpeekb(_dos_ds, _PacketOffset(index) + 8);
}

//==========================================================================
//
//	_PacketCompletionCode
//
//==========================================================================

static byte _PacketCompletionCode(int index)
{
	return _farpeekb(_dos_ds, _PacketOffset(index) + 9);
}

//==========================================================================
//
//  _FreeDOSMemory
//
//==========================================================================

static void _FreeDOSMemory(void)
{
	if (packets_offset)
	{
		_go32_dpmi_free_dos_memory(&packets_info);
	}
}

//**************************************************************************
//
//	IPX Socket
//
//**************************************************************************

//==========================================================================
//
//  IPX_PollProcedure
//
//==========================================================================

void IPX_PollProcedure(void*)
{
	_IPX_RelinquishControl();
	SchedulePollProcedure(&pollProcedure, 0.01);
}

//==========================================================================
//
//  IPX_Init
//
//==========================================================================

int IPX_Init(void)
{
	size_t		i;
	sockaddr_t	addr;
	char		*colon;

	if (M_CheckParm("-noipx"))
		return -1;

	// find the IPX far call entry point
	if (_IPX_GetFunction())
    	return -1;

	// grab a chunk of memory down in DOS land
	if (_InitDOSMemory())
    	return -1;

	// init socket handles
	handlesInUse = 0;
	for (i = 0; i < IPXSOCKETS; i++)
	{
		Socket[i] = 0;
	}

	// allocate a control socket
    net_controlsocket = IPX_OpenSocket(0);
	if (net_controlsocket == -1)
	{
	    _FreeDOSMemory();
		cond << "IPX_Init: Unable to open control socket\n";
		return -1;
	}

	SchedulePollProcedure(&pollProcedure, 0.01);

	IPX_GetSocketAddr(net_controlsocket, &addr);
	strcpy(my_ipx_address, IPX_AddrToString(&addr));
	colon = strrchr(my_ipx_address, ':');
	if (colon)
		*colon = 0;

	ipxAvailable = true;
	con << "IPX initialized\n";
	return net_controlsocket;
}

//==========================================================================
//
//  IPX_Shutdown
//
//==========================================================================

void IPX_Shutdown(void)
{
	IPX_Listen(false);
	IPX_CloseSocket(net_controlsocket);
    _FreeDOSMemory();
}

//==========================================================================
//
//  IPX_Listen
//
//==========================================================================

void IPX_Listen(boolean state)
{
	if (state)
	{
		// enable listening
		if (net_acceptsocket == -1)
        {
			net_acceptsocket = IPX_OpenSocket(net_hostport);
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
			IPX_CloseSocket(net_acceptsocket);
			net_acceptsocket = -1;
		}
	}
}

//==========================================================================
//
//  IPX_OpenSocket
//
//==========================================================================

int IPX_OpenSocket(int port)
{
	int		handle;
	int		socket;

	if (handlesInUse == IPXSOCKETS)
		return -1;

	// open the IPX socket
	socket = _IPX_OpenSocket(port);
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

				_GetPacket(BasePacket[handle] + i, &packet);
				packet.ecb.ECBSocket = socket;
				packet.ecb.InUseFlag = 0;
				_PutPacket(BasePacket[handle] + i, &packet);
				if (i)
					_IPX_ListenForPacket(_PacketOffset(BasePacket[handle] + i));
			}
			handlesInUse++;
			return handle;
		}
	}

	// "this will NEVER happen"
	Sys_Error("IPX_OpenSocket: handle allocation failed\n");
}

//==========================================================================
//
//  IPX_CloseSocket
//
//==========================================================================

int IPX_CloseSocket(int handle)
{
	// if there's a send in progress, give it one last chance
	if (_PacketInUse(BasePacket[handle]))
    {
		_IPX_RelinquishControl();
	}

	// close the socket (all pending sends/received are cancelled)
	_IPX_CloseSocket(Socket[handle]);

	Socket[handle] = 0;
	handlesInUse--;

	return 0;
}

//==========================================================================
//
//  IPX_Connect
//
//==========================================================================

int IPX_Connect(int handle, sockaddr_t *addr)
{
	IPXAddr		ipxaddr;
	packet_t	packet;

	memcpy(&ipxaddr, &((sockaddr_ipx *)addr)->sipx_addr, sizeof(IPXAddr));

	_GetPacket(BasePacket[handle], &packet);
	if (_IPX_GetLocalTarget(&ipxaddr, packet.ecb.ImmediateAddress))
	{
		con << "Get Local Target failed\n";
		return -1;
	}
	_PutPacket(BasePacket[handle], &packet);

	return 0;
}

//==========================================================================
//
//  IPX_CheckNewConnections
//
//==========================================================================

int IPX_CheckNewConnections(void)
{
	int n;

	if (net_acceptsocket == -1)
		return -1;

	for (n = 1; n < IPXSOCKBUFFERS; n++)
		if (_PacketInUse(BasePacket[net_acceptsocket] + n) == 0)
			return net_acceptsocket;
	return -1;
}

//==========================================================================
//
//  IPX_Read
//
//==========================================================================

int IPX_Read(int handle, byte *buf, int len, sockaddr_t *addr)
{
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
		if (!_PacketInUse(i))
		{
			_GetPacket(i, &packet);
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
	_GetPacket(packetnum, &packet);

	if (packet.ecb.CompletionCode)
	{
		con << "Warning: IPX_Read error " << packet.ecb.CompletionCode << endl;
		packet.ecb.fSize = sizeof(packet_t) - sizeof(ECB);
		_PutPacket(packetnum, &packet);
		_IPX_ListenForPacket(_PacketOffset(packetnum));
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
	_GetPacket(BasePacket[handle], &send_packet);
	memcpy(send_packet.ecb.ImmediateAddress, packet.ecb.ImmediateAddress, 6);
	_PutPacket(BasePacket[handle], &send_packet);

	// get this ecb listening again
	packet.ecb.fSize = sizeof(packet_t) - sizeof(ECB);
	_PutPacket(packetnum, &packet);
	_IPX_ListenForPacket(_PacketOffset(packetnum));

	return copylen;
}

//==========================================================================
//
//  IPX_Write
//
//==========================================================================

int IPX_Write(int handle, byte *buf, int len, sockaddr_t *addr)
{
	packet_t	packet;

	// has the previous send completed?
	while (_PacketInUse(BasePacket[handle]))
	{
		_IPX_RelinquishControl();
	}

	switch (_PacketCompletionCode(BasePacket[handle]))
	{
		case 0x00: // success
		case 0xfc: // request cancelled
			break;

		case 0xfd: // malformed packet
		default:
			con << "IPX driver send failure: " << (int)_PacketCompletionCode(BasePacket[handle]) << endl;
			break;

		case 0xfe: // packet undeliverable
		case 0xff: // unable to send packet
			con << "IPX lost route, trying to re-establish\n";

			// look for a new route
			_GetPacket(BasePacket[handle], &packet);
			if (_IPX_GetLocalTarget(&packet.ipx.destination, packet.ecb.ImmediateAddress))
				return -1;
			_PutPacket(BasePacket[handle], &packet);

			// re-send the one that failed
			_IPX_SendPacket(_PacketOffset(BasePacket[handle]));

			// report that we did not send the current one
			return 0;
	}

	_GetPacket(BasePacket[handle], &packet);

	// set the length (ipx + sequence + datalength)
	packet.ecb.fSize = sizeof(IPXPacket) + sizeof(long) + len;

	// ipx header : type
	packet.ipx.PacketType = PTYPE_IPX;

	// ipx header : destination
	// set the address
	memcpy(&packet.ipx.destination, &((sockaddr_ipx*)addr)->sipx_addr, sizeof(IPXAddr));
//#ifndef FIXME
	memcpy(packet.ecb.ImmediateAddress, ((sockaddr_ipx*)addr)->sipx_addr.node, 6);
//#endif

	// sequence number
    packet.sequence = Sequence[handle];
	Sequence[handle]++;

	// copy down the data
	memcpy(packet.data, buf, len);

	_PutPacket(BasePacket[handle], &packet);

	// send the packet
	_IPX_SendPacket(_PacketOffset(BasePacket[handle]));

	return len;
}

//==========================================================================
//
//  IPX_Broadcast
//
//==========================================================================

int IPX_Broadcast(int handle, byte *buf, int len)
{
	sockaddr_ipx	addr;
	packet_t		packet;

	memset(addr.sipx_addr.network, 0x00, 4);
	memset(addr.sipx_addr.node, 0xff, 6);
	addr.sipx_addr.socket = BigShort(net_hostport);

	_GetPacket(BasePacket[handle], &packet);
	memset(packet.ecb.ImmediateAddress, 0xff, 6);
	_PutPacket(BasePacket[handle], &packet);

	return IPX_Write(handle, buf, len, (sockaddr_t *)&addr);
}

//==========================================================================
//
//  IPX_AddrToString
//
//==========================================================================

char *IPX_AddrToString(sockaddr_t *addr)
{
	static char buf[28];

	sprintf(buf, "%02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%u",
		((sockaddr_ipx *)addr)->sipx_addr.network[0],
		((sockaddr_ipx *)addr)->sipx_addr.network[1],
		((sockaddr_ipx *)addr)->sipx_addr.network[2],
		((sockaddr_ipx *)addr)->sipx_addr.network[3],
		((sockaddr_ipx *)addr)->sipx_addr.node[0],
		((sockaddr_ipx *)addr)->sipx_addr.node[1],
		((sockaddr_ipx *)addr)->sipx_addr.node[2],
		((sockaddr_ipx *)addr)->sipx_addr.node[3],
		((sockaddr_ipx *)addr)->sipx_addr.node[4],
		((sockaddr_ipx *)addr)->sipx_addr.node[5],
		(word)BigShort(((sockaddr_ipx *)addr)->sipx_addr.socket)
		);
	return buf;
}

//==========================================================================
//
//  IPX_StringToAddr
//
//==========================================================================

int IPX_StringToAddr(char *string, sockaddr_t *addr)
{
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
}

//==========================================================================
//
//  IPX_GetSocketAddr
//
//==========================================================================

int IPX_GetSocketAddr(int handle, sockaddr_t *addr)
{
	memset(addr, 0, sizeof(sockaddr_t));
	addr->sa_family = AF_NETWARE;
	_IPX_GetLocalAddress(&((sockaddr_ipx *)addr)->sipx_addr);
	((sockaddr_ipx *)addr)->sipx_addr.socket = Socket[handle];
	return 0;
}

//==========================================================================
//
//  IPX_GetNameFromAddr
//
//==========================================================================

int IPX_GetNameFromAddr(sockaddr_t *addr, char *name)
{
	strcpy(name, IPX_AddrToString(addr));
	return 0;
}

//==========================================================================
//
//  IPX_GetAddrFromName
//
//==========================================================================

int IPX_GetAddrFromName(char *name, sockaddr_t *addr)
{
	int		n;
	char	buf[32];

	n = strlen(name);

	if (n == 12)
	{
		sprintf(buf, "00000000:%s:%u", name, net_hostport);
		return IPX_StringToAddr(buf, addr);
	}
	if (n == 21)
	{
		sprintf(buf, "%s:%u", name, net_hostport);
		return IPX_StringToAddr(buf, addr);
	}
	if (n > 21 && n <= 27)
		return IPX_StringToAddr(name, addr);

	return -1;
}

//==========================================================================
//
//  IPX_AddrCompare
//
//==========================================================================

int IPX_AddrCompare(sockaddr_t *addr1, sockaddr_t *addr2)
{
	if (addr1->sa_family != addr2->sa_family)
		return -1;

	if (memcmp(&((sockaddr_ipx *)addr1)->sipx_addr, &((sockaddr_ipx *)addr2)->sipx_addr, 10))
		return -1;

	if (((sockaddr_ipx *)addr1)->sipx_addr.socket != ((sockaddr_ipx *)addr2)->sipx_addr.socket)
		return 1;

	return 0;
}

//==========================================================================
//
//  IPX_GetSocketPort
//
//==========================================================================

int IPX_GetSocketPort(sockaddr_t *addr)
{
	return (word)BigShort(((sockaddr_ipx *)addr)->sipx_addr.socket);
}

//==========================================================================
//
//  IPX_SetSocketPort
//
//==========================================================================

int IPX_SetSocketPort(sockaddr_t *addr, int port)
{
	((sockaddr_ipx *)addr)->sipx_addr.socket = (word)BigShort(port);
	return 0;
}

