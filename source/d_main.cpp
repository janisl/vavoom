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

#include "d_local.h"

// MACROS ------------------------------------------------------------------

#define NUM_MIPS	4

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

#ifdef USEASM
extern "C"
{
void D_Surf8Start(void);
void D_Surf8End(void);
void D_Surf8Patch(void);
void D_Surf16Start(void);
void D_Surf16End(void);
void D_Surf16Patch(void);
void D_Surf32Start(void);
void D_Surf32End(void);
void D_Surf32Patch(void);

void D_PolysetAff8Start(void);
void D_PolysetAff8End(void);
void D_Aff8Patch(void);
void D_PolysetAff16Start(void);
void D_PolysetAff16End(void);
void D_Aff16Patch(void);
void D_PolysetAff32Start(void);
void D_PolysetAff32End(void);
void D_Aff32Patch(void);
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
extern surfcache_t		*d_initial_rover;
extern surfcache_t		*sc_rover;
extern surfcache_t		*sc_base;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

byte					*scrn;
word					*scrn16;

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

short					*zbuffer = NULL;

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

static TSoftwareDrawer	SoftwareDrawer;

static TCvarI			d_subdiv("d_subdiv", "1", CVAR_ARCHIVE);

static TCvarI			d_mipcap("d_mipcap", "0");
static TCvarF			d_mipscale("d_mipscale", "1");

static float			basemip[NUM_MIPS - 1] = {1.0, 0.5 * 0.8, 0.25 * 0.8};

static byte*			r_backscreen = NULL;
static int				viewarea;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TSoftwareDrawer::TSoftwareDrawer
//
//==========================================================================

TSoftwareDrawer::TSoftwareDrawer(void)
{
	_SoftwareDrawer = this;
}

//==========================================================================
//
//	TSoftwareDrawer::AllocMemory
//
//==========================================================================

bool TSoftwareDrawer::AllocMemory(int width, int height, int bpp)
{
	scrn = (byte*)Z_Malloc(width * height * ((bpp + 7) >> 3), PU_VIDEO, 0);
	if (!scrn)
	{
		con << "Not enough memory for screen buffer\n";
		return false;
	}

	zbuffer = (short*)Z_Malloc(width * height * 2, PU_VIDEO, 0);
	if (!zbuffer)
	{
		con << "Not enough memory for Z-buffer\n";
		return false;
	}

	int size = D_SurfaceCacheForRes(width, height, bpp);
	void *buffer = Z_Malloc(size, PU_VIDEO, 0);
	if (!buffer)
	{
		con << "Not enough memory for cache\n";
		return false;
	}
	D_InitCaches(buffer, size);

	return true;
}

//==========================================================================
//
//	TSoftwareDrawer::FreeMemory
//
//==========================================================================

void TSoftwareDrawer::FreeMemory(void)
{
	D_FlushCaches(true);
	D_FlushTextureCaches();
//FIXME use Z_FreeTag(PU_VIDEO)
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
}

//==========================================================================
//
//	TSoftwareDrawer::InitResolution
//
//	Calculate image scaling
//
//==========================================================================

void TSoftwareDrawer::InitResolution(void)
{
	scrn16 = (word*)scrn;

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
}

//==========================================================================
//
//	InitViewBorder
//
//	Called whenever the view size changes.
//
//==========================================================================

static void InitViewBorder(const refdef_t *rd)
{ 
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
} 

//==========================================================================
//
//	VideoErase
//
// 	Copy a screen buffer.
//
//==========================================================================

static void VideoErase(unsigned ofs, int count)
{
	ofs *= PixelBytes;
	count *= PixelBytes;
    memcpy(scrn + ofs, r_backscreen + ofs, count);
} 

//==========================================================================
//
//	EraseViewBorder
//
// 	Draws the border around the view for different size windows
//
//==========================================================================

static void EraseViewBorder(const refdef_t *rd)
{ 
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
} 

//==========================================================================
//
//	TSoftwareDrawer::StartUpdate
//
//==========================================================================

void TSoftwareDrawer::StartUpdate(void)
{
}

//==========================================================================
//
//	TSoftwareDrawer::BeginDirectUpdate
//
//==========================================================================

void TSoftwareDrawer::BeginDirectUpdate(void)
{
}

//==========================================================================
//
//	TSoftwareDrawer::EndDirectUpdate
//
//==========================================================================

void TSoftwareDrawer::EndDirectUpdate(void)
{
	Update();
}

//==========================================================================
//
//	TSoftwareDrawer::SetupView
//
//==========================================================================

void TSoftwareDrawer::SetupView(const refdef_t *rd)
{
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
	D_BeginEdgeFrame();
}

//==========================================================================
//
//	TSoftwareDrawer::EndView
//
//==========================================================================

void TSoftwareDrawer::EndView(void)
{
	// back to high floating-point precision
	Sys_HighFPPrecision();

	if (r_cache_thrash)
	{
		T_SetFont(font_small);
		T_SetAlign(hright, vtop);
		T_DrawText(318, 10, "RAM");
	}
}

//==========================================================================
//
//	TSoftwareDrawer::ReadScreen
//
//==========================================================================

void *TSoftwareDrawer::ReadScreen(int *bpp, bool *bot2top)
{
	void *dst;
	if (ScreenBPP == 8)
	{
		dst = Z_Malloc(ScreenWidth * ScreenHeight * PixelBytes, PU_VIDEO, 0);
		if (!dst)
		{
			return NULL;
		}
		memcpy(dst, scrn, ScreenWidth * ScreenHeight * PixelBytes);
		*bpp = ScreenBPP;
	}
	else if (PixelBytes == 2)
	{
		dst = Z_Malloc(ScreenWidth * ScreenHeight * sizeof(rgb_t), PU_VIDEO, 0);
		if (!dst)
		{
			return NULL;
		}
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
		dst = Z_Malloc(ScreenWidth * ScreenHeight * sizeof(rgb_t), PU_VIDEO, 0);
		if (!dst)
		{
			return NULL;
		}
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
}

//**************************************************************************
//
//	$Log$
//	Revision 1.12  2001/12/18 19:01:34  dj_jl
//	Changes for MSVC asm
//
//	Revision 1.11  2001/12/01 17:52:52  dj_jl
//	no message
//	
//	Revision 1.10  2001/11/02 18:35:54  dj_jl
//	Sky optimizations
//	
//	Revision 1.9  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.8  2001/09/12 17:31:27  dj_jl
//	Rectangle drawing and direct update for plugins
//	
//	Revision 1.7  2001/08/21 17:50:55  dj_jl
//	Beautification
//	
//	Revision 1.6  2001/08/15 17:15:55  dj_jl
//	Drawer API changes, removed wipes
//	
//	Revision 1.5  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.4  2001/08/02 17:45:37  dj_jl
//	Added support for colored lit and translucent models
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
