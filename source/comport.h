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

#ifndef _COMPORT_H
#define _COMPORT_H

// HEADER FILES ------------------------------------------------------------

#include <dpmi.h>
#include <dos.h>

// MACROS ------------------------------------------------------------------

//	4 standarta porti un 4 TC-212 kartes paplaýinÆtie porti
#define NUM_COM_PORTS	8

//
//	Porta reÖistru ofseti un bitu flagi
//
// 8250, 16550 definitions
#define TRANSMIT_HOLDING_REGISTER			0x00//Write: 8 bits of character to be sent.
#define RECEIVE_BUFFER_REGISTER				0x00//Read:  8 bits of character received.
#define INTERRUPT_ENABLE_REGISTER			0x01//Write: interrupt enable register
#define   IER_RX_DATA_READY					0x01//enable interrupt when rec'd data is available
#define   IER_TX_HOLDING_REGISTER_EMPTY		0x02//enable interrupt when transmit buffer is empty
#define   IER_LINE_STATUS					0x04//enable int on rec'r line status (err or break)
#define   IER_MODEM_STATUS					0x08//enable int on modem status (CTS,DSR,RI,RLSD)
#define DIVISOR_LATCH_LOW					0x00//Write: (if DLAB=1) divisor latch low byte
#define DIVISOR_LATCH_HIGH					0x01//Write: divisor latch high byte (when DLAB=1)
#define INTERRUPT_ID_REGISTER				0x02//Read: interrupt identification reg.  When an interrupt occurs, read this register to find what caused it.
#define   IIR_NO_INTERRUPT					0x01//no interrupt pending; can be used in polling
#define   IIR_MODEM_STATUS_INTERRUPT		0x00//modem status.  Occurs upon: Clear To Send, Data Set Ready, Ring Ind, or Rec'd Line Signal Detect. Reset by reading modem status (port pb+6).
#define   IIR_TX_HOLDING_REGISTER_INTERRUPT	0x02//transmitter buffer empty. Reset by writing transmitter buffer (pb+0)
#define   IIR_RX_DATA_READY_INTERRUPT		0x04//received data available. Reset by reading receiver buffer (port pb+0)
#define   IIR_LINE_STATUS_INTERRUPT			0x06//receiver line status interrupt.  Occurs on: overrun, parity, or framing error, or break. Reset by reading line status (port pb+5)
#define   IIR_FIFO_TIMEOUT					0x0c//(16550 UARTs) 1=Receiver FIFO time-out
#define   IIR_FIFO_ENABLED					0xc0//(16550 UARTs) 00=FIFOs disabled (or old 8250), 11=FIFOs enabled, 01=FIFOs enabled and DMA mode
#define FIFO_CONTROL_REGISTER				0x02//Write: (16550 UARTs) FIFO control register (write only)
#define   FCR_FIFO_ENABLE					0x01//enable FIFO mode
#define   FCR_RCVR_FIFO_RESET				0x02//clear receiver FIFO
#define   FCR_XMIT_FIFO_RESET				0x04//clear transmit FIFO
#define   FCR_DMA_MODE						0x08//DMA mode select
#define   FCR_TRIGGER_01					0x00//FIFO interrupt triggger level: 1 byte;
#define   FCR_TRIGGER_04					0x40//4 bytes
#define   FCR_TRIGGER_08					0x80//8 bytes
#define   FCR_TRIGGER_16					0xc0//16 bytes
#define LINE_CONTROL_REGISTER				0x03//Read/Write: line control register
#define   LCR_DATA_BITS_5					0x00//word length: 5
#define   LCR_DATA_BITS_6					0x01//word length: 6
#define   LCR_DATA_BITS_7					0x02//word length: 7
#define   LCR_DATA_BITS_8					0x03//word length: 8
#define   LCR_STOP_BITS_1					0x00//stop bits: 1
#define   LCR_STOP_BITS_2					0x04//stop bits: 2 (some oddball exceptions)
#define   LCR_PARITY_NONE					0x00//parity: None
#define   LCR_PARITY_ODD					0x08//parity: Odd
#define   LCR_PARITY_EVEN					0x18//parity: Even
#define   LCR_PARITY_MARK					0x28//stuck parity  (not used by BIOS)
#define   LCR_PARITY_SPACE					0x38
#define   LCR_SET_BREAK						0x40//enable break control. 1=start sending 0s (spcs)
#define   LCR_DLAB							0x80//DLAB (Divisor Latch Access Bit)  Determines mode of ports pb+1 and pb+2.  1=set baud, 0=normal
#define MODEM_CONTROL_REGISTER				0x04//Write: modem control register
#define   MCR_DTR							0x01//activate -DTR (-data trmnl rdy)
#define   MCR_RTS							0x02//activate -RTS (-request to send)
#define   MCR_OUT1							0x04//activate -OUT1 (spare, user-designated output)
#define   MCR_OUT2							0x08//activate -OUT2
#define   MCR_LOOPBACK						0x10//activate loopback for diagnostic testing
#define LINE_STATUS_REGISTER				0x05//Read: line status register. Note: bits 1-4 cause interrupt if enabled (pb+1)
#define   LSR_DATA_READY					0x01//data ready (DR). Reset by reading recv'r buffer
#define   LSR_OVERRUN_ERROR					0x02//overrun error (OE).  Previous character is lost
#define   LSR_PARITY_ERROR					0x04//parity error (PE). Reset by reading line status
#define   LSR_FRAMING_ERROR					0x08//framing error (FE). Bad stop bit in character
#define   LSR_BREAK_DETECT					0x10//break indicated (BI).  Sustained space received
#define   LSR_TRANSMITTER_BUFFER_EMPTY		0x20//transmitter holding register empty.  OK to send
#define   LSR_TRANSMITTER_EMPTY				0x40//transmitter empty.  No data being processed.
#define   LSR_FIFO_DIRTY					0x80//(16450 UARTs) Receiver FIFO error
#define MODEM_STATUS_REGISTER				0x06//Read: modem status register. Note: bits 0-3 cause an interrupt if enabled (pb+1)
#define   MSR_DELTA_CTS						0x01//Delta Clear To Send (DCTS) has changed state
#define   MSR_DELTA_DSR						0x02//Delta Data Set Ready (DDSR) has changed state
#define   MSR_DELTA_RI						0x04//Trailing Edge Ring Indicator (TERI) is active
#define   MSR_DELTA_CD						0x08//Delta Data Carrier Detect (DDCD) has changed
#define   MSR_CTS							0x10//Clear To Send (CTS) is active
#define   MSR_DSR							0x20//Data Set Ready (DSR) is active
#define   MSR_RI							0x40//Ring Indicator (RI) is active
#define   MSR_CD							0x80//Data Carrier Detect (DCD) is active
#define SCRATCH_PAD_REGISTER				0x07//Read/Write scratch pad

//	NolasØýnas kõÝdas
#define ERR_TTY_LINE_STATUS		-1	//	LØnijas kõÝda
#define ERR_TTY_MODEM_STATUS	-2	//	Modema kõÝda
#define ERR_TTY_NODATA			-3	//	Nav datu

#define MODEM_STATUS_MASK	(MSR_CTS | MSR_DSR | MSR_CD)

//
//	Ievada / izvada datu virknes
//
#define QUEUESIZE	8192
#define QUEUEMASK	(QUEUESIZE - 1)

#define FULL(q)			(q.head == ((q.tail - 1) & QUEUEMASK))
#define EMPTY(q)		(q.tail == q.head)
#define ENQUEUE(q, b)	(q.data[q.head] = b, q.head = (q.head + 1) & QUEUEMASK)
#define DEQUEUE(q, b)	(b = q.data[q.tail], q.tail = (q.tail + 1) & QUEUEMASK)

// TYPES -------------------------------------------------------------------

enum
{
	UART_AUTO,
	UART_8250,
	UART_16550
};

struct queue
{
	volatile int  head;
	volatile int  tail;
	volatile byte data[QUEUESIZE];
};

class TComPort
{
 public:
	//
    //	Konstruktors
    //
	TComPort(int num = 0)
	{
		Port = num;
		Enabled = false;
		ResetConfig();
	}

    //
    //	Dekonstruktors
    //
    ~TComPort(void)
	{
		Close();
	}

    //	KonfigurÆcijas mai·as funkcijas
	void ResetConfig(void);
	void SetConfig(int = -1, int = -1, int = -1, int = -1, int = -1);
	void SetModemConfig(bool = false, char* = NULL, char* = NULL, char* = NULL, char* = NULL);

	//	VispÆrØgas porta funkcijas
	void Init(void);
    int ReadByte(void);
	int WriteByte(byte);
	void Flush(void);
	void Close(void);

	//	PÆrtraukumu funkcijas
	void Enable(void);
	void ISR_8250(void);
	void ISR_16550(void);
	void Disable(void);

	//	Modema funkijas
	void InitModem(void);
	int Connect(char *host);
	bool CheckForConnection(void);
	int CheckStatus(void);
	int ModemCommand(char *commandString);
	char *ModemResponse(void);
	void Disconnect(void);
	void ModemHangup(void);

    //
    //	PÆris palØgfunkcijas
    //
	bool IsEnabled(void)
	{
		return Enabled;
	}

	bool IsModem(void)
	{
		return UseModem;
	}

	bool OutputQueueIsEmpty(void)
	{
		return EMPTY(OutputQueue);
	}

 protected:
	//
    //	Ieraksta un nolasa no portiem
    //
	void WriteData(byte data)
	{
		outportb(Uart + TRANSMIT_HOLDING_REGISTER, data);
	}

	byte ReadData(void)
	{
		return inportb(Uart + RECEIVE_BUFFER_REGISTER);
	}

	void SetInterrupts(byte flags)
	{
		outportb(Uart + INTERRUPT_ENABLE_REGISTER, flags);
	}

	void SetBaud(int baud)
	{
		int divisor = 115200 / baud;
	    byte old_flags = GetLineParms();

		SetLineParms(LCR_DLAB);
		outportb(Uart + DIVISOR_LATCH_LOW, divisor & 0xff);
		outportb(Uart + DIVISOR_LATCH_HIGH, (divisor >> 8) & 0xff);
		SetLineParms(old_flags);
	}

	byte GetIntID(void)
	{
		return inportb(Uart + INTERRUPT_ID_REGISTER);
	}

	void SetFIFO(byte flags)
	{
		outportb(Uart + FIFO_CONTROL_REGISTER, flags);
	}

	byte GetLineParms(void)
	{
		return inportb(Uart + LINE_CONTROL_REGISTER);
	}

	void SetLineParms(byte flags)
	{
		outportb(Uart + LINE_CONTROL_REGISTER, flags);
	}

	byte GetModemParms(void)
	{
		return inportb(Uart + MODEM_CONTROL_REGISTER);
	}

	void SetModemParms(byte flags)
	{
		outportb(Uart + MODEM_CONTROL_REGISTER, flags);
	}

	byte GetLineStatus(void)
	{
		return inportb(Uart + LINE_STATUS_REGISTER);
	}

	byte GetModemStatus(void)
	{
		return (inportb(Uart + MODEM_STATUS_REGISTER) & MODEM_STATUS_MASK) | ModemStatusIgnore;
	}

	byte GetScratchPad(void)
	{
		return inportb(Uart + SCRATCH_PAD_REGISTER);
	}

	void SetScratchPad(byte pad)
	{
		outportb(Uart + SCRATCH_PAD_REGISTER, pad);
	}

	//	Porta parametri
	int					Port;		//	Porta nummurs
	int					Uart;		//	BÆzes ports
	int					IRQNum;		//	PÆrtraukuma nummurs
	int					BaudRate;	//	Frekvence
	byte				UartType;	//	Tips
	bool				Enabled;	//	Inicializñts
	byte				LineControl;//	LØnijas parametri

    //	PÆrtraukumu vektoru informÆija
	_go32_dpmi_seginfo	ProtectedModeInfo;		//	MÝsu pÆrtraukuma vektors
	_go32_dpmi_seginfo	ProtectedModeSaveInfo;	//	Vecais pÆrtraukuma vektors

	//	Modema parametri
	bool				UseModem;			//	Modems tiek izmantots
	bool				ModemInitialized;	//	Modems ir inicializñts
	bool				ModemRang;			//	Medems zvanØja
	bool				ModemConnected;		//	Modems ir pieslñdzies
	byte				ModemStatusIgnore;	//	Modema statusa ignorñjamie biti
	char				DialType;			//	ZvanØýanas tips
	char				ClearStr[16];		//	AttØrØýanas komanda
	char				StartupStr[32];		//	Palaiýanas komanda
	char				ShutdownStr[16];	//	Aizvñrýanas komanda
	byte				BufferUsed;			//	Izmantotais bufera izmñrs
	char				Buffer[128];		//	Buferis
	double				Timestamp;			//	???

	//	Datu virknes
	queue				InputQueue;	//	Ieejas virkne
	queue				OutputQueue;//	Izejas virkne

	//	StÆvokõi
	volatile byte		LineStatus;		//	LØnijas statuss
	volatile byte		ModemStatus;	//	Modema statuss
	volatile bool		StatusUpdated;	//	Statuss ir mainØjies
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
