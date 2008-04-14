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

#include "cl_local.h"
#include "r_shared.h"

#define BLOCK_WIDTH					128
#define BLOCK_HEIGHT				128
#define NUM_BLOCK_SURFS				32
#define NUM_CACHE_BLOCKS			(8 * 1024)

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

class VHardwareDrawer : public VDrawer
{
public:
	VHardwareDrawer();

	void InitData();
	void NewMap();
	void FreeSurfCache(surfcache_t*);

protected:
	VRenderLevelDrawer*	RendLev;

	//	Lightmaps.
	rgba_t			light_block[NUM_BLOCK_SURFS][BLOCK_WIDTH * BLOCK_HEIGHT];
	bool			block_changed[NUM_BLOCK_SURFS];
	surfcache_t*	light_chain[NUM_BLOCK_SURFS];

	//	Specular lightmaps.
	rgba_t			add_block[NUM_BLOCK_SURFS][BLOCK_WIDTH * BLOCK_HEIGHT];
	bool			add_changed[NUM_BLOCK_SURFS];
	surfcache_t*	add_chain[NUM_BLOCK_SURFS];

	//	Surface cache.
	surfcache_t*	freeblocks;
	surfcache_t*	cacheblocks[NUM_BLOCK_SURFS];
	surfcache_t		blockbuf[NUM_CACHE_BLOCKS];
	vuint32			cacheframecount;

	surface_t*		SimpleSurfsHead;
	surface_t*		SimpleSurfsTail;
	surface_t*		SkyPortalsHead;
	surface_t*		SkyPortalsTail;
	surface_t*		HorizonPortalsHead;
	surface_t*		HorizonPortalsTail;

	enum { SHADEDOT_QUANT = 16 };
	static float	r_avertexnormal_dots[SHADEDOT_QUANT][256];

	void FlushCaches(bool);
	void FlushOldCaches();
	surfcache_t	*AllocBlock(int, int);
	surfcache_t	*FreeBlock(surfcache_t*, bool);
	void CacheSurface(surface_t*);

	static int ToPowerOf2(int val);
	void AdjustGamma(rgba_t*, int);
	void ResampleTexture(int, int, const vuint8*, int, int, vuint8*);
	void MipMap(int, int, vuint8*);
};
