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

#define ALIAS_Z_CLIP_PLANE	5

// flags in finalvert_t.flags
#define ALIAS_LEFT_CLIP				0x0001
#define ALIAS_TOP_CLIP				0x0002
#define ALIAS_RIGHT_CLIP			0x0004
#define ALIAS_BOTTOM_CLIP			0x0008
#define ALIAS_Z_CLIP				0x0010
#define ALIAS_XY_CLIP_MASK			0x000F

// TYPES -------------------------------------------------------------------

struct miptexture_t
{
	int		width;
	int		height;
	dword	offsets[4];
};

struct espan_t
{
	int			u;
	int			v;
	int			count;
	espan_t		*pnext;
};

struct sspan_t
{
	int			u;
	int			v;
	int			count;
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

struct finalvert_t
{
	int		u;
	int		v;
	int		zi;
	int		r;
	int		g;
	int		b;
	int		flags;
	int		reserved;
};

struct finalstvert_t
{
	int			s;
	int			t;
};

struct affinetridesc_t
{
	void				*pskin;
	int					skinwidth;
	int					skinheight;
	mtriangle_t			*ptriangles;
	finalvert_t			*pfinalverts;
	finalstvert_t		*pstverts;
	int					numtriangles;
	boolean				coloredlight;
};

struct auxvert_t
{
	float	fv[3];		// viewspace x, y
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
	void BeginDirectUpdate(void);
	void EndDirectUpdate(void);
	void Shutdown(void);
	void* ReadScreen(int*, bool*);
	void FreeSurfCache(surfcache_t*);

	//	Rendering stuff
	void SetupView(const refdef_t*);
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
	void BeginSky(void);
	void DrawSkyPolygon(TVec*, int, int, float, int, float);
	void EndSky(void);
	void DrawMaskedPolygon(TVec*, int, int, int);
	void DrawSpritePolygon(TVec*, int, int, int, dword);
	void DrawAliasModel(const TVec&, const TAVec&, model_t*, int, const char*, dword, int, bool);

	//	Particles
	void StartParticles(void);
	void DrawParticle(particle_t *);
	void EndParticles(void);

	//	Drawing
	void DrawPic(float, float, float, float, float, float, float, float, int, int);
	void DrawPicShadow(float, float, float, float, float, float, float, float, int, int);
	void FillRectWithFlat(float, float, float, float, float, float, float, float, const char*);
	void FillRect(float, float, float, float, dword);
	void ShadeRect(int, int, int, int, int);
	void DrawConsoleBackground(int);
	void DrawSpriteLump(float, float, float, float, int, int, boolean);

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

void D_DrawSpans8_16(espan_t*);
void D_DrawSpans16_16(espan_t *pspan);
void D_DrawSkySpans_16(espan_t*);

void D_DrawSpans8_32(espan_t*);
void D_DrawSpans16_32(espan_t *pspan);
void D_DrawSkySpans_32(espan_t*);

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

void D_BeginEdgeFrame(void);
void D_FlushSpriteCache(void);

int D_SurfaceCacheForRes(int, int, int);
void D_InitCaches(void*, int);
void D_FlushCaches(bool);
surfcache_t *D_SCAlloc(int, int);

surfcache_t *D_CacheSurface(surface_t *surface, int miplevel);

void SetSpriteLump(int, dword, int);

extern "C" void D_AliasProjectFinalVert(finalvert_t *fv, auxvert_t *av);
void D_AliasClipTriangle(mtriangle_t *ptri);
void D_PolysetSetupDrawer(int);
void D_PolysetDraw(void);

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
extern int				d_skysmask;
extern int				d_skytmask;
extern byte*			d_transluc;
extern word*			d_srctranstab;
extern word*			d_dsttranstab;

extern int				ylookup[MAXSCREENHEIGHT];

extern spanfunc_t		spanfunc;
extern spritespanfunc_t	spritespanfunc;

extern spanfunc_t		D_DrawSpans;
extern spanfunc_t		D_DrawSkySpans;
extern spritespanfunc_t	D_DrawSpriteSpans;
extern spritespanfunc_t	D_DrawFuzzSpriteSpans;
extern spritespanfunc_t	D_DrawAltFuzzSpriteSpans;
extern particle_func_t	D_DrawParticle;

extern float			centerxfrac;
extern float			centeryfrac;
extern float			xprojection;
extern float			yprojection;
extern float			aliasxcenter;
extern float			aliasycenter;

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
extern word				scaletable[32][256];

extern byte				*scrn;
extern word				*scrn16;
extern int				usegamma;

extern short			*zbuffer;

extern word				pal8_to16[256];
extern dword			pal2rgb[256];

extern int				rshift;
extern int				gshift;
extern int				bshift;

extern int				bppindex;

extern byte				*d_rgbtable;

extern miptexture_t		*miptexture;

extern int				viewwidth;
extern int				viewheight;

extern float			vrectx_adj;
extern float			vrecty_adj;
extern float			vrectw_adj;
extern float			vrecth_adj;

extern affinetridesc_t	d_affinetridesc;
extern finalvert_t		*pfinalverts;
extern finalstvert_t	*pfinalstverts;
extern auxvert_t		*pauxverts;

//==========================================================================
//
//	Inlines for 8bpp
//
//==========================================================================

inline dword MakeCol8(byte r, byte g, byte b)
{
	return d_rgbtable[((r << 7) & 0x7c00) +
		((g << 2) & 0x3e0) + ((b >> 3) & 0x1f)];
}

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
	return byte((col >> rshift) << 3);
}

inline byte GetCol15G(dword col)
{
	return byte((col >> gshift) << 3);
}

inline byte GetCol15B(dword col)
{
	return byte((col >> bshift) << 3);
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
	return byte((col >> rshift) << 3);
}

inline byte GetCol16G(dword col)
{
	return byte((col >> gshift) << 2);
}

inline byte GetCol16B(dword col)
{
	return byte((col >> bshift) << 3);
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
	return byte(col >> rshift);
}

inline byte GetCol32G(dword col)
{
	return byte(col >> gshift);
}

inline byte GetCol32B(dword col)
{
	return byte(col >> bshift);
}

//==========================================================================
//
//	Inlines for color making
//
//==========================================================================

inline dword MakeCol(byte r, byte g, byte b)
{
	if (ScreenBPP == 8)
	{
		return MakeCol8(r, g, b);
	}
	if (ScreenBPP == 15)
	{
		return MakeCol15(r, g, b);
	}
	if (ScreenBPP == 16)
	{
		return MakeCol16(r, g, b);
	}
	if (ScreenBPP == 32)
	{
		return MakeCol32(r, g, b);
	}
	return 0;
}

inline byte GetColR(dword col)
{
	if (ScreenBPP == 15)
	{
		return GetCol15R(col);
	}
	if (ScreenBPP == 16)
	{
		return GetCol16R(col);
	}
	if (ScreenBPP == 32)
	{
		return GetCol32R(col);
	}
	return 0;
}

inline byte GetColG(dword col)
{
	if (ScreenBPP == 15)
	{
		return GetCol15G(col);
	}
	if (ScreenBPP == 16)
	{
		return GetCol16G(col);
	}
	if (ScreenBPP == 32)
	{
		return GetCol32G(col);
	}
	return 0;
}

inline byte GetColB(dword col)
{
	if (ScreenBPP == 15)
	{
		return GetCol15B(col);
	}
	if (ScreenBPP == 16)
	{
		return GetCol16B(col);
	}
	if (ScreenBPP == 32)
	{
		return GetCol32B(col);
	}
	return 0;
}

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.13  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//
//	Revision 1.12  2001/10/09 17:21:39  dj_jl
//	Added sky begining and ending functions
//	
//	Revision 1.11  2001/10/04 17:23:29  dj_jl
//	Got rid of some warnings
//	
//	Revision 1.10  2001/09/12 17:31:27  dj_jl
//	Rectangle drawing and direct update for plugins
//	
//	Revision 1.9  2001/08/23 17:47:22  dj_jl
//	Started work on pics with custom palettes
//	
//	Revision 1.8  2001/08/21 17:46:08  dj_jl
//	Added R_TextureAnimation, made SetTexture recognize flats
//	
//	Revision 1.7  2001/08/15 17:15:55  dj_jl
//	Drawer API changes, removed wipes
//	
//	Revision 1.6  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.5  2001/08/04 17:29:11  dj_jl
//	Added depth hack for weapon models
//	
//	Revision 1.4  2001/08/01 17:33:58  dj_jl
//	Fixed drawing of spite lump for player setup menu, beautification
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
