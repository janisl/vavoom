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
//**
//**	Functions to draw patches (by post) directly to screen.
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

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VOpenGLDrawer::DrawPic
//
//==========================================================================

void VOpenGLDrawer::DrawPic(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2, int handle, int trans)
{
	guard(VOpenGLDrawer::DrawPic);
	SetPic(handle);
	if (trans)
	{
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glColor4f(1, 1, 1, (100.0 - trans) / 100.0);
	}
	else
	{
		glColor4f(1, 1, 1, 1);
	}
	glBegin(GL_QUADS);
	glTexCoord2f(s1 * tex_iw, t1 * tex_ih);
	glVertex2f(x1, y1);
	glTexCoord2f(s2 * tex_iw, t1 * tex_ih);
	glVertex2f(x2, y1);
	glTexCoord2f(s2 * tex_iw, t2 * tex_ih);
	glVertex2f(x2, y2);
	glTexCoord2f(s1 * tex_iw, t2 * tex_ih);
	glVertex2f(x1, y2);
	glEnd();
	if (trans)
	{
		glDisable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
	}
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawPicShadow
//
//==========================================================================

void VOpenGLDrawer::DrawPicShadow(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2, int handle, int shade)
{
	guard(VOpenGLDrawer::DrawPicShadow);
	SetPic(handle);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glColor4f(0, 0, 0, (float)shade / 255.0);
	glBegin(GL_QUADS);
	glTexCoord2f(s1 * tex_iw, t1 * tex_ih);
	glVertex2f(x1, y1);
	glTexCoord2f(s2 * tex_iw, t1 * tex_ih);
	glVertex2f(x2, y1);
	glTexCoord2f(s2 * tex_iw, t2 * tex_ih);
	glVertex2f(x2, y2);
	glTexCoord2f(s1 * tex_iw, t2 * tex_ih);
	glVertex2f(x1, y2);
	glEnd();
	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	unguard;
}

//==========================================================================
//
//  VOpenGLDrawer::FillRectWithFlat
//
// 	Fills rectangle with flat.
//
//==========================================================================

void VOpenGLDrawer::FillRectWithFlat(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2, const char* fname)
{
	guard(VOpenGLDrawer::FillRectWithFlat);
	SetTexture(GTextureManager.NumForName(VName(fname, VName::AddLower8),
		TEXTYPE_Flat, true, false));

	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);
	glTexCoord2f(s1 * tex_iw, t1 * tex_ih);
	glVertex2f(x1, y1);
	glTexCoord2f(s2 * tex_iw, t1 * tex_ih);
	glVertex2f(x2, y1);
	glTexCoord2f(s2 * tex_iw, t2 * tex_ih);
	glVertex2f(x2, y2);
	glTexCoord2f(s1 * tex_iw, t2 * tex_ih);
	glVertex2f(x1, y2);
	glEnd();
	unguard;
}

//==========================================================================
//
//  VOpenGLDrawer::FillRect
//
//==========================================================================

void VOpenGLDrawer::FillRect(float x1, float y1, float x2, float y2,
	vuint32 color)
{
	guard(VOpenGLDrawer::FillRect);
	SetColor(color);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_ALPHA_TEST);
	glBegin(GL_QUADS);
	glVertex2f(x1, y1);
	glVertex2f(x2, y1);
	glVertex2f(x2, y2);
	glVertex2f(x1, y2);
	glEnd();
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::ShadeRect
//
//  Fade all the screen buffer, so that the menu is more readable,
// especially now that we use the small hufont in the menus...
//
//==========================================================================

void VOpenGLDrawer::ShadeRect(int x, int y, int w, int h, int darkening)
{
	guard(VOpenGLDrawer::ShadeRect);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glColor4f(0, 0, 0, darkening / 32.0);
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + w, y);
	glVertex2f(x + w, y + h);
	glVertex2f(x, y + h);
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawConsoleBackground
//
//==========================================================================

void VOpenGLDrawer::DrawConsoleBackground(int h)
{
	guard(VOpenGLDrawer::DrawConsoleBackground);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glColor4f(0, 0, 0.5, 0.75);
	glBegin(GL_QUADS);
	glVertex2f(0, 0);
	glVertex2f(ScreenWidth, 0);
	glVertex2f(ScreenWidth, h);
	glVertex2f(0, h);
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawSpriteLump
//
//==========================================================================

void VOpenGLDrawer::DrawSpriteLump(float x1, float y1, float x2, float y2,
	int lump, int translation, bool flip)
{
	guard(VOpenGLDrawer::DrawSpriteLump);
	SetSpriteLump(lump, translation);

	VTexture* Tex = GTextureManager.Textures[lump];
	float s1, s2;
	if (flip)
	{
		s1 = Tex->GetWidth() * tex_iw;
		s2 = 0;
	}
	else
	{
		s1 = 0;
		s2 = Tex->GetWidth() * tex_iw;
	}
	float texh = Tex->GetHeight() * tex_ih;

	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);

	glTexCoord2f(s1, 0);
	glVertex2f(x1, y1);
	glTexCoord2f(s2, 0);
	glVertex2f(x2, y1);
	glTexCoord2f(s2, texh);
	glVertex2f(x2, y2);
	glTexCoord2f(s1, texh);
	glVertex2f(x1, y2);

	glEnd();
	unguard;
}


//==========================================================================
//
//	VOpenGLDrawer::StartAutomap
//
//==========================================================================

void VOpenGLDrawer::StartAutomap()
{
	guard(VOpenGLDrawer::StartAutomap);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBegin(GL_LINES);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::DrawLine
//
//==========================================================================

void VOpenGLDrawer::DrawLine(int x1, int y1, vuint32 c1, int x2, int y2,	
	vuint32 c2)
{
	guard(VOpenGLDrawer::DrawLine);
	SetColor(c1);
	glVertex2f(x1, y1);
	SetColor(c2);
	glVertex2f(x2, y2);
	unguard;
}

//==========================================================================
//
//	VOpenGLDrawer::EndAutomap
//
//==========================================================================

void VOpenGLDrawer::EndAutomap()
{
	guard(VOpenGLDrawer::EndAutomap);
	glEnd();
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);
	unguard;
}
