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

void R_InitData();
void R_FreeSkyboxData();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

void R_InitParticles();
void R_ClearParticles();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int						screenblocks = 0;

TVec					vieworg;
TVec					viewforward;
TVec					viewright;
TVec					viewup;	
TAVec					viewangles;

// bumped light from gun blasts
int						extralight;
int						fixedlight;

TClipPlane				view_clipplanes[4];

int						r_visframecount;

VCvarI					r_fog("r_fog", "0");
VCvarF					r_fog_r("r_fog_r", "0.5");
VCvarF					r_fog_g("r_fog_g", "0.5");
VCvarF					r_fog_b("r_fog_b", "0.5");
VCvarF					r_fog_start("r_fog_start", "1.0");
VCvarF					r_fog_end("r_fog_end", "2048.0");
VCvarF					r_fog_density("r_fog_density", "0.5");

VCvarI					r_draw_particles("r_draw_particles", "1", CVAR_Archive);

VCvarI					old_aspect("r_old_aspect_ratio", "1", CVAR_Archive);

VDrawer					*Drawer;

refdef_t				refdef;

float					PixelAspect;

//
//	Translation tables
//
vuint8*					translationtables;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static FDrawerDesc		*DrawerList[DRAWER_MAX];

static VCvarI			screen_size("screen_size", "10", CVAR_Archive);
static bool				set_resolutioon_needed = true;

// Angles in the SCREENWIDTH wide window.
static VCvarF			fov("fov", "90");
static float			old_fov = 90.0;

static int				prev_old_aspect;

static TVec				clip_base[4];

subsector_t				*r_viewleaf;
subsector_t				*r_oldviewleaf;

// if true, load all graphics at start
static VCvarI			precache("precache", "1", CVAR_Archive);

static VCvarI			_driver("_driver", "0", CVAR_Rom);

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
	R_InitData();
	R_InitModels();
	Drawer->InitTextures();
	Drawer->InitData();
	R_InitParticles();
	unguard;
}

//==========================================================================
//
//  R_Start
//
//==========================================================================

void R_Start()
{
	guard(R_Start);
	r_oldviewleaf = NULL;

	R_ClearLights();
	R_ClearParticles();
	R_InitSky();

	r_fog = cl_level.FadeTable == NAME_fogmap;

	screenblocks = 0;

	Drawer->NewMap();

	// preload graphics
	if (precache)
	{
		R_PrecacheLevel();
	}
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
		TVec(0, 0, 0), TVec(0, 0, 0), 0, 0, 1, 0);
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
		TVec(0, 0, 0), TVec(0, 0, 0), 0, 0, 1, 0);
}

//==========================================================================
//
// 	R_ExecuteSetViewSize
//
//==========================================================================

static void R_ExecuteSetViewSize()
{
	guard(R_ExecuteSetViewSize);
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
//	R_DrawViewBorder
//
//==========================================================================

void R_DrawViewBorder()
{
	guard(R_DrawViewBorder);
	GClGame->eventDrawViewBorder(320 - screenblocks * 32,
		(480 - sb_height - screenblocks * (480 - sb_height) / 10) / 2,
		screenblocks * 64, screenblocks * (480 - sb_height) / 10);
	unguard;
}

//==========================================================================
//
//	R_TransformFrustum
//
//==========================================================================

static void R_TransformFrustum()
{
	guard(R_TransformFrustum);
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
//  R_SetupFrame
//
//==========================================================================

VCvarI			r_chasecam("r_chasecam", "0", CVAR_Archive);
VCvarF			r_chase_dist("r_chase_dist", "32.0", CVAR_Archive);
VCvarF			r_chase_up("r_chase_up", "32.0", CVAR_Archive);
VCvarF			r_chase_right("r_chase_right", "0", CVAR_Archive);
VCvarI			r_chase_front("r_chase_front", "0", CVAR_Archive);

static void R_SetupFrame()
{
	guard(R_SetupFrame);
	// change the view size if needed
	if (screen_size != screenblocks || !screenblocks ||
		set_resolutioon_needed || old_fov != fov ||
		old_aspect != prev_old_aspect)
	{
		R_ExecuteSetViewSize();
	}

	viewangles = cl->ViewAngles;
	if (r_chasecam && r_chase_front)
	{
		//	This is used to see how weapon looks in player's hands
		viewangles.yaw = AngleMod(viewangles.yaw + 180);
		viewangles.pitch = -viewangles.pitch;
	}
	AngleVectors(viewangles, viewforward, viewright, viewup);

	if (r_chasecam)
	{
		vieworg = cl_mobjs[cl->ClientNum + 1]->Origin + TVec(0.0, 0.0, 32.0)
			- r_chase_dist * viewforward + r_chase_up * viewup
			+ r_chase_right * viewright;
	}
	else
	{
		vieworg = cl->ViewOrg;
	}

	R_TransformFrustum();

	extralight = cl->ExtraLight;
	if (cl->FixedColourmap >= 32)
	{
		fixedlight = 255;
	}
	else if (cl->FixedColourmap)
	{
		fixedlight = 255 - (cl->FixedColourmap << 3);
	}
	else
	{
		fixedlight = 0;
	}

	r_viewleaf = CL_PointInSubsector(cl->ViewOrg.x, cl->ViewOrg.y);

	Drawer->SetupView(&refdef);
	unguard;
}

//==========================================================================
//
//	R_MarkLeaves
//
//==========================================================================

static void R_MarkLeaves()
{
	guard(R_MarkLeaves);
	byte	*vis;
	node_t	*node;
	int		i;

	if (r_oldviewleaf == r_viewleaf)
		return;
	
	r_visframecount++;
	r_oldviewleaf = r_viewleaf;

	vis = GClLevel->LeafPVS(r_viewleaf);

	for (i = 0; i < GClLevel->NumSubsectors; i++)
	{
		if (vis[i >> 3] & (1 << (i & 7)))
		{
			subsector_t *sub = &GClLevel->Subsectors[i];
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

//**************************************************************************
//**
//**	PARTICLES
//**
//**************************************************************************

#define MAX_PARTICLES			2048	// default max # of particles at one
										//  time
#define ABSOLUTE_MIN_PARTICLES	512		// no fewer than this no matter what's
										//  on the command line

particle_t	*active_particles, *free_particles;

particle_t	*particles;
int			r_numparticles;

//==========================================================================
//
//	R_InitParticles
//
//==========================================================================

void R_InitParticles()
{
	guard(R_InitParticles);
	const char* p = GArgs.CheckValue("-particles");

	if (p)
	{
		r_numparticles = atoi(p);
		if (r_numparticles < ABSOLUTE_MIN_PARTICLES)
			r_numparticles = ABSOLUTE_MIN_PARTICLES;
	}
	else
	{
		r_numparticles = MAX_PARTICLES;
	}

	particles = new particle_t[r_numparticles];
	unguard;
}

//==========================================================================
//
//	R_ClearParticles
//
//==========================================================================

void R_ClearParticles()
{
	guard(R_ClearParticles);
	int		i;
	
	free_particles = &particles[0];
	active_particles = NULL;

	for (i = 0; i < r_numparticles; i++)
		particles[i].next = &particles[i + 1];
	particles[r_numparticles - 1].next = NULL;
	unguard;
}

//==========================================================================
//
//	R_NewParticle
//
//==========================================================================

particle_t *R_NewParticle()
{
	guard(R_NewParticle);
	if (!free_particles)
	{
		//	No free particles
		return NULL;
	}
	//	Remove from list of free particles
	particle_t *p = free_particles;
	free_particles = p->next;
	//	Clean
	memset(p, 0, sizeof(*p));
	//	Add to active particles
	p->next = active_particles;
	active_particles = p;
	return p;
	unguard;
}

//==========================================================================
//
//	R_UpdateParticles
//
//==========================================================================

void R_UpdateParticles()
{
	guard(R_UpdateParticles);
	particle_t		*p, *kill;
	float			frametime;

//	frametime = cl->time - cl->oldtime;
	frametime = host_frametime;
	
	kill = active_particles;
	while (kill && kill->die < GClGame->time)
	{
		active_particles = kill->next;
		kill->next = free_particles;
		free_particles = kill;
		kill = active_particles;
	}

	for (p = active_particles; p; p = p->next)
	{
		kill = p->next;
		while (kill && kill->die < GClGame->time)
		{
			p->next = kill->next;
			kill->next = free_particles;
			free_particles = kill;
			kill = p->next;
		}

		p->org += p->vel * frametime;

		GClGame->eventUpdateParticle(p, frametime);
	}
	unguard;
}

//==========================================================================
//
//	R_DrawParticles
//
//==========================================================================

void R_DrawParticles()
{
	guard(R_DrawParticles);
	if (!r_draw_particles)
	{
		return;
	}
	Drawer->StartParticles();
	for (particle_t* p = active_particles; p; p = p->next)
	{
		Drawer->DrawParticle(p);
	}
	Drawer->EndParticles();
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
	R_UpdateParticles();

	R_SetupFrame();

	R_MarkLeaves();

	R_PushDlights();

	R_UpdateWorld();

	R_RenderWorld();

	R_RenderMobjs();

	R_DrawParticles();

	R_DrawTranslucentPolys();

	// draw the psprites on top of everything
	if (fov <= 90.0)
	{
		R_DrawPlayerSprites();
	}

	Drawer->EndView();

	// Draw croshair
	R_DrawCroshair();
	unguard;
}

//==========================================================================
//
//	R_DrawPic
//
//==========================================================================

void R_DrawPic(int x, int y, int handle, int trans)
{
	guard(R_DrawPic);
	picinfo_t	info;

	if (handle < 0)
	{
		return;
	}

	GTextureManager.GetTextureInfo(handle, &info);
	x -= info.xoffset;
	y -= info.yoffset;
	GTextureManager.Textures[handle]->GetPixels();
	Drawer->DrawPic(fScaleX * x, fScaleY * y,
		fScaleX * (x + info.width), fScaleY * (y + info.height),
		0, 0, info.width, info.height, handle, trans);
	unguard;
}

//==========================================================================
//
//	R_DrawShadowedPic
//
//==========================================================================

void R_DrawShadowedPic(int x, int y, int handle)
{
	guard(R_DrawShadowedPic);
	picinfo_t	info;

	if (handle < 0)
	{
		return;
	}

	GTextureManager.GetTextureInfo(handle, &info);
	x -= info.xoffset;
	y -= info.yoffset;
	Drawer->DrawPicShadow(fScaleX * (x + 2), fScaleY * (y + 2),
		fScaleX * (x + 2 + info.width), fScaleY * (y + 2 + info.height),
		0, 0, info.width, info.height, handle, 160);
	Drawer->DrawPic(fScaleX * x, fScaleY * y,
		fScaleX * (x + info.width), fScaleY * (y + info.height),
		0, 0, info.width, info.height, handle, 0);
	unguard;
}

//==========================================================================
//
//  R_FillRectWithFlat
//
// 	Fills rectangle with flat.
//
//==========================================================================

void R_FillRectWithFlat(int DestX, int DestY, int width, int height, const char* fname)
{
	guard(R_FillRectWithFlat);
	Drawer->FillRectWithFlat(fScaleX * DestX, fScaleY * DestY,
		fScaleX * (DestX + width), fScaleY * (DestY + height),
		0, 0, width, height, fname);
	unguard;
}

//==========================================================================
//
//	R_ShadeRect
//
//==========================================================================

void R_ShadeRect(int x, int y, int width, int height, int shade)
{
	guard(R_ShadeRect);
	Drawer->ShadeRect((int)(x * fScaleX), (int)(y * fScaleY),
		(int)((x + width) * fScaleX) - (int)(x * fScaleX),
		(int)((y + height) * fScaleY) - (int)(y * fScaleY), shade);
	unguard;
}

//==========================================================================
//
// 	R_PrecacheLevel
//
// 	Preloads all relevant graphics for the level.
//
//==========================================================================

void R_PrecacheLevel()
{
	guard(R_PrecacheLevel);
	int			i;

	if (cls.demoplayback)
		return;

#ifdef __GNUC__
	char texturepresent[GTextureManager.Textures.Num()];
#else
	char* texturepresent = (char*)Z_Malloc(GTextureManager.Textures.Num());
#endif
	memset(texturepresent, 0, GTextureManager.Textures.Num());

	for (i = 0; i < GClLevel->NumSectors; i++)
	{
		texturepresent[GClLevel->Sectors[i].floor.pic] = true;
		texturepresent[GClLevel->Sectors[i].ceiling.pic] = true;
	}
	
	for (i = 0; i < GClLevel->NumSides; i++)
	{
		texturepresent[GClLevel->Sides[i].toptexture] = true;
		texturepresent[GClLevel->Sides[i].midtexture] = true;
		texturepresent[GClLevel->Sides[i].bottomtexture] = true;
	}

	// Precache textures.
	for (i = 1; i < GTextureManager.Textures.Num(); i++)
	{
		if (texturepresent[i])
		{
			Drawer->SetTexture(i);
		}
	}

#ifndef __GNUC__
	Z_Free(texturepresent);
#endif
	unguard;
}

//==========================================================================
//
// 	InitTranslationTables
//
//==========================================================================

static void InitTranslationTables()
{
	guard(InitTranslationTables);
	VStream* Strm = W_CreateLumpReaderName(NAME_translat);
	int TabLen = Strm->TotalSize();
	translationtables = new vuint8[TabLen];
	Strm->Serialise(translationtables, TabLen);
	delete Strm;
	for (int i = 0; i < TabLen; i++)
	{
		if ((i & 0xff) == 0)
		{
			//	Make sure that 0 always maps to 0.
			translationtables[i] = 0;
		}
		else
		{
			//	Make sure that normal colours doesn't map to colour 0.
			if (translationtables[i] == 0)
				translationtables[i] = r_black_colour;
		}
	}
	unguard;
}

//==========================================================================
//
//	R_InitData
//
//==========================================================================

void R_InitData()
{
	guard(R_InitData);
	//	We use colour 0 as transparent colour, so we must find an alternate
	// index for black colour. In Doom, Heretic and Strife there is another
	// black colour, in Hexen it's almost black.
	//	I think that originaly Doom uses colour 255 as transparent colour,
	// but utilites created by others uses the alternate black colour and
	// these graphics can contain pixels of colour 255.
	//	Heretic and Hexen also uses colour 255 as transparent, even more - in
	// colourmaps it's maped to colour 0. Posibly this can cause problems
	// with modified graphics.
	//	Strife uses colour 0 as transparent. I already had problems with fact
	// that colour 255 is normal colour, now there shouldn't be any problems.
	VStream* Strm = W_CreateLumpReaderName(NAME_playpal);
	rgba_t* pal = r_palette;
	int best_dist = 0x10000;
	for (int i = 0; i < 256; i++)
	{
		*Strm << pal[i].r
			<< pal[i].g
			<< pal[i].b;
		if (i == 0)
		{
			pal[i].a = 0;
		}
		else
		{
			pal[i].a = 255;
			int dist = pal[i].r * pal[i].r + pal[i].g * pal[i].g +
				pal[i].b * pal[i].b;
			if (dist < best_dist)
			{
				r_black_colour = i;
				best_dist = dist;
			}
		}
	}
	delete Strm;

	InitTranslationTables();
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
	R_FreeSpriteData();
	R_FreeModels();
	if (particles)
	{
		delete[] particles;
	}
	if (translationtables)
	{
		Z_Free(translationtables);
	}
	R_FreeSkyboxData();
	R_ClipShutdown();
	unguard;
}
