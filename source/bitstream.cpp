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
//  VBitStreamWriter::VBitStreamWriter
//
//==========================================================================

VBitStreamWriter::VBitStreamWriter(vint32 AMax)
: Max(AMax)
, Pos(0)
{
	Data.SetNum((AMax + 7) >> 3);
	memset(Data.Ptr(), 0, (Max + 7) >> 3);
}

//==========================================================================
//
//  VBitStreamWriter::Serialise
//
//==========================================================================

void VBitStreamWriter::Serialise(void* data, vint32 length)
{
	SerialiseBits(data, length << 3);
}

//==========================================================================
//
//  VBitStreamWriter::SerialiseBits
//
//==========================================================================

void VBitStreamWriter::SerialiseBits(void* Src, vint32 Length)
{
	guard(VBitStreamWriter::SerialiseBits);
	if (!Length)
	{
		return;
	}

	if (Pos + Length > Max)
	{
		bError = true;
		return;
	}

	if (Length <= 8)
	{
		int Byte1 = Pos >> 3;
		int Byte2 = (Pos + Length - 1) >> 3;

		vuint8 Val = ((vuint8*)Src)[0] & ((1 << Length) - 1);
		int Shift = Pos & 7;
		if (Byte1 == Byte2)
		{
			Data[Byte1] |= Val << Shift;
		}
		else
		{
			Data[Byte1] |= Val << Shift;
			Data[Byte2] |= Val >> (8 - Shift);
		}
		Pos += Length;
		return;
	}

	int Bytes = Length >> 3;
	if (Bytes)
	{
		if (Pos & 7)
		{
			vuint8* pSrc = (vuint8*)Src;
			vuint8* pDst = (vuint8*)Data.Ptr() + (Pos >> 3);
			for (int i = 0; i < Bytes; i++, pSrc++, pDst++)
			{
				pDst[0] |= *pSrc << (Pos & 7);
				pDst[1] |= *pSrc >> (8 - (Pos & 7));
			}
		}
		else
		{
			memcpy(Data.Ptr() + ((Pos + 7) >> 3), Src, Length >> 3);
		}
		Pos += Length & ~7;
	}

	if (Length & 7)
	{
		int Byte1 = Pos >> 3;
		int Byte2 = (Pos + (Length & 7) - 1) >> 3;
		vuint8 Val = ((vuint8*)Src)[Length >> 3] & ((1 << (Length & 7)) - 1);
		int Shift = Pos & 7;
		if (Byte1 == Byte2)
		{
			Data[Byte1] |= Val << Shift;
		}
		else
		{
			Data[Byte1] |= Val << Shift;
			Data[Byte2] |= Val >> (8 - Shift);
		}
		Pos += Length & 7;
	}
	unguard;
}

//==========================================================================
//
//  VBitStreamWriter::WriteBit
//
//==========================================================================

void VBitStreamWriter::WriteBit(bool Bit)
{
	guard(VBitStreamWriter::WriteBit);
	if (Pos + 1 > Max)
	{
		bError = true;
		return;
	}

	if (Bit)
	{
		Data[Pos >> 3] |= 1 << (Pos & 7);
	}
	Pos++;
	unguard;
}

//==========================================================================
//
//  VBitStreamWriter::WriteInt
//
//==========================================================================

void VBitStreamWriter::WriteInt(vuint32 Val, vuint32 Maximum)
{
	guard(VBitStreamWriter::WriteInt);
	checkSlow(Val < Maximum);
	//	With maximum of 1 the only possible value is 0.
	if (Maximum <= 1)
	{
		return;
	}

	//	Check for the case when it will take all 32 bits.
	if (Maximum > 0x80000000)
	{
		*this << Val;
		return;
	}

	for (vuint32 Mask = 1; Mask && Mask < Maximum; Mask <<= 1)
	{
		if (Pos + 1 > Max)
		{
			bError = true;
			return;
		}
		if (Val & Mask)
		{
			Data[Pos >> 3] |= 1 << (Pos & 7);
		}
		Pos++;
	}
	unguard;
}

//==========================================================================
//
//  VBitStreamReader::VBitStreamReader
//
//==========================================================================

VBitStreamReader::VBitStreamReader(vuint8* Src, vint32 Length)
: Num(Length)
, Pos(0)
{
	bLoading = true;
	Data.SetNum((Length + 7) >> 3);
	if (Src)
	{
		memcpy(Data.Ptr(), Src, (Length + 7) >> 3);
	}
}

//==========================================================================
//
//  VBitStreamReader::SetData
//
//==========================================================================

void VBitStreamReader::SetData(VBitStreamReader& Src, int Length)
{
	guard(VBitStreamReader::SetData);
	Data.SetNum((Length + 7) >> 3);
	Src.SerialiseBits(Data.Ptr(), Length);
	Num = Length;
	Pos = 0;
	unguard;
}

//==========================================================================
//
//  VBitStreamReader::Serialise
//
//==========================================================================

void VBitStreamReader::Serialise(void* AData, int ALen)
{
	SerialiseBits(AData, ALen << 3);
}

//==========================================================================
//
//  VBitStreamReader::SerialiseBits
//
//==========================================================================

void VBitStreamReader::SerialiseBits(void* Dst, int Length)
{
	guard(VBitStreamReader::SerialiseBits);
	if (!Length)
	{
		return;
	}

	if (Pos + Length > Num)
	{
		bError = true;
		memset(Dst, 0, (Length + 7) >> 3);
		return;
	}

	if (Pos & 7)
	{
		int SrcPos = Pos >> 3;
		int Shift1 = Pos & 7;
		int Shift2 = 8 - Shift1;
		int Count = Length >> 3;
		for (int i = 0; i < Count; i++, SrcPos++)
		{
			((vuint8*)Dst)[i] = (Data[SrcPos] >> Shift1) |
				Data[SrcPos + 1] << Shift2;
		}
		if (Length & 7)
		{
			if ((Length & 7) > Shift2)
			{
				((vuint8*)Dst)[Count] = ((Data[SrcPos] >> Shift1) |
					Data[SrcPos + 1] << Shift2) & ((1 << (Length & 7)) - 1);
			}
			else
			{
				((vuint8*)Dst)[Count] = (Data[SrcPos] >> Shift1) &
					((1 << (Length & 7)) - 1);
			}
		}
	}
	else
	{
		int Count = Length >> 3;
		memcpy(Dst, Data.Ptr() + (Pos >> 3), Count);
		if (Length & 7)
		{
			((vuint8*)Dst)[Count] = Data[(Pos >> 3) + Count] & ((1 << (Length & 7)) - 1);
		}
	}
	Pos += Length;
	unguard;
}

//==========================================================================
//
//  VBitStreamReader::ReadBit
//
//==========================================================================

bool VBitStreamReader::ReadBit()
{
	guard(VBitStreamReader::ReadBit);
	if (Pos + 1 > Num)
	{
		bError = true;
		return false;
	}

	bool Ret = !!(Data[Pos >> 3] & (1 << (Pos & 7)));
	Pos++;
	return Ret;
	unguard;
}

//==========================================================================
//
//  VBitStreamReader::ReadInt
//
//==========================================================================

vuint32 VBitStreamReader::ReadInt(vuint32 Maximum)
{
	guard(VBitStreamReader::ReadInt);
	//	With maximum of 1 the only possible value is 0.
	if (Maximum <= 1)
	{
		return 0;
	}

	//	Check for the case when it will take all 32 bits.
	if (Maximum > 0x80000000)
	{
		return Streamer<vuint32>(*this);
	}

	vuint32 Val = 0;
	for (vuint32 Mask = 1; Mask && Mask < Maximum; Mask <<= 1)
	{
		if (Pos + 1 > Num)
		{
			bError = true;
			return 0;
		}
		if (Data[Pos >> 3] & (1 << (Pos & 7)))
		{
			Val |= Mask;
		}
		Pos++;
	}
	return Val;
	unguard;
}

//==========================================================================
//
//  VBitStreamReader::AtEnd
//
//==========================================================================

bool VBitStreamReader::AtEnd()
{
	return bError || Pos >= Num;
}
