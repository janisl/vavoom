//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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
	tex_linear("gl_tex_linear", "1", CVAR_ARCHIVE),
	clear("gl_clear", "0", CVAR_ARCHIVE),
	ztrick("gl_ztrick", "0", CVAR_ARCHIVE)
{
	_OpenGLDrawer = this;
}

//==========================================================================
//
//	TOpenGLDrawer::InitResolution
//
//	Calculate image scaling
//
//==========================================================================

void TOpenGLDrawer::InitResolution(void)
{
	con << "GL_VENDOR: " << glGetString(GL_VENDOR) << endl;
	con << "GL_RENDERER: " << glGetString(GL_RENDERER) << endl;
	con << "GL_VERSION: " << glGetString (GL_VERSION) << endl;
	con << "GL_EXTENSIONS: " << glGetString(GL_EXTENSIONS) << endl;

	// Check the maximum texture size.
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
	con << "Maximum texture size: " << maxTexSize << endl;

	glClearColor(0.0, 0.0, 0.0, 0.0);	// Black Background
	glClearDepth(1.0);					// Depth Buffer Setup

	glEnable(GL_TEXTURE_2D);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	GenerateTextures();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc(GL_GREATER, 0.666);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_FLAT);

	float fogColor[4] = { 0.5, 0.5, 0.5, 1.0 };
	glFogi(GL_FOG_MODE, GL_LINEAR);		// Fog Mode
	glFogfv(GL_FOG_COLOR, fogColor);	// Set Fog Color
	glFogf(GL_FOG_DENSITY, 0.5f);		// How Dense Will The Fog Be
	glHint(GL_FOG_HINT, GL_DONT_CARE);	// Fog Hint Value
	glFogf(GL_FOG_START, 1.0f);			// Fog Start Depth
	glFogf(GL_FOG_END, 2048.0f);		// Fog End Depth
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
	float gldepthmin;
	float gldepthmax;

	if (ztrick)
	{
		if (clear)
			glClear(GL_COLOR_BUFFER_BIT);

		trickframe++;
		if (trickframe & 1)
		{
			gldepthmin = 0.0;
			gldepthmax = 0.5;
			glDepthFunc(GL_LEQUAL);
		}
		else
		{
			gldepthmin = 1.0;
			gldepthmax = 0.5;
			glDepthFunc(GL_GEQUAL);
		}
	}
	else
	{
		if (clear)
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		else
			glClear(GL_DEPTH_BUFFER_BIT);
		gldepthmin = 0;
		gldepthmax = 1;
		glDepthFunc(GL_LEQUAL);
	}
	glDepthRange(gldepthmin, gldepthmax);

	Setup2D();
}

//==========================================================================
//
//	TOpenGLDrawer::SetupView
//
//==========================================================================

void TOpenGLDrawer::SetupView(int x, int y, int width, int height, float fovx, float fovy)
{
	view_x = x;
	view_y = ScreenHeight - height - y;
	view_w = width;
	view_h = height;
	view_fovx = fovx;
	view_fovy = fovy;
}

//==========================================================================
//
//	TOpenGLDrawer::SetupFrame
//
//==========================================================================

void TOpenGLDrawer::SetupFrame(void)
{
	if (viewwidth != ScreenWidth)
	{
		// 	Draws the border around the view for different size windows
		R_DrawViewBorder();
	}

	glViewport(view_x, view_y, view_w, view_h);
	
	glMatrixMode(GL_PROJECTION);		// Select The Projection Matrix
	glLoadIdentity();					// Reset The Projection Matrix

	GLdouble zNear = 1.0;
	GLdouble zFar =	8192.0;

	GLdouble xmax = zNear * view_fovx;
	GLdouble xmin = -xmax;

	GLdouble ymax = zNear * view_fovy;
	GLdouble ymin = -ymax;

	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);			// Select The Modelview Matrix

	glLoadIdentity();
	glRotatef(-90, 1, 0, 0);
	glRotatef(90, 0, 0, 1);
	glRotatef(-BAM2DEG(cl.viewangles.roll), 1, 0, 0);
	glRotatef(-BAM2DEG(cl.viewangles.pitch), 0, 1, 0);
	glRotatef(-BAM2DEG(cl.viewangles.yaw), 0, 0, 1);
	glTranslatef(-vieworg.x, -vieworg.y, -vieworg.z);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	if (r_use_fog)
		glEnable(GL_FOG);
	else
		glDisable(GL_FOG);

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
//	TOpenGLDrawer::InitWipe
//
//==========================================================================

bool TOpenGLDrawer::InitWipe(void)
{
	return false;
}

//==========================================================================
//
//	TOpenGLDrawer::DoWipe
//
//==========================================================================

void TOpenGLDrawer::DoWipe(int)
{
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


