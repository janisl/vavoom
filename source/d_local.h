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
	dword		Light;		// checked for strobe flash
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

typedef void (*picspanfunc_t)(fixed_t, fixed_t, fixed_t, int, byte*, void*);

class VSoftwareDrawer : public VDrawer
{
	DECLARE_CLASS(VSoftwareDrawer, VDrawer, 0)
	NO_DEFAULT_CONSTRUCTOR(VSoftwareDrawer)

	void Init(void);
	void InitData(void);
	bool SetResolution(int, int, int);
	void InitResolution(void);
	void NewMap(void);
	void SetPalette(int);
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
	void DrawLine(int, int, dword, int, int, dword);
	void EndAutomap(void);

private:
	//	Main.
	bool AllocMemory(int, int, int);
	void FreeMemory(void);
	void InitViewBorder(const refdef_t *rd);
	void VideoErase(unsigned ofs, int count);
	void EraseViewBorder(const refdef_t *rd);

	//	Palette and color lookup table management.
	void SetPalette8(byte*);
	void UpdatePalette(void);

	//	Textures.
	void FlushTextureCaches(void);
	static void	MakeMips(miptexture_t *mip);
	static void DrawColumnInCache(column_t* column, byte* cache,
		int originx, int originy, int cachewidth, int cacheheight, bool dsky);
	void GenerateTexture(int texnum, bool double_sky);
	void LoadSkyMap(const char *name, void **dataptr);
	void GenerateFlat(int num);
	void GenerateSprite(int lump, int slot, dword light, int translation);
	void SetSpriteLump(int, dword, int);
	void LoadImage(const char *name, void **dataptr);
	void* SetSkin(const char *name);
	void GeneratePicFromPatch(int handle);
	void GeneratePicFromRaw(int handle);
	byte* SetPic(int handle);

	//	Edge drawing.
	void BeginEdgeFrame(void);
	void DrawSurfaces(void);

	//	Surface cache memory management.
	int SurfaceCacheForRes(int, int, int);
	void CheckCacheGuard(void);
	void ClearCacheGuard(void);
	void InitCaches(void*, int);
	void FlushCaches(bool);
	surfcache_t *SCAlloc(int, int);
	void SCDump(FOutputDevice& Ar);

	//	Surface caching.
	surfcache_t *CacheSurface(surface_t *surface, int miplevel);
	surfcache_t *CacheSkySurface(surface_t *surface, int texture1,
		int texture2, float offs1, float offs2);

	//	Sprite drawing.
	static void SpriteClipEdge(const TVec &v0, const TVec &v1,
		TClipPlane *clip, int clipflags);
	static void SpriteScanLeftEdge(TVec *vb, int count);
	static void SpriteScanRightEdge(TVec *vb, int count);
	void SpriteCaclulateGradients(int lump);
	void MaskedSurfCaclulateGradients(surface_t *surf);
	void SpriteDrawPolygon(TVec *cv, int count, surface_t *surf, int lump,
		int translation, int translucency, dword light);

	//	Drawing of the aliased models, i.e. md2
	bool AliasCheckBBox(model_t *model, const TAVec &angles, int frame);
	void AliasSetUpTransform(const TAVec &angles, int frame, int trivial_accept);
	void AliasSetupSkin(const char *skin);
	void AliasSetupLighting(dword light);
	void AliasSetupFrame(int frame);
	void AliasPrepareUnclippedPoints(void);
	void AliasPreparePoints(void);
	void AliasClipTriangle(mtriangle_t *ptri);
	void PolysetSetupDrawer(int);
	void PolysetDraw(void);

	//	Drawing of onscreen graphics.
	static void DrawPicSpan_8(fixed_t s, fixed_t t, 
		fixed_t sstep, int count, byte *src, void* dst);
	static void DrawPicSpanFuzz_8(fixed_t s, fixed_t t, 
		fixed_t sstep, int count, byte *src, void* dst);
	static void DrawPicSpanAltFuzz_8(fixed_t s, fixed_t t, 
		fixed_t sstep, int count, byte *src, void* dst);
	static void DrawPicSpanShadow_8(fixed_t s, fixed_t t, 
		fixed_t sstep, int count, byte *src, void* dst);
	static void DrawFlatSpan_8(fixed_t s, fixed_t t, 
		fixed_t sstep, int count, byte *src, void* dst);
	static void FillRect_8(float x1, float y1, float x2, float y2, dword color);
	static void ShadeRect_8(int xx, int yy, int ww, int hh, int darkening);
	static void DrawConsoleBackground_8(int h);
	static void PutDot_8(int x, int y, dword c);
	static void DrawPicSpan_16(fixed_t s, fixed_t t,
		fixed_t sstep, int count, byte *src, void* dst);
	static void DrawSpritePicSpan_16(fixed_t s, fixed_t t,
		fixed_t sstep, int count, byte *_src, void* dst);
	static void DrawPicSpanFuzz_16(fixed_t s, fixed_t t,
		fixed_t sstep, int count, byte *src, void* dst);
	static void DrawPicSpanShadow_16(fixed_t s, fixed_t t,
		fixed_t sstep, int count, byte *src, void* dst);
	static void DrawFlatSpan_16(fixed_t s, fixed_t t,
		fixed_t sstep, int count, byte *src, void* dst);
	static void FillRect_16(float x1, float y1, float x2, float y2, dword color);
	static void ShadeRect_16(int xx, int yy, int ww, int hh, int darkening);
	static void DrawConsoleBackground_16(int h);
	static void PutDot_16(int x, int y, dword c);
	static void DrawPicSpan_32(fixed_t s, fixed_t t,
		fixed_t sstep, int count, byte *src, void* dst);
	static void DrawSpritePicSpan_32(fixed_t s, fixed_t t,
		fixed_t sstep, int count, byte *_src, void* dst);
	static void DrawPicSpanFuzz_32(fixed_t s, fixed_t t,
		fixed_t sstep, int count, byte *src, void* dst);
	static void DrawPicSpanShadow_32(fixed_t s, fixed_t t,
		fixed_t sstep, int count, byte *src, void* dst);
	static void DrawFlatSpan_32(fixed_t s, fixed_t t,
		fixed_t sstep, int count, byte *src, void* dst);
	static void FillRect_32(float x1, float y1, float x2, float y2, dword color);
	static void ShadeRect_32(int xx, int yy, int ww, int hh, int darkening);
	static void DrawConsoleBackground_32(int h);
	static void PutDot_32(int x, int y, dword c);
	static void DrawPic(float x1, float y1, float x2, float y2,
		float s1, float t1, float s2, float t2);
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

//	Functions implemented in asm, if enabled.
extern "C" {

//	Span drawers.
void D_DrawSpans8_8(espan_t*);
void D_DrawSpans16_8(espan_t *pspan);
void D_DrawSpans8_16(espan_t*);
void D_DrawSpans16_16(espan_t *pspan);
void D_DrawSpans8_32(espan_t*);
void D_DrawSpans16_32(espan_t *pspan);
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
void D_DrawParticle_8(particle_t *pparticle);
void D_DrawParticle_15(particle_t *pparticle);
void D_DrawParticle_16(particle_t *pparticle);
void D_DrawParticle_32(particle_t *pparticle);

//	For models.
void D_AliasProjectFinalVert(finalvert_t *fv, auxvert_t *av);

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
extern byte*			d_transluc;
extern word*			d_srctranstab;
extern word*			d_dsttranstab;

extern int				ylookup[MAXSCREENHEIGHT];

extern float			centerxfrac;
extern float			centeryfrac;
extern float			xprojection;
extern float			yprojection;
extern float			aliasxcenter;
extern float			aliasycenter;

extern int				d_pix_shift;
extern int				d_pix_min;
extern int				d_pix_max;
extern int				d_y_aspect_shift;

extern int				d_particle_right;
extern int				d_particle_top;

extern byte				*fadetable;
extern word				*fadetable16;
extern word				*fadetable16r;
extern word				*fadetable16g;
extern word				*fadetable16b;
extern dword			*fadetable32;
extern byte				*fadetable32r;
extern byte				*fadetable32g;
extern byte				*fadetable32b;

extern byte				*scrn;
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

extern byte				*d_rgbtable;

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

extern byte				*colormaps;

extern byte				*tinttables[5];
extern word				scaletable[32][256];

extern word				pal8_to16[256];
extern dword			pal2rgb[256];

extern int				bppindex;

extern miptexture_t		*miptexture;

extern finalvert_t		*pfinalverts;
extern finalstvert_t	*pfinalstverts;
extern auxvert_t		*pauxverts;

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
//	Revision 1.23  2003/03/08 12:08:04  dj_jl
//	Beautification.
//
//	Revision 1.22  2002/11/16 17:11:15  dj_jl
//	Improving software driver class.
//	
//	Revision 1.21  2002/08/28 16:39:19  dj_jl
//	Implemented sector light color.
//	
//	Revision 1.20  2002/07/13 07:38:00  dj_jl
//	Added drawers to the object tree.
//	
//	Revision 1.19  2002/01/15 18:30:43  dj_jl
//	Some fixes and improvements suggested by Malcolm Nixon
//	
//	Revision 1.18  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.17  2001/12/18 19:01:34  dj_jl
//	Changes for MSVC asm
//	
//	Revision 1.16  2001/11/02 18:35:54  dj_jl
//	Sky optimizations
//	
//	Revision 1.15  2001/10/27 07:47:52  dj_jl
//	Public gamma variables
//	
//	Revision 1.14  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
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
