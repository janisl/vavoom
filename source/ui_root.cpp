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

IMPLEMENT_CLASS(V, RootWidget);

VRootWidget*		GRoot;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VRootWidget::VRootWidget
//
//==========================================================================

VRootWidget::VRootWidget()
{
}

//==========================================================================
//
//	VRootWidget::Init
//
//==========================================================================

void VRootWidget::Init()
{
	guard(VRootWidget::Init);
	Super::Init(NULL);
	SetSize(640, 480);

	MouseCursorPic = GTextureManager.AddPatch("mc_arrow", TEXTYPE_Pic);
	unguard;
}

//==========================================================================
//
//	VRootWidget::DrawWidgets
//
//==========================================================================

void VRootWidget::DrawWidgets()
{
	guard(VRootWidget::DrawWidgets)
	DrawTree();

	if (RootFlags & RWF_MouseEnabled)
	{
		DrawPic(MouseX - 16, MouseY - 16, MouseCursorPic);
	}
	unguard;
}

//==========================================================================
//
//	VRootWidget::TickWidgets
//
//==========================================================================

void VRootWidget::TickWidgets(float DeltaTime)
{
	guard(VRootWidget::TickWidgets);
	if (SizeScaleX != fScaleX)
	{
		SizeScaleX = fScaleX;
		SizeScaleY = fScaleY;
		ClipTree();
	}
	TickTree(DeltaTime);
	unguard;
}

//==========================================================================
//
//	VRootWidget::Responder
//
//==========================================================================

bool VRootWidget::Responder(event_t* Event)
{
	guard(VRootWidget::Responder);
	if (RootFlags & RWF_MouseEnabled)
	{
		//	Handle mouse movement.
		if (Event->type == ev_mouse)
		{
			MouseMoveEvent(MouseX + Event->data2, MouseY - Event->data3);
			return true;
		}

		//	Handle mouse buttons
		if ((Event->type == ev_keydown || Event->type == ev_keyup) &&
			Event->data1 >= K_MOUSE1 && Event->data1 <= K_MOUSE3)
		{
		}
	}

	//	Handle keyboard events.
	if (Event->type == ev_keydown || Event->type == ev_keyup)
	{
		//	Find the top-most focused widget.
		VWidget* W = CurrentFocusChild;
		while (W && W->CurrentFocusChild)
		{
			W = W->CurrentFocusChild;
		}

		//	Call event handlers
		while (W)
		{
			if (Event->type == ev_keydown)
			{
				if (W->OnKeyDown(Event->data1))
				{
					return true;
				}
			}
			else
			{
				if (W->OnKeyUp(Event->data1))
				{
					return true;
				}
			}
			W = W->ParentWidget;
		}
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VRootWidget::SetMouse
//
//==========================================================================

void VRootWidget::SetMouse(bool MouseOn)
{
	guard(VRootWidget::SetMouse);
	if (MouseOn)
		RootFlags |= RWF_MouseEnabled;
	else
		RootFlags &= ~RWF_MouseEnabled;
	unguard;
}

//==========================================================================
//
//	VRootWidget::MouseMoveEvent
//
//==========================================================================

void VRootWidget::MouseMoveEvent(int NewX, int NewY)
{
	guard(VRootWidget::MouseMoveEvent);
	//	Remember old mouse coordinates.
	int OldMouseX = MouseX;
	int OldMouseY = MouseY;

	//	Update mouse position.
	MouseX = NewX;
	MouseY = NewY;

	//	Clip mouse coordinates against window boundaries.
	if (MouseX < 0)
	{
		MouseX = 0;
	}
	if (MouseX >= SizeWidth)
	{
		MouseX = SizeWidth - 1;
	}
	if (MouseY < 0)
	{
		MouseY = 0;
	}
	if (MouseY >= SizeHeight)
	{
		MouseY = SizeHeight - 1;
	}

	//	Check if mouse position has changed.
	if (MouseX == OldMouseX && MouseY == OldMouseY)
	{
		return;
	}

	//	Find widget onder old position.
	float ScaledOldX = OldMouseX * SizeScaleX;
	float ScaledOldY = OldMouseY * SizeScaleY;
	float ScaledNewX = MouseX * SizeScaleX;
	float ScaledNewY = MouseY * SizeScaleY;
	VWidget* OldFocus = GetWidgetAt(ScaledOldX, ScaledOldY);
	for (VWidget* W = OldFocus; W; W = W->ParentWidget)
	{
		if (W->OnMouseMove(
			(int)((ScaledOldX - W->ClipRect.OriginX) / W->ClipRect.ScaleX),
			(int)((ScaledOldY - W->ClipRect.OriginY) / W->ClipRect.ScaleY),
			(int)((ScaledNewX - W->ClipRect.OriginX) / W->ClipRect.ScaleX),
			(int)((ScaledNewY - W->ClipRect.OriginY) / W->ClipRect.ScaleY)))
		{
			break;
		}
	}
	VWidget* NewFocus = GetWidgetAt(ScaledNewX, ScaledNewY);
dprintf("Focus on %s\n", NewFocus->GetClass()->GetName());
	if (OldFocus != NewFocus)
	{
		OldFocus->OnMouseLeave();
		NewFocus->OnMouseEnter();
	}
	unguard;
}

//==========================================================================
//
//	VRootWidget::StaticInit
//
//==========================================================================

void VRootWidget::StaticInit()
{
	guard(VRootWidget::StaticInit);
	GRoot = Spawn<VRootWidget>();
	GRoot->Init();
	GClGame->GRoot = GRoot;
	unguard;
}

//==========================================================================
//
//	Natives
//
//==========================================================================

IMPLEMENT_FUNCTION(VRootWidget, SetMouse)
{
	P_GET_BOOL(MouseOn);
	P_GET_SELF;
	Self->SetMouse(MouseOn);
}
