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
#include "r_local.h"

// MACROS ------------------------------------------------------------------

#define ANIM_SCRIPT_NAME	"ANIMDEFS"
#define MAX_ANIM_DEFS 		64
#define MAX_FRAME_DEFS 		256
#define ANIM_FLAT 			0
#define ANIM_TEXTURE 		1
#define SCI_FLAT 			"flat"
#define SCI_TEXTURE 		"texture"
#define SCI_PIC 			"pic"
#define SCI_TICS 			"tics"
#define SCI_RAND 			"rand"

// TYPES -------------------------------------------------------------------

struct frameDef_t
{
	int index;
	int tics;
};

struct animDef_t
{
	int type;
	int index;
	int tics;
	int currentFrameDef;
	int startFrameDef;
	int endFrameDef;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

//
// Texture data
//
int				numtextures;
texdef_t**		textures;
int*			texturetranslation; // Animation

//
// Flats data
//
int				numflats;
int*			flatlumps;
int*			flattranslation;    // Animation
int				skyflatnum;			// sky mapping

//
// Sprite lumps data
//
int				numspritelumps;
int*			spritelumps;
int*			spritewidth;		// needed for pre rendering
int*			spriteheight;
int*			spriteoffset;
int*			spritetopoffset;

//
//	Translation tables
//
byte*			translationtables;

//
//	2D graphics
//
pic_info_t		pic_list[MAX_PICS];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static animDef_t	AnimDefs[MAX_ANIM_DEFS];
static frameDef_t	FrameDefs[MAX_FRAME_DEFS];
static int			AnimDefCount;

static bool			inflats;
static bool			insprites;

static float*		textureheight;		// needed for texture pegging

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	InitTextures
//
// 	Initializes the texture list with the textures from the world map.
//
//==========================================================================

template<class T> void InitTextures(void)
{
	T*				mtexture;
	texdef_t*		texture;
	texpatch_t*		patch;

    int				i;
    int				j;

    int*			maptex;
    int*			maptex2;
    int*			maptex1;
    
    char			name[9];
    char*			names;
    char*			name_p;
    
    int*			patchlookup;
    
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
    patchlookup = (int*)Z_Malloc(nummappatches*sizeof(*patchlookup), PU_HIGH, 0);

    for (i = 0; i < nummappatches; i++)
	{
		strncpy(name, name_p + i * 8, 8);
		patchlookup[i] = W_CheckNumForName(name);
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
    numtextures = numtextures1 + numtextures2;

    textures = (texdef_t**)Z_Calloc(numtextures * 4);
    textureheight = (float*)Z_Calloc(numtextures * 4);
    texturetranslation = (int*)Z_Calloc((numtextures + 1) * 4);

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
	
		mtexture = (T*)((byte *)maptex + offset);

		texture = textures[i] =
	    	(texdef_t*)Z_Malloc(sizeof(texdef_t)
		      + sizeof(texpatch_t) * (LittleShort(mtexture->patchcount) - 1),
		      PU_STATIC, 0);
	
		texture->width = LittleShort(mtexture->width);
		texture->height = LittleShort(mtexture->height);
		texture->patchcount = LittleShort(mtexture->patchcount);

		memcpy(texture->name, mtexture->name, sizeof(texture->name));
		patch = texture->patches;

		for (j = 0; j < texture->patchcount; j++, patch++)
		{
	    	patch->originx = LittleShort(mtexture->patches[j].originx);
	    	patch->originy = LittleShort(mtexture->patches[j].originy);
	    	patch->patch = patchlookup[LittleShort(mtexture->patches[j].patch)];
	    	if (patch->patch == -1)
	    	{
				Sys_Error("InitTextures: Missing patch in texture %s",
			 		texture->name);
	    	}
		}

		if (Game == Heretic)
		{
			if (!stricmp(texture->name, "SKY1") ||
				!stricmp(texture->name, "SKY2") ||
				!stricmp(texture->name, "SKY3") ||
				!stricmp(texture->name, "SKY4"))
			{
				texture->height = 240;
			}
		}

		textureheight[i] = texture->height;
		
    	// Create translation table for global animation.
		texturetranslation[i] = i;
    }

	Z_Free(patchlookup);
    Z_Free(maptex1);
    if (maptex2)
		Z_Free(maptex2);
}

//==========================================================================
//
//  R_CheckTextureNumForName
//
// 	Check whether texture is available. Filter out NoTexture indicator.
//
//==========================================================================

int	R_CheckTextureNumForName(const char *name)
{
    int		i;

    // "NoTexture" marker.
    if (name[0] == '-')		
		return 0;
		
    for (i=0 ; i<numtextures ; i++)
		if (!strnicmp(textures[i]->name, name, 8))
	    	return i;
		
    return -1;
}

//==========================================================================
//
// 	R_TextureNumForName
//
// 	Calls R_CheckTextureNumForName, aborts with error message.
//
//==========================================================================

int	R_TextureNumForName(const char* name)
{
    int		i;
	
    i = R_CheckTextureNumForName (name);

    if (i==-1)
    {
		Host_Error("R_TextureNumForName: %s not found", name);
    }
    return i;
}

//==========================================================================
//
//	R_TextureHeight
//
//==========================================================================

float R_TextureHeight(int pic)
{
	if (pic & TEXF_FLAT)
	{
		return 64.0;
	}
	return textureheight[pic];
}

//==========================================================================
//
//	FlatFunc
//
//==========================================================================

static bool FlatFunc(int lump, const char *name, int)
{
	if (!stricmp(name, "F_START") || !strnicmp(name, "FF_START", 8))
	{
		//	Found a start marker
		inflats = true;
	}
	else if (!stricmp(name, "F_END") || !stricmp(name, "FF_END"))
	{
		//	Found end marker
		inflats = false;
	}
	else if (inflats)
	{
		//	Add flat
		numflats++;
		Z_Resize((void**)&flatlumps, numflats * 4);
		flatlumps[numflats - 1] = lump;
	}
	return true;	//	Continue
}

//==========================================================================
//
//	InitFlats
//
//==========================================================================

static void InitFlats(void)
{
	flatlumps = (int*)Z_Malloc(1, PU_STATIC, 0);
    numflats = 0;
	inflats = false;

	W_ForEachLump(FlatFunc);

    // Create translation table for global animation.
    flattranslation = (int*)Z_Malloc((numflats + 1) * 4, PU_STATIC, 0);
    for (int i = 0; i < numflats; i++)
		flattranslation[i] = i;
}

//==========================================================================
//
//  R_CheckFlatNumForName
//
//==========================================================================

int R_CheckFlatNumForName(const char* name)
{
	for (int i = numflats - 1; i >= 0; i--)
	{
		if (!strnicmp(name, W_LumpName(flatlumps[i]), 8))
		{
			return i | TEXF_FLAT;
		}
	}
	return -1;
}

//==========================================================================
//
//	R_FlatNumForName
//
//	Retrieval, get a flat number for a flat name.
//
//==========================================================================

int R_FlatNumForName(const char* name)
{
    char	namet[9];
    int		i;

	i = R_CheckFlatNumForName(name);
    if (i == -1)
    {
		namet[8] = 0;
		memcpy(namet, name,8);
		Host_Error("R_FlatNumForName: %s not found",namet);
    }
    return i;
}

//==========================================================================
//
//	SpriteCallback
//
//==========================================================================

static bool	SpriteCallback(int lump, const char *name, int)
{
	if (!stricmp(name, "S_START") || !strnicmp(name, "SS_START", 8))
	{
		//	Found start marker
		insprites = true;
	}
	else if (!stricmp(name, "S_END") || !stricmp(name, "SS_END"))
	{
		//	Found end marker
		insprites = false;
	}
	else if (insprites)
	{
		//	Add sprite lump
		numspritelumps++;
		Z_Resize((void**)&spritelumps, numspritelumps * 4);
		spritelumps[numspritelumps - 1] = lump;
	}
	return true;	//	Continue
}

//==========================================================================
//
//	InitSpriteLumps
//
//==========================================================================

static void InitSpriteLumps(void)
{
    int			i;

	spritelumps = (int*)Z_Malloc (1, PU_STATIC, 0);
    numspritelumps = 0;
	insprites = false;

	W_ForEachLump(SpriteCallback);

    spritewidth = (int*)Z_Malloc(numspritelumps * 4, PU_STATIC, 0);
    spriteheight = (int*)Z_Malloc(numspritelumps * 4, PU_STATIC, 0);
    spriteoffset = (int*)Z_Malloc(numspritelumps * 4, PU_STATIC, 0);
    spritetopoffset = (int*)Z_Malloc(numspritelumps * 4, PU_STATIC, 0);
	
    for (i = 0; i < numspritelumps; i++)
	{
		spritewidth[i] = -1;
		spriteheight[i] = -1;
		spriteoffset[i] = -1;
		spritetopoffset[i] = -1;
	}
}

//==========================================================================
//
// 	InitTranslationTables
//
//==========================================================================

static void InitTranslationTables(void)
{
	translationtables = (byte*)W_CacheLumpName("TRANSLAT", PU_STATIC);
}

//==========================================================================
//
//	InitFTAnims
//
//	Initialize flat and texture animation lists.
//
//==========================================================================

static void InitFTAnims(void)
{
	int 		base;
	int 		mod;
	int 		fd;
	animDef_t 	*ad;
	boolean 	ignore;
	boolean 	done;

	fd = 0;
	ad = AnimDefs;
	AnimDefCount = 0;
	SC_Open(ANIM_SCRIPT_NAME);
	while (SC_GetString())
	{
		if (AnimDefCount == MAX_ANIM_DEFS)
		{
			Sys_Error("P_InitFTAnims: too many AnimDefs.");
		}
		if (SC_Compare(SCI_FLAT))
		{
			ad->type = ANIM_FLAT;
		}
		else if (SC_Compare(SCI_TEXTURE))
		{
			ad->type = ANIM_TEXTURE;
		}
		else
		{
			SC_ScriptError(NULL);
		}
		SC_MustGetString(); // Name
		ignore = false;
		if (ad->type == ANIM_FLAT)
		{
			if (R_CheckFlatNumForName(sc_String) == -1)
			{
				ignore = true;
			}
			else
			{
				ad->index = R_FlatNumForName(sc_String) & ~TEXF_FLAT;
			}
		}
		else
		{ // Texture
			if (R_CheckTextureNumForName(sc_String) == -1)
			{
				ignore = true;
			}
			else
			{
				ad->index = R_TextureNumForName(sc_String);
			}
		}
		ad->startFrameDef = fd;
		done = false;
		while (done == false)
		{
			if (SC_GetString())
			{
				if (SC_Compare(SCI_PIC))
				{
					if (fd == MAX_FRAME_DEFS)
					{
						Sys_Error("P_InitFTAnims: too many FrameDefs.");
					}
					SC_MustGetNumber();
					if (ignore == false)
					{
						FrameDefs[fd].index = ad->index + sc_Number - 1;
					}
					SC_MustGetString();
					if (SC_Compare(SCI_TICS))
					{
						SC_MustGetNumber();
						if (ignore == false)
						{
							FrameDefs[fd].tics = sc_Number;
							fd++;
						}
					}
					else if (SC_Compare(SCI_RAND))
					{
						SC_MustGetNumber();
						base = sc_Number;
						SC_MustGetNumber();
						if (ignore == false)
						{
							mod = sc_Number - base + 1;
							FrameDefs[fd].tics = (base << 16) + (mod << 8);
							fd++;
						}
					}
					else
					{
						SC_ScriptError(NULL);
					}
				}
				else
				{
					SC_UnGet();
					done = true;
				}
			}
			else
			{
				done = true;
			}
		}
		if ((ignore == false) && (fd - ad->startFrameDef < 2))
		{
			Sys_Error("P_InitFTAnims: AnimDef has framecount < 2.");
		}
		if (ignore == false)
		{
			ad->endFrameDef = fd-1;
			ad->currentFrameDef = ad->endFrameDef;
			ad->tics = 1; // Force 1st game tic to animate
			AnimDefCount++;
			ad++;
		}
	}
	SC_Close();
}

//==========================================================================
//
//	R_AnimateSurfaces
//
//==========================================================================

#ifdef CLIENT
void R_AnimateSurfaces(void)
{
	int i;
	animDef_t *ad;

	//	Animate flats and textures
	for(i = 0; i < AnimDefCount; i++)
	{
		ad = &AnimDefs[i];
		ad->tics--;
		if (ad->tics == 0)
		{
			if (ad->currentFrameDef == ad->endFrameDef)
			{
				ad->currentFrameDef = ad->startFrameDef;
			}
			else
			{
				ad->currentFrameDef++;
			}
			ad->tics = FrameDefs[ad->currentFrameDef].tics;
			if (ad->tics > 255)
			{ // Random tics
				ad->tics = (ad->tics>>16)
					+ rand() % ((ad->tics & 0xff00) >> 8);
			}
			if (ad->type == ANIM_FLAT)
			{
				flattranslation[ad->index] =
					FrameDefs[ad->currentFrameDef].index;
			}
			else
			{ // Texture
				texturetranslation[ad->index] =
					FrameDefs[ad->currentFrameDef].index;
			}
		}
	}

	R_AnimateSky();
}
#endif

//==========================================================================
//
//	R_InitTexture
//
//==========================================================================

void R_InitTexture(void)
{
	if (Game == Strife)
		InitTextures<maptexture_strife_t>();
	else
		InitTextures<maptexture_t>();
	InitFlats();
	InitSpriteLumps();
	InitTranslationTables();
	InitFTAnims(); // Init flat and texture animations

    skyflatnum = R_CheckFlatNumForName("F_SKY");
	if (skyflatnum < 0)
	    skyflatnum = R_CheckFlatNumForName("F_SKY001");
	if (skyflatnum < 0)
	    skyflatnum = R_FlatNumForName("F_SKY1");
#ifdef SERVER
	svpr.SetGlobal("skyflatnum", skyflatnum);
#endif
}

#ifndef CLIENT

//==========================================================================
//
//  R_Init
//
//==========================================================================

void R_Init(void)
{
	R_InitTexture();
}

#else

//==========================================================================
//
// 	R_PrecacheLevel
//
// 	Preloads all relevant graphics for the level.
//
//==========================================================================

void R_PrecacheLevel(void)
{
	int			i;
    
	if (cls.demoplayback)
		return;
    
#ifdef __GNUC__
	char flatpresent[numflats];
	char texturepresent[numtextures];
#else
	char* flatpresent = (char*)Z_StrMalloc(numflats);
	char* texturepresent = (char*)Z_StrMalloc(numtextures);
#endif
	memset(flatpresent, 0, numflats);
	memset(texturepresent, 0, numtextures);

#define MARK(tex) \
if (tex & TEXF_FLAT)\
{\
	flatpresent[tex & ~TEXF_FLAT] = true;\
}\
else\
{\
	texturepresent[tex] = true;\
}
	for (i = 0; i < cl_level.numsectors; i++)
	{
		MARK(cl_level.sectors[i].floor.pic)
		MARK(cl_level.sectors[i].ceiling.pic)
	}
	
	for (i = 0; i < cl_level.numsides; i++)
	{
		MARK(cl_level.sides[i].toptexture)
		MARK(cl_level.sides[i].midtexture)
		MARK(cl_level.sides[i].bottomtexture)
	}

	// Precache flats.
	for (i = 0; i < numflats; i++)
	{
		if (flatpresent[i])
		{
			Drawer->SetFlat(i);
		}
	}

	// Precache textures.
	for (i = 0; i < numtextures; i++)
	{
		if (texturepresent[i])
		{
			Drawer->SetTexture(i);
		}
	}

#ifndef __GNUC__
	Z_Free(flatpresent);
	Z_Free(texturepresent);
#endif
}

//==========================================================================
//
//	R_RegisterPic
//
//==========================================================================

int R_RegisterPic(const char *name, int type)
{
	for (int i = 0; i < MAX_PICS; i++)
	{
		if (!pic_list[i].name[0])
		{
//			cond << "Registering pic " << i << " " << name << endl;
			strcpy(pic_list[i].name, name);
			pic_list[i].type = type;
			return i;
		}
		if (!stricmp(pic_list[i].name, name))
		{
			return i;
		}
	}
	cond << "R_RegisterPic: No more free slots\n";
	return -1;
}

//==========================================================================
//
//	R_GetPicInfo
//
//==========================================================================

void R_GetPicInfo(int handle, picinfo_t *info)
{
	if (handle < 0)
	{
		memset(info, 0, sizeof(*info));
	}
	else
	{
		patch_t *patch;

		switch (pic_list[handle].type)
	 	{
 		 case PIC_PATCH:
			patch = (patch_t*)W_CacheLumpName(pic_list[handle].name, PU_CACHE);
			info->width = LittleShort(patch->width);
			info->height = LittleShort(patch->height);
			info->xoffset = LittleShort(patch->leftoffset);
			info->yoffset = LittleShort(patch->topoffset);
			break;

		 case PIC_RAW:
			info->width = 320;
			info->height = 200;
			info->xoffset = 0;
			info->yoffset = 0;
			break;
		}
	}
}

//==========================================================================
//
//	R_DrawPic
//
//==========================================================================

void R_DrawPic(int x, int y, int handle, int trans)
{
	picinfo_t	info;

	if (handle < 0)
	{
		return;
	}

	R_GetPicInfo(handle, &info);
	x -= info.xoffset;
	y -= info.yoffset;
	Drawer->DrawPic(fScaleX * x, fScaleY * y,
		fScaleX * (x + info.width), fScaleY * (y + info.height),
		0, 0, info.width, info.height, handle, trans);
}

//==========================================================================
//
//	R_DrawShadowedPic
//
//==========================================================================

void R_DrawShadowedPic(int x, int y, int handle)
{
	picinfo_t	info;

	if (handle < 0)
	{
		return;
	}

	R_GetPicInfo(handle, &info);
	x -= info.xoffset;
	y -= info.yoffset;
	Drawer->DrawPicShadow(fScaleX * (x + 2), fScaleY * (y + 2),
		fScaleX * (x + 2 + info.width), fScaleY * (y + 2 + info.height),
		0, 0, info.width, info.height, handle, 160);
	Drawer->DrawPic(fScaleX * x, fScaleY * y,
		fScaleX * (x + info.width), fScaleY * (y + info.height),
		0, 0, info.width, info.height, handle, 0);
}
#endif

//**************************************************************************
//
//	$Log$
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
