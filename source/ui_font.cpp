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

VFont*				VFont::Fonts;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VFont::StaticShutdown
//
//==========================================================================

void VFont::StaticShutdown()
{
	guard(VFont::StaticShutdown);
	VFont* F = Fonts;
	while (F)
	{
		VFont* Next = F->Next;
		delete F;
		F = Next;
	}
	Fonts = NULL;
	unguard;
}

//==========================================================================
//
//	VFont::FindFont
//
//==========================================================================

VFont* VFont::FindFont(VName AName)
{
	guard(VFont::FindFont);
	for (VFont* F = Fonts; F; F = F->Next)
	{
		if (F->Name == AName)
		{
			return F;
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VFont::VFont
//
//==========================================================================

VFont::VFont(VName AName, const VStr& FormatStr, int First, int Count,
	int StartIndex)
{
	guard(VFont::VFont);
	Name = AName;
	Next = Fonts;
	Fonts = this;

	for (int i = 0; i < 128; i++)
	{
		AsciiChars[i] = -1;
	}
	FontHeight = 0;
	FirstChar = -1;
	LastChar = -1;

	for (int i = 0; i < Count; i++)
	{
		int Char = i + First;
		char Buffer[10];
		sprintf(Buffer, *FormatStr, i + StartIndex);
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
			VTexture* Tex = GTextureManager[GTextureManager.AddPatch(LumpName,
				TEXTYPE_Pic)];
			FFontChar& FChar = Chars.Alloc();
			FChar.Char = Char;
			FChar.Tex = Tex;
			if (Char < 128)
			{
				AsciiChars[Char] = Chars.Num() - 1;
			}

			//	Calculate height of font character and adjust font height
			// as needed.
			int Height = Tex->GetScaledHeight();
			int TOffs = Tex->GetScaledTOffset();
			Height += abs(TOffs);
			if (FontHeight < Height)
			{
				FontHeight = Height;
			}

			//	Update first and last characters.
			if (FirstChar == -1)
			{
				FirstChar = Char;
			}
			LastChar = Char;
		}
	}

	//	Set up width of a space character as half width of N character
	// or 4 if character N has no graphic for it.
	int NIdx = FindChar('N');
	if (NIdx >= 0)
	{
		SpaceWidth = (Chars[NIdx].Tex->GetScaledWidth() + 1) / 2;
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

int VFont::FindChar(int Chr) const
{
	//	Check if character is outside of available character range.
	if (Chr < FirstChar || Chr > LastChar)
	{
		return -1;
	}

	//	Fast look-up for ASCII characters
	if (Chr < 128)
	{
		return AsciiChars[Chr];
	}

	//	A slower one for unicode.
	for (int i = 0; i < Chars.Num(); i++)
	{
		if (Chars[i].Char == Chr)
		{
			return i;
		}
	}
	return -1;
}

//==========================================================================
//
//	VFont::GetChar
//
//==========================================================================

VTexture* VFont::GetChar(int Chr, int* pWidth) const
{
	guard(VFont::GetChar);
	int Idx = FindChar(Chr);
	if (Idx < 0)
	{
		//	Try upper-case letter.
		Chr = VStr::ToUpper(Chr);
		Idx = FindChar(Chr);
		if (Idx < 0)
		{
			*pWidth = SpaceWidth;
			return NULL;
		}
	}

	*pWidth = Chars[Idx].Tex->GetScaledWidth();
	return Chars[Idx].Tex;
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
	int Idx = FindChar(Chr);
	if (Idx < 0)
	{
		//	Try upper-case letter.
		Chr = VStr::ToUpper(Chr);
		Idx = FindChar(Chr);
		if (Idx < 0)
		{
			return SpaceWidth;
		}
	}

	return Chars[Idx].Tex->GetScaledWidth();
	unguard;
}
