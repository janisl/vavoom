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

#include "d_local.h"

// MACROS ------------------------------------------------------------------

#define NUM_MIPS	4

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

#ifdef USEASM
extern "C"
{
void D_Surf8Start();
void D_Surf8End();
void D_Surf8Patch();
void D_Surf16Start();
void D_Surf16End();
void D_Surf16Patch();
void D_Surf32Start();
void D_Surf32End();
void D_Surf32Patch();

void D_PolysetAff8Start();
void D_PolysetAff8End();
void D_Aff8Patch();
void D_PolysetAff16Start();
void D_PolysetAff16End();
void D_Aff16Patch();
void D_PolysetAff32Start();
void D_PolysetAff32End();
void D_Aff32Patch();
}
#endif

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

#ifdef USEASM
extern "C" {
extern int				d_rowbytes;
extern int				d_zrowbytes;
}
#endif

extern bool				d_roverwrapped;
extern bool				r_cache_thrash;
extern surfcache_t*		d_initial_rover;
extern surfcache_t*		sc_rover;
extern surfcache_t*		sc_base;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

byte*					scrn;
short*					zbuffer;

word					pal8_to16[256];
dword					pal2rgb[256];

extern "C" {
int						rshift = 11;
int						gshift = 5;
int						bshift = 0;
int						roffs;
int						goffs;
int						boffs;
}

int						bppindex;

int						viewx;
int						viewy;
int						viewwidth;
int						viewheight;

float					vrectx_adj;
float					vrecty_adj;
float					vrectw_adj;
float					vrecth_adj;

float					centerxfrac;
float					centeryfrac;
float					xprojection;
float					yprojection;

int						ylookup[MAXSCREENHEIGHT];

spanfunc_t				D_DrawSpans;
spritespanfunc_t		D_DrawSpriteSpans;
spritespanfunc_t		D_DrawFuzzSpriteSpans;
spritespanfunc_t		D_DrawAltFuzzSpriteSpans;

spanfunc_t				spanfunc;
spritespanfunc_t		spritespanfunc;

float					scale_for_mip;
float					d_scalemip[NUM_MIPS-1];
int						d_minmip;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VCvarI			d_subdiv("d_subdiv", "1", CVAR_Archive);

static VCvarI			d_mipcap("d_mipcap", "0");
static VCvarF			d_mipscale("d_mipscale", "1");

static float			basemip[NUM_MIPS - 1] = {1.0, 0.5 * 0.8, 0.25 * 0.8};

static byte*			r_backscreen = NULL;
static int				viewarea;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VSoftwareDrawer::AllocMemory
//
//==========================================================================

bool VSoftwareDrawer::AllocMemory(int width, int height, int bpp)
{
	guard(VSoftwareDrawer::AllocMemory);
	scrn = (byte*)Z_Malloc(width * height * ((bpp + 7) >> 3));

	zbuffer = (short*)Z_Malloc(width * height * 2);

	int size = SurfaceCacheForRes(width, height, bpp);
	void *buffer = Z_Malloc(size);
	if (!buffer)
	{
		GCon->Log(NAME_Init, "Not enough memory for cache");
		return false;
	}
	InitCaches(buffer, size);

	return true;
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::FreeMemory
//
//==========================================================================

void VSoftwareDrawer::FreeMemory()
{
	guard(VSoftwareDrawer::FreeMemory);
	FlushCaches(true);
	FlushTextureCaches();
	if (sc_base)
	{
		Z_Free(sc_base);
		sc_base = NULL;
	}
	if (zbuffer)
	{
		Z_Free(zbuffer);
		zbuffer = NULL;
	}
	if (scrn)
	{
		Z_Free(scrn);
		scrn = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::InitResolution
//
//	Calculate image scaling
//
//==========================================================================

void VSoftwareDrawer::InitResolution()
{
	guard(VSoftwareDrawer::InitResolution);
	if (ScreenBPP == 8)
	{
		bppindex = 0;

		D_DrawSpans = D_DrawSpans8_8;
		D_DrawSpriteSpans = D_DrawSpriteSpans_8;
		D_DrawFuzzSpriteSpans = D_DrawFuzzSpriteSpans_8;
		D_DrawAltFuzzSpriteSpans = D_DrawAltFuzzSpriteSpans_8;
		D_DrawParticle = D_DrawParticle_8;
#ifdef USEASM
		Sys_MakeCodeWriteable((long)D_Surf8Start,
			(long)D_Surf8End - (long)D_Surf8Start);
		D_Surf8Patch();
		Sys_MakeCodeWriteable((long)D_PolysetAff8Start,
			(long)D_PolysetAff8End - (long)D_PolysetAff8Start);
		D_Aff8Patch();
#endif
	}
	else if (ScreenBPP == 15)
	{
		bppindex = 1;

		D_DrawSpans = D_DrawSpans8_16;
		D_DrawSpriteSpans = D_DrawSpriteSpans_16;
		D_DrawFuzzSpriteSpans = D_DrawFuzzSpriteSpans_15;
		D_DrawAltFuzzSpriteSpans = D_DrawFuzzSpriteSpans_15;
		D_DrawParticle = D_DrawParticle_15;
#ifdef USEASM
		Sys_MakeCodeWriteable((long)D_Surf16Start,
			(long)D_Surf16End - (long)D_Surf16Start);
		D_Surf16Patch();
		Sys_MakeCodeWriteable((long)D_PolysetAff16Start,
			(long)D_PolysetAff16End - (long)D_PolysetAff16Start);
		D_Aff16Patch();
#endif
	}
	else if (ScreenBPP == 16)
	{
		bppindex = 2;

		D_DrawSpans = D_DrawSpans8_16;
		D_DrawSpriteSpans = D_DrawSpriteSpans_16;
		D_DrawFuzzSpriteSpans = D_DrawFuzzSpriteSpans_16;
		D_DrawAltFuzzSpriteSpans = D_DrawFuzzSpriteSpans_16;
		D_DrawParticle = D_DrawParticle_16;
#ifdef USEASM
		Sys_MakeCodeWriteable((long)D_Surf16Start,
			(long)D_Surf16End - (long)D_Surf16Start);
		D_Surf16Patch();
		Sys_MakeCodeWriteable((long)D_PolysetAff16Start,
			(long)D_PolysetAff16End - (long)D_PolysetAff16Start);
		D_Aff16Patch();
#endif
	}
	else if (ScreenBPP == 32)
	{
		bppindex = 3;
		roffs = rshift / 8;
		goffs = gshift / 8;
		boffs = bshift / 8;

		D_DrawSpans = D_DrawSpans8_32;
		D_DrawSpriteSpans = D_DrawSpriteSpans_32;
		D_DrawFuzzSpriteSpans = D_DrawFuzzSpriteSpans_32;
		D_DrawAltFuzzSpriteSpans = D_DrawFuzzSpriteSpans_32;
		D_DrawParticle = D_DrawParticle_32;
#ifdef USEASM
		Sys_MakeCodeWriteable((long)D_Surf32Start,
			(long)D_Surf32End - (long)D_Surf32Start);
		D_Surf32Patch();
		Sys_MakeCodeWriteable((long)D_PolysetAff32Start,
			(long)D_PolysetAff32End - (long)D_PolysetAff32Start);
		D_Aff32Patch();
#endif
	}
	else
	{
		Sys_Error("Invalid BPP");
	}

#ifdef USEASM
	d_rowbytes = -ScreenWidth * PixelBytes;
	d_zrowbytes = -ScreenWidth * 2;
#endif
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::InitViewBorder
//
//	Called whenever the view size changes.
//
//==========================================================================

void VSoftwareDrawer::InitViewBorder(const refdef_t* rd)
{
	guard(VSoftwareDrawer::InitViewBorder);
    if (r_backscreen)
    {
      	Z_Free(r_backscreen);
		r_backscreen = NULL;
    }

    if (rd->width == ScreenWidth)
		return;

	r_backscreen = (byte*)Z_Malloc(ScreenWidth * (ScreenHeight - SB_REALHEIGHT) * PixelBytes);

	R_DrawViewBorder();

	memcpy(r_backscreen, scrn, ScreenWidth * (ScreenHeight - SB_REALHEIGHT) * PixelBytes);
	unguard;
} 

//==========================================================================
//
//	VSoftwareDrawer::VideoErase
//
// 	Copy a screen buffer.
//
//==========================================================================

void VSoftwareDrawer::VideoErase(unsigned ofs, int count)
{
	ofs *= PixelBytes;
	count *= PixelBytes;
    memcpy(scrn + ofs, r_backscreen + ofs, count);
} 

//==========================================================================
//
//	VSoftwareDrawer::EraseViewBorder
//
// 	Draws the border around the view for different size windows
//
//==========================================================================

void VSoftwareDrawer::EraseViewBorder(const refdef_t* rd)
{
	guard(VSoftwareDrawer::EraseViewBorder);
	int top;
	int side;
	int ofs;
	int i;

	if (rd->width == ScreenWidth)
		return;

	top = ((ScreenHeight - SB_REALHEIGHT) - rd->height) / 2;
	side = (ScreenWidth - rd->width) / 2;

	// copy top and one line of left side
	VideoErase(0, top * ScreenWidth + side);

	// copy one line of right side and bottom
	ofs = (rd->height + top - 1) * ScreenWidth + ScreenWidth - side;
	VideoErase(ofs, top * ScreenWidth + side);

	// copy sides using wraparound
	ofs = top * ScreenWidth + ScreenWidth - side;
	side = side << 1;

	for (i=1 ; i < rd->height ; i++)
	{
		VideoErase(ofs, side);
		ofs += ScreenWidth;
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::StartUpdate
//
//==========================================================================

void VSoftwareDrawer::StartUpdate()
{
}

//==========================================================================
//
//	VSoftwareDrawer::BeginDirectUpdate
//
//==========================================================================

void VSoftwareDrawer::BeginDirectUpdate()
{
}

//==========================================================================
//
//	VSoftwareDrawer::EndDirectUpdate
//
//==========================================================================

void VSoftwareDrawer::EndDirectUpdate()
{
	Update();
}

//==========================================================================
//
//	VSoftwareDrawer::SetupView
//
//==========================================================================

void VSoftwareDrawer::SetupView(const refdef_t* rd)
{
	guard(VSoftwareDrawer::SetupView);
	int i;

	viewx = rd->x;
	viewy = rd->y;
	viewwidth = rd->width;
	viewheight = rd->height;

	//	Setup projection
	xprojection = (float)(viewwidth / 2) / rd->fovx;
	yprojection = (float)(viewheight / 2) / rd->fovy;

    centerxfrac = (float)(viewwidth / 2) - 0.5;
    centeryfrac = (float)(viewheight / 2) - 0.5;

    aliasxcenter = (float)(viewwidth / 2);
    aliasycenter = (float)(viewheight / 2);

    // Preclaculate all row offsets.
    for (i = 0; i < viewheight; i++)
	{
		ylookup[i] = viewx + (viewheight - 1 - i + viewy) * ScreenWidth;
	}

	if (!rd->drawworld)
	{
		//	Since world will be not drawn we must clear z-buffer
		for (i = 0; i < viewheight; i++)
		{
			memset(zbuffer + ylookup[i], 0, viewwidth * 2);
		}

		// make FDIV fast.
		Sys_LowFPPrecision();

		return;
	}

	vrectx_adj = -0.5;
	vrecty_adj = -0.5;
	vrectw_adj = viewwidth - 0.5;
	vrecth_adj = viewheight - 0.5;

	scale_for_mip = xprojection;
	if (yprojection > xprojection)
		scale_for_mip = yprojection;

	//	Setup for particles
	xscaleshrink = (float)(viewwidth - 6) / 2 / rd->fovx;
	yscaleshrink = xscaleshrink * PixelAspect;

	d_pix_min = viewwidth / 320;
	if (d_pix_min < 1)
		d_pix_min = 1;

	d_pix_max = (int)((float)viewwidth / (320.0 / 4.0) + 0.5);
	d_pix_shift = 8 - (int)((float)viewwidth / 320.0 + 0.5);
	if (d_pix_max < 1)
		d_pix_max = 1;

	if (PixelAspect > 1.4)
		d_y_aspect_shift = 1;
	else
		d_y_aspect_shift = 0;

	d_particle_right = viewwidth - d_pix_max;
	d_particle_top = viewheight - (d_pix_max << d_y_aspect_shift);

	// draw the border
	if (rd->width * rd->height != viewarea)
	{
		InitViewBorder(rd);
		viewarea = rd->width * rd->height;
	}
	else if (rd->width != ScreenWidth)
	{
		EraseViewBorder(rd);
	}

#ifdef USEASM
	if (ScreenBPP == 8)
	{
		if (d_subdiv == 2)
			D_DrawSpans = D_DrawSpans16_8;
		else
			D_DrawSpans = D_DrawSpans8_8;
	}
	else if (PixelBytes == 2)
	{
		if (d_subdiv == 2)
			D_DrawSpans = D_DrawSpans16_16;
		else
			D_DrawSpans = D_DrawSpans8_16;
	}
	else if (PixelBytes == 4)
	{
		if (d_subdiv == 2)
			D_DrawSpans = D_DrawSpans16_32;
		else
			D_DrawSpans = D_DrawSpans8_32;
	}
#endif

	d_roverwrapped = false;
	r_cache_thrash = false;
	d_initial_rover = sc_rover;

	d_minmip = d_mipcap;
	if (d_minmip > 3)
		d_minmip = 3;
	else if (d_minmip < 0)
		d_minmip = 0;

	for (i = 0; i < (NUM_MIPS - 1); i++)
		d_scalemip[i] = basemip[i] * d_mipscale;

	// make FDIV fast. This reduces timing precision after we've been running
	// for a while, so we don't do it globally. This also sets chop mode, and
	// we do it here so that setup stuff like the refresh area calculations
	// match what's done in screen.c
	Sys_LowFPPrecision();

	UpdatePalette();
	BeginEdgeFrame();
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::EndView
//
//==========================================================================

void VSoftwareDrawer::EndView()
{
	guard(VSoftwareDrawer::EndView);
	// back to high floating-point precision
	Sys_HighFPPrecision();

	if (r_cache_thrash)
	{
		T_SetFont(font_small);
		T_SetAlign(hright, vtop);
		T_DrawText(318, 10, "RAM");
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::ReadScreen
//
//==========================================================================

void* VSoftwareDrawer::ReadScreen(int* bpp, bool* bot2top)
{
	guard(VSoftwareDrawer::ReadScreen);
	void *dst;
	if (ScreenBPP == 8)
	{
		dst = Z_Malloc(ScreenWidth * ScreenHeight * PixelBytes);
		memcpy(dst, scrn, ScreenWidth * ScreenHeight * PixelBytes);
		*bpp = ScreenBPP;
	}
	else if (PixelBytes == 2)
	{
		dst = Z_Malloc(ScreenWidth * ScreenHeight * sizeof(rgb_t));
		word *psrc = (word*)scrn;
		rgb_t *pdst = (rgb_t*)dst;
		for (int i = 0; i < ScreenWidth * ScreenHeight; i++)
		{
			pdst->r = GetColR(*psrc);
			pdst->g = GetColG(*psrc);
			pdst->b = GetColB(*psrc);
			psrc++;
			pdst++;
		}
		*bpp = 24;
	}
	else
	{
		dst = Z_Malloc(ScreenWidth * ScreenHeight * sizeof(rgb_t));
		dword *psrc = (dword*)scrn;
		rgb_t *pdst = (rgb_t*)dst;
		for (int i = 0; i < ScreenWidth * ScreenHeight; i++)
		{
			pdst->r = GetColR(*psrc);
			pdst->g = GetColG(*psrc);
			pdst->b = GetColB(*psrc);
			psrc++;
			pdst++;
		}
		*bpp = 24;
	}
	*bot2top = false;
	return dst;
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::FreeAllMemory
//
//==========================================================================

void VSoftwareDrawer::FreeAllMemory()
{
	guard(VSoftwareDrawer::FreeAllMemory);
	FreeMemory();
	for (int i = 0; i < GTextureManager.Textures.Num(); i++)
	{
		if (GTextureManager.Textures[i]->DriverData)
		{
			Z_Free(GTextureManager.Textures[i]->DriverData);
			GTextureManager.Textures[i]->DriverData = NULL;
		}
	}

	Z_Free(colormaps);
	Z_Free(fadetable16);
	Z_Free(fadetable16r);
	Z_Free(fadetable16g);
	Z_Free(fadetable16b);
	Z_Free(fadetable32);
	Z_Free(fadetable32r);
	Z_Free(fadetable32g);
	Z_Free(fadetable32b);

	delete[] tinttables[0];
	delete[] tinttables[1];
	delete[] tinttables[2];
	delete[] tinttables[3];
	delete[] tinttables[4];

	if (consbgmap)
	{
		delete[] consbgmap;
		consbgmap = NULL;
	}
	unguard;
}
