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
//	VSoftwareDrawer::SurfaceCacheForRes
//
//==========================================================================

int VSoftwareDrawer::SurfaceCacheForRes(int width, int height, int bpp)
{
	guard(VSoftwareDrawer::SurfaceCacheForRes);
	int             size, pix, pixbytes;

	const char* p = GArgs.CheckValue("-surfcachesize");
	if (p)
	{
		size = atoi(p) * 1024;
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
//	VSoftwareDrawer::CheckCacheGuard
//
//==========================================================================

void VSoftwareDrawer::CheckCacheGuard(void)
{
	guard(VSoftwareDrawer::CheckCacheGuard);
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
//	VSoftwareDrawer::ClearCacheGuard
//
//==========================================================================

void VSoftwareDrawer::ClearCacheGuard(void)
{
	guard(VSoftwareDrawer::ClearCacheGuard);
	byte    *s;
	int             i;
	
	s = (byte *)sc_base + sc_size;
	for (i = 0; i < GUARDSIZE; i++)
		s[i] = (byte)i;
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::InitCaches
//
//==========================================================================

void VSoftwareDrawer::InitCaches(void *buffer, int size)
{
	guard(VSoftwareDrawer::InitCaches);
//	if (!msg_suppress_1)
		GCon->Logf(NAME_Init, "%dk surface cache", size / 1024);

	sc_size = size - GUARDSIZE;
	sc_base = (surfcache_t *)buffer;
	sc_rover = sc_base;
	
	sc_base->next = NULL;
	sc_base->owner = NULL;
	sc_base->size = sc_size;
	
	ClearCacheGuard();
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::FlushCaches
//
//==========================================================================

void VSoftwareDrawer::FlushCaches(bool free_blocks)
{
	guard(VSoftwareDrawer::FlushCaches);
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
//	VSoftwareDrawer::SCAlloc
//
//==========================================================================

surfcache_t *VSoftwareDrawer::SCAlloc(int width, int height)
{
	guard(VSoftwareDrawer::SCAlloc);
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

	CheckCacheGuard();   // DEBUG
	return newb;
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::FreeSurfCache
//
//==========================================================================

void VSoftwareDrawer::FreeSurfCache(surfcache_t* cache)
{
	guard(VSoftwareDrawer::FreeSurfCache);
	*cache->owner = NULL;
	cache->owner = NULL;
	unguard;
}

//==========================================================================
//
//	VSoftwareDrawer::SCDump
//
//==========================================================================

void VSoftwareDrawer::SCDump(FOutputDevice& Ar)
{
	guard(VSoftwareDrawer::SCDump);
	for (surfcache_t* test = sc_base ; test ; test = test->next)
	{
		if (test == sc_rover)
			Ar.Log("ROVER:");
		Ar.Logf("%p : %i bytes     %i width",test, test->size, test->width);
	}
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.8  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
//	Revision 1.7  2002/11/16 17:11:15  dj_jl
//	Improving software driver class.
//	
//	Revision 1.6  2002/07/13 07:38:00  dj_jl
//	Added drawers to the object tree.
//	
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
