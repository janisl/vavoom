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

// HEADER FILES ------------------------------------------------------------

#include <SDL/SDL.h>
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
	guard(TOpenGLDrawer::SetResolution);
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

	hw_screen = SDL_SetVideoMode(Width, Height, BPP, SDL_OPENGL | SDL_FULLSCREEN);
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
//	TOpenGLDrawer::GetExtFuncPtr
//
//==========================================================================

void *TOpenGLDrawer::GetExtFuncPtr(const char *name)
{
	guard(TOpenGLDrawer::GetExtFuncPtr);
	return SDL_GL_GetProcAddress(name);
	unguard;
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
	guard(TOpenGLDrawer::Update);
	SDL_GL_SwapBuffers();
	unguard;
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
	guard(TOpenGLDrawer::Shutdown);
	if (hw_screen != NULL)
		SDL_FreeSurface(hw_screen);
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2002/01/03 18:39:42  dj_jl
//	Added SDL port
//
//**************************************************************************
