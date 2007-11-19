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
#include "r_tex.h"
#include "ui.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct VColTranslationDef
{
	rgba_t			From;
	rgba_t			To;
	int				LumFrom;
	int				LumTo;
};

struct VTextColourDef
{
	TArray<VColTranslationDef>	Translations;
	TArray<VColTranslationDef>	ConsoleTranslations;
	rgba_t						FlatColour;
	int							Index;
};

struct VColTransMap
{
	VName		Name;
	int			Index;
};

//
//	VSpecialFont
//
//	Like regular font, but initialised using explicit list of patches.
//
class VSpecialFont : public VFont
{
public:
	VSpecialFont(VName, const TArray<int>&, const TArray<VName>&,
		const bool*);
};

//
//	VFontChar
//
//	Texture class for regular font characters.
//
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
	VTexture* GetHighResolutionTexture();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VFont*								SmallFont;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

VFont*								VFont::Fonts;

static TArray<VTextColourDef>		TextColours;
static TArray<VColTransMap>			TextColourLookup;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VFont::StaticInit
//
//==========================================================================

void VFont::StaticInit()
{
	guard(VFont::StaticInit);
	ParseTextColours();

	//
	//	Initialise standard fonts.
	//

	//	Small font.
	if (W_CheckNumForName(NAME_fonta_s) >= 0)
	{
		SmallFont = new VFont(NAME_smallfont, "fonta%02d", 33, 95, 1);
	}
	else
	{
		SmallFont = new VFont(NAME_smallfont, "stcfn%03d", 33, 95, 33);
	}
	//	Strife's second small font.
	if (W_CheckNumForName(NAME_stbfn033) >= 0)
	{
		new VFont(NAME_smallfont2, "stbfn%03d", 33, 95, 33);
	}
	//	Big font.
	new VFont(NAME_bigfont, "fontb%02d", 33, 95, 1);

	//	Load custom fonts.
	ParseFontDefs();
	unguard;
}

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
	TextColours.Clear();
	TextColourLookup.Clear();
	unguard;
}

//==========================================================================
//
//	VFont::ParseTextColours
//
//==========================================================================

void VFont::ParseTextColours()
{
	guard(VFont::ParseTextColours);
	TArray<VTextColourDef>		TempDefs;
	TArray<VColTransMap>		TempColours;
	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) != NAME_textcolo)
		{
			continue;
		}
		VScriptParser sc(*W_LumpName(Lump), W_CreateLumpReaderNum(Lump));
		while (!sc.AtEnd())
		{
			VTextColourDef& Col = TempDefs.Alloc();
			Col.FlatColour.r = 0;
			Col.FlatColour.g = 0;
			Col.FlatColour.b = 0;
			Col.FlatColour.a = 255;
			Col.Index = -1;

			TArray<VName> Names;
			VColTranslationDef TDef;
			TDef.LumFrom = -1;
			TDef.LumTo = -1;

			//	Name for this colour.
			sc.ExpectString();
			Names.Append(*sc.String.ToLower());
			//	Additional names.
			while (!sc.Check("{"))
			{
				if (Names[0] == "untranslated")
				{
					sc.Error("Colour \"untranslated\" cannot have any other names");
				}
				sc.ExpectString();
				Names.Append(*sc.String.ToLower());
			}

			int TranslationMode = 0;
			while (!sc.Check("}"))
			{
				if (sc.Check("Console:"))
				{
					if (TranslationMode == 1)
					{
						sc.Error("Only one console text colour definition allowed");
					}
					TranslationMode = 1;
					TDef.LumFrom = -1;
					TDef.LumTo = -1;
				}
				else if (sc.Check("Flat:"))
				{
					sc.ExpectString();
					vuint32 C = M_ParseColour(sc.String);
					Col.FlatColour.r = (C >> 16) & 0xff;
					Col.FlatColour.g = (C >> 8) & 0xff;
					Col.FlatColour.b = C & 0xff;
					Col.FlatColour.a = 255;
				}
				else
				{
					//	From colour.
					sc.ExpectString();
					vuint32 C = M_ParseColour(sc.String);
					TDef.From.r = (C >> 16) & 0xff;
					TDef.From.g = (C >> 8) & 0xff;
					TDef.From.b = C & 0xff;
					TDef.From.a = 255;

					//	To colour.
					sc.ExpectString();
					C = M_ParseColour(sc.String);
					TDef.To.r = (C >> 16) & 0xff;
					TDef.To.g = (C >> 8) & 0xff;
					TDef.To.b = C & 0xff;
					TDef.To.a = 255;

					if (sc.CheckNumber())
					{
						//	Optional luminosity ranges.
						if (TDef.LumFrom == -1 && sc.Number != 0)
						{
							sc.Error("First colour range must start at position 0");
						}
						if (sc.Number < 0 || sc.Number > 256)
						{
							sc.Error("Colour index must be in range from 0 to 256");
						}
						if (sc.Number <= TDef.LumTo)
						{
							sc.Error("Colour range must start after end of previous range");
						}
						TDef.LumFrom = sc.Number;

						sc.ExpectNumber();
						if (sc.Number < 0 || sc.Number > 256)
						{
							sc.Error("Colour index must be in range from 0 to 256");
						}
						if (sc.Number <= TDef.LumFrom)
						{
							sc.Error("Ending colour index must be greater than start index");
						}
						TDef.LumTo = sc.Number;
					}
					else
					{
						//	Set default luminosity range.
						TDef.LumFrom = 0;
						TDef.LumTo = 256;
					}
	
					if (TranslationMode == 0)
					{
						Col.Translations.Append(TDef);
					}
					else if (TranslationMode == 1)
					{
						Col.ConsoleTranslations.Append(TDef);
					}
				}
			}

			if (Names[0] == "untranslated")
			{
				if (Col.Translations.Num() != 0 ||
					Col.ConsoleTranslations.Num() != 0)
				{
					sc.Error("The \"untranslated\" colour must be left undefined");
				}
			}
			else
			{
				if (Col.Translations.Num() == 0)
				{
					sc.Error("There must be at least one normal range for a colour");
				}
				if (Col.ConsoleTranslations.Num() == 0)
				{
					//	If console colour translation is not defined, make
					// it white.
					TDef.From.r = 0;
					TDef.From.g = 0;
					TDef.From.b = 0;
					TDef.From.a = 255;
					TDef.To.r = 255;
					TDef.To.g = 255;
					TDef.To.b = 255;
					TDef.To.a = 255;
					TDef.LumFrom = 0;
					TDef.LumTo = 256;
					Col.ConsoleTranslations.Append(TDef);
				}
			}

			//	Add all names to the list of colours.
			for (int i = 0; i < Names.Num(); i++)
			{
				//	Check for redefined colours.
				int CIdx;
				for (CIdx = 0; CIdx < TempColours.Num(); CIdx++)
				{
					if (TempColours[CIdx].Name == Names[i])
					{
						TempColours[CIdx].Index = TempDefs.Num() - 1;
						break;
					}
				}
				if (CIdx == TempColours.Num())
				{
					VColTransMap& CMap = TempColours.Alloc();
					CMap.Name = Names[i];
					CMap.Index = TempDefs.Num() - 1;
				}
			}
		}
	}

	//	Put colour definitions in it's final location.
	for (int i = 0; i < TempColours.Num(); i++)
	{
		VColTransMap& TmpCol = TempColours[i];
		VTextColourDef& TmpDef = TempDefs[TmpCol.Index];
		if (TmpDef.Index == -1)
		{
			TmpDef.Index = TextColours.Num();
			TextColours.Append(TmpDef);
		}
		VColTransMap& Col = TextColourLookup.Alloc();
		Col.Name = TmpCol.Name;
		Col.Index = TmpDef.Index;
	}

	//	Make sure all biilt-in colours are defined.
	check(TextColours.Num() >= NUM_TEXT_COLOURS);
	unguard;
}

//==========================================================================
//
//	VFont::ParseFontDefs
//
//==========================================================================

void VFont::ParseFontDefs()
{
	guard(VFont::ParseFontDefs);
	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) != NAME_fontdefs)
		{
			continue;
		}
		VScriptParser sc(*W_LumpName(Lump), W_CreateLumpReaderNum(Lump));
		while (!sc.AtEnd())
		{
			//	Name of the font.
			sc.ExpectString();
			VName FontName = *sc.String.ToLower();
			sc.Expect("{");

#define CHECK_TYPE(Id) if (FontType == Id) \
	sc.Error(va("Invalid combination of properties in font '%s'", *FontName))
			int FontType = 0;
			VStr Template;
			int First = 33;
			int Count = 223;
			int Start = 33;
			TArray<int> CharIndexes;
			TArray<VName> CharLumps;
			bool NoTranslate[256];
			memset(NoTranslate, 0, sizeof(NoTranslate));

			while (!sc.Check("}"))
			{
				if (sc.Check("template"))
				{
					CHECK_TYPE(2);
					sc.ExpectString();
					Template = sc.String;
					FontType = 1;
				}
				else if (sc.Check("first"))
				{
					CHECK_TYPE(2);
					sc.ExpectNumber();
					First = sc.Number;
					FontType = 1;
				}
				else if (sc.Check("count"))
				{
					CHECK_TYPE(2);
					sc.ExpectNumber();
					Count = sc.Number;
					FontType = 1;
				}
				else if (sc.Check("base"))
				{
					CHECK_TYPE(2);
					sc.ExpectNumber();
					Start = sc.Number;
					FontType = 1;
				}
				else if (sc.Check("notranslate"))
				{
					CHECK_TYPE(1);
					while (sc.CheckNumber() && !sc.Crossed)
					{
						if (sc.Number >= 0 && sc.Number < 256)
						{
							NoTranslate[sc.Number] = true;
						}
					}
					FontType = 2;
				}
				else
				{
					CHECK_TYPE(1);
					sc.ExpectString();
					const char* CPtr = *sc.String;
					int CharIdx = VStr::GetChar(CPtr);
					sc.ExpectString();
					VName LumpName(*sc.String, VName::AddLower8);
					if (W_CheckNumForName(LumpName, WADNS_Graphics) >= 0)
					{
						CharIndexes.Append(CharIdx);
						CharLumps.Append(LumpName);
					}
					FontType = 2;
				}
			}
			if (FontType == 1)
			{
				new VFont(FontName, Template, First, Count, Start);
			}
			else if (FontType == 2)
			{
				if (CharIndexes.Num())
				{
					new VSpecialFont(FontName, CharIndexes, CharLumps,
						NoTranslate);
				}
			}
			else
			{
				sc.Error("Font has no attributes");
			}
		}
	}
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
//	VFont::GetFont
//
//==========================================================================

VFont* VFont::GetFont(VName AName)
{
	guard(VFont::GetFont);
	return FindFont(AName);
	unguard;
}

//==========================================================================
//
//	VFont::VFont
//
//==========================================================================

VFont::VFont()
{
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
	Translation = NULL;
	bool ColoursUsed[256];
	memset(ColoursUsed, 0, sizeof(ColoursUsed));

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
			FChar.BaseTex = Tex;
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
		SpaceWidth = (Chars[NIdx].BaseTex->GetScaledWidth() + 1) / 2;
	}
	else
	{
		SpaceWidth = 4;
	}

	BuildTranslations(ColoursUsed);

	//	Create texture objects for all different colours.
	for (int i = 0; i < Chars.Num(); i++)
	{
		Chars[i].Textures = new VTexture*[TextColours.Num()];
		for (int j = 0; j < TextColours.Num(); j++)
		{
			Chars[i].Textures[j] = new VFontChar(Chars[i].BaseTex,
				Translation + j * 256);
			//	Currently all render drivers expect all textures to be
			// registered in texture manager.
			GTextureManager.AddTexture(Chars[i].Textures[j]);
		}
	}
	unguard;
}

//==========================================================================
//
//	VFont::~VFont
//
//==========================================================================

VFont::~VFont()
{
	guard(VFont::~VFont);
	for (int i = 0; i < Chars.Num(); i++)
	{
		if (Chars[i].Textures)
		{
			delete[] Chars[i].Textures;
		}
	}
	Chars.Clear();
	if (Translation)
	{
		delete[] Translation;
	}
	unguard;
}

//==========================================================================
//
//	VFont::BuildTranslations
//
//==========================================================================

void VFont::BuildTranslations(const bool* ColoursUsed)
{
	guard(VFont::BuildTranslations);
	//	Calculate luminosity for all colours and find minimal and maximal
	// values for used colours.
	float Luminosity[256];
	float MinLum = 1000000.0;
	float MaxLum = 0.0;
	for (int i = 1; i < 256; i++)
	{
		Luminosity[i] = r_palette[i].r * 0.299 + r_palette[i].g * 0.587 +
			r_palette[i].b * 0.114;
		if (ColoursUsed[i])
		{
			if (MinLum > Luminosity[i])
			{
				MinLum = Luminosity[i];
			}
			if (MaxLum < Luminosity[i])
			{
				MaxLum = Luminosity[i];
			}
		}
	}
	//	Create gradual luminosity values.
	for (int i = 1; i < 256; i++)
	{
		Luminosity[i] = (Luminosity[i] - MinLum) / (MaxLum - MinLum);
		Luminosity[i] = MID(0.0, Luminosity[i], 1.0);
	}

	Translation = new rgba_t[256 * TextColours.Num()];
	for (int ColIdx = 0; ColIdx < TextColours.Num(); ColIdx++)
	{
		rgba_t* pOut = Translation + ColIdx * 256;
		const TArray<VColTranslationDef>& TList =
			TextColours[ColIdx].Translations;
		if (ColIdx == CR_UNTRANSLATED || !TList.Num())
		{
			memcpy(pOut, r_palette, 4 * 256);
			continue;
		}

		pOut[0] = r_palette[0];
		for (int i = 1; i < 256; i++)
		{
			int ILum = (int)(Luminosity[i] * 256);
			int TDefIdx = 0;
			while (TDefIdx < TList.Num() - 1 && ILum > TList[TDefIdx].LumTo)
			{
				TDefIdx++;
			}
			const VColTranslationDef& TDef = TList[TDefIdx];

			//	Linearly interpolate between colours.
			float v = ((float)(ILum - TDef.LumFrom) /
				(float)(TDef.LumTo - TDef.LumFrom));
			int r = (int)((1.0 - v) * TDef.From.r + v * TDef.To.r);
			int g = (int)((1.0 - v) * TDef.From.g + v * TDef.To.g);
			int b = (int)((1.0 - v) * TDef.From.b + v * TDef.To.b);
			pOut[i].r = MID(0, r, 255);
			pOut[i].g = MID(0, g, 255);
			pOut[i].b = MID(0, b, 255);
			pOut[i].a = 255;
		}
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

VTexture* VFont::GetChar(int Chr, int* pWidth, int Colour) const
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

	if (Colour < 0 || Colour >= TextColours.Num())
	{
		Colour = CR_UNTRANSLATED;
	}
	VTexture* Tex = Chars[Idx].Textures ? Chars[Idx].Textures[Colour] :
		Chars[Idx].BaseTex;
	*pWidth = Tex->GetScaledWidth();
	return Tex;
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

	return Chars[Idx].BaseTex->GetScaledWidth();
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
//	VFont::ParseColourEscape
//
//	Assumes that pColour points to the character right after the colour
// escape character.
//
//==========================================================================

int VFont::ParseColourEscape(const char*& pColour, int NormalColour,
	int BoldColour)
{
	guard(VFont::ParseColourEscape);
	const char* Chr = pColour;
	int Col = *Chr++;

	//	Standard colors, upper case
	if (Col >= 'A' && Col < 'A' + NUM_TEXT_COLOURS)
	{
		Col -= 'A';
	}
	//	Standard colors, lower case
	else if (Col >= 'a' && Col < 'a' + NUM_TEXT_COLOURS)
	{
		Col -= 'a';
	}
	//	Normal colour
	else if (Col == '-')
	{
		Col = NormalColour;
	}
	//	Bold colour
	else if (Col == '+')
	{
		Col = BoldColour;
	}
	//	Named colours.
	else if (Col == '[')
	{
		VStr CName;
		while (*Chr && *Chr != ']')
		{
			CName += *Chr++;
		}
		if (*Chr == ']')
		{
			Chr++;
		}
		Col = FindTextColour(*CName.ToLower());
	}
	else
	{
		if (!Col)
		{
			Chr--;
		}
		Col = CR_UNDEFINED;
	}

	//	Set pointer after the colour definition.
	pColour = Chr;
	return Col;
	unguard;
}

//==========================================================================
//
//	VFont::FindTextColour
//
//==========================================================================

int VFont::FindTextColour(VName Name)
{
	guard(VFont::FindTextColour);
	for (int i = 0; i < TextColourLookup.Num(); i++)
	{
		if (TextColourLookup[i].Name == Name)
		{
			return TextColourLookup[i].Index;
		}
	}
	return CR_UNTRANSLATED;
	unguard;
}

//==========================================================================
//
//	VSpecialFont::VSpecialFont
//
//==========================================================================

VSpecialFont::VSpecialFont(VName AName, const TArray<int>& CharIndexes,
	const TArray<VName>& CharLumps, const bool* NoTranslate)
{
	guard(VSpecialFont::VSpecialFont);
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
	Translation = NULL;
	bool ColoursUsed[256];
	memset(ColoursUsed, 0, sizeof(ColoursUsed));

	check(CharIndexes.Num() == CharLumps.Num());
	for (int i = 0; i < CharIndexes.Num(); i++)
	{
		int Char = CharIndexes[i];
		VName LumpName = CharLumps[i];

		VTexture* Tex = GTextureManager[GTextureManager.AddPatch(LumpName,
			TEXTYPE_Pic)];
		FFontChar& FChar = Chars.Alloc();
		FChar.Char = Char;
		FChar.BaseTex = Tex;
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

	//	Exclude non-translated colours from calculations.
	for (int i = 0; i < 256; i++)
	{
		if (NoTranslate[i])
		{
			ColoursUsed[i] = false;
		}
	}

	//	Set up width of a space character as half width of N character
	// or 4 if character N has no graphic for it.
	int NIdx = FindChar('N');
	if (NIdx >= 0)
	{
		SpaceWidth = (Chars[NIdx].BaseTex->GetScaledWidth() + 1) / 2;
	}
	else
	{
		SpaceWidth = 4;
	}

	BuildTranslations(ColoursUsed);

	//	Map non-translated colours to their original values
	for (int i = 0; i < TextColours.Num(); i++)
	{
		for (int j = 0; j < 256; j++)
		{
			if (NoTranslate[j])
			{
				Translation[i * 256 + j] = r_palette[j];
			}
		}
	}

	//	Create texture objects for all different colours.
	for (int i = 0; i < Chars.Num(); i++)
	{
		Chars[i].Textures = new VTexture*[TextColours.Num()];
		for (int j = 0; j < TextColours.Num(); j++)
		{
			Chars[i].Textures[j] = new VFontChar(Chars[i].BaseTex,
				Translation + j * 256);
			//	Currently all render drivers expect all textures to be
			// registered in texture manager.
			GTextureManager.AddTexture(Chars[i].Textures[j]);
		}
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

//==========================================================================
//
//	VFontChar::GetHighResolutionTexture
//
//==========================================================================

VTexture* VFontChar::GetHighResolutionTexture()
{
	guard(VFontChar::GetHighResolutionTexture);
	if (!r_hirestex)
	{
		return NULL;
	}
	if (!HiResTexture)
	{
		VTexture* Tex = BaseTex->GetHighResolutionTexture();
		if (Tex)
		{
			HiResTexture = new VFontChar(Tex, Palette);
		}
	}
	return HiResTexture;
	unguard;
}
