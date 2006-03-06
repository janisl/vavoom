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
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
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
//**	THINKERS
//**
//**	All thinkers should be allocated by Z_Malloc so they can be operated
//**  on uniformly. The actual structures will vary in size, but the first
//**  element must be VThinker.
//**	
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"
extern VLevelInfo*		GLevelInfo;

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int			VThinker::FIndex_Tick;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(V, Thinker)

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VThinker::VThinker
//
//==========================================================================

VThinker::VThinker()
{
}

//==========================================================================
//
//	VThinker::Serialise
//
//==========================================================================

void VThinker::Serialise(VStream& Strm)
{
	guard(VThinker::Serialise);
	Super::Serialise(Strm);
	if (Strm.IsLoading())
	{
		XLevel->AddThinker(this);
	}
	unguard;
}

//==========================================================================
//
//	VThinker::Tick
//
//==========================================================================

void VThinker::Tick(float DeltaTime)
{
	guard(VThinker::Tick);
	P_PASS_SELF;
	P_PASS_FLOAT(DeltaTime);
	EV_RET_VOID(FIndex_Tick);
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2006/03/06 13:12:12  dj_jl
//	Client now uses entity class.
//
//**************************************************************************
