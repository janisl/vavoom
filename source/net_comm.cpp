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
//**	Copyright (C) 1999-2002 J∆nis Legzdi∑˝
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
//**    COM porta un modema modulis, b∆zÒts uz Quake izejas kodiem.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "net_loc.h"
#include "comport.h"

// MACROS ------------------------------------------------------------------

//	IRQ p∆rtraukuma re∆lais p∆rtraukuma nummurs
#define _map_irq(irq)   (((irq) > 7) ? ((irq) + 104) : ((irq) + 8))

//	P∆rtraukuma beigu komanda
#define _eoi(irq) { outportb(0x20, 0x20); if ((irq) > 7) outportb(0xA0, 0x20); }

#define DISABLE()	asm volatile ("cli")
#define ENABLE()	asm volatile ("sti")

//	P∆rtraukumu funkcijas
#define ISR_8250_HANDLER(n) \
static void COM##n##_ISR_8250(void) \
{ \
	ComPorts[n - 1]->ISR_8250(); \
}

#define ISR_16550_HANDLER(n) \
static void COM##n##_ISR_16550(void) \
{ \
	ComPorts[n - 1]->ISR_16550(); \
}

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int ComPortUarts[NUM_COM_PORTS] =
{
	//	Standarta b∆zes porti
	0x3f8, 0x2f8, 0x3e8, 0x2e8,
    //	TC-212 kartes papla˝in∆tio COM portu b∆zes porti
	0x2f0, 0x3e0, 0x2e0, 0x260
};

//	Papla˝in∆to portu p∆rtraukumu nummuri nav pareizi
static int ComPortIRQs[NUM_COM_PORTS] = {4, 3, 4, 3, 4, 3, 4, 3};

static TComPort *ComPorts[NUM_COM_PORTS];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	P∆rtraukumu funkcijas
//
//==========================================================================

ISR_8250_HANDLER(1)
ISR_8250_HANDLER(2)
ISR_8250_HANDLER(3)
ISR_8250_HANDLER(4)
ISR_8250_HANDLER(5)
ISR_8250_HANDLER(6)
ISR_8250_HANDLER(7)
ISR_8250_HANDLER(8)

ISR_16550_HANDLER(1)
ISR_16550_HANDLER(2)
ISR_16550_HANDLER(3)
ISR_16550_HANDLER(4)
ISR_16550_HANDLER(5)
ISR_16550_HANDLER(6)
ISR_16550_HANDLER(7)
ISR_16550_HANDLER(8)

//==========================================================================
//
//	NoklusÒto parametru uzst∆dÿ˝ana
//
//==========================================================================

void TComPort::ResetConfig(void)
{
	if (Enabled) return;
	UseModem = false;
	UartType = UART_AUTO;
    Uart = ComPortUarts[Port];
	IRQNum = ComPortIRQs[Port];
	ModemStatusIgnore = MSR_CD | MSR_CTS | MSR_DSR;
	BaudRate = 57600;
	LineControl = LCR_DATA_BITS_8 | LCR_STOP_BITS_1 | LCR_PARITY_NONE;
	DialType = 'T';
	strcpy(ClearStr, "ATZ");
	strcpy(StartupStr, "");
	strcpy(ShutdownStr, "AT H");
	ModemRang = false;
	ModemConnected = false;
	StatusUpdated = false;
	OutputQueue.head = OutputQueue.tail = 0;
	InputQueue.head = InputQueue.tail = 0;
}

//==========================================================================
//
//	Uzliek porta parametrus
//
//==========================================================================

void TComPort::SetConfig(int port, int irq, int baud, int lineCtl, int uartType)
{
	if (port > 0) Uart = port;
	if (irq >= 0) IRQNum = irq;
	if (baud > 0) BaudRate = baud;
    if (lineCtl >= 0) LineControl = lineCtl;
	if (uartType >= 0) UartType = uartType;
}

//==========================================================================
//
//	Uzliek modema parametrus
//
//==========================================================================

void TComPort::SetModemConfig(bool useModem, char *dialType, char *clear, char *init, char *hangup)
{
	UseModem = useModem;
	if (dialType) DialType = dialType[0];
	if (clear) strcpy(ClearStr, clear);
	if (init) strcpy(StartupStr, init);
	if (hangup) strcpy(ShutdownStr, hangup);

	ModemInitialized = false;
}

//==========================================================================
//
//	InicializÒ portu
//
//==========================================================================

void TComPort::Init(void)
{
	if (Enabled)
		return;

	Enable();

	if (Enabled && UseModem && !ModemInitialized)
		InitModem();
}

//==========================================================================
//
//	Nolasa baitu
//
//==========================================================================

int TComPort::ReadByte(void)
{
	int		ret;

	if ((ret = CheckStatus()) != 0)
		return ret;
	
	if (EMPTY(InputQueue))
		return ERR_TTY_NODATA;

	DEQUEUE(InputQueue, ret);
	return (ret & 0xff);
}

//==========================================================================
//
//	S›ta baitu
//
//==========================================================================

int TComPort::WriteByte(byte data)
{
	if (FULL(OutputQueue))
		return -1;

	ENQUEUE(OutputQueue, data);
	return 0;
}

//==========================================================================
//
//	Ja var s›tÿt, tad s›ta
//
//==========================================================================

void TComPort::Flush(void)
{
	byte b;

	if (GetLineStatus() & LSR_TRANSMITTER_EMPTY)
	{
		DEQUEUE(OutputQueue, b);
		WriteData(b);
	}
}

//==========================================================================
//
//	Aizver portu
//
//==========================================================================

void TComPort::Close(void)
{
	if (!Enabled)
    	return;

	double startTime = Sys_Time();
	while ((Sys_Time() - startTime) < 1.0)
		if (EMPTY(OutputQueue))
			break;

	if (UseModem && ModemConnected)
		ModemHangup();

	Disable();
}

//==========================================================================
//
//	Uzliek porta p∆rtraukumu
//
//==========================================================================

void TComPort::Enable(void)
{
	void	(*isr)(void) = NULL;
	int		n;
	byte	b;

	if (Enabled)
	{
		GCon->Logf(NAME_DevNet, "Already enabled");
		return;
	}

	// disable all UART interrupts
	SetInterrupts(0);

	// clear out any buffered uncoming data
	while (GetLineStatus() & LSR_DATA_READY)
		ReadData();

	// get the current line and modem status
	ModemStatus = GetModemStatus();
	LineStatus = GetLineStatus();

	// clear any UART interrupts
	do
	{
		n = GetIntID() & 7;
		if (n == IIR_RX_DATA_READY_INTERRUPT)
			ReadData();
	} while (!(n & 1));

	if (UartType == UART_AUTO)
	{
		SetFIFO(FCR_FIFO_ENABLE);
		b = GetIntID();
		if ((b & IIR_FIFO_ENABLED) == IIR_FIFO_ENABLED)
        {
			UartType = UART_16550;
			GCon->Log(NAME_DevNet, "UART = 16550");
		}
		else
		{
			UartType = UART_8250;
			GCon->Log(NAME_DevNet, "UART = 8250");
		}
	}

	// save the old interrupt handler
	_go32_dpmi_get_protected_mode_interrupt_vector(_map_irq(IRQNum), &ProtectedModeSaveInfo);

	if (UartType == UART_8250)
	{
		SetFIFO(0);
        switch (Port)
        {
         case 0: isr = COM1_ISR_8250; break;
         case 1: isr = COM2_ISR_8250; break;
         case 2: isr = COM3_ISR_8250; break;
         case 3: isr = COM4_ISR_8250; break;
         case 4: isr = COM5_ISR_8250; break;
         case 5: isr = COM6_ISR_8250; break;
         case 6: isr = COM7_ISR_8250; break;
         case 7: isr = COM8_ISR_8250; break;
		}
	}
	else
	{
		SetFIFO(FCR_FIFO_ENABLE | FCR_RCVR_FIFO_RESET | FCR_XMIT_FIFO_RESET | FCR_TRIGGER_08);
        switch (Port)
        {
         case 0: isr = COM1_ISR_16550; break;
         case 1: isr = COM2_ISR_16550; break;
         case 2: isr = COM3_ISR_16550; break;
         case 3: isr = COM4_ISR_16550; break;
         case 4: isr = COM5_ISR_16550; break;
         case 5: isr = COM6_ISR_16550; break;
         case 6: isr = COM7_ISR_16550; break;
         case 7: isr = COM8_ISR_16550; break;
		}
	}

	ProtectedModeInfo.pm_offset = (int)isr;

	n = _go32_dpmi_allocate_iret_wrapper(&ProtectedModeInfo);
	if (n)
	{
		GCon->Logf(NAME_DevNet, "serial: protected mode callback allocation failed");
		return;
	}

	ComPorts[Port] = this;

	// disable interrupts at the processor
	DISABLE();

	// install our interrupt handlers now
	_go32_dpmi_set_protected_mode_interrupt_vector(_map_irq(IRQNum), &ProtectedModeInfo);

	// enable our interrupt at the PIC
    if (IRQNum < 8)
		outportb(0x21, inportb(0x21) & ~(1 << IRQNum));
	else
		outportb(0xa1, inportb(0xa1) & ~(1 << (IRQNum - 8)));

	// enable interrupts at the processor
	ENABLE();

	// enable interrupts at the PIC
	outportb(0x20, 0xc2);
	if (IRQNum > 7)
    	outportb(0xa0, 0xc2);

	// set baud rate & line control
	if (UseModem)
	{
		if (BaudRate == 14400)
			BaudRate = 19200;
		if (BaudRate == 28800)
			BaudRate = 38400;
	}
	SetBaud(BaudRate);
	SetLineParms(LineControl);

	// set modem control register & enable uart interrupt generation
	SetModemParms(MCR_OUT2 | MCR_RTS | MCR_DTR);

	// enable the individual interrupts at the uart
	SetInterrupts(IER_RX_DATA_READY | IER_TX_HOLDING_REGISTER_EMPTY | IER_LINE_STATUS | IER_MODEM_STATUS);

	Enabled = true;
}

//==========================================================================
//
//	ISR 8250 p∆rtraukuma apstr∆de
//
//==========================================================================

void TComPort::ISR_8250(void)
{
	byte	source = 0;
	byte	b;

	DISABLE();

	while ((source = GetIntID() & 0x07) != IIR_NO_INTERRUPT)
	{
		switch (source)
		{
		 case IIR_RX_DATA_READY_INTERRUPT:
			b = ReadData();
			if (!FULL(InputQueue))
			{
				ENQUEUE(InputQueue, b);
			}
			else
			{
				LineStatus |= LSR_OVERRUN_ERROR;
				StatusUpdated = true;
			}
			break;

		 case IIR_TX_HOLDING_REGISTER_INTERRUPT:
			if (!EMPTY(OutputQueue))
			{
				DEQUEUE(OutputQueue, b);
				WriteData(b);
			}
			break;

		 case IIR_MODEM_STATUS_INTERRUPT:
			ModemStatus = GetModemStatus();
			StatusUpdated = true;
			break;

		 case IIR_LINE_STATUS_INTERRUPT:
			LineStatus = GetLineStatus();
			StatusUpdated = true;
			break;
		}
		source = GetIntID() & 0x07;
	}
	_eoi(IRQNum)
}

//==========================================================================
//
//	ISR 16550 p∆rtraukuma apstr∆de
//
//==========================================================================

void TComPort::ISR_16550(void)
{
	int		count;
	byte	source;
	byte	b;

	DISABLE();
	while ((source = GetIntID() & 0x07) != IIR_NO_INTERRUPT)
	{
		switch (source)
		{
			case IIR_RX_DATA_READY_INTERRUPT:
				do
				{
					b = ReadData();
					if (!FULL(InputQueue))
					{
						ENQUEUE(InputQueue, b);
					}
					else
					{
						LineStatus |= LSR_OVERRUN_ERROR;
						StatusUpdated = true;
					}
				} while (GetLineStatus() & LSR_DATA_READY);
				break;

			case IIR_TX_HOLDING_REGISTER_INTERRUPT:
				count = 16;
				while ((!EMPTY(OutputQueue)) && count--)
				{
					DEQUEUE(OutputQueue, b);
					WriteData(b);
				}
				break;

			case IIR_MODEM_STATUS_INTERRUPT:
				ModemStatus = GetModemStatus();
				StatusUpdated = true;
				break;

			case IIR_LINE_STATUS_INTERRUPT:
				LineStatus = GetLineStatus();
				StatusUpdated = true;
				break;
		}
		source = GetIntID() & 0x07;
	}

	// check for lost IIR_TX_HOLDING_REGISTER_INTERRUPT on 16550a!
	if (GetLineStatus() & LSR_TRANSMITTER_EMPTY)
	{
		count = 16;
		while ((!EMPTY(OutputQueue)) && count--)
		{
			DEQUEUE(OutputQueue, b);
			WriteData(b);
		}
	}

	_eoi(IRQNum)
}

//==========================================================================
//
//	Atjauno vecos p∆rtraukumus
//
//==========================================================================

void TComPort::Disable(void)
{
	if (!Enabled)
	{
		GCon->Logf(NAME_DevNet, "Already disabled");
		return;
	}

	// disable interrupts at the uart
	SetInterrupts(0);

	// disable our interrupt at the PIC
	if (IRQNum < 8)
		outportb(0x21, inportb(0x21) | (1 << IRQNum));
	else
		outportb(0xa1, inportb(0xa1) | (1 << IRQNum));

	// disable interrupts at the processor
	DISABLE();

	// restore the old interrupt handler
	_go32_dpmi_set_protected_mode_interrupt_vector(_map_irq(IRQNum), &ProtectedModeSaveInfo);
	_go32_dpmi_free_iret_wrapper(&ProtectedModeInfo);

	// enable interrupts at the processor
	ENABLE();

	Enabled = false;
}

//==========================================================================
//
//	InicializÒ modemu
//
//==========================================================================

void TComPort::InitModem(void)
{
	double	start;
	char	*response;

	GCon->Logf(NAME_Init, "Initializing modem...");

	// write 0 to MCR, wait 1/2 sec, then write the real value back again
	// I got this from the guys at head-to-head who say it's necessary.
	SetModemParms(0);
	start = Sys_Time();
	while ((Sys_Time() - start) < 0.5)
		;
	SetModemParms(MCR_OUT2 | MCR_RTS | MCR_DTR);
	start = Sys_Time();
	while ((Sys_Time() - start) < 0.25)
		;

	if (*ClearStr)
	{
		ModemCommand(ClearStr);
		start = Sys_Time();
		while(1)
		{
			if ((Sys_Time() - start) > 3.0)
			{
				GCon->Logf(NAME_Init, "No response - clear failed");
				Disable();
				return;
			}
			response = ModemResponse();
			if (!response)
				continue;
			if (strncmp(response, "OK", 2) == 0)
				break;
			if (strncmp(response, "ERROR", 5) == 0)
			{
				Disable();
				return;
			}
		}
	}

	if (*StartupStr)
	{
		ModemCommand(StartupStr);
		start = Sys_Time();
		while(1)
		{
			if ((Sys_Time() - start) > 3.0)
			{
				Disable();
				return;
			}
			response = ModemResponse();
			if (!response)
				continue;
			if (strncmp(response, "OK", 2) == 0)
				break;
			if (strncmp(response, "ERROR", 5) == 0)
			{
				Disable();
				return;
			}
		}
	}

	ModemInitialized = true;
/*
failed:
	if (m_return_onerror)
	{
		key_dest = key_menu;
		m_state = m_return_state;
		m_return_onerror = false;
		Q_strcpy(m_return_reason, "Initialization Failed");
	}*/
}

//==========================================================================
//
//	Tie˝∆ pieslÒguma gadÿjum∆ nekas nav j∆dara
//	Modema gadÿjum∆ zvana pa doto t∆lru∑a nummuru
//
//==========================================================================

int TComPort::Connect(char *host)
{
	double	start;
	char	*response = NULL;
	char	dialstring[64];
	byte	b;

	if ((ModemStatus & MODEM_STATUS_MASK) != MODEM_STATUS_MASK)
	{
		FString Msg = "Serial: line not ready (";
		if ((ModemStatus & MSR_CTS) == 0)
			Msg += " CTS";
		if ((ModemStatus & MSR_DSR) == 0)
			Msg += " DSR";
		if ((ModemStatus & MSR_CD) == 0)
			Msg += " CD";
		Msg += " )";
		GCon->Logf(NAME_DevNet, Msg);
		return -1;
	}

	// discard any scraps in the input buffer
	while (!EMPTY(InputQueue))
		DEQUEUE(InputQueue, b);

	CheckStatus();

	if (UseModem)
	{
/*		save_key_dest = key_dest;
		key_dest = key_console;
		key_count = -2;*/

		GCon->Logf(NAME_DevNet, "Dialing...");
		sprintf(dialstring, "AT D%c %s\r", DialType, host);
		ModemCommand(dialstring);
		start = Sys_Time();
		while (1)
		{
			if ((Sys_Time() - start) > 60.0)
			{
				GCon->Logf(NAME_DevNet, "Dialing failure!");
				break;
			}

/*
			Sys_SendKeyEvents ();
			if (key_count == 0)
			{
				if (key_lastpress != K_ESCAPE)
				{
					key_count = -2;
					continue;
				}
				Con_Printf("Aborting...\n");
				while ((Sys_Time() - start) < 5.0)
					;
				disable();
				p->outputQueue.head = p->outputQueue.tail = 0;
				p->inputQueue.head = p->inputQueue.tail = 0;
				outportb(p->uart + MODEM_CONTROL_REGISTER, inportb(p->uart + MODEM_CONTROL_REGISTER) & ~MCR_DTR);
				enable();
				start = Sys_Time();
				while ((Sys_Time() - start) < 0.75)
					;
				outportb(p->uart + MODEM_CONTROL_REGISTER, inportb(p->uart + MODEM_CONTROL_REGISTER) | MCR_DTR);
				response = "Aborted";
				break;
			}
*/

			response = ModemResponse();
			if (!response)
				continue;
			if (strncmp(response, "CONNECT", 7) == 0)
			{
				DISABLE();
				ModemRang = true;
				ModemConnected = true;
				OutputQueue.head = OutputQueue.tail = 0;
				InputQueue.head = InputQueue.tail = 0;
				ENABLE();
/*				key_dest = save_key_dest;
				key_count = 0;
				m_return_onerror = false;*/
				return 0;
			}
			if (strncmp(response, "NO CARRIER", 10) == 0)
				break;
			if (strncmp(response, "NO DIALTONE", 11) == 0)
				break;
			if (strncmp(response, "NO DIAL TONE", 12) == 0)
				break;
			if (strncmp(response, "NO ANSWER", 9) == 0)
				break;
			if (strncmp(response, "BUSY", 4) == 0)
				break;
			if (strncmp(response, "ERROR", 5) == 0)
				break;
		}
/*		key_dest = save_key_dest;
		key_count = 0;
		if (m_return_onerror)
		{
			key_dest = key_menu;
			m_state = m_return_state;
			m_return_onerror = false;
			Q_strncpy(m_return_reason, response, 31);
		}*/
		return -1;
	}
//	m_return_onerror = false;
	return 0;
}

//==========================================================================
//
//	Tie˝∆ pieslÒguma gadÿjum∆ nekas nav j∆dara
//  Modema gadÿjum∆ p∆rbauda vai k∆ds cits nemÒ÷ina piezvanÿt
//
//==========================================================================

bool TComPort::CheckForConnection(void)
{
	CheckStatus();

	if (UseModem)
	{
		if (!ModemRang)
		{
			if (!ModemResponse())
				return false;

			if (strncmp(Buffer, "RING", 4) == 0)
			{
				ModemCommand("ATA");
				ModemRang = true;
				Timestamp = net_time;
			}
			return false;
		}
		if (!ModemConnected)
		{
			if ((net_time - Timestamp) > 35.0)
			{
				GCon->Logf(NAME_DevNet, "Unable to establish modem connection");
				ModemRang = false;
				return false;
			}

			if (!ModemResponse())
				return false;

			if (strncmp(Buffer, "CONNECT", 7) != 0)
				return false;

			DISABLE();
			ModemConnected = true;
			OutputQueue.head = OutputQueue.tail = 0;
			InputQueue.head = InputQueue.tail = 0;
			ENABLE();
			GCon->Logf(NAME_DevNet, "Modem Connect");
			return true;
		}
		return true;
	}

	// direct connect case
	if (EMPTY(InputQueue))
		return false;
	return true;
}

//==========================================================================
//
//	P∆rbauda st∆vokli
//
//==========================================================================

int TComPort::CheckStatus(void)
{
	int		ret = 0;

	if (StatusUpdated)
	{
		StatusUpdated = false;

		if (LineStatus & (LSR_OVERRUN_ERROR | LSR_PARITY_ERROR | LSR_FRAMING_ERROR | LSR_BREAK_DETECT))
		{
			if (LineStatus & LSR_OVERRUN_ERROR)
				GCon->Log(NAME_DevNet, "Serial overrun error");
			if (LineStatus & LSR_PARITY_ERROR)
				GCon->Log(NAME_DevNet, "Serial parity error");
			if (LineStatus & LSR_FRAMING_ERROR)
				GCon->Log(NAME_DevNet, "Serial framing error");
			if (LineStatus & LSR_BREAK_DETECT)
				GCon->Log(NAME_DevNet, "Serial break detect");
			ret = ERR_TTY_LINE_STATUS;
		}

		if ((ModemStatus & MODEM_STATUS_MASK) != MODEM_STATUS_MASK)
		{
			if (!(ModemStatus & MSR_CTS))
				GCon->Logf(NAME_DevNet, "Serial lost CTS");
			if (!(ModemStatus & MSR_DSR))
				GCon->Logf(NAME_DevNet, "Serial lost DSR");
			if (!(ModemStatus & MSR_CD))
				GCon->Logf(NAME_DevNet, "Serial lost Carrier");
			ret = ERR_TTY_MODEM_STATUS;
		}
	}

	return ret;
}

//==========================================================================
//
//	S›ta modema komandu
//
//==========================================================================

int TComPort::ModemCommand(char *commandString)
{
	byte	b;

	if (CheckStatus())
		return -1;

	DISABLE();
	OutputQueue.head = OutputQueue.tail = 0;
	InputQueue.head = InputQueue.tail = 0;
	ENABLE();
	BufferUsed = 0;

	while (*commandString)
		ENQUEUE(OutputQueue, *commandString++);
	ENQUEUE(OutputQueue, '\r');

	// get the transmit rolling
	DEQUEUE(OutputQueue, b);
	WriteData(b);

	return 0;
}

//==========================================================================
//
//	Nolasa modema atbildi
//
//==========================================================================

char *TComPort::ModemResponse(void)
{
	byte	b;

	if (CheckStatus())
		return NULL;

	while (!EMPTY(InputQueue))
	{
		DEQUEUE(InputQueue, b);

		if (BufferUsed == (sizeof(Buffer) - 1))
			b = '\r';

		if (b == '\r' && BufferUsed)
		{
			Buffer[BufferUsed] = 0;
			GCon->Logf(NAME_DevNet, Buffer);
			SCR_Update();
			BufferUsed = 0;
			return Buffer;
		}

		if (b < ' ' || b > 'z')
			continue;
		Buffer[BufferUsed] = b;
		BufferUsed++;
	}

	return NULL;
}

//==========================================================================
//
//	AtslÒdz modemu
//
//==========================================================================

void TComPort::Disconnect(void)
{
	if (UseModem && ModemConnected)
		ModemHangup();
}

//==========================================================================
//
//	Apst∆dina modemu
//
//==========================================================================

void TComPort::ModemHangup(void)
{
	double start;

	GCon->Logf(NAME_DevNet, "Hanging up modem...");
	DISABLE();
	ModemRang = false;
	OutputQueue.head = OutputQueue.tail = 0;
	InputQueue.head = InputQueue.tail = 0;
	SetModemParms(GetModemParms() & ~MCR_DTR);
	ENABLE();
	start = Sys_Time();
	while ((Sys_Time() - start) < 1.5)
		;

	SetModemParms(GetModemParms() | MCR_DTR);
	ModemCommand("+++");
	start = Sys_Time();
	while ((Sys_Time() - start) < 1.5)
		;

	ModemCommand(ShutdownStr);
	start = Sys_Time();
	while ((Sys_Time() - start) < 1.5)
		;

	ModemResponse();
	GCon->Logf(NAME_DevNet, "Hangup complete");
	ModemConnected = false;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//
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
