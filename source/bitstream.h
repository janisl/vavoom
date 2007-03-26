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
//**	MESSAGE IO FUNCTIONS
//**
//**    Handles byte ordering and avoids alignment errors
//**
//**************************************************************************

//
//	VBitStreamWriter
//
class VBitStreamWriter : public VStream
{
protected:
	TArray<vuint8>		Data;
	vint32				Max;
	vint32				Pos;

public:
	VBitStreamWriter(vint32);
	void Serialise(void*, int);
	void SerialiseBits(void*, int);
	void WriteBit(bool);
	void WriteInt(vuint32, vuint32);
	vuint8* GetData()
	{
		return Data.Ptr();
	}
	int GetNumBits()
	{
		return Pos;
	}
	int GetNumBytes()
	{
		return (Pos + 7) >> 3;
	}
};

//
//	VBitStreamReader
//
class VBitStreamReader : public VStream
{
protected:
	TArray<vuint8>		Data;
	vint32				Num;
	vint32				Pos;

public:
	VBitStreamReader(vuint8* = NULL, vint32 = 0);
	void SetData(VBitStreamReader&, int);
	void Serialise(void*, int);
	void SerialiseBits(void*, int);
	bool ReadBit();
	vuint32 ReadInt(vuint32);
	bool AtEnd();
	vuint8* GetData()
	{
		return Data.Ptr();
	}
	int GetNumBits()
	{
		return Num;
	}
	int GetNumBytes()
	{
		return (Num + 7) >> 3;
	}
	int GetPosBits()
	{
		return (Num + 7) >> 3;
	}
};
