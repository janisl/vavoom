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

class VChannel;

class VMessageIn : public VBitStreamReader
{
public:
	VMessageIn(vuint8* Src = NULL, vint32 Length = 0)
	: VBitStreamReader(Src, Length)
	, Next(NULL)
	{
	}

	VMessageIn*		Next;
	vuint8			ChanType;
	vint32			ChanIndex;
	bool			bReliable;		//	Reliable message
	bool			bOpen;			//	Open channel message
	bool			bClose;			//	Close channel message
	vuint32			Sequence;		//	Reliable message sequence ID

	vuint8 ReadByte() { vuint8 c; *this << c; return c; }
	vint16 ReadShort() { vint16	c; *this << c; return c; }
	VStr ReadString() { VStr S; *this << S; return S; }
};

class VMessageOut : public VBitStreamWriter
{
public:
	VMessageOut(vint32 AMax)
	: VBitStreamWriter(AMax)
	, ChanType(0)
	, ChanIndex(0)
	, AllowOverflow(false)
	, bReliable(false)
	, bOpen(false)
	, bClose(false)
	, bReceivedAck(false)
	{
	}
	VMessageOut(VChannel*);

	VMessageOut*	Next;
	vuint8			ChanType;
	vint32			ChanIndex;
	vint8			AllowOverflow;	// if false, do a Sys_Error
	bool			bReliable;		//	Needs ACK or not
	bool			bOpen;
	bool			bClose;
	bool			bReceivedAck;
	vuint32			Sequence;		//	Reliable message sequence ID
	double			Time;			//	Time this message has been sent
	vuint32			PacketId;		//	Packet in which this message was sent

	void SerialiseBits(void*, int);
	void Clear();
	bool CheckSpaceBits(vint32 length) const
	{
		return Pos + length <= Max;
	}

	//
	//	writing functions
	//
	VMessageOut& operator << (VMessageOut& msg);
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
