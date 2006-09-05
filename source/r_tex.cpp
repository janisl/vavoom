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
//**	Preparation of data for rendering, generation of lookups, caching,
//**  retrieval by name.
//**
//** 	Graphics.
//**
//** 	DOOM graphics for walls and sprites is stored in vertical runs of
//**  opaque pixels (posts). A column is composed of zero or more posts, a
//**  patch or sprite is composed of zero or more columns.
//**
//** 	Texture definition.
//**
//** 	Each texture is composed of one or more patches, with patches being
//**  lumps stored in the WAD. The lumps are referenced by number, and
//**  patched into the rectangular texture space using origin and possibly
//**  other attributes.
//**
//** 	Texture definition.
//**
//** 	A DOOM wall texture is a list of patches which are to be combined in
//**  a predefined order.
//**
//** 	A single patch from a texture definition, basically a rectangular
//**  area within the texture rectangle.
//**
//** 	A maptexturedef_t describes a rectangular texture, which is composed
//**  of one or more mappatch_t structures that arrange graphic patches.
//**
//** 	MAPTEXTURE_T CACHING
//**
//** 	When a texture is first needed, it counts the number of composite
//**  columns required in the texture and allocates space for a column
//**  directory and any new columns. The directory will simply point inside
//**  other patches if there is only one patch in a given column, but any
//**  columns with multiple patches will have new column_ts generated.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <png.h>
#include "gamedefs.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

//	This one is missing in older versions of libpng
#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr)		((png_ptr)->jmpbuf)
#endif

// TYPES -------------------------------------------------------------------

struct frameDef_t
{
	int		index;
	short	baseTime;
	short	randomRange;
};

struct animDef_t
{
	int		index;
	float	time;
	int		currentFrameDef;
	int		startFrameDef;
	int		endFrameDef;
	bool	IsRange;
	bool	Backwards;
	int		CurrentRangeFrame;
};

#pragma pack(1)

struct pcx_t
{
	vint8		manufacturer;
	vint8		version;
	vint8		encoding;
	vint8		bits_per_pixel;

	vuint16		xmin;
	vuint16		ymin;
	vuint16		xmax;
	vuint16		ymax;

	vuint16		hres;
	vuint16		vres;

	vuint8		palette[48];

	vint8		reserved;
	vint8		color_planes;
	vuint16		bytes_per_line;
	vuint16		palette_type;

	vint8		filler[58];

	friend VStream& operator<<(VStream& Strm, pcx_t& h)
	{
		Strm << h.manufacturer << h.version << h.encoding << h.bits_per_pixel
			<< h.xmin << h.ymin << h.xmax << h.ymax << h.hres << h.vres;
		Strm.Serialise(h.palette, 48);
		Strm << h.reserved << h.color_planes << h.bytes_per_line
			<< h.palette_type;
		Strm.Serialise(h.filler, 58);
		return Strm;
	}
};

struct tgaHeader_t
{
	vuint8		id_length;
	vuint8		pal_type;
	vuint8		img_type;
	vuint16		first_colour;
	vuint16		pal_colours;
	vuint8		pal_entry_size;
	vuint16		left;
	vuint16		top;
	vuint16		width;
	vuint16		height;
	vuint8		bpp;
	vuint8		descriptor_bits;

	friend VStream& operator<<(VStream& Strm, tgaHeader_t& h)
	{
		return Strm << h.id_length << h.pal_type << h.img_type
			<< h.first_colour << h.pal_colours << h.pal_entry_size << h.left
			<< h.top << h.width << h.height << h.bpp << h.descriptor_bits;
	}
};

#pragma pack()

//
//	A dummy texture.
//
class VDummyTexture : public VTexture
{
public:
	VDummyTexture();
	vuint8* GetPixels();
	void Unload();
};

//
//	A standard Doom patch.
//
class TPatchTexture : public VTexture
{
public:
	int			LumpNum;
	vuint8*		Pixels;

	TPatchTexture(int InType, int InLumpNum);
	~TPatchTexture();
	void GetDimensions();
	vuint8* GetPixels();
	void Unload();
};

//
//	A texture defined in TEXTURE1/TEXTURE2 lumps.
//	A maptexturedef_t describes a rectangular texture, which is composed of
// one or more mappatch_t structures that arrange graphic patches
//
class VMultiPatchTexture : public VTexture
{
private:
	struct VTexPatch
	{
		//	Block origin (allways UL), which has allready accounted for the
		// internal origin of the patch.
		short		XOrigin;
		short		YOrigin;
		VTexture*	Tex;
	};

	//	All the Patches[PatchCount] are drawn back to front into the cached
	// texture.
	int				PatchCount;
	VTexPatch*		Patches;
	vuint8*			Pixels;

public:
	VMultiPatchTexture(VStream& Strm, int DirectoryIndex,
		VTexture** PatchLookup, int NumPatchLookup, bool IsStrife);
	~VMultiPatchTexture();
	void SetFrontSkyLayer();
	vuint8* GetPixels();
	void Unload();
};

//
//	A standard Doom flat.
//
class VFlatTexture : public VTexture
{
public:
	int			LumpNum;
	vuint8*		Pixels;

	VFlatTexture(int InLumpNum);
	~VFlatTexture();
	vuint8* GetPixels();
	void Unload();
};

//
//	Raven's raw screens.
//
class VRawPicTexture : public VTexture
{
public:
	int			LumpNum;
	int			PalLumpNum;
	vuint8*		Pixels;
	rgba_t*		Palette;

	VRawPicTexture(int InLumpNum, int InPalLumpNum);
	~VRawPicTexture();
	vuint8* GetPixels();
	rgba_t* GetPalette();
	void Unload();
};

//
//	ZDoom's IMGZ grapnics.
// [RH] Just a format I invented to avoid WinTex's palette remapping
// when I wanted to insert some alpha maps.
//
class VImgzTexture : public VTexture
{
public:
	int			LumpNum;
	vuint8*		Pixels;

	VImgzTexture(int InType, int InLumpNum);
	~VImgzTexture();
	void GetDimensions();
	vuint8* GetPixels();
	void Unload();
};

//
//	A PCX file.
//
class VPcxTexture : public VTexture
{
public:
	vuint8*		Pixels;
	rgba_t*		Palette;

	VPcxTexture(int InType, VName InName);
	~VPcxTexture();
	void GetDimensions();
	vuint8* GetPixels();
	rgba_t* GetPalette();
	void Unload();
};

//
//	A TGA file.
//
class VTgaTexture : public VTexture
{
public:
	vuint8*		Pixels;
	rgba_t*		Palette;

	VTgaTexture(int InType, VName InName);
	~VTgaTexture();
	void GetDimensions();
	vuint8* GetPixels();
	rgba_t* GetPalette();
	void Unload();
};

//
//	A PNG file.
//
class VPngTexture : public VTexture
{
public:
	int			LumpNum;
	vuint8*		Pixels;
	rgba_t*		Palette;

	VPngTexture(int InType, int InLumpNum);
	VPngTexture(int InType, VName InName);
	~VPngTexture();
	void GetDimensions();
	static void ReadFunc(png_structp png, png_bytep data, png_size_t len);
	vuint8* GetPixels();
	rgba_t* GetPalette();
	void Unload();
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

//
//	Texture manager
//
VTextureManager		GTextureManager;

//
// Flats data
//
int					skyflatnum;			// sky mapping

//
//	Main palette
//
rgba_t				r_palette[256];
vuint8				r_black_colour;

//	Switches
TArray<TSwitch>		Switches;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TArray<animDef_t>	AnimDefs;
static TArray<frameDef_t>	FrameDefs;

// CODE --------------------------------------------------------------------

//BEGIN VTextureManager

//==========================================================================
//
//	VTextureManager::VTextureManager
//
//==========================================================================

VTextureManager::VTextureManager()
: RgbTable(NULL)
{
}

//==========================================================================
//
//	VTextureManager::Init
//
//==========================================================================

void VTextureManager::Init()
{
	guard(VTextureManager::Init);
	//	Add a dummy texture.
	AddTexture(new VDummyTexture);

	//	Initialise wall textures.
	InitTextures();

	//	Initialise flats.
	InitFlats();

	//	Initialise overloaded textures.
	InitOverloads();

	//	Initialise sprites.
	InitSpriteLumps();

	//	Find sky flat number.
	skyflatnum = CheckNumForName(NAME_f_sky, TEXTYPE_Flat, true, false);
	if (skyflatnum < 0)
		skyflatnum = CheckNumForName(NAME_f_sky001, TEXTYPE_Flat, true, false);
	if (skyflatnum < 0)
		skyflatnum = NumForName(NAME_f_sky1, TEXTYPE_Flat, true, false);
	unguard;
}

//==========================================================================
//
//	VTextureManager::Shutdown
//
//==========================================================================

void VTextureManager::Shutdown()
{
	guard(VTextureManager::Shutdown);
	for (int i = 0; i < Textures.Num(); i++)
		delete Textures[i];
	Textures.Clear();
	if (RgbTable)
		delete[] RgbTable;
	unguard;
}

//==========================================================================
//
//	VTextureManager::AddTexture
//
//==========================================================================

int VTextureManager::AddTexture(VTexture* Tex)
{
	guard(VTextureManager::AddTexture);
	Textures.Append(Tex);
	Tex->TextureTranslation = Textures.Num() - 1;
	return Textures.Num() - 1;
	unguard;
}

//==========================================================================
//
//  VTextureManager::CheckNumForName
//
// 	Check whether texture is available. Filter out NoTexture indicator.
//
//==========================================================================

int	VTextureManager::CheckNumForName(VName Name, int Type, bool bOverload,
	bool bCheckAny)
{
	guard(VTextureManager::CheckNumForName);
	//	Check for "NoTexture" marker.
	if ((*Name)[0] == '-' && (*Name)[1] == 0)
		return 0;

	for (int i = Textures.Num() - 1; i >= 0; i--)
	{
		if (Textures[i]->Name != Name)
			continue;

		if (Type == TEXTYPE_Any || Textures[i]->Type == Type ||
			(bOverload && Textures[i]->Type == TEXTYPE_Overload))
		{
			return i;
		}
	}

	if (bCheckAny)
	{
		return CheckNumForName(Name, TEXTYPE_Any, bOverload, false);
	}

	return -1;
	unguard;
}

//==========================================================================
//
// 	VTextureManager::NumForName
//
// 	Calls R_CheckTextureNumForName, aborts with error message.
//
//==========================================================================

int	VTextureManager::NumForName(VName Name, int Type, bool bOverload,
	bool bCheckAny)
{
	guard(VTextureManager::NumForName);
	int i = CheckNumForName(Name, Type, bOverload, bCheckAny);
	if (i == -1)
	{
		Host_Error("VTextureManager::NumForName: %s not found", *Name);
	}
	return i;
	unguard;
}

//==========================================================================
//
//	VTextureManager::TextureHeight
//
//==========================================================================

float VTextureManager::TextureHeight(int TexNum)
{
	guard(VTextureManager::TextureHeight);
	return Textures[TexNum]->GetHeight() / Textures[TexNum]->TScale;
	unguard;
}

//==========================================================================
//
//	VTextureManager::TextureAnimation
//
//==========================================================================

int VTextureManager::TextureAnimation(int InTex)
{
	guard(VTextureManager::TextureAnimation);
	return Textures[InTex]->TextureTranslation;
	unguard;
}

//==========================================================================
//
//	VTextureManager::SetFrontSkyLayer
//
//==========================================================================

void VTextureManager::SetFrontSkyLayer(int tex)
{
	guard(VTextureManager::SetFrontSkyLayer);
	Textures[tex]->SetFrontSkyLayer();
	unguard;
}

//==========================================================================
//
//	VTextureManager::GetTextureInfo
//
//==========================================================================

void VTextureManager::GetTextureInfo(int TexNum, picinfo_t* info)
{
	guard(VTextureManager::GetTextureInfo);
	if (TexNum < 0)
	{
		memset(info, 0, sizeof(*info));
	}
	else
	{
		VTexture* Tex = Textures[TexNum];
		info->width = Tex->GetWidth();
		info->height = Tex->GetHeight();
		info->xoffset = Tex->SOffset;
		info->yoffset = Tex->TOffset;
	}
	unguard;
}

//==========================================================================
//
//	VTextureManager::AddPatch
//
//==========================================================================

int VTextureManager::AddPatch(VName Name, int Type)
{
	guard(VTextureManager::AddPatch);
	//	Find the lump number.
	int LumpNum = W_CheckNumForName(Name, WADNS_Graphics);
	if (LumpNum < 0)
		LumpNum = W_CheckNumForName(Name, WADNS_Sprites);
	if (LumpNum < 0)
	{
		GCon->Logf("VTextureManager::AddPatch: Pic %s not found", *Name);
		return -1;
	}

	//	Check if it's already registered.
	int i = CheckNumForName(Name, Type);
	if (i >= 0)
	{
		return i;
	}

	//	Create new patch texture.
	return CreatePatch(Type, LumpNum);
	unguard;
}

//==========================================================================
//
//	VTextureManager::CreatePatch
//
//==========================================================================

int VTextureManager::CreatePatch(int Type, int LumpNum)
{
	guard(VTextureManager::CreatePatch);
	//	Read lump header and see what type of lump it is.
	vuint8 LumpHeader[16];
	memset(LumpHeader, 0, sizeof(LumpHeader));
	W_ReadFromLump(LumpNum, LumpHeader, 0, MIN(W_LumpLength(LumpNum), 16));

	//	Check for ZDoom's IMGZ image.
	if (LumpHeader[0] == 'I' && LumpHeader[1] == 'M' &&
		LumpHeader[2] == 'G' && LumpHeader[3] == 'Z')
	{
		return AddTexture(new VImgzTexture(Type, LumpNum));
	}

	//	Check for PNG image.
	if (LumpHeader[0] == 137 && LumpHeader[1] == 'P' &&
		LumpHeader[2] == 'N' && LumpHeader[3] == 'G')
	{
		return AddTexture(new VPngTexture(Type, LumpNum));
	}

	//	Check for automap background.
	if (W_LumpName(LumpNum) == NAME_autopage)
	{
		return AddTexture(new VRawPicTexture(LumpNum, -1));
	}

	if (W_LumpLength(LumpNum) == 64000)
	{
		return AddTexture(new VRawPicTexture(LumpNum, -1));
	}

	return AddTexture(new TPatchTexture(Type, LumpNum));
	unguard;
}

//==========================================================================
//
//	VTextureManager::AddRawWithPal
//
//	Adds a raw image with custom palette lump. It's here to support
// Heretic's episode 2 finale pic.
//
//==========================================================================

int VTextureManager::AddRawWithPal(VName Name, VName PalName)
{
	guard(VTextureManager::AddRawWithPal);
	int LumpNum = W_CheckNumForName(Name, WADNS_Graphics);
	if (LumpNum < 0)
	{
		GCon->Logf("VTextureManager::AddRawWithPal: %s not found", *Name);
		return -1;
	}
	//	Check if lump's size to see if it really is a raw image. If not,
	// load it as regular image.
	if (W_LumpLength(LumpNum) != 64000)
	{
		GCon->Logf("VTextureManager::AddRawWithPal: %s doesn't appear to be"
			" a raw image", *Name);
		return AddPatch(Name, TEXTYPE_Pic);
	}

	int i = CheckNumForName(Name, TEXTYPE_Pic);
	if (i >= 0)
	{
		return i;
	}

	return AddTexture(new VRawPicTexture(LumpNum,
		W_GetNumForName(PalName)));
	unguard;
}

//==========================================================================
//
//	VTextureManager::AddFileTexture
//
//==========================================================================

int VTextureManager::AddFileTexture(VName Name, int Type)
{
	guard(VTextureManager::AddFileTexture)
	int i = CheckNumForName(Name, Type);
	if (i >= 0)
	{
		return i;
	}

	VStr Ext = VStr(Name).ExtractFileExtension();
	if (Ext == "pcx")
	{
		return AddTexture(new VPcxTexture(Type, Name));
	}
	else if (Ext == "tga")
	{
		return AddTexture(new VTgaTexture(Type, Name));
	}
	else if (Ext == "png")
	{
		return AddTexture(new VPngTexture(Type, Name));
	}
	else
	{
		Sys_Error("Unsupported texture type (%s).", *Name);
		return -1;
	}
	unguard;
}

//==========================================================================
//
//	VTextureManager::InitTextures
//
// 	Initializes the texture list with the textures from the world map.
//
//==========================================================================

void VTextureManager::InitTextures()
{
	guard(VTextureManager::InitTextures);
	//	Load the patch names from pnames.lmp.
	VStream* Strm = W_CreateLumpReaderName(NAME_pnames);
	vint32 nummappatches = Streamer<vint32>(*Strm);
	VTexture** patchtexlookup = new VTexture*[nummappatches];
	for (int i = 0; i < nummappatches; i++)
	{
		//	Read patch name.
		char TmpName[12];
		Strm->Serialise(TmpName, 8);
		TmpName[8] = 0;
		VName PatchName(TmpName, VName::AddLower8);

		//	Get wad lump number.
		int LNum = W_CheckNumForName(PatchName, WADNS_Patches);
		//	Sprites also can be used as patches.
		if (LNum < 0)
		{
			LNum = W_CheckNumForName(PatchName, WADNS_Sprites);
		}

		//	Add it to textures.
		if (LNum < 0)
		{
			patchtexlookup[i] = NULL;
		}
		else
		{
			patchtexlookup[i] = Textures[CreatePatch(TEXTYPE_WallPatch, LNum)];
		}
	}
	delete Strm;

	//	Load the map texture definitions from textures.lmp.
	//	The data is contained in one or two lumps, TEXTURE1 for shareware,
	// plus TEXTURE2 for commercial.
	Strm = W_CreateLumpReaderName(NAME_texture1);
	vint32 NumTex = Streamer<vint32>(*Strm);

	//	Check the texture file format.
	bool IsStrife = false;
	vint32 PrevOffset = Streamer<vint32>(*Strm);
	for (int i = 0; i < NumTex - 1; i++)
	{
		vint32 Offset = Streamer<vint32>(*Strm);
		if (Offset - PrevOffset == 24)
		{
			IsStrife = true;
			GCon->Log(NAME_Init, "Strife textures detected");
			break;
		}
		PrevOffset = Offset;
	}

	for (int i = 0; i < NumTex; i++)
	{
		AddTexture(new VMultiPatchTexture(*Strm, i,
			patchtexlookup, nummappatches, IsStrife));
	}
	delete Strm;

	//	Read texture2 if present.
	if (W_CheckNumForName(NAME_texture2) != -1)
	{
		Strm = W_CreateLumpReaderName(NAME_texture2);
		NumTex = Streamer<vint32>(*Strm);
		for (int i = 0; i < NumTex; i++)
		{
			AddTexture(new VMultiPatchTexture(*Strm, i,
				patchtexlookup, nummappatches, IsStrife));
		}
		delete Strm;
	}

	delete[] patchtexlookup;
	unguard;
}

//==========================================================================
//
//	VTextureManager::InitFlats
//
//==========================================================================

void VTextureManager::InitFlats()
{
	guard(VTextureManager::InitFlats);
	for (int Lump = W_IterateNS(-1, WADNS_Flats); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Flats))
	{
		AddTexture(new VFlatTexture(Lump));
	}
	unguard;
}

//==========================================================================
//
//	VTextureManager::InitOverloads
//
//==========================================================================

void VTextureManager::InitOverloads()
{
	guard(VTextureManager::InitOverloads);
	for (int Lump = W_IterateNS(-1, WADNS_NewTextures); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_NewTextures))
	{
		CreatePatch(TEXTYPE_Overload, Lump);
	}
	unguard;
}

//==========================================================================
//
//	VTextureManager::InitSpriteLumps
//
//==========================================================================

void VTextureManager::InitSpriteLumps()
{
	guard(VTextureManager::InitSpriteLumps);
	for (int Lump = W_IterateNS(-1, WADNS_Sprites); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Sprites))
	{
		CreatePatch(TEXTYPE_Sprite, Lump);
	}
	unguard;
}

//==========================================================================
//
//	VTexture::VTexture
//
//==========================================================================

vuint8* VTextureManager::GetRgbTable()
{
	if (!RgbTable)
	{
		VStream* Strm = W_CreateLumpReaderName(NAME_rgbtable);
		check(Strm);
		check(Strm->TotalSize() == 0x8001);
		RgbTable = new vuint8[0x8001];
		Strm->Serialise(RgbTable, 0x8001);
		delete Strm;
	}
	return RgbTable;
}

//END

//BEGIN VTexture

//**************************************************************************
//	VTexture
//**************************************************************************

//==========================================================================
//
//	VTexture::VTexture
//
//==========================================================================

VTexture::VTexture()
: Type(TEXTYPE_Any)
, Format(TEXFMT_8)
, Name(NAME_None)
, Width(-1)
, Height(-1)
, SOffset(0)
, TOffset(0)
, bNoRemap0(false)
, SScale(1)
, TScale(1)
, TextureTranslation(0)
, DriverData(0)
, Pixels8Bit(0)
, HiResTexture(0)
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
	}
	if (HiResTexture)
	{
		delete HiResTexture;
	}
}

//==========================================================================
//
//	VTexture::GetDimensions
//
//==========================================================================

void VTexture::GetDimensions()
{
	guardSlow(VTexture::GetDimensions);
	Width = 0;
	Height = 0;
	unguardSlow;
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
//	VTexture::GetPixels8
//
//==========================================================================

vuint8* VTexture::GetPixels8()
{
	guard(VTexture::GetPixels8);
	//	If already have converted version, then just return it.
	if (Pixels8Bit)
	{
		return Pixels8Bit;
	}

	vuint8* Pixels = GetPixels();
	if (Format == TEXFMT_8Pal)
	{
		//	Remap to game palette
		vuint8* RGBTable = GTextureManager.GetRgbTable();
		int NumPixels = Width * Height;
		rgba_t* Pal = GetPalette();
		vuint8 Remap[256];
		Remap[0] = 0;
		int i;
		for (i = 1; i < 256; i++)
		{
			Remap[i] = RGBTable[((Pal[i].r << 7) & 0x7c00) +
				((Pal[i].g << 2) & 0x3e0) + ((Pal[i].b >> 3) & 0x1f)];
		}

		Pixels8Bit = new vuint8[NumPixels];
		vuint8* pSrc = Pixels;
		vuint8* pDst = Pixels8Bit;
		for (i = 0; i < NumPixels; i++, pSrc++, pDst++)
		{
			*pDst = Remap[*pSrc];
		}
		return Pixels8Bit;
	}
	else if (Format == TEXFMT_RGBA)
	{
		vuint8* RGBTable = GTextureManager.GetRgbTable();
		int NumPixels = Width * Height;
		Pixels8Bit = new vuint8[NumPixels];
		rgba_t* pSrc = (rgba_t*)Pixels;
		vuint8* pDst = Pixels8Bit;
		for (int i = 0; i < NumPixels; i++, pSrc++, pDst++)
		{
			if (pSrc->a < 128)
				*pDst = 0;
			else
				*pDst = RGBTable[((pSrc->r << 7) & 0x7c00) +
					((pSrc->g << 2) & 0x3e0) + ((pSrc->b >> 3) & 0x1f)];
		}
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
		DirName = "graphics";
		break;
	default:
		return NULL;
	}

	//	First try PNG.
	char HighResName[80];
	sprintf(HighResName, "hirestex/%s/%s.png", DirName, *Name);
	if (FL_FileExists(HighResName))
	{
		//	Create new high-resolution texture.
		HiResTexture = new VPngTexture(Type, HighResName);
		return HiResTexture;
	}

	//	Then try TGA.
	sprintf(HighResName, "hirestex/%s/%s.tga", DirName, *Name);
	if (FL_FileExists(HighResName))
	{
		//	Create new high-resolution texture.
		HiResTexture = new VTgaTexture(Type, HighResName);
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

//END

//BEGIN VDummyTexture

//**************************************************************************
//	VDummyTexture
//**************************************************************************

//==========================================================================
//
//	VDummyTexture::VDummyTexture
//
//==========================================================================

VDummyTexture::VDummyTexture()
{
	Type = TEXTYPE_Any;
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

//END

//BEGIN TPatchTexture

//**************************************************************************
//	TPatchTexture
//**************************************************************************

//==========================================================================
//
//	TPatchTexture::TPatchTexture
//
//==========================================================================

TPatchTexture::TPatchTexture(int InType, int InLumpNum)
: LumpNum(InLumpNum)
, Pixels(0)
{
	Type = InType;
	Name = W_LumpName(InLumpNum);
	Format = TEXFMT_8;
}

//==========================================================================
//
//	TPatchTexture::~TPatchTexture
//
//==========================================================================

TPatchTexture::~TPatchTexture()
{
	guard(TPatchTexture::~TPatchTexture);
	if (Pixels)
	{
		delete[] Pixels;
	}
	unguard;
}

//==========================================================================
//
//	TPatchTexture::GetDimensions
//
//==========================================================================

void TPatchTexture::GetDimensions()
{
	guard(TPatchTexture::GetDimensions);
	VStream* Strm = W_CreateLumpReaderNum(LumpNum);
	Width = Streamer<vint16>(*Strm);
	Height = Streamer<vint16>(*Strm);
	SOffset = Streamer<vint16>(*Strm);
	TOffset = Streamer<vint16>(*Strm);
	delete Strm;
	unguard;
}

//==========================================================================
//
//	TPatchTexture::GetPixels
//
//==========================================================================

vuint8* TPatchTexture::GetPixels()
{
	guard(TPatchTexture::GetPixels);
	//	If already got pixels, then just return them.
	if (Pixels)
	{
		return Pixels;
	}

	//	Open stream.
	VStream* Strm = W_CreateLumpReaderNum(LumpNum);

	//	Make sure header is present.
	if (Strm->TotalSize() < 8)
	{
		GCon->Logf("Patch %s is too small", *Name);
		Width = 1;
		Height = 1;
		SOffset = 0;
		TOffset = 0;
		Pixels = new vuint8[1];
		Pixels[0] = 0;
		return Pixels;
	}

	//	Read header.
	Width = Streamer<vint16>(*Strm);
	Height = Streamer<vint16>(*Strm);
	SOffset = Streamer<vint16>(*Strm);
	TOffset = Streamer<vint16>(*Strm);

	//	Allocate image data.
	Pixels = new vuint8[Width * Height];
	memset(Pixels, 0, Width * Height);

	//	Make sure all column offsets are there.
	if (Strm->TotalSize() < 8 + Width * 4)
	{
		GCon->Logf("Patch %s is too small", *Name);
		return Pixels;
	}

	//	Read data.
	for (int x = 0; x < Width; x++)
	{
		//	Get offset of the column.
		Strm->Seek(8 + x * 4);
		vint32 Offset = Streamer<vint32>(*Strm);
		if (Offset < 8 + Width * 4 || Offset >= Strm->TotalSize() - 1)
		{
			GCon->Logf("Bad offset in patch %s", *Name);
			continue;
		}
		Strm->Seek(Offset);

		// step through the posts in a column
		int top = -1;	//	DeepSea tall patches support
		vuint8 TopDelta;
		*Strm << TopDelta;
		while (TopDelta != 0xff)
		{
			//	Make sure length is there.
			if (Strm->TotalSize() - Strm->Tell() < 2)
			{
				GCon->Logf("Broken column in patch %s", *Name);
				break;
			}

			//	Calculate top offset.
			if (TopDelta <= top)
			{
				top += TopDelta;
			}
			else
			{
				top = TopDelta;
			}

			//	Read column length and skip unused byte.
			vuint8 Len;
			*Strm << Len;
			Streamer<vuint8>(*Strm);

			//	Make sure column doesn't go out of the bounds of the image.
			if (top + Len > Height)
			{
				GCon->Logf("Column too long in patch %s", *Name);
				break;
			}

			//	Make sure all post data is there.
			if (Strm->TotalSize() - Strm->Tell() < Len)
			{
				GCon->Logf("Broken column in patch %s", *Name);
				break;
			}

			//	Read post, convert colour 0 to black if needed.
			int count = Len;
			vuint8* dest = Pixels + x + top * Width;
			while (count--)
			{
				*Strm << *dest;
				if (!*dest && !bNoRemap0)
					*dest = r_black_colour;
				dest += Width;
			}

			//	Make sure unused byte and next post's top offset is there.
			if (Strm->TotalSize() - Strm->Tell() < 2)
			{
				GCon->Logf("Broken column in patch %s", *Name);
				break;
			}

			//	Skip unused byte and get top offset of the next post.
			Streamer<vuint8>(*Strm);
			*Strm << TopDelta;
		}
	}

	//	Close stream.
	delete Strm;

	return Pixels;
	unguard;
}

//==========================================================================
//
//	TPatchTexture::Unload
//
//==========================================================================

void TPatchTexture::Unload()
{
	guard(TPatchTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	unguard;
}

//END

//BEGIN VMultiPatchTexture

//**************************************************************************
//	VMultiPatchTexture
//**************************************************************************

//==========================================================================
//
//	VMultiPatchTexture::VMultiPatchTexture
//
//==========================================================================

VMultiPatchTexture::VMultiPatchTexture(VStream& Strm, int DirectoryIndex,
	VTexture** PatchLookup, int NumPatchLookup, bool IsStrife)
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

		//	Read patch index and find patch texture.
		vint16 PatchIdx = Streamer<vint16>(Strm);
		if (PatchIdx < 0 || PatchIdx >= NumPatchLookup)
		{
			Sys_Error("InitTextures: Bad patch index in texture %s", *Name);
		}
		patch->Tex = PatchLookup[PatchIdx];
		if (!patch->Tex)
		{
			Sys_Error("InitTextures: Missing patch in texture %s", *Name);
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
//	VMultiPatchTexture::~VMultiPatchTexture
//
//==========================================================================

VMultiPatchTexture::~VMultiPatchTexture()
{
	guard(VMultiPatchTexture::~VMultiPatchTexture);
	if (Patches)
	{
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
	Format = TEXFMT_8;
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

		// Composite the columns together.
		VTexPatch* patch = Patches;
		for (int i = 0; i < PatchCount; i++, patch++)
		{
			VTexture* PatchTex = patch->Tex;
			vuint8* PatchPixels = PatchTex->GetPixels8();
			int x1 = patch->XOrigin;
			int x2 = x1 + PatchTex->GetWidth();
			if (x2 > Width)
				x2 = Width;
			int y1 = patch->YOrigin;
			int y2 = y1 + PatchTex->GetHeight();
			if (y2 > Height)
				y2 = Height;
			for (int y = y1 < 0 ? 0 : y1; y < y2; y++)
			{
				for (int x = x1 < 0 ? 0 : x1; x < x2; x++)
				{
					int PIdx = (x - x1) + (y - y1) * PatchTex->GetWidth();
					if (PatchPixels[PIdx])
						Pixels[x + y * Width] = PatchPixels[PIdx];
				}
			}
		}
	}
	else
	{
		Pixels = new vuint8[Width * Height * 4];
		memset(Pixels, 0, Width * Height * 4);

		// Composite the columns together.
		VTexPatch* patch = Patches;
		for (int i = 0; i < PatchCount; i++, patch++)
		{
			VTexture* PatchTex = patch->Tex;
			vuint8* PatchPixels = PatchTex->GetPixels();
			int x1 = patch->XOrigin;
			int x2 = x1 + PatchTex->GetWidth();
			if (x2 > Width)
				x2 = Width;
			int y1 = patch->YOrigin;
			int y2 = y1 + PatchTex->GetHeight();
			if (y2 > Height)
				y2 = Height;
			for (int y = y1 < 0 ? 0 : y1; y < y2; y++)
			{
				for (int x = x1 < 0 ? 0 : x1; x < x2; x++)
				{
					//	Get pixel.
					int PIdx = (x - x1) + (y - y1) * PatchTex->GetWidth();
					rgba_t col;
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

					//	Add to texture.
					if (col.a == 255)
					{
						((rgba_t*)Pixels)[x + y * Width] = col;
					}
					else if (col.a)
					{
						rgba_t& Dst = ((rgba_t*)Pixels)[x + y * Width];
						float a = col.a / 255.0;
						float ia = (Dst.a / 255.0) * (255.0 - col.a) / 255.0;
						Dst.r = vuint8(Dst.r * ia + col.r * a);
						Dst.g = vuint8(Dst.g * ia + col.g * a);
						Dst.b = vuint8(Dst.b * ia + col.b * a);
						Dst.a = Dst.a + col.a > 255 ? 255 : Dst.a + col.a;
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

//END

//BEGIN VFlatTexture

//**************************************************************************
//	VFlatTexture
//**************************************************************************

//==========================================================================
//
//	VFlatTexture::VFlatTexture
//
//==========================================================================

VFlatTexture::VFlatTexture(int InLumpNum)
: LumpNum(InLumpNum)
, Pixels(0)
{
	guard(VFlatTexture::VFlatTexture);
	Type = TEXTYPE_Flat;
	Format = TEXFMT_8;
	Name = W_LumpName(LumpNum);
	Width = 64;
	//	Check for larger flats.
	while (W_LumpLength(LumpNum) >= Width * Width * 4)
	{
		Width *= 2;
	}
	Height = Width;
	//	Scale to 64x64.
	SScale = Width / 64;
	TScale = Width / 64;
	unguard;
}

//==========================================================================
//
//	VFlatTexture::~VFlatTexture
//
//==========================================================================

VFlatTexture::~VFlatTexture()
{
	guard(VFlatTexture::~VFlatTexture);
	if (Pixels)
	{
		delete[] Pixels;
	}
	unguard;
}

//==========================================================================
//
//	VFlatTexture::GetPixels
//
//==========================================================================

vuint8* VFlatTexture::GetPixels()
{
	guard(VFlatTexture::GetPixels);
	//	If already got pixels, then just return them.
	if (Pixels)
	{
		return Pixels;
	}

	//	Allocate memory buffer.
	Pixels = new vuint8[Width * Height];

	//	A flat must be at least 64x64, if it's smaller, then ignore it.
	if (W_LumpLength(LumpNum) < 64 * 64)
	{
		memset(Pixels, 0, 64 * 64);
		return Pixels;
	}

	//	Read data.
	VStream* Strm = W_CreateLumpReaderNum(LumpNum);
	for (int i = 0; i < Width * Height; i++)
	{
		*Strm << Pixels[i];
		if (!Pixels[i])
			Pixels[i] = r_black_colour;
	}
	delete Strm;

	return Pixels;
	unguard;
}

//==========================================================================
//
//	VFlatTexture::Unload
//
//==========================================================================

void VFlatTexture::Unload()
{
	guard(VFlatTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	unguard;
}

//END

//BEGIN VRawPicTexture

//**************************************************************************
//	VRawPicTexture
//**************************************************************************

//==========================================================================
//
//	VRawPicTexture::VRawPicTexture
//
//==========================================================================

VRawPicTexture::VRawPicTexture(int InLumpNum, int InPalLumpNum)
: LumpNum(InLumpNum)
, PalLumpNum(InPalLumpNum)
, Pixels(0)
, Palette(0)
{
	Type = TEXTYPE_Pic;
	Name = W_LumpName(InLumpNum);
	Width = 320;
	Height = W_LumpLength(InLumpNum) / 320;
	Format = PalLumpNum >= 0 ? TEXFMT_8Pal : TEXFMT_8;
}

//==========================================================================
//
//	VRawPicTexture::~VRawPicTexture
//
//==========================================================================

VRawPicTexture::~VRawPicTexture()
{
	guard(VRawPicTexture::~VRawPicTexture);
	if (Pixels)
	{
		delete[] Pixels;
	}
	if (Palette)
	{
		delete[] Palette;
	}
	unguard;
}

//==========================================================================
//
//	VRawPicTexture::GetPixels
//
//==========================================================================

vuint8* VRawPicTexture::GetPixels()
{
	guard(VRawPicTexture::GetPixels);
	//	If already got pixels, then just return them.
	if (Pixels)
	{
		return Pixels;
	}

	int len = W_LumpLength(LumpNum);
	Height = len / 320;

	Pixels = new vuint8[len];

	//	Set up palette.
	int black;
	if (PalLumpNum < 0)
	{
		black = r_black_colour;
	}
	else
	{
		//	Load palette and find black colour for remaping.
		Palette = new rgba_t[256];
		VStream* PStrm = W_CreateLumpReaderNum(PalLumpNum);
		int best_dist = 0x10000;
		black = 0;
		for (int i = 0; i < 256; i++)
		{
			*PStrm << Palette[i].r
					<< Palette[i].g
					<< Palette[i].b;
			if (i == 0)
			{
				Palette[i].a = 0;
			}
			else
			{
				Palette[i].a = 255;
				int dist = Palette[i].r * Palette[i].r + Palette[i].g *
					Palette[i].g + Palette[i].b * Palette[i].b;
				if (dist < best_dist)
				{
					black = i;
					best_dist = dist;
				}
			}
		}
		delete PStrm;
	}

	//	Read data.
	VStream* Strm = W_CreateLumpReaderNum(LumpNum);
	vuint8* dst = Pixels;
	for (int i = 0; i < len; i++, dst++)
	{
		*Strm << *dst;
		if (!*dst)
			*dst = black;
	}
	delete Strm;

	return Pixels;
	unguard;
}

//==========================================================================
//
//	VRawPicTexture::GetPalette
//
//==========================================================================

rgba_t* VRawPicTexture::GetPalette()
{
	guardSlow(VRawPicTexture::GetPalette);
	return Palette ? Palette : r_palette;
	unguardSlow;
}

//==========================================================================
//
//	VRawPicTexture::Unload
//
//==========================================================================

void VRawPicTexture::Unload()
{
	guard(VRawPicTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	if (Palette)
	{
		delete[] Palette;
		Palette = NULL;
	}
	unguard;
}

//END

//BEGIN VImgzTexture

//**************************************************************************
//	VImgzTexture
//**************************************************************************

//==========================================================================
//
//	VImgzTexture::VImgzTexture
//
//==========================================================================

VImgzTexture::VImgzTexture(int InType, int InLumpNum)
: LumpNum(InLumpNum)
, Pixels(0)
{
	Type = InType;
	Name = W_LumpName(InLumpNum);
	Format = TEXFMT_8;
}

//==========================================================================
//
//	VImgzTexture::~VImgzTexture
//
//==========================================================================

VImgzTexture::~VImgzTexture()
{
	guard(VImgzTexture::~VImgzTexture);
	if (Pixels)
	{
		delete[] Pixels;
	}
	unguard;
}

//==========================================================================
//
//	VImgzTexture::GetDimensions
//
//==========================================================================

void VImgzTexture::GetDimensions()
{
	guard(VImgzTexture::GetDimensions);
	//	Read header.
	VStream* Strm = W_CreateLumpReaderNum(LumpNum);
	Strm->Seek(4);	//	Skip magic.
	Width = Streamer<vuint16>(*Strm);
	Height = Streamer<vuint16>(*Strm);
	SOffset = Streamer<vint16>(*Strm);
	TOffset = Streamer<vint16>(*Strm);
	delete Strm;
	unguard;
}

//==========================================================================
//
//	VImgzTexture::GetPixels
//
//==========================================================================

vuint8* VImgzTexture::GetPixels()
{
	guard(VImgzTexture::GetPixels);
	//	If already got pixels, then just return them.
	if (Pixels)
	{
		return Pixels;
	}

	VStream* Strm = W_CreateLumpReaderNum(LumpNum);

	//	Read header.
	Strm->Seek(4);	//	Skip magic.
	Width = Streamer<vuint16>(*Strm);
	Height = Streamer<vuint16>(*Strm);
	SOffset = Streamer<vint16>(*Strm);
	TOffset = Streamer<vint16>(*Strm);
	vuint8 Compression = Streamer<vuint8>(*Strm);
	Strm->Seek(24);	//	Skip reserved space.

	//	Read data.
	Pixels = new vuint8[Width * Height];
	memset(Pixels, 0, Width * Height);
	if (!Compression)
	{
		Strm->Serialise(Pixels, Width * Height);
	}
	else
	{
		//	IMGZ compression is the same RLE used by IFF ILBM files
		vuint8* pDst = Pixels;
		int runlen = 0, setlen = 0;
		vuint8 setval = 0;  // Shut up, GCC

		for (int y = Height; y != 0; --y)
		{
			for (int x = Width; x != 0; )
			{
				if (runlen != 0)
				{
					*Strm << *pDst;
					pDst++;
					x--;
					runlen--;
				}
				else if (setlen != 0)
				{
					*pDst = setval;
					pDst++;
					x--;
					setlen--;
				}
				else
				{
					vint8 code;
					*Strm << code;
					if (code >= 0)
					{
						runlen = code + 1;
					}
					else if (code != -128)
					{
						setlen = (-code) + 1;
						*Strm << setval;
					}
				}
			}
		}
	}
	delete Strm;

	return Pixels;
	unguard;
}

//==========================================================================
//
//	VImgzTexture::Unload
//
//==========================================================================

void VImgzTexture::Unload()
{
	guard(VImgzTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	unguard;
}

//END

//BEGIN VPcxTexture

//**************************************************************************
//	VPcxTexture
//**************************************************************************

//==========================================================================
//
//	VPcxTexture::VPcxTexture
//
//==========================================================================

VPcxTexture::VPcxTexture(int InType, VName InName)
: Pixels(0)
, Palette(0)
{
	Type = InType;
	Name = InName;
}

//==========================================================================
//
//	VPcxTexture::~VPcxTexture
//
//==========================================================================

VPcxTexture::~VPcxTexture()
{
	guard(VPcxTexture::~VPcxTexture);
	if (Pixels)
	{
		delete[] Pixels;
	}
	if (Palette)
	{
		delete[] Palette;
	}
	unguard;
}

//==========================================================================
//
//	VPcxTexture::GetDimensions
//
//==========================================================================

void VPcxTexture::GetDimensions()
{
	guard(VPcxTexture::GetDimensions);
	GetPixels();
	unguard;
}

//==========================================================================
//
//	VPcxTexture::GetPixels
//
//==========================================================================

vuint8* VPcxTexture::GetPixels()
{
	guard(VPcxTexture::GetPixels);
	int			c;
	int			bytes_per_line;
	vint8		ch;

	//	If we already have loaded pixels, return them.
	if (Pixels)
	{
		return Pixels;
	}

	//	Open stream.
	VStream* Strm = FL_OpenFileRead(*Name);
	if (!Strm)
	{
		Sys_Error("Couldn't find file %s", *Name);
	}

	//	Read header.
	pcx_t pcx;
	*Strm << pcx;

	//	We only support 8-bit pcx files.
	if (pcx.bits_per_pixel != 8)
	{
		// we like 8 bit color planes
		Sys_Error("No 8-bit planes\n");
	}
	if (pcx.color_planes != 1)
	{
		Sys_Error("Not 8 bpp\n");
	}

	Width = pcx.xmax - pcx.xmin + 1;
	Height = pcx.ymax - pcx.ymin + 1;
	Format = TEXFMT_8Pal;

	bytes_per_line = pcx.bytes_per_line;

	Pixels = new vuint8[Width * Height];

	for (int y = 0; y < Height; y++)
	{
		// decompress RLE encoded PCX data
		int x = 0;

		while (x < bytes_per_line)
		{
			*Strm << ch;
			if ((ch & 0xC0) == 0xC0)
			{
				c = (ch & 0x3F);
				*Strm << ch;
			}
			else
			{
				c = 1;
			}

			while (c--)
			{
				if (x < Width)
					Pixels[y * Width + x] = ch;
				x++;
			}
		}
	}

	//	If not followed by palette ID, assume palette is at the end of file.
	*Strm << ch;
	if (ch != 12)
	{
		Strm->Seek(Strm->TotalSize() - 768);
	}

	//	Read palette.
	Palette = new rgba_t[256];
	for (c = 0; c < 256; c++)
	{
		*Strm << Palette[c].r
			<< Palette[c].g
			<< Palette[c].b;
		Palette[c].a = 255;
	}
	FixupPalette(Pixels, Palette);

	delete Strm;
	return Pixels;
	unguard;
}

//==========================================================================
//
//	VPcxTexture::GetPalette
//
//==========================================================================

rgba_t* VPcxTexture::GetPalette()
{
	guardSlow(VPcxTexture::GetPalette);
	return Palette;
	unguardSlow;
}

//==========================================================================
//
//	VPcxTexture::Unload
//
//==========================================================================

void VPcxTexture::Unload()
{
	guard(VPcxTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	if (Palette)
	{
		delete[] Palette;
		Palette = NULL;
	}
	unguard;
}

//END

//BEGIN VTgaTexture

//**************************************************************************
//	VTgaTexture
//**************************************************************************

//==========================================================================
//
//	VTgaTexture::VTgaTexture
//
//==========================================================================

VTgaTexture::VTgaTexture(int InType, VName InName)
: Pixels(0)
, Palette(0)
{
	Type = InType;
	Name = InName;
}

//==========================================================================
//
//	VTgaTexture::~VTgaTexture
//
//==========================================================================

VTgaTexture::~VTgaTexture()
{
	guard(VTgaTexture::~VTgaTexture);
	if (Pixels)
	{
		delete[] Pixels;
	}
	if (Palette)
	{
		delete[] Palette;
	}
	unguard;
}

//==========================================================================
//
//	VTgaTexture::GetDimensions
//
//==========================================================================

void VTgaTexture::GetDimensions()
{
	guard(VTgaTexture::GetDimensions);
	GetPixels();
	unguard;
}

//==========================================================================
//
//	VTgaTexture::GetPixels
//
//==========================================================================

vuint8* VTgaTexture::GetPixels()
{
	guard(VTgaTexture::GetPixels);
	//	If we already have loaded pixels, return them.
	if (Pixels)
	{
		return Pixels;
	}

	//	Load texture.
	int			count;
	int			c;

	VStream* Strm = FL_OpenFileRead(*Name);
	if (!Strm)
	{
		Sys_Error("Couldn't find file %s", *Name);
	}

	tgaHeader_t hdr;
	*Strm << hdr;

	Width = hdr.width;
	Height = hdr.height;

	Strm->Seek(Strm->Tell() + hdr.id_length);

	if (hdr.pal_type == 1)
	{
		Palette = new rgba_t[256];
		for (int i = 0; i < hdr.pal_colours; i++)
		{
			vuint16 col;
			switch (hdr.pal_entry_size)
			{
			case 16:
				*Strm << col;
				Palette[i].r = (col & 0x1F) << 3;
				Palette[i].g = ((col >> 5) & 0x1F) << 3;
				Palette[i].b = ((col >> 10) & 0x1F) << 3;
				Palette[i].a = 255;
				break;
			case 24:
				*Strm << Palette[i].b
					<< Palette[i].g
					<< Palette[i].r;
				Palette[i].a = 255;
				break;
			case 32:
				*Strm << Palette[i].b
					<< Palette[i].g
					<< Palette[i].r
					<< Palette[i].a;
				break;
			}
		}
	}

	/* Image type:
	*    0 = no image data
	*    1 = uncompressed color mapped
	*    2 = uncompressed true color
	*    3 = grayscale
	*    9 = RLE color mapped
	*   10 = RLE true color
	*   11 = RLE grayscale
	*/

	if (hdr.img_type == 1 || hdr.img_type == 3 ||
		hdr.img_type == 9 || hdr.img_type == 11)
	{
		Format = TEXFMT_8Pal;
		Pixels = new vuint8[Width * Height];
	}
	else
	{
		Format = TEXFMT_RGBA;
		Pixels = new vuint8[Width * Height * 4];
	}

	if (hdr.img_type == 1 && hdr.bpp == 8 && hdr.pal_type == 1)
	{
		// 8-bit, uncompressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			vuint8* dst = Pixels + yc * Width;

			Strm->Serialise(dst, Width);
		}
	}
	else if (hdr.img_type == 2 && hdr.pal_type == 0 && hdr.bpp == 16)
	{
		// 16-bit uncompressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			rgba_t *dst = (rgba_t*)(Pixels + yc * Width * 4);

			for (int x = 0; x < Width; x++, dst++)
			{
				vuint16 col;
				*Strm << col;
				dst->r = ((col >> 10) & 0x1F) << 3;
				dst->g = ((col >> 5) & 0x1F) << 3;
				dst->b = (col & 0x1F) << 3;
				dst->a = 255;
			}
		}
	}
	else if (hdr.img_type == 2 && hdr.pal_type == 0 && hdr.bpp == 24)
	{
		// 24-bit uncompressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			rgba_t *dst = (rgba_t*)(Pixels + yc * Width * 4);

			for (int x = 0; x < Width; x++, dst++)
			{
				*Strm << dst->b
					<< dst->g
					<< dst->r;
				dst->a = 255;
			}
		}
	}
	else if (hdr.img_type == 2 && hdr.pal_type == 0 && hdr.bpp == 32)
	{
		// 32-bit uncompressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			rgba_t *dst = (rgba_t*)(Pixels + yc * Width * 4);

			for (int x = 0; x < Width; x++, dst++)
			{
				*Strm << dst->b
					<< dst->g
					<< dst->r
					<< dst->a;
			}
		}
	}
	else if (hdr.img_type == 3 && hdr.bpp == 8 && hdr.pal_type == 1)
	{
		// Grayscale uncompressed
		for (int i = 0; i < 256; i++)
		{
			Palette[i].r = i;
			Palette[i].g = i;
			Palette[i].b = i;
			Palette[i].a = 255;
		}
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			byte *dst = Pixels + yc * Width;

			Strm->Serialise(dst, Width);
		}
	}
	else if (hdr.img_type == 9 && hdr.bpp == 8 && hdr.pal_type == 1)
	{
		// 8-bit RLE compressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			vuint8* dst = Pixels + yc * Width;
			c = 0;

			do
			{
				count = Streamer<vuint8>(*Strm);
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					vuint8 col;
					*Strm << col;
					while (count--)
						*(dst++) = col;
				}
				else
				{
					count++;
					c += count;
					Strm->Serialise(dst, count);
					dst += count;
				}
			}
			while (c < Width);
		}
	}
	else if (hdr.img_type == 10 && hdr.pal_type == 0 && hdr.bpp == 16)
	{
		// 16-bit RLE compressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			rgba_t *dst = (rgba_t*)(Pixels + yc * Width * 4);
			c = 0;

			do
			{
				count = Streamer<vuint8>(*Strm);
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					vuint16 col;
					*Strm << col;
					while (count--)
					{
						dst->r = ((col >> 10) & 0x1F) << 3;
						dst->g = ((col >> 5) & 0x1F) << 3;
						dst->b = (col & 0x1F) << 3;
						dst->a = 255;
						dst++;
					}
				}
				else
				{
					count++;
					c += count;
					while (count--)
					{
						vuint16 col;
						*Strm << col;
						dst->r = ((col >> 10) & 0x1F) << 3;
						dst->g = ((col >> 5) & 0x1F) << 3;
						dst->b = (col & 0x1F) << 3;
						dst->a = 255;
						dst++;
					}
				}
			}
			while (c < Width);
		}
	}
	else if (hdr.img_type == 10 && hdr.pal_type == 0 && hdr.bpp == 24)
	{
		// 24-bit REL compressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			rgba_t *dst = (rgba_t*)(Pixels + yc * Width * 4);
			c = 0;

			do
			{
				count = Streamer<vuint8>(*Strm);
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					rgba_t col;
					*Strm << col.b << col.g << col.r;
					col.a = 255;
					while (count--)
					{
						*dst = col;
						dst++;
					}
				}
				else
				{
					count++;
					c += count;
					while (count--)
					{
						*Strm << dst->b
							<< dst->g
							<< dst->r;
						dst->a = 255;
						dst++;
					}
				}
			}
			while (c < Width);
		}
	}
	else if (hdr.img_type == 10 && hdr.pal_type == 0 && hdr.bpp == 32)
	{
		// 32-bit RLE compressed
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			rgba_t* dst = (rgba_t*)(Pixels + yc * Width * 4);
			c = 0;

			do
			{
				count = Streamer<vuint8>(*Strm);
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					rgba_t col;
					*Strm << col.b << col.g << col.r << col.a;
					while (count--)
					{
						*dst = col;
						dst++;
					}
				}
				else
				{
					count++;
					c += count;
					while (count--)
					{
						*Strm << dst->b
							<< dst->g
							<< dst->r
							<< dst->a;
						dst++;
					}
				}
			}
			while (c < Width);
		}
	}
	else if (hdr.img_type == 11 && hdr.bpp == 8 && hdr.pal_type == 1)
	{
		// Grayscale RLE compressed
		for (int i = 0; i < 256; i++)
		{
			Palette[i].r = i;
			Palette[i].g = i;
			Palette[i].b = i;
			Palette[i].a = 255;
		}
		for (int y = Height; y; y--)
		{
			int yc = hdr.descriptor_bits & 0x20 ? Height - y : y - 1;
			byte *dst = Pixels + yc * Width;
			c = 0;

			do
			{
				count = Streamer<vuint8>(*Strm);
				if (count & 0x80)
				{
					count = (count & 0x7F) + 1;
					c += count;
					vuint8 col;
					*Strm << col;
					while (count--)
						*(dst++) = col;
				}
				else
				{
					count++;
					c += count;
					Strm->Serialise(dst, count);
					dst += count;
				}
			}
			while (c < Width);
		}
	}
	else
	{
		Sys_Error("Nonsupported tga format");
	}

	delete Strm;

	//	For 8-bit textures remap colour 0.
	if (Format == TEXFMT_8Pal)
	{
		FixupPalette(Pixels, Palette);
	}
	return Pixels;
	unguard;
}

//==========================================================================
//
//	VTgaTexture::GetPalette
//
//==========================================================================

rgba_t* VTgaTexture::GetPalette()
{
	guardSlow(VTgaTexture::GetPalette);
	return Palette;
	unguardSlow;
}

//==========================================================================
//
//	VTgaTexture::Unload
//
//==========================================================================

void VTgaTexture::Unload()
{
	guard(VTgaTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	if (Palette)
	{
		delete[] Palette;
		Palette = NULL;
	}
	unguard;
}

//END

//BEGIN VPngTexture

//**************************************************************************
//	VPngTexture
//**************************************************************************

//==========================================================================
//
//	VPngTexture::VPngTexture
//
//==========================================================================

VPngTexture::VPngTexture(int InType, int InLumpNum)
: LumpNum(InLumpNum)
, Pixels(0)
, Palette(0)
{
	Type = InType;
	Name = W_LumpName(InLumpNum);
	Format = TEXFMT_8;
}

//==========================================================================
//
//	VPngTexture::VPngTexture
//
//==========================================================================

VPngTexture::VPngTexture(int InType, VName InName)
: LumpNum(-1)
, Pixels(0)
, Palette(0)
{
	Type = InType;
	Name = InName;
}

//==========================================================================
//
//	VPngTexture::~VPngTexture
//
//==========================================================================

VPngTexture::~VPngTexture()
{
	guard(VPngTexture::~VPngTexture);
	if (Pixels)
	{
		delete[] Pixels;
	}
	if (Palette)
	{
		delete[] Palette;
	}
	unguard;
}

//==========================================================================
//
//	VPngTexture::GetDimensions
//
//==========================================================================

void VPngTexture::GetDimensions()
{
	guard(VPngTexture::GetDimensions);
	GetPixels();
	unguard;
}

//==========================================================================
//
//	ReadFunc
//
//==========================================================================

void VPngTexture::ReadFunc(png_structp png, png_bytep data, png_size_t len)
{
	guard(ReadFunc);
#ifdef CLIENT
	VStream* Strm = (VStream*)png_get_io_ptr(png);
	Strm->Serialise(data, len);
#endif
	unguard;
}

//==========================================================================
//
//	VPngTexture::GetPixels
//
//==========================================================================

vuint8* VPngTexture::GetPixels()
{
	guard(VPngTexture::GetPixels);
#ifdef CLIENT
	//	If we already have loaded pixels, return them.
	if (Pixels)
	{
		return Pixels;
	}

	//	Create reading structure.
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL);
	if (!png_ptr)
	{
		Sys_Error("Couldn't create png_ptr");
	}

	//	Create info structure.
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		Sys_Error("Couldn't create info_ptr");
	}

	//	Create end info structure.
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info)
	{
		Sys_Error("Couldn't create end_info");
	}

	//	Set up error handling.
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		Sys_Error("Error reading PNG file");
	}

	//	Open stream.
	VStream* Strm;
	if (LumpNum != -1)
	{
		Strm = W_CreateLumpReaderNum(LumpNum);
	}
	else
	{
		Strm = FL_OpenFileRead(*Name);
	}

	//	Verify signature.
	png_byte Signature[8];
	Strm->Seek(0);
	Strm->Serialise(Signature, 8);
	if (png_sig_cmp(Signature, 0, 8))
	{
		Sys_Error("%s is not a valid PNG file", *Name);
	}

	//	Set my read function.
	Strm->Seek(0);
	png_set_read_fn(png_ptr, Strm, ReadFunc);

	//	Read image info.
	png_read_info(png_ptr, info_ptr);
	Width = png_get_image_width(png_ptr, info_ptr);
	Height = png_get_image_height(png_ptr, info_ptr);
	int BitDepth = png_get_bit_depth(png_ptr, info_ptr);
	int ColourType = png_get_color_type(png_ptr, info_ptr);

	//	Set up transformations.
	if (ColourType == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png_ptr);
	}
	if (ColourType == PNG_COLOR_TYPE_GRAY && BitDepth < 8)
	{
		png_set_gray_1_2_4_to_8(png_ptr);
	}
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
	}
	if (BitDepth == 16)
	{
		png_set_strip_16(png_ptr);
	}
	if (ColourType == PNG_COLOR_TYPE_PALETTE ||
		ColourType == PNG_COLOR_TYPE_RGB ||
		ColourType == PNG_COLOR_TYPE_GRAY)
	{
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	}
	if (ColourType == PNG_COLOR_TYPE_GRAY ||
		ColourType == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		png_set_gray_to_rgb(png_ptr);
	}

	//	Set up unpacking buffer and row pointers.
	Format = TEXFMT_RGBA;
	Pixels = new vuint8[Width * Height * 4];
	png_bytep* RowPtrs = new png_bytep[Height];
	for (int i = 0; i < Height; i++)
	{
		RowPtrs[i] = Pixels + i * Width * 4;
	}
	png_read_image(png_ptr, RowPtrs);

	//	Finish reading.
	png_read_end(png_ptr, end_info);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	delete[] RowPtrs;

	//	Free memory.
	delete Strm;
	return Pixels;
#else
	return NULL;
#endif
	unguard;
}

//==========================================================================
//
//	VPngTexture::GetPalette
//
//==========================================================================

rgba_t* VPngTexture::GetPalette()
{
	guardSlow(VPngTexture::GetPalette);
	return Palette;
	unguardSlow;
}

//==========================================================================
//
//	VPngTexture::Unload
//
//==========================================================================

void VPngTexture::Unload()
{
	guard(VPngTexture::Unload);
	if (Pixels)
	{
		delete[] Pixels;
		Pixels = NULL;
	}
	if (Palette)
	{
		delete[] Palette;
		Palette = NULL;
	}
	unguard;
}

//END

#ifdef CLIENT
//==========================================================================
//
//  WritePCX
//
//==========================================================================

void WritePCX(char* filename, void* data, int width, int height, int bpp,
	bool bot2top)
{
	guard(WritePCX);
	int i;
	int j;

	VStream* Strm = FL_OpenFileWrite(filename);
	if (!Strm)
	{
		GCon->Log("Couldn't write pcx");
		return;
	}
	
	pcx_t pcx;
	pcx.manufacturer = 0x0a;	// PCX id
	pcx.version = 5;			// 256 color
	pcx.encoding = 1;			// uncompressed
	pcx.bits_per_pixel = 8;		// 256 color
	pcx.xmin = 0;
	pcx.ymin = 0;
	pcx.xmax = width - 1;
	pcx.ymax = height - 1;
	pcx.hres = width;
	pcx.vres = height;
	memset(pcx.palette, 0, sizeof(pcx.palette));
	pcx.color_planes = bpp == 8 ? 1 : 3;
	pcx.bytes_per_line = width;
	pcx.palette_type = 1;	// not a grey scale
	memset(pcx.filler, 0, sizeof(pcx.filler));
	*Strm << pcx;

	// pack the image
	if (bpp == 8)
	{
		for (j = 0; j < height; j++)
		{
			byte *src = (byte*)data + j * width;
			for (i = 0; i < width; i++)
			{
				if ((src[i] & 0xc0) == 0xc0)
				{
					byte tmp = 0xc1;
					*Strm << tmp;
				}
				*Strm << src[i];
			}
		}

		// write the palette
		byte PalId = 0x0c;	// palette ID byte
		*Strm << PalId;
		for (i = 0; i < 256; i++)
		{
			*Strm << r_palette[i].r
				<< r_palette[i].g
				<< r_palette[i].b;
		}
	}
	else if	(bpp == 24)
	{
		for (j = 0; j < height; j++)
		{
			rgb_t *src = (rgb_t*)data + (bot2top ? height - j - 1 : j) * width;
			for (i = 0; i < width; i++)
			{
				if ((src[i].r & 0xc0) == 0xc0)
				{
					byte tmp = 0xc1;
					*Strm << tmp;
				}
				*Strm << src[i].r;
			}
			for (i = 0; i < width; i++)
			{
				if ((src[i].g & 0xc0) == 0xc0)
				{
					byte tmp = 0xc1;
					*Strm << tmp;
				}
				*Strm << src[i].g;
			}
			for (i = 0; i < width; i++)
			{
				if ((src[i].b & 0xc0) == 0xc0)
				{
					byte tmp = 0xc1;
					*Strm << tmp;
				}
				*Strm << src[i].b;
			}
		}
	}

	delete Strm;
	unguard;
}

//==========================================================================
//
//	WriteTGA
//
//==========================================================================

void WriteTGA(char* filename, void* data, int width, int height, int bpp,
	bool bot2top)
{
	guard(WriteTGA);
	VStream* Strm = FL_OpenFileWrite(filename);
	if (!Strm)
	{
		GCon->Log("Couldn't write tga");
		return;
	}

	tgaHeader_t hdr;
	hdr.id_length = 0;
	hdr.pal_type = (bpp == 8) ? 1 : 0;
	hdr.img_type = (bpp == 8) ? 1 : 2;
	hdr.first_colour = 0;
	hdr.pal_colours = (bpp == 8) ? 256 : 0;
	hdr.pal_entry_size = (bpp == 8) ? 24 : 0;
	hdr.left = 0;
	hdr.top = 0;
	hdr.width = width;
	hdr.height = height;
	hdr.bpp = bpp;
	hdr.descriptor_bits = bot2top ? 0 : 0x20;
	*Strm << hdr;

	if (bpp == 8)
	{
		for (int i = 0; i < 256; i++)
		{
			*Strm << r_palette[i].b
				<< r_palette[i].g
				<< r_palette[i].r;
		}
	}

	if (bpp == 8)
	{
		Strm->Serialise(data, width * height);
	}
	else if (bpp == 24)
	{
		rgb_t *src = (rgb_t *)data;
		for (int i = 0; i < width * height; i++, src++)
		{
			*Strm << src->b 
				<< src->g 
				<< src->r;
		}
	}

	delete Strm;
	unguard;
}
#endif

//==========================================================================
//
//	P_InitAnimated
//
//	Load the table of animation definitions, checking for existence of
// the start and end of each frame. If the start doesn't exist the sequence
// is skipped, if the last doesn't exist, BOOM exits.
//
//	Wall/Flat animation sequences, defined by name of first and last frame,
// The full animation sequence is given using all lumps between the start
// and end entry, in the order found in the WAD file.
//
//	This routine modified to read its data from a predefined lump or
// PWAD lump called ANIMATED rather than a static table in this module to
// allow wad designers to insert or modify animation sequences.
//
//	Lump format is an array of byte packed animdef_t structures, terminated
// by a structure with istexture == -1. The lump can be generated from a
// text source file using SWANTBLS.EXE, distributed with the BOOM utils.
// The standard list of switches and animations is contained in the example
// source text file DEFSWANI.DAT also in the BOOM util distribution.
//
//==========================================================================

void P_InitAnimated()
{
	animDef_t 	ad;
	frameDef_t	fd;

	if (W_CheckNumForName(NAME_animated) < 0)
	{
		return;
	}

	VStream* Strm = W_CreateLumpReaderName(NAME_animated);
	while (Strm->TotalSize() - Strm->Tell() >= 23)
	{
		int pic1, pic2;
		vint8 Type;
		char TmpName1[9];
		char TmpName2[9];
		vint32 BaseTime;

		*Strm << Type;
		Strm->Serialise(TmpName1, 9);
		Strm->Serialise(TmpName2, 9);
		*Strm << BaseTime;

		if (Type == -1)
		{
			//	Terminator marker.
			break;
		}

		if (Type & 1)
		{
			pic1 = GTextureManager.CheckNumForName(VName(TmpName2,
				VName::AddLower8), TEXTYPE_Wall, true, false);
			pic2 = GTextureManager.CheckNumForName(VName(TmpName1,
				VName::AddLower8), TEXTYPE_Wall, true, false);
		}
		else
		{
			pic1 = GTextureManager.CheckNumForName(VName(TmpName2,
				VName::AddLower8), TEXTYPE_Flat, true, false);
			pic2 = GTextureManager.CheckNumForName(VName(TmpName1,
				VName::AddLower8), TEXTYPE_Flat, true, false);
		}

		// different episode ?
		if (pic1 == -1 || pic2 == -1)
			continue;		

		memset(&ad, 0, sizeof(ad));
		memset(&fd, 0, sizeof(fd));

		ad.startFrameDef = FrameDefs.Num();
		ad.IsRange = true;

		// [RH] Allow for either forward or backward animations.
		if (pic1 < pic2)
		{
			ad.index = pic1;
			fd.index = pic2;
		}
		else
		{
			ad.index = pic2;
			fd.index = pic1;
			ad.Backwards = true;
		}

		if (fd.index - ad.index < 1)
			Sys_Error("P_InitPicAnims: bad cycle from %s to %s", TmpName2,
				TmpName1);
		
		fd.baseTime = BaseTime;
		fd.randomRange = 0;
		FrameDefs.Append(fd);

		ad.endFrameDef = FrameDefs.Num() - 1;
		ad.CurrentRangeFrame = FrameDefs[ad.startFrameDef].index - ad.index;
		ad.currentFrameDef = ad.endFrameDef;
		ad.time = 0.01; // Force 1st game tic to animate
		AnimDefs.Append(ad);
	}
	delete Strm;
}

//==========================================================================
//
//	ParseFTAnim
//
//	Parse flat or texture animation.
//
//==========================================================================

static void ParseFTAnim(VScriptParser* sc, int IsFlat)
{
	animDef_t 	ad;
	frameDef_t	fd;

	memset(&ad, 0, sizeof(ad));

	//	Optional flag.
	bool optional = false;
	if (sc->Check("optional"))
	{
		optional = true;
	}

	//	Name
	bool ignore = false;
	sc->ExpectName8();
	ad.index = GTextureManager.CheckNumForName(sc->Name8,
		IsFlat ? TEXTYPE_Flat : TEXTYPE_Wall, true, true);
	if (ad.index == -1)
	{
		ignore = true;
		if (!optional)
		{
			GCon->Logf("ANIMDEFS: Can't find %s", *sc->String);
		}
	}
	bool missing = ignore && optional;

	bool HadPic = false;
	ad.startFrameDef = FrameDefs.Num();
	while (1)
	{
		if (sc->Check("allowdecals"))
		{
			//	Since we don't have decals yet, ignore it.
			continue;
		}

		if (sc->Check("pic"))
		{
			if (ad.IsRange)
			{
				sc->Error("You cannot use pic together with range.");
			}
			HadPic = true;
		}
		else if (sc->Check("range"))
		{
			if (ad.IsRange)
			{
				sc->Error("You can only use range once in a single animation.");
			}
			if (HadPic)
			{
				sc->Error("You cannot use range together with pic.");
			}
			ad.IsRange = true;
		}
		else
		{
			break;
		}

		memset(&fd, 0, sizeof(fd));
		if (sc->CheckNumber())
		{
			fd.index = ad.index + sc->Number - 1;
		}
		else
		{
			sc->ExpectName8();
			fd.index = GTextureManager.CheckNumForName(sc->Name8,
				IsFlat ? TEXTYPE_Flat : TEXTYPE_Wall, true, true);
			if (fd.index == -1 && !missing)
			{
				sc->Error(va("Unknown texture %s", *sc->String));
			}
		}
		if (sc->Check("tics"))
		{
			sc->ExpectNumber();
			fd.baseTime = sc->Number;
			fd.randomRange = 0;
		}
		else if (sc->Check("rand"))
		{
			sc->ExpectNumber();
			fd.baseTime = sc->Number;
			sc->ExpectNumber();
			fd.randomRange = sc->Number - fd.baseTime + 1;
		}
		else
		{
			sc->Error("bad command");
		}
		if (ad.IsRange)
		{
			if (fd.index < ad.index)
			{
				int tmp = ad.index;
				ad.index = fd.index;
				fd.index = tmp;
				ad.Backwards = true;
			}
		}
		if (!ignore)
		{
			FrameDefs.Append(fd);
		}
	}

	if (!ignore && !ad.IsRange && FrameDefs.Num() - ad.startFrameDef < 2)
	{
		Sys_Error("P_InitFTAnims: AnimDef has framecount < 2.");
	}

	if (!ignore)
	{
		ad.endFrameDef = FrameDefs.Num() - 1;
		ad.CurrentRangeFrame = FrameDefs[ad.startFrameDef].index - ad.index;
		ad.currentFrameDef = ad.endFrameDef;
		ad.time = 0.01; // Force 1st game tic to animate
		AnimDefs.Append(ad);
	}
}

//==========================================================================
//
//	ParseSwitchDef
//
//==========================================================================

static void ParseSwitchDef(VScriptParser* sc)
{
	//	Skip game specifier.
	if (sc->Check("doom"))
	{
		sc->ExpectNumber();
	}
	else if (sc->Check("heretic"))
	{
	}
	else if (sc->Check("hexen"))
	{
	}
	else if (sc->Check("strife"))
	{
	}
	else if (sc->Check("any"))
	{
	}

	//	Switch texture
	sc->ExpectName8();
	int t1 = GTextureManager.CheckNumForName(sc->Name8, TEXTYPE_Wall, true,
		false);
	int t2 = -1;
	VName SndName = NAME_None;

	//	Currently only basic switch definition is supported.
	while (1)
	{
		if (sc->Check("quest"))
		{
		}
		else if (sc->Check("on"))
		{
			while (1)
			{
				if (sc->Check("sound"))
				{
					sc->ExpectString();
					SndName = *sc->String;
				}
				else if (sc->Check("pic"))
				{
					sc->ExpectName8();
					t2 = GTextureManager.CheckNumForName(sc->Name8,
						TEXTYPE_Wall, true, false);
					sc->Expect("tics");
					sc->Expect("0");
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			break;
		}
	}

	if (t1 < 0 || t2 < 0)
	{
		return;
	}
	TSwitch& sw = Switches.Alloc();
	if (SndName == NAME_None)
		sw.Sound = 0;
	else
		sw.Sound = GSoundManager->GetSoundID(SndName);
	sw.Tex1 = t1;
	sw.Tex2 = t2;
}

//==========================================================================
//
//	ParseFTAnims
//
//	Initialize flat and texture animation lists.
//
//==========================================================================

static void ParseFTAnims(VScriptParser* sc)
{
	guard(ParseFTAnims);
	while (!sc->AtEnd())
	{
		if (sc->Check("flat"))
		{
			ParseFTAnim(sc, true);
		}
		else if (sc->Check("texture"))
		{
			ParseFTAnim(sc, false);
		}
		else if (sc->Check("switch"))
		{
			ParseSwitchDef(sc);
		}
		else
		{
			sc->Error("bad command");
		}
	}
	delete sc;
	unguard;
}

//==========================================================================
//
//	InitFTAnims
//
//	Initialise flat and texture animation lists.
//
//==========================================================================

static void InitFTAnims()
{
	guard(InitFTAnims);
	//	Process all animdefs lumps.
	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) == NAME_animdefs)
		{
			ParseFTAnims(new VScriptParser(*W_LumpName(Lump),
				W_CreateLumpReaderNum(Lump)));
		}
	}

	//	Optionally parse script file.
	if (fl_devmode && FL_FileExists("scripts/animdefs.txt"))
	{
		ParseFTAnims(new VScriptParser("scripts/animdefs.txt",
			FL_OpenFileRead("scripts/animdefs.txt")));
	}

	//	Read Boom's animated lump if present.
	P_InitAnimated();

	FrameDefs.Condense();
	AnimDefs.Condense();
	unguard;
}

//==========================================================================
//
//	P_InitSwitchList
//
//	Only called at game initialization.
//	Parse BOOM style switches lump.
//
//==========================================================================

void P_InitSwitchList()
{
	guard(P_InitSwitchList);
	int lump = W_CheckNumForName(NAME_switches);
	if (lump != -1)
	{
		VStream* Strm = W_CreateLumpReaderNum(lump);
		while (Strm->TotalSize() - Strm->Tell() >= 20)
		{
			char TmpName1[9];
			char TmpName2[9];
			vint16 Episode;

			//	Read data.
			Strm->Serialise(TmpName1, 9);
			Strm->Serialise(TmpName2, 9);
			*Strm << Episode;
			if (!Episode)
			{
				//	Terminator marker.
				break;
			}
			TmpName1[8] = 0;
			TmpName2[8] = 0;

			// Check for switches that aren't really switches
			if (!VStr::ICmp(TmpName1, TmpName2))
			{
				GCon->Logf(NAME_Init, "Switch %s in SWITCHES has the same 'on' state", TmpName1);
				continue;
			}
			int t1 = GTextureManager.CheckNumForName(VName(TmpName1,
				VName::AddLower8), TEXTYPE_Wall, true, false);
			int t2 = GTextureManager.CheckNumForName(VName(TmpName2,
				VName::AddLower8), TEXTYPE_Wall, true, false);
			if (t1 < 0 || t2 < 0)
			{
				continue;
			}
			TSwitch& sw = Switches.Alloc();
			sw.Sound = 0;
			sw.Tex1 = t1;
			sw.Tex2 = t2;
		}
		delete Strm;
	}
	Switches.Condense();
	unguard;
}

//==========================================================================
//
//	R_AnimateSurfaces
//
//==========================================================================

#ifdef CLIENT
void R_AnimateSurfaces()
{
	guard(R_AnimateSurfaces);
	//	Animate flats and textures
	for (int i = 0; i < AnimDefs.Num(); i++)
	{
		animDef_t* ad = &AnimDefs[i];
		ad->time -= host_frametime;
		if (ad->time <= 0.0)
		{
			if (!ad->IsRange)
			{
				if (ad->currentFrameDef == ad->endFrameDef)
				{
					ad->currentFrameDef = ad->startFrameDef;
				}
				else
				{
					ad->currentFrameDef++;
				}
			}
			frameDef_t fd = FrameDefs[ad->currentFrameDef];
			ad->time = fd.baseTime / 35.0;
			if (fd.randomRange)
			{ 
				// Random tics
				ad->time += Random() * (fd.randomRange / 35.0);
			}
			if (!ad->IsRange)
			{
				GTextureManager.Textures[ad->index]->TextureTranslation = fd.index;
			}
			else
			{
				int Range = fd.index - ad->index + 1;
				if (ad->CurrentRangeFrame >= Range - 1)
				{
					ad->CurrentRangeFrame = 0;
				}
				else
				{
					ad->CurrentRangeFrame++;
				}
				for (int i = 0; i < Range; i++)
				{
					GTextureManager.Textures[ad->index + i]->TextureTranslation = ad->index +
						(ad->Backwards ?
						(Range - 1 - (ad->CurrentRangeFrame + i) % Range) :
						((ad->CurrentRangeFrame + i) % Range));
				}
			}
		}
	}

	R_AnimateSky();
	unguard;
}
#endif

//==========================================================================
//
//	R_InitTexture
//
//==========================================================================

void R_InitTexture()
{
	guard(R_InitTexture);
	GTextureManager.Init();
	InitFTAnims(); // Init flat and texture animations
	unguard;
}

//==========================================================================
//
//	R_ShutdownTexture
//
//==========================================================================

void R_ShutdownTexture()
{
	guard(R_ShutdownTexture);
	//	Clean up animation and switch definitions.
	Switches.Clear();
	AnimDefs.Clear();
	FrameDefs.Clear();

	//	Shut down texture manager.
	GTextureManager.Shutdown();
	unguard;
}
