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

// An output device.
class FOutputDevice : public VLogListener
{
public:
	// FOutputDevice interface.
	virtual ~FOutputDevice();

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

int superatoi(const char *s);

int PassFloat(float f);

int ParseHex(const char* Str);
vuint32 M_ParseColour(VStr Name);

void M_RgbToHsv(vuint8, vuint8, vuint8, vuint8&, vuint8&, vuint8&);
void M_RgbToHsv(float, float, float, float&, float&, float&);
void M_HsvToRgb(vuint8, vuint8, vuint8, vuint8&, vuint8&, vuint8&);
void M_HsvToRgb(float, float, float, float&, float&, float&);
