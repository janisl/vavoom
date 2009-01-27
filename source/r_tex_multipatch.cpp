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
#include "r_local.h"

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
//	VMultiPatchTexture::VMultiPatchTexture
//
//==========================================================================

VMultiPatchTexture::VMultiPatchTexture(VStream& Strm, int DirectoryIndex,
	VTexture** PatchLookup, int NumPatchLookup, int FirstTex, bool IsStrife)
: Pixels(0)
{
	guard(VMultiPatchTexture::VMultiPatchTexture);
	Type = TEXTYPE_Wall;
	Format = TEXFMT_8;

	//	Read offset and seek to the starting position.
	Strm.Seek(4 + DirectoryIndex * 4);
	vint32 Offset = Streamer<vint32>(Strm);
	if (Offset < 0 || Offset >= Strm.TotalSize())
	{
		Sys_Error("InitTextures: bad texture directory");
	}
	Strm.Seek(Offset);

	//	Read name.
	char TmpName[12];
	Strm.Serialise(TmpName, 8);
	TmpName[8] = 0;
	Name = VName(TmpName, VName::AddLower8);

	//	In Doom textures were searched from the beginning, so to avoid
	// problems, especially with animated textures, set name to a blank one
	// if this one is a duplicate.
	if (GTextureManager.CheckNumForName(Name, TEXTYPE_Wall, false, false) >= FirstTex)
	{
		Name = NAME_None;
	}

	//	Skip unused value.
	Streamer<vint16>(Strm);	//	Masked, unused.

	//	Read scaling.
	vuint8 TmpSScale = Streamer<vuint8>(Strm);
	vuint8 TmpTScale = Streamer<vuint8>(Strm);
	SScale = TmpSScale ? TmpSScale / 8.0 : 1.0;
	TScale = TmpTScale ? TmpTScale / 8.0 : 1.0;

	//	Read dimensions.
	Width = Streamer<vint16>(Strm);
	Height = Streamer<vint16>(Strm);

	//	Skip unused value.
	if (!IsStrife)
		Streamer<vint32>(Strm);	//	ColumnDirectory, unused.

	//	Create list of patches.
	PatchCount = Streamer<vint16>(Strm);
	Patches = new VTexPatch[PatchCount];
	memset(Patches, 0, sizeof(VTexPatch) * PatchCount);

	//	Read patches.
	VTexPatch* patch = Patches;
	for (int i = 0; i < PatchCount; i++, patch++)
	{
		//	Read origin.
		patch->XOrigin = Streamer<vint16>(Strm);
		patch->YOrigin = Streamer<vint16>(Strm);
		patch->Rot = 0;
		patch->Trans = NULL;
		patch->bOwnTrans = false;
		patch->Blend.r = 0;
		patch->Blend.g = 0;
		patch->Blend.b = 0;
		patch->Blend.a = 0;
		patch->Style = STYLE_Copy;
		patch->Alpha = 1.0;

		//	Read patch index and find patch texture.
		vint16 PatchIdx = Streamer<vint16>(Strm);
		if (PatchIdx < 0 || PatchIdx >= NumPatchLookup)
		{
			Sys_Error("InitTextures: Bad patch index in texture %s", *Name);
		}
		patch->Tex = PatchLookup[PatchIdx];
		if (!patch->Tex)
		{
			GCon->Logf("InitTextures: Missing patch in texture %s", *Name);
			i--;
			PatchCount--;
			patch--;
		}

		//	Skip unused values.
		if (!IsStrife)
		{
			Streamer<vint16>(Strm);	//	Step dir, unused.
			Streamer<vint16>(Strm);	//	Colour map, unused.
		}
	}

	//	Fix sky texture heights for Heretic, but it can also be used
	// for Doom and Strife
	if (!VStr::NICmp(*Name, "sky", 3) && Height == 128)
	{
		if (Patches[0].Tex->GetHeight() > Height)
		{
			Height = Patches[0].Tex->GetHeight();
		}
	}
	unguard;
}

//==========================================================================
//
//	VMultiPatchTexture::VMultiPatchTexture
//
//==========================================================================

VMultiPatchTexture::VMultiPatchTexture(VScriptParser* sc, int AType)
: PatchCount(0)
, Patches(NULL)
, Pixels(0)
{
	guard(VMultiPatchTexture::VMultiPatchTexture);
	Type = AType;
	Format = TEXFMT_8;

	sc->SetCMode(true);
	sc->ExpectString();
	Name = VName(*sc->String, VName::AddLower8);
	sc->Expect(",");
	sc->ExpectNumber();
	Width = sc->Number;
	sc->Expect(",");
	sc->ExpectNumber();
	Height = sc->Number;

	if (sc->Check("{"))
	{
		TArray<VTexPatch> Parts;
		while (!sc->Check("}"))
		{
			if (sc->Check("offset"))
			{
				sc->ExpectNumber();
				SOffset = sc->Number;
				sc->Expect(",");
				sc->ExpectNumber();
				TOffset = sc->Number;
			}
			else if (sc->Check("xscale"))
			{
				sc->ExpectFloat();
				SScale = sc->Float;
			}
			else if (sc->Check("yscale"))
			{
				sc->ExpectFloat();
				TScale = sc->Float;
			}
			else if (sc->Check("worldpanning"))
			{
				bWorldPanning = true;
			}
			else if (sc->Check("nulltexture"))
			{
				Type = TEXTYPE_Null;
			}
			else if (sc->Check("nodecals"))
			{
			}
			else if (sc->Check("patch"))
			{
				VTexPatch& P = Parts.Alloc();
				sc->ExpectString();
				VName PatchName = VName(*sc->String, VName::AddLower8);
				int Tex = GTextureManager.CheckNumForName(PatchName,
					TEXTYPE_WallPatch, false, false);
				P.Tex = GTextureManager[Tex];

				//	Parse origin.
				sc->Expect(",");
				sc->ExpectNumber();
				P.XOrigin = sc->Number;
				sc->Expect(",");
				sc->ExpectNumber();
				P.YOrigin = sc->Number;

				//	Initialise parameters.
				int Flip = 0;
				P.Rot = 0;
				P.Trans = NULL;
				P.bOwnTrans = false;
				P.Blend.r = 0;
				P.Blend.g = 0;
				P.Blend.b = 0;
				P.Blend.a = 0;
				P.Style = STYLE_Copy;
				P.Alpha = 1.0;

				if (sc->Check("{"))
				{
					while (!sc->Check("}"))
					{
						if (sc->Check("flipx"))
						{
							Flip |= 1;
						}
						else if (sc->Check("flipy"))
						{
							Flip |= 2;
						}
						else if (sc->Check("rotate"))
						{
							sc->ExpectNumber();
							int Rot = ((sc->Number + 90) % 360) - 90;
							if (Rot != 0 && Rot !=90 && Rot != 180 && Rot != -90)
							{
								sc->Error("Rotation must be a multiple of 90 degrees.");
							}
							P.Rot = (Rot / 90) & 3;
						}
						else if (sc->Check("translation"))
						{
							Format = TEXFMT_RGBA;
							if (P.bOwnTrans)
							{
								delete P.Trans;
								P.bOwnTrans = false;
							}
							P.Trans = NULL;
							P.Blend.r = 0;
							P.Blend.g = 0;
							P.Blend.b = 0;
							P.Blend.a = 0;

							if (sc->Check("inverse"))
							{
								P.Trans = &ColourMaps[CM_Inverse];
							}
							else if (sc->Check("gold"))
							{
								P.Trans = &ColourMaps[CM_Gold];
							}
							else if (sc->Check("red"))
							{
								P.Trans = &ColourMaps[CM_Red];
							}
							else if (sc->Check("green"))
							{
								P.Trans = &ColourMaps[CM_Green];
							}
							else if (sc->Check("ice"))
							{
								P.Trans = &IceTranslation;
							}
							else if (sc->Check("desaturate"))
							{
								sc->Expect(",");
								sc->ExpectNumber();
							}
							else
							{
								P.Trans = new VTextureTranslation();
								P.bOwnTrans = true;
								do
								{
									sc->ExpectString();
									P.Trans->AddTransString(sc->String);
								}
								while (sc->Check(","));
							}
						}
						else if (sc->Check("blend"))
						{
							Format = TEXFMT_RGBA;
							if (P.bOwnTrans)
							{
								delete P.Trans;
								P.bOwnTrans = false;
							}
							P.Trans = NULL;
							P.Blend.r = 0;
							P.Blend.g = 0;
							P.Blend.b = 0;
							P.Blend.a = 0;

							if (!sc->CheckNumber())
							{
								sc->ExpectString();
								vuint32 Col = M_ParseColour(sc->String);
								P.Blend.r = (Col >> 16) & 0xff;
								P.Blend.g = (Col >> 8) & 0xff;
								P.Blend.b = Col & 0xff;
							}
							else
							{
								P.Blend.r = MID(0, sc->Number, 255);
								sc->Expect(",");
								sc->ExpectNumber();
								P.Blend.g = MID(0, sc->Number, 255);
								sc->Expect(",");
								sc->ExpectNumber();
								P.Blend.b = MID(0, sc->Number, 255);
								sc->Expect(",");
							}
							if (sc->Check(","))
							{
								sc->ExpectFloat();
								P.Blend.a = MID(0, int(sc->Float * 255), 254);
							}
							else
							{
								P.Blend.a = 255;
							}
						}
						else if (sc->Check("alpha"))
						{
							sc->ExpectFloat();
							P.Alpha = MID(0, sc->Float, 1);
						}
						else if (sc->Check("style"))
						{
							if (sc->Check("copy"))
							{
								P.Style = STYLE_Copy;
							}
							else if (sc->Check("translucent"))
							{
								P.Style = STYLE_Translucent;
							}
							else if (sc->Check("add"))
							{
								P.Style = STYLE_Add;
							}
							else if (sc->Check("subtract"))
							{
								P.Style = STYLE_Subtract;
							}
							else if (sc->Check("reversesubtract"))
							{
								P.Style = STYLE_ReverseSubtract;
							}
							else if (sc->Check("modulate"))
							{
								P.Style = STYLE_Modulate;
							}
							else if (sc->Check("copyalpha"))
							{
								P.Style = STYLE_CopyAlpha;
							}
							else
							{
								sc->Error("Bad style");
							}
							if (P.Style != STYLE_Copy)
							{
								Format = TEXFMT_RGBA;
							}
						}
						else
						{
							sc->Error("Bad texture patch command");
						}
					}
				}

				if (Flip & 2)
				{
					P.Rot = (P.Rot + 2) & 3;
					Flip ^= 1;
				}
				if (Flip & 1)
				{
					P.Rot |= 4;
				}
			}
			else
			{
				sc->Error("Bad texture command");
			}
		}

		PatchCount = Parts.Num();
		Patches = new VTexPatch[PatchCount];
		memcpy(Patches, Parts.Ptr(), sizeof(VTexPatch) * PatchCount);
	}

	sc->SetCMode(false);
	unguard;
}

//==========================================================================
//
//	VMultiPatchTexture::~VMultiPatchTexture
//
//==========================================================================

VMultiPatchTexture::~VMultiPatchTexture()
{
	guard(VMultiPatchTexture::~VMultiPatchTexture);
	if (Patches)
	{
		for (int i = 0; i < PatchCount; i++)
		{
			if (Patches[i].bOwnTrans)
			{
				delete Patches[i].Trans;
			}
		}
		delete[] Patches;
	}
	if (Pixels)
	{
		delete[] Pixels;
	}
	unguard;
}

//==========================================================================
//
//	VMultiPatchTexture::SetFrontSkyLayer
//
//==========================================================================

void VMultiPatchTexture::SetFrontSkyLayer()
{
	guard(VMultiPatchTexture::SetFrontSkyLayer);
	for (int i = 0; i < PatchCount; i++)
	{
		Patches[i].Tex->SetFrontSkyLayer();
	}
	bNoRemap0 = true;
	unguard;
}

//==========================================================================
//
//	VMultiPatchTexture::GetPixels
//
// 	Using the texture definition, the composite texture is created from the
// patches, and each column is cached.
//
//==========================================================================

vuint8* VMultiPatchTexture::GetPixels()
{
	guard(VMultiPatchTexture::GetPixels);
	//	If already got pixels, then just return them.
	if (Pixels)
	{
		return Pixels;
	}

	//	Load all patches, if any of them is not in standard palette, then
	// switch to 32 bit mode.
	for (int i = 0; i < PatchCount; i++)
	{
		Patches[i].Tex->GetPixels();
		if (Patches[i].Tex->Format != TEXFMT_8)
		{
			Format = TEXFMT_RGBA;
		}
	}

	if (Format == TEXFMT_8)
	{
		Pixels = new vuint8[Width * Height];
		memset(Pixels, 0, Width * Height);
	}
	else
	{
		Pixels = new vuint8[Width * Height * 4];
		memset(Pixels, 0, Width * Height * 4);
	}

	// Composite the columns together.
	VTexPatch* patch = Patches;
	for (int i = 0; i < PatchCount; i++, patch++)
	{
		VTexture* PatchTex = patch->Tex;
		vuint8* PatchPixels = patch->Trans ? PatchTex->GetPixels8() :
			PatchTex->GetPixels();
		int PWidth = PatchTex->GetWidth();
		int PHeight = PatchTex->GetHeight();
		int x1 = patch->XOrigin;
		int x2 = x1 + (patch->Rot & 1 ? PHeight : PWidth);
		if (x2 > Width)
		{
			x2 = Width;
		}
		int y1 = patch->YOrigin;
		int y2 = y1 + (patch->Rot & 1 ? PWidth : PHeight);
		if (y2 > Height)
		{
			y2 = Height;
		}
		float IAlpha = 1.0 - patch->Alpha;

		for (int y = y1 < 0 ? 0 : y1; y < y2; y++)
		{
			int PIdxY;
			switch (patch->Rot)
			{
			case 0:
			case 4:
				PIdxY = (y - y1) * PWidth;
				break;
			case 1:
			case 7:
				PIdxY = y - y1;
				break;
			case 2:
			case 6:
				PIdxY = (PHeight - y + y1 - 1) * PWidth;
				break;
			case 3:
			case 5:
				PIdxY = PWidth - y + y1 - 1;
				break;
			}

			for (int x = x1 < 0 ? 0 : x1; x < x2; x++)
			{
				int PIdx;
				switch (patch->Rot)
				{
				case 0:
				case 6:
					PIdx = (x - x1) + PIdxY;
					break;
				case 1:
				case 5:
					PIdx = (PHeight - x + x1 - 1) * PWidth + PIdxY;
					break;
				case 2:
				case 4:
					PIdx = (PWidth - x + x1 - 1) + PIdxY;
					break;
				case 3:
				case 7:
					PIdx = (x - x1) * PWidth + PIdxY;
					break;
				}

				if (Format == TEXFMT_8)
				{
					if (PatchPixels[PIdx])
					{
						Pixels[x + y * Width] = PatchPixels[PIdx];
					}
				}
				else
				{
					//	Get pixel.
					rgba_t col;
					if (patch->Trans)
					{
						col = patch->Trans->GetPalette()[PatchPixels[PIdx]];
					}
					else
					{
						switch (PatchTex->Format)
						{
						case TEXFMT_8:
							col = r_palette[PatchPixels[PIdx]];
							break;
						case TEXFMT_8Pal:
							col = PatchTex->GetPalette()[PatchPixels[PIdx]];
							break;
						case TEXFMT_RGBA:
							col = ((rgba_t*)PatchPixels)[PIdx];
							break;
						default:
							//	Shut up compiler
							col.r = 0;
							col.g = 0;
							col.b = 0;
							col.a = 0;
							break;
						}
					}

					if (patch->Blend.a == 255)
					{
						col.r = col.r * patch->Blend.r / 255;
						col.g = col.g * patch->Blend.g / 255;
						col.b = col.b * patch->Blend.b / 255;
					}
					else if (patch->Blend.a)
					{
						col.r = col.r * (255 - patch->Blend.a) / 255 +
							patch->Blend.r * patch->Blend.a / 255;
						col.g = col.g * (255 - patch->Blend.a) / 255 +
							patch->Blend.g * patch->Blend.a / 255;
						col.b = col.b * (255 - patch->Blend.a) / 255 +
							patch->Blend.b * patch->Blend.a / 255;
					}

					//	Add to texture.
					if (col.a)
					{
						rgba_t& Dst = ((rgba_t*)Pixels)[x + y * Width];
						switch (patch->Style)
						{
						case STYLE_Copy:
							Dst = col;
							break;
						case STYLE_Translucent:
							Dst.r = vuint8(Dst.r * IAlpha + col.r * patch->Alpha);
							Dst.g = vuint8(Dst.g * IAlpha + col.g * patch->Alpha);
							Dst.b = vuint8(Dst.b * IAlpha + col.b * patch->Alpha);
							Dst.a = col.a;
							break;
						case STYLE_Add:
							Dst.r = MIN(Dst.r + vuint8(col.r * patch->Alpha), 255);
							Dst.g = MIN(Dst.g + vuint8(col.g * patch->Alpha), 255);
							Dst.b = MIN(Dst.b + vuint8(col.b * patch->Alpha), 255);
							Dst.a = col.a;
							break;
						case STYLE_Subtract:
							Dst.r = MAX(Dst.r - vuint8(col.r * patch->Alpha), 0);
							Dst.g = MAX(Dst.g - vuint8(col.g * patch->Alpha), 0);
							Dst.b = MAX(Dst.b - vuint8(col.b * patch->Alpha), 0);
							Dst.a = col.a;
							break;
						case STYLE_ReverseSubtract:
							Dst.r = MAX(vuint8(col.r * patch->Alpha) - Dst.r, 0);
							Dst.g = MAX(vuint8(col.g * patch->Alpha) - Dst.g, 0);
							Dst.b = MAX(vuint8(col.b * patch->Alpha) - Dst.b, 0);
							Dst.a = col.a;
							break;
						case STYLE_Modulate:
							Dst.r = Dst.r * col.r / 255;
							Dst.g = Dst.g * col.g / 255;
							Dst.b = Dst.b * col.b / 255;
							Dst.a = col.a;
							break;
						case STYLE_CopyAlpha:
							if (col.a == 255)
							{
								Dst = col;
							}
							else
							{
								float a = col.a / 255.0;
								float ia = (255.0 - col.a) / 255.0;
								Dst.r = vuint8(Dst.r * ia + col.r * a);
								Dst.g = vuint8(Dst.g * ia + col.g * a);
								Dst.b = vuint8(Dst.b * ia + col.b * a);
								Dst.a = col.a;
							}
							break;
						}
					}
				}
			}
		}
	}

	return Pixels;
	unguard;
}

//==========================================================================
//
//	VMultiPatchTexture::Unload
//
//==========================================================================

void VMultiPatchTexture::Unload()
{
	guard(VMultiPatchTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	unguard;
}
