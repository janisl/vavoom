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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// An output device.
class FOutputDevice
{
public:
	// FOutputDevice interface.
	virtual ~FOutputDevice() {}
	virtual void Serialise(const char* V, EName Event) = 0;

	// Simple text printing.
	void Log(const char* S);
	void Log(EName Type, const char* S);
	void Log(const VStr& S);
	void Log(EName Type, const VStr& S);
	void Logf(const char* Fmt, ...);
	void Logf(EName Type, const char* Fmt, ...);
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void C_Init(void);
boolean	C_Responder(event_t*);
void C_Drawer(void);
bool C_Active(void);
void C_Start(void);
void C_StartFull(void);
void C_Stop(void);
void C_ClearNotify(void);
void C_NotifyMessage(const char *msg);
void C_CenterMessage(const char *msg);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern FOutputDevice	*GCon;
