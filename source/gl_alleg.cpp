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

// HEADER FILES ------------------------------------------------------------

#include "gl_local.h"
extern "C" {
#include <GL/amesa.h>
}

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static AMesaVisual		RenderVisual;
static AMesaBuffer		RenderBuffer;
static AMesaContext		RenderContext = NULL;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TOpenGLDrawer::Init
//
// 	Determine the hardware configuration
//
//==========================================================================

void TOpenGLDrawer::Init(void)
{
}

//==========================================================================
//
// 	TOpenGLDrawer::SetResolution
//
// 	Set up the video mode
//
//==========================================================================

bool TOpenGLDrawer::SetResolution(int Width, int Height, int BPP)
{
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
		//	True-color only
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
}

//==========================================================================
//
//	TOpenGLDrawer::Update
//
// 	Blit to the screen / Flip surfaces
//
//==========================================================================

void TOpenGLDrawer::Update(void)
{
	glFlush();
	AMesaSwapBuffers(RenderBuffer);
}

//==========================================================================
//
// 	TOpenGLDrawer::Shutdown
//
//	Close the graphics
//
//==========================================================================

void TOpenGLDrawer::Shutdown(void)
{
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
}

