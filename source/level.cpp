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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"
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

IMPLEMENT_CLASS(V, Level);

VLevel*			GLevel;
VLevel*			GClLevel;

int				GMaxEntities = 4096;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  VLevel::PointInSubsector
//
//==========================================================================

subsector_t* VLevel::PointInSubsector(const TVec &point) const
{
	guard(VLevel::PointInSubsector);
	node_t *node;
	int side;
	int nodenum;

	// single subsector is a special case
	if (!NumNodes)
		return Subsectors;

	nodenum = NumNodes - 1;

	while (!(nodenum & NF_SUBSECTOR))
	{
		node = &Nodes[nodenum];
		side = node->PointOnSide(point);
		nodenum = node->children[side];
	}
	return &Subsectors[nodenum & ~NF_SUBSECTOR];
	unguard;
}

//==========================================================================
//
//  VLevel::LeafPVS
//
//==========================================================================

byte *VLevel::LeafPVS(const subsector_t *ss) const
{
	guard(VLevel::LeafPVS);
	int sub = ss - Subsectors;
	if (VisData)
	{
		return VisData + (((NumSubsectors + 7) >> 3) * sub);
	}
	else
	{
		return NoVis;
	}
	unguard;
}

//==========================================================================
//
//	VLevel::Serialise
//
//==========================================================================

void VLevel::Serialise(VStream& Strm)
{
	guard(VLevel::Serialise);
	int i;
	int j;
	sector_t* sec;
	line_t* li;
	side_t* si;

	//
	//	Sectors
	//
	guard(Sectors);
	for (i = 0, sec = Sectors; i < NumSectors; i++, sec++)
	{
		Strm << sec->floor.dist
			<< sec->ceiling.dist
			<< sec->floor.pic
			<< sec->ceiling.pic
			<< sec->params.lightlevel
			<< sec->special
			<< sec->tag
			<< sec->seqType
			<< sec->SoundTarget
			<< sec->FloorData
			<< sec->CeilingData
			<< sec->LightingData
			<< sec->AffectorData
			<< sec->Damage
			<< sec->Friction
			<< sec->MoveFactor
			<< sec->Gravity;
		if (Strm.IsLoading())
		{
			CalcSecMinMaxs(sec);
		}
	}
	unguard;

	//
	//	Lines
	//
	guard(Lines);
	for (i = 0, li = Lines; i < NumLines; i++, li++)
	{
		//	Temporary hack to save seen on automap flags.
#ifdef CLIENT
		if (Strm.IsSaving() && cls.state == ca_connected)
		{
			li->flags |= GClLevel->Lines[i].flags & ML_MAPPED;
		}
#endif
		Strm << li->flags
			<< li->special
			<< li->arg1
			<< li->arg2
			<< li->arg3
			<< li->arg4
			<< li->arg5
			<< li->LineTag;
		for (j = 0; j < 2; j++)
		{
			if (li->sidenum[j] == -1)
			{
				continue;
			}
			si = &Sides[li->sidenum[j]];
			Strm << si->textureoffset
				<< si->rowoffset
				<< si->toptexture
				<< si->bottomtexture
				<< si->midtexture;
		}
	}
	unguard;

	//
	//	Polyobjs
	//
	guard(Polyobjs);
	for (i = 0; i < NumPolyObjs; i++)
	{
		if (Strm.IsLoading())
		{
			float angle, polyX, polyY;

			Strm << angle
				<< polyX 
				<< polyY;
			RotatePolyobj(PolyObjs[i].tag, angle);
			MovePolyobj(PolyObjs[i].tag,
				polyX - PolyObjs[i].startSpot.x,
				polyY - PolyObjs[i].startSpot.y);
		}
		else
		{
			Strm << PolyObjs[i].angle
				<< PolyObjs[i].startSpot.x
				<< PolyObjs[i].startSpot.y;
		}
		Strm << PolyObjs[i].SpecialData;
	}
	unguard;

	//
	//	Static lights
	//
	guard(StaticLights);
	Strm << STRM_INDEX(NumStaticLights);
	if (Strm.IsLoading())
	{
		if (StaticLights)
		{
			delete[] StaticLights;
			StaticLights = NULL;
		}
		if (NumStaticLights)
		{
			StaticLights = new rep_light_t[NumStaticLights];
		}
	}
	for (int i = 0; i < NumStaticLights; i++)
	{
		Strm << StaticLights[i].Origin
			<< StaticLights[i].Radius
			<< StaticLights[i].Colour;
	}
	unguard;

	//
	//	ACS
	//
	guard(ACS);
	Acs->Serialise(Strm);
	unguard;
	unguard;
}

//==========================================================================
//
//	VLevel::ClearReferences
//
//==========================================================================

void VLevel::ClearReferences()
{
	guard(VLevel::ClearReferences);
	Super::ClearReferences();
	for (int i = 0; i < NumSectors; i++)
	{
		sector_t* sec = Sectors + i;
		if (sec->SoundTarget && sec->SoundTarget->GetFlags() & _OF_CleanupRef)
			sec->SoundTarget = NULL;
		if (sec->FloorData && sec->FloorData->GetFlags() & _OF_CleanupRef)
			sec->FloorData = NULL;
		if (sec->CeilingData && sec->CeilingData->GetFlags() & _OF_CleanupRef)
			sec->CeilingData = NULL;
		if (sec->LightingData && sec->LightingData->GetFlags() & _OF_CleanupRef)
			sec->LightingData = NULL;
		if (sec->AffectorData && sec->AffectorData->GetFlags() & _OF_CleanupRef)
			sec->AffectorData = NULL;
	}
	for (int i = 0; i < NumPolyObjs; i++)
	{
		if (PolyObjs[i].SpecialData && PolyObjs[i].SpecialData->GetFlags() & _OF_CleanupRef)
			PolyObjs[i].SpecialData = NULL;
	}
	unguard;
}

//==========================================================================
//
//	VLevel::Destroy
//
//==========================================================================

void VLevel::Destroy()
{
	guard(VLevel::Destroy);
	//	Destroy all thinkers.
	DestroyAllThinkers();

	//	Free level data.
	if (RenderData)
	{
		delete RenderData;
	}

	for (int i = 0; i < NumPolyObjs; i++)
	{
		delete[] PolyObjs[i].segs;
		delete[] PolyObjs[i].originalPts;
		if (PolyObjs[i].prevPts)
		{
			delete[] PolyObjs[i].prevPts;
		}
	}
	if (PolyBlockMap)
	{
		for (int i = 0; i < BlockMapWidth * BlockMapHeight; i++)
		{
			for (polyblock_t* pb = PolyBlockMap[i]; pb;)
			{
				polyblock_t* Next = pb->next;
				delete pb;
				pb = Next;
			}
		}
		delete[] PolyBlockMap;
	}
	if (PolyObjs)
	{
		delete[] PolyObjs;
	}
	if (PolyAnchorPoints)
	{
		delete[] PolyAnchorPoints;
	}

	if (Sectors)
	{
		for (int i = 0; i < NumSectors; i++)
		{
			sec_region_t* Next;
			for (sec_region_t* r = Sectors[i].botregion; r; r = Next)
			{
				Next = r->next;
				delete r;
			}
		}
		delete[] Sectors[0].lines;
	}

	delete[] Vertexes;
	delete[] Sectors;
	delete[] Sides;
	delete[] Lines;
	delete[] Segs;
	delete[] Subsectors;
	delete[] Nodes;
	if (VisData)
	{
		delete[] VisData;
	}
	else
	{
		delete[] NoVis;
	}
	delete[] BlockMapLump;
	delete[] BlockLinks;
	delete[] RejectMatrix;
	delete[] Things;

	delete[] BaseSides;
	delete[] BaseSectors;
	delete[] BasePolyObjs;

	if (Acs)
	{
		delete Acs;
	}
	if (GenericSpeeches)
	{
		delete[] GenericSpeeches;
	}
	if (LevelSpeeches)
	{
		delete[] LevelSpeeches;
	}
	if (StaticLights)
	{
		delete[] StaticLights;
	}

	ActiveSequences.Clear();

	//	Call parent class's Destroy method.
	Super::Destroy();
	unguard;
}

//==========================================================================
//
//	VLevel::ClampOffsets
//
//==========================================================================

void VLevel::ClampOffsets()
{
	guard(VLevel::ClampOffsets);
	//	Clamp side offsets.
	for (int i = 0; i < NumSides; i++)
	{
		if (Sides[i].textureoffset > 0x7fff ||
			Sides[i].textureoffset < -0x8000)
		{
			Sides[i].textureoffset = 0;
		}
		if (Sides[i].rowoffset > 0x7fff || Sides[i].rowoffset < -0x8000)
		{
			Sides[i].rowoffset = 0;
		}
	}

	//	Clamp sector offsets.
	for (int i = 0; i < NumSectors; i++)
	{
		if (Sectors[i].floor.xoffs > 0x7fff ||
			Sectors[i].floor.xoffs < -0x8000)
		{
			Sectors[i].floor.xoffs = 0;
		}
		if (Sectors[i].floor.yoffs > 0x7fff ||
			Sectors[i].floor.yoffs < -0x8000)
		{
			Sectors[i].floor.yoffs = 0;
		}

		if (Sectors[i].ceiling.xoffs > 0x7fff ||
			Sectors[i].ceiling.xoffs < -0x8000)
		{
			Sectors[i].ceiling.xoffs = 0;
		}
		if (Sectors[i].ceiling.yoffs > 0x7fff ||
			Sectors[i].ceiling.yoffs < -0x8000)
		{
			Sectors[i].ceiling.yoffs = 0;
		}
	}
	unguard;
}

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

void SV_LoadLevel(VName MapName)
{
	guard(SV_LoadLevel);
#ifdef CLIENT
	if (GClLevel == GLevel)
	{
		GClLevel = NULL;
	}
#endif
	if (GLevel)
	{
		delete GLevel;
		GLevel = NULL;
	}

	GLevel = Spawn<VLevel>();
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

void CL_LoadLevel(VName MapName)
{
	guard(CL_LoadLevel);
	if (GClLevel)
	{
		delete GClLevel;
		GClLevel = NULL;
	}

	GClLevel = Spawn<VLevel>();
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
			region = new sec_region_t;
			memset(region, 0, sizeof(*region));
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

//==========================================================================
//
//	Natives
//
//==========================================================================

IMPLEMENT_FUNCTION(VLevel, PointInSector)
{
	P_GET_VEC(Point);
	P_GET_SELF;
	RET_PTR(Self->PointInSubsector(Point)->sector);
}

IMPLEMENT_FUNCTION(VLevel, ChangeSector)
{
	P_GET_INT(crunch);
	P_GET_PTR(sector_t, sec);
	P_GET_SELF;
	RET_BOOL(Self->ChangeSector(sec, crunch));
}

IMPLEMENT_FUNCTION(VLevel, AddExtraFloor)
{
	P_GET_PTR(sector_t, dst);
	P_GET_PTR(line_t, line);
	P_GET_SELF;
	RET_PTR(AddExtraFloor(line, dst));
}

IMPLEMENT_FUNCTION(VLevel, SwapPlanes)
{
	P_GET_PTR(sector_t, s);
	P_GET_SELF;
	SwapPlanes(s);
}

IMPLEMENT_FUNCTION(VLevel, SetFloorLightSector)
{
	P_GET_PTR(sector_t, SrcSector);
	P_GET_PTR(sector_t, Sector);
	P_GET_SELF;
	Sector->floor.LightSourceSector = SrcSector - Self->Sectors;
}

IMPLEMENT_FUNCTION(VLevel, SetCeilingLightSector)
{
	P_GET_PTR(sector_t, SrcSector);
	P_GET_PTR(sector_t, Sector);
	P_GET_SELF;
	Sector->ceiling.LightSourceSector = SrcSector - Self->Sectors;
}

IMPLEMENT_FUNCTION(VLevel, SetHeightSector)
{
	P_GET_INT(Flags);
	P_GET_PTR(sector_t, SrcSector);
	P_GET_PTR(sector_t, Sector);
	P_GET_SELF;
	if (Self->RenderData)
	{
		Self->RenderData->SetupFakeFloors(Sector);
	}
}

IMPLEMENT_FUNCTION(VLevel, FindSectorFromTag)
{
	P_GET_INT(start);
	P_GET_INT(tag);
	P_GET_SELF;
	int Ret = -1;
	for (int i = start + 1; i < Self->NumSectors; i++)
		if (Self->Sectors[i].tag == tag)
		{
			Ret = i;
			break;
		}
	RET_INT(Ret);
}
