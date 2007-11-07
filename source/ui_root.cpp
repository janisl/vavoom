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

IMPLEMENT_CLASS(V, RootWindow);

VRootWindow *GRoot;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VRootWindow::VRootWindow
//
//==========================================================================

VRootWindow::VRootWindow()
{
	WindowType = WIN_Root;
}

//==========================================================================
//
//	VRootWindow::Init
//
//==========================================================================

void VRootWindow::Init()
{
	Super::Init(NULL);
	SetSize(640, 480);
}

//==========================================================================
//
//	VRootWindow::DrawWidgets
//
//==========================================================================

void VRootWindow::DrawWidgets()
{
	DrawTree();
}

//==========================================================================
//
//	VRootWindow::TickWindows
//
//==========================================================================

void VRootWindow::TickWindows(float DeltaTime)
{
	if (SizeScaleX != fScaleX)
	{
		SizeScaleX = fScaleX;
		SizeScaleY = fScaleY;
		ClipTree();
	}
	TickTree(DeltaTime);
}

//==========================================================================
//
//	VRootWindow::StaticInit
//
//==========================================================================

void VRootWindow::StaticInit()
{
	guard(VRootWindow::StaticInit);
	GRoot = Spawn<VRootWindow>();
	GRoot->Init();
	GClGame->GRoot = GRoot;
	unguard;
}
