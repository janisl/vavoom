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

#include "glvisint.h"

namespace VavoomUtils {

#include "fmapdefs.h"

// MACROS ------------------------------------------------------------------

#define TEMP_FILE	"$glvis$$.$$$"

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	GLVisMalloc
//
//==========================================================================

static void *GLVisMalloc(size_t size)
{
	void *ptr = malloc(size);
	if (!ptr)
	{
		throw GLVisError("Couldn't alloc %ld bytes", size);
	}
	memset(ptr, 0, size);
	return ptr;
}

//==========================================================================
//
//	GLVisFree
//
//==========================================================================

static void GLVisFree(void *ptr)
{
	free(ptr);
}

//==========================================================================
//
//	GLVisError::GLVisError
//
//==========================================================================

GLVisError::GLVisError(const char *error, ...)
{
	va_list		argptr;

	va_start(argptr, error);
	vsprintf(message, error, argptr);
	va_end(argptr);
}

//==========================================================================
//
//	TVisBuilder::TVisBuilder
//
//==========================================================================

TVisBuilder::TVisBuilder(TGLVis &AOwner) : Owner(AOwner),
	numvertexes(0), vertexes(NULL), gl_vertexes(NULL),
	numsegs(0), segs(NULL),
	numsubsectors(0), subsectors(NULL),
	numportals(0), portals(NULL),
	vissize(0), vis(NULL),
	bitbytes(0), bitlongs(0),
	portalsee(0), c_leafsee(0), c_portalsee(0),
	c_chains(0), c_portalskip(0), c_leafskip(0),
	c_vistest(0), c_mighttest(0),
	c_portaltest(0), c_portalpass(0), c_portalcheck(0),
	totalvis(0), rowbytes(0)
{
}

//==========================================================================
//
//	TVisBuilder::LoadVertexes
//
//==========================================================================

void TVisBuilder::LoadVertexes(int lump, int gl_lump)
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
	outwad.AddLump(glwad->LumpName(gl_lump), gldata,
		glwad->LumpSize(gl_lump));
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
	li = vertexes = New<vertex_t>(numvertexes);

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
//	TVisBuilder::LoadSegs
//
//==========================================================================

void TVisBuilder::LoadSegs(int lump)
{
	void*		data;
	int			i;
	mapglseg_t*	ml;
	seg_t*		li;

	numsegs = glwad->LumpSize(lump) / sizeof(mapglseg_t);
	segs = New<seg_t>(numsegs);
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

	portals = New<portal_t>(numportals);
}

//==========================================================================
//
//	TVisBuilder::LoadSubsectors
//
//==========================================================================

void TVisBuilder::LoadSubsectors(int lump)
{
	void*				data;
	int					i;
	mapsubsector_t*		ms;
	subsector_t*		ss;

	numsubsectors = glwad->LumpSize(lump) / sizeof(mapsubsector_t);
	subsectors = New<subsector_t>(numsubsectors);
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
//	TVisBuilder::LoadNodes
//
//==========================================================================

void TVisBuilder::LoadNodes(int lump)
{
	void*	data;

	data = glwad->GetLump(lump);
	outwad.AddLump(glwad->LumpName(lump), data, glwad->LumpSize(lump));
	Free(data);
}

//==========================================================================
//
//	TVisBuilder::CreatePortals
//
//==========================================================================

void TVisBuilder::CreatePortals(void)
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
				throw GLVisError("Leaf with too many portals");
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
		throw GLVisError("Portals miscounted");
}
	
//==========================================================================
//
//	TVisBuilder::LoadLevel
//
//==========================================================================

void TVisBuilder::LoadLevel(int lumpnum, int gl_lumpnum)
{
	const char *levelname = mainwad->LumpName(lumpnum);

	LoadVertexes(lumpnum + ML_VERTEXES, gl_lumpnum + ML_GL_VERT);
	LoadSegs(gl_lumpnum + ML_GL_SEGS);
	LoadSubsectors(gl_lumpnum + ML_GL_SSECT);
	LoadNodes(gl_lumpnum + ML_GL_NODES);

	CreatePortals();

	Owner.DisplayMessage(
		"\nLoaded %s, %d vertexes, %d segs, %d subsectors, %d portals\n",
		levelname, numvertexes, numsegs, numsubsectors, numportals);
	Owner.DisplayStartMap(levelname);
}

//==========================================================================
//
//	TVisBuilder::FreeLevel
//
//==========================================================================

void TVisBuilder::FreeLevel(void)
{
	Delete(vertexes);
	Delete(segs);
	Delete(subsectors);
	Delete(portals);
	Delete(vis);
}

//==========================================================================
//
//	TVisBuilder::IsLevelName
//
//==========================================================================

bool TVisBuilder::IsLevelName(int lump)
{
	if (lump + 4 >= glwad->numlumps)
	{
		return false;
	}

	const char	*name = glwad->LumpName(lump);

	if (name[0] != 'G' || name[1] != 'L' || name[2] != '_')
	{
		return false;
	}

	if (Owner.num_specified_maps)
	{
		for (int i = 0; i < Owner.num_specified_maps; i++)
		{
			if (!stricmp(Owner.specified_maps[i], name + 3))
			{
				return true;
			}
		}
	}
	else
	{
		if (!strcmp(glwad->LumpName(lump + 1), "GL_VERT") &&
			!strcmp(glwad->LumpName(lump + 2), "GL_SEGS") &&
			!strcmp(glwad->LumpName(lump + 3), "GL_SSECT") &&
			!strcmp(glwad->LumpName(lump + 4), "GL_NODES"))
		{
			return true;
		}
	}
	return false;
}

//==========================================================================
//
//	TVisBuilder::Run
//
//==========================================================================

void TVisBuilder::Run(const char *srcfile)
{
	char filename[1024];
	char destfile[1024];
	char bakext[8];
	int i;

	if (Owner.Malloc && Owner.Free)
	{
		Malloc = Owner.Malloc;
		Free = Owner.Free;
	}
	else
	{
		Malloc = GLVisMalloc;
		Free = GLVisFree;
	}

	strcpy(filename, srcfile);
	DefaultExtension(filename, ".wad");
	strcpy(destfile, filename);
	inwad.Open(filename);
	mainwad = &inwad;

	StripExtension(filename);
	strcat(filename, ".gwa");
	FILE *ff = fopen(filename, "rb");
	if (ff)
	{
		fclose(ff);
		gwa.Open(filename);
		glwad = &gwa;
		strcpy(destfile, filename);
		strcpy(bakext, ".~gw");
	}
	else
	{
		glwad = &inwad;
		strcpy(bakext, ".~wa");
	}

	outwad.Open(TEMP_FILE, glwad->wadid);

	//	Process lumps
	i = 0;
	while (i < glwad->numlumps)
	{
		void *ptr =	glwad->GetLump(i);
		const char *name = glwad->LumpName(i);
		outwad.AddLump(name, ptr, glwad->LumpSize(i));
		Free(ptr);
		if (IsLevelName(i))
		{
			LoadLevel(mainwad->LumpNumForName(name + 3), i);
			i += 5;
			if (!strcmp("GL_PVS", glwad->LumpName(i)))
			{
				i++;
			}
			BuildPVS();

			//	Write lump
			outwad.AddLump("GL_PVS", vis, vissize);

			FreeLevel();
		}
		else
		{
			i++;
		}
	}

	inwad.Close();
	if (gwa.handle)
	{
		gwa.Close();
	}
	outwad.Close();

	strcpy(filename, destfile);
	StripExtension(filename);
	strcat(filename, bakext);
	remove(filename);
	rename(destfile, filename);
	rename(TEMP_FILE, destfile);
}

//==========================================================================
//
//	TGLVis::Build
//
//==========================================================================

void TGLVis::Build(const char *srcfile)
{
	try
	{
		TVisBuilder VisBuilder(*this);

		VisBuilder.Run(srcfile);
	}
	catch (WadLibError &e)
	{
		throw GLVisError("%s", e.message);
	}
}

} // namespace VavoomUtils

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2001/09/12 17:28:38  dj_jl
//	Created glVIS plugin
//
//	Revision 1.3  2001/08/24 17:08:34  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
