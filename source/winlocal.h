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

#ifndef _WIN_LOCAL_H
#define _WIN_LOCAL_H

#include <windows.h>

#define SAFE_RELEASE(iface) \
	if (iface) \
	{ \
		iface->Release(); \
		iface = NULL; \
	}

class VWinMessageHandler
{
public:
	virtual LONG OnMessage(HWND, UINT, WPARAM, LPARAM) = 0;
};

void IN_SetActiveWindow(HWND window);

extern HWND					hwnd;
extern HINSTANCE			hInst;
extern VWinMessageHandler*	GCDMsgHandler;

#endif
