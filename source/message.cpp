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
//  VMessage::Alloc
//
//==========================================================================

void VMessage::Alloc(vint32 startsize)
{
	guard(VMessage::Alloc);
	int AllocSize = startsize;
	if (AllocSize < 256)
		AllocSize = 256;
	Data = (vuint8*)Z_Malloc(AllocSize);
	MaxSize = AllocSize;
	CurSize = 0;
	Overflowed = false;
	unguard;
}

//==========================================================================
//
//  VMessage::Free
//
//==========================================================================

void VMessage::Free()
{
	guard(VMessage::Free);
	Z_Free(Data);
	Data = NULL;
	MaxSize = 0;
	CurSize = 0;
	unguard;
}

//==========================================================================
//
//  VMessage::Clear
//
//==========================================================================

void VMessage::Clear()
{
	CurSize = 0;
}

//==========================================================================
//
//  VMessage::GetSpace
//
//==========================================================================

void* VMessage::GetSpace(vint32 length)
{
	guard(VMessage::GetSpace);
	if (CurSize + length > MaxSize)
	{
		if (!AllowOverflow)
			Sys_Error("TSizeBuf::GetSpace: overflow without allowoverflow set");

		if (length > MaxSize)
			Sys_Error("TSizeBuf::GetSpace: %i is > full buffer size", length);

		Overflowed = true;
		GCon->Log("TSizeBuf::GetSpace: overflow");
		Clear();
	}

	void* data = Data + CurSize;
	CurSize += length;

	return data;
	unguard;
}

//==========================================================================
//
//  VMessage::Write
//
//==========================================================================

void VMessage::Write(const void* data, vint32 length)
{
	guard(VMessage::Write);
	memcpy(GetSpace(length), data, length);
	unguard;
}

//==========================================================================
//
//  VMessage::operator << vint8
//
//==========================================================================

VMessage& VMessage::operator << (vint8 c)
{
	guard(VMessage::operator << vint8);
	vuint8* buf = (vuint8*)GetSpace(1);
	buf[0] = c;

	return *this;
	unguard;
}

//==========================================================================
//
//  VMessage::operator << vint16
//
//==========================================================================

VMessage& VMessage::operator << (vint16 c)
{
	guard(VMessage::operator << vint16);
	vuint8* buf = (vuint8*)GetSpace(2);
	buf[0] = c & 0xff;
	buf[1] = (c >> 8) & 0xff;

	return *this;
	unguard;
}

//==========================================================================
//
//  VMessage::operator << vint32
//
//==========================================================================

VMessage& VMessage::operator << (vint32 c)
{
	guard(VMessage::operator << vint32);
	vuint8* buf = (vuint8*)GetSpace(4);
	buf[0] =  c        & 0xff;
	buf[1] = (c >>  8) & 0xff;
	buf[2] = (c >> 16) & 0xff;
	buf[3] = (c >> 24) & 0xff;

	return *this;
	unguard;
}

//==========================================================================
//
//  VMessage::operator << float
//
//==========================================================================

VMessage& VMessage::operator << (float f)
{
	guard(VMessage::operator << float);
	vint32 c = *(vint32*)&f;
	vuint8* buf = (vuint8*)GetSpace(4);
	buf[0] =  c        & 0xff;
	buf[1] = (c >>  8) & 0xff;
	buf[2] = (c >> 16) & 0xff;
	buf[3] = (c >> 24) & 0xff;

	return *this;
	unguard;
}

//==========================================================================
//
//  VMessage::operator << const char*
//
//==========================================================================

VMessage& VMessage::operator << (const char* s)
{
	guard(VMessage::operator << const char*);
	if (!s)
		Write("", 1);
	else
		Write(s, strlen(s) + 1);

	return *this;
	unguard;
}

//==========================================================================
//
//  VMessage::operator << VStr
//
//==========================================================================

VMessage& VMessage::operator << (const VStr& s)
{
	guard(VMessage::operator << VStr);
	return *this << *s;
	unguard;
}

//==========================================================================
//
//	VMessage::operator << VMessage
//
//==========================================================================

VMessage& VMessage::operator << (const VMessage &msg)
{
	guard(VMessage::operator << VMessage);
	Write(msg.Data, msg.CurSize);
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessage::BeginReading
//
//==========================================================================

void VMessage::BeginReading()
{
	ReadCount = 0;
	BadRead = false;
}

//==========================================================================
//
//  VMessage::operator >> vint8
//
//	Returns -1 and sets msg_badread if no more characters are available.
//
//==========================================================================

VMessage& VMessage::operator >> (vint8& c)
{
	guard(VMessage::operator >> vint8);
	if (ReadCount + 1 > CurSize)
	{
		BadRead = true;
		c = -1;
	}
	else
	{
		c = (vint8)Data[ReadCount];
		ReadCount++;
	}
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessage::operator >> vint16
//
//==========================================================================

VMessage& VMessage::operator >> (vint16& c)
{
	guard(VMessage::operator >> vint16);
	if (ReadCount + 2 > CurSize)
	{
		BadRead = true;
		c = -1;
	}
	else
	{
		c = (vint16)(Data[ReadCount] + (Data[ReadCount + 1] << 8));
		ReadCount += 2;
	}
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessage::operator >> vint32
//
//==========================================================================

VMessage& VMessage::operator >> (vint32& c)
{
	guard(VMessage::operator >> vint32);
	if (ReadCount + 4 > CurSize)
	{
		BadRead = true;
		c = -1;
	}
	else
	{
		c = Data[ReadCount]
			+ (Data[ReadCount + 1] << 8)
			+ (Data[ReadCount + 2] << 16)
			+ (Data[ReadCount + 3] << 24);
		ReadCount += 4;
	}
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessage::operator >> float
//
//==========================================================================

VMessage& VMessage::operator >> (float& f)
{
	guard(VMessage::operator >> float);
	if (ReadCount + 4 > CurSize)
	{
		BadRead = true;
		f = -1;
	}
	else
	{
		vint32 c = Data[ReadCount]
				+ (Data[ReadCount + 1] << 8)
				+ (Data[ReadCount + 2] << 16)
				+ (Data[ReadCount + 3] << 24);
		ReadCount += 4;
		f = *(float*)&c;
	}
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessage::operator >> const char*
//
//==========================================================================

VMessage& VMessage::operator >> (const char*& s)
{
	guard(VMessage::operator >> const char*);
	s = ReadString();
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessage::operator >> VStr
//
//==========================================================================

VMessage& VMessage::operator >> (VStr& s)
{
	guard(VMessage::operator >> VStr);
	s = ReadString();
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessage::ReadString
//
//==========================================================================

const char* VMessage::ReadString()
{
	guard(VMessage::ReadString);
	static char		string[2048];
	vint32			l, c;

	l = 0;
	do
	{
		if (ReadCount + 1 > CurSize)
		{
			BadRead = true;
			c = -1;
		}
		else
		{
			c = (signed char)Data[ReadCount];
			ReadCount++;
		}
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while (l < (vint32)sizeof(string) - 1);

	string[l] = 0;

	return string;
	unguard;
}

//==========================================================================
//
//  VMessage::operator >> VMessage
//
//==========================================================================

VMessage& VMessage::operator >> (VMessage& msg)
{
	guard(VMessage::operator >> VMessage);
	msg.Clear();
	if (!BadRead)
	{
		msg.Write(Data + ReadCount, CurSize - ReadCount);
		ReadCount = CurSize;
	}
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessage::ReadChar
//
//	Returns -1 and sets msg_badread if no more characters are available.
//
//==========================================================================

vuint8 VMessage::ReadByte()
{
	guard(VMessage::ReadByte);
	vuint8	c;

	if (ReadCount + 1 > CurSize)
	{
		BadRead = true;
		c = 0xff;
	}
	else
	{
		c = Data[ReadCount];
		ReadCount++;
	}
	return c;
	unguard;
}

//==========================================================================
//
//  VMessage::ReadShort
//
//==========================================================================

vint16 VMessage::ReadShort()
{
	guard(VMessage::ReadShort);
	vint16	c;

	if (ReadCount + 2 > CurSize)
	{
		BadRead = true;
		c = -1;
	}
	else
	{
		c = (vint16)(Data[ReadCount] + (Data[ReadCount + 1] << 8));
		ReadCount += 2;
	}
	return c;
	unguard;
}
