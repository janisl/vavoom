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

#include "gamedefs.h"
#include "r_tex.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

typedef VTexture* (*VTexCreateFunc)(VStream&, int);

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VTexture::CreateTexture
//
//==========================================================================

VTexture* VTexture::CreateTexture(int Type, int LumpNum)
{
	guard(VTexture::CreateTexture);
	static const struct
	{
		VTexCreateFunc	Create;
		int				Type;
	} TexTable[] =
	{
		{ VImgzTexture::Create,		TEXTYPE_Any },
		{ VPngTexture::Create,		TEXTYPE_Any },
		{ VJpegTexture::Create,		TEXTYPE_Any },
		{ VPcxTexture::Create,		TEXTYPE_Any },
		{ VTgaTexture::Create,		TEXTYPE_Any },
		{ VFlatTexture::Create,		TEXTYPE_Flat },
		{ VRawPicTexture::Create,	TEXTYPE_Pic },
		{ VPatchTexture::Create,	TEXTYPE_Any },
		{ VAutopageTexture::Create,	TEXTYPE_Autopage },
	};

	if (LumpNum < 0)
	{
		return NULL;
	}
	VStream* Strm = W_CreateLumpReaderNum(LumpNum);

	for (size_t i = 0; i < ARRAY_COUNT(TexTable); i++)
	{
		if (TexTable[i].Type == Type || TexTable[i].Type == TEXTYPE_Any)
		{
			VTexture* Tex = TexTable[i].Create(*Strm, LumpNum);
			if (Tex)
			{
				Tex->Type = Type;
				delete Strm;
				Strm = NULL;
				return Tex;
			}
		}
	}

	delete Strm;
	Strm = NULL;
	return NULL;
	unguard;
}

//==========================================================================
//
//	VTexture::VTexture
//
//==========================================================================

VTexture::VTexture()
: Type(TEXTYPE_Any)
, Format(TEXFMT_8)
, Name(NAME_None)
, Width(0)
, Height(0)
, SOffset(0)
, TOffset(0)
, bNoRemap0(false)
, bWorldPanning(false)
, bIsCameraTexture(false)
, WarpType(0)
, SScale(1)
, TScale(1)
, TextureTranslation(0)
, DriverData(0)
, Pixels8Bit(0)
, HiResTexture(0)
, Pixels8BitValid(false)
, SourceLump(-1)
{
}

//==========================================================================
//
//	VTexture::~VTexture
//
//==========================================================================

VTexture::~VTexture()
{
	if (Pixels8Bit)
	{
		delete[] Pixels8Bit;
		Pixels8Bit = NULL;
	}
	if (HiResTexture)
	{
		delete HiResTexture;
		HiResTexture = NULL;
	}
}

//==========================================================================
//
//	VTexture::SetFrontSkyLayer
//
//==========================================================================

void VTexture::SetFrontSkyLayer()
{
	guardSlow(VTexture::SetFrontSkyLayer);
	bNoRemap0 = true;
	unguardSlow;
}

//==========================================================================
//
//	VTexture::CheckModified
//
//==========================================================================

bool VTexture::CheckModified()
{
	return false;
}

//==========================================================================
//
//	VTexture::GetPixels8
//
//==========================================================================

vuint8* VTexture::GetPixels8()
{
	guard(VTexture::GetPixels8);
	//	If already have converted version, then just return it.
	if (Pixels8Bit && Pixels8BitValid)
	{
		return Pixels8Bit;
	}

	vuint8* Pixels = GetPixels();
	if (Format == TEXFMT_8Pal)
	{
		//	Remap to game palette
		int NumPixels = Width * Height;
		rgba_t* Pal = GetPalette();
		vuint8 Remap[256];
		Remap[0] = 0;
		int i;
		for (i = 1; i < 256; i++)
		{
			Remap[i] = r_rgbtable[((Pal[i].r << 7) & 0x7c00) +
				((Pal[i].g << 2) & 0x3e0) + ((Pal[i].b >> 3) & 0x1f)];
		}

		if (!Pixels8Bit)
		{
			Pixels8Bit = new vuint8[NumPixels];
		}
		vuint8* pSrc = Pixels;
		vuint8* pDst = Pixels8Bit;
		for (i = 0; i < NumPixels; i++, pSrc++, pDst++)
		{
			*pDst = Remap[*pSrc];
		}
		Pixels8BitValid = true;
		return Pixels8Bit;
	}
	else if (Format == TEXFMT_RGBA)
	{
		int NumPixels = Width * Height;
		if (!Pixels8Bit)
		{
			Pixels8Bit = new vuint8[NumPixels];
		}
		rgba_t* pSrc = (rgba_t*)Pixels;
		vuint8* pDst = Pixels8Bit;
		for (int i = 0; i < NumPixels; i++, pSrc++, pDst++)
		{
			if (pSrc->a < 128)
				*pDst = 0;
			else
				*pDst = r_rgbtable[((pSrc->r << 7) & 0x7c00) +
					((pSrc->g << 2) & 0x3e0) + ((pSrc->b >> 3) & 0x1f)];
		}
		Pixels8BitValid = true;
		return Pixels8Bit;
	}
	return Pixels;
	unguard;
}

//==========================================================================
//
//	VTexture::GetPalette
//
//==========================================================================

rgba_t* VTexture::GetPalette()
{
	guardSlow(VTexture::GetPalette);
	return r_palette;
	unguardSlow;
}

//==========================================================================
//
//	VTexture::GetHighResolutionTexture
//
//	Return high-resolution version of this texture, or self if it doesn't
// exist.
//
//==========================================================================

VTexture* VTexture::GetHighResolutionTexture()
{
	guard(VTexture::GetHighResolutionTexture);
#ifdef CLIENT
	if (!r_hirestex)
	{
		return NULL;
	}
	//	If high resolution texture is already created, then just return it.
	if (HiResTexture)
	{
		return HiResTexture;
	}

	//	Determine directory name depending on type.
	const char* DirName;
	switch (Type)
	{
	case TEXTYPE_Wall:
		DirName = "walls";
		break;
	case TEXTYPE_Flat:
		DirName = "flats";
		break;
	case TEXTYPE_Overload:
		DirName = "textures";
		break;
	case TEXTYPE_Sprite:
		DirName = "sprites";
		break;
	case TEXTYPE_Pic:
	case TEXTYPE_Autopage:
		DirName = "graphics";
		break;
	default:
		return NULL;
	}

	//	Try to find it.
	static const char* Exts[] = { "png", "jpg", "tga", NULL };
	int LumpNum = W_FindLumpByFileNameWithExts(VStr("hirestex/") + DirName +
		"/" + *Name, Exts);
	if (LumpNum >= 0)
	{
		//	Create new high-resolution texture.
		HiResTexture = CreateTexture(Type, LumpNum);
		HiResTexture->Name = Name;
		return HiResTexture;
	}
#endif
	//	No hi-res texture found.
	return NULL;
	unguard;
}

//==========================================================================
//
//	VTexture::FixupPalette
//
//==========================================================================

void VTexture::FixupPalette(vuint8* Pixels, rgba_t* Palette)
{
	guard(VTexture::FixupPalette);
	//	Find black colour for remaping.
	int i;
	int black = 0;
	int best_dist = 0x10000;
	for (i = 1; i < 256; i++)
	{
		int dist = Palette[i].r * Palette[i].r + Palette[i].g *
				Palette[i].g + Palette[i].b * Palette[i].b;
		if (dist < best_dist && Palette[i].a == 255)
		{
			black = i;
			best_dist = dist;
		}
	}
	for (i = 0; i < Width * Height; i++)
	{
		if (Palette[Pixels[i]].a == 0)
			Pixels[i] = 0;
		else if (!Pixels[i])
			Pixels[i] = black;
	}
	Palette[0].r = 0;
	Palette[0].g = 0;
	Palette[0].b = 0;
	Palette[0].a = 0;
	unguard;
}

//==========================================================================
//
//	VTexture::FindDriverTrans
//
//==========================================================================

VTexture::VTransData* VTexture::FindDriverTrans(
	VTextureTranslation* TransTab, int CMap)
{
	guard(VTexture::FindDriverTrans);
	for (int i = 0; i < DriverTranslated.Num(); i++)
	{
		if (DriverTranslated[i].Trans == TransTab &&
			DriverTranslated[i].ColourMap == CMap)
		{
			return &DriverTranslated[i];
		}
	}
	return NULL;
	unguard;
}

//==========================================================================
//
//	VTexture::AdjustGamma
//
//==========================================================================

void VTexture::AdjustGamma(rgba_t* data, int size)
{
#ifdef CLIENT
	guard(VTexture::AdjustGamma);
	vuint8* gt = gammatable[usegamma];
	for (int i = 0; i < size; i++)
	{
		data[i].r = gt[data[i].r];
		data[i].g = gt[data[i].g];
		data[i].b = gt[data[i].b];
	}
	unguard;
#endif
}

//==========================================================================
//
//	VTexture::SmoothEdges
//
//	This one comes directly from GZDoom
//
//==========================================================================

#define CHKPIX(ofs) (l1[(ofs) * 4 + MSB] == 255 ? (( ((vuint32*)l1)[0] = ((vuint32*)l1)[ofs] & SOME_MASK), trans = true ) : false)

void VTexture::SmoothEdges(vuint8* buffer, int w, int h, vuint8* dataout)
{
	guard(VTexture::SmoothEdges);
	int x, y;
	//	Why the fuck you would use 0 on big endian here?
	int MSB = 3;
	vuint32 SOME_MASK = GBigEndian ? 0xffffff00 : 0x00ffffff;

	bool trans = buffer[MSB] == 0;	// If I set this to false here the code won't detect textures
									// that only contain transparent pixels.
	vuint8* l1;

	if (h <= 1 || w <= 1)
	{
		return;  // makes (a) no sense and (b) doesn't work with this code!
	}
	l1 = buffer;

	if (l1[MSB] == 0 && !CHKPIX(1))
	{
		CHKPIX(w);
	}
	l1 += 4;

	for (x = 1; x < w - 1; x++, l1 += 4)
	{
		if (l1[MSB] == 0 && !CHKPIX(-1) && !CHKPIX(1))
		{
			CHKPIX(w);
		}
	}
	if (l1[MSB] == 0 && !CHKPIX(-1))
	{
		CHKPIX(w);
	}
	l1 += 4;

	for (y = 1; y < h - 1; y++)
	{
		if (l1[MSB] == 0 && !CHKPIX(-w) && !CHKPIX(1))
		{
			CHKPIX(w);
		}
		l1 += 4;

		for (x = 1; x < w - 1; x++, l1 += 4)
		{
			if (l1[MSB] == 0 && !CHKPIX(-w) && !CHKPIX(-1) && !CHKPIX(1) && !CHKPIX(-w - 1) && !CHKPIX(-w + 1) && !CHKPIX(w - 1) && !CHKPIX(w + 1))
			{
				CHKPIX(w);
			}
		}
		if (l1[MSB] == 0 && !CHKPIX(-w) && !CHKPIX(-1))
		{
			CHKPIX(w);
		}
		l1 += 4;
	}

	if (l1[MSB] == 0 && !CHKPIX(-w))
	{
		CHKPIX(1);
	}
	l1 += 4;
	for (x = 1; x < w - 1; x++, l1 += 4)
	{
		if (l1[MSB] == 0 && !CHKPIX(-w) && !CHKPIX(-1))
		{
			CHKPIX(1);
		}
	}
	if (l1[MSB] == 0 && !CHKPIX(-w))
	{
		CHKPIX(-1);
	}

	dataout = l1;
	unguard;
}

//==========================================================================
//
//	VTexture::ResampleTexture
//
//	Resizes	texture.
//	This is a simplified version of gluScaleImage from sources of MESA 3.0
//
//==========================================================================

void VTexture::ResampleTexture(int widthin, int heightin,
	const vuint8* datain, int widthout, int heightout, vuint8* dataout, int sampling_type)
{
	guard(VTexture::ResampleTexture);
	int i, j, k;
	float sx, sy;

	if (widthout > 1)
	{
		sx = float(widthin - 1) / float(widthout - 1);
	}
	else
	{
		sx = float(widthin - 1);
	}
	if (heightout > 1)
	{
		sy = float(heightin - 1) / float(heightout - 1);
	}
	else
	{
		sy = float(heightin - 1);
	}

	if (sampling_type == 1)
	{
		// Use point sample
		for (i = 0; i < heightout; i++)
		{
			int ii = int(i * sy);
			for (j = 0; j < widthout; j++)
			{
				int jj = int(j * sx);

				const vuint8* src = datain + (ii * widthin + jj) * 4;
				vuint8* dst = dataout + (i * widthout + j) * 4;

				for (k = 0; k < 4; k++)
				{
					*dst++ = *src++;
				}
			}
		}
	}
	else
	{
		// Use weighted sample
		if (sx <= 1.0 && sy <= 1.0)
		{
			/* magnify both width and height:  use weighted sample of 4 pixels */
			int i0, i1, j0, j1;
			float alpha, beta;
			const vuint8 *src00, *src01, *src10, *src11;
			float s1, s2;
			vuint8* dst;

			for (i = 0; i < heightout; i++)
			{
				i0 = int(i * sy);
				i1 = i0 + 1;
				if (i1 >= heightin)
				{
					i1 = heightin - 1;
				}
				alpha = i * sy - i0;
				for (j = 0; j < widthout; j++)
				{
					j0 = int(j * sx);
					j1 = j0 + 1;
					if (j1 >= widthin)
					{
						j1 = widthin - 1;
					}
					beta = j * sx - j0;

					/* compute weighted average of pixels in rect (i0,j0)-(i1,j1) */
					src00 = datain + (i0 * widthin + j0) * 4;
					src01 = datain + (i0 * widthin + j1) * 4;
					src10 = datain + (i1 * widthin + j0) * 4;
					src11 = datain + (i1 * widthin + j1) * 4;

					dst = dataout + (i * widthout + j) * 4;

					for (k = 0; k < 4; k++)
					{
						s1 = *src00++ * (1.0 - beta) + *src01++ * beta;
						s2 = *src10++ * (1.0 - beta) + *src11++ * beta;
						*dst++ = vuint8(s1 * (1.0 - alpha) + s2 * alpha);
					}
				}
			}
		}
		else
		{
			/* shrink width and/or height:  use an unweighted box filter */
			int i0, i1;
			int j0, j1;
			int ii, jj;
			int sum;
			vuint8* dst;

			for (i = 0; i < heightout; i++)
			{
				i0 = int(i * sy);
				i1 = i0 + 1;
				if (i1 >= heightin)
				{
					i1 = heightin - 1;
				}
				for (j = 0; j < widthout; j++)
				{
					j0 = int(j * sx);
					j1 = j0 + 1;
					if (j1 >= widthin)
					{
						j1 = widthin - 1;
					}

					dst = dataout + (i * widthout + j) * 4;

					/* compute average of pixels in the rectangle (i0,j0)-(i1,j1) */
					for (k = 0; k < 4; k++)
					{
						sum = 0;
						for (ii = i0; ii <= i1; ii++)
						{
							for (jj = j0; jj <= j1; jj++)
							{
								sum += *(datain + (ii * widthin + jj) * 4 + k);
							}
						}
						sum /= (j1 - j0 + 1) * (i1 - i0 + 1);
						*dst++ = vuint8(sum);
					}
				}
			}
		}
	}
	unguard;
}

//==========================================================================
//
//	VTexture::MipMap
//
//	Scales image down for next mipmap level, operates in place
//
//==========================================================================

void VTexture::MipMap(int width, int height, vuint8* InIn)
{
	guard(VTexture::MipMap);
	vuint8* in = InIn;
	int		i, j;
	vuint8* out = in;

	if (width == 1 || height == 1)
	{
		//	Special case when only one dimension is scaled
		int total = width * height / 2;
		for (i = 0; i < total; i++, in += 8, out += 4)
		{
			out[0] = vuint8((in[0] + in[4]) >> 1);
			out[1] = vuint8((in[1] + in[5]) >> 1);
			out[2] = vuint8((in[2] + in[6]) >> 1);
			out[3] = vuint8((in[3] + in[7]) >> 1);
		}
		return;
	}

	//	Scale down in both dimensions
	width <<= 2;
	height >>= 1;
	for (i = 0; i < height; i++, in += width)
	{
		for (j = 0; j < width; j += 8, in += 8, out += 4)
		{
			out[0] = vuint8((in[0] + in[4] + in[width + 0] + in[width + 4]) >> 2);
			out[1] = vuint8((in[1] + in[5] + in[width + 1] + in[width + 5]) >> 2);
			out[2] = vuint8((in[2] + in[6] + in[width + 2] + in[width + 6]) >> 2);
			out[3] = vuint8((in[3] + in[7] + in[width + 3] + in[width + 7]) >> 2);
		}
	}
	unguard;
}

//==========================================================================
//
//	VDummyTexture::VDummyTexture
//
//==========================================================================

VDummyTexture::VDummyTexture()
{
	Type = TEXTYPE_Null;
	Format = TEXFMT_8;
}

//==========================================================================
//
//	VDummyTexture::GetPixels
//
//==========================================================================

vuint8* VDummyTexture::GetPixels()
{
	return NULL;
}

//==========================================================================
//
//	VDummyTexture::Unload
//
//==========================================================================

void VDummyTexture::Unload()
{
}
