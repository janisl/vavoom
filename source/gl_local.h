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

#ifndef _GL_LOCAL_H
#define _GL_LOCAL_H

// HEADER FILES ------------------------------------------------------------

#ifdef _WIN32
#include "winlocal.h"
#endif
#include <GL/gl.h>

#ifndef APIENTRY
#define APIENTRY
#endif

#include "gamedefs.h"
#include "cl_local.h"
#include "r_shared.h"

// MACROS ------------------------------------------------------------------

#define MAX_TRANSLATED_SPRITES		256

#define BLOCK_WIDTH					128
#define BLOCK_HEIGHT				128
#define NUM_BLOCK_SURFS				32
#define NUM_CACHE_BLOCKS			(8 * 1024)

// TYPES -------------------------------------------------------------------

//
//	Extensions
//

// ARB_multitexture
#ifndef GL_ARB_multitexture
#define GL_TEXTURE0_ARB						0x84c0
#define GL_TEXTURE1_ARB						0x84c1
#endif

typedef void (APIENTRY*MultiTexCoord2f_t)(GLenum, GLfloat, GLfloat);
typedef void (APIENTRY*SelectTexture_t)(GLenum);

// EXT_point_parameters
#ifndef GL_EXT_point_parameters
#define GL_POINT_SIZE_MIN_EXT				0x8126
#define GL_POINT_SIZE_MAX_EXT				0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE_EXT	0x8128
#define GL_DISTANCE_ATTENUATION_EXT			0x8129
#endif

typedef void (APIENTRY*PointParameterf_t)(GLenum, GLfloat);
typedef void (APIENTRY*PointParameterfv_t)(GLenum, const GLfloat *);

// EXT_texture_filter_anisotropic
#ifndef GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT		0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT	0x84FF
#endif

// SGIS_texture_edge_clamp
#ifndef GL_SGIS_texture_edge_clamp
#define GL_CLAMP_TO_EDGE_SGIS				0x812F
#endif

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

class VOpenGLDrawer : public VDrawer
{
public:
	//
	// VDrawer interface
	//
	VOpenGLDrawer();
	void InitData();
	void InitResolution();
	void NewMap();
	void SetPalette(int);
	void StartUpdate();
	void Setup2D();
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
	void DrawPolygon(TVec*, int, int, int);
	void BeginSky();
	void DrawSkyPolygon(TVec*, int, int, float, int, float);
	void EndSky();
	void DrawMaskedPolygon(TVec*, int, int, int);
	void DrawSpritePolygon(TVec*, int, int, int, vuint32);
	void DrawAliasModel(const TVec&, const TAVec&, VModel*, int, int, const char*, vuint32, int, bool);

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

protected:
	GLint		maxTexSize;
	bool		texturesGenerated;

	GLuint		trspr_id[MAX_TRANSLATED_SPRITES];
	bool		trspr_sent[MAX_TRANSLATED_SPRITES];
	int			trspr_lump[MAX_TRANSLATED_SPRITES];
	int			trspr_tnum[MAX_TRANSLATED_SPRITES];

	GLuint		particle_texture;

	GLuint		lmap_id[NUM_BLOCK_SURFS];
	rgba_t		light_block[NUM_BLOCK_SURFS][BLOCK_WIDTH * BLOCK_HEIGHT];
	bool		block_changed[NUM_BLOCK_SURFS];
	surfcache_t	*light_chain[NUM_BLOCK_SURFS];

	GLuint		addmap_id[NUM_BLOCK_SURFS];
	rgba_t		add_block[NUM_BLOCK_SURFS][BLOCK_WIDTH * BLOCK_HEIGHT];
	bool		add_changed[NUM_BLOCK_SURFS];
	surfcache_t	*add_chain[NUM_BLOCK_SURFS];

	surfcache_t	*freeblocks;
	surfcache_t	*cacheblocks[NUM_BLOCK_SURFS];
	surfcache_t	blockbuf[NUM_CACHE_BLOCKS];
	vuint32		cacheframecount;

	float		tex_iw;
	float		tex_ih;

	int			lastgamma;

	GLenum		maxfilter;
	GLenum		minfilter;
	GLenum		mipfilter;
	GLenum		ClampToEdge;
	GLfloat		max_anisotropy;

	//
	//	Console variables
	//
	static VCvarI tex_linear;
	static VCvarI clear;
	static VCvarI blend_sprites;
	static VCvarI ext_multitexture;
	static VCvarI ext_point_parameters;
	static VCvarI ext_anisotropy;
	static VCvarF maxdist;
	static VCvarI model_lighting;
	static VCvarI specular_highlights;

	//	Extensions
	bool CheckExtension(const char*);
	virtual void* GetExtFuncPtr(const char*) = 0;

	void FlushCaches(bool);
	void FlushOldCaches();
	surfcache_t	*AllocBlock(int, int);
	surfcache_t	*FreeBlock(surfcache_t*, bool);
	void CacheSurface(surface_t*);

	static int ToPowerOf2(int val);
	void GenerateTextures();
	void FlushTextures();
	void DeleteTextures();
	void SetSpriteLump(int, int);
	void SetPic(int);
	void GenerateTexture(int);
	void GenerateTranslatedSprite(int, int, int);
	void AdjustGamma(rgba_t *, int);
	void ResampleTexture(int, int, const byte*, int, int, byte*);
	void MipMap(int, int, byte*);
	void UploadTexture8(int, int, byte*, rgba_t*);
	void UploadTexture(int, int, rgba_t*);

	bool				mtexable;
	MultiTexCoord2f_t	p_MultiTexCoord2f;
	SelectTexture_t		p_SelectTexture;

	void MultiTexCoord(int level, GLfloat s, GLfloat t)
	{
		p_MultiTexCoord2f(GLenum(GL_TEXTURE0_ARB + level), s, t);
	}
	void SelectTexture(int level)
	{
		p_SelectTexture(GLenum(GL_TEXTURE0_ARB + level));
	}

	bool				pointparmsable;
	PointParameterf_t	p_PointParameterf;
	PointParameterfv_t	p_PointParameterfv;

	static void SetColor(vuint32 c)
	{
		glColor4ub(byte((c >> 16) & 0xff), byte((c >> 8) & 0xff),
			byte(c & 0xff), byte(c >> 24));
	}
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif
