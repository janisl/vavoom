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
//**
//**	OpenGL driver for Allegro and AllegroGL
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <allegro.h>
#include <alleggl.h>
#include "gl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VAllegroOpenGLDrawer : public VOpenGLDrawer
{
public:
	void Init();
	bool SetResolution(int, int, int);
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
//	Determine the hardware configuration
//
//==========================================================================

void VOpenGLDrawer::Init()
{
}

//==========================================================================
//
//	VAllegroOpenGLDrawer::SetResolution
//
//	Set up the video mode
//
//==========================================================================

bool VAllegroOpenGLDrawer::SetResolution(int InWidth, int InHeight, int InBPP)
{
	guard(VAllegroOpenGLDrawer::SetResolution);
	int Width = InWidth;
	int Height = InHeight;
	int BPP = InBPP;
	bool default_mode = false;
	if (!Width || !Height)
	{
		//	Set defaults
		Width = 640;
		Height = 480;
		BPP = 16;
		default_mode = true;
	}

	if (BPP == 15) BPP = 16;

	if (BPP < 16)
	{
		//	True-color only
		GCon->Log(NAME_Init, "Attempt to set a paletized video mode for OpenGL");
		return false;
	}

	//	Shut down current mode
	Shutdown();

	install_allegro_gl();

	allegro_gl_clear_settings();
	allegro_gl_set(AGL_COLOR_DEPTH, BPP);
	allegro_gl_set(AGL_Z_DEPTH, 8);
	allegro_gl_set(AGL_WINDOWED, !!GArgs.CheckParm("-window"));
	allegro_gl_set(AGL_DOUBLEBUFFER, 1);
	allegro_gl_set(AGL_RENDERMETHOD, 1);
	allegro_gl_set(AGL_SUGGEST, AGL_COLOR_DEPTH | AGL_DOUBLEBUFFER
			| AGL_RENDERMETHOD | AGL_Z_DEPTH | AGL_WINDOWED);

	set_color_depth(BPP);
	if (set_gfx_mode(GFX_OPENGL, Width, Height, 0, 0))
	{	
		allegro_message("Error setting OpenGL graphics mode:\n%s\n"
			"Allegro GL error : %s\n",
			allegro_error, allegro_gl_error);
		return false;
	}
	
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

void* VAllegroOpenGLDrawer::GetExtFuncPtr(const char* name)
{
	guard(VAllegroOpenGLDrawer::GetExtFuncPtr);
	return allegro_gl_get_proc_address(name);
	unguard;
}

//==========================================================================
//
//	VAllegroOpenGLDrawer::Update
//
//	Blit to the screen / Flip surfaces
//
//==========================================================================

void VAllegroOpenGLDrawer::Update()
{
	guard(VAllegroOpenGLDrawer::Update);
	allegro_gl_flip();
	unguard;
}

//==========================================================================
//
//	VAllegroOpenGLDrawer::Shutdown
//
//	Close the graphics
//
//==========================================================================

void VAllegroOpenGLDrawer::Shutdown()
{
	guard(VAllegroOpenGLDrawer::Shutdown);
	DeleteTextures();
	set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	remove_allegro_gl();
	unguard;
}
