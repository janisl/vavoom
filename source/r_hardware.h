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

#include "cl_local.h"
#include "r_shared.h"

class VHardwareDrawer : public VDrawer
{
public:
	VHardwareDrawer();

	VRenderLevelDrawer*	RendLev;

	surface_t*		SimpleSurfsHead;
	surface_t*		SimpleSurfsTail;
	surface_t*		SkyPortalsHead;
	surface_t*		SkyPortalsTail;
	surface_t*		HorizonPortalsHead;
	surface_t*		HorizonPortalsTail;

	int			    PortalDepth;

	enum { SHADEDOT_QUANT = 16 };
	static float	r_avertexnormal_dots[SHADEDOT_QUANT][256];
};
