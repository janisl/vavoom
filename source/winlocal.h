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
//**	Copyright (C) 1999-2002 J�nis Legzdi��
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

#ifndef _WIN_LOCAL_H
#define _WIN_LOCAL_H

// HEADER FILES ------------------------------------------------------------

#define boolean		fucked_boolean
#define byte		fucked_byte
#include <windows.h>
#undef MINCHAR
#undef MINSHORT
#undef MINLONG
#undef MAXCHAR
#undef MAXSHORT
#undef MAXLONG
#undef boolean
#undef byte

// MACROS ------------------------------------------------------------------

#define SAFE_RELEASE(iface) \
	if (iface) \
	{ \
		iface->Release(); \
		iface = NULL; \
	}

// TYPES -------------------------------------------------------------------

class VWinMessageHandler
{
	virtual LONG OnMessage(HWND, UINT, WPARAM, LPARAM) = 0;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void IN_SetActiveWindow(HWND window);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern HWND					hwnd;
extern HINSTANCE			hInst;
extern VWinMessageHandler*	GCDMsgHandler;

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.7  2005/09/14 11:58:48  dj_jl
//	Created CD audio device class.
//
//	Revision 1.6  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.5  2001/10/04 17:26:10  dj_jl
//	Moved SAFE_RELEASE here
//	
//	Revision 1.4  2001/08/30 17:42:57  dj_jl
//	Changes for OpenGL window
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
