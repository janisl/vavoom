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

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void BuildPVS(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

#ifdef CLIENT
cl_level_t cl_level;
#endif
#ifdef SERVER
sv_level_t level;
#endif

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static vertex_t *gl_vertexes;
static int numglvertexes;

static byte novis[32 * 1024 / 8];

static bool AuxiliaryMap;

static TCvarI ignore_missing_textures("ignore_missing_textures", "0");
static TCvarI strict_level_errors("strict_level_errors", "1");

// CODE --------------------------------------------------------------------

//==========================================================================
//
//  FTNumForName
//
//  Retrieval, get a texture or flat number for a name.
//
//==========================================================================

int FTNumForName(const char *name)
{
	guard(FTNumForName);
	char namet[9];
	int i;

	i = R_CheckFlatNumForName(name);
	if (i == -1)
	{
		i = R_CheckTextureNumForName(name);
		if (i == -1)
		{
			namet[8] = 0;
			memcpy(namet, name, 8);
			if (ignore_missing_textures)
			{
				con << "FTNumForName: " << namet << " not found\n";
				return 0;
			}
			Host_Error("FTNumForName: %s not found", namet);
		}
	}
	return i;
	unguard;
}

//==========================================================================
//
//  TFNumForName
//
//  Retrieval, get a texture or flat number for a name.
//
//==========================================================================

int TFNumForName(const char *name)
{
	guard(TFNumForName);
	char namet[9];
	int i;

	i = R_CheckTextureNumForName(name);
	if (i == -1)
	{
		i = R_CheckFlatNumForName(name);
		if (i == -1)
		{
			namet[8] = 0;
			memcpy(namet, name, 8);
			if (ignore_missing_textures)
			{
				con << "TFNumForName: " << namet << " not found\n";
				return 0;
			}
			Host_Error("TFNumForName: %s not found", namet);
		}
	}
	return i;
	unguard;
}

#ifdef SERVER

//==========================================================================
//
//  LoadBlockMap
//
//==========================================================================

static void LoadBlockMap(int lump, sv_level_t &loadlevel)
{
	guard(LoadBlockMap);
	int i;
	int count;

   	loadlevel.blockmaplump = (short*)W_CacheLumpNum(lump, PU_LEVEL);
   	loadlevel.blockmap = loadlevel.blockmaplump + 4;
   	count = W_LumpLength(lump) / 2;

   	for (i=0 ; i<count ; i++)
		loadlevel.blockmaplump[i] = LittleShort(loadlevel.blockmaplump[i]);

	loadlevel.bmaporgx = loadlevel.blockmaplump[0];
	loadlevel.bmaporgy = loadlevel.blockmaplump[1];
	loadlevel.bmapwidth = loadlevel.blockmaplump[2];
	loadlevel.bmapheight = loadlevel.blockmaplump[3];

	// clear out mobj chains
	count = loadlevel.bmapwidth * loadlevel.bmapheight;
	loadlevel.blocklinks = Z_CNew<VMapObject*>(count, PU_LEVEL, 0);
	unguard;
}

#endif

//==========================================================================
//
//  LoadVertexes
//
//==========================================================================

static void LoadVertexes(int lump, int gl_lump, base_level_t &loadlevel)
{
	guard(LoadVertexes);
	int i;
	void *data;
	mapvertex_t *ml;
	vertex_t *li;
	int base_verts;
	int gl_verts;

	//  Determine number of lumps:
	// total lump length / vertex record length.
	base_verts = W_LumpLength(lump) / sizeof(mapvertex_t);
	gl_verts = W_LumpLength(gl_lump) / sizeof(mapvertex_t);
	loadlevel.numvertexes = base_verts + gl_verts;

	// Allocate zone memory for buffer.
	loadlevel.vertexes = Z_CNew<vertex_t>(loadlevel.numvertexes, PU_LEVEL, 0);

	// Load data into cache.
	data = W_CacheLumpNum(lump, PU_STATIC);
	ml = (mapvertex_t *)data;
	li = loadlevel.vertexes;

	// Copy and convert vertex, internal representation as vector.
	for (i = 0; i < base_verts; i++, li++, ml++)
	{
		*li = TVec(LittleShort(ml->x), LittleShort(ml->y), 0);
	}

	// Free buffer memory.
	Z_Free(data);

	//	Save pointer to GL vertexes for seg loading
	gl_vertexes = li;

   	// Load data into cache.
	data = W_CacheLumpNum(gl_lump, PU_STATIC);
	if (!strncmp((char*)data, GL_V2_VERTEX_MAGIC, 4))
	{
		gl_mapvertex_t *glml;

		gl_verts = (W_LumpLength(gl_lump) - 4) / sizeof(gl_mapvertex_t);
		loadlevel.numvertexes = base_verts + gl_verts;
		Z_Resize((void**)&loadlevel.vertexes, loadlevel.numvertexes * sizeof(vertex_t));
		glml = (gl_mapvertex_t *)((byte*)data + 4);

		// Copy and convert vertex, internal representation as vector.
		for (i = 0; i < gl_verts; i++, li++, glml++)
		{
			*li = TVec(LittleLong(glml->x) / 65536.0,
				LittleLong(glml->y) / 65536.0, 0);
		}
	}
	else
	{
		ml = (mapvertex_t *)data;

		// Copy and convert vertex, internal representation as vector.
		for (i = 0; i < gl_verts; i++, li++, ml++)
		{
			*li = TVec(LittleShort(ml->x), LittleShort(ml->y), 0);
		}
	}
	numglvertexes = gl_verts;

   	// Free buffer memory.
	Z_Free(data);
	unguard;
}

//==========================================================================
//
//  LoadSectors
//
//==========================================================================

static void LoadSectors(int lump, base_level_t &loadlevel)
{
	guard(LoadSectors);
	byte *data;
	int i;
	mapsector_t *ms;
	sector_t *ss;
	sec_region_t *region;

	loadlevel.numsectors = W_LumpLength(lump) / sizeof(mapsector_t);
	loadlevel.sectors = Z_CNew<sector_t>(loadlevel.numsectors, PU_LEVEL, 0);
	data = (byte*)W_CacheLumpNum(lump, PU_STATIC);

	ms = (mapsector_t *)data;
	ss = loadlevel.sectors;

	for (i=0 ; i<loadlevel.numsectors ; i++, ss++, ms++)
	{
		//	Floor
		ss->floor.Set(TVec(0, 0, 1), LittleShort(ms->floorheight));
		ss->floor.pic = FTNumForName(ms->floorpic);
		ss->floor.base_pic = ss->floor.pic;
		ss->floor.xoffs = 0;
		ss->floor.yoffs = 0;
		ss->floor.minz = LittleShort(ms->floorheight);
		ss->floor.maxz = LittleShort(ms->floorheight);

		//	Ceiling
		ss->ceiling.Set(TVec(0, 0, -1), -LittleShort(ms->ceilingheight));
		ss->ceiling.pic = FTNumForName(ms->ceilingpic);
		ss->ceiling.base_pic = ss->ceiling.pic;
		ss->ceiling.xoffs = 0;
		ss->ceiling.yoffs = 0;
		ss->ceiling.minz = LittleShort(ms->ceilingheight);
		ss->ceiling.maxz = LittleShort(ms->ceilingheight);

		//	Params
		ss->params.lightlevel = LittleShort(ms->lightlevel);

		//	Region
		region = Z_CNew<sec_region_t>(PU_LEVEL, 0);
		region->floor = &ss->floor;
		region->ceiling = &ss->ceiling;
		region->params = &ss->params;
		ss->topregion = region;
		ss->botregion = region;

		ss->floorheight = LittleShort(ms->floorheight);
		ss->ceilingheight = LittleShort(ms->ceilingheight);
		ss->special = LittleShort(ms->special);
		ss->tag = LittleShort(ms->tag);
		ss->seqType = 0;//SEQTYPE_STONE; // default seqType

		ss->base_floorheight = ss->floor.dist;
		ss->base_ceilingheight = ss->ceiling.dist;
		ss->base_lightlevel = ss->params.lightlevel;
	}

	Z_Free(data);
	unguard;
}

//==========================================================================
//
//  LoadSideDefs
//
//==========================================================================

static void LoadSideDefs(int lump, base_level_t &loadlevel)
{
	guard(LoadSideDefs);
	byte *data;
	int i;
	mapsidedef_t *msd;
	side_t *sd;

	loadlevel.numsides = W_LumpLength(lump) / sizeof(mapsidedef_t);
	loadlevel.sides = Z_CNew<side_t>(loadlevel.numsides, PU_LEVEL, 0);
	data = (byte*)W_CacheLumpNum(lump, PU_STATIC);

	msd = (mapsidedef_t *)data;
	sd = loadlevel.sides;

	for (i = 0; i < loadlevel.numsides; i++, msd++, sd++)
	{
		sd->textureoffset = LittleShort(msd->textureoffset);
		sd->rowoffset = LittleShort(msd->rowoffset);
		sd->sector = &loadlevel.sectors[LittleShort(msd->sector)];
		sd->midtexture = TFNumForName(msd->midtexture);
		sd->toptexture = TFNumForName(msd->toptexture);
		sd->bottomtexture = TFNumForName(msd->bottomtexture);

		sd->base_textureoffset = sd->textureoffset;
		sd->base_rowoffset = sd->rowoffset;
		sd->base_midtexture = sd->midtexture;
		sd->base_toptexture = sd->toptexture;
		sd->base_bottomtexture = sd->bottomtexture;
	}

	Z_Free(data);
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
//	SetupLineSides
//
//==========================================================================

inline void SetupLineSides(line_t *ld, base_level_t &loadlevel)
{
	if (ld->sidenum[0] == -1)
	{
		if (strict_level_errors)
		{
			Host_Error("Bad WAD: Line %d has no front side",
				ld - loadlevel.lines);
		}
		else
		{
			GCon->Logf("Bad WAD: Line %d has no front side",
				ld - loadlevel.lines);
			ld->sidenum[0] = 0;
		}
	}
	ld->frontsector = loadlevel.sides[ld->sidenum[0]].sector;

	if (ld->sidenum[1] != -1)
	{
		ld->backsector = loadlevel.sides[ld->sidenum[1]].sector;
		// Just a warning
		if (!(ld->flags & ML_TWOSIDED))
		{
			GCon->Logf("Bad WAD: Line %d is two-sided but has no TWO-SIDED flag set",
				ld - loadlevel.lines);
		}
	}
	else
	{
		if (ld->flags & ML_TWOSIDED)
		{
			if (strict_level_errors)
			{
				Host_Error("Bad WAD: Line %d is marked as TWO-SIDED but has only one side",
					ld - loadlevel.lines);
			}
			else
			{
				GCon->Logf("Bad WAD: Line %d is marked as TWO-SIDED but has only one side",
					ld - loadlevel.lines);
				ld->flags &= ~ML_TWOSIDED;
			}
		}
		ld->backsector = 0;
	}
}

//==========================================================================
//
//  LoadLineDefs1
//
//  For Doom and Heretic
//
//==========================================================================

static void LoadLineDefs1(int lump, base_level_t &loadlevel)
{
	guard(LoadLineDefs1);
	byte *data;
	int i;
	maplinedef1_t *mld;
	line_t *ld;

	loadlevel.numlines = W_LumpLength(lump) / sizeof(maplinedef1_t);
	loadlevel.lines = Z_CNew<line_t>(loadlevel.numlines, PU_LEVEL, 0);
	data = (byte*)W_CacheLumpNum(lump, PU_STATIC);

	mld = (maplinedef1_t *)data;
	ld = loadlevel.lines;
	for (i = 0; i < loadlevel.numlines; i++, mld++, ld++)
	{
		ld->flags = LittleShort(mld->flags);
		ld->special = LittleShort(mld->special);
		ld->arg1 = LittleShort(mld->tag);
		ld->v1 = &loadlevel.vertexes[LittleShort(mld->v1)];
		ld->v2 = &loadlevel.vertexes[LittleShort(mld->v2)];

		CalcLine(ld);

		ld->sidenum[0] = LittleShort(mld->sidenum[0]);
		ld->sidenum[1] = LittleShort(mld->sidenum[1]);
		SetupLineSides(ld, loadlevel);
	}

	Z_Free(data);
	unguard;
}

//==========================================================================
//
//  LoadLineDefs2
//
//  For Hexen
//
//==========================================================================

static void LoadLineDefs2(int lump, base_level_t &loadlevel)
{
	guard(LoadLineDefs2);
	byte *data;
	int i;
	maplinedef2_t *mld;
	line_t *ld;

	loadlevel.numlines = W_LumpLength(lump) / sizeof(maplinedef2_t);
	loadlevel.lines = Z_CNew<line_t>(loadlevel.numlines, PU_LEVEL, 0);
	data = (byte*)W_CacheLumpNum(lump, PU_STATIC);

	mld = (maplinedef2_t *)data;
	ld = loadlevel.lines;
	for (i = 0; i < loadlevel.numlines; i++, mld++, ld++)
	{
		ld->flags = LittleShort(mld->flags);

		// New line special info ...
		ld->special = mld->special;
		ld->arg1 = mld->arg1;
		ld->arg2 = mld->arg2;
		ld->arg3 = mld->arg3;
		ld->arg4 = mld->arg4;
		ld->arg5 = mld->arg5;

		ld->v1 = &loadlevel.vertexes[LittleShort(mld->v1)];
		ld->v2 = &loadlevel.vertexes[LittleShort(mld->v2)];

		CalcLine(ld);

		ld->sidenum[0] = LittleShort(mld->sidenum[0]);
		ld->sidenum[1] = LittleShort(mld->sidenum[1]);
		SetupLineSides(ld, loadlevel);
	}

	Z_Free(data);
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

//==========================================================================
//
//  LoadGLSegs
//
//==========================================================================

static void LoadGLSegs(int lump, base_level_t &loadlevel)
{
	guard(LoadGLSegs);
	void *data;
	int i;
	mapglseg_t *ml;
	seg_t *li;
	line_t *ldef;
	int linedef;
	int side;

	loadlevel.numsegs = W_LumpLength(lump) / sizeof(mapglseg_t);
	loadlevel.segs = Z_CNew<seg_t>(loadlevel.numsegs, PU_LEVEL, 0);
	data = W_CacheLumpNum(lump, PU_STATIC);

	ml = (mapglseg_t *)data;
	li = loadlevel.segs;

	for (i=0 ; i<loadlevel.numsegs ; i++, li++, ml++)
	{
		word	v1num =	LittleShort(ml->v1);
		word	v2num =	LittleShort(ml->v2);

		if (v1num & GL_VERTEX)
		{
			v1num ^= GL_VERTEX;
			li->v1 = &gl_vertexes[v1num];
		}
		else
		{
			li->v1 = &loadlevel.vertexes[v1num];
		}
		if (v2num & GL_VERTEX)
		{
			v2num ^= GL_VERTEX;
			li->v2 = &gl_vertexes[v2num];
		}
		else
		{
			li->v2 = &loadlevel.vertexes[v2num];
		}

		linedef = LittleShort(ml->linedef);
		side = LittleShort(ml->side);

		if (linedef >= 0)
		{
			ldef = &loadlevel.lines[linedef];
			li->linedef = ldef;
			li->sidedef = &loadlevel.sides[ldef->sidenum[side]];
			li->frontsector = loadlevel.sides[ldef->sidenum[side]].sector;

			if (ldef->flags & ML_TWOSIDED)
				li->backsector = loadlevel.sides[ldef->sidenum[side^1]].sector;

			if (side)
				li->offset = Length(*li->v1 - *ldef->v2);
			else
				li->offset = Length(*li->v1 - *ldef->v1);
			li->length = Length(*li->v2 - *li->v1);
			li->side = side;
		}

		//	Partner is not used

		//	Calc seg's plane params
		CalcSeg(li);
	}

	Z_Free(data);
	unguard;
}

//==========================================================================
//
//  LoadSubsectors
//
//==========================================================================

static void LoadSubsectors(int lump, base_level_t &loadlevel)
{
	guard(LoadSubsectors);
	void *data;
	int i;
	int j;
	mapsubsector_t *ms;
	subsector_t *ss;
	seg_t *seg;

	loadlevel.numsubsectors = W_LumpLength(lump) / sizeof(mapsubsector_t);
	loadlevel.subsectors = Z_CNew<subsector_t>(loadlevel.numsubsectors, PU_LEVEL, 0);
	data = W_CacheLumpNum(lump, PU_STATIC);

	ms = (mapsubsector_t *)data;
	ss = loadlevel.subsectors;

	for (i = 0; i < loadlevel.numsubsectors; i++, ss++, ms++)
	{
		ss->numlines = (word)LittleShort(ms->numsegs);
		ss->firstline = (word)LittleShort(ms->firstseg);

		// look up sector number for each subsector
		seg = &loadlevel.segs[ss->firstline];
		for (j = 0; j < ss->numlines; j++)
		{
			if (seg[j].linedef)
			{
				ss->sector = seg[j].sidedef->sector;
				ss->seclink = ss->sector->subsectors;
				ss->sector->subsectors = ss;
				break;
			}
		}
		if (!ss->sector)
			Sys_Error("Subsector %d without sector", i);
	}

	Z_Free(data);
	unguard;
}

//==========================================================================
//
//  LoadNodes
//
//==========================================================================

static void LoadNodes(int lump, base_level_t &loadlevel)
{
	guard(LoadNodes);
	byte *data;
	int i;
	int j;
	mapnode_t *mn;
	node_t *no;

	loadlevel.numnodes = W_LumpLength(lump) / sizeof(mapnode_t);
	loadlevel.nodes = Z_CNew<node_t>(loadlevel.numnodes, PU_LEVEL, 0);
	data = (byte*)W_CacheLumpNum(lump,PU_STATIC);

	mn = (mapnode_t *)data;
	no = loadlevel.nodes;

	for (i = 0; i < loadlevel.numnodes; i++, no++, mn++)
	{
		no->SetPointDir(TVec(LittleShort(mn->x), LittleShort(mn->y), 0),
			TVec(LittleShort(mn->dx), LittleShort(mn->dy), 0));

		for (j = 0; j < 2; j++)
		{
			no->children[j] = LittleShort(mn->children[j]);
			no->bbox[j][0] = LittleShort(mn->bbox[j][BOXLEFT]);
			no->bbox[j][1] = LittleShort(mn->bbox[j][BOXBOTTOM]);
			no->bbox[j][2] = -32768.0;
			no->bbox[j][3] = LittleShort(mn->bbox[j][BOXRIGHT]);
			no->bbox[j][4] = LittleShort(mn->bbox[j][BOXTOP]);
			no->bbox[j][5] = 32768.0;
		}
	}
	Z_Free(data);
	unguard;
}

//==========================================================================
//
//  LoadPVS
//
//==========================================================================

static void	LoadPVS(int lump, base_level_t &loadlevel)
{
	guard(LoadPVS);
	if (strcmp(W_LumpName(lump), "GL_PVS") || W_LumpLength(lump) == 0)
	{
		cond << "Empty or missing PVS lump\n";
		loadlevel.vis_data = NULL;
		memset(novis, 0xff, sizeof(novis));
	}
	else
	{
		loadlevel.vis_data = (byte*)W_CacheLumpNum(lump, PU_LEVEL);
	}
	unguard;
}

#ifdef SERVER
//==========================================================================
//
//  LoadThings1
//
//==========================================================================

static void LoadThings1(int lump)
{
	guard(LoadThings1);
	sv_level_t &loadlevel = level;
	byte *data;
	int i;
	mapthing1_t *mt;
	mthing_t *mth;

	data = (byte*)W_CacheLumpNum(lump,PU_STATIC);
	loadlevel.numthings = W_LumpLength(lump) / sizeof(mapthing1_t);
	loadlevel.things = Z_CNew<mthing_t>(loadlevel.numthings, PU_HIGH, 0);

	mt = (mapthing1_t *)data;
	mth = loadlevel.things;
	for (i = 0; i < loadlevel.numthings; i++, mt++, mth++)
	{
		mth->x = LittleShort(mt->x);
		mth->y = LittleShort(mt->y);
		mth->angle = LittleShort(mt->angle);
		mth->type = LittleShort(mt->type);
		mth->options = LittleShort(mt->options);
	}
	Z_Free(data);
	unguard;
}

//==========================================================================
//
//  LoadThings2
//
//==========================================================================

static void LoadThings2(int lump)
{
	guard(LoadThings2);
	sv_level_t &loadlevel = level;
	byte *data;
	int i;
	mapthing2_t *mt;
	mthing_t *mth;

	data = (byte*)W_CacheLumpNum(lump, PU_STATIC);
	loadlevel.numthings = W_LumpLength(lump)/sizeof(mapthing2_t);
	loadlevel.things = Z_CNew<mthing_t>(loadlevel.numthings, PU_HIGH, 0);

	mt = (mapthing2_t *)data;
	mth = loadlevel.things;
	for (i = 0; i < loadlevel.numthings; i++, mt++, mth++)
	{
		mth->tid = LittleShort(mt->tid);
		mth->x = LittleShort(mt->x);
		mth->y = LittleShort(mt->y);
		mth->height = LittleShort(mt->height);
		mth->angle = LittleShort(mt->angle);
		mth->type = LittleShort(mt->type);
		mth->options = LittleShort(mt->options);
		mth->special = mt->special;
		mth->arg1 = mt->arg1;
		mth->arg2 = mt->arg2;
		mth->arg3 = mt->arg3;
		mth->arg4 = mt->arg4;
		mth->arg5 = mt->arg5;
	}
	Z_Free(data);
	unguard;
}
#endif

//==========================================================================
//
//  ClearBox
//
//==========================================================================

static void ClearBox(float *box)
{
	guardSlow(ClearBox);
	box[BOXTOP] = box[BOXRIGHT] = -99999.0;
	box[BOXBOTTOM] = box[BOXLEFT] = 99999.0;
	unguardSlow;
}

//==========================================================================
//
//  AddToBox
//
//==========================================================================

static void AddToBox(float* box, float x, float y)
{
	guardSlow(AddToBox);
	if (x < box[BOXLEFT])
		box[BOXLEFT] = x;
	else if (x > box[BOXRIGHT])
		box[BOXRIGHT] = x;
	if (y < box[BOXBOTTOM])
		box[BOXBOTTOM] = y;
	else if (y > box[BOXTOP])
		box[BOXTOP] = y;
	unguardSlow;
}

//==========================================================================
//
//  LinkNode
//
//==========================================================================

static void LinkNode(int bspnum, node_t *parent, base_level_t &loadlevel)
{
	guardSlow(LinkNode);
	if (bspnum & NF_SUBSECTOR)
	{
		int num;

		if (bspnum == -1)
			num = 0;
		else
			num = bspnum & (~NF_SUBSECTOR);
		loadlevel.subsectors[num].parent = parent;
	}
	else
	{
		node_t* bsp = &loadlevel.nodes[bspnum];
		bsp->parent = parent;
	
		LinkNode(bsp->children[0], bsp, loadlevel);
		LinkNode(bsp->children[1], bsp, loadlevel);
	}
	unguardSlow;
}

#ifdef SERVER

//==========================================================================
//
//  GroupLines
//
//  Builds sector line lists and subsector sector numbers. Finds block
// bounding boxes for sectors.
//
//==========================================================================

static void GroupLines(sv_level_t &loadlevel)
{
	guard(GroupLines);
	line_t ** linebuffer;
	int i;
	int j;
	int total;
	line_t *li;
	sector_t *sector;
	float bbox[4];
	int block;

	LinkNode(loadlevel.numnodes - 1, NULL, loadlevel);

	// count number of lines in each sector
	li = loadlevel.lines;
	total = 0;
	for (i = 0; i < loadlevel.numlines; i++, li++)
	{
		total++;
		li->frontsector->linecount++;

		if (li->backsector && li->backsector != li->frontsector)
		{
			li->backsector->linecount++;
			total++;
		}
	}

	// build line tables for each sector
	linebuffer = Z_New<line_t*>(total, PU_LEVEL, 0);
	sector = loadlevel.sectors;
	for (i = 0; i < loadlevel.numsectors; i++, sector++)
	{
		ClearBox(bbox);
		sector->lines = linebuffer;
		li = loadlevel.lines;
		for (j = 0; j < loadlevel.numlines; j++, li++)
		{
			if (li->frontsector == sector || li->backsector == sector)
			{
				*linebuffer++ = li;
				AddToBox(bbox, li->v1->x, li->v1->y);
				AddToBox(bbox, li->v2->x, li->v2->y);
			}
		}
		if (linebuffer - sector->lines != sector->linecount)
			Sys_Error("GroupLines: miscounted");

		// set the soundorg to the middle of the bounding box
		sector->soundorg = TVec((bbox[BOXRIGHT] + bbox[BOXLEFT]) / 2.0,
			(bbox[BOXTOP] + bbox[BOXBOTTOM]) / 2.0, 0);

		// adjust bounding box to map blocks
		block = MapBlock(bbox[BOXTOP] - loadlevel.bmaporgy + MAXRADIUS);
		block = block >= loadlevel.bmapheight ? loadlevel.bmapheight - 1 : block;
		sector->blockbox[BOXTOP] = block;

		block = MapBlock(bbox[BOXBOTTOM] - loadlevel.bmaporgy - MAXRADIUS);
		block = block < 0 ? 0 : block;
		sector->blockbox[BOXBOTTOM] = block;

		block = MapBlock(bbox[BOXRIGHT] - loadlevel.bmaporgx + MAXRADIUS);
		block = block >= loadlevel.bmapwidth ? loadlevel.bmapwidth - 1 : block;
		sector->blockbox[BOXRIGHT] = block;

		block = MapBlock(bbox[BOXLEFT] - loadlevel.bmaporgx - MAXRADIUS);
		block = block < 0 ? 0 : block;
		sector->blockbox[BOXLEFT] = block;
	}
	unguard;
}

#endif
#ifdef CLIENT

//==========================================================================
//
//  GroupLines
//
//  Builds sector line lists and subsector sector numbers. Finds block
// bounding boxes for sectors.
//
//==========================================================================

static void GroupLines(cl_level_t &loadlevel)
{
	guard(GroupLines);
	line_t ** linebuffer;
	int i;
	int j;
	int total;
	line_t *li;
	sector_t *sector;
	float bbox[4];

	LinkNode(loadlevel.numnodes - 1, NULL, loadlevel);

	// count number of lines in each sector
	li = loadlevel.lines;
	total = 0;
	for (i = 0; i < loadlevel.numlines; i++, li++)
	{
		total++;
		li->frontsector->linecount++;

		if (li->backsector && li->backsector != li->frontsector)
		{
			li->backsector->linecount++;
			total++;
		}
	}

	// build line tables for each sector
	linebuffer = Z_New<line_t*>(total, PU_LEVEL, 0);
	sector = loadlevel.sectors;
	for (i = 0; i < loadlevel.numsectors; i++, sector++)
	{
		ClearBox(bbox);
		sector->lines = linebuffer;
		li = loadlevel.lines;
		for (j = 0; j < loadlevel.numlines; j++, li++)
		{
			if (li->frontsector == sector || li->backsector == sector)
			{
				*linebuffer++ = li;
				AddToBox(bbox, li->v1->x, li->v1->y);
				AddToBox(bbox, li->v2->x, li->v2->y);
			}
		}
		if (linebuffer - sector->lines != sector->linecount)
			Sys_Error("GroupLines: miscounted");

		// set the soundorg to the middle of the bounding box
		sector->soundorg = TVec((bbox[BOXRIGHT] + bbox[BOXLEFT]) / 2.0,
			(bbox[BOXTOP] + bbox[BOXBOTTOM]) / 2.0, 0);
	}
	unguard;
}

#endif

//==========================================================================
//
//	LoadBaseLevel
//
//==========================================================================

static bool LoadBaseLevel(base_level_t &lev, const char *mapname,
	int &lumpnum, int &gl_lumpnum, bool open_aux)
{
	guard(LoadBaseLevel);
	if (open_aux)
	{
		W_CloseAuxiliary();
		AuxiliaryMap = false;
		// if working with a devlopment map, reload it
		if (fl_devmode)
		{
			char aux_file_name[MAX_OSPATH];

			if (FL_FindFile(va("maps/%s.wad", lev.mapname), aux_file_name))
			{
				W_OpenAuxiliary(aux_file_name);
				AuxiliaryMap = true;
			}
		}
	}

	lumpnum = W_CheckNumForName(mapname);
	if (lumpnum < 0)
	{
		Host_Error("Map %s not found\n", mapname);
	}
	gl_lumpnum = W_CheckNumForName(va("GL_%s", mapname));
#ifdef CLIENT
	if (gl_lumpnum < lumpnum)
	{
		W_BuildGLNodes(lumpnum);
		gl_lumpnum = W_GetNumForName(va("GL_%s", mapname));
	}
	else if (strcmp(W_LumpName(gl_lumpnum + ML_GL_PVS), "GL_PVS") ||
		W_LumpLength(gl_lumpnum + ML_GL_PVS) == 0)
	{
		W_BuildPVS(lumpnum, gl_lumpnum);
		lumpnum = W_GetNumForName(mapname);
		gl_lumpnum = W_GetNumForName(va("GL_%s", mapname));
	}
#else
	if (gl_lumpnum < lumpnum)
	{
		// Dedicated servers doesn't have plugins
		Host_Error("Map %s is missing GL-Nodes\n", mapname);
	}
#endif
	bool extended = !strcmp(W_LumpName(lumpnum + ML_BEHAVIOR), "BEHAVIOR");

	//
	// Begin processing map lumps
	// Note: most of this ordering is important
	//
	LoadVertexes(lumpnum + ML_VERTEXES, gl_lumpnum + ML_GL_VERT, lev);
	LoadSectors(lumpnum + ML_SECTORS, lev);
	LoadSideDefs(lumpnum + ML_SIDEDEFS, lev);
	if (!extended)
	{
		LoadLineDefs1(lumpnum + ML_LINEDEFS, lev);
	}
	else
	{
		LoadLineDefs2(lumpnum + ML_LINEDEFS, lev);
	}
	LoadGLSegs(gl_lumpnum + ML_GL_SEGS, lev);
	LoadSubsectors(gl_lumpnum + ML_GL_SSECT, lev);
   	LoadNodes(gl_lumpnum + ML_GL_NODES, lev);
	LoadPVS(gl_lumpnum + ML_GL_PVS, lev);

	return extended;
	unguard;
}

#ifdef SERVER

//==========================================================================
//
//  LoadLevel
//
//==========================================================================

void LoadLevel(sv_level_t &lev, const char *mapname)
{
	guard(LoadLevel);
	int lumpnum;
	int gl_lumpnum;

	bool extended = LoadBaseLevel(lev, mapname, lumpnum, gl_lumpnum, true);
	LoadBlockMap(lumpnum + ML_BLOCKMAP, lev);
	lev.rejectmatrix = (byte*)W_CacheLumpNum(lumpnum + ML_REJECT, PU_LEVEL);

	GroupLines(lev);

	if (extended)
	{
		LoadThings2(lumpnum + ML_THINGS);
		//	ACS object code
		lev.behavior = (int*)W_CacheLumpNum(lumpnum + ML_BEHAVIOR, PU_LEVEL);
		lev.behaviorsize = W_LumpLength(lumpnum + ML_BEHAVIOR);
	}
	else
	{
		LoadThings1(lumpnum + ML_THINGS);
		//	Inform ACS, that we don't have scripts
		lev.behavior = NULL;
		//	Translate level to Hexen format
		svpr.Exec("TranslateLevel");
	}

#ifndef CLIENT
	//
	// End of map lump processing
	//
	if (AuxiliaryMap)
	{
		// Close the auxiliary file, but don't free its loaded lumps.
		// The next call to W_OpenAuxiliary() will do a full shutdown
		// of the current auxiliary WAD (free lumps and info lists).
		W_CloseAuxiliaryFile();
	}
#endif
	unguard;
}

#endif
#ifdef CLIENT

//==========================================================================
//
//  LoadLevel
//
//==========================================================================

void LoadLevel(cl_level_t &lev, const char *mapname)
{
	guard(LoadLevel);
	int lumpnum;
	int gl_lumpnum;

	LoadBaseLevel(lev, mapname, lumpnum, gl_lumpnum,
#ifdef SERVER
			!sv.active
#else
			true
#endif
		);
	GroupLines(lev);

	//
	// End of map lump processing
	//
	if (AuxiliaryMap)
	{
		// Close the auxiliary file, but don't free its loaded lumps.
		// The next call to W_OpenAuxiliary() will do a full shutdown
		// of the current auxiliary WAD (free lumps and info lists).
		W_CloseAuxiliaryFile();
	}
	unguard;
}

#endif

//==========================================================================
//
//  PointInSubsector
//
//==========================================================================

subsector_t* PointInSubsector(const base_level_t &lev, float x, float y)
{
	guard(PointInSubsector);
	node_t *node;
	int side;
	int nodenum;
	TVec point(x, y, 0);

	// single subsector is a special case
	if (!lev.numnodes)
		return lev.subsectors;

	nodenum = lev.numnodes - 1;

	while (!(nodenum & NF_SUBSECTOR))
	{
		node = &lev.nodes[nodenum];
		side = node->PointOnSide(point);
		nodenum = node->children[side];
	}
	return &lev.subsectors[nodenum & ~NF_SUBSECTOR];
	unguard;
}

//==========================================================================
//
//  LeafPVS
//
//==========================================================================

byte *LeafPVS(const base_level_t &lev, const subsector_t *ss)
{
	guard(LeafPVS);
	int sub = ss - lev.subsectors;
	if (lev.vis_data)
	{
		return lev.vis_data + (((lev.numsubsectors + 7) >> 3) * sub);
	}
	else
	{
		return novis;
	}
	unguard;
}

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
	con << "Invalid extra floor, tag " << dst->tag << endl;
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

//**************************************************************************
//
//  $Log$
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
