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

#define Random()	((float)(rand() & 0x7fff) / (float)0x8000)

//	Endianess handling
void M_InitByteOrder();
extern bool GBigEndian;
extern short (*LittleShort)(short);
extern short (*BigShort)(short);
extern int (*LittleLong)(int);
extern int (*BigLong)(int);
extern float (*LittleFloat)(float);
extern float (*BigFloat)(float);

// An output device.
class FOutputDevice
{
public:
	// FOutputDevice interface.
	virtual ~FOutputDevice();
	virtual void Serialise(const char* V, EName Event) = 0;

	// Simple text printing.
	void Log(const char* S);
	void Log(EName Type, const char* S);
	void Log(const VStr& S);
	void Log(EName Type, const VStr& S);
	void Logf(const char* Fmt, ...);
	void Logf(EName Type, const char* Fmt, ...);
};

//	Error logs.
extern FOutputDevice*		GLogSysError;
extern FOutputDevice*		GLogHostError;

int M_ReadFile(const char* name, byte** buffer);
bool M_WriteFile(const char* name, const void* source, int length);

int superatoi(const char *s);

char *va(const char *text, ...) __attribute__ ((format(printf, 1, 2)));
int PassFloat(float f);
