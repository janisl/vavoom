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

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// An output device.
class FOutputDevice
{
public:
	// FOutputDevice interface.
	virtual void Serialise(const char* V, EName Event) = 0;

	// Simple text printing.
	void Log(const char* S);
	void Log(EName Type, const char* S);
	void Log(const FString& S);
	void Log(EName Type, const FString& S);
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

//**************************************************************************
//
//	$Log$
//	Revision 1.9  2004/12/03 16:15:46  dj_jl
//	Implemented support for extended ACS format scripts, functions, libraries and more.
//
//	Revision 1.8  2002/07/23 16:29:55  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.7  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.6  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.5  2001/08/15 17:26:35  dj_jl
//	Made console not active when closing
//	
//	Revision 1.4  2001/08/07 16:49:26  dj_jl
//	Added C_Active
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
