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
//**	OpenGL driver for X-Windows
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <allegro.h>
#include <xalleg.h>
#include <dlfcn.h>
#include "gl_local.h"
#include <GL/glx.h>

#if defined(ALLEGRO_XWINDOWS_WITH_XF86DGA) || \
	defined(ALLEGRO_XWINDOWS_WITH_XF86DGA2)
#define USE_FULLSCREEN
#include <X11/extensions/xf86dga.h>
#endif

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static Display		*RenderDisplay = NULL;
static int			RenderScreen;
static Window		RenderWindow;
static GLXContext	RenderContext = NULL;

#ifdef USE_FULLSCREEN
static bool			vidmode_ext = false;
static XF86VidModeModeInfo	**vidmodes;
static int			num_vidmodes;
static bool			vidmode_active = false;

static bool			dgamouse;
#endif

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

bool VOpenGLDrawer::SetResolution(int InWidth, int InHeight, int InBPP)
{
	guard(VOpenGLDrawer::SetResolution);
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

	int attrib1[] =
	{
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 1,
		None
	};
	int attrib2[] =
	{
		GLX_RGBA,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 1,
		None
	};
	int *attrib = BPP < 24 ? attrib1 : attrib2;
	XSetWindowAttributes attr;
	unsigned long mask;
	Window root;
	XVisualInfo *visinfo;

	XLOCK();

	RenderDisplay = _xwin.display;
	if (!RenderDisplay)
	{
		GCon->Log(NAME_Init, "No display - Allegro X-Windows driver not initialized");
		XUNLOCK();
		return false;
	}

	RenderScreen = _xwin.screen;
	root = RootWindow(RenderDisplay, RenderScreen);

#ifdef USE_FULLSCREEN
	// Get video mode list
	int MajorVersion, MinorVersion;
	MajorVersion = MinorVersion = 0;
	if (!XF86VidModeQueryVersion(RenderDisplay, &MajorVersion, &MinorVersion))
	{
		vidmode_ext = false;
	}
	else
	{
		GCon->Logf(NAME_Init, "Using XFree86-VidModeExtension Version %d.%d",
			MajorVersion, MinorVersion);
		vidmode_ext = true;
	}
#endif

	visinfo = glXChooseVisual(RenderDisplay, RenderScreen, attrib);
	if (!visinfo)
	{
		GCon->Log(NAME_Init, "Failed to choose visual");
		XUNLOCK();
		return false;
	}

#ifdef USE_FULLSCREEN
	if (vidmode_ext && !GArgs.CheckParm("-window"))
	{
		int best_fit;

		XF86VidModeGetAllModeLines(RenderDisplay, RenderScreen, &num_vidmodes, &vidmodes);

		// Let's change video mode
		best_fit = -1;
		int i;

		for (i = 0; i < num_vidmodes; i++)
		{
			if (Width != vidmodes[i]->hdisplay ||
				Height != vidmodes[i]->vdisplay)
				continue;
			best_fit = i;
		}

		if (best_fit != -1)
		{
			// change to the mode
			XF86VidModeSwitchToMode(RenderDisplay, RenderScreen, vidmodes[best_fit]);
			vidmode_active = true;

			// Move the viewport to top left
			XF86VidModeSetViewPort(RenderDisplay, RenderScreen, 0, 0);
		}
		else if (!default_mode)
		{
			// No such mode, if we are trying to set default mode,
			// we will use windowed mode, otherwise complain.
			GCon->Log(NAME_Init, "No such video mode");
			XUNLOCK();
			return false;
		}
	}
#endif

	/* window attributes */
	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(RenderDisplay, root, visinfo->visual, AllocNone);
	attr.event_mask =
		( KeyPressMask | KeyReleaseMask
		| EnterWindowMask | LeaveWindowMask
		| FocusChangeMask | ExposureMask
		| ButtonPressMask | ButtonReleaseMask | PointerMotionMask
		/*| MappingNotifyMask (SubstructureRedirectMask?)*/
	);
#ifdef USE_FULLSCREEN
	if (vidmode_active)
	{
		mask = CWBackPixel | CWColormap | CWSaveUnder | CWBackingStore |
			CWEventMask | CWOverrideRedirect;
		attr.override_redirect = True;
		attr.backing_store = NotUseful;
		attr.save_under = False;
	}
	else
#endif
	{
		mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
	}

	if (_xwin.window)
	{
		XDestroyWindow(_xwin.display, _xwin.window);
		_xwin.window = 0;
	}
	RenderWindow = XCreateWindow(RenderDisplay, root, 0, 0, Width, Height,
						0, visinfo->depth, InputOutput,
						visinfo->visual, mask, &attr);
	_xwin.window = RenderWindow;
	// Set title.
	XStoreName(RenderDisplay, RenderWindow, "Vavoom");
	XMapWindow(RenderDisplay, RenderWindow);

#ifdef USE_FULLSCREEN
	if (vidmode_active)
	{
		XMoveWindow(RenderDisplay, RenderWindow, 0, 0);
		XRaiseWindow(RenderDisplay, RenderWindow);
		XWarpPointer(RenderDisplay, None, RenderWindow, 0, 0, 0, 0, 0, 0);
		XFlush(RenderDisplay);
		// Move the viewport to top left
		XF86VidModeSetViewPort(RenderDisplay, RenderScreen, 0, 0);
	}
#endif

	XFlush(RenderDisplay);

	RenderContext = glXCreateContext(RenderDisplay, visinfo, NULL, True);
	if (!RenderContext)
	{
		GCon->Log(NAME_Dev, "Failed to create OpenGL context");
		XUNLOCK();
		return false;
	}
	glXMakeCurrent(RenderDisplay, RenderWindow, RenderContext);

	// Create invisible X cursor.
	Pixmap cursormask;
	XGCValues xgc;
	GC gc;
	XColor dummycolour;
	Cursor cursor;

	cursormask = XCreatePixmap(RenderDisplay, RenderWindow, 1, 1, 1);
	if (cursormask)
	{
		xgc.function = GXclear;
		gc = XCreateGC(RenderDisplay, cursormask, GCFunction, &xgc);
		XFillRectangle(RenderDisplay, cursormask, gc, 0, 0, 1, 1);
		XFreeGC(RenderDisplay, gc);
		dummycolour.pixel = 0;
		dummycolour.red = 0;
		dummycolour.flags = 04;
		cursor = XCreatePixmapCursor(RenderDisplay, cursormask, cursormask,
			&dummycolour, &dummycolour, 0, 0);
		XFreePixmap(RenderDisplay, cursormask);
		XDefineCursor(RenderDisplay, RenderWindow, cursor);
	}

#ifdef USE_FULLSCREEN
	if (XF86DGAQueryVersion(RenderDisplay, &MajorVersion, &MinorVersion))
	{
		dgamouse = true;
		_xwin.in_dga_mode = 2;
		XF86DGADirectVideo(RenderDisplay, RenderScreen, XF86DGADirectMouse);
		XWarpPointer(RenderDisplay, None, RenderWindow, 0, 0, 0, 0, 0, 0);
	}
	else
	{
		// unable to query, probalby not supported
		GCon->Log(NAME_Init, "Failed to detect XF86DGA Mouse");
		XWarpPointer(RenderDisplay, None, RenderWindow,
			0, 0, 0, 0, Width / 2, Height / 2);
	}
#else
	XWarpPointer(RenderDisplay, None, RenderWindow,
		0, 0, 0, 0, Width / 2, Height / 2);
#endif

#ifdef USE_FULLSCREEN
	if (vidmode_active)
	{
		if (XGrabPointer(RenderDisplay, RenderWindow,
			True,
			0,
			GrabModeAsync, GrabModeAsync,
			RenderWindow,
			None,
			CurrentTime) != GrabSuccess)
		{
			GCon->Log(NAME_Init, "Failed to grab mouse");
			return false;
		}
		_xwin.mouse_grabbed = 1;

		if (XGrabKeyboard(RenderDisplay, RenderWindow, False,
			GrabModeAsync, GrabModeAsync, CurrentTime) != GrabSuccess)
		{
			GCon->Log(NAME_Init, "Failed to grab keyboard");
			return false;
		}
		_xwin.keyboard_grabbed = 1;
	}
#endif

	ScreenWidth = Width;
	ScreenHeight = Height;
	ScreenBPP = BPP;

	XUNLOCK();

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
	glFlush();
	glXSwapBuffers(RenderDisplay, RenderWindow);
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
	XLOCK();

	DeleteTextures();
	if (RenderDisplay)
	{
		if (RenderContext)
		{
			glXDestroyContext(RenderDisplay, RenderContext);
			RenderContext = NULL;
		}
		if (RenderWindow)
		{
#ifdef USE_FULLSCREEN
			if (dgamouse)
			{
				dgamouse = false;
				_xwin.in_dga_mode = 0;
				XF86DGADirectVideo(RenderDisplay, RenderScreen, 0);
			}
#endif
			XUndefineCursor(RenderDisplay, RenderWindow);
//			XDestroyWindow(RenderDisplay, RenderWindow);
			RenderWindow = 0;
		}
		if (_xwin.mouse_grabbed)
		{
			XUngrabPointer(_xwin.display, CurrentTime);
			_xwin.mouse_grabbed = 0;
		}
		if (_xwin.keyboard_grabbed)
		{
			XUngrabKeyboard(_xwin.display, CurrentTime);
			_xwin.keyboard_grabbed = 0;
		}
#ifdef USE_FULLSCREEN
		if (vidmode_active)
		{
			XF86VidModeSwitchToMode(RenderDisplay, RenderScreen, vidmodes[0]);
			vidmode_active = false;
		}
#endif
		RenderDisplay = NULL;
	}

	XUNLOCK();
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.14  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
//	Revision 1.13  2005/04/28 07:16:15  dj_jl
//	Fixed some warnings, other minor fixes.
//	
//	Revision 1.12  2005/03/28 07:26:54  dj_jl
//	New OpenGL driver for Allegro.
//	
//	Revision 1.11  2004/11/22 07:34:06  dj_jl
//	Updated to match latest Allegro.
//	
//	Revision 1.10  2002/07/13 07:38:00  dj_jl
//	Added drawers to the object tree.
//	
//	Revision 1.9  2002/01/11 08:12:01  dj_jl
//	Added guard macros
//	
//	Revision 1.8  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.7  2001/09/20 16:22:51  dj_jl
//	Removed workarounds for Allegro bugs that are now fixed
//	
//	Revision 1.6  2001/08/23 17:46:18  dj_jl
//	Better integrity with Allegro, fixed crashes on exit, mouse
//	
//	Revision 1.5  2001/08/17 17:43:40  dj_jl
//	LINUX fixes
//
//	Revision 1.4  2001/08/04 17:32:04  dj_jl
//	Added support for multitexture extensions
//
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
