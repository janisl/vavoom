//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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
//**
//**	Creates PVS(Potentially Visible Set)
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "glvis.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int			bitbytes;				// (portalleafs+63)>>3
static int			bitlongs;

static byte			*portalsee;
static int			c_leafsee, c_portalsee;

static int			c_chains;
static int			c_portalskip, c_leafskip;
static int			c_vistest, c_mighttest;
static int			c_portaltest, c_portalpass, c_portalcheck;

static int			totalvis;
static int			rowbytes;
static byte			*vis;

static const char	progress_chars[] = {'|', '/', '-', '\\'};

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	SimpleFlood
//
//==========================================================================

static void SimpleFlood(portal_t *srcportal, int leafnum)
{
	int			i;
	leaf_t		*leaf;
	portal_t	*p;

	if (srcportal->mightsee[leafnum >> 3] & (1 << (leafnum & 7)))
		return;
	srcportal->mightsee[leafnum >> 3] |= (1 << (leafnum & 7));
	c_leafsee++;
	
	leaf = &subsectors[leafnum];
	
	for (i = 0; i < leaf->numportals; i++)
	{
		p = leaf->portals[i];
		if (!portalsee[p - portals])
			continue;
		SimpleFlood(srcportal, p->leaf);
	}
}

//==========================================================================
//
//	BasePortalVis
//
//	This is a rough first-order aproximation that is used to trivially
// reject some of the final calculations.
//
//==========================================================================

void BasePortalVis(void)
{
	int			i, j, k;
	portal_t	*tp, *p;
	float		d;
	winding_t	*w;

	portalsee = new byte[numportals];
	for (i = 0, p = portals; i < numportals; i++, p++)
	{
		if (show_progress && !(i & 0x1f))
		{
			fprintf(stderr, "%c\b", progress_chars[(i >> 5) & 3]);
		}

		p->mightsee = new byte[bitbytes];
		
		c_portalsee = 0;
		memset(portalsee, 0, numportals);

		for (j = 0, tp = portals; j < numportals; j++, tp++)
		{
			if (j == i)
				continue;
			w = &tp->winding;
			for (k = 0; k < 2; k++)
			{
				d = DotProduct(w->points[k], p->normal) - p->dist;
				if (d > ON_EPSILON)
					break;
			}
			if (k == 2)
				continue;	// no points on front

			
			w = &p->winding;
			for (k = 0; k < 2; k++)
			{
				d = DotProduct(w->points[k], tp->normal) - tp->dist;
				if (d < -ON_EPSILON)
					break;
			}
			if (k == 2)
				continue;	// no points on front

			portalsee[j] = 1;		
			c_portalsee++;
		}
		
		c_leafsee = 0;
		SimpleFlood(p, p->leaf);
		p->nummightsee = c_leafsee;
//		printf ("portal:%4i  c_leafsee:%4i \n", i, c_leafsee);
	}
	delete[] portalsee;
}

//==========================================================================
//
//	CheckStack
//
//==========================================================================

static void CheckStack(leaf_t *leaf, threaddata_t *thread)
{
	pstack_t	*p;

	for (p = thread->pstack_head.next; p; p = p->next)
		if (p->leaf == leaf)
			Error("CheckStack: leaf recursion");
}

//==========================================================================
//
//	FreeWinding
//
//==========================================================================

static void FreeWinding(winding_t *w)
{
	if (!w->original)
		delete w;
}

//==========================================================================
//
//	CopyWinding
//
//==========================================================================

static winding_t *CopyWinding(winding_t *w)
{
	winding_t	*c;
	
	c = new winding_t;
	c->points[0] = w->points[0];
	c->points[1] = w->points[1];
	c->original = false;
	return c;
}

//==========================================================================
//
//	ClipWinding
//
//	Clips the winding to the plane, returning the new winding on the
// positive side. Frees the input winding.
//
//==========================================================================

static winding_t *ClipWinding(winding_t *in, TPlane *split)
{
	float		dists[2];
	float		dot;
	TVec		mid;
	winding_t	*neww;
	
	// determine sides for each point
	dists[0] = DotProduct(in->points[0], split->normal) - split->dist;
	dists[1] = DotProduct(in->points[1], split->normal) - split->dist;
	
	if (dists[0] < ON_EPSILON && dists[1] < ON_EPSILON)
	{
		FreeWinding(in);
		return NULL;
	}
	if (dists[0] >= -ON_EPSILON && dists[1] >= -ON_EPSILON)
	{
		return in;
	}
	
	neww = new winding_t;

	// generate a split point
	TVec &p1 = in->points[0];
	TVec &p2 = in->points[1];
	dot = dists[0] / (dists[0] - dists[1]);

	// avoid round off error when possible
	if (split->normal.x == 1)
		mid.x = split->dist;
	else if (split->normal.x == -1)
		mid.x = -split->dist;
	else
		mid.x = p1.x + dot * (p2.x - p1.x);

	if (split->normal.y == 1)
		mid.y = split->dist;
	else if (split->normal.y == -1)
		mid.y = -split->dist;
	else
		mid.y = p1.y + dot * (p2.y - p1.y);

	if (dists[0] < -ON_EPSILON)
	{
		neww->points[0] = mid;
		neww->points[1] = in->points[1];
	}
	else
	{
		neww->points[0] = in->points[0];
		neww->points[1] = mid;
	}
	neww->original = false;

	// free the original winding
	FreeWinding(in);

	return neww;
}

//==========================================================================
//
//	ClipToSeperators
//
//	Source, pass, and target are an ordering of portals.
//	Generates seperating planes canidates by taking two points from source
// and one point from pass, and clips target by them.
//	If target is totally clipped away, that portal can not be seen through.
//
//==========================================================================

static winding_t *ClipToSeperators(winding_t *source, winding_t *pass, winding_t *target)
{
	int			i, j;
	TPlane		plane;
	TVec		v2;
	float		d;
	float		length;

	// check all combinations
	for (i = 0; i < 2; i++)
	{
		// fing a vertex of pass that makes a plane that puts all of the
		// vertexes of pass on the front side and all of the vertexes of
		// source on the back side
		for (j = 0; j < 2; j++)
		{
			v2 = pass->points[j] - source->points[i];

			plane.normal.x = -v2.y;
			plane.normal.y = v2.x;
			
			// if points don't make a valid plane, skip it

			length = plane.normal.x * plane.normal.x
					+ plane.normal.y * plane.normal.y;
			
			if (length < ON_EPSILON)
				continue;

			length = 1 / sqrt(length);
			
			plane.normal.x *= length;
			plane.normal.y *= length;

			plane.dist = DotProduct(pass->points[j], plane.normal);

			//
			// find out which side of the generated seperating plane has the
			// source portal
			//
			d = DotProduct(source->points[i ^ 1], plane.normal) - plane.dist;
			if (d > ON_EPSILON)
			{
				//  source is on the positive side, so we want all
				// pass and target on the negative side
				//  flip the normal if the source portal is backwards
				plane.normal = -plane.normal;
				plane.dist = -plane.dist;
			}
			else if (d >= -ON_EPSILON)
			{
				// planar with source portal
				continue;
			}

			//
			// if all of the pass portal points are now on the positive side,
			// this is the seperating plane
			//
			d = DotProduct(pass->points[j ^ 1], plane.normal) - plane.dist;
			if (d <= ON_EPSILON)
			{
				// planar with seperating plane or
				// points on negative side, not a seperating plane
				continue;
			}
		
			//
			// clip target by the seperating plane
			//
			target = ClipWinding(target, &plane);
			if (!target)
			{
				return NULL;		// target is not visible
			}
		}
	}
	
	return target;
}

//==========================================================================
//
//	RecursiveLeafFlow
//
//	Flood fill through the leafs
//	If src_portal is NULL, this is the originating leaf
//
//==========================================================================

static void RecursiveLeafFlow(int leafnum, threaddata_t *thread, pstack_t *prevstack)
{
	pstack_t	stack;
	portal_t	*p;
	TPlane		backplane;
	winding_t	*source, *target;
	leaf_t 		*leaf;
	int			i, j;
	long		*test, *might, *vis;
	bool		more;

	c_chains++;

	leaf = &subsectors[leafnum];
	CheckStack(leaf, thread);
	
	// mark the leaf as visible
	if (!(thread->leafvis[leafnum >> 3] & (1 << (leafnum & 7))))
	{
		thread->leafvis[leafnum >> 3] |= 1 << (leafnum & 7);
		thread->base->numcansee++;
	}
	
	prevstack->next = &stack;
	stack.next = NULL;
	stack.leaf = leaf;
	stack.portal = NULL;
	stack.mightsee = new byte[bitbytes];
	might = (long *)stack.mightsee;
	vis = (long *)thread->leafvis;
	
	// check all portals for flowing into other leafs
	for (i = 0; i < leaf->numportals; i++)
	{
		p = leaf->portals[i];

		if (!(prevstack->mightsee[p->leaf >> 3] & (1 << (p->leaf & 7))))
		{
			c_leafskip++;
			continue;	// can't possibly see it
		}

		// if the portal can't see anything we haven't allready seen, skip it
		if (p->status == stat_done)
		{
			c_vistest++;
			test = (long *)p->visbits;
		}
		else
		{
			c_mighttest++;
			test = (long *)p->mightsee;
		}
		more = false;
		for (j = 0; j < bitlongs; j++)
		{
			might[j] = ((long *)prevstack->mightsee)[j] & test[j];
			if (might[j] & ~vis[j])
				more = true;
		}
		
		if (!more)
		{
			// can't see anything new
			c_portalskip++;
			continue;
		}
		
		// get plane of portal, point normal into the neighbor leaf
		stack.portalplane = *p;
		backplane.normal = -p->normal;
		backplane.dist = -p->dist;
			
		if (prevstack->portalplane.normal == backplane.normal)
			continue;	// can't go out a coplanar face
	
		c_portalcheck++;
		
		stack.portal = p;
		stack.next = NULL;
		
		target = ClipWinding(&p->winding, &thread->pstack_head.portalplane);
		if (!target)
			continue;
			
		if (!prevstack->pass)
		{
			// the second leaf can only be blocked if coplanar
			stack.source = prevstack->source;
			stack.pass = target;
			RecursiveLeafFlow(p->leaf, thread, &stack);
			FreeWinding(target);
			continue;
		}

		target = ClipWinding(target, &prevstack->portalplane);
		if (!target)
			continue;
		
		source = CopyWinding(prevstack->source);

		source = ClipWinding(source, &backplane);
		if (!source)
		{
			FreeWinding(target);
			continue;
		}

		c_portaltest++;

		if (testlevel > 0)
		{
			target = ClipToSeperators(source, prevstack->pass, target);
			if (!target)
			{
				FreeWinding(source);
				continue;
			}
		}
		
		if (testlevel > 1)
		{
			source = ClipToSeperators(target, prevstack->pass, source);
			if (!source)
			{
				FreeWinding(target);
				continue;
			}
		}
		
		stack.source = source;
		stack.pass = target;

		c_portalpass++;
	
		// flow through it for real
		RecursiveLeafFlow(p->leaf, thread, &stack);
		
		FreeWinding(source);
		FreeWinding(target);
	}
	
	delete[] stack.mightsee;
}

//==========================================================================
//
//	PortalFlow
//
//==========================================================================

void PortalFlow(portal_t *p)
{
	threaddata_t	data;

	if (p->status != stat_working)
		Error("PortalFlow: reflowed");
	p->status = stat_working;
	
	p->visbits = new byte[bitbytes];

	memset(&data, 0, sizeof(data));
	data.leafvis = p->visbits;
	data.base = p;
	
	data.pstack_head.portal = p;
	data.pstack_head.source = &p->winding;
	data.pstack_head.portalplane = *p;
	data.pstack_head.mightsee = p->mightsee;
		
	RecursiveLeafFlow(p->leaf, &data, &data.pstack_head);

	delete[] p->mightsee;
	p->status = stat_done;
}

//==========================================================================
//
//	GetNextPortal
//
//	Returns the next portal for a thread to work on
//	Returns the portals from the least complex, so the later ones can reuse
// the earlier information.
//
//==========================================================================

static portal_t *GetNextPortal(void)
{
	int			j;
	portal_t	*p, *tp;
	int			min;
	
	min = 99999;
	p = NULL;
	
	for (j = 0, tp = portals; j < numportals; j++, tp++)
	{
		if (tp->nummightsee < min && tp->status == stat_none)
		{
			min = tp->nummightsee;
			p = tp;
		}
	}

	if (p)
		p->status = stat_working;

	return p;
}

//==========================================================================
//
//	CalcPortalVis
//
//==========================================================================

static void CalcPortalVis(void)
{
	int		i;

	// fastvis just uses mightsee for a very loose bound
	if (fastvis)
	{
		for (i = 0; i < numportals; i++)
		{
			portals[i].visbits = portals[i].mightsee;
			portals[i].status = stat_done;
		}
		return;
	}

	portal_t	*p;
	i = 0;

	do
	{
		if (show_progress && (!i || (numportals - i) % 10 == 0))
		{
			fprintf(stderr, "%04d\b\b\b\b", numportals - i);
		}

		p = GetNextPortal();
		if (!p)
			break;
			
		PortalFlow(p);
		
		if (verbose)
			printf("portal:%4i  mightsee:%4i  cansee:%4i\n", (int)(p - portals), p->nummightsee, p->numcansee);
		i++;
	} while (1);

	if (verbose)
	{
		printf("portalcheck: %i  portaltest: %i  portalpass: %i\n",c_portalcheck, c_portaltest, c_portalpass);
		printf("c_vistest: %i  c_mighttest: %i\n",c_vistest, c_mighttest);
	}
}

//==========================================================================
//
//	LeafFlow
//
//	Builds the entire visibility list for a leaf
//
//==========================================================================

static void LeafFlow(int leafnum)
{
	leaf_t		*leaf;
	byte		*outbuffer;
	int			i, j;
	portal_t	*p;
	int			numvis;

	//
	// flow through all portals, collecting visible bits
	//
	outbuffer = vis + leafnum * rowbytes;
	leaf = &subsectors[leafnum];
	for (i = 0; i < leaf->numportals; i++)
	{
		p = leaf->portals[i];
		if (p->status != stat_done)
			Error("portal %d not done", (int)(p - portals));
		for (j = 0; j < rowbytes; j++)
			outbuffer[j] |= p->visbits[j];
		delete[] p->visbits;
	}

	if (outbuffer[leafnum >> 3] & (1 << (leafnum & 7)))
		Error("Leaf portals saw into leaf");
		
	outbuffer[leafnum >> 3] |= (1 << (leafnum & 7));

	numvis = 0;
	for (i = 0; i < numsubsectors; i++)
		if (outbuffer[i >> 3] & (1 << (i & 3)))
			numvis++;
	totalvis += numvis;
			
	if (verbose)
		printf("leaf %4i : %4i visible\n", leafnum, numvis);
}

//==========================================================================
//
//	BuildPVS
//
//==========================================================================

void BuildPVS(void)
{
	int i;

	if (!silent_mode)
	{
		cerr << "Creating vis data ... ";
	}

	bitbytes = ((numsubsectors + 63) & ~63) >> 3;
	bitlongs = bitbytes / sizeof(long);
	rowbytes = (numsubsectors + 7) >> 3;

	BasePortalVis();
	
	CalcPortalVis();

	//
	// assemble the leaf vis lists by oring and compressing the portal lists
	//
	totalvis = 0;
	vis = new byte[rowbytes * numsubsectors];
	for (i = 0; i < numsubsectors; i++)
	{
		LeafFlow(i);
	}
		
	if (!silent_mode)
	{
		cerr << totalvis << " accepts, " << (numsubsectors * numsubsectors - totalvis) << " rejects\n";
	}

	//	Write lump
	outwad.AddLump("GL_PVS", vis, rowbytes * numsubsectors);

	delete vis;
}

