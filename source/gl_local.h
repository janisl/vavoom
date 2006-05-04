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
	dword		Light;		// checked for strobe flash
	int			dlight;
	surface_t	*surf;
	dword		lastframe;
};

class VOpenGLDrawer : public VDrawer
{
public:
	//
	// VDrawer interface
	//
	VOpenGLDrawer();
	void Init();
	void InitData();
	bool SetResolution(int, int, int);
	void InitResolution();
	void NewMap();
	void SetPalette(int);
	void StartUpdate();
	void Setup2D();
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

	//	Polygon drawing
	void DrawPolygon(TVec*, int, int, int);
	void BeginSky();
	void DrawSkyPolygon(TVec*, int, int, float, int, float);
	void EndSky();
	void DrawMaskedPolygon(TVec*, int, int, int);
	void DrawSpritePolygon(TVec*, int, int, int, dword);
	void DrawAliasModel(const TVec&, const TAVec&, VModel*, int, int, const char*, dword, int, bool);

	//	Particles
	void StartParticles();
	void DrawParticle(particle_t *);
	void EndParticles();

	//	Drawing
	void DrawPic(float, float, float, float, float, float, float, float, int, int);
	void DrawPicShadow(float, float, float, float, float, float, float, float, int, int);
	void FillRectWithFlat(float, float, float, float, float, float, float, float, const char*);
	void FillRect(float, float, float, float, dword);
	void ShadeRect(int, int, int, int, int);
	void DrawConsoleBackground(int);
	void DrawSpriteLump(float, float, float, float, int, int, boolean);

	//	Automap
	void StartAutomap();
	void DrawLine(int, int, dword, int, int, dword);
	void EndAutomap();

private:
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
	dword		cacheframecount;

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
	void *GetExtFuncPtr(const char*);

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

	static void SetColor(dword c)
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
//	Revision 1.32  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
//	Revision 1.31  2006/02/05 14:11:00  dj_jl
//	Fixed conflict with Solaris.
//	
//	Revision 1.30  2006/01/03 19:57:45  dj_jl
//	Fixed anisotropic texture filtering.
//	
//	Revision 1.29  2005/05/26 16:50:14  dj_jl
//	Created texture manager class
//	
//	Revision 1.28  2005/05/03 14:57:06  dj_jl
//	Added support for specifying skin index.
//	
//	Revision 1.27  2005/01/24 12:53:54  dj_jl
//	Skybox fixes.
//	
//	Revision 1.26  2004/08/21 17:22:15  dj_jl
//	Changed rendering driver declaration.
//	
//	Revision 1.25  2004/02/09 17:29:58  dj_jl
//	Increased block count
//	
//	Revision 1.24  2003/10/22 06:13:52  dj_jl
//	Freeing old blocks on overflow
//	
//	Revision 1.23  2003/03/08 12:08:04  dj_jl
//	Beautification.
//	
//	Revision 1.22  2002/08/28 16:39:19  dj_jl
//	Implemented sector light color.
//	
//	Revision 1.21  2002/07/13 07:38:00  dj_jl
//	Added drawers to the object tree.
//	
//	Revision 1.20  2002/03/28 17:56:52  dj_jl
//	Increased lightmap texture count.
//	
//	Revision 1.19  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.18  2001/11/09 14:18:40  dj_jl
//	Added specular highlights
//	
//	Revision 1.17  2001/10/27 07:45:01  dj_jl
//	Added gamma controls
//	
//	Revision 1.16  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.15  2001/10/12 17:28:26  dj_jl
//	Blending of sprite borders
//	
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
