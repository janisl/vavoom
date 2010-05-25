//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: r_main.cpp 4198 2010-03-28 18:22:57Z dj_jl $
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

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern subsector_t*		r_oldviewleaf;
extern VCvarI			precache;
extern bool				set_resolutioon_needed;
extern VCvarI			screen_size;
extern VCvarF			fov;
extern float			old_fov;
extern int				prev_old_aspect;
extern TVec				clip_base[4];
extern VCvarI			r_fog_test;
extern VTextureTranslation*	PlayerTranslations[MAXPLAYERS + 1];
extern VCvarI			r_dynamic;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VAdvancedRenderLevel::VAdvancedRenderLevel
//
//==========================================================================

VAdvancedRenderLevel::VAdvancedRenderLevel(VLevel* ALevel)
: VRenderLevelShared(ALevel)
, c_subdivides(0)
, free_wsurfs(0)
, c_seg_div(0)
, AllocatedWSurfBlocks(0)
, AllocatedSubRegions(0)
, AllocatedDrawSegs(0)
, AllocatedSegParts(0)
, CurrentSky1Texture(-1)
, CurrentSky2Texture(-1)
, CurrentDoubleSky(false)
, CurrentLightning(false)
, Particles(0)
, ActiveParticles(0)
, FreeParticles(0)
, LightVis(NULL)
{
	guard(VAdvancedRenderLevel::VAdvancedRenderLevel);
	NeedsInfiniteFarClip = true;
	r_oldviewleaf = NULL;
	trans_sprites = MainTransSprites;

	memset(DLights, 0, sizeof(DLights));
	memset(MainTransSprites, 0, sizeof(MainTransSprites));

	VisSize = (Level->NumSubsectors + 7) >> 3;
	BspVis = new vuint8[VisSize];
	LightVis = new vuint8[VisSize];
	LightBspVis = new vuint8[VisSize];

	InitParticles();
	ClearParticles();

	screenblocks = 0;

	Drawer->NewMap();

	// preload graphics
	if (precache)
	{
		PrecacheLevel();
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
	//	Free fake floor data.
	for (int i = 0; i < Level->NumSectors; i++)
	{
		if (Level->Sectors[i].fakefloors)
		{
			delete Level->Sectors[i].fakefloors;
		}
	}

	for (int i = 0; i < Level->NumSubsectors; i++)
	{
		for (subregion_t* r = Level->Subsectors[i].regions; r; r = r->next)
		{
			FreeSurfaces(r->floor->surfs);
			delete r->floor;
			FreeSurfaces(r->ceil->surfs);
			delete r->ceil;
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
	delete[] AllocatedDrawSegs;
	delete[] AllocatedSegParts;

	delete[] Particles;
	delete[] BspVis;

	for (int i = 0; i < SideSkies.Num(); i++)
	{
		delete SideSkies[i];
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::ExecuteSetViewSize
//
//==========================================================================

void VAdvancedRenderLevel::ExecuteSetViewSize()
{
	guard(VAdvancedRenderLevel::ExecuteSetViewSize);
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

	if (old_aspect)
		PixelAspect = ((float)ScreenHeight * 320.0) / ((float)ScreenWidth * 200.0);
	else
		PixelAspect = ((float)ScreenHeight * 4.0) / ((float)ScreenWidth * 3.0);
	prev_old_aspect = old_aspect;

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
//	VAdvancedRenderLevel::TransformFrustum
//
//==========================================================================

void VAdvancedRenderLevel::TransformFrustum()
{
	guard(VAdvancedRenderLevel::TransformFrustum);
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
//	VAdvancedRenderLevel::SetupFrame
//
//==========================================================================

extern VCvarI			r_chasecam;
extern VCvarF			r_chase_dist;
extern VCvarF			r_chase_up;
extern VCvarF			r_chase_right;
extern VCvarI			r_chase_front;

void VAdvancedRenderLevel::SetupFrame()
{
	guard(VAdvancedRenderLevel::SetupFrame);
	// change the view size if needed
	if (screen_size != screenblocks || !screenblocks ||
		set_resolutioon_needed || old_fov != fov ||
		old_aspect != prev_old_aspect)
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
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::SetupCameraFrame
//
//==========================================================================

void VAdvancedRenderLevel::SetupCameraFrame(VEntity* Camera, VTexture* Tex, int FOV,
	refdef_t* rd)
{
	guard(VAdvancedRenderLevel::SetupCameraFrame);
	rd->width = Tex->GetWidth();
	rd->height = Tex->GetHeight();
	rd->y = 0;
	rd->x = 0;

	if (old_aspect)
		PixelAspect = ((float)rd->height * 320.0) / ((float)rd->width * 200.0);
	else
		PixelAspect = ((float)rd->height * 4.0) / ((float)rd->width * 3.0);

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
	set_resolutioon_needed = true;
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::MarkLeaves
//
//==========================================================================

void VAdvancedRenderLevel::MarkLeaves()
{
	guard(VAdvancedRenderLevel::MarkLeaves);
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
//  VAdvancedRenderLevel::RenderScene
//
//==========================================================================

void VAdvancedRenderLevel::RenderScene(const refdef_t* RD, const VViewClipper* Range)
{
	guard(VAdvancedRenderLevel::RenderScene);
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
	if (!FixedLight)
	{
		for (int i = 0; i < Lights.Num(); i++)
		{
			RenderLightShadows(RD, Range, Lights[i].origin, Lights[i].radius, Lights[i].colour);
		}
	}

	Drawer->DrawWorldTexturesPass();
	RenderMobjsTextures();

	Drawer->DrawWorldFogPass();
	RenderMobjsFog();
	Drawer->EndFogPass();

	RenderMobjs();

	DrawParticles();

	DrawTranslucentPolys();
	unguard;
}

//==========================================================================
//
//  VAdvancedRenderLevel::RenderPlayerView
//
//==========================================================================

void VAdvancedRenderLevel::RenderPlayerView()
{
	guard(VAdvancedRenderLevel::RenderPlayerView);
	if (!Level->LevelInfo)
	{
		return;
	}

	GTextureManager.Time = Level->Time;

	BuildPlayerTranslations();

	AnimateSky(host_frametime);

	UpdateParticles(host_frametime);

	r_dlightframecount = 1;

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
//	VAdvancedRenderLevel::UpdateCameraTexture
//
//==========================================================================

void VAdvancedRenderLevel::UpdateCameraTexture(VEntity* Camera, int TexNum, int FOV)
{
	guard(VAdvancedRenderLevel::UpdateCameraTexture);
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
//	VAdvancedRenderLevel::GetFade
//
//==========================================================================

vuint32 VAdvancedRenderLevel::GetFade(sec_region_t* Reg)
{
	guard(VAdvancedRenderLevel::GetFade);
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
// 	VAdvancedRenderLevel::PrecacheLevel
//
// 	Preloads all relevant graphics for the level.
//
//==========================================================================

void VAdvancedRenderLevel::PrecacheLevel()
{
	guard(VAdvancedRenderLevel::PrecacheLevel);
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
//	VAdvancedRenderLevel::GetTranslation
//
//==========================================================================

VTextureTranslation* VAdvancedRenderLevel::GetTranslation(int TransNum)
{
	guard(VAdvancedRenderLevel::GetTranslation);
	return R_GetCachedTranslation(TransNum, Level);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::BuildPlayerTranslations
//
//==========================================================================

void VAdvancedRenderLevel::BuildPlayerTranslations()
{
	guard(VAdvancedRenderLevel::BuildPlayerTranslations);
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
