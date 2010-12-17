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

#ifndef _DRAWER_H_
#define _DRAWER_H_

#define BLOCK_WIDTH					128
#define BLOCK_HEIGHT				128
#define NUM_BLOCK_SURFS				32
#define NUM_CACHE_BLOCKS			(8 * 1024)

struct surface_t;
struct surfcache_t;
struct mmdl_t;
struct VMeshModel;
class VPortal;

struct particle_t
{
	//	Drawing info
	TVec		org;	//	position
	vuint32		colour;	//	ARGB colour
	float		Size;
	//	Handled by refresh
	particle_t*	next;	//	next in the list
	TVec		vel;	//	velocity
	TVec		accel;	//	acceleration
	float		die;	//	cl.time when particle will be removed
	int			type;
	float		ramp;
	float		gravity;
};

struct refdef_t
{
	int			x;
	int			y;
	int			width;
	int			height;
	float		fovx;
	float		fovy;
	bool		drawworld;
	bool		DrawCamera;
};

struct surfcache_t
{
	int				s;			// position in light surface
	int				t;
	int				width;		// size
	int				height;
	surfcache_t*	bprev;		// line list in block
	surfcache_t*	bnext;
	surfcache_t*	lprev;		// cache list in line
	surfcache_t*	lnext;
	surfcache_t*	chain;		// list of drawable surfaces
	surfcache_t*	addchain;	// list of specular surfaces
	int				blocknum;	// light surface index
	surfcache_t**	owner;
	vuint32			Light;		// checked for strobe flash
	int				dlight;
	surface_t*		surf;
	vuint32			lastframe;
};

class VRenderLevelDrawer : public VRenderLevelPublic
{
public:
	bool			NeedsInfiniteFarClip;

	//	Lightmaps.
	rgba_t			light_block[NUM_BLOCK_SURFS][BLOCK_WIDTH * BLOCK_HEIGHT];
	bool			block_changed[NUM_BLOCK_SURFS];
	surfcache_t*	light_chain[NUM_BLOCK_SURFS];

	//	Specular lightmaps.
	rgba_t			add_block[NUM_BLOCK_SURFS][BLOCK_WIDTH * BLOCK_HEIGHT];
	bool			add_changed[NUM_BLOCK_SURFS];
	surfcache_t*	add_chain[NUM_BLOCK_SURFS];

	surface_t*		SimpleSurfsHead;
	surface_t*		SimpleSurfsTail;
	surface_t*		SkyPortalsHead;
	surface_t*		SkyPortalsTail;
	surface_t*		HorizonPortalsHead;
	surface_t*		HorizonPortalsTail;

	int			    PortalDepth;

	virtual bool BuildLightMap(surface_t*, int) = 0;
	virtual void CacheSurface(surface_t*) = 0;
};

class VDrawer
{
public:
	bool				HaveStencil;
	bool				HaveMultiTexture;

	VRenderLevelDrawer*	RendLev;

	VDrawer()
	: HaveStencil(false)
	, HaveMultiTexture(false)
	, RendLev(NULL)
	{}
	virtual ~VDrawer()
	{}

	virtual void Init() = 0;
	virtual bool SetResolution(int, int, int, bool) = 0;
	virtual void InitResolution() = 0;
	virtual void StartUpdate() = 0;
	virtual void Update() = 0;
	virtual void BeginDirectUpdate() = 0;
	virtual void EndDirectUpdate() = 0;
	virtual void Shutdown() = 0;
	virtual void* ReadScreen(int*, bool*) = 0;
	virtual void ReadBackScreen(int, int, rgba_t*) = 0;

	//	Rendring stuff
	virtual void SetupView(VRenderLevelDrawer*, const refdef_t*) = 0;
	virtual void SetupViewOrg() = 0;
	virtual void WorldDrawing() = 0;
	virtual void EndView() = 0;

	//	Texture stuff
	virtual void PrecacheTexture(VTexture*) = 0;

	//	Polygon drawing
	virtual void DrawSkyPolygon(surface_t*, bool, VTexture*, float, VTexture*,
		float, int) = 0;
	virtual void DrawMaskedPolygon(surface_t*, float, bool) = 0;
	virtual void DrawSpritePolygon(TVec*, VTexture*, float, bool,
		VTextureTranslation*, int, vuint32, vuint32, const TVec&, float,
		const TVec&, const TVec&, const TVec&) = 0;
	virtual void DrawAliasModel(const TVec&, const TAVec&, const TVec&,
		const TVec&, VMeshModel*, int, int, VTexture*, VTextureTranslation*, int,
		vuint32, vuint32, float, bool, bool, float, bool) = 0;
	virtual bool StartPortal(VPortal*, bool) = 0;
	virtual void EndPortal(VPortal*, bool) = 0;

	//	Particles
	virtual void StartParticles() = 0;
	virtual void DrawParticle(particle_t *) = 0;
	virtual void EndParticles() = 0;

	//	Drawing
	virtual void DrawPic(float, float, float, float, float, float, float,
		float, VTexture*, VTextureTranslation*, float) = 0;
	virtual void DrawPicShadow(float, float, float, float, float, float,
		float, float, VTexture*, float) = 0;
	virtual void FillRectWithFlat(float, float, float, float, float, float,
		float, float, VTexture*) = 0;
	virtual void FillRect(float, float, float, float, vuint32) = 0;
	virtual void ShadeRect(int, int, int, int, float) = 0;
	virtual void DrawConsoleBackground(int) = 0;
	virtual void DrawSpriteLump(float, float, float, float, VTexture*,
		VTextureTranslation*, bool) = 0;

	//	Automap
	virtual void StartAutomap() = 0;
	virtual void DrawLine(int, int, vuint32, int, int, vuint32) = 0;
	virtual void EndAutomap() = 0;

	//	Advanced drawing.
	virtual bool SupportsAdvancedRendering() = 0;
	virtual void DrawWorldAmbientPass() = 0;
	virtual void BeginShadowVolumesPass() = 0;
	virtual void BeginLightShadowVolumes() = 0;
	virtual void RenderSurfaceShadowVolume(surface_t*, TVec&, float) = 0;
	virtual void BeginLightPass(TVec&, float, vuint32) = 0;
	virtual void DrawSurfaceLight(surface_t*) = 0;
	virtual void DrawWorldTexturesPass() = 0;
	virtual void DrawWorldFogPass() = 0;
	virtual void EndFogPass() = 0;
	virtual void DrawAliasModelAmbient(const TVec&, const TAVec&, const TVec&,
		const TVec&, VMeshModel*, int, int, VTexture*, vuint32, float, bool) = 0;
	virtual void DrawAliasModelTextures(const TVec&, const TAVec&, const TVec&,
		const TVec&, VMeshModel*, int, int, VTexture*, VTextureTranslation*, int,
		float, bool) = 0;
	virtual void BeginModelsLightPass(TVec&, float, vuint32) = 0;
	virtual void DrawAliasModelLight(const TVec&, const TAVec&, const TVec&,
		const TVec&, VMeshModel*, int, int, VTexture*, float, bool) = 0;
	virtual void BeginModelsShadowsPass(TVec&, float) = 0;
	virtual void DrawAliasModelShadow(const TVec&, const TAVec&, const TVec&,
		const TVec&, VMeshModel*, int, int, float, bool, const TVec&, float) = 0;
	virtual void DrawAliasModelFog(const TVec&, const TAVec&, const TVec&,
		const TVec&, VMeshModel*, int, int, VTexture*, vuint32, float, bool) = 0;
};

//	Drawer types, menu system uses these numbers.
enum
{
	DRAWER_OpenGL,
	DRAWER_Direct3D,

	DRAWER_MAX
};

//	Drawer description.
struct FDrawerDesc
{
	const char*		Name;
	const char*		Description;
	const char*		CmdLineArg;
	VDrawer*		(*Creator)();

	FDrawerDesc(int Type, const char* AName, const char* ADescription,
		const char* ACmdLineArg, VDrawer* (*ACreator)());
};

//	Drawer driver declaration macro.
#define IMPLEMENT_DRAWER(TClass, Type, Name, Description, CmdLineArg) \
static VDrawer* Create##TClass() \
{ \
	return new TClass(); \
} \
FDrawerDesc TClass##Desc(Type, Name, Description, CmdLineArg, Create##TClass);

extern VDrawer			*Drawer;

#endif
