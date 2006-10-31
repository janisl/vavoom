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

#include "winlocal.h"
#include <ddraw.h>
#include "d_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VWin32SoftwareDrawer : public VSoftwareDrawer
{
public:
	LPDIRECTDRAW			DDraw;
	LPDIRECTDRAWSURFACE		PrimarySurface;
	LPDIRECTDRAWPALETTE		Palette;
	PALETTEENTRY			PaletteEntries[256];

	bool					Windowed;
	bool					new_palette;

	void Init();
	bool SetResolution(int, int, int, bool);
	void SetPalette8(vuint8*);
	void Update();
	void Shutdown();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_DRAWER(VWin32SoftwareDrawer, DRAWER_Software, "Software",
	"DirectDraw software rasteriser", NULL);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VWin32SoftwareDrawer::Init
//
//	Determine the hardware configuration
//
//==========================================================================

void VWin32SoftwareDrawer::Init()
{
	guard(VWin32SoftwareDrawer::Init);
	DDraw = NULL;
	PrimarySurface = NULL;
	Palette = NULL;
	Windowed = false;
	new_palette = false;
	unguard;
}

//==========================================================================
//
//	VWin32SoftwareDrawer::SetResolution
//
//	Set up the video mode
//
//==========================================================================

bool VWin32SoftwareDrawer::SetResolution(int AWidth, int AHeight, int ABPP,
	bool AWindowed)
{
	guard(VWin32SoftwareDrawer::SetResolution);
	int Width = AWidth;
	int Height = AHeight;
	int BPP = ABPP;
	Windowed = AWindowed;
	if (!Width || !Height)
	{
		//	Set default mode for Windows
		Width = 640;
		Height = 480;
		BPP = 8;
	}

	DDSURFACEDESC ddsd;

	if (Palette)
	{
		Palette->Release();
		Palette = NULL;
	}
	if (PrimarySurface)
	{
		PrimarySurface->Release();
		PrimarySurface = NULL;
	}
	if (DDraw)
	{
		DDraw->Release();
		DDraw = NULL;
	}
	FreeMemory();

	if (BPP == 15)
		BPP = 16;
	if (BPP != 8 && BPP != 16 && BPP != 32)
		return false;

	HRESULT			result;

	// Create DirectDraw object
	result = CoCreateInstance(CLSID_DirectDraw, NULL,
		CLSCTX_ALL, IID_IDirectDraw, (void**)&DDraw);
	if (result != DD_OK)
	{
		Sys_Error("Failed to create DirecDraw7");
	}

	// Initialise
	result = DDraw->Initialize(NULL);
	if (result != DD_OK)
	{
		Sys_Error("Failed to initialise DirectDraw");
	}

	// Set cooperative level
	result = DDraw->SetCooperativeLevel(hwnd,
		Windowed ? DDSCL_NORMAL :
		DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX);
	if (result != DD_OK)
	{
		Sys_Error("Failed to set cooperative level.");
	}

	if (Windowed)
	{
		RECT WindowRect;
		WindowRect.left = 0;
		WindowRect.right = Width;
		WindowRect.top = 0;
		WindowRect.bottom = Height;
		AdjustWindowRectEx(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE,
			WS_EX_APPWINDOW);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, WindowRect.right - WindowRect.left,
			WindowRect.bottom - WindowRect.top, SWP_NOMOVE);
	}
	else
	{
		if (DDraw->SetDisplayMode(Width, Height, BPP) != DD_OK)
			return false;
	}

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (DDraw->CreateSurface(&ddsd, &PrimarySurface, NULL) != DD_OK)
		Sys_Error("I_SetResolution: Failed to create primary surface");

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
		else if (ddsd.ddpfPixelFormat.dwRBitMask == 0x00ff0000 &&
			ddsd.ddpfPixelFormat.dwGBitMask == 0x0000ff00 &&
			ddsd.ddpfPixelFormat.dwBBitMask == 0x000000ff)
		{
			BPP = 32;
			rshift = 16;
			gshift = 8;
			bshift = 0;
		}
		else if (ddsd.ddpfPixelFormat.dwRBitMask == 0x000000ff &&
			ddsd.ddpfPixelFormat.dwGBitMask == 0x0000ff00 &&
			ddsd.ddpfPixelFormat.dwBBitMask == 0x00ff0000)
		{
			BPP = 32;
			rshift = 0;
			gshift = 8;
			bshift = 16;
		}
		else
		{
			GCon->Log(NAME_Init, "Pixel format not supported");
			return false;
		}
	}

	// Create palette
	if (DDraw->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256,
		PaletteEntries, &Palette, NULL) != DD_OK)
	{
		Sys_Error("Failed to create palette");
	}

	if (BPP == 8)
	{
		if (PrimarySurface->SetPalette(Palette) != DD_OK)
			Sys_Error("Failed to set palette");
	}

	if (!AllocMemory(Width, Height, BPP))
	{
		return false;
	}

	ScreenWidth = Width;
	ScreenHeight = Height;
	ScreenBPP = BPP;

	return true;
	unguard;
}

//==========================================================================
//
//	VWin32SoftwareDrawer::SetPalette8
//
//	Sets palette.
//
//==========================================================================

void VWin32SoftwareDrawer::SetPalette8(vuint8* palette)
{
	guard(VWin32SoftwareDrawer::SetPalette8);
	vuint8* table = gammatable[usegamma];
	vuint8* p = palette;
	for (int i = 0; i < 256; i++)
	{
		PaletteEntries[i].peRed = table[*p++];
		PaletteEntries[i].peGreen = table[*p++];
		PaletteEntries[i].peBlue = table[*p++];
		PaletteEntries[i].peFlags = 0;
	}
	new_palette = true;
	unguard;
}

//==========================================================================
//
//	VWin32SoftwareDrawer::Update
//
// 	Blit to the screen / Flip surfaces
//
//==========================================================================

void VWin32SoftwareDrawer::Update()
{
	guard(VWin32SoftwareDrawer::Update);
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
	if (Windowed)
	{
		POINT Point = {0, 0};
		ClientToScreen(hwnd, &Point);
		RECT ScreenRect = {Point.x, Point.y, Point.x + ScreenWidth, Point.y + ScreenHeight};
		if (PrimarySurface->Lock(&ScreenRect, &ddsd, DDLOCK_WAIT, NULL) != DD_OK)
			Sys_Error("V_Update: Failed to lock screen");
	}
	else
	{
		if (PrimarySurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL) != DD_OK)
			Sys_Error("V_Update: Failed to lock screen");
	}

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
	unguard;
}

//==========================================================================
//
//	VWin32SoftwareDrawer::Shutdown
//
//	Close the graphics
//
//==========================================================================

void VWin32SoftwareDrawer::Shutdown()
{
	if (DDraw)
	{
		if (PrimarySurface)
		{
			PrimarySurface->Release();
		}
		PrimarySurface = NULL;

		if (Palette)
		{
			Palette->Release();
		}
		Palette = NULL;

		DDraw->Release();
	}
	DDraw = NULL;
	FreeAllMemory();
}
