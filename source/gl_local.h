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

typedef void (APIENTRY*glMultiTexCoord2fARB_t)(GLenum, GLfloat, GLfloat);
typedef void (APIENTRY*glActiveTextureARB_t)(GLenum);

// EXT_point_parameters
#ifndef GL_EXT_point_parameters
#define GL_POINT_SIZE_MIN_EXT				0x8126
#define GL_POINT_SIZE_MAX_EXT				0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE_EXT	0x8128
#define GL_DISTANCE_ATTENUATION_EXT			0x8129
#endif

typedef void (APIENTRY*glPointParameterfEXT_t)(GLenum, GLfloat);
typedef void (APIENTRY*glPointParameterfvEXT_t)(GLenum, const GLfloat *);

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

#ifndef GL_ARB_shader_objects
#define GL_PROGRAM_OBJECT_ARB				0x8B40
#define GL_SHADER_OBJECT_ARB				0x8B48
#define GL_OBJECT_TYPE_ARB					0x8B4E
#define GL_OBJECT_SUBTYPE_ARB				0x8B4F
#define GL_FLOAT_VEC2_ARB					0x8B50
#define GL_FLOAT_VEC3_ARB					0x8B51
#define GL_FLOAT_VEC4_ARB					0x8B52
#define GL_INT_VEC2_ARB						0x8B53
#define GL_INT_VEC3_ARB						0x8B54
#define GL_INT_VEC4_ARB						0x8B55
#define GL_BOOL_ARB							0x8B56
#define GL_BOOL_VEC2_ARB					0x8B57
#define GL_BOOL_VEC3_ARB					0x8B58
#define GL_BOOL_VEC4_ARB					0x8B59
#define GL_FLOAT_MAT2_ARB					0x8B5A
#define GL_FLOAT_MAT3_ARB					0x8B5B
#define GL_FLOAT_MAT4_ARB					0x8B5C
#define GL_SAMPLER_1D_ARB					0x8B5D
#define GL_SAMPLER_2D_ARB					0x8B5E
#define GL_SAMPLER_3D_ARB					0x8B5F
#define GL_SAMPLER_CUBE_ARB					0x8B60
#define GL_SAMPLER_1D_SHADOW_ARB			0x8B61
#define GL_SAMPLER_2D_SHADOW_ARB			0x8B62
#define GL_SAMPLER_2D_RECT_ARB				0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW_ARB		0x8B64
#define GL_OBJECT_DELETE_STATUS_ARB			0x8B80
#define GL_OBJECT_COMPILE_STATUS_ARB		0x8B81
#define GL_OBJECT_LINK_STATUS_ARB			0x8B82
#define GL_OBJECT_VALIDATE_STATUS_ARB		0x8B83
#define GL_OBJECT_INFO_LOG_LENGTH_ARB		0x8B84
#define GL_OBJECT_ATTACHED_OBJECTS_ARB		0x8B85
#define GL_OBJECT_ACTIVE_UNIFORMS_ARB		0x8B86
#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB	0x8B87
#define GL_OBJECT_SHADER_SOURCE_LENGTH_ARB	0x8B88

typedef char GLcharARB;
typedef unsigned int GLhandleARB;
#endif

typedef void (APIENTRY*glDeleteObjectARB_t)(GLhandleARB);
typedef GLhandleARB (APIENTRY*glGetHandleARB_t)(GLenum);
typedef void (APIENTRY*glDetachObjectARB_t)(GLhandleARB, GLhandleARB);
typedef GLhandleARB (APIENTRY*glCreateShaderObjectARB_t)(GLenum);
typedef void (APIENTRY*glShaderSourceARB_t)(GLhandleARB, GLsizei, const GLcharARB* *, const GLint *);
typedef void (APIENTRY*glCompileShaderARB_t)(GLhandleARB);
typedef GLhandleARB (APIENTRY*glCreateProgramObjectARB_t)(void);
typedef void (APIENTRY*glAttachObjectARB_t)(GLhandleARB, GLhandleARB);
typedef void (APIENTRY*glLinkProgramARB_t)(GLhandleARB);
typedef void (APIENTRY*glUseProgramObjectARB_t)(GLhandleARB);
typedef void (APIENTRY*glValidateProgramARB_t)(GLhandleARB);
typedef void (APIENTRY*glUniform1fARB_t)(GLint, GLfloat);
typedef void (APIENTRY*glUniform2fARB_t)(GLint, GLfloat, GLfloat);
typedef void (APIENTRY*glUniform3fARB_t)(GLint, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY*glUniform4fARB_t)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY*glUniform1iARB_t)(GLint, GLint);
typedef void (APIENTRY*glUniform2iARB_t)(GLint, GLint, GLint);
typedef void (APIENTRY*glUniform3iARB_t)(GLint, GLint, GLint, GLint);
typedef void (APIENTRY*glUniform4iARB_t)(GLint, GLint, GLint, GLint, GLint);
typedef void (APIENTRY*glUniform1fvARB_t)(GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY*glUniform2fvARB_t)(GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY*glUniform3fvARB_t)(GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY*glUniform4fvARB_t)(GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY*glUniform1ivARB_t)(GLint, GLsizei, const GLint *);
typedef void (APIENTRY*glUniform2ivARB_t)(GLint, GLsizei, const GLint *);
typedef void (APIENTRY*glUniform3ivARB_t)(GLint, GLsizei, const GLint *);
typedef void (APIENTRY*glUniform4ivARB_t)(GLint, GLsizei, const GLint *);
typedef void (APIENTRY*glUniformMatrix2fvARB_t)(GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY*glUniformMatrix3fvARB_t)(GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY*glUniformMatrix4fvARB_t)(GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY*glGetObjectParameterfvARB_t)(GLhandleARB, GLenum, GLfloat *);
typedef void (APIENTRY*glGetObjectParameterivARB_t)(GLhandleARB, GLenum, GLint *);
typedef void (APIENTRY*glGetInfoLogARB_t)(GLhandleARB, GLsizei, GLsizei *, GLcharARB *);
typedef void (APIENTRY*glGetAttachedObjectsARB_t)(GLhandleARB, GLsizei, GLsizei *, GLhandleARB *);
typedef GLint (APIENTRY*glGetUniformLocationARB_t)(GLhandleARB, const GLcharARB *);
typedef void (APIENTRY*glGetActiveUniformARB_t)(GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *);
typedef void (APIENTRY*glGetUniformfvARB_t)(GLhandleARB, GLint, GLfloat *);
typedef void (APIENTRY*glGetUniformivARB_t)(GLhandleARB, GLint, GLint *);
typedef void (APIENTRY*glGetShaderSourceARB_t)(GLhandleARB, GLsizei, GLsizei *, GLcharARB *);

#ifndef GL_ARB_vertex_shader
#define GL_VERTEX_SHADER_ARB				0x8B31
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB	0x8B4A
#define GL_MAX_VARYING_FLOATS_ARB			0x8B4B
#define GL_MAX_VERTEX_ATTRIBS_ARB			0x8869
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB		0x8872
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB	0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB	0x8B4D
#define GL_MAX_TEXTURE_COORDS_ARB			0x8871
#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB	0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_ARB		0x8643
#define GL_OBJECT_ACTIVE_ATTRIBUTES_ARB		0x8B89
#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB	0x8B8A
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB	0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB		0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB	0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB		0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB	0x886A
#define GL_CURRENT_VERTEX_ATTRIB_ARB		0x8626
#define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB	0x8645
#endif

typedef void (APIENTRY*glVertexAttrib1dARB_t)(GLuint, GLdouble);
typedef void (APIENTRY*glVertexAttrib1dvARB_t)(GLuint, const GLdouble *);
typedef void (APIENTRY*glVertexAttrib1fARB_t)(GLuint, GLfloat);
typedef void (APIENTRY*glVertexAttrib1fvARB_t)(GLuint, const GLfloat *);
typedef void (APIENTRY*glVertexAttrib1sARB_t)(GLuint, GLshort);
typedef void (APIENTRY*glVertexAttrib1svARB_t)(GLuint, const GLshort *);
typedef void (APIENTRY*glVertexAttrib2dARB_t)(GLuint, GLdouble, GLdouble);
typedef void (APIENTRY*glVertexAttrib2dvARB_t)(GLuint, const GLdouble *);
typedef void (APIENTRY*glVertexAttrib2fARB_t)(GLuint, GLfloat, GLfloat);
typedef void (APIENTRY*glVertexAttrib2fvARB_t)(GLuint, const GLfloat *);
typedef void (APIENTRY*glVertexAttrib2sARB_t)(GLuint, GLshort, GLshort);
typedef void (APIENTRY*glVertexAttrib2svARB_t)(GLuint, const GLshort *);
typedef void (APIENTRY*glVertexAttrib3dARB_t)(GLuint, GLdouble, GLdouble, GLdouble);
typedef void (APIENTRY*glVertexAttrib3dvARB_t)(GLuint, const GLdouble *);
typedef void (APIENTRY*glVertexAttrib3fARB_t)(GLuint, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY*glVertexAttrib3fvARB_t)(GLuint, const GLfloat *);
typedef void (APIENTRY*glVertexAttrib3sARB_t)(GLuint, GLshort, GLshort, GLshort);
typedef void (APIENTRY*glVertexAttrib3svARB_t)(GLuint, const GLshort *);
typedef void (APIENTRY*glVertexAttrib4NbvARB_t)(GLuint, const GLbyte *);
typedef void (APIENTRY*glVertexAttrib4NivARB_t)(GLuint, const GLint *);
typedef void (APIENTRY*glVertexAttrib4NsvARB_t)(GLuint, const GLshort *);
typedef void (APIENTRY*glVertexAttrib4NubARB_t)(GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
typedef void (APIENTRY*glVertexAttrib4NubvARB_t)(GLuint, const GLubyte *);
typedef void (APIENTRY*glVertexAttrib4NuivARB_t)(GLuint, const GLuint *);
typedef void (APIENTRY*glVertexAttrib4NusvARB_t)(GLuint, const GLushort *);
typedef void (APIENTRY*glVertexAttrib4bvARB_t)(GLuint, const GLbyte *);
typedef void (APIENTRY*glVertexAttrib4dARB_t)(GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
typedef void (APIENTRY*glVertexAttrib4dvARB_t)(GLuint, const GLdouble *);
typedef void (APIENTRY*glVertexAttrib4fARB_t)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY*glVertexAttrib4fvARB_t)(GLuint, const GLfloat *);
typedef void (APIENTRY*glVertexAttrib4ivARB_t)(GLuint, const GLint *);
typedef void (APIENTRY*glVertexAttrib4sARB_t)(GLuint, GLshort, GLshort, GLshort, GLshort);
typedef void (APIENTRY*glVertexAttrib4svARB_t)(GLuint, const GLshort *);
typedef void (APIENTRY*glVertexAttrib4ubvARB_t)(GLuint, const GLubyte *);
typedef void (APIENTRY*glVertexAttrib4uivARB_t)(GLuint, const GLuint *);
typedef void (APIENTRY*glVertexAttrib4usvARB_t)(GLuint, const GLushort *);
typedef void (APIENTRY*glVertexAttribPointerARB_t)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
typedef void (APIENTRY*glEnableVertexAttribArrayARB_t)(GLuint);
typedef void (APIENTRY*glDisableVertexAttribArrayARB_t)(GLuint);
typedef void (APIENTRY*glBindAttribLocationARB_t)(GLhandleARB, GLuint, const GLcharARB *);
typedef void (APIENTRY*glGetActiveAttribARB_t)(GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *);
typedef GLint (APIENTRY*glGetAttribLocationARB_t)(GLhandleARB, const GLcharARB *);
typedef void (APIENTRY*glGetVertexAttribdvARB_t)(GLuint, GLenum, GLdouble *);
typedef void (APIENTRY*glGetVertexAttribfvARB_t)(GLuint, GLenum, GLfloat *);
typedef void (APIENTRY*glGetVertexAttribivARB_t)(GLuint, GLenum, GLint *);
typedef void (APIENTRY*glGetVertexAttribPointervARB_t)(GLuint, GLenum, GLvoid* *);

#ifndef GL_ARB_fragment_shader
#define GL_FRAGMENT_SHADER_ARB				0x8B30
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB	0x8B49
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB	0x8B8B
#endif

#ifndef GL_ARB_shading_language_100
#define GL_SHADING_LANGUAGE_VERSION_ARB		0x8B8C
#endif

class VOpenGLDrawer : public VAdvDrawer
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
	void WorldDrawingShaders();
	void DrawWorldAmbientPass();
	void BeginShadowVolumesPass();
	void BeginLightShadowVolumes();
	void RenderSurfaceShadowVolume(surface_t *surf, TVec& LightPos, float Radius);
	void DrawLightShadowsPass(TVec& LightPos, float Radius, vuint32 Colour);
	void DrawWorldTexturesPass();
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
	void DrawAliasModelAmbient(const TVec&, const TAVec&, const TVec&,
		const TVec&, mmdl_t*, int, int, VTexture*, vuint32, float, bool);
	void DrawAliasModelTextures(const TVec&, const TAVec&, const TVec&, const TVec&,
		mmdl_t*, int, int, VTexture*, VTextureTranslation*, int, float, bool);
	void BeginModelsLightPass(TVec& LightPos, float Radius, vuint32 Colour);
	void DrawAliasModelLight(const TVec&, const TAVec&, const TVec&,
		const TVec&, mmdl_t*, int, int, VTexture*, float, bool);
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
	GLint					maxTexSize;
	bool					texturesGenerated;

	GLuint					particle_texture;

	GLuint					lmap_id[NUM_BLOCK_SURFS];

	GLuint					addmap_id[NUM_BLOCK_SURFS];

	float					tex_iw;
	float					tex_ih;

	GLenum					maxfilter;
	GLenum					minfilter;
	GLenum					mipfilter;
	GLenum					ClampToEdge;
	GLfloat					max_anisotropy;

	int						lastgamma;
	int						CurrentFade;

	bool					mtexable;
	bool					pointparmsable;
	bool					HaveShaders;

	TArray<GLhandleARB>		CreatedShaderObjects;

	GLhandleARB				DrawSimpleProgram;
	GLint					DrawSimpleTextureLoc;
	GLint					DrawSimpleAlphaLoc;

	GLhandleARB				DrawShadowProgram;
	GLint					DrawShadowTextureLoc;
	GLint					DrawShadowAlphaLoc;

	GLhandleARB				DrawFixedColProgram;
	GLint					DrawFixedColColourLoc;

	GLhandleARB				DrawAutomapProgram;

	GLhandleARB				SurfZBufProgram;

	GLhandleARB				SurfSimpleProgram;
	GLint					SurfSimpleSAxisLoc;
	GLint					SurfSimpleTAxisLoc;
	GLint					SurfSimpleSOffsLoc;
	GLint					SurfSimpleTOffsLoc;
	GLint					SurfSimpleTexIWLoc;
	GLint					SurfSimpleTexIHLoc;
	GLint					SurfSimpleTextureLoc;
	GLint					SurfSimpleLightLoc;
	GLint					SurfSimpleFogEnabledLoc;
	GLint					SurfSimpleFogTypeLoc;
	GLint					SurfSimpleFogColourLoc;
	GLint					SurfSimpleFogDensityLoc;
	GLint					SurfSimpleFogStartLoc;
	GLint					SurfSimpleFogEndLoc;

	GLhandleARB				SurfLightmapProgram;
	GLint					SurfLightmapSAxisLoc;
	GLint					SurfLightmapTAxisLoc;
	GLint					SurfLightmapSOffsLoc;
	GLint					SurfLightmapTOffsLoc;
	GLint					SurfLightmapTexIWLoc;
	GLint					SurfLightmapTexIHLoc;
	GLint					SurfLightmapTexMinSLoc;
	GLint					SurfLightmapTexMinTLoc;
	GLint					SurfLightmapCacheSLoc;
	GLint					SurfLightmapCacheTLoc;
	GLint					SurfLightmapTextureLoc;
	GLint					SurfLightmapLightMapLoc;
	GLint					SurfLightmapSpecularMapLoc;
	GLint					SurfLightmapFogEnabledLoc;
	GLint					SurfLightmapFogTypeLoc;
	GLint					SurfLightmapFogColourLoc;
	GLint					SurfLightmapFogDensityLoc;
	GLint					SurfLightmapFogStartLoc;
	GLint					SurfLightmapFogEndLoc;

	GLhandleARB				SurfSkyProgram;
	GLint					SurfSkyTextureLoc;
	GLint					SurfSkyBrightnessLoc;
	GLint					SurfSkyTexCoordLoc;

	GLhandleARB				SurfDSkyProgram;
	GLint					SurfDSkyTextureLoc;
	GLint					SurfDSkyTexture2Loc;
	GLint					SurfDSkyBrightnessLoc;
	GLint					SurfDSkyTexCoordLoc;
	GLint					SurfDSkyTexCoord2Loc;

	GLhandleARB				SurfMaskedProgram;
	GLint					SurfMaskedTextureLoc;
	GLint					SurfMaskedLightLoc;
	GLint					SurfMaskedFogEnabledLoc;
	GLint					SurfMaskedFogTypeLoc;
	GLint					SurfMaskedFogColourLoc;
	GLint					SurfMaskedFogDensityLoc;
	GLint					SurfMaskedFogStartLoc;
	GLint					SurfMaskedFogEndLoc;
	GLint					SurfMaskedAlphaRefLoc;
	GLint					SurfMaskedTexCoordLoc;

	GLhandleARB				SurfModelProgram;
	GLint					SurfModelInterLoc;
	GLint					SurfModelTextureLoc;
	GLint					SurfModelFogEnabledLoc;
	GLint					SurfModelFogTypeLoc;
	GLint					SurfModelFogColourLoc;
	GLint					SurfModelFogDensityLoc;
	GLint					SurfModelFogStartLoc;
	GLint					SurfModelFogEndLoc;
	GLint					SurfModelVert2Loc;
	GLint					SurfModelTexCoordLoc;
	GLint					SurfModelLightValLoc;

	GLhandleARB				SurfPartProgram;
	GLint					SurfPartTexCoordLoc;
	GLint					SurfPartLightValLoc;

	GLhandleARB				ShadowsAmbientProgram;
	GLint					ShadowsAmbientLightLoc;

	GLhandleARB				ShadowsLightProgram;
	GLint					ShadowsLightLightPosLoc;
	GLint					ShadowsLightLightRadiusLoc;
	GLint					ShadowsLightLightColourLoc;
	GLint					ShadowsLightSurfNormalLoc;
	GLint					ShadowsLightSurfDistLoc;

	GLhandleARB				ShadowsTextureProgram;
	GLint					ShadowsTextureTexCoordLoc;
	GLint					ShadowsTextureTextureLoc;

	GLhandleARB				ShadowsModelAmbientProgram;
	GLint					ShadowsModelAmbientInterLoc;
	GLint					ShadowsModelAmbientTextureLoc;
	GLint					ShadowsModelAmbientLightLoc;
	GLint					ShadowsModelAmbientModelToWorldMatLoc;
	GLint					ShadowsModelAmbientVert2Loc;
	GLint					ShadowsModelAmbientTexCoordLoc;

	GLhandleARB				ShadowsModelTexturesProgram;
	GLint					ShadowsModelTexturesInterLoc;
	GLint					ShadowsModelTexturesTextureLoc;
	GLint					ShadowsModelTexturesModelToWorldMatLoc;
	GLint					ShadowsModelTexturesVert2Loc;
	GLint					ShadowsModelTexturesTexCoordLoc;

	GLhandleARB				ShadowsModelLightProgram;
	GLint					ShadowsModelLightInterLoc;
	GLint					ShadowsModelLightTextureLoc;
	GLint					ShadowsModelLightLightPosLoc;
	GLint					ShadowsModelLightLightRadiusLoc;
	GLint					ShadowsModelLightLightColourLoc;
	GLint					ShadowsModelLightModelToWorldMatLoc;
	GLint					ShadowsModelLightNormalToWorldMatLoc;
	GLint					ShadowsModelLightVert2Loc;
	GLint					ShadowsModelLightVertNormalLoc;
	GLint					ShadowsModelLightVert2NormalLoc;
	GLint					ShadowsModelLightTexCoordLoc;

	//
	//	Console variables
	//
	static VCvarI tex_linear;
	static VCvarI clear;
	static VCvarI blend_sprites;
	static VCvarI ext_multitexture;
	static VCvarI ext_point_parameters;
	static VCvarI ext_anisotropy;
	static VCvarI ext_shaders;
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

	GLhandleARB LoadShader(GLenum Type, const VStr& FileName);
	GLhandleARB CreateProgram(GLhandleARB VertexShader, GLhandleARB FragmentShader);

#define _(x)	x##_t	p_##x
	_(glMultiTexCoord2fARB);
	_(glActiveTextureARB);

	_(glPointParameterfEXT);
	_(glPointParameterfvEXT);

	_(glStencilFuncSeparate);
	_(glStencilOpSeparate);

	_(glDeleteObjectARB);
	_(glGetHandleARB);
	_(glDetachObjectARB);
	_(glCreateShaderObjectARB);
	_(glShaderSourceARB);
	_(glCompileShaderARB);
	_(glCreateProgramObjectARB);
	_(glAttachObjectARB);
	_(glLinkProgramARB);
	_(glUseProgramObjectARB);
	_(glValidateProgramARB);
	_(glUniform1fARB);
	_(glUniform2fARB);
	_(glUniform3fARB);
	_(glUniform4fARB);
	_(glUniform1iARB);
	_(glUniform2iARB);
	_(glUniform3iARB);
	_(glUniform4iARB);
	_(glUniform1fvARB);
	_(glUniform2fvARB);
	_(glUniform3fvARB);
	_(glUniform4fvARB);
	_(glUniform1ivARB);
	_(glUniform2ivARB);
	_(glUniform3ivARB);
	_(glUniform4ivARB);
	_(glUniformMatrix2fvARB);
	_(glUniformMatrix3fvARB);
	_(glUniformMatrix4fvARB);
	_(glGetObjectParameterfvARB);
	_(glGetObjectParameterivARB);
	_(glGetInfoLogARB);
	_(glGetAttachedObjectsARB);
	_(glGetUniformLocationARB);
	_(glGetActiveUniformARB);
	_(glGetUniformfvARB);
	_(glGetUniformivARB);
	_(glGetShaderSourceARB);

	_(glVertexAttrib1dARB);
	_(glVertexAttrib1dvARB);
	_(glVertexAttrib1fARB);
	_(glVertexAttrib1fvARB);
	_(glVertexAttrib1sARB);
	_(glVertexAttrib1svARB);
	_(glVertexAttrib2dARB);
	_(glVertexAttrib2dvARB);
	_(glVertexAttrib2fARB);
	_(glVertexAttrib2fvARB);
	_(glVertexAttrib2sARB);
	_(glVertexAttrib2svARB);
	_(glVertexAttrib3dARB);
	_(glVertexAttrib3dvARB);
	_(glVertexAttrib3fARB);
	_(glVertexAttrib3fvARB);
	_(glVertexAttrib3sARB);
	_(glVertexAttrib3svARB);
	_(glVertexAttrib4NbvARB);
	_(glVertexAttrib4NivARB);
	_(glVertexAttrib4NsvARB);
	_(glVertexAttrib4NubARB);
	_(glVertexAttrib4NubvARB);
	_(glVertexAttrib4NuivARB);
	_(glVertexAttrib4NusvARB);
	_(glVertexAttrib4bvARB);
	_(glVertexAttrib4dARB);
	_(glVertexAttrib4dvARB);
	_(glVertexAttrib4fARB);
	_(glVertexAttrib4fvARB);
	_(glVertexAttrib4ivARB);
	_(glVertexAttrib4sARB);
	_(glVertexAttrib4svARB);
	_(glVertexAttrib4ubvARB);
	_(glVertexAttrib4uivARB);
	_(glVertexAttrib4usvARB);
	_(glVertexAttribPointerARB);
	_(glEnableVertexAttribArrayARB);
	_(glDisableVertexAttribArrayARB);
	_(glBindAttribLocationARB);
	_(glGetActiveAttribARB);
	_(glGetAttribLocationARB);
	_(glGetVertexAttribdvARB);
	_(glGetVertexAttribfvARB);
	_(glGetVertexAttribivARB);
	_(glGetVertexAttribPointervARB);
#undef _

	void MultiTexCoord(int level, GLfloat s, GLfloat t)
	{
		p_glMultiTexCoord2fARB(GLenum(GL_TEXTURE0_ARB + level), s, t);
	}
	void SelectTexture(int level)
	{
		p_glActiveTextureARB(GLenum(GL_TEXTURE0_ARB + level));
	}

	static void SetColour(vuint32 c)
	{
		glColor4ub(byte((c >> 16) & 0xff), byte((c >> 8) & 0xff),
			byte(c & 0xff), byte(c >> 24));
	}
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif
