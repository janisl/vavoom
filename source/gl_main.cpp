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

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TOpenGLDrawer	OpenGLDrawer;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TOpenGLDrawer::TOpenGLDrawer
//
//==========================================================================

TOpenGLDrawer::TOpenGLDrawer(void) :
	tex_linear("gl_tex_linear", "2", CVAR_ARCHIVE),
	clear("gl_clear", "0", CVAR_ARCHIVE),
	blend_sprites("gl_blend_sprites", "0", CVAR_ARCHIVE),
	ext_multitexture("gl_ext_multitexture", "1", CVAR_ARCHIVE),
	ext_point_parameters("gl_ext_point_parameters", "1", CVAR_ARCHIVE),
	ext_anisotropy("gl_ext_anisotropy", "1", CVAR_ARCHIVE),
	maxdist("gl_maxdist", "8192.0", CVAR_ARCHIVE),
	model_lighting("gl_model_lighting", "0", CVAR_ARCHIVE)
{
	_OpenGLDrawer = this;
}

//==========================================================================
//
//	TOpenGLDrawer::InitResolution
//
//==========================================================================

void TOpenGLDrawer::InitResolution(void)
{
	con << "GL_VENDOR: " << glGetString(GL_VENDOR) << endl;
	con << "GL_RENDERER: " << glGetString(GL_RENDERER) << endl;
	con << "GL_VERSION: " << glGetString (GL_VERSION) << endl;

	con << "GL_EXTENSIONS:\n";
	char *sbuf = Z_StrDup((char*)glGetString(GL_EXTENSIONS));
	for (char *s = strtok(sbuf, " "); s; s = strtok(NULL, " "))
	{
		con << "- " << s << endl;
	}
	Z_Free(sbuf);

	// Check the maximum texture size.
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
	con << "Maximum texture size: " << maxTexSize << endl;

	//	Check multi-texture extensions
	if (ext_multitexture && CheckExtension("GL_ARB_multitexture"))
	{
		con << "Found GL_ARB_multitexture...\n";

		p_MultiTexCoord2f = MultiTexCoord2f_t(GetExtFuncPtr("glMultiTexCoord2fARB"));
		p_SelectTexture = SelectTexture_t(GetExtFuncPtr("glActiveTextureARB"));

		if (p_MultiTexCoord2f && p_SelectTexture)
		{
			con << "Multitexture extensions found.\n";
			mtexable = true;
		}
		else
		{
			con << "Symbol not found, disabled.\n";
			mtexable = false;
		}
	}
	else
	{
		mtexable = false;
	}
	if (ext_point_parameters && CheckExtension("GL_EXT_point_parameters"))
	{
		con << "Found GL_EXT_point_parameters...\n";

		p_PointParameterf = PointParameterf_t(GetExtFuncPtr("glPointParameterfEXT"));
		p_PointParameterfv = PointParameterfv_t(GetExtFuncPtr("glPointParameterfvEXT"));
		if (p_PointParameterf && p_PointParameterfv)
		{
			con << "Point parameters extensions found\n";
			pointparmsable = true;
		}
		else
		{
			con << "Symbol not found, disabled.\n";
			pointparmsable = false;
		}
	}
	else
	{
		pointparmsable = false;
	}
	if (ext_anisotropy && CheckExtension("GL_EXT_texture_filter_anisotropic"))
	{
		GLfloat		max_anisotropy;

		glGetFloatv(GLenum(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT), &max_anisotropy);
		glTexParameterfv(GL_TEXTURE_2D, GLenum(GL_TEXTURE_MAX_ANISOTROPY_EXT), &max_anisotropy);
		con << "Max anisotropy " << max_anisotropy << endl;
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
}

//==========================================================================
//
//	TOpenGLDrawer::CheckExtension
//
//==========================================================================

bool TOpenGLDrawer::CheckExtension(const char *ext)
{
	char *sbuf = Z_StrDup((char*)glGetString(GL_EXTENSIONS));
	for (char *s = strtok(sbuf, " "); s; s = strtok(NULL, " "))
	{
		if (!strcmp(ext, s))
		{
			Z_Free(sbuf);
			return true;
		}
	}
	Z_Free(sbuf);
	return false;
}

//==========================================================================
//
//	TOpenGLDrawer::NewMap
//
//==========================================================================

void TOpenGLDrawer::NewMap(void)
{
	FlushCaches(false);
}

//==========================================================================
//
//	TOpenGLDrawer::Setup2D
//
//==========================================================================

void TOpenGLDrawer::Setup2D(void)
{
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
}

//==========================================================================
//
//	TOpenGLDrawer::StartUpdate
//
//==========================================================================

void TOpenGLDrawer::StartUpdate(void)
{
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
	Setup2D();
}

//==========================================================================
//
//	TOpenGLDrawer::BeginDirectUpdate
//
//==========================================================================

void TOpenGLDrawer::BeginDirectUpdate(void)
{
	glFinish();
	glDrawBuffer(GL_FRONT);
}

//==========================================================================
//
//	TOpenGLDrawer::EndDirectUpdate
//
//==========================================================================

void TOpenGLDrawer::EndDirectUpdate(void)
{
	glDrawBuffer(GL_BACK);
}

//==========================================================================
//
//	TOpenGLDrawer::SetupView
//
//==========================================================================

void TOpenGLDrawer::SetupView(const refdef_t *rd)
{
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
		float fogColor[4] = { r_fog_r, r_fog_g, r_fog_b, 1.0 };

		glFogi(GL_FOG_MODE, fogMode[r_fog & 3]);
		glFogfv(GL_FOG_COLOR, fogColor);
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
}

//==========================================================================
//
//	TOpenGLDrawer::EndView
//
//==========================================================================

void TOpenGLDrawer::EndView(void)
{
	Setup2D();

	cl.cshifts[7] = cl.prev_cshifts[7];
	for (int i = 0; i < NUM_CSHIFTS; i++)
	{
		if (!cl.cshifts[i])
		{
			continue;
		}
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		glColor4f((float)((cl.cshifts[i] >> 16) & 0xff) / 255.0,
				(float)((cl.cshifts[i] >> 8) & 0xff) / 255.0,
				(float)(cl.cshifts[i] & 0xff) / 255.0,
				(float)((cl.cshifts[i] >> 24) & 0xff) / 255.0);
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
}

//==========================================================================
//
//	TOpenGLDrawer::ReadScreen
//
//==========================================================================

void *TOpenGLDrawer::ReadScreen(int *bpp, bool *bot2top)
{
	void *dst = Z_Malloc(ScreenWidth * ScreenHeight * 3, PU_VIDEO, 0);
	if (!dst)
	{
		return NULL;
	}
	glReadBuffer(GL_FRONT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, ScreenWidth, ScreenHeight, GL_RGB, GL_UNSIGNED_BYTE, dst);
	*bpp = 24;
	*bot2top = true;
	return dst;
}

//==========================================================================
//
//	TOpenGLDrawer::SetPalette
//
//==========================================================================

void TOpenGLDrawer::SetPalette(int pnum)
{
	byte *pal = (byte*)W_CacheLumpName("PLAYPAL", PU_CACHE) + 768 * pnum;
	int cmax = MAX(MAX(pal[0], pal[1]), pal[2]);
	if (!cmax)
	{
		cl.cshifts[7] = 0;
	}
	else
	{
		cl.cshifts[7] = (cmax << 24) | ((255 * pal[0] / cmax) << 16) |
			((255 * pal[1] / cmax) << 8) | (255 * pal[2] / cmax);
	}
	cl.prev_cshifts[7] = cl.cshifts[7];
}

//**************************************************************************
//
//	$Log$
//	Revision 1.13  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//
//	Revision 1.12  2001/10/12 17:28:26  dj_jl
//	Blending of sprite borders
//	
//	Revision 1.11  2001/09/14 16:53:17  dj_jl
//	Added glFinish to direct update
//	
//	Revision 1.10  2001/09/12 17:31:27  dj_jl
//	Rectangle drawing and direct update for plugins
//	
//	Revision 1.9  2001/09/05 12:21:42  dj_jl
//	Release changes
//	
//	Revision 1.8  2001/08/31 17:25:38  dj_jl
//	Anisotropy filtering
//	
//	Revision 1.7  2001/08/23 17:50:15  dj_jl
//	Texture filtering mode set in globals
//	
//	Revision 1.6  2001/08/15 17:15:55  dj_jl
//	Drawer API changes, removed wipes
//	
//	Revision 1.5  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.4  2001/08/04 17:32:04  dj_jl
//	Added support for multitexture extensions
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
