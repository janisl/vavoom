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
//**	Beame & Whiteside TCP/IP driver for DOS. Taken from QUAKE sources.
//**
//**	Removed near pointer usage. Now uses DJGPP's transfer buffer.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <dpmi.h>
#include <go32.h>

#include "gamedefs.h"
#include "net_loc.h"

// MACROS ------------------------------------------------------------------

// this section is general Unix stuff that we need

#define	EIO		 		5	/* I/O error */
#define EWOULDBLOCK		35	/* function would block */
#define EMSGSIZE		40	/* message to big for buffers */
#define	EPROTONOSUPPORT	43	/* Protocol not supported */
#define	ESOCKTNOSUPPORT	44	/* Socket type not supported */
#define	EPFNOSUPPORT	46	/* Protocol family not supported */
#define EAFNOSUPPORT	47	/* Address family not supported */
#define ECONNABORTED	53	/* User requested hangup */
#define ENOBUFS 		55	/* No buffers available */
#define EISCONN 		56	/* Socket has closed */
#define ENOTCONN		57	/* Socket is not connected */
#define ESHUTDOWN		58	/* Socket is closed */
#define ETOOMANYREFS	59	/* Too many sockets open */
#define ETIMEDOUT		60	/* Connection timed out */
#define ECONNREFUSED	61	/* Connection refused */

#define AF_INET 		2	/* internet */

#define MAXHOSTNAMELEN	256

// this section is B&W specific constants & structures

#define BW_IOCTL_BIND			0
#define BW_IOCTL_CLEAROPTIONS	5
#define BW_IOCTL_SETOPTIONS		6
#define BW_IOCTL_PEEK			7
#define BW_IOCTL_SETWINMASK		8

#define BW_OPTION_BLOCKING		0x01
#define BW_OPTION_REUSEBUFFERS	0x80

#define BW_ERR_USR_HANGUP		50
#define BW_ERR_HANGUP			51
#define BW_ERR_NET_ERR			52
#define BW_ERR_IS_CLOSED		53
#define BW_ERR_TIME_OUT			54
#define BW_ERR_RESET			55
#define BW_ERR_FULL				56
#define BW_ERR_BLOCK			57
#define BW_ERR_SHUTDOWN			58

#define LOWMEM_SIZE				4096

// TYPES -------------------------------------------------------------------

struct in_addr
{
	union
	{
		struct { vuint8 s_b1, s_b2, s_b3, s_b4; } S_un_b;
		struct { vuint16 s_w1, s_w2; } S_un_w;
		vuint32 S_addr;
	} S_un;
};
#define	s_addr	S_un.S_addr	/* can be used for most tcp & ip code */
#define	s_host	S_un.S_un_b.s_b2	/* host on imp */
#define	s_net	S_un.S_un_b.s_b1	/* network */
#define	s_imp	S_un.S_un_w.s_w2	/* imp */
#define	s_impno	S_un.S_un_b.s_b4	/* imp # */
#define	s_lh	S_un.S_un_b.s_b3	/* logical host */

struct sockaddr_in
{
    vint16		sin_family;
    vuint16		sin_port;
	in_addr		sin_addr;
    vint8		sin_zero[8];
};

#pragma pack(1)

struct BW_UDPinfo_t
{
	char			state;			// always 1
	short			localPort;
	in_addr			localAddr;
	char			reason;			// always 0
	char			options;
	short			dataAvailable;
};

struct BW_UDPreadInfo1_t
{
	char			reserved1[6];
	vuint16			info2Offset;
	char			reserved2[18];
	in_addr			remoteAddr;
};

struct BW_UDPreadInfo2_t
{
	short			remotePort;
	char			reserved1[2];
	vuint16			dataLenPlus8;
	char			reserved2[2];
	char			data[1];			// actual size is <dataLenPlus8> - 8		
};

struct BW_writeInfo_t
{
	char			reserved1[2];
	short			remotePort;
	vuint16			dataLen;
	in_addr			remoteAddr;
	char			reserved2[42];
	char			data[1];			// actual size is <datalen>
};

struct BW_ethdevinfo_t
{
	short	ioport;
	vuint8	dma;
	vuint8	vector;
	vuint8	irq;
	short	bufferSize;
	short	maxWindow;
	short	timeZone;
	vuint8	myType;
	int		inetAddr;
	short	value;
	vuint8	subnetMask;
	short	etherPointer;
	short	logserverPointer;
	short	nameserverPointer;
	short	printserverPointer;
	short	timeserverPointer;
	short	gatewayPointer;
	short	driverSegment;
	vuint8	transferSize;
	char	cardName[9];
};

#pragma pack()

class VBeameWhitesideDriver : public VNetLanDriver
{
public:
	BW_ethdevinfo_t	ethdevinfo;
	int				netmask;
	in_addr 		bcastaddr;

	__dpmi_regs		regs;

	int				net_acceptsocket;	// socket for fielding new connections
	int				net_controlsocket;

	VBeameWhitesideDriver();
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

	static int dos_int86(int, __dpmi_regs*);
	int BW_ioctl(int, char*, int);
	static int BW_TranslateError(int);
	int GetEthdevinfo();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VBeameWhitesideDriver	Impl;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VBeameWhitesideDriver::VBeameWhitesideDriver
//
//==========================================================================

VBeameWhitesideDriver::VBeameWhitesideDriver()
: VNetLanDriver(0, "Beame & Whiteside TCP/IP")
, netmask(0)
, net_acceptsocket(-1)
, net_controlsocket(0)
{
	memset(&ethdevinfo, 0, sizeof(ethdevinfo));
	memset(&bcastaddr, 0, sizeof(bcastaddr));
	memset(&regs, 0, sizeof(regs));
}

//==========================================================================
//
//  VBeameWhitesideDriver::Init
//
//==========================================================================

int VBeameWhitesideDriver::Init()
{
	guard(VBeameWhitesideDriver::Init);
	sockaddr_t	addr;
	char		*colon;

	if (GArgs.CheckParm("-noudp"))
		return -1;

	if (GetEthdevinfo())
	{
		GCon->Log(NAME_DevNet, "Beame & Whiteside TCP/IP not detected");
		return -1;
	}
	netmask = 0xffffffff >> (32 - ethdevinfo.subnetMask);
	bcastaddr.s_addr = (ethdevinfo.inetAddr & netmask) | (~netmask);

	if ((net_controlsocket = OpenSocket(0)) == -1)
	{
		GCon->Log(NAME_DevNet, "BW_Init unable to open control socket; disabled");
		return -1;
	}

	GetSocketAddr(net_controlsocket, &addr);
	VStr::Cpy(Net->MyIpAddress, AddrToString(&addr));
	colon = strrchr(Net->MyIpAddress, ':');
	if (colon)
		*colon = 0;

	GCon->Log(NAME_Init, "BW_Init: UDP initialised");
	Net->IpAvailable = true;

	return net_controlsocket;
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::Shutdown
//
//==========================================================================

void VBeameWhitesideDriver::Shutdown()
{
	guard(VBeameWhitesideDriver::Shutdown);
	Listen(false);
	CloseSocket(net_controlsocket);
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::Listen
//
//==========================================================================

void VBeameWhitesideDriver::Listen(bool state)
{
	guard(VBeameWhitesideDriver::Listen);
	if (state)
	{
		// enable listening
		if (net_acceptsocket == -1)
		{
			net_acceptsocket = OpenSocket(Net->HostPort);
			if (net_acceptsocket == -1)
				Sys_Error("BW_Listen: Unable to open accept socket\n");
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
//  VBeameWhitesideDriver::OpenSocket
//
//  OpenSocket returns a handle to a network socket that has been opened,
// set to nonblocking, and bound to <port>.  Additional socket options
// should be set here if they are needed.  -1 is returned on failure.
//
//==========================================================================

int VBeameWhitesideDriver::OpenSocket(int port)
{
	guard(VBeameWhitesideDriver::OpenSocket);
	int s;
	int ret;
	int deadman = 3 * 1024;
	static int dynamic = 1024;
	static char reuse_msg[2] = {BW_IOCTL_SETOPTIONS, BW_OPTION_REUSEBUFFERS};
	static char bind_msg[3] = {BW_IOCTL_BIND, 0, 0};
	static char nonblock_msg[2] = {BW_IOCTL_CLEAROPTIONS, BW_OPTION_BLOCKING};

	// allocate a UDP socket
	dosmemput("UDP-IP10", 9, __tb);
	regs.x.ax = 0x3d42;
	regs.x.ds = __tb >> 4;
	regs.x.dx = __tb & 0xf;
	if (dos_int86(0x21, &regs))
	{
		GCon->Logf(NAME_DevNet, "BW_OpenSocket failed: %d", 
			BW_TranslateError(regs.x.ax));
		return -1;
	}
	s = regs.x.ax;

	// set file descriptor to raw mode
	regs.x.ax = 0x4401;
	regs.x.bx = s;
	regs.x.dx = 0x60;
	dos_int86(0x21, &regs);

	if (BW_ioctl(s, reuse_msg, 2))
	{
		GCon->Log(NAME_DevNet, "BW_OpenSocket ioctl(reuse) failed");
		return -1;
	}

	if (BW_ioctl(s, nonblock_msg, 2))
	{
		GCon->Log(NAME_DevNet, "BW_OpenSocket ioctl(nonblocking) failed");
		return -1;
	}

	// if a socket was specified, bind to it and return
	if (port)
	{
		*(short *)&bind_msg[1] = port;
		if (BW_ioctl(s, bind_msg, 3))
		{
			CloseSocket(s);
			return -1;
		}
		return s;
	}

	// B&W does NOT do dynamic allocation, so if port == 0 we must fake it
	do
	{
		port = dynamic++;
		if (dynamic == 4096)
			dynamic = 1024;
		deadman--;
		*(short *)&bind_msg[1] = port;
		ret = BW_ioctl(s, bind_msg, 3);
	}
	while (ret && deadman);
	if (ret)
		return -1;
	return s;
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::CloseSocket
//
//==========================================================================

int VBeameWhitesideDriver::CloseSocket(int socket)
{
	guard(VBeameWhitesideDriver::CloseSocket);
	regs.h.ah = 0x3e;
	regs.x.bx = socket;
	if (dos_int86(0x21, &regs))
	{
		GCon->Logf(NAME_DevNet,
			"VBeameWhitesideDriver::CloseSocket %d failed: %d", socket,
			BW_TranslateError(regs.x.ax));
		return -1;
	}
	return 0;
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::Connect
//
//==========================================================================

int VBeameWhitesideDriver::Connect(int, sockaddr_t *)
{
	return 0;
}

//==========================================================================
//
//  VBeameWhitesideDriver::CheckNewConnections
//
//==========================================================================

int VBeameWhitesideDriver::CheckNewConnections()
{
	guard(VBeameWhitesideDriver::CheckNewConnections);
	if (net_acceptsocket == 0)
		return -1;

	// see if there's anything waiting
	regs.x.ax = 0x4406;
	regs.x.bx = net_acceptsocket;
	dos_int86(0x21, &regs);
	if (regs.x.ax == 0)
		return -1;
	return net_acceptsocket;
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::Read
//
//==========================================================================

int VBeameWhitesideDriver::Read(int s, vuint8* buf, int len, sockaddr_t* from)
{
	guard(BW_Read);
	BW_UDPreadInfo1_t *info1;
	BW_UDPreadInfo2_t *info2;
	int copylen;

	// ask if there's anything waiting
	regs.x.ax = 0x4406;
	regs.x.bx = s;
	dos_int86(0x21, &regs);
	if (regs.x.ax == 0)
		return 0;

	// there was, so let's get it
	regs.h.ah = 0x3f;
	regs.x.cx = /* len + 53 */ LOWMEM_SIZE;
	regs.x.es = regs.x.ds = __tb >> 4;
	regs.x.dx = __tb & 0xf;
	regs.x.bx = s;
	if (dos_int86(0x21, &regs))
	{
		GCon->Logf(NAME_DevNet, "BW UDP read error: %d", 
			BW_TranslateError(regs.x.ax));
		return -1;
	}

	vuint8 buffer[LOWMEM_SIZE];
	dosmemget(__tb, LOWMEM_SIZE, buffer);

	info1 = (BW_UDPreadInfo1_t *)buffer;
	info2 = (BW_UDPreadInfo2_t *)(buffer + info1->info2Offset);

	if (from)
	{
		from->sa_family = AF_INET;
		((sockaddr_in *)from)->sin_addr = info1->remoteAddr;
		((sockaddr_in *)from)->sin_port = BigShort(info2->remotePort);
	}

	copylen = info2->dataLenPlus8 - 8;
	if (copylen > len)
	{
		GCon->Logf(NAME_DevNet, "BW UDP read packet too large: %d", copylen);
		return -1;
	}
	memcpy(buf, info2->data, copylen);

	return copylen;
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::Write
//
//==========================================================================

int VBeameWhitesideDriver::Write(int s, vuint8* msg, int len, sockaddr_t* to)
{
	guard(VBeameWhitesideDriver::Write);
	BW_writeInfo_t *writeInfo;

	// ask if we're clear to send
	regs.x.ax = 0x4407;
	regs.x.bx = s;
	dos_int86(0x21, &regs);
	if (regs.x.ax == 0)
		return 0;

	// yes, let's do it
	vuint8 buffer[LOWMEM_SIZE];
	writeInfo = (BW_writeInfo_t *)buffer;

	writeInfo->remoteAddr = ((sockaddr_in *)to)->sin_addr;
	writeInfo->remotePort = BigShort(((sockaddr_in *)to)->sin_port);
	writeInfo->dataLen = len;
	if (len > (int)NET_DATAGRAMSIZE)
		Sys_Error("BW UDP write packet too large: %u\n", len);
	memcpy(writeInfo->data, msg, len);
	writeInfo->data[len] = 0;
	dosmemput(buffer, LOWMEM_SIZE, __tb);
	regs.h.ah = 0x40;
	regs.x.bx = s;
	regs.x.cx = len + sizeof(BW_writeInfo_t);
	regs.x.es = regs.x.ds = __tb >> 4;
	regs.x.dx = __tb & 0xf;
	if (dos_int86(0x21, &regs))
	{
		GCon->Logf(NAME_DevNet, "BW_Write failed: %d", BW_TranslateError(regs.x.ax));
		return -1;
	}

	return len;
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::Broadcast
//
//==========================================================================

int VBeameWhitesideDriver::Broadcast(int s, vuint8* msg, int len)
{
	guard(BW_Broadcast);
	BW_writeInfo_t *writeInfo;

	// ask if we're clear to send
	regs.x.ax = 0x4407;
	regs.x.bx = s;
	dos_int86(0x21, &regs);
	if (regs.x.ax == 0)
		return 0;

	// yes, let's do it
	vuint8 buffer[LOWMEM_SIZE];
	writeInfo = (BW_writeInfo_t *)buffer;
	writeInfo->remoteAddr = bcastaddr;
	writeInfo->remotePort = Net->HostPort;
	writeInfo->dataLen = len;
	if (len > (int)NET_DATAGRAMSIZE)
		Sys_Error("BW UDP write packet too large: %u\n", len);
	memcpy(writeInfo->data, msg, len);
	writeInfo->data[len] = 0;
	dosmemput(buffer, LOWMEM_SIZE, __tb);
	regs.h.ah = 0x40;
	regs.x.bx = s;
	regs.x.cx = len + sizeof(BW_writeInfo_t);
	regs.x.es = regs.x.ds = __tb >> 4;
	regs.x.dx = __tb & 0xf;
	if (dos_int86(0x21, &regs))
	{
		GCon->Logf(NAME_DevNet, "BW_Broadcast failed: %d",
			BW_TranslateError(regs.x.ax));
		return -1;
	}

	return len;
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::AddrToString
//
//==========================================================================

char* VBeameWhitesideDriver::AddrToString(sockaddr_t *addr)
{
	guard(VBeameWhitesideDriver::AddrToString);
	static char buffer[22];

	sprintf(buffer, "%d.%d.%d.%d:%d",
		((sockaddr_in*)addr)->sin_addr.s_net,
		((sockaddr_in*)addr)->sin_addr.s_host,
		((sockaddr_in*)addr)->sin_addr.s_lh,
		((sockaddr_in*)addr)->sin_addr.s_impno,
		(vuint16)BigShort(((sockaddr_in*)addr)->sin_port)
		);
	return buffer;
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::StringToAddr
//
//==========================================================================

int VBeameWhitesideDriver::StringToAddr(const char* string, sockaddr_t* addr)
{
	guard(VBeameWhitesideDriver::StringToAddr);
	int ha1, ha2, ha3, ha4, hp;
	int ipaddr;

	sscanf(string, "%d.%d.%d.%d:%d", &ha1, &ha2, &ha3, &ha4, &hp);
	ipaddr = (ha1 << 24) | (ha2 << 16) | (ha3 << 8) | ha4;

	addr->sa_family = AF_INET;
	((sockaddr_in*)addr)->sin_addr.s_addr = BigLong(ipaddr);
	((sockaddr_in*)addr)->sin_port = BigShort((short)hp);
	return 0;
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::GetSocketAddr
//
//==========================================================================

int VBeameWhitesideDriver::GetSocketAddr(int socket, sockaddr_t* addr)
{
	guard(VBeameWhitesideDriver::GetSocketAddr);
	regs.x.ax = 0x4402;
	regs.x.bx = socket;
	regs.x.cx = sizeof(BW_UDPinfo_t);
	regs.x.dx = __tb & 0xf;
	regs.x.ds = __tb >> 4;
	dos_int86(0x21, &regs);

	BW_UDPinfo_t	buffer;
	dosmemget(__tb, sizeof(buffer), &buffer);
	addr->sa_family = AF_INET;
	((sockaddr_in*)addr)->sin_addr.s_addr = buffer.localAddr.s_addr;
	((sockaddr_in*)addr)->sin_port = BigShort(buffer.localPort);

	return 0;
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::GetNameFromAddr
//
//==========================================================================

VStr VBeameWhitesideDriver::GetNameFromAddr(sockaddr_t* addr)
{
	guard(VBeameWhitesideDriver::GetNameFromAddr);
	return AddrToString(addr);
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::GetAddrFromName
//
//==========================================================================

int VBeameWhitesideDriver::GetAddrFromName(const char* name, sockaddr_t* hostaddr)
{
	guard(VBeameWhitesideDriver::GetAddrFromName);
	char buff[MAXHOSTNAMELEN];
	char *b;
	int addr;
	int num;
	int mask;
	int run;
	int port;

	if (name[0] < '0' || name[0] > '9')
		return -1;

	buff[0] = '.';
	b = buff;
	VStr::Cpy(buff + 1, name);
	if (buff[1] == '.')
		b++;

	addr = 0;
	mask = -1;
	while (*b == '.')
	{
		b++;
		num = 0;
		run = 0;
		while (!( *b < '0' || *b > '9'))
		{
			num = num * 10 + *b++ - '0';
			if (++run > 3)
				return -1;
		}
		if ((*b < '0' || *b > '9') && *b != '.' && *b != ':' && *b != 0)
			return -1;
		if (num < 0 || num > 255)
			return -1;
		mask<<=8;
		addr = (addr<<8) + num;
	}
	addr = BigLong(addr);
	mask = BigLong(mask);

	if (*b++ == ':')
		port = atoi(b);
	else
		port = Net->HostPort;

	hostaddr->sa_family = AF_INET;
	((sockaddr_in*)hostaddr)->sin_port = BigShort((short)port);
	((sockaddr_in*)hostaddr)->sin_addr.s_addr =
	((ethdevinfo.inetAddr & mask) | addr);

	return 0;
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::AddrCompare
//
//==========================================================================

int VBeameWhitesideDriver::AddrCompare(sockaddr_t* addr1, sockaddr_t* addr2)
{
	guard(VBeameWhitesideDriver::AddrCompare);
	if (addr1->sa_family != addr2->sa_family)
		return -1;

	if (((sockaddr_in*)addr1)->sin_addr.s_addr != ((sockaddr_in*)addr2)->sin_addr.s_addr)
		return -1;

	if (((sockaddr_in*)addr1)->sin_port != ((sockaddr_in*)addr2)->sin_port)
		return 1;

	return 0;
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::GetSocketPort
//
//==========================================================================

int VBeameWhitesideDriver::GetSocketPort(sockaddr_t* addr)
{
	guard(VBeameWhitesideDriver::GetSocketPort);
	return BigShort(((sockaddr_in*)addr)->sin_port);
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::SetSocketPort
//
//==========================================================================

int VBeameWhitesideDriver::SetSocketPort(sockaddr_t* addr, int port)
{
	guard(VBeameWhitesideDriver::SetSocketPort);
	((sockaddr_in*)addr)->sin_port = BigShort(port);
	return 0;
	unguard;
}

//==========================================================================
//
//  VBeameWhitesideDriver::dos_int86
//
//	Returns 0 on success
//
//==========================================================================

int VBeameWhitesideDriver::dos_int86(int vec, __dpmi_regs* regs)
{
	guardSlow(VBeameWhitesideDriver::dos_int86);
	regs->x.ss = regs->x.sp = 0;
	int rc = _go32_dpmi_simulate_int(vec, regs);
	return rc || (regs->x.flags & 1);
	unguardSlow;
}

//==========================================================================
//
//  VBeameWhitesideDriver::BW_ioctl
//
//==========================================================================

int VBeameWhitesideDriver::BW_ioctl(int s, char *msg, int msglen)
{
	guardSlow(VBeameWhitesideDriver::BW_ioctl);
	dosmemput(msg, msglen, __tb);

	regs.x.ax = 0x4403;
	regs.x.bx = s;
	regs.x.cx = msglen;
	regs.x.dx = __tb & 0xf;
	regs.x.ds = __tb >> 4;
	if (dos_int86(0x21, &regs))
		return regs.x.ax;
	return 0;
	unguardSlow;
}

//==========================================================================
//
//  VBeameWhitesideDriver::BW_TranslateError
//
//==========================================================================

int VBeameWhitesideDriver::BW_TranslateError(int error)
{
	guardSlow(VBeameWhitesideDriver::BW_TranslateError);
	switch (error)
	{
	case BW_ERR_USR_HANGUP:	return ECONNABORTED;
	case BW_ERR_HANGUP:		return EISCONN;
	case BW_ERR_NET_ERR:	return ENOTCONN;
	case BW_ERR_IS_CLOSED:	return ENOTCONN;
	case BW_ERR_TIME_OUT:	return ETIMEDOUT;
	case BW_ERR_RESET:		return ECONNREFUSED;
	case BW_ERR_FULL:		return ETOOMANYREFS;
	case BW_ERR_BLOCK:		return EWOULDBLOCK;
	case BW_ERR_SHUTDOWN:	return ESHUTDOWN;
	}
	return EIO;
	unguardSlow;
}

//==========================================================================
//
//  VBeameWhitesideDriver::GetEthdevinfo
//
//==========================================================================

int VBeameWhitesideDriver::GetEthdevinfo()
{
	guard(VBeameWhitesideDriver::GetEthdevinfo);
	int fd;

	dosmemput("ETHDEV27", 9, __tb);
	regs.x.ax = 0x3d42;
	regs.x.ds = __tb >> 4;
	regs.x.dx = __tb & 0xf;
	if (dos_int86(0x21, &regs))
		return -1;
	fd = regs.x.ax;

	regs.x.ax = 0x4401;
	regs.x.bx = fd;
	regs.x.dx = 0x60;
	dos_int86(0x21, &regs);

	regs.h.ah = 0x3f;
	regs.x.cx = sizeof(ethdevinfo);
	regs.x.es = regs.x.ds = __tb >> 4;
	regs.x.dx = __tb & 0xf;
	regs.x.bx = fd;
	if (dos_int86(0x21, &regs))
		return -1;
	dosmemget(__tb, regs.x.ax, &ethdevinfo);

	regs.h.ah = 0x3e;
	regs.x.bx = fd;
	dos_int86(0x21, &regs);

	return 0;
	unguard;
}
