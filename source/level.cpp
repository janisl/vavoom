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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"
#include "fmapdefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(VLevel);

VLevel*			GLevel;
VLevel*			GClLevel;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static vertex_t *gl_vertexes;
static int numglvertexes;

static bool AuxiliaryMap;

static TCvarI ignore_missing_textures("ignore_missing_textures", "0");
static TCvarI strict_level_errors("strict_level_errors", "1");

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VLevel::LoadMap
//
//==========================================================================

void VLevel::LoadMap(const char *mapname)
{
	guard(VLevel::LoadMap);
#ifdef SERVER
	if (bForServer || !sv.active)
#endif
	{
		W_CloseAuxiliary();
		AuxiliaryMap = false;
		// if working with a devlopment map, reload it
		if (fl_devmode)
		{
			char aux_file_name[MAX_OSPATH];

			if (FL_FindFile(va("maps/%s.wad", mapname), aux_file_name))
			{
				W_OpenAuxiliary(aux_file_name);
				AuxiliaryMap = true;
			}
		}
	}

	int lumpnum = W_CheckNumForName(mapname);
	if (lumpnum < 0)
	{
		Host_Error("Map %s not found\n", mapname);
	}
	int gl_lumpnum = W_CheckNumForName(va("GL_%s", mapname));
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
	bExtended = !strcmp(W_LumpName(lumpnum + ML_BEHAVIOR), "BEHAVIOR");

	//
	// Begin processing map lumps
	// Note: most of this ordering is important
	//
	LoadVertexes(lumpnum + ML_VERTEXES, gl_lumpnum + ML_GL_VERT);
	LoadSectors(lumpnum + ML_SECTORS);
	LoadSideDefs(lumpnum + ML_SIDEDEFS);
	if (!bExtended)
	{
		LoadLineDefs1(lumpnum + ML_LINEDEFS);
	}
	else
	{
		LoadLineDefs2(lumpnum + ML_LINEDEFS);
	}
	LoadGLSegs(gl_lumpnum + ML_GL_SEGS);
	LoadSubsectors(gl_lumpnum + ML_GL_SSECT);
   	LoadNodes(gl_lumpnum + ML_GL_NODES);
	LoadPVS(gl_lumpnum + ML_GL_PVS);
	if (bForServer)
	{
		LoadBlockMap(lumpnum + ML_BLOCKMAP);
		RejectMatrix = (byte*)W_CacheLumpNum(lumpnum + ML_REJECT, PU_LEVEL);
		if (bExtended)
		{
			LoadThings2(lumpnum + ML_THINGS);
			//	ACS object code
			Behavior = (int*)W_CacheLumpNum(lumpnum + ML_BEHAVIOR, PU_LEVEL);
			BehaviorSize = W_LumpLength(lumpnum + ML_BEHAVIOR);
		}
		else
		{
			LoadThings1(lumpnum + ML_THINGS);
			//	Inform ACS, that we don't have scripts
			Behavior = NULL;
			BehaviorSize = 0;
		}
	}

	GroupLines();

	//	Load conversations.
	mapInfo_t info;
	P_GetMapInfo(mapname, info);
	LoadRogueConScript("SCRIPT00", GenericSpeeches, NumGenericSpeeches);
	LoadRogueConScript(info.speechLump, LevelSpeeches, NumLevelSpeeches);

	//
	// End of map lump processing
	//
#ifdef CLIENT
	if (!bForServer && AuxiliaryMap)
#else
	if (AuxiliaryMap)
#endif
	{
		// Close the auxiliary file, but don't free its loaded lumps.
		// The next call to W_OpenAuxiliary() will do a full shutdown
		// of the current auxiliary WAD (free lumps and info lists).
		W_CloseAuxiliaryFile();
	}
	unguard;
}

//==========================================================================
//
//  VLevel::LoadVertexes
//
//==========================================================================

void VLevel::LoadVertexes(int Lump, int GLLump)
{
	guard(VLevel::LoadVertexes);
	int i;
	void *Data;
	mapvertex_t *pSrc;
	vertex_t *pDst;
	int NumBaseVerts;
	int NumGLVerts;

	//  Determine number of lumps:
	// total lump length / vertex record length.
	NumBaseVerts = W_LumpLength(Lump) / sizeof(mapvertex_t);
	NumGLVerts = W_LumpLength(GLLump) / sizeof(mapvertex_t);
	NumVertexes = NumBaseVerts + NumGLVerts;

	// Allocate zone memory for buffer.
	Vertexes = Z_CNew<vertex_t>(NumVertexes, PU_LEVEL, 0);

	// Load data into cache.
	Data = W_CacheLumpNum(Lump, PU_STATIC);
	pSrc = (mapvertex_t *)Data;
	pDst = Vertexes;

	// Copy and convert vertex, internal representation as vector.
	for (i = 0; i < NumBaseVerts; i++, pDst++, pSrc++)
	{
		*pDst = TVec(LittleShort(pSrc->x), LittleShort(pSrc->y), 0);
	}

	// Free buffer memory.
	Z_Free(Data);

	//	Save pointer to GL vertexes for seg loading
	gl_vertexes = pDst;

   	// Load data into cache.
	Data = W_CacheLumpNum(GLLump, PU_STATIC);
	if (!strncmp((char*)Data, GL_V2_VERTEX_MAGIC, 4))
	{
		gl_mapvertex_t *pGLSrc;

		NumGLVerts = (W_LumpLength(GLLump) - 4) / sizeof(gl_mapvertex_t);
		NumVertexes = NumBaseVerts + NumGLVerts;
		Z_Resize((void**)&Vertexes, NumVertexes * sizeof(vertex_t));
		pGLSrc = (gl_mapvertex_t *)((byte*)Data + 4);

		// Copy and convert vertex, internal representation as vector.
		for (i = 0; i < NumGLVerts; i++, pDst++, pGLSrc++)
		{
			*pDst = TVec(LittleLong(pGLSrc->x) / 65536.0,
				LittleLong(pGLSrc->y) / 65536.0, 0);
		}
	}
	else
	{
		pSrc = (mapvertex_t *)Data;

		// Copy and convert vertex, internal representation as vector.
		for (i = 0; i < NumGLVerts; i++, pDst++, pSrc++)
		{
			*pDst = TVec(LittleShort(pSrc->x), LittleShort(pSrc->y), 0);
		}
	}
	numglvertexes = NumGLVerts;

   	// Free buffer memory.
	Z_Free(Data);
	unguard;
}

//==========================================================================
//
//  VLevel::LoadSectors
//
//==========================================================================

void VLevel::LoadSectors(int Lump)
{
	guard(VLevel::LoadSectors);
	byte *data;
	int i;
	mapsector_t *ms;
	sector_t *ss;
	sec_region_t *region;

	NumSectors = W_LumpLength(Lump) / sizeof(mapsector_t);
	Sectors = Z_CNew<sector_t>(NumSectors, PU_LEVEL, 0);
	data = (byte*)W_CacheLumpNum(Lump, PU_STATIC);

	ms = (mapsector_t *)data;
	ss = Sectors;

	for (i = 0; i < NumSectors; i++, ss++, ms++)
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
		ss->params.LightColor = 0x00ffffff;

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
//  VLevel::LoadSideDefs
//
//==========================================================================

void VLevel::LoadSideDefs(int Lump)
{
	guard(VLevel::LoadSideDefs);
	byte *data;
	int i;
	mapsidedef_t *msd;
	side_t *sd;

	NumSides = W_LumpLength(Lump) / sizeof(mapsidedef_t);
	Sides = Z_CNew<side_t>(NumSides, PU_LEVEL, 0);
	data = (byte*)W_CacheLumpNum(Lump, PU_STATIC);

	msd = (mapsidedef_t *)data;
	sd = Sides;

	for (i = 0; i < NumSides; i++, msd++, sd++)
	{
		sd->textureoffset = LittleShort(msd->textureoffset);
		sd->rowoffset = LittleShort(msd->rowoffset);
		sd->sector = &Sectors[LittleShort(msd->sector)];
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
//	VLevel::SetupLineSides
//
//==========================================================================

inline void VLevel::SetupLineSides(line_t *ld) const
{
	CalcLine(ld);
	if (ld->sidenum[0] == -1)
	{
		if (strict_level_errors)
		{
			Host_Error("Bad WAD: Line %d has no front side", ld - Lines);
		}
		else
		{
			GCon->Logf("Bad WAD: Line %d has no front side", ld - Lines);
			ld->sidenum[0] = 0;
		}
	}
	ld->frontsector = Sides[ld->sidenum[0]].sector;

	if (ld->sidenum[1] != -1)
	{
		ld->backsector = Sides[ld->sidenum[1]].sector;
		// Just a warning
		if (!(ld->flags & ML_TWOSIDED))
		{
			GCon->Logf("Bad WAD: Line %d is two-sided but has no TWO-SIDED flag set",
				ld - Lines);
		}
	}
	else
	{
		if (ld->flags & ML_TWOSIDED)
		{
			if (strict_level_errors)
			{
				Host_Error("Bad WAD: Line %d is marked as TWO-SIDED but has only one side",
					ld - Lines);
			}
			else
			{
				GCon->Logf("Bad WAD: Line %d is marked as TWO-SIDED but has only one side",
					ld - Lines);
				ld->flags &= ~ML_TWOSIDED;
			}
		}
		ld->backsector = 0;
	}
}

//==========================================================================
//
//  VLevel::LoadLineDefs1
//
//  For Doom and Heretic
//
//==========================================================================

void VLevel::LoadLineDefs1(int Lump)
{
	guard(VLevel::LoadLineDefs1);
	byte *data;
	int i;
	maplinedef1_t *mld;
	line_t *ld;

	NumLines = W_LumpLength(Lump) / sizeof(maplinedef1_t);
	Lines = Z_CNew<line_t>(NumLines, PU_LEVEL, 0);
	data = (byte*)W_CacheLumpNum(Lump, PU_STATIC);

	mld = (maplinedef1_t *)data;
	ld = Lines;
	for (i = 0; i < NumLines; i++, mld++, ld++)
	{
		ld->flags = LittleShort(mld->flags);
		ld->special = LittleShort(mld->special);
		ld->arg1 = LittleShort(mld->tag);
		ld->v1 = &Vertexes[LittleShort(mld->v1)];
		ld->v2 = &Vertexes[LittleShort(mld->v2)];
		ld->sidenum[0] = LittleShort(mld->sidenum[0]);
		ld->sidenum[1] = LittleShort(mld->sidenum[1]);
		SetupLineSides(ld);
	}

	Z_Free(data);
	unguard;
}

//==========================================================================
//
//  VLevel::LoadLineDefs2
//
//  For Hexen
//
//==========================================================================

void VLevel::LoadLineDefs2(int Lump)
{
	guard(VLevel::LoadLineDefs2);
	byte *data;
	int i;
	maplinedef2_t *mld;
	line_t *ld;

	NumLines = W_LumpLength(Lump) / sizeof(maplinedef2_t);
	Lines = Z_CNew<line_t>(NumLines, PU_LEVEL, 0);
	data = (byte*)W_CacheLumpNum(Lump, PU_STATIC);

	mld = (maplinedef2_t *)data;
	ld = Lines;
	for (i = 0; i < NumLines; i++, mld++, ld++)
	{
		ld->flags = LittleShort(mld->flags);

		// New line special info ...
		ld->special = mld->special;
		ld->arg1 = mld->arg1;
		ld->arg2 = mld->arg2;
		ld->arg3 = mld->arg3;
		ld->arg4 = mld->arg4;
		ld->arg5 = mld->arg5;

		ld->v1 = &Vertexes[LittleShort(mld->v1)];
		ld->v2 = &Vertexes[LittleShort(mld->v2)];
		ld->sidenum[0] = LittleShort(mld->sidenum[0]);
		ld->sidenum[1] = LittleShort(mld->sidenum[1]);
		SetupLineSides(ld);
	}

	Z_Free(data);
	unguard;
}

//==========================================================================
//
//  VLevel::LoadGLSegs
//
//==========================================================================

void VLevel::LoadGLSegs(int Lump)
{
	guard(VLevel::LoadGLSegs);
	void *data;
	int i;
	mapglseg_t *ml;
	seg_t *li;
	line_t *ldef;
	int linedef;
	int side;

	NumSegs = W_LumpLength(Lump) / sizeof(mapglseg_t);
	Segs = Z_CNew<seg_t>(NumSegs, PU_LEVEL, 0);
	data = W_CacheLumpNum(Lump, PU_STATIC);

	ml = (mapglseg_t *)data;
	li = Segs;

	for (i = 0; i < NumSegs; i++, li++, ml++)
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
			li->v1 = &Vertexes[v1num];
		}
		if (v2num & GL_VERTEX)
		{
			v2num ^= GL_VERTEX;
			li->v2 = &gl_vertexes[v2num];
		}
		else
		{
			li->v2 = &Vertexes[v2num];
		}

		linedef = LittleShort(ml->linedef);
		side = LittleShort(ml->side);

		if (linedef >= 0)
		{
			ldef = &Lines[linedef];
			li->linedef = ldef;
			li->sidedef = &Sides[ldef->sidenum[side]];
			li->frontsector = Sides[ldef->sidenum[side]].sector;

			if (ldef->flags & ML_TWOSIDED)
				li->backsector = Sides[ldef->sidenum[side^1]].sector;

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
//  VLevel::LoadSubsectors
//
//==========================================================================

void VLevel::LoadSubsectors(int Lump)
{
	guard(VLevel::LoadSubsectors);
	void *data;
	int i;
	int j;
	mapsubsector_t *ms;
	subsector_t *ss;
	seg_t *seg;

	NumSubsectors = W_LumpLength(Lump) / sizeof(mapsubsector_t);
	Subsectors = Z_CNew<subsector_t>(NumSubsectors, PU_LEVEL, 0);
	data = W_CacheLumpNum(Lump, PU_STATIC);

	ms = (mapsubsector_t *)data;
	ss = Subsectors;

	for (i = 0; i < NumSubsectors; i++, ss++, ms++)
	{
		ss->numlines = (word)LittleShort(ms->numsegs);
		ss->firstline = (word)LittleShort(ms->firstseg);

		// look up sector number for each subsector
		seg = &Segs[ss->firstline];
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
			Host_Error("Subsector %d without sector", i);
	}

	Z_Free(data);
	unguard;
}

//==========================================================================
//
//  VLevel::LoadNodes
//
//==========================================================================

void VLevel::LoadNodes(int Lump)
{
	guard(VLevel::LoadNodes);
	byte *data;
	int i;
	int j;
	mapnode_t *mn;
	node_t *no;

	NumNodes = W_LumpLength(Lump) / sizeof(mapnode_t);
	Nodes = Z_CNew<node_t>(NumNodes, PU_LEVEL, 0);
	data = (byte*)W_CacheLumpNum(Lump, PU_STATIC);

	mn = (mapnode_t *)data;
	no = Nodes;

	for (i = 0; i < NumNodes; i++, no++, mn++)
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
//  VLevel::LoadPVS
//
//==========================================================================

void VLevel::LoadPVS(int Lump)
{
	guard(VLevel::LoadPVS);
	if (strcmp(W_LumpName(Lump), "GL_PVS") || W_LumpLength(Lump) == 0)
	{
		GCon->Logf(NAME_Dev, "Empty or missing PVS lump");
		VisData = NULL;
		NoVis = Z_New<byte>((NumSubsectors + 7) / 8);
		memset(NoVis, 0xff, (NumSubsectors + 7) / 8);
	}
	else
	{
		VisData = (byte*)W_CacheLumpNum(Lump, PU_LEVEL);
	}
	unguard;
}

//==========================================================================
//
//  VLevel::LoadBlockMap
//
//==========================================================================

void VLevel::LoadBlockMap(int Lump)
{
	guard(VLevel::LoadBlockMap);
	int i;
	int count;

	if (W_LumpLength(Lump) <= 8)
	{
		//	This is fatal.
		Host_Error("Missing a blockmap");
	}

   	BlockMapLump = (short *)W_CacheLumpNum(Lump, PU_LEVEL);
   	BlockMap = (word *)BlockMapLump + 4;
   	count = W_LumpLength(Lump) / 2;

   	for (i = 0; i < count; i++)
		BlockMapLump[i] = LittleShort(BlockMapLump[i]);

	BlockMapOrgX = BlockMapLump[0];
	BlockMapOrgY = BlockMapLump[1];
	BlockMapWidth = BlockMapLump[2];
	BlockMapHeight = BlockMapLump[3];

	// clear out mobj chains
	count = BlockMapWidth * BlockMapHeight;
	BlockLinks = Z_CNew<VEntity*>(count, PU_LEVEL, 0);
	unguard;
}

//==========================================================================
//
//  VLevel::LoadThings1
//
//==========================================================================

void VLevel::LoadThings1(int Lump)
{
	guard(VLevel::LoadThings1);
	byte *data;
	int i;
	mapthing1_t *mt;
	mthing_t *mth;

	data = (byte*)W_CacheLumpNum(Lump, PU_STATIC);
	NumThings = W_LumpLength(Lump) / sizeof(mapthing1_t);
	Things = Z_CNew<mthing_t>(NumThings, PU_HIGH, 0);

	mt = (mapthing1_t *)data;
	mth = Things;
	for (i = 0; i < NumThings; i++, mt++, mth++)
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
//  VLevel::LoadThings2
//
//==========================================================================

void VLevel::LoadThings2(int Lump)
{
	guard(VLevel::LoadThings2);
	byte *data;
	int i;
	mapthing2_t *mt;
	mthing_t *mth;

	data = (byte*)W_CacheLumpNum(Lump, PU_STATIC);
	NumThings = W_LumpLength(Lump) / sizeof(mapthing2_t);
	Things = Z_CNew<mthing_t>(NumThings, PU_HIGH, 0);

	mt = (mapthing2_t *)data;
	mth = Things;
	for (i = 0; i < NumThings; i++, mt++, mth++)
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

//==========================================================================
//
//  VLevel::FTNumForName
//
//  Retrieval, get a texture or flat number for a name.
//
//==========================================================================

int VLevel::FTNumForName(const char *name) const
{
	guard(VLevel::FTNumForName);
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
				GCon->Logf("FTNumForName: %s not found", namet);
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
//  VLevel::TFNumForName
//
//  Retrieval, get a texture or flat number for a name.
//
//==========================================================================

int VLevel::TFNumForName(const char *name) const
{
	guard(VLevel::TFNumForName);
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
				GCon->Logf("TFNumForName: %s not found", namet);
				return 0;
			}
			Host_Error("TFNumForName: %s not found", namet);
		}
	}
	return i;
	unguard;
}

//==========================================================================
//
//  VLevel::ClearBox
//
//==========================================================================

inline void VLevel::ClearBox(float *box) const
{
	guardSlow(VLevel::ClearBox);
	box[BOXTOP] = box[BOXRIGHT] = -99999.0;
	box[BOXBOTTOM] = box[BOXLEFT] = 99999.0;
	unguardSlow;
}

//==========================================================================
//
//  VLevel::AddToBox
//
//==========================================================================

inline void VLevel::AddToBox(float* box, float x, float y) const
{
	guardSlow(VLevel::AddToBox);
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
//  VLevel::GroupLines
//
//  Builds sector line lists and subsector sector numbers. Finds block
// bounding boxes for sectors.
//
//==========================================================================

void VLevel::GroupLines(void) const
{
	guard(VLevel::GroupLines);
	line_t ** linebuffer;
	int i;
	int j;
	int total;
	line_t *li;
	sector_t *sector;
	float bbox[4];
	int block;

	LinkNode(NumNodes - 1, NULL);

	// count number of lines in each sector
	li = Lines;
	total = 0;
	for (i = 0; i < NumLines; i++, li++)
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
	sector = Sectors;
	for (i = 0; i < NumSectors; i++, sector++)
	{
		ClearBox(bbox);
		sector->lines = linebuffer;
		li = Lines;
		for (j = 0; j < NumLines; j++, li++)
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

		if (bForServer)
		{
			// adjust bounding box to map blocks
			block = MapBlock(bbox[BOXTOP] - BlockMapOrgY + MAXRADIUS);
			block = block >= BlockMapHeight ? BlockMapHeight - 1 : block;
			sector->blockbox[BOXTOP] = block;

			block = MapBlock(bbox[BOXBOTTOM] - BlockMapOrgY - MAXRADIUS);
			block = block < 0 ? 0 : block;
			sector->blockbox[BOXBOTTOM] = block;

			block = MapBlock(bbox[BOXRIGHT] - BlockMapOrgX + MAXRADIUS);
			block = block >= BlockMapWidth ? BlockMapWidth - 1 : block;
			sector->blockbox[BOXRIGHT] = block;

			block = MapBlock(bbox[BOXLEFT] - BlockMapOrgX - MAXRADIUS);
			block = block < 0 ? 0 : block;
			sector->blockbox[BOXLEFT] = block;
		}
	}
	unguard;
}

//==========================================================================
//
//  VLevel::LinkNode
//
//==========================================================================

void VLevel::LinkNode(int BSPNum, node_t *pParent) const
{
	guardSlow(LinkNode);
	if (BSPNum & NF_SUBSECTOR)
	{
		int num;

		if (BSPNum == -1)
			num = 0;
		else
			num = BSPNum & (~NF_SUBSECTOR);
	    if (num < 0 || num >= NumSubsectors)
			Host_Error("ss %i with numss = %i", num, NumSubsectors);
		Subsectors[num].parent = pParent;
	}
	else
	{
	    if (BSPNum < 0 || BSPNum >= NumNodes)
			Host_Error("bsp %i with numnodes = %i", NumNodes, NumNodes);
		node_t* bsp = &Nodes[BSPNum];
		bsp->parent = pParent;
	
		LinkNode(bsp->children[0], bsp);
		LinkNode(bsp->children[1], bsp);
	}
	unguardSlow;
}

//==========================================================================
//
//	VLevel::LoadRogueConScript
//
//==========================================================================

void VLevel::LoadRogueConScript(const char *LumpName,
	FRogueConSpeech *&SpeechList, int &NumSpeeches) const
{
	//	Clear variables.
	SpeechList = NULL;
	NumSpeeches = 0;

	//	Check for empty name.
	if (!LumpName || !*LumpName)
		return;

	//	Get lump num.
	int LumpNum = W_CheckNumForName(LumpName);
	if (LumpNum < 0)
		return;	//	Not here.

	//	Load them.
	SpeechList = (FRogueConSpeech *)W_CacheLumpNum(LumpNum, PU_LEVEL);
	NumSpeeches = W_LumpLength(LumpNum) / sizeof(FRogueConSpeech);

	//FIXME byte-swap, extend text strings with ending chars.
}

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
//	Natives
//
//==========================================================================

IMPLEMENT_FUNCTION(VLevel, PointInSector)
{
    TVec Point = PR_Popv();
	VLevel *Self = (VLevel *)PR_Pop();
    PR_Push((int)Self->PointInSubsector(Point)->sector);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2002/09/07 16:34:23  dj_jl
//	Added Level class.
//
//**************************************************************************
