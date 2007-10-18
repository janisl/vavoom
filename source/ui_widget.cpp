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
