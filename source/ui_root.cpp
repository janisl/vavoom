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
