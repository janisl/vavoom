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
//**	Streams for loading and saving data.
//**
//**************************************************************************

//
//	VStream
//
//	Base class for various streams.
//
class VStream
{
protected:
	bool		bLoading;	//	Are we loading or saving?
	bool		bError;		//	Did we have any errors?

public:
	//	Structors.
	VStream()
	: bLoading(false)
	, bError(false)
	{}
	virtual ~VStream();

	//	Status requests.
	bool IsLoading()
	{
		return bLoading;
	}
	bool IsSaving()
	{
		return !bLoading;
	}
	bool IsError()
	{
		return bError;
	}

	//	Stream interface.
	virtual void Serialise(void*, int);
	virtual void SerialiseBits(void*, int);
	virtual void SerialiseInt(vuint32&, vuint32);
	virtual void Seek(int);
	virtual int Tell();
	virtual int TotalSize();
	virtual bool AtEnd();
	virtual void Flush();
	virtual bool Close();

	//	Interface functions for objects and classes streams.
	virtual VStream& operator<<(VName&);
	virtual VStream& operator<<(VObject*&);
	virtual void SerialiseStructPointer(void*&, VStruct*);
	virtual VStream& operator<<(VMemberBase*&);

	//	Serialise integers in particular byte order.
	void SerialiseLittleEndian(void*, int);
	void SerialiseBigEndian(void*, int);

	//	Stream serialisation operators.
	friend VStream& operator<<(VStream& Strm, vint8& Val)
	{
		Strm.Serialise(&Val, 1);
		return Strm;
	}
	friend VStream& operator<<(VStream& Strm, vuint8& Val)
	{
		Strm.Serialise(&Val, 1);
		return Strm;
	}
	friend VStream& operator<<(VStream& Strm, vint16& Val)
	{
		Strm.SerialiseLittleEndian(&Val, sizeof(Val));
		return Strm;
	}
	friend VStream& operator<<(VStream& Strm, vuint16& Val)
	{
		Strm.SerialiseLittleEndian(&Val, sizeof(Val));
		return Strm;
	}
	friend VStream& operator<<(VStream& Strm, vint32& Val)
	{
		Strm.SerialiseLittleEndian(&Val, sizeof(Val));
		return Strm;
	}
	friend VStream& operator<<(VStream& Strm, vuint32& Val)
	{
		Strm.SerialiseLittleEndian(&Val, sizeof(Val));
		return Strm;
	}
	friend VStream& operator<<(VStream& Strm, float& Val)
	{
		Strm.SerialiseLittleEndian(&Val, sizeof(Val));
		return Strm;
	}
	friend VStream& operator<<(VStream& Strm, double& Val)
	{
		Strm.SerialiseLittleEndian(&Val, sizeof(Val));
		return Strm;
	}
};

//
//	Stream reader helper.
//
template<class T> T Streamer(VStream& Strm)
{
	T Val;
	Strm << Val;
	return Val;
}

//
//	VStreamCompactIndex
//
//	Class for serialising integer values in a compact way.
//
class VStreamCompactIndex
{
public:
	vint32		Val;
	friend VStream& operator<<(VStream&, VStreamCompactIndex&);
};
#define STRM_INDEX(val) \
	(*(VStreamCompactIndex*)&(val))

//
//	VMemoryStream
//
//	Stream for reading and writing in memory.
//
class VMemoryStream : public VStream
{
protected:
	TArray<vuint8>	Array;
	int				Pos;

public:
	//	Initialise empty writing stream.
	VMemoryStream();
	//	Initialise reading streams.
	VMemoryStream(void*, int);
	VMemoryStream(const TArray<vuint8>&);

	void Serialise(void*, int);
	void Seek(int);
	int Tell();
	int TotalSize();

	void BeginRead()
	{
		bLoading = true;
	}
	void BeginWrite()
	{
		bLoading = false;
	}
	TArray<vuint8>& GetArray()
	{
		return Array;
	}
};
