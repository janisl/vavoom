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

void R_InitData(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

void R_InitParticles(void);
void R_ClearParticles(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int						screenblocks = 0;

TVec					vieworg;
TVec					viewforward;
TVec					viewright;
TVec					viewup;
TAVec					viewangles;

// bumped light from gun blasts
int                     extralight;
int						fixedlight;

TClipPlane				view_clipplanes[4];

int						r_visframecount;

TCvarI					r_fog("r_fog", "0");
TCvarF					r_fog_r("r_fog_r", "0.5");
TCvarF					r_fog_g("r_fog_g", "0.5");
TCvarF					r_fog_b("r_fog_b", "0.5");
TCvarF					r_fog_start("r_fog_start", "1.0");
TCvarF					r_fog_end("r_fog_end", "2048.0");
TCvarF					r_fog_density("r_fog_density", "0.5");

TDrawer					*Drawer;
TDrawer					*_SoftwareDrawer = NULL;
TDrawer					*_OpenGLDrawer = NULL;
TDrawer					*_Direct3DDrawer = NULL;

bool					r_back2front;

refdef_t				refdef;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static TCvarI			screen_size("screen_size", "10", CVAR_ARCHIVE);
static boolean			set_resolutioon_needed = true;

// Angles in the SCREENWIDTH wide window.
static TCvarF			fov("fov", "90");
static float			old_fov = 90.0;

static TVec				clip_base[4];

subsector_t				*r_viewleaf;
subsector_t				*r_oldviewleaf;

// if true, load all graphics at start
static TCvarI			precache("precache", "1", CVAR_ARCHIVE);

static int				pf_DrawViewBorder;
static int				pf_UpdateParticle;
static int				pg_frametime;

static TCvarI			_driver("_driver", "0", CVAR_ROM);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  R_Init
//
//==========================================================================

void R_Init(void)
{
	r_back2front = (Drawer != _SoftwareDrawer);
	R_InitSkyBoxes();
	R_InitData();
	Drawer->InitTextures();
	Drawer->InitData();
	R_InitParticles();
	pf_DrawViewBorder = clpr.FuncNumForName("DrawViewBorder");
	pf_UpdateParticle = clpr.FuncNumForName("UpdateParticle");
	pg_frametime = clpr.GlobalNumForName("frametime");
}

//==========================================================================
//
//  R_Start
//
//==========================================================================

void R_Start(const mapInfo_t &info)
{
	r_oldviewleaf = NULL;

	R_ClearLights();
	R_ClearParticles();
	R_InitSky(info);

	r_fog = !stricmp(info.fadetable, "FOGMAP");

	screenblocks = 0;

	Drawer->NewMap();

	// preload graphics
	if (precache)
	{
		R_PrecacheLevel();
	}
	Drawer->SetPalette(0);
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
	if (blocks > 2)
    {
		screen_size = blocks;
	}
    set_resolutioon_needed = true;
}

//==========================================================================
//
//  COMMAND SizeDown
//
//==========================================================================

COMMAND(SizeDown)
{
	R_SetViewSize(screenblocks - 1);
	S_StartSoundName("MenuSwitch");
}

//==========================================================================
//
//  COMMAND SizeUp
//
//==========================================================================

COMMAND(SizeUp)
{
	R_SetViewSize(screenblocks + 1);
	S_StartSoundName("MenuSwitch");
}

//==========================================================================
//
// 	R_ExecuteSetViewSize
//
//==========================================================================

static void R_ExecuteSetViewSize(void)
{
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

	refdef.fovx = tan(DEG2RAD(fov) / 2);
	refdef.fovy = refdef.fovx * refdef.height / refdef.width / PixelAspect;

	// left side clip
	clip_base[0] = Normalize(TVec(1, 1.0 / refdef.fovx, 0));
	
	// right side clip
	clip_base[1] = Normalize(TVec(1, -1.0 / refdef.fovx, 0));
	
	// top side clip
	clip_base[2] = Normalize(TVec(1, 0, -1.0 / refdef.fovy));
	
	// bottom side clip
	clip_base[3] = Normalize(TVec(1, 0, 1.0 / refdef.fovy));

	refdef.drawworld = true;
}

//==========================================================================
//
//	R_DrawViewBorder
//
//==========================================================================

void R_DrawViewBorder(void)
{
	clpr.Exec(pf_DrawViewBorder, 160 - screenblocks * 16,
		(200 - sb_height - screenblocks * (200 - sb_height) / 10) / 2,
		screenblocks * 32, screenblocks * (200 - sb_height) / 10);
}

//==========================================================================
//
//	R_TransformFrustum
//
//==========================================================================

static void R_TransformFrustum(void)
{
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
}

//==========================================================================
//
//  R_SetupFrame
//
//==========================================================================

TCvarI			r_chasecam("r_chasecam", "0", CVAR_ARCHIVE);
TCvarF			r_chase_dist("r_chase_dist", "32.0", CVAR_ARCHIVE);
TCvarF			r_chase_up("r_chase_up", "32.0", CVAR_ARCHIVE);
TCvarF			r_chase_right("r_chase_right", "0", CVAR_ARCHIVE);
TCvarI			r_chase_front("r_chase_front", "0", CVAR_ARCHIVE);

static void R_SetupFrame(void)
{
    // change the view size if needed
	if (screen_size != screenblocks || !screenblocks ||
		set_resolutioon_needed || old_fov != fov)
    {
		R_ExecuteSetViewSize();
    }

	viewangles = cl.viewangles;
	if (r_chasecam && r_chase_front)
	{
		//	This is used to see how weapon looks in player's hands
		viewangles.yaw = AngleMod(viewangles.yaw + 180);
		viewangles.pitch = -viewangles.pitch;
	}
	AngleVectors(viewangles, viewforward, viewright, viewup);

	if (r_chasecam)
	{
		vieworg = cl_mobjs[cl.clientnum + 1].origin + TVec(0.0, 0.0, 32.0)
			- r_chase_dist * viewforward + r_chase_up * viewup
			+ r_chase_right * viewright;
	}
	else
	{
		vieworg = cl.vieworg;
	}

	R_TransformFrustum();

    extralight = cl.extralight;
	if (cl.fixedcolormap >= 32)
	{
		fixedlight = 255;
	}
	else if (cl.fixedcolormap)
	{
		fixedlight = 255 - (cl.fixedcolormap << 3);
	}
	else
	{
		fixedlight = 0;
	}

	r_viewleaf = CL_PointInSubsector(cl.vieworg.x, cl.vieworg.y);

	Drawer->SetupView(&refdef);
}

//==========================================================================
//
//	R_MarkLeaves
//
//==========================================================================

static void R_MarkLeaves(void)
{
	byte	*vis;
	node_t	*node;
	int		i;

	if (r_oldviewleaf == r_viewleaf)
		return;
	
	r_visframecount++;
	r_oldviewleaf = r_viewleaf;

	vis = LeafPVS(cl_level, r_viewleaf);

	for (i = 0; i < cl_level.numsubsectors; i++)
	{
		if (vis[i >> 3] & (1 << (i & 7)))
		{
			subsector_t *sub = &cl_level.subsectors[i];
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

void R_InitParticles(void)
{
	int		i;

	i = M_CheckParm("-particles");

	if (i)
	{
		r_numparticles = atoi(myargv[i + 1]);
		if (r_numparticles < ABSOLUTE_MIN_PARTICLES)
			r_numparticles = ABSOLUTE_MIN_PARTICLES;
	}
	else
	{
		r_numparticles = MAX_PARTICLES;
	}

	particles = (particle_t *)Z_Malloc(r_numparticles * sizeof(particle_t));
}

//==========================================================================
//
//	R_ClearParticles
//
//==========================================================================

void R_ClearParticles(void)
{
	int		i;
	
	free_particles = &particles[0];
	active_particles = NULL;

	for (i = 0; i < r_numparticles; i++)
		particles[i].next = &particles[i + 1];
	particles[r_numparticles - 1].next = NULL;
}

//==========================================================================
//
//	R_NewParticle
//
//==========================================================================

particle_t *R_NewParticle(void)
{
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
}

//==========================================================================
//
//	R_DrawParticles
//
//==========================================================================

void R_DrawParticles(void)
{
	particle_t		*p, *kill;
	float			frametime;

	Drawer->StartParticles();

//	frametime = cl.time - cl.oldtime;
	frametime = host_frametime;
	clpr.SetGlobal(pg_frametime, PassFloat(frametime));
	
	kill = active_particles;
	while (kill && kill->die < cl.time)
	{
		active_particles = kill->next;
		kill->next = free_particles;
		free_particles = kill;
		kill = active_particles;
	}

	for (p = active_particles; p; p = p->next)
	{
		kill = p->next;
		while (kill && kill->die < cl.time)
		{
			p->next = kill->next;
			kill->next = free_particles;
			free_particles = kill;
			kill = p->next;
		}

		Drawer->DrawParticle(p);

		p->org += p->vel * frametime;

		clpr.Exec(pf_UpdateParticle, (int)p);
	}

	Drawer->EndParticles();
}

//==========================================================================
//
//  R_RenderPlayerView
//
//==========================================================================

void R_RenderPlayerView(void)
{
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
	int			i;
	double		start, stop, time;
	float		startangle;

	startangle = cl.viewangles.yaw;
	
	start = Sys_Time();
	for (i = 0; i < 128; i++)
	{
		cl.viewangles.yaw = (float)(i) * 360.0 / 128.0;

		Drawer->StartUpdate();

		R_RenderPlayerView();

		Drawer->Update();
	}
	stop = Sys_Time();
	time = stop - start;
	con << time << " seconds (" << (128 / time) << " fps)\n";
	
	cl.viewangles.yaw = startangle;
}

//==========================================================================
//
//	V_Init
//
//==========================================================================

void V_Init(void)
{
	if (M_CheckParm("-d3d"))
	{
		Drawer = _Direct3DDrawer;
		_driver = 2;
		if (!Drawer)
		{
			Sys_Error("Direct3D drawer is not available");
		}
	}
	else if (M_CheckParm("-opengl"))
	{
		Drawer = _OpenGLDrawer;
		_driver = 1;
		if (!Drawer)
		{
			Sys_Error("OpenGL drawer is not available");
		}
	}
	else
	{
		Drawer = _SoftwareDrawer;
		_driver = 0;
		if (!Drawer)
		{
			Sys_Error("Software drawer is not available");
		}
	}
	Drawer->Init();
}

//==========================================================================
//
//	V_Shutdown
//
//==========================================================================

void V_Shutdown(void)
{
	if (Drawer)
	{
		Drawer->Shutdown();
		Drawer = NULL;
	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.17  2002/01/25 18:08:19  dj_jl
//	Beautification
//
//	Revision 1.16  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.15  2001/12/27 17:36:47  dj_jl
//	Some speedup
//	
//	Revision 1.14  2001/11/09 14:22:09  dj_jl
//	R_InitTexture now called from Host_init
//	
//	Revision 1.13  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.12  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.11  2001/10/12 17:31:13  dj_jl
//	no message
//	
//	Revision 1.10  2001/10/08 17:34:57  dj_jl
//	A lots of small changes and cleanups
//	
//	Revision 1.9  2001/08/30 17:39:51  dj_jl
//	Moved view border and message box to progs
//	
//	Revision 1.8  2001/08/23 17:47:22  dj_jl
//	Started work on pics with custom palettes
//	
//	Revision 1.7  2001/08/21 17:43:49  dj_jl
//	Moved precache to r_main.cpp
//	
//	Revision 1.6  2001/08/15 17:29:05  dj_jl
//	Beautification
//	
//	Revision 1.5  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.4  2001/08/04 17:28:26  dj_jl
//	Removed game.h
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
