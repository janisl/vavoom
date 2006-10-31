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

// HEADER FILES ------------------------------------------------------------

#include "gl_local.h"
extern "C" {
#include <GL/amesa.h>
}

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VAllegroOpenGLDrawer : public VOpenGLDrawer
{
public:
	AMesaVisual		RenderVisual;
	AMesaBuffer		RenderBuffer;
	AMesaContext	RenderContext;

	void Init();
	bool SetResolution(int, int, int, bool);
	void* GetExtFuncPtr(const char*);
	void Update();
	void Shutdown();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_DRAWER(VAllegroOpenGLDrawer, DRAWER_OpenGL, "OpenGL",
	"Allegro OpenGL rasteriser device", "-opengl");

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VAllegroOpenGLDrawer::Init
//
// 	Determine the hardware configuration
//
//==========================================================================

void VAllegroOpenGLDrawer::Init()
{
	RenderVisual = NULL;
	RenderBuffer = NULL;
	RenderContext = NULL;
}

//==========================================================================
//
//	VAllegroOpenGLDrawer::SetResolution
//
//	Set up the video mode
//
//==========================================================================

bool VAllegroOpenGLDrawer::SetResolution(int AWidth, int AHeight, int ABPP,
	bool)
{
	guard(VAllegroOpenGLDrawer::SetResolution);
	int Width = AWidth;
	int Height = AHeight;
	int BPP = ABPP;
	if (!Width || !Height)
	{
		//	Set defaults
		Width = 640;
		Height = 480;
		BPP = 16;
	}

	if (BPP == 15) BPP = 16;

	if (BPP < 16)
	{
		//	True-colour only
		return false;
	}

	Shutdown();

	RenderVisual = AMesaCreateVisual(true, BPP, 16, 0, 0);
	if (!RenderVisual)
	{
		return false;
	}
	RenderBuffer = AMesaCreateBuffer(RenderVisual, Width, Height);
	if (!RenderBuffer)
	{
		return false;
	}
	RenderContext = AMesaCreateContext(RenderVisual, NULL);
	if (!RenderContext)
	{
		return false;
	}
	AMesaMakeCurrent(RenderContext, RenderBuffer);

	ScreenWidth = Width;
	ScreenHeight = Height;
	ScreenBPP = BPP;

	return true;
	unguard;
}

//==========================================================================
//
//	VAllegroOpenGLDrawer::GetExtFuncPtr
//
//==========================================================================

void* VAllegroOpenGLDrawer::GetExtFuncPtr(const char*)
{
	return NULL;
}

//==========================================================================
//
//	VAllegroOpenGLDrawer::Update
//
// 	Blit to the screen / Flip surfaces
//
//==========================================================================

void VAllegroOpenGLDrawer::Update()
{
	guard(VAllegroOpenGLDrawer::Update);
	glFlush();
	AMesaSwapBuffers(RenderBuffer);
	unguard;
}

//==========================================================================
//
// 	VAllegroOpenGLDrawer::Shutdown
//
//	Close the graphics
//
//==========================================================================

void VAllegroOpenGLDrawer::Shutdown()
{
	guard(VAllegroOpenGLDrawer::Shutdown);
	DeleteTextures();
	if (RenderContext)
	{
		AMesaDestroyContext(RenderContext);
		RenderContext = NULL;
	}
	if (RenderBuffer)
	{
		AMesaDestroyBuffer(RenderBuffer);
		RenderBuffer = NULL;
	}
	if (RenderVisual)
	{
		AMesaDestroyVisual(RenderVisual);
		RenderVisual = NULL;
	}
	unguard;
}
