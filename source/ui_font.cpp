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

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VFont::GetChar
//
//==========================================================================

int VFont::GetChar(int Chr, int* pWidth)
{
	guard(VFont::GetChar);
	if (Chr < 32 || Chr >= 128 || Pics[Chr - 32] < 0)
	{
		//	Try upper-case letter.
		Chr = VStr::ToUpper(Chr);
		if (Chr < 32 || Chr >= 128 || Pics[Chr - 32] < 0)
		{
			*pWidth = SpaceWidth;
			return -1;
		}
	}
	*pWidth = PicInfo[Chr - 32].width;
	return Pics[Chr - 32];
	unguard;
}

//==========================================================================
//
//	VFont::GetCharWidth
//
//==========================================================================

int VFont::GetCharWidth(int Chr)
{
	guard(VFont::GetCharWidth);
	if (Chr < 32 || Chr >= 128 || Pics[Chr - 32] < 0)
	{
		//	Try upper-case letter.
		Chr = VStr::ToUpper(Chr);
		if (Chr < 32 || Chr >= 128 || Pics[Chr - 32] < 0)
		{
			return SpaceWidth;
		}
	}
	return PicInfo[Chr - 32].width;
	unguard;
}

//==========================================================================
//
//	VFont::GetCharHeight
//
//==========================================================================

int VFont::GetCharHeight(int Chr)
{
	guard(VFont::GetCharHeight);
	if (Chr < 32 || Chr >= 128 || Pics[Chr - 32] < 0)
	{
		//	Try upper-case letter.
		Chr = VStr::ToUpper(Chr);
		if (Chr < 32 || Chr >= 128 || Pics[Chr - 32] < 0)
		{
			return SpaceHeight;
		}
	}
	return PicInfo[Chr - 32].height;
	unguard;
}
