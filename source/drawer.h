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

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct rgb_t
{
	byte	r;
	byte	g;
	byte	b;
};

struct rgba_t
{
	byte	r;
	byte	g;
	byte	b;
	byte	a;
};

struct surfcache_t;

struct texinfo_t
{
	TVec			saxis;
	float			soffs;
	TVec			taxis;
	float			toffs;
	TVec			texorg;
	int				pic;
	//	0 for solid surfaces
	// translucency + 1 for masked surfaces
	int				translucency;
};

struct surface_t
{
	surface_t		*next;
	texinfo_t		*texinfo;
	TPlane			*plane;
	int				lightlevel;
	byte			*lightmap;
	rgb_t			*lightmap_rgb;
	int				dlightframe;
	int				dlightbits;
	int				count;
	short			texturemins[2];
	short			extents[2];
	surfcache_t		*cachespots[4];
	TVec			verts[1];
};

struct particle_t
{
	//	Drawing info
	TVec		org;	//	position
	dword		color;	//	ARGB color
	//	Handled by refresh
	particle_t	*next;	//	next in the list
	TVec		vel;	//	velocity
	float		die;	//	cl.time when particle will be removed
	int			user_fields[7];
};

struct refdef_t
{
	int			x;
 	int			y;
 	int			width;
 	int			height;
 	float		fovx;
 	float		fovy;
	boolean		drawworld;
};

struct model_t
{
	char		name[128];
	void		*data;		// only access through Mod_Extradata
};

class TDrawer
{
 public:
	virtual void Init(void) = 0;
	virtual void InitData(void) = 0;
	virtual bool SetResolution(int, int, int) = 0;
	virtual void InitResolution(void) = 0;
	virtual void NewMap(void) = 0;
	virtual void SetPalette(int) = 0;
	virtual void StartUpdate(void) = 0;
	virtual void Update(void) = 0;
	virtual void BeginDirectUpdate(void) = 0;
	virtual void EndDirectUpdate(void) = 0;
	virtual void Shutdown(void) = 0;
	virtual void* ReadScreen(int*, bool*) = 0;
	virtual void FreeSurfCache(surfcache_t*) = 0;

	//	Rendring stuff
	virtual void SetupView(const refdef_t*) = 0;
	virtual void WorldDrawing(void) = 0;
	virtual void EndView(void) = 0;

	//	Texture stuff
	virtual void InitTextures(void) = 0;
	virtual void SetTexture(int) = 0;
	virtual void SetSkyTexture(int, bool) = 0;
	virtual void SetFlat(int) = 0;

	//	Polygon drawing
	virtual void DrawPolygon(TVec*, int, int, int) = 0;
	virtual void DrawSkyPolygon(TVec*, int, int, float, int, float) = 0;
	virtual void DrawMaskedPolygon(TVec*, int, int, int) = 0;
	virtual void DrawSpritePolygon(TVec*, int, int, int, dword) = 0;
	virtual void DrawAliasModel(const TVec&, const TAVec&, model_t*, int, const char*, dword, int, bool) = 0;

	//	Particles
	virtual void StartParticles(void) = 0;
	virtual void DrawParticle(particle_t *) = 0;
	virtual void EndParticles(void) = 0;

	//	Drawing
	virtual void DrawPic(float, float, float, float, float, float, float, float, int, int) = 0;
	virtual void DrawPicShadow(float, float, float, float, float, float, float, float, int, int) = 0;
	virtual void FillRectWithFlat(float, float, float, float, float, float, float, float, const char*) = 0;
	virtual void FillRect(float, float, float, float, dword) = 0;
	virtual void ShadeRect(int, int, int, int, int) = 0;
	virtual void DrawConsoleBackground(int) = 0;
	virtual void DrawSpriteLump(float, float, float, float, int, int, boolean) = 0;

	//	Automap
	virtual void StartAutomap(void) = 0;
	virtual void DrawLine(int, int, dword, int, int, dword) = 0;
	virtual void EndAutomap(void) = 0;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern TDrawer			*Drawer;
extern TDrawer			*_SoftwareDrawer;
extern TDrawer			*_OpenGLDrawer;
extern TDrawer			*_Direct3DDrawer;

//**************************************************************************
//
//	$Log$
//	Revision 1.9  2001/09/12 17:31:27  dj_jl
//	Rectangle drawing and direct update for plugins
//
//	Revision 1.8  2001/09/05 12:21:42  dj_jl
//	Release changes
//	
//	Revision 1.7  2001/08/15 17:15:55  dj_jl
//	Drawer API changes, removed wipes
//	
//	Revision 1.6  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.5  2001/08/04 17:29:11  dj_jl
//	Added depth hack for weapon models
//	
//	Revision 1.4  2001/08/01 17:42:22  dj_jl
//	Fixed sprite lump drawing in player setup menu, beautification
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
