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

class VRootWindow:public VWindow
{
	DECLARE_CLASS(VRootWindow, VWindow, 0);

	VRootWindow(void);
	void Init(void);
	void Init(VWindow *) { Sys_Error("Root canot have a parent"); }

	void PaintWindows(void);
	void TickWindows(float DeltaTime);

	static void StaticInit(void);
};

extern VRootWindow *GRoot;

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2002/05/29 16:51:50  dj_jl
//	Started a work on native Window classes.
//
//	Revision 1.4  2002/05/18 16:56:35  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//**************************************************************************
