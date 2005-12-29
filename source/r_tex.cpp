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

#include "gamedefs.h"
#include "ftexdefs.h"
#include "fgfxdefs.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct frameDef_t
{
	int index;
	short baseTime;
	short randomRange;
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

//
//	ZDoom's IMGZ grapnics.
// [RH] Just a format I invented to avoid WinTex's palette remapping
// when I wanted to insert some alpha maps.
//
struct TIMGZHeader
{
	byte	Magic[4];
	word	Width;
	word	Height;
	short	LeftOffset;
	short	TopOffset;
	byte	Compression;
	byte	Reserved[11];
};

//	A maptexturedef_t describes a rectangular texture, which is composed of
// one or more mappatch_t structures that arrange graphic patches

struct texpatch_t
{
	// Block origin (allways UL),
	// which has allready accounted
	// for the internal origin of the patch.
	short		XOrigin;
	short		YOrigin;
	TTexture*	Tex;
};

//
//	A dummy texture.
//
class TDummyTexture : public TTexture
{
public:
	TDummyTexture();
	byte* GetPixels();
	void MakePurgable();
	void Unload();
};

//
//	A standard Doom patch.
//
class TPatchTexture : public TTexture
{
public:
	int			LumpNum;
	byte*		Pixels;

	TPatchTexture(int InType, int InLumpNum);
	void GetDimensions();
	byte* GetPixels();
	void MakePurgable();
	void Unload();
};

//
//	A texture defined in TEXTURE1/TEXTURE2 lumps.
//
class TMultiPatchTexture : public TTexture
{
public:
	// All the patches[patchcount]
	//are drawn back to front into the cached texture.
	short		PatchCount;
	texpatch_t*	Patches;
	byte*		Pixels;

	TMultiPatchTexture(FName InName, int InWidth, int InHeight,
		float InSScale, float InTScale, int InPatchCount);
	void SetFrontSkyLayer();
	byte* GetPixels();
	void MakePurgable();
	void Unload();
};

//
//	A standard Doom flat.
//
class TFlatTexture : public TTexture
{
public:
	int			LumpNum;
	byte*		Pixels;

	TFlatTexture(int InLumpNum);
	byte* GetPixels();
	void MakePurgable();
	void Unload();
};

//
//	Raven's raw screens.
//
class TRawPicTexture : public TTexture
{
public:
	int			LumpNum;
	int			PalLumpNum;
	byte*		Pixels;
	rgba_t*		Palette;

	TRawPicTexture(int InLumpNum, int InPalLumpNum);
	byte* GetPixels();
	rgba_t* GetPalette();
	void MakePurgable();
	void Unload();
};

//
//	ZDoom's IMGZ image.
//
class TImgzTexture : public TTexture
{
public:
	int			LumpNum;
	byte*		Pixels;

	TImgzTexture(int InType, int InLumpNum);
	void GetDimensions();
	byte* GetPixels();
	void MakePurgable();
	void Unload();
};

//
//	An PNG file in a lump.
//
class TPngLumpTexture : public TTexture
{
public:
	int			LumpNum;
	byte*		Pixels;
	rgba_t*		Palette;

	TPngLumpTexture(int InType, int InLumpNum);
	void GetDimensions();
	byte* GetPixels();
	rgba_t* GetPalette();
	void MakePurgable();
	void Unload();
};

//
//	Base class for textures loaded from files, not lumps.
//
class TFileTexture : public TTexture
{
public:
	byte*		Pixels;
	rgba_t*		Palette;

	TFileTexture(int InType, FName InName);
	void GetDimensions();
	byte* GetPixels();
	rgba_t* GetPalette();
	void MakePurgable();
	void Unload();
};

//
//	A PCX file.
//
class TPcxFileTexture : public TFileTexture
{
public:
	TPcxFileTexture(int InType, FName InName);
};

//
//	A TGA file.
//
class TTgaFileTexture : public TFileTexture
{
public:
	TTgaFileTexture(int InType, FName InName);
};

//
//	A PNG file.
//
class TPngFileTexture : public TFileTexture
{
public:
	TPngFileTexture(int InType, FName InName);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

//
//	Texture manager
//
TTextureManager		GTextureManager;

//
// Flats data
//
int					skyflatnum;			// sky mapping

//
//	Translation tables
//
byte*				translationtables;

//
//	Main palette
//
rgba_t				r_palette[256];
byte				r_black_colour;

//	Switches
TArray<TSwitch>		Switches;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TArray<animDef_t>	AnimDefs;
static TArray<frameDef_t>	FrameDefs;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	TTextureManager::TTextureManager
//
//==========================================================================

TTextureManager::TTextureManager()
{
}

//==========================================================================
//
//	TTextureManager::Init
//
//==========================================================================

void TTextureManager::Init()
{
	guard(TTextureManager::Init);
	//	Add a dummy texture.
	AddTexture(new(PU_STATIC) TDummyTexture);

	//	Initialise wall textures.
	if (IsStrifeTexture())
	{
		GCon->Log(NAME_Init, "Strife textures detected");
		InitTextures2();
	}
	else
	{
		InitTextures();
	}

	//	Initialise flats.
	InitFlats();

	//	Initialise overloaded textures.
	InitOverloads();

	//	Initialise sprites.
	InitSpriteLumps();

	//	Find sky flat number.
	skyflatnum = CheckNumForName(FName("F_SKY", FNAME_AddLower8),
		TEXTYPE_Flat, true, false);
	if (skyflatnum < 0)
	    skyflatnum = CheckNumForName(FName("F_SKY001", FNAME_AddLower8),
			TEXTYPE_Flat, true, false);
	if (skyflatnum < 0)
	    skyflatnum = NumForName(FName("F_SKY1", FNAME_AddLower8),
			TEXTYPE_Flat, true, false);
	unguard;
}

//==========================================================================
//
//	TTextureManager::AddTexture
//
//==========================================================================

int TTextureManager::AddTexture(TTexture* Tex)
{
	guard(TTextureManager::AddTexture);
	Textures.AddItem(Tex);
	Tex->TextureTranslation = Textures.Num() - 1;
	return Textures.Num() - 1;
	unguard;
}

//==========================================================================
//
//  TTextureManager::CheckNumForName
//
// 	Check whether texture is available. Filter out NoTexture indicator.
//
//==========================================================================

int	TTextureManager::CheckNumForName(FName Name, int Type, bool bOverload,
	bool bCheckAny)
{
	guard(TTextureManager::CheckNumForName);
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
// 	TTextureManager::NumForName
//
// 	Calls R_CheckTextureNumForName, aborts with error message.
//
//==========================================================================

int	TTextureManager::NumForName(FName Name, int Type, bool bOverload,
	bool bCheckAny)
{
	guard(TTextureManager::NumForName);
	int i = CheckNumForName(Name, Type, bOverload, bCheckAny);
	if (i == -1)
	{
		Host_Error("TTextureManager::NumForName: %s not found", *Name);
	}
	return i;
	unguard;
}

//==========================================================================
//
//	TTextureManager::TextureHeight
//
//==========================================================================

float TTextureManager::TextureHeight(int TexNum)
{
	guard(TTextureManager::TextureHeight);
	return Textures[TexNum]->GetHeight() / Textures[TexNum]->TScale;
	unguard;
}

//==========================================================================
//
//	TTextureManager::TextureAnimation
//
//==========================================================================

int TTextureManager::TextureAnimation(int InTex)
{
	guard(TTextureManager::TextureAnimation);
	return Textures[InTex]->TextureTranslation;
	unguard;
}

//==========================================================================
//
//	TTextureManager::SetFrontSkyLayer
//
//==========================================================================

void TTextureManager::SetFrontSkyLayer(int tex)
{
	guard(TTextureManager::SetFrontSkyLayer);
	Textures[tex]->SetFrontSkyLayer();
	unguard;
}

//==========================================================================
//
//	TTextureManager::GetTextureInfo
//
//==========================================================================

void TTextureManager::GetTextureInfo(int TexNum, picinfo_t* info)
{
	guard(TTextureManager::GetTextureInfo);
	if (TexNum < 0)
	{
		memset(info, 0, sizeof(*info));
	}
	else
	{
		TTexture* Tex = Textures[TexNum];
		info->width = Tex->GetWidth();
		info->height = Tex->GetHeight();
		info->xoffset = Tex->SOffset;
		info->yoffset = Tex->TOffset;
	}
	unguard;
}

//==========================================================================
//
//	TTextureManager::AddPatch
//
//==========================================================================

int TTextureManager::AddPatch(FName Name, int Type)
{
	guard(TTextureManager::AddPatch);
	//	Find the lump number.
	int LumpNum = W_CheckNumForName(*Name);
	if (LumpNum < 0)
		LumpNum = W_CheckNumForName(*Name, WADNS_Sprites);
	if (LumpNum < 0)
	{
		GCon->Logf("TTextureManager::AddPatch: Pic %s not found", *Name);
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
//	TTextureManager::CreatePatch
//
//==========================================================================

int TTextureManager::CreatePatch(int Type, int LumpNum)
{
	guard(TTextureManager::CreatePatch);
	//	Read lump header and see what type of lump it is.
	byte LumpHeader[16];
	W_ReadFromLump(LumpNum, LumpHeader, 0, 16);

	//	Check for ZDoom's IMGZ image.
	if (LumpHeader[0] == 'I' && LumpHeader[1] == 'M' &&
		LumpHeader[2] == 'G' && LumpHeader[3] == 'Z')
	{
		return AddTexture(new(PU_STATIC) TImgzTexture(Type, LumpNum));
	}

	//	Check for PNG image.
	if (LumpHeader[0] == 137 && LumpHeader[1] == 'P' &&
		LumpHeader[2] == 'N' && LumpHeader[3] == 'G')
	{
		return AddTexture(new(PU_STATIC) TPngLumpTexture(Type, LumpNum));
	}

	//	Check for automap background.
	if (!strcmp(W_LumpName(LumpNum), "AUTOPAGE"))
	{
		return AddTexture(new(PU_STATIC) TRawPicTexture(LumpNum, -1));
	}

	if (W_LumpLength(LumpNum) == 64000)
	{
		return AddTexture(new(PU_STATIC) TRawPicTexture(LumpNum, -1));
	}

	return AddTexture(new(PU_STATIC) TPatchTexture(Type, LumpNum));
	unguard;
}

//==========================================================================
//
//	TTextureManager::AddRawWithPal
//
//	Adds a raw image with custom palette lump. It's here to support
// Heretic's episode 2 finale pic.
//
//==========================================================================

int TTextureManager::AddRawWithPal(FName Name, FName PalName)
{
	guard(TTextureManager::AddRawWithPal);
	int LumpNum = W_CheckNumForName(*Name);
	if (LumpNum < 0)
	{
		GCon->Logf("TTextureManager::AddRawWithPal: %s not found", *Name);
		return -1;
	}
	//	Check if lump's size to see if it really is a raw image. If not,
	// load it as regular image.
	if (W_LumpLength(LumpNum) != 64000)
	{
		GCon->Logf("TTextureManager::AddRawWithPal: %s doesn't appear to be"
			" a raw image", *Name);
		return AddPatch(Name, TEXTYPE_Pic);
	}

	int i = CheckNumForName(Name, TEXTYPE_Pic);
	if (i >= 0)
	{
		return i;
	}

	return AddTexture(new(PU_STATIC) TRawPicTexture(LumpNum,
		W_GetNumForName(*PalName)));
	unguard;
}

//==========================================================================
//
//	TTextureManager::AddFileTexture
//
//==========================================================================

int TTextureManager::AddFileTexture(FName Name, int Type)
{
	guard(TTextureManager::AddFileTexture)
	char Ext[8];

	int i = CheckNumForName(Name, Type);
	if (i >= 0)
	{
		return i;
	}

	FL_ExtractFileExtension(*Name, Ext);
	if (!stricmp(Ext, "pcx"))
	{
		return AddTexture(new(PU_STATIC) TPcxFileTexture(Type, Name));
	}
	else if (!stricmp(Ext, "tga"))
	{
		return AddTexture(new(PU_STATIC) TTgaFileTexture(Type, Name));
	}
	else if (!stricmp(Ext, "png"))
	{
		return AddTexture(new(PU_STATIC) TPngFileTexture(Type, Name));
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
//	TTextureManager::IsStrifeTexture
//
//==========================================================================

bool TTextureManager::IsStrifeTexture()
{
	guard(TTextureManager::IsStrifeTexture);
	int *plump = (int*)W_CacheLumpName("TEXTURE1", PU_STATIC);
	int numtex = LittleLong(*plump);
	int *texdir = plump + 1;
	int i;
	for (i = 0; i < numtex - 1; i++)
	{
		if (LittleLong(texdir[i + 1]) - LittleLong(texdir[i]) == sizeof(maptexture_strife_t))
		{
			break;
		}
	}
	Z_Free(plump);
	return i != numtex - 1;
	unguard;
}

//==========================================================================
//
//	TTextureManager::InitTextures
//
// 	Initializes the texture list with the textures from the world map.
//
//==========================================================================

void TTextureManager::InitTextures()
{
	guard(TTextureManager::InitTextures);
	maptexture_t*	mtexture;
	texpatch_t*		patch;

	int				i;
	int				j;

	int*			maptex;
	int*			maptex2;
	int*			maptex1;

	char			name[9];
	char*			names;
	char*			name_p;

	int				nummappatches;
	int				offset;
	int				maxoff;
	int				maxoff2;
	int				numtextures1;
	int				numtextures2;

	int*			directory;

	// Load the patch names from pnames.lmp.
	name[8] = 0;
	names = (char*)W_CacheLumpName("PNAMES", PU_STATIC);
	nummappatches = LittleLong(*((int *)names));
	name_p = names + 4;
	TTexture** patchtexlookup = (TTexture**)Z_Malloc(nummappatches * sizeof(*patchtexlookup), PU_HIGH, 0);

	for (i = 0; i < nummappatches; i++)
	{
		strncpy(name, name_p + i * 8, 8);
		int LumpNum = W_CheckNumForName(name);
		//	Sprites also can be used as patches.
		if (LumpNum < 0)
			LumpNum = W_CheckNumForName(name, WADNS_Sprites);

		if (LumpNum < 0)
			patchtexlookup[i] = NULL;
		else
			patchtexlookup[i] = Textures[CreatePatch(TEXTYPE_WallPatch, LumpNum)];
	}
	Z_Free(names);

	// Load the map texture definitions from textures.lmp.
	// The data is contained in one or two lumps,
	//  TEXTURE1 for shareware, plus TEXTURE2 for commercial.
	maptex = maptex1 = (int*)W_CacheLumpName("TEXTURE1", PU_HIGH);
	numtextures1 = LittleLong(*maptex);
	maxoff = W_LumpLength(W_GetNumForName("TEXTURE1"));
	directory = maptex+1;

	if (W_CheckNumForName("TEXTURE2") != -1)
	{
		maptex2 = (int*)W_CacheLumpName("TEXTURE2", PU_HIGH);
		numtextures2 = LittleLong(*maptex2);
		maxoff2 = W_LumpLength(W_GetNumForName ("TEXTURE2"));
	}
	else
	{
		maptex2 = NULL;
		numtextures2 = 0;
		maxoff2 = 0;
	}
	int numtextures = numtextures1 + numtextures2;

	for (i = 0; i < numtextures; i++, directory++)
	{
		if (i == numtextures1)
		{
			// Start looking in second texture file.
			maptex = maptex2;
			maxoff = maxoff2;
			directory = maptex+1;
		}
	
		offset = LittleLong(*directory);

		if (offset > maxoff)
		{
			Sys_Error("InitTextures: bad texture directory");
		}

		mtexture = (maptexture_t*)((byte *)maptex + offset);

		TMultiPatchTexture* Tex = new(PU_STATIC) TMultiPatchTexture(
			FName(mtexture->name, FNAME_AddLower8),
			LittleShort(mtexture->width),
			LittleShort(mtexture->height),
			mtexture->sscale ? mtexture->sscale / 8.0 : 1.0,
			mtexture->tscale ? mtexture->tscale / 8.0 : 1.0,
			LittleShort(mtexture->patchcount));
		AddTexture(Tex);

		patch = Tex->Patches;

		for (j = 0; j < Tex->PatchCount; j++, patch++)
		{
			patch->XOrigin = LittleShort(mtexture->patches[j].originx);
			patch->YOrigin = LittleShort(mtexture->patches[j].originy);
			patch->Tex = patchtexlookup[LittleShort(mtexture->patches[j].patch)];
			if (!patch->Tex)
			{
				Sys_Error("InitTextures: Missing patch in texture %s",
					*Tex->Name);
			}
		}

		//	Fix sky texture heights for Heretic, but it can also be used
		// for Doom and Strife
		if (!strnicmp(*Tex->Name, "SKY", 3) && Tex->Height == 128)
		{
			if (Tex->Patches[0].Tex->GetHeight() > Tex->Height)
			{
				Tex->Height = Tex->Patches[0].Tex->GetHeight();
			}
		}
	}

	Z_Free(patchtexlookup);
	Z_Free(maptex1);
	if (maptex2)
		Z_Free(maptex2);
	unguard;
}

//==========================================================================
//
//	TTextureManager::InitTextures2
//
// 	Initializes the texture list with the textures from the world map.
//	Strife texture format version.
//
//==========================================================================

void TTextureManager::InitTextures2()
{
	guard(TTextureManager::InitTextures2);
	maptexture_strife_t	*mtexture;
	texpatch_t*		patch;

	int				i;
	int				j;

	int*			maptex;
	int*			maptex2;
	int*			maptex1;

	char			name[9];
	char*			names;
	char*			name_p;

	int				nummappatches;
	int				offset;
	int				maxoff;
	int				maxoff2;
	int				numtextures1;
	int				numtextures2;

	int*			directory;

	// Load the patch names from pnames.lmp.
	name[8] = 0;
	names = (char*)W_CacheLumpName("PNAMES", PU_STATIC);
	nummappatches = LittleLong(*((int *)names));
	name_p = names + 4;
	TTexture** patchtexlookup = (TTexture**)Z_Malloc(nummappatches * sizeof(*patchtexlookup), PU_HIGH, 0);

	for (i = 0; i < nummappatches; i++)
	{
		strncpy(name, name_p + i * 8, 8);
		int LumpNum = W_CheckNumForName(name);
		//	Sprites also can be used as patches.
		if (LumpNum < 0)
			LumpNum = W_CheckNumForName(name, WADNS_Sprites);

		if (LumpNum < 0)
			patchtexlookup[i] = NULL;
		else
			patchtexlookup[i] = Textures[CreatePatch(TEXTYPE_WallPatch, LumpNum)];
	}
	Z_Free(names);

	// Load the map texture definitions from textures.lmp.
	// The data is contained in one or two lumps,
	//  TEXTURE1 for shareware, plus TEXTURE2 for commercial.
	maptex = maptex1 = (int*)W_CacheLumpName("TEXTURE1", PU_HIGH);
	numtextures1 = LittleLong(*maptex);
	maxoff = W_LumpLength(W_GetNumForName("TEXTURE1"));
	directory = maptex+1;

	if (W_CheckNumForName("TEXTURE2") != -1)
	{
		maptex2 = (int*)W_CacheLumpName("TEXTURE2", PU_HIGH);
		numtextures2 = LittleLong(*maptex2);
		maxoff2 = W_LumpLength(W_GetNumForName ("TEXTURE2"));
	}
	else
	{
		maptex2 = NULL;
		numtextures2 = 0;
		maxoff2 = 0;
	}
	int numtextures = numtextures1 + numtextures2;

	for (i=0 ; i<numtextures ; i++, directory++)
	{
		if (i == numtextures1)
		{
			// Start looking in second texture file.
			maptex = maptex2;
			maxoff = maxoff2;
			directory = maptex+1;
		}

		offset = LittleLong(*directory);

		if (offset > maxoff)
		{
			Sys_Error("InitTextures: bad texture directory");
		}
	
		mtexture = (maptexture_strife_t*)((byte *)maptex + offset);

		TMultiPatchTexture* Tex = new(PU_STATIC) TMultiPatchTexture(
			FName(mtexture->name, FNAME_AddLower8),
			LittleShort(mtexture->width),
			LittleShort(mtexture->height), 1.0, 1.0,
			LittleShort(mtexture->patchcount));
		AddTexture(Tex);

		patch = Tex->Patches;

		for (j = 0; j < Tex->PatchCount; j++, patch++)
		{
			patch->XOrigin = LittleShort(mtexture->patches[j].originx);
			patch->YOrigin = LittleShort(mtexture->patches[j].originy);
			patch->Tex = patchtexlookup[LittleShort(mtexture->patches[j].patch)];
			if (!patch->Tex)
			{
				Sys_Error("InitTextures: Missing patch in texture %s",
					*Tex->Name);
			}
		}

		//	Fix sky texture heights for Heretic, but it can also be used
		// for Doom and Strife
		if (!strnicmp(*Tex->Name, "SKY", 3) && Tex->Height == 128)
		{
			if (Tex->Patches[0].Tex->GetHeight() > Tex->Height)
			{
				Tex->Height = Tex->Patches[0].Tex->GetHeight();
			}
		}
	}

	Z_Free(patchtexlookup);
	Z_Free(maptex1);
	if (maptex2)
		Z_Free(maptex2);
	unguard;
}

//==========================================================================
//
//	TTextureManager::InitFlats
//
//==========================================================================

void TTextureManager::InitFlats()
{
	guard(TTextureManager::InitFlats);
	for (int Lump = W_IterateNS(-1, WADNS_Flats); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Flats))
	{
		AddTexture(new(PU_STATIC) TFlatTexture(Lump));
	}
	unguard;
}

//==========================================================================
//
//	TTextureManager::InitOverloads
//
//==========================================================================

void TTextureManager::InitOverloads()
{
	guard(TTextureManager::InitOverloads);
	for (int Lump = W_IterateNS(-1, WADNS_NewTextures); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_NewTextures))
	{
		CreatePatch(TEXTYPE_Overload, Lump);
	}
	unguard;
}

//==========================================================================
//
//	TTextureManager::InitSpriteLumps
//
//==========================================================================

void TTextureManager::InitSpriteLumps()
{
	guard(TTextureManager::InitSpriteLumps);
	for (int Lump = W_IterateNS(-1, WADNS_Sprites); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Sprites))
	{
		CreatePatch(TEXTYPE_Sprite, Lump);
	}
	unguard;
}

//**************************************************************************
//	TTexture
//**************************************************************************

//==========================================================================
//
//	TTexture::TTexture
//
//==========================================================================

TTexture::TTexture()
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
, DriverHandle(0)
{
}

//==========================================================================
//
//	TTexture::~TTexture
//
//==========================================================================

TTexture::~TTexture()
{
}

//==========================================================================
//
//	TTexture::GetDimensions
//
//==========================================================================

void TTexture::GetDimensions()
{
	guardSlow(TTexture::GetDimensions);
	Width = 0;
	Height = 0;
	unguardSlow;
}

//==========================================================================
//
//	TTexture::SetFrontSkyLayer
//
//==========================================================================

void TTexture::SetFrontSkyLayer()
{
	guardSlow(TTexture::SetFrontSkyLayer);
	bNoRemap0 = true;
	unguardSlow;
}

//==========================================================================
//
//	TTexture::GetPixels8
//
//==========================================================================

byte* TTexture::GetPixels8()
{
	guard(TTexture::GetPixels8);
	static byte* RGBTable = 0;

	byte* Pixels = GetPixels();
	if (Format == TEXFMT_8Pal)
	{
		//	Remap to game palette
		if (!RGBTable)
		{
			RGBTable = (byte*)W_CacheLumpName("RGBTABLE", PU_STATIC);
		}
		int NumPixels = Width * Height;
		rgba_t* Pal = GetPalette();
		byte Remap[256];
		Remap[0] = 0;
		int i;
		for (i = 1; i < 256; i++)
		{
			Remap[i] = RGBTable[((Pal[i].r << 7) & 0x7c00) +
				((Pal[i].g << 2) & 0x3e0) + ((Pal[i].b >> 3) & 0x1f)];
		}

		byte* pPix = Pixels;
		for (i = 0; i < NumPixels; i++, pPix++)
		{
			*pPix = Remap[*pPix];
		}
		Format = TEXFMT_8;
	}
	else if (Format == TEXFMT_RGBA)
	{
		if (!RGBTable)
		{
			RGBTable = (byte*)W_CacheLumpName("RGBTABLE", PU_STATIC);
		}
		int NumPixels = Width * Height;
		rgba_t* pSrc = (rgba_t*)Pixels;
		byte* pDst = Pixels;
		for (int i = 0; i < NumPixels; i++, pSrc++, pDst++)
		{
			if (pSrc->a < 128)
				*pDst = 0;
			else
				*pDst = RGBTable[((pSrc->r << 7) & 0x7c00) +
					((pSrc->g << 2) & 0x3e0) + ((pSrc->b >> 3) & 0x1f)];
		}
		Z_Resize((void**)&Pixels, Width * Height);
		Format = TEXFMT_8;
	}
	return Pixels;
	unguard;
}

//==========================================================================
//
//	TTexture::GetPalette
//
//==========================================================================

rgba_t* TTexture::GetPalette()
{
	guardSlow(TTexture::GetPalette);
	return r_palette;
	unguardSlow;
}

//==========================================================================
//
//	TTexture::GetHighResPixels
//
//==========================================================================

rgba_t* TTexture::GetHighResPixels(int& HRWidth, int& HRHeight)
{
	guard(TTexture::GetHighResPixels);
#ifdef CLIENT
	const char* DirName;
	switch (Type)
	{
	case TEXTYPE_Wall:
		DirName = "walls";
		break;
	case TEXTYPE_Flat:
		DirName = "flats";
		break;
	case TEXTYPE_Pic:
		DirName = "pics";
		break;
	default:
		return NULL;
	}
	char HighResName[80];
	sprintf(HighResName, "textures/%s/%s.png", DirName, *Name);
	if (!FL_FindFile(HighResName, NULL))
	{
		return NULL;
	}
	Mod_LoadSkin(HighResName, 0);
	HRWidth = SkinWidth;
	HRHeight = SkinHeight;
	if (SkinBPP == 8)
	{
		rgba_t *buf = (rgba_t*)Z_Malloc(SkinWidth * SkinHeight * 4);
		for (int i = 0; i < SkinWidth * SkinHeight; i++)
		{
			buf[i] = SkinPal[SkinData[i]];
		}
		Z_Free(SkinData);
		SkinData = (byte*)buf;
	}
	return (rgba_t*)SkinData;
#else
	return NULL;
#endif
	unguard;
}

//**************************************************************************
//	TDummyTexture
//**************************************************************************

//==========================================================================
//
//	TDummyTexture::TDummyTexture
//
//==========================================================================

TDummyTexture::TDummyTexture()
{
	Type = TEXTYPE_Any;
	Format = TEXFMT_8;
}

//==========================================================================
//
//	TDummyTexture::GetPixels
//
//==========================================================================

byte* TDummyTexture::GetPixels()
{
	return NULL;
}

//==========================================================================
//
//	TDummyTexture::MakePurgable
//
//==========================================================================

void TDummyTexture::MakePurgable()
{
}

//==========================================================================
//
//	TDummyTexture::Unload
//
//==========================================================================

void TDummyTexture::Unload()
{
}

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
	Name = FName(W_LumpName(InLumpNum), FNAME_AddLower8);
	Format = TEXFMT_8;
}

//==========================================================================
//
//	TPatchTexture::GetDimensions
//
//==========================================================================

void TPatchTexture::GetDimensions()
{
	guard(TPatchTexture::GetDimensions);
	patch_t* patch = (patch_t*)W_CacheLumpNum(LumpNum, PU_CACHE);
	Width = LittleShort(patch->width);
	Height = LittleShort(patch->height);
	SOffset = LittleShort(patch->leftoffset);
	TOffset = LittleShort(patch->topoffset);
	unguard;
}

//==========================================================================
//
//	TPatchTexture::GetPixels
//
//==========================================================================

byte* TPatchTexture::GetPixels()
{
	guard(TPatchTexture::GetPixels);
	if (Pixels)
	{
		Z_ChangeTag(Pixels, PU_STATIC);
		return Pixels;
	}

	patch_t *patch = (patch_t*)W_CacheLumpNum(LumpNum, PU_STATIC);
	Width = LittleShort(patch->width);
	Height = LittleShort(patch->height);
	SOffset = LittleShort(patch->leftoffset);
	TOffset = LittleShort(patch->topoffset);
	Pixels = (byte*)Z_Calloc(Width * Height, PU_STATIC, (void**)&Pixels);
	int black = r_black_colour;
	for (int x = 0; x < Width; x++)
	{
		column_t* column = (column_t*)((byte*)patch +
			LittleLong(patch->columnofs[x]));
		// step through the posts in a column
		int top = -1;	//	DeepSea tall patches support
		while (column->topdelta != 0xff)
		{
			if (column->topdelta <= top)
			{
				top += column->topdelta;
			}
			else
			{
				top = column->topdelta;
			}
			byte* source = (byte*)column + 3;
			byte* dest = Pixels + x + top * Width;
			int count = column->length;
			while (count--)
			{
				*dest = *source || bNoRemap0 ? *source : black;
				source++;
				dest += Width;
			}
			column = (column_t*)((byte*)column + column->length + 4);
		}
	}
	Z_ChangeTag(patch, PU_CACHE);
	return Pixels;
	unguard;
}

//==========================================================================
//
//	TPatchTexture::MakePurgable
//
//==========================================================================

void TPatchTexture::MakePurgable()
{
	guardSlow(TPatchTexture::MakePurgable);
	Z_ChangeTag(Pixels, PU_CACHE);
	unguardSlow;
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
		Z_Free(Pixels);
		Pixels = NULL;
	}
	unguard;
}

//**************************************************************************
//	TMultiPatchTexture
//**************************************************************************

//==========================================================================
//
//	TMultiPatchTexture::TMultiPatchTexture
//
//==========================================================================

TMultiPatchTexture::TMultiPatchTexture(FName InName, int InWidth,
	int InHeight, float InSScale, float InTScale, int InPatchCount)
: Pixels(0)
{
	Type = TEXTYPE_Wall;
	Format = TEXFMT_8;
	Name = InName;
	Width = InWidth;
	Height = InHeight;
	SScale = InSScale;
	TScale = InTScale;
	PatchCount = InPatchCount;
	Patches = (texpatch_t*)Z_Calloc(PatchCount * sizeof(texpatch_t));
}

//==========================================================================
//
//	TMultiPatchTexture::SetFrontSkyLayer
//
//==========================================================================

void TMultiPatchTexture::SetFrontSkyLayer()
{
	guard(TMultiPatchTexture::SetFrontSkyLayer);
	for (int i = 0; i < PatchCount; i++)
	{
		Patches[i].Tex->SetFrontSkyLayer();
	}
	bNoRemap0 = true;
	unguard;
}

//==========================================================================
//
//	TMultiPatchTexture::GetPixels
//
// 	Using the texture definition, the composite texture is created from the
// patches, and each column is cached.
//
//==========================================================================

byte* TMultiPatchTexture::GetPixels()
{
	guard(TMultiPatchTexture::GetPixels);
	if (Pixels)
	{
		Z_ChangeTag(Pixels, PU_STATIC);
		return Pixels;
	}

	Pixels = (byte*)Z_Calloc(Width * Height, PU_STATIC, (void**)&Pixels);

	// Composite the columns together.
	texpatch_t* patch = Patches;
	for (int i = 0; i < PatchCount; i++, patch++)
	{
		TTexture* PatchTex = patch->Tex;
		byte* PatchPixels = PatchTex->GetPixels8();
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
		PatchTex->MakePurgable();
	}
	return Pixels;
	unguard;
}

//==========================================================================
//
//	TMultiPatchTexture::MakePurgable
//
//==========================================================================

void TMultiPatchTexture::MakePurgable()
{
	guard(TMultiPatchTexture::MakePurgable);
	Z_ChangeTag(Pixels, PU_CACHE);
	unguard;
}

//==========================================================================
//
//	TMultiPatchTexture::Unload
//
//==========================================================================

void TMultiPatchTexture::Unload()
{
	guard(TMultiPatchTexture::Unload);
	if (Pixels)
	{
		Z_Free(Pixels);
		Pixels = NULL;
	}
	unguard;
}

//**************************************************************************
//	TFlatTexture
//**************************************************************************

//==========================================================================
//
//	TFlatTexture::TFlatTexture
//
//==========================================================================

TFlatTexture::TFlatTexture(int InLumpNum)
: Pixels(0)
{
	Type = TEXTYPE_Flat;
	Format = TEXFMT_8;
	Name = FName(W_LumpName(InLumpNum), FNAME_AddLower8);
	Width = 64;
	Height = 64;
	LumpNum = InLumpNum;
}

//==========================================================================
//
//	TFlatTexture::GetPixels
//
//==========================================================================

byte* TFlatTexture::GetPixels()
{
	guard(TFlatTexture::GetPixels);
	if (Pixels)
	{
		Z_ChangeTag(Pixels, PU_STATIC);
		return Pixels;
	}

	Pixels = (byte*)Z_Malloc(64 * 64, PU_STATIC, (void**)&Pixels);
	if (W_LumpLength(LumpNum) < 64 * 64)
	{
		memset(Pixels, 0, 64 * 64);
		return Pixels;
	}
	byte* data = (byte*)W_CacheLumpNum(LumpNum, PU_TEMP);
	for (int i = 0; i < 64 * 64; i++)
	{
		Pixels[i] = data[i] ? data[i] : r_black_colour;
	}
	return Pixels;
	unguard;
}

//==========================================================================
//
//	TFlatTexture::MakePurgable
//
//==========================================================================

void TFlatTexture::MakePurgable()
{
	guard(TFlatTexture::MakePurgable);
	Z_ChangeTag(Pixels, PU_CACHE);
	unguard;
}

//==========================================================================
//
//	TFlatTexture::Unload
//
//==========================================================================

void TFlatTexture::Unload()
{
	guard(TFlatTexture::Unload);
	if (Pixels)
	{
		Z_Free(Pixels);
		Pixels = NULL;
	}
	unguard;
}

//**************************************************************************
//	TRawPicTexture
//**************************************************************************

//==========================================================================
//
//	TRawPicTexture::TRawPicTexture
//
//==========================================================================

TRawPicTexture::TRawPicTexture(int InLumpNum, int InPalLumpNum)
: LumpNum(InLumpNum)
, PalLumpNum(InPalLumpNum)
, Pixels(0)
, Palette(0)
{
	Type = TEXTYPE_Pic;
	Name = FName(W_LumpName(InLumpNum), FNAME_AddLower8);
	Width = 320;
	Height = W_LumpLength(InLumpNum) / 320;
	Format = PalLumpNum >= 0 ? TEXFMT_8Pal : TEXFMT_8;
}

//==========================================================================
//
//	TRawPicTexture::GetPixels
//
//==========================================================================

byte* TRawPicTexture::GetPixels()
{
	guard(TRawPicTexture::GetPixels);
	if (Pixels)
	{
		Z_ChangeTag(Pixels, PU_STATIC);
		return Pixels;
	}

	int len = W_LumpLength(LumpNum);
	Height = len / 320;

	Pixels = (byte*)Z_Calloc(len, PU_STATIC, (void**)&Pixels);
	byte *raw = (byte*)W_CacheLumpNum(LumpNum, PU_STATIC);

	//	Set up palette.
	int black;
	if (PalLumpNum < 0)
	{
		black = r_black_colour;
	}
	else
	{
		//	Load palette and find black colour for remaping.
		Palette = (rgba_t*)Z_Malloc(256 * 4, PU_STATIC, (void**)&Palette);
		byte *psrc = (byte*)W_CacheLumpNum(PalLumpNum, PU_TEMP);
		int best_dist = 0x10000;
		black = 0;
		for (int i = 0; i < 256; i++)
		{
			Palette[i].r = *psrc++;
			Palette[i].g = *psrc++;
			Palette[i].b = *psrc++;
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
	}

	byte *src = raw;
	byte *dst = Pixels;
	for (int i = 0; i < len; i++, src++, dst++)
	{
		*dst = *src ? *src : black;
	}
	Z_Free(raw);
	return Pixels;
	unguard;
}

//==========================================================================
//
//	TRawPicTexture::GetPalette
//
//==========================================================================

rgba_t* TRawPicTexture::GetPalette()
{
	guardSlow(TRawPicTexture::GetPalette);
	return Palette ? Palette : r_palette;
	unguardSlow;
}

//==========================================================================
//
//	TRawPicTexture::MakePurgable
//
//==========================================================================

void TRawPicTexture::MakePurgable()
{
	guard(TRawPicTexture::MakePurgable);
	Z_ChangeTag(Pixels, PU_CACHE);
	if (Palette)
	{
		Z_ChangeTag(Palette, PU_CACHE);
	}
	unguard;
}

//==========================================================================
//
//	TRawPicTexture::Unload
//
//==========================================================================

void TRawPicTexture::Unload()
{
	guard(TRawPicTexture::Unload);
	if (Pixels)
	{
		Z_Free(Pixels);
		Pixels = NULL;
	}
	if (Palette)
	{
		Z_Free(Palette);
		Palette = NULL;
	}
	unguard;
}

//**************************************************************************
//	TImgzTexture
//**************************************************************************

//==========================================================================
//
//	TImgzTexture::TImgzTexture
//
//==========================================================================

TImgzTexture::TImgzTexture(int InType, int InLumpNum)
: LumpNum(InLumpNum)
, Pixels(0)
{
	Type = InType;
	Name = FName(W_LumpName(InLumpNum), FNAME_AddLower8);
	Format = TEXFMT_8;
}

//==========================================================================
//
//	TImgzTexture::GetDimensions
//
//==========================================================================

void TImgzTexture::GetDimensions()
{
	guard(TImgzTexture::GetDimensions);
	TIMGZHeader* Hdr = (TIMGZHeader*)W_CacheLumpNum(LumpNum, PU_CACHE);
	Width = LittleShort(Hdr->Width);
	Height = LittleShort(Hdr->Height);
	SOffset = LittleShort(Hdr->LeftOffset);
	TOffset = LittleShort(Hdr->TopOffset);
	unguard;
}

//==========================================================================
//
//	TImgzTexture::GetPixels
//
//==========================================================================

byte* TImgzTexture::GetPixels()
{
	guard(TImgzTexture::GetPixels);
	if (Pixels)
	{
		Z_ChangeTag(Pixels, PU_STATIC);
		return Pixels;
	}

	TIMGZHeader* Hdr = (TIMGZHeader*)W_CacheLumpNum(LumpNum, PU_STATIC);
	Width = LittleShort(Hdr->Width);
	Height = LittleShort(Hdr->Height);
	SOffset = LittleShort(Hdr->LeftOffset);
	TOffset = LittleShort(Hdr->TopOffset);
	Pixels = (byte*)Z_Calloc(Width * Height, PU_STATIC, (void**)&Pixels);
	if (!Hdr->Compression)
	{
		memcpy(Pixels, &Hdr[1], Width * Height);
	}
	else
	{
		//	IMGZ compression is the same RLE used by IFF ILBM files
		byte* pSrc = (byte*)&Hdr[1];
		byte* pDst = Pixels;
		int runlen = 0, setlen = 0;
		byte setval = 0;  // Shut up, GCC

		for (int y = Height; y != 0; --y)
		{
			for (int x = Width; x != 0; )
			{
				if (runlen != 0)
				{
					byte color = *pSrc;
					*pDst = color;
					pDst++;
					pSrc++;
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
					char code = *pSrc++;
					if (code >= 0)
					{
						runlen = code + 1;
					}
					else if (code != -128)
					{
						setlen = (-code) + 1;
						setval = *pSrc++;
					}
				}
			}
		}
	}
	Z_ChangeTag(Hdr, PU_CACHE);
	return Pixels;
	unguard;
}

//==========================================================================
//
//	TImgzTexture::MakePurgable
//
//==========================================================================

void TImgzTexture::MakePurgable()
{
	guardSlow(TImgzTexture::MakePurgable);
	Z_ChangeTag(Pixels, PU_CACHE);
	unguardSlow;
}

//==========================================================================
//
//	TImgzTexture::Unload
//
//==========================================================================

void TImgzTexture::Unload()
{
	guard(TImgzTexture::Unload);
	if (Pixels)
	{
		Z_Free(Pixels);
		Pixels = NULL;
	}
	unguard;
}

//**************************************************************************
//	TPngLumpTexture
//**************************************************************************

//==========================================================================
//
//	TPngLumpTexture::TPngLumpTexture
//
//==========================================================================

TPngLumpTexture::TPngLumpTexture(int InType, int InLumpNum)
: LumpNum(InLumpNum)
, Pixels(0)
, Palette(0)
{
	Type = InType;
	Name = FName(W_LumpName(InLumpNum), FNAME_AddLower8);
	Format = TEXFMT_8;
}

//==========================================================================
//
//	TPngLumpTexture::GetDimensions
//
//==========================================================================

void TPngLumpTexture::GetDimensions()
{
	guard(TPngLumpTexture::GetDimensions);
	GetPixels();
	MakePurgable();
	unguard;
}

//==========================================================================
//
//	TPngLumpTexture::GetPixels
//
//==========================================================================

byte* TPngLumpTexture::GetPixels()
{
	guard(TPngLumpTexture::GetPixels);
#ifdef CLIENT
	//	If this is 8 bit texture and we have pixels but palette is missing,
	// then also discard pixels.
	if (Pixels && Format == TEXFMT_8Pal && !Palette)
	{
		Z_Free(Pixels);
	}

	//	If we already have loaded pixels, return them.
	if (Pixels)
	{
		//	Make them non-cachable.
		Z_ChangeTag(Pixels, PU_STATIC);
		if (Palette)
		{
			Z_ChangeTag(Palette, PU_STATIC);
		}
		return Pixels;
	}

	//	Discard palette if it's still present.
	if (Palette)
	{
		Z_Free(Palette);
	}

	//	Load texture.
	LoadPNGLump(LumpNum, (void**)&Pixels);
	Width = SkinWidth;
	Height = SkinHeight;
	Format = SkinBPP == 8 ? TEXFMT_8Pal : TEXFMT_RGBA;

	//	For 8-bit textures create a local copy of the palette and remap
	// colour 0.
	if (Format == TEXFMT_8Pal)
	{
		Palette = (rgba_t*)Z_Malloc(256 * 4, PU_STATIC, (void**)&Palette);
		memcpy(Palette, SkinPal, 256 * 4);

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
	}
	return Pixels;
#else
	return NULL;
#endif
	unguard;
}

//==========================================================================
//
//	TPngLumpTexture::GetPalette
//
//==========================================================================

rgba_t* TPngLumpTexture::GetPalette()
{
	guardSlow(TPngLumpTexture::GetPalette);
	return Palette;
	unguardSlow;
}

//==========================================================================
//
//	TPngLumpTexture::MakePurgable
//
//==========================================================================

void TPngLumpTexture::MakePurgable()
{
	guardSlow(TPngLumpTexture::MakePurgable);
	Z_ChangeTag(Pixels, PU_CACHE);
	if (Palette)
	{
		Z_ChangeTag(Palette, PU_CACHE);
	}
	unguardSlow;
}

//==========================================================================
//
//	TPngLumpTexture::Unload
//
//==========================================================================

void TPngLumpTexture::Unload()
{
	guard(TPngLumpTexture::Unload);
	if (Pixels)
	{
		Z_Free(Pixels);
		Pixels = NULL;
	}
	if (Palette)
	{
		Z_Free(Palette);
		Palette = NULL;
	}
	unguard;
}

//**************************************************************************
//	TFileTexture
//**************************************************************************

//==========================================================================
//
//	TFileTexture::TFileTexture
//
//==========================================================================

TFileTexture::TFileTexture(int InType, FName InName)
: Pixels(0)
, Palette(0)
{
	Type = InType;
	Name = InName;
}

//==========================================================================
//
//	TFileTexture::GetDimensions
//
//==========================================================================

void TFileTexture::GetDimensions()
{
	guard(TFileTexture::GetDimensions);
	GetPixels();
	MakePurgable();
	unguard;
}

//==========================================================================
//
//	TFileTexture::GetPixels
//
//==========================================================================

byte* TFileTexture::GetPixels()
{
	guard(TFileTexture::GetPixels);
#ifdef CLIENT
	//	If this is 8 bit texture and we have pixels but palette is missing,
	// then also discard pixels.
	if (Pixels && Format == TEXFMT_8Pal && !Palette)
	{
		Z_Free(Pixels);
	}

	//	If we already have loaded pixels, return them.
	if (Pixels)
	{
		//	Make them non-cachable.
		Z_ChangeTag(Pixels, PU_STATIC);
		if (Palette)
		{
			Z_ChangeTag(Palette, PU_STATIC);
		}
		return Pixels;
	}

	//	Discard palette if it's still present.
	if (Palette)
	{
		Z_Free(Palette);
	}

	//	Load texture.
	Mod_LoadSkin(*Name, (void**)&Pixels);
	Width = SkinWidth;
	Height = SkinHeight;
	Format = SkinBPP == 8 ? TEXFMT_8Pal : TEXFMT_RGBA;

	//	For 8-bit textures create a local copy of the palette and remap
	// colour 0.
	if (Format == TEXFMT_8Pal)
	{
		Palette = (rgba_t*)Z_Malloc(256 * 4, PU_STATIC, (void**)&Palette);
		memcpy(Palette, SkinPal, 256 * 4);

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
	}
	return Pixels;
#else
	return NULL;
#endif
	unguard;
}

//==========================================================================
//
//	TFileTexture::GetPalette
//
//==========================================================================

rgba_t* TFileTexture::GetPalette()
{
	guardSlow(TFileTexture::GetPalette);
	return Palette;
	unguardSlow;
}

//==========================================================================
//
//	TFileTexture::MakePurgable
//
//==========================================================================

void TFileTexture::MakePurgable()
{
	guard(TFileTexture::MakePurgable);
	//	Make pixels and palette cacheable.
	Z_ChangeTag(Pixels, PU_CACHE);
	if (Palette)
	{
		Z_ChangeTag(Palette, PU_CACHE);
	}
	unguard;
}

//==========================================================================
//
//	TFileTexture::Unload
//
//==========================================================================

void TFileTexture::Unload()
{
	guard(TFileTexture::Unload);
	if (Pixels)
	{
		Z_Free(Pixels);
		Pixels = NULL;
	}
	if (Palette)
	{
		Z_Free(Palette);
		Palette = NULL;
	}
	unguard;
}

//**************************************************************************
//	TPcxFileTexture
//**************************************************************************

//==========================================================================
//
//	TPcxFileTexture::TPcxFileTexture
//
//==========================================================================

TPcxFileTexture::TPcxFileTexture(int InType, FName InName)
: TFileTexture(InType, InName)
{
}

//**************************************************************************
//	TTgaFileTexture
//**************************************************************************

//==========================================================================
//
//	TTgaFileTexture::TTgaFileTexture
//
//==========================================================================

TTgaFileTexture::TTgaFileTexture(int InType, FName InName)
: TFileTexture(InType, InName)
{
}

//**************************************************************************
//	TPngFileTexture
//**************************************************************************

//==========================================================================
//
//	TPngFileTexture::TPngFileTexture
//
//==========================================================================

TPngFileTexture::TPngFileTexture(int InType, FName InName)
: TFileTexture(InType, InName)
{
}

//==========================================================================
//
//	P_InitAnimated
//
// Load the table of animation definitions, checking for existence of
// the start and end of each frame. If the start doesn't exist the sequence
// is skipped, if the last doesn't exist, BOOM exits.
//
// Wall/Flat animation sequences, defined by name of first and last frame,
// The full animation sequence is given using all lumps between the start
// and end entry, in the order found in the WAD file.
//
// This routine modified to read its data from a predefined lump or
// PWAD lump called ANIMATED rather than a static table in this module to
// allow wad designers to insert or modify animation sequences.
//
// Lump format is an array of byte packed animdef_t structures, terminated
// by a structure with istexture == -1. The lump can be generated from a
// text source file using SWANTBLS.EXE, distributed with the BOOM utils.
// The standard list of switches and animations is contained in the example
// source text file DEFSWANI.DAT also in the BOOM util distribution.
//
// [RH] Rewritten to support BOOM ANIMATED lump but also make absolutely
//		no assumptions about how the compiler packs the animdefs array.
//
//==========================================================================

void P_InitAnimated()
{
	animDef_t 	ad;
	frameDef_t	fd;

	if (W_CheckNumForName("ANIMATED") < 0)
	{
		return;
	}

	char *animdefs = (char*)W_CacheLumpName("ANIMATED", PU_STRING);
	char *anim_p;
	int pic1, pic2;

	for (anim_p = animdefs; *anim_p != -1; anim_p += 23)
	{
		if (*anim_p & 1)
		{
			pic1 = GTextureManager.CheckNumForName(FName(anim_p + 10,
				FNAME_AddLower8), TEXTYPE_Wall, true, false);
			pic2 = GTextureManager.CheckNumForName(FName(anim_p + 1,
				FNAME_AddLower8), TEXTYPE_Wall, true, false);
		}
		else
		{
			pic1 = GTextureManager.CheckNumForName(FName(anim_p + 10,
				FNAME_AddLower8), TEXTYPE_Flat, true, false);
			pic2 = GTextureManager.CheckNumForName(FName(anim_p + 1,
				FNAME_AddLower8), TEXTYPE_Flat, true, false);
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
			Sys_Error("P_InitPicAnims: bad cycle from %s to %s", anim_p + 10,
				anim_p + 1);
		
		fd.baseTime = (anim_p[19] << 0) | (anim_p[20] << 8) |
					(anim_p[21] << 16) | (anim_p[22] << 24);
		fd.randomRange = 0;
		FrameDefs.AddItem(fd);

		ad.endFrameDef = FrameDefs.Num() - 1;
		ad.CurrentRangeFrame = FrameDefs[ad.startFrameDef].index - ad.index;
		ad.currentFrameDef = ad.endFrameDef;
		ad.time = 0.01; // Force 1st game tic to animate
		AnimDefs.AddItem(ad);
	}
	Z_Free(animdefs);
}

//==========================================================================
//
//	ParseFTAnim
//
//	Parse flat or texture animation.
//
//==========================================================================

static void ParseFTAnim(int IsFlat)
{
	animDef_t 	ad;
	frameDef_t	fd;
	bool 		ignore;
	bool		optional;

	memset(&ad, 0, sizeof(ad));

	//	Optional flag.
	optional = false;
	SC_MustGetString();
	if (SC_Compare("optional"))
	{
		optional = true;
		SC_MustGetString();
	}

	//	Name
	ignore = false;
	ad.index = GTextureManager.CheckNumForName(FName(sc_String,
		FNAME_AddLower8), IsFlat ? TEXTYPE_Flat : TEXTYPE_Wall, true, true);
	if (ad.index == -1)
	{
		ignore = true;
		if (!optional)
		{
			GCon->Logf("ANIMDEFS: Can't find %s", sc_String);
		}
	}
	bool missing = ignore && optional;

	bool HadPic = false;
	ad.startFrameDef = FrameDefs.Num();
	while (SC_GetString())
	{
		if (SC_Compare ("allowdecals"))
		{
			//	Since we don't have decals yet, ignore it.
			continue;
		}

		if (SC_Compare("pic"))
		{
			if (ad.IsRange)
			{
				SC_ScriptError ("You cannot use pic together with range.");
			}
			HadPic = true;
		}
		else if (SC_Compare ("range"))
		{
			if (ad.IsRange)
			{
				SC_ScriptError("You can only use range once in a single animation.");
			}
			if (HadPic)
			{
				SC_ScriptError("You cannot use range together with pic.");
			}
			ad.IsRange = true;
		}
		else
		{
			SC_UnGet();
			break;
		}

		memset(&fd, 0, sizeof(fd));
		if (SC_CheckNumber())
		{
			fd.index = ad.index + sc_Number - 1;
		}
		else
		{
			SC_MustGetString();
			fd.index = GTextureManager.CheckNumForName(FName(sc_String,
				FNAME_AddLower8), IsFlat ? TEXTYPE_Flat : TEXTYPE_Wall, true, true);
			if (fd.index == -1 && !missing)
			{
				SC_ScriptError(va("Unknown texture %s", sc_String));
			}
		}
		SC_MustGetString();
		if (SC_Compare("tics"))
		{
			SC_MustGetNumber();
			fd.baseTime = sc_Number;
			fd.randomRange = 0;
		}
		else if (SC_Compare("rand"))
		{
			SC_MustGetNumber();
			fd.baseTime = sc_Number;
			SC_MustGetNumber();
			fd.randomRange = sc_Number - fd.baseTime + 1;
		}
		else
		{
			SC_ScriptError(NULL);
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
		if (ignore == false)
		{
			FrameDefs.AddItem(fd);
		}
	}

	if ((ignore == false) && !ad.IsRange &&
		(FrameDefs.Num() - ad.startFrameDef < 2))
	{
		Sys_Error("P_InitFTAnims: AnimDef has framecount < 2.");
	}

	if (ignore == false)
	{
		ad.endFrameDef = FrameDefs.Num() - 1;
		ad.CurrentRangeFrame = FrameDefs[ad.startFrameDef].index - ad.index;
		ad.currentFrameDef = ad.endFrameDef;
		ad.time = 0.01; // Force 1st game tic to animate
		AnimDefs.AddItem(ad);
	}
}

//==========================================================================
//
//	ParseSwitchDef
//
//==========================================================================

static void ParseSwitchDef()
{
	SC_MustGetString();

	//	Skip game specifier.
	if (SC_Compare("doom"))
	{
		SC_MustGetNumber();
		SC_MustGetString();
	}
	else if (SC_Compare ("heretic"))
	{
		SC_MustGetString();
	}
	else if (SC_Compare ("hexen"))
	{
		SC_MustGetString();
	}
	else if (SC_Compare ("strife"))
	{
		SC_MustGetString();
	}
	else if (SC_Compare ("any"))
	{
		SC_MustGetString();
	}

	//	Switch texture
	int t1 = GTextureManager.CheckNumForName(FName(sc_String,
		FNAME_AddLower8), TEXTYPE_Wall, true, false);
	int t2 = -1;
	char SndName[64];
	strcpy(SndName, "Switch");

	//	Currently only basic switch definition is supported.
	while (SC_GetString())
	{
		if (SC_Compare("quest"))
		{
		}
		else if (SC_Compare("on"))
		{
			while (SC_GetString())
			{
				if (SC_Compare("sound"))
				{
					SC_MustGetString();
					strcpy(SndName, sc_String);
				}
				else if (SC_Compare("pic"))
				{
					SC_MustGetString();
					t2 = GTextureManager.CheckNumForName(FName(sc_String,
						FNAME_AddLower8), TEXTYPE_Wall, true, false);
					SC_MustGetStringName("tics");
					SC_MustGetStringName("0");
				}
				else
				{
					SC_UnGet();
					break;
				}
			}
		}
		else
		{
			SC_UnGet();
			break;
		}
	}

	if (t1 < 0 || t2 < 0)
	{
		return;
	}
	TSwitch *sw = new(Switches) TSwitch;
	sw->Sound = S_GetSoundID(SndName);
	sw->Tex1 = t1;
	sw->Tex2 = t2;
}

//==========================================================================
//
//	ParseFTAnims
//
//	Initialize flat and texture animation lists.
//
//==========================================================================

static void ParseFTAnims()
{
	guard(ParseFTAnims);
	while (SC_GetString())
	{
		if (SC_Compare("flat"))
		{
			ParseFTAnim(true);
		}
		else if (SC_Compare("texture"))
		{
			ParseFTAnim(false);
		}
		else if (SC_Compare("switch"))
		{
			ParseSwitchDef();
		}
		else
		{
			SC_ScriptError(NULL);
		}
	}
	SC_Close();
	unguard;
}

//==========================================================================
//
//	AnimsCallback
//
//==========================================================================

static bool AnimsCallback(int lump, const char* name, int, EWadNamespace NS)
{
	if (NS == WADNS_Global && !stricmp(name, "animdefs"))
	{
		SC_OpenLumpNum(lump);
		ParseFTAnims();
	}
	return true;
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
	char filename[MAX_OSPATH];

	//	Process all animdefs lumps.
	W_ForEachLump(AnimsCallback);

	//	Optionally parse script file.
	if (fl_devmode && FL_FindFile("scripts/animdefs.txt", filename))
	{
		SC_OpenFile(filename);
		ParseFTAnims();
	}

	//	Read Boom's animated lump if present.
	P_InitAnimated();

	FrameDefs.Shrink();
	AnimDefs.Shrink();
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
	int lump = W_CheckNumForName("SWITCHES");
	if (lump != -1)
	{
		const char* alphSwitchList = (const char*)W_CacheLumpNum(lump, PU_STATIC);
		const char* list_p;

		for (list_p = alphSwitchList; list_p[18] || list_p[19]; list_p += 20)
		{
			// Check for switches that aren't really switches
			if (!stricmp(list_p, list_p + 9))
			{
				GCon->Logf(NAME_Init, "Switch %s in SWITCHES has the same 'on' state", list_p);
				continue;
			}
			int t1 = GTextureManager.CheckNumForName(FName(list_p,
				FNAME_AddLower8), TEXTYPE_Wall, true, false);
			int t2 = GTextureManager.CheckNumForName(FName(list_p + 9,
				FNAME_AddLower8), TEXTYPE_Wall, true, false);
			if (t1 < 0 || t2 < 0)
			{
				continue;
			}
			TSwitch *sw = new(Switches) TSwitch;
			sw->Sound = S_GetSoundID("Switch");
			sw->Tex1 = t1;
			sw->Tex2 = t2;
		}
		Z_Free((void*)alphSwitchList);
	}
	Switches.Shrink();
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
	for (TArray<animDef_t>::TIterator ad(AnimDefs); ad; ++ad)
	{
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

//**************************************************************************
//
//	$Log$
//	Revision 1.35  2005/12/29 20:17:41  dj_jl
//	Fix for too small flats.
//
//	Revision 1.34  2005/11/20 10:39:57  dj_jl
//	Fixed skin index check.
//	
//	Revision 1.33  2005/10/02 23:14:47  dj_jl
//	Case insensitive extension check.
//	
//	Revision 1.32  2005/09/04 14:57:24  dj_jl
//	Fixed check for any texture.
//	
//	Revision 1.31  2005/07/05 22:52:10  dj_jl
//	Fixes to compile under M$VC
//	
//	Revision 1.30  2005/07/03 12:06:56  dj_jl
//	Moved switches to animdefs lump.
//	Added support for BOOM switches lump.
//	
//	Revision 1.29  2005/06/08 22:52:16  dj_jl
//	Fixed automap background.
//	
//	Revision 1.28  2005/05/30 18:34:03  dj_jl
//	Added support for IMGZ and PNG lump textures
//	
//	Revision 1.27  2005/05/26 16:50:15  dj_jl
//	Created texture manager class
//	
//	Revision 1.26  2005/05/03 15:00:11  dj_jl
//	Moved switch list, animdefs enhancements.
//	
//	Revision 1.25  2005/04/28 07:16:15  dj_jl
//	Fixed some warnings, other minor fixes.
//	
//	Revision 1.24  2004/12/22 07:51:52  dj_jl
//	Warning about non-existing graphics.
//	
//	Revision 1.23  2004/11/23 12:43:10  dj_jl
//	Wad file lump namespaces.
//	
//	Revision 1.22  2004/08/18 18:05:47  dj_jl
//	Support for higher virtual screen resolutions.
//	
//	Revision 1.21  2002/09/07 16:31:51  dj_jl
//	Added Level class.
//	
//	Revision 1.20  2002/07/27 18:10:11  dj_jl
//	Implementing Strife conversations.
//	
//	Revision 1.19  2002/07/13 07:51:49  dj_jl
//	Replacing console's iostream with output device.
//	
//	Revision 1.18  2002/03/28 17:58:02  dj_jl
//	Added support for scaled textures.
//	
//	Revision 1.17  2002/03/20 19:11:21  dj_jl
//	Added guarding.
//	
//	Revision 1.16  2002/02/22 18:09:52  dj_jl
//	Some improvements, beautification.
//	
//	Revision 1.15  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.14  2001/12/27 17:36:47  dj_jl
//	Some speedup
//	
//	Revision 1.13  2001/12/12 19:26:40  dj_jl
//	Added dynamic arrays
//	
//	Revision 1.12  2001/11/09 14:22:10  dj_jl
//	R_InitTexture now called from Host_init
//	
//	Revision 1.11  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.10  2001/10/08 17:34:57  dj_jl
//	A lots of small changes and cleanups
//	
//	Revision 1.9  2001/10/04 17:23:29  dj_jl
//	Got rid of some warnings
//	
//	Revision 1.8  2001/08/30 17:44:07  dj_jl
//	Removed memory leaks after startup
//	
//	Revision 1.7  2001/08/23 17:47:22  dj_jl
//	Started work on pics with custom palettes
//	
//	Revision 1.6  2001/08/21 17:46:08  dj_jl
//	Added R_TextureAnimation, made SetTexture recognize flats
//	
//	Revision 1.5  2001/08/15 17:21:14  dj_jl
//	Removed game dependency
//	
//	Revision 1.4  2001/08/01 17:33:58  dj_jl
//	Fixed drawing of spite lump for player setup menu, beautification
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
