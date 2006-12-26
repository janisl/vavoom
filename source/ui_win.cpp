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
, WindowFlags(WF_IsVisible | WF_IsSensitive)
{
}

//==========================================================================
//
//	VWindow::Init
//
//==========================================================================

void VWindow::Init(VWindow *InParent)
{
	guard(VWindow::Init);
	Parent = InParent;
	WinGC = Spawn<VGC>();
	if (Parent)
	{
		Parent->AddChild(this);
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
	if (Parent)
		Parent->RemoveChild(this);
	if (WinGC)
		WinGC->ConditionalDestroy();
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
		win = win->Parent;
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
		win = win->Parent;
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
	return Parent;
}

//==========================================================================
//
//	VWindow::GetBottomChild
//
//==========================================================================

VWindow *VWindow::GetBottomChild(bool bVisibleOnly)
{
	guard(VWindow::GetBottomChild);
	VWindow *win = FirstChild;
	if (bVisibleOnly)
	{
		while (win && !(win->WindowFlags & WF_IsVisible))
		{
			win = win->NextSibling;
		}
	}
	return win;
	unguard;
}

//==========================================================================
//
//	VWindow::GetTopChild
//
//==========================================================================

VWindow *VWindow::GetTopChild(bool bVisibleOnly)
{
	guard(VWindow::GetTopChild);
	VWindow *win = LastChild;
	if (bVisibleOnly)
	{
		while (win && !(win->WindowFlags & WF_IsVisible))
		{
			win = win->PrevSibling;
		}
	}
	return win;
	unguard;
}

//==========================================================================
//
//	VWindow::GetLowerSibling
//
//==========================================================================

VWindow *VWindow::GetLowerSibling(bool bVisibleOnly)
{
	guard(VWindow::GetLowerSibling);
	VWindow *win = PrevSibling;
	if (bVisibleOnly)
	{
		while (win && !(win->WindowFlags & WF_IsVisible))
		{
			win = win->PrevSibling;
		}
	}
	return win;
	unguard;
}

//==========================================================================
//
//	VWindow::GetHigherSibling
//
//==========================================================================

VWindow *VWindow::GetHigherSibling(bool bVisibleOnly)
{
	guard(VWindow::GetHigherSibling);
	VWindow *win = NextSibling;
	if (bVisibleOnly)
	{
		while (win && !(win->WindowFlags & WF_IsVisible))
		{
			win = win->NextSibling;
		}
	}
	return win;
	unguard;
}

//==========================================================================
//
//	VWindow::Raise
//
//==========================================================================

void VWindow::Raise()
{
	guard(VWindow::Raise);
	if (!Parent)
	{
		Sys_Error("Can't raise root window");
	}
	if (Parent->LastChild == this)
	{
		//	Already there
		return;
	}
	//	Unlink from current location
	if (PrevSibling)
	{
		PrevSibling->NextSibling = NextSibling;
	}
	else
	{
		Parent->FirstChild = NextSibling;
	}
	NextSibling->PrevSibling = PrevSibling;
	//	Link on top
	PrevSibling = Parent->LastChild;
	NextSibling = NULL;
	Parent->LastChild->NextSibling = this;
	Parent->LastChild = this;
	unguard;
}

//==========================================================================
//
//	VWindow::Lower
//
//==========================================================================

void VWindow::Lower()
{
	guard(VWindow::Lower);
	if (!Parent)
	{
		Sys_Error("Can't lower root window");
	}
	if (Parent->FirstChild == this)
	{
		//	Already there
		return;
	}
	//	Unlink from current location
	PrevSibling->NextSibling = NextSibling;
	if (NextSibling)
	{
		NextSibling->PrevSibling = PrevSibling;
	}
	else
	{
		Parent->LastChild = PrevSibling;
	}
	//	Link on bottom
	PrevSibling = NULL;
	NextSibling = Parent->FirstChild;
	Parent->FirstChild->PrevSibling = this;
	Parent->FirstChild = this;
	unguard;
}

//==========================================================================
//
//	VWindow::SetVisibility
//
//==========================================================================

void VWindow::SetVisibility(bool NewVisibility)
{
	guard(VWindow::SetVisibility);
	if (!!(WindowFlags & WF_IsVisible) != NewVisibility)
	{
		if (NewVisibility)
			WindowFlags |= WF_IsVisible;
		else
			WindowFlags &= ~WF_IsVisible;
		VisibilityChanged(NewVisibility);
	}
	unguard;
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
/*		if (WindowFlags & WF_IsVisible && WindowFlags & WF_IsSensitive)
		{
			if (WindowFlags & WF_IsSelectable)
				GetModalWindow()->AddWindowToTables(this);
			else
				GetModalWindow()->RemoveWindowFromTables(this);
		}*/
	}
	unguard;
}

//==========================================================================
//
//	VWindow::SetPos
//
//==========================================================================

void VWindow::SetPos(int NewX, int NewY)
{
	guard(VWindow::SetPos);
	X = NewX;
	Y = NewY;
	ClipTree();
	ConfigurationChanged();
	unguard;
}

//==========================================================================
//
//	VWindow::SetSize
//
//==========================================================================

void VWindow::SetSize(int NewWidth, int NewHeight)
{
	guard(VWindow::SetSize);
	Width = NewWidth;
	Height = NewHeight;
	ClipTree();
	ConfigurationChanged();
	unguard;
}

//==========================================================================
//
//	VWindow::SetConfiguration
//
//==========================================================================

void VWindow::SetConfiguration(int NewX, int NewY, int NewWidth, int HewHeight)
{
	guard(VWindow::SetConfiguration);
	X = NewX;
	Y = NewY;
	Width = NewWidth;
	Height = HewHeight;
	ClipTree();
	ConfigurationChanged();
	unguard;
}

//==========================================================================
//
//	VWindow::SetWidth
//
//==========================================================================

void VWindow::SetWidth(int NewWidth)
{
	guard(VWindow::SetWidth);
	Width = NewWidth;
	ClipTree();
	ConfigurationChanged();
	unguard;
}

//==========================================================================
//
//	VWindow::SetHeight
//
//==========================================================================

void VWindow::SetHeight(int NewHeight)
{
	guard(VWindow::SetHeight);
	Height = NewHeight;
	ClipTree();
	ConfigurationChanged();
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
	while (FirstChild)
	{
		FirstChild->ConditionalDestroy();
	}
	unguard;
}

//==========================================================================
//
//	VWindow::AddChild
//
//==========================================================================

void VWindow::AddChild(VWindow *NewChild)
{
	guard(VWindow::AddChild);
	NewChild->PrevSibling = LastChild;
	NewChild->NextSibling = NULL;
	if (LastChild)
	{
		LastChild->NextSibling = NewChild;
	}
	else
	{
		FirstChild = NewChild;
	}
	LastChild = NewChild;
	ChildAdded(NewChild);
	for (VWindow *w = this; w; w = w->Parent)
	{
		w->DescendantAdded(NewChild);
	}
	unguard;
}

//==========================================================================
//
//	VWindow::RemoveChild
//
//==========================================================================

void VWindow::RemoveChild(VWindow *InChild)
{
	guard(VWindow::RemoveChild);
	if (InChild->PrevSibling)
	{
		InChild->PrevSibling->NextSibling = InChild->NextSibling;
	}
	else
	{
		FirstChild = InChild->NextSibling;
	}
	if (InChild->NextSibling)
	{
		InChild->NextSibling->PrevSibling = InChild->PrevSibling;
	}
	else
	{
		LastChild = InChild->PrevSibling;
	}
	InChild->PrevSibling = NULL;
	InChild->NextSibling = NULL;
	InChild->Parent = NULL;
	ChildRemoved(InChild);
	for (VWindow *w = this; w; w = w->Parent)
	{
		w->DescendantRemoved(InChild);
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
	if (!(WindowFlags & WF_IsVisible) || !ClipRect.HasArea())
	{
		//	Nowhere to draw.
		return;
	}
	WinGC->SetClipRect(ClipRect);
	DrawWindow(WinGC);
	for (VWindow *c = FirstChild; c; c = c->NextSibling)
	{
		c->DrawTree();
	}
	PostDrawWindow(WinGC);
	unguard;
}

//==========================================================================
//
//	VWindow::ClipTree
//
//==========================================================================

void VWindow::ClipTree()
{
	guard(VWindow::ClipTree);
	if (Parent)
	{
		ClipRect = VClipRect(Parent->ClipRect.OriginX + X, 
			Parent->ClipRect.OriginY + Y, Width, Height);
		ClipRect.Intersect(Parent->ClipRect);
	}
	else
	{
		ClipRect = VClipRect(X, Y, Width, Height);
	}
	for (VWindow *c = FirstChild; c; c = c->NextSibling)
	{
		c->ClipTree();
	}
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
	for (VWindow *c = FirstChild; c; c = c->NextSibling)
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
IMPLEMENT_FUNCTION(VWindow, Raise)
{
	P_GET_SELF;
	Self->Raise();
}
IMPLEMENT_FUNCTION(VWindow, Lower)
{
	P_GET_SELF;
	Self->Lower();
}
IMPLEMENT_FUNCTION(VWindow, SetVisibility)
{
	P_GET_BOOL(bNewVisibility);
	P_GET_SELF;
	Self->SetVisibility(bNewVisibility);
}
IMPLEMENT_FUNCTION(VWindow, Show)
{
	P_GET_SELF;
	Self->Show();
}
IMPLEMENT_FUNCTION(VWindow, Hide)
{
	P_GET_SELF;
	Self->Hide();
}
IMPLEMENT_FUNCTION(VWindow, IsVisible)
{
	P_GET_SELF;
	RET_BOOL(Self->IsVisible());
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

IMPLEMENT_FUNCTION(VWindow, SetPos)
{
	P_GET_INT(NewY);
	P_GET_INT(NewX);
	P_GET_SELF;
	Self->SetPos(NewX, NewY);
}
IMPLEMENT_FUNCTION(VWindow, SetSize)
{
	P_GET_INT(NewHeight);
	P_GET_INT(NewWidth);
	P_GET_SELF;
	Self->SetSize(NewWidth, NewHeight);
}
IMPLEMENT_FUNCTION(VWindow, SetConfiguration)
{
	P_GET_INT(NewHeight);
	P_GET_INT(NewWidth);
	P_GET_INT(NewY);
	P_GET_INT(NewX);
	P_GET_SELF;
	Self->SetConfiguration(NewX, NewY, NewWidth, NewHeight);
}
IMPLEMENT_FUNCTION(VWindow, SetWidth)
{
	P_GET_INT(NewWidth);
	P_GET_SELF;
	Self->SetWidth(NewWidth);
}
IMPLEMENT_FUNCTION(VWindow, SetHeight)
{
	P_GET_INT(NewHeight);
	P_GET_SELF;
	Self->SetHeight(NewHeight);
}

IMPLEMENT_FUNCTION(VWindow, GetBottomChild)
{
	P_GET_BOOL(bVisibleOnly);
	P_GET_SELF;
	Self->GetBottomChild(bVisibleOnly);
}
IMPLEMENT_FUNCTION(VWindow, GetTopChild)
{
	P_GET_BOOL(bVisibleOnly);
	P_GET_SELF;
	Self->GetTopChild(bVisibleOnly);
}
IMPLEMENT_FUNCTION(VWindow, GetLowerSibling)
{
	P_GET_BOOL(bVisibleOnly);
	P_GET_SELF;
	Self->GetLowerSibling(bVisibleOnly);
}
IMPLEMENT_FUNCTION(VWindow, GetHigherSibling)
{
	P_GET_BOOL(bVisibleOnly);
	P_GET_SELF;
	Self->GetHigherSibling(bVisibleOnly);
}

IMPLEMENT_FUNCTION(VWindow, DestroyAllChildren)
{
	P_GET_SELF;
	Self->DestroyAllChildren();
}
