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

// CODE --------------------------------------------------------------------

#if DIRECT3D_VERSION < 0x0800

//==========================================================================
//
//	VDirect3DDrawer::LogPrimCaps
//
//==========================================================================

void VDirect3DDrawer::LogPrimCaps(FOutputDevice &Ar, const D3DPRIMCAPS &pc)
{
	Ar.Log("Misc caps:");
	if (pc.dwMiscCaps & D3DPMISCCAPS_MASKPLANES)
	{
		Ar.Log(" MaskPlanes");
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_MASKZ)
	{
		Ar.Log(" MaskZ");
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_LINEPATTERNREP)
	{
		Ar.Log(" LinePatternRep");
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_CONFORMANT)
	{
		Ar.Log(" Conformant");
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_CULLNONE)
	{
		Ar.Log(" CullNone");
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_CULLCW)
	{
		Ar.Log(" CullClockwise");
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_CULLCCW)
	{
		Ar.Log(" CullCounterClockwise");
	}

	Ar.Log("Rasterization caps:");
	if (pc.dwRasterCaps & D3DPRASTERCAPS_DITHER)
	{
		Ar.Log(" Dither");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ROP2)
	{
		Ar.Log(" ROp2");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_XOR)
	{
		Ar.Log(" XOR");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_PAT)
	{
		Ar.Log(" Pat");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ZTEST)
	{
		Ar.Log(" ZTest");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_SUBPIXEL)
	{
		Ar.Log(" SubPixel");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_SUBPIXELX)
	{
		Ar.Log(" SubPixelX");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX)
	{
		Ar.Log(" FogVertex");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE)
	{
		Ar.Log(" FogTable");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_STIPPLE)
	{
		Ar.Log(" Stipple");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT)
	{
		Ar.Log(" AntialiasSortDependent");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT)
	{
		Ar.Log(" AntialiasSortIndependent");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASEDGES)
	{
		Ar.Log(" AntialiasEdges");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS)
	{
		Ar.Log(" MipMapLodBias");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ZBIAS)
	{
		Ar.Log(" ZBias");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR)
	{
		Ar.Log(" ZBufferLessHSR");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_FOGRANGE)
	{
		Ar.Log(" FogRange");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ANISOTROPY)
	{
		Ar.Log(" Anisotropy");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_WBUFFER)
	{
		Ar.Log(" WBuffer");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT)
	{
		Ar.Log(" TranslucentSortIndependent");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_WFOG)
	{
		Ar.Log(" WFog");
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ZFOG)
	{
		Ar.Log(" ZFog");
	}

	Ar.Log("Supported depth tests:");
	if (pc.dwZCmpCaps & D3DPCMPCAPS_ALWAYS)
	{
		Ar.Log(" always");
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_EQUAL)
	{
		Ar.Log(" equal");
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_GREATER)
	{
		Ar.Log(" greater");
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_GREATEREQUAL)
	{
		Ar.Log(" greater_equal");
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_LESS)
	{
		Ar.Log(" less");
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_LESSEQUAL)
	{
		Ar.Log(" less_equal");
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_NEVER)
	{
		Ar.Log(" never");
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_NOTEQUAL)
	{
		Ar.Log(" not_equal");
	}

	Ar.Log("Src blend caps:");
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_ZERO)
	{
		Ar.Log(" Zero");
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_ONE)
	{
		Ar.Log(" One");
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_SRCCOLOR)
	{
		Ar.Log(" SrcColor");
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR)
	{
		Ar.Log(" InvSrcColor");
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA)
	{
		Ar.Log(" SrcAlpha");
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)
	{
		Ar.Log(" InvSrcAlpha");
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_DESTALPHA)
	{
		Ar.Log(" DestAlpha");
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_INVDESTALPHA)
	{
		Ar.Log(" InvDestAlpha");
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_DESTCOLOR)
	{
		Ar.Log(" DestColor");
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_INVDESTCOLOR)
	{
		Ar.Log(" InvDestColor");
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHASAT)
	{
		Ar.Log(" SrcAlphaSat");
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_BOTHSRCALPHA)
	{
		Ar.Log(" BothSrcAlpha");
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_BOTHINVSRCALPHA)
	{
		Ar.Log(" BothInvSrcAlpha");
	}

	Ar.Log("Dest blend caps:");
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_ZERO)
	{
		Ar.Log(" Zero");
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_ONE)
	{
		Ar.Log(" One");
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_SRCCOLOR)
	{
		Ar.Log(" SrcColor");
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR)
	{
		Ar.Log(" InvSrcColor");
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_SRCALPHA)
	{
		Ar.Log(" SrcAlpha");
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)
	{
		Ar.Log(" InvSrcAlpha");
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_DESTALPHA)
	{
		Ar.Log(" DestAlpha");
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_INVDESTALPHA)
	{
		Ar.Log(" InvDestAlpha");
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_DESTCOLOR)
	{
		Ar.Log(" DestColor");
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_INVDESTCOLOR)
	{
		Ar.Log(" InvDestColor");
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_SRCALPHASAT)
	{
		Ar.Log(" SrcAlphaSat");
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_BOTHSRCALPHA)
	{
		Ar.Log(" BothSrcAlpha");
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_BOTHINVSRCALPHA)
	{
		Ar.Log(" BothInvSrcAlpha");
	}

	Ar.Log("Supported alpaha tests:");
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_ALWAYS)
	{
		Ar.Log(" always");
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_EQUAL)
	{
		Ar.Log(" equal");
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_GREATER)
	{
		Ar.Log(" greater");
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_GREATEREQUAL)
	{
		Ar.Log(" greater_equal");
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_LESS)
	{
		Ar.Log(" less");
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_LESSEQUAL)
	{
		Ar.Log(" less_equal");
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_NEVER)
	{
		Ar.Log(" never");
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_NOTEQUAL)
	{
		Ar.Log(" not_equal");
	}

	Ar.Log("Shade caps:");
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORFLATMONO)
	{
		Ar.Log(" ColorFlatMono");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORFLATRGB)
	{
		Ar.Log(" ColorFlatRGB");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDMONO)
	{
		Ar.Log(" ColorGouraudMono");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB)
	{
		Ar.Log(" ColorGouraudRGB");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORPHONGMONO)
	{
		Ar.Log(" ColorPhongMono");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORPHONGRGB)
	{
		Ar.Log(" ColorPhongRGB");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARFLATMONO)
	{
		Ar.Log(" SpecularFlatMono");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARFLATRGB)
	{
		Ar.Log(" SpecularFlatRGB");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARGOURAUDMONO)
	{
		Ar.Log(" SpecularGouraudMono");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARGOURAUDRGB)
	{
		Ar.Log(" SpecularGouraudRGB");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARPHONGMONO)
	{
		Ar.Log(" SpecularPhongMono");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARPHONGRGB)
	{
		Ar.Log(" SpecularPhongRGB");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAFLATBLEND)
	{
		Ar.Log(" AlphaFlatBlend");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAFLATSTIPPLED)
	{
		Ar.Log(" AlphaFlatStippled");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND)
	{
		Ar.Log(" AlphaGouraudBlend");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED)
	{
		Ar.Log(" AlphaGouraudStippled");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAPHONGBLEND)
	{
		Ar.Log(" AlphaPhongBlend");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAPHONGSTIPPLED)
	{
		Ar.Log(" AlphaPhongStippled");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_FOGFLAT)
	{
		Ar.Log(" FogFlat");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_FOGGOURAUD)
	{
		Ar.Log(" FogGouraud");
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_FOGPHONG)
	{
		Ar.Log(" FogPhong");
	}

	Ar.Log("Texture caps:");
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE)
	{
		Ar.Log("- Perspective-correct texturing is supported");
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_POW2)
	{
		Ar.Log("- Power-of-2 texture dimensions are required");
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_ALPHA)
	{
		Ar.Log("- Alpha in texture pixels is supported");
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY)
	{
		Ar.Log("- Color-keyed textures are supported");
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_BORDER)
	{
		Ar.Log("- obsolete, see D3DPTADDRESSCAPS_BORDER");
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
	{
		Ar.Log("- Only square textures are supported");
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE)
	{
		Ar.Log("- Texture indices are not scaled by the texture size prior to interpolation.");
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE)
	{
		Ar.Log("- Device can draw alpha from texture palettes");
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
	{
		Ar.Log("- Device can use non-POW2 textures");
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_PROJECTED)
	{
		Ar.Log("- Device can divide transformed texture coordinates by the COUNTth texture coordinate (can do D3DTTFF_PROJECTED)");
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_CUBEMAP)
	{
		Ar.Log("- Device can do cubemap textures");
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_COLORKEYBLEND)
	{
		Ar.Log("- colorkeyblend");
	}

	Ar.Log("Texture filter caps:");
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_NEAREST)
	{
		Ar.Log(" Nearest");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR)
	{
		Ar.Log(" Linear");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPNEAREST)
	{
		Ar.Log(" MipNearest");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPLINEAR)
	{
		Ar.Log(" MipLinear");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPNEAREST)
	{
		Ar.Log(" LinearMipNearest");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPLINEAR)
	{
		Ar.Log(" LinearMipLinear");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT)
	{
		Ar.Log(" MinFPoint");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)
	{
		Ar.Log(" MinFLinear");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
	{
		Ar.Log(" MinFAnisotropic");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT)
	{
		Ar.Log(" MipFPoint");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR)
	{
		Ar.Log(" MipFLinear");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT)
	{
		Ar.Log(" MagFPoint");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR)
	{
		Ar.Log(" MagFLinear");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC)
	{
		Ar.Log(" MagFAnisotropic");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFAFLATCUBIC)
	{
		Ar.Log(" MagFAFlatCubic");
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC)
	{
		Ar.Log(" MagFGaussianCubic");
	}

	Ar.Log("Texture blend caps:");
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_DECAL)
	{
		Ar.Log(" Decal");
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATE)
	{
		Ar.Log(" Modulate");
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_DECALALPHA)
	{
		Ar.Log(" DecalAlpha");
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA)
	{
		Ar.Log(" ModulateAlpha");
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_DECALMASK)
	{
		Ar.Log(" DecalMask");
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEMASK)
	{
		Ar.Log(" ModulateMask");
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_COPY)
	{
		Ar.Log(" Copy");
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_ADD)
	{
		Ar.Log(" Add");
	}

	Ar.Log("Texture address caps:");
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_WRAP)
	{
		Ar.Log(" Wrap");
	}
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_MIRROR)
	{
		Ar.Log(" Mirror");
	}
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_CLAMP)
	{
		Ar.Log(" Clamp");
	}
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_BORDER)
	{
		Ar.Log(" Border");
	}
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_INDEPENDENTUV)
	{
		Ar.Log(" IndependentUV");
	}

    Ar.Logf("Stipple width: %d", pc.dwStippleWidth);
    Ar.Logf("Stipple height: %d", pc.dwStippleHeight);
}

//==========================================================================
//
//	VDirect3DDrawer::LogDeviceDesc
//
//==========================================================================

void VDirect3DDrawer::LogDeviceDesc(FOutputDevice &Ar, const LPD3DDEVICEDESC7 dd)
{
	Ar.Log("Device caps:");
	if (dd->dwDevCaps & D3DDEVCAPS_FLOATTLVERTEX)
	{
		Ar.Log("- accepts floating point for post-transform vertex data.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_SORTINCREASINGZ)
	{
		Ar.Log("- needs data sorted for increasing depth.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_SORTDECREASINGZ)
	{
		Ar.Log("- needs data sorted for decreasing depth.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_SORTEXACT)
	{
		Ar.Log("- needs data sorted exactly.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_EXECUTESYSTEMMEMORY)
	{
		Ar.Log("- can use execute buffers from system memory.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_EXECUTEVIDEOMEMORY)
	{
		Ar.Log("- can use execute buffer from video memory.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TLVERTEXSYSTEMMEMORY)
	{
		Ar.Log("- can use buffers from system memory for transformed and lit vertices.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TLVERTEXVIDEOMEMORY)
	{
		Ar.Log("- can use buffers from video memory for transformed and lit vertices.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TEXTURESYSTEMMEMORY)
	{
		Ar.Log("- can retrieve textures from system memory.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TEXTUREVIDEOMEMORY)
	{
		Ar.Log("- can retrieve textures from device memory.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_DRAWPRIMTLVERTEX)
	{
		Ar.Log("- exports a DrawPrimitive-aware HAL.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_CANRENDERAFTERFLIP)
	{
		Ar.Log("- can queue rendering commands after a page flip.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TEXTURENONLOCALVIDMEM)
	{
		Ar.Log("- can retrieve textures from non-local video (AGP) memory.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_DRAWPRIMITIVES2)
	{
		Ar.Log("- can support DrawPrimitives2.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES)
	{
		Ar.Log("- uses discrete texture memory pools for each stage.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_DRAWPRIMITIVES2EX)
	{
		Ar.Log("- can support Extended DrawPrimitives2 i.e. DX7 compliant driver.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		Ar.Log("- supports transformation and lighting in hardware.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_CANBLTSYSTONONLOCAL)
	{
		Ar.Log("- supports blits from system-memory textures to non-local video-memory textures.");
	}
	if (dd->dwDevCaps & D3DDEVCAPS_HWRASTERIZATION)
	{
		Ar.Log("- has HW acceleration for rasterization.");
	}
	Ar.Log("");

	Ar.Log("-- Line caps --");
	LogPrimCaps(Ar, dd->dpcLineCaps);
	Ar.Log("");

	Ar.Log("-- Triangle caps --");
	LogPrimCaps(Ar, dd->dpcTriCaps);
 	Ar.Log("");

	Ar.Log("Rendering bit depth:");
 	if (dd->dwDeviceRenderBitDepth & DDBD_8) Ar.Log(" 8");
 	if (dd->dwDeviceRenderBitDepth & DDBD_16) Ar.Log(" 16");
 	if (dd->dwDeviceRenderBitDepth & DDBD_24) Ar.Log(" 24");
 	if (dd->dwDeviceRenderBitDepth & DDBD_32) Ar.Log(" 32");
	Ar.Log("Z-Buffer bit depth:");
 	if (dd->dwDeviceZBufferBitDepth & DDBD_8) Ar.Log(" 8");
 	if (dd->dwDeviceZBufferBitDepth & DDBD_16) Ar.Log(" 16");
 	if (dd->dwDeviceZBufferBitDepth & DDBD_24) Ar.Log(" 24");
 	if (dd->dwDeviceZBufferBitDepth & DDBD_32) Ar.Log(" 32");
	Ar.Log("");

	Ar.Logf("Min texture width: %d", dd->dwMinTextureWidth);
	Ar.Logf("Min texture height: %d", dd->dwMinTextureHeight);
	Ar.Logf("Max texture width: %d", dd->dwMaxTextureWidth);
	Ar.Logf("Max texture height: %d", dd->dwMaxTextureHeight);
	Ar.Log("");

	Ar.Logf("Max texture repeat: %d", dd->dwMaxTextureRepeat);
	Ar.Logf("Max texture aspect ratio: %d", dd->dwMaxTextureAspectRatio);
	Ar.Logf("Max anisotropy: %d", dd->dwMaxAnisotropy);
	Ar.Log("");

	Ar.Logf("Guard band left: %d", dd->dvGuardBandLeft);
	Ar.Logf("Guard band top: %d", dd->dvGuardBandTop);
	Ar.Logf("Guard band right: %d", dd->dvGuardBandRight);
	Ar.Logf("Guard band bottom: %d", dd->dvGuardBandBottom);
	Ar.Log("");

	Ar.Logf("Extents adjust: %d", dd->dvExtentsAdjust);
	Ar.Log("");

	Ar.Log("Stencil caps:");
	if (dd->dwStencilCaps & D3DSTENCILCAPS_DECR)
	{
		Ar.Log(" DECR");
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_DECRSAT)
	{
		Ar.Log(" DECRSAT");
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_INCR)
	{
		Ar.Log(" INCR");
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_INCRSAT)
	{
		Ar.Log(" INCRSAT");
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_INVERT)
	{
		Ar.Log(" INVERT");
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_KEEP)
	{
		Ar.Log(" KEEP");
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_REPLACE)
	{
		Ar.Log(" REPLACE");
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_ZERO)
	{
		Ar.Log(" ZERO");
	}

	Ar.Log("Flexible vertex format capabilities:");
	if (dd->dwFVFCaps & D3DFVFCAPS_DONOTSTRIPELEMENTS)
	{
		Ar.Log("- prefers that vertex elements not be stripped.");
	}
	Ar.Logf("- total %d texture coordinate sets", dd->dwFVFCaps & D3DFVFCAPS_TEXCOORDCOUNTMASK);

	Ar.Log("Supported texture operations:");
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_ADD)
	{
		Ar.Log(" ADD");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_ADDSIGNED)
	{
		Ar.Log(" ADDSIGNED");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_ADDSIGNED2X)
	{
		Ar.Log(" ADDSIGNED2X");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_ADDSMOOTH)
	{
		Ar.Log(" ADDSMOOTH");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDCURRENTALPHA)
	{
		Ar.Log(" BLENDCURRENTALPHA");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDDIFFUSEALPHA)
	{
		Ar.Log(" BLENDDIFFUSEALPHA");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDFACTORALPHA)
	{
		Ar.Log(" BLENDFACTORALPHA");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDTEXTUREALPHA)
	{
		Ar.Log(" BLENDTEXTUREALPHA");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDTEXTUREALPHAPM)
	{
		Ar.Log(" BLENDTEXTUREALPHAPM");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP)
	{
		Ar.Log(" BUMPENVMAP");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BUMPENVMAPLUMINANCE)
	{
		Ar.Log(" BUMPENVMAPLUMINANCE");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_DISABLE)
	{
		Ar.Log(" DISABLE");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3)
	{
		Ar.Log(" DOTPRODUCT3");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATE)
	{
		Ar.Log(" MODULATE");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATE2X)
	{
		Ar.Log(" MODULATE2X");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATE4X)
	{
		Ar.Log(" MODULATE4X");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR)
	{
		Ar.Log(" MODULATEALPHA_ADDCOLOR");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA)
	{
		Ar.Log(" MODULATEALPHA_ADDCOLOR");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR)
	{
		Ar.Log(" MODULATEINVALPHA_ADDCOLOR");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA)
	{
		Ar.Log(" MODULATEINVCOLOR_ADDALPHA");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_PREMODULATE)
	{
		Ar.Log(" PREMODULATE");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_SELECTARG1)
	{
		Ar.Log(" SELECTARG1");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_SELECTARG2)
	{
		Ar.Log(" SELECTARG2");
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_SUBTRACT)
	{
		Ar.Log(" SUBTRACT");
	}
	Ar.Logf("Max texture blend stages: %d", dd->wMaxTextureBlendStages);
	Ar.Logf("Max simultaneous textures: %d", dd->wMaxSimultaneousTextures);
	Ar.Logf("Max active lights: %d", dd->dwMaxActiveLights);
	Ar.Logf("Max vertex W: %d", dd->dvMaxVertexW);
	Ar.Logf("Max user clip planes: %d", dd->wMaxUserClipPlanes);
	Ar.Logf("Max vertex blend matrices: %d", dd->wMaxVertexBlendMatrices);

/*
    GUID        deviceGUID;

    DWORD       dwVertexProcessingCaps;
*/
}

//==========================================================================
//
//	VDirect3DDrawer::LogPixelFormat
//
//==========================================================================

void VDirect3DDrawer::LogPixelFormat(FOutputDevice &Ar, const LPDDPIXELFORMAT pf)
{
	if (pf->dwFlags & DDPF_ALPHAPIXELS)
		Ar.Log("- has alpha channel.");
	if (pf->dwFlags & DDPF_ALPHA)
		Ar.Log("- Alpha-only surface.");
	if (pf->dwFlags & DDPF_FOURCC)
		Ar.Log("- contains a FOURCC code describing a non-RGB pixel format.");
	if (pf->dwFlags & DDPF_PALETTEINDEXED1)
		Ar.Log("- 1-bit color indexed.");
	if (pf->dwFlags & DDPF_PALETTEINDEXED2)
		Ar.Log("- 2-bit color indexed.");
	if (pf->dwFlags & DDPF_PALETTEINDEXED4)
		Ar.Log("- 4-bit color indexed.");
	if (pf->dwFlags & DDPF_PALETTEINDEXED8)
		Ar.Log("- 8-bit color indexed.");
	if (pf->dwFlags & DDPF_PALETTEINDEXEDTO8)
		Ar.Log("- indexed to an 8-bit palette.");
	if (pf->dwFlags & DDPF_RGB)
		Ar.Log("- RGB surface.");
	if (pf->dwFlags & DDPF_COMPRESSED)
		Ar.Log("- will accept pixel data in the specified format and compress it during the write operation.");
	if (pf->dwFlags & DDPF_RGBTOYUV)
		Ar.Log("- translates RGB data YUV data.");
	if (pf->dwFlags & DDPF_YUV)
		Ar.Log("- YUV surface.");
	if (pf->dwFlags & DDPF_ZBUFFER)
		Ar.Log("- z-buffer surface.");
	if (pf->dwFlags & DDPF_ZPIXELS)
		Ar.Log("- contains z information in the pixels.");
	if (pf->dwFlags & DDPF_STENCILBUFFER)
		Ar.Log("- contains stencil information along with Z.");
	if (pf->dwFlags & DDPF_ALPHAPREMULT)
		Ar.Log("- uses the premultiplied alpha format.");
	if (pf->dwFlags & DDPF_LUMINANCE)
		Ar.Log("- luminance-only or luminance-alpha surface.");
	if (pf->dwFlags & DDPF_BUMPLUMINANCE)
		Ar.Log("- luminance off bumpmap surface.");
	if (pf->dwFlags & DDPF_BUMPDUDV)
		Ar.Log("- has Bump-map.");
	Ar.Logf("- Bit depth: %d", pf->dwRGBBitCount);
	Ar.Logf("- Mask for red bits: %x", pf->dwRBitMask);
	Ar.Logf("- Mask for green bits: %x", pf->dwGBitMask);
	Ar.Logf("- Mask for blue bits: %x", pf->dwBBitMask);
	Ar.Logf("- Mask for alpha/Z channel: %x", pf->dwRGBAlphaBitMask);
}

#endif
