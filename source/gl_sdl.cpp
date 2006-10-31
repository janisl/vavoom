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

#include <SDL.h>
#include "gl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VSdlOpenGLDrawer : public VOpenGLDrawer
{
public:
	SDL_Surface*	hw_screen;

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

IMPLEMENT_DRAWER(VSdlOpenGLDrawer, DRAWER_OpenGL, "OpenGL",
	"SDL OpenGL rasteriser device", "-opengl");

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VSdlOpenGLDrawer::Init
//
// 	Determine the hardware configuration
//
//==========================================================================

void VSdlOpenGLDrawer::Init()
{
	hw_screen = NULL;
}

//==========================================================================
//
//	VSdlOpenGLDrawer::SetResolution
//
//	Set up the video mode
//
//==========================================================================

bool VSdlOpenGLDrawer::SetResolution(int AWidth, int AHeight, int ABPP,
	bool Windowed)
{
	guard(VSdlOpenGLDrawer::SetResolution);
	int Width = AWidth;
	int Height = AHeight;
	int BPP = ABPP;
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
		// True-colour only
		return false;
	}

	// Sut down current mode
	Shutdown();

	Uint32 flags = SDL_OPENGL;
	if (!Windowed)
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
//	VSdlOpenGLDrawer::GetExtFuncPtr
//
//==========================================================================

void* VSdlOpenGLDrawer::GetExtFuncPtr(const char* name)
{
	guard(VSdlOpenGLDrawer::GetExtFuncPtr);
	return SDL_GL_GetProcAddress(name);
	unguard;
}

//==========================================================================
//
//	VSdlOpenGLDrawer::Update
//
//	Blit to the screen / Flip surfaces
//
//==========================================================================

void VSdlOpenGLDrawer::Update()
{
	guard(VSdlOpenGLDrawer::Update);
	SDL_GL_SwapBuffers();
	unguard;
}

//==========================================================================
//
//	VSdlOpenGLDrawer::Shutdown
//
//	Close the graphics
//
//==========================================================================

void VSdlOpenGLDrawer::Shutdown()
{
	guard(VSdlOpenGLDrawer::Shutdown);
	DeleteTextures();
	if (hw_screen != NULL)
		SDL_FreeSurface(hw_screen);
	unguard;
}
