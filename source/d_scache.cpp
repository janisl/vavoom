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
//**	rasterization driver surface heap manager
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "d_local.h"

// MACROS ------------------------------------------------------------------

#define SURFCACHE_SIZE_AT_320X200	600*1024

#define GUARDSIZE       4

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

bool			d_roverwrapped;
surfcache_t		*d_initial_rover;
bool			r_cache_thrash;         // set if surface cache is thrashing

// PRIVATE DATA DEFINITIONS ------------------------------------------------

int				sc_size;
surfcache_t		*sc_rover, *sc_base;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	D_SurfaceCacheForRes
//
//==========================================================================

int D_SurfaceCacheForRes(int width, int height, int bpp)
{
	guard(D_SurfaceCacheForRes);
	int             size, pix, pixbytes;

	if (M_CheckParm("-surfcachesize"))
	{
		size = atoi(myargv[M_CheckParm("-surfcachesize") + 1]) * 1024;
		return size;
	}
	
	size = SURFCACHE_SIZE_AT_320X200;

	pixbytes = (bpp + 7) / 8;
	pix = width * height * pixbytes;
	if (pix > 64000)
		size += (pix - 64000) * 3;

	return size;
	unguard;
}

//==========================================================================
//
//	D_CheckCacheGuard
//
//==========================================================================

static void D_CheckCacheGuard(void)
{
	guard(D_CheckCacheGuard);
	byte		*s;
	int			i;

	s = (byte *)sc_base + sc_size;
	for (i = 0; i < GUARDSIZE; i++)
		if (s[i] != (byte)i)
			Sys_Error("D_CheckCacheGuard: failed");
	unguard;
}

//==========================================================================
//
//	D_ClearCacheGuard
//
//==========================================================================

static void D_ClearCacheGuard(void)
{
	guard(D_ClearCacheGuard);
	byte    *s;
	int             i;
	
	s = (byte *)sc_base + sc_size;
	for (i = 0; i < GUARDSIZE; i++)
		s[i] = (byte)i;
	unguard;
}

//==========================================================================
//
//	D_InitCaches
//
//==========================================================================

void D_InitCaches(void *buffer, int size)
{
	guard(D_InitCaches);
//	if (!msg_suppress_1)
		con << (size / 1024) << "k surface cache\n";

	sc_size = size - GUARDSIZE;
	sc_base = (surfcache_t *)buffer;
	sc_rover = sc_base;
	
	sc_base->next = NULL;
	sc_base->owner = NULL;
	sc_base->size = sc_size;
	
	D_ClearCacheGuard();
	unguard;
}

//==========================================================================
//
//	D_FlushCaches
//
//==========================================================================

void D_FlushCaches(bool free_blocks)
{
	guard(D_FlushCaches);
	surfcache_t     *c;
	
	if (!sc_base)
		return;

	if (free_blocks)
	{
		for (c = sc_base; c; c = c->next)
		{
			if (c->owner)
				*c->owner = NULL;
		}
	}
	
	sc_rover = sc_base;
	sc_base->next = NULL;
	sc_base->owner = NULL;
	sc_base->size = sc_size;
	unguard;
}

//==========================================================================
//
//	D_SCAlloc
//
//==========================================================================

surfcache_t *D_SCAlloc(int width, int height)
{
	guard(D_SCAlloc);
	surfcache_t		*newb;
	bool			wrapped_this_time;

	int size = width * height * PixelBytes;
	if ((width < 0) || (width > 256))
		Sys_Error("D_SCAlloc: bad cache width %d\n", width);

	if ((height < 0) || (height > 256))
		Sys_Error("D_SCAlloc: bad cache height %d\n", height);

	size = (int)&((surfcache_t *)0)->data[size];
	size = (size + 3) & ~3;
	if (size > sc_size)
		Sys_Error("D_SCAlloc: %i > cache size", size);

	// if there is not size bytes after the rover, reset to the start
	wrapped_this_time = false;

	if (!sc_rover || (byte *)sc_rover - (byte *)sc_base > sc_size - size)
	{
		if (sc_rover)
		{
			wrapped_this_time = true;
		}
		sc_rover = sc_base;
	}
		
	// colect and free surfcache_t blocks until the rover block is large enough
	newb = sc_rover;
	if (sc_rover->owner)
		*sc_rover->owner = NULL;
	
	while (newb->size < size)
	{
		// free another
		sc_rover = sc_rover->next;
		if (!sc_rover)
			Sys_Error("D_SCAlloc: hit the end of memory");
		if (sc_rover->owner)
			*sc_rover->owner = NULL;
			
		newb->size += sc_rover->size;
		newb->next = sc_rover->next;
	}

	// create a fragment out of any leftovers
	if (newb->size - size > 256)
	{
		sc_rover = (surfcache_t *)( (byte *)newb + size);
		sc_rover->size = newb->size - size;
		sc_rover->next = newb->next;
		sc_rover->width = 0;
		sc_rover->owner = NULL;
		newb->next = sc_rover;
		newb->size = size;
	}
	else
		sc_rover = newb->next;
	
	newb->width = width;
// DEBUG
	newb->height = height;

	newb->owner = NULL;              // should be set properly after return

	if (d_roverwrapped)
	{
		if (wrapped_this_time || (sc_rover >= d_initial_rover))
			r_cache_thrash = true;
	}
	else if (wrapped_this_time)
	{       
		d_roverwrapped = true;
	}

	D_CheckCacheGuard();   // DEBUG
	return newb;
	unguard;
}

//==========================================================================
//
//	TSoftwareDrawer::FreeSurfCache
//
//==========================================================================

void TSoftwareDrawer::FreeSurfCache(surfcache_t* cache)
{
	guard(TSoftwareDrawer::FreeSurfCache);
	*cache->owner = NULL;
	cache->owner = NULL;
	unguard;
}

//==========================================================================
//
//	D_SCDump
//
//==========================================================================

#if 0
void D_SCDump(void)
{
	surfcache_t             *test;

	for (test = sc_base ; test ; test = test->next)
	{
		if (test == sc_rover)
			printf("ROVER:\n");
		printf("%p : %i bytes     %i width\n",test, test->size, test->width);
	}
}
#endif

//**************************************************************************
//
//	$Log$
//	Revision 1.5  2002/03/20 19:11:21  dj_jl
//	Added guarding.
//
//	Revision 1.4  2002/01/07 12:16:42  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
