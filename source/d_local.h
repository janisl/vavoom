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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************
//**
//**	Local header for software drawer
//**
//**************************************************************************

#ifndef _D_LOCAL_H
#define _D_LOCAL_H

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "cl_local.h"
#include "r_shared.h"

// MACROS ------------------------------------------------------------------

#define DS_SPAN_LIST_END	-128

#define PARTICLE_Z_CLIP		8.0

#define	CACHE_SIZE			32

// TYPES -------------------------------------------------------------------

struct miptexture_t
{
	int		width;
	int		height;
	dword	offsets[4];
};

struct sspan_t
{
	int			u;
	int			v;
	int			count;
};

struct espan_t
{
	int			u;
	int			v;
	int			count;
	espan_t		*pnext;
};

struct surfcache_t
{
	surfcache_t	*next;
	surfcache_t	**owner;	// NULL is an empty chunk of memory
	int			lightlevel;	// checked for strobe flush
	int			dlight;
	int			size;		// including header
	unsigned	width;
	unsigned	height;		// DEBUG only needed for debug
	float		mipscale;
	int			texture;	// checked for animating textures
	byte		data[4];	// width*height elements
};

typedef void (*spanfunc_t)(espan_t*);
typedef void (*spritespanfunc_t)(sspan_t*);
typedef void (*particle_func_t)(particle_t *pparticle);

typedef void (*picspanfunc_t)(int, int, fixed_t, fixed_t, fixed_t, int, byte*);

class TSoftwareDrawer : public TDrawer
{
 public:
	TSoftwareDrawer(void);
	void Init(void);
	void InitData(void);
	bool SetResolution(int, int, int);
	void InitResolution(void);
	void NewMap(void);
	void SetPalette(int);
	void SetPalette8(byte*);
	void StartUpdate(void);
	void Update(void);
	void Shutdown(void);
	void* ReadScreen(int*, bool*);
	void FreeSurfCache(surfcache_t*);

	//	Screen wipes
	bool InitWipe(void);
	void DoWipe(int);

	//	Rendering stuff
	void SetupView(int, int, int, int, float, float);
	void SetupFrame(void);
	void WorldDrawing(void);
	void EndView(void);

	//	Texture stuff
	void InitTextures(void);
	void SetTexture(int);
	void SetSkyTexture(int, bool);
	void SetFlat(int);
	void SetSkin(const char*);

	//	Polygon drawing
	void DrawPolygon(TVec*, int, int, int);
	void DrawSkyPolygon(TVec*, int, int, float, int, float);
	void DrawMaskedPolygon(TVec*, int, int, int);
	void DrawSpritePolygon(TVec*, int, int, int, dword);
	void DrawAliasModel(const TVec&, const TAVec&, model_t*, int, int, dword, int);

	//	Particles
	void StartParticles(void);
	void DrawParticle(particle_t *);
	void EndParticles(void);

	//	Drawing
	void DrawPic(float, float, float, float, float, float, float, float, int, int);
	void DrawPicShadow(float, float, float, float, float, float, float, float, int, int);
	void FillRectWithFlat(int, int, int, int, const char*);
	void ShadeRect(int, int, int, int, int);
	void DrawConsoleBackground(int);
	void DrawSpriteLump(int, int, int, int, boolean);

	//	Automap
	void StartAutomap(void);
	void PutDot(int, int, dword);
	void DrawLine(int, int, dword, int, int, dword);
	void EndAutomap(void);

 private:
	void UpdatePalette(void);

	bool AllocMemory(int, int, int);
	void FreeMemory(void);
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

extern "C" {

void D_DrawSpans8_8(espan_t*);
void D_DrawSpans16_8(espan_t *pspan);
void D_DrawSkySpans_8(espan_t*);
void D_DrawDoubleSkySpans_8(espan_t*);

void D_DrawSpans8_16(espan_t*);
void D_DrawSpans16_16(espan_t *pspan);
void D_DrawSkySpans_16(espan_t*);
void D_DrawDoubleSkySpans_16(espan_t*);

void D_DrawSpans8_32(espan_t*);
void D_DrawSpans16_32(espan_t *pspan);
void D_DrawSkySpans_32(espan_t*);
void D_DrawDoubleSkySpans_32(espan_t*);

void D_DrawZSpans(espan_t*);

void D_DrawSpriteSpans_8(sspan_t*);
void D_DrawFuzzSpriteSpans_8(sspan_t*);
void D_DrawAltFuzzSpriteSpans_8(sspan_t*);
void D_DrawSpriteSpans_16(sspan_t*);
void D_DrawFuzzSpriteSpans_15(sspan_t*);
void D_DrawFuzzSpriteSpans_16(sspan_t*);
void D_DrawSpriteSpans_32(sspan_t*);
void D_DrawFuzzSpriteSpans_32(sspan_t*);

void D_DrawParticle_8(particle_t *pparticle);
void D_DrawParticle_15(particle_t *pparticle);
void D_DrawParticle_16(particle_t *pparticle);
void D_DrawParticle_32(particle_t *pparticle);

} // extern "C"

void TransformVector(const TVec &in, TVec &out);

void D_ClearPolys(void);
void D_FlushSpriteCache(void);
int D_TextureAnimation(int);

int D_SurfaceCacheForRes(int, int, int);
void D_InitCaches(void*, int);
void D_FlushCaches(bool);
surfcache_t *D_SCAlloc(int, int);

surfcache_t *D_CacheSurface(surface_t *surface, int miplevel);

void SetSpriteLump(int, dword, int);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern float			d_zistepu;
extern float			d_zistepv;
extern float			d_ziorigin;
extern float			d_sdivzstepu;
extern float			d_tdivzstepu;
extern float			d_sdivzstepv;
extern float			d_tdivzstepv;
extern float			d_sdivzorigin;
extern float			d_tdivzorigin;
extern fixed_t			sadjust;
extern fixed_t			tadjust;
extern fixed_t			bbextents;
extern fixed_t			bbextentt;
extern void*			cacheblock;
extern int				cachewidth;
extern byte*			ds_transluc;
extern int				ds_transluc16;

extern int				ylookup[MAXSCREENHEIGHT];

extern spanfunc_t		spanfunc;
extern spritespanfunc_t	spritespanfunc;

extern spanfunc_t		D_DrawSpans;
extern spanfunc_t		D_DrawSkySpans;
extern spanfunc_t		D_DrawDoubleSkySpans;
extern spritespanfunc_t	D_DrawSpriteSpans;
extern spritespanfunc_t	D_DrawFuzzSpriteSpans;
extern spritespanfunc_t	D_DrawAltFuzzSpriteSpans;
extern particle_func_t	D_DrawParticle;

extern float			centerxfrac;
extern float			centeryfrac;
extern float			xprojection;
extern float			yprojection;

extern float			xscaleshrink;
extern float			yscaleshrink;

extern int				d_pix_shift;
extern int				d_pix_min;
extern int				d_pix_max;
extern int				d_y_aspect_shift;

extern int				d_particle_right;
extern int				d_particle_top;

extern byte				*colormaps;
extern byte				*fadetable;
extern word				*fadetable16;
extern word				*fadetable16r;
extern word				*fadetable16g;
extern word				*fadetable16b;
extern dword			*fadetable32;
extern byte				*fadetable32r;
extern byte				*fadetable32g;
extern byte				*fadetable32b;

extern byte				*tinttables[5];

extern byte				*scrn;
extern word				*scrn16;
extern int				usegamma;

extern short			*zbuffer;

extern word				pal8_to16[256];
extern dword			pal2rgb[256];

extern int				rshift;
extern int				gshift;
extern int				bshift;

extern int				black_color;

extern byte				*d_rgbtable;

extern miptexture_t		*miptexture;

//==========================================================================
//
//	Inlines for 15bpp
//
//==========================================================================

inline dword MakeCol15(byte r, byte g, byte b)
{
	return	((r >> 3) << rshift) |
			((g >> 3) << gshift) |
			((b >> 3) << bshift);
}

inline byte GetCol15R(dword col)
{
	return ((col >> rshift) << 3) & 0xff;
}

inline byte GetCol15G(dword col)
{
	return ((col >> gshift) << 3) & 0xff;
}

inline byte GetCol15B(dword col)
{
	return ((col >> bshift) << 3) & 0xff;
}

//==========================================================================
//
//	Inlines for 16bpp
//
//==========================================================================

inline dword MakeCol16(byte r, byte g, byte b)
{
	return	((r >> 3) << rshift) |
			((g >> 2) << gshift) |
			((b >> 3) << bshift);
}

inline byte GetCol16R(dword col)
{
	return ((col >> rshift) << 3) & 0xff;
}

inline byte GetCol16G(dword col)
{
	return ((col >> gshift) << 2) & 0xff;
}

inline byte GetCol16B(dword col)
{
	return ((col >> bshift) << 3) & 0xff;
}

//==========================================================================
//
//	Inlines for 32bpp
//
//==========================================================================

inline dword MakeCol32(byte r, byte g, byte b)
{
	return	(r << rshift) |
			(g << gshift) |
			(b << bshift);
}

inline byte GetCol32R(dword col)
{
	return (col >> rshift) & 0xff;
}

inline byte GetCol32G(dword col)
{
	return (col >> gshift) & 0xff;
}

inline byte GetCol32B(dword col)
{
	return (col >> bshift) & 0xff;
}

//==========================================================================
//
//	Inlines for color making
//
//==========================================================================

inline dword MakeCol(byte r, byte g, byte b)
{
	switch (ScreenBPP)
	{
	 case 15:
		return MakeCol15(r, g, b);
	 case 16:
		return MakeCol16(r, g, b);
	 case 32:
		return MakeCol32(r, g, b);
	 default:
		return 0;
	}
}

inline byte GetColR(dword col)
{
	switch (ScreenBPP)
	{
	 case 15:
		return GetCol15R(col);
	 case 16:
		return GetCol16R(col);
	 case 32:
		return GetCol32R(col);
	 default:
		return 0;
	}
}

inline byte GetColG(dword col)
{
	switch (ScreenBPP)
	{
	 case 15:
		return GetCol15G(col);
	 case 16:
		return GetCol16G(col);
	 case 32:
		return GetCol32G(col);
	 default:
		return 0;
	}
}

inline byte GetColB(dword col)
{
	switch (ScreenBPP)
	{
	 case 15:
		return GetCol15B(col);
	 case 16:
		return GetCol16B(col);
	 case 32:
		return GetCol32B(col);
	 default:
		return 0;
	}
}

#endif

