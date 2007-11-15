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

static void T_LoadFont(font_e FontNr, const char* Name, int SpaceW, int SpaceH);
static void T_LoadFont2(font_e FontNr, const char* Name, int SpaceW, int SpaceH);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

VFont*					Fonts[NUMFONTTYPES];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	T_Init
//
//==========================================================================

void T_Init()
{
	guard(T_Init);
	// Load fonts
	if (W_CheckNumForName(NAME_stcfn033) >= 0)
	{
		T_LoadFont2(font_small, "STCFN", 4, 7);
		T_LoadFont2(font_yellow, "STBFN", 4, 7);
	}
	else
	{
		T_LoadFont(font_small, "FONTA", 4, 7);
		T_LoadFont(font_yellow, "FONTAY", 4, 7);
	}
	T_LoadFont(font_big, "FONTB", 8, 10);
	unguard;
}

//==========================================================================
//
//	T_Shutdown
//
//==========================================================================

void T_Shutdown()
{
	guard(T_Shutdown);
	for (int i = 0; i < NUMFONTTYPES; i++)
	{
		if (Fonts[i])
		{
			delete Fonts[i];
		}
	}
	unguard;
}

//==========================================================================
//
//	T_LoadFont
//
//==========================================================================

static void T_LoadFont(font_e FontNr, const char* Name, int SpaceW, int SpaceH)
{
	int		i;
	char   	buffer[10];

	Fonts[FontNr] = new VFont;
	memset(Fonts[FontNr], 0, sizeof(VFont));
	Fonts[FontNr]->SpaceWidth = SpaceW;
	Fonts[FontNr]->SpaceHeight = SpaceH;
	for (i = 0; i < 96; i++)
	{
		Fonts[FontNr]->Pics[i] = -1;
	}
	for (i = 0; i < 96; i++)
	{
		sprintf(buffer, "%s%02d", Name, i);
		if (W_CheckNumForName(VName(buffer, VName::AddLower8), WADNS_Graphics) >= 0)
		{
			Fonts[FontNr]->Pics[i] = GTextureManager.AddPatch(VName(buffer,
				VName::AddLower8), TEXTYPE_Pic);
			GTextureManager.GetTextureInfo(Fonts[FontNr]->Pics[i],
				&Fonts[FontNr]->PicInfo[i]);
		}
	}
}

//==========================================================================
//
//	T_LoadFont2
//
//==========================================================================

static void T_LoadFont2(font_e FontNr, const char* Name, int SpaceW, int SpaceH)
{
	int		i;
	char   	buffer[10];

	Fonts[FontNr] = new VFont;
	memset(Fonts[FontNr], 0, sizeof(VFont));
	Fonts[FontNr]->SpaceWidth = SpaceW;
	Fonts[FontNr]->SpaceHeight = SpaceH;
	for (i = 0; i < 96; i++)
	{
		Fonts[FontNr]->Pics[i] = -1;
	}
	for (i = 0; i < 64; i++)
	{
		sprintf(buffer, "%s%03d", Name, i + 32);
		if (W_CheckNumForName(VName(buffer, VName::AddLower8), WADNS_Graphics) >= 0)
		{
			Fonts[FontNr]->Pics[i] = GTextureManager.AddPatch(VName(buffer,
				VName::AddLower8), TEXTYPE_Pic);
			GTextureManager.GetTextureInfo(Fonts[FontNr]->Pics[i],
				&Fonts[FontNr]->PicInfo[i]);
		}
	}
}

//==========================================================================
//
//	T_SetFont
//
//==========================================================================

void T_SetFont(font_e FontNr)
{
	GRoot->SetFont(FontNr);
}

//==========================================================================
//
//	T_SetAlign
//
//==========================================================================

void T_SetAlign(halign_e NewHAlign, valign_e NewVAlign)
{
	GRoot->SetTextAlign(NewHAlign, NewVAlign);
}

//==========================================================================
//
//	T_SetShadow
//
//==========================================================================

void T_SetShadow(bool state)
{
	GRoot->SetTextShadow(state);
}

//==========================================================================
//
//	T_StringWidth
//
//==========================================================================

int T_StringWidth(VFont* Font, const VStr& String)
{
	guard(T_StringWidth);
	int w = 0;
	for (const char* SPtr = *String; *SPtr;)
	{
		int c = VStr::GetChar(SPtr);
		w += Font->GetCharWidth(c);
	}
	return w;
	unguard;
}

//==========================================================================
//
//	T_StringHeight
//
//==========================================================================

int T_StringHeight(VFont* Font, const VStr& String)
{
	guard(T_StringHeight);
	int h = Font->SpaceHeight;
	for (const char* SPtr = *String; *SPtr;)
	{
		int c = VStr::GetChar(SPtr);
		if (h < Font->GetCharHeight(c))
		{
			h = Font->GetCharHeight(c);
		}
	}
	return h;
	unguard;
}

//==========================================================================
//
//	T_TextWidth
//
//==========================================================================

int T_TextWidth(VFont* Font, const VStr& String)
{
	guard(T_TextWidth);
	size_t		i;
	int			w1;
	int			w = 0;
	int			start = 0;

	for (i = 0; i <= String.Length(); i++)
		if ((String[i] == '\n') || !String[i])
		{
			w1 = T_StringWidth(Font, VStr(String, start, i - start));
			if (w1 > w)
				w = w1;
			start = i;
		}
	return w;
	unguard;
}

//==========================================================================
//
//	T_TextHeight
//
//==========================================================================

int T_TextHeight(VFont* Font, const VStr& String)
{
	guard(T_TextHeight);
	size_t		i;
	int			h = 0;
	int			start = 0;

	for (i=0; i <= String.Length(); i++)
		if ((String[i] == '\n') || !String[i])
		{
			h += T_StringHeight(Font, VStr(String, start, i - start));
			start = i;
		}
	return h;
	unguard;
}

//==========================================================================
//
//	T_DrawString
//
//	Write a string using the font.
//
//==========================================================================

void T_DrawString(int x, int y, const VStr& String)
{
	GRoot->DrawString(x, y, String);
}

//==========================================================================
//
//	T_DrawText
//
//==========================================================================

void T_DrawText(int x, int y, const VStr& String)
{
	GRoot->DrawText(x, y, String);
}

//==========================================================================
//
//	T_DrawTextW
//
//==========================================================================

int T_DrawTextW(int x, int y, const VStr& String, int w)
{
	return GRoot->DrawTextW(x, y, String, w);
}

//==========================================================================
//
//	T_DrawCursor
//
//==========================================================================

void T_DrawCursor()
{
	GRoot->DrawCursor();
}

//==========================================================================
//
//	T_DrawCursorAt
//
//==========================================================================

void T_DrawCursorAt(int x, int y)
{
	GRoot->DrawCursorAt(x, y);
}

//==========================================================================
//
//	T_DrawString8
//
//	Write a string using the font with fixed width 8.
//
//==========================================================================

void T_DrawString8(int x, int y, const VStr& String)
{
	GRoot->DrawString8(x, y, String);
}
