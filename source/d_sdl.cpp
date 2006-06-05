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
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
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

#include <SDL.h>
#include "d_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VSdlSoftwareDrawer : public VSoftwareDrawer
{
public:
	SDL_Surface*	hw_screen;
	SDL_Palette		hw_palette;
	bool			new_palette;

	void Init();
	bool SetResolution(int, int, int);
	void SetPalette8(vuint8*);
	void Update();
	void Shutdown();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_DRAWER(VSdlSoftwareDrawer, DRAWER_Software, "Software",
	"SDL software rasteriser", NULL);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VSdlSoftwareDrawer::Init
//
// 	Determine the hardware configuration
//
//==========================================================================

void VSdlSoftwareDrawer::Init()
{
	hw_screen = NULL;
	hw_palette.ncolors = 256;
	hw_palette.colors = NULL;
	new_palette = false;
}

//==========================================================================
//
// 	VSdlSoftwareDrawer::SetResolution
//
// 	Set up the video mode
//
//==========================================================================

bool VSdlSoftwareDrawer::SetResolution(int InWidth, int InHeight, int InBPP)
{
	guard(VSdlSoftwareDrawer::SetResolution);
	int Width = InWidth;
	int Height = InHeight;
	int BPP = InBPP;
	Uint32 flags = 0;

	if (!Width || !Height)
	{
		// Set default mode for Windows
		Width = 640;
		Height = 480;
		BPP = 8;
	}

	if (hw_screen != NULL)
	{
		SDL_FreeSurface(hw_screen);
		hw_screen = NULL;
		if (hw_palette.colors != NULL)
		{
			delete[] hw_palette.colors;
			hw_palette.colors = NULL;
		}
	}
	FreeMemory();

	if (BPP == 15)
		BPP = 16;
	if (BPP != 8 && BPP != 16 && BPP != 32)
		return false;

	if (BPP == 8)
	{
		flags |= SDL_HWPALETTE;
	}
	if (!GArgs.CheckParm("-window"))
	{
		flags |= SDL_FULLSCREEN;
	}
	flags |= SDL_HWSURFACE;

	hw_screen = SDL_SetVideoMode(Width, Height, BPP, flags);
	if (hw_screen == NULL)
		return false;

/*	if (hw_screen->flags != flags || hw_screen->pixels == NULL)
	{
		return false;
	}*/

	if (BPP == 8)
	{
		hw_palette.colors = new SDL_Color[hw_palette.ncolors];
	}

	if (!AllocMemory(Width, Height, BPP))
	{
		return false;
	}

	if (BPP != 8)
	{
		rshift = hw_screen->format->Rshift;
		gshift = hw_screen->format->Gshift;
		bshift = hw_screen->format->Bshift;
	}

	ScreenWidth = Width;
	ScreenHeight = Height;
	ScreenBPP = BPP;

	return true;
	unguard;
}

//==========================================================================
//
//	VSdlSoftwareDrawer::SetPalette8
//
//	Sets palette.
//
//==========================================================================

void VSdlSoftwareDrawer::SetPalette8(vuint8* palette)
{
	guard(VSdlSoftwareDrawer::SetPalette8);
	vuint8* table = gammatable[usegamma];
	vuint8* p = palette;
	for (int i = 0; i < hw_palette.ncolors; i++)
	{
		hw_palette.colors[i].r = table[*p++];
		hw_palette.colors[i].g = table[*p++];
		hw_palette.colors[i].b = table[*p++];
		hw_palette.colors[i].unused = 0;
	}
	new_palette = true;
	unguard;
}

//==========================================================================
//
//	VSdlSoftwareDrawer::Update
//
// 	Blit to the screen / Flip surfaces
//
//==========================================================================

void VSdlSoftwareDrawer::Update()
{
	guard(VSdlSoftwareDrawer::Update);
	if (SDL_MUSTLOCK(hw_screen))
	{
		while (SDL_LockSurface(hw_screen));
	}

	if (new_palette)
		SDL_SetColors(hw_screen, hw_palette.colors, 0, hw_palette.ncolors);

	int scrnpitch = ScreenWidth * PixelBytes;
	vuint8* psrc = (vuint8*)scrn;
	vuint8* pdst = (vuint8*)hw_screen->pixels;
	for (int i = 0; i < ScreenHeight; i++)
	{
		memcpy(pdst, psrc, scrnpitch);
		psrc += scrnpitch;
		pdst += hw_screen->pitch;
	}
	SDL_UpdateRect(hw_screen, 0, 0, 0, 0);

	if (SDL_MUSTLOCK(hw_screen))
	{
		SDL_UnlockSurface(hw_screen);
	}
	unguard;
}

//==========================================================================
//
//	VSdlSoftwareDrawer::Shutdown
//
//	Close the graphics
//
//==========================================================================

void VSdlSoftwareDrawer::Shutdown()
{
	if (hw_screen != NULL)
	{
		SDL_FreeSurface(hw_screen);
		hw_screen = NULL;
		if (hw_palette.colors != NULL)
		{
			delete[] hw_palette.colors;
			hw_palette.colors = NULL;
		}
	}
	FreeAllMemory();
}
