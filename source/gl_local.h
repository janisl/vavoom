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

#ifndef _GL_LOCAL_H
#define _GL_LOCAL_H

// HEADER FILES ------------------------------------------------------------

#ifdef _WIN32
#include "winlocal.h"
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#include "gamedefs.h"
#include "cl_local.h"
#include "r_shared.h"

// MACROS ------------------------------------------------------------------

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

class TOpenGLDrawer : public TDrawer
{
 public:
	TOpenGLDrawer(void);
	void Init(void);
	void InitData(void);
	bool SetResolution(int, int, int);
	void InitResolution(void);
	void NewMap(void);
	void SetPalette(int);
	void StartUpdate(void);
	void Setup2D(void);
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
	void PutDot(int, int, dword);
	void DrawLine(int, int, dword, int, int, dword);
	void EndAutomap(void);

 private:
	int			view_x;
	int			view_y;
	int			view_w;
	int			view_h;
	float		view_fovx;
	float		view_fovy;

	GLint		maxTexSize;
	bool		texturesGenerated;

	GLuint		*texture_id;
	bool		*texture_sent;
	float		*texture_iw;
	float		*texture_ih;

	GLuint		*flat_id;
	bool		*flat_sent;

	GLuint		*sprite_id;
	bool		*sprite_sent;
	float		*spriteiw;
	float		*spriteih;

	GLuint		trspr_id[MAX_TRANSLATED_SPRITES];
	bool		trspr_sent[MAX_TRANSLATED_SPRITES];
	int			trspr_lump[MAX_TRANSLATED_SPRITES];
	int			trspr_tnum[MAX_TRANSLATED_SPRITES];
	float		trspriw[MAX_TRANSLATED_SPRITES];
	float		trsprih[MAX_TRANSLATED_SPRITES];

	GLuint		skin_id[MAX_SKIN_CACHE];
	char		skin_name[MAX_SKIN_CACHE][64];

	GLuint		particle_texture;

	GLuint		pic_id[MAX_PICS];
	bool		pic_sent[MAX_PICS];
	float		pic_iw[MAX_PICS];
	float		pic_ih[MAX_PICS];

	float		tex_iw;
	float		tex_ih;

	rgba_t		pal8_to24[256];

	int			trickframe;

	TCvarI		tex_linear;
	TCvarI		clear;
	TCvarI		ztrick;

	GLuint		lmap_id[NUM_BLOCK_SURFS];
	rgba_t		light_block[NUM_BLOCK_SURFS][BLOCK_WIDTH * BLOCK_HEIGHT];
	bool		block_changed[NUM_BLOCK_SURFS];
	surfcache_t	*light_chain[NUM_BLOCK_SURFS];
	surfcache_t	*freeblocks;
	surfcache_t	*cacheblocks[NUM_BLOCK_SURFS];
	surfcache_t	blockbuf[NUM_CACHE_BLOCKS];

	void FlushCaches(bool);
	surfcache_t	*AllocBlock(int width, int height);
	void CacheSurface(surface_t *surface);

	int ToPowerOf2(int val);
	void GenerateTextures(void);
	void DeleteTextures(void);
	void DrawColumnInCache(column_t*, rgba_t*, int, int, int, int);
	void GenerateTexture(int);
	void GenerateFlat(int);
	void GenerateSprite(int);
	void GenerateTranslatedSprite(int, int, int);
	void SetPic(int);
	void GeneratePicFromPatch(int);
	void GeneratePicFromRaw(int);
	void SetSkin(const char*);
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif

