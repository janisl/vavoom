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
//	VFont::VFont
//
//==========================================================================

VFont::VFont(const VStr& Name, int SpaceW, int SpaceH, int StartIndex)
: SpaceWidth(SpaceW)
, SpaceHeight(SpaceH)
{
	guard(VFont::VFont);
	for (int i = 0; i < 96; i++)
	{
		Pics[i] = NULL;
	}

	for (int i = 0; i < 96; i++)
	{
		char Buffer[10];
		sprintf(Buffer, *Name, i + StartIndex);
		VName LumpName(Buffer, VName::AddLower8);
		int Lump = W_CheckNumForName(LumpName, WADNS_Graphics);

		//	In Doom stcfn121 is actually an upper-case 'I' and not 'y' and
		// may wad authors provide it as such, so load it only if wad also
		// provides stcfn120 ('x') and stcfn122 ('z').
		if (LumpName == "stcfn121" &&
			(W_CheckNumForName("stcfn120", WADNS_Graphics) == -1 ||
			W_CheckNumForName("stcfn122", WADNS_Graphics) == -1))
		{
			Lump = -1;
		}

		if (Lump >= 0)
		{
			Pics[i] = GTextureManager[GTextureManager.AddPatch(LumpName,
				TEXTYPE_Pic)];
		}
	}
	unguard;
}

//==========================================================================
//
//	VFont::GetChar
//
//==========================================================================

VTexture* VFont::GetChar(int Chr, int* pWidth)
{
	guard(VFont::GetChar);
	if (Chr < 32 || Chr >= 128 || !Pics[Chr - 32])
	{
		//	Try upper-case letter.
		Chr = VStr::ToUpper(Chr);
		if (Chr < 32 || Chr >= 128 || !Pics[Chr - 32])
		{
			*pWidth = SpaceWidth;
			return NULL;
		}
	}

	*pWidth = Pics[Chr - 32]->GetScaledWidth();
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
	if (Chr < 32 || Chr >= 128 || !Pics[Chr - 32])
	{
		//	Try upper-case letter.
		Chr = VStr::ToUpper(Chr);
		if (Chr < 32 || Chr >= 128 || !Pics[Chr - 32])
		{
			return SpaceWidth;
		}
	}

	return Pics[Chr - 32]->GetScaledWidth();
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
	if (Chr < 32 || Chr >= 128 || !Pics[Chr - 32])
	{
		//	Try upper-case letter.
		Chr = VStr::ToUpper(Chr);
		if (Chr < 32 || Chr >= 128 || !Pics[Chr - 32])
		{
			return SpaceHeight;
		}
	}

	return Pics[Chr - 32]->GetScaledHeight();
	unguard;
}
