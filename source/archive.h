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

//
// Archive class. Used for loading, saving, and garbage collecting
// in a byte order neutral way.
//
class FArchive
{
public:

	// FArchive interface.
	virtual ~FArchive()
	{}
	virtual void Serialize(void* V, int Length)
	{}
	/*virtual void SerializeBits(void* V, int LengthBits)
	{
		Serialize(V, (LengthBits + 7) / 8);
		if (IsLoading())
			((byte *)V)[LengthBits / 8] &= ((1 << (LengthBits & 7)) - 1);
	}*/
	/*virtual void SerializeInt(dword& Value, dword Max)
	{
		*this << *(dword*)Value;
	}*/

	// Constructor.
	FArchive()
	:	ArIsLoading		(0)
	,	ArIsSaving		(0)
//	,   ArIsTrans		(0)
	,	ArIsPersistent  (0)
//	,   ArIsError       (0)
	{}

	// Status accessors.
	bool IsLoading()	{return ArIsLoading;}
	bool IsSaving()		{return ArIsSaving;}
//	bool IsTrans()		{return ArIsTrans;}
	bool IsPersistent()	{return ArIsPersistent;}
//	bool IsError()		{return ArIsError;}

	// Friend archivers.
/*	friend FArchive& operator << (FArchive& Ar, char& C)
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
		Ar.Serialize(&W, sizeof(W));
		return Ar;
	}
	friend FArchive& operator << (FArchive& Ar, short& S)
	{
		Ar.Serialize(&S, sizeof(S));
		return Ar;
	}
	friend FArchive& operator << (FArchive& Ar, dword& D)
	{
		Ar.Serialize(&D, sizeof(D));
		return Ar;
	}
	friend FArchive& operator << (FArchive& Ar, int& I)
	{
		Ar.Serialize(&I, sizeof(I));
		return Ar;
	}*/
	friend FArchive& operator << (FArchive& Ar, float& F)
	{
		Ar.Serialize(&F, sizeof(F));
		return Ar;
	}
	/*friend FArchive& operator << (FArchive& Ar, double& F)
	{
		Ar.Serialize(&F, sizeof(F));
		return Ar;
	}*/

protected:
	// Status variables.
	bool ArIsLoading;
	bool ArIsSaving;
//	bool ArIsTrans;
	bool ArIsPersistent;
//	bool ArIsError;
};

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2001/12/18 19:03:16  dj_jl
//	A lots of work on VObject
//
//**************************************************************************
