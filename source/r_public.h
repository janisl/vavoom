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

struct particle_t;
struct dlight_t;
class VTexture;

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
	TEXTYPE_Autopage,
	TEXTYPE_Null,
};

struct picinfo_t
{
	vint32		width;
	vint32		height;
	vint32		xoffset;
	vint32		yoffset;
};

struct TSwitchFrame
{
	vint16		Texture;
	vint16		BaseTime;
	vint16		RandomRange;
};

struct TSwitch
{
	vint16			Tex;
	vint16			PairIndex;
	vint16			Sound;
	vint16			NumFrames;
	TSwitchFrame*	Frames;
	bool			Quest;

	TSwitch()
	: Frames(NULL)
	{}
	~TSwitch()
	{
		if (Frames)
			delete[] Frames;
	}
};

struct VAnimDoorDef
{
	vint32		Texture;
	VName		OpenSound;
	VName		CloseSound;
	vint32		NumFrames;
	vint32*		Frames;
};

class VRenderLevelPublic : public VVirtualObjectBase
{
public:
	virtual void PreRender() = 0;
	virtual void SegMoved(seg_t*) = 0;
	virtual void SetupFakeFloors(sector_t*) = 0;

	virtual void ForceLightning() = 0;

	virtual void AddStaticLight(const TVec&, float, vuint32) = 0;
	virtual dlight_t* AllocDlight(VThinker*) = 0;
	virtual void DecayLights(float) = 0;

	virtual particle_t* NewParticle() = 0;
};

class VTextureManager
{
public:
	TArray<VTexture*>	Textures;
	vint32				DefaultTexture;
	float				Time;	//	Time value for warp textures

	VTextureManager();
	void Init();
	void Shutdown();
	int AddTexture(VTexture* Tex);
	int	CheckNumForName(VName Name, int Type, bool bOverload = false,
		bool bCheckAny = false);
	int	NumForName(VName Name, int Type, bool bOverload = false,
		bool bCheckAny = false);
	VName GetTextureName(int TexNum);
	float TextureHeight(int TexNum);
	int TextureAnimation(int InTex);
	void SetFrontSkyLayer(int tex);
	void GetTextureInfo(int TexNum, picinfo_t* info);
	int AddPatch(VName Name, int Type, bool Silent = false);
	int AddRawWithPal(VName Name, VName PalName);
	int AddFileTexture(VName Name, int Type);

private:
	void AddTextures();
	void AddTexturesLump(int, int, int, bool);
	void AddGroup(int, EWadNamespace);
	void AddHiResTextures();
};

// r_main
void R_Init(); // Called by startup code.
void R_Start(VLevel*);
void R_SetViewSize(int blocks);
void R_RenderPlayerView();

// r_tex
void R_InitTexture();
void R_ShutdownTexture();
VAnimDoorDef* R_FindAnimDoor(vint32);
void R_AnimateSurfaces();

// r_things
void R_DrawSpritePatch(int x, int y, int sprite, int frame, int rot, int = 0);
void R_InitSprites();

//	2D graphics
void R_DrawPic(int x, int y, int handle, float Aplha = 1.0);
void R_DrawShadowedPic(int x, int y, int handle);
void R_FillRectWithFlat(int DestX,int DestY,int width,int height,const char* fname);

extern VTextureManager	GTextureManager;

extern int				validcount;

extern int				skyflatnum;

//	Switches
extern TArray<TSwitch*>	Switches;
