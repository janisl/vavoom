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
//**    Build nodes using glBSP.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "fwaddefs.h"
extern "C" {
#define vertex_t		glbsp_vertex_t
#define sector_t		glbsp_sector_t
#define seg_t			glbsp_seg_t
#define node_t			glbsp_node_t
#include "../utils/glbsp/system.h"
#include "../utils/glbsp/wad.h"
#include "../utils/glbsp/util.h"
#include "../utils/glbsp/level.h"
#include "../utils/glbsp/blockmap.h"
#include "../utils/glbsp/node.h"
#include "../utils/glbsp/seg.h"
#undef vertex_t
#undef sector_t
#undef seg_t
#undef node_t
};

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

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

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	GLBSP_PrintMsg
//
//==========================================================================

static void GLBSP_PrintMsg(const char *str, ...)
{
	char		message_buf[1024];
	va_list		args;

	va_start(args, str);
	vsprintf(message_buf, str, args);
	va_end(args);

	GCon->Logf("GB: %s", message_buf);
}

//==========================================================================
//
//	GLBSP_FatalError
//
//	Terminates the program reporting an error.
//
//==========================================================================

static void GLBSP_FatalError(const char *str, ...)
{
	char		message_buf[1024];
	va_list		args;

	va_start(args, str);
	vsprintf(message_buf, str, args);
	va_end(args);

	Sys_Error("Builing nodes failed: %s\n", message_buf);
}

static void GLBSP_Ticker()
{
}

static boolean_g GLBSP_DisplayOpen(displaytype_e)
{
	return true;
}

static void GLBSP_DisplaySetTitle(const char *)
{
}

static void GLBSP_DisplaySetBarText(int, const char*)
{
}

static void GLBSP_DisplaySetBarLimit(int, int)
{
}

static void GLBSP_DisplaySetBar(int, int)
{
}

static void GLBSP_DisplayClose()
{
}

static const nodebuildfuncs_t build_funcs =
{
	GLBSP_FatalError,
	GLBSP_PrintMsg,
	GLBSP_Ticker,

	GLBSP_DisplayOpen,
	GLBSP_DisplaySetTitle,
	GLBSP_DisplaySetBar,
	GLBSP_DisplaySetBarLimit,
	GLBSP_DisplaySetBarText,
	GLBSP_DisplayClose
};

static glbsp_node_t *root_node;

//==========================================================================
//
//	CopyNode
//
//==========================================================================

static void CopyNode(int& NodeIndex, glbsp_node_t* SrcNode, node_t* Nodes)
{
	if (SrcNode->r.node)
	{
		CopyNode(NodeIndex, SrcNode->r.node, Nodes);
	}

	if (SrcNode->l.node)
	{
		CopyNode(NodeIndex, SrcNode->l.node, Nodes);
	}

	SrcNode->index = NodeIndex;

	node_t* Node = &Nodes[NodeIndex];
	Node->SetPointDir(TVec(SrcNode->x, SrcNode->y, 0),
		TVec(SrcNode->dx, SrcNode->dy, 0));

	Node->bbox[0][0] = SrcNode->r.bounds.minx;
	Node->bbox[0][1] = SrcNode->r.bounds.miny;
	Node->bbox[0][2] = -32768.0;
	Node->bbox[0][3] = SrcNode->r.bounds.maxx;
	Node->bbox[0][4] = SrcNode->r.bounds.maxy;
	Node->bbox[0][5] = 32768.0;

	Node->bbox[1][0] = SrcNode->l.bounds.minx;
	Node->bbox[1][1] = SrcNode->l.bounds.miny;
	Node->bbox[1][2] = -32768.0;
	Node->bbox[1][3] = SrcNode->l.bounds.maxx;
	Node->bbox[1][4] = SrcNode->l.bounds.maxy;
	Node->bbox[1][5] = 32768.0;

	if (SrcNode->r.node)
	{
		Node->children[0] = SrcNode->r.node->index;
	}
	else if (SrcNode->r.subsec)
	{
		Node->children[0] = SrcNode->r.subsec->index | NF_SUBSECTOR;
	}

	if (SrcNode->l.node)
	{
		Node->children[1] = SrcNode->l.node->index;
	}
	else if (SrcNode->l.subsec)
	{
		Node->children[1] = SrcNode->l.subsec->index | NF_SUBSECTOR;
	}

	NodeIndex++;
}

//==========================================================================
//
//	HandleLevel
//
//==========================================================================

static glbsp_ret_e HandleLevel()
{
  superblock_t *seg_list;
  glbsp_node_t *root_stale_node;
  subsec_t *root_sub;

  glbsp_ret_e ret;

  cur_comms->build_pos = 0;

  LoadLevel();

  InitBlockmap();

  // create initial segs
  seg_list = CreateSegs();

  root_stale_node = (num_stale_nodes == 0) ? NULL : 
      LookupStaleNode(num_stale_nodes - 1);

  // recursively create nodes
  ret = BuildNodes(seg_list, &root_node, &root_sub, 0, root_stale_node);
  FreeSuper(seg_list);

  if (ret == GLBSP_E_OK)
  {
    ClockwiseBspTree(root_node);

    SaveLevel(root_node);
  }

  return ret;
}

//==========================================================================
//
//	MyGlbspBuildNodes
//
//==========================================================================

static glbsp_ret_e MyGlbspBuildNodes(const nodebuildinfo_t *info,
    const nodebuildfuncs_t *funcs, volatile nodebuildcomms_t *comms)
{
  glbsp_ret_e ret = GLBSP_E_OK;

  cur_info  = info;
  cur_funcs = funcs;
  cur_comms = comms;

  cur_comms->total_big_warn = 0;
  cur_comms->total_small_warn = 0;

  InitDebug();
  InitEndian();
 
  // opens and reads directory from the input wad
  ret = ReadWadFile(cur_info->input_file);

  if (ret != GLBSP_E_OK)
  {
    TermDebug();
    return ret;
  }

  cur_comms->file_pos = 0;
  
  // loop over each level in the wad
  while (FindNextLevel())
  {
    ret = HandleLevel();

    if (ret != GLBSP_E_OK)
      break;

    cur_comms->file_pos += 10;
  }

  // writes all the lumps to the output wad
  if (ret == GLBSP_E_OK)
  {
    ret = WriteWadFile(cur_info->output_file);

    ReportFailedLevels();
  }

  // close wads and free memory
  CloseWads();

  TermDebug();

  cur_info  = NULL;
  cur_comms = NULL;
  cur_funcs = NULL;

  return ret;
}

//==========================================================================
//
//	VLevel::BuildNodes
//
//==========================================================================

void VLevel::BuildNodes(int Lump)
{
	guard(VLevel::BuildNodes);
	//	Write WAD file.
	VStr FName = fl_savedir + "/temp.wad";
	VStr GwaName = fl_savedir + "/temp.gwa";
	FILE* f = fopen(*FName, "wb");

	wadinfo_t Hdr;
	Hdr.identification[0] = 'P';
	Hdr.identification[1] = 'W';
	Hdr.identification[2] = 'A';
	Hdr.identification[3] = 'D';
	Hdr.numlumps = (LevelFlags & LF_Extended) ? ML_BEHAVIOR + 1 : ML_BEHAVIOR;
	fwrite(&Hdr, 1, 12, f);

	filelump_t Lumps[ML_BEHAVIOR + 1];
	memset(Lumps, 0, sizeof(Lumps));
	for (int i = 0; i < Hdr.numlumps; i++)
	{
		VName LName = W_LumpName(Lump + i);
		for (int j = 0; j < 8 && (*LName)[j]; j++)
		{
			Lumps[i].name[j] = VStr::ToUpper((*LName)[j]);
		}
		Lumps[i].filepos = LittleLong(ftell(f));
		VStream* Src = W_CreateLumpReaderNum(Lump + i);
		Lumps[i].size = LittleLong(Src->TotalSize());
		if (Src->TotalSize())
		{
			TArray<vuint8> Buf;
			Buf.SetNum(Src->TotalSize());
			Src->Serialise(&Buf[0], Src->TotalSize());
			fwrite(&Buf[0], 1, Src->TotalSize(), f);
		}
	}
	Hdr.infotableofs = LittleLong(ftell(f));
	fwrite(Lumps, 1, 16 * Hdr.numlumps, f);
	Hdr.numlumps = LittleLong(Hdr.numlumps);
	fseek(f, 0, SEEK_SET);
	fwrite(&Hdr, 1, 12, f);
	fclose(f);

	//	Call glBSP to build nodes.
	nodebuildinfo_t nb_info = default_buildinfo;
	nodebuildcomms_t nb_comms = default_buildcomms;
	nb_info.input_file = *FName;
	nb_info.output_file = *GwaName;
	nb_info.quiet = true;
	if (GLBSP_E_OK != GlbspCheckInfo(&nb_info, &nb_comms))
	{
		Sys_Error("???");
	}
	glbsp_ret_e ret = MyGlbspBuildNodes(&nb_info, &build_funcs, &nb_comms);
	if (ret != GLBSP_E_OK)
	{
		Host_Error("Node build failed");
	}

	//	Load the nodes built.
	int gl_lumpnum = W_OpenAuxiliary(*GwaName);
	int NumBaseVerts;
	TVec* OldVertexes = Vertexes;
	LoadVertexes(Lump + ML_VERTEXES, gl_lumpnum + ML_GL_VERT, NumBaseVerts);
	vertex_t* GLVertexes = Vertexes + NumBaseVerts;
	W_CloseAuxiliary();

	//	Update pointer to vertexes in lines.
	for (int i = 0; i < NumLines; i++)
	{
		line_t* ld = &Lines[i];
		ld->v1 = &Vertexes[ld->v1 - OldVertexes];
		ld->v2 = &Vertexes[ld->v2 - OldVertexes];
	}
	delete[] OldVertexes;

	guard(Segs);
	//	Build ordered list of source segs.
	glbsp_seg_t** SrcSegs = new glbsp_seg_t*[num_complete_seg];
	for (int i = 0; i < num_segs; i++)
	{
		glbsp_seg_t* Seg = LookupSeg(i);
		// ignore degenerate segs
		if (Seg->degenerate)
			continue;
		SrcSegs[Seg->index] = Seg;
	}

	//	Copy segs.
	NumSegs = num_complete_seg;
	Segs = new seg_t[NumSegs];
	memset(Segs, 0, sizeof(seg_t) * NumSegs);
	seg_t* li = Segs;
	for (int i = 0; i < NumSegs; i++, li++)
	{
		glbsp_seg_t* SrcSeg = SrcSegs[i];

		if (SrcSeg->start->index & IS_GL_VERTEX)
		{
			li->v1 = &GLVertexes[SrcSeg->start->index & ~IS_GL_VERTEX];
		}
		else
		{
			li->v1 = &Vertexes[SrcSeg->start->index];
		}
		if (SrcSeg->end->index & IS_GL_VERTEX)
		{
			li->v2 = &GLVertexes[SrcSeg->end->index & ~IS_GL_VERTEX];
		}
		else
		{
			li->v2 = &Vertexes[SrcSeg->end->index];
		}

		if (SrcSeg->linedef)
		{
			line_t* ldef = &Lines[SrcSeg->linedef->index];
			li->linedef = ldef;
			li->sidedef = &Sides[ldef->sidenum[SrcSeg->side]];
			li->frontsector = Sides[ldef->sidenum[SrcSeg->side]].sector;

			if (ldef->flags & ML_TWOSIDED)
				li->backsector = Sides[ldef->sidenum[SrcSeg->side ^ 1]].sector;

			if (SrcSeg->side)
				li->offset = Length(*li->v1 - *ldef->v2);
			else
				li->offset = Length(*li->v1 - *ldef->v1);
			li->length = Length(*li->v2 - *li->v1);
			li->side = SrcSeg->side;
		}

		//	Calc seg's plane params
		CalcSeg(li);
	}
	unguard;

	//	Copy subsectors
	guard(Subsectors);
	NumSubsectors = num_subsecs;
	Subsectors = new subsector_t[NumSubsectors];
	memset(Subsectors, 0, sizeof(subsector_t) * NumSubsectors);
	subsector_t* ss = Subsectors;
	for (int i = 0; i < NumSubsectors; i++, ss++)
	{
		subsec_t* SrcSub = LookupSubsec(i);
		ss->numlines = SrcSub->seg_count;
		ss->firstline = SrcSub->seg_list->index;

		//	Look up sector number for each subsector
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
		{
			Host_Error("Subsector %d without sector", i);
		}
	}
	unguard;

	//	Copy nodes.
	guard(Nodes);
	NumNodes = num_nodes;
	Nodes = new node_t[NumNodes];
	memset(Nodes, 0, sizeof(node_t) * NumNodes);
	if (root_node)
	{
		int NodeIndex = 0;
		CopyNode(NodeIndex, root_node, Nodes);
	}
	unguard;

	FreeLevel();
	FreeQuickAllocCuts();
	FreeQuickAllocSupers();

	//	Create dummy VIS data.
	VisData = NULL;
	NoVis = new vuint8[(NumSubsectors + 7) / 8];
	memset(NoVis, 0xff, (NumSubsectors + 7) / 8);
	unguard;
}
