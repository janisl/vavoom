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

//==========================================================================
//
//	Fixed point, 32bit as 16.16.
//
//==========================================================================

#define FRACBITS		16
#define FRACUNIT		(1<<FRACBITS)

typedef int fixed_t;

#define FL(x)	((float)(x) / (float)FRACUNIT)
#define FX(x)	(fixed_t)((x) * FRACUNIT)

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
	vuint32	offsets[4];
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
	vuint32		Light;		// checked for strobe flash
	int			dlight;
	int			size;		// including header
	unsigned	width;
	unsigned	height;		// DEBUG only needed for debug
	float		mipscale;
	int			texture;	// checked for animating textures
	vuint8		data[4];	// width*height elements
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
	void*				pskin;
	int					skinwidth;
	int					skinheight;
	mtriangle_t*		ptriangles;
	finalvert_t*		pfinalverts;
	finalstvert_t*		pstverts;
	int					numtriangles;
	int					colouredlight;
};

struct auxvert_t
{
	float	fv[3];		// viewspace x, y
};

typedef void (*spanfunc_t)(espan_t*);
typedef void (*spritespanfunc_t)(sspan_t*);
typedef void (*particle_func_t)(particle_t *pparticle);

typedef void (*picspanfunc_t)(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);

class VSoftwareDrawer : public VDrawer
{
public:
	void InitData();
	void InitResolution();
	void NewMap();
	void StartUpdate();
	void BeginDirectUpdate();
	void EndDirectUpdate();
	void* ReadScreen(int*, bool*);
	void FreeSurfCache(surfcache_t*);

	//	Rendering stuff
	void SetupView(const refdef_t*);
	void WorldDrawing();
	void EndView();

	//	Texture stuff
	void InitTextures();
	void SetTexture(int);

	//	Polygon drawing
	void DrawPolygon(surface_t*, int);
	void DrawSkyPortal(surface_t*, int);
	void BeginSky();
	void DrawSkyPolygon(surface_t*, bool, int, float, int, float);
	void EndSky();
	void DrawMaskedPolygon(surface_t*, float);
	void DrawSpritePolygon(TVec*, int, float, int, vuint32, const TVec&,
		float, const TVec&, const TVec&, const TVec&);
	void DrawAliasModel(const TVec&, const TAVec&, const TVec&, const TVec&,
		mmdl_t*, int, int, vuint32, float, bool);

	//	Particles
	void StartParticles();
	void DrawParticle(particle_t *);
	void EndParticles();

	//	Drawing
	void DrawPic(float, float, float, float, float, float, float, float, int, float);
	void DrawPicShadow(float, float, float, float, float, float, float, float, int, float);
	void FillRectWithFlat(float, float, float, float, float, float, float, float, const char*);
	void FillRect(float, float, float, float, vuint32);
	void ShadeRect(int, int, int, int, float);
	void DrawConsoleBackground(int);
	void DrawSpriteLump(float, float, float, float, int, int, bool);

	//	Automap
	void StartAutomap();
	void DrawLine(int, int, vuint32, int, int, vuint32);
	void EndAutomap();

protected:
	//	Main.
	bool AllocMemory(int, int, int);
	void FreeMemory();
	void FreeAllMemory();
	void InitViewBorder(const refdef_t *rd);
	void VideoErase(unsigned ofs, int count);
	void EraseViewBorder(const refdef_t *rd);

	//	Palette and colour lookup table management.
	virtual void SetPalette8(vuint8*) = 0;
	void UpdatePalette();

	//	Textures.
	void FlushTextureCaches();
	void SetSpriteLump(int, vuint32, int);
	vuint8* SetPic(int);
	void GenerateTexture(int);
	static void	MakeMips(miptexture_t*);
	void LoadSkyMap(int);
	void GenerateSprite(int, int, vuint32, int);
	void GeneratePic(int);

	//	Edge drawing.
	void BeginEdgeFrame();
	void DrawSurfaces();

	//	Surface cache memory management.
	int SurfaceCacheForRes(int, int, int);
	void CheckCacheGuard();
	void ClearCacheGuard();
	void InitCaches(void*, int);
	void FlushCaches(bool);
	surfcache_t *SCAlloc(int, int);
	void SCDump(FOutputDevice&);
	void SCInvalidateTexture(int);

	//	Surface caching.
	surfcache_t* CacheSurface(surface_t*, int);
	surfcache_t* CacheSkySurface(surface_t*, int, int, float, float);

	//	Sprite drawing.
	static void SpriteClipEdge(const TVec&, const TVec&, TClipPlane*, int);
	static void SpriteScanLeftEdge(TVec*, int);
	static void SpriteScanRightEdge(TVec*, int);
	void SpriteCaclulateGradients(int, const TVec&, float, const TVec&,
		const TVec&, const TVec&);
	void MaskedSurfCaclulateGradients(surface_t*);
	void SpriteDrawPolygon(TVec*, int, surface_t*, int, int, float, vuint32,
		const TVec&, float, const TVec&, const TVec&, const TVec&);

	//	Drawing of the aliased models, i.e. md2
	bool AliasCheckBBox(mmdl_t*, const TAVec&, const TVec&, const TVec&, int);
	void AliasSetUpTransform(const TAVec&, const TVec&, const TVec&, int, int);
	void AliasSetupSkin(int);
	void AliasSetupLighting(vuint32);
	void AliasSetupFrame(int);
	void AliasPrepareUnclippedPoints();
	void AliasPreparePoints();
	void AliasClipTriangle(mtriangle_t*);
	void PolysetSetupDrawer(float);
	void PolysetDraw();

	//	Drawing of onscreen graphics.
	static void DrawPicSpan_8(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void DrawPicSpanFuzz_8(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void DrawPicSpanAltFuzz_8(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void DrawPicSpanShadow_8(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void DrawFlatSpan_8(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void FillRect_8(float, float, float, float, vuint32);
	static void ShadeRect_8(int, int, int, int, float);
	static void DrawConsoleBackground_8(int);
	static void PutDot_8(int, int, vuint32);
	static void DrawPicSpan_16(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void DrawSpritePicSpan_16(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void DrawPicSpanFuzz_16(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void DrawPicSpanShadow_16(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void DrawFlatSpan_16(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void FillRect_16(float, float, float, float, vuint32);
	static void ShadeRect_16(int, int, int, int, float);
	static void DrawConsoleBackground_16(int);
	static void PutDot_16(int, int, vuint32);
	static void DrawPicSpan_32(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void DrawSpritePicSpan_32(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void DrawPicSpanFuzz_32(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void DrawPicSpanShadow_32(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void DrawFlatSpan_32(fixed_t, fixed_t, fixed_t, int, vuint8*, void*);
	static void FillRect_32(float, float, float, float, vuint32);
	static void ShadeRect_32(int, int, int, int, float);
	static void DrawConsoleBackground_32(int);
	static void PutDot_32(int, int, vuint32);
	static void DrawPic(float, float, float, float, float, float, float, float);
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

//	Functions implemented in asm, if enabled.
extern "C" {

//	Span drawers.
void D_DrawSpans8_8(espan_t*);
void D_DrawSpans16_8(espan_t*);
void D_DrawSpans8_16(espan_t*);
void D_DrawSpans16_16(espan_t*);
void D_DrawSpans8_32(espan_t*);
void D_DrawSpans16_32(espan_t*);
void D_DrawZSpans(espan_t*);

//	Sprite drawers.
void D_DrawSpriteSpans_8(sspan_t*);
void D_DrawFuzzSpriteSpans_8(sspan_t*);
void D_DrawAltFuzzSpriteSpans_8(sspan_t*);
void D_DrawSpriteSpans_16(sspan_t*);
void D_DrawFuzzSpriteSpans_15(sspan_t*);
void D_DrawFuzzSpriteSpans_16(sspan_t*);
void D_DrawSpriteSpans_32(sspan_t*);
void D_DrawFuzzSpriteSpans_32(sspan_t*);

//	Particle drawers.
void D_DrawParticle_8(particle_t*);
void D_DrawParticle_15(particle_t*);
void D_DrawParticle_16(particle_t*);
void D_DrawParticle_32(particle_t*);

//	For models.
void D_AliasProjectFinalVert(finalvert_t*, auxvert_t*);

} // extern "C"

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//	Variables used by asm code.
extern "C" {

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
extern vuint8*			d_transluc;
extern vuint16*			d_srctranstab;
extern vuint16*			d_dsttranstab;

extern int				ylookup[MAXSCREENHEIGHT];

extern float			centrexfrac;
extern float			centreyfrac;
extern float			xprojection;
extern float			yprojection;
extern float			aliasxcentre;
extern float			aliasycentre;

extern int				d_pix_shift;
extern int				d_pix_min;
extern int				d_pix_max;
extern int				d_y_aspect_shift;

extern int				d_particle_right;
extern int				d_particle_top;

extern vuint8			d_fadetable[32 * 256];
extern vuint16			d_fadetable16[32 * 256];
extern vuint16			d_fadetable16r[32 * 256];
extern vuint16			d_fadetable16g[32 * 256];
extern vuint16			d_fadetable16b[32 * 256];
extern vuint32			d_fadetable32[32 * 256];
extern vuint8			d_fadetable32r[32 * 256];
extern vuint8			d_fadetable32g[32 * 256];
extern vuint8			d_fadetable32b[32 * 256];

extern vuint8*			consbgmap;

extern vuint8			*scrn;
extern short			*zbuffer;

extern int				rshift;
extern int				gshift;
extern int				bshift;

extern int				viewwidth;
extern int				viewheight;

extern float			vrectx_adj;
extern float			vrecty_adj;
extern float			vrectw_adj;
extern float			vrecth_adj;

extern affinetridesc_t	d_affinetridesc;

} // extern "C"

extern spanfunc_t		spanfunc;
extern spritespanfunc_t	spritespanfunc;

extern spanfunc_t		D_DrawSpans;
extern spritespanfunc_t	D_DrawSpriteSpans;
extern spritespanfunc_t	D_DrawFuzzSpriteSpans;
extern spritespanfunc_t	D_DrawAltFuzzSpriteSpans;
extern particle_func_t	D_DrawParticle;

extern float			xscaleshrink;
extern float			yscaleshrink;

extern vuint8*			colourmaps;

extern vuint8*			tinttables[5];
extern vuint16			scaletable[32][256];

extern vuint16			pal8_to16[256];
extern vuint32			pal2rgb[256];

extern int				bppindex;

extern miptexture_t*	miptexture;

extern finalvert_t*		pfinalverts;
extern finalstvert_t*	pfinalstverts;
extern auxvert_t*		pauxverts;

//==========================================================================
//
//	General inlines
//
//==========================================================================

inline void TransformVector(const TVec &in, TVec &out)
{
	out.x = DotProduct(in, viewright);
	out.y = DotProduct(in, viewup);
	out.z = DotProduct(in, viewforward);
}

//==========================================================================
//
//	Inlines for 8bpp
//
//==========================================================================

inline vuint32 MakeCol8(vuint8 r, vuint8 g, vuint8 b)
{
	return r_rgbtable[((r << 7) & 0x7c00) +
		((g << 2) & 0x3e0) + ((b >> 3) & 0x1f)];
}

//==========================================================================
//
//	Inlines for 15bpp
//
//==========================================================================

inline vuint32 MakeCol15(vuint8 r, vuint8 g, vuint8 b)
{
	return	((r >> 3) << rshift) |
			((g >> 3) << gshift) |
			((b >> 3) << bshift);
}

inline vuint8 GetCol15R(vuint32 col)
{
	return vuint8((col >> rshift) << 3);
}

inline vuint8 GetCol15G(vuint32 col)
{
	return vuint8((col >> gshift) << 3);
}

inline vuint8 GetCol15B(vuint32 col)
{
	return vuint8((col >> bshift) << 3);
}

//==========================================================================
//
//	Inlines for 16bpp
//
//==========================================================================

inline vuint32 MakeCol16(vuint8 r, vuint8 g, vuint8 b)
{
	return	((r >> 3) << rshift) |
			((g >> 2) << gshift) |
			((b >> 3) << bshift);
}

inline vuint8 GetCol16R(vuint32 col)
{
	return vuint8((col >> rshift) << 3);
}

inline vuint8 GetCol16G(vuint32 col)
{
	return vuint8((col >> gshift) << 2);
}

inline vuint8 GetCol16B(vuint32 col)
{
	return vuint8((col >> bshift) << 3);
}

//==========================================================================
//
//	Inlines for 32bpp
//
//==========================================================================

inline vuint32 MakeCol32(vuint8 r, vuint8 g, vuint8 b)
{
	return	(r << rshift) |
			(g << gshift) |
			(b << bshift);
}

inline vuint8 GetCol32R(vuint32 col)
{
	return vuint8(col >> rshift);
}

inline vuint8 GetCol32G(vuint32 col)
{
	return vuint8(col >> gshift);
}

inline vuint8 GetCol32B(vuint32 col)
{
	return vuint8(col >> bshift);
}

//==========================================================================
//
//	Inlines for colour making
//
//==========================================================================

inline vuint32 MakeCol(vuint8 r, vuint8 g, vuint8 b)
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

inline vuint8 GetColR(vuint32 col)
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

inline vuint8 GetColG(vuint32 col)
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

inline vuint8 GetColB(vuint32 col)
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
