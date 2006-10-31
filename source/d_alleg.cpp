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

#include <allegro.h>
#include <allegro/internal/aintern.h>
#include "d_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VAllegroSoftwareDrawer : public VSoftwareDrawer
{
public:
	BITMAP*		gamebitmap;

	void Init();
	bool SetResolution(int, int, int, bool);
	void SetPalette8(vuint8*);
	void Update();
	void Shutdown();

	BITMAP* my_create_bitmap_ex(int, int, int);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_DRAWER(VAllegroSoftwareDrawer, DRAWER_Software, "Software",
	"Allegro software rasteriser", NULL);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VAllegroSoftwareDrawer::Init
//
//==========================================================================

void VAllegroSoftwareDrawer::Init()
{
	gamebitmap = NULL;
}

//==========================================================================
//
//	VAllegroSoftwareDrawer::my_create_bitmap_ex
//
//	Creates a new memory bitmap in the specified colour_depth
//
//==========================================================================

BITMAP* VAllegroSoftwareDrawer::my_create_bitmap_ex(int colour_depth,
	int width, int height)
{
	guard(VAllegroSoftwareDrawer::my_create_bitmap_ex);
	GFX_VTABLE* vtable = _get_vtable(colour_depth);
	if (!vtable)
		return NULL;

	BITMAP* bitmap = (BITMAP*)Z_Malloc(sizeof(BITMAP) + (sizeof(char*) * height));

	bitmap->dat = scrn;

	bitmap->w = bitmap->cr = width;
	bitmap->h = bitmap->cb = height;
	bitmap->clip = TRUE;
	bitmap->cl = bitmap->ct = 0;
	bitmap->vtable = vtable;
	bitmap->write_bank = bitmap->read_bank = (void*)_stub_bank_switch;
	bitmap->id = 0;
	bitmap->extra = NULL;
	bitmap->x_ofs = 0;
	bitmap->y_ofs = 0;
	bitmap->seg = _default_ds();

	bitmap->line[0] = (vuint8*)bitmap->dat;
	for (int i = 1; i < height; i++)
	{
		bitmap->line[i] = bitmap->line[i - 1] + width *
			BYTES_PER_PIXEL(colour_depth);
	}

	if (system_driver->created_bitmap)
		system_driver->created_bitmap(bitmap);

	return bitmap;
	unguard;
}

//==========================================================================
//
// 	VAllegroSoftwareDrawer::SetResolution
//
// 	Set up the video mode
//
//==========================================================================

bool VAllegroSoftwareDrawer::SetResolution(int AWidth, int AHeight,
	int ABPP, bool Windowed)
{
	guard(VAllegroSoftwareDrawer::SetResolution);
	int Width = InWidth;
	int Height = InHeight;
	int BPP = InBPP;
	if (!Width || !Height)
	{
		//	Set defaults
		Width = 640;
		Height = 480;
		BPP = 8;
	}

	if (gamebitmap)
	{
		Z_Free(gamebitmap);
		gamebitmap = NULL;
	}
	FreeMemory();

	set_color_depth(BPP);
	if (set_gfx_mode(Windowed ? GFX_AUTODETECT_WINDOWED : GFX_AUTODETECT,
		Width, Height, 0, 0))
	{
		GCon->Log(NAME_Init, "Failed to set video mode:");
		GCon->Log(NAME_Init, allegro_error);
		return false;
	}

	if (!AllocMemory(SCREEN_W, SCREEN_H, BPP))
	{
		return false;
	}
	gamebitmap = my_create_bitmap_ex(BPP, SCREEN_W, SCREEN_H);
	if (!gamebitmap)
	{
		GCon->Log(NAME_Init, "Failed to create game bitmap:");
		return false;
	}

	ScreenWidth = SCREEN_W;
	ScreenHeight = SCREEN_H;
	ScreenBPP = BPP;

	memset(scrn, 0, SCREEN_W * SCREEN_H * ((BPP + 7) >> 3));

	if (ScreenBPP == 15)
	{
		rshift = _rgb_r_shift_15;
		gshift = _rgb_g_shift_15;
		bshift = _rgb_b_shift_15;
	}
	else if (ScreenBPP == 16)
	{
		rshift = _rgb_r_shift_16;
		gshift = _rgb_g_shift_16;
		bshift = _rgb_b_shift_16;
	}
	else if (ScreenBPP == 32)
	{
		rshift = _rgb_r_shift_32;
		gshift = _rgb_g_shift_32;
		bshift = _rgb_b_shift_32;
	}

	return true;
	unguard;
}

//==========================================================================
//
//	VAllegroSoftwareDrawer::SetPalette8
//
//	Takes full 8 bit values.
//
//==========================================================================

void VAllegroSoftwareDrawer::SetPalette8(vuint8* palette)
{
	guard(VAllegroSoftwareDrawer::SetPalette8);
	if (ScreenBPP != 8)
		return;

	vuint8* table = gammatable[usegamma];
	vuint8* p = palette;

	PALETTE		pal;
	for (int i = 0; i < 256; i++)
	{
		pal[i].r = table[*p++] >> 2;
		pal[i].g = table[*p++] >> 2;
		pal[i].b = table[*p++] >> 2;
	}
	set_palette(pal);
	unguard;
}

//==========================================================================
//
//	VAllegroSoftwareDrawer::Update
//
// 	Blit to the screen / Flip surfaces
//
//==========================================================================

void VAllegroSoftwareDrawer::Update()
{
	guard(VAllegroSoftwareDrawer::Update);
	blit(gamebitmap, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	unguard;
}

//==========================================================================
//
// 	VAllegroSoftwareDrawer::Shutdown
//
// 	Restore text mode
//
//==========================================================================

void VAllegroSoftwareDrawer::Shutdown()
{
	set_gfx_mode(GFX_TEXT, 80, 25, 0, 0);
	FreeAllMemory();
}
