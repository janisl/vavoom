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
//**
//**	Rendering of particles.
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

VCvarI				r_draw_particles("r_draw_particles", "1", CVAR_Archive);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VRenderLevelShared::InitParticles
//
//==========================================================================

void VRenderLevelShared::InitParticles()
{
	guard(VRenderLevelShared::InitParticles);
	const char* p = GArgs.CheckValue("-particles");

	if (p)
	{
		NumParticles = atoi(p);
		if (NumParticles < ABSOLUTE_MIN_PARTICLES)
			NumParticles = ABSOLUTE_MIN_PARTICLES;
	}
	else
	{
		NumParticles = MAX_PARTICLES;
	}

	Particles = new particle_t[NumParticles];
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::ClearParticles
//
//==========================================================================

void VRenderLevelShared::ClearParticles()
{
	guard(VRenderLevelShared::ClearParticles);
	FreeParticles = &Particles[0];
	ActiveParticles = NULL;

	for (int i = 0; i < NumParticles; i++)
		Particles[i].next = &Particles[i + 1];
	Particles[NumParticles - 1].next = NULL;
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::NewParticle
//
//==========================================================================

particle_t* VRenderLevelShared::NewParticle()
{
	guard(VRenderLevelShared::NewParticle);
	if (!FreeParticles)
	{
		//	No free particles
		return NULL;
	}
	//	Remove from list of free particles
	particle_t* p = FreeParticles;
	FreeParticles = p->next;
	//	Clean
	memset(p, 0, sizeof(*p));
	//	Add to active particles
	p->next = ActiveParticles;
	ActiveParticles = p;
	return p;
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::UpdateParticles
//
//==========================================================================

void VRenderLevelShared::UpdateParticles(float frametime)
{
	guard(VRenderLevelShared::UpdateParticles);
	particle_t		*p, *kill;

	if (GGameInfo->IsPaused() ||
		(Level->LevelInfo->LevelInfoFlags2 & VLevelInfo::LIF2_Frozen))
	{
		return;
	}

	kill = ActiveParticles;
	while (kill && kill->die < Level->Time)
	{
		ActiveParticles = kill->next;
		kill->next = FreeParticles;
		FreeParticles = kill;
		kill = ActiveParticles;
	}

	for (p = ActiveParticles; p; p = p->next)
	{
		kill = p->next;
		while (kill && kill->die < Level->Time)
		{
			p->next = kill->next;
			kill->next = FreeParticles;
			FreeParticles = kill;
			kill = p->next;
		}

		p->org += (p->vel * frametime);
		Level->LevelInfo->eventUpdateParticle(p, frametime);
	}
	unguard;
}

//==========================================================================
//
//	VRenderLevelShared::DrawParticles
//
//==========================================================================

void VRenderLevelShared::DrawParticles()
{
	guard(VRenderLevelShared::DrawParticles);
	if (!r_draw_particles)
	{
		return;
	}
	Drawer->StartParticles();
	for (particle_t* p = ActiveParticles; p; p = p->next)
	{
		if (ColourMap)
		{
			vuint32 Col = p->colour;
			rgba_t TmpCol = ColourMaps[ColourMap].GetPalette()[R_LookupRGB(
				(Col >> 16) & 255, (Col >> 8) & 255, Col & 255)];
			p->colour = (Col & 0xff000000) | (TmpCol.r << 16) |
				(TmpCol.g << 8) | TmpCol.b;
			Drawer->DrawParticle(p);
			p->colour = Col;
		}
		else
		{
			Drawer->DrawParticle(p);
		}
	}
	Drawer->EndParticles();
	unguard;
}
