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
//**    Copyright (C) 1999-2006 Jānis Legzdiņš
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
#include "zipstream.h"
#ifdef SERVER
#include "sv_local.h"
#endif

// MACROS ------------------------------------------------------------------

// Lump order in a map WAD: each map needs a couple of lumps
// to provide a complete scene geometry description.
enum
{
	ML_LABEL,		// A separator, name, ExMx or MAPxx
	ML_THINGS,		// Monsters, items..
	ML_LINEDEFS,	// LineDefs, from editing
	ML_SIDEDEFS,	// SideDefs, from editing
	ML_VERTEXES,	// Vertices, edited and BSP splits generated
	ML_SEGS,		// LineSegs, from LineDefs split by BSP
	ML_SSECTORS,	// SubSectors, list of LineSegs
	ML_NODES,		// BSP nodes
	ML_SECTORS,		// Sectors, from editing
	ML_REJECT,		// LUT, sector-sector visibility
	ML_BLOCKMAP,	// LUT, motion clipping, walls/grid element
	ML_BEHAVIOR		// ACS scripts
};

//	Lump order from "GL-Friendly Nodes" specs.
enum
{
	ML_GL_LABEL,	// A separator name, GL_ExMx or GL_MAPxx
	ML_GL_VERT,		// Extra Vertices
	ML_GL_SEGS,		// Segs, from linedefs & minisegs
	ML_GL_SSECT,	// SubSectors, list of segs
	ML_GL_NODES,	// GL BSP nodes
	ML_GL_PVS		// Potentially visible set
};

//	GL-node version identifiers.
#define GL_V2_MAGIC			"gNd2"
#define GL_V3_MAGIC			"gNd3"
#define GL_V5_MAGIC			"gNd5"
#define ZGL_MAGIC			"ZGLN"

//	Indicates a GL-specific vertex.
#define	GL_VERTEX			0x8000
#define	GL_VERTEX_V3		0x40000000
#define	GL_VERTEX_V5		0x80000000

//	GL-seg flags.
#define GL_SEG_FLAG_SIDE	0x0001

//	Old subsector flag.
#define	NF_SUBSECTOR_OLD	0x8000

//	Line specials used in sidedef loading.
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

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VCvarI strict_level_errors("strict_level_errors", "1");
static VCvarI build_blockmap("build_blockmap", "0", CVAR_Archive);
static VCvarI build_gwa("build_gwa", "1");

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VLevel::LoadMap
//
//==========================================================================

void VLevel::LoadMap(VName AMapName)
{
	guard(VLevel::LoadMap);
	bool AuxiliaryMap = false;
	int lumpnum;
	VName MapLumpName;

	MapName = AMapName;
	//	If working with a devlopment map, reload it.
	VStr aux_file_name = va("maps/%s.wad", *MapName);
	if (FL_FileExists(aux_file_name))
	{
		lumpnum = W_OpenAuxiliary(aux_file_name);
		MapLumpName = W_LumpName(lumpnum);
		AuxiliaryMap = true;
	}
	else
	{
		//	Find map and GL nodes.
		MapLumpName = MapName;
		lumpnum = W_CheckNumForName(MapName);
		if (lumpnum < 0)
		{
			Host_Error("Map %s not found\n", *MapName);
		}
	}

	//	Verify that it's a valid map.
	if (W_LumpName(lumpnum + ML_THINGS) != "things" ||
		W_LumpName(lumpnum + ML_LINEDEFS) != "linedefs" ||
		W_LumpName(lumpnum + ML_SIDEDEFS) != "sidedefs" ||
		W_LumpName(lumpnum + ML_VERTEXES) != "vertexes" ||
		W_LumpName(lumpnum + ML_SEGS) != "segs" ||
		W_LumpName(lumpnum + ML_SSECTORS) != "ssectors" ||
		W_LumpName(lumpnum + ML_NODES) != "nodes" ||
		W_LumpName(lumpnum + ML_SECTORS) != "sectors" ||
		W_LumpName(lumpnum + ML_REJECT) != "reject" ||
		W_LumpName(lumpnum + ML_BLOCKMAP) != "blockmap")
	{
		Host_Error("Map %s is not a valid map", *MapName);
	}

	int gl_lumpnum = -100;
	bool UseComprGLNodes = false;
	VStream* TmpStrm = W_CreateLumpReaderNum(lumpnum + ML_SSECTORS);
	if (TmpStrm->TotalSize() > 4)
	{
		char Hdr[4];
		TmpStrm->Serialise(Hdr, 4);
		if (!VStr::NCmp(Hdr, ZGL_MAGIC, 4))
		{
			UseComprGLNodes = true;
		}
	}
	delete TmpStrm;

	bool NeedNodesBuild = false;
	if (!UseComprGLNodes)
	{
		gl_lumpnum = FindGLNodes(MapLumpName);
#ifdef CLIENT
		if (build_gwa)
		{
			//	If missing GL nodes or VIS data, then build them.
			if (gl_lumpnum < lumpnum)
			{
				W_BuildGLNodes(lumpnum);
				gl_lumpnum = FindGLNodes(MapLumpName);
			}
			else if (W_LumpName(gl_lumpnum + ML_GL_PVS) != NAME_gl_pvs ||
				W_LumpLength(gl_lumpnum + ML_GL_PVS) == 0)
			{
				W_BuildPVS(lumpnum, gl_lumpnum);
				lumpnum = W_GetNumForName(MapLumpName);
				gl_lumpnum = FindGLNodes(MapLumpName);
			}
		}
#endif
		if (gl_lumpnum < lumpnum)
		{
			if (build_gwa)
			{
				Host_Error("Map %s is missing GL-Nodes\n", *MapName);
			}
			else
			{
				NeedNodesBuild = true;
			}
		}
	}

	//	Determine level format.
	if (W_LumpName(lumpnum + ML_BEHAVIOR) == NAME_behavior)
		LevelFlags |= LF_Extended;

	//
	//	Begin processing map lumps.
	// Note: most of this ordering is important
	//
	int NumBaseVerts;
	LevelFlags &= ~LF_GLNodesV5;
	LoadVertexes(lumpnum + ML_VERTEXES, gl_lumpnum + ML_GL_VERT, NumBaseVerts);
	LoadSectors(lumpnum + ML_SECTORS);
	LoadSideDefsPass1(lumpnum + ML_SIDEDEFS);
	if (!(LevelFlags & LF_Extended))
	{
		LoadLineDefs1(lumpnum + ML_LINEDEFS, NumBaseVerts);
	}
	else
	{
		LoadLineDefs2(lumpnum + ML_LINEDEFS, NumBaseVerts);
	}

	if (NeedNodesBuild)
	{
		BuildNodes(lumpnum);
	}
	else if (UseComprGLNodes)
	{
		LoadCompressedGLNodes(lumpnum + ML_SSECTORS);
	}
	else
	{
		LoadGLSegs(gl_lumpnum + ML_GL_SEGS, NumBaseVerts);
		LoadSubsectors(gl_lumpnum + ML_GL_SSECT);
		LoadNodes(gl_lumpnum + ML_GL_NODES);
		LoadPVS(gl_lumpnum + ML_GL_PVS);
	}
	LoadBlockMap(lumpnum + ML_BLOCKMAP);
	LoadReject(lumpnum + ML_REJECT);

	if (!(LevelFlags & LF_Extended))
	{
		LoadThings1(lumpnum + ML_THINGS);
	}
	else
	{
		LoadThings2(lumpnum + ML_THINGS);
	}
	//	ACS object code
	if (LevelFlags & LF_Extended)
	{
		LoadACScripts(lumpnum + ML_BEHAVIOR);
	}
	else
	{
		LoadACScripts(-1);
	}

	if (!(LevelFlags & LF_Extended))
	{
		//	Translate level to Hexen format
		GGameInfo->eventTranslateLevel(this);
	}
	//	Set up textures after loading lines because for some Boom line
	// specials there can be special meaning of some texture names.
	LoadSideDefsPass2(lumpnum + ML_SIDEDEFS);

	GroupLines();

	//	Load conversations.
	LoadRogueConScript(GGameInfo->GenericConScript, GenericSpeeches,
		NumGenericSpeeches);
	LoadRogueConScript(GGameInfo->eventGetConScriptName(MapName),
		LevelSpeeches, NumLevelSpeeches);

	//	Set up polyobjs, slopes, 3D floors and some other static stuff.
	GGameInfo->eventSpawnWorld(this);
	InitPolyobjs();	// Initialise the polyobjs

	//	We need this for client.
	for (int i = 0; i < NumSectors; i++)
	{
		CalcSecMinMaxs(&Sectors[i]);
	}

	CreateRepBase();

	//
	// End of map lump processing
	//
	if (AuxiliaryMap)
	{
		// Close the auxiliary file.
		W_CloseAuxiliary();
	}
	unguard;
}

//==========================================================================
//
//  VLevel::FindGLNodes
//
//==========================================================================

int VLevel::FindGLNodes(VName name) const
{
	guard(VLevel::FindGLNodes);
	if (VStr::Length(*name) < 6)
	{
		return W_CheckNumForName(VName(va("gl_%s", *name), VName::AddLower8));
	}

	//	Long map name, check GL_LEVEL lumps.
	for (int Lump = W_IterateNS(-1, WADNS_Global); Lump >= 0;
		Lump = W_IterateNS(Lump, WADNS_Global))
	{
		if (W_LumpName(Lump) != NAME_gl_level)
		{
			continue;
		}
		if (W_LumpLength(Lump) < 12)
		{
			//	Lump is too short.
			continue;
		}
		char Buf[16];
		VStream* Strm = W_CreateLumpReaderNum(Lump);
		Strm->Serialise(Buf, Strm->TotalSize() < 16 ? Strm->TotalSize() : 16);
		delete Strm;
		if (memcmp(Buf, "LEVEL=", 6))
		{
			//	LEVEL keyword expected, but missing.
			continue;
		}
		for (int i = 11; i < 14; i++)
		{
			if (Buf[i] == '\n' || Buf[i] == '\r')
			{
				Buf[i] = 0;
				break;
			}
		}
		Buf[14] = 0;
		if (!VStr::ICmp(Buf + 6, *name))
		{
			return Lump;
		}
	}
	return -1;
	unguard;
}

//==========================================================================
//
//	VLevel::LoadVertexes
//
//==========================================================================

void VLevel::LoadVertexes(int Lump, int GLLump, int& NumBaseVerts)
{
	guard(VLevel::LoadVertexes);
	int GlFormat = 0;
	if (GLLump >= 0)
	{
		//	Read header of the GL vertexes lump and determinte GL vertex format.
		char Magic[4];
		W_ReadFromLump(GLLump, Magic, 0, 4);
		GlFormat = !VStr::NCmp((char*)Magic, GL_V2_MAGIC, 4) ? 2 :
			!VStr::NCmp((char*)Magic, GL_V5_MAGIC, 4) ? 5 : 1;
		if (GlFormat ==  5)
		{
			LevelFlags |= LF_GLNodesV5;
		}
	}

	//	Determine number of vertexes: total lump length / vertex record length.
	NumBaseVerts = W_LumpLength(Lump) / 4;
	int NumGLVerts = GlFormat == 0 ? 0 : GlFormat == 1 ?
		(W_LumpLength(GLLump) / 4) : ((W_LumpLength(GLLump) - 4) / 8);
	NumVertexes = NumBaseVerts + NumGLVerts;

	//	Allocate memory for vertexes.
	Vertexes = new vertex_t[NumVertexes];

	//	Load base vertexes.
	VStream* Strm = W_CreateLumpReaderNum(Lump);
	vertex_t* pDst = Vertexes;
	for (int i = 0; i < NumBaseVerts; i++, pDst++)
	{
		vint16 x, y;
		*Strm << x << y;
		*pDst = TVec(x, y, 0);
	}
	delete Strm;

	if (GLLump >= 0)
	{
		//	Load GL vertexes.
		Strm = W_CreateLumpReaderNum(GLLump);
		if (GlFormat == 1)
		{
			//	GL version 1 vertexes, same as normal ones.
			for (int i = 0; i < NumGLVerts; i++, pDst++)
			{
				vint16 x, y;
				*Strm << x << y;
				*pDst = TVec(x, y, 0);
			}
		}
		else
		{
			//	GL version 2 or greater vertexes, as fixed point.
			Strm->Seek(4);
			for (int i = 0; i < NumGLVerts; i++, pDst++)
			{
				vint32 x, y;
				*Strm << x << y;
				*pDst = TVec(x / 65536.0, y / 65536.0, 0);
			}
		}
		delete Strm;
	}
	unguard;
}

//==========================================================================
//
//	VLevel::LoadSectors
//
//==========================================================================

void VLevel::LoadSectors(int Lump)
{
	guard(VLevel::LoadSectors);
	//	Allocate memory for sectors.
	NumSectors = W_LumpLength(Lump) / 26;
	Sectors = new sector_t[NumSectors];
	memset(Sectors, 0, sizeof(sector_t) * NumSectors);

	//	Load sectors.
	VStream* Strm = W_CreateLumpReaderNum(Lump);
	sector_t* ss = Sectors;
	for (int i = 0; i < NumSectors; i++, ss++)
	{
		//	Read data.
		vint16 floorheight, ceilingheight, lightlevel, special, tag;
		char floorpic[8];
		char ceilingpic[8];
		*Strm << floorheight << ceilingheight;
		Strm->Serialise(floorpic, 8);
		Strm->Serialise(ceilingpic, 8);
		*Strm << lightlevel << special << tag;

		//	Floor
		ss->floor.Set(TVec(0, 0, 1), floorheight);
		ss->floor.pic = TexNumForName(floorpic, TEXTYPE_Flat);
		ss->floor.xoffs = 0;
		ss->floor.yoffs = 0;
		ss->floor.minz = floorheight;
		ss->floor.maxz = floorheight;
		ss->floor.Alpha = 1.0;
		ss->floor.LightSourceSector = -1;

		//	Ceiling
		ss->ceiling.Set(TVec(0, 0, -1), -ceilingheight);
		ss->ceiling.pic = TexNumForName(ceilingpic, TEXTYPE_Flat);
		ss->ceiling.xoffs = 0;
		ss->ceiling.yoffs = 0;
		ss->ceiling.minz = ceilingheight;
		ss->ceiling.maxz = ceilingheight;
		ss->ceiling.Alpha = 1.0;
		ss->ceiling.LightSourceSector = -1;

		//	Params
		ss->params.lightlevel = lightlevel;
		ss->params.LightColour = 0x00ffffff;

		//	Region
		sec_region_t* region = new sec_region_t;
		memset(region, 0, sizeof(*region));
		region->floor = &ss->floor;
		region->ceiling = &ss->ceiling;
		region->params = &ss->params;
		ss->topregion = region;
		ss->botregion = region;

		ss->floorheight = floorheight;
		ss->ceilingheight = ceilingheight;
		ss->special = special;
		ss->tag = tag;
		ss->seqType = -1;	// default seqType

		ss->Gravity = 1.0;	// default sector gravity of 1.0
	}
	delete Strm;
	unguard;
}

//==========================================================================
//
//	VLevel::LoadSideDefsPass1
//
//	Pass 1: Create side defs and load data, except texture information.
//
//==========================================================================

void VLevel::LoadSideDefsPass1(int Lump)
{
	guard(VLevel::LoadSideDefs);
	//	Allocate memory for side defs.
	NumSides = W_LumpLength(Lump) / 30;
	Sides = new side_t[NumSides];
	memset(Sides, 0, sizeof(side_t) * NumSides);

	//	Load data.
	VStream* Strm = W_CreateLumpReaderNum(Lump);
	side_t* sd = Sides;
	for (int i = 0; i < NumSides; i++, sd++)
	{
		vint16 textureoffset, rowoffset, sector;
		*Strm << textureoffset << rowoffset;
		Strm->Seek(Strm->Tell() + 24);	//	Skip texture names.
		*Strm << sector;

		if (sector < 0 || sector >= NumSectors)
			Host_Error("Bad sector index %d", sector);

		sd->textureoffset = textureoffset;
		sd->rowoffset = rowoffset;
		sd->sector = &Sectors[sector];
	}
	delete Strm;
	unguard;
}

//==========================================================================
//
//	VLevel::LoadSideDefsPass2
//
//	Pass 2: Load texture definitions. It's been split out because in Boom
// meaning of some texture names depend on line special.
//
//==========================================================================

void VLevel::LoadSideDefsPass2(int Lump)
{
	guard(VLevel::LoadSideDefs);
	//	Assign line specials to sidedefs midtexture and arg1 to toptexture.
	for (int i = 0; i < NumLines; i++)
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

	//	Load data.
	VStream* Strm = W_CreateLumpReaderNum(Lump);
	side_t* sd = Sides;
	for (int i = 0; i < NumSides; i++, sd++)
	{
		char toptexture[8];
		char bottomtexture[8];
		char midtexture[8];
		Strm->Seek(Strm->Tell() + 4);
		Strm->Serialise(toptexture, 8);
		Strm->Serialise(bottomtexture, 8);
		Strm->Serialise(midtexture, 8);
		Strm->Seek(Strm->Tell() + 2);

		switch (sd->midtexture)
		{
		case LNSPEC_LineTranslucent:
			//	In BOOM midtexture can be translucency table lump name.
			sd->midtexture = GTextureManager.CheckNumForName(
				VName(midtexture, VName::AddLower8),
				TEXTYPE_Wall, true, true);
			if (sd->midtexture == -1)
			{
				sd->midtexture = 0;
			}
			sd->toptexture = TexNumForName(toptexture, TEXTYPE_Wall);
			sd->bottomtexture = TexNumForName(bottomtexture, TEXTYPE_Wall);
			break;

		case LNSPEC_TransferHeights:
			sd->midtexture = TexNumForName(midtexture, TEXTYPE_Wall, true);
			sd->toptexture = TexNumForName(toptexture, TEXTYPE_Wall, true);
			sd->bottomtexture = TexNumForName(bottomtexture, TEXTYPE_Wall, true);
			break;

		default:
			sd->midtexture = TexNumForName(midtexture, TEXTYPE_Wall);
			sd->toptexture = TexNumForName(toptexture, TEXTYPE_Wall);
			sd->bottomtexture = TexNumForName(bottomtexture, TEXTYPE_Wall);
			break;
		}
	}
	delete Strm;
	unguard;
}

//==========================================================================
//
//  VLevel::LoadLineDefs1
//
//  For Doom and Heretic
//
//==========================================================================

void VLevel::LoadLineDefs1(int Lump, int NumBaseVerts)
{
	guard(VLevel::LoadLineDefs1);
	NumLines = W_LumpLength(Lump) / 14;
	Lines = new line_t[NumLines];
	memset(Lines, 0, sizeof(line_t) * NumLines);

	VStream* Strm = W_CreateLumpReaderNum(Lump);
	line_t* ld = Lines;
	for (int i = 0; i < NumLines; i++, ld++)
	{
		vint16 v1, v2, flags, special, tag, side0, side1;
		*Strm << v1 << v2 << flags << special << tag << side0 << side1;

		if (v1 < 0 || v1 >= NumBaseVerts)
			Host_Error("Bad vertex index %d", v1);
		if (v2 < 0 || v2 >= NumBaseVerts)
			Host_Error("Bad vertex index %d", v2);

		ld->flags = flags;
		ld->special = special;
		ld->arg1 = tag;
		ld->v1 = &Vertexes[v1];
		ld->v2 = &Vertexes[v2];
		ld->sidenum[0] = side0;
		ld->sidenum[1] = side1;
		SetupLineSides(ld);
	}
	delete Strm;
	unguard;
}

//==========================================================================
//
//  VLevel::LoadLineDefs2
//
//  For Hexen
//
//==========================================================================

void VLevel::LoadLineDefs2(int Lump, int NumBaseVerts)
{
	guard(VLevel::LoadLineDefs2);
	NumLines = W_LumpLength(Lump) / 16;
	Lines = new line_t[NumLines];
	memset(Lines, 0, sizeof(line_t) * NumLines);

	VStream* Strm = W_CreateLumpReaderNum(Lump);
	line_t* ld = Lines;
	for (int i = 0; i < NumLines; i++, ld++)
	{
		vint16 v1, v2, flags;
		vuint8 special, arg1, arg2, arg3, arg4, arg5;
		vint16 side0, side1;
		*Strm << v1 << v2 << flags << special << arg1 << arg2 << arg3 << arg4
			<< arg5 << side0 << side1;

		if (v1 < 0 || v1 >= NumBaseVerts)
			Host_Error("Bad vertex index %d", v1);
		if (v2 < 0 || v2 >= NumBaseVerts)
			Host_Error("Bad vertex index %d", v2);

		ld->flags = flags;

		// New line special info ...
		ld->special = special;
		ld->arg1 = arg1;
		ld->arg2 = arg2;
		ld->arg3 = arg3;
		ld->arg4 = arg4;
		ld->arg5 = arg5;

		ld->v1 = &Vertexes[v1];
		ld->v2 = &Vertexes[v2];
		ld->sidenum[0] = side0;
		ld->sidenum[1] = side1;
		SetupLineSides(ld);
	}
	delete Strm;
	unguard;
}

//==========================================================================
//
//	VLevel::SetupLineSides
//
//==========================================================================

void VLevel::SetupLineSides(line_t* ld) const
{
	ld->alpha = 1.0;

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
	if (ld->sidenum[0] < 0 || ld->sidenum[0] >= NumSides)
	{
		Host_Error("Bad side-def index %d", ld->sidenum[0]);
	}
	ld->frontsector = Sides[ld->sidenum[0]].sector;

	if (ld->sidenum[1] != -1)
	{
		if (ld->sidenum[1] < 0 || ld->sidenum[1] >= NumSides)
		{
			Host_Error("Bad side-def index %d", ld->sidenum[1]);
		}
		ld->backsector = Sides[ld->sidenum[1]].sector;
		// Just a warning
		if (!(ld->flags & ML_TWOSIDED))
		{
			GCon->Logf("Bad WAD: Line %d is two-sided but has no TWO-SIDED "
				"flag set", ld - Lines);
		}
	}
	else
	{
		if (ld->flags & ML_TWOSIDED)
		{
			if (strict_level_errors)
			{
				Host_Error("Bad WAD: Line %d is marked as TWO-SIDED but has "
					"only one side", ld - Lines);
			}
			else
			{
				GCon->Logf("Bad WAD: Line %d is marked as TWO-SIDED but has "
					"only one side", ld - Lines);
				ld->flags &= ~ML_TWOSIDED;
			}
		}
		ld->backsector = 0;
	}
}

//==========================================================================
//
//  VLevel::LoadGLSegs
//
//==========================================================================

void VLevel::LoadGLSegs(int Lump, int NumBaseVerts)
{
	guard(VLevel::LoadGLSegs);
	vertex_t* GLVertexes = Vertexes + NumBaseVerts;
	int NumGLVertexes = NumVertexes - NumBaseVerts;

	//	Determine format of the segs data.
	int Format;
	vuint32 GLVertFlag;
	if (LevelFlags & LF_GLNodesV5)
	{
		Format = 5;
		NumSegs = W_LumpLength(Lump) / 16;
		GLVertFlag = GL_VERTEX_V5;
	}
	else
	{
		char Header[4];
		W_ReadFromLump(Lump, Header, 0, 4);
		if (!VStr::NCmp(Header, GL_V3_MAGIC, 4))
		{
			Format = 3;
			NumSegs = (W_LumpLength(Lump) - 4) / 16;
			GLVertFlag = GL_VERTEX_V3;
		}
		else
		{
			Format = 1;
			NumSegs = W_LumpLength(Lump) / 10;
			GLVertFlag = GL_VERTEX;
		}
	}

	//	Allocate memory for segs data.
	Segs = new seg_t[NumSegs];
	memset(Segs, 0, sizeof(seg_t) * NumSegs);

	//	Read data.
	VStream* Strm = W_CreateLumpReaderNum(Lump);
	if (Format == 3)
	{
		Strm->Seek(4);
	}
	seg_t* li = Segs;
	for (int i = 0; i < NumSegs; i++, li++)
	{
		vuint32 v1num;
		vuint32 v2num;
		vint16 linedef;	// -1 for minisegs
		vint16 side;

		if (Format < 3)
		{
			vuint16 v1, v2;
			vint16 partner;	// -1 on one-sided walls
			*Strm << v1 << v2 << linedef << side << partner;
			v1num = v1;
			v2num = v2;
		}
		else
		{
			vuint32 v1, v2;
			vint16 flags;
			vint32 partner;	// -1 on one-sided walls
			*Strm << v1 << v2 << linedef << flags << partner;
			v1num = v1;
			v2num = v2;
			side = flags & GL_SEG_FLAG_SIDE;
		}

		if (v1num & GLVertFlag)
		{
			v1num ^= GLVertFlag;
			if (v1num >= (vuint32)NumGLVertexes)
				Host_Error("Bad GL vertex index %d", v1num);
			li->v1 = &GLVertexes[v1num];
		}
		else
		{
			if (v1num >= (vuint32)NumVertexes)
				Host_Error("Bad vertex index %d", v1num);
			li->v1 = &Vertexes[v1num];
		}
		if (v2num & GLVertFlag)
		{
			v2num ^= GLVertFlag;
			if (v2num >= (vuint32)NumGLVertexes)
				Host_Error("Bad GL vertex index %d", v2num);
			li->v2 = &GLVertexes[v2num];
		}
		else
		{
			if (v2num >= (vuint32)NumVertexes)
				Host_Error("Bad vertex index %d", v2num);
			li->v2 = &Vertexes[v2num];
		}

		if (linedef >= 0)
		{
			line_t* ldef = &Lines[linedef];
			li->linedef = ldef;
			li->sidedef = &Sides[ldef->sidenum[side]];
			li->frontsector = Sides[ldef->sidenum[side]].sector;

			if (ldef->flags & ML_TWOSIDED)
				li->backsector = Sides[ldef->sidenum[side ^ 1]].sector;

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
	delete Strm;
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
	//	Determine format of the subsectors data.
	int Format;
	if (LevelFlags & LF_GLNodesV5)
	{
		Format = 5;
		NumSubsectors = W_LumpLength(Lump) / 8;
	}
	else
	{
		char Header[4];
		W_ReadFromLump(Lump, Header, 0, 4);
		if (!VStr::NCmp(Header, GL_V3_MAGIC, 4))
		{
			Format = 3;
			NumSubsectors = (W_LumpLength(Lump) - 4) / 8;
		}
		else
		{
			Format = 1;
			NumSubsectors = W_LumpLength(Lump) / 4;
		}
	}

	//	Allocate memory for subsectors.
	Subsectors = new subsector_t[NumSubsectors];
	memset(Subsectors, 0, sizeof(subsector_t) * NumSubsectors);

	//	Read data.
	VStream* Strm = W_CreateLumpReaderNum(Lump);
	if (Format == 3)
	{
		Strm->Seek(4);
	}
	subsector_t* ss = Subsectors;
	for (int i = 0; i < NumSubsectors; i++, ss++)
	{
		if (Format < 3)
		{
			vuint16 numsegs, firstseg;
			*Strm << numsegs << firstseg;
			ss->numlines = numsegs;
			ss->firstline = firstseg;
		}
		else
		{
			vint32 numsegs, firstseg;
			*Strm << numsegs << firstseg;
			ss->numlines = numsegs;
			ss->firstline = firstseg;
		}

		if (ss->firstline < 0 || ss->firstline >= NumSegs)
			Host_Error("Bad seg index %d", ss->firstline);
		if (ss->numlines <= 0 || ss->firstline + ss->numlines > NumSegs)
			Host_Error("Bad segs range %d %d", ss->firstline, ss->numlines);

			// look up sector number for each subsector
		seg_t* seg = &Segs[ss->firstline];
		for (int j = 0; j < ss->numlines; j++)
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
	delete Strm;
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
	if (LevelFlags & LF_GLNodesV5)
	{
		NumNodes = W_LumpLength(Lump) / 32;
	}
	else
	{
		NumNodes = W_LumpLength(Lump) / 28;
	}
	Nodes = new node_t[NumNodes];
	memset(Nodes, 0, sizeof(node_t) * NumNodes);

	VStream* Strm = W_CreateLumpReaderNum(Lump);
	node_t* no = Nodes;
	for (int i = 0; i < NumNodes; i++, no++)
	{
		vint16 x, y, dx, dy;
		vint16 bbox[2][4];
		vuint32 children[2];
		*Strm << x << y << dx << dy
			<< bbox[0][0] << bbox[0][1] << bbox[0][2] << bbox[0][3]
			<< bbox[1][0] << bbox[1][1] << bbox[1][2] << bbox[1][3];
		if (LevelFlags & LF_GLNodesV5)
		{
			*Strm << children[0] << children[1];
		}
		else
		{
			vuint16 child0, child1;
			*Strm << child0 << child1;
			children[0] = child0;
			if (children[0] & NF_SUBSECTOR_OLD)
				children[0] ^= NF_SUBSECTOR_OLD | NF_SUBSECTOR;
			children[1] = child1;
			if (children[1] & NF_SUBSECTOR_OLD)
				children[1] ^= NF_SUBSECTOR_OLD | NF_SUBSECTOR;
		}

		no->SetPointDir(TVec(x, y, 0), TVec(dx, dy, 0));

		for (int j = 0; j < 2; j++)
		{
			no->children[j] = children[j];
			no->bbox[j][0] = bbox[j][BOXLEFT];
			no->bbox[j][1] = bbox[j][BOXBOTTOM];
			no->bbox[j][2] = -32768.0;
			no->bbox[j][3] = bbox[j][BOXRIGHT];
			no->bbox[j][4] = bbox[j][BOXTOP];
			no->bbox[j][5] = 32768.0;
		}
	}
	delete Strm;
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
	if (W_LumpName(Lump) != NAME_gl_pvs || W_LumpLength(Lump) == 0)
	{
		GCon->Logf(NAME_Dev, "Empty or missing PVS lump");
		VisData = NULL;
		NoVis = new vuint8[(NumSubsectors + 7) / 8];
		memset(NoVis, 0xff, (NumSubsectors + 7) / 8);
	}
	else
	{
		VisData = new byte[W_LumpLength(Lump)];
		VStream* Strm = W_CreateLumpReaderNum(Lump);
		Strm->Serialise(VisData, W_LumpLength(Lump));
		delete Strm;
	}
	unguard;
}

//==========================================================================
//
//  VLevel::LoadCompressedGLNodes
//
//==========================================================================

void VLevel::LoadCompressedGLNodes(int Lump)
{
	guard(VLevel::LoadCompressedGLNodes);
	VStream* BaseStrm = W_CreateLumpReaderNum(Lump);
	//	Skip header.
	BaseStrm->Seek(4);
	//	Create reader stream for the zipped data.
	vuint8* TmpData = new vuint8[BaseStrm->TotalSize() - 4];
	BaseStrm->Serialise(TmpData, BaseStrm->TotalSize() - 4);
	VStream* DataStrm = new VMemoryStream(TmpData, BaseStrm->TotalSize() - 4);
	delete[] TmpData;
	delete BaseStrm;
	VStream* Strm = new VZipStreamReader(DataStrm);

	//	Read extra vertex data
	guard(Vertexes);
	vuint32 OrgVerts;
	vuint32 NewVerts;
	*Strm << OrgVerts << NewVerts;

	if (OrgVerts + NewVerts != (vuint32)NumVertexes)
	{
		vertex_t* OldVerts = Vertexes;
		NumVertexes = OrgVerts + NewVerts;
		Vertexes = new vertex_t[NumVertexes];
		memcpy(Vertexes, OldVerts, OrgVerts * sizeof(vertex_t));
		//	Fix up vertex pointers in linedefs
		for (int i = 0; i < NumLines; i++)
		{
			line_t& L = Lines[i];
			int v1 = L.v1 - OldVerts;
			int v2 = L.v2 - OldVerts;
			L.v1 = &Vertexes[v1];
			L.v2 = &Vertexes[v2];
		}
		delete[] OldVerts;
	}

	vertex_t* DstVert = Vertexes + OrgVerts;
	for (vuint32 i = 0; i < NewVerts; i++, DstVert++)
	{
		vint32 x, y;
		*Strm << x << y;
		*DstVert = TVec(x / 65536.0, y / 65536.0, 0);
	}
	unguard;

	//	Load subsectors
	guard(Subsectors);
	NumSubsectors = Streamer<vuint32>(*Strm);
	Subsectors = new subsector_t[NumSubsectors];
	memset(Subsectors, 0, sizeof(subsector_t) * NumSubsectors);
	subsector_t* ss = Subsectors;
	int FirstSeg = 0;
	for (int i = 0; i < NumSubsectors; i++, ss++)
	{
		vuint32 NumSubSegs;
		*Strm << NumSubSegs;
		ss->numlines = NumSubSegs;
		ss->firstline = FirstSeg;
		FirstSeg += NumSubSegs;
	}
	unguard;

	//	Load segs
	guard(Segs);
	NumSegs = Streamer<vuint32>(*Strm);
	Segs = new seg_t[NumSegs];
	memset(Segs, 0, sizeof(seg_t) * NumSegs);
	seg_t* li = Segs;
	for (int i = 0; i < NumSegs; i++, li++)
	{
		vuint32 v1;
		vuint32 partner;
		vuint16 linedef;
		vuint8 side;

		*Strm << v1 << partner << linedef << side;

		if (v1 >= (vuint32)NumVertexes)
		{
			Host_Error("Bad vertex index %d", v1);
		}
		li->v1 = &Vertexes[v1];

		if (linedef != 0xffff)
		{
			if (linedef >= NumLines)
			{
				Host_Error("Bad linedef index %d", linedef);
			}
			if (side > 1)
			{
				Host_Error("Bad seg side %d", side);
			}
			line_t* ldef = &Lines[linedef];
			li->linedef = ldef;
			li->sidedef = &Sides[ldef->sidenum[side]];
			li->frontsector = Sides[ldef->sidenum[side]].sector;

			if (ldef->flags & ML_TWOSIDED)
				li->backsector = Sides[ldef->sidenum[side ^ 1]].sector;

			if (side)
			{
				check(li);
				check(li->v1);
				check(ldef->v2);
				li->offset = Length(*li->v1 - *ldef->v2);
			}
			else
			{
				check(li);
				check(li->v1);
				check(ldef->v1);
				li->offset = Length(*li->v1 - *ldef->v1);
			}
			li->side = side;
		}
	}
	unguard;

	//	Load nodes.
	guard(Nodes);
	NumNodes = Streamer<vuint32>(*Strm);
	Nodes = new node_t[NumNodes];
	memset(Nodes, 0, sizeof(node_t) * NumNodes);
	node_t* no = Nodes;
	for (int i = 0; i < NumNodes; i++, no++)
	{
		vint16 x, y, dx, dy;
		vint16 bbox[2][4];
		vuint32 children[2];
		*Strm << x << y << dx << dy
			<< bbox[0][0] << bbox[0][1] << bbox[0][2] << bbox[0][3]
			<< bbox[1][0] << bbox[1][1] << bbox[1][2] << bbox[1][3]
			<< children[0] << children[1];

		no->SetPointDir(TVec(x, y, 0), TVec(dx, dy, 0));

		for (int j = 0; j < 2; j++)
		{
			no->children[j] = children[j];
			no->bbox[j][0] = bbox[j][BOXLEFT];
			no->bbox[j][1] = bbox[j][BOXBOTTOM];
			no->bbox[j][2] = -32768.0;
			no->bbox[j][3] = bbox[j][BOXRIGHT];
			no->bbox[j][4] = bbox[j][BOXTOP];
			no->bbox[j][5] = 32768.0;
		}
	}
	unguard;

	//	Set v2 of the segs.
	guard(Set up seg v2);
	subsector_t* Sub = Subsectors;
	for (int i = 0; i < NumSubsectors; i++, Sub++)
	{
		seg_t* Seg = Segs + Sub->firstline;
		for (int j = 0; j < Sub->numlines - 1; j++, Seg++)
		{
			Seg->v2 = Seg[1].v1;
		}
		Seg->v2 = Segs[Sub->firstline].v1;
	}
	unguard;

	guard(Calc segs);
	seg_t* li = Segs;
	for (int i = 0; i < NumSegs; i++, li++)
	{
		//	Calc seg's plane params
		li->length = Length(*li->v2 - *li->v1);
		CalcSeg(li);
	}
	unguard;

	guard(Calc subsectors);
	subsector_t* ss = Subsectors;
	for (int i = 0; i < NumSubsectors; i++, ss++)
	{
		// look up sector number for each subsector
		seg_t* seg = &Segs[ss->firstline];
		for (int j = 0; j < ss->numlines; j++)
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
	unguard;

	//	Create dummy VIS data.
	VisData = NULL;
	NoVis = new vuint8[(NumSubsectors + 7) / 8];
	memset(NoVis, 0xff, (NumSubsectors + 7) / 8);

	delete Strm;
	delete DataStrm;
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
	VStream* Strm = W_CreateLumpReaderNum(Lump);

	if (Strm->TotalSize() == 0 || Strm->TotalSize() / 2 >= 0x10000 ||
		build_blockmap)
	{
		GCon->Logf("Creating BLOCKMAP");
		CreateBlockMap();
	}
	else
	{
		// killough 3/1/98: Expand wad blockmap into larger internal one,
		// by treating all offsets except -1 as unsigned and zero-extending
		// them. This potentially doubles the size of blockmaps allowed,
		// because Doom originally considered the offsets as always signed.

		//	Allocate memory for blockmap.
		int Count = Strm->TotalSize() / 2;
		BlockMapLump = new vint32[Count];

		//	Read data.
		BlockMapLump[0] = Streamer<vint16>(*Strm);
		BlockMapLump[1] = Streamer<vint16>(*Strm);
		BlockMapLump[2] = Streamer<vuint16>(*Strm);
		BlockMapLump[3] = Streamer<vuint16>(*Strm);
		for (int i = 4; i < Count; i++)
		{
			vint16 Tmp;
			*Strm << Tmp;
			BlockMapLump[i] = Tmp == -1 ? -1 : (vuint16)Tmp & 0xffff;
		}
	}

	delete Strm;

	//	Read blockmap origin and size.
	BlockMapOrgX = BlockMapLump[0];
	BlockMapOrgY = BlockMapLump[1];
	BlockMapWidth = BlockMapLump[2];
	BlockMapHeight = BlockMapLump[3];
	BlockMap = BlockMapLump + 4;

	//	Clear out mobj chains.
	int Count = BlockMapWidth * BlockMapHeight;
	BlockLinks = new VEntity*[Count];
	memset(BlockLinks, 0, sizeof(VEntity*) * Count);
	unguard;
}

//==========================================================================
//
//  VLevel::CreateBlockMap
//
//==========================================================================

void VLevel::CreateBlockMap()
{
	guard(VLevel::CreateBlockMap);
	//	Determine bounds of the map.
	float MinX = Vertexes[0].x;
	float MaxX = MinX;
	float MinY = Vertexes[0].y;
	float MaxY = MinY;
	for (int i = 0; i < NumVertexes; i++)
	{
		if (MinX > Vertexes[i].x)
		{
			MinX = Vertexes[i].x;
		}
		if (MaxX < Vertexes[i].x)
		{
			MaxX = Vertexes[i].x;
		}
		if (MinY > Vertexes[i].y)
		{
			MinY = Vertexes[i].y;
		}
		if (MaxY < Vertexes[i].y)
		{
			MaxY = Vertexes[i].y;
		}
	}

	//	They should be integers, but just in case round them.
	MinX = floor(MinX);
	MinY = floor(MinY);
	MaxX = ceil(MaxX);
	MaxY = ceil(MaxY);

	int Width = MapBlock(MaxX - MinX) + 1;
	int Height = MapBlock(MaxY - MinY) + 1;

	//	Add all lines to their corresponding blocks
	TArray<vuint16>* BlockLines = new TArray<vuint16>[Width * Height];
	for (int i = 0; i < NumLines; i++)
	{
		//	Determine starting and ending blocks.
		line_t& Line = Lines[i];
		int X1 = MapBlock(Line.v1->x - MinX);
		int Y1 = MapBlock(Line.v1->y - MinY);
		int X2 = MapBlock(Line.v2->x - MinX);
		int Y2 = MapBlock(Line.v2->y - MinY);

		if (X1 > X2)
		{
			int Tmp = X2;
			X2 = X1;
			X1 = Tmp;
		}
		if (Y1 > Y2)
		{
			int Tmp = Y2;
			Y2 = Y1;
			Y1 = Tmp;
		}

		if (X1 == X2 && Y1 == Y2)
		{
			//	Line is inside a single block.
			BlockLines[X1 + Y1 * Width].Append(i);
		}
		else if (Y1 == Y2)
		{
			//	Horisontal line of blocks.
			for (int x = X1; x <= X2; x++)
			{
				BlockLines[x + Y1 * Width].Append(i);
			}
		}
		else if (X1 == X2)
		{
			//	Vertical line of blocks.
			for (int y = Y1; y <= Y2; y++)
			{
				BlockLines[X1 + y * Width].Append(i);
			}
		}
		else
		{
			//	Diagonal line.
			for (int x = X1; x <= X2; x++)
			{
				for (int y = Y1; y <= Y2; y++)
				{
					//	Check if line crosses the block
					if (Line.slopetype == ST_POSITIVE)
					{
						int p1 = Line.PointOnSide(TVec(MinX + x * 128,
							MinY + (y + 1) * 128, 0));
						int p2 = Line.PointOnSide(TVec(MinX + (x + 1) * 128,
							MinY + y * 128, 0));
						if (p1 == p2)
							continue;
					}
					else
					{
						int p1 = Line.PointOnSide(TVec(MinX + x * 128,
							MinY + y * 128, 0));
						int p2 = Line.PointOnSide(TVec(MinX + (x + 1) * 128,
							MinY + (y + 1) * 128, 0));
						if (p1 == p2)
							continue;
					}
					BlockLines[x + y * Width].Append(i);
				}
			}
		}
	}

	//	Build blockmap lump.
	TArray<vint32> BMap;
	BMap.SetNum(4 + Width * Height);
	BMap[0] = (int)MinX;
	BMap[1] = (int)MinY;
	BMap[2] = Width;
	BMap[3] = Height;
	for (int i = 0; i < Width * Height; i++)
	{
		//	Write offset.
		BMap[i + 4] = BMap.Num();
		TArray<vuint16>& Block = BlockLines[i];
		//	Add dummy start marker.
		BMap.Append(0);
		//	Add lines in this block.
		for (int j = 0; j < Block.Num(); j++)
		{
			BMap.Append(Block[j]);
		}
		//	Add terminator marker.
		BMap.Append(-1);
	}

	//	Copy data
	BlockMapLump = new vint32[BMap.Num()];
	memcpy(BlockMapLump, BMap.Ptr(), BMap.Num() * sizeof(vint32));

	delete[] BlockLines;
	unguard;
}

//==========================================================================
//
//  VLevel::LoadReject
//
//==========================================================================

void VLevel::LoadReject(int Lump)
{
	guard(VLevel::LoadReject);
	VStream* Strm = W_CreateLumpReaderNum(Lump);
	//	Check for empty reject lump
	if (Strm->TotalSize())
	{
		//	Check if reject lump is required bytes long.
		int NeededSize = (NumSectors * NumSectors + 7) / 8;
		if (Strm->TotalSize() < NeededSize)
		{
			GCon->Logf("Reject data is %d bytes too short",
				NeededSize - Strm->TotalSize());
		}
		else
		{
			//	Read it.
			RejectMatrix = new vuint8[Strm->TotalSize()];
			Strm->Serialise(RejectMatrix, Strm->TotalSize());

			//	Check if it's an all-zeroes lump, in which case it's useless
			// and can be discarded.
			bool Blank = true;
			for (int i = 0; i < NeededSize; i++)
			{
				if (RejectMatrix[i])
				{
					Blank = false;
					break;
				}
			}
			if (Blank)
			{
				delete[] RejectMatrix;
				RejectMatrix = NULL;
			}
		}
	}
	delete Strm;
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
	NumThings = W_LumpLength(Lump) / 10;
	Things = new mthing_t[NumThings];
	memset(Things, 0, sizeof(mthing_t) * NumThings);

	VStream* Strm = W_CreateLumpReaderNum(Lump);
	mthing_t* th = Things;
	for (int i = 0; i < NumThings; i++, th++)
	{
		vint16 x, y, angle, type, options;
		*Strm << x << y << angle << type << options;

		th->x = x;
		th->y = y;
		th->angle = angle;
		th->type = type;
		th->options = options;
	}
	delete Strm;
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
	NumThings = W_LumpLength(Lump) / 20;
	Things = new mthing_t[NumThings];
	memset(Things, 0, sizeof(mthing_t) * NumThings);

	VStream* Strm = W_CreateLumpReaderNum(Lump);
	mthing_t* th = Things;
	for (int i = 0; i < NumThings; i++, th++)
	{
		vint16 tid, x, y, height, angle, type, options;
		vuint8 special, arg1, arg2, arg3, arg4, arg5;
		*Strm << tid << x << y << height << angle << type << options
			<< special << arg1 << arg2 << arg3 << arg4 << arg5;

		th->tid = tid;
		th->x = x;
		th->y = y;
		th->height = height;
		th->angle = angle;
		th->type = type;
		th->options = options;
		th->special = special;
		th->arg1 = arg1;
		th->arg2 = arg2;
		th->arg3 = arg3;
		th->arg4 = arg4;
		th->arg5 = arg5;
	}
	delete Strm;
	unguard;
}

//==========================================================================
//
//	VLevel::LoadACScripts
//
//==========================================================================

void VLevel::LoadACScripts(int Lump)
{
	guard(VLevel::LoadACScripts);
	Acs = new VAcsLevel(this);

	//	Load level's BEHAVIOR lump if it has one.
	if (Lump >= 0)
	{
		Acs->LoadObject(Lump);
	}

	//	Load ACS helper scripts if needed (for Strife).
	if (GGameInfo->AcsHelper != NAME_None)
	{
		Acs->LoadObject(W_GetNumForName(GGameInfo->AcsHelper,
			WADNS_ACSLibrary));
	}

	//	Load user-specified default ACS libraries.
	for (int ScLump = W_IterateNS(-1, WADNS_Global); ScLump >= 0;
		ScLump = W_IterateNS(ScLump, WADNS_Global))
	{
		if (W_LumpName(ScLump) != NAME_loadacs)
		{
			continue;
		}

		VScriptParser* sc = new VScriptParser(*W_LumpName(ScLump),
			W_CreateLumpReaderNum(ScLump));
		while (!sc->AtEnd())
		{
			sc->ExpectName8();
			int AcsLump = W_CheckNumForName(sc->Name8);
			if (AcsLump >= 0)
			{
				Acs->LoadObject(AcsLump);
			}
			else
			{
				GCon->Logf("No such autoloaded ACS library %s", *sc->String);
			}
		}
		delete sc;
	}
	unguard;
}

//==========================================================================
//
//  VLevel::TexNumForName
//
//  Retrieval, get a texture or flat number for a name.
//
//==========================================================================

int VLevel::TexNumForName(const char *name, int Type, bool CMap) const
{
	guard(VLevel::TexNumForName);
	VName Name(name, VName::AddLower8);
	int i = GTextureManager.CheckNumForName(Name, Type, true, true);
	if (i == -1)
	{
		if (CMap)
		{
			return 0;
		}
		GCon->Logf("FTNumForName: %s not found", *Name);
		return GTextureManager.DefaultTexture;
	}
	return i;
	unguard;
}

//==========================================================================
//
//	VLevel::LoadRogueConScript
//
//==========================================================================

void VLevel::LoadRogueConScript(VName LumpName, FRogueConSpeech*& SpeechList,
	int& NumSpeeches) const
{
	guard(VLevel::LoadRogueConScript);
	//	Clear variables.
	SpeechList = NULL;
	NumSpeeches = 0;

	//	Check for empty name.
	if (LumpName == NAME_None)
		return;

	//	Get lump num.
	int LumpNum = W_CheckNumForName(LumpName);
	if (LumpNum < 0)
		return;	//	Not here.

	//	Load them.
	NumSpeeches = W_LumpLength(LumpNum) / 1516;
	SpeechList = new FRogueConSpeech[NumSpeeches];

	VStream* Strm = W_CreateLumpReaderNum(LumpNum);
	for (int i = 0; i < NumSpeeches; i++)
	{
		char Tmp[324];

		FRogueConSpeech& S = SpeechList[i];
		*Strm << S.SpeakerID << S.DropItem << S.CheckItem1 << S.CheckItem2
			<< S.CheckItem3 << S.JumpToConv;
		Strm->Serialise(Tmp, 16);
		Tmp[16] = 0;
		S.Name = Tmp;
		Strm->Serialise(Tmp, 8);
		Tmp[8] = 0;
		S.Voice = VName(Tmp, VName::AddLower8);
		if (S.Voice != NAME_None)
			S.Voice = va("svox/%s", *S.Voice);
		Strm->Serialise(Tmp, 8);
		Tmp[8] = 0;
		S.BackPic = VName(Tmp, VName::AddLower8);
		Strm->Serialise(Tmp, 320);
		Tmp[320] = 0;
		S.Text = Tmp;
		for (int j = 0; j < 5; j++)
		{
			FRogueConChoice& C = S.Choices[j];
			*Strm << C.GiveItem << C.NeedItem1 << C.NeedItem2 << C.NeedItem3
				<< C.NeedAmount1 << C.NeedAmount2 << C.NeedAmount3;
			Strm->Serialise(Tmp, 32);
			Tmp[32] = 0;
			C.Text = Tmp;
			Strm->Serialise(Tmp, 80);
			Tmp[80] = 0;
			C.TextOK = Tmp;
			*Strm << C.Next << C.Objectives;
			Strm->Serialise(Tmp, 80);
			Tmp[80] = 0;
			C.TextNo = Tmp;
		}
	}
	delete Strm;
	unguard;
}

//==========================================================================
//
//  VLevel::ClearBox
//
//==========================================================================

inline void VLevel::ClearBox(float* box) const
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

void VLevel::GroupLines() const
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
	linebuffer = new line_t*[total];
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

		if (LevelFlags & LF_ForServer)
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

void VLevel::LinkNode(int BSPNum, node_t* pParent) const
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
//  VLevel::CreateRepBase
//
//==========================================================================

void VLevel::CreateRepBase()
{
	guard(VLevel::CreateRepBase);
	BaseSides = new rep_side_t[NumSides];
	for (int i = 0; i < NumSides; i++)
	{
		side_t& S = Sides[i];
		rep_side_t& B = BaseSides[i];
		B.textureoffset = S.textureoffset;
		B.rowoffset = S.rowoffset;
		B.toptexture = S.toptexture;
		B.bottomtexture = S.bottomtexture;
		B.midtexture = S.midtexture;
	}

	BaseSectors = new rep_sector_t[NumSectors];
	for (int i = 0; i < NumSectors; i++)
	{
		sector_t& S = Sectors[i];
		rep_sector_t& B = BaseSectors[i];
		B.floor_pic = S.floor.pic;
		B.floor_dist = S.floor.dist;
		B.floor_xoffs = S.floor.xoffs;
		B.floor_yoffs = S.floor.yoffs;
		B.ceil_pic = S.ceiling.pic;
		B.ceil_dist = S.ceiling.dist;
		B.ceil_xoffs = S.ceiling.xoffs;
		B.ceil_yoffs = S.ceiling.yoffs;
		B.lightlevel = S.params.lightlevel;
		B.Fade = S.params.Fade;
	}

	BasePolyObjs = new rep_polyobj_t[NumPolyObjs];
	for (int i = 0; i < NumPolyObjs; i++)
	{
		polyobj_t& P = PolyObjs[i];
		rep_polyobj_t& B = BasePolyObjs[i];
		B.startSpot = P.startSpot;
		B.angle = P.angle;
	}
	unguard;
}
