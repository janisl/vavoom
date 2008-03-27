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

class VLogSysError : public FOutputDevice
{
public:
	void Serialise(const char* V, EName Event);
};

class VLogHostError : public FOutputDevice
{
public:
	void Serialise(const char* V, EName Event);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

static VLogSysError		LogSysError;
static VLogHostError	LogHostError;

FOutputDevice*			GLogSysError = &LogSysError;
FOutputDevice*			GLogHostError = &LogHostError;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static char va_buffer[4][1024];
static int va_bufnum;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	Endianess handling, swapping 16bit and 32bit.
//	WAD files are stored little endian.
//
//==========================================================================

bool GBigEndian;
short (*LittleShort)(short);
short (*BigShort)(short);
int (*LittleLong)(int);
int (*BigLong)(int);
float (*LittleFloat)(float);
float (*BigFloat)(float);

//	Swaping
static short ShortSwap(short x)
{
	return  ((word)x >> 8) |
			((word)x << 8);
}
static short ShortNoSwap(short x)
{
	return x;
}
static int LongSwap(int x)
{
	return 	((vuint32)x >> 24) |
			(((vuint32)x >> 8) & 0xff00) |
			(((vuint32)x << 8) & 0xff0000) |
			((vuint32)x << 24);
}
static int LongNoSwap(int x)
{
	return x;
}
static float FloatSwap(float x)
{
	union { float f; long l; } a;
	a.f = x;
	a.l = LongSwap(a.l);
	return a.f;
}
static float FloatNoSwap(float x)
{
	return x;
}

void M_InitByteOrder()
{
	byte    swaptest[2] = {1, 0};

	// set the byte swapping variables in a portable manner
	if (*(short *)swaptest == 1)
	{
		GBigEndian = false;
		BigShort = ShortSwap;
		LittleShort = ShortNoSwap;
		BigLong = LongSwap;
		LittleLong = LongNoSwap;
		BigFloat = FloatSwap;
		LittleFloat = FloatNoSwap;
	}
	else
	{
		GBigEndian = true;
		BigShort = ShortNoSwap;
		LittleShort = ShortSwap;
		BigLong = LongNoSwap;
		LittleLong = LongSwap;
		BigFloat = FloatNoSwap;
		LittleFloat = FloatSwap;
	}
}

//==========================================================================
//
//	FOutputDevice implementation.
//
//==========================================================================

FOutputDevice::~FOutputDevice()
{
}
void FOutputDevice::Log(const char* S)
{
	Serialise(S, NAME_Log);
}
void FOutputDevice::Log(EName Type, const char* S)
{
	Serialise(S, Type);
}
void FOutputDevice::Log(const VStr& S)
{
	Serialise(*S, NAME_Log);
}
void FOutputDevice::Log(EName Type, const VStr& S)
{
	Serialise(*S, Type);
}
void FOutputDevice::Logf(const char* Fmt, ...)
{
	va_list argptr;
	char string[1024];
	
	va_start(argptr, Fmt);
	vsprintf(string, Fmt, argptr);
	va_end(argptr);

	Serialise(string, NAME_Log);
}
void FOutputDevice::Logf(EName Type, const char* Fmt, ...)
{
	va_list argptr;
	char string[1024];
	
	va_start(argptr, Fmt);
	vsprintf(string, Fmt, argptr);
	va_end(argptr);

	Serialise(string, Type);
}

//==========================================================================
//
//	VLogSysError
//
//==========================================================================

void VLogSysError::Serialise(const char* V, EName)
{
	Sys_Error(V);
}

//==========================================================================
//
//	VLogHostError
//
//==========================================================================

void VLogHostError::Serialise(const char* V, EName)
{
	Host_Error(V);
}

//==========================================================================
//
//	M_ReadFile
//
//==========================================================================

int M_ReadFile(const char* name, byte** buffer)
{
	int			handle;
	int			count;
	int			length;
	byte		*buf;
	
	handle = Sys_FileOpenRead(name);
	if (handle == -1)
	{
		Sys_Error("Couldn't open file %s", name);
	}
	length = Sys_FileSize(handle);
	buf = (byte*)Z_Malloc(length + 1);
	count = Sys_FileRead(handle, buf, length);
	buf[length] = 0;
	Sys_FileClose(handle);
	
	if (count < length)
	{
		Sys_Error("Couldn't read file %s", name);
	}

	*buffer = buf;
	return length;
}

//==========================================================================
//
//	M_WriteFile
//
//==========================================================================

bool M_WriteFile(const char* name, const void* source, int length)
{
	int		handle;
	int		count;
	
	handle = Sys_FileOpenWrite(name);
	if (handle == -1)
	{
		return false;
	}

	count = Sys_FileWrite(handle, source, length);
	Sys_FileClose(handle);
	
	if (count < length)
	{
		return false;
	}
		
	return true;
}

//==========================================================================
//
//  superatoi
//
//==========================================================================

int superatoi(const char *s)
{
	int n=0, r=10, x, mul=1;
	const char *c=s;

	for (; *c; c++)
	{
		x = (*c & 223) - 16;

		if (x == -3)
		{
			mul = -mul;
		}
		else if (x == 72 && r == 10)
		{
			n -= (r=n);
			if (!r) r=16;
			if (r<2 || r>36) return -1;
		}
		else
		{
			if (x>10) x-=39;
			if (x >= r) return -1;
			n = (n*r) + x;
		}
	}
	return(mul*n);
}

//==========================================================================
//
//	va
//
//	Very usefull function from QUAKE
//	Does a varargs printf into a temp buffer, so I don't need to have
// varargs versions of all text functions.
//	FIXME: make this buffer size safe someday
//
//==========================================================================

char *va(const char *text, ...)
{
	va_list args;

	va_bufnum = (va_bufnum + 1) & 3;
	va_start(args, text);
	vsprintf(va_buffer[va_bufnum], text, args);
	va_end(args);

	return va_buffer[va_bufnum];
}

//==========================================================================
//
//	PassFloat
//
//==========================================================================

int PassFloat(float f)
{
	union
	{
		float	f;
		int		i;
	} v;

	v.f = f;
	return v.i;
}

//==========================================================================
//
//	LookupColourName
//
//==========================================================================

static VStr LookupColourName(VStr& Name)
{
	guard(LookupColourName);
	//	Check that X111R6RGB lump exists.
	int Lump = W_CheckNumForName(NAME_x11r6rgb);
	if (Lump < 0)
	{
		GCon->Logf("X11R6RGB lump not found");
		return Name;
	}

	//	Read the lump.
	VStream* Strm = W_CreateLumpReaderNum(Lump);
	char* Buf = new char[Strm->TotalSize() + 1];
	Strm->Serialise(Buf, Strm->TotalSize());
	Buf[Strm->TotalSize()] = 0;
	char* BufEnd = Buf + Strm->TotalSize();
	delete Strm;

	vuint8 Col[3];
	int Count = 0;
	for (char* pBuf = Buf; pBuf < BufEnd;)
	{
		if ((vuint8)*pBuf <= ' ')
		{
			//	Skip whitespace
			pBuf++;
		}
		else if (Count == 0 && *pBuf == '!')
		{
			//	Skip comment
			while (pBuf < BufEnd && *pBuf != '\n')
			{
				pBuf++;
			}
		}
		else if (Count < 3)
		{
			//	Parse colour component
			char* pEnd;
			Col[Count] = strtoul(pBuf, &pEnd, 10);
			if (pEnd == pBuf)
			{
				GCon->Logf("Bad colour component value");
				break;
			}
			pBuf = pEnd;
			Count++;
		}
		else
		{
			//	Colour name
			char* Start = pBuf;
			while (pBuf < BufEnd && *pBuf != '\n')
			{
				pBuf++;
			}
			//	Skip trailing whitespace
			while (pBuf > Start && pBuf[-1] >= 0 && pBuf[-1] <= ' ')
			{
				pBuf--;
			}
			if (pBuf == Start)
			{
				GCon->Logf("Missing name of the colour");
				break;
			}
			*pBuf = 0;
			if ((size_t)(pBuf - Start) == Name.Length() && !Name.ICmp(Start))
			{
				char ValBuf[8];
				sprintf(ValBuf, "#%02x%02x%02x", Col[0], Col[1], Col[2]);
				delete[] Buf;
				return ValBuf;
			}
			Count = 0;
		}
	}
	delete[] Buf;
	return Name;
	unguard;
}

//==========================================================================
//
//	ParseHex
//
//==========================================================================

int ParseHex(const char* Str)
{
	guard(ParseHex);
	int Ret = 0;
	int Mul = 1;
	const char* c = Str;
	if (*c == '-')
	{
		c++;
		Mul = -1;
	}
	for (; *c; c++)
	{
		if (*c >= '0' && *c <= '9')
		{
			Ret = (Ret << 4) + *c - '0';
		}
		else if (*c >= 'a' && *c <= 'f')
		{
			Ret = (Ret << 4) + *c - 'a' + 10;
		}
		else if (*c >= 'A' && *c <= 'F')
		{
			Ret = (Ret << 4) + *c - 'A' + 10;
		}
	}
	return Ret * Mul;
	unguard;
}

//==========================================================================
//
//	M_ParseColour
//
//==========================================================================

vuint32 M_ParseColour(VStr Name)
{
	guard(M_ParseColour);
	if (!Name.Length())
	{
		return 0xff000000;
	}

	VStr Str = LookupColourName(Name);
	vuint8 Col[3];
	if (Str[0] == '#')
	{
		//	Looks like an HTML-style colur
		if (Str.Length() == 7)
		{
			//	#rrggbb format colour
			for (int i = 0; i < 3; i++)
			{
				char Val[3];
				Val[0] = Str[i * 2 + 1];
				Val[1] = Str[i * 2 + 2];
				Val[2] = 0;
				Col[i] = ParseHex(Val);
			}
		}
		else if (Str.Length() == 4)
		{
			//	#rgb format colur
			for (int i = 0; i < 3; i++)
			{
				char Val[3];
				Val[0] = Str[i + 1];
				Val[1] = Str[i + 1];
				Val[2] = 0;
				Col[i] = ParseHex(Val);
			}
		}
		else
		{
			//	Assume it's a bad colour value, set it to black
			Col[0] = 0;
			Col[1] = 0;
			Col[2] = 0;
		}
	}
	else
	{
		//	Treat like space separated hex values
		size_t Idx = 0;
		for (int i = 0; i < 3; i++)
		{
			//	Skip whitespace.
			while (Idx < Str.Length() && (vuint8)Str[Idx] <= ' ')
			{
				Idx++;
			}
			int Count = 0;
			char Val[3];
			while (Idx < Str.Length() && (vuint8)Str[Idx] > ' ')
			{
				if (Count < 2)
				{
					Val[Count++] = Str[Idx];
				}
				Idx++;
			}
			if (Count == 0)
			{
				Col[i] = 0;
			}
			else
			{
				if (Count == 1)
				{
					Val[1] = Val[0];
				}
				Val[2] = 0;
				Col[i] = ParseHex(Val);
			}
		}
	}
	return 0xff000000 | (Col[0] << 16) | (Col[1] << 8) | Col[2];
	unguard;
}
