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
//**
//**	MESSAGE IO FUNCTIONS
//**
//**    Handles byte ordering and avoids alignment errors
//**
//**************************************************************************

class VMessage
{
public:
	VMessage()
	{
		AllowOverflow = false;
		Overflowed = false;
		Data = NULL;
		MaxSize = 0;
		CurSize = 0;
	}
	VMessage(vuint8* AData, vint32 ASize)
	{
		AllowOverflow = false;
		Overflowed = false;
		Data = AData;
		MaxSize = ASize;
		CurSize = 0;
	}

	void Alloc(vint32 startsize);
	void Free();
	void Clear();
	void* GetSpace(vint32 length);
	void Write(const void* data, vint32 length);
	bool CheckSpace(vint32 length) const
	{
		return CurSize + length <= MaxSize;
	}

	//
	//	writing functions
	//
	VMessage& operator << (vint8 c);
	VMessage& operator << (vuint8 c)  { return operator << ((vint8)c); }
	VMessage& operator << (vint16 c);
	VMessage& operator << (vuint16 c)  { return operator << ((vint16)c); }
	VMessage& operator << (vint32 c);
	VMessage& operator << (vuint32 c) { return operator << ((vint32)c); }
	VMessage& operator << (dword c) { return operator << ((vint32)c); }
	VMessage& operator << (float c);
	VMessage& operator << (const char* c);
	VMessage& operator << (const VStr& c);
	VMessage& operator << (const VMessage& msg);

	//
	//	reading functions
	//
	void BeginReading();
	VMessage& operator >> (vint8& c);
	VMessage& operator >> (vuint8& c)  { return operator >> ((vint8&)c); }
	VMessage& operator >> (vint16& c);
	VMessage& operator >> (vuint16& c)  { return operator >> ((vint16&)c); }
	VMessage& operator >> (vint32& c);
	VMessage& operator >> (vuint32& c) { return operator >> ((vint32&)c); }
	VMessage& operator >> (dword& c) { return operator >> ((vint32&)c); }
	VMessage& operator >> (float& f);
	VMessage& operator >> (const char*& s);
	VMessage& operator >> (VStr& s);
	VMessage& operator >> (VMessage& msg);

	vuint8 ReadByte();
	vint16 ReadShort();
	const char* ReadString();

	bool	AllowOverflow;	// if false, do a Sys_Error
	bool	Overflowed;		// set to true if the buffer size failed
	bool	BadRead;
	vuint8*	Data;
	vint32	MaxSize;
	vint32	CurSize;
	vint32	ReadCount;
};

inline float ByteToAngle(vuint8 angle)
{
	return (float)angle * 360.0 / 256.0;
}

inline vuint8 AngleToByte(float angle)
{
	return (vuint8)(angle * 256.0 / 360.0);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.7  2006/04/05 17:21:00  dj_jl
//	Merged size buffer with message class.
//
//	Revision 1.6  2006/03/29 22:32:27  dj_jl
//	Changed console variables and command buffer to use dynamic strings.
//	
//	Revision 1.5  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.4  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
