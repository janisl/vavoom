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

#if DIRECT3D_VERSION >= 0x0800

ostream &operator << (ostream &str, const D3DCAPS8 *)
{
	return str;
}

#else

//==========================================================================
//
//
//
//==========================================================================

ostream &operator << (ostream &str, const D3DPRIMCAPS pc)
{
	str << "Misc caps:";
	if (pc.dwMiscCaps & D3DPMISCCAPS_MASKPLANES)
	{
		str << " MaskPlanes";
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_MASKZ)
	{
		str << " MaskZ";
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_LINEPATTERNREP)
	{
		str << " LinePatternRep";
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_CONFORMANT)
	{
		str << " Conformant";
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_CULLNONE)
	{
		str << " CullNone";
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_CULLCW)
	{
		str << " CullClockwise";
	}
	if (pc.dwMiscCaps & D3DPMISCCAPS_CULLCCW)
	{
		str << " CullCounterClockwise";
	}
	str << endl;

	str << "Rasterization caps:";
	if (pc.dwRasterCaps & D3DPRASTERCAPS_DITHER)
	{
		str << " Dither";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ROP2)
	{
		str << " ROp2";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_XOR)
	{
		str << " XOR";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_PAT)
	{
		str << " Pat";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ZTEST)
	{
		str << " ZTest";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_SUBPIXEL)
	{
		str << " SubPixel";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_SUBPIXELX)
	{
		str << " SubPixelX";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX)
	{
		str << " FogVertex";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE)
	{
		str << " FogTable";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_STIPPLE)
	{
		str << " Stipple";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT)
	{
		str << " AntialiasSortDependent";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT)
	{
		str << " AntialiasSortIndependent";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASEDGES)
	{
		str << " AntialiasEdges";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS)
	{
		str << " MipMapLodBias";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ZBIAS)
	{
		str << " ZBias";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR)
	{
		str << " ZBufferLessHSR";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_FOGRANGE)
	{
		str << " FogRange";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ANISOTROPY)
	{
		str << " Anisotropy";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_WBUFFER)
	{
		str << " WBuffer";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT)
	{
		str << " TranslucentSortIndependent";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_WFOG)
	{
		str << " WFog";
	}
	if (pc.dwRasterCaps & D3DPRASTERCAPS_ZFOG)
	{
		str << " ZFog";
	}
	str << endl;

	str << "Supported depth tests:";
	if (pc.dwZCmpCaps & D3DPCMPCAPS_ALWAYS)
	{
		str << " always";
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_EQUAL)
	{
		str << " equal";
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_GREATER)
	{
		str << " greater";
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_GREATEREQUAL)
	{
		str << " greater_equal";
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_LESS)
	{
		str << " less";
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_LESSEQUAL)
	{
		str << " less_equal";
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_NEVER)
	{
		str << " never";
	}
	if (pc.dwZCmpCaps & D3DPCMPCAPS_NOTEQUAL)
	{
		str << " not_equal";
	}
	str << endl;

	str << "Src blend caps:";
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_ZERO)
	{
		str << " Zero";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_ONE)
	{
		str << " One";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_SRCCOLOR)
	{
		str << " SrcColor";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR)
	{
		str << " InvSrcColor";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA)
	{
		str << " SrcAlpha";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)
	{
		str << " InvSrcAlpha";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_DESTALPHA)
	{
		str << " DestAlpha";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_INVDESTALPHA)
	{
		str << " InvDestAlpha";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_DESTCOLOR)
	{
		str << " DestColor";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_INVDESTCOLOR)
	{
		str << " InvDestColor";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHASAT)
	{
		str << " SrcAlphaSat";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_BOTHSRCALPHA)
	{
		str << " BothSrcAlpha";
	}
	if (pc.dwSrcBlendCaps & D3DPBLENDCAPS_BOTHINVSRCALPHA)
	{
		str << " BothInvSrcAlpha";
	}
	str << endl;

	str << "Dest blend caps:";
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_ZERO)
	{
		str << " Zero";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_ONE)
	{
		str << " One";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_SRCCOLOR)
	{
		str << " SrcColor";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR)
	{
		str << " InvSrcColor";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_SRCALPHA)
	{
		str << " SrcAlpha";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)
	{
		str << " InvSrcAlpha";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_DESTALPHA)
	{
		str << " DestAlpha";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_INVDESTALPHA)
	{
		str << " InvDestAlpha";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_DESTCOLOR)
	{
		str << " DestColor";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_INVDESTCOLOR)
	{
		str << " InvDestColor";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_SRCALPHASAT)
	{
		str << " SrcAlphaSat";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_BOTHSRCALPHA)
	{
		str << " BothSrcAlpha";
	}
	if (pc.dwDestBlendCaps & D3DPBLENDCAPS_BOTHINVSRCALPHA)
	{
		str << " BothInvSrcAlpha";
	}
	str << endl;

	str << "Supported alpaha tests:";
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_ALWAYS)
	{
		str << " always";
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_EQUAL)
	{
		str << " equal";
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_GREATER)
	{
		str << " greater";
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_GREATEREQUAL)
	{
		str << " greater_equal";
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_LESS)
	{
		str << " less";
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_LESSEQUAL)
	{
		str << " less_equal";
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_NEVER)
	{
		str << " never";
	}
	if (pc.dwAlphaCmpCaps & D3DPCMPCAPS_NOTEQUAL)
	{
		str << " not_equal";
	}
	str << endl;

	str << "Shade caps:";
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORFLATMONO)
	{
		str << " ColorFlatMono";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORFLATRGB)
	{
		str << " ColorFlatRGB";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDMONO)
	{
		str << " ColorGouraudMono";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB)
	{
		str << " ColorGouraudRGB";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORPHONGMONO)
	{
		str << " ColorPhongMono";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_COLORPHONGRGB)
	{
		str << " ColorPhongRGB";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARFLATMONO)
	{
		str << " SpecularFlatMono";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARFLATRGB)
	{
		str << " SpecularFlatRGB";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARGOURAUDMONO)
	{
		str << " SpecularGouraudMono";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARGOURAUDRGB)
	{
		str << " SpecularGouraudRGB";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARPHONGMONO)
	{
		str << " SpecularPhongMono";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_SPECULARPHONGRGB)
	{
		str << " SpecularPhongRGB";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAFLATBLEND)
	{
		str << " AlphaFlatBlend";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAFLATSTIPPLED)
	{
		str << " AlphaFlatStippled";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND)
	{
		str << " AlphaGouraudBlend";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED)
	{
		str << " AlphaGouraudStippled";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAPHONGBLEND)
	{
		str << " AlphaPhongBlend";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_ALPHAPHONGSTIPPLED)
	{
		str << " AlphaPhongStippled";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_FOGFLAT)
	{
		str << " FogFlat";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_FOGGOURAUD)
	{
		str << " FogGouraud";
	}
	if (pc.dwShadeCaps & D3DPSHADECAPS_FOGPHONG)
	{
		str << " FogPhong";
	}
	str << endl;

	str << "Texture caps:\n";
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE)
	{
		str << "- Perspective-correct texturing is supported\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_POW2)
	{
		str << "- Power-of-2 texture dimensions are required\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_ALPHA)
	{
		str << "- Alpha in texture pixels is supported\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY)
	{
		str << "- Color-keyed textures are supported\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_BORDER)
	{
		str << "- obsolete, see D3DPTADDRESSCAPS_BORDER\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
	{
		str << "- Only square textures are supported\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE)
	{
		str << "- Texture indices are not scaled by the texture size prior to interpolation.\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE)
	{
		str << "- Device can draw alpha from texture palettes\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
	{
		str << "- Device can use non-POW2 textures\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_PROJECTED)
	{
		str << "- Device can divide transformed texture coordinates by the COUNTth texture coordinate (can do D3DTTFF_PROJECTED)\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_CUBEMAP)
	{
		str << "- Device can do cubemap textures\n";
	}
	if (pc.dwTextureCaps & D3DPTEXTURECAPS_COLORKEYBLEND)
	{
		str << "- colorkeyblend\n";
	}

	str << "Texture filter caps:";
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_NEAREST)
	{
		str << " Nearest";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR)
	{
		str << " Linear";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPNEAREST)
	{
		str << " MipNearest";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPLINEAR)
	{
		str << " MipLinear";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPNEAREST)
	{
		str << " LinearMipNearest";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPLINEAR)
	{
		str << " LinearMipLinear";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT)
	{
		str << " MinFPoint";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)
	{
		str << " MinFLinear";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
	{
		str << " MinFAnisotropic";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT)
	{
		str << " MipFPoint";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR)
	{
		str << " MipFLinear";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT)
	{
		str << " MagFPoint";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR)
	{
		str << " MagFLinear";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC)
	{
		str << " MagFAnisotropic";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFAFLATCUBIC)
	{
		str << " MagFAFlatCubic";
	}
	if (pc.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC)
	{
		str << " MagFGaussianCubic";
	}
	str << endl;

	str << "Texture blend caps:";
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_DECAL)
	{
		str << " Decal";
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATE)
	{
		str << " Modulate";
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_DECALALPHA)
	{
		str << " DecalAlpha";
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA)
	{
		str << " ModulateAlpha";
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_DECALMASK)
	{
		str << " DecalMask";
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEMASK)
	{
		str << " ModulateMask";
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_COPY)
	{
		str << " Copy";
	}
	if (pc.dwTextureBlendCaps & D3DPTBLENDCAPS_ADD)
	{
		str << " Add";
	}
	str << endl;

	str << "Texture address caps:";
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_WRAP)
	{
		str << " Wrap";
	}
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_MIRROR)
	{
		str << " Mirror";
	}
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_CLAMP)
	{
		str << " Clamp";
	}
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_BORDER)
	{
		str << " Border";
	}
	if (pc.dwTextureAddressCaps & D3DPTADDRESSCAPS_INDEPENDENTUV)
	{
		str << " IndependentUV";
	}
	str << endl;

    str << "Stipple width: " << pc.dwStippleWidth << endl;
    str << "Stipple height: " << pc.dwStippleHeight << endl;
	return str;
}

//==========================================================================
//
//
//
//==========================================================================

ostream &operator << (ostream &str, const LPD3DDEVICEDESC7 dd)
{
	str << "Device caps:\n";
	if (dd->dwDevCaps & D3DDEVCAPS_FLOATTLVERTEX)
	{
		str << "- accepts floating point for post-transform vertex data.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_SORTINCREASINGZ)
	{
		str << "- needs data sorted for increasing depth.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_SORTDECREASINGZ)
	{
		str << "- needs data sorted for decreasing depth.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_SORTEXACT)
	{
		str << "- needs data sorted exactly.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_EXECUTESYSTEMMEMORY)
	{
		str << "- can use execute buffers from system memory.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_EXECUTEVIDEOMEMORY)
	{
		str << "- can use execute buffer from video memory.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TLVERTEXSYSTEMMEMORY)
	{
		str << "- can use buffers from system memory for transformed and lit vertices.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TLVERTEXVIDEOMEMORY)
	{
		str << "- can use buffers from video memory for transformed and lit vertices.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TEXTURESYSTEMMEMORY)
	{
		str << "- can retrieve textures from system memory.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TEXTUREVIDEOMEMORY)
	{
		str << "- can retrieve textures from device memory.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_DRAWPRIMTLVERTEX)
	{
		str << "- exports a DrawPrimitive-aware HAL.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_CANRENDERAFTERFLIP)
	{
		str << "- can queue rendering commands after a page flip.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_TEXTURENONLOCALVIDMEM)
	{
		str << "- can retrieve textures from non-local video (AGP) memory.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_DRAWPRIMITIVES2)
	{
		str << "- can support DrawPrimitives2.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES)
	{
		str << "- uses discrete texture memory pools for each stage.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_DRAWPRIMITIVES2EX)
	{
		str << "- can support Extended DrawPrimitives2 i.e. DX7 compliant driver.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		str << "- supports transformation and lighting in hardware.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_CANBLTSYSTONONLOCAL)
	{
		str << "- supports blits from system-memory textures to non-local video-memory textures.\n";
	}
	if (dd->dwDevCaps & D3DDEVCAPS_HWRASTERIZATION)
	{
		str << "- has HW acceleration for rasterization.\n";
	}
	str << endl;

	str << "-- Line caps --\n" << dd->dpcLineCaps;
	str << endl;

	str << "-- Triangle caps --\n" << dd->dpcTriCaps;
 	str << endl;

	str << "Rendering bit depth: ";
 	if (dd->dwDeviceRenderBitDepth & DDBD_8) str << "8 ";
 	if (dd->dwDeviceRenderBitDepth & DDBD_16) str << "16 ";
 	if (dd->dwDeviceRenderBitDepth & DDBD_24) str << "24 ";
 	if (dd->dwDeviceRenderBitDepth & DDBD_32) str << "32 ";
 	str << endl;
	str << "Z-Buffer bit depth: ";
 	if (dd->dwDeviceZBufferBitDepth & DDBD_8) str << "8 ";
 	if (dd->dwDeviceZBufferBitDepth & DDBD_16) str << "16 ";
 	if (dd->dwDeviceZBufferBitDepth & DDBD_24) str << "24 ";
 	if (dd->dwDeviceZBufferBitDepth & DDBD_32) str << "32 ";
 	str << endl;
	str << endl;

	str << "Min texture width: " << dd->dwMinTextureWidth << endl;
	str << "Min texture height: " << dd->dwMinTextureHeight << endl;
	str << "Max texture width: " << dd->dwMaxTextureWidth << endl;
	str << "Max texture height: " << dd->dwMaxTextureHeight << endl;
	str << endl;

	str << "Max texture repeat: " << dd->dwMaxTextureRepeat << endl;
	str << "Max texture aspect ratio: " << dd->dwMaxTextureAspectRatio << endl;
	str << "Max anisotropy: " << dd->dwMaxAnisotropy << endl;
	str << endl;

	str << "Guard band left: " << dd->dvGuardBandLeft << endl;
	str << "Guard band top: " << dd->dvGuardBandTop << endl;
	str << "Guard band right: " << dd->dvGuardBandRight << endl;
	str << "Guard band bottom: " << dd->dvGuardBandBottom << endl;
	str << endl;

	str << "Extents adjust: " << dd->dvExtentsAdjust << endl;
	str << endl;

	str << "Stencil caps: ";
	if (dd->dwStencilCaps & D3DSTENCILCAPS_DECR)
	{
		str << "DECR ";
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_DECRSAT)
	{
		str << "DECRSAT ";
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_INCR)
	{
		str << "INCR ";
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_INCRSAT)
	{
		str << "INCRSAT ";
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_INVERT)
	{
		str << "INVERT ";
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_KEEP)
	{
		str << "KEEP ";
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_REPLACE)
	{
		str << "REPLACE ";
	}
	if (dd->dwStencilCaps & D3DSTENCILCAPS_ZERO)
	{
		str << "ZERO ";
	}
	str << endl;

	str << "Flexible vertex format capabilities:\n";
	if (dd->dwFVFCaps & D3DFVFCAPS_DONOTSTRIPELEMENTS)
	{
		str << "- prefers that vertex elements not be stripped.\n";
	}
	str << "- total " << (dd->dwFVFCaps & D3DFVFCAPS_TEXCOORDCOUNTMASK) << " texture coordinate sets\n";

	str << "Supported texture operations: ";
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_ADD)
	{
		str << "ADD ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_ADDSIGNED)
	{
		str << "ADDSIGNED ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_ADDSIGNED2X)
	{
		str << "ADDSIGNED2X ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_ADDSMOOTH)
	{
		str << "ADDSMOOTH ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDCURRENTALPHA)
	{
		str << "BLENDCURRENTALPHA ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDDIFFUSEALPHA)
	{
		str << "BLENDDIFFUSEALPHA ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDFACTORALPHA)
	{
		str << "BLENDFACTORALPHA ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDTEXTUREALPHA)
	{
		str << "BLENDTEXTUREALPHA ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BLENDTEXTUREALPHAPM)
	{
		str << "BLENDTEXTUREALPHAPM ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP)
	{
		str << "BUMPENVMAP ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_BUMPENVMAPLUMINANCE)
	{
		str << "BUMPENVMAPLUMINANCE ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_DISABLE)
	{
		str << "DISABLE ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3)
	{
		str << "DOTPRODUCT3 ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATE)
	{
		str << "MODULATE ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATE2X)
	{
		str << "MODULATE2X ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATE4X)
	{
		str << "MODULATE4X ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR)
	{
		str << "MODULATEALPHA_ADDCOLOR ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA)
	{
		str << "MODULATEALPHA_ADDCOLOR ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR)
	{
		str << "MODULATEINVALPHA_ADDCOLOR ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA)
	{
		str << "MODULATEINVCOLOR_ADDALPHA ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_PREMODULATE)
	{
		str << "PREMODULATE ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_SELECTARG1)
	{
		str << "SELECTARG1 ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_SELECTARG2)
	{
		str << "SELECTARG2 ";
	}
	if (dd->dwTextureOpCaps & D3DTEXOPCAPS_SUBTRACT)
	{
		str << "SUBTRACT ";
	}
	str << endl;
	str << "Max texture blend stages: " << dd->wMaxTextureBlendStages << endl;
	str << "Max simultaneous textures: " << dd->wMaxSimultaneousTextures << endl;
	str << "Max active lights: " << dd->dwMaxActiveLights << endl;
	str << "Max vertex W: " << dd->dvMaxVertexW << endl;
	str << "Max user clip planes: " << dd->wMaxUserClipPlanes << endl;
	str << "Max vertex blend matrices: " << dd->wMaxVertexBlendMatrices << endl;

/*
    GUID        deviceGUID;

    DWORD       dwVertexProcessingCaps;
*/
	return str;
}

//==========================================================================
//
//
//
//==========================================================================

ostream &operator << (ostream &str, const LPDDPIXELFORMAT pf)
{
	if (pf->dwFlags & DDPF_ALPHAPIXELS)
		str << "- has alpha channel." << endl;
	if (pf->dwFlags & DDPF_ALPHA)
		str << "- Alpha-only surface." << endl;
	if (pf->dwFlags & DDPF_FOURCC)
		str << "- contains a FOURCC code describing a non-RGB pixel format." << endl;
	if (pf->dwFlags & DDPF_PALETTEINDEXED1)
		str << "- 1-bit color indexed." << endl;
	if (pf->dwFlags & DDPF_PALETTEINDEXED2)
		str << "- 2-bit color indexed." << endl;
	if (pf->dwFlags & DDPF_PALETTEINDEXED4)
		str << "- 4-bit color indexed." << endl;
	if (pf->dwFlags & DDPF_PALETTEINDEXED8)
		str << "- 8-bit color indexed." << endl;
	if (pf->dwFlags & DDPF_PALETTEINDEXEDTO8)
		str << "- indexed to an 8-bit palette." << endl;
	if (pf->dwFlags & DDPF_RGB)
		str << "- RGB surface." << endl;
	if (pf->dwFlags & DDPF_COMPRESSED)
		str << "- will accept pixel data in the specified format and compress it during the write operation." << endl;
	if (pf->dwFlags & DDPF_RGBTOYUV)
		str << "- translates RGB data YUV data." << endl;
	if (pf->dwFlags & DDPF_YUV)
		str << "- YUV surface." << endl;
	if (pf->dwFlags & DDPF_ZBUFFER)
		str << "- z-buffer surface." << endl;
	if (pf->dwFlags & DDPF_ZPIXELS)
		str << "- contains z information in the pixels." << endl;
	if (pf->dwFlags & DDPF_STENCILBUFFER)
		str << "- contains stencil information along with Z." << endl;
	if (pf->dwFlags & DDPF_ALPHAPREMULT)
		str << "- uses the premultiplied alpha format." << endl;
	if (pf->dwFlags & DDPF_LUMINANCE)
		str << "- luminance-only or luminance-alpha surface." << endl;
	if (pf->dwFlags & DDPF_BUMPLUMINANCE)
		str << "- luminance off bumpmap surface." << endl;
	if (pf->dwFlags & DDPF_BUMPDUDV)
		str << "- has Bump-map." << endl;
	str << "- Bit depth: " << pf->dwRGBBitCount << endl;
	str << "- Mask for red bits: " << hex << pf->dwRBitMask << dec << endl;
	str << "- Mask for green bits: " << hex << pf->dwGBitMask << dec << endl;
	str << "- Mask for blue bits: " << hex << pf->dwBBitMask << dec << endl;
	str << "- Mask for alpha/Z channel: " << hex << pf->dwRGBAlphaBitMask << dec << endl;

	return str;
}

#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.2  2001/09/14 16:48:22  dj_jl
//	Switched to DirectX 8
//
//	Revision 1.1  2001/08/30 17:40:21  dj_jl
//	Seperated from d3d_main
//	
//**************************************************************************
