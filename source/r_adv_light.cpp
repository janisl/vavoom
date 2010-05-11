//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: r_light.cpp 4220 2010-04-24 15:24:35Z dj_jl $
//**
//**	Copyright (C) 1999-2006 Jānis Legzdiņš
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
#include "r_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

extern VCvarI				r_darken;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VAdvancedRenderLevel::AddStaticLight
//
//==========================================================================

void VAdvancedRenderLevel::AddStaticLight(const TVec &origin, float radius,
	vuint32 colour)
{
	guard(VAdvancedRenderLevel::AddStaticLight);
	light_t& L = Lights.Alloc();
	L.origin = origin;
	L.radius = radius;
	L.colour = colour;
	L.leafnum = Level->PointInSubsector(origin) - Level->Subsectors;
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::CastRay
//
//	Returns the distance between the points, or -1 if blocked
//
//==========================================================================

float VAdvancedRenderLevel::CastRay(const TVec &p1, const TVec &p2,
	float squaredist)
{
	guard(VAdvancedRenderLevel::CastRay);
	linetrace_t		Trace;

	TVec delta = p2 - p1;
	float t = DotProduct(delta, delta);
	if (t > squaredist)
		return -1;		// too far away
		
	if (!Level->TraceLine(Trace, p1, p2, SPF_NOBLOCKSIGHT))
		return -1;		// ray was blocked
		
	if (t == 0)
		t = 1;			// don't blow up...
	return sqrt(t);
	unguard;
}

//**************************************************************************
//**
//**	DYNAMIC LIGHTS
//**
//**************************************************************************

//==========================================================================
//
//	VAdvancedRenderLevel::AllocDlight
//
//==========================================================================

dlight_t* VAdvancedRenderLevel::AllocDlight(VThinker* Owner)
{
	guard(VAdvancedRenderLevel::AllocDlight);
	int			i;
	dlight_t*	dl;

	// first look for an exact key match
	if (Owner)
	{
		dl = DLights;
		for (i = 0; i < MAX_DLIGHTS; i++, dl++)
		{
			if (dl->Owner == Owner)
			{
				memset(dl, 0, sizeof(*dl));
				dl->Owner = Owner;
				return dl;
			}
		}
	}

	// then look for anything else
	dl = DLights;
	for (i = 0; i < MAX_DLIGHTS; i++, dl++)
	{
		if (dl->die < Level->Time)
		{
			memset(dl, 0, sizeof(*dl));
			dl->Owner = Owner;
			return dl;
		}
	}

	int bestnum = 0;
	float bestdist = 0.0;
	for (i = 0; i < MAX_DLIGHTS; i++, dl++)
	{
		float dist = Length(dl->origin - cl->ViewOrg);
		if (dist > bestdist)
		{
			bestnum = i;
			bestdist = dist;
		}
	}
	dl = &DLights[bestnum];
	memset(dl, 0, sizeof(*dl));
	dl->Owner = Owner;
	return dl;
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::DecayLights
//
//==========================================================================

void VAdvancedRenderLevel::DecayLights(float time)
{
	guard(VAdvancedRenderLevel::DecayLights);
	dlight_t* dl = DLights;
	for (int i = 0; i < MAX_DLIGHTS; i++, dl++)
	{
		if (dl->die < Level->Time || !dl->radius)
			continue;

		dl->radius -= time * dl->decay;
		if (dl->radius < 0)
			dl->radius = 0;
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::LightPoint
//
//==========================================================================

vuint32 VAdvancedRenderLevel::LightPoint(const TVec &p)
{
	guard(VAdvancedRenderLevel::LightPoint);
	subsector_t		*sub;
	subregion_t		*reg;
	float			l, lr, lg, lb, d, add;
	int				i, s, t, ds, dt;
	surface_t		*surf;
	int				ltmp;
	rgb_t			*rgbtmp;
	int				leafnum;
	linetrace_t		Trace;

	if (FixedLight)
	{
		return FixedLight | (FixedLight << 8) | (FixedLight << 16) | (FixedLight << 24);
	}

	sub = Level->PointInSubsector(p);
	vuint8* dyn_facevis = Level->LeafPVS(sub);
	reg = sub->regions;
	while (reg->next)
	{
		d = DotProduct(p, reg->floor->secplane->normal) - reg->floor->secplane->dist;

		if (d >= 0.0)
		{
			break;
		}

		reg = reg->next;
	}

	//	Region's base light
	l = reg->secregion->params->lightlevel + ExtraLight;
	if (r_darken)
	{
		l = light_remap[MIN(255, (int)l)];
	}
	l = MIN(255, l);
	int SecLightColour = reg->secregion->params->LightColour;
	lr = ((SecLightColour >> 16) & 255) * l / 255.0;
	lg = ((SecLightColour >> 8) & 255) * l / 255.0;
	lb = (SecLightColour & 255) * l / 255.0;

	//	Add static lights
	for (i = 0; i < Lights.Num(); i++)
	{
		leafnum = Level->PointInSubsector(Lights[i].origin) -
			Level->Subsectors;

		// Check potential visibility
		if (!(dyn_facevis[leafnum >> 3] & (1 << (leafnum & 7))))
		{
			continue;
		}

		add = Lights[i].radius - Length(p - Lights[i].origin);
		if (add <= 0)
		{
			continue;
		}

		if (!Level->TraceLine(Trace, Lights[i].origin, p, SPF_NOBLOCKSIGHT))
		{
			// ray was blocked
			continue;
		}

		l += add;
		lr += add * ((Lights[i].colour >> 16) & 0xff) / 255.0;
		lg += add * ((Lights[i].colour >> 8) & 0xff) / 255.0;
		lb += add * (Lights[i].colour & 0xff) / 255.0;
	}

	//	Add dynamic lights
	for (i = 0; i < MAX_DLIGHTS; i++)
	{
		leafnum = Level->PointInSubsector(DLights[i].origin) -
			Level->Subsectors;

		// Check potential visibility
		if (!(dyn_facevis[leafnum >> 3] & (1 << (leafnum & 7))))
		{
			continue;
		}

		add = (DLights[i].radius - DLights[i].minlight) - Length(p - DLights[i].origin);

		if (add > 0)
		{
			l += add;
			lr += add * ((DLights[i].colour >> 16) & 0xff) / 255.0;
			lg += add * ((DLights[i].colour >> 8) & 0xff) / 255.0;
			lb += add * (DLights[i].colour & 0xff) / 255.0;
		}
	}

	if (l > 255)
		l = 255;
	if (lr > 255)
		lr = 255;
	if (lg > 255)
		lg = 255;
	if (lb > 255)
		lb = 255;

	return ((int)l << 24) | ((int)lr << 16) | ((int)lg << 8) | ((int)lb);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::LightPointAmbient
//
//==========================================================================

vuint32 VAdvancedRenderLevel::LightPointAmbient(const TVec &p)
{
	guard(VAdvancedRenderLevel::LightPointAmbient);
	subsector_t		*sub;
	subregion_t		*reg;
	float			l, lr, lg, lb, d, add;
	int				i, s, t, ds, dt;
	surface_t		*surf;
	int				ltmp;
	rgb_t			*rgbtmp;
	int				leafnum;
	linetrace_t		Trace;

	if (FixedLight)
	{
		return FixedLight | (FixedLight << 8) | (FixedLight << 16) | (FixedLight << 24);
	}

	sub = Level->PointInSubsector(p);
	vuint8* dyn_facevis = Level->LeafPVS(sub);
	reg = sub->regions;
	while (reg->next)
	{
		d = DotProduct(p, reg->floor->secplane->normal) - reg->floor->secplane->dist;

		if (d >= 0.0)
		{
			break;
		}

		reg = reg->next;
	}

	//	Region's base light
	l = reg->secregion->params->lightlevel + ExtraLight;
	if (r_darken)
	{
		l = light_remap[MIN(255, (int)l)];
	}
	l = MIN(255, l);
	int SecLightColour = reg->secregion->params->LightColour;
	lr = ((SecLightColour >> 16) & 255) * l / 255.0;
	lg = ((SecLightColour >> 8) & 255) * l / 255.0;
	lb = (SecLightColour & 255) * l / 255.0;

	return ((int)l << 24) | ((int)lr << 16) | ((int)lg << 8) | ((int)lb);
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::BuildLightMap
//
//==========================================================================

bool VAdvancedRenderLevel::BuildLightMap(surface_t *surf, int shift)
{
	return true;
}
