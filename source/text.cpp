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

static font_t*			Fonts[NUMFONTTYPES];
font_t*					Font;

//Distance between chars and strings
int						HDistance;
int						VDistance;

//
// Alignements
//
halign_e				HAlign;
valign_e				VAlign;

//
// Shadow
//
bool					t_shadowed = false;

//
// Cursor
//
int						LastX;
int						LastY;

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

	// Set default values
	T_SetFont(font_small);
	T_SetAlign(hleft, vtop);
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

	Fonts[FontNr] = new font_t;
	memset(Fonts[FontNr], 0, sizeof(font_t));
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
			if ((i + 32 >= 'a') && (i + 32 <= 'z') &&
				Fonts[FontNr]->Pics[i + 'A' - 'a'] < 0)
			{
				Fonts[FontNr]->Pics[i + 'A' - 'a'] = Fonts[FontNr]->Pics[i];
				Fonts[FontNr]->PicInfo[i + 'A' - 'a'] = Fonts[FontNr]->PicInfo[i];
			}
			if ((i + 32 >= 'A') && (i + 32 <= 'Z') &&
				Fonts[FontNr]->Pics[i + 'a' - 'A'] < 0)
			{
				Fonts[FontNr]->Pics[i + 'a' - 'A'] = Fonts[FontNr]->Pics[i];
				Fonts[FontNr]->PicInfo[i + 'a' - 'A'] = Fonts[FontNr]->PicInfo[i];
			}
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

	Fonts[FontNr] = new font_t;
	memset(Fonts[FontNr], 0, sizeof(font_t));
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
			if ((i + 32 >= 'a') && (i + 32 <= 'z') &&
				Fonts[FontNr]->Pics[i + 'A' - 'a'] < 0)
			{
				Fonts[FontNr]->Pics[i + 'A' - 'a'] = Fonts[FontNr]->Pics[i];
				Fonts[FontNr]->PicInfo[i + 'A' - 'a'] = Fonts[FontNr]->PicInfo[i];
			}
			if ((i + 32 >= 'A') && (i + 32 <= 'Z') &&
				Fonts[FontNr]->Pics[i + 'a' - 'A'] < 0)
			{
				Fonts[FontNr]->Pics[i + 'a' - 'A'] = Fonts[FontNr]->Pics[i];
				Fonts[FontNr]->PicInfo[i + 'a' - 'A'] = Fonts[FontNr]->PicInfo[i];
			}
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
	Font = Fonts[FontNr];
}

//==========================================================================
//
//	T_SetAlign
//
//==========================================================================

void T_SetAlign(halign_e NewHAlign, valign_e NewVAlign)
{
	HAlign = NewHAlign;
	VAlign = NewVAlign;
}

//==========================================================================
//
//	T_SetShadow
//
//==========================================================================

void T_SetShadow(bool state)
{
	t_shadowed = state;
}

//==========================================================================
//
//	T_StringWidth
//
//==========================================================================

int T_StringWidth(const VStr& String)
{
	guard(T_StringWidth);
	size_t			i;
	int				w = 0;
	int				c;
	
	for (i = 0; i < String.Length();i++)
	{
		c = String[i] - 32;

		if (c < 0 || c >= 96 || Font->Pics[c] < 0)
			w += Font->SpaceWidth + HDistance;
		else
			w += Font->PicInfo[c].width + HDistance;
	}
	if (w) w -= HDistance;
		
	return w;
	unguard;
}

//==========================================================================
//
//	T_StringHeight
//
//==========================================================================

int T_StringHeight(const VStr& String)
{
	guard(T_StringHeight);
	size_t	i;
	int		c;
	int		h = Font->SpaceHeight;

	for (i = 0;i < String.Length();i++)
	{
		c = String[i] - 32;

		if (c >= 0 && c < 96 && Font->Pics[c] >= 0
			&& (h < Font->PicInfo[c].height))
				h = Font->PicInfo[c].height;
	}
		
	return h;
	unguard;
}

//==========================================================================
//
//	T_TextWidth
//
//==========================================================================

int T_TextWidth(const VStr& String)
{
	guard(T_TextWidth);
	size_t		i;
	int			w1;
	int			w = 0;
	int			start = 0;

	for (i = 0; i <= String.Length(); i++)
		if ((String[i] == '\n') || !String[i])
		{
			w1 = T_StringWidth(VStr(String, start, i - start));
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

int T_TextHeight(const VStr& String)
{
	guard(T_TextHeight);
	VStr		cs;
	size_t		i;
	int			h = 0;
	int			start = 0;

	for (i=0; i <= String.Length(); i++)
		if ((String[i] == '\n') || !String[i])
		{
			h += T_StringHeight(VStr(String, start, i - start)) + VDistance;
			start = i;
		}
	h -= VDistance;
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
	guard(T_DrawNString);
	int		w;
	int		c;
	int		cx;
	int		cy;
	int		i;
		
	if (!String)
		return;
		
	cx = x;
	cy = y;

	if (HAlign == hcentre)
		cx -= T_StringWidth(String) / 2;
	if (HAlign == hright)
		cx -= T_StringWidth(String);

	int length = String.Length();
	
	if (cx < 0 || cy < 0 || cx >= VirtualWidth || cy >= VirtualHeight)
	{
		GCon->Logf(NAME_Dev, "T_DrawNString: Draw text \"%s\" at (%d,%d)",
			*String, cx, cy);
		return;
	}

	for (i=0; i<length; i++)
	{
		c = String[i] - 32;

		if (c < 0)
		{
			continue;
		}
		if (c >= 96 || Font->Pics[c] < 0)
		{
			cx += Font->SpaceWidth + HDistance;
			continue;
		}
		
		w = Font->PicInfo[c].width;
		if (cx + w > VirtualWidth)
		{
			if (HAlign != hleft)
				break;
			cx = x;
			cy += T_StringHeight(String) + VDistance;
		}
		if (t_shadowed)
			R_DrawShadowedPic(cx, cy, Font->Pics[c]);
		else
			R_DrawPic(cx, cy, Font->Pics[c]);
		cx += w + HDistance;
	}
	LastX = cx;
	LastY = cy;
	unguard;
}

//==========================================================================
//
//	T_DrawText
//
//==========================================================================

void T_DrawText(int x, int y, const VStr& String)
{
	guard(T_DrawText);
	int			start = 0;
	int			cx;
	int			cy;
	int			i;

	cx = x;
	cy = y;

	if (VAlign == vcentre)
		cy -= T_TextHeight(String) / 2;
	if (VAlign == vbottom)
		cy -= T_TextHeight(String);

	int length = String.Length();

	//	Need this for correct cursor position with empty strings.
	LastX = cx;
	LastY = cy;

	for (i=0; i<length; i++)
	{
		if (String[i] == '\n')
		{
			VStr cs(String, start, i - start);
			T_DrawString(cx, cy, cs);
			cy += T_StringHeight(cs) + VDistance;
			start = i + 1;
		}
		if (i == length - 1)
		{
			T_DrawString(cx, cy, VStr(String, start, length - start));
		}
	}
	unguard;
}

//==========================================================================
//
//	T_DrawTextW
//
//==========================================================================

int T_DrawTextW(int x, int y, const VStr& String, int w)
{
	guard(T_DrawTextW);
	int			start = 0;
	int			cx;
	int			cy;
	int			i;
	bool		wordStart = true;
	int			LinesPrinted = 0;

	cx = x;
	cy = y;

	//	These won't work correctly so don't use them for now.
	if (VAlign == vcentre)
		cy -= T_TextHeight(String) / 2;
	if (VAlign == vbottom)
		cy -= T_TextHeight(String);

	//	Need this for correct cursor position with empty strings.
	LastX = cx;
	LastY = cy;

	for (i = 0; String[i]; i++)
	{
		if (String[i] == '\n')
		{
			VStr cs(String, start, i - start);
			T_DrawString(cx, cy, cs);
			cy += T_StringHeight(cs) + VDistance;
			start = i + 1;
			wordStart = true;
			LinesPrinted++;
		}
		else if (wordStart && String[i] > ' ')
		{
			int j = i;
			while (String[j] > ' ')
				j++;
			if (T_StringWidth(VStr(String, start, j - start)) > w)
			{
				VStr cs(String, start, i - start);
				T_DrawString(cx, cy, cs);
				cy += T_StringHeight(cs) + VDistance;
				start = i;
				LinesPrinted++;
			}
			wordStart = false;
		}
		else if (String[i] <= ' ')
		{
			wordStart = true;
		}
		if (!String[i + 1])
		{
			T_DrawString(cx, cy, VStr(String, start, i - start + 1));
			LinesPrinted++;
		}
	}
	return LinesPrinted;
	unguard;
}

//==========================================================================
//
//	T_DrawCursor
//
//==========================================================================

void T_DrawCursor()
{
	T_DrawCursorAt(LastX, LastY);
}

//==========================================================================
//
//	T_DrawCursorAt
//
//==========================================================================

void T_DrawCursorAt(int x, int y)
{
	guard(T_DrawCursorAt);
	if ((int)(host_time * 4) & 1)
		R_DrawPic(x, y, Font->Pics['_' - 32]);
	unguard;
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
	guard(T_DrawString8);
	int		w;
	int		c;
	int		cx;
	int		cy;
	int		i;
	int		length;

	if (!String)
		return;
		
	cx = x;
	cy = y;

	if (HAlign == hcentre)
		cx -= T_StringWidth(String) / 2;
	if (HAlign == hright)
		cx -= T_StringWidth(String);

	length = (int)String.Length();
	
	if (cx >= VirtualWidth || cy >= VirtualHeight)
	{
		GCon->Logf(NAME_Dev, "T_DrawString8: Draw text \"%s\" at (%d,%d)",
			*String, cx, cy);
		return;
	}

	for (i = 0; i < length && cx < VirtualWidth; i++)
	{
		c = String[i] - 32;

		if (c < 0)
		{
			continue;
		}
		if (c >= 96 || Font->Pics[c] < 0)
		{
			cx += 8;
			continue;
		}
		
		w = Font->PicInfo[c].width;
		R_DrawPic(cx + (8 - w) / 2, cy, Font->Pics[c]);
		cx += 8;
	}
	LastX = cx;
	LastY = cy;
	unguard;
}
