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

#ifndef GLVIS_H
#define GLVIS_H

// HEADER FILES ------------------------------------------------------------

#include <math.h>
#include "cmdlib.h"
#include "wadlib.h"

#include "vector.h"

// MACROS ------------------------------------------------------------------

#define	ON_EPSILON	0.1

//
// Indicate a leaf.
//
#define	NF_SUBSECTOR	0x8000

// TYPES -------------------------------------------------------------------

struct winding_t
{
	bool	original;			// don't free, it's part of the portal
	TVec	points[2];
};

enum vstatus_t { stat_none, stat_working, stat_done };
struct portal_t : TPlane	// normal pointing into neighbor
{
	int			leaf;		// neighbor
	winding_t	winding;
	vstatus_t	status;
	byte		*visbits;
	byte		*mightsee;
	int			nummightsee;
	int			numcansee;
};

#define	MAX_PORTALS_ON_LEAF		128
struct leaf_t
{
	int			numportals;
	portal_t	*portals[MAX_PORTALS_ON_LEAF];
};

struct pstack_t
{
	pstack_t	*next;
	leaf_t		*leaf;
	portal_t	*portal;	// portal exiting
	winding_t	*source, *pass;
	TPlane		portalplane;
	byte		*mightsee;		// bit string
};

struct threaddata_t
{
	byte		*leafvis;		// bit string
	portal_t	*base;
	pstack_t	pstack_head;
};

typedef TVec vertex_t;

typedef leaf_t subsector_t;

struct seg_t : public TPlane
{
	vertex_t	*v1;
	vertex_t	*v2;

	seg_t		*partner;
	int			leaf;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void LoadLevel(int lumpnum, int gl_lumpnum);
void FreeLevel(void);

void BuildPVS(void);

void PortalFlow(portal_t *p);
void BasePortalVis(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern TIWadFile		*mainwad;
extern TIWadFile		*glwad;
extern TOWadFile		outwad;

extern bool				silent_mode;
extern bool				show_progress;
extern bool				fastvis;
extern bool				verbose;

extern int				testlevel;

extern int				numvertexes;
extern vertex_t			*vertexes;

extern int				numsegs;
extern seg_t			*segs;

extern int				numsubsectors;
extern subsector_t		*subsectors;

extern int				numportals;
extern portal_t			*portals;

#endif

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
