//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	PIC_PATCH,
	PIC_RAW
};

struct picinfo_t
{
	int		width;
	int		height;
	int		xoffset;
	int		yoffset;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// R_Main
subsector_t* SV_PointInSubsector(float x, float y);
subsector_t* CL_PointInSubsector(float x, float y);
void R_SetViewSize(int blocks);
void R_RenderPlayerView(void);
void R_Init(void); // Called by startup code.
void R_Start(const mapInfo_t &info);
void R_PreRender(void);
void R_SegMoved(seg_t *seg);

// R_Data
int R_TextureNumForName(const char *name);
int R_CheckTextureNumForName(const char *name);
int R_FlatNumForName(const char* name);
int R_CheckFlatNumForName(const char* name);

// R_Border
void R_FillRectWithFlat(int DestX,int DestY,int width,int height,const char* fname);
void R_DrawBorder(int x,int y,int w,int h);

// R_Things
void R_DrawSpritePatch(int x, int y, int sprite, int frame, int rot, int = 0);
void R_InitSprites(void);

// R_Anim
void R_AnimateSurfaces(void);

// R_Sky
void R_ForceLightning(void);

//	2D graphics
int R_RegisterPic(const char *name, int type);
void R_GetPicInfo(int handle, picinfo_t *info);
void R_DrawPic(int x, int y, int handle, int trans = 0);
void R_DrawShadowedPic(int x, int y, int handle);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int				validcount;

extern int				numflats;

extern int				skyflatnum;

