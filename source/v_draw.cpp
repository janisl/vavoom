//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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
//**
//**	Functions to draw patches (by post) directly to screen.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "cl_local.h"

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
//	V_Init
//
//==========================================================================

void V_Init(void)
{
	if (M_CheckParm("-d3d"))
	{
		Drawer = _Direct3DDrawer;
		if (!Drawer)
		{
			Sys_Error("Direct3D drawer is not available");
		}
	}
	else if (M_CheckParm("-opengl"))
	{
		Drawer = _OpenGLDrawer;
		if (!Drawer)
		{
			Sys_Error("OpenGL drawer is not available");
		}
	}
	else
	{
		Drawer = _SoftwareDrawer;
		if (!Drawer)
		{
			Sys_Error("Software drawer is not available");
		}
	}
	Drawer->Init();
}

//==========================================================================
//
//	V_Shutdown
//
//==========================================================================

void V_Shutdown(void)
{
	if (Drawer)
	{
		Drawer->Shutdown();
		Drawer = NULL;
	}
}

//==========================================================================
//
//  R_FillRectWithFlat
//
// 	Fills rectangle with flat.
//
//==========================================================================

void R_FillRectWithFlat(int DestX, int DestY, int width, int height, const char* fname)
{
	Drawer->FillRectWithFlat((int)(fScaleX * DestX), (int)(fScaleY * DestY),
		(int)(fScaleX * width), (int)(fScaleY * height), fname);
}

//==========================================================================
//
//	V_DarkenScreen
//
//  Fade all the screen buffer, so that the menu is more readable,
// especially now that we use the small hufont in the menus...
//
//==========================================================================

void V_DarkenScreen(int darkening)
{
	Drawer->ShadeRect(0, 0, ScreenWidth, ScreenHeight, darkening);
}


