//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

//
// Fonts
//
struct font_t
{
	// Font patches
	int			Pics[96];
	picinfo_t	PicInfo[96];

    //Empty character width and empty strinh height
	int			SpaceWidth;
    int			SpaceHeight;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void T_LoadFont(font_e FontNr, char* Name, int SpaceW, int SpaceH);
static void T_LoadFont2(font_e FontNr, char* Name, int SpaceW, int SpaceH);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static font_t*			Fonts[NUMFONTTYPES];
static font_t*			Font;

//Distance between chars and strings
static int				HDistance;
static int				VDistance;

//
// Alignements
//
static halign_e			HAlign;
static valign_e			VAlign;

//
// Shadow
//
static bool				t_shadowed = false;

//
// Cursor
//
static int				LastX;
static int				LastY;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	T_Init
//
//==========================================================================

void T_Init(void)
{
    // Load fonts
	if (W_CheckNumForName("STCFN033") >= 0)
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
	T_SetDist(-1, 1);
	T_SetAlign(hleft, vtop);
}

//==========================================================================
//
//	T_LoadFont
//
//==========================================================================

static void T_LoadFont(font_e FontNr, char* Name, int SpaceW, int SpaceH)
{
	int		i;
    char   	buffer[10];

	Fonts[FontNr] = (font_t*)Z_Malloc(sizeof(font_t), PU_STATIC, 0);
	memset(Fonts[FontNr], 0, sizeof(font_t));
    Fonts[FontNr]->SpaceWidth = SpaceW;
    Fonts[FontNr]->SpaceHeight = SpaceH;
    for (i = 0; i < 96; i++)
    {
		sprintf(buffer, "%s%02d", Name, i);
		if (W_CheckNumForName(buffer) >= 0)
		{
			Fonts[FontNr]->Pics[i] = R_RegisterPic(buffer, PIC_PATCH);
			R_GetPicInfo(Fonts[FontNr]->Pics[i], &Fonts[FontNr]->PicInfo[i]);
			if ((i + 32 >= 'a') && (i + 32 <= 'z')
				&& !Fonts[FontNr]->Pics[i + 'A' - 'a'])
			{
				Fonts[FontNr]->Pics[i + 'A' - 'a'] = Fonts[FontNr]->Pics[i];
				Fonts[FontNr]->PicInfo[i + 'A' - 'a'] = Fonts[FontNr]->PicInfo[i];
			}
			if ((i + 32 >= 'A') && (i + 32 <= 'Z')
				&& !Fonts[FontNr]->Pics[i + 'a' - 'A'])
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

static void T_LoadFont2(font_e FontNr, char* Name, int SpaceW, int SpaceH)
{
	int		i;
    char   	buffer[10];

	Fonts[FontNr] = (font_t*)Z_Malloc(sizeof(font_t), PU_STATIC, 0);
	memset(Fonts[FontNr], 0, sizeof(font_t));
    Fonts[FontNr]->SpaceWidth = SpaceW;
    Fonts[FontNr]->SpaceHeight = SpaceH;
    for (i = 0; i < 64; i++)
    {
		sprintf(buffer, "%s%03d", Name, i + 32);
		if (W_CheckNumForName(buffer) >= 0)
		{
			Fonts[FontNr]->Pics[i] = R_RegisterPic(buffer, PIC_PATCH);
			R_GetPicInfo(Fonts[FontNr]->Pics[i], &Fonts[FontNr]->PicInfo[i]);
			if ((i + 32 >= 'a') && (i + 32 <= 'z')
				&& !Fonts[FontNr]->Pics[i + 'A' - 'a'])
			{
				Fonts[FontNr]->Pics[i + 'A' - 'a'] = Fonts[FontNr]->Pics[i];
				Fonts[FontNr]->PicInfo[i + 'A' - 'a'] = Fonts[FontNr]->PicInfo[i];
			}
			if ((i + 32 >= 'A') && (i + 32 <= 'Z')
				&& !Fonts[FontNr]->Pics[i + 'a' - 'A'])
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
//	T_SetDist
//
//==========================================================================

void T_SetDist(int HDist, int VDist)
{
	HDistance = HDist;
	VDistance = VDist;
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

int T_StringWidth(char* String)
{
    int             i;
    int             w = 0;
    int             c;
	
    for (i = 0; i < (int)strlen(String);i++)
    {
		c = String[i] - 32;

		if (c < 0 || c >= 96 || !Font->Pics[c])
	    	w += Font->SpaceWidth + HDistance;
		else
	    	w += Font->PicInfo[c].width + HDistance;
    }
	if (w) w -= HDistance;
		
    return w;
}

//==========================================================================
//
//	T_StringHeight
//
//==========================================================================

int T_StringHeight(char* String)
{
    int		i;
    int		c;
    int		h = Font->SpaceHeight;

    for (i = 0;i < (int)strlen(String);i++)
    {
		c = String[i] - 32;

		if (c >= 0 && c < 96 && Font->Pics[c]
            && (h < Font->PicInfo[c].height))
          		h = Font->PicInfo[c].height;
    }
		
    return h;
}

//==========================================================================
//
//	T_TextWidth
//
//==========================================================================

int T_TextWidth(char* String)
{
	char		cs[40];
    int			i;
	int			w1;
    int         w = 0;
    int         start = 0;

	for (i = 0; i <= (int)strlen(String); i++)
		if ((String[i] == '\n') || !String[i])
		{
		    memset(cs,0,40);
		    strncpy(cs, String + start, i - start);
			w1 = T_StringWidth(cs);
			if (w1 > w) w = w1;
		    start = i;
		}
	return w;
}

//==========================================================================
//
//	T_TextHeight
//
//==========================================================================

int T_TextHeight(char* String)
{
	char		cs[40];
    int			i;
    int         h = 0;
    int         start = 0;

	for (i=0; i <= (int)strlen(String); i++)
		if ((String[i] == '\n') || !String[i])
		{
		    memset(cs,0,40);
		    strncpy(cs, String + start, i - start);
			h += T_StringHeight(cs) + VDistance;
		    start = i;
		}
	h -= VDistance;
	return h;
}

//==========================================================================
//
//	T_DrawNString
//
//	Write a string using the font with maximal lenght.
//
//==========================================================================

void T_DrawNString(int x,int y,char* String,int lenght)
{
    int		w;
    int		c;
    int		cx;
    int		cy;
    int		i;
		
	if (!String)
		return;
		
    cx = x;
    cy = y;

	if (HAlign == hcenter)
    	cx -= T_StringWidth(String) / 2;
	if (HAlign == hright)
		cx -= T_StringWidth(String);

    if (lenght > (int)strlen(String))
		lenght = (int)strlen(String);
	
	if ((cx < 0) || (cy < 0) || (cx >= 320) || (cy >= 200))
	{
       	cond << "T_DrawNString: Draw text \"" << String << "\" at ("
       		<< cx << "," << cy << ")\n";
      	return;
	}

    for (i=0; i<lenght; i++)
    {
		c = String[i] - 32;

		if (c < 0)
		{
	    	continue;
		}
		if (c >= 96 || !Font->Pics[c])
		{
	    	cx += Font->SpaceWidth + HDistance;
	    	continue;
		}
		
		w = Font->PicInfo[c].width;
		if (cx + w > ScreenWidth)
	    	break;
		if (t_shadowed)
			R_DrawShadowedPic(cx, cy, Font->Pics[c]);
		else
			R_DrawPic(cx, cy, Font->Pics[c]);
		cx += w + HDistance;
    }
	LastX = cx;
    LastY = cy;
}

//==========================================================================
//
//	T_DrawString
//
//==========================================================================

void T_DrawString(int x,int y,char* String)
{
    T_DrawNString(x, y, String, strlen(String));
}

//==========================================================================
//
//	T_DrawNText
//
//==========================================================================

void T_DrawNText(int x,int y,char* String,int lenght)
{
	int			start = 0;
    int			cx;
    int			cy;
    char		cs[80];
    int			i;

    cx = x;
    cy = y;

	if (VAlign == vcenter)
		cy -= T_TextHeight(String) / 2;
	if (VAlign == vbottom)
		cy -= T_TextHeight(String);

    if (lenght > (int)strlen(String))
		lenght = (int)strlen(String);

	for (i=0; i<lenght; i++)
	{
		if (String[i] == '\n')
		{
		    memset(cs, 0, sizeof(cs));
		    strncpy(cs, String + start, i - start);
			T_DrawNString(cx, cy, cs, lenght - start);
    		cy += T_StringHeight(cs) + VDistance;
		    start = i + 1;
		}
		if (i == lenght - 1)
		{
			T_DrawNString(cx, cy, String + start, lenght - start);
		}
	}
}

//==========================================================================
//
//	T_DrawText
//
//==========================================================================

void T_DrawText(int x,int y,char* String)
{
	T_DrawNText(x, y, String, strlen(String));
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
	if ((int)(host_time * 4) & 1)
		R_DrawPic(x, y, Font->Pics['_' - 32]);
}

//==========================================================================
//
//	T_DrawString8
//
//	Write a string using the font with fixed width 8.
//
//==========================================================================

void T_DrawString8(int x, int y, char* String)
{
    int		w;
    int		c;
    int		cx;
    int		cy;
    int		i;
    int		lenght;

	if (!String)
		return;
		
    cx = x;
    cy = y;

	if (HAlign == hcenter)
    	cx -= T_StringWidth(String) / 2;
	if (HAlign == hright)
		cx -= T_StringWidth(String);

	lenght = (int)strlen(String);
	
	if ((cx >= 320) || (cy >= 200))
	{
       	cond << "T_DrawString8: Draw text \"" << String << "\" at ("
       		<< cx << "," << cy << ")\n";
      	return;
	}

    for (i = 0; i < lenght && cx < 320; i++)
    {
		c = String[i] - 32;

		if (c < 0)
		{
	    	continue;
		}
		if (c >= 96 || !Font->Pics[c])
		{
	    	cx += 8;
	    	continue;
		}
		
		w = Font->PicInfo[c].width;
		if (t_shadowed)
			R_DrawShadowedPic(cx + (8 - w) / 2, cy, Font->Pics[c]);
		else
			R_DrawPic(cx + (8 - w) / 2, cy, Font->Pics[c]);
		cx += 8;
    }
	LastX = cx;
    LastY = cy;
}


