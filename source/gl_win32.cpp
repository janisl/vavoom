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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VWin32OpenGLDrawer : public VOpenGLDrawer
{
public:
	bool		Windowed;
	HDC			DeviceContext;
	HGLRC		RenderContext;
	HWND		RenderWindow;

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

IMPLEMENT_DRAWER(VWin32OpenGLDrawer, DRAWER_OpenGL, "OpenGL",
	"Win32 OpenGL rasteriser device", "-opengl");

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VWin32OpenGLDrawer::Init
//
// 	Determine the hardware configuration
//
//==========================================================================

void VWin32OpenGLDrawer::Init()
{
	Windowed = !!GArgs.CheckParm("-window");
	DeviceContext = NULL;
	RenderContext = NULL;
	RenderWindow = NULL;
}

//==========================================================================
//
// 	VWin32OpenGLDrawer::SetResolution
//
// 	Set up the video mode
//
//==========================================================================

bool VWin32OpenGLDrawer::SetResolution(int InWidth, int InHeight, int InBPP)
{
	guard(VWin32OpenGLDrawer::SetResolution);
	int Width = InWidth;
	int Height = InHeight;
	int BPP = InBPP;
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
		//	True-colour only
		return false;
	}

	//	Sut down current mode
	Shutdown();

	if (!Windowed)
	{
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
	}

	//	Create window
	RenderWindow = CreateWindow("VAVOOM", "VAVOOM for Windows'95",
		Windowed ? WS_OVERLAPPEDWINDOW : WS_POPUP,
		0, 0, 2, 2, hwnd, NULL, hInst, NULL);
	if (!RenderWindow)
	{
		GCon->Log(NAME_Init, "Couldn't create window");
		return false;
	}

	//	Make the window visible & update its client area
	ShowWindow(RenderWindow, SW_SHOWDEFAULT);
	UpdateWindow(RenderWindow);

	//	Switch input to this window
	IN_SetActiveWindow(RenderWindow);

	//	Now we try to make sure we get the focus on the mode switch, because
	// sometimes in some systems we don't. We grab the foreground, pump all
	// our messages, and sleep for a little while to let messages finish
	// bouncing around the system, then we put ourselves at the top of the z
	// order, then grab the foreground again.
	//	Who knows if it helps, but it probably doesn't hurt
	SetForegroundWindow(RenderWindow);

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	Sleep(100);

	if (Windowed)
	{
		RECT WindowRect;
		WindowRect.left=(long)0;
		WindowRect.right=(long)Width;
		WindowRect.top=(long)0;
		WindowRect.bottom=(long)Height;
		AdjustWindowRectEx(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE,
			WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
		SetWindowPos(RenderWindow, HWND_TOP, 0, 0,
			WindowRect.right-WindowRect.left,
			WindowRect.bottom-WindowRect.top, SWP_NOMOVE);
	}
	else
	{
		SetWindowPos(RenderWindow, HWND_TOP, 0, 0, Width, Height, SWP_NOMOVE);
	}

	SetForegroundWindow(RenderWindow);

	//	Get device context
	DeviceContext = GetDC(RenderWindow);
	if (!DeviceContext)
	{
		GCon->Log(NAME_Init, "Failed to get device context");
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
		byte(BPP),						// Select Our Colour Depth
		0, 0, 0, 0, 0, 0,				// Colour Bits Ignored
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
		GCon->Log(NAME_Init, "Failed to create context");
		return false;
	}

	//	Make this context current
	if (!wglMakeCurrent(DeviceContext, RenderContext))
	{
		GCon->Log(NAME_Init, "Make current failed");
		return false;
	}

	//	Everything is fine, set some globals and finish
	ScreenWidth = Width;
	ScreenHeight = Height;
	ScreenBPP = BPP;

	return true;
	unguard;
}

//==========================================================================
//
//	VWin32OpenGLDrawer::GetExtFuncPtr
//
//==========================================================================

void* VWin32OpenGLDrawer::GetExtFuncPtr(const char *name)
{
	guard(VWin32OpenGLDrawer::GetExtFuncPtr);
	return (void*)wglGetProcAddress(name);
	unguard;
}

//==========================================================================
//
//	VWin32OpenGLDrawer::Update
//
//	Blit to the screen / Flip surfaces
//
//==========================================================================

void VWin32OpenGLDrawer::Update()
{
	guard(VWin32OpenGLDrawer::Update);
	SwapBuffers(DeviceContext);
	unguard;
}

//==========================================================================
//
//	VWin32OpenGLDrawer::Shutdown
//
//	Close the graphics
//
//==========================================================================

void VWin32OpenGLDrawer::Shutdown()
{
	guard(VWin32OpenGLDrawer::Shutdown);
	DeleteTextures();

	if (RenderContext)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(RenderContext);
		RenderContext = 0;
	}

	if (DeviceContext)
	{
		ReleaseDC(RenderWindow, DeviceContext);
		DeviceContext = 0;
	}

	if (RenderWindow)
	{
		IN_SetActiveWindow(hwnd);
		SetForegroundWindow(hwnd);
		DestroyWindow(RenderWindow);
		RenderWindow = NULL;
	}

	if (!Windowed)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	unguard;
}
