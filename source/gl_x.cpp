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

#ifdef ALLEGRO_XWINDOWS_WITH_XF86DGA
#define USE_FULLSCREEN
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
		cond << "Attempt to set a paletized video mode for OpenGL\n";
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
		cond << "No display - Allegro X-Windows driver not initialized\n";
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
		con << "Using XFree86-VidModeExtension Version " << MajorVersion << '.' << MinorVersion << endl;
		vidmode_ext = true;
	}
#endif

	visinfo = glXChooseVisual(RenderDisplay, RenderScreen, attrib);
	if (!visinfo)
	{
		cond << "Failed to choose visual\n";
		XUNLOCK();
		return false;
	}

#ifdef USE_FULLSCREEN
	if (vidmode_ext)
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
			cond << "No such video mode\n";
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
		cond << "Failed to create OpenGL context\n";
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

	if (XGrabPointer(RenderDisplay, RenderWindow,
				 True,
				 0,
				 GrabModeAsync, GrabModeAsync,
				 RenderWindow,
				 None,
				 CurrentTime) != GrabSuccess)
	{
		con << "Failed to grab mouse\n";
		return false;
	}
	_xwin.mouse_grabbed = 1;

#ifdef USE_FULLSCREEN
	// Allow workaround for buggy servers (e.g. 3dfx Voodoo 3/Banshee).
	if (get_config_int(NULL, "dga_mouse", 1) == 0)
		_xwin.disable_dga_mouse = 1;

	if (!_xwin.disable_dga_mouse)
	{
		if (!XF86DGAQueryVersion(RenderDisplay, &MajorVersion, &MinorVersion))
		{
			// unable to query, probalby not supported
			con << "Failed to detect XF86DGA Mouse\n";
			XWarpPointer(RenderDisplay, None, RenderWindow,
				0, 0, 0, 0, Width / 2, Height / 2);
		}
		else
		{
			dgamouse = true;
			_xwin.in_dga_mode = 2;
			XF86DGADirectVideo(RenderDisplay, RenderScreen, XF86DGADirectMouse);
			XWarpPointer(RenderDisplay, None, RenderWindow, 0, 0, 0, 0, 0, 0);
		}
	}
	else
#endif
	{
		XWarpPointer(RenderDisplay, None, RenderWindow,
			0, 0, 0, 0, Width / 2, Height / 2);
	}

	if (XGrabKeyboard(RenderDisplay, RenderWindow, False,
		GrabModeAsync, GrabModeAsync, CurrentTime) != GrabSuccess)
	{
		con << "Failed to grab keyboard\n";
		return false;
	}
	_xwin.keyboard_grabbed = 1;

	ScreenWidth = Width;
	ScreenHeight = Height;
	ScreenBPP = BPP;

	XUNLOCK();

	return true;
}

//==========================================================================
//
//	TOpenGLDrawer::GetExtFuncPtr
//
//==========================================================================

void *TOpenGLDrawer::GetExtFuncPtr(const char *name)
{
	void *prjobj = dlopen(NULL, RTLD_LAZY);
	if (!prjobj)
	{
		con << "Unable to open symbol list for main program.\n";
		return NULL;
	}
	void *ptr = dlsym(prjobj, name);
	dlclose(prjobj);
	return ptr;
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
	glXSwapBuffers(RenderDisplay, RenderWindow);
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
}

//**************************************************************************
//
//	$Log$
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
