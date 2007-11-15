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

VFont::VFont(const VStr& Name, int StartIndex)
{
	guard(VFont::VFont);
	for (int i = 0; i < 96; i++)
	{
		Pics[i] = NULL;
	}
	FontHeight = 0;

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

			//	Calculate height of font character and adjust font height
			// as needed.
			int Height = Pics[i]->GetScaledHeight();
			int TOffs = Pics[i]->GetScaledTOffset();
			Height += abs(TOffs);
			if (FontHeight < Height)
			{
				FontHeight = Height;
			}
		}
	}

	//	Set up width of a space character as half width of N character
	// or 4 if character N has no graphic for it.
	if (Pics['N' - 32])
	{
		SpaceWidth = (Pics['N' - 32]->GetScaledWidth() + 1) / 2;
	}
	else
	{
		SpaceWidth = 4;
	}
	unguard;
}

//==========================================================================
//
//	VFont::GetChar
//
//==========================================================================

VTexture* VFont::GetChar(int Chr, int* pWidth) const
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

int VFont::GetCharWidth(int Chr) const
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
