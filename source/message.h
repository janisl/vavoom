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

class VMessageIn : public VStream
{
public:
	VMessageIn()
	{
		Data = NULL;
		MaxSize = 0;
		CurSize = 0;
	}
	~VMessageIn()
	{
		Free();
	}

	vint8	BadRead;
	vuint8*	Data;
	vint32	MaxSize;
	vint32	CurSize;
	vint32	ReadCount;

	//	VStream interface
	void Serialise(void*, int);

	void Alloc(vint32 startsize);
	void Free();
	void SetData(const void* data, vint32 length);

	//
	//	reading functions
	//
	void BeginReading();
	VMessageIn& operator >> (vint8& c) { *this << c; return *this; }
	VMessageIn& operator >> (vuint8& c)  { return operator >> ((vint8&)c); }
	VMessageIn& operator >> (vint16& c) { *this << c; return *this; }
	VMessageIn& operator >> (vuint16& c)  { return operator >> ((vint16&)c); }
	VMessageIn& operator >> (vint32& c) { *this << c; return *this; }
	VMessageIn& operator >> (vuint32& c) { return operator >> ((vint32&)c); }
	VMessageIn& operator >> (float& f) { *this << f; return *this; }
	VMessageIn& operator >> (const char*& s) { s = ReadString(); return *this; }
	VMessageIn& operator >> (VStr& s) { *this << s; return *this; }
	VMessageIn& operator >> (VMessageIn& msg);

	vuint8 ReadByte() { vuint8 c; *this << c; return c; }
	vint16 ReadShort() { vint16	c; *this << c; return c; }
	const char* ReadString();
};

class VMessageOut : public VStream
{
public:
	VMessageOut()
	{
		AllowOverflow = false;
		Overflowed = false;
		Data = NULL;
		MaxSize = 0;
		CurSize = 0;
	}
	~VMessageOut()
	{
		Free();
	}

	vint8	AllowOverflow;	// if false, do a Sys_Error
	vint8	Overflowed;		// set to true if the buffer size failed
	vuint8*	Data;
	vint32	MaxSize;
	vint32	CurSize;

	//	VStream interface
	void Serialise(void*, int);

	void Alloc(vint32 startsize);
	void Free();
	void Clear();
	void* GetSpace(vint32 length);
	bool CheckSpace(vint32 length) const
	{
		return CurSize + length <= MaxSize;
	}

	//
	//	writing functions
	//
	VMessageOut& operator << (const VMessageOut& msg);
};

inline VMessageOut& operator << (VMessageOut& Msg, const vuint8& Val)
{
	((VStream&)Msg) << const_cast<vuint8&>(Val);
	return Msg;
}
inline VMessageOut& operator << (VMessageOut& Msg, vuint8& Val)
{
	((VStream&)Msg) << Val;
	return Msg;
}
inline VMessageOut& operator << (VMessageOut& Msg, const vuint16& Val)
{
	((VStream&)Msg) << const_cast<vuint16&>(Val);
	return Msg;
}
inline VMessageOut& operator << (VMessageOut& Msg, vuint16& Val)
{
	((VStream&)Msg) << Val;
	return Msg;
}
inline VMessageOut& operator << (VMessageOut& Msg, const vint16& Val)
{
	((VStream&)Msg) << const_cast<vint16&>(Val);
	return Msg;
}
inline VMessageOut& operator << (VMessageOut& Msg, vint16& Val)
{
	((VStream&)Msg) << Val;
	return Msg;
}
inline VMessageOut& operator << (VMessageOut& Msg, const vint32& Val)
{
	((VStream&)Msg) << const_cast<vint32&>(Val);
	return Msg;
}
inline VMessageOut& operator << (VMessageOut& Msg, vint32& Val)
{
	((VStream&)Msg) << Val;
	return Msg;
}
inline VMessageOut& operator << (VMessageOut& Msg, const vuint32& Val)
{
	((VStream&)Msg) << const_cast<vuint32&>(Val);
	return Msg;
}
inline VMessageOut& operator << (VMessageOut& Msg, vuint32& Val)
{
	((VStream&)Msg) << Val;
	return Msg;
}
inline VMessageOut& operator << (VMessageOut& Msg, const float& Val)
{
	((VStream&)Msg) << const_cast<float&>(Val);
	return Msg;
}
inline VMessageOut& operator << (VMessageOut& Msg, float& Val)
{
	((VStream&)Msg) << Val;
	return Msg;
}
inline VMessageOut& operator << (VMessageOut& Msg, const VStr& Val)
{
	((VStream&)Msg) << const_cast<VStr&>(Val);
	return Msg;
}
inline VMessageOut& operator << (VMessageOut& Msg, VStr& Val)
{
	((VStream&)Msg) << Val;
	return Msg;
}
inline VMessageOut& operator << (VMessageOut& Msg, const char* s)
{
	return Msg << VStr(s);
}

inline float ByteToAngle(vuint8 angle)
{
	return (float)angle * 360.0 / 256.0;
}

inline vuint8 AngleToByte(float angle)
{
	return (vuint8)(angle * 256.0 / 360.0);
}
