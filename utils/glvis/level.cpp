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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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

#include "glvis.h"
#include "fmapdefs.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int			numvertexes;
vertex_t	*vertexes;

int			numsegs;
seg_t		*segs;

int			numsubsectors;
subsector_t	*subsectors;

int			numportals;
portal_t	*portals;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static vertex_t*		gl_vertexes;

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	LoadVertexes
//
//==========================================================================

static void LoadVertexes(int lump, int gl_lump)
{
	int				i;
	void*			data;
	void*			gldata;
	mapvertex_t*	ml;
	vertex_t*		li;
	int				base_verts;
	int				gl_verts;

	base_verts = mainwad->LumpSize(lump) / sizeof(mapvertex_t);

	gldata = glwad->GetLump(gl_lump);
	outwad.AddLump(glwad->LumpName(gl_lump), gldata, glwad->LumpSize(gl_lump));
	if (!strncmp((char*)gldata, GL_V2_VERTEX_MAGIC, 4))
	{
		gl_verts = (glwad->LumpSize(gl_lump) - 4) / sizeof(gl_mapvertex_t);
	}
	else
	{
		gl_verts = glwad->LumpSize(gl_lump) / sizeof(mapvertex_t);
	}
	numvertexes = base_verts + gl_verts;

	// Allocate zone memory for buffer.
	li = vertexes = new vertex_t[numvertexes];

	// Load data into cache.
	data = mainwad->GetLump(lump);
	ml = (mapvertex_t *)data;

	// Copy and convert vertex, internal representation as vector.
	for (i = 0; i < base_verts; i++, li++, ml++)
	{
		*li = TVec(LittleShort(ml->x), LittleShort(ml->y));
	}

	// Free buffer memory.
	Free(data);

	//	Save pointer to GL vertexes for seg loading
	gl_vertexes = li;

	if (!strncmp((char*)gldata, GL_V2_VERTEX_MAGIC, 4))
	{
		gl_mapvertex_t*		glml;

		glml = (gl_mapvertex_t *)((byte*)gldata + 4);

		// Copy and convert vertex, internal representation as vector.
		for (i = 0; i < gl_verts; i++, li++, glml++)
		{
			*li = TVec((float)LittleLong(glml->x) / (float)0x10000,
					   (float)LittleLong(glml->y) / (float)0x10000);
		}
	}
	else
	{
		ml = (mapvertex_t *)gldata;

		// Copy and convert vertex, internal representation as vector.
		for (i = 0; i < gl_verts; i++, li++, ml++)
		{
			*li = TVec(LittleShort(ml->x), LittleShort(ml->y));
		}
	}

   	// Free buffer memory.
	Free(gldata);
}

//==========================================================================
//
//	LoadSegs
//
//==========================================================================

static void LoadSegs(int lump)
{
	void*		data;
	int			i;
	mapglseg_t*	ml;
	seg_t*		li;

	numsegs = glwad->LumpSize(lump) / sizeof(mapglseg_t);
	segs = new seg_t[numsegs];
	data = glwad->GetLump(lump);
	outwad.AddLump(glwad->LumpName(lump), data, glwad->LumpSize(lump));

	ml = (mapglseg_t *)data;
	li = segs;
	numportals = 0;

	for (i = 0; i < numsegs; i++, li++, ml++)
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
			li->v1 = &vertexes[v1num];
		}
		if (v2num & GL_VERTEX)
		{
			v2num ^= GL_VERTEX;
			li->v2 = &gl_vertexes[v2num];
		}
		else
		{
			li->v2 = &vertexes[v2num];
		}

		int partner = LittleShort(ml->partner);
		if (partner >= 0)
		{
			li->partner = &segs[partner];
			numportals++;
		}

		//	Calc seg's plane params
		li->Set2Points(*li->v1, *li->v2);
	}

	Free(data);

	portals = new portal_t[numportals];
}

//==========================================================================
//
//	LoadSubsectors
//
//==========================================================================

static void LoadSubsectors(int lump)
{
	void*				data;
	int					i;
	mapsubsector_t*		ms;
	subsector_t*		ss;

	numsubsectors = glwad->LumpSize(lump) / sizeof(mapsubsector_t);
	subsectors = new subsector_t[numsubsectors];
	data = glwad->GetLump(lump);
	outwad.AddLump(glwad->LumpName(lump), data, glwad->LumpSize(lump));

	ms = (mapsubsector_t *)data;
	ss = subsectors;

	for (i = 0; i < numsubsectors; i++, ss++, ms++)
	{
		//	Set seg subsector links
		int count = LittleShort(ms->numsegs);
		seg_t *line = &segs[LittleShort(ms->firstseg)];
		while (count--)
		{
			line->leaf = i;
			line++;
		}
	}

	Free(data);
}

//==========================================================================
//
//	LoadNodes
//
//==========================================================================

static void LoadNodes(int lump)
{
	void*	data;

	data = glwad->GetLump(lump);
	outwad.AddLump(glwad->LumpName(lump), data, glwad->LumpSize(lump));
	Free(data);
}

//==========================================================================
//
//	CreatePortals
//
//==========================================================================

static void CreatePortals(void)
{
	int i;

	portal_t *p = portals;
	for (i = 0; i < numsegs; i++)
	{
		seg_t *line = &segs[i];
		subsector_t *sub = &subsectors[line->leaf];
		if (line->partner)
		{
			// create portal
			if (sub->numportals == MAX_PORTALS_ON_LEAF)
				Error("Leaf with too many portals");
			sub->portals[sub->numportals] = p;
			sub->numportals++;

			p->winding.original = true;
			p->winding.points[0] = *line->v1;
			p->winding.points[1] = *line->v2;
			p->normal = line->partner->normal;
			p->dist = line->partner->dist;
			p->leaf = line->partner->leaf;
			p++;
		}
	}
	if (p - portals != numportals)
		Error("Portals miscounted");
}
	
//==========================================================================
//
//	LoadLevel
//
//==========================================================================

void LoadLevel(int lumpnum, int gl_lumpnum)
{
	LoadVertexes(lumpnum + ML_VERTEXES, gl_lumpnum + ML_GL_VERT);
	LoadSegs(gl_lumpnum + ML_GL_SEGS);
	LoadSubsectors(gl_lumpnum + ML_GL_SSECT);
	LoadNodes(gl_lumpnum + ML_GL_NODES);

	CreatePortals();

	cerr << "\nLoaded " << mainwad->LumpName(lumpnum) << ", "
		<< numvertexes << " vertexes, "
		<< numsegs << " segs, "
		<< numsubsectors << " subsectors, "
		<< numportals << " portals\n";
}

//==========================================================================
//
//	FreeLevel
//
//==========================================================================

void FreeLevel(void)
{
	delete vertexes;
	delete segs;
	delete subsectors;
	delete portals;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/08/24 17:08:34  dj_jl
//	Beautification
//
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
