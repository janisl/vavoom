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
//	VWidget::TransferAndClipRect
//
//==========================================================================

void VWidget::TransferAndClipRect(float& X1, float& Y1, float& X2, float& Y2,
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
	TransferAndClipRect(X1, Y1, X2, Y2, S1, T1, S2, T2);
	Drawer->DrawPic(X1, Y1, X2, Y2, S1, T1, S2, T2, Handle, Alpha);
	unguard;
}

//==========================================================================
//
//	Natives
//
//==========================================================================

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

IMPLEMENT_FUNCTION(VWidget, DrawPic)
{
	P_GET_FLOAT_OPT(Alpha, 1.0);
	P_GET_INT(Handle);
	P_GET_INT(Y);
	P_GET_INT(X);
	P_GET_SELF;
	Self->DrawPic(X, Y, Handle, Alpha);
}
