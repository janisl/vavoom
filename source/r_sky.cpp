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

struct skysurface_t : surface_t
{
	TVec			__verts[3];
};

struct sky_t
{
	int 			texture1;
	int 			texture2;
	int 			baseTexture1;
	int 			baseTexture2;
	float			columnOffset1;
	float			columnOffset2;
	float			scrollDelta1;
	float			scrollDelta2;
	skysurface_t	surf;
	TPlane			plane;
	texinfo_t		texinfo;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void R_LightningFlash(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static boolean		LevelHasLightning;
static int			NextLightningFlash;
static int			LightningFlash;
static int			*LightningLightLevels;

static sky_t		sky[6];

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	R_InitSkyBoxes
//
//==========================================================================

void R_InitSkyBoxes(void)
{
	SC_Open("skyboxes");

	SC_Close();
}

//==========================================================================
//
//	R_InitOldSky
//
//==========================================================================

static void R_InitOldSky(const mapInfo_t &info)
{
	memset(sky, 0, sizeof(sky));

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

	float skyheight = textures[info.sky1Texture]->height;
	float skytop;
	float skybot;

	if (skyheight <= 128.0)
	{
		skytop = 95;
	}
	else
	{
		skytop = 190;
	}
	skybot = skytop - skyheight;

	sky[0].surf.verts[0] = TVec(128, 128, skybot);
	sky[0].surf.verts[1] = TVec(128, 128, skytop);
	sky[0].surf.verts[2] = TVec(128, -128, skytop);
	sky[0].surf.verts[3] = TVec(128, -128, skybot);

	sky[1].surf.verts[0] = TVec(128, -128, skybot);
	sky[1].surf.verts[1] = TVec(128, -128, skytop);
	sky[1].surf.verts[2] = TVec(-128, -128, skytop);
	sky[1].surf.verts[3] = TVec(-128, -128, skybot);

	sky[2].surf.verts[0] = TVec(-128, -128, skybot);
	sky[2].surf.verts[1] = TVec(-128, -128, skytop);
	sky[2].surf.verts[2] = TVec(-128, 128, skytop);
	sky[2].surf.verts[3] = TVec(-128, 128, skybot);

	sky[3].surf.verts[0] = TVec(-128, 128, skybot);
	sky[3].surf.verts[1] = TVec(-128, 128, skytop);
	sky[3].surf.verts[2] = TVec(128, 128, skytop);
	sky[3].surf.verts[3] = TVec(128, 128, skybot);

	sky[4].surf.verts[0] = TVec(128.0, 128.0, skytop);
	sky[4].surf.verts[1] = TVec(-128.0, 128.0, skytop);
	sky[4].surf.verts[2] = TVec(-128.0, -128.0, skytop);
	sky[4].surf.verts[3] = TVec(128.0, -128.0, skytop);

	sky[5].surf.verts[0] = TVec(128, 128, skybot);
	sky[5].surf.verts[1] = TVec(128, -128, skybot);
	sky[5].surf.verts[2] = TVec(-128, -128, skybot);
	sky[5].surf.verts[3] = TVec(-128, 128, skybot);

	sky[0].plane.Set(TVec(-1, 0, 0), -128);
	sky[0].texinfo.saxis = TVec(0, -1.0, 0);
	sky[0].texinfo.taxis = TVec(0, 0, -1.0);
	sky[0].texinfo.texorg = TVec(128, 128, skytop);

	sky[1].plane.Set(TVec(0, 1, 0), -128);
	sky[1].texinfo.saxis = TVec(-1.0, 0, 0);
	sky[1].texinfo.taxis = TVec(0, 0, -1.0);
	sky[1].texinfo.texorg = TVec(128 + 256, -128, skytop);

	sky[2].plane.Set(TVec(1, 0, 0), -128);
	sky[2].texinfo.saxis = TVec(0, 1.0, 0);
	sky[2].texinfo.taxis = TVec(0, 0, -1.0);
	sky[2].texinfo.texorg = TVec(-128, -128 - 512, skytop);

	sky[3].plane.Set(TVec(0, -1, 0), -128);
	sky[3].texinfo.saxis = TVec(1.0, 0, 0);
	sky[3].texinfo.taxis = TVec(0, 0, -1.0);
	sky[3].texinfo.texorg = TVec(-128 - 768, 128, skytop);

	sky[4].plane.Set(TVec(0, 0, -1), -skytop);
	sky[4].texinfo.saxis = TVec(0, -1.0, 0);
	sky[4].texinfo.taxis = TVec(1.0, 0, 0);
	sky[4].texinfo.texorg = TVec(-128, 128, skytop);

	sky[5].plane.Set(TVec(0, 0, 1), skybot);
	sky[5].texinfo.saxis = TVec(0, -1.0, 0);
	sky[5].texinfo.taxis = TVec(1.0, 0, 0);
	sky[5].texinfo.texorg = TVec(-128, 128, skybot);

	for (int j = 0; j < 6; j++)
	{
		sky[j].baseTexture1 = info.sky1Texture;
		sky[j].baseTexture2 = info.sky2Texture;
		if (info.doubleSky)
		{
			sky[j].texture1 = sky[j].baseTexture2;
			sky[j].texture2 = sky[j].baseTexture1;
			sky[j].scrollDelta1 = info.sky2ScrollDelta;
			sky[j].scrollDelta2 = info.sky1ScrollDelta;
		}
		else
		{
			sky[j].texture1 = sky[j].baseTexture1;
			sky[j].scrollDelta1 = info.sky1ScrollDelta;
		}
		sky[j].surf.plane = &sky[j].plane;
		sky[j].surf.texinfo = &sky[j].texinfo;
		sky[j].surf.count = 4;
	}

	sky[4].texinfo.taxis *= skyheight / 256.0;
	sky[5].texinfo.taxis *= skyheight / 256.0;

	//	Precache textures
	Drawer->SetSkyTexture(info.sky1Texture, info.doubleSky);
	Drawer->SetSkyTexture(info.sky2Texture, false);
}

//==========================================================================
//
//	R_InitSky
//
//	Called at level load.
//
//==========================================================================

void R_InitSky(const mapInfo_t &info)
{
	if (info.skybox[0])
	{
	}
	else
	{
		R_InitOldSky(info);
	}
}

//==========================================================================
//
//	R_AnimateSky
//
//==========================================================================

void R_AnimateSky(void)
{
	//	Update sky column offsets
	for (int i = 0; i < 6; i++)
	{
		sky[i].columnOffset1 += sky[i].scrollDelta1 * host_frametime;
		sky[i].columnOffset2 += sky[i].scrollDelta2 * host_frametime;
	}

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
			for (i = 0; i < 6; i++)
			{
				sky[i].texture1 = sky[i].baseTexture1;
			}
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
		for (i = 0; i < 6; i++)
		{
			sky[i].texture1 = sky[i].baseTexture2; // set alternate sky
		}
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
//	R_DrawSky
//
//==========================================================================

void R_DrawSky(void)
{
	Drawer->BeginSky();

	for (int i = 0; i < 6; i++)
	{
		r_saxis = sky[i].texinfo.saxis;
		r_taxis = sky[i].texinfo.taxis;
		r_texorg = sky[i].texinfo.texorg;

		r_normal = sky[i].plane.normal;
		r_dist = sky[i].plane.dist;

		sky[i].surf.lightlevel = 255;

		r_surface = &sky[i].surf;
		Drawer->DrawSkyPolygon(sky[i].surf.verts, 4, sky[i].texture1,
			sky[i].columnOffset1, sky[i].texture2, sky[i].columnOffset2);
	}

	Drawer->EndSky();
}

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2001/10/12 17:31:13  dj_jl
//	no message
//
//	Revision 1.4  2001/10/09 17:21:39  dj_jl
//	Added sky begining and ending functions
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
