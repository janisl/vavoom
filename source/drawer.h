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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:54  dj_jl
//**	Update with Id-s and Log-s, some fixes
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
	virtual void Shutdown(void) = 0;
	virtual void* ReadScreen(int*, bool*) = 0;
	virtual void FreeSurfCache(surfcache_t*) = 0;

	//	Screen wipes
	virtual bool InitWipe(void) = 0;
	virtual void DoWipe(int) = 0;

	//	Rendring stuff
	virtual void SetupView(int, int, int, int, float, float) = 0;
	virtual void SetupFrame(void) = 0;
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
	virtual void DrawAliasModel(const TVec&, const TAVec&, model_t*, int, int, dword, int) = 0;

	//	Particles
	virtual void StartParticles(void) = 0;
	virtual void DrawParticle(particle_t *) = 0;
	virtual void EndParticles(void) = 0;

	//	Drawing
	virtual void DrawPic(float, float, float, float, float, float, float, float, int, int) = 0;
	virtual void DrawPicShadow(float, float, float, float, float, float, float, float, int, int) = 0;
	virtual void FillRectWithFlat(int, int, int, int, const char*) = 0;
	virtual void ShadeRect(int, int, int, int, int) = 0;
	virtual void DrawConsoleBackground(int) = 0;
	virtual void DrawSpriteLump(int, int, int, int, boolean) = 0;

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

