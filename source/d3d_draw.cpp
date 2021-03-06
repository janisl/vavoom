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
//	VDirect3DDrawer::DrawPic
//
//==========================================================================

void VDirect3DDrawer::DrawPic(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2, VTexture* Tex,
	VTextureTranslation* Trans, float Alpha)
{
	guard(VDirect3DDrawer::DrawPic);
	MyD3DVertex	dv[4];
	int l = ((int)(Alpha * 255) << 24) | 0xffffff;

	SetPic(Tex, Trans, CM_Default);

	dv[0] = MyD3DVertex(x1, y1, l, s1 * tex_iw, t1 * tex_ih);
	dv[1] = MyD3DVertex(x2, y1, l, s2 * tex_iw, t1 * tex_ih);
	dv[2] = MyD3DVertex(x2, y2, l, s2 * tex_iw, t2 * tex_ih);
	dv[3] = MyD3DVertex(x1, y2, l, s1 * tex_iw, t2 * tex_ih);

	if (Alpha < 1.0)
	{
		RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	}

	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));

	if (Alpha < 1.0)
	{
		RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawPicShadow
//
//==========================================================================

void VDirect3DDrawer::DrawPicShadow(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2, VTexture* Tex, float shade)
{
	guard(VDirect3DDrawer::DrawPicShadow);
	MyD3DVertex	dv[4];
	int l = (int)(shade * 255) << 24;

	SetPic(Tex, NULL, CM_Default);

	dv[0] = MyD3DVertex(x1, y1, l, s1 * tex_iw, t1 * tex_ih);
	dv[1] = MyD3DVertex(x2, y1, l, s2 * tex_iw, t1 * tex_ih);
	dv[2] = MyD3DVertex(x2, y2, l, s2 * tex_iw, t2 * tex_ih);
	dv[3] = MyD3DVertex(x1, y2, l, s1 * tex_iw, t2 * tex_ih);

	RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));

	RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	unguard;
}

//==========================================================================
//
//  VDirect3DDrawer::FillRectWithFlat
//
// 	Fills rectangle with flat.
//
//==========================================================================

void VDirect3DDrawer::FillRectWithFlat(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2, VTexture* Tex)
{
	guard(VDirect3DDrawer::FillRectWithFlat);
	MyD3DVertex	dv[4];
	int l = 0xffffffff;

	SetTexture(Tex, CM_Default);

	dv[0] = MyD3DVertex(x1, y1, l, s1 * tex_iw, t1 * tex_ih);
	dv[1] = MyD3DVertex(x2, y1, l, s2 * tex_iw, t1 * tex_ih);
	dv[2] = MyD3DVertex(x2, y2, l, s2 * tex_iw, t2 * tex_ih);
	dv[3] = MyD3DVertex(x1, y2, l, s1 * tex_iw, t2 * tex_ih);

	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));
	unguard;
}

//==========================================================================
//
//  VDirect3DDrawer::FillRect
//
//==========================================================================

void VDirect3DDrawer::FillRect(float x1, float y1, float x2, float y2,
	vuint32 colour)
{
	guard(VDirect3DDrawer::FillRect);
	MyD3DVertex	dv[4];

	dv[0] = MyD3DVertex(x1, y1, colour, 0, 0);
	dv[1] = MyD3DVertex(x2, y1, colour, 0, 0);
	dv[2] = MyD3DVertex(x2, y2, colour, 0, 0);
	dv[3] = MyD3DVertex(x1, y2, colour, 0, 0);

	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::ShadeRect
//
//  Fade all the screen buffer, so that the menu is more readable,
// especially now that we use the small hufont in the menus...
//
//==========================================================================

void VDirect3DDrawer::ShadeRect(int x, int y, int w, int h, float darkening)
{
	guard(VDirect3DDrawer::ShadeRect);
	MyD3DVertex	dv[4];
	int l = (int)(darkening * 255) << 24;

	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	dv[0] = MyD3DVertex(x, y, l, 0, 0);
	dv[1] = MyD3DVertex(x + w, y, l, 0, 0);
	dv[2] = MyD3DVertex(x + w, y + h, l, 0, 0);
	dv[3] = MyD3DVertex(x, y + h, l, 0, 0);

	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));

	RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawConsoleBackground
//
//==========================================================================

void VDirect3DDrawer::DrawConsoleBackground(int h)
{
	guard(VDirect3DDrawer::DrawConsoleBackground);
	MyD3DVertex	dv[4];
	int l = 0xc000007f;

	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	dv[0] = MyD3DVertex(0, 0, l, 0, 0);
	dv[1] = MyD3DVertex(ScreenWidth, 0, l, 0, 0);
	dv[2] = MyD3DVertex(ScreenWidth, h, l, 0, 0);
	dv[3] = MyD3DVertex(0, h, l, 0, 0);

	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));

	RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawSpriteLump
//
//==========================================================================

void VDirect3DDrawer::DrawSpriteLump(float x1, float y1, float x2, float y2,
	VTexture* Tex, VTextureTranslation* Translation, bool flip)
{
	guard(VDirect3DDrawer::DrawSpriteLump);
	SetSpriteLump(Tex, Translation, CM_Default);

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

	MyD3DVertex	dv[4];

	dv[0] = MyD3DVertex(x1, y1, 0xffffffff, s1, 0);
	dv[1] = MyD3DVertex(x2, y1, 0xffffffff, s2, 0);
	dv[2] = MyD3DVertex(x2, y2, 0xffffffff, s2, texh);
	dv[3] = MyD3DVertex(x1, y2, 0xffffffff, s1, texh);

	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::StartAutomap
//
//==========================================================================

void VDirect3DDrawer::StartAutomap()
{
	guard(VDirect3DDrawer::StartAutomap);
	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawLine
//
//==========================================================================

void VDirect3DDrawer::DrawLine(int x1, int y1, vuint32 c1, int x2, int y2,
	vuint32 c2)
{
	guard(VDirect3DDrawer::DrawLine);
	MyD3DVertex out[2];
 	out[0] = MyD3DVertex(x1, y1, c1, 0, 0);
 	out[1] = MyD3DVertex(x2, y2, c2, 0, 0);
	RenderDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, out, sizeof(MyD3DVertex));
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::EndAutomap
//
//==========================================================================

void VDirect3DDrawer::EndAutomap()
{
	guard(VDirect3DDrawer::EndAutomap);
	RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	unguard;
}
