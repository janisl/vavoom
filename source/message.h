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
//**
//**	MESSAGE IO FUNCTIONS
//**
//**    Handles byte ordering and avoids alignment errors
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class TMessage : public TSizeBuf
{
 public:
	TMessage(void) { }
	TMessage(byte *AData, int ASize) : TSizeBuf(AData, ASize) { }

	//
	//	writing functions
	//
	TMessage &operator << (char c);
	TMessage &operator << (byte c)  { return operator << ((char)c); }
	TMessage &operator << (short c);
	TMessage &operator << (word c)  { return operator << ((short)c); }
	TMessage &operator << (int c);
	TMessage &operator << (long c)  { return operator << ((int)c); }
	TMessage &operator << (dword c) { return operator << ((int)c); }
	TMessage &operator << (float c);
	TMessage &operator << (const char *c);
	TMessage &operator << (const TMessage &msg);

	//
	//	reading functions
	//
	void BeginReading(void);
	TMessage &operator >> (char &c);
	TMessage &operator >> (byte &c)  { return operator >> ((char&)c); }
	TMessage &operator >> (short &c);
	TMessage &operator >> (word &c)  { return operator >> ((short&)c); }
	TMessage &operator >> (int &c);
	TMessage &operator >> (long &c)  { return operator >> ((int&)c); }
	TMessage &operator >> (dword &c) { return operator >> ((int&)c); }
	TMessage &operator >> (float &f);
	TMessage &operator >> (char *&s);
	TMessage &operator >> (TMessage &msg);

	byte ReadByte(void);
	short ReadShort(void);
	char *ReadString(void);

	int			readcount;
	boolean		badread;
};

inline float ByteToAngle(byte angle)
{
	return (float)angle * 360.0 / 256.0;
}

inline byte AngleToByte(float angle)
{
	return (byte)(angle * 256.0 / 360.0);
}

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//**************************************************************************
//
//	$Log$
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
