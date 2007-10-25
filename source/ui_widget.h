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

struct VClipRectNew
{
	float		OriginX;	//	Origin of the widget, in absolute coordinates.
	float		OriginY;

	float		ScaleX;		//	Accomulative scale.
	float		ScaleY;

	float		ClipX1;		//	Clipping rectangle, in absolute coordinates.
	float		ClipY1;
	float		ClipX2;
	float		ClipY2;
};

class VWidget : public VObject
{
	DECLARE_CLASS(VWidget, VObject, 0)

private:
	//	Parent container widget.
	VWidget*			ParentWidget;
	//	Linked list of child widgets.
	VWidget*			FirstChildWidget;
	VWidget*			LastChildWidget;
	//	Links in the linked list of widgets.
	VWidget*			PrevWidget;
	VWidget*			NextWidget;

	//	Position of the widget in the parent widget.
	int					PosX;
	int					PosY;
	//	Size of the child area of the widget.
	int					SizeWidth;
	int					SizeHeight;
	//	Scaling of the widget.
	float				SizeScaleX;
	float				SizeScaleY;

	VClipRectNew		ClipRectNew;

	void AddChild(VWidget*);
	void RemoveChild(VWidget*);

	friend class VWindow;
	friend class VRootWindow;

public:
	//	Methods to move widget on top or bottom.
	void Lower();
	void Raise();

	virtual void OnChildAdded(VWidget* Child)
	{
		P_PASS_SELF;
		P_PASS_REF(Child);
		EV_RET_VOID(NAME_OnChildAdded);
	}
	virtual void OnChildRemoved(VWidget* Child)
	{
		P_PASS_SELF;
		P_PASS_REF(Child);
		EV_RET_VOID(NAME_OnChildRemoved);
	}

	void TestDrawImage(int X, int Y, int Handle);

	DECLARE_FUNCTION(Raise)
	DECLARE_FUNCTION(Lower)

	DECLARE_FUNCTION(TestDrawImage)
};
