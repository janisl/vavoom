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
//**	Sky rendering. The DOOM sky is a texture map like any wall, wrapping
//**  around. A 1024 columns equal 360 degrees. The default sky map is 256
//**  columns and repeats 4 times on a 320 screen
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

#define LIGHTNING_OUTDOOR	197
#define LIGHTNING_SPECIAL	198
#define LIGHTNING_SPECIAL2	199
#define SKYCHANGE_SPECIAL	200

#define VDIVS		8
#define HDIVS		16
#define RADIUS		128.0

// TYPES -------------------------------------------------------------------

struct skyboxinfo_t
{
	struct skyboxsurf_t
	{
		int texture;
	};

	char name[128];
	skyboxsurf_t surfs[6];
};

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

skymap_t			*skymaps;
int					numskymaps;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static skyboxinfo_t	*skyboxinfo;
static int			numskyboxes;

static boolean		LevelHasLightning;
static int			NextLightningFlash;
static int			LightningFlash;
static int			*LightningLightLevels;

static sky_t		sky[HDIVS * VDIVS];
static int			NumSkySurfs;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	FindSkyMap
//
//==========================================================================

static int FindSkyMap(const char *name)
{
	guard(FindSkyMap);
	for (int i = 0; i < numskymaps; i++)
	{
		if (!strcmp(skymaps[i].name, name))
		{
			return i | TEXF_SKY_MAP;
		}
	}
	numskymaps++;
	Z_Resize((void **)&skymaps, numskymaps * sizeof(skymap_t));
	strcpy(skymaps[numskymaps - 1].name, name);
	return (numskymaps - 1) | TEXF_SKY_MAP;
	unguard;
}

//==========================================================================
//
//	R_InitSkyBoxes
//
//==========================================================================

void R_InitSkyBoxes()
{
	guard(R_InitSkyBoxes);
	SC_Open("skyboxes");

	skymaps = (skymap_t *)Z_StrMalloc(1);
	numskymaps = 0;
	skyboxinfo = (skyboxinfo_t *)Z_StrMalloc(1);
	numskyboxes = 0;
	while (SC_GetString())
	{
		numskyboxes++;
		Z_Resize((void **)&skyboxinfo, numskyboxes * sizeof(skyboxinfo_t));
		skyboxinfo_t &info = skyboxinfo[numskyboxes - 1];
		memset(&info, 0, sizeof(info));

		strcpy(info.name, sc_String);
		SC_MustGetStringName("{");
		for (int i = 0; i < 6; i++)
		{
			SC_MustGetStringName("{");
			SC_MustGetStringName("map");
			SC_MustGetString();
			info.surfs[i].texture = FindSkyMap(sc_String);
			SC_MustGetStringName("}");
		}
		SC_MustGetStringName("}");
	}

	SC_Close();
	unguard;
}

//==========================================================================
//
//	R_InitOldSky
//
//==========================================================================

static void R_InitOldSky()
{
	guard(R_InitOldSky);
	memset(sky, 0, sizeof(sky));

	// Check if the level is a lightning level
	LevelHasLightning = false;
	LightningFlash = 0;
	if (cl_level.lightning)
	{
		int secCount = 0;
		for (int i = 0; i < GClLevel->NumSectors; i++)
		{
			if (GClLevel->Sectors[i].ceiling.pic == skyflatnum ||
				GClLevel->Sectors[i].special == LIGHTNING_OUTDOOR ||
				GClLevel->Sectors[i].special == LIGHTNING_SPECIAL ||
				GClLevel->Sectors[i].special == LIGHTNING_SPECIAL2)
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

	int skyheight = textures[cl_level.sky1Texture]->height;
	float skytop;
	float skybot;
	int j;

	if (skyheight <= 128)
	{
		skytop = 95;
	}
	else
	{
		skytop = 190;
	}
	skybot = skytop - skyheight;
	int skyh = (int)skytop;

	for (j = 0; j < VDIVS; j++)
	{
		float va0 = 90.0 - j * (180.0 / VDIVS);
		float va1 = 90.0 - (j + 1) * (180.0 / VDIVS);
		float vsina0 = msin(va0);
		float vcosa0 = mcos(va0);
		float vsina1 = msin(va1);
		float vcosa1 = mcos(va1);

//		float theight = skytop;
//		float bheight = skybot;
		float theight = vsina0 * RADIUS;
		float bheight = vsina1 * RADIUS;
//		float tradius = RADIUS;
//		float vradius = RADIUS;
		float tradius = vcosa0 * RADIUS;
		float vradius = vcosa1 * RADIUS;
		for (int i = 0; i < HDIVS; i++)
		{
			sky_t& s = sky[j * HDIVS + i];
			float a0 = 45 - i * (360.0 / HDIVS);
			float a1 = 45 - (i + 1) * (360.0 / HDIVS);
			float sina0 = msin(a0);
			float cosa0 = mcos(a0);
			float sina1 = msin(a1);
			float cosa1 = mcos(a1);

			s.surf.verts[0] = TVec(cosa0 * vradius, sina0 * vradius, bheight);
			s.surf.verts[1] = TVec(cosa0 * tradius, sina0 * tradius, theight);
			s.surf.verts[2] = TVec(cosa1 * tradius, sina1 * tradius, theight);
			s.surf.verts[3] = TVec(cosa1 * vradius, sina1 * vradius, bheight);

			TVec hdir = j < VDIVS / 2 ? s.surf.verts[3] - s.surf.verts[0] :
				s.surf.verts[2] - s.surf.verts[1];
			TVec vdir = s.surf.verts[0] - s.surf.verts[1];
			TVec normal = Normalise(CrossProduct(vdir, hdir));
			s.plane.Set(normal, DotProduct(s.surf.verts[1], normal));

			s.texinfo.saxis = hdir * (1024 / HDIVS / DotProduct(hdir, hdir));
float tk = skyh / RADIUS;
			s.texinfo.taxis = TVec(0, 0, -tk);
			s.texinfo.soffs = -DotProduct(s.surf.verts[j < VDIVS / 2 ? 0 : 1],
				s.texinfo.saxis);
			s.texinfo.toffs = skyh;

			float mins = DotProduct(s.surf.verts[j < VDIVS / 2 ? 0 : 1],
				s.texinfo.saxis) + s.texinfo.soffs;
			float maxs = DotProduct(s.surf.verts[j < VDIVS / 2 ? 3 : 2],
				s.texinfo.saxis) + s.texinfo.soffs;
			int bmins = (int)floor(mins / 16);
			int bmaxs = (int)ceil(maxs / 16);
			s.surf.texturemins[0] = bmins * 16;
			s.surf.extents[0] = (bmaxs - bmins) * 16;
			//s.surf.extents[0] = 256;
			mins = DotProduct(s.surf.verts[1], s.texinfo.taxis) + s.texinfo.toffs;
			maxs = DotProduct(s.surf.verts[0], s.texinfo.taxis) + s.texinfo.toffs;
			bmins = (int)floor(mins / 16);
			bmaxs = (int)ceil(maxs / 16);
			s.surf.texturemins[1] = bmins * 16;
			s.surf.extents[1] = (bmaxs - bmins) * 16;
			//s.surf.extents[1] = skyh;

			s.columnOffset1 = s.columnOffset2 = -i * (1024 / HDIVS);
		}
	}

	NumSkySurfs = VDIVS * HDIVS;

	for (j = 0; j < NumSkySurfs; j++)
	{
		sky[j].baseTexture1 = cl_level.sky1Texture;
		sky[j].baseTexture2 = cl_level.sky2Texture;
		if (cl_level.doubleSky)
		{
			sky[j].texture1 = sky[j].baseTexture2;
			sky[j].texture2 = sky[j].baseTexture1;
			sky[j].scrollDelta1 = cl_level.sky2ScrollDelta;
			sky[j].scrollDelta2 = cl_level.sky1ScrollDelta;
		}
		else
		{
			sky[j].texture1 = sky[j].baseTexture1;
			sky[j].scrollDelta1 = cl_level.sky1ScrollDelta;
		}
		sky[j].surf.plane = &sky[j].plane;
		sky[j].surf.texinfo = &sky[j].texinfo;
		sky[j].surf.count = 4;
	}

	//	Precache textures
	Drawer->SetSkyTexture(cl_level.sky1Texture, cl_level.doubleSky);
	Drawer->SetSkyTexture(cl_level.sky2Texture, false);
	unguard;
}

//==========================================================================
//
//	R_InitSkyBox
//
//==========================================================================

static void R_InitSkyBox()
{
	guard(R_InitSkyBox);
	int num;

	for (num = 0; num < numskyboxes; num++)
	{
		if (!strcmp(cl_level.skybox, skyboxinfo[num].name))
		{
			break;
		}
	}
	if (num == numskyboxes)
	{
		Host_Error("No such skybox %s", cl_level.skybox);
	}
	skyboxinfo_t &sinfo = skyboxinfo[num];

	memset(sky, 0, sizeof(sky));

	LevelHasLightning = false;
	LightningFlash = 0;

	sky[0].surf.verts[0] = TVec(128, 128, -128);
	sky[0].surf.verts[1] = TVec(128, 128, 128);
	sky[0].surf.verts[2] = TVec(128, -128, 128);
	sky[0].surf.verts[3] = TVec(128, -128, -128);

	sky[1].surf.verts[0] = TVec(128, -128, -128);
	sky[1].surf.verts[1] = TVec(128, -128, 128);
	sky[1].surf.verts[2] = TVec(-128, -128, 128);
	sky[1].surf.verts[3] = TVec(-128, -128, -128);

	sky[2].surf.verts[0] = TVec(-128, -128, -128);
	sky[2].surf.verts[1] = TVec(-128, -128, 128);
	sky[2].surf.verts[2] = TVec(-128, 128, 128);
	sky[2].surf.verts[3] = TVec(-128, 128, -128);

	sky[3].surf.verts[0] = TVec(-128, 128, -128);
	sky[3].surf.verts[1] = TVec(-128, 128, 128);
	sky[3].surf.verts[2] = TVec(128, 128, 128);
	sky[3].surf.verts[3] = TVec(128, 128, -128);

	sky[4].surf.verts[0] = TVec(128.0, 128.0, 128);
	sky[4].surf.verts[1] = TVec(-128.0, 128.0, 128);
	sky[4].surf.verts[2] = TVec(-128.0, -128.0, 128);
	sky[4].surf.verts[3] = TVec(128.0, -128.0, 128);

	sky[5].surf.verts[0] = TVec(128, 128, -128);
	sky[5].surf.verts[1] = TVec(128, -128, -128);
	sky[5].surf.verts[2] = TVec(-128, -128, -128);
	sky[5].surf.verts[3] = TVec(-128, 128, -128);

	sky[0].plane.Set(TVec(-1, 0, 0), -128);
	sky[0].texinfo.saxis = TVec(0, -1.0, 0);
	sky[0].texinfo.taxis = TVec(0, 0, -1.0);
	sky[0].texinfo.soffs = 128;
	sky[0].texinfo.toffs = 128;

	sky[1].plane.Set(TVec(0, 1, 0), -128);
	sky[1].texinfo.saxis = TVec(-1.0, 0, 0);
	sky[1].texinfo.taxis = TVec(0, 0, -1.0);
	sky[1].texinfo.soffs = 128;
	sky[1].texinfo.toffs = 128;

	sky[2].plane.Set(TVec(1, 0, 0), -128);
	sky[2].texinfo.saxis = TVec(0, 1.0, 0);
	sky[2].texinfo.taxis = TVec(0, 0, -1.0);
	sky[2].texinfo.soffs = 128;
	sky[2].texinfo.toffs = 128;

	sky[3].plane.Set(TVec(0, -1, 0), -128);
	sky[3].texinfo.saxis = TVec(1.0, 0, 0);
	sky[3].texinfo.taxis = TVec(0, 0, -1.0);
	sky[3].texinfo.soffs = 128;
	sky[3].texinfo.toffs = 128;

	sky[4].plane.Set(TVec(0, 0, -1), -128);
	sky[4].texinfo.saxis = TVec(0, -1.0, 0);
	sky[4].texinfo.taxis = TVec(1.0, 0, 0);
	sky[4].texinfo.soffs = 128;
	sky[4].texinfo.toffs = 128;

	sky[5].plane.Set(TVec(0, 0, 1), -128);
	sky[5].texinfo.saxis = TVec(0, -1.0, 0);
	sky[5].texinfo.taxis = TVec(-1.0, 0, 0);
	sky[5].texinfo.soffs = 128;
	sky[5].texinfo.toffs = 128;

	NumSkySurfs = 6;
	for (int j = 0; j < 6; j++)
	{
		int smap = sky[j].texture1 & ~TEXF_SKY_MAP;

		sky[j].texture1 = sinfo.surfs[j].texture;
		sky[j].surf.plane = &sky[j].plane;
		sky[j].surf.texinfo = &sky[j].texinfo;
		sky[j].surf.count = 4;

		//	Precache texture
		Drawer->SetSkyTexture(sky[j].texture1, false);

		sky[j].surf.extents[0] = skymaps[smap].width;
		sky[j].surf.extents[1] = skymaps[smap].height;
		sky[j].texinfo.saxis *= skymaps[smap].width / 256.0;
		sky[j].texinfo.soffs *= skymaps[smap].width / 256.0;
		sky[j].texinfo.taxis *= skymaps[smap].height / 256.0;
		sky[j].texinfo.toffs *= skymaps[smap].height / 256.0;
	}
	unguard;
}

//==========================================================================
//
//	R_InitSky
//
//	Called at level load.
//
//==========================================================================

void R_InitSky()
{
	guard(R_InitSky);
	if (cl_level.skybox[0])
	{
		R_InitSkyBox();
	}
	else
	{
		R_InitOldSky();
	}
	unguard;
}

//==========================================================================
//
//	R_SkyChanged
//
//==========================================================================

void R_SkyChanged()
{
	guard(R_SkyChanged);
	if (cl_level.skybox[0])
	{
		return;
	}

	for (int j = 0; j < NumSkySurfs; j++)
	{
		sky[j].baseTexture1 = cl_level.sky1Texture;
		sky[j].baseTexture2 = cl_level.sky2Texture;
		if (cl_level.doubleSky)
		{
			sky[j].texture1 = sky[j].baseTexture2;
			sky[j].texture2 = sky[j].baseTexture1;
		}
		else
		{
			sky[j].texture1 = sky[j].baseTexture1;
		}
	}

	//	Precache textures
	Drawer->SetSkyTexture(cl_level.sky1Texture, cl_level.doubleSky);
	Drawer->SetSkyTexture(cl_level.sky2Texture, false);
	unguard;
}

//==========================================================================
//
//	R_AnimateSky
//
//==========================================================================

void R_AnimateSky()
{
	guard(R_AnimateSky);
	//	Update sky column offsets
	for (int i = 0; i < NumSkySurfs; i++)
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
	unguard;
}

//==========================================================================
//
//	R_LightningFlash
//
//==========================================================================

static void R_LightningFlash()
{
	guard(R_LightningFlash);
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
			tempSec = GClLevel->Sectors;
			for (i = 0; i < GClLevel->NumSectors; i++, tempSec++)
			{
				if (tempSec->ceiling.pic == skyflatnum ||
					tempSec->special == LIGHTNING_OUTDOOR ||
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
			tempSec = GClLevel->Sectors;
			for (i = 0; i < GClLevel->NumSectors; i++, tempSec++)
			{
				if (tempSec->ceiling.pic == skyflatnum ||
					tempSec->special == LIGHTNING_OUTDOOR ||
					tempSec->special == LIGHTNING_SPECIAL ||
					tempSec->special == LIGHTNING_SPECIAL2)
				{
					tempSec->params.lightlevel = *tempLight;
					tempLight++;
				}
			}
			for (i = 0; i < NumSkySurfs; i++)
			{
				sky[i].texture1 = sky[i].baseTexture1;
			}
		}
		return;
	}

	LightningFlash = (rand() & 7) + 8;
	flashLight = 200 + (rand() & 31);
	tempSec = GClLevel->Sectors;
	tempLight = LightningLightLevels;
	foundSec = false;
	for (i = 0; i < GClLevel->NumSectors; i++, tempSec++)
	{
		if (tempSec->ceiling.pic == skyflatnum ||
			tempSec->special == LIGHTNING_OUTDOOR ||
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
		for (i = 0; i < NumSkySurfs; i++)
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
	unguard;
}

//==========================================================================
//
//	R_ForceLightning
//
//==========================================================================

void R_ForceLightning()
{
	guard(R_ForceLightning);
	NextLightningFlash = 0;
	unguard;
}

//==========================================================================
//
//	R_DrawSky
//
//==========================================================================

void R_DrawSky()
{
	guard(R_DrawSky);
	Drawer->BeginSky();

	for (int i = 0; i < NumSkySurfs; i++)
	{
		r_normal = sky[i].plane.normal;
		r_dist = sky[i].plane.dist;

		sky[i].surf.Light = 0xffffffff;

		r_surface = &sky[i].surf;
		Drawer->DrawSkyPolygon(sky[i].surf.verts, 4, sky[i].texture1,
			sky[i].columnOffset1, sky[i].texture2, sky[i].columnOffset2);
	}

	Drawer->EndSky();
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.15  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//
//	Revision 1.14  2004/11/22 07:36:29  dj_jl
//	Implemented all sector specials in all games.
//	
//	Revision 1.13  2004/10/08 12:37:47  dj_jl
//	Better rendering of old skies.
//	
//	Revision 1.12  2002/09/07 16:31:51  dj_jl
//	Added Level class.
//	
//	Revision 1.11  2002/08/28 16:39:19  dj_jl
//	Implemented sector light color.
//	
//	Revision 1.10  2002/03/28 17:58:02  dj_jl
//	Added support for scaled textures.
//	
//	Revision 1.9  2002/03/20 19:11:21  dj_jl
//	Added guarding.
//	
//	Revision 1.8  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.7  2001/11/02 18:35:55  dj_jl
//	Sky optimizations
//	
//	Revision 1.6  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
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
