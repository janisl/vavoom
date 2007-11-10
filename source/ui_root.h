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

class VRootWidget : public VWidget
{
	DECLARE_CLASS(VRootWidget, VWidget, 0)

private:
	enum
	{
		//	True if mouse cursor is currently enabled.
		RWF_MouseEnabled		= 0x0001,
	};
	vuint32				RootFlags;

	//	Current mouse cursor position.
	int					MouseX;
	int					MouseY;

	//	Current mouse cursor graphic.
	int					MouseCursorPic;

	void MouseMoveEvent(int, int);

public:
	VRootWidget();
	void Init();
	void Init(VWidget*) { Sys_Error("Root canot have a parent"); }

	void DrawWidgets();
	void TickWidgets(float DeltaTime);
	bool Responder(event_t* Event);

	void SetMouse(bool MouseOn);

	static void StaticInit();

	DECLARE_FUNCTION(SetMouse)
};

extern VRootWidget*		GRoot;
