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

#include "winlocal.h"
#include <ddraw.h>
#include "d_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern byte					gammatable[5][256];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

dword						mmx_mask4 = 0;
dword						mmx_mask8 = 0;
dword						mmx_mask16 = 0;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static LPDIRECTDRAW			DDraw = NULL;
static LPDIRECTDRAWSURFACE	PrimarySurface = NULL;
static LPDIRECTDRAWPALETTE	Palette = NULL;
static PALETTEENTRY			PaletteEntries[256];

static boolean				new_palette = false;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TSoftwareDrawer::Init
//
// 	Determine the hardware configuration
//
//==========================================================================

void TSoftwareDrawer::Init(void)
{
	HRESULT			result;

	// Create DirectDraw object
	result = CoCreateInstance(CLSID_DirectDraw, NULL,
		CLSCTX_ALL, IID_IDirectDraw, (void**)&DDraw);
	if (result != DD_OK)
		Sys_Error("I_InitGraphics: Failed to create DirecDraw7");

	// Initialize
	result = DDraw->Initialize(NULL);
	if (result != DD_OK)
		Sys_Error("I_InitGraphics: Failed to initialize DirectDraw");

	// Set cooperative level
	result = DDraw->SetCooperativeLevel(hwnd,
		DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX);
	if (result != DD_OK)
		Sys_Error("I_InitGraphics: Failed to set cooperative level.");

	// Create palette
	result = DDraw->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256,
		PaletteEntries, &Palette, NULL);
	if (result != DD_OK)
		Sys_Error("I_InitGraphics: Failed to create palette");
}

//==========================================================================
//
// 	TSoftwareDrawer::SetResolution
//
// 	Set up the video mode
//
//==========================================================================

bool TSoftwareDrawer::SetResolution(int Width, int Height, int BPP)
{
	if (!Width || !Height)
	{
		//	Set default mode for Windows
		Width = 640;
		Height = 480;
		BPP = 8;
	}

	DDSURFACEDESC	ddsd;
	DDSCAPS			ddscaps;

	if (PrimarySurface)
	{
		PrimarySurface->Release();
		PrimarySurface = NULL;
	}
	FreeMemory();

	if (BPP == 15)
		BPP = 16;
	if (BPP != 8 && BPP != 16)
		return false;

	if (DDraw->SetDisplayMode(Width, Height, BPP) != DD_OK)
		return false;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (DDraw->CreateSurface(&ddsd, &PrimarySurface, NULL) != DD_OK)
		Sys_Error("I_SetResolution: Failed to create primary surface");

	if (BPP == 8)
	{
		if (PrimarySurface->SetPalette(Palette) != DD_OK)
			Sys_Error("I_SetResolution: Failed to set palette");
	}

	if (!AllocMemory(Width, Height, BPP))
	{
		return false;
	}

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	PrimarySurface->GetSurfaceDesc(&ddsd);
	if (BPP != 8)
	{
		if (ddsd.ddpfPixelFormat.dwRBitMask == 0x7c00 &&
			ddsd.ddpfPixelFormat.dwGBitMask == 0x03e0 &&
			ddsd.ddpfPixelFormat.dwBBitMask == 0x001f)
		{
			BPP = 15;
			rshift = 10;
			gshift = 5;
			bshift = 0;
		}
		else if (ddsd.ddpfPixelFormat.dwRBitMask == 0x001f &&
			ddsd.ddpfPixelFormat.dwGBitMask == 0x07e0 &&
			ddsd.ddpfPixelFormat.dwBBitMask == 0x7c00)
		{
			BPP = 15;
			rshift = 0;
			gshift = 5;
			bshift = 10;
		}
		else if (ddsd.ddpfPixelFormat.dwRBitMask == 0xf800 &&
			ddsd.ddpfPixelFormat.dwGBitMask == 0x07e0 &&
			ddsd.ddpfPixelFormat.dwBBitMask == 0x001f)
		{
			BPP = 16;
			rshift = 11;
			gshift = 5;
			bshift = 0;
		}
		else if (ddsd.ddpfPixelFormat.dwRBitMask == 0x001f &&
			ddsd.ddpfPixelFormat.dwGBitMask == 0x07e0 &&
			ddsd.ddpfPixelFormat.dwBBitMask == 0xf800)
		{
			BPP = 16;
			rshift = 0;
			gshift = 5;
			bshift = 11;
		}
		else
		{
			con << "Pixel format not supported\n";
			return false;
		}
	}

	ScreenWidth = Width;
	ScreenHeight = Height;
	ScreenBPP = BPP;

	return true;
}

//==========================================================================
//
//	TSoftwareDrawer::SetPalette8
//
//	Sets palette.
//
//==========================================================================

void TSoftwareDrawer::SetPalette8(byte *palette)
{
  	int				i;

	for (i = 0; i < 256; i++)
	{
		PaletteEntries[i].peRed = gammatable[usegamma][*palette++];
		PaletteEntries[i].peGreen = gammatable[usegamma][*palette++];
		PaletteEntries[i].peBlue = gammatable[usegamma][*palette++];
		PaletteEntries[i].peFlags = 0;
	}
	new_palette = true;
}

//==========================================================================
//
//	TSoftwareDrawer::Update
//
// 	Blit to the screen / Flip surfaces
//
//==========================================================================

void TSoftwareDrawer::Update(void)
{
	DDSURFACEDESC	ddsd;

	// Check for lost surface
	if (PrimarySurface->IsLost() != DD_OK)
	{
		PrimarySurface->Restore();
	}

	//	If palette changed, set new palette
	if (new_palette)
	{
		if (Palette->SetEntries(0, 0, 256, PaletteEntries) != DD_OK)
			Sys_Error("I_SetPalette: Failed to det palette entries");
		new_palette = false;
	}

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
	if (PrimarySurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL) != DD_OK)
		Sys_Error("V_Update: Failed to lock screen");

	byte *psrc = (byte*)scrn;
	byte *pdst = (byte*)ddsd.lpSurface;
	int wbytes = ScreenWidth * PixelBytes;
	for (int i = 0; i < ScreenHeight; i++)
	{
		memcpy(pdst, psrc, wbytes);
		psrc += wbytes;
		pdst += ddsd.lPitch;
	}

	if (PrimarySurface->Unlock(NULL) != DD_OK)
		Sys_Error("UnlockBuf: Failed to unlock screen");
}

//==========================================================================
//
// 	TSoftwareDrawer::Shutdown
//
//	Close the graphics
//
//==========================================================================

void TSoftwareDrawer::Shutdown(void)
{
	if (DDraw)
	{
		if (PrimarySurface)
		{
			PrimarySurface->Release();
		}
		PrimarySurface = NULL;

		if (Palette)
			Palette->Release();
		Palette = NULL;

		DDraw->Release();
	}
	DDraw = NULL;
}

