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
