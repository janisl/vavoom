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

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum
{
	PIC_PATCH,	// A standard Doom patch
	PIC_RAW,	// Raw screens in Heretic and Hexen
	PIC_IMAGE	// External graphic image file
};

struct picinfo_t
{
	int		width;
	int		height;
	int		xoffset;
	int		yoffset;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// r_main
void R_Init(); // Called by startup code.
void R_Start();
void R_SetViewSize(int blocks);
void R_RenderPlayerView();

// r_tex
void R_InitTexture();
int R_TextureNumForName(const char *name);
int R_CheckTextureNumForName(const char *name);
int R_FlatNumForName(const char* name);
int R_CheckFlatNumForName(const char* name);
float R_TextureHeight(int pic);
void R_AnimateSurfaces();

// r_surf
void R_PreRender();
void R_SegMoved(seg_t *seg);

// r_things
void R_DrawSpritePatch(int x, int y, int sprite, int frame, int rot, int = 0);
void R_InitSprites();

// r_sky
void R_ForceLightning();
void R_SkyChanged();

//	2D graphics
int R_RegisterPic(const char *name, int type);
int R_RegisterPicPal(const char *name, int type, const char*);
void R_GetPicInfo(int handle, picinfo_t *info);
void R_DrawPic(int x, int y, int handle, int trans = 0);
void R_DrawShadowedPic(int x, int y, int handle);
void R_FillRectWithFlat(int DestX,int DestY,int width,int height,const char* fname);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern int				validcount;

extern int				skyflatnum;

//**************************************************************************
//
//	$Log$
//	Revision 1.7  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//
//	Revision 1.6  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.5  2001/11/09 14:25:15  dj_jl
//	Cleaned up
//	
//	Revision 1.4  2001/08/23 17:47:22  dj_jl
//	Started work on pics with custom palettes
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
