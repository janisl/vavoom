//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  TMessage::WriteChar
//
//==========================================================================

TMessage &TMessage::operator << (char c)
{
	byte    *buf;
	
	buf = (byte*)GetSpace(1);
	buf[0] = c;

	return *this;
}

//==========================================================================
//
//  TMessage::WriteShort
//
//==========================================================================

TMessage &TMessage::operator << (short c)
{
	byte    *buf;
	
	buf = (byte*)GetSpace(2);
	buf[0] = c & 0xff;
	buf[1] = (c >> 8) & 0xff;

	return *this;
}

//==========================================================================
//
//  TMessage::WriteLong
//
//==========================================================================

TMessage &TMessage::operator << (int c)
{
	byte    *buf;
	
	buf = (byte*)GetSpace(4);
	buf[0] =  c        & 0xff;
	buf[1] = (c >>  8) & 0xff;
	buf[2] = (c >> 16) & 0xff;
	buf[3] = (c >> 24) & 0xff;

	return *this;
}

//==========================================================================
//
//  TMessage::WriteFloat
//
//==========================================================================

TMessage &TMessage::operator << (float f)
{
	union
	{
		float   f;
		int     l;
	} dat;
	
	dat.f = f;
	dat.l = LittleLong(dat.l);
	
	Write(&dat.l, 4);

	return *this;
}

//==========================================================================
//
//  TMessage::WriteString
//
//==========================================================================

TMessage &TMessage::operator << (const char *s)
{
	if (!s)
		Write("", 1);
	else
		Write(s, strlen(s) + 1);

	return *this;
}

//==========================================================================
//
//
//
//==========================================================================

TMessage &TMessage::operator << (const TMessage &msg)
{
	Write(msg.Data, msg.CurSize);
	return *this;
}

//==========================================================================
//
//  TMessage::BeginReading
//
//==========================================================================

void TMessage::BeginReading(void)
{
	readcount = 0;
	badread = false;
}

//==========================================================================
//
//  TMessage::ReadChar
//
//==========================================================================

// returns -1 and sets msg_badread if no more characters are available
TMessage &TMessage::operator >> (char &c)
{
	if (readcount + 1 > CurSize)
	{
		badread = true;
		c = -1;
	}
	else
	{
		c = (signed char)Data[readcount];
		readcount++;
	}
	return *this;
}

//==========================================================================
//
//  TMessage::ReadShort
//
//==========================================================================

TMessage &TMessage::operator >> (short &c)
{
	if (readcount + 2 > CurSize)
	{
		badread = true;
		c = -1;
	}
	else
    {
		c = (short)(Data[readcount] + (Data[readcount + 1] << 8));
		readcount += 2;
	}
	return *this;
}

//==========================================================================
//
//  TMessage::ReadLong
//
//==========================================================================

TMessage &TMessage::operator >> (int &c)
{
	if (readcount + 4 > CurSize)
	{
		badread = true;
		c = -1;
	}
	else
	{
		c = Data[readcount]
			+ (Data[readcount + 1] << 8)
			+ (Data[readcount + 2] << 16)
			+ (Data[readcount + 3] << 24);
		readcount += 4;
	}
	return *this;
}

//==========================================================================
//
//  TMessage::ReadFloat
//
//==========================================================================

TMessage &TMessage::operator >> (float &f)
{
	if (readcount + 4 > CurSize)
	{
		badread = true;
		f = -1;
	}
	else
	{
		union
		{
			byte    b[4];
			float   f;
			int     l;
		} dat;
	
		dat.b[0] = Data[readcount];
		dat.b[1] = Data[readcount + 1];
		dat.b[2] = Data[readcount + 2];
		dat.b[3] = Data[readcount + 3];
		readcount += 4;
		dat.l = LittleLong(dat.l);
		f = dat.f;
	}
	return *this;
}

//==========================================================================
//
//  TMessage::ReadString
//
//==========================================================================

TMessage &TMessage::operator >> (char *&s)
{
	s = ReadString();
	return *this;
}

//==========================================================================
//
//  TMessage::ReadString
//
//==========================================================================

char *TMessage::ReadString(void)
{
	static char     string[2048];
	int             l, c;
	
	l = 0;
	do
	{
		if (readcount + 1 > CurSize)
		{
			badread = true;
			c = -1;
		}
		else
		{
			c = (signed char)Data[readcount];
			readcount++;
		}
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while (l < (int)sizeof(string) - 1);
	
	string[l] = 0;
	
	return string;
}

//==========================================================================
//
//  TMessage::ReadString
//
//==========================================================================

TMessage &TMessage::operator >> (TMessage &msg)
{
	msg.Clear();
	if (!badread)
	{
		msg.Write(Data + readcount, CurSize - readcount);
		readcount = CurSize;
	}
	return *this;
}

//==========================================================================
//
//  TMessage::ReadChar
//
//==========================================================================

// returns -1 and sets msg_badread if no more characters are available
byte TMessage::ReadByte(void)
{
	byte	c;

	if (readcount + 1 > CurSize)
	{
		badread = true;
		c = 0xff;
	}
	else
	{
		c = Data[readcount];
		readcount++;
	}
	return c;
}

//==========================================================================
//
//  TMessage::ReadShort
//
//==========================================================================

short TMessage::ReadShort(void)
{
	short	c;

	if (readcount + 2 > CurSize)
	{
		badread = true;
		c = -1;
	}
	else
    {
		c = (short)(Data[readcount] + (Data[readcount + 1] << 8));
		readcount += 2;
	}
	return c;
}

//==========================================================================
//
//
//
//==========================================================================

#if 0
void MSG_WriteCoord (sizebuf_t *sb, float f)
{
	MSG_WriteShort (sb, (int)(f*8));
}

void MSG_WriteAngle (sizebuf_t *sb, float f)
{
	MSG_WriteByte (sb, ((int)f*256/360) & 255);
}

float MSG_ReadCoord (void)
{
	return MSG_ReadShort() * (1.0/8);
}

float MSG_ReadAngle (void)
{
	return MSG_ReadChar() * (360.0/256);
}

#endif
