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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
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

static HDC			DeviceContext;
static HGLRC		RenderContext;

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

	int pixelformat;

	Shutdown();

	DEVMODE dmScreenSettings;
	memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
	dmScreenSettings.dmSize = sizeof(dmScreenSettings);
	dmScreenSettings.dmPelsWidth = Width;
	dmScreenSettings.dmPelsHeight = Height;
	dmScreenSettings.dmBitsPerPel = BPP;
	dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return false;
	}
	SetWindowPos(hwnd, HWND_TOP, 0, 0, Width, Height, SWP_NOMOVE);

	DeviceContext = GetDC(hwnd);
	if (!DeviceContext)
	{
		con << "Failed to get device context\n";
		return false;
	}

	PIXELFORMATDESCRIPTOR pfd =	// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// Size Of This Pixel Format Descriptor
		1,								// Version Number
		PFD_DRAW_TO_WINDOW |			// Format Must Support Window
		PFD_SUPPORT_OPENGL |			// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,				// Must Support Double Buffering
		PFD_TYPE_RGBA,					// Request An RGBA Format
		BPP,							// Select Our Color Depth
		0, 0, 0, 0, 0, 0,				// Color Bits Ignored
		0,								// No Alpha Buffer
		0,								// Shift Bit Ignored
		0,								// No Accumulation Buffer
		0, 0, 0, 0,						// Accumulation Bits Ignored
		16,								// 16Bit Z-Buffer (Depth Buffer)
		0,								// No Stencil Buffer
		0,								// No Auxiliary Buffer
		PFD_MAIN_PLANE,					// Main Drawing Layer
		0,								// Reserved
		0, 0, 0							// Layer Masks Ignored
	};

	if ((pixelformat = ChoosePixelFormat(DeviceContext, &pfd)) == 0)
	{
		Sys_Error("ChoosePixelFormat failed");
	}

	if (SetPixelFormat(DeviceContext, pixelformat, &pfd) == FALSE)
	{
		Sys_Error("SetPixelFormat failed");
	}

	RenderContext = wglCreateContext(DeviceContext);
	if (!RenderContext)
	{
		con << "Failed to create context\n";
		return false;
	}

	if (!wglMakeCurrent(DeviceContext, RenderContext))
	{
		char* lpMsgBuf;
 
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		    NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		    (LPTSTR) &lpMsgBuf, 0, NULL);

		// Display the string.
		con << "Make current failed: " << lpMsgBuf << endl;

		// Free the buffer.
		LocalFree(lpMsgBuf);

		return false;
	}

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
	SwapBuffers(DeviceContext);
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
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(RenderContext);
		RenderContext = 0;
	}

	if (DeviceContext)
	{
		ReleaseDC(hwnd, DeviceContext);
		DeviceContext = 0;
	}

	ChangeDisplaySettings(NULL, 0);
}

