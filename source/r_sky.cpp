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
//**	Sky rendering. The DOOM sky is a texture map like any wall, wrapping
//**  around. A 1024 columns equal 360 degrees. The default sky map is 256
//**  columns and repeats 4 times on a 320 screen
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

#define LIGHTNING_SPECIAL 	198
#define LIGHTNING_SPECIAL2 	199
#define SKYCHANGE_SPECIAL 	200

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void R_LightningFlash(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int 			Sky1BaseTexture;
static int 			Sky2BaseTexture;
static int 			Sky1TopBaseTexture;
static int 			Sky2TopBaseTexture;
static int 			Sky1BotBaseTexture;
static int 			Sky2BotBaseTexture;

static int 			Sky1Texture;
static int 			Sky2Texture;
static int 			Sky1TopTexture;
static int 			Sky2TopTexture;
static int 			Sky1BotTexture;
static int 			Sky2BotTexture;

static float		Sky1ColumnOffset;
static float		Sky2ColumnOffset;
static float		Sky1ScrollDelta;
static float		Sky2ScrollDelta;

static float		Sky1TopXOffset;
static float		Sky1TopYOffset;
static float		Sky2TopXOffset;
static float		Sky2TopYOffset;
static float		Sky1TopXDelta;
static float		Sky1TopYDelta;
static float		Sky2TopXDelta;
static float		Sky2TopYDelta;

static boolean 		DoubleSky;

static boolean		LevelHasLightning;
static int			NextLightningFlash;
static int			LightningFlash;
static int			*LightningLightLevels;

static float		skytop;
static float		skybot;

struct skysurface_t : surface_t
{
	TVec			__verts[3];
};

static skysurface_t		skysurf[6];
static TPlane			skyplane[6];
static texinfo_t		skytexinfo[6];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	R_InitSky
//
//	Called at level load.
//
//==========================================================================

void R_InitSky(const mapInfo_t &info)
{
	Sky1Texture = Sky1BaseTexture = info.sky1Texture;
	Sky2Texture = Sky2BaseTexture = info.sky2Texture;
	Sky1ScrollDelta = info.sky1ScrollDelta;
	Sky2ScrollDelta = info.sky2ScrollDelta;
	Sky1ColumnOffset = 0.0;
	Sky2ColumnOffset = 0.0;
	DoubleSky =	info.doubleSky;

	Sky1TopTexture = Sky1TopBaseTexture = Sky1Texture;
	Sky2TopTexture = Sky2TopBaseTexture = Sky2Texture;
	Sky1BotTexture = Sky1BotBaseTexture = Sky1Texture;
	Sky2BotTexture = Sky2BotBaseTexture = Sky2Texture;
	Sky1TopXOffset = 0;
	Sky1TopYOffset = 0;
	Sky2TopXOffset = 0;
	Sky2TopYOffset = 0;
	Sky1TopXDelta = 32;
	Sky1TopYDelta = 8;
	Sky2TopXDelta = 32;
	Sky2TopYDelta = 8;

	// Check if the level is a lightning level
	LevelHasLightning = false;
	LightningFlash = 0;
	if (info.lightning)
	{
		int secCount = 0;
		for (int i = 0; i < cl_level.numsectors; i++)
		{
			if (cl_level.sectors[i].ceiling.pic == skyflatnum ||
				cl_level.sectors[i].special == LIGHTNING_SPECIAL ||
				cl_level.sectors[i].special == LIGHTNING_SPECIAL2)
			{
				secCount++;
			}
		}
		if (secCount)
		{
			LevelHasLightning = true;
			LightningLightLevels = (int *)Z_Malloc(secCount * sizeof(int),
				PU_LEVEL, NULL);
			NextLightningFlash = ((rand() & 15) + 5) * 35; // don't flash at level start
		}
	}

	float skyheight = textures[Sky1Texture]->height;
	if (skyheight <= 128.0)
	{
		skytop = 95;
	}
	else
	{
		skytop = 190;
	}
	skybot = skytop - skyheight;

	memset(skysurf, 0, sizeof(skysurf));

	skysurf[0].verts[0] = TVec(128, 128, skybot);
	skysurf[0].verts[1] = TVec(128, 128, skytop);
	skysurf[0].verts[2] = TVec(128, -128, skytop);
	skysurf[0].verts[3] = TVec(128, -128, skybot);

	skysurf[1].verts[0] = TVec(128, -128, skybot);
	skysurf[1].verts[1] = TVec(128, -128, skytop);
	skysurf[1].verts[2] = TVec(-128, -128, skytop);
	skysurf[1].verts[3] = TVec(-128, -128, skybot);

	skysurf[2].verts[0] = TVec(-128, -128, skybot);
	skysurf[2].verts[1] = TVec(-128, -128, skytop);
	skysurf[2].verts[2] = TVec(-128, 128, skytop);
	skysurf[2].verts[3] = TVec(-128, 128, skybot);

	skysurf[3].verts[0] = TVec(-128, 128, skybot);
	skysurf[3].verts[1] = TVec(-128, 128, skytop);
	skysurf[3].verts[2] = TVec(128, 128, skytop);
	skysurf[3].verts[3] = TVec(128, 128, skybot);

	skysurf[4].verts[0] = TVec(128.0, 128.0, skytop);
	skysurf[4].verts[1] = TVec(-128.0, 128.0, skytop);
	skysurf[4].verts[2] = TVec(-128.0, -128.0, skytop);
	skysurf[4].verts[3] = TVec(128.0, -128.0, skytop);

	skysurf[5].verts[0] = TVec(128, 128, skybot);
	skysurf[5].verts[1] = TVec(128, -128, skybot);
	skysurf[5].verts[2] = TVec(-128, -128, skybot);
	skysurf[5].verts[3] = TVec(-128, 128, skybot);

	skyplane[0].Set(TVec(-1, 0, 0), -128);
	skytexinfo[0].saxis = TVec(0, -1.0, 0);
	skytexinfo[0].taxis = TVec(0, 0, -1.0);
	skytexinfo[0].texorg = TVec(128, 128, skytop);

	skyplane[1].Set(TVec(0, 1, 0), -128);
	skytexinfo[1].saxis = TVec(-1.0, 0, 0);
	skytexinfo[1].taxis = TVec(0, 0, -1.0);
	skytexinfo[1].texorg = TVec(128 + 256, -128, skytop);

	skyplane[2].Set(TVec(1, 0, 0), -128);
	skytexinfo[2].saxis = TVec(0, 1.0, 0);
	skytexinfo[2].taxis = TVec(0, 0, -1.0);
	skytexinfo[2].texorg = TVec(-128, -128 - 512, skytop);

	skyplane[3].Set(TVec(0, -1, 0), -128);
	skytexinfo[3].saxis = TVec(1.0, 0, 0);
	skytexinfo[3].taxis = TVec(0, 0, -1.0);
	skytexinfo[3].texorg = TVec(-128 - 768, 128, skytop);

	skyplane[4].Set(TVec(0, 0, -1), -skytop);
	skytexinfo[4].saxis = TVec(0, -1.0, 0);
	skytexinfo[4].taxis = TVec(1.0, 0, 0);
	skytexinfo[4].texorg = TVec(-128, 128, skytop);

	skyplane[5].Set(TVec(0, 0, 1), skybot);
	skytexinfo[5].saxis = TVec(0, -1.0, 0);
	skytexinfo[5].taxis = TVec(1.0, 0, 0);
	skytexinfo[5].texorg = TVec(-128, 128, skybot);

	for (int j = 0; j < 6; j++)
	{
		skysurf[j].plane = &skyplane[j];
		skysurf[j].texinfo = &skytexinfo[j];
		skysurf[j].count = 4;
	}

	//	Precache textures
	Drawer->SetSkyTexture(Sky1Texture, DoubleSky);
	Drawer->SetSkyTexture(Sky2Texture, false);
	Drawer->SetSkyTexture(Sky1TopTexture, DoubleSky);
	Drawer->SetSkyTexture(Sky2TopTexture, false);
	Drawer->SetSkyTexture(Sky1BotTexture, DoubleSky);
	Drawer->SetSkyTexture(Sky2BotTexture, false);
}

//==========================================================================
//
//	R_AnimateSky
//
//==========================================================================

void R_AnimateSky(void)
{
	//	Update sky column offsets
	Sky1ColumnOffset += Sky1ScrollDelta * host_frametime;
	Sky2ColumnOffset += Sky2ScrollDelta * host_frametime;
	Sky1TopXOffset += Sky1TopXDelta * host_frametime;
	Sky1TopYOffset += Sky1TopYDelta * host_frametime;
	Sky2TopXOffset += Sky2TopXDelta * host_frametime;
	Sky2TopYOffset += Sky2TopYDelta * host_frametime;

	//	Update lightning
	if (LevelHasLightning)
	{
		if (!NextLightningFlash || LightningFlash)
		{
			R_LightningFlash();
		}
		else
		{
			NextLightningFlash--;
		}
	}
}

//==========================================================================
//
//	R_LightningFlash
//
//==========================================================================

static void R_LightningFlash(void)
{
	int 		i;
	sector_t 	*tempSec;
	int 		*tempLight;
	boolean 	foundSec;
	int 		flashLight;

	if (LightningFlash)
	{
		LightningFlash--;
		if (LightningFlash)
		{
			tempLight = LightningLightLevels;
			tempSec = cl_level.sectors;
			for (i = 0; i < cl_level.numsectors; i++, tempSec++)
			{
				if (tempSec->ceiling.pic == skyflatnum ||
					tempSec->special == LIGHTNING_SPECIAL ||
					tempSec->special == LIGHTNING_SPECIAL2)
				{
					if (*tempLight < tempSec->params.lightlevel - 4)
					{
						tempSec->params.lightlevel -= 4;
					}
					tempLight++;
				}
			}
		}					
		else
		{ // remove the alternate lightning flash special
			tempLight = LightningLightLevels;
			tempSec = cl_level.sectors;
			for (i = 0; i < cl_level.numsectors; i++, tempSec++)
			{
				if (tempSec->ceiling.pic == skyflatnum
					|| tempSec->special == LIGHTNING_SPECIAL
					|| tempSec->special == LIGHTNING_SPECIAL2)
				{
					tempSec->params.lightlevel = *tempLight;
					tempLight++;
				}
			}
			Sky1Texture = Sky1BaseTexture;
			Sky1TopTexture = Sky1TopBaseTexture;
			Sky1BotTexture = Sky1BotBaseTexture;
		}
		return;
	}

	LightningFlash = (rand() & 7) + 8;
	flashLight = 200 + (rand() & 31);
	tempSec = cl_level.sectors;
	tempLight = LightningLightLevels;
	foundSec = false;
	for (i = 0; i < cl_level.numsectors; i++, tempSec++)
	{
		if (tempSec->ceiling.pic == skyflatnum ||
			tempSec->special == LIGHTNING_SPECIAL ||
			tempSec->special == LIGHTNING_SPECIAL2)
		{
			*tempLight = tempSec->params.lightlevel;
			if (tempSec->special == LIGHTNING_SPECIAL)
			{ 
				tempSec->params.lightlevel += 64;
				if (tempSec->params.lightlevel > flashLight)
				{
					tempSec->params.lightlevel = flashLight;
				}
			}
			else if (tempSec->special == LIGHTNING_SPECIAL2)
			{
				tempSec->params.lightlevel += 32;
				if (tempSec->params.lightlevel > flashLight)
				{
					tempSec->params.lightlevel = flashLight;
				}
			}
			else
			{
				tempSec->params.lightlevel = flashLight;
			}
			if (tempSec->params.lightlevel < *tempLight)
			{
				tempSec->params.lightlevel = *tempLight;
			}
			tempLight++;
			foundSec = true;
		}
	}
	if (foundSec)
	{
		Sky1Texture = Sky2BaseTexture; // set alternate sky
		Sky1TopTexture = Sky2TopBaseTexture; // set alternate sky
		Sky1BotTexture = Sky2BotBaseTexture; // set alternate sky
		S_StartSoundName("ThunderCrash");
	}
	// Calculate the next lighting flash
	if (!NextLightningFlash)
	{
		if ((rand() & 0xff) < 50)
		{
			// Immediate Quick flash
			NextLightningFlash = (rand() & 15) + 16;
		}
		else
		{
			if ((rand() & 0xff) < 128 && !(cl_level.tictime & 32))
			{
				NextLightningFlash = ((rand() & 7) + 2) * 35;
			}
			else
			{
				NextLightningFlash = ((rand() & 15) + 5) * 35;
			}
		}
	}
}

//==========================================================================
//
//	R_ForceLightning
//
//==========================================================================

void R_ForceLightning(void)
{
	NextLightningFlash = 0;
}

//==========================================================================
//
//	R_DrawSkyBox
//
//==========================================================================

void R_DrawSkyBox(void)
{
	for (int i = 0; i < 6; i++)
	{
		r_saxis = skytexinfo[i].saxis;
		r_taxis = skytexinfo[i].taxis;
		r_texorg = skytexinfo[i].texorg;

		r_normal = skyplane[i].normal;
		r_dist = skyplane[i].dist;

		skysurf[i].lightlevel = 255;

		r_surface = &skysurf[i];
		if (DoubleSky)
		{
			Drawer->DrawSkyPolygon(skysurf[i].verts, 4, Sky2Texture,
				Sky2ColumnOffset, Sky1Texture, Sky1ColumnOffset);
		}
		else
		{
			Drawer->DrawSkyPolygon(skysurf[i].verts, 4,
				Sky1Texture, Sky1ColumnOffset, 0, 0);
		}
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
