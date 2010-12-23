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

#ifndef _R_LOCAL_H
#define _R_LOCAL_H

// HEADER FILES ------------------------------------------------------------

#include "cl_local.h"
#include "r_shared.h"

// MACROS ------------------------------------------------------------------

#define MAX_SPRITE_MODELS	10*1024

// TYPES -------------------------------------------------------------------

//
//  Sprites are patches with a special naming convention
// so they can be recognized by R_InitSprites.
//  The base name is NNNNFx or NNNNFxFx, with
// x indicating the rotation, x = 0, 1-7.
//  The sprite and frame specified by a thing_t
// is range checked at run time.
//  A sprite is a patch_t that is assumed to represent
// a three dimensional object and may have multiple
// rotations pre drawn.
//  Horizontal flipping is used to save space,
// thus NNNNF2F5 defines a mirrored patch.
//  Some sprites will only have one picture used
// for all views: NNNNF0
//
struct spriteframe_t
{
	// If false use 0 for any position.
	// Note: as eight entries are available,
	//  we might as well insert the same name eight times.
	bool		rotate;

	// Lump to use for view angles 0-7.
	short		lump[16];

	// Flip bit (1 = flip) to use for view angles 0-7.
	bool		flip[16];
};

//
// 	A sprite definition:
// a number of animation frames.
//
struct spritedef_t
{
	int				numframes;
	spriteframe_t*	spriteframes;
};

struct segpart_t
{
	segpart_t*		next;
	texinfo_t		texinfo;
	surface_t*		surfs;
	float			frontTopDist;
	float			frontBotDist;
	float			backTopDist;
	float			backBotDist;
	float			TextureOffset;
	float			RowOffset;
};

struct drawseg_t
{
	seg_t*			seg;
	drawseg_t*		next;

	segpart_t*		top;
	segpart_t*		mid;
	segpart_t*		bot;
	segpart_t*		topsky;
	segpart_t*		extra;

	surface_t*		HorizonTop;
	surface_t*		HorizonBot;
};

struct sec_surface_t
{
	sec_plane_t*	secplane;
	texinfo_t		texinfo;
	float			dist;
	float			XScale;
	float			YScale;
	float			Angle;
	surface_t*		surfs;
};

struct subregion_t
{
	sec_region_t*	secregion;
	subregion_t*	next;
	sec_plane_t*	floorplane;
	sec_plane_t*	ceilplane;
	sec_surface_t*	floor;
	sec_surface_t*	ceil;
	int				count;
	drawseg_t*		lines;
};

struct fakefloor_t
{
	sec_plane_t		floorplane;
	sec_plane_t		ceilplane;
	sec_params_t	params;
};

struct skysurface_t : surface_t
{
	TVec			__verts[3];
};

struct sky_t
{
	int 			texture1;
	int 			texture2;
	float			columnOffset1;
	float			columnOffset2;
	float			scrollDelta1;
	float			scrollDelta2;
	skysurface_t	surf;
	TPlane			plane;
	texinfo_t		texinfo;
};

class VSky
{
public:
	enum
	{
		VDIVS			= 8,
		HDIVS			= 16
	};

	sky_t			sky[HDIVS * VDIVS];
	int				NumSkySurfs;
	int				SideTex;
	bool			bIsSkyBox;
	bool			SideFlip;

	void InitOldSky(int, int, float, float, bool, bool, bool);
	void InitSkyBox(VName, VName);
	void Init(int, int, float, float, bool, bool, bool, bool);
	void Draw(int);
};

class VSkyPortal : public VPortal
{
public:
	VSky*			Sky;

	VSkyPortal(VRenderLevelShared* ARLev, VSky* ASky)
	: VPortal(ARLev)
	, Sky(ASky)
	{}
	bool NeedsDepthBuffer() const;
	bool IsSky() const;
	bool MatchSky(VSky*) const;
	void DrawContents();
};

class VSkyBoxPortal : public VPortal
{
public:
	VEntity*		Viewport;

	VSkyBoxPortal(VRenderLevelShared* ARLev, VEntity* AViewport)
	: VPortal(ARLev)
	, Viewport(AViewport)
	{}
	bool IsSky() const;
	bool MatchSkyBox(VEntity*) const;
	void DrawContents();
};

class VSectorStackPortal : public VPortal
{
public:
	VEntity*		Viewport;

	VSectorStackPortal(VRenderLevelShared* ARLev, VEntity* AViewport)
	: VPortal(ARLev)
	, Viewport(AViewport)
	{}
	bool MatchSkyBox(VEntity*) const;
	void DrawContents();
};

class VMirrorPortal : public VPortal
{
public:
	TPlane*			Plane;

	VMirrorPortal(VRenderLevelShared* ARLev, TPlane* APlane)
	: VPortal(ARLev)
	, Plane(APlane)
	{}
	bool MatchMirror(TPlane*) const;
	void DrawContents();
};

enum ERenderPass
{
	//	For regular renderer.
	RPASS_Normal,
	//	For advanced renderer.
	RPASS_Ambient,
	RPASS_ShadowVolumes,
	RPASS_Light,
	RPASS_Textures,
	RPASS_Fog,
	RPASS_NonShadow,
};

class VRenderLevelShared : public VRenderLevelDrawer
{
protected:
	friend class VPortal;
	friend class VSkyPortal;
	friend class VSkyBoxPortal;
	friend class VSectorStackPortal;
	friend class VMirrorPortal;

	enum
	{
		MAX_TRANS_SPRITES	= 256,

		MAX_PARTICLES			= 2048,	// default max # of particles at one
										//  time
		ABSOLUTE_MIN_PARTICLES	= 512,	// no fewer than this no matter what's
										//  on the command line
		MAX_DLIGHTS	= 32,
	};

	struct trans_sprite_t
	{
		TVec			Verts[4];
		union
		{
			surface_t*	surf;
			VEntity*	Ent;
		};
		int				lump;
		TVec			normal;
		union
		{
			float		pdist;
			float		TimeFrac;
		};
		TVec			saxis;
		TVec			taxis;
		TVec			texorg;
		float			Alpha;
		bool			Additive;
		int				translation;
		int				type;
		float			dist;
		vuint32			light;
		vuint32			Fade;
	};

	struct world_surf_t
	{
		surface_t*		Surf;
		vuint8			Type;
	};

	struct light_t
	{
		TVec			origin;
		float			radius;
		vuint32			colour;
		int				leafnum;
	};

	VLevel*			Level;

	VEntity*		ViewEnt;

	int				FrustumIndexes[5][6];
	int				MirrorLevel;
	int				PortalLevel;
	int				VisSize;
	vuint8*			BspVis;

	subsector_t*	r_viewleaf;
	subsector_t*	r_oldviewleaf;
	float			old_fov;
	int				prev_aspect_ratio;

	//	Bumped light from gun blasts
	int				ExtraLight;
	int				FixedLight;
	int				ColourMap;

	int				NumParticles;
	particle_t*		Particles;
	particle_t*		ActiveParticles;
	particle_t*		FreeParticles;

	//	Sky variables
	int				CurrentSky1Texture;
	int				CurrentSky2Texture;
	bool			CurrentDoubleSky;
	bool			CurrentLightning;
	VSky			BaseSky;

	//	World render variables
	VViewClipper			ViewClip;
	TArray<world_surf_t>	WorldSurfs;
	TArray<VPortal*>		Portals;
	TArray<VSky*>			SideSkies;

	trans_sprite_t*	trans_sprites;
	trans_sprite_t	MainTransSprites[MAX_TRANS_SPRITES];

	sec_plane_t		sky_plane;
	float			skyheight;
	surface_t*		free_wsurfs;
	void*			AllocatedWSurfBlocks;
	subregion_t*	AllocatedSubRegions;
	drawseg_t*		AllocatedDrawSegs;
	segpart_t*		AllocatedSegParts;

	//	Light variables
	TArray<light_t>	Lights;
	dlight_t		DLights[MAX_DLIGHTS];

	// Only regular renderer needs this.
	vuint32			cacheframecount;

	//	Moved here so that model rendering methods can be merged.
	TVec			CurrLightPos;
	float			CurrLightRadius;

	VRenderLevelShared(VLevel* ALevel);
	~VRenderLevelShared();

	virtual void RenderScene(const refdef_t*, const VViewClipper*) = 0;
	virtual void PushDlights() = 0;
	virtual vuint32 LightPoint(const TVec &p) = 0;
	virtual void InitSurfs(surface_t*, texinfo_t*, TPlane*, subsector_t*) = 0;
	virtual surface_t* SubdivideFace(surface_t*, const TVec&, const TVec*) = 0;
	virtual surface_t* SubdivideSeg(surface_t*, const TVec&, const TVec*) = 0;
	virtual void QueueWorldSurface(surface_t*) = 0;
	virtual void FreeSurfCache(surfcache_t*);

	//	General
	void ExecuteSetViewSize();
	void TransformFrustum();
	void SetupFrame();
	void SetupCameraFrame(VEntity*, VTexture*, int, refdef_t*);
	void MarkLeaves();
	void UpdateCameraTexture(VEntity*, int, int);
	vuint32 GetFade(sec_region_t*);
	void PrecacheLevel();
	VTextureTranslation* GetTranslation(int);
	void BuildPlayerTranslations();

	//	Particles
	void InitParticles();
	void ClearParticles();
	void UpdateParticles(float);
	void DrawParticles();

	//	Sky methods
	void InitSky();
	void AnimateSky(float);

	//	World BSP rendering
	void SetUpFrustumIndexes();
	void QueueSimpleSurf(surface_t*);
	void QueueSkyPortal(surface_t*);
	void QueueHorizonPortal(surface_t*);
	void DrawSurfaces(surface_t*, texinfo_t*, VEntity*, int, int, bool, bool);
	void RenderHorizon(drawseg_t*);
	void RenderMirror(drawseg_t*);
	void RenderLine(drawseg_t*);
	void RenderSecSurface(sec_surface_t*, VEntity*);
	void RenderSubRegion(subregion_t*);
	void RenderSubsector(int);
	void RenderBSPNode(int, float*, int);
	void RenderBspWorld(const refdef_t*, const VViewClipper*);
	void RenderPortals();

	//	Surf methods
	void SetupSky();
	void FlushSurfCaches(surface_t*);
	sec_surface_t* CreateSecSurface(subsector_t*, sec_plane_t*);
	void UpdateSecSurface(sec_surface_t*, sec_plane_t*, subsector_t*);
	surface_t* NewWSurf();
	void FreeWSurfs(surface_t*);
	surface_t* CreateWSurfs(TVec*, texinfo_t*, seg_t*, subsector_t*);
	int CountSegParts(seg_t*);
	void CreateSegParts(drawseg_t*, seg_t*);
	void UpdateRowOffset(segpart_t*, float);
	void UpdateTextureOffset(segpart_t*, float);
	void UpdateDrawSeg(drawseg_t*, bool);
	void CreateWorldSurfaces();
	void UpdateSubRegion(subregion_t*, bool);
	bool CopyPlaneIfValid(sec_plane_t*, const sec_plane_t*,
		const sec_plane_t*);
	void UpdateFakeFlats(sector_t*);
	void FreeSurfaces(surface_t*);
	void FreeSegParts(segpart_t*);

	//	Models
	bool DrawAliasModel(const TVec&, const TAVec&, float, float, VModel*,
		int, int, VTextureTranslation*, int, vuint32, vuint32, float, bool,
		bool, float, bool, ERenderPass);
	bool DrawAliasModel(const TVec&, const TAVec&, float, float, VState*,
		VState*, VTextureTranslation*, int, vuint32, vuint32, float, bool,
		bool, float, bool, ERenderPass);
	bool DrawEntityModel(VEntity*, vuint32, vuint32, float, bool, float,
		ERenderPass);
	bool CheckAliasModelFrame(VEntity*, float);

	//	Things
	void DrawTranslucentPoly(surface_t*, TVec*, int, int, float, bool, int,
		bool, vuint32, vuint32, const TVec&, float, const TVec&, const TVec&,
		const TVec&);
	void RenderSprite(VEntity*, vuint32, vuint32, float, bool);
	void RenderTranslucentAliasModel(VEntity*, vuint32, vuint32, float, bool,
		float);
	bool RenderAliasModel(VEntity*, vuint32, vuint32, float, bool,
		ERenderPass);
	void RenderThing(VEntity*, ERenderPass);
	void RenderMobjs(ERenderPass);
	void DrawTranslucentPolys();
	void RenderPSprite(VViewState*, float, vuint32, vuint32, float, bool);
	bool RenderViewModel(VViewState*, vuint32, vuint32, float, bool);
	void DrawPlayerSprites();
	void DrawCroshair();

public:
	particle_t* NewParticle();

	void RenderPlayerView();

	void SegMoved(seg_t*);
	void SetupFakeFloors(sector_t*);

	void AddStaticLight(const TVec&, float, vuint32);
	dlight_t* AllocDlight(VThinker*);
	void DecayLights(float);
};

class VRenderLevel : public VRenderLevelShared
{
private:
	int				c_subdivides;
	int				c_seg_div;

	//	Surface cache.
	surfcache_t*	freeblocks;
	surfcache_t*	cacheblocks[NUM_BLOCK_SURFS];
	surfcache_t		blockbuf[NUM_CACHE_BLOCKS];

	//	General
	void RenderScene(const refdef_t*, const VViewClipper*);

	//	Surf methods
	void InitSurfs(surface_t*, texinfo_t*, TPlane*, subsector_t*);
	surface_t* SubdivideFace(surface_t*, const TVec&, const TVec*);
	surface_t* SubdivideSeg(surface_t*, const TVec&, const TVec*);
	void UpdateSubsector(int, float*);
	void UpdateBSPNode(int, float*);
	void UpdateWorld(const refdef_t*, const VViewClipper*);

	//	Light methods
	static void CalcMinMaxs(surface_t*);
	float CastRay(const TVec&, const TVec&, float);
	static void CalcFaceVectors(surface_t*);
	void CalcPoints(surface_t*);
	void SingleLightFace(light_t*, surface_t*);
	void LightFace(surface_t*, subsector_t*);
	void MarkLights(dlight_t*, int, int);
	void AddDynamicLights(surface_t*);
	void PushDlights();
	void FlushCaches();
	void FlushOldCaches();
	surfcache_t* AllocBlock(int, int);
	surfcache_t	*FreeBlock(surfcache_t*, bool);
	void FreeSurfCache(surfcache_t*);
	void CacheSurface(surface_t*);

	//	World BSP rendering
	void QueueWorldSurface(surface_t*);
	void RenderWorld(const refdef_t*, const VViewClipper*);

public:
	VRenderLevel(VLevel*);

	void PreRender();

	vuint32 LightPoint(const TVec &p);
	bool BuildLightMap(surface_t*, int);
};

class VAdvancedRenderLevel : public VRenderLevelShared
{
private:
	VViewClipper			LightClip;
	byte*					LightVis;
	byte*					LightBspVis;
	vuint32					CurrLightColour;

	//	General
	void RenderScene(const refdef_t*, const VViewClipper*);

	//	Surf methods
	void InitSurfs(surface_t*, texinfo_t*, TPlane*, subsector_t*);
	surface_t* SubdivideFace(surface_t*, const TVec&, const TVec*);
	surface_t* SubdivideSeg(surface_t*, const TVec&, const TVec*);
	void UpdateSubsector(int, float*);
	void UpdateBSPNode(int, float*);
	void UpdateWorld();

	//	Light methods
	void PushDlights();
	vuint32 LightPointAmbient(const TVec &p);

	//	World BSP rendering
	void QueueWorldSurface(surface_t*);
	void RenderWorld(const refdef_t*, const VViewClipper*);

	void BuildLightVis(int bspnum, float* bbox);
	void DrawShadowSurfaces(surface_t* InSurfs, texinfo_t *texinfo,
		bool CheckSkyBoxAlways);
	void RenderShadowLine(drawseg_t* dseg);
	void RenderShadowSecSurface(sec_surface_t* ssurf, VEntity* SkyBox);
	void RenderShadowSubRegion(subregion_t* region);
	void RenderShadowSubsector(int num);
	void RenderShadowBSPNode(int bspnum, float* bbox);
	void DrawLightSurfaces(surface_t* InSurfs, texinfo_t *texinfo,
		VEntity* SkyBox, bool CheckSkyBoxAlways);
	void RenderLightLine(drawseg_t* dseg);
	void RenderLightSecSurface(sec_surface_t* ssurf, VEntity* SkyBox);
	void RenderLightSubRegion(subregion_t* region);
	void RenderLightSubsector(int num);
	void RenderLightBSPNode(int bspnum, float* bbox);
	void RenderLightShadows(const refdef_t* RD, const VViewClipper* Range,
		TVec& Pos, float Radius, vuint32 Colour);

	//	Things
	void RenderThingAmbient(VEntity*);
	void RenderMobjsAmbient();
	void RenderThingTextures(VEntity*);
	void RenderMobjsTextures();
	bool IsTouchedByLight(VEntity*);
	void RenderThingLight(VEntity*);
	void RenderMobjsLight();
	void RenderThingShadow(VEntity*);
	void RenderMobjsShadow();
	void RenderThingFog(VEntity*);
	void RenderMobjsFog();

public:
	VAdvancedRenderLevel(VLevel*);
	~VAdvancedRenderLevel();

	void PreRender();

	vuint32 LightPoint(const TVec &p);
	bool BuildLightMap(surface_t*, int);
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

//
// R_Sky
//
void R_InitSkyBoxes();

//
//	r_model
//
void R_InitModels();
void R_FreeModels();

int R_SetMenuPlayerTrans(int, int, int);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern spritedef_t		sprites[MAX_SPRITE_MODELS];

//
// R_Main
//
extern int      		screenblocks;
extern int				r_visframecount;

extern byte				light_remap[256];
extern VCvarI			r_darken;
extern VCvarI			r_dynamic;
extern VCvarI			r_static_lights;

extern refdef_t			refdef;

extern VCvarI			aspect_ratio;
extern VCvarI			r_interpolate_frames;

extern VTextureTranslation**		TranslationTables;
extern int							NumTranslationTables;
extern VTextureTranslation			IceTranslation;
extern TArray<VTextureTranslation*>	DecorateTranslations;
extern TArray<VTextureTranslation*>	BloodTranslations;

#endif
