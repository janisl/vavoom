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
#include "ui.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(V, Window);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VWindow::VWindow
//
//==========================================================================

VWindow::VWindow()
: WindowType(WIN_Normal)
{
	WindowFlags |= WF_IsSensitive;
}

//==========================================================================
//
//	VWindow::Init
//
//==========================================================================

void VWindow::Init(VWindow *InParent)
{
	guard(VWindow::Init);
	ParentWidget = InParent;
	if (ParentWidget)
	{
		static_cast<VWindow*>(ParentWidget)->AddChild(this);
	}
	ClipTree();
	InitWindow();
	WindowReady();
	WindowFlags |= WF_IsInitialised;
	unguard;
}

//==========================================================================
//
//	VWindow::CleanUp
//
//==========================================================================

void VWindow::CleanUp()
{
}

//==========================================================================
//
//	VWindow::Destroy
//
//==========================================================================

void VWindow::Destroy()
{
	guard(VWindow::Destroy);
	WindowFlags |= WF_BeingDestroyed;
	DestroyWindow();
	KillAllChildren();
	if (ParentWidget)
		static_cast<VWindow*>(ParentWidget)->RemoveChild(this);
	Super::Destroy();
	unguard;
}

//==========================================================================
//
//	VWindow::GetRootWindow
//
//==========================================================================

VRootWindow *VWindow::GetRootWindow()
{
	guard(VWindow::GetRootWindow);
	VWindow *win = this;
	while (win->WindowType < WIN_Root)
	{
		win = static_cast<VWindow*>(win->ParentWidget);
	}
	return (VRootWindow *)win;
	unguard;
}

//==========================================================================
//
//	VWindow::GetModalWindow
//
//==========================================================================

VModalWindow *VWindow::GetModalWindow()
{
	guard(VWindow::GetModalWindow);
	VWindow *win = this;
	while (win->WindowType < WIN_Modal)
	{
		win = static_cast<VWindow*>(win->ParentWidget);
	}
	return (VModalWindow *)win;
	unguard;
}

//==========================================================================
//
//	VWindow::GetParent
//
//==========================================================================

VWindow *VWindow::GetParent()
{
	return static_cast<VWindow*>(ParentWidget);
}

//==========================================================================
//
//	VWindow::SetSensitivity
//
//==========================================================================

void VWindow::SetSensitivity(bool NewSensitivity)
{
	guard(VWindow::SetSensitivity);
	if (!!(WindowFlags & WF_IsSensitive) != NewSensitivity)
	{
		if (NewSensitivity)
			WindowFlags |= WF_IsSensitive;
		else
			WindowFlags &= ~WF_IsSensitive;
		SensitivityChanged(NewSensitivity);
	}
	unguard;
}

//==========================================================================
//
//	VWindow::SetSelectability
//
//==========================================================================

void VWindow::SetSelectability(bool NewSelectability)
{
	guard(VWindow::SetSelectability);
	if (!!(WindowFlags & WF_IsSelectable) != NewSelectability)
	{
		if (NewSelectability)
			WindowFlags |= WF_IsSelectable;
		else
			WindowFlags &= ~WF_IsSelectable;
	}
	unguard;
}

//==========================================================================
//
//	VWindow::KillAllChildren
//
//==========================================================================

void VWindow::KillAllChildren()
{
	guard(VWindow::KillAllChildren);
	while (FirstChildWidget)
	{
		FirstChildWidget->ConditionalDestroy();
	}
	unguard;
}

//==========================================================================
//
//	VWindow::DrawTree
//
//==========================================================================

void VWindow::DrawTree()
{
	guard(VWindow::DrawTree);
	if (!(WidgetFlags & WF_IsVisible) || !ClipRect.HasArea())
	{
		//	Nowhere to draw.
		return;
	}
	DrawWindow();
	for (VWindow *c = static_cast<VWindow*>(FirstChildWidget); c; c = static_cast<VWindow*>(c->NextWidget))
	{
		c->DrawTree();
	}
	PostDrawWindow();
	unguard;
}

//==========================================================================
//
//	VWindow::TickTree
//
//==========================================================================

void VWindow::TickTree(float DeltaTime)
{
	guard(VWindow::TickTree);
	Tick(DeltaTime);
	if (WindowFlags & WF_TickEnabled)
	{
		eventTick(DeltaTime);
	}
	for (VWindow *c = static_cast<VWindow*>(FirstChildWidget); c; c = static_cast<VWindow*>(c->NextWidget))
	{
		c->TickTree(DeltaTime);
	}
	unguard;
}

//==========================================================================
//
//	VWindow::CreateNewWindow
//
//==========================================================================

VWindow *VWindow::CreateNewWindow(VClass *NewClass, VWindow *ParentWindow)
{
	guard(VWindow::CreateNewWindow);
	VWindow *win;

	win = (VWindow *)StaticSpawnObject(NewClass);
	win->Init(ParentWindow);
	return win;
	unguardf(("(%s)", NewClass->GetName()));
}

//==========================================================================
//
//	Natives
//
//==========================================================================

IMPLEMENT_FUNCTION(VWindow, Destroy)
{
	P_GET_SELF;
	delete Self;
}
IMPLEMENT_FUNCTION(VWindow, NewChild)
{
	P_GET_PTR(VClass, ChildClass);
	P_GET_SELF;
	RET_REF(CreateNewWindow(ChildClass, Self));
}
IMPLEMENT_FUNCTION(VWindow, SetSensitivity)
{
	P_GET_BOOL(bNewSensitivity);
	P_GET_SELF;
	Self->SetSensitivity(bNewSensitivity);
}
IMPLEMENT_FUNCTION(VWindow, Enable)
{
	P_GET_SELF;
	Self->Enable();
}
IMPLEMENT_FUNCTION(VWindow, Disable)
{
	P_GET_SELF;
	Self->Disable();
}
IMPLEMENT_FUNCTION(VWindow, IsSensitive)
{
	P_GET_SELF;
	RET_BOOL(Self->IsSensitive());
}
IMPLEMENT_FUNCTION(VWindow, SetSelectability)
{
	P_GET_BOOL(bNewSelectability);
	P_GET_SELF;
	Self->SetSelectability(bNewSelectability);
}

IMPLEMENT_FUNCTION(VWindow, GetRootWindow)
{
	P_GET_SELF;
	RET_REF(Self->GetRootWindow());
}
IMPLEMENT_FUNCTION(VWindow, GetModalWindow)
{
	P_GET_SELF;
	RET_REF(Self->GetModalWindow());
}
IMPLEMENT_FUNCTION(VWindow, GetParent)
{
	P_GET_SELF;
	RET_REF(Self->GetParent());
}

IMPLEMENT_FUNCTION(VWindow, DestroyAllChildren)
{
	P_GET_SELF;
	Self->DestroyAllChildren();
}
