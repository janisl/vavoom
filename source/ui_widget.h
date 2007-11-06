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

struct VClipRect
{
	float		OriginX;	//	Origin of the widget, in absolute coordinates.
	float		OriginY;

	float		ScaleX;		//	Accomulative scale.
	float		ScaleY;

	float		ClipX1;		//	Clipping rectangle, in absolute coordinates.
	float		ClipY1;
	float		ClipX2;
	float		ClipY2;

	bool HasArea() const
	{
		return ClipX1 < ClipX2 && ClipY1 < ClipY2;
	}
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

	VClipRect			ClipRect;

	void AddChild(VWidget*);
	void RemoveChild(VWidget*);

	void ClipTree();

	void TransferAndClipRect(float&, float&, float&, float&, float&, float&,
		float&, float&) const;

	friend class VWindow;
	friend class VRootWindow;

public:
	//	Methods to move widget on top or bottom.
	void Lower();
	void Raise();

	//	Methods to set position, size and scale.
	void SetPos(int NewX, int NewY)
	{
		SetConfiguration(NewX, NewY, SizeWidth, SizeHeight, SizeScaleX,
			SizeScaleY);
	}
	void SetX(int NewX)
	{
		SetPos(NewX, PosY);
	}
	void SetY(int NewY)
	{
		SetPos(PosX, NewY);
	}
	void SetSize(int NewWidth, int NewHeight)
	{
		SetConfiguration(PosX, PosY, NewWidth, NewHeight, SizeScaleX,
			SizeScaleY);
	}
	void SetWidth(int NewWidth)
	{
		SetSize(NewWidth, SizeHeight);
	}
	void SetHeight(int NewHeight)
	{
		SetSize(SizeWidth, NewHeight);
	}
	void SetScale(float NewScaleX, float NewScaleY)
	{
		SetConfiguration(PosX, PosY, SizeWidth, SizeHeight, NewScaleX,
			NewScaleY);
	}
	void SetConfiguration(int, int, int, int, float = 1.0, float = 1.0);

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
	virtual void OnConfigurationChanged()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_OnConfigurationChanged);
	}

	void DrawPic(int, int, int, float = 1.0);
	void DrawShadowedPic(int, int, int);
	void FillRectWithFlat(int, int, int, int, VName);
	void ShadeRect(int, int, int, int, float);

	void DrawString(int, int, const VStr&);
	void DrawText(int, int, const VStr&);
	int DrawTextW(int, int, const VStr&, int);

	DECLARE_FUNCTION(Raise)
	DECLARE_FUNCTION(Lower)

	DECLARE_FUNCTION(SetPos)
	DECLARE_FUNCTION(SetX)
	DECLARE_FUNCTION(SetY)
	DECLARE_FUNCTION(SetSize)
	DECLARE_FUNCTION(SetWidth)
	DECLARE_FUNCTION(SetHeight)
	DECLARE_FUNCTION(SetScale)
	DECLARE_FUNCTION(SetConfiguration)

	DECLARE_FUNCTION(DrawPic)
	DECLARE_FUNCTION(DrawShadowedPic)
	DECLARE_FUNCTION(FillRectWithFlat)
	DECLARE_FUNCTION(ShadeRect)

	DECLARE_FUNCTION(DrawText)
	DECLARE_FUNCTION(DrawTextW)
};
