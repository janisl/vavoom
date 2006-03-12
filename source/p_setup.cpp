//**************************************************************************
//**
//**    ##   ##    ##    ##   ##   ####     ####   ###     ###
//**    ##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**     ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**     ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**      ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**       #    ##    ##    #      ####     ####   ##       ##
//**
//**    $Id$
//**
//**    Copyright (C) 1999-2002 JÆnis Legzdi·ý
//**
//**    This program is free software; you can redistribute it and/or
//**  modify it under the terms of the GNU General Public License
//**  as published by the Free Software Foundation; either version 2
//**  of the License, or (at your option) any later version.
//**
//**    This program is distributed in the hope that it will be useful,
//**  but WITHOUT ANY WARRANTY; without even the implied warranty of
//**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//**  GNU General Public License for more details.
//**
//**************************************************************************
//**
//**    Do all the WAD I/O, get map description, set up initial state and
//**  misc. LUTs.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"
#include "fmapdefs.h"
#ifdef CLIENT
#include "cl_local.h"
#endif

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int			GMaxEntities = 4096;

#ifdef CLIENT
level_t cl_level;
#endif
#ifdef SERVER
level_t level;
#endif

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  CalcLine
//
//==========================================================================

void CalcLine(line_t *line)
{
	guard(CalcLine);
	//	Calc line's slopetype
	line->dir = *line->v2 - *line->v1;
	if (!line->dir.x)
	{
		line->slopetype = ST_VERTICAL;
	}
	else if (!line->dir.y)
	{
		line->slopetype = ST_HORIZONTAL;
	}
	else
	{
		if (line->dir.y / line->dir.x > 0)
		{
			line->slopetype = ST_POSITIVE;
		}
		else
		{
			line->slopetype = ST_NEGATIVE;
		}
	}

	line->SetPointDir(*line->v1, line->dir);

	//	Calc line's bounding box
	if (line->v1->x < line->v2->x)
	{
		line->bbox[BOXLEFT] = line->v1->x;
		line->bbox[BOXRIGHT] = line->v2->x;
	}
	else
	{
		line->bbox[BOXLEFT] = line->v2->x;
		line->bbox[BOXRIGHT] = line->v1->x;
	}
	if (line->v1->y < line->v2->y)
	{
		line->bbox[BOXBOTTOM] = line->v1->y;
		line->bbox[BOXTOP] = line->v2->y;
	}
	else
	{
		line->bbox[BOXBOTTOM] = line->v2->y;
		line->bbox[BOXTOP] = line->v1->y;
	}
	unguard;
}

//==========================================================================
//
//  CalcSeg
//
//==========================================================================

void CalcSeg(seg_t *seg)
{
	guardSlow(CalcSeg);
	seg->Set2Points(*seg->v1, *seg->v2);
	unguardSlow;
}

#ifdef SERVER

//==========================================================================
//
//  SV_LoadLevel
//
//==========================================================================

void SV_LoadLevel(const char *MapName)
{
	guard(SV_LoadLevel);
	if (GLevel)
	{
		delete GLevel;
		GLevel = NULL;
	}

	GLevel = (VLevel*)VObject::StaticSpawnObject(VLevel::StaticClass(), PU_STATIC);
	GLevel->LevelFlags |= VLevel::LF_ForServer;

	GLevel->LoadMap(MapName);
	unguard;
}

#endif
#ifdef CLIENT

//==========================================================================
//
//	CL_LoadLevel
//
//==========================================================================

void CL_LoadLevel(const char *MapName)
{
	guard(CL_LoadLevel);
	if (GClLevel)
	{
		delete GClLevel;
		GClLevel = NULL;
	}

	GClLevel = (VLevel *)VObject::StaticSpawnObject(VLevel::StaticClass(), PU_STATIC);
	GClGame->GLevel = GClLevel;

	GClLevel->LoadMap(MapName);
	unguard;
}

#endif

//==========================================================================
//
//  AddExtraFloor
//
//==========================================================================

sec_region_t *AddExtraFloor(line_t *line, sector_t *dst)
{
	guard(AddExtraFloor);
	sec_region_t *region;
	sec_region_t *inregion;
	sector_t *src;

	src = line->frontsector;
	src->SectorFlags |= sector_t::SF_ExtrafloorSource;
	dst->SectorFlags |= sector_t::SF_HasExtrafloors;
	float floorz = src->floor.GetPointZ(dst->soundorg);
	float ceilz = src->ceiling.GetPointZ(dst->soundorg);
	for (inregion = dst->botregion; inregion; inregion = inregion->next)
	{
		float infloorz = inregion->floor->GetPointZ(dst->soundorg);
		float inceilz = inregion->ceiling->GetPointZ(dst->soundorg);
		if (infloorz <= ceilz && inceilz >= floorz)
		{
			region = Z_CNew<sec_region_t>(PU_LEVEL, 0);
			region->floor = inregion->floor;
			region->ceiling = &src->ceiling;
			region->params = &src->params;
			region->extraline = line;
			inregion->floor = &src->floor;
			if (inregion->prev)
			{
				inregion->prev->next = region;
			}
			else
			{
				dst->botregion = region;
			}
			region->prev = inregion->prev;
			region->next = inregion;
			inregion->prev = region;
			return region;
		}
	}
	GCon->Logf("Invalid extra floor, tag %d", dst->tag);
	return NULL;
	unguard;
}

//==========================================================================
//
//	SwapPlanes
//
//==========================================================================

void SwapPlanes(sector_t *s)
{
	guard(SwapPlanes);
	float tempHeight;
	int tempTexture;

	tempHeight = s->floorheight;
	tempTexture = s->floor.pic;

	//	Floor
	s->floorheight = s->ceilingheight;
	s->floor.dist = s->floorheight;
	s->floor.minz = s->floorheight;
	s->floor.maxz = s->floorheight;

	s->ceilingheight = tempHeight;
	s->ceiling.dist = -s->ceilingheight;
	s->ceiling.minz = s->ceilingheight;
	s->ceiling.maxz = s->ceilingheight;

	s->floor.pic = s->ceiling.pic;
	s->ceiling.pic = tempTexture;

	s->floor.base_pic = s->floor.pic;
	s->ceiling.base_pic = s->ceiling.pic;
	s->base_floorheight = s->floor.dist;
	s->base_ceilingheight = s->ceiling.dist;
	unguard;
}

//==========================================================================
//
//	CalcSecMinMaxs
//
//==========================================================================

void CalcSecMinMaxs(sector_t *sector)
{
	guard(CalcSecMinMaxs);
	float	minz;
	float	maxz;
	int		i;

	if (sector->floor.normal.z == 1.0)
	{
		//	Horisontal floor
		sector->floor.minz = sector->floor.dist;
		sector->floor.maxz = sector->floor.dist;
	}
	else
	{
		//	Sloped floor
		minz = 99999.0;
		maxz = -99999.0;
		for (i = 0; i < sector->linecount; i++)
		{
			float z;
			z = sector->floor.GetPointZ(*sector->lines[i]->v1);
			if (minz > z)
				minz = z;
			if (maxz < z)
				maxz = z;
		}
		sector->floor.minz = minz;
		sector->floor.maxz = maxz;
	}

	if (sector->ceiling.normal.z == -1.0)
	{
		//	Horisontal ceiling
		sector->ceiling.minz = -sector->ceiling.dist;
		sector->ceiling.maxz = -sector->ceiling.dist;
	}
	else
	{
		//	Sloped ceiling
		minz = 99999.0;
		maxz = -99999.0;
		for (i = 0; i < sector->linecount; i++)
		{
			float z;
			z = sector->ceiling.GetPointZ(*sector->lines[i]->v1);
			if (minz > z)
				minz = z;
			if (maxz < z)
				maxz = z;
		}
		sector->ceiling.minz = minz;
		sector->ceiling.maxz = maxz;
	}

	sector->floorheight = sector->floor.minz;
	sector->ceilingheight = sector->ceiling.maxz;
	unguard;
}

//**************************************************************************
//
//  $Log$
//  Revision 1.31  2006/03/12 12:54:49  dj_jl
//  Removed use of bitfields for portability reasons.
//
//  Revision 1.30  2006/02/15 23:28:18  dj_jl
//  Moved all server progs global variables to classes.
//
//  Revision 1.29  2006/02/13 18:34:34  dj_jl
//  Moved all server progs global functions to classes.
//
//  Revision 1.28  2006/02/09 22:35:54  dj_jl
//  Moved all client game code to classes.
//
//  Revision 1.27  2005/11/24 20:09:23  dj_jl
//  Removed unused fields from Object class.
//
//  Revision 1.26  2005/03/28 07:28:19  dj_jl
//  Transfer lighting and other BOOM stuff.
//
//  Revision 1.25  2003/07/04 15:00:28  dj_jl
//  Moving floors fix
//
//  Revision 1.24  2003/07/03 18:11:13  dj_jl
//  Moving extrafloors
//
//  Revision 1.23  2003/03/08 11:33:39  dj_jl
//  Got rid of some warnings.
//
//  Revision 1.22  2002/09/07 16:31:51  dj_jl
//  Added Level class.
//
//  Revision 1.21  2002/08/28 16:39:19  dj_jl
//  Implemented sector light color.
//
//  Revision 1.20  2002/08/24 14:51:50  dj_jl
//  Fixes for large blockmaps.
//
//  Revision 1.19  2002/07/27 18:10:11  dj_jl
//  Implementing Strife conversations.
//
//  Revision 1.18  2002/07/23 16:29:56  dj_jl
//  Replaced console streams with output device class.
//
//  Revision 1.17  2002/07/13 07:44:50  dj_jl
//  Added some error checks.
//
//  Revision 1.16  2002/01/11 08:09:34  dj_jl
//  Added sector plane swapping
//
//  Revision 1.15  2002/01/07 12:16:43  dj_jl
//  Changed copyright year
//
//  Revision 1.14  2001/12/18 19:08:12  dj_jl
//  Beautification
//
//  Revision 1.13  2001/12/01 17:48:09  dj_jl
//  Added behaviorsize
//
//  Revision 1.12  2001/10/22 17:25:55  dj_jl
//  Floatification of angles
//
//  Revision 1.11  2001/10/09 17:25:36  dj_jl
//  Fixed auxiliary maps
//
//  Revision 1.10  2001/10/08 17:33:01  dj_jl
//  Different client and server level structures
//
//  Revision 1.9  2001/09/20 16:27:02  dj_jl
//  Removed degenmobj
//
//  Revision 1.8  2001/09/14 16:52:14  dj_jl
//  Added dynamic build of GWA file
//
//  Revision 1.7  2001/09/12 17:36:20  dj_jl
//  Using new zone templates
//
//  Revision 1.6  2001/08/21 17:42:42  dj_jl
//  Removed -devmaps code, in devgame mode look for map in <gamedir>/maps
//	
//  Revision 1.5  2001/08/02 17:46:38  dj_jl
//  Added sending info about changed textures to new clients
//	
//  Revision 1.4  2001/08/01 17:37:34  dj_jl
//  Made walls check texture list before flats
//	
//  Revision 1.3  2001/07/31 17:16:31  dj_jl
//  Just moved Log to the end of file
//	
//  Revision 1.2  2001/07/27 14:27:54  dj_jl
//  Update with Id-s and Log-s, some fixes
//
//**************************************************************************
