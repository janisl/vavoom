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
#include <dlfcn.h>
#include "gl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VOpenGLDrawer::Init
//
// 	Determine the hardware configuration
//
//==========================================================================

void VOpenGLDrawer::Init()
{
}

//==========================================================================
//
// 	VOpenGLDrawer::SetResolution
//
// 	Set up the video mode
//
//==========================================================================

bool VOpenGLDrawer::SetResolution(int Width, int Height, int BPP)
{
	guard(VOpenGLDrawer::SetResolution);
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
	allegro_gl_set(AGL_WINDOWED, !!M_CheckParm("-window"));
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
//	VOpenGLDrawer::GetExtFuncPtr
//
//==========================================================================

void *VOpenGLDrawer::GetExtFuncPtr(const char* name)
{
	guard(VOpenGLDrawer::GetExtFuncPtr);
	void *prjobj = dlopen(NULL, RTLD_LAZY);
	if (!prjobj)
	{
		GCon->Log(NAME_Init, "Unable to open symbol list for main program.");
		return NULL;
	}
	void *ptr = dlsym(prjobj, name);
	dlclose(prjobj);
	return ptr;
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::Update
//
// 	Blit to the screen / Flip surfaces
//
//==========================================================================

void VOpenGLDrawer::Update()
{
	guard(VOpenGLDrawer::Update);
	allegro_gl_flip();
	unguard;
}

//==========================================================================
//
// 	VOpenGLDrawer::Shutdown
//
//	Close the graphics
//
//==========================================================================

void VOpenGLDrawer::Shutdown()
{
	guard(VOpenGLDrawer::Shutdown);
	DeleteTextures();
	set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	remove_allegro_gl();
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2005/03/28 07:26:54  dj_jl
//	New OpenGL driver for Allegro.
//
//**************************************************************************
