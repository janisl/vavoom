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
//**	Local header for Direct3D drawer
//**
//**************************************************************************

#ifndef _D3D_LOCAL_H
#define _D3D_LOCAL_H

// HEADER FILES ------------------------------------------------------------

#include "winlocal.h"
#include "gamedefs.h"
#if 1
#define D3D_OVERLOADS
#include <d3d.h>
#else
#include <d3d8.h>
#include <dx7todx8.h>
#endif
#include "cl_local.h"
#include "r_shared.h"

// MACROS ------------------------------------------------------------------

#define MAX_TRANSLATED_SPRITES		256

#define BLOCK_WIDTH					128
#define BLOCK_HEIGHT				128
#define NUM_BLOCK_SURFS				32
#define NUM_CACHE_BLOCKS			(8 * 1024)

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
	surfcache_t	*addchain;	// list of specular surfaces
	int			blocknum;	// light surface index
	surfcache_t	**owner;
	vuint32		Light;		// checked for strobe flash
	int			dlight;
	surface_t	*surf;
	vuint32		lastframe;
};

struct MyD3DVertex
{
	float		x;			// Homogeneous coordinates
	float		y;
	float		z;
	vuint32		colour;		// Vertex colour
	float		texs;		// Texture coordinates
	float		text;
	float		lights;		// Lightmap coordinates
	float		lightt;

	MyD3DVertex() { }
	MyD3DVertex(const TVec& v, vuint32 _colour, float _s, float _t)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		colour = _colour;
		texs = _s;
		text = _t;
		lights = 0.0;
		lightt = 0.0;
	}
	MyD3DVertex(const TVec& v, vuint32 _colour, float _s, float _t,
		float _ls, float _lt)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		colour = _colour;
		texs = _s;
		text = _t;
		lights = _ls;
		lightt = _lt;
	}
	MyD3DVertex(float _x, float _y, vuint32 _colour, float _s, float _t)
	{
		x = _x;
		y = _y;
		z = 0.0;
		colour = _colour;
		texs = _s;
		text = _t;
		lights = 0.0;
		lightt = 0.0;
	}
};

#define MYD3D_VERTEX_FORMAT		(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2)

struct MyD3DMatrix : public D3DMATRIX
{
public:
	MyD3DMatrix()
	{
	}
	MyD3DMatrix(float f11, float f12, float f13, float f14,
				float f21, float f22, float f23, float f24,
				float f31, float f32, float f33, float f34,
				float f41, float f42, float f43, float f44 )
	{
		_11 = f11; _12 = f12; _13 = f13; _14 = f14;
		_21 = f21; _22 = f22; _23 = f23; _24 = f24;
		_31 = f31; _32 = f32; _33 = f33; _34 = f34;
		_41 = f41; _42 = f42; _43 = f43; _44 = f44;
	}

	// access grants
	float& operator () ( UINT iRow, UINT iCol )
	{
		return m[iRow][iCol];
	}
	float operator () ( UINT iRow, UINT iCol ) const
	{
		return m[iRow][iCol];
	}

	friend void MatrixMultiply(MyD3DMatrix &out, const MyD3DMatrix& a,
		const MyD3DMatrix& b);
	MyD3DMatrix operator * (const MyD3DMatrix& mat) const
	{
		MyD3DMatrix matT;
		MatrixMultiply(matT, *this, mat);
		return matT;
	}
};

class VDirect3DDrawer : public VDrawer
{
public:
	VDirect3DDrawer();
	void Init();
	void InitData();
	bool SetResolution(int, int, int, bool);
	void InitResolution();
	void NewMap();
	void StartUpdate();
	void Update();
	void BeginDirectUpdate();
	void EndDirectUpdate();
	void Shutdown();
	void* ReadScreen(int*, bool*);
	void FreeSurfCache(surfcache_t*);

	//	Rendering stuff
	void SetupView(const refdef_t*);
	void WorldDrawing();
	void EndView();

	//	Texture stuff
	void InitTextures();
	void SetTexture(int);
	void SetSpriteLump(int, int);

	//	Polygon drawing
	void DrawPolygon(surface_t*, int);
	void DrawSkyPortal(surface_t*, int);
	void BeginSky();
	void DrawSkyPolygon(surface_t*, bool, int, float, int, float);
	void EndSky();
	void DrawMaskedPolygon(surface_t*, int);
	void DrawSpritePolygon(TVec*, int, int, int, vuint32, const TVec&, float,
		const TVec&, const TVec&, const TVec&);
	void DrawAliasModel(const TVec&, const TAVec&, VModel*, int, int,
		const char*, vuint32, int, bool);

	//	Particles
	void StartParticles();
	void DrawParticle(particle_t *);
	void EndParticles();

	//	Drawing
	void DrawPic(float, float, float, float, float, float, float, float, int, int);
	void DrawPicShadow(float, float, float, float, float, float, float, float, int, int);
	void FillRectWithFlat(float, float, float, float, float, float, float, float, const char*);
	void FillRect(float, float, float, float, vuint32);
	void ShadeRect(int, int, int, int, int);
	void DrawConsoleBackground(int);
	void DrawSpriteLump(float, float, float, float, int, int, bool);

	//	Automap
	void StartAutomap();
	void DrawLine(int, int, vuint32, int, int, vuint32);
	void EndAutomap();

private:
	void Setup2D();
	void FlushTextures();
	void ReleaseTextures();
	static int ToPowerOf2(int);
#if DIRECT3D_VERSION >= 0x0800
	LPDIRECT3DTEXTURE8 CreateSurface(int, int, int, bool);
#else
	LPDIRECTDRAWSURFACE7 CreateSurface(int, int, int, bool);
#endif
	void SetPic(int);
	void GenerateTexture(int);
	void GenerateTranslatedSprite(int, int, int);
#if DIRECT3D_VERSION >= 0x0800
	void UploadTextureImage(LPDIRECT3DTEXTURE8, int, int, int, rgba_t*);
#else
	void UploadTextureImage(LPDIRECTDRAWSURFACE7, int, int, rgba_t*);
#endif
	void AdjustGamma(rgba_t *, int);
	void ResampleTexture(int, int, const byte*, int, int, byte*);
	void MipMap(int, int, byte*);
#if DIRECT3D_VERSION >= 0x0800
	LPDIRECT3DTEXTURE8 UploadTexture8(int, int, byte*, rgba_t*);
	LPDIRECT3DTEXTURE8 UploadTexture(int, int, rgba_t*);
#else
	LPDIRECTDRAWSURFACE7 UploadTexture8(int, int, byte*, rgba_t*);
	LPDIRECTDRAWSURFACE7 UploadTexture(int, int, rgba_t*);
#endif

	void FlushCaches(bool);
	void FlushOldCaches();
	surfcache_t	*AllocBlock(int, int);
	surfcache_t	*FreeBlock(surfcache_t*, bool);
	void CacheSurface(surface_t*);

#if DIRECT3D_VERSION < 0x0800
	static HRESULT CALLBACK EnumDevicesCallback(
		LPSTR lpDeviceDesc,
		LPSTR lpDeviceName,
		LPD3DDEVICEDESC7 lpD3DDeviceDesc,
		LPVOID);
	static HRESULT CALLBACK EnumZBufferCallback(LPDDPIXELFORMAT pf, void* dst);
	static HRESULT CALLBACK EnumPixelFormatsCallback(LPDDPIXELFORMAT pf, void* dst);
	static HRESULT CALLBACK EnumPixelFormats32Callback(LPDDPIXELFORMAT pf, void* dst);
	static void LogPrimCaps(FOutputDevice &Ar, const D3DPRIMCAPS &pc);
	static void LogDeviceDesc(FOutputDevice &Ar, const LPD3DDEVICEDESC7 dd);
	static void LogPixelFormat(FOutputDevice &Ar, const LPDDPIXELFORMAT pf);
#endif

	word MakeCol16(byte r, byte g, byte b, byte a)
	{
		return word(((a >> (8 - abits)) << ashift) |
			((r >> (8 - rbits)) << rshift) |
			((g >> (8 - gbits)) << gshift) |
			((b >> (8 - bbits)) << bshift));
	}
	vuint32 MakeCol32(byte r, byte g, byte b, byte a)
	{
		return (a << ashift32) | (r << rshift32) |
			(g << gshift32) | (b << bshift32);
	}

	bool						Windowed;

#if DIRECT3D_VERSION >= 0x0800
	HMODULE						DLLHandle;

	//	Direct3D interfaces
	LPDIRECT3D8					Direct3D;
	LPDIRECT3DDEVICE8			RenderDevice;

	D3DVIEWPORT8				viewData;
#else
	//	DirectDraw interfaces
	LPDIRECTDRAW7				DDraw;
	LPDIRECTDRAWSURFACE7		PrimarySurface;
	LPDIRECTDRAWSURFACE7		RenderSurface;
	LPDIRECTDRAWSURFACE7		ZBuffer;

	//	Direct3D interfaces
	LPDIRECT3D7					Direct3D;
	LPDIRECT3DDEVICE7			RenderDevice;

	D3DVIEWPORT7				viewData;

	DDPIXELFORMAT				PixelFormat;
	DDPIXELFORMAT				PixelFormat32;
#endif
	MyD3DMatrix					IdentityMatrix;
	MyD3DMatrix					matProj;
	MyD3DMatrix					matView;
	vuint32						SurfaceMemFlag;
	bool						square_textures;
	int							maxTexSize;
	int							maxMultiTex;
	int							TexStage;

	int							abits;
	int							ashift;
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

	float						tex_iw;
	float						tex_ih;

	int							lastgamma;

	//	Texture filters.
#if DIRECT3D_VERSION >= 0x0800
	D3DTEXTUREFILTERTYPE		magfilter;
	D3DTEXTUREFILTERTYPE		minfilter;
	D3DTEXTUREFILTERTYPE		mipfilter;
#else
	D3DTEXTUREMAGFILTER			magfilter;
	D3DTEXTUREMINFILTER			minfilter;
	D3DTEXTUREMIPFILTER			mipfilter;
#endif

	//	Textures.
#if DIRECT3D_VERSION >= 0x0800
	LPDIRECT3DTEXTURE8			*trsprdata;
	LPDIRECT3DTEXTURE8			particle_texture;
#else
	LPDIRECTDRAWSURFACE7		*trsprdata;
	LPDIRECTDRAWSURFACE7		particle_texture;
#endif
	int							trsprlump[MAX_TRANSLATED_SPRITES];
	int							trsprtnum[MAX_TRANSLATED_SPRITES];
	int							tscount;

	surface_t*					SimpleSurfsHead;
	surface_t*					SimpleSurfsTail;
	surface_t*					SkyPortalsHead;
	surface_t*					SkyPortalsTail;

	//	Lightmaps.
#if DIRECT3D_VERSION >= 0x0800
	LPDIRECT3DTEXTURE8			*light_surf;
#else
	LPDIRECTDRAWSURFACE7		*light_surf;
#endif
	rgba_t						light_block[NUM_BLOCK_SURFS][BLOCK_WIDTH * BLOCK_HEIGHT];
	bool						block_changed[NUM_BLOCK_SURFS];
	surfcache_t					*light_chain[NUM_BLOCK_SURFS];

	//	Specular lightmaps.
#if DIRECT3D_VERSION >= 0x0800
	LPDIRECT3DTEXTURE8			*add_surf;
#else
	LPDIRECTDRAWSURFACE7		*add_surf;
#endif
	rgba_t						add_block[NUM_BLOCK_SURFS][BLOCK_WIDTH * BLOCK_HEIGHT];
	bool						add_changed[NUM_BLOCK_SURFS];
	surfcache_t					*add_chain[NUM_BLOCK_SURFS];

	//	Surface cache.
	surfcache_t					*freeblocks;
	surfcache_t					*cacheblocks[NUM_BLOCK_SURFS];
	surfcache_t					blockbuf[NUM_CACHE_BLOCKS];
	vuint32						cacheframecount;

	static VCvarI device;
	static VCvarI clear;
	static VCvarI tex_linear;
	static VCvarI dither;
	static VCvarI blend_sprites;
	static VCvarF maxdist;
	static VCvarI model_lighting;
	static VCvarI specular_highlights;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif
