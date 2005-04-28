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

#include "winlocal.h"
#include <ddraw.h>
#include "d_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static LPDIRECTDRAW			DDraw = NULL;
static LPDIRECTDRAWSURFACE	PrimarySurface = NULL;
static LPDIRECTDRAWPALETTE	Palette = NULL;
static PALETTEENTRY			PaletteEntries[256];

static bool					Windowed;
static bool					new_palette = false;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VSoftwareDrawer::Init
//
// 	Determine the hardware configuration
//
//==========================================================================

void VSoftwareDrawer::Init()
{
	guard(VSoftwareDrawer::Init);
	HRESULT			result;

	Windowed = !!M_CheckParm("-window");

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
		Windowed ? DDSCL_NORMAL :
		DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX);
	if (result != DD_OK)
		Sys_Error("I_InitGraphics: Failed to set cooperative level.");

	// Create palette
	result = DDraw->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256,
		PaletteEntries, &Palette, NULL);
	if (result != DD_OK)
		Sys_Error("I_InitGraphics: Failed to create palette");
	unguard;
}

//==========================================================================
//
// 	VSoftwareDrawer::SetResolution
//
// 	Set up the video mode
//
//==========================================================================

bool VSoftwareDrawer::SetResolution(int InWidth, int InHeight, int InBPP)
{
	guard(VSoftwareDrawer::SetResolution);
	int Width = InWidth;
	int Height = InHeight;
	int BPP = InBPP;
	if (!Width || !Height)
	{
		//	Set default mode for Windows
		Width = 640;
		Height = 480;
		BPP = 8;
	}

	DDSURFACEDESC ddsd;

	if (PrimarySurface)
	{
		PrimarySurface->Release();
		PrimarySurface = NULL;
	}
	FreeMemory();

	if (BPP == 15)
		BPP = 16;
	if (BPP != 8 && BPP != 16 && BPP != 32)
		return false;

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

void VSoftwareDrawer::SetPalette8(byte* palette)
{
	guard(VSoftwareDrawer::SetPalette8);
	byte* table = gammatable[usegamma];
	byte* p = palette;
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
//	VSoftwareDrawer::Update
//
// 	Blit to the screen / Flip surfaces
//
//==========================================================================

void VSoftwareDrawer::Update(void)
{
	guard(VSoftwareDrawer::Update);
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
// 	VSoftwareDrawer::Shutdown
//
//	Close the graphics
//
//==========================================================================

void VSoftwareDrawer::Shutdown(void)
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

//**************************************************************************
//
//	$Log$
//	Revision 1.11  2005/04/28 07:16:15  dj_jl
//	Fixed some warnings, other minor fixes.
//
//	Revision 1.10  2004/04/08 15:19:40  dj_jl
//	Windowed mode
//	
//	Revision 1.9  2002/07/13 07:38:00  dj_jl
//	Added drawers to the object tree.
//	
//	Revision 1.8  2002/01/21 18:24:22  dj_jl
//	Fixed 32 bit mode
//	
//	Revision 1.7  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.6  2002/01/03 18:38:25  dj_jl
//	Added guard macros and core dumps
//	
//	Revision 1.5  2001/10/27 07:47:52  dj_jl
//	Public gamma variables
//	
//	Revision 1.4  2001/10/04 17:23:29  dj_jl
//	Got rid of some warnings
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
