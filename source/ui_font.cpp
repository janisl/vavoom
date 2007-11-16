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
#include "r_shared.h"
#include "ui.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VFontChar : public VTexture
{
private:
	VTexture*		BaseTex;
	rgba_t*			Palette;

public:
	VFontChar(VTexture*, rgba_t*);
	~VFontChar();
	vuint8* GetPixels();
	rgba_t* GetPalette();
	void Unload();
};

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
	FirstChar = -1;
	LastChar = -1;
	FontHeight = 0;
	Kerning = 0;
	bool ColoursUsed[256];

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
			FChar.Tex = new VFontChar(Tex, Translation);
			//	Currently render drivers expects all textures to be registered
			// in texture manager.
			GTextureManager.AddTexture(FChar.Tex);
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

			//	Mark colours that are used by this texture.
			MarkUsedColours(Tex, ColoursUsed);
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

	//	0 is transparent.
	Translation[0] = r_palette[0];
	for (int i = 1; i < 256; i++)
	{
#if 0
		if (!ColoursUsed[i])
		{
			continue;
		}
		int r = r_palette[i].r;
		int g = r_palette[i].g;
		int b = r_palette[i].b;
		r = (int)(r * 0.3 + g * 0.5 + b * 0.2);
		g = r;
		b = r;
		Translation[i].r = MID(0, r, 255);
		Translation[i].g = MID(0, g, 255);
		Translation[i].b = MID(0, b, 255);
		Translation[i].a = 255;
#else
		Translation[i] = r_palette[i];
#endif
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

//==========================================================================
//
//	VFont::MarkUsedColours
//
//==========================================================================

void VFont::MarkUsedColours(VTexture* Tex, bool* Used)
{
	guard(VFont::MarkUsedColours);
	const vuint8* Pixels = Tex->GetPixels8();
	int Count = Tex->GetWidth() * Tex->GetHeight();
	for (int i = 0; i < Count; i++)
	{
		Used[Pixels[i]] = true;
	}
	unguard;
}

//==========================================================================
//
//	VFontChar::VFontChar
//
//==========================================================================

VFontChar::VFontChar(VTexture* ATex, rgba_t* APalette)
: BaseTex(ATex)
, Palette(APalette)
{
	Type = TEXTYPE_FontChar;
	Format = TEXFMT_8Pal;
	Name = NAME_None;
	Width = BaseTex->GetWidth();
	Height = BaseTex->GetHeight();
	SOffset = BaseTex->SOffset;
	TOffset = BaseTex->TOffset;
	SScale = BaseTex->SScale;
	TScale = BaseTex->TScale;
}

//==========================================================================
//
//	VFontChar::~VFontChar
//
//==========================================================================

VFontChar::~VFontChar()
{
}

//==========================================================================
//
//	VFontChar::GetPixels
//
//==========================================================================

vuint8* VFontChar::GetPixels()
{
	guard(VFontChar::GetPixels);
	return BaseTex->GetPixels8();
	unguard;
}

//==========================================================================
//
//	VFontChar::GetPalette
//
//==========================================================================

rgba_t* VFontChar::GetPalette()
{
	guard(VFontChar::GetPalette);
	return Palette;
	unguard;
}

//==========================================================================
//
//	VFontChar::Unload
//
//==========================================================================

void VFontChar::Unload()
{
	guard(VFontChar::Unload);
	BaseTex->Unload();
	unguard;
}
