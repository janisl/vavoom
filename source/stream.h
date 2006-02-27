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
//**	Copyright (C) 1999-2002 J�nis Legzdi��
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
	virtual void Seek(int);
	virtual int Tell();
	virtual int TotalSize();
	virtual bool AtEnd();
	virtual void Flush();
	virtual bool Close();

	//	Interface functions for objects and classes streams.
	virtual VStream& operator<<(FName&);
	virtual void SerialiseReference(VObject*&, VClass*);
	virtual void SerialiseStructPointer(void*&, VStruct*);

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
	TArray<byte, PU_STATIC>	Array;
	int						Pos;

public:
	//	Initialise empty writing stream.
	VMemoryStream();
	//	Initialise reading streams.
	VMemoryStream(void*, int);
	VMemoryStream(const TArray<byte, PU_STATIC>&);

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
	TArray<byte, PU_STATIC>& GetArray()
	{
		return Array;
	}
};

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2006/02/27 18:44:25  dj_jl
//	Serialisation of indexes in a compact way.
//
//	Revision 1.2  2006/02/25 17:14:19  dj_jl
//	Implemented proper serialisation of the objects.
//	
//	Revision 1.1  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//	
//**************************************************************************
