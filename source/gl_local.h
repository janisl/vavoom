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
#define MAX_SKIN_CACHE				256

#define BLOCK_WIDTH					128
#define BLOCK_HEIGHT				128
#define NUM_BLOCK_SURFS				16
#define NUM_CACHE_BLOCKS			4000

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
	void SetSpriteLump(int, int);

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

	GLenum		maxfilter;
	GLenum		minfilter;
	GLenum		mipfilter;

	rgba_t		pal8_to24[256];

	TCvarI		tex_linear;
	TCvarI		clear;
	TCvarI		ext_multitexture;
	TCvarI		ext_point_parameters;
	TCvarI		ext_anisotropy;

	GLuint		lmap_id[NUM_BLOCK_SURFS];
	rgba_t		light_block[NUM_BLOCK_SURFS][BLOCK_WIDTH * BLOCK_HEIGHT];
	bool		block_changed[NUM_BLOCK_SURFS];
	surfcache_t	*light_chain[NUM_BLOCK_SURFS];
	surfcache_t	*freeblocks;
	surfcache_t	*cacheblocks[NUM_BLOCK_SURFS];
	surfcache_t	blockbuf[NUM_CACHE_BLOCKS];

	//	Extensions
	bool CheckExtension(const char*);
	void *GetExtFuncPtr(const char*);

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
	void ResampleTexture(int, int, const byte*, int, int, byte*);
	void MipMap(int, int, byte*);
	void UploadTexture(int, int, rgba_t*);
	void UploadTextureNoMip(int, int, rgba_t*);

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

	void SetColor(dword c)
	{
		glColor4ub(byte((c >> 16) & 0xff), byte((c >> 8) & 0xff),
			byte(c & 0xff), byte(c >> 24));
	}
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.14  2001/10/09 17:21:39  dj_jl
//	Added sky begining and ending functions
//
//	Revision 1.13  2001/10/04 17:23:29  dj_jl
//	Got rid of some warnings
//	
//	Revision 1.12  2001/09/12 17:31:27  dj_jl
//	Rectangle drawing and direct update for plugins
//	
//	Revision 1.11  2001/09/05 12:21:42  dj_jl
//	Release changes
//	
//	Revision 1.10  2001/08/31 17:25:38  dj_jl
//	Anisotropy filtering
//	
//	Revision 1.9  2001/08/23 17:51:12  dj_jl
//	My own mipmap creation code, glu not used anymore
//	
//	Revision 1.8  2001/08/21 17:47:51  dj_jl
//	Localized GL/glu.h
//	
//	Revision 1.7  2001/08/15 17:15:55  dj_jl
//	Drawer API changes, removed wipes
//	
//	Revision 1.6  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.5  2001/08/04 17:31:16  dj_jl
//	Added depth hack for weapon models
//	Added support for multitexture extensions
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
