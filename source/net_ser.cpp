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
#include "net_ser.h"
#include "comport.h"

// MACROS ------------------------------------------------------------------

#define MAXPACKET	512
#define	FRAMECHAR	0x70

#define NFLAG_ACK		0x10000000

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void CheckAbort(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TComPort	*ComPort;

static char		startup[256], shutdown[256], baudrate[256];

static char		packet[MAXPACKET];
static int		packetlen;
static int		inescape;
static int		newpacket;

static int		uart = -1;
static int		irq = -1;
static int		baud = -1;
static int		lineCtl = -1;
static int		uartType = -1;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  write_buffer
//
//==========================================================================

static void write_buffer( char *buffer, unsigned int count )
{
	// if this would overrun the buffer, throw everything else out
//	if (FULL(ComPort->OutputQueue))
//		ComPort->OutputQueue.tail = ComPort->OutputQueue.head;

	while (count--)
		ComPort->WriteByte(*buffer++);

	ComPort->Flush();
}

//==========================================================================
//
//  ReadPacket
//
//==========================================================================

static bool ReadPacket(void)
{
	int	c;

// if the buffer has overflowed, throw everything out

/*	if (FULL(ComPort->InputQueue)	// check for buffer overflow
	{
		ComPort->InputQueue.tail = ComPort->InputQueue.head = 0;
		newpacket = true;
		return false;
	}*/

	if (newpacket)
	{
		packetlen = 0;
		newpacket = 0;
	}

	do
	{
		c = ComPort->ReadByte();
		if (c < 0)
			return false;		// haven't read a complete packet
		if (inescape)
		{
			inescape = false;
			if (c!=FRAMECHAR)
			{
				newpacket = 1;
				return true;	// got a good packet
			}
		}
		else if (c==FRAMECHAR)
		{
			inescape = true;
			continue;			// don't know yet if it is a terminator
		}						// or a literal FRAMECHAR

		if (packetlen >= MAXPACKET)
			continue;			// oversize packet
		packet[packetlen] = c;
		packetlen++;
	} while (1);

}

//==========================================================================
//
//  WritePacket
//
//==========================================================================

static void WritePacket(char *buffer, int len)
{
	int				b;
	static char		localbuffer[MAXPACKET*2+2];

	b = 0;
	if (len > MAXPACKET)
		return;

	while (len--)
	{
		if (*buffer == FRAMECHAR)
			localbuffer[b++] = FRAMECHAR;	// escape it for literal
		localbuffer[b++] = *buffer++;
	}

	localbuffer[b++] = FRAMECHAR;
	localbuffer[b++] = 0;

	write_buffer (localbuffer, b);
}

//==========================================================================
//
//	Ser_Connect
//
//  Figures out who is player 0 and 1
//
//==========================================================================

void Ser_Connect(void)
{
	struct time	time;
	int			oldsec;
	int			localstage, remotestage;
	char		str[20];
	char		idstr[7];
	char		remoteidstr[7];
	unsigned long		idnum;
	int			i;
	
	//
	// wait for a good packet
	//
	con << "Attempting to connect across serial link, press escape to abort.\n";

	//
	// allow override of automatic player ordering to allow a slower computer
	// to be set as player 1 allways
	//

	//
	// build a (hopefully) unique id string by hashing up the current
	// milliseconds and the interrupt table
	//
	if (M_CheckParm ("-player1"))
		idnum = 0;
	else if (M_CheckParm ("-player2"))
		idnum = 999999;
	else
	{
		gettime (&time);
		idnum = time.ti_sec*100+time.ti_hund;
		for (i=0 ; i<512 ; i++)
			idnum += ((unsigned short*)0)[i];
		idnum %= 1000000;
	}
	
	idstr[0] = '0' + idnum/ 100000l;
	idnum -= (idstr[0]-'0')*100000l;
	idstr[1] = '0' + idnum/ 10000l;
	idnum -= (idstr[1]-'0')*10000l;
	idstr[2] = '0' + idnum/ 1000l;
	idnum -= (idstr[2]-'0')*1000l;
	idstr[3] = '0' + idnum/ 100l;
	idnum -= (idstr[3]-'0')*100l;
	idstr[4] = '0' + idnum/ 10l;
	idnum -= (idstr[4]-'0')*10l;
	idstr[5] = '0' + idnum;
	idstr[6] = 0;
	
	//
	// sit in a loop until things are worked out
	//
	// the packet is:  ID000000_0
	// the first field is the idnum, the second is the acknowledge stage
	// ack stage starts out 0, is bumped to 1 after the other computer's id
	// is known, and is bumped to 2 after the other computer has raised to 1
	//
	oldsec = -1;
	localstage = remotestage = 0;

	do
	{
    	CheckAbort();

		if (ReadPacket ())
		{
			packet[packetlen] = 0;
			con << "read : " << packet << endl;
			if (packetlen != 10)
				continue;
			if (strncmp(packet,"ID",2) )
				continue;
			if (!strncmp (packet+2,idstr,6))
				I_Error ("\n\nDuplicate id string, try again or check modem init string.");
			strncpy (remoteidstr,packet+2,6);

			remotestage = packet[9] - '0';
			localstage = remotestage+1;
			oldsec = -1;
		}

		gettime (&time);
		if (time.ti_sec != oldsec)
		{
			oldsec = time.ti_sec;
			sprintf (str,"ID%s_%i",idstr,localstage);
			WritePacket (str,strlen(str));
			con << "wrote: " << str << endl;
		}

	} while (localstage < 2);

	//
	// decide who is who
	//
#ifdef FIXME
	if (strcmp(remoteidstr, idstr) > 0)
		consoleplayer = 0;
	else
		consoleplayer = 1;
#endif

	//
	// flush out any extras
	//
	while (ReadPacket());
}

//==========================================================================
//
//  ReadLine
//
//==========================================================================

static void ReadLine(FILE *f, char *dest)
{
	int	c;

	do
	{
		c = fgetc (f);
		if (c == EOF || c == '\r' || c == '\n')
			break;
		*dest++ = c;
	} while (1);
	*dest = 0;
}

//==========================================================================
//
//  ReadModemCfg
//
//==========================================================================

static void ReadModemCfg(void)
{
	FILE		*f;
	unsigned	modem_baud;

	f = fopen("modem.cfg","r");
	if (!f)
	{
		//	Use default settings
		ComPort->SetModemConfig(true, NULL, NULL, NULL, NULL);
		con << "Couldn't read MODEM.CFG\n";
		return;
	}
	ReadLine(f, startup);
	ReadLine(f, shutdown);
	ReadLine(f, baudrate);
	fclose(f);

	modem_baud = atol(baudrate);
	if (modem_baud)
		baud = modem_baud;

	ComPort->SetModemConfig(true, NULL, "", startup, shutdown);
}

//==========================================================================
//
//  Dial
//
//==========================================================================

static void Dial(void)
{
	int		p;

	con << "\nDialing...\n\n";
	p = M_CheckParm ("-dial");
	if (ComPort->Connect(myargv[p + 1]))
    	I_Error("Dial failed\n");

#ifdef FIXME
	consoleplayer = 1;
#endif
}

//==========================================================================
//
//  Answer
//
//==========================================================================

static void Answer(void)
{
	con << "\nWaiting for ring...\n\n";

    while (!ComPort->CheckForConnection())
    	CheckAbort();

#ifdef FIXME
	consoleplayer = 0;
#endif
}

//==========================================================================
//
//  Serial_Init
//
//==========================================================================

void Serial_Init(void)
{
	//
	// set network characteristics
	//
	num_net_players = 2;

	int			i;

	//
	// find the irq and io address of the port
	//
	int	   		comport;
	if (M_CheckParm ("-com2"))
		comport = 2;
	else if (M_CheckParm ("-com3"))
		comport = 3;
	else if (M_CheckParm ("-com4"))
		comport = 4;
	else
		comport = 1;
	ComPort = new TComPort(comport - 1);

	//
	// establish communications
	//

    //	Modem config
	if (M_CheckParm ("-dial") || M_CheckParm ("-answer") )
		ReadModemCfg ();		// may set baudbits

    //	Uart
	i = M_CheckParm("-uart");
    if (i && i < myargc - 1)
    	uart = atoi(myargv[i + 1]);

    //	IRQ
	i = M_CheckParm("-irq");
    if (i && i < myargc - 1)
    	irq = atoi(myargv[i + 1]);

	//
	// allow command-line override of modem.cfg baud rate
	//
	if (M_CheckParm ("-9600")) baud = 9600;
	else if (M_CheckParm ("-14400")) baud = 14400;
	else if (M_CheckParm ("-19200")) baud = 19200;
	else if (M_CheckParm ("-38400")) baud = 38400;
	else if (M_CheckParm ("-57600")) baud = 57600;
	else if (M_CheckParm ("-115200")) baud = 115200;

	//	allow a forced 8250
	if (M_CheckParm("-8250"))
		uartType = UART_8250;

	ComPort->SetConfig(uart, irq, baud, lineCtl, uartType);

    ComPort->Init();

	if (M_CheckParm ("-dial"))
		Dial();
	else if (M_CheckParm ("-answer"))
		Answer();

	Ser_Connect();
}

//==========================================================================
//
//  NetbufferChecksum
//
//==========================================================================

static word NetbufferChecksum(const byte *buf, int len)
{
	TCRC	crc;

	crc.Init();

	for (int i = 0; i < len; i++)
	{
		crc + buf[i];
	}

	return crc;
}

//==========================================================================
//
//	Serial_SendMessage
//
//==========================================================================

int Serial_SendMessage(qsocket_t *sock, TSizeBuf *message)
{
	if (!sock->canSend)
    {
		I_Error("Can't send\n");
	}

    memcpy(sock->sendMessage, message->Data, message->CurSize);
    sock->sendMessageLength = message->CurSize;

/*	cond << "Send [";
    for (int i = 0; i < message->CurSize; i++)
    	cond << (int)message->Data[i] << ' ';
	cond << "]\n";*/

	word		crc;
	byte		buf[MAX_MSGLEN];
    TMessage	msg(buf, MAX_MSGLEN);

	crc = NetbufferChecksum(message->Data, message->CurSize);

    msg << (int)BigLong(((message->CurSize + 8) << 16) | crc)
    	<< (int)BigLong(sock->sendSequence);
	msg.Write(message->Data, message->CurSize);

	WritePacket((char*)msg.Data, msg.CurSize);

    sock->lastSendTime = Sys_Time();
	sock->canSend = false;
	sock->sendSequence++;
	return 1;
}

//==========================================================================
//
//	Serial_SendUnreliableMessage
//
//==========================================================================

int Serial_SendUnreliableMessage(qsocket_t *sock, TSizeBuf *message)
{
/*	cond << "Send [";
    for (int i = 0; i < message->CurSize; i++)
    	cond << (int)message->Data[i] << ' ';
	cond << "]\n";*/

	word		crc;
	byte		buf[MAX_MSGLEN];
    TMessage	msg(buf, MAX_MSGLEN);

	crc = NetbufferChecksum(message->Data, message->CurSize);

    msg << (int)BigLong(((message->CurSize + 8) << 16) | crc)
    	<< (int)BigLong(sock->unreliableSendSequence++);
	msg.Write(message->Data, message->CurSize);

	WritePacket((char*)msg.Data, msg.CurSize);
	return 1;
}

//==========================================================================
//
//	ResendSer
//
//==========================================================================

static void ResendSer(qsocket_t *sock)
{
	word	crc;
   	int		len;

	len = sock->sendMessageLength;

	byte		buf[MAX_MSGLEN];
    TMessage	msg(buf, MAX_MSGLEN);

	crc = NetbufferChecksum(sock->sendMessage, len);

    msg << (int)BigLong(((len + 8) << 16) | crc)
    	<< (int)BigLong(sock->sendSequence - 1);
    msg.Write(sock->sendMessage, len);

	WritePacket((char*)msg.Data, msg.CurSize);

    sock->lastSendTime = Sys_Time();
}

//==========================================================================
//
//	Serial_GetMessage
//
//==========================================================================

int Serial_GetMessage(qsocket_t *sock)
{
	byte		buf[MAX_MSGLEN];
    TMessage	msg(buf, MAX_MSGLEN);
	int		len;
    dword	seq;
	int		flags;
	int		crc;

	if (!sock->canSend && Sys_Time() - sock->lastSendTime > 0.5)
		ResendSer(sock);

 try_again:

	if (ReadPacket () && packetlen <= msg.MaxSize)
	{
    	msg.Clear();
		msg.Write(&packet, packetlen);
	}
	else
	{
		return 0;
	}

	msg.BeginReading();
    msg >> len >> seq;
    len = BigLong(len);
    seq = BigLong(seq);
	flags = len & 0xf0000000;
	crc = len & 0x0000ffff;
	len = (len & 0x0fff0000) >> 16;

	if (msg.CurSize != len)
    {
		cond << "Bad len\n";
    	goto try_again;
	}
    len -= 8;

    if (flags & NFLAG_ACK)
    {
		if (seq != sock->sendSequence - 1)
		{
			cond << "Stale ACK received\n";
			goto try_again;
		}
		if (seq == sock->ackSequence)
		{
			sock->ackSequence++;
			if (sock->ackSequence != sock->sendSequence)
				cond << "ack sequencing error\n";
		}
		else
		{
			cond << "Duplicate ACK received\n";
			goto try_again;
		}

        sock->sendMessageLength = 0;
		sock->canSend = true;

		goto try_again;
    }

	word buf_crc = NetbufferChecksum(msg.Data + 8, len);
    if (buf_crc != crc)
    {
    	cond << "bad packet checksum " << buf_crc << ' ' << crc << endl;
		goto try_again;
    }

	net_msg.Clear();
	net_msg.Write(msg.Data + 8, len);

    msg.Clear();
    msg << (int)BigLong(NFLAG_ACK | (8 << 16))
    	<< (int)BigLong(seq);
	WritePacket((char*)msg.Data, msg.CurSize);

	if (seq != sock->receiveSequence)
	{
		cond << "Invalid rec seq " << seq << " " << sock->receiveSequence << endl;
		goto try_again;
	}
	sock->receiveSequence++;
	sock->lastMessageTime = Sys_Time();
	return 1;
}

//==========================================================================
//
//	Serial_CanSendMessage
//
//==========================================================================

boolean Serial_CanSendMessage(qsocket_t *sock)
{
	return sock->canSend;
}

//==========================================================================
//
//	Serial_CanSendUnreliableMessage
//
//==========================================================================

boolean Serial_CanSendUnreliableMessage(qsocket_t *)
{
	return true;
}

//==========================================================================
//
//  Serial_Shutdown
//
//==========================================================================

void Serial_Shutdown(void)
{
	ComPort->Close();
	
	delete ComPort;
}

qsocket_t *Serial_Connect(char *) { return NULL; }

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
