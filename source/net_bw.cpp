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
#include "net_bw.h"

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
		struct { byte s_b1, s_b2, s_b3, s_b4; } S_un_b;
		struct { word s_w1, s_w2; } S_un_w;
		dword S_addr;
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
    short		sin_family;
    word		sin_port;
	in_addr		sin_addr;
    char		sin_zero[8];
};

#pragma pack(1)

typedef struct
{
	char			state;			// always 1
	short			localPort;
	in_addr			localAddr;
	char			reason;			// always 0
	char			options;
	short			dataAvailable;
} BW_UDPinfo_t;

typedef struct
{
	char			reserved1[6];
	word			info2Offset;
	char			reserved2[18];
	in_addr			remoteAddr;
} BW_UDPreadInfo1_t;

typedef struct
{
	short			remotePort;
	char			reserved1[2];
	word			dataLenPlus8;
	char			reserved2[2];
	char			data[1];			// actual size is <dataLenPlus8> - 8		
} BW_UDPreadInfo2_t;

typedef struct
{
	char			reserved1[2];
	short			remotePort;
	word			dataLen;
	in_addr			remoteAddr;
	char			reserved2[42];
	char			data[1];			// actual size is <datalen>
} BW_writeInfo_t;

typedef struct
{
	short	ioport;
	byte	dma;
	byte	vector;
	byte	irq;
	short	bufferSize;
	short	maxWindow;
	short	timeZone;
	byte	myType;
	int		inetAddr;
	short	value;
	byte	subnetMask;
	short	etherPointer;
	short	logserverPointer;
	short	nameserverPointer;
	short	printserverPointer;
	short	timeserverPointer;
	short	gatewayPointer;
	short	driverSegment;
	byte	transferSize;
	char	cardName[9];
} BW_ethdevinfo_t;

#pragma pack()

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static BW_ethdevinfo_t	ethdevinfo;
static int				netmask;
static in_addr 			bcastaddr;

static __dpmi_regs		regs;

static int				net_acceptsocket = -1;	// socket for fielding new connections
static int				net_controlsocket = 0;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  dos_int86
//
//	Returns 0 on success
//
//==========================================================================

static int dos_int86(int vec, __dpmi_regs *)
{
    int rc;
    regs.x.ss = regs.x.sp = 0;
    rc = _go32_dpmi_simulate_int(vec, &regs);
    return rc || (regs.x.flags & 1);
}

//==========================================================================
//
//  BW_ioctl
//
//==========================================================================

static int BW_ioctl(int s, char *msg, int msglen)
{
	dosmemput(msg, msglen, __tb);

	regs.x.ax = 0x4403;
	regs.x.bx = s;
	regs.x.cx = msglen;
	regs.x.dx = __tb & 0xf;
	regs.x.ds = __tb >> 4;
	if (dos_int86(0x21, &regs))
		return regs.x.ax;
	return 0;
}

//==========================================================================
//
//  BW_TranslateError
//
//==========================================================================

static int BW_TranslateError(int error)
{
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
}

//==========================================================================
//
//  GetEthdevinfo
//
//==========================================================================

static int GetEthdevinfo(void)
{
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
}

//==========================================================================
//
//  BW_Init
//
//==========================================================================

int BW_Init(void)
{
	sockaddr_t	addr;
	char		*colon;

	if (M_CheckParm("-noudp"))
		return -1;

	if (GetEthdevinfo())
	{
		cond << "Beame & Whiteside TCP/IP not detected\n";
		return -1;
	}
	netmask = 0xffffffff >> (32 - ethdevinfo.subnetMask);
	bcastaddr.s_addr = (ethdevinfo.inetAddr & netmask) | (~netmask);

	if ((net_controlsocket = BW_OpenSocket(0)) == -1)
	{
		cond << "BW_Init unable to open control socket; disabled\n";
		return -1;
	}

	BW_GetSocketAddr(net_controlsocket, &addr);
	strcpy(my_tcpip_address, BW_AddrToString(&addr));
	colon = strrchr(my_tcpip_address, ':');
	if (colon)
		*colon = 0;

	con << "BW_Init: UDP initialized\n";
	tcpipAvailable = true;

	return net_controlsocket;
}

//==========================================================================
//
//  BW_Shutdown
//
//==========================================================================

void BW_Shutdown(void)
{
	BW_Listen(false);
	BW_CloseSocket(net_controlsocket);
}

//==========================================================================
//
//  BW_Listen
//
//==========================================================================

void BW_Listen(boolean state)
{
	if (state)
	{
		// enable listening
		if (net_acceptsocket == -1)
        {
            net_acceptsocket = BW_OpenSocket(net_hostport);
			if (net_acceptsocket == -1)
				Sys_Error("BW_Listen: Unable to open accept socket\n");
		}
	}
	else
	{
		// disable listening
		if (net_acceptsocket != -1)
		{
			BW_CloseSocket(net_acceptsocket);
			net_acceptsocket = -1;
		}
	}
}

//==========================================================================
//
//  BW_OpenSocket
//
//  OpenSocket returns a handle to a network socket that has been opened,
// set to nonblocking, and bound to <port>.  Additional socket options
// should be set here if they are needed.  -1 is returned on failure.
//
//==========================================================================

int BW_OpenSocket(int port)
{
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
		con << "BW_OpenSocket failed: " << BW_TranslateError(regs.x.ax) << endl;
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
		con << "BW_OpenSocket ioctl(reuse) failed\n";
		return -1;
	}

	if (BW_ioctl(s, nonblock_msg, 2))
	{
		con << "BW_OpenSocket ioctl(nonblocking) failed\n";
		return -1;
	}

	// if a socket was specified, bind to it and return
	if (port)
	{
		*(short *)&bind_msg[1] = port;
		if (BW_ioctl(s, bind_msg, 3))
		{
			BW_CloseSocket(s);
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
}

//==========================================================================
//
//  BW_CloseSocket
//
//==========================================================================

int BW_CloseSocket(int socket)
{
	regs.h.ah = 0x3e;
	regs.x.bx = socket;
	if (dos_int86(0x21, &regs))
	{
		con << "BW_CloseSocket " << socket << " failed: " << BW_TranslateError(regs.x.ax) << endl;
		return -1;
	}
	return 0;
}

//==========================================================================
//
//  BW_Connect
//
//==========================================================================

int BW_Connect(int, sockaddr_t *)
{
	return 0;
}

//==========================================================================
//
//  BW_CheckNewConnections
//
//==========================================================================

int BW_CheckNewConnections(void)
{
	if (net_acceptsocket == 0)
		return -1;

	// see if there's anything waiting
	regs.x.ax = 0x4406;
	regs.x.bx = net_acceptsocket;
	dos_int86(0x21, &regs);
	if (regs.x.ax == 0)
		return -1;
	return net_acceptsocket;
}

//==========================================================================
//
//  BW_Read
//
//==========================================================================

int BW_Read(int s, byte *buf, int len, sockaddr_t *from)
{
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
		con << "BW UDP read error: " << BW_TranslateError(regs.x.ax) << endl;
		return -1;
	}

	byte buffer[LOWMEM_SIZE];
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
		con << "BW UDP read packet too large: " << copylen << endl;
		return -1;
	}
	memcpy(buf, info2->data, copylen);

	return copylen;
}

//==========================================================================
//
//  BW_Write
//
//==========================================================================

int BW_Write(int s, byte *msg, int len, sockaddr_t *to)
{
	BW_writeInfo_t *writeInfo;

	// ask if we're clear to send
	regs.x.ax = 0x4407;
	regs.x.bx = s;
	dos_int86(0x21, &regs);
	if (regs.x.ax == 0)
		return 0;

	// yes, let's do it
	byte buffer[LOWMEM_SIZE];
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
		con << "BW_Write failed: " << BW_TranslateError(regs.x.ax) << endl;
		return -1;
	}

	return len;
}

//==========================================================================
//
//  BW_Broadcast
//
//==========================================================================

int BW_Broadcast(int s, byte *msg, int len)
{
	BW_writeInfo_t *writeInfo;

	// ask if we're clear to send
	regs.x.ax = 0x4407;
	regs.x.bx = s;
	dos_int86(0x21, &regs);
	if (regs.x.ax == 0)
		return 0;

	// yes, let's do it
    byte buffer[LOWMEM_SIZE];
	writeInfo = (BW_writeInfo_t *)buffer;
	writeInfo->remoteAddr = bcastaddr;
	writeInfo->remotePort = net_hostport;
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
		con << "BW_Broadcast failed: " << BW_TranslateError(regs.x.ax) << endl;
		return -1;
	}

	return len;
}

//==========================================================================
//
//  BW_AddrToString
//
//==========================================================================

char *BW_AddrToString(sockaddr_t *addr)
{
	static char buffer[22];

	sprintf(buffer, "%d.%d.%d.%d:%d",
		((sockaddr_in *)addr)->sin_addr.s_net,
		((sockaddr_in *)addr)->sin_addr.s_host,
		((sockaddr_in *)addr)->sin_addr.s_lh,
		((sockaddr_in *)addr)->sin_addr.s_impno,
		(word)BigShort(((sockaddr_in *)addr)->sin_port)
		);
	return buffer;
}

//==========================================================================
//
//  BW_StringToAddr
//
//==========================================================================

int BW_StringToAddr(char *string, sockaddr_t *addr)
{
	int ha1, ha2, ha3, ha4, hp;
	int ipaddr;

	sscanf(string, "%d.%d.%d.%d:%d", &ha1, &ha2, &ha3, &ha4, &hp);
	ipaddr = (ha1 << 24) | (ha2 << 16) | (ha3 << 8) | ha4;

	addr->sa_family = AF_INET;
	((sockaddr_in *)addr)->sin_addr.s_addr = BigLong(ipaddr);
	((sockaddr_in *)addr)->sin_port = BigShort((short)hp);
	return 0;
}

//==========================================================================
//
//  BW_GetSocketAddr
//
//==========================================================================

int BW_GetSocketAddr(int socket, sockaddr_t *addr)
{
	regs.x.ax = 0x4402;
	regs.x.bx = socket;
	regs.x.cx = sizeof(BW_UDPinfo_t);
	regs.x.dx = __tb & 0xf;
	regs.x.ds = __tb >> 4;
	dos_int86(0x21, &regs);

    BW_UDPinfo_t	buffer;
    dosmemget(__tb, sizeof(buffer), &buffer);
	addr->sa_family = AF_INET;
	((sockaddr_in *)addr)->sin_addr.s_addr = buffer.localAddr.s_addr;
	((sockaddr_in *)addr)->sin_port = BigShort(buffer.localPort);

	return 0;
}

//==========================================================================
//
//  BW_GetNameFromAddr
//
//==========================================================================

int BW_GetNameFromAddr(sockaddr_t *addr, char *name)
{
	strcpy(name, BW_AddrToString(addr));
	return 0;
}

//==========================================================================
//
//  BW_GetAddrFromName
//
//==========================================================================

int BW_GetAddrFromName(char *name, sockaddr_t *hostaddr)
{
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
	strcpy(buff + 1, name);
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
		  num = num*10 + *b++ - '0';
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
		port = net_hostport;

	hostaddr->sa_family = AF_INET;
	((sockaddr_in *)hostaddr)->sin_port = BigShort((short)port);
	((sockaddr_in *)hostaddr)->sin_addr.s_addr =
	((ethdevinfo.inetAddr & mask) | addr);

	return 0;
}

//==========================================================================
//
//  BW_AddrCompare
//
//==========================================================================

int BW_AddrCompare(sockaddr_t *addr1, sockaddr_t *addr2)
{
	if (addr1->sa_family != addr2->sa_family)
		return -1;

	if (((sockaddr_in *)addr1)->sin_addr.s_addr != ((sockaddr_in *)addr2)->sin_addr.s_addr)
		return -1;

	if (((sockaddr_in *)addr1)->sin_port != ((sockaddr_in *)addr2)->sin_port)
		return 1;

	return 0;
}

//==========================================================================
//
//  BW_GetSocketPort
//
//==========================================================================

int BW_GetSocketPort(sockaddr_t *addr)
{
	return BigShort(((sockaddr_in *)addr)->sin_port);
}

//==========================================================================
//
//  BW_SetSocketPort
//
//==========================================================================

int BW_SetSocketPort(sockaddr_t *addr, int port)
{
	((sockaddr_in *)addr)->sin_port = BigShort(port);
	return 0;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
