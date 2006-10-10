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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	PIC_PATCH,	// A standard Doom patch
	PIC_RAW,	// Raw screens in Heretic and Hexen
	PIC_IMAGE	// External graphic image file
};

enum
{
	TEXTYPE_Any,
	TEXTYPE_WallPatch,
	TEXTYPE_Wall,
	TEXTYPE_Flat,
	TEXTYPE_Overload,
	TEXTYPE_Sprite,
	TEXTYPE_SkyMap,
	TEXTYPE_Skin,
	TEXTYPE_Pic,
};

struct picinfo_t
{
	int		width;
	int		height;
	int		xoffset;
	int		yoffset;
};

struct TSwitch
{
	int Tex1;
	int Tex2;
	int Sound;
};

struct VAnimDoorDef
{
	vint32		Texture;
	VName		OpenSound;
	VName		CloseSound;
	vint32		NumFrames;
	vint32*		Frames;
};

class VTexture;

class VTextureManager
{
public:
	TArray<VTexture*>	Textures;
	vint32				DefaultTexture;

	VTextureManager();
	void Init();
	void Shutdown();
	int AddTexture(VTexture* Tex);
	int	CheckNumForName(VName Name, int Type, bool bOverload = false,
		bool bCheckAny = false);
	int	NumForName(VName Name, int Type, bool bOverload = false,
		bool bCheckAny = false);
	float TextureHeight(int TexNum);
	int TextureAnimation(int InTex);
	void SetFrontSkyLayer(int tex);
	void GetTextureInfo(int TexNum, picinfo_t* info);
	int AddPatch(VName Name, int Type);
	int CreatePatch(int Type, int LumpNum);
	int AddRawWithPal(VName Name, VName PalName);
	int AddFileTexture(VName Name, int Type);
	vuint8* GetRgbTable();

private:
	vuint8*		RgbTable;

	void InitTextures();
	void InitFlats();
	void InitOverloads();
	void InitSpriteLumps();
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// r_main
void R_Init(); // Called by startup code.
void R_Start();
void R_SetViewSize(int blocks);
void R_RenderPlayerView();

// r_tex
void R_InitTexture();
void R_ShutdownTexture();
VAnimDoorDef* R_FindAnimDoor(vint32);
void R_AnimateSurfaces();

// r_surf
void R_PreRender();
void R_SegMoved(seg_t *seg);
void R_SetupFakeFloors(sector_t* Sec);
void R_FreeLevelData();

// r_things
void R_DrawSpritePatch(int x, int y, int sprite, int frame, int rot, int = 0);
void R_InitSprites();

// r_sky
void R_ForceLightning();
void R_SkyChanged();

//	2D graphics
void R_DrawPic(int x, int y, int handle, int trans = 0);
void R_DrawShadowedPic(int x, int y, int handle);
void R_FillRectWithFlat(int DestX,int DestY,int width,int height,const char* fname);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern VTextureManager	GTextureManager;

extern int				validcount;

extern int				skyflatnum;

//	Switches
extern TArray<TSwitch>	Switches;
