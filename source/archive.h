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

//
// Archive class. Used for loading, saving, and garbage collecting
// in a byte order neutral way.
//
class FArchive
{
public:
	// FArchive interface.
	virtual ~FArchive(void)
	{}
	virtual void Serialize(void*, int)
	{}
	virtual void SerializeBits(void* V, int LengthBits)
	{
		Serialize(V, (LengthBits + 7) / 8);
		if (IsLoading())
			((byte *)V)[LengthBits / 8] &= ((1 << (LengthBits & 7)) - 1);
	}
	virtual void SerializeInt(dword& Value, dword)
	{
		*this << *(dword*)Value;
	}
/*	virtual void Preload(VObject* Object)
	{}
	virtual void CountBytes(size_t InNum, size_t InMax)
	{}*/
	virtual FArchive& operator<<(FName&)
	{
		return *this;
	}
	virtual FArchive& operator<<(VObject*&)
	{
		return *this;
	}
	virtual int MapName(FName*)
	{
		return 0;
	}
	virtual int MapObject(VObject*)
	{
		return 0;
	}
	virtual int Tell(void)
	{
		return -1;
	}
	virtual int TotalSize(void)
	{
		return -1;
	}
	virtual bool AtEnd(void)
	{
		int Pos = Tell();
		return Pos != -1 && Pos >= TotalSize();
	}
	virtual void Seek(int)
	{}
	virtual void Flush(void)
	{}
	virtual bool Close(void)
	{
		return !ArIsError;
	}
	virtual bool GetError(void)
	{
		return ArIsError;
	}

	// Hardcoded datatype routines that may not be overridden.
	FArchive& ByteOrderSerialize(void* V, int Length)
	{
#if defined __i386__ || defined _M_IX86
		Serialize(V, Length);
#else
		if (GBigEndian && ArIsPersistent)
		{
			// Transferring between memory and file, so flip the byte order.
			for (int i = Length - 1; i >= 0; i--)
				Serialize((byte*)V + i, 1);
		}
		else
		{
			// Transferring around within memory, so keep the byte order.
			Serialize(V, Length);
		}
#endif
		return *this;
	}

	// Constructor.
	FArchive()
	:	ArIsLoading		(0)
	,	ArIsSaving		(0)
//	,   ArIsTrans		(0)
	,	ArIsPersistent  (0)
	,   ArIsError       (0)
	{}

	// Status accessors.
	bool IsLoading()	{return ArIsLoading;}
	bool IsSaving()		{return ArIsSaving;}
//	bool IsTrans()		{return ArIsTrans;}
	bool IsPersistent()	{return ArIsPersistent;}
	bool IsError()		{return ArIsError;}

	// Friend archivers.
	friend FArchive& operator << (FArchive& Ar, char& C)
	{
		Ar.Serialize(&C, 1);
		return Ar;
	}
	friend FArchive& operator << (FArchive& Ar, byte& B)
	{
		Ar.Serialize(&B, 1);
		return Ar;
	}
	friend FArchive& operator << (FArchive& Ar, signed char& B)
	{
		Ar.Serialize(&B, 1);
		return Ar;
	}
	friend FArchive& operator << (FArchive& Ar, word& W)
	{
		Ar.ByteOrderSerialize(&W, sizeof(W));
		return Ar;
	}
	friend FArchive& operator << (FArchive& Ar, short& S)
	{
		Ar.ByteOrderSerialize(&S, sizeof(S));
		return Ar;
	}
	friend FArchive& operator << (FArchive& Ar, dword& D)
	{
		Ar.ByteOrderSerialize(&D, sizeof(D));
		return Ar;
	}
	friend FArchive& operator << (FArchive& Ar, int& I)
	{
		Ar.ByteOrderSerialize(&I, sizeof(I));
		return Ar;
	}
	friend FArchive& operator << (FArchive& Ar, float& F)
	{
		Ar.ByteOrderSerialize(&F, sizeof(F));
		return Ar;
	}
	friend FArchive& operator << (FArchive& Ar, double& F)
	{
		Ar.ByteOrderSerialize(&F, sizeof(F));
		return Ar;
	}

protected:
	// Status variables.
	bool ArIsLoading;
	bool ArIsSaving;
//	bool ArIsTrans;
	bool ArIsPersistent;
	bool ArIsError;
};

//
// Archive constructor.
//
template <class T> T Arctor(FArchive& Ar)
{
	T Tmp;
	Ar << Tmp;
	return Tmp;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//
//	Revision 1.3  2002/02/15 19:12:53  dj_jl
//	Got rid of warnings
//	
//	Revision 1.2  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2001/12/18 19:03:16  dj_jl
//	A lots of work on VObject
//	
//**************************************************************************
