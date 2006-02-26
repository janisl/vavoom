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

enum
{
	LNSPEC_LineTranslucent = 208,
	LNSPEC_TransferHeights = 209,
};

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(V, Level);

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
	int gl_lumpnum = FindGLNodes(mapname);
#ifdef CLIENT
	if (gl_lumpnum < lumpnum)
	{
		W_BuildGLNodes(lumpnum);
		gl_lumpnum = FindGLNodes(mapname);
	}
	else if (strcmp(W_LumpName(gl_lumpnum + ML_GL_PVS), "GL_PVS") ||
		W_LumpLength(gl_lumpnum + ML_GL_PVS) == 0)
	{
		W_BuildPVS(lumpnum, gl_lumpnum);
		lumpnum = W_GetNumForName(mapname);
		gl_lumpnum = FindGLNodes(mapname);
	}
#endif
	if (gl_lumpnum < lumpnum)
	{
		Host_Error("Map %s is missing GL-Nodes\n", mapname);
	}
	bExtended = !strcmp(W_LumpName(lumpnum + ML_BEHAVIOR), "BEHAVIOR");

	//
	// Begin processing map lumps
	// Note: most of this ordering is important
	//
	bGLNodesV5 = false;
	LoadVertexes(lumpnum + ML_VERTEXES, gl_lumpnum + ML_GL_VERT);
	LoadSectors(lumpnum + ML_SECTORS);
	LoadSideDefsPass1(lumpnum + ML_SIDEDEFS);
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
#ifdef SERVER
			//	ACS object code
			P_LoadACScripts(lumpnum + ML_BEHAVIOR);
#endif
		}
		else
		{
			LoadThings1(lumpnum + ML_THINGS);
#ifdef SERVER
			mapInfo_t mi;
			P_GetMapInfo(mapname, mi);
			if (mi.acsLump[0])
			{
				//	ACS object code
				P_LoadACScripts(W_GetNumForName(mi.acsLump, WADNS_ACSLibrary));
			}
			else
			{
				//	Inform ACS, that we don't have scripts
				P_LoadACScripts(-1);
			}
#endif
		}
	}

#ifdef SERVER
	if (!bExtended)
	{
		//	Translate level to Hexen format
		GGameInfo->eventTranslateLevel(this);
	}
#endif
	//	Set up textures after loading lines because for some Boom line
	// specials there can be special meaning of some texture names.
	LoadSideDefsPass2(lumpnum + ML_SIDEDEFS);

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
//  VLevel::FindGLNodes
//
//==========================================================================

int VLevel::FindGLNodes(const char* name) const
{
	guard(VLevel::FindGLNodes);
	if (strlen(name) < 6)
	{
		return W_CheckNumForName(va("GL_%s", name));
	}

	//	Long map name, check GL_LEVEL lumps.
	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (stricmp(W_LumpName(Lump), "GL_LEVEL"))
		{
			continue;
		}
		if (W_LumpLength(Lump) < 12)
		{
			//	Lump is too short.
			continue;
		}
		char* Ptr = (char*)W_CacheLumpNum(Lump, PU_STATIC);
		if (memcmp(Ptr, "LEVEL=", 6))
		{
			//	LEVEL keyword expected, but missing.
			Z_Free(Ptr);
			continue;
		}
		for (int i = 11; i < 14; i++)
		{
			if (Ptr[i] == '\n' || Ptr[i] == '\r')
			{
				Ptr[i] = 0;
				break;
			}
		}
		Ptr[14] = 0;
		if (!stricmp(Ptr + 6, name))
		{
			Z_Free(Ptr);
			return Lump;
		}
		Z_Free(Ptr);
	}
	return -1;
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
	if (!strncmp((char*)Data, GL_V2_MAGIC, 4) ||
		!strncmp((char*)Data, GL_V5_MAGIC, 4))
	{
		gl_mapvertex_t *pGLSrc;

		if (!strncmp((char*)Data, GL_V5_MAGIC, 4))
		{
GCon->Logf("Version5 nodes");
			bGLNodesV5 = true;
		}
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
		ss->floor.LightSourceSector = -1;

		//	Ceiling
		ss->ceiling.Set(TVec(0, 0, -1), -LittleShort(ms->ceilingheight));
		ss->ceiling.pic = FTNumForName(ms->ceilingpic);
		ss->ceiling.base_pic = ss->ceiling.pic;
		ss->ceiling.xoffs = 0;
		ss->ceiling.yoffs = 0;
		ss->ceiling.minz = LittleShort(ms->ceilingheight);
		ss->ceiling.maxz = LittleShort(ms->ceilingheight);
		ss->ceiling.LightSourceSector = -1;

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
//  VLevel::LoadSideDefsPass1
//
//==========================================================================

void VLevel::LoadSideDefsPass1(int Lump)
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

		sd->base_textureoffset = sd->textureoffset;
		sd->base_rowoffset = sd->rowoffset;
	}

	Z_Free(data);
	unguard;
}

//==========================================================================
//
//  VLevel::LoadSideDefsPass2
//
//==========================================================================

void VLevel::LoadSideDefsPass2(int Lump)
{
	guard(VLevel::LoadSideDefs);
	byte *data;
	int i;
	mapsidedef_t *msd;
	side_t *sd;

	//	Assign line specials to sidedefs midtexture and arg1 to toptexture.
	for (i = 0; i < NumLines; i++)
	{
		if (Lines[i].sidenum[0] != -1)
		{
			Sides[Lines[i].sidenum[0]].midtexture = Lines[i].special;
			Sides[Lines[i].sidenum[0]].toptexture = Lines[i].arg1;
		}
		if (Lines[i].sidenum[1] != -1)
		{
			Sides[Lines[i].sidenum[1]].midtexture = Lines[i].special;
			Sides[Lines[i].sidenum[1]].toptexture = Lines[i].arg1;
		}
	}

	data = (byte*)W_CacheLumpNum(Lump, PU_STATIC);
	msd = (mapsidedef_t *)data;
	sd = Sides;

	for (i = 0; i < NumSides; i++, msd++, sd++)
	{
		switch (sd->midtexture)
		{
		case LNSPEC_LineTranslucent:
			//	In BOOM midtexture can be translucency table lump name.
			sd->midtexture = GTextureManager.CheckNumForName(
				FName(msd->midtexture, FNAME_AddLower8),
				TEXTYPE_Wall, true, true);
			if (sd->midtexture == -1)
			{
				sd->midtexture = 0;
			}
			sd->toptexture = TFNumForName(msd->toptexture);
			sd->bottomtexture = TFNumForName(msd->bottomtexture);
			break;

		case LNSPEC_TransferHeights:
			sd->midtexture = CMapTFNumForName(msd->midtexture);
			sd->toptexture = CMapTFNumForName(msd->toptexture);
			sd->bottomtexture = CMapTFNumForName(msd->bottomtexture);
			break;

		default:
			sd->midtexture = TFNumForName(msd->midtexture);
			sd->toptexture = TFNumForName(msd->toptexture);
			sd->bottomtexture = TFNumForName(msd->bottomtexture);
			break;
		}

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
	seg_t *li;
	line_t *ldef;
	int linedef;
	int side;

	NumSegs = W_LumpLength(Lump) / sizeof(mapglseg_t);
	Segs = Z_CNew<seg_t>(NumSegs, PU_LEVEL, 0);
	data = W_CacheLumpNum(Lump, PU_STATIC);

	if (bGLNodesV5 || !strncmp((char*)data, GL_V3_MAGIC, 4))
	{
		int HdrSize = bGLNodesV5 ? 0 : 4;
		dword GLVertFlag = bGLNodesV5 ? GL_VERTEX_V5 : GL_VERTEX_V3;

		NumSegs = (W_LumpLength(Lump) - HdrSize) / sizeof(mapglseg_v3_t);
		Z_Resize((void**)&Segs, NumSegs * sizeof(seg_t));

		mapglseg_v3_t* ml = (mapglseg_v3_t*)((byte*)data + HdrSize);
		li = Segs;

		for (i = 0; i < NumSegs; i++, li++, ml++)
		{
			dword	v1num =	LittleLong(ml->v1);
			dword	v2num =	LittleLong(ml->v2);
	
			if (v1num & GLVertFlag)
			{
				v1num ^= GLVertFlag;
				li->v1 = &gl_vertexes[v1num];
			}
			else
			{
				li->v1 = &Vertexes[v1num];
			}
			if (v2num & GLVertFlag)
			{
				v2num ^= GLVertFlag;
				li->v2 = &gl_vertexes[v2num];
			}
			else
			{
				li->v2 = &Vertexes[v2num];
			}

			linedef = LittleShort(ml->linedef);
			side = LittleShort(ml->flags) & GL_SEG_FLAG_SIDE;

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
	}
	else
	{
		mapglseg_t* ml = (mapglseg_t *)data;
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
	subsector_t *ss;
	seg_t *seg;

	NumSubsectors = W_LumpLength(Lump) / sizeof(mapsubsector_t);
	Subsectors = Z_CNew<subsector_t>(NumSubsectors, PU_LEVEL, 0);
	data = W_CacheLumpNum(Lump, PU_STATIC);

	if (bGLNodesV5 || !strncmp((char*)data, GL_V3_MAGIC, 4))
	{
		int HdrSize = bGLNodesV5 ? 0 : 4;

		NumSubsectors = (W_LumpLength(Lump) - HdrSize) /
			sizeof(mapglsubsector_v3_t);
		Z_Resize((void**)&Subsectors, NumSubsectors * sizeof(subsector_t));

		mapglsubsector_v3_t* ms = (mapglsubsector_v3_t*)((byte*)data +
			HdrSize);
		ss = Subsectors;
	
		for (i = 0; i < NumSubsectors; i++, ss++, ms++)
		{
			ss->numlines = LittleLong(ms->numsegs);
			ss->firstline = LittleLong(ms->firstseg);
	
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
	}
	else
	{
		mapsubsector_t* ms = (mapsubsector_t *)data;
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
	node_t *no;

	if (bGLNodesV5)
	{
		NumNodes = W_LumpLength(Lump) / sizeof(mapglnode_v5_t);
		Nodes = Z_CNew<node_t>(NumNodes, PU_LEVEL, 0);
		data = (byte*)W_CacheLumpNum(Lump, PU_STATIC);
		mapglnode_v5_t* mn = (mapglnode_v5_t*)data;
		no = Nodes;

		for (int i = 0; i < NumNodes; i++, no++, mn++)
		{
			no->SetPointDir(TVec(LittleShort(mn->x), LittleShort(mn->y), 0),
				TVec(LittleShort(mn->dx), LittleShort(mn->dy), 0));

			for (int j = 0; j < 2; j++)
			{
				no->children[j] = LittleLong(mn->children[j]);
				no->bbox[j][0] = LittleShort(mn->bbox[j][BOXLEFT]);
				no->bbox[j][1] = LittleShort(mn->bbox[j][BOXBOTTOM]);
				no->bbox[j][2] = -32768.0;
				no->bbox[j][3] = LittleShort(mn->bbox[j][BOXRIGHT]);
				no->bbox[j][4] = LittleShort(mn->bbox[j][BOXTOP]);
				no->bbox[j][5] = 32768.0;
			}
		}
	}
	else
	{
		NumNodes = W_LumpLength(Lump) / sizeof(mapnode_t);
		Nodes = Z_CNew<node_t>(NumNodes, PU_LEVEL, 0);
		data = (byte*)W_CacheLumpNum(Lump, PU_STATIC);
		mapnode_t* mn = (mapnode_t *)data;
		no = Nodes;

		for (int i = 0; i < NumNodes; i++, no++, mn++)
		{
			no->SetPointDir(TVec(LittleShort(mn->x), LittleShort(mn->y), 0),
				TVec(LittleShort(mn->dx), LittleShort(mn->dy), 0));

			for (int j = 0; j < 2; j++)
			{
				no->children[j] = (word)LittleShort(mn->children[j]);
				if (no->children[j] & NF_SUBSECTOR_OLD)
				{
					no->children[j] &= ~NF_SUBSECTOR_OLD;
					no->children[j] |= NF_SUBSECTOR;
				}
				no->bbox[j][0] = LittleShort(mn->bbox[j][BOXLEFT]);
				no->bbox[j][1] = LittleShort(mn->bbox[j][BOXBOTTOM]);
				no->bbox[j][2] = -32768.0;
				no->bbox[j][3] = LittleShort(mn->bbox[j][BOXRIGHT]);
				no->bbox[j][4] = LittleShort(mn->bbox[j][BOXTOP]);
				no->bbox[j][5] = 32768.0;
			}
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
	FName Name(name, FNAME_AddLower8);
	int i = GTextureManager.CheckNumForName(Name, TEXTYPE_Flat, true, true);
	if (i == -1)
	{
		if (ignore_missing_textures)
		{
			GCon->Logf("FTNumForName: %s not found", *Name);
			return 0;
		}
		Host_Error("FTNumForName: %s not found", *Name);
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
	FName Name(name, FNAME_AddLower8);
	int i = GTextureManager.CheckNumForName(Name, TEXTYPE_Wall, true, true);
	if (i == -1)
	{
		if (ignore_missing_textures)
		{
			GCon->Logf("TFNumForName: %s not found", *Name);
			return 0;
		}
		Host_Error("TFNumForName: %s not found", *Name);
	}
	return i;
	unguard;
}

//==========================================================================
//
//  VLevel::CMapTFNumForName
//
//  Retrieval, get a texture or flat number for a name.
//
//==========================================================================

int VLevel::CMapTFNumForName(const char *name) const
{
	guard(VLevel::CMapTFNumForName);
	int i = GTextureManager.CheckNumForName(FName(name, FNAME_AddLower8),
		TEXTYPE_Wall, true, true);
	if (i == -1)
	{
		return 0;
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
	SpeechList = (FRogueConSpeech *)Z_Malloc(W_LumpLength(LumpNum), PU_LEVEL, 0);
	memcpy(SpeechList, W_CacheLumpNum(LumpNum, PU_CACHE), W_LumpLength(LumpNum));
	NumSpeeches = W_LumpLength(LumpNum) / sizeof(FRogueConSpeech);

	for (int i = 0; i < NumSpeeches; i++)
	{
		FRogueConSpeech& S = SpeechList[i];
		S.SpeakerID = LittleLong(S.SpeakerID);
		S.Unknown1 = LittleLong(S.Unknown1);
		S.Unknown2 = LittleLong(S.Unknown2);
		S.Unknown3 = LittleLong(S.Unknown3);
		S.Unknown4 = LittleLong(S.Unknown4);
		S.Unknown5 = LittleLong(S.Unknown5);
		for (int j = 0; j < 5; j++)
		{
			S.Choices[j].Unknown1 = LittleLong(S.Choices[j].Unknown1);
			S.Choices[j].Unknown2 = LittleLong(S.Choices[j].Unknown2);
			S.Choices[j].Unknown3 = LittleLong(S.Choices[j].Unknown3);
			S.Choices[j].Unknown4 = LittleLong(S.Choices[j].Unknown4);
			S.Choices[j].Unknown5 = LittleLong(S.Choices[j].Unknown5);
			S.Choices[j].Unknown6 = LittleLong(S.Choices[j].Unknown6);
			S.Choices[j].Unknown7 = LittleLong(S.Choices[j].Unknown7);
			S.Choices[j].Next = LittleLong(S.Choices[j].Next);
			S.Choices[j].Objectives = LittleLong(S.Choices[j].Objectives);
		}
	}
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
	VStruct* SSector = svpr.FindStruct("sector_t", VObject::StaticClass());
	for (i = 0, sec = Sectors; i < NumSectors; i++, sec++)
	{
		Strm << sec->floor.dist
			<< sec->ceiling.dist
			<< sec->floor.pic
			<< sec->ceiling.pic
			<< sec->params.lightlevel
			<< sec->special
			<< sec->tag
			<< sec->seqType;
		if (Strm.IsLoading())
		{
			CalcSecMinMaxs(sec);
		}
		//	Serialise added fields.
		for (VField* F = SSector->Fields; F; F = F->Next)
		{
			byte* Data = (byte*)sec + F->Ofs;
			if (Data >= (byte*)sec->user_fields)
			{
				VField::SerialiseFieldValue(Strm, Data, F->Type);
			}
		}
	}

	//
	//	Lines
	//
	VStruct* SLine = svpr.FindStruct("line_t", VObject::StaticClass());
	for (i = 0, li = Lines; i < NumLines; i++, li++)
	{
		//	Temporary hack to save seen on automap flags.
#ifdef CLIENT
		if (cls.state == ca_connected)
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
			<< li->arg5;
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
		//	Serialise added fields.
		for (VField* F = SLine->Fields; F; F = F->Next)
		{
			byte* Data = (byte*)li + F->Ofs;
			if (Data >= (byte*)li->user_fields)
			{
				VField::SerialiseFieldValue(Strm, Data, F->Type);
			}
		}
	}

	//
	//	Polyobjs
	//
	VStruct* SPolyobj = svpr.FindStruct("polyobj_t", VObject::StaticClass());
	for (i = 0; i < NumPolyObjs; i++)
	{
		if (Strm.IsLoading())
		{
			float angle, polyX, polyY;

			Strm << angle 
				<< polyX 
				<< polyY;
			PO_RotatePolyobj(PolyObjs[i].tag, angle);
			PO_MovePolyobj(PolyObjs[i].tag, 
				polyX - PolyObjs[i].startSpot.x, 
				polyY - PolyObjs[i].startSpot.y);
		}
		else
		{
			Strm << PolyObjs[i].angle
				<< PolyObjs[i].startSpot.x
				<< PolyObjs[i].startSpot.y;
		}
		//	Serialise added fields.
		for (VField* F = SPolyobj->Fields; F; F = F->Next)
		{
			byte* Data = (byte*)&PolyObjs[i] + F->Ofs;
			if (Data >= (byte*)PolyObjs[i].user_fields)
			{
				VField::SerialiseFieldValue(Strm, Data, F->Type);
			}
		}
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
//	Revision 1.10  2006/02/26 20:52:48  dj_jl
//	Proper serialisation of level and players.
//
//	Revision 1.9  2006/02/13 18:34:34  dj_jl
//	Moved all server progs global functions to classes.
//	
//	Revision 1.8  2006/02/11 12:21:10  dj_jl
//	Byte swap Strife conversation lumps.
//	
//	Revision 1.7  2005/11/14 19:34:16  dj_jl
//	Added support for version 5 GL nodes.
//	
//	Revision 1.6  2005/05/26 16:52:29  dj_jl
//	Created texture manager class
//	
//	Revision 1.5  2005/03/28 07:28:19  dj_jl
//	Transfer lighting and other BOOM stuff.
//	
//	Revision 1.4  2004/12/03 16:15:46  dj_jl
//	Implemented support for extended ACS format scripts, functions, libraries and more.
//	
//	Revision 1.3  2004/10/11 15:55:43  dj_jl
//	Support for version 3 GL nodes and ACS helpers.
//	
//	Revision 1.2  2004/08/21 15:03:07  dj_jl
//	Remade VClass to be standalone class.
//	
//	Revision 1.1  2002/09/07 16:34:23  dj_jl
//	Added Level class.
//	
//**************************************************************************
