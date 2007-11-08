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

#include "gamedefs.h"
#include "cl_local.h"
#include "ui.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(V, Widget);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VWidget::CreateNewWidget
//
//==========================================================================

VWidget* VWidget::CreateNewWidget(VClass* AClass, VWidget* AParent)
{
	guard(VWidget::CreateNewWidget);
	VWidget* W = (VWidget*)StaticSpawnObject(AClass);
	W->Init(AParent);
	return W;
	unguardf(("(%s)", AClass->GetName()));
}

//==========================================================================
//
//	VWidget::Init
//
//==========================================================================

void VWidget::Init(VWidget* AParent)
{
	guard(VWidget::Init);
	ParentWidget = AParent;
	if (ParentWidget)
	{
		ParentWidget->AddChild(this);
	}
	ClipTree();
	OnCreate();
	unguard;
}

//==========================================================================
//
//	VWidget::Destroy
//
//==========================================================================

void VWidget::Destroy()
{
	guard(VWidget::Destroy);
	OnDestroy();
	DestroyAllChildren();
	if (ParentWidget)
	{
		ParentWidget->RemoveChild(this);
	}
	Super::Destroy();
	unguard;
}

//==========================================================================
//
//	VWidget::AddChild
//
//==========================================================================

void VWidget::AddChild(VWidget* NewChild)
{
	guard(VWidget::AddChild);
	NewChild->PrevWidget = LastChildWidget;
	NewChild->NextWidget = NULL;
	if (LastChildWidget)
	{
		LastChildWidget->NextWidget = NewChild;
	}
	else
	{
		FirstChildWidget = NewChild;
	}
	LastChildWidget = NewChild;
	OnChildAdded(NewChild);
	unguard;
}

//==========================================================================
//
//	VWidget::RemoveChild
//
//==========================================================================

void VWidget::RemoveChild(VWidget* InChild)
{
	guard(VWidget::RemoveChild);
	if (InChild->PrevWidget)
	{
		InChild->PrevWidget->NextWidget = InChild->NextWidget;
	}
	else
	{
		FirstChildWidget = InChild->NextWidget;
	}
	if (InChild->NextWidget)
	{
		InChild->NextWidget->PrevWidget = InChild->PrevWidget;
	}
	else
	{
		LastChildWidget = InChild->PrevWidget;
	}
	InChild->PrevWidget = NULL;
	InChild->NextWidget = NULL;
	InChild->ParentWidget = NULL;
	OnChildRemoved(InChild);
	unguard;
}

//==========================================================================
//
//	VWidget::DestroyAllChildren
//
//==========================================================================

void VWidget::DestroyAllChildren()
{
	guard(VWidget::DestroyAllChildren);
	while (FirstChildWidget)
	{
		FirstChildWidget->ConditionalDestroy();
	}
	unguard;
}

//==========================================================================
//
//	VWidget::Lower
//
//==========================================================================

void VWidget::Lower()
{
	guard(VWidget::Lower);
	if (!ParentWidget)
	{
		Sys_Error("Can't lower root window");
	}

	if (ParentWidget->FirstChildWidget == this)
	{
		//	Already there
		return;
	}

	//	Unlink from current location
	PrevWidget->NextWidget = NextWidget;
	if (NextWidget)
	{
		NextWidget->PrevWidget = PrevWidget;
	}
	else
	{
		ParentWidget->LastChildWidget = PrevWidget;
	}

	//	Link on bottom
	PrevWidget = NULL;
	NextWidget = ParentWidget->FirstChildWidget;
	ParentWidget->FirstChildWidget->PrevWidget = this;
	ParentWidget->FirstChildWidget = this;
	unguard;
}

//==========================================================================
//
//	VWidget::Raise
//
//==========================================================================

void VWidget::Raise()
{
	guard(VWidget::Raise);
	if (!ParentWidget)
	{
		Sys_Error("Can't raise root window");
	}

	if (ParentWidget->LastChildWidget == this)
	{
		//	Already there
		return;
	}

	//	Unlink from current location
	NextWidget->PrevWidget = PrevWidget;
	if (PrevWidget)
	{
		PrevWidget->NextWidget = NextWidget;
	}
	else
	{
		ParentWidget->FirstChildWidget = NextWidget;
	}

	//	Link on top
	PrevWidget = ParentWidget->LastChildWidget;
	NextWidget = NULL;
	ParentWidget->LastChildWidget->NextWidget = this;
	ParentWidget->LastChildWidget = this;
	unguard;
}

//==========================================================================
//
//	VWidget::ClipTree
//
//==========================================================================

void VWidget::ClipTree()
{
	guard(VWidget::ClipTree);
	//	Set up clipping rectangle.
	if (ParentWidget)
	{
		//	Clipping rectangle is relative to the parent widget.
		ClipRect.OriginX = ParentWidget->ClipRect.OriginX +
			ParentWidget->ClipRect.ScaleX * PosX;
		ClipRect.OriginY = ParentWidget->ClipRect.OriginY +
			ParentWidget->ClipRect.ScaleY * PosY;
		ClipRect.ScaleX = ParentWidget->ClipRect.ScaleX * SizeScaleX;
		ClipRect.ScaleY = ParentWidget->ClipRect.ScaleY * SizeScaleY;
		ClipRect.ClipX1 = ClipRect.OriginX;
		ClipRect.ClipY1 = ClipRect.OriginY;
		ClipRect.ClipX2 = ClipRect.OriginX + ClipRect.ScaleX * SizeWidth;
		ClipRect.ClipY2 = ClipRect.OriginY + ClipRect.ScaleY * SizeHeight;

		//	Clip against the parent widget's clipping rectangle.
		if (ClipRect.ClipX1 < ParentWidget->ClipRect.ClipX1)
		{
			ClipRect.ClipX1 = ParentWidget->ClipRect.ClipX1;
		}
		if (ClipRect.ClipY1 < ParentWidget->ClipRect.ClipY1)
		{
			ClipRect.ClipY1 = ParentWidget->ClipRect.ClipY1;
		}
		if (ClipRect.ClipX2 > ParentWidget->ClipRect.ClipX2)
		{
			ClipRect.ClipX2 = ParentWidget->ClipRect.ClipX2;
		}
		if (ClipRect.ClipY2 > ParentWidget->ClipRect.ClipY2)
		{
			ClipRect.ClipY2 = ParentWidget->ClipRect.ClipY2;
		}
	}
	else
	{
		//	This is the root widget.
		ClipRect.OriginX = PosX;
		ClipRect.OriginY = PosY;
		ClipRect.ScaleX = SizeScaleX;
		ClipRect.ScaleY = SizeScaleY;
		ClipRect.ClipX1 = ClipRect.OriginX;
		ClipRect.ClipY1 = ClipRect.OriginY;
		ClipRect.ClipX2 = ClipRect.OriginX + ClipRect.ScaleX * SizeWidth;
		ClipRect.ClipY2 = ClipRect.OriginY + ClipRect.ScaleY * SizeHeight;
	}

	//	Set up clipping rectangles in child widgets.
	for (VWidget* W = FirstChildWidget; W; W = W->NextWidget)
	{
		W->ClipTree();
	}
	unguard;
}

//==========================================================================
//
//	VWidget::SetConfiguration
//
//==========================================================================

void VWidget::SetConfiguration(int NewX, int NewY, int NewWidth,
	int HewHeight, float NewScaleX, float NewScaleY)
{
	guard(VWidget::SetConfiguration);
	PosX = NewX;
	PosY = NewY;
	SizeWidth = NewWidth;
	SizeHeight = HewHeight;
	SizeScaleX = NewScaleX;
	SizeScaleY = NewScaleY;
	ClipTree();
	OnConfigurationChanged();
	unguard;
}

//==========================================================================
//
//	VWidget::SetVisibility
//
//==========================================================================

void VWidget::SetVisibility(bool NewVisibility)
{
	guard(VWidget::SetVisibility);
	if (!!(WidgetFlags & WF_IsVisible) != NewVisibility)
	{
		if (NewVisibility)
			WidgetFlags |= WF_IsVisible;
		else
			WidgetFlags &= ~WF_IsVisible;
		OnVisibilityChanged(NewVisibility);
	}
	unguard;
}

//==========================================================================
//
//	VWidget::SetEnabled
//
//==========================================================================

void VWidget::SetEnabled(bool NewEnabled)
{
	guard(VWidget::SetEnabled);
	if (!!(WidgetFlags & WF_IsEnabled) != NewEnabled)
	{
		if (NewEnabled)
			WidgetFlags |= WF_IsEnabled;
		else
			WidgetFlags &= ~WF_IsEnabled;
		OnEnableChanged(NewEnabled);
	}
	unguard;
}

//==========================================================================
//
//	VWidget::DrawTree
//
//==========================================================================

void VWidget::DrawTree()
{
	guard(VWidget::DrawTree);
	if (!(WidgetFlags & WF_IsVisible) || !ClipRect.HasArea())
	{
		//	Not visible or clipped away.
		return;
	}

	//	Main draw event for this widget.
	OnDraw();

	//	Draw chid widgets.
	for (VWidget* c = FirstChildWidget; c; c = c->NextWidget)
	{
		c->DrawTree();
	}

	//	Do any drawing after child wigets have been drawn.
	OnPostDraw();
	unguard;
}

//==========================================================================
//
//	VWidget::TickTree
//
//==========================================================================

void VWidget::TickTree(float DeltaTime)
{
	guard(VWidget::TickTree);
	if (WidgetFlags & WF_TickEnabled)
	{
		Tick(DeltaTime);
	}
	for (VWidget* c = FirstChildWidget; c; c = c->NextWidget)
	{
		c->TickTree(DeltaTime);
	}
	unguard;
}

//==========================================================================
//
//	VWidget::TransferAndClipRect
//
//==========================================================================

bool VWidget::TransferAndClipRect(float& X1, float& Y1, float& X2, float& Y2,
	float& S1, float& T1, float& S2, float& T2) const
{
	guard(VWidget::TransferAndClipRect);
	X1 = ClipRect.ScaleX * X1 + ClipRect.OriginX;
	Y1 = ClipRect.ScaleY * Y1 + ClipRect.OriginY;
	X2 = ClipRect.ScaleX * X2 + ClipRect.OriginX;
	Y2 = ClipRect.ScaleY * Y2 + ClipRect.OriginY;
	if (X1 < ClipRect.ClipX1)
	{
		S1 = S1 + (X1 - ClipRect.ClipX1) / (X1 - X2) * (S2 - S1);
		X1 = ClipRect.ClipX1;
	}
	if (X2 > ClipRect.ClipX2)
	{
		S2 = S2 + (X2 - ClipRect.ClipX2) / (X1 - X2) * (S2 - S1);
		X2 = ClipRect.ClipX2;
	}
	if (Y1 < ClipRect.ClipY1)
	{
		T1 = T1 + (Y1 - ClipRect.ClipY1) / (Y1 - Y2) * (T2 - T1);
		Y1 = ClipRect.ClipY1;
	}
	if (Y2 > ClipRect.ClipY2)
	{
		T2 = T2 + (Y2 - ClipRect.ClipY2) / (Y1 - Y2) * (T2 - T1);
		Y2 = ClipRect.ClipY2;
	}
	return X1 < X2 && Y1 < Y2;
	unguard;
}

//==========================================================================
//
//	VWidget::DrawPic
//
//==========================================================================

void VWidget::DrawPic(int X, int Y, int Handle, float Alpha)
{
	guard(VWidget::DrawPic);
	if (Handle < 0)
	{
		return;
	}

	picinfo_t Info;
	GTextureManager.GetTextureInfo(Handle, &Info);
	X -= Info.xoffset;
	Y -= Info.yoffset;
	float X1 = X;
	float Y1 = Y;
	float X2 = X + Info.width;
	float Y2 = Y + Info.height;
	float S1 = 0;
	float T1 = 0;
	float S2 = Info.width;
	float T2 = Info.height;
	if (TransferAndClipRect(X1, Y1, X2, Y2, S1, T1, S2, T2))
	{
		Drawer->DrawPic(X1, Y1, X2, Y2, S1, T1, S2, T2, Handle, Alpha);
	}
	unguard;
}

//==========================================================================
//
//	VWidget::DrawShadowedPic
//
//==========================================================================

void VWidget::DrawShadowedPic(int X, int Y, int Handle)
{
	guard(VWidget::DrawShadowedPic);
	if (Handle < 0)
	{
		return;
	}

	picinfo_t Info;
	GTextureManager.GetTextureInfo(Handle, &Info);
	float X1 = X - Info.xoffset + 2;
	float Y1 = Y - Info.yoffset + 2;
	float X2 = X - Info.xoffset + 2 + Info.width;
	float Y2 = Y - Info.yoffset + 2 + Info.height;
	float S1 = 0;
	float T1 = 0;
	float S2 = Info.width;
	float T2 = Info.height;
	if (TransferAndClipRect(X1, Y1, X2, Y2, S1, T1, S2, T2))
	{
		Drawer->DrawPicShadow(X1, Y1, X2, Y2, S1, T1, S2, T2, Handle, 0.625);
	}

	DrawPic(X, Y, Handle);
	unguard;
}

//==========================================================================
//
//	VWidget::FillRectWithFlat
//
//==========================================================================

void VWidget::FillRectWithFlat(int X, int Y, int Width, int Height,
	VName Name)
{
	guard(VWidget::FillRectWithFlat);
	float X1 = X;
	float Y1 = Y;
	float X2 = X + Width;
	float Y2 = Y + Height;
	float S1 = 0;
	float T1 = 0;
	float S2 = Width;
	float T2 = Height;
	if (TransferAndClipRect(X1, Y1, X2, Y2, S1, T1, S2, T2))
	{
		Drawer->FillRectWithFlat(X1, Y1, X2, Y2, S1, T1, S2, T2, Name);
	}
	unguard;
}

//==========================================================================
//
//	VWidget::ShadeRect
//
//==========================================================================

void VWidget::ShadeRect(int X, int Y, int Width, int Height, float Shade)
{
	guard(VWidget::ShadeRect);
	float X1 = X;
	float Y1 = Y;
	float X2 = X + Width;
	float Y2 = Y + Height;
	float S1 = 0;
	float T1 = 0;
	float S2 = 0;
	float T2 = 0;
	if (TransferAndClipRect(X1, Y1, X2, Y2, S1, T1, S2, T2))
	{
		Drawer->ShadeRect((int)X1, (int)Y1, (int)X2 - (int)X1, (int)Y2 - (int)Y1, Shade);
	}
	unguard;
}

//==========================================================================
//
//	VWidget::DrawString
//
//==========================================================================

void VWidget::DrawString(int x, int y, const VStr& String)
{
	guard(VWidget::DrawNString);
	if (!String)
		return;

	int cx = x;
	int cy = y;

	if (HAlign == hcentre)
		cx -= T_StringWidth(String) / 2;
	if (HAlign == hright)
		cx -= T_StringWidth(String);

	for (size_t i = 0; i < String.Length(); i++)
	{
		int c = String[i] - 32;

		if (c < 0)
		{
			continue;
		}
		if (c >= 96 || Font->Pics[c] < 0)
		{
			cx += Font->SpaceWidth + HDistance;
			continue;
		}

		int w = Font->PicInfo[c].width;
		if (t_shadowed)
			DrawShadowedPic(cx, cy, Font->Pics[c]);
		else
			DrawPic(cx, cy, Font->Pics[c]);
		cx += w + HDistance;
	}
	LastX = cx;
	LastY = cy;
	unguard;
}

//==========================================================================
//
//	VWidget::DrawText
//
//==========================================================================

void VWidget::DrawText(int x, int y, const VStr& String)
{
	guard(VWidget::DrawText);
	int start = 0;
	int cx = x;
	int cy = y;

	if (VAlign == vcentre)
		cy -= T_TextHeight(String) / 2;
	if (VAlign == vbottom)
		cy -= T_TextHeight(String);

	//	Need this for correct cursor position with empty strings.
	LastX = cx;
	LastY = cy;

	for (size_t i = 0; i < String.Length(); i++)
	{
		if (String[i] == '\n')
		{
			VStr cs(String, start, i - start);
			DrawString(cx, cy, cs);
			cy += T_StringHeight(cs) + VDistance;
			start = i + 1;
		}
		if (i == String.Length() - 1)
		{
			DrawString(cx, cy, VStr(String, start, String.Length() - start));
		}
	}
	unguard;
}

//==========================================================================
//
//	VWidget::DrawTextW
//
//==========================================================================

int VWidget::DrawTextW(int x, int y, const VStr& String, int w)
{
	guard(VWidget::DrawTextW);
	int			start = 0;
	int			cx;
	int			cy;
	int			i;
	bool		wordStart = true;
	int			LinesPrinted = 0;

	cx = x;
	cy = y;

	//	These won't work correctly so don't use them for now.
	if (VAlign == vcentre)
		cy -= T_TextHeight(String) / 2;
	if (VAlign == vbottom)
		cy -= T_TextHeight(String);

	//	Need this for correct cursor position with empty strings.
	LastX = cx;
	LastY = cy;

	for (i = 0; String[i]; i++)
	{
		if (String[i] == '\n')
		{
			VStr cs(String, start, i - start);
			DrawString(cx, cy, cs);
			cy += T_StringHeight(cs) + VDistance;
			start = i + 1;
			wordStart = true;
			LinesPrinted++;
		}
		else if (wordStart && String[i] > ' ')
		{
			int j = i;
			while (String[j] > ' ')
				j++;
			if (T_StringWidth(VStr(String, start, j - start)) > w)
			{
				VStr cs(String, start, i - start);
				DrawString(cx, cy, cs);
				cy += T_StringHeight(cs) + VDistance;
				start = i;
				LinesPrinted++;
			}
			wordStart = false;
		}
		else if (String[i] <= ' ')
		{
			wordStart = true;
		}
		if (!String[i + 1])
		{
			DrawString(cx, cy, VStr(String, start, i - start + 1));
			LinesPrinted++;
		}
	}
	return LinesPrinted;
	unguard;
}

//==========================================================================
//
//	Natives
//
//==========================================================================

IMPLEMENT_FUNCTION(VWidget, NewChild)
{
	P_GET_PTR(VClass, ChildClass);
	P_GET_SELF;
	RET_REF(CreateNewWidget(ChildClass, Self));
}

IMPLEMENT_FUNCTION(VWidget, Destroy)
{
	P_GET_SELF;
	delete Self;
}

IMPLEMENT_FUNCTION(VWidget, DestroyAllChildren)
{
	P_GET_SELF;
	Self->DestroyAllChildren();
}

IMPLEMENT_FUNCTION(VWidget, Lower)
{
	P_GET_SELF;
	Self->Lower();
}

IMPLEMENT_FUNCTION(VWidget, Raise)
{
	P_GET_SELF;
	Self->Raise();
}

IMPLEMENT_FUNCTION(VWidget, SetPos)
{
	P_GET_INT(NewY);
	P_GET_INT(NewX);
	P_GET_SELF;
	Self->SetPos(NewX, NewY);
}

IMPLEMENT_FUNCTION(VWidget, SetX)
{
	P_GET_INT(NewX);
	P_GET_SELF;
	Self->SetX(NewX);
}

IMPLEMENT_FUNCTION(VWidget, SetY)
{
	P_GET_INT(NewY);
	P_GET_SELF;
	Self->SetY(NewY);
}

IMPLEMENT_FUNCTION(VWidget, SetSize)
{
	P_GET_INT(NewHeight);
	P_GET_INT(NewWidth);
	P_GET_SELF;
	Self->SetSize(NewWidth, NewHeight);
}

IMPLEMENT_FUNCTION(VWidget, SetWidth)
{
	P_GET_INT(NewWidth);
	P_GET_SELF;
	Self->SetWidth(NewWidth);
}

IMPLEMENT_FUNCTION(VWidget, SetHeight)
{
	P_GET_INT(NewHeight);
	P_GET_SELF;
	Self->SetHeight(NewHeight);
}

IMPLEMENT_FUNCTION(VWidget, SetScale)
{
	P_GET_FLOAT(NewScaleY);
	P_GET_FLOAT(NewScaleX);
	P_GET_SELF;
	Self->SetScale(NewScaleX, NewScaleY);
}

IMPLEMENT_FUNCTION(VWidget, SetConfiguration)
{
	P_GET_FLOAT_OPT(NewScaleY, 1.0);
	P_GET_FLOAT_OPT(NewScaleX, 1.0);
	P_GET_INT(NewHeight);
	P_GET_INT(NewWidth);
	P_GET_INT(NewY);
	P_GET_INT(NewX);
	P_GET_SELF;
	Self->SetConfiguration(NewX, NewY, NewWidth, NewHeight, NewScaleX,
		NewScaleY);
}

IMPLEMENT_FUNCTION(VWidget, SetVisibility)
{
	P_GET_BOOL(bNewVisibility);
	P_GET_SELF;
	Self->SetVisibility(bNewVisibility);
}

IMPLEMENT_FUNCTION(VWidget, Show)
{
	P_GET_SELF;
	Self->Show();
}

IMPLEMENT_FUNCTION(VWidget, Hide)
{
	P_GET_SELF;
	Self->Hide();
}

IMPLEMENT_FUNCTION(VWidget, IsVisible)
{
	P_GET_BOOL_OPT(Recurse, true);
	P_GET_SELF;
	RET_BOOL(Self->IsVisible(Recurse));
}

IMPLEMENT_FUNCTION(VWidget, SetEnabled)
{
	P_GET_BOOL(bNewEnabled);
	P_GET_SELF;
	Self->SetEnabled(bNewEnabled);
}

IMPLEMENT_FUNCTION(VWidget, Enable)
{
	P_GET_SELF;
	Self->Enable();
}

IMPLEMENT_FUNCTION(VWidget, Disable)
{
	P_GET_SELF;
	Self->Disable();
}

IMPLEMENT_FUNCTION(VWidget, IsEnabled)
{
	P_GET_BOOL_OPT(Recurse, true);
	P_GET_SELF;
	RET_BOOL(Self->IsEnabled(Recurse));
}

IMPLEMENT_FUNCTION(VWidget, DrawPic)
{
	P_GET_FLOAT_OPT(Alpha, 1.0);
	P_GET_INT(Handle);
	P_GET_INT(Y);
	P_GET_INT(X);
	P_GET_SELF;
	Self->DrawPic(X, Y, Handle, Alpha);
}

IMPLEMENT_FUNCTION(VWidget, DrawShadowedPic)
{
	P_GET_INT(Handle);
	P_GET_INT(Y);
	P_GET_INT(X);
	P_GET_SELF;
	Self->DrawShadowedPic(X, Y, Handle);
}

IMPLEMENT_FUNCTION(VWidget, FillRectWithFlat)
{
	P_GET_NAME(Name);
	P_GET_INT(Height);
	P_GET_INT(Width);
	P_GET_INT(Y);
	P_GET_INT(X);
	P_GET_SELF;
	Self->FillRectWithFlat(X, Y, Width, Height, Name);
}

IMPLEMENT_FUNCTION(VWidget, ShadeRect)
{
	P_GET_FLOAT(Shade);
	P_GET_INT(Height);
	P_GET_INT(Width);
	P_GET_INT(Y);
	P_GET_INT(X);
	P_GET_SELF;
	Self->ShadeRect(X, Y, Width, Height, Shade);
}

IMPLEMENT_FUNCTION(VWidget, DrawText)
{
	P_GET_STR(String);
	P_GET_INT(Y);
	P_GET_INT(X);
	P_GET_SELF;
	Self->DrawText(X, Y, String);
}

IMPLEMENT_FUNCTION(VWidget, DrawTextW)
{
	P_GET_INT(w);
	P_GET_STR(txt);
	P_GET_INT(y);
	P_GET_INT(x);
	P_GET_SELF;
	RET_INT(Self->DrawTextW(x, y, txt, w));
}
