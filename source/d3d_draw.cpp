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

void TDirect3DDrawer::DrawPic(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2, int handle, int trans)
{
	MyD3DVertex	dv[4];
	int l = (((100 - trans) * 255 / 100) << 24) | 0xffffff;

	SetPic(handle);

	dv[0] = MyD3DVertex(x1, y1, l, s1 * tex_iw, t1 * tex_ih);
	dv[1] = MyD3DVertex(x2, y1, l, s2 * tex_iw, t1 * tex_ih);
	dv[2] = MyD3DVertex(x2, y2, l, s2 * tex_iw, t2 * tex_ih);
	dv[3] = MyD3DVertex(x1, y2, l, s1 * tex_iw, t2 * tex_ih);

	if (trans)
	{
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	}

#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, dv, 4, 0);
#endif

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

void TDirect3DDrawer::DrawPicShadow(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2, int handle, int shade)
{
	MyD3DVertex	dv[4];
	int l = shade << 24;

	SetPic(handle);

	dv[0] = MyD3DVertex(x1, y1, l, s1 * tex_iw, t1 * tex_ih);
	dv[1] = MyD3DVertex(x2, y1, l, s2 * tex_iw, t1 * tex_ih);
	dv[2] = MyD3DVertex(x2, y2, l, s2 * tex_iw, t2 * tex_ih);
	dv[3] = MyD3DVertex(x1, y2, l, s1 * tex_iw, t2 * tex_ih);

	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, dv, 4, 0);
#endif

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

void TDirect3DDrawer::FillRectWithFlat(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2, const char* fname)
{
	MyD3DVertex	dv[4];
	int l = 0xffffffff;

	SetFlat(R_FlatNumForName(fname));

	dv[0] = MyD3DVertex(x1, y1, l, s1 * tex_iw, t1 * tex_ih);
	dv[1] = MyD3DVertex(x2, y1, l, s2 * tex_iw, t1 * tex_ih);
	dv[2] = MyD3DVertex(x2, y2, l, s2 * tex_iw, t2 * tex_ih);
	dv[3] = MyD3DVertex(x1, y2, l, s1 * tex_iw, t2 * tex_ih);

#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, dv, 4, 0);
#endif
}

//==========================================================================
//
//  TDirect3DDrawer::FillRect
//
//==========================================================================

void TDirect3DDrawer::FillRect(float x1, float y1, float x2, float y2,
	dword color)
{
	MyD3DVertex	dv[4];

	dv[0] = MyD3DVertex(x1, y1, color, 0, 0);
	dv[1] = MyD3DVertex(x2, y1, color, 0, 0);
	dv[2] = MyD3DVertex(x2, y2, color, 0, 0);
	dv[3] = MyD3DVertex(x1, y2, color, 0, 0);

	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, dv, 4, 0);
#endif
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
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
	MyD3DVertex	dv[4];
	int l = darkening << 27;

	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

	dv[0] = MyD3DVertex(x, y, l, 0, 0);
	dv[1] = MyD3DVertex(x + w, y, l, 0, 0);
	dv[2] = MyD3DVertex(x + w, y + h, l, 0, 0);
	dv[3] = MyD3DVertex(x, y + h, l, 0, 0);

#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, dv, 4, 0);
#endif

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
	MyD3DVertex	dv[4];
	int l = 0xc000007f;

	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

	dv[0] = MyD3DVertex(0, 0, l, 0, 0);
	dv[1] = MyD3DVertex(ScreenWidth, 0, l, 0, 0);
	dv[2] = MyD3DVertex(ScreenWidth, h, l, 0, 0);
	dv[3] = MyD3DVertex(0, h, l, 0, 0);

#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, dv, 4, 0);
#endif

	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
}

//==========================================================================
//
//	TDirect3DDrawer::DrawSpriteLump
//
//==========================================================================

void TDirect3DDrawer::DrawSpriteLump(float x1, float y1, float x2, float y2,
	int lump, int translation, boolean flip)
{
	SetSpriteLump(lump, translation);

	float s1, s2;
	if (flip)
	{
		s1 = spritewidth[lump] * tex_iw;
		s2 = 0;
	}
	else
	{
		s1 = 0;
		s2 = spritewidth[lump] * tex_iw;
	}
	float texh = spriteheight[lump] * tex_ih;

	MyD3DVertex	dv[4];

	dv[0] = MyD3DVertex(x1, y1, 0xffffffff, s1, 0);
	dv[1] = MyD3DVertex(x2, y1, 0xffffffff, s2, 0);
	dv[2] = MyD3DVertex(x2, y2, 0xffffffff, s2, texh);
	dv[3] = MyD3DVertex(x1, y2, 0xffffffff, s1, texh);

#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, dv, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MYD3D_VERTEX_FORMAT, dv, 4, 0);
#endif
}

//==========================================================================
//
//	TDirect3DDrawer::StartAutomap
//
//==========================================================================

void TDirect3DDrawer::StartAutomap(void)
{
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
	MyD3DVertex out[2];
 	out[0] = MyD3DVertex(x1, y1, c1, 0, 0);
 	out[1] = MyD3DVertex(x2, y2, c2, 0, 0);
#if DIRECT3D_VERSION >= 0x0800
	RenderDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, out, sizeof(MyD3DVertex));
#else
	RenderDevice->DrawPrimitive(D3DPT_LINELIST, MYD3D_VERTEX_FORMAT, out, 2, 0);
#endif
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

//**************************************************************************
//
//	$Log$
//	Revision 1.8  2001/09/14 16:48:22  dj_jl
//	Switched to DirectX 8
//
//	Revision 1.7  2001/09/12 17:31:27  dj_jl
//	Rectangle drawing and direct update for plugins
//	
//	Revision 1.6  2001/08/29 17:49:01  dj_jl
//	Line colors in RGBA format
//	
//	Revision 1.5  2001/08/15 17:15:55  dj_jl
//	Drawer API changes, removed wipes
//	
//	Revision 1.4  2001/08/01 17:33:58  dj_jl
//	Fixed drawing of spite lump for player setup menu, beautification
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
