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
//  VMessageOut::Alloc
//
//==========================================================================

void VMessageOut::Alloc(vint32 startsize)
{
	guard(VMessageOut::Alloc);
	if (Data)
		Z_Free(Data);
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
//  VMessageOut::Free
//
//==========================================================================

void VMessageOut::Free()
{
	guard(VMessageOut::Free);
	Z_Free(Data);
	Data = NULL;
	MaxSize = 0;
	CurSize = 0;
	unguard;
}

//==========================================================================
//
//  VMessageOut::Clear
//
//==========================================================================

void VMessageOut::Clear()
{
	CurSize = 0;
}

//==========================================================================
//
//  VMessageOut::GetSpace
//
//==========================================================================

void* VMessageOut::GetSpace(vint32 length)
{
	guard(VMessageOut::GetSpace);
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
//  VMessageOut::Serialise
//
//==========================================================================

void VMessageOut::Serialise(void* data, vint32 length)
{
	guard(VMessageOut::Serialise);
	memcpy(GetSpace(length), data, length);
	unguard;
}

//==========================================================================
//
//  VMessageOut::operator << vint8
//
//==========================================================================

VMessageOut& VMessageOut::operator << (vint8 c)
{
	guard(VMessageOut::operator << vint8);
	vuint8* buf = (vuint8*)GetSpace(1);
	buf[0] = c;

	return *this;
	unguard;
}

//==========================================================================
//
//  VMessageOut::operator << vint16
//
//==========================================================================

VMessageOut& VMessageOut::operator << (vint16 c)
{
	guard(VMessageOut::operator << vint16);
	vuint8* buf = (vuint8*)GetSpace(2);
	buf[0] = c & 0xff;
	buf[1] = (c >> 8) & 0xff;

	return *this;
	unguard;
}

//==========================================================================
//
//  VMessageOut::operator << vint32
//
//==========================================================================

VMessageOut& VMessageOut::operator << (vint32 c)
{
	guard(VMessageOut::operator << vint32);
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
//  VMessageOut::operator << float
//
//==========================================================================

VMessageOut& VMessageOut::operator << (float f)
{
	guard(VMessageOut::operator << float);
	union
	{
		vuint8	b[4];
		float	f;
	} c;
	c.f = f;
	vuint8* buf = (vuint8*)GetSpace(4);
	buf[0] = c.b[0];
	buf[1] = c.b[1];
	buf[2] = c.b[2];
	buf[3] = c.b[3];

	return *this;
	unguard;
}

//==========================================================================
//
//  VMessageOut::operator << const char*
//
//==========================================================================

VMessageOut& VMessageOut::operator << (const char* s)
{
	guard(VMessageOut::operator << const char*);
	if (!s)
		Serialise(const_cast<char*>(""), 1);
	else
		Serialise(const_cast<char*>(s), VStr::Length(s) + 1);

	return *this;
	unguard;
}

//==========================================================================
//
//  VMessageOut::operator << VStr
//
//==========================================================================

VMessageOut& VMessageOut::operator << (const VStr& s)
{
	guard(VMessageOut::operator << VStr);
	return *this << *s;
	unguard;
}

//==========================================================================
//
//	VMessageOut::operator << VMessageOut
//
//==========================================================================

VMessageOut& VMessageOut::operator << (const VMessageOut &msg)
{
	guard(VMessageOut::operator << VMessageOut);
	Serialise(msg.Data, msg.CurSize);
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessageIn::Alloc
//
//==========================================================================

void VMessageIn::Alloc(vint32 startsize)
{
	guard(VMessageIn::Alloc);
	int AllocSize = startsize;
	if (AllocSize < 256)
		AllocSize = 256;
	Data = (vuint8*)Z_Malloc(AllocSize);
	MaxSize = AllocSize;
	CurSize = 0;
	unguard;
}

//==========================================================================
//
//  VMessageIn::Free
//
//==========================================================================

void VMessageIn::Free()
{
	guard(VMessageIn::Free);
	Z_Free(Data);
	Data = NULL;
	MaxSize = 0;
	CurSize = 0;
	unguard;
}

//==========================================================================
//
//  VMessageIn::SetData
//
//==========================================================================

void VMessageIn::SetData(const void* AData, vint32 ALength)
{
	guard(VMessageIn::SetData);
	if (ALength > MaxSize)
		Sys_Error("TSizeBuf::GetSpace: %i is > full buffer size", ALength);
	memcpy(Data, AData, ALength);
	CurSize = ALength;
	unguard;
}

//==========================================================================
//
//  VMessageIn::BeginReading
//
//==========================================================================

void VMessageIn::BeginReading()
{
	ReadCount = 0;
	BadRead = false;
}

//==========================================================================
//
//  VMessageIn::Serialise
//
//==========================================================================

void VMessageIn::Serialise(void* AData, int ALen)
{
	guard(VMessageIn::Serialise);
	if (ReadCount + ALen > CurSize)
	{
		BadRead = true;
		memset(AData, 0, ALen);
	}
	else
	{
		memcpy(AData, Data + ReadCount, ALen);
		ReadCount += ALen;
	}
	unguard;
}

//==========================================================================
//
//  VMessageIn::operator >> vint8
//
//	Returns -1 and sets msg_badread if no more characters are available.
//
//==========================================================================

VMessageIn& VMessageIn::operator >> (vint8& c)
{
	guard(VMessageIn::operator >> vint8);
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
//  VMessageIn::operator >> vint16
//
//==========================================================================

VMessageIn& VMessageIn::operator >> (vint16& c)
{
	guard(VMessageIn::operator >> vint16);
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
//  VMessageIn::operator >> vint32
//
//==========================================================================

VMessageIn& VMessageIn::operator >> (vint32& c)
{
	guard(VMessageIn::operator >> vint32);
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
//  VMessageIn::operator >> float
//
//==========================================================================

VMessageIn& VMessageIn::operator >> (float& f)
{
	guard(VMessageIn::operator >> float);
	if (ReadCount + 4 > CurSize)
	{
		BadRead = true;
		f = -1;
	}
	else
	{
		union
		{
			vuint8	b[4];
			float	f;
		} d;
		d.b[0] = Data[ReadCount];
		d.b[1] = Data[ReadCount + 1];
		d.b[2] = Data[ReadCount + 2];
		d.b[3] = Data[ReadCount + 3];
		ReadCount += 4;
		f = d.f;
	}
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessageIn::operator >> const char*
//
//==========================================================================

VMessageIn& VMessageIn::operator >> (const char*& s)
{
	guard(VMessageIn::operator >> const char*);
	s = ReadString();
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessageIn::operator >> VStr
//
//==========================================================================

VMessageIn& VMessageIn::operator >> (VStr& s)
{
	guard(VMessage::operator >> VStr);
	s = ReadString();
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessageIn::ReadString
//
//==========================================================================

const char* VMessageIn::ReadString()
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
//  VMessageIn::operator >> VMessageIn
//
//==========================================================================

VMessageIn& VMessageIn::operator >> (VMessageIn& msg)
{
	guard(VMessageIn::operator >> VMessageIn);
	msg.CurSize = 0;
	if (!BadRead)
	{
		msg.SetData(Data + ReadCount, CurSize - ReadCount);
		ReadCount = CurSize;
	}
	return *this;
	unguard;
}

//==========================================================================
//
//  VMessageIn::ReadChar
//
//	Returns -1 and sets msg_badread if no more characters are available.
//
//==========================================================================

vuint8 VMessageIn::ReadByte()
{
	guard(VMessageIn::ReadByte);
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
//  VMessageIn::ReadShort
//
//==========================================================================

vint16 VMessageIn::ReadShort()
{
	guard(VMessageIn::ReadShort);
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
