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
#include "../utils/glbsp/glbsp.h"

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

//==========================================================================
//
//	GLBSP_BuildNodes
//
//==========================================================================

static bool _GLBSP_BuildNodes(const char *name, const char* gwafile)
{
	nodebuildinfo_t nb_info;
	nodebuildcomms_t nb_comms;
	glbsp_ret_e ret;

	nb_info = default_buildinfo;
	nb_comms = default_buildcomms;

	nb_info.input_file = name;
	nb_info.output_file = gwafile;

	// FIXME: check parm "-node-factor"

	if (GLBSP_E_OK != GlbspCheckInfo(&nb_info, &nb_comms))
		return false;

	ret = GlbspBuildNodes(&nb_info, &build_funcs, &nb_comms);

	if (ret != GLBSP_E_OK)
		return false;

	return true;
}

//==========================================================================
//
//	VLevel::BuildNodes
//
//==========================================================================

void VLevel::BuildNodes(int Lump)
{
	guard(VLevel::BuildNodes);
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

	_GLBSP_BuildNodes(*FName, *GwaName);

	int gl_lumpnum = W_OpenAuxiliary(*GwaName);
	int NumBaseVerts;
	TVec* OldVertexes = Vertexes;
	LoadVertexes(Lump + ML_VERTEXES, gl_lumpnum + ML_GL_VERT, NumBaseVerts);

	for (int i = 0; i < NumLines; i++)
	{
		line_t* ld = &Lines[i];
		ld->v1 = &Vertexes[ld->v1 - OldVertexes];
		ld->v2 = &Vertexes[ld->v2 - OldVertexes];
	}
	delete[] OldVertexes;

	LoadGLSegs(gl_lumpnum + ML_GL_SEGS, NumBaseVerts);
	LoadSubsectors(gl_lumpnum + ML_GL_SSECT);
	LoadNodes(gl_lumpnum + ML_GL_NODES);
	LoadPVS(gl_lumpnum + ML_GL_PVS);
	W_CloseAuxiliary();
	unguard;
}
