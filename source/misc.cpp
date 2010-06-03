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

// CODE --------------------------------------------------------------------

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
	Strm = NULL;

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
				Buf = NULL;
				return ValBuf;
			}
			Count = 0;
		}
	}
	delete[] Buf;
	Buf = NULL;
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

//==========================================================================
//
//	M_RgbToHsv
//
//==========================================================================

void M_RgbToHsv(vuint8 r, vuint8 g, vuint8 b, vuint8& h, vuint8& s, vuint8& v)
{
	guardSlow(M_RgbToHsv);
	vuint8 min = MIN(MIN(r, g), b);
	vuint8 max = MAX(MAX(r, g), b);
	v = max;
	if (max == min)
	{
		//	Gray.
		s = 0;
		h = 0;
		return;
	}
	s = 255 - 255 * min / max;
	if (max == r)
	{
		h = 0 + 43 * (g - b) / (max - min);
	}
	else if (max == g)
	{
		h = 85 + 43 * (b - r) / (max - min);
	}
	else
	{
		h = 171 + 43 * (r - g) / (max - min);
	}
	unguardSlow;
}

//==========================================================================
//
//	M_RgbToHsv
//
//==========================================================================

void M_RgbToHsv(float r, float g, float b, float& h, float& s, float& v)
{
	guardSlow(M_RgbToHsv);
	float min = MIN(MIN(r, g), b);
	float max = MAX(MAX(r, g), b);
	v = max;
	if (max == min)
	{
		//	Gray.
		s = 0;
		h = 0;
		return;
	}
	s = 1.0 - min / max;
	if (max == r)
	{
		h = 0.0 + 60.0 * (g - b) / (max - min);
		if (h < 0)
		{
			h += 360.0;
		}
	}
	else if (max == g)
	{
		h = 120.0 + 60.0 * (b - r) / (max - min);
	}
	else
	{
		h = 240.0 + 60.0 * (r - g) / (max - min);
	}
	unguardSlow;
}

//==========================================================================
//
//	M_HsvToRgb
//
//==========================================================================

void M_HsvToRgb(vuint8 h, vuint8 s, vuint8 v, vuint8& r, vuint8& g, vuint8& b)
{
	guardSlow(M_HsvToRgb);
	if (s == 0)
	{
		//	Gray.
		r = v;
		g = v;
		b = v;
		return;
	}
	int i = h / 43;
	vuint8 f = (h - i * 43) * 255 / 43;
	vuint8 p = v * (255 - s) / 255;
	vuint8 q = v * (255 - f * s / 255) / 255;
	vuint8 t = v * (255 - (255 - f) * s / 255) / 255;
	switch (i)
	{
	case 0:
		r = v;
		g = t;
		b = p;
		break;
	case 1:
		r = q;
		g = v;
		b = p;
		break;
	case 2:
		r = p;
		g = v;
		b = t;
		break;
	case 3:
		r = p;
		g = q;
		b = v;
		break;
	case 4:
		r = t;
		g = p;
		b = v;
		break;
	default:
		r = v;
		g = p;
		b = q;
		break;
	}
	unguardSlow;
}

//==========================================================================
//
//	M_HsvToRgb
//
//==========================================================================

void M_HsvToRgb(float h, float s, float v, float& r, float& g, float& b)
{
	guardSlow(M_HsvToRgb);
	if (s == 0)
	{
		//	Gray.
		r = v;
		g = v;
		b = v;
		return;
	}
	int i = (int)(h / 60.0);
	float f = h / 60.0 - i;
	float p = v * (1.0 - s);
	float q = v * (1.0 - f * s);
	float t = v * (1.0 - (1.0 - f) * s);
	switch (i)
	{
	case 0:
		r = v;
		g = t;
		b = p;
		break;
	case 1:
		r = q;
		b = v;
		b = p;
		break;
	case 2:
		r = p;
		g = v;
		b = t;
		break;
	case 3:
		r = p;
		g = q;
		b = v;
		break;
	case 4:
		r = t;
		g = p;
		b = v;
		break;
	default:
		r = v;
		g = p;
		b = q;
		break;
	}
	unguardSlow;
}
