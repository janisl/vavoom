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
//**
//**	Local header for Direct3D drawer
//**
//**************************************************************************

#ifndef _D3D_LOCAL_H
#define _D3D_LOCAL_H

// HEADER FILES ------------------------------------------------------------

#define D3D_OVERLOADS
#include "winlocal.h"
#include <d3d.h>
#include "gamedefs.h"
#include "cl_local.h"
#include "r_shared.h"

// MACROS ------------------------------------------------------------------

//#define BUMP_TEST

#define SAFE_RELEASE(iface) \
	if (iface) \
	{ \
		iface->Release(); \
		iface = NULL; \
	}

#define MAX_TRANSLATED_SPRITES		256
#define MAX_SKIN_CACHE				256

#define BLOCK_WIDTH					128
#define BLOCK_HEIGHT				128
#define NUM_BLOCK_SURFS				16
#define NUM_CACHE_BLOCKS			4000

// TYPES -------------------------------------------------------------------

struct surfcache_t
{
	int			s;			// position in light surface
	int			t;
	int			width;		// size
	int			height;
	surfcache_t	*bprev;		// line list in block
	surfcache_t	*bnext;
	surfcache_t	*lprev;		// cache list in line
	surfcache_t	*lnext;
	surfcache_t	*chain;		// list of drawable surfaces
	int			blocknum;	// light surface index
	surfcache_t	**owner;
	int			lightlevel;	// checked for strobe flush
	int			dlight;
	surface_t	*surf;
};

class TDirect3DDrawer : public TDrawer
{
 public:
	TDirect3DDrawer(void);
	void Init(void);
	void InitData(void);
	bool SetResolution(int, int, int);
	void InitResolution(void);
	void NewMap(void);
	void SetPalette(int);
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
	void SetSpriteLump(int, int);

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
	void DrawLine(int, int, dword, int, int, dword);
	void EndAutomap(void);

 private:
	void Setup2D(void);
	void ReleaseTextures(void);
	int ToPowerOf2(int);
	LPDIRECTDRAWSURFACE7 CreateSurface(int, int, int);
	word *LockSurface(LPDIRECTDRAWSURFACE7);
	void DrawColumnInCache(column_t*, word*, int, int, int, int);
	void GenerateTexture(int);
	void GenerateFlat(int);
	void GenerateSprite(int);
	void GenerateTranslatedSprite(int, int, int);
	void SetPic(int);
	void GeneratePicFromPatch(int);
	void GeneratePicFromRaw(int);
	void SetSkin(const char*);

	void FlushCaches(bool);
	surfcache_t	*AllocBlock(int width, int height);
	void CacheSurface(surface_t *surface);

	word MakeCol16(byte r, byte g, byte b)
	{
		return amask |
			((r >> (8 - rbits)) << rshift) |
			((g >> (8 - gbits)) << gshift) |
			((b >> (8 - bbits)) << bshift);
	}
	dword MakeCol32(byte r, byte g, byte b, byte a)
	{
		return (a << ashift32) | (r << rshift32) |
			(g << gshift32) | (b << bshift32);
	}

	//	DirectDraw interfaces
	LPDIRECTDRAW7				DDraw;
	LPDIRECTDRAWSURFACE7		PrimarySurface;
	LPDIRECTDRAWSURFACE7		RenderSurface;
	LPDIRECTDRAWSURFACE7		ZBuffer;

	//	Direct3D interfaces
	LPDIRECT3D7					Direct3D;
	LPDIRECT3DDEVICE7			RenderDevice;

	D3DMATRIX					IdentityMatrix;

	D3DVIEWPORT7				viewData;
	D3DMATRIX					matProj;
	D3DMATRIX					matView;

	DDPIXELFORMAT				PixelFormat;
	DDPIXELFORMAT				PixelFormat32;
	dword						SurfaceMemFlag;
	bool						square_textures;
	int							maxTexSize;
	int							maxMultiTex;

	int							amask;
	int							rbits;
	int							rshift;
	int							gbits;
	int							gshift;
	int							bbits;
	int							bshift;

	int							ashift32;
	int							rshift32;
	int							gshift32;
	int							bshift32;

	word						pal8_to16[256];

	float						tex_iw;
	float						tex_ih;

	LPDIRECTDRAWSURFACE7		*texturesurfaces;
	int							numsurfaces;
	int							tscount;

	LPDIRECTDRAWSURFACE7		*texturedata;
	float						*textureiw;
	float						*textureih;
	LPDIRECTDRAWSURFACE7		*flatdata;
	LPDIRECTDRAWSURFACE7		*spritedata;
	float						*spriteiw;
	float						*spriteih;
	LPDIRECTDRAWSURFACE7		*trsprdata;
	int							trsprlump[MAX_TRANSLATED_SPRITES];
	int							trsprtnum[MAX_TRANSLATED_SPRITES];
	float						trspriw[MAX_TRANSLATED_SPRITES];
	float						trsprih[MAX_TRANSLATED_SPRITES];
	LPDIRECTDRAWSURFACE7		*skin_data;
	char						skin_name[MAX_SKIN_CACHE][64];
	LPDIRECTDRAWSURFACE7		particle_texture;

	LPDIRECTDRAWSURFACE7		*picdata;
	float						piciw[MAX_PICS];
	float						picih[MAX_PICS];

	LPDIRECTDRAWSURFACE7		*light_surf;
	word						light_block[NUM_BLOCK_SURFS][BLOCK_WIDTH * BLOCK_HEIGHT];
	bool						block_changed[NUM_BLOCK_SURFS];
	surfcache_t					*light_chain[NUM_BLOCK_SURFS];
	surfcache_t					*freeblocks;
	surfcache_t					*cacheblocks[NUM_BLOCK_SURFS];
	surfcache_t					blockbuf[NUM_CACHE_BLOCKS];

	TCvarI						device;
	TCvarI						clear;
	TCvarI						tex_linear;
	TCvarI						dither;

#ifdef BUMP_TEST
	LPDIRECTDRAWSURFACE7		bumpTexture;
#endif
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif

