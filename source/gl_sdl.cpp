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

// HEADER FILES ------------------------------------------------------------

#include <SDL.h>
#include "gl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static SDL_Surface	*hw_screen = NULL;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VOpenGLDrawer::Init
//
// 	Determine the hardware configuration
//
//==========================================================================

void VOpenGLDrawer::Init(void)
{
}

//==========================================================================
//
// 	VOpenGLDrawer::SetResolution
//
// 	Set up the video mode
//
//==========================================================================

bool VOpenGLDrawer::SetResolution(int InWidth, int InHeight, int InBPP)
{
	guard(VOpenGLDrawer::SetResolution);
	int Width = InWidth;
	int Height = InHeight;
	int BPP = InBPP;
	if (!Width || !Height)
	{
		// Set defaults
		Width = 640;
		Height = 480;
		BPP = 16;
	}

	if (BPP == 15)
		BPP = 16;

	if (BPP < 16)
	{
		// True-color only
		return false;
	}

	// Sut down current mode
	Shutdown();

	Uint32 flags = SDL_OPENGL;
	if (!M_CheckParm("-window"))
	{
		flags |= SDL_FULLSCREEN;
	}

	hw_screen = SDL_SetVideoMode(Width, Height, BPP, flags);
	if (hw_screen == NULL)
		return false;

	// Everything is fine, set some globals and finish
	ScreenWidth = Width;
	ScreenHeight = Height;
	ScreenBPP = BPP;

	return true;
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::GetExtFuncPtr
//
//==========================================================================

void *VOpenGLDrawer::GetExtFuncPtr(const char *name)
{
	guard(VOpenGLDrawer::GetExtFuncPtr);
	return SDL_GL_GetProcAddress(name);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::Update
//
// 	Blit to the screen / Flip surfaces
//
//==========================================================================

void VOpenGLDrawer::Update(void)
{
	guard(VOpenGLDrawer::Update);
	SDL_GL_SwapBuffers();
	unguard;
}

//==========================================================================
//
// 	VOpenGLDrawer::Shutdown
//
//	Close the graphics
//
//==========================================================================

void VOpenGLDrawer::Shutdown(void)
{
	guard(VOpenGLDrawer::Shutdown);
	if (hw_screen != NULL)
		SDL_FreeSurface(hw_screen);
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2005/04/28 07:16:15  dj_jl
//	Fixed some warnings, other minor fixes.
//
//	Revision 1.4  2004/10/08 12:39:01  dj_jl
//	Added windowing mode.
//	
//	Revision 1.3  2002/07/13 07:38:00  dj_jl
//	Added drawers to the object tree.
//	
//	Revision 1.2  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.1  2002/01/03 18:39:42  dj_jl
//	Added SDL port
//	
//**************************************************************************
