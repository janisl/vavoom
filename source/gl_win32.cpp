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
	int			pixelformat;
	MSG			msg;

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

	//	Sut down current mode
	Shutdown();

	//	Try to switch to the new mode
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

	//	Now we try to make sure we get the focus on the mode switch, because
	// sometimes in some systems we don't. We grab the foreground, pump all
	// our messages, and sleep for a little while to let messages finish
	// bouncing around the system, then we put ourselves at the top of the z
	// order, then grab the foreground again.
	//	Who knows if it helps, but it probably doesn't hurt
	SetForegroundWindow(hwnd);

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
      	TranslateMessage(&msg);
      	DispatchMessage(&msg);
	}

	Sleep(100);

	SetWindowPos(hwnd, HWND_TOP, 0, 0, Width, Height, SWP_NOMOVE);

	SetForegroundWindow(hwnd);

	//	Get device context
	DeviceContext = GetDC(hwnd);
	if (!DeviceContext)
	{
		con << "Failed to get device context\n";
		return false;
	}

	//	Because we have set the background brush for the window to NULL
	// (to avoid flickering when re-sizing the window on the desktop), we
	// clear the window to black when created, otherwise it will be
	// empty while Vavoom starts up.
	PatBlt(DeviceContext, 0, 0, Width, Height, BLACKNESS);

	//	Set up pixel format
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
		32,								// 32Bit Z-Buffer (Depth Buffer)
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

	//	Create rendering context
	RenderContext = wglCreateContext(DeviceContext);
	if (!RenderContext)
	{
		con << "Failed to create context\n";
		return false;
	}

	//	Make this context current
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

	//	Everything is fine, set some globals and finish
	ScreenWidth = Width;
	ScreenHeight = Height;
	ScreenBPP = BPP;

	return true;
}

//==========================================================================
//
//	TOpenGLDrawer::GetExtFuncPtr
//
//==========================================================================

void *TOpenGLDrawer::GetExtFuncPtr(const char *name)
{
	return wglGetProcAddress(name);
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

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2001/08/04 17:32:04  dj_jl
//	Added support for multitexture extensions
//
//	Revision 1.4  2001/08/01 17:43:51  dj_jl
//	Some modeset improvements
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
