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

#define RADIUS		128.0

// TYPES -------------------------------------------------------------------

struct skyboxinfo_t
{
	struct skyboxsurf_t
	{
		int			texture;
	};

	VName			Name;
	skyboxsurf_t	surfs[6];
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TArray<skyboxinfo_t>		skyboxinfo;

static VCvarI		r_skyboxes("r_skyboxes", "1", CVAR_Archive);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	R_InitSkyBoxes
//
//==========================================================================

static void ParseSkyBoxesScript(VScriptParser* sc)
{
	guard(R_InitSkyBoxes);
	while (!sc->AtEnd())
	{
		skyboxinfo_t& info = skyboxinfo.Alloc();
		memset(&info, 0, sizeof(info));

		sc->ExpectString();
		info.Name = *sc->String;
		sc->Expect("{");
		for (int i = 0; i < 6; i++)
		{
			sc->Expect("{");
			sc->Expect("map");
			sc->ExpectString();
			info.surfs[i].texture = GTextureManager.AddFileTexture(
				VName(*sc->String), TEXTYPE_SkyMap);
			sc->Expect("}");
		}
		sc->Expect("}");
	}
	delete sc;
	unguard;
}

//==========================================================================
//
//	R_InitSkyBoxes
//
//==========================================================================

void R_InitSkyBoxes()
{
	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) == NAME_skyboxes)
		{
			ParseSkyBoxesScript(new VScriptParser("skyboxes",
				W_CreateLumpReaderNum(Lump)));
		}
	}
	//	Optionally parse script file.
	if (fl_devmode && FL_FileExists("scripts/skyboxes.txt"))
	{
		ParseSkyBoxesScript(new VScriptParser("scripts/skyboxes.txt",
			FL_OpenFileRead("scripts/skyboxes.txt")));
	}
}

//==========================================================================
//
//	CheckSkyboxNumForName
//
//==========================================================================

static int CheckSkyboxNumForName(VName Name)
{
	guard(CheckSkyboxNumForName);
	for (int num = skyboxinfo.Num() - 1; num >= 0 ; num--)
	{
		if (skyboxinfo[num].Name == Name)
		{
			return num;
		}
	}
	return -1;
	unguard;
}

//==========================================================================
//
//	R_FreeSkyboxData
//
//==========================================================================

void R_FreeSkyboxData()
{
	guard(R_FreeSkyboxData);
	skyboxinfo.Clear();
	unguard;
}

//==========================================================================
//
//	VSky::InitOldSky
//
//==========================================================================

void VSky::InitOldSky(int Sky1Texture, int Sky2Texture, float Sky1ScrollDelta,
	float Sky2ScrollDelta, bool DoubleSky, bool ForceNoSkyStretch, bool Flip)
{
	guard(VSky::InitOldSky);
	memset(sky, 0, sizeof(sky));
	bIsSkyBox = false;

	int skyheight = GTextureManager[Sky1Texture]->GetHeight();
	if (ForceNoSkyStretch)
	{
		skyheight = 256;
	}
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

			s.columnOffset1 = s.columnOffset2 = -i * (1024 / HDIVS) + 128;

			float mins;
			float maxs;

			if (Flip)
			{
				s.texinfo.saxis = -s.texinfo.saxis;
				s.texinfo.soffs = -s.texinfo.soffs;
				s.columnOffset1 = -s.columnOffset1;
				s.columnOffset2 = -s.columnOffset2;

				mins = DotProduct(s.surf.verts[j < VDIVS / 2 ? 3 : 2],
					s.texinfo.saxis) + s.texinfo.soffs;
				maxs = DotProduct(s.surf.verts[j < VDIVS / 2 ? 0 : 1],
					s.texinfo.saxis) + s.texinfo.soffs;
			}
			else
			{
				mins = DotProduct(s.surf.verts[j < VDIVS / 2 ? 0 : 1],
					s.texinfo.saxis) + s.texinfo.soffs;
				maxs = DotProduct(s.surf.verts[j < VDIVS / 2 ? 3 : 2],
					s.texinfo.saxis) + s.texinfo.soffs;
			}

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
		}
	}

	NumSkySurfs = VDIVS * HDIVS;

	for (j = 0; j < NumSkySurfs; j++)
	{
		sky[j].baseTexture1 = Sky1Texture;
		sky[j].baseTexture2 = Sky2Texture;
		if (DoubleSky)
		{
			sky[j].texture1 = sky[j].baseTexture2;
			sky[j].texture2 = sky[j].baseTexture1;
			sky[j].scrollDelta1 = Sky2ScrollDelta;
			sky[j].scrollDelta2 = Sky1ScrollDelta;
		}
		else
		{
			sky[j].texture1 = sky[j].baseTexture1;
			sky[j].scrollDelta1 = Sky1ScrollDelta;
		}
		sky[j].surf.plane = &sky[j].plane;
		sky[j].surf.texinfo = &sky[j].texinfo;
		sky[j].surf.count = 4;
		sky[j].surf.Light = 0xffffffff;
	}

	//	Precache textures
	Drawer->PrecacheTexture(GTextureManager[Sky1Texture]);
	Drawer->PrecacheTexture(GTextureManager[Sky2Texture]);
	unguard;
}

//==========================================================================
//
//	VSky::InitSkyBox
//
//==========================================================================

void VSky::InitSkyBox(VName Name1, VName Name2)
{
	guard(VSky::InitSkyBox);
	int num = CheckSkyboxNumForName(Name1);
	if (num == -1)
	{
		Host_Error("No such skybox %s", *Name1);
	}
	skyboxinfo_t& s1info = skyboxinfo[num];
	if (Name2 != NAME_None)
	{
		num = CheckSkyboxNumForName(Name2);
		if (num == -1)
		{
			Host_Error("No such skybox %s", *Name2);
		}
	}
	skyboxinfo_t& s2info = skyboxinfo[num];

	memset(sky, 0, sizeof(sky));

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
		sky[j].texture1 = s1info.surfs[j].texture;
		sky[j].baseTexture1 = s1info.surfs[j].texture;
		sky[j].baseTexture2 = s2info.surfs[j].texture;
		sky[j].surf.plane = &sky[j].plane;
		sky[j].surf.texinfo = &sky[j].texinfo;
		sky[j].surf.count = 4;
		sky[j].surf.Light = 0xffffffff;

		//	Precache texture
		Drawer->PrecacheTexture(GTextureManager[sky[j].texture1]);

		VTexture* STex = GTextureManager[sky[j].texture1];

		sky[j].surf.extents[0] = STex->GetWidth();
		sky[j].surf.extents[1] = STex->GetHeight();
		sky[j].texinfo.saxis *= STex->GetWidth() / 256.0;
		sky[j].texinfo.soffs *= STex->GetWidth() / 256.0;
		sky[j].texinfo.taxis *= STex->GetHeight() / 256.0;
		sky[j].texinfo.toffs *= STex->GetHeight() / 256.0;
	}
	bIsSkyBox = true;
	unguard;
}

//==========================================================================
//
//	VSky::Init
//
//==========================================================================

void VSky::Init(int Sky1Texture, int Sky2Texture, float Sky1ScrollDelta,
	float Sky2ScrollDelta, bool DoubleSky, bool ForceNoSkyStretch,
	bool Flip, bool Lightning)
{
	guard(VSky::Init);
	int Num1 = -1;
	int Num2 = -1;
	VName Name1(NAME_None);
	VName Name2(NAME_None);
	//	Check if we want to replace old sky with a skybox. We can't do
	// this if level is using double sky or it's scrolling.
	if (r_skyboxes && !DoubleSky && !Sky1ScrollDelta)
	{
		Name1 = GTextureManager[Sky1Texture]->Name;
		Name2 = Lightning ? GTextureManager[Sky2Texture]->Name : Name1;
		Num1 = CheckSkyboxNumForName(Name1);
		Num2 = CheckSkyboxNumForName(Name2);
	}
	if (Num1 != -1 && Num2 != -1)
	{
		InitSkyBox(Name1, Name2);
	}
	else
	{
		InitOldSky(Sky1Texture, Sky2Texture, Sky1ScrollDelta, Sky2ScrollDelta,
			DoubleSky, ForceNoSkyStretch, Flip);
	}
	SideTex = Sky1Texture;
	SideFlip = Flip;
	unguard;
}

//==========================================================================
//
//	VSky::Draw
//
//==========================================================================

void VSky::Draw(int ColourMap)
{
	guard(VSky::Draw);
	Drawer->BeginSky();

	for (int i = 0; i < NumSkySurfs; i++)
	{
		Drawer->DrawSkyPolygon(&sky[i].surf, bIsSkyBox,
			GTextureManager(sky[i].texture1), sky[i].columnOffset1,
			GTextureManager(sky[i].texture2), sky[i].columnOffset2,
			ColourMap);
	}

	Drawer->EndSky();
	unguard;
}

//==========================================================================
//
//	VRenderLevel::InitSky
//
//	Called at level load.
//
//==========================================================================

void VRenderLevel::InitSky()
{
	guard(VRenderLevel::InitSky);
	if (CurrentSky1Texture == Level->LevelInfo->Sky1Texture &&
		CurrentSky2Texture == Level->LevelInfo->Sky2Texture &&
		CurrentDoubleSky == !!(Level->LevelInfo->LevelInfoFlags & VLevelInfo::LIF_DoubleSky) &&
		CurrentLightning == !!(Level->LevelInfo->LevelInfoFlags & VLevelInfo::LIF_Lightning))
	{
		return;
	}
	CurrentSky1Texture = Level->LevelInfo->Sky1Texture;
	CurrentSky2Texture = Level->LevelInfo->Sky2Texture;
	CurrentDoubleSky = !!(Level->LevelInfo->LevelInfoFlags & VLevelInfo::LIF_DoubleSky);
	CurrentLightning = !!(Level->LevelInfo->LevelInfoFlags & VLevelInfo::LIF_Lightning);

	// Check if the level is a lightning level
	LevelHasLightning = false;
	LightningFlash = 0;
	if (Level->LevelInfo->LevelInfoFlags & VLevelInfo::LIF_Lightning)
	{
		int secCount = 0;
		for (int i = 0; i < Level->NumSectors; i++)
		{
			if (Level->Sectors[i].ceiling.pic == skyflatnum ||
				Level->Sectors[i].special == LIGHTNING_OUTDOOR ||
				Level->Sectors[i].special == LIGHTNING_SPECIAL ||
				Level->Sectors[i].special == LIGHTNING_SPECIAL2)
			{
				secCount++;
			}
		}
		if (secCount)
		{
			LevelHasLightning = true;
			LightningLightLevels = new int[secCount];
			NextLightningFlash = ((rand() & 15) + 5) * 35; // don't flash at level start
		}
	}

	if (Level->LevelInfo->SkyBox != NAME_None)
	{
		BaseSky.InitSkyBox(Level->LevelInfo->SkyBox, NAME_None);
	}
	else
	{
		BaseSky.Init(CurrentSky1Texture, CurrentSky2Texture,
			Level->LevelInfo->Sky1ScrollDelta,
			Level->LevelInfo->Sky2ScrollDelta, CurrentDoubleSky,
			!!(Level->LevelInfo->LevelInfoFlags &
			VLevelInfo::LIF_ForceNoSkyStretch), true, CurrentLightning);
	}
	unguard;
}

//==========================================================================
//
//	VRenderLevel::AnimateSky
//
//==========================================================================

void VRenderLevel::AnimateSky(float frametime)
{
	guard(VRenderLevel::AnimateSky);
	InitSky();

	//	Update sky column offsets
	for (int i = 0; i < BaseSky.NumSkySurfs; i++)
	{
		BaseSky.sky[i].columnOffset1 += BaseSky.sky[i].scrollDelta1 * frametime;
		BaseSky.sky[i].columnOffset2 += BaseSky.sky[i].scrollDelta2 * frametime;
	}

	//	Update lightning
	if (LevelHasLightning)
	{
		if (!NextLightningFlash || LightningFlash)
		{
			DoLightningFlash();
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
//	VRenderLevel::DoLightningFlash
//
//==========================================================================

void VRenderLevel::DoLightningFlash()
{
	guard(VRenderLevel::DoLightningFlash);
	int 		i;
	sector_t 	*tempSec;
	int 		*tempLight;
	bool	 	foundSec;
	int 		flashLight;

	if (LightningFlash)
	{
		LightningFlash--;
		if (LightningFlash)
		{
			tempLight = LightningLightLevels;
			tempSec = Level->Sectors;
			for (i = 0; i < Level->NumSectors; i++, tempSec++)
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
			tempSec = Level->Sectors;
			for (i = 0; i < Level->NumSectors; i++, tempSec++)
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
			for (i = 0; i < BaseSky.NumSkySurfs; i++)
			{
				BaseSky.sky[i].texture1 = BaseSky.sky[i].baseTexture1;
			}
		}
		return;
	}

	LightningFlash = (rand() & 7) + 8;
	flashLight = 200 + (rand() & 31);
	tempSec = Level->Sectors;
	tempLight = LightningLightLevels;
	foundSec = false;
	for (i = 0; i < Level->NumSectors; i++, tempSec++)
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
		for (i = 0; i < BaseSky.NumSkySurfs; i++)
		{
			BaseSky.sky[i].texture1 = BaseSky.sky[i].baseTexture2; // set alternate sky
		}
		GAudio->PlaySound(GSoundManager->GetSoundID("world/thunder"),
			TVec(0, 0, 0), TVec(0, 0, 0), 0, 0, 1, 0, false);
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
			if ((rand() & 0xff) < 128 && !(Level->TicTime & 32))
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
//	VRenderLevel::ForceLightning
//
//==========================================================================

void VRenderLevel::ForceLightning()
{
	guard(VRenderLevel::ForceLightning);
	NextLightningFlash = 0;
	unguard;
}

//==========================================================================
//
//	VRenderLevel::DrawSky
//
//==========================================================================

void VRenderLevel::DrawSky()
{
	guard(VRenderLevel::DrawSky);
	InitSky();
	BaseSky.Draw(ColourMap);
	unguard;
}

//==========================================================================
//
//	VSkyPortal::DrawContents
//
//==========================================================================

void VSkyPortal::DrawContents()
{
	guard(VSkyPortal::DrawContents);
	Sky->Draw(RLev->ColourMap);
	unguard;
}

//==========================================================================
//
//	VSkyPortal::MatchSky
//
//==========================================================================

bool VSkyPortal::MatchSky(VSky* ASky)
{
	return Sky == ASky;
}
