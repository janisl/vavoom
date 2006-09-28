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
//**	OpenGL driver, main module
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VCvarI VOpenGLDrawer::tex_linear("gl_tex_linear", "2", CVAR_Archive);
VCvarI VOpenGLDrawer::clear("gl_clear", "0", CVAR_Archive);
VCvarI VOpenGLDrawer::blend_sprites("gl_blend_sprites", "0", CVAR_Archive);
VCvarI VOpenGLDrawer::ext_multitexture("gl_ext_multitexture", "1", CVAR_Archive);
VCvarI VOpenGLDrawer::ext_point_parameters("gl_ext_point_parameters", "1", CVAR_Archive);
VCvarI VOpenGLDrawer::ext_anisotropy("gl_ext_anisotropy", "1", CVAR_Archive);
VCvarF VOpenGLDrawer::maxdist("gl_maxdist", "8192.0", CVAR_Archive);
VCvarI VOpenGLDrawer::model_lighting("gl_model_lighting", "0", CVAR_Archive);
VCvarI VOpenGLDrawer::specular_highlights("gl_specular_highlights", "1", CVAR_Archive);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VOpenGLDrawer::VOpenGLDrawer
//
//==========================================================================

VOpenGLDrawer::VOpenGLDrawer()
: texturesGenerated(false)
, freeblocks(0)
, cacheframecount(0)
, lastgamma(0)
{
	memset(light_block, 0, sizeof(light_block));
	memset(block_changed, 0, sizeof(block_changed));
	memset(light_chain, 0, sizeof(light_chain));
	memset(add_block, 0, sizeof(add_block));
	memset(add_changed, 0, sizeof(add_changed));
	memset(add_chain, 0, sizeof(add_chain));
	memset(cacheblocks, 0, sizeof(cacheblocks));
	memset(blockbuf, 0, sizeof(blockbuf));
}

//==========================================================================
//
//	VOpenGLDrawer::InitResolution
//
//==========================================================================

void VOpenGLDrawer::InitResolution()
{
	guard(VOpenGLDrawer::InitResolution);
	GCon->Logf(NAME_Init, "GL_VENDOR: %s", glGetString(GL_VENDOR));
	GCon->Logf(NAME_Init, "GL_RENDERER: %s", glGetString(GL_RENDERER));
	GCon->Logf(NAME_Init, "GL_VERSION: %s", glGetString (GL_VERSION));

	GCon->Log(NAME_Init, "GL_EXTENSIONS:");
	TArray<VStr> Exts;
	VStr((char*)glGetString(GL_EXTENSIONS)).Split(' ', Exts);
	for (int i = 0; i < Exts.Num(); i++)
	{
		GCon->Log(NAME_Init, VStr("- ") + Exts[i]);
	}

	// Check the maximum texture size.
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
	GCon->Logf(NAME_Init, "Maximum texture size: %d", maxTexSize);

	//	Check multi-texture extensions
	if (ext_multitexture && CheckExtension("GL_ARB_multitexture"))
	{
		GCon->Log(NAME_Init, "Found GL_ARB_multitexture...");

		p_MultiTexCoord2f = MultiTexCoord2f_t(GetExtFuncPtr("glMultiTexCoord2fARB"));
		p_SelectTexture = SelectTexture_t(GetExtFuncPtr("glActiveTextureARB"));

		if (p_MultiTexCoord2f && p_SelectTexture)
		{
			GCon->Log(NAME_Init, "Multitexture extensions found.");
			mtexable = true;
		}
		else
		{
			GCon->Log(NAME_Init, "Symbol not found, disabled.");
			mtexable = false;
		}
	}
	else
	{
		mtexable = false;
	}

	//  Check point parameters extensions
	if (ext_point_parameters && CheckExtension("GL_EXT_point_parameters"))
	{
		GCon->Log(NAME_Init, "Found GL_EXT_point_parameters...");

		p_PointParameterf = PointParameterf_t(GetExtFuncPtr("glPointParameterfEXT"));
		p_PointParameterfv = PointParameterfv_t(GetExtFuncPtr("glPointParameterfvEXT"));
		if (p_PointParameterf && p_PointParameterfv)
		{
			GCon->Log(NAME_Init, "Point parameters extensions found");
			pointparmsable = true;
		}
		else
		{
			GCon->Log(NAME_Init, "Symbol not found, disabled.");
			pointparmsable = false;
		}
	}
	else
	{
		pointparmsable = false;
	}

	//  Anisotropy extension
	max_anisotropy = 1.0;
	if (ext_anisotropy && CheckExtension("GL_EXT_texture_filter_anisotropic"))
	{
		glGetFloatv(GLenum(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT), &max_anisotropy);
		GCon->Logf(NAME_Init, "Max anisotropy %f", max_anisotropy);
	}

	//	Clamp to edge extension
	if (CheckExtension("GL_SGIS_texture_edge_clamp") ||
		CheckExtension("GL_EXT_texture_edge_clamp"))
	{
		GCon->Log(NAME_Init, "Clamp to edge extension found.");
		ClampToEdge = GL_CLAMP_TO_EDGE_SGIS;
	}
	else
	{
		ClampToEdge = GL_CLAMP;
	}

	glClearColor(0.0, 0.0, 0.0, 0.0);	// Black Background
	glClearDepth(1.0);					// Depth Buffer Setup

	glClear(GL_COLOR_BUFFER_BIT);
	Update();
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	GenerateTextures();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc(GL_GREATER, 0.666);
	glShadeModel(GL_FLAT);

	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0, 1.0);

	glDisable(GL_POLYGON_SMOOTH);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::CheckExtension
//
//==========================================================================

bool VOpenGLDrawer::CheckExtension(const char *ext)
{
	guard(VOpenGLDrawer::CheckExtension);
	TArray<VStr> Exts;
	VStr((char*)glGetString(GL_EXTENSIONS)).Split(' ', Exts);
	for (int i = 0; i < Exts.Num(); i++)
	{
		if (Exts[i] == ext)
		{
			return true;
		}
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::NewMap
//
//==========================================================================

void VOpenGLDrawer::NewMap()
{
	FlushCaches(false);
}

//==========================================================================
//
//	VOpenGLDrawer::Setup2D
//
//==========================================================================

void VOpenGLDrawer::Setup2D()
{
	guard(VOpenGLDrawer::Setup2D);
	glViewport(0, 0, ScreenWidth, ScreenHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ScreenWidth, ScreenHeight, 0, -99999, 99999);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_FOG);
	glEnable(GL_ALPHA_TEST);

	glColor4f(1,1,1,1);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::StartUpdate
//
//==========================================================================

void VOpenGLDrawer::StartUpdate()
{
	guard(VOpenGLDrawer::StartUpdate);
	glFinish();
	if (clear)
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	if (tex_linear == 3)
	{
		maxfilter = GL_LINEAR;
		minfilter = GL_LINEAR;
		mipfilter = GL_LINEAR_MIPMAP_LINEAR;
	}
	else if (tex_linear == 2)
	{
		maxfilter = GL_LINEAR;
		minfilter = GL_LINEAR;
		mipfilter = GL_LINEAR_MIPMAP_NEAREST;
	}
	else if (tex_linear)
	{
		maxfilter = GL_LINEAR;
		minfilter = GL_LINEAR;
		mipfilter = GL_LINEAR;
	}
	else
	{
		maxfilter = GL_NEAREST;
		minfilter = GL_NEAREST;
		mipfilter = GL_NEAREST;
	}

	if (usegamma != lastgamma)
	{
		FlushTextures();
		lastgamma = usegamma;
	}

	Setup2D();
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::BeginDirectUpdate
//
//==========================================================================

void VOpenGLDrawer::BeginDirectUpdate()
{
	guard(VOpenGLDrawer::BeginDirectUpdate);
	glFinish();
	glDrawBuffer(GL_FRONT);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::EndDirectUpdate
//
//==========================================================================

void VOpenGLDrawer::EndDirectUpdate()
{
	guard(VOpenGLDrawer::EndDirectUpdate);
	glDrawBuffer(GL_BACK);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::SetupView
//
//==========================================================================

void VOpenGLDrawer::SetupView(const refdef_t *rd)
{
	guard(VOpenGLDrawer::SetupView);
	if (rd->drawworld && rd->width != ScreenWidth)
	{
		// 	Draws the border around the view for different size windows
		R_DrawViewBorder();
	}

	glViewport(rd->x, ScreenHeight - rd->height - rd->y, rd->width, rd->height);

	glClear(GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);		// Select The Projection Matrix
	glLoadIdentity();					// Reset The Projection Matrix

	GLdouble zNear = 1.0;
	GLdouble zFar =	maxdist;

	GLdouble xmax = zNear * rd->fovx;
	GLdouble xmin = -xmax;

	GLdouble ymax = zNear * rd->fovy;
	GLdouble ymin = -ymax;

	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);			// Select The Modelview Matrix

	glLoadIdentity();
	glRotatef(-90, 1, 0, 0);
	glRotatef(90, 0, 0, 1);
	glRotatef(-viewangles.roll, 1, 0, 0);
	glRotatef(-viewangles.pitch, 0, 1, 0);
	glRotatef(-viewangles.yaw, 0, 0, 1);
	glTranslatef(-vieworg.x, -vieworg.y, -vieworg.z);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	if (r_fog)
	{
		static GLenum fogMode[4] = { GL_LINEAR, GL_LINEAR, GL_EXP, GL_EXP2 };
		float fogColour[4] = { r_fog_r, r_fog_g, r_fog_b, 1.0 };

		glFogi(GL_FOG_MODE, fogMode[r_fog & 3]);
		glFogfv(GL_FOG_COLOR, fogColour);
		glFogf(GL_FOG_DENSITY, r_fog_density);
		glHint(GL_FOG_HINT, r_fog < 4 ? GL_DONT_CARE : GL_NICEST);
		glFogf(GL_FOG_START, r_fog_start);
		glFogf(GL_FOG_END, r_fog_end);
		glEnable(GL_FOG);
	}

	if (pointparmsable)
	{
		int shift = 8 - (int)((float)rd->width / 320.0 + 0.5);
		glPointSize(0x8000 >> shift);
	}

	memset(light_chain, 0, sizeof(light_chain));
	memset(add_chain, 0, sizeof(add_chain));
	cacheframecount++;
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::EndView
//
//==========================================================================

void VOpenGLDrawer::EndView()
{
	guard(VOpenGLDrawer::EndView);
	Setup2D();

	cl->CShifts[7] = GClGame->prev_cshifts[7];
	for (int i = 0; i < NUM_CSHIFTS; i++)
	{
		if (!cl->CShifts[i])
		{
			continue;
		}
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		glColor4f((float)((cl->CShifts[i] >> 16) & 0xff) / 255.0,
				(float)((cl->CShifts[i] >> 8) & 0xff) / 255.0,
				(float)(cl->CShifts[i] & 0xff) / 255.0,
				(float)((cl->CShifts[i] >> 24) & 0xff) / 255.0);
		glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(ScreenWidth, 0);
		glVertex2f(ScreenWidth, ScreenHeight);
		glVertex2f(0, ScreenHeight);
		glEnd();

		glDisable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_TEXTURE_2D);
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::ReadScreen
//
//==========================================================================

void *VOpenGLDrawer::ReadScreen(int *bpp, bool *bot2top)
{
	guard(VOpenGLDrawer::ReadScreen);
	void* dst = Z_Malloc(ScreenWidth * ScreenHeight * 3);
	glReadBuffer(GL_FRONT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, ScreenWidth, ScreenHeight, GL_RGB, GL_UNSIGNED_BYTE, dst);
	*bpp = 24;
	*bot2top = true;
	return dst;
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::SetPalette
//
//==========================================================================

void VOpenGLDrawer::SetPalette(int pnum)
{
	guard(VOpenGLDrawer::SetPalette);
	vuint8* pal = r_playpal + 768 * pnum;
	int cmax = MAX(MAX(pal[0], pal[1]), pal[2]);
	if (!cmax)
	{
		cl->CShifts[7] = 0;
	}
	else
	{
		cl->CShifts[7] = (cmax << 24) | ((255 * pal[0] / cmax) << 16) |
			((255 * pal[1] / cmax) << 8) | (255 * pal[2] / cmax);
	}
	GClGame->prev_cshifts[7] = cl->CShifts[7];
	unguard;
}
