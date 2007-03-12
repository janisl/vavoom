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
	bLoading = true;
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
		memset(AData, -1, ALen);
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
//  VMessageIn::ReadString
//
//==========================================================================

const char* VMessageIn::ReadString()
{
	guard(VMessage::ReadString);
	static char		string[2048];
	VStr S;
	*this << S;
	VStr::Cpy(string, *S);

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
