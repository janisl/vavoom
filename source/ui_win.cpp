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

VWindow::VWindow(void)
: WindowType(WIN_Normal)
, bIsVisible(true)
, bIsSensitive(true)
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
	WinGC = (VGC *)StaticSpawnObject(VGC::StaticClass(), NULL, PU_STRING);
	if (Parent)
	{
		Parent->AddChild(this);
	}
	ClipTree();
	InitWindow();
	WindowReady();
	bIsInitialized = true;
	unguard;
}

//==========================================================================
//
//	VWindow::CleanUp
//
//==========================================================================

void VWindow::CleanUp(void)
{
}

//==========================================================================
//
//	VWindow::Destroy
//
//==========================================================================

void VWindow::Destroy(void)
{
	guard(VWindow::Destroy);
	bBeingDestroyed = true;
	DestroyWindow();
	KillAllChildren();
	if (Parent)
		Parent->RemoveChild(this);
	if (WinGC)
		delete WinGC;
	Super::Destroy();
	unguard;
}

//==========================================================================
//
//	VWindow::GetRootWindow
//
//==========================================================================

VRootWindow *VWindow::GetRootWindow(void)
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

VModalWindow *VWindow::GetModalWindow(void)
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

VWindow *VWindow::GetParent(void)
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
		while (win && !win->bIsVisible)
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
		while (win && !win->bIsVisible)
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
		while (win && !win->bIsVisible)
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
		while (win && !win->bIsVisible)
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

void VWindow::Raise(void)
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

void VWindow::Lower(void)
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
	if (!!bIsVisible != NewVisibility)
	{
		bIsVisible = NewVisibility;
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
	if (!!bIsSensitive != NewSensitivity)
	{
		bIsSensitive = NewSensitivity;
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
	if (!!bIsSelectable != NewSelectability)
	{
		bIsSelectable = NewSelectability;
/*		if (bIsVisible && bIsSensitive)
		{
			if (bIsSelectable)
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

void VWindow::KillAllChildren(void)
{
	guard(VWindow::KillAllChildren);
	while (FirstChild)
	{
		delete FirstChild;
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

void VWindow::DrawTree(void)
{
	guard(VWindow::DrawTree);
	if (!bIsVisible || !ClipRect.HasArea())
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

void VWindow::ClipTree(void)
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
	if (bTickEnabled)
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

	win = (VWindow *)StaticSpawnObject(NewClass, NULL, PU_STRING);
	win->Init(ParentWindow);
	return win;
	unguardf(("(%s)", NewClass->GetName()));
}

//==========================================================================
//
//	Natives
//
//==========================================================================

inline VWindow *PR_PopWin(void)
{
	return (VWindow *)PR_Pop();
}

IMPLEMENT_FUNCTION(VWindow, Destroy)
{
	VWindow *Self = PR_PopWin();
	delete Self;
}
IMPLEMENT_FUNCTION(VWindow, NewChild)
{
	VClass *ChildClass = (VClass *)PR_Pop();
	VWindow *Self = PR_PopWin();
	PR_Push((int)CreateNewWindow(ChildClass, Self));
}
IMPLEMENT_FUNCTION(VWindow, Raise)
{
	VWindow *Self = PR_PopWin();
	Self->Raise();
}
IMPLEMENT_FUNCTION(VWindow, Lower)
{
	VWindow *Self = PR_PopWin();
	Self->Lower();
}
IMPLEMENT_FUNCTION(VWindow, SetVisibility)
{
	bool bNewVisibility = !!PR_Pop();
	VWindow *Self = PR_PopWin();
	Self->SetVisibility(bNewVisibility);
}
IMPLEMENT_FUNCTION(VWindow, Show)
{
	VWindow *Self = PR_PopWin();
	Self->Show();
}
IMPLEMENT_FUNCTION(VWindow, Hide)
{
	VWindow *Self = PR_PopWin();
	Self->Hide();
}
IMPLEMENT_FUNCTION(VWindow, IsVisible)
{
	VWindow *Self = PR_PopWin();
	PR_Push(Self->IsVisible());
}
IMPLEMENT_FUNCTION(VWindow, SetSensitivity)
{
	bool bNewSensitivity = !!PR_Pop();
	VWindow *Self = PR_PopWin();
	Self->SetSensitivity(bNewSensitivity);
}
IMPLEMENT_FUNCTION(VWindow, Enable)
{
	VWindow *Self = PR_PopWin();
	Self->Enable();
}
IMPLEMENT_FUNCTION(VWindow, Disable)
{
	VWindow *Self = PR_PopWin();
	Self->Disable();
}
IMPLEMENT_FUNCTION(VWindow, IsSensitive)
{
	VWindow *Self = PR_PopWin();
	PR_Push(Self->IsSensitive());
}
IMPLEMENT_FUNCTION(VWindow, SetSelectability)
{
	bool bNewSelectability = !!PR_Pop();
	VWindow *Self = PR_PopWin();
	Self->SetSelectability(bNewSelectability);
}

IMPLEMENT_FUNCTION(VWindow, GetRootWindow)
{
	VWindow *Self = PR_PopWin();
	PR_Push((int)Self->GetRootWindow());
}
IMPLEMENT_FUNCTION(VWindow, GetModalWindow)
{
	VWindow *Self = PR_PopWin();
	PR_Push((int)Self->GetModalWindow());
}
IMPLEMENT_FUNCTION(VWindow, GetParent)
{
	VWindow *Self = PR_PopWin();
	PR_Push((int)Self->GetParent());
}

IMPLEMENT_FUNCTION(VWindow, SetPos)
{
	int NewY = PR_Pop();
	int NewX = PR_Pop();
	VWindow *Self = PR_PopWin();
	Self->SetPos(NewX, NewY);
}
IMPLEMENT_FUNCTION(VWindow, SetSize)
{
	int NewHeight = PR_Pop();
	int NewWidth = PR_Pop();
	VWindow *Self = PR_PopWin();
	Self->SetSize(NewWidth, NewHeight);
}
IMPLEMENT_FUNCTION(VWindow, SetConfiguration)
{
	int NewHeight = PR_Pop();
	int NewWidth = PR_Pop();
	int NewY = PR_Pop();
	int NewX = PR_Pop();
	VWindow *Self = PR_PopWin();
	Self->SetConfiguration(NewX, NewY, NewWidth, NewHeight);
}
IMPLEMENT_FUNCTION(VWindow, SetWidth)
{
	int NewWidth = PR_Pop();
	VWindow *Self = PR_PopWin();
	Self->SetWidth(NewWidth);
}
IMPLEMENT_FUNCTION(VWindow, SetHeight)
{
	int NewHeight = PR_Pop();
	VWindow *Self = PR_PopWin();
	Self->SetHeight(NewHeight);
}

IMPLEMENT_FUNCTION(VWindow, GetBottomChild)
{
	bool bVisibleOnly = !!PR_Pop();
	VWindow *Self = PR_PopWin();
	Self->GetBottomChild(bVisibleOnly);
}
IMPLEMENT_FUNCTION(VWindow, GetTopChild)
{
	bool bVisibleOnly = !!PR_Pop();
	VWindow *Self = PR_PopWin();
	Self->GetTopChild(bVisibleOnly);
}
IMPLEMENT_FUNCTION(VWindow, GetLowerSibling)
{
	bool bVisibleOnly = !!PR_Pop();
	VWindow *Self = PR_PopWin();
	Self->GetLowerSibling(bVisibleOnly);
}
IMPLEMENT_FUNCTION(VWindow, GetHigherSibling)
{
	bool bVisibleOnly = !!PR_Pop();
	VWindow *Self = PR_PopWin();
	Self->GetHigherSibling(bVisibleOnly);
}

IMPLEMENT_FUNCTION(VWindow, DestroyAllChildren)
{
	VWindow *Self = PR_PopWin();
	Self->DestroyAllChildren();
}

//==========================================================================
//
//
//
//==========================================================================

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//
//	Revision 1.5  2003/03/08 12:10:13  dj_jl
//	API fixes.
//	
//	Revision 1.4  2002/08/05 17:20:00  dj_jl
//	Added guarding.
//	
//	Revision 1.3  2002/07/27 18:12:14  dj_jl
//	Added Selectability flag.
//	
//	Revision 1.2  2002/06/14 15:39:22  dj_jl
//	Some fixes for Borland.
//	
//	Revision 1.1  2002/05/29 16:51:50  dj_jl
//	Started a work on native Window classes.
//	
//**************************************************************************
