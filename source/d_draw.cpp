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
//**	Functions to draw patches (by post) directly to screen.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "d_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static picspanfunc_t	picspanfunc;
static byte				*picsource;

static int				ds_shade;

static void (*PutDot)(int, int, dword);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VSoftwareDrawer::DrawPicSpan_8
//
//==========================================================================

void VSoftwareDrawer::DrawPicSpan_8(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *src, void* dst)
{
	src += (t >> FRACBITS) * cachewidth;
    byte* dest = (byte*)dst;
	while (count--)
    {
		byte color = src[s >> FRACBITS];
		if (color)
		{
			*dest = color;
		}
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawPicSpanFuzz_8
//
//==========================================================================

void VSoftwareDrawer::DrawPicSpanFuzz_8(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *src, void* dst)
{
	src += (t >> FRACBITS) * cachewidth;
    byte* dest = (byte*)dst;
	while (count--)
    {
		byte color = src[s >> FRACBITS];
		if (color)
		{
			*dest = d_transluc[*dest + (color << 8)];
		}
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawPicSpanAltFuzz_8
//
//==========================================================================

void VSoftwareDrawer::DrawPicSpanAltFuzz_8(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *src, void* dst)
{
	src += (t >> FRACBITS) * cachewidth;
    byte* dest = (byte*)dst;
	while (count--)
    {
		byte color = src[s >> FRACBITS];
		if (color)
		{
			*dest = d_transluc[(*dest << 8) + color];
		}
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawPicSpanShadow_8
//
//==========================================================================

void VSoftwareDrawer::DrawPicSpanShadow_8(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *src, void* dst)
{
	src += (t >> FRACBITS) * cachewidth;
    byte* dest = (byte*)dst;
	byte *cmap = colormaps + ((ds_shade >> 3) << 8);
	while (count--)
    {
		byte color = src[s >> FRACBITS];
		if (color)
		{
			*dest = cmap[*dest];
		}
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawFlatSpan_8
//
//==========================================================================

void VSoftwareDrawer::DrawFlatSpan_8(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *src, void* dst)
{
	src += (t >> 10) & 0xfc0;
    byte* dest = (byte*)dst;
	while (count--)
    {
		byte color = src[(s >> FRACBITS) & 0x3f];
		*dest = color;
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::FillRect_8
//
//==========================================================================

void VSoftwareDrawer::FillRect_8(float x1, float y1, float x2, float y2,
	dword color)
{
	int ix1 = int(x1);
	int iy1 = int(y1);
	int ix2 = int(x2);
	int iy2 = int(y2);

	int iWid = ix2 - ix1;

	byte bColor = color = d_rgbtable[((color >> 9) & 0x7c00) | 
		((color >> 6) & 0x03e0) | ((color >> 3) & 0x1f)];

	byte *dest = (byte*)scrn + ix1 + ScreenWidth * iy1;
	for (int y = iy1; y < iy2; y++, dest += ScreenWidth)
	{
		memset(dest, bColor, iWid);
	}
}

//==========================================================================
//
//	VSoftwareDrawer::ShadeRect_8
//
//  Fade all the screen buffer, so that the menu is more readable,
// especially now that we use the small hufont in the menus...
//
//==========================================================================

void VSoftwareDrawer::ShadeRect_8(int xx, int yy, int ww, int hh, int darkening)
{
	int x1 = xx;
	int y1 = yy;
	int x2 = xx + ww;
	int y2 = yy + hh;
	if (x2 > ScreenWidth) x2 = ScreenWidth;
	if (y2 > ScreenHeight) y2 = ScreenHeight;

    byte *shades = colormaps + darkening * 256;
	for (int y = y1; y < y2; y++)
	{
		byte *dest = scrn + x1 + ScreenWidth * y;
		for (int x = x1; x < x2; x++, dest++)
		{
			*dest = shades[*dest];
		}
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawConsoleBackground_8
//
//==========================================================================

void VSoftwareDrawer::DrawConsoleBackground_8(int h)
{
	static byte *consbgmap = NULL;
    if (!consbgmap)
	{
       	consbgmap = (byte*)W_CacheLumpName("CONSMAP", PU_STATIC);
	}

	int w = ScreenWidth >> 2;
	for (int y = 0; y < h; y++)
	{
		dword* buf = (dword*)(scrn + ScreenWidth * y);
		for (int x = 0; x < w; x++)
		{
   			dword quad = buf[x];
   			byte p1 = consbgmap[quad & 255];
   			byte p2 = consbgmap[(quad >> 8) & 255];
   			byte p3 = consbgmap[(quad >> 16) & 255];
   			byte p4 = consbgmap[quad >> 24];
   			buf[x] = (p4 << 24) | (p3 << 16) | (p2 << 8) | p1;
		 }
	}
}

//==========================================================================
//
//	VSoftwareDrawer::PutDot_8
//
//==========================================================================

void VSoftwareDrawer::PutDot_8(int x, int y, dword c)
{
	((byte*)scrn)[y * ScreenWidth + x] = c;
}

//==========================================================================
//
//	VSoftwareDrawer::DrawPicSpan_16
//
//==========================================================================

void VSoftwareDrawer::DrawPicSpan_16(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *src, void* dst)
{
	src += (t >> FRACBITS) * cachewidth;
    word* dest = (word*)dst;
	while (count--)
    {
		byte color = src[s >> FRACBITS];
		if (color)
		{
			*dest = pal8_to16[color];
		}
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawSpritePicSpan_16
//
//==========================================================================

void VSoftwareDrawer::DrawSpritePicSpan_16(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *_src, void* dst)
{
	word *src = (word*)_src;
	src += (t >> FRACBITS) * cachewidth;
    word* dest = (word*)dst;
	while (count--)
    {
		word color = src[s >> FRACBITS];
		if (color)
		{
			*dest = color;
		}
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawPicSpanFuzz_16
//
//==========================================================================

void VSoftwareDrawer::DrawPicSpanFuzz_16(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *src, void* dst)
{
	src += (t >> FRACBITS) * cachewidth;
    word* dest = (word*)dst;
	while (count--)
    {
		byte color = src[s >> FRACBITS];
		if (color)
		{
			word color16 = pal8_to16[color];
			byte r1 = GetColR(*dest);
			byte g1 = GetColG(*dest);
			byte b1 = GetColB(*dest);
			byte r2 = GetColR(color16);
			byte g2 = GetColG(color16);
			byte b2 = GetColB(color16);
			byte r = (d_dsttranstab[r1] + d_srctranstab[r2]) >> 8;
			byte g = (d_dsttranstab[g1] + d_srctranstab[g2]) >> 8;
			byte b = (d_dsttranstab[b1] + d_srctranstab[b2]) >> 8;
			*dest = MakeCol(r, g, b);
		}
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawPicSpanShadow_16
//
//==========================================================================

void VSoftwareDrawer::DrawPicSpanShadow_16(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *src, void* dst)
{
	src += (t >> FRACBITS) * cachewidth;
    word* dest = (word*)dst;
	int ishade = 255 - ds_shade;
	while (count--)
    {
		byte color = src[s >> FRACBITS];
		if (color)
		{
			byte r = GetColR(*dest);
			byte g = GetColG(*dest);
			byte b = GetColB(*dest);
			*dest = MakeCol(r * ishade / 256, g * ishade / 256, b * ishade / 256);
		}
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawFlatSpan_16
//
//==========================================================================

void VSoftwareDrawer::DrawFlatSpan_16(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *src, void* dst)
{
	src += (t >> 10) & 0xfc0;
    word* dest = (word*)dst;
	while (count--)
    {
		byte color = src[(s >> FRACBITS) & 0x3f];
		*dest = pal8_to16[color];
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::FillRect_16
//
//==========================================================================

void VSoftwareDrawer::FillRect_16(float x1, float y1, float x2, float y2,
	dword color)
{
	int ix1 = int(x1);
	int iy1 = int(y1);
	int ix2 = int(x2);
	int iy2 = int(y2);

	int wid = ix2 - ix1;

	word wColor;
	if (ScreenBPP == 15)
	{
		wColor = (word)MakeCol15((byte)(color >> 16), (byte)(color >> 8),
			(byte)color);
	}
	else
	{
		wColor = (word)MakeCol16((byte)(color >> 16), (byte)(color >> 8),
			(byte)color);
	}

	for (int y = iy1; y < iy2; y++)
	{
		word *dest = (word*)scrn + ix1 + ScreenWidth * y;
		for (int i = wid; i; i--)
		{
			*dest++ = wColor;
		}
	}
}

//==========================================================================
//
//	VSoftwareDrawer::ShadeRect_16
//
//  Fade all the screen buffer, so that the menu is more readable,
// especially now that we use the small hufont in the menus...
//
//==========================================================================

void VSoftwareDrawer::ShadeRect_16(int xx, int yy, int ww, int hh, int darkening)
{
	int x1 = xx;
	int y1 = yy;
	int x2 = xx + ww;
	int y2 = yy + hh;
	if (x2 > ScreenWidth) x2 = ScreenWidth;
	if (y2 > ScreenHeight) y2 = ScreenHeight;

	darkening = 32 - darkening;
	for (int y = y1; y < y2; y++)
	{
		word *dest = (word*)scrn + x1 + ScreenWidth * y;
		for (int x = x1; x < x2; x++)
		{
			byte r = GetColR(*dest);
			byte g = GetColG(*dest);
			byte b = GetColB(*dest);
			*dest = MakeCol(r * darkening / 32, g * darkening / 32, b * darkening / 32);
			dest++;
		}
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawConsoleBackground_16
//
//==========================================================================

void VSoftwareDrawer::DrawConsoleBackground_16(int h)
{
	for (int y = 0; y < h; y++)
	{
		word *dest = (word*)scrn + ScreenWidth * y;
		for (int x = 0; x < ScreenWidth; x++)
		{
			byte r = GetColR(*dest);
			byte g = GetColG(*dest);
			byte b = GetColB(*dest);
			byte col = (r * 5 + g * 8 + b * 3) / 16;
			*dest = MakeCol(0, 0, col);
			dest++;
		}
	}
}

//==========================================================================
//
//	VSoftwareDrawer::PutDot_16
//
//==========================================================================

void VSoftwareDrawer::PutDot_16(int x, int y, dword c)
{
	((word*)scrn)[y * ScreenWidth + x] = c;
}

//==========================================================================
//
//	VSoftwareDrawer::DrawPicSpan_32
//
//==========================================================================

void VSoftwareDrawer::DrawPicSpan_32(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *src, void* dst)
{
	src += (t >> FRACBITS) * cachewidth;
    dword* dest = (dword*)dst;
	while (count--)
    {
		byte color = src[s >> FRACBITS];
		if (color)
		{
			*dest = pal2rgb[color];
		}
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawSpritePicSpan_32
//
//==========================================================================

void VSoftwareDrawer::DrawSpritePicSpan_32(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *_src, void* dst)
{
	dword *src = (dword*)_src;
	src += (t >> FRACBITS) * cachewidth;
    dword* dest = (dword*)dst;
	while (count--)
    {
		dword color = src[s >> FRACBITS];
		if (color)
		{
			*dest = color;
		}
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawPicSpanFuzz_32
//
//==========================================================================

void VSoftwareDrawer::DrawPicSpanFuzz_32(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *src, void* dst)
{
	src += (t >> FRACBITS) * cachewidth;
    dword* dest = (dword*)dst;
	while (count--)
    {
		byte color = src[s >> FRACBITS];
		if (color)
		{
			dword color32 = pal2rgb[color];
			byte r1 = GetCol32R(*dest);
			byte g1 = GetCol32G(*dest);
			byte b1 = GetCol32B(*dest);
			byte r2 = GetCol32R(color32);
			byte g2 = GetCol32G(color32);
			byte b2 = GetCol32B(color32);
			byte r = (d_dsttranstab[r1] + d_srctranstab[r2]) >> 8;
			byte g = (d_dsttranstab[g1] + d_srctranstab[g2]) >> 8;
			byte b = (d_dsttranstab[b1] + d_srctranstab[b2]) >> 8;
			*dest = MakeCol32(r, g, b);
		}
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawPicSpanShadow_32
//
//==========================================================================

void VSoftwareDrawer::DrawPicSpanShadow_32(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *src, void* dst)
{
	src += (t >> FRACBITS) * cachewidth;
    dword* dest = (dword*)dst;
	int ishade = 255 - ds_shade;
	while (count--)
    {
		byte color = src[s >> FRACBITS];
		if (color)
		{
			byte r = GetCol32R(*dest);
			byte g = GetCol32G(*dest);
			byte b = GetCol32B(*dest);
			*dest = MakeCol32(r * ishade / 256, g * ishade / 256, b * ishade / 256);
		}
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawFlatSpan_32
//
//==========================================================================

void VSoftwareDrawer::DrawFlatSpan_32(fixed_t s, fixed_t t, fixed_t sstep,
	int count, byte *src, void* dst)
{
	src += (t >> 10) & 0xfc0;
    dword* dest = (dword*)dst;
	while (count--)
    {
		byte color = src[(s >> FRACBITS) & 0x3f];
		*dest = pal2rgb[color];
		s += sstep;
		dest++;
	}
}

//==========================================================================
//
//	VSoftwareDrawer::FillRect_32
//
//==========================================================================

void VSoftwareDrawer::FillRect_32(float x1, float y1, float x2, float y2,
	dword color)
{
	int ix1 = int(x1);
	int iy1 = int(y1);
	int ix2 = int(x2);
	int iy2 = int(y2);

	int wid = ix2 - ix1;

	color = MakeCol32((byte)(color >> 16), (byte)(color >> 8), (byte)color);

	for (int y = iy1; y < iy2; y++)
	{
		dword *dest = (dword*)scrn + ix1 + ScreenWidth * y;
		for (int i = wid; i; i--)
		{
			*dest++ = color;
		}
	}
}

//==========================================================================
//
//	VSoftwareDrawer::ShadeRect_32
//
//  Fade all the screen buffer, so that the menu is more readable,
// especially now that we use the small hufont in the menus...
//
//==========================================================================

void VSoftwareDrawer::ShadeRect_32(int xx, int yy, int ww, int hh, int darkening)
{
	int x1 = xx;
	int y1 = yy;
	int x2 = xx + ww;
	int y2 = yy + hh;
	if (x2 > ScreenWidth) x2 = ScreenWidth;
	if (y2 > ScreenHeight) y2 = ScreenHeight;

	darkening = 32 - darkening;
	for (int y = y1; y < y2; y++)
	{
		dword *dest = (dword*)scrn + x1 + ScreenWidth * y;
		for (int x = x1; x < x2; x++)
		{
			byte r = GetCol32R(*dest);
			byte g = GetCol32G(*dest);
			byte b = GetCol32B(*dest);
			*dest = MakeCol32(r * darkening / 32, g * darkening / 32, b * darkening / 32);
			dest++;
		}
	}
}

//==========================================================================
//
//	VSoftwareDrawer::DrawConsoleBackground_32
//
//==========================================================================

void VSoftwareDrawer::DrawConsoleBackground_32(int h)
{
	for (int y = 0; y < h; y++)
	{
		dword *dest = (dword*)scrn + ScreenWidth * y;
		for (int x = 0; x < ScreenWidth; x++)
		{
			byte r = GetCol32R(*dest);
			byte g = GetCol32G(*dest);
			byte b = GetCol32B(*dest);
			byte col = (r * 5 + g * 8 + b * 3) / 16;
			*dest = MakeCol32(0, 0, col);
			dest++;
		}
	}
}

//==========================================================================
//
//	VSoftwareDrawer::PutDot_32
//
//==========================================================================

void VSoftwareDrawer::PutDot_32(int x, int y, dword c)
{
	((dword*)scrn)[y * ScreenWidth + x] = c;
}

//==========================================================================
//
//	VSoftwareDrawer::DrawPic
//
//==========================================================================

void VSoftwareDrawer::DrawPic(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2)
{
	if (x1 < 0)
	{
		s1 = s1 + x1 / (x1 - x2) * (s2 - s1);
		x1 = 0;
	}
	if (x2 > ScreenWidth)
	{
		s2 = s2 + (x2 - ScreenWidth) / (x1 - x2) * (s2 - s1);
		x2 = ScreenWidth;
	}
	if (y1 < 0)
	{
		t1 = t1 + y1 / (y1 - y2) * (t2 - t1);
		y1 = 0;
	}
	if (y2 > ScreenHeight)
	{
		t2 = t2 + (y2 - ScreenHeight) / (y1 - y2) * (t2 - t1);
		y2 = ScreenHeight;
	}

	int top = (int)y1;
	int bot = (int)y2;
	int lines = bot - top;
	if (lines <= 0)
	{
		return;
	}

	int left = (int)x1;
	int right = (int)x2;
	int count = right - left;
	if (count <= 0)
	{
		return;
	}

	fixed_t sstep = FX((s2 - s1) / (x2 - x1));
	fixed_t tstep = FX((t2 - t1) / (y2 - y1));

	fixed_t s = FX(s1);
	fixed_t t = FX(t1);

	byte* dst = (byte *)scrn + (left + top * ScreenWidth) * PixelBytes;
	int DstStep = ScreenWidth * PixelBytes;

	do
	{
		picspanfunc(s, t, sstep, count, picsource, dst);
		dst += DstStep;
		t += tstep;
	}
	while (--lines);
}

//==========================================================================
//
//	VSoftwareDrawer::DrawPic
//
//==========================================================================

void VSoftwareDrawer::DrawPic(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2, int handle, int trans)
{
	guard(VSoftwareDrawer::DrawPic);
	picsource = SetPic(handle);
	if (ScreenBPP == 8)
	{
		if (trans < 5)
		{
			picspanfunc = DrawPicSpan_8;
		}
		else if (trans < 45)
		{
			d_transluc = tinttables[(trans - 5) / 10];
			picspanfunc = DrawPicSpanFuzz_8;
		}
		else if (trans < 95)
		{
			d_transluc = tinttables[(94 - trans) / 10];
			picspanfunc = DrawPicSpanAltFuzz_8;
		}
		else
		{
			return;
		}
	}
	else if (PixelBytes == 2)
	{
		if (trans <= 0)
		{
			picspanfunc = DrawPicSpan_16;
		}
		else if (trans < 100)
		{
			int trindex = trans * 31 / 100;
			d_dsttranstab = scaletable[trindex];
			d_srctranstab = scaletable[31 - trindex];
			picspanfunc = DrawPicSpanFuzz_16;
		}
		else
		{
			return;
		}
	}
	else
	{
		if (trans <= 0)
		{
			picspanfunc = DrawPicSpan_32;
		}
		else if (trans < 100)
		{
			int trindex = trans * 31 / 100;
			d_dsttranstab = scaletable[trindex];
			d_srctranstab = scaletable[31 - trindex];
			picspanfunc = DrawPicSpanFuzz_32;
		}
		else
		{
			return;
		}
	}
	DrawPic(x1, y1, x2, y2, s1, t1, s2, t2);
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::DrawPicShadow
//
//==========================================================================

void VSoftwareDrawer::DrawPicShadow(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2, int handle, int shade)
{
	guard(VSoftwareDrawer::DrawPicShadow);
	picsource = SetPic(handle);
	ds_shade = shade;
	if (ScreenBPP == 8)
	{
		picspanfunc = DrawPicSpanShadow_8;
	}
	else if (PixelBytes == 2)
	{
		picspanfunc = DrawPicSpanShadow_16;
	}
	else
	{
		picspanfunc = DrawPicSpanShadow_32;
	}
	DrawPic(x1, y1, x2, y2, s1, t1, s2, t2);
	unguard;
}

//==========================================================================
//
//  VSoftwareDrawer::FillRectWithFlat
//
// 	Fills rectangle with flat.
//
//==========================================================================

void VSoftwareDrawer::FillRectWithFlat(float x1, float y1, float x2, float y2,
	float s1, float t1, float s2, float t2, const char* fname)
{
	guard(VSoftwareDrawer::FillRectWithFlat);
	SetTexture(GTextureManager.NumForName(FName(fname, FNAME_AddLower8),
		TEXTYPE_Flat, true, true));
	picsource = (byte*)cacheblock;
	if (ScreenBPP == 8)
	{
		picspanfunc = DrawFlatSpan_8;
	}
	else if (PixelBytes == 2)
	{
		picspanfunc = DrawFlatSpan_16;
	}
	else
	{
		picspanfunc = DrawFlatSpan_32;
	}
	DrawPic(x1, y1, x2, y2, s1, t1, s2, t2);
	unguard;
}

//==========================================================================
//
//  VSoftwareDrawer::FillRect
//
// 	Fills rectangle with color.
//
//==========================================================================

void VSoftwareDrawer::FillRect(float x1, float y1, float x2, float y2,
	dword color)
{
	guard(VSoftwareDrawer::FillRect);
	if (ScreenBPP == 8)
	{
		FillRect_8(x1, y1, x2, y2, color);
	}
	else if (PixelBytes == 2)
	{
		FillRect_16(x1, y1, x2, y2, color);
	}
	else
	{
		FillRect_32(x1, y1, x2, y2, color);
	}
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::ShadeRect
//
//  Fade all the screen buffer, so that the menu is more readable,
// especially now that we use the small hufont in the menus...
//
//==========================================================================

void VSoftwareDrawer::ShadeRect(int x, int y, int w, int h, int darkening)
{
	guard(VSoftwareDrawer::ShadeRect);
	if (ScreenBPP == 8)
		ShadeRect_8(x, y, w, h, darkening);
	else if (PixelBytes == 2)
		ShadeRect_16(x, y, w, h, darkening);
	else
		ShadeRect_32(x, y, w, h, darkening);
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::DrawConsoleBackground
//
//==========================================================================

void VSoftwareDrawer::DrawConsoleBackground(int h)
{
	guard(VSoftwareDrawer::DrawConsoleBackground);
	if (ScreenBPP == 8)
		DrawConsoleBackground_8(h);
	else if (PixelBytes == 2)
		DrawConsoleBackground_16(h);
	else
		DrawConsoleBackground_32(h);
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::DrawSpriteLump
//
//==========================================================================

void VSoftwareDrawer::DrawSpriteLump(float x1, float y1, float x2, float y2,
	int lump, int translation, boolean flip)
{
	guard(VSoftwareDrawer::DrawSpriteLump);
	float w = GTextureManager.Textures[lump]->GetWidth();
	float h = GTextureManager.Textures[lump]->GetHeight();

	SetSpriteLump(lump, 0xffffffff, translation);
	picsource = (byte*)cacheblock;
	picspanfunc = ScreenBPP == 8 ? DrawPicSpan_8 :
		PixelBytes == 2 ? DrawSpritePicSpan_16 : DrawSpritePicSpan_32;

    if (flip)
		DrawPic(x1, y1, x2, y2, w - 0.0001, 0, 0, h);
    else
		DrawPic(x1, y1, x2, y2, 0, 0, w, h);
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::StartAutomap
//
//==========================================================================

void VSoftwareDrawer::StartAutomap(void)
{
	guard(VSoftwareDrawer::StartAutomap);
	if (PixelBytes == 1)
		PutDot = PutDot_8;
	else if (PixelBytes == 2)
		PutDot = PutDot_16;
	else
		PutDot = PutDot_32;
	unguard;
}

#if 0
//==========================================================================
//
//  AM_PutDot
//
//==========================================================================

static void AM_PutDot(short x, short y, byte *cc, byte *cm)
{
	byte		*oldcc=cc;

    short		xx;
    short		yy;

	xx = (int)(x * fScaleXI);
    yy = (int)(y * fScaleYI);

	//	Fade on boarders
	if (xx < 32)
		cc += 7 - (xx >> 2);
	else if (xx > (AM_W - 32))
		cc += 7 - ((AM_W - xx) >> 2);
	if (yy < 32)
		cc += 7 - (yy >> 2);
	else if (yy > (AM_H - 32))
		cc += 7 - ((AM_H - yy) >> 2);

	if (cc > cm && cm != NULL)
	{
		cc = cm;
	}
	else if (cc > oldcc + 6) // don't let the color escape from the fade table...
	{
		cc = oldcc + 6;
	}

	Drawer->PutDot(x, y, *(cc));
}

//==========================================================================
//
//  DrawWuLine
//
//	Wu antialiased line drawer.
//	(X0,Y0),(X1,Y1) = line to draw
//	BaseColor = color # of first color in block used for antialiasing, the
//          100% intensity version of the drawing color
//	NumLevels = size of color block, with BaseColor+NumLevels-1 being the
//          0% intensity version of the drawing color
//	IntensityBits = log base 2 of NumLevels; the # of bits used to describe
//          the intensity of the drawing color. 2**IntensityBits==NumLevels
//
//==========================================================================

static void DrawWuLine(int X0, int Y0, int X1, int Y1, byte *BaseColor,
	int NumLevels, unsigned short IntensityBits)
{
	unsigned short IntensityShift, ErrorAdj, ErrorAcc;
	unsigned short ErrorAccTemp, Weighting, WeightingComplementMask;
	short DeltaX, DeltaY, Temp, XDir;

	/* Make sure the line runs top to bottom */
	if (Y0 > Y1)
	{
    	Temp = Y0; Y0 = Y1; Y1 = Temp;
		Temp = X0; X0 = X1; X1 = Temp;
	}
   	/* Draw the initial pixel, which is always exactly intersected by
      the line and so needs no weighting */
	AM_PutDot(X0, Y0, &BaseColor[0], NULL);

	if ((DeltaX = X1 - X0) >= 0)
	{
    	XDir = 1;
	}
	else
	{
    	XDir = -1;
		DeltaX = -DeltaX; /* make DeltaX positive */
	}
	/* Special-case horizontal, vertical, and diagonal lines, which
      require no weighting because they go right through the center of
      every pixel */
	if ((DeltaY = Y1 - Y0) == 0)
	{
    	/* Horizontal line */
		while (DeltaX-- != 0)
		{
        	X0 += XDir;
			AM_PutDot(X0, Y0, &BaseColor[0], NULL);
		}
		return;
	}
	if (DeltaX == 0)
	{
		/* Vertical line */
		do
		{
			Y0++;
			AM_PutDot(X0, Y0, &BaseColor[0], NULL);
		} while (--DeltaY != 0);
		return;
	}
	//diagonal line.
	if (DeltaX == DeltaY)
	{
		do
		{
			X0 += XDir;
			Y0++;
			AM_PutDot(X0, Y0, &BaseColor[0], NULL);
		} while (--DeltaY != 0);
		return;
	}
   /* Line is not horizontal, diagonal, or vertical */
   ErrorAcc = 0;  /* initialize the line error accumulator to 0 */
   /* # of bits by which to shift ErrorAcc to get intensity level */
   IntensityShift = 16 - IntensityBits;
   /* Mask used to flip all bits in an intensity weighting, producing the
      result (1 - intensity weighting) */
   WeightingComplementMask = NumLevels - 1;
   /* Is this an X-major or Y-major line? */
   if (DeltaY > DeltaX)
   {
      /* Y-major line; calculate 16-bit fixed-point fractional part of a
         pixel that X advances each time Y advances 1 pixel, truncating the
         result so that we won't overrun the endpoint along the X axis */
      ErrorAdj = (word)(((unsigned long) DeltaX << 16) / (unsigned long) DeltaY);
      /* Draw all pixels other than the first and last */
      while (--DeltaY)
      {
         ErrorAccTemp = ErrorAcc;   /* remember currrent accumulated error */
         ErrorAcc += ErrorAdj;      /* calculate error for next pixel */
         if (ErrorAcc <= ErrorAccTemp)
         {
            /* The error accumulator turned over, so advance the X coord */
            X0 += XDir;
         }
         Y0++; /* Y-major, so always advance Y */
         /* The IntensityBits most significant bits of ErrorAcc give us the
            intensity weighting for this pixel, and the complement of the
            weighting for the paired pixel */
         Weighting = ErrorAcc >> IntensityShift;
			AM_PutDot(X0, Y0, &BaseColor[Weighting], &BaseColor[7]);
         AM_PutDot(X0 + XDir, Y0,
               &BaseColor[(Weighting ^ WeightingComplementMask)], &BaseColor[7]);
      }
      /* Draw the final pixel, which is always exactly intersected by the line
         and so needs no weighting */
      AM_PutDot(X1, Y1, &BaseColor[0], NULL);
      return;
   }
   /* It's an X-major line; calculate 16-bit fixed-point fractional part of a
      pixel that Y advances each time X advances 1 pixel, truncating the
      result to avoid overrunning the endpoint along the X axis */
   ErrorAdj = (word)(((unsigned long) DeltaY << 16) / (unsigned long) DeltaX);
   /* Draw all pixels other than the first and last */
   while (--DeltaX)
   {
      ErrorAccTemp = ErrorAcc;   /* remember currrent accumulated error */
      ErrorAcc += ErrorAdj;      /* calculate error for next pixel */
      if (ErrorAcc <= ErrorAccTemp)
      {
         /* The error accumulator turned over, so advance the Y coord */
         Y0++;
      }
      X0 += XDir; /* X-major, so always advance X */
      /* The IntensityBits most significant bits of ErrorAcc give us the
         intensity weighting for this pixel, and the complement of the
         weighting for the paired pixel */
      Weighting = ErrorAcc >> IntensityShift;
      AM_PutDot(X0, Y0, &BaseColor[Weighting], &BaseColor[7]);
      AM_PutDot(X0, Y0 + 1,
      		&BaseColor[(Weighting ^ WeightingComplementMask)], &BaseColor[7]);

   }
   /* Draw the final pixel, which is always exactly intersected by the line
      and so needs no weighting */
   AM_PutDot(X1, Y1, &BaseColor[0], NULL);
}
#endif

//==========================================================================
//
//	VSoftwareDrawer::DrawLine
//
//==========================================================================

void VSoftwareDrawer::DrawLine(int x1, int y1, dword color, int x2, int y2, dword)
{
	guard(VSoftwareDrawer::DrawLine);
    register int x;
    register int y;
    register int dx;
    register int dy;
    register int sx;
    register int sy;
    register int ax;
    register int ay;
    register int d;
    register int e;

#if 0
	if (color == WALLCOLORS)
	{
		DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, &antialias[0][0], 8, 3);
		return;
	}
	if (color == FDWALLCOLORS)
	{
		DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, &antialias[1][0], 8, 3);
		return;
	}
	if (color == CDWALLCOLORS)
	{
		DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y, &antialias[2][0], 8, 3);
		return;
	}
#endif

	if (ScreenBPP == 8)
		color = d_rgbtable[((color >> 9) & 0x7c00) | ((color >> 6) & 0x03e0) | ((color >> 3) & 0x1f)];
	else if (ScreenBPP == 15)
		color = MakeCol15((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
	else if (ScreenBPP == 16)
		color = MakeCol16((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
	else
		color = MakeCol32((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);

    dx = x2 - x1;
    ax = 2 * (dx < 0 ? -dx : dx);
	sx = dx < 0 ? -1 : 1;

    dy = y2 - y1;
	ay = 2 * (dy < 0 ? -dy : dy);
    sy = dy < 0 ? -1 : 1;

    x = x1;
	y = y1;

	if (ax > ay)
	{
		d = ay - ax/2;
       	e = x2;
		while (1)
		{
			PutDot(x, y, color);
    		if (x == e) return;
	    	if (d >= 0)
		    {
				y += sy;
				d -= ax;
		    }
	    	x += sx;
		    d += ay;
		}
    }
    else
	{
		d = ax - ay/2;
        e = y2;
		while (1)
		{
			PutDot(x, y, color);
	    	if (y == e) return;
		    if (d >= 0)
		    {
				x += sx;
				d -= ay;
    		}
		    y += sy;
		    d += ax;
		}
    }
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::EndAutomap
//
//==========================================================================

void VSoftwareDrawer::EndAutomap(void)
{
}

//**************************************************************************
//
//	$Log$
//	Revision 1.15  2005/05/26 16:50:14  dj_jl
//	Created texture manager class
//
//	Revision 1.14  2002/11/16 17:11:15  dj_jl
//	Improving software driver class.
//	
//	Revision 1.13  2002/07/13 07:38:00  dj_jl
//	Added drawers to the object tree.
//	
//	Revision 1.12  2002/03/20 19:09:53  dj_jl
//	DeepSea tall patches support.
//	
//	Revision 1.11  2002/01/15 18:30:43  dj_jl
//	Some fixes and improvements suggested by Malcolm Nixon
//	
//	Revision 1.10  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.9  2001/11/09 14:20:48  dj_jl
//	Drawing of images with different palettes
//	
//	Revision 1.8  2001/09/12 17:31:27  dj_jl
//	Rectangle drawing and direct update for plugins
//	
//	Revision 1.7  2001/08/29 17:49:01  dj_jl
//	Line colors in RGBA format
//	
//	Revision 1.6  2001/08/23 17:47:22  dj_jl
//	Started work on pics with custom palettes
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
