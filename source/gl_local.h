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
#include "r_hardware.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

//
//	Extensions
//

// ARB_multitexture
#ifndef GL_ARB_multitexture
#define GL_TEXTURE0_ARB						0x84C0
#define GL_TEXTURE1_ARB						0x84C1
#define GL_TEXTURE2_ARB						0x84C2
#define GL_TEXTURE3_ARB						0x84C3
#define GL_TEXTURE4_ARB						0x84C4
#define GL_TEXTURE5_ARB						0x84C5
#define GL_TEXTURE6_ARB						0x84C6
#define GL_TEXTURE7_ARB						0x84C7
#define GL_TEXTURE8_ARB						0x84C8
#define GL_TEXTURE9_ARB						0x84C9
#define GL_TEXTURE10_ARB					0x84CA
#define GL_TEXTURE11_ARB					0x84CB
#define GL_TEXTURE12_ARB					0x84CC
#define GL_TEXTURE13_ARB					0x84CD
#define GL_TEXTURE14_ARB					0x84CE
#define GL_TEXTURE15_ARB					0x84CF
#define GL_TEXTURE16_ARB					0x84D0
#define GL_TEXTURE17_ARB					0x84D1
#define GL_TEXTURE18_ARB					0x84D2
#define GL_TEXTURE19_ARB					0x84D3
#define GL_TEXTURE20_ARB					0x84D4
#define GL_TEXTURE21_ARB					0x84D5
#define GL_TEXTURE22_ARB					0x84D6
#define GL_TEXTURE23_ARB					0x84D7
#define GL_TEXTURE24_ARB					0x84D8
#define GL_TEXTURE25_ARB					0x84D9
#define GL_TEXTURE26_ARB					0x84DA
#define GL_TEXTURE27_ARB					0x84DB
#define GL_TEXTURE28_ARB					0x84DC
#define GL_TEXTURE29_ARB					0x84DD
#define GL_TEXTURE30_ARB					0x84DE
#define GL_TEXTURE31_ARB					0x84DF
#define GL_ACTIVE_TEXTURE_ARB				0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB		0x84E1
#define GL_MAX_TEXTURE_UNITS_ARB			0x84E2
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

typedef void (APIENTRY*glStencilFuncSeparate_t)(GLenum, GLenum, GLint, GLuint);
typedef void (APIENTRY*glStencilOpSeparate_t)(GLenum, GLenum, GLenum, GLenum);

#ifndef GL_EXT_stencil_wrap
#define GL_INCR_WRAP_EXT					0x8507
#define GL_DECR_WRAP_EXT					0x8508
#endif

class VOpenGLDrawer : public VHardwareDrawer
{
public:
	//
	// VDrawer interface
	//
	VOpenGLDrawer();
	void InitResolution();
	void StartUpdate();
	void Setup2D();
	void BeginDirectUpdate();
	void EndDirectUpdate();
	void* ReadScreen(int*, bool*);
	void ReadBackScreen(int, int, rgba_t*);

	//	Rendering stuff
	void SetupView(VRenderLevelDrawer*, const refdef_t*);
	void SetupViewOrg();
	void WorldDrawing();
	void DoWorldDrawing();
	void DoLight(TVec& LightPos);
	void DrawSurfsDepth();
	void RenderSurfaceShadowVolume(surface_t *surf, TVec& LightPos);
	void DrawSurfsShadowVolumes(TVec& LightPos);
	void DrawShadow(TVec& LightPos, float Radius);
	void DrawTextures();
	void EndView();

	//	Texture stuff
	void PrecacheTexture(VTexture*);

	//	Polygon drawing
	void DrawPolygon(surface_t*, int);
	void DrawSkyPortal(surface_t*, int);
	void DrawSkyPolygon(surface_t*, bool, VTexture*, float, VTexture*, float,
		int);
	void DrawHorizonPolygon(surface_t*, int);
	void DrawMaskedPolygon(surface_t*, float, bool);
	void DrawSpritePolygon(TVec*, VTexture*, float, bool, VTextureTranslation*,
		int, vuint32, vuint32, const TVec&, float, const TVec&, const TVec&,
		const TVec&);
	void DrawAliasModel(const TVec&, const TAVec&, const TVec&, const TVec&,
		mmdl_t*, int, int, VTexture*, VTextureTranslation*, int, vuint32, vuint32,
		float, bool, bool, float, bool);
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

protected:
	GLint		maxTexSize;
	bool		texturesGenerated;

	GLuint		particle_texture;

	GLuint		lmap_id[NUM_BLOCK_SURFS];

	GLuint		addmap_id[NUM_BLOCK_SURFS];

	float		tex_iw;
	float		tex_ih;

	int			lastgamma;
	int			CurrentFade;

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

	void SetFade(vuint32 NewFade);

	void GenerateTextures();
	void FlushTextures();
	void DeleteTextures();
	void FlushTexture(VTexture*);
	void SetTexture(VTexture*, int);
	void SetSpriteLump(VTexture*, VTextureTranslation*, int);
	void SetPic(VTexture*, VTextureTranslation*, int);
	void GenerateTexture(VTexture*, GLuint*, VTextureTranslation*, int);
	void UploadTexture8(int, int, const vuint8*, const rgba_t*);
	void UploadTexture(int, int, const rgba_t*);

	void DoHorizonPolygon(surface_t*);
	void DrawPortalArea(VPortal*);

	bool				mtexable;
	MultiTexCoord2f_t	p_MultiTexCoord2f;
	SelectTexture_t		p_SelectTexture;

	glStencilFuncSeparate_t	p_glStencilFuncSeparate;
	glStencilOpSeparate_t	p_glStencilOpSeparate;

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

	static void SetColour(vuint32 c)
	{
		glColor4ub(byte((c >> 16) & 0xff), byte((c >> 8) & 0xff),
			byte(c & 0xff), byte(c >> 24));
	}
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif
