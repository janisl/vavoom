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
//**	Rendering main loop and setup functions, utility functions (BSP,
//**  geometry, trigonometry). See tables.c, too.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "r_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void R_FreeSkyboxData();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int						screenblocks = 0;

TVec					vieworg;
TVec					viewforward;
TVec					viewright;
TVec					viewup;
TAVec					viewangles;

TClipPlane				view_clipplanes[5];

int						r_visframecount;

VCvarI					r_fog("r_fog", "0");
VCvarI					r_fog_test("r_fog_test", "0");
VCvarF					r_fog_r("r_fog_r", "0.5");
VCvarF					r_fog_g("r_fog_g", "0.5");
VCvarF					r_fog_b("r_fog_b", "0.5");
VCvarF					r_fog_start("r_fog_start", "1.0");
VCvarF					r_fog_end("r_fog_end", "2048.0");
VCvarF					r_fog_density("r_fog_density", "0.5");

VCvarI					aspect_ratio("r_aspect_ratio", "1", CVAR_Archive);
VCvarI					r_interpolate_frames("r_interpolate_frames", "1", CVAR_Archive);
VCvarI					r_vsync("r_vsync", "1", CVAR_Archive);
VCvarI					r_fade_light("r_fade_light", "0", CVAR_Archive);
VCvarF					r_fade_factor("r_fade_factor", "4.0", CVAR_Archive);
VCvarF					r_sky_bright_factor("r_sky_bright_factor", "1.0", CVAR_Archive);

VDrawer					*Drawer;

refdef_t				refdef;

float					PixelAspect;

bool					MirrorFlip;
bool					MirrorClip;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static FDrawerDesc		*DrawerList[DRAWER_MAX];

VCvarI					screen_size("screen_size", "10", CVAR_Archive);
bool					set_resolutioon_needed = true;

// Angles in the SCREENWIDTH wide window.
VCvarF					fov("fov", "90");

TVec					clip_base[4];

//
//	Translation tables
//
VTextureTranslation*	PlayerTranslations[MAXPLAYERS + 1];
static TArray<VTextureTranslation*>	CachedTranslations;

// if true, load all graphics at start
VCvarI					precache("precache", "1", CVAR_Archive);

static VCvarI			_driver("_driver", "0", CVAR_Rom);

static VCvarI			r_level_renderer("r_level_renderer", "0", CVAR_Archive);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  FDrawerDesc::FDrawerDesc
//
//==========================================================================

FDrawerDesc::FDrawerDesc(int Type, const char* AName, const char* ADescription,
	const char* ACmdLineArg, VDrawer* (*ACreator)())
: Name(AName)
, Description(ADescription)
, CmdLineArg(ACmdLineArg)
, Creator(ACreator)
{
	guard(FDrawerDesc::FDrawerDesc);
	DrawerList[Type] = this;
	unguard
}

//==========================================================================
//
//  R_Init
//
//==========================================================================

void R_Init()
{
	guard(R_Init);
	R_InitSkyBoxes();
	R_InitModels();

	for (int i = 0; i < 256; i++)
	{
		light_remap[i] = byte(i * i / 255);
	}
	unguard;
}

//==========================================================================
//
//  R_Start
//
//==========================================================================

void R_Start(VLevel* ALevel)
{
	guard(R_Start);
	switch (r_level_renderer)
	{
	case 1:
		ALevel->RenderData = new VRenderLevel(ALevel);
		break;

	case 2:
		ALevel->RenderData = new VAdvancedRenderLevel(ALevel);
		break;

	default:
		if (Drawer->SupportsAdvancedRendering())
		{
			ALevel->RenderData = new VAdvancedRenderLevel(ALevel);
		}
		else
		{
			ALevel->RenderData = new VRenderLevel(ALevel);
		}
		break;
	}
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::VRenderLevelShared
//
//==========================================================================

VRenderLevelShared::VRenderLevelShared(VLevel* ALevel)
: Level(ALevel)
, ViewEnt(NULL)
, MirrorLevel(0)
, PortalLevel(0)
, VisSize(0)
, BspVis(NULL)
, r_viewleaf(NULL)
, r_oldviewleaf(NULL)
, old_fov(90.0)
, prev_aspect_ratio(0)
, ExtraLight(0)
, FixedLight(0)
, Particles(0)
, ActiveParticles(0)
, FreeParticles(0)
, CurrentSky1Texture(-1)
, CurrentSky2Texture(-1)
, CurrentDoubleSky(false)
, CurrentLightning(false)
, trans_sprites(MainTransSprites)
, free_wsurfs(NULL)
, AllocatedWSurfBlocks(NULL)
, AllocatedSubRegions(NULL)
, AllocatedDrawSegs(NULL)
, AllocatedSegParts(NULL)
, cacheframecount(0)
{
	guard(VRenderLevelShared::VRenderLevelShared);
	memset(MainTransSprites, 0, sizeof(MainTransSprites));

	memset(light_block, 0, sizeof(light_block));
	memset(block_changed, 0, sizeof(block_changed));
	memset(light_chain, 0, sizeof(light_chain));
	memset(add_block, 0, sizeof(add_block));
	memset(add_changed, 0, sizeof(add_changed));
	memset(add_chain, 0, sizeof(add_chain));
	SimpleSurfsHead = NULL;
	SimpleSurfsTail = NULL;
	SkyPortalsHead = NULL;
	SkyPortalsTail = NULL;
	HorizonPortalsHead = NULL;
	HorizonPortalsTail = NULL;
	PortalDepth = 0;

	VisSize = (Level->NumSubsectors + 7) >> 3;
	BspVis = new vuint8[VisSize];

	InitParticles();
	ClearParticles();

	screenblocks = 0;

	// preload graphics
	if (precache)
	{
		PrecacheLevel();
	}
	unguard;
}

//==========================================================================
//
//	VRenderLevel::VRenderLevel
//
//==========================================================================

VRenderLevel::VRenderLevel(VLevel* ALevel)
: VRenderLevelShared(ALevel)
, c_subdivides(0)
, c_seg_div(0)
, freeblocks(NULL)
{
	guard(VRenderLevel::VRenderLevel);
	NeedsInfiniteFarClip = false;

	memset(cacheblocks, 0, sizeof(cacheblocks));
	memset(blockbuf, 0, sizeof(blockbuf));

	FlushCaches();

	memset(DLights, 0, sizeof(DLights));
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::VAdvancedRenderLevel
//
//==========================================================================

VAdvancedRenderLevel::VAdvancedRenderLevel(VLevel* ALevel)
: VRenderLevelShared(ALevel)
, LightVis(NULL)
{
	guard(VAdvancedRenderLevel::VAdvancedRenderLevel);
	NeedsInfiniteFarClip = true;

	memset(DLights, 0, sizeof(DLights));

	LightVis = new vuint8[VisSize];
	LightBspVis = new vuint8[VisSize];
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::~VRenderLevelShared
//
//==========================================================================

VRenderLevelShared::~VRenderLevelShared()
{
	guard(VRenderLevelShared::~VRenderLevelShared);
	//	Free fake floor data.
	for (int i = 0; i < Level->NumSectors; i++)
	{
		if (Level->Sectors[i].fakefloors)
		{
			delete Level->Sectors[i].fakefloors;
			Level->Sectors[i].fakefloors = NULL;
		}
	}

	for (int i = 0; i < Level->NumSubsectors; i++)
	{
		for (subregion_t* r = Level->Subsectors[i].regions; r; r = r->next)
		{
			FreeSurfaces(r->floor->surfs);
			delete r->floor;
			r->floor = NULL;
			FreeSurfaces(r->ceil->surfs);
			delete r->ceil;
			r->ceil = NULL;
		}
	}

	//	Free seg parts.
	for (int i = 0; i < Level->NumSegs; i++)
	{
		for (drawseg_t* ds = Level->Segs[i].drawsegs; ds; ds = ds->next)
		{
			FreeSegParts(ds->top);
			FreeSegParts(ds->mid);
			FreeSegParts(ds->bot);
			FreeSegParts(ds->topsky);
			FreeSegParts(ds->extra);
			if (ds->HorizonTop)
			{
				Z_Free(ds->HorizonTop);
			}
			if (ds->HorizonBot)
			{
				Z_Free(ds->HorizonBot);
			}
		}
	}
	//	Free allocated wall surface blocks.
	for (void* Block = AllocatedWSurfBlocks; Block;)
	{
		void* Next = *(void**)Block;
		Z_Free(Block);
		Block = Next;
	}
	AllocatedWSurfBlocks = NULL;

	//	Free big blocks.
	delete[] AllocatedSubRegions;
	AllocatedSubRegions = NULL;
	delete[] AllocatedDrawSegs;
	AllocatedDrawSegs = NULL;
	delete[] AllocatedSegParts;
	AllocatedSegParts = NULL;

	delete[] Particles;
	Particles = NULL;
	delete[] BspVis;
	BspVis = NULL;

	for (int i = 0; i < SideSkies.Num(); i++)
	{
		delete SideSkies[i];
		SideSkies[i] = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::~VAdvancedRenderLevel
//
//==========================================================================

VAdvancedRenderLevel::~VAdvancedRenderLevel()
{
	guard(VAdvancedRenderLevel::~VAdvancedRenderLevel);
	delete[] LightVis;
	LightVis = NULL;
	delete[] LightBspVis;
	LightBspVis = NULL;
	unguard;
}

//==========================================================================
//
// 	R_SetViewSize
//
// 	Do not really change anything here, because it might be in the middle
// of a refresh. The change will take effect next refresh.
//
//==========================================================================

void R_SetViewSize(int blocks)
{
	guard(R_SetViewSize);
	if (blocks > 2)
	{
		screen_size = blocks;
	}
	set_resolutioon_needed = true;
	unguard;
}

//==========================================================================
//
//  COMMAND SizeDown
//
//==========================================================================

COMMAND(SizeDown)
{
	R_SetViewSize(screenblocks - 1);
	GAudio->PlaySound(GSoundManager->GetSoundID("menu/change"),
		TVec(0, 0, 0), TVec(0, 0, 0), 0, 0, 1, 0, false);
}

//==========================================================================
//
//  COMMAND SizeUp
//
//==========================================================================

COMMAND(SizeUp)
{
	R_SetViewSize(screenblocks + 1);
	GAudio->PlaySound(GSoundManager->GetSoundID("menu/change"),
		TVec(0, 0, 0), TVec(0, 0, 0), 0, 0, 1, 0, false);
}

//==========================================================================
//
//	VRenderLevelShared::ExecuteSetViewSize
//
//==========================================================================

void VRenderLevelShared::ExecuteSetViewSize()
{
	guard(VRenderLevelShared::ExecuteSetViewSize);
	set_resolutioon_needed = false;
	if (screen_size < 3)
	{
		screen_size = 3;
	}
	if (screen_size > 11)
	{
		screen_size = 11;
	}
	screenblocks = screen_size;

	if (fov < 5.0)
	{
		fov = 5.0;
	}
	if (fov > 175.0)
	{
		fov = 175.0;
	}
	old_fov = fov;

	if (screenblocks > 10)
	{
		refdef.width = ScreenWidth;
		refdef.height = ScreenHeight;
		refdef.y = 0;
	}
	else if (GGameInfo->NetMode == NM_TitleMap)
	{
		//	No status bar for titlemap.
		refdef.width = screenblocks * ScreenWidth / 10;
		refdef.height = (screenblocks * ScreenHeight / 10);
		refdef.y = (ScreenHeight - refdef.height) >> 1;
	}
	else
	{
		refdef.width = screenblocks * ScreenWidth / 10;
		refdef.height = (screenblocks * (ScreenHeight - SB_REALHEIGHT) / 10);
		refdef.y = (ScreenHeight - SB_REALHEIGHT - refdef.height) >> 1;
	}
	refdef.x = (ScreenWidth - refdef.width) >> 1;

	if (aspect_ratio == 0)
	{
		// Original aspect ratio
		PixelAspect = ((float)ScreenHeight * 320.0) / ((float)ScreenWidth * 200.0);
	}
	else if (aspect_ratio == 1)
	{
		// 4:3 aspect ratio
		PixelAspect = ((float)ScreenHeight * 4.0) / ((float)ScreenWidth * 3.0);
	}
	else if (aspect_ratio == 2)
	{
		// 16:9 aspect ratio
		PixelAspect = ((float)ScreenHeight * 16.0) / ((float)ScreenWidth * 9.0);
	}
	else
	{
		// 16:10 aspect ratio
		PixelAspect = ((float)ScreenHeight * 16.0) / ((float)ScreenWidth * 10.0);
	}
	prev_aspect_ratio = aspect_ratio;

	refdef.fovx = tan(DEG2RAD(fov) / 2);
	refdef.fovy = refdef.fovx * refdef.height / refdef.width / PixelAspect;

	// left side clip
	clip_base[0] = Normalise(TVec(1, 1.0 / refdef.fovx, 0));
	
	// right side clip
	clip_base[1] = Normalise(TVec(1, -1.0 / refdef.fovx, 0));
	
	// top side clip
	clip_base[2] = Normalise(TVec(1, 0, -1.0 / refdef.fovy));
	
	// bottom side clip
	clip_base[3] = Normalise(TVec(1, 0, 1.0 / refdef.fovy));

	refdef.drawworld = true;
	unguard;
}

//==========================================================================
//
//	R_DrawViewBorder
//
//==========================================================================

void R_DrawViewBorder()
{
	guard(R_DrawViewBorder);
	if (GGameInfo->NetMode == NM_TitleMap)
	{
		GClGame->eventDrawViewBorder(320 - screenblocks * 32,
			(480 - screenblocks * 480 / 10) / 2,
			screenblocks * 64, screenblocks * 480 / 10);
	}
	else
	{
		GClGame->eventDrawViewBorder(320 - screenblocks * 32,
			(480 - sb_height - screenblocks * (480 - sb_height) / 10) / 2,
			screenblocks * 64, screenblocks * (480 - sb_height) / 10);
	}
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::TransformFrustum
//
//==========================================================================

void VRenderLevelShared::TransformFrustum()
{
	guard(VRenderLevelShared::TransformFrustum);
	for (int i = 0; i < 4; i++)
	{
		TVec &v = clip_base[i];
		TVec v2;

		v2.x = v.y * viewright.x + v.z * viewup.x + v.x * viewforward.x;
		v2.y = v.y * viewright.y + v.z * viewup.y + v.x * viewforward.y;
		v2.z = v.y * viewright.z + v.z * viewup.z + v.x * viewforward.z;

		view_clipplanes[i].Set(v2, DotProduct(vieworg, v2));

		view_clipplanes[i].next = i == 3 ? NULL : &view_clipplanes[i + 1];
		view_clipplanes[i].clipflag = 1 << i;
	}
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::SetupFrame
//
//==========================================================================

VCvarI			r_chasecam("r_chasecam", "0", CVAR_Archive);
VCvarF			r_chase_dist("r_chase_dist", "32.0", CVAR_Archive);
VCvarF			r_chase_up("r_chase_up", "128.0", CVAR_Archive);
VCvarF			r_chase_right("r_chase_right", "0", CVAR_Archive);
VCvarI			r_chase_front("r_chase_front", "0", CVAR_Archive);

void VRenderLevelShared::SetupFrame()
{
	guard(VRenderLevelShared::SetupFrame);
	// change the view size if needed
	if (screen_size != screenblocks || !screenblocks ||
		set_resolutioon_needed || old_fov != fov ||
		aspect_ratio != prev_aspect_ratio)
	{
		ExecuteSetViewSize();
	}

	ViewEnt = cl->Camera;
	viewangles = cl->ViewAngles;
	if (r_chasecam && r_chase_front)
	{
		//	This is used to see how weapon looks in player's hands
		viewangles.yaw = AngleMod(viewangles.yaw + 180);
		viewangles.pitch = -viewangles.pitch;
	}
	AngleVectors(viewangles, viewforward, viewright, viewup);

	if (r_chasecam && cl->MO == cl->Camera)
	{
		vieworg = cl->MO->Origin + TVec(0.0, 0.0, 32.0)
			- r_chase_dist * viewforward + r_chase_up * viewup
			+ r_chase_right * viewright;
	}
	else
	{
		vieworg = cl->ViewOrg;
	}

	ExtraLight = ViewEnt->Player ? ViewEnt->Player->ExtraLight * 8 : 0;
	if (cl->Camera == cl->MO)
	{
		ColourMap = CM_Default;
		if (cl->FixedColourmap == INVERSECOLOURMAP)
		{
			ColourMap = CM_Inverse;
			FixedLight = 255;
		}
		else if (cl->FixedColourmap == GOLDCOLOURMAP)
		{
			ColourMap = CM_Gold;
			FixedLight = 255;
		}
		else if (cl->FixedColourmap == REDCOLOURMAP)
		{
			ColourMap = CM_Red;
			FixedLight = 255;
		}
		else if (cl->FixedColourmap == GREENCOLOURMAP)
		{
			ColourMap = CM_Green;
			FixedLight = 255;
		}
		else if (cl->FixedColourmap >= NUMCOLOURMAPS)
		{
			FixedLight = 255;
		}
		else if (cl->FixedColourmap)
		{
			FixedLight = 255 - (cl->FixedColourmap << 3);
		}
		else
		{
			FixedLight = 0;
		}
	}
	else
	{
		FixedLight = 0;
		ColourMap = 0;
	}
	//	Inverse colourmap flash effect.
	if (cl->ExtraLight == 255)
	{
		ExtraLight = 0;
		ColourMap = CM_Inverse;
		FixedLight = 255;
	}

	Drawer->SetupView(this, &refdef);
	cacheframecount++;
	PortalDepth = 0;
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::SetupCameraFrame
//
//==========================================================================

void VRenderLevelShared::SetupCameraFrame(VEntity* Camera, VTexture* Tex,
	int FOV, refdef_t* rd)
{
	guard(VRenderLevelShared::SetupCameraFrame);
	rd->width = Tex->GetWidth();
	rd->height = Tex->GetHeight();
	rd->y = 0;
	rd->x = 0;

	if (aspect_ratio == 0)
	{
		PixelAspect = ((float)rd->height * 320.0) / ((float)rd->width * 200.0);
	}
	else if (aspect_ratio == 1)
	{
		PixelAspect = ((float)rd->height * 4.0) / ((float)rd->width * 3.0);
	}
	else if (aspect_ratio == 2)
	{
		PixelAspect = ((float)rd->height * 16.0) / ((float)rd->width * 9.0);
	}
	else if (aspect_ratio > 2)
	{
		PixelAspect = ((float)rd->height * 16.0) / ((float)rd->width * 10.0);
	}

	rd->fovx = tan(DEG2RAD(FOV) / 2);
	rd->fovy = rd->fovx * rd->height / rd->width / PixelAspect;

	// left side clip
	clip_base[0] = Normalise(TVec(1, 1.0 / rd->fovx, 0));
	
	// right side clip
	clip_base[1] = Normalise(TVec(1, -1.0 / rd->fovx, 0));
	
	// top side clip
	clip_base[2] = Normalise(TVec(1, 0, -1.0 / rd->fovy));
	
	// bottom side clip
	clip_base[3] = Normalise(TVec(1, 0, 1.0 / rd->fovy));

	rd->drawworld = true;

	ViewEnt = Camera;
	viewangles = Camera->Angles;
	AngleVectors(viewangles, viewforward, viewright, viewup);

	vieworg = Camera->Origin;

	ExtraLight = 0;
	FixedLight = 0;
	ColourMap = 0;

	Drawer->SetupView(this, rd);
	cacheframecount++;
	PortalDepth = 0;
	set_resolutioon_needed = true;
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::MarkLeaves
//
//==========================================================================

void VRenderLevelShared::MarkLeaves()
{
	guard(VRenderLevelShared::MarkLeaves);
	byte	*vis;
	node_t	*node;
	int		i;

	if (r_oldviewleaf == r_viewleaf)
		return;
	
	r_visframecount++;
	r_oldviewleaf = r_viewleaf;

	vis = Level->LeafPVS(r_viewleaf);

	for (i = 0; i < Level->NumSubsectors; i++)
	{
		if (vis[i >> 3] & (1 << (i & 7)))
		{
			subsector_t *sub = &Level->Subsectors[i];
			sub->VisFrame = r_visframecount;
			node = sub->parent;
			while (node)
			{
				if (node->VisFrame == r_visframecount)
					break;
				node->VisFrame = r_visframecount;
				node = node->parent;
			}
		}
	}
	unguard;
}

//==========================================================================
//
//  VRenderLevel::RenderScene
//
//==========================================================================

void VRenderLevel::RenderScene(const refdef_t* RD, const VViewClipper* Range)
{
	guard(VRenderLevel::RenderScene);
	r_viewleaf = Level->PointInSubsector(vieworg);

	TransformFrustum();

	Drawer->SetupViewOrg();

	MarkLeaves();

	UpdateWorld(RD, Range);

	RenderWorld(RD, Range);

	RenderMobjs(RPASS_Normal);

	DrawParticles();

	DrawTranslucentPolys();
	unguard;
}

//==========================================================================
//
//  VAdvancedRenderLevel::RenderScene
//
//==========================================================================

void VAdvancedRenderLevel::RenderScene(const refdef_t* RD, const VViewClipper* Range)
{
	guard(VAdvancedRenderLevel::RenderScene);
	if (!Drawer->SupportsAdvancedRendering())
	{
		Host_Error("Advanced rendering not supported by graphics card");
	}

	r_viewleaf = Level->PointInSubsector(vieworg);

	TransformFrustum();

	Drawer->SetupViewOrg();

	MarkLeaves();

	UpdateWorld();

	RenderWorld(RD, Range);
	RenderMobjsAmbient();

	Drawer->BeginShadowVolumesPass();
	if (!FixedLight && r_dynamic)
	{
		dlight_t* l = DLights;
		for (int i = 0; i < MAX_DLIGHTS; i++, l++)
		{
			if (l->die < Level->Time || !l->radius)
			{
				continue;
			}
			RenderLightShadows(RD, Range, l->origin, l->radius, l->colour);
		}
	}
	if (!FixedLight && r_static_lights)
	{
		for (int i = 0; i < Lights.Num(); i++)
		{
			if (!Lights[i].radius)
			{
				continue;
			}
			RenderLightShadows(RD, Range, Lights[i].origin, Lights[i].radius, Lights[i].colour);
		}
	}

	Drawer->DrawWorldTexturesPass();
	RenderMobjsTextures();

	Drawer->DrawWorldFogPass();
	RenderMobjsFog();
	Drawer->EndFogPass();

	RenderMobjs(RPASS_NonShadow);

	DrawParticles();

	DrawTranslucentPolys();
	unguard;
}

//==========================================================================
//
//  R_RenderPlayerView
//
//==========================================================================

void R_RenderPlayerView()
{
	guard(R_RenderPlayerView);
	GClLevel->RenderData->RenderPlayerView();
	unguard;
}

//==========================================================================
//
//  VRenderLevelShared::RenderPlayerView
//
//==========================================================================

void VRenderLevelShared::RenderPlayerView()
{
	guard(VRenderLevelShared::RenderPlayerView);
	if (!Level->LevelInfo)
	{
		return;
	}

	GTextureManager.Time = Level->Time;

	BuildPlayerTranslations();

	AnimateSky(host_frametime);

	UpdateParticles(host_frametime);
	PushDlights();

	//	Update camera textures that were visible in last frame.
	for (int i = 0; i < Level->CameraTextures.Num(); i++)
	{
		UpdateCameraTexture(Level->CameraTextures[i].Camera,
			Level->CameraTextures[i].TexNum, Level->CameraTextures[i].FOV);
	}

	SetupFrame();

	RenderScene(&refdef, NULL);

	// draw the psprites on top of everything
	if (fov <= 90.0 && cl->MO == cl->Camera &&
		GGameInfo->NetMode != NM_TitleMap)
	{
		DrawPlayerSprites();
	}

	Drawer->EndView();

	// Draw croshair
	if (cl->MO == cl->Camera && GGameInfo->NetMode != NM_TitleMap)
	{
		DrawCroshair();
	}
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::UpdateCameraTexture
//
//==========================================================================

void VRenderLevelShared::UpdateCameraTexture(VEntity* Camera, int TexNum,
	int FOV)
{
	guard(VRenderLevelShared::UpdateCameraTexture);
	if (!Camera)
	{
		return;
	}

	if (!GTextureManager[TexNum]->bIsCameraTexture)
	{
		return;
	}
	VCameraTexture* Tex = (VCameraTexture*)GTextureManager[TexNum];
	if (!Tex->bNeedsUpdate)
	{
		return;
	}

	refdef_t		CameraRefDef;
	CameraRefDef.DrawCamera = true;

	SetupCameraFrame(Camera, Tex, FOV, &CameraRefDef);

	RenderScene(&CameraRefDef, NULL);

	Drawer->EndView();

	Tex->CopyImage();
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::GetFade
//
//==========================================================================

vuint32 VRenderLevelShared::GetFade(sec_region_t* Reg)
{
	guard(VRenderLevelShared::GetFade);
	if (r_fog_test)
	{
		return 0xff000000 | (int(255 * r_fog_r) << 16) |
			(int(255 * r_fog_g) << 8) | int(255 * r_fog_b);
	}
	if (Reg->params->Fade)
	{
		return Reg->params->Fade;
	}
	if (Level->LevelInfo->OutsideFog && Reg->ceiling->pic == skyflatnum)
	{
		return Level->LevelInfo->OutsideFog;
	}
	if (Level->LevelInfo->Fade)
	{
		return Level->LevelInfo->Fade;
	}
	if (Level->LevelInfo->FadeTable == NAME_fogmap)
	{
		return 0xff7f7f7f;
	}
	if (r_fade_light)
	{
		// Simulate light fading using dark fog
		return FADE_LIGHT;
	}
	else
	{
		return 0;
	}
	unguard;
}

//==========================================================================
//
//	R_DrawPic
//
//==========================================================================

void R_DrawPic(int x, int y, int handle, float Alpha)
{
	guard(R_DrawPic);
	if (handle < 0)
	{
		return;
	}

	VTexture* Tex = GTextureManager(handle);
	x -= Tex->GetScaledSOffset();
	y -= Tex->GetScaledTOffset();
	Drawer->DrawPic(fScaleX * x, fScaleY * y,
		fScaleX * (x + Tex->GetScaledWidth()), fScaleY * (y + Tex->GetScaledHeight()),
		0, 0, Tex->GetWidth(), Tex->GetHeight(), Tex, NULL, Alpha);
	unguard;
}

//==========================================================================
//
// 	VRenderLevelShared::PrecacheLevel
//
// 	Preloads all relevant graphics for the level.
//
//==========================================================================

void VRenderLevelShared::PrecacheLevel()
{
	guard(VRenderLevelShared::PrecacheLevel);
	int			i;

	if (cls.demoplayback)
		return;

#ifdef __GNUC__
	char texturepresent[GTextureManager.GetNumTextures()];
#else
	char* texturepresent = (char*)Z_Malloc(GTextureManager.GetNumTextures());
#endif
	memset(texturepresent, 0, GTextureManager.GetNumTextures());

	for (i = 0; i < Level->NumSectors; i++)
	{
		texturepresent[Level->Sectors[i].floor.pic] = true;
		texturepresent[Level->Sectors[i].ceiling.pic] = true;
	}
	
	for (i = 0; i < Level->NumSides; i++)
	{
		texturepresent[Level->Sides[i].TopTexture] = true;
		texturepresent[Level->Sides[i].MidTexture] = true;
		texturepresent[Level->Sides[i].BottomTexture] = true;
	}

	// Precache textures.
	for (i = 1; i < GTextureManager.GetNumTextures(); i++)
	{
		if (texturepresent[i])
		{
			Drawer->PrecacheTexture(GTextureManager[i]);
		}
	}

#ifndef __GNUC__
	Z_Free(texturepresent);
#endif
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::GetTranslation
//
//==========================================================================

VTextureTranslation* VRenderLevelShared::GetTranslation(int TransNum)
{
	guard(VRenderLevelShared::GetTranslation);
	return R_GetCachedTranslation(TransNum, Level);
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::BuildPlayerTranslations
//
//==========================================================================

void VRenderLevelShared::BuildPlayerTranslations()
{
	guard(VRenderLevelShared::BuildPlayerTranslations);
	for (TThinkerIterator<VPlayerReplicationInfo> It(Level); It; ++It)
	{
		if (It->PlayerNum < 0 || It->PlayerNum >= MAXPLAYERS)
		{
			//	Should not happen.
			continue;
		}
		if (!It->TranslStart || !It->TranslEnd)
		{
			continue;
		}

		VTextureTranslation* Tr = PlayerTranslations[It->PlayerNum];
		if (Tr && Tr->TranslStart == It->TranslStart &&
			Tr->TranslEnd == It->TranslEnd && Tr->Colour == It->Colour)
		{
			continue;
		}

		if (!Tr)
		{
			Tr = new VTextureTranslation;
			PlayerTranslations[It->PlayerNum] = Tr;
		}
		//	Don't waste time clearing if it's the same range.
		if (Tr->TranslStart != It->TranslStart ||
			Tr->TranslEnd != It->TranslEnd)
		{
			Tr->Clear();
		}
		Tr->BuildPlayerTrans(It->TranslStart, It->TranslEnd, It->Colour);
	}
	unguard;
}

//==========================================================================
//
//	R_SetMenuPlayerTrans
//
//==========================================================================

int R_SetMenuPlayerTrans(int Start, int End, int Col)
{
	guard(R_SetMenuPlayerTrans);
	if (!Start || !End)
	{
		return 0;
	}

	VTextureTranslation* Tr = PlayerTranslations[MAXPLAYERS];
	if (Tr && Tr->TranslStart == Start && Tr->TranslEnd == End &&
		Tr->Colour == Col)
	{
		return (TRANSL_Player << TRANSL_TYPE_SHIFT) + MAXPLAYERS;
	}

	if (!Tr)
	{
		Tr = new VTextureTranslation;
		PlayerTranslations[MAXPLAYERS] = Tr;
	}
	if (Tr->TranslStart != Start || Tr->TranslEnd == End)
	{
		Tr->Clear();
	}
	Tr->BuildPlayerTrans(Start, End, Col);
	return (TRANSL_Player << TRANSL_TYPE_SHIFT) + MAXPLAYERS;
	unguard;
}

//==========================================================================
//
//	R_GetCachedTranslation
//
//==========================================================================

VTextureTranslation* R_GetCachedTranslation(int TransNum, VLevel* Level)
{
	guard(R_GetCachedTranslation);
	int Type = TransNum >> TRANSL_TYPE_SHIFT;
	int Index = TransNum & ((1 << TRANSL_TYPE_SHIFT) - 1);
	VTextureTranslation* Tr;
	switch (Type)
	{
	case TRANSL_Standard:
		if (Index == 7)
		{
			Tr = &IceTranslation;
		}
		else
		{
			if (Index < 0 || Index >= NumTranslationTables)
			{
				return NULL;
			}
			Tr = TranslationTables[Index];
		}
		break;

	case TRANSL_Player:
		if (Index < 0 || Index >= MAXPLAYERS + 1)
		{
			return NULL;
		}
		Tr = PlayerTranslations[Index];
		break;

	case TRANSL_Level:
		if (!Level || Index < 0 || Index >= Level->Translations.Num())
		{
			return NULL;
		}
		Tr = Level->Translations[Index];
		break;

	case TRANSL_BodyQueue:
		if (!Level || Index < 0 || Index >= Level->BodyQueueTrans.Num())
		{
			return NULL;
		}
		Tr = Level->BodyQueueTrans[Index];
		break;

	case TRANSL_Decorate:
		if (Index < 0 || Index >= DecorateTranslations.Num())
		{
			return NULL;
		}
		Tr = DecorateTranslations[Index];
		break;

	case TRANSL_Blood:
		if (Index < 0 || Index >= BloodTranslations.Num())
		{
			return NULL;
		}
		Tr = BloodTranslations[Index];
		break;

	default:
		return NULL;
	}

	if (!Tr)
	{
		return NULL;
	}

	for (int i = 0; i < CachedTranslations.Num(); i++)
	{
		VTextureTranslation* Check = CachedTranslations[i];
		if (Check->Crc != Tr->Crc)
		{
			continue;
		}
		if (memcmp(Check->Palette, Tr->Palette, sizeof(Tr->Palette)))
		{
			continue;
		}
		return Check;
	}

	VTextureTranslation* Copy = new VTextureTranslation;
	*Copy = *Tr;
	CachedTranslations.Append(Copy);
	return Copy;
	unguard;
}

//==========================================================================
//
//	COMMAND TimeRefresh
//
//	For program optimization
//
//==========================================================================

COMMAND(TimeRefresh)
{
	guard(COMMAND TimeRefresh);
	int			i;
	double		start, stop, time, RenderTime, UpdateTime;
	float		startangle;

	if (!cl)
	{
		return;
	}

	startangle = cl->ViewAngles.yaw;

	RenderTime = 0;
	UpdateTime = 0;
	start = Sys_Time();
	for (i = 0; i < 128; i++)
	{
		cl->ViewAngles.yaw = (float)(i) * 360.0 / 128.0;

		Drawer->StartUpdate();

		RenderTime -= Sys_Time();
		R_RenderPlayerView();
		RenderTime += Sys_Time();

		UpdateTime -= Sys_Time();
		Drawer->Update();
		UpdateTime += Sys_Time();
	}
	stop = Sys_Time();
	time = stop - start;
	GCon->Logf("%f seconds (%f fps)", time, 128 / time);
	GCon->Logf("Render time %f, update time %f", RenderTime, UpdateTime);

	cl->ViewAngles.yaw = startangle;
	unguard;
}

//==========================================================================
//
//	V_Init
//
//==========================================================================

void V_Init()
{
	guard(V_Init);
	int DIdx = -1;
	for (int i = 0; i < DRAWER_MAX; i++)
	{
		if (!DrawerList[i])
			continue;
		//	Pick first available as default.
		if (DIdx == -1)
			DIdx = i;
		//	Check for user driver selection.
		if (DrawerList[i]->CmdLineArg && GArgs.CheckParm(DrawerList[i]->CmdLineArg))
			DIdx = i;
	}
	if (DIdx == -1)
		Sys_Error("No drawers are available");
	_driver = DIdx;
	GCon->Logf(NAME_Init, "Selected %s", DrawerList[DIdx]->Description);
	//	Create drawer.
	Drawer = DrawerList[DIdx]->Creator();
	Drawer->Init();
	unguard;
}

//==========================================================================
//
//	V_Shutdown
//
//==========================================================================

void V_Shutdown()
{
	guard(V_Shutdown);
	if (Drawer)
	{
		Drawer->Shutdown();
		delete Drawer;
		Drawer = NULL;
	}
	R_FreeModels();
	for (int i = 0; i < MAXPLAYERS + 1; i++)
	{
		if (PlayerTranslations[i])
		{
			delete PlayerTranslations[i];
			PlayerTranslations[i] = NULL;
		}
	}
	for (int i = 0; i < CachedTranslations.Num(); i++)
	{
		delete CachedTranslations[i];
		CachedTranslations[i] = NULL;
	}
	CachedTranslations.Clear();
	R_FreeSkyboxData();
	unguard;
}
