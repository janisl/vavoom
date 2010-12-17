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

#include "d3d_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static byte ptex[8][8] =
{
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0 },
	{ 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	MatrixMultiply
//
//==========================================================================

void MatrixMultiply(MyD3DMatrix &out, const MyD3DMatrix& a, const MyD3DMatrix& b)
{
	MyD3DMatrix ret;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			ret(i, j) = 0.0f;
			for (int k = 0; k < 4; k++)
			{
				ret(i, j) += a(i, k) * b(k, j);
			}
		}
	}
	out = ret;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawPolygon
//
//==========================================================================

void VDirect3DDrawer::DrawPolygon(surface_t* surf, int)
{
	guard(VDirect3DDrawer::DrawPolygon);
	bool lightmaped = surf->lightmap != NULL ||
		surf->dlightframe == r_dlightframecount;

	if (lightmaped)
	{
		RendLev->CacheSurface(surf);
		if (maxMultiTex >= 2)
		{
			return;
		}
	}

	RendLev->QueueSimpleSurf(surf);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::WorldDrawing
//
//==========================================================================

void VDirect3DDrawer::WorldDrawing()
{
	guard(VDirect3DDrawer::WorldDrawing);
	MyD3DVertex		out[256];
	int				lb, i;
	surfcache_t		*cache;
	float			s, t, lights, lightt;
	surface_t		*surf;
	texinfo_t		*tex;
	vuint32			light;

	//	First draw horizons.
	if (RendLev->HorizonPortalsHead)
	{
		for (surf = RendLev->HorizonPortalsHead; surf; surf = surf->DrawNext)
		{
			DoHorizonPolygon(surf);
		}
	}

	//	For sky areas we just write to the depth buffer to prevent drawing
	// polygons behind the sky.
	if (RendLev->SkyPortalsHead)
	{
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
		RenderDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
		for (surf = RendLev->SkyPortalsHead; surf; surf = surf->DrawNext)
		{
			for (i = 0; i < surf->count; i++)
			{
				out[i] = MyD3DVertex(surf->verts[i], 0, 0, 0);
			}
			RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
		}
        RenderDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0f);
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	}

	//	Draw surfaces.
	if (RendLev->SimpleSurfsHead)
	{
		for (surf = RendLev->SimpleSurfsHead; surf; surf = surf->DrawNext)
		{
			texinfo_t *tex = surf->texinfo;
			SetTexture(tex->Tex, tex->ColourMap);

			if (surf->lightmap != NULL ||
				surf->dlightframe == r_dlightframecount)
			{
				light = 0xffffffff;
			}
			else
			{
				int lev = surf->Light >> 24;
				int lR = ((surf->Light >> 16) & 255) * lev / 255;
				int lG = ((surf->Light >> 8) & 255) * lev / 255;
				int lB = (surf->Light & 255) * lev / 255;
				light = 0xff000000 | (lR << 16) | (lG << 8) | lB;
			}
			SetFade(surf->Fade);

			for (i = 0; i < surf->count; i++)
			{
				out[i] = MyD3DVertex(surf->verts[i], light,
					(DotProduct(surf->verts[i], tex->saxis) + tex->soffs) * tex_iw,
					(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
			}

			RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
		}
	}

	if (maxMultiTex >= 2)
	{
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		RenderDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
		RenderDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		RenderDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

		for (lb = 0; lb < NUM_BLOCK_SURFS; lb++)
		{
			if (!RendLev->light_chain[lb])
			{
				continue;
			}

			if (!light_surf[lb])
			{
				light_surf[lb] = CreateSurface(BLOCK_WIDTH, BLOCK_HEIGHT, 16, false);
				RendLev->block_changed[lb] = true;
			}
			if (RendLev->block_changed[lb])
			{
				RendLev->block_changed[lb] = false;
				UploadTextureImage(light_surf[lb], 0, BLOCK_WIDTH, BLOCK_HEIGHT, RendLev->light_block[lb]);
			}

			RenderDevice->SetTexture(1, light_surf[lb]);

			for (cache = RendLev->light_chain[lb]; cache; cache = cache->chain)
			{
				surf = cache->surf;
				tex = surf->texinfo;
				SetTexture(tex->Tex, tex->ColourMap);
				SetFade(surf->Fade);
				for (i = 0; i < surf->count; i++)
				{
					s = DotProduct(surf->verts[i], tex->saxis) + tex->soffs;
					t = DotProduct(surf->verts[i], tex->taxis) + tex->toffs;
					lights = (s - surf->texturemins[0]) / 16 + cache->s + 0.5;
					lightt = (t - surf->texturemins[1]) / 16 + cache->t + 0.5;
					out[i] = MyD3DVertex(surf->verts[i], 0xffffffff,
						s * tex_iw, t * tex_ih,
						lights / BLOCK_WIDTH, lightt / BLOCK_HEIGHT);
				}
				RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
			}
		}

		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	}
	else
	{
		RenderDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		RenderDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		RenderDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
		RenderDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		RenderDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
		RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		RenderDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);	// don't bother writing Z

		for (lb = 0; lb < NUM_BLOCK_SURFS; lb++)
		{
			if (!RendLev->light_chain[lb])
			{
				continue;
			}

			if (!light_surf[lb])
			{
				light_surf[lb] = CreateSurface(BLOCK_WIDTH, BLOCK_HEIGHT, 16, false);
				RendLev->block_changed[lb] = true;
			}
			if (RendLev->block_changed[lb])
			{
				RendLev->block_changed[lb] = false;
				UploadTextureImage(light_surf[lb], 0, BLOCK_WIDTH, BLOCK_HEIGHT, RendLev->light_block[lb]);
			}

			RenderDevice->SetTexture(0, light_surf[lb]);

			for (cache = RendLev->light_chain[lb]; cache; cache = cache->chain)
			{
				surf = cache->surf;
				tex = surf->texinfo;
				SetFade(surf->Fade);
				for (i = 0; i < surf->count; i++)
				{
					s = DotProduct(surf->verts[i], tex->saxis) + tex->soffs;
					t = DotProduct(surf->verts[i], tex->taxis) + tex->toffs;
					lights = (s - surf->texturemins[0]) / 16 + cache->s + 0.5;
					lightt = (t - surf->texturemins[1]) / 16 + cache->t + 0.5;
					out[i] = MyD3DVertex(surf->verts[i], 0xffffffff,
						lights / BLOCK_WIDTH, lightt / BLOCK_HEIGHT);
				}
				RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
			}
		}

		RenderDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, magfilter);
		RenderDevice->SetSamplerState(0, D3DSAMP_MINFILTER, minfilter);
		RenderDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, mipfilter);
		RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		RenderDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		RenderDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);		// back to normal Z buffering
	}

	//
	//  Add specular hightlights
	//
	if (specular_highlights)
	{
		RenderDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		RenderDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		RenderDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
		RenderDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		RenderDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		RenderDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);	// don't bother writing Z
		SetFade(0);

		for (lb = 0; lb < NUM_BLOCK_SURFS; lb++)
		{
			if (!RendLev->add_chain[lb])
			{
				continue;
			}

			if (!add_surf[lb])
			{
				add_surf[lb] = CreateSurface(BLOCK_WIDTH, BLOCK_HEIGHT, 16, false);
				RendLev->add_changed[lb] = true;
			}
			if (RendLev->add_changed[lb])
			{
				RendLev->add_changed[lb] = false;
				UploadTextureImage(add_surf[lb], 0, BLOCK_WIDTH, BLOCK_HEIGHT, RendLev->add_block[lb]);
			}

			RenderDevice->SetTexture(0, add_surf[lb]);

			for (cache = RendLev->add_chain[lb]; cache; cache = cache->addchain)
			{
				surf = cache->surf;
				tex = surf->texinfo;
				for (i = 0; i < surf->count; i++)
				{
					s = DotProduct(surf->verts[i], tex->saxis) + tex->soffs;
					t = DotProduct(surf->verts[i], tex->taxis) + tex->toffs;
					lights = (s - surf->texturemins[0]) / 16 + cache->s + 0.5;
					lightt = (t - surf->texturemins[1]) / 16 + cache->t + 0.5;
					out[i] = MyD3DVertex(surf->verts[i], 0xffffffff,
						lights / BLOCK_WIDTH, lightt / BLOCK_HEIGHT);
				}
				RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
			}
		}

		RenderDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, magfilter);
		RenderDevice->SetSamplerState(0, D3DSAMP_MINFILTER, minfilter);
		RenderDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, mipfilter);
		RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		RenderDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		RenderDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);		// back to normal Z buffering
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DoHorizonPolygon
//
//==========================================================================

void VDirect3DDrawer::DoHorizonPolygon(surface_t* Surf)
{
	guard(VDirect3DDrawer::DoHorizonPolygon);
	float Dist = 4096.0;
	TVec v[4];
	if (Surf->HorizonPlane->normal.z > 0.0)
	{
		v[0] = Surf->verts[0];
		v[3] = Surf->verts[3];
		TVec HDir = -Surf->plane->normal;

		TVec Dir1 = Normalise(vieworg - Surf->verts[1]);
		TVec Dir2 = Normalise(vieworg - Surf->verts[2]);
		float Mul1 = 1.0 / DotProduct(HDir, Dir1);
		v[1] = Surf->verts[1] + Dir1 * Mul1 * Dist;
		float Mul2 = 1.0 / DotProduct(HDir, Dir2);
		v[2] = Surf->verts[2] + Dir2 * Mul2 * Dist;
		if (v[1].z < v[0].z)
		{
			v[1] = Surf->verts[1] + Dir1 * Mul1 * Dist * (Surf->verts[1].z -
				Surf->verts[0].z) / (Surf->verts[1].z - v[1].z);
			v[2] = Surf->verts[2] + Dir2 * Mul2 * Dist * (Surf->verts[2].z -
				Surf->verts[3].z) / (Surf->verts[2].z - v[2].z);
		}
	}
	else
	{
		v[1] = Surf->verts[1];
		v[2] = Surf->verts[2];
		TVec HDir = -Surf->plane->normal;

		TVec Dir1 = Normalise(vieworg - Surf->verts[0]);
		TVec Dir2 = Normalise(vieworg - Surf->verts[3]);
		float Mul1 = 1.0 / DotProduct(HDir, Dir1);
		v[0] = Surf->verts[0] + Dir1 * Mul1 * Dist;
		float Mul2 = 1.0 / DotProduct(HDir, Dir2);
		v[3] = Surf->verts[3] + Dir2 * Mul2 * Dist;
		if (v[1].z < v[0].z)
		{
			v[0] = Surf->verts[0] + Dir1 * Mul1 * Dist * (Surf->verts[1].z -
				Surf->verts[0].z) / (v[0].z - Surf->verts[0].z);
			v[3] = Surf->verts[3] + Dir2 * Mul2 * Dist * (Surf->verts[2].z -
				Surf->verts[3].z) / (v[3].z - Surf->verts[3].z);
		}
	}

	texinfo_t* Tex = Surf->texinfo;
	SetTexture(Tex->Tex, Tex->ColourMap);

	int lev = Surf->Light >> 24;
	int lR = ((Surf->Light >> 16) & 255) * lev / 255;
	int lG = ((Surf->Light >> 8) & 255) * lev / 255;
	int lB = (Surf->Light & 255) * lev / 255;
	vuint32 light = 0xff000000 | (lR << 16) | (lG << 8) | lB;
	SetFade(Surf->Fade);

	//	Draw it
	MyD3DVertex	out[4];
	RenderDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	for (int i = 0; i < 4; i++)
	{
		out[i] = MyD3DVertex(v[i], light,
			(DotProduct(v[i], Tex->saxis) + Tex->soffs) * tex_iw,
			(DotProduct(v[i], Tex->taxis) + Tex->toffs) * tex_ih);
	}
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, out, sizeof(MyD3DVertex));
	RenderDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	//	Write to the depth buffer.
	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	RenderDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
	for (int i = 0; i < Surf->count; i++)
	{
		out[i] = MyD3DVertex(Surf->verts[i], 0, 0, 0);
	}
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, out, sizeof(MyD3DVertex));
	RenderDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0f);
	RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawSkyPolygon
//
//==========================================================================

void VDirect3DDrawer::DrawSkyPolygon(surface_t* surf, bool bIsSkyBox,
	VTexture* Texture1, float offs1, VTexture* Texture2, float offs2,
	int CMap)
{
	guard(VDirect3DDrawer::DrawSkyPolygon);
	MyD3DVertex		out[256];
	int				i;
	int				sidx[4];

	SetFade(surf->Fade);
	sidx[0] = 0;
	sidx[1] = 1;
	sidx[2] = 2;
	sidx[3] = 3;
	if (!bIsSkyBox)
	{
		if (surf->verts[1].z > 0)
		{
			sidx[1] = 0;
			sidx[2] = 3;
		}
		else
		{
			sidx[0] = 1;
			sidx[3] = 2;
		}
	}
	texinfo_t *tex = surf->texinfo;
	if (maxMultiTex >= 2 && Texture2->Type != TEXTYPE_Null)
	{
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		RenderDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);

		SetTexture(Texture1, CMap);
		TexStage = 1;
		SetTexture(Texture2, CMap);
		TexStage = 0;
		for (i = 0; i < surf->count; i++)
		{
			out[i] = MyD3DVertex(surf->verts[i] + vieworg, 0xff000000 | (int(r_sky_bright_factor * 255) << 16) & 0x00ff0000 |
					(int(r_sky_bright_factor * 255) << 8) & 0x0000ff00 | int(r_sky_bright_factor * 255) & 0x000000ff,
				(DotProduct(surf->verts[sidx[i]], tex->saxis) + tex->soffs - offs1) * tex_iw,
				(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih,
				(DotProduct(surf->verts[sidx[i]], tex->saxis) + tex->soffs - offs2) * tex_iw,
				(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
		}
		RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));

		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		RenderDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	}
	else
	{
		SetTexture(Texture1, CMap);
		for (i = 0; i < surf->count; i++)
		{
			out[i] = MyD3DVertex(surf->verts[i] + vieworg, 0xff000000 | (int(r_sky_bright_factor * 255) << 16) & 0x00ff0000 |
					(int(r_sky_bright_factor * 255) << 8) & 0x0000ff00 | int(r_sky_bright_factor * 255) & 0x000000ff,
				(DotProduct(surf->verts[sidx[i]], tex->saxis) + tex->soffs - offs1) * tex_iw,
				(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
		}
		RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));

		if (Texture2->Type != TEXTYPE_Null)
		{
			SetTexture(Texture2, CMap);
			for (i = 0; i < surf->count; i++)
			{
				out[i] = MyD3DVertex(surf->verts[i] + vieworg, 0xff000000 | (int(r_sky_bright_factor * 255) << 16) & 0x00ff0000 |
					(int(r_sky_bright_factor * 255) << 8) & 0x0000ff00 | int(r_sky_bright_factor * 255) & 0x000000ff,
					(DotProduct(surf->verts[sidx[i]], tex->saxis) + tex->soffs - offs2) * tex_iw,
					(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
			}
			RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));
			RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawMaskedPolygon
//
//==========================================================================

void VDirect3DDrawer::DrawMaskedPolygon(surface_t* surf, float Alpha,
	bool Additive)
{
	guard(VDirect3DDrawer::DrawMaskedPolygon);
	MyD3DVertex	out[256];
	int			l;

	texinfo_t* tex = surf->texinfo;
	SetTexture(tex->Tex, tex->ColourMap);

	if (surf->lightmap != NULL ||
		surf->dlightframe == r_dlightframecount)
	{
		RendLev->BuildLightMap(surf, 0);
		int w = (surf->extents[0] >> 4) + 1;
		int h = (surf->extents[1] >> 4) + 1;
		int size = w * h;
		int r = 0;
		int g = 0;
		int b = 0;
		for (int i = 0; i < size; i++)
		{
			r += 255 * 256 - blocklightsr[i];
			g += 255 * 256 - blocklightsg[i];
			b += 255 * 256 - blocklightsb[i];
		}
		r = r / (size * 256);
		g = g / (size * 256);
		b = b / (size * 256);
		int alpha = (int)(Alpha * 255);
		l = (alpha << 24) | (r << 16) | (g << 8) | b;
	}
	else
	{
		int lev = surf->Light >> 24;
		int lR = ((surf->Light >> 16) & 255) * lev / 255;
		int lG = ((surf->Light >> 8) & 255) * lev / 255;
		int lB = (surf->Light & 255) * lev / 255;
		int alpha = (int)(Alpha * 255);
		l = (alpha << 24) | (lR << 16) | (lG << 8) | lB;
	}
	SetFade(surf->Fade);

	for (int i = 0; i < surf->count; i++)
	{
		out[i] = MyD3DVertex(surf->verts[i], l,
			(DotProduct(surf->verts[i], tex->saxis) + tex->soffs) * tex_iw,
			(DotProduct(surf->verts[i], tex->taxis) + tex->toffs) * tex_ih);
	}

	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	if (blend_sprites || Additive || Alpha < 1.0)
	{
		RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		RenderDevice->SetRenderState(D3DRS_ALPHAREF, 0);
	}
	if (Additive)
	{
		RenderDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	}

	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, surf->count - 2, out, sizeof(MyD3DVertex));

	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	if (blend_sprites || Additive || Alpha < 1.0)
	{
		RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetRenderState(D3DRS_ALPHAREF, 170);
	}
	if (Additive)
	{
		RenderDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawSpritePolygon
//
//==========================================================================

void VDirect3DDrawer::DrawSpritePolygon(TVec *cv, VTexture* Tex, float Alpha,
	bool Additive, VTextureTranslation* Translation, int CMap, vuint32 light,
	vuint32 Fade, const TVec&, float, const TVec& saxis, const TVec& taxis,
	const TVec& texorg)
{
	guard(VDirect3DDrawer::DrawSpritePolygon);
	MyD3DVertex		out[4];

	SetSpriteLump(Tex, Translation, CMap);

	int l = ((int)(Alpha * 255) << 24) | (light & 0x00ffffff);
	for (int i = 0; i < 4; i++)
	{
		TVec texpt = cv[i] - texorg;
		out[i] = MyD3DVertex(cv[i], l,
			DotProduct(texpt, saxis) * tex_iw,
			DotProduct(texpt, taxis) * tex_ih);
	}
	SetFade(Fade);

	if (blend_sprites || Additive || Alpha < 1.0)
	{
		RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		RenderDevice->SetRenderState(D3DRS_ALPHAREF, 0);
	}
	if (Additive)
	{
		RenderDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	}
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);

	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, out, sizeof(MyD3DVertex));

	if (blend_sprites || Additive || Alpha < 1.0)
	{
		RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		RenderDevice->SetRenderState(D3DRS_ALPHAREF, 170);
	}
	if (Additive)
	{
		RenderDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawAliasModel
//
//==========================================================================

void VDirect3DDrawer::DrawAliasModel(const TVec &origin, const TAVec &angles,
	const TVec& Offset, const TVec& Scale, VMeshModel* Mdl, int frame,
	int nextframe, VTexture* Skin, VTextureTranslation* Trans, int CMap,
	vuint32 light, vuint32 Fade, float Alpha, bool Additive,
	bool is_view_model, float Inter, bool Interpolate)
{
	guard(VDirect3DDrawer::DrawAliasModel);
	mframe_t			*pframedesc;
	mframe_t			*pnextframedesc;
	float 				l;
	int					index;
	trivertx_t			*verts;
	trivertx_t			*verts2;
	int					*order;
	int					count;
	float				shadelightr;
	float				shadelightg;
	float				shadelightb;
	float				*shadedots;
	MyD3DVertex			out[256];//FIXME
	D3DPRIMITIVETYPE	primtype;
	int					i;
	MyD3DMatrix			matWorld;
	MyD3DMatrix			matTmp;
	TVec				alias_forward;
	TVec				alias_right;
	TVec				alias_up;
	vuint32				alpha;

	if (is_view_model)
	{
		// hack the depth range to prevent view model from poking into walls
		viewData.MaxZ = 0.3;
		RenderDevice->SetViewport(&viewData);
	}

	//
	// get lighting information
	//
	shadelightr = ((light >> 16) & 0xff) / 512.0;
	shadelightg = ((light >> 8) & 0xff) / 512.0;
	shadelightb = (light & 0xff) / 512.0;
	shadedots = r_avertexnormal_dots[((int)(angles.yaw * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];
	light &= 0x00ffffff;
	alpha = int(Alpha * 255) << 24;

	//
	// draw all the triangles
	//
	mmdl_t* pmdl = Mdl->Data;
	pframedesc = (mframe_t*)((byte *)pmdl + pmdl->ofsframes + frame * pmdl->framesize);
	pnextframedesc = (mframe_t*)((byte *)pmdl + pmdl->ofsframes + nextframe * pmdl->framesize);

	AngleVectors(angles, alias_forward, alias_right, alias_up);

	matWorld = IdentityMatrix;
	for (i = 0; i < 3; i++)
	{
		matWorld(0, i) = alias_forward[i];
		matWorld(1, i) = -alias_right[i];
		matWorld(2, i) = alias_up[i];
	}
	matWorld(3, 0) = origin.x;
	matWorld(3, 1) = origin.y;
	matWorld(3, 2) = origin.z;

	matTmp = IdentityMatrix;
	matTmp(0, 0) = Scale.x;
	matTmp(1, 1) = Scale.y;
	matTmp(2, 2) = Scale.z;
	matTmp(3, 0) = Offset.x * Scale.x;
	matTmp(3, 1) = Offset.y * Scale.y;
	matTmp(3, 2) = Offset.z * Scale.z;
	matWorld = matTmp * matWorld;

	TVec scale;
	TVec scale_origin;
	if (Interpolate)
	{
		scale[0] = pframedesc->scale[0] + Inter * (pnextframedesc->scale[0] - pframedesc->scale[0]);
		scale[1] = pframedesc->scale[1] + Inter * (pnextframedesc->scale[1] - pframedesc->scale[1]);
		scale[2] = pframedesc->scale[2] + Inter * (pnextframedesc->scale[2] - pframedesc->scale[2]);
		scale_origin[0] = ((1 - Inter) * pframedesc->scale_origin[0] + Inter * pnextframedesc->scale_origin[0]);
		scale_origin[1] = ((1 - Inter) * pframedesc->scale_origin[1] + Inter * pnextframedesc->scale_origin[1]);
		scale_origin[2] = ((1 - Inter) * pframedesc->scale_origin[2] + Inter * pnextframedesc->scale_origin[2]);
	}
	else
	{
		scale[0] = pframedesc->scale[0];
		scale[1] = pframedesc->scale[1];
		scale[2] = pframedesc->scale[2];
		scale_origin[0] = pframedesc->scale_origin[0];
		scale_origin[1] = pframedesc->scale_origin[1];
		scale_origin[2] = pframedesc->scale_origin[2];
	}

	matTmp = IdentityMatrix;
	matTmp(0, 0) = scale[0];
	matTmp(1, 1) = scale[1];
	matTmp(2, 2) = scale[2];
	matTmp(3, 0) = scale_origin[0];
	matTmp(3, 1) = scale_origin[1];
	matTmp(3, 2) = scale_origin[2];
	matWorld = matTmp * matWorld;

	RenderDevice->SetTransform(D3DTS_WORLD, &matWorld);

	SetPic(Skin, Trans, CMap);

	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	if (Additive)
	{
		RenderDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	}
	RenderDevice->SetRenderState(D3DRS_ALPHAREF, 0);

	SetFade(Fade);

	verts = (trivertx_t *)(pframedesc + 1);
	order = (int *)((byte *)pmdl + pmdl->ofscmds);
	if (Interpolate)
	{
		verts2 = (trivertx_t *)(pnextframedesc + 1);
	}

	while (*order)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (count < 0)
		{
			count = -count;
			primtype = D3DPT_TRIANGLEFAN;
		}
		else
		{
			primtype = D3DPT_TRIANGLESTRIP;
		}

		if (count > 256)
		{
			Sys_Error("Too many command verts");
		}

		for (i = 0; i < count; i++)
		{
			// texture coordinates come from the draw list
			out[i].texs = ((float *)order)[0];
			out[i].text = ((float *)order)[1];
			order += 2;

			// normals and vertexes come from the frame list
			index = *order++;
			if (model_lighting)
			{
				l = shadedots[verts[index].lightnormalindex];
				out[i].colour = alpha |
					(((int)(l * shadelightr * 0xff) << 16) & 0x00ff0000) |
					(((int)(l * shadelightg * 0xff) <<  8) & 0x0000ff00) |
					(((int)(l * shadelightb * 0xff)      ) & 0x000000ff);
			}
			else
			{
				out[i].colour = alpha | light;
			}
			if (Interpolate)
			{
				out[i].x = ((1 - Inter) * verts[index].v[0] + (Inter) * verts2[index].v[0]);
				out[i].y = ((1 - Inter) * verts[index].v[1] + (Inter) * verts2[index].v[1]);
				out[i].z = ((1 - Inter) * verts[index].v[2] + (Inter) * verts2[index].v[2]);
			}
			else
			{
				out[i].x = verts[index].v[0];
				out[i].y = verts[index].v[1];
				out[i].z = verts[index].v[2];
			}
		}

		RenderDevice->DrawPrimitiveUP(primtype, count - 2, out, sizeof(MyD3DVertex));
	}

	RenderDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
	RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	if (Additive)
	{
		RenderDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	RenderDevice->SetRenderState(D3DRS_ALPHAREF, 170);
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	RenderDevice->SetTransform(D3DTS_WORLD, &IdentityMatrix);
	if (is_view_model)
	{
		viewData.MaxZ = 1.0;
		RenderDevice->SetViewport(&viewData);
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::StartParticles
//
//==========================================================================

void VDirect3DDrawer::StartParticles()
{
	guard(VDirect3DDrawer::StartParticles);
	if (!particle_texture)
	{
		rgba_t		pbuf[8][8];

		for (int j = 0; j < 8; j++)
		{
			for (int i = 0; i < 8; i++)
			{
				pbuf[j][i].r = 255;
				pbuf[j][i].g = 255;
				pbuf[j][i].b = 255;
				pbuf[j][i].a = byte(ptex[j][i] * 255);
			}
		}
		particle_texture = CreateSurface(8, 8, 16, false);
		UploadTextureImage(particle_texture, 0, 8, 8, &pbuf[0][0]);
	}
	RenderDevice->SetTexture(0, particle_texture);
	RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	RenderDevice->SetRenderState(D3DRS_ALPHAREF, 0);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawParticle
//
//==========================================================================

void VDirect3DDrawer::DrawParticle(particle_t *p)
{
	guard(VDirect3DDrawer::DrawParticle);
	MyD3DVertex out[4];
	out[0] = MyD3DVertex(p->org - viewright * p->Size + viewup * p->Size, p->colour, 0, 0);
	out[1] = MyD3DVertex(p->org + viewright * p->Size + viewup * p->Size, p->colour, 1, 0);
	out[2] = MyD3DVertex(p->org + viewright * p->Size - viewup * p->Size, p->colour, 1, 1);
	out[3] = MyD3DVertex(p->org - viewright * p->Size - viewup * p->Size, p->colour, 0, 1);
	RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, out, sizeof(MyD3DVertex));
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::EndParticles
//
//==========================================================================

void VDirect3DDrawer::EndParticles()
{
	guard(VDirect3DDrawer::EndParticles);
	RenderDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	RenderDevice->SetRenderState(D3DRS_ALPHAREF, 170);
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::StartPortal
//
//==========================================================================

bool VDirect3DDrawer::StartPortal(VPortal* Portal, bool UseStencil)
{
	guard(VDirect3DDrawer::StartPortal);
	if (UseStencil)
	{
		//	Disable drawing
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
		RenderDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
		RenderDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

		//	Set up stencil test.
		if (!RendLev->PortalDepth)
		{
			RenderDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
		}
		RenderDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
		RenderDevice->SetRenderState(D3DRS_STENCILREF, RendLev->PortalDepth);
		RenderDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR);

		//	Mark the portal area.
		DrawPortalArea(Portal);

		//	Set up stencil test for portal
		RenderDevice->SetRenderState(D3DRS_STENCILREF, RendLev->PortalDepth + 1);
		RenderDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

		if (Portal->NeedsDepthBuffer())
		{
			RenderDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			//	Clear depth buffer
			viewData.MinZ = 1.0;
			RenderDevice->SetViewport(&viewData);
			RenderDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
			DrawPortalArea(Portal);
			RenderDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			viewData.MinZ = 0.0;
			RenderDevice->SetViewport(&viewData);
		}
		else
		{
			RenderDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			RenderDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		}

		//	Enable drawing.
		RenderDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0f);
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

		RendLev->PortalDepth++;
	}
	else
	{
		if (!Portal->NeedsDepthBuffer())
		{
			RenderDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			RenderDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		}
	}
	return true;
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::DrawPortalArea
//
//==========================================================================

void VDirect3DDrawer::DrawPortalArea(VPortal* Portal)
{
	guard(VDirect3DDrawer::DrawPortalArea);
	MyD3DVertex	out[256];
	for (int i = 0; i < Portal->Surfs.Num(); i++)
	{
		const surface_t* Surf = Portal->Surfs[i];
		for (int i = 0; i < Surf->count; i++)
		{
			out[i] = MyD3DVertex(Surf->verts[i], 0, 0, 0);
		}
	    RenderDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, Surf->count - 2,
            out, sizeof(MyD3DVertex));
	}
	unguard;
}

//==========================================================================
//
//	VDirect3DDrawer::EndPortal
//
//==========================================================================

void VDirect3DDrawer::EndPortal(VPortal* Portal, bool UseStencil)
{
	guard(VDirect3DDrawer::EndPortal);
	if (UseStencil)
	{
		if (Portal->NeedsDepthBuffer())
		{
			//	Clear depth buffer
			viewData.MinZ = 1.0;
			RenderDevice->SetViewport(&viewData);
			RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
			RenderDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
			RenderDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
			DrawPortalArea(Portal);
			RenderDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			RenderDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0f);
			RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			viewData.MinZ = 0.0;
			RenderDevice->SetViewport(&viewData);
		}
		else
		{
			RenderDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			RenderDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		}

	    RenderDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECR);

		//	Draw proper z-buffer for the portal area.
		RenderDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
		RenderDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
		DrawPortalArea(Portal);
		RenderDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0f);
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		RenderDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

		RenderDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

		RendLev->PortalDepth--;
		RenderDevice->SetRenderState(D3DRS_STENCILREF, RendLev->PortalDepth);
		if (!RendLev->PortalDepth)
		{
			RenderDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
		}
	}
	else
	{
		if (Portal->NeedsDepthBuffer())
		{
			//	Clear depth buffer
			RenderDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0, 0);
		}
		else
		{
			RenderDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			RenderDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		}

		//	Draw proper z-buffer for the portal area.
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
		RenderDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
		DrawPortalArea(Portal);
		RenderDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0f);
		RenderDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	}
	unguard;
}
