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

#include <SDL/SDL.h>
#include "d_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static SDL_Surface	*hw_screen = NULL;
static SDL_Palette	hw_palette = { 256, NULL };
static bool			new_palette = false;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VSoftwareDrawer::Init
//
// 	Determine the hardware configuration
//
//==========================================================================

void VSoftwareDrawer::Init(void)
{
}

//==========================================================================
//
// 	VSoftwareDrawer::SetResolution
//
// 	Set up the video mode
//
//==========================================================================

bool VSoftwareDrawer::SetResolution(int Width, int Height, int BPP)
{
	guard(VSoftwareDrawer::SetResolution);
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
			free(hw_palette.colors);
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
	flags |= SDL_HWSURFACE | SDL_FULLSCREEN;
	hw_screen = SDL_SetVideoMode(Width, Height, BPP, flags);
	if (hw_screen == NULL)
		return false;

/*	if (hw_screen->flags != flags || hw_screen->pixels == NULL)
	{
		return false;
	}*/

	if (BPP == 8)
	{
		hw_palette.colors = (SDL_Color *)malloc(sizeof(SDL_Color) * hw_palette.ncolors);
		if (hw_palette.colors == NULL)
		{
			return false;
		}
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
//	VSoftwareDrawer::SetPalette8
//
//	Sets palette.
//
//==========================================================================

void VSoftwareDrawer::SetPalette8(byte *palette)
{
	guard(VSoftwareDrawer::SetPalette8);
	int i;

	for (i = 0; i < hw_palette.ncolors; i++)
	{
		hw_palette.colors[i].r      = gammatable[usegamma][*palette++];
		hw_palette.colors[i].g      = gammatable[usegamma][*palette++];
		hw_palette.colors[i].b      = gammatable[usegamma][*palette++];
		hw_palette.colors[i].unused = 0;
	}
	new_palette = true;
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::Update
//
// 	Blit to the screen / Flip surfaces
//
//==========================================================================

void VSoftwareDrawer::Update(void)
{
	guard(VSoftwareDrawer::Update);
	int i;
	int scrnpitch;
	byte *psrc = NULL;
	byte *pdst = NULL;

	if (SDL_MUSTLOCK(hw_screen))
	{
		while (SDL_LockSurface(hw_screen));
	}

	if (new_palette)
		SDL_SetColors(hw_screen, hw_palette.colors, 0, hw_palette.ncolors);

	scrnpitch = ScreenWidth * PixelBytes;
	psrc = (byte *)scrn;
	pdst = (byte *)hw_screen->pixels;
	for (i = 0; i < ScreenHeight; i++)
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
// 	VSoftwareDrawer::Shutdown
//
//	Close the graphics
//
//==========================================================================

void VSoftwareDrawer::Shutdown(void)
{
	if (hw_screen != NULL)
	{
		SDL_FreeSurface(hw_screen);
		hw_screen = NULL;
		if (hw_palette.colors != NULL)
		{
			free(hw_palette.colors);
			hw_palette.colors = NULL;
		}
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2002/07/13 07:38:00  dj_jl
//	Added drawers to the object tree.
//
//	Revision 1.2  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2002/01/03 18:39:42  dj_jl
//	Added SDL port
//	
//**************************************************************************
