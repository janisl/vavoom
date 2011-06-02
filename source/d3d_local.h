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
#include <d3d9.h>
#include "cl_local.h"
#include "r_shared.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

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
	bool SetResolution(int, int, int, bool);
	void InitResolution();
	void StartUpdate();
	void Update();
	void BeginDirectUpdate();
	void EndDirectUpdate();
	void Shutdown();
	void* ReadScreen(int*, bool*);
	void ReadBackScreen(int, int, rgba_t*);

	//	Rendering stuff
	void SetupView(VRenderLevelDrawer*, const refdef_t*);
	void SetupViewOrg();
	void WorldDrawing();
	void EndView();

	//	Texture stuff
	void PrecacheTexture(VTexture*);

	//	Polygon drawing
	void DrawSkyPolygon(surface_t*, bool, VTexture*, float, VTexture*, float,
		int);
	void DrawMaskedPolygon(surface_t*, float, bool);
	void DrawSpritePolygon(TVec*, VTexture*, float, bool, VTextureTranslation*,
		int, vuint32, vuint32, const TVec&, float, const TVec&, const TVec&,
		const TVec&);
	void DrawAliasModel(const TVec&, const TAVec&, const TVec&, const TVec&,
		VMeshModel*, int, int, VTexture*, VTextureTranslation*, int, vuint32,
		vuint32, float, bool, bool, float, bool);
	bool StartPortal(VPortal*, bool);
	void EndPortal(VPortal*, bool);

	//	Particles
	void StartParticles();
	void DrawParticle(particle_t *);
	void EndParticles();

	//	Drawing
	void DrawPic(float, float, float, float, float, float, float, float,
		VTexture*, VTextureTranslation*, float);
	void DrawPicShadow(float, float, float, float, float, float, float,
		float, VTexture*, float);
	void FillRectWithFlat(float, float, float, float, float, float, float,
		float, VTexture*);
	void FillRect(float, float, float, float, vuint32);
	void ShadeRect(int, int, int, int, float);
	void DrawConsoleBackground(int);
	void DrawSpriteLump(float, float, float, float, VTexture*,
		VTextureTranslation*, bool);

	//	Automap
	void StartAutomap();
	void DrawLine(int, int, vuint32, int, int, vuint32);
	void EndAutomap();

	//	Advanced drawing.
	bool SupportsAdvancedRendering();
	void DrawWorldAmbientPass();
	void BeginShadowVolumesPass();
	void BeginLightShadowVolumes();
	void RenderSurfaceShadowVolume(surface_t*, TVec&, float);
	void BeginLightPass(TVec&, float, vuint32);
	void DrawSurfaceLight(surface_t*, TVec&, float);
	void DrawWorldTexturesPass();
	void DrawWorldFogPass();
	void EndFogPass();
	void DrawAliasModelAmbient(const TVec&, const TAVec&, const TVec&,
		const TVec&, VMeshModel*, int, int, VTexture*, vuint32, float, float, bool);
	void DrawAliasModelTextures(const TVec&, const TAVec&, const TVec&,
		const TVec&, VMeshModel*, int, int, VTexture*, VTextureTranslation*, int,
		float, float, bool);
	void BeginModelsLightPass(TVec&, float, vuint32);
	void DrawAliasModelLight(const TVec&, const TAVec&, const TVec&,
		const TVec&, VMeshModel*, int, int, VTexture*, float, bool);
	void BeginModelsShadowsPass(TVec&, float);
	void DrawAliasModelShadow(const TVec&, const TAVec&, const TVec&,
		const TVec&, VMeshModel*, int, int, float, bool, const TVec&, float);
	void DrawAliasModelFog(const TVec&, const TAVec&, const TVec&,
		const TVec&, VMeshModel*, int, int, VTexture*, vuint32, float, float, bool);

private:
	bool Reset();
	void Setup2D();
	void FlushTextures();
	void ReleaseTextures();
	void FlushTexture(VTexture*);
	LPDIRECT3DTEXTURE9 CreateSurface(int, int, int, bool);
	void SetTexture(VTexture*, int);
	void SetSpriteLump(VTexture*, VTextureTranslation*, int);
	void SetPic(VTexture*, VTextureTranslation*, int);
	void GenerateTexture(VTexture*, void**, VTextureTranslation*, int);
	void UploadTextureImage(LPDIRECT3DTEXTURE9, int, int, int, const rgba_t*);
	LPDIRECT3DTEXTURE9 UploadTexture8(int, int, const vuint8*, const rgba_t*);
	LPDIRECT3DTEXTURE9 UploadTexture(int, int, const rgba_t*);

	void SetFade(vuint32 NewFade);

	void DoHorizonPolygon(surface_t*);
	void DrawPortalArea(VPortal*);

	vuint16 MakeCol16(vuint8 r, vuint8 g, vuint8 b, vuint8 a)
	{
		return vuint16(((a & 0x80) << 8) |
			((r & 0xf8) << 7) |
			((g & 0xf8) << 2) |
			(b >> 3));
	}
	vuint32 MakeCol32(vuint8 r, vuint8 g, vuint8 b, vuint8 a)
	{
		return (a << 24) | (r << 16) | (g << 8) | b;
	}

	bool						Windowed;

	HMODULE						DLLHandle;

	//	Direct3D interfaces
	LPDIRECT3D9					Direct3D;
	LPDIRECT3DDEVICE9			RenderDevice;


	D3DVIEWPORT9				viewData;
	MyD3DMatrix					IdentityMatrix;
	MyD3DMatrix					matProj;
	MyD3DMatrix					matView;
	vuint32						SurfaceMemFlag;
	bool						square_textures;
	int							maxTexSize;
	int							TexStage;

	float						tex_iw;
	float						tex_ih;

	int							lastgamma;
	vuint32						CurrentFade;

	//	Texture filters.
	D3DTEXTUREFILTERTYPE		magfilter;
	D3DTEXTUREFILTERTYPE		minfilter;
	D3DTEXTUREFILTERTYPE		mipfilter;

	//	Textures.
	LPDIRECT3DTEXTURE9			particle_texture;
    int                         tscount;

	//	Lightmaps.
	LPDIRECT3DTEXTURE9			light_surf[NUM_BLOCK_SURFS];

	//	Specular lightmaps.
	LPDIRECT3DTEXTURE9			add_surf[NUM_BLOCK_SURFS];

	IDirect3DSurface9			*DXBlockSurface[2];
	BYTE dblock;

	static VCvarI device;
	static VCvarI clear;
	static VCvarI tex_linear;
	static VCvarI dither;
	static VCvarI blend_sprites;
	static VCvarF maxdist;
	static VCvarI model_lighting;
	static VCvarI specular_highlights;
	static VCvarI avoid_input_lag;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif
