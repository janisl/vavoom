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
//**	Revision 1.2  2001/07/27 14:27:53  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************
//**
//**	Functions to draw patches (by post) directly to screen.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "d3d_local.h"

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
//	TDirect3DDrawer::DrawPic
//
//==========================================================================

void TDirect3DDrawer::DrawPic(float x1, float x2, float y1, float y2,
	float s1, float s2, float t1, float t2, int handle, int trans)
{
	D3DLVERTEX	dv[4];
	int l = (((100 - trans) * 255 / 100) << 24) | 0xffffff;

	SetPic(handle);

	dv[0] = D3DLVERTEX(D3DVECTOR(x1, y1, 0), l, 0, s1 * tex_iw, t1 * tex_ih);
	dv[1] = D3DLVERTEX(D3DVECTOR(x2, y1, 0), l, 0, s2 * tex_iw, t1 * tex_ih);
	dv[2] = D3DLVERTEX(D3DVECTOR(x2, y2, 0), l, 0, s2 * tex_iw, t2 * tex_ih);
	dv[3] = D3DLVERTEX(D3DVECTOR(x1, y2, 0), l, 0, s1 * tex_iw, t2 * tex_ih);

	if (trans)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	}

	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_LVERTEX, dv, 4, 0);

	if (trans)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	}
}

//==========================================================================
//
//	TDirect3DDrawer::DrawPicShadow
//
//==========================================================================

void TDirect3DDrawer::DrawPicShadow(float x1, float x2, float y1, float y2,
	float s1, float s2, float t1, float t2, int handle, int shade)
{
	D3DLVERTEX	dv[4];
	int l = shade << 24;

	SetPic(handle);

	dv[0] = D3DLVERTEX(D3DVECTOR(x1, y1, 0), l, 0, s1 * tex_iw, t1 * tex_ih);
	dv[1] = D3DLVERTEX(D3DVECTOR(x2, y1, 0), l, 0, s2 * tex_iw, t1 * tex_ih);
	dv[2] = D3DLVERTEX(D3DVECTOR(x2, y2, 0), l, 0, s2 * tex_iw, t2 * tex_ih);
	dv[3] = D3DLVERTEX(D3DVECTOR(x1, y2, 0), l, 0, s1 * tex_iw, t2 * tex_ih);

	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_LVERTEX, dv, 4, 0);

	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
}

//==========================================================================
//
//  TDirect3DDrawer::FillRectWithFlat
//
// 	Fills rectangle with flat.
//
//==========================================================================

void TDirect3DDrawer::FillRectWithFlat(int x, int y, int w, int h, const char* fname)
{
	D3DLVERTEX	dv[4];
	int l = 0xffffffff;

	SetFlat(R_FlatNumForName(fname));

	dv[0] = D3DLVERTEX(D3DVECTOR(x, y, 0), l, 0, 0, 0);
	dv[1] = D3DLVERTEX(D3DVECTOR(x + w, y, 0), l, 0, w / 64.0, 0);
	dv[2] = D3DLVERTEX(D3DVECTOR(x + w, y + h, 0), l, 0, w / 64.0, h / 64.0);
	dv[3] = D3DLVERTEX(D3DVECTOR(x, y + h, 0), l, 0, 0, h / 64.0);

	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_LVERTEX, dv, 4, 0);
}

//==========================================================================
//
//	TDirect3DDrawer::DarkenScreen
//
//  Fade all the screen buffer, so that the menu is more readable,
// especially now that we use the small hufont in the menus...
//
//==========================================================================

void TDirect3DDrawer::ShadeRect(int x, int y, int w, int h, int darkening)
{
	D3DLVERTEX	dv[4];
	int l = darkening << 27;

	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

	dv[0] = D3DLVERTEX(D3DVECTOR(x, y, 0), l, 0, 0, 0);
	dv[1] = D3DLVERTEX(D3DVECTOR(x + w, y, 0), l, 0, 0, 0);
	dv[2] = D3DLVERTEX(D3DVECTOR(x + w, y + h, 0), l, 0, 0, 0);
	dv[3] = D3DLVERTEX(D3DVECTOR(x, y + h, 0), l, 0, 0, 0);

	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_LVERTEX, dv, 4, 0);

	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
}

//==========================================================================
//
//	TDirect3DDrawer::DrawConsoleBackground
//
//==========================================================================

void TDirect3DDrawer::DrawConsoleBackground(int h)
{
	D3DLVERTEX	dv[4];
	int l = 0xc000007f;

	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

	dv[0] = D3DLVERTEX(D3DVECTOR(0, 0, 0), l, 0, 0, 0);
	dv[1] = D3DLVERTEX(D3DVECTOR(ScreenWidth, 0, 0), l, 0, 0, 0);
	dv[2] = D3DLVERTEX(D3DVECTOR(ScreenWidth, h, 0), l, 0, 0, 0);
	dv[3] = D3DLVERTEX(D3DVECTOR(0, h, 0), l, 0, 0, 0);

	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_LVERTEX, dv, 4, 0);

	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
}

//==========================================================================
//
//	TDirect3DDrawer::DrawSpriteLump
//
//==========================================================================

void TDirect3DDrawer::DrawSpriteLump(int x, int y, int lump, int translation, boolean flip)
{
	SetSpriteLump(lump, translation);

	x -= (int)spriteoffset[lump];
	y -= (int)spritetopoffset[lump];
	float w = spritewidth[lump];
	float h = spriteheight[lump];
	float texw = w * tex_iw;
	float texh = h * tex_ih;

	D3DLVERTEX	dv[4];
	int l = 0xffffffff;

	if (flip)
	{
		dv[0] = D3DLVERTEX(D3DVECTOR(x, y, 0), l, 0, texw, 0);
		dv[1] = D3DLVERTEX(D3DVECTOR(x + w, y, 0), l, 0, 0, 0);
		dv[2] = D3DLVERTEX(D3DVECTOR(x + w, y + h, 0), l, 0, 0, texh);
		dv[3] = D3DLVERTEX(D3DVECTOR(x, y + h, 0), l, 0, texw, texh);
	}
	else
	{
		dv[0] = D3DLVERTEX(D3DVECTOR(x, y, 0), l, 0, 0, 0);
		dv[1] = D3DLVERTEX(D3DVECTOR(x + w, y, 0), l, 0, texw, 0);
		dv[2] = D3DLVERTEX(D3DVECTOR(x + w, y + h, 0), l, 0, texw, texh);
		dv[3] = D3DLVERTEX(D3DVECTOR(x, y + h, 0), l, 0, 0, texh);
	}

	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_LVERTEX, dv, 4, 0);
}

//==========================================================================
//
//	TDirect3DDrawer::StartAutomap
//
//==========================================================================

static rgb_t	*pal2rgb = NULL;
void TDirect3DDrawer::StartAutomap(void)
{
	if (!pal2rgb)
	{
		pal2rgb = (rgb_t*)Z_Malloc(256 * 3);
		memcpy(pal2rgb, W_CacheLumpName("PLAYPAL", PU_CACHE), 256 * 3);
	}
	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
}

//==========================================================================
//
//	TDirect3DDrawer::DrawLine
//
//==========================================================================

void TDirect3DDrawer::DrawLine(int x1, int y1, dword c1, int x2, int y2, dword c2)
{
	struct
	{
		TVec		origin;
		dword		color;
	} out[2];
 	out[0].origin = TVec(x1, y1, 0);
	out[0].color =	0xff000000 | (pal2rgb[c1].r << 16) | (pal2rgb[c1].g << 8) | pal2rgb[c1].b;
 	out[1].origin = TVec(x2, y2, 0);
	out[1].color =	0xff000000 | (pal2rgb[c2].r << 16) | (pal2rgb[c2].g << 8) | pal2rgb[c2].b;
	RenderDevice->DrawPrimitive(D3DPT_LINELIST, D3DFVF_XYZ | D3DFVF_DIFFUSE, out, 2, 0);
}

//==========================================================================
//
//	TDirect3DDrawer::EndAutomap
//
//==========================================================================

void TDirect3DDrawer::EndAutomap(void)
{
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
}

