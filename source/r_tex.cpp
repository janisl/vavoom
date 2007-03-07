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

#include "gamedefs.h"
#include "r_tex.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	ANIM_Normal,
	ANIM_Forward,
	ANIM_Backward,
	ANIM_OscillateUp,
	ANIM_OscillateDown,
};

struct frameDef_t
{
	vint16		Index;
	vint16		BaseTime;
	vint16		RandomRange;
};

struct animDef_t
{
	vint16		Index;
	vint16		NumFrames;
	float		Time;
	vint16		StartFrameDef;
	vint16		CurrentFrame;
	vuint8		Type;
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

extern "C" {
vuint8				r_rgbtable[32 * 32 * 32 + 4];
};

//	Switches
TArray<TSwitch*>	Switches;

VCvarI				r_hirestex("r_hirestex", "1", CVAR_Archive);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TArray<animDef_t>	AnimDefs;
static TArray<frameDef_t>	FrameDefs;
static TArray<VAnimDoorDef>	AnimDoorDefs;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VTextureManager::VTextureManager
//
//==========================================================================

VTextureManager::VTextureManager()
: DefaultTexture(-1)
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
	AddTextures();

	//	Initialise flats.
	AddGroup(TEXTYPE_Flat, WADNS_Flats);

	//	Initialise overloaded textures.
	AddGroup(TEXTYPE_Overload, WADNS_NewTextures);

	//	Initialise sprites.
	AddGroup(TEXTYPE_Sprite, WADNS_Sprites);

	//	Initialise hires textures.
	AddHiResTextures();

	//	Find default texture.
	DefaultTexture = CheckNumForName("-noflat-", TEXTYPE_Overload, false,
		false);
	if (DefaultTexture == -1)
	{
		Sys_Error("Default texture -noflat- not found");
	}

	//	Find sky flat number.
	skyflatnum = CheckNumForName(NAME_f_sky, TEXTYPE_Flat, true, false);
	if (skyflatnum < 0)
		skyflatnum = CheckNumForName(NAME_f_sky001, TEXTYPE_Flat, true,
			false);
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
	if (!Tex)
	{
		return -1;
	}
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
		GCon->Logf("VTextureManager::NumForName: %s not found", *Name);
		i = DefaultTexture;
	}
	return i;
	unguard;
}

//==========================================================================
//
//	VTextureManager::TextureHeight
//
//==========================================================================

VName VTextureManager::GetTextureName(int TexNum)
{
	guard(VTextureManager::GetTextureName);
	if (TexNum < 0 || TexNum >= Textures.Num())
	{
		return NAME_None;
	}
	return Textures[TexNum]->Name;
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

int VTextureManager::AddPatch(VName Name, int Type, bool Silent)
{
	guard(VTextureManager::AddPatch);
	//	Find the lump number.
	int LumpNum = W_CheckNumForName(Name, WADNS_Graphics);
	if (LumpNum < 0)
		LumpNum = W_CheckNumForName(Name, WADNS_Sprites);
	if (LumpNum < 0)
	{
		if (!Silent)
		{
			GCon->Logf("VTextureManager::AddPatch: Pic %s not found", *Name);
		}
		return -1;
	}

	//	Check if it's already registered.
	int i = CheckNumForName(Name, Type);
	if (i >= 0)
	{
		return i;
	}

	//	Create new patch texture.
	return AddTexture(VTexture::CreateTexture(Type, LumpNum));
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

	i = W_CheckNumForFileName(*Name);
	if (i >= 0)
	{
		VTexture* Tex = VTexture::CreateTexture(Type, i);
		if (Tex)
		{
			Tex->Name = Name;
			return AddTexture(Tex);
		}
	}

	GCon->Logf("Couldn\'t create texture %s.", *Name);
	return DefaultTexture;
	unguard;
}

//==========================================================================
//
//	VTextureManager::AddTextures
//
// 	Initialises the texture list with the textures from the world map.
//
//==========================================================================

void VTextureManager::AddTextures()
{
	guard(VTextureManager::AddTextures);
	int NamesFile = -1;
	int LumpTex1 = -1;
	int LumpTex2 = -1;
	int FirstTex;

	//	For each PNAMES lump load TEXTURE1 and TEXTURE2 from the same wad.
	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) != NAME_pnames)
		{
			continue;
		}
		NamesFile = W_LumpFile(Lump);
		LumpTex1 = W_CheckNumForNameInFile(NAME_texture1, NamesFile);
		LumpTex2 = W_CheckNumForNameInFile(NAME_texture2, NamesFile);
		FirstTex = Textures.Num();
		AddTexturesLump(Lump, LumpTex1, FirstTex, true);
		AddTexturesLump(Lump, LumpTex2, FirstTex, false);
	}

	//	If last TEXTURE1 or TEXTURE2 are in a wad without a PNAMES, they
	// must be loaded too.
	int LastTex1 = W_CheckNumForName(NAME_texture1);
	int LastTex2 = W_CheckNumForName(NAME_texture2);
	if (LastTex1 >= 0 && (LastTex1 == LumpTex1 ||
		W_LumpFile(LastTex1) <= NamesFile))
	{
		LastTex1 = -1;
	}
	if (LastTex2 >= 0 && (LastTex2 == LumpTex2 ||
		W_LumpFile(LastTex2) <= NamesFile))
	{
		LastTex2 = -1;
	}
	FirstTex = Textures.Num();
	AddTexturesLump(W_GetNumForName(NAME_pnames), LastTex1, FirstTex, true);
	AddTexturesLump(W_GetNumForName(NAME_pnames), LastTex2, FirstTex, false);
	unguard;
}

//==========================================================================
//
//	VTextureManager::AddTexturesLump
//
//==========================================================================

void VTextureManager::AddTexturesLump(int NamesLump, int TexLump,
	int FirstTex, bool First)
{
	guard(VTextureManager::AddTexturesLump);
	if (TexLump < 0)
	{
		return;
	}

	//	Load the patch names from pnames.lmp.
	VStream* Strm = W_CreateLumpReaderNum(NamesLump);
	vint32 nummappatches = Streamer<vint32>(*Strm);
	VTexture** patchtexlookup = new VTexture*[nummappatches];
	for (int i = 0; i < nummappatches; i++)
	{
		//	Read patch name.
		char TmpName[12];
		Strm->Serialise(TmpName, 8);
		TmpName[8] = 0;
		VName PatchName(TmpName, VName::AddLower8);

		//	Check if it's already has ben added.
		int PIdx = CheckNumForName(PatchName, TEXTYPE_WallPatch, false, false);
		if (PIdx >= 0)
		{
			patchtexlookup[i] = Textures[PIdx];
			continue;
		}

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
			patchtexlookup[i] = VTexture::CreateTexture(TEXTYPE_WallPatch,
				LNum);
			AddTexture(patchtexlookup[i]);
		}
	}
	delete Strm;

	//	Load the map texture definitions from textures.lmp.
	//	The data is contained in one or two lumps, TEXTURE1 for shareware,
	// plus TEXTURE2 for commercial.
	Strm = W_CreateLumpReaderNum(TexLump);
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
		VMultiPatchTexture* Tex = new VMultiPatchTexture(*Strm, i,
			patchtexlookup, nummappatches, FirstTex, IsStrife);
		AddTexture(Tex);
		if (i == 0 && First)
		{
			//	Copy dimensions of the first texture to the dummy texture in
			// case they are used.
			Textures[0]->Width = Tex->Width;
			Textures[0]->Height = Tex->Height;
			Tex->Type = TEXTYPE_Null;
		}
	}
	delete Strm;
	delete[] patchtexlookup;
	unguard;
}

//==========================================================================
//
//	VTextureManager::AddGroup
//
//==========================================================================

void VTextureManager::AddGroup(int Type, EWadNamespace Namespace)
{
	guard(VTextureManager::AddGroup);
	for (int Lump = W_IterateNS(-1, Namespace); Lump >= 0;
		Lump = W_IterateNS(Lump, Namespace))
	{
		//	To avoid duplicates, add only the last one.
		if (W_GetNumForName(W_LumpName(Lump), Namespace) != Lump)
		{
			continue;
		}
		AddTexture(VTexture::CreateTexture(Type, Lump));
	}
	unguard;
}

//==========================================================================
//
//	VTextureManager::AddHiResTextures
//
//==========================================================================

void VTextureManager::AddHiResTextures()
{
	guard(VTextureManager::AddHiResTextures);
	for (int Lump = W_IterateNS(-1, WADNS_HiResTextures); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_HiResTextures))
	{
		VName Name = W_LumpName(Lump);
		//	To avoid duplicates, add only the last one.
		if (W_GetNumForName(Name, WADNS_HiResTextures) != Lump)
		{
			continue;
		}

		//	Create new texture.
		VTexture* NewTex = VTexture::CreateTexture(TEXTYPE_Any, Lump);
		if (!NewTex)
		{
			continue;
		}

		//	Find texture to replace.
		int OldIdx = CheckNumForName(Name, TEXTYPE_Wall, true, true);
		if (OldIdx < 0)
		{
			OldIdx = AddPatch(Name, TEXTYPE_Pic, true);
		}

		if (OldIdx < 0)
		{
			//	Add it as a new texture.
			NewTex->Type = TEXTYPE_Overload;
			AddTexture(NewTex);
		}
		else
		{
			//	Repalce existing texture by adjusting scale and offsets.
			VTexture* OldTex = Textures[OldIdx];
			NewTex->bWorldPanning = true;
			NewTex->SScale = NewTex->GetWidth() / OldTex->GetWidth();
			NewTex->TScale = NewTex->GetHeight() / OldTex->GetHeight();
			NewTex->SOffset = (int)floor(OldTex->SOffset * NewTex->SScale);
			NewTex->TOffset = (int)floor(OldTex->TOffset * NewTex->TScale);
			NewTex->Type = OldTex->Type;
			NewTex->TextureTranslation = OldTex->TextureTranslation;
			Textures[OldIdx] = NewTex;
			delete OldTex;
		}
	}

	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) != "hirestex")
		{
			continue;
		}

		VScriptParser* sc = new VScriptParser("hirestex",
			W_CreateLumpReaderNum(Lump));
		while (!sc->AtEnd())
		{
			if (sc->Check("remap"))
			{
				sc->ExpectName8();
				int OldIdx = CheckNumForName(sc->Name8, TEXTYPE_Any, false,
					false);
				if (OldIdx < 0)
				{
					OldIdx = AddPatch(sc->Name8, TEXTYPE_Pic, true);
				}

				sc->ExpectName8();
				int LumpIdx = W_CheckNumForName(sc->Name8, WADNS_Graphics);
				if (OldIdx < 0 || LumpIdx < 0)
				{
					continue;
				}

				//	Create new texture.
				VTexture* NewTex = VTexture::CreateTexture(TEXTYPE_Any,
					LumpIdx);
				if (!NewTex)
				{
					continue;
				}
				//	Repalce existing texture by adjusting scale and offsets.
				VTexture* OldTex = Textures[OldIdx];
				NewTex->bWorldPanning = true;
				NewTex->SScale = NewTex->GetWidth() / OldTex->GetWidth();
				NewTex->TScale = NewTex->GetHeight() / OldTex->GetHeight();
				NewTex->SOffset = (int)floor(OldTex->SOffset * NewTex->SScale);
				NewTex->TOffset = (int)floor(OldTex->TOffset * NewTex->TScale);
				NewTex->Name = OldTex->Name;
				NewTex->Type = OldTex->Type;
				NewTex->TextureTranslation = OldTex->TextureTranslation;
				Textures[OldIdx] = NewTex;
				delete OldTex;
			}
			else if (sc->Check("define"))
			{
				sc->ExpectName8();
				VName Name = sc->Name8;
				int LumpIdx = W_CheckNumForName(sc->Name8, WADNS_Graphics);

				//	Dimensions
				sc->ExpectNumber();
				int Width = sc->Number;
				sc->ExpectNumber();
				int Height = sc->Number;
				if (LumpIdx < 0)
				{
					continue;
				}

				//	Create new texture.
				VTexture* NewTex = VTexture::CreateTexture(TEXTYPE_Overload,
					LumpIdx);
				if (!NewTex)
				{
					continue;
				}

				//	Repalce existing texture by adjusting scale and offsets.
				NewTex->bWorldPanning = true;
				NewTex->SScale = NewTex->GetWidth() / Width;
				NewTex->TScale = NewTex->GetHeight() / Height;
				NewTex->Name = Name;

				int OldIdx = CheckNumForName(Name, TEXTYPE_Overload, false,
					false);
				if (OldIdx >= 0)
				{
					VTexture* OldTex = Textures[OldIdx];
					NewTex->TextureTranslation = OldTex->TextureTranslation;
					Textures[OldIdx] = NewTex;
					delete OldTex;
				}
				else
				{
					AddTexture(NewTex);
				}
			}
			else
			{
				sc->Error("Bad command");
			}
		}
		delete sc;
	}
	unguard;
}

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

		ad.StartFrameDef = FrameDefs.Num();
		ad.Type = ANIM_Forward;

		// [RH] Allow for either forward or backward animations.
		if (pic1 < pic2)
		{
			ad.Index = pic1;
			fd.Index = pic2;
		}
		else
		{
			ad.Index = pic2;
			fd.Index = pic1;
			ad.Type = ANIM_Backward;
		}

		if (fd.Index - ad.Index < 1)
			Sys_Error("P_InitPicAnims: bad cycle from %s to %s", TmpName2,
				TmpName1);
		
		fd.BaseTime = BaseTime;
		fd.RandomRange = 0;
		FrameDefs.Append(fd);

		ad.NumFrames = FrameDefs[ad.StartFrameDef].Index - ad.Index + 1;
		ad.CurrentFrame = ad.NumFrames - 1;
		ad.Time = 0.01; // Force 1st game tic to animate
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
	ad.Index = GTextureManager.CheckNumForName(sc->Name8,
		IsFlat ? TEXTYPE_Flat : TEXTYPE_Wall, true, true);
	if (ad.Index == -1)
	{
		ignore = true;
		if (!optional)
		{
			GCon->Logf("ANIMDEFS: Can't find %s", *sc->String);
		}
	}
	bool missing = ignore && optional;

	int CurType = 0;
	ad.StartFrameDef = FrameDefs.Num();
	ad.Type = ANIM_Normal;
	while (1)
	{
		if (sc->Check("allowdecals"))
		{
			//	Since we don't have decals yet, ignore it.
			continue;
		}

		if (sc->Check("pic"))
		{
			if (CurType == 2)
			{
				sc->Error("You cannot use pic together with range.");
			}
			CurType = 1;
		}
		else if (sc->Check("range"))
		{
			if (CurType == 2)
			{
				sc->Error("You can only use range once in a single animation.");
			}
			if (CurType == 1)
			{
				sc->Error("You cannot use range together with pic.");
			}
			CurType = 2;
			ad.Type = ANIM_Forward;
		}
		else
		{
			break;
		}

		memset(&fd, 0, sizeof(fd));
		if (sc->CheckNumber())
		{
			fd.Index = ad.Index + sc->Number - 1;
		}
		else
		{
			sc->ExpectName8();
			fd.Index = GTextureManager.CheckNumForName(sc->Name8,
				IsFlat ? TEXTYPE_Flat : TEXTYPE_Wall, true, true);
			if (fd.Index == -1 && !missing)
			{
				sc->Error(va("Unknown texture %s", *sc->String));
			}
		}
		if (sc->Check("tics"))
		{
			sc->ExpectNumber();
			fd.BaseTime = sc->Number;
			fd.RandomRange = 0;
		}
		else if (sc->Check("rand"))
		{
			sc->ExpectNumber();
			fd.BaseTime = sc->Number;
			sc->ExpectNumber();
			fd.RandomRange = sc->Number - fd.BaseTime + 1;
		}
		else
		{
			sc->Error("bad command");
		}
		if (ad.Type != ANIM_Normal)
		{
			if (fd.Index < ad.Index)
			{
				int tmp = ad.Index;
				ad.Index = fd.Index;
				fd.Index = tmp;
				ad.Type = ANIM_Backward;
			}
			if (sc->Check("oscillate"))
			{
				ad.Type = ANIM_OscillateUp;
			}
		}
		if (!ignore)
		{
			FrameDefs.Append(fd);
		}
	}

	if (!ignore && ad.Type == ANIM_Normal &&
		FrameDefs.Num() - ad.StartFrameDef < 2)
	{
		sc->Error("AnimDef has framecount < 2.");
	}

	if (!ignore)
	{
		if (ad.Type == ANIM_Normal)
		{
			ad.NumFrames = FrameDefs.Num() - ad.StartFrameDef;
			ad.CurrentFrame = ad.NumFrames - 1;
		}
		else
		{
			ad.NumFrames = FrameDefs[ad.StartFrameDef].Index - ad.Index + 1;
			ad.CurrentFrame = 0;
		}
		ad.Time = 0.01; // Force 1st game tic to animate
		AnimDefs.Append(ad);
	}
}

//==========================================================================
//
//	AddSwitchDef
//
//==========================================================================

static int AddSwitchDef(TSwitch* Switch)
{
	for (int i = 0; i < Switches.Num(); i++)
	{
		if (Switches[i]->Tex == Switch->Tex)
		{
			delete Switches[i];
			Switches[i] = Switch;
			return i;
		}
	}
	return Switches.Append(Switch);
}

//==========================================================================
//
//	ParseSwitchState
//
//==========================================================================

static TSwitch* ParseSwitchState(VScriptParser* sc, bool IgnoreBad)
{
	TArray<TSwitchFrame>	Frames;
	int						Sound = 0;
	bool					Bad = false;

	while (1)
	{
		if (sc->Check("sound"))
		{
			if (Sound)
			{
				sc->Error("Switch state already has a sound");
			}
			sc->ExpectString();
			Sound = GSoundManager->GetSoundID(*sc->String);
		}
		else if (sc->Check("pic"))
		{
			sc->ExpectName8();
			int Tex = GTextureManager.CheckNumForName(sc->Name8,
				TEXTYPE_Wall, true, false);
			if (Tex < 0 && !IgnoreBad)
			{
				Bad = true;
			}
			TSwitchFrame& F = Frames.Alloc();
			F.Texture = Tex;
			if (sc->Check("tics"))
			{
				sc->ExpectNumber();
				F.BaseTime = sc->Number;
				F.RandomRange = 0;
			}
			else if (sc->Check("range"))
			{
				sc->ExpectNumber();
				int Min = sc->Number;
				sc->ExpectNumber();
				int Max = sc->Number;
				if (Min < Max)
				{
					F.BaseTime = Min;
					F.RandomRange = Max - Min + 1;
				}
				else
				{
					F.BaseTime = Max;
					F.RandomRange = Min - Max + 1;
				}
			}
			else
			{
				sc->Error("Must specify a duration for switch frame");
			}
		}
		else
		{
			break;
		}
	}

	if (!Frames.Num())
	{
		sc->Error("Switch state needs at least one frame");
	}
	if (Bad)
	{
		return NULL;
	}

	TSwitch* Def = new TSwitch();
	Def->Sound = Sound;
	Def->NumFrames = Frames.Num();
	Def->Frames = new TSwitchFrame[Frames.Num()];
	for (int i = 0; i < Frames.Num(); i++)
	{
		Def->Frames[i].Texture = Frames[i].Texture;
		Def->Frames[i].BaseTime = Frames[i].BaseTime;
		Def->Frames[i].RandomRange = Frames[i].RandomRange;
	}
	return Def;
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
	bool Quest = false;
	TSwitch* Def1 = NULL;
	TSwitch* Def2 = NULL;

	//	Currently only basic switch definition is supported.
	while (1)
	{
		if (sc->Check("quest"))
		{
			Quest = true;
		}
		else if (sc->Check("on"))
		{
			if (Def1)
			{
				sc->Error("Switch already has an on state");
			}
			Def1 = ParseSwitchState(sc, t1 == -1);
		}
		else if (sc->Check("off"))
		{
			if (Def2)
			{
				sc->Error("Switch already has an off state");
			}
			Def2 = ParseSwitchState(sc, t1 == -1);
		}
		else
		{
			break;
		}
	}

	if (t1 < 0 || !Def1)
	{
		if (Def1)
			delete Def1;
		if (Def2)
			delete Def2;
		return;
	}

	if (!Def2)
	{
		//	If switch has no off state create one that just switches
		// back to base texture.
		Def2 = new TSwitch();
		Def2->Sound = Def1->Sound;
		Def2->NumFrames = 1;
		Def2->Frames = new TSwitchFrame[1];
		Def2->Frames[0].Texture = t1;
		Def2->Frames[0].BaseTime = 0;
		Def2->Frames[0].RandomRange = 0;
	}

	Def1->Tex = t1;
	Def2->Tex = Def1->Frames[Def1->NumFrames - 1].Texture;
	if (Def1->Tex == Def2->Tex)
	{
		sc->Error("On state must not end on base texture");
	}
	Def1->Quest = Quest;
	Def2->Quest = Quest;
	Def2->PairIndex = AddSwitchDef(Def1);
	Def1->PairIndex = AddSwitchDef(Def2);
}

//==========================================================================
//
//	ParseAnimatedDoor
//
//==========================================================================

static void ParseAnimatedDoor(VScriptParser* sc)
{
	guard(ParseAnimatedDoor);
	//	Get base texture name.
	bool ignore = false;
	sc->ExpectName8();
	vint32 BaseTex = GTextureManager.CheckNumForName(sc->Name8,
		TEXTYPE_Wall, true, true);
	if (BaseTex == -1)
	{
		ignore = true;
		GCon->Logf("ANIMDEFS: Can't find %s", *sc->String);
	}

	VName OpenSound(NAME_None);
	VName CloseSound(NAME_None);
	TArray<vint32> Frames;
	while (!sc->AtEnd())
	{
		if (sc->Check("opensound"))
		{
			sc->ExpectString();
			OpenSound = *sc->String;
		}
		else if (sc->Check("closesound"))
		{
			sc->ExpectString();
			CloseSound = *sc->String;
		}
		else if (sc->Check("pic"))
		{
			vint32 v;
			if (sc->CheckNumber())
			{
				v = BaseTex + sc->Number - 1;
			}
			else
			{
				sc->ExpectName8();
				v = GTextureManager.CheckNumForName(sc->Name8,
					TEXTYPE_Wall, true, true);
				if (v == -1 && !ignore)
				{
					sc->Error(va("Unknown texture %s", *sc->String));
				}
			}
			Frames.Append(v);
		}
		else
		{
			break;
		}
	}

	if (!ignore)
	{
		VAnimDoorDef& A = AnimDoorDefs.Alloc();
		A.Texture = BaseTex;
		A.OpenSound = OpenSound;
		A.CloseSound = CloseSound;
		A.NumFrames = Frames.Num();
		A.Frames = new vint32[Frames.Num()];
		for (int i = 0; i < A.NumFrames; i++)
			A.Frames[i] = Frames[i];
	}
	unguard;
};

//==========================================================================
//
//	ParseWarp
//
//==========================================================================

static void ParseWarp(VScriptParser* sc, int Type)
{
	guard(ParseWarp);
	int TexType = TEXTYPE_Wall;
	if (sc->Check("texture"))
	{
		TexType = TEXTYPE_Wall;
	}
	else if (sc->Check("flat"))
	{
		TexType = TEXTYPE_Flat;
	}
	else
	{
		sc->Error("Texture type expected");
	}

	sc->ExpectName8();
	int TexNum = GTextureManager.CheckNumForName(sc->Name8, TexType, true, true);
	if (TexNum < 0)
	{
		return;
	}

	VTexture* SrcTex = GTextureManager.Textures[TexNum];
	VTexture* WarpTex = SrcTex;
	//	Warp only once.
	if (!SrcTex->WarpType)
	{
		if (Type == 1)
			WarpTex = new VWarpTexture(SrcTex);
		else
			WarpTex = new VWarp2Texture(SrcTex);
		WarpTex->Name = SrcTex->Name;
		WarpTex->Type = SrcTex->Type;
		WarpTex->TextureTranslation = SrcTex->TextureTranslation;
		GTextureManager.Textures[TexNum] = WarpTex;
	}
	//	Ignored for now.
	sc->Check("allowdecals");
	unguard;
}

//==========================================================================
//
//	ParseFTAnims
//
//	Initialise flat and texture animation lists.
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
		else if (sc->Check("animateddoor"))
		{
			ParseAnimatedDoor(sc);
		}
		else if (sc->Check("warp"))
		{
			ParseWarp(sc, 1);
		}
		else if (sc->Check("warp2"))
		{
			ParseWarp(sc, 2);
		}
		else if (sc->Check("cameratexture"))
		{
			//	Just skip it for now.
			sc->ExpectString();
			sc->ExpectNumber();
			sc->ExpectNumber();
			if (sc->Check("fit"))
			{
				sc->ExpectNumber();
				sc->ExpectNumber();
			}
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
			TSwitch* Def1 = new TSwitch();
			TSwitch* Def2 = new TSwitch();
			Def1->Sound = 0;
			Def2->Sound = 0;
			Def1->Tex = t1;
			Def2->Tex = t2;
			Def1->NumFrames = 1;
			Def2->NumFrames = 1;
			Def1->Frames = new TSwitchFrame[1];
			Def2->Frames = new TSwitchFrame[1];
			Def1->Frames[0].Texture = t2;
			Def1->Frames[0].BaseTime = 0;
			Def1->Frames[0].RandomRange = 0;
			Def2->Frames[0].Texture = t1;
			Def2->Frames[0].BaseTime = 0;
			Def2->Frames[0].RandomRange = 0;
			Def2->PairIndex = AddSwitchDef(Def1);
			Def1->PairIndex = AddSwitchDef(Def2);
		}
		delete Strm;
	}
	Switches.Condense();
	unguard;
}

//==========================================================================
//
//	R_FindAnimDoor
//
//==========================================================================

VAnimDoorDef* R_FindAnimDoor(vint32 BaseTex)
{
	guard(R_FindAnimDoor);
	for (int i = 0; i < AnimDoorDefs.Num(); i++)
	{
		if (AnimDoorDefs[i].Texture == BaseTex)
		{
			return &AnimDoorDefs[i];
		}
	}
	return NULL;
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
		animDef_t& ad = AnimDefs[i];
		ad.Time -= host_frametime;
		if (ad.Time > 0.0)
		{
			continue;
		}

		switch (ad.Type)
		{
		case ANIM_Normal:
		case ANIM_Forward:
			ad.CurrentFrame = (ad.CurrentFrame + 1) % ad.NumFrames;
			break;

		case ANIM_Backward:
			if (ad.CurrentFrame == 0)
			{
				ad.CurrentFrame = ad.NumFrames - 1;
			}
			else
			{
				ad.CurrentFrame--;
			}
			break;

		case ANIM_OscillateUp:
			ad.CurrentFrame++;
			if (ad.CurrentFrame == ad.NumFrames - 1)
			{
				ad.Type = ANIM_OscillateDown;
			}
			break;

		case ANIM_OscillateDown:
			ad.CurrentFrame--;
			if (ad.CurrentFrame == 0)
			{
				ad.Type = ANIM_OscillateUp;
			}
			break;
		}
		const frameDef_t& fd = FrameDefs[ad.StartFrameDef +
			(ad.Type == ANIM_Normal ? ad.CurrentFrame : 0)];
		ad.Time = fd.BaseTime / 35.0;
		if (fd.RandomRange)
		{
			// Random tics
			ad.Time += Random() * (fd.RandomRange / 35.0);
		}
		if (ad.Type == ANIM_Normal)
		{
			GTextureManager.Textures[ad.Index]->TextureTranslation = fd.Index;
		}
		else
		{
			for (int i = 0; i < ad.NumFrames; i++)
			{
				GTextureManager.Textures[ad.Index + i]->TextureTranslation =
					ad.Index + (ad.CurrentFrame + i) % ad.NumFrames;
			}
		}
	}
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
	for (int i = 0; i < Switches.Num(); i++)
	{
		delete Switches[i];
	}
	Switches.Clear();
	AnimDefs.Clear();
	FrameDefs.Clear();
	for (int i = 0; i < AnimDoorDefs.Num(); i++)
	{
		delete[] AnimDoorDefs[i].Frames;
	}
	AnimDoorDefs.Clear();

	//	Shut down texture manager.
	GTextureManager.Shutdown();
	unguard;
}
