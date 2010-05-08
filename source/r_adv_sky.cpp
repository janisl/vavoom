//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id: r_sky.cpp 4179 2010-03-19 19:46:35Z firebrand_kh $
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
//**	Sky rendering. The DOOM sky is a texture map like any wall, wrapping
//**  around. A 1024 columns equal 360 degrees. The default sky map is 256
//**  columns and repeats 4 times on a 320 screen
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

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VAdvancedRenderLevel::InitSky
//
//	Called at level load.
//
//==========================================================================

void VAdvancedRenderLevel::InitSky()
{
	guard(VAdvancedRenderLevel::InitSky);
	if (CurrentSky1Texture == Level->LevelInfo->Sky1Texture &&
		CurrentSky2Texture == Level->LevelInfo->Sky2Texture &&
		CurrentDoubleSky == !!(Level->LevelInfo->LevelInfoFlags & VLevelInfo::LIF_DoubleSky) &&
		CurrentLightning == !!(Level->LevelInfo->LevelInfoFlags & VLevelInfo::LIF_Lightning))
	{
		return;
	}
	CurrentSky1Texture = Level->LevelInfo->Sky1Texture;
	CurrentSky2Texture = Level->LevelInfo->Sky2Texture;
	CurrentDoubleSky = !!(Level->LevelInfo->LevelInfoFlags & VLevelInfo::LIF_DoubleSky);
	CurrentLightning = !!(Level->LevelInfo->LevelInfoFlags & VLevelInfo::LIF_Lightning);

	if (Level->LevelInfo->SkyBox != NAME_None)
	{
		BaseSky.InitSkyBox(Level->LevelInfo->SkyBox, NAME_None);
	}
	else
	{
		BaseSky.Init(CurrentSky1Texture, CurrentSky2Texture,
			Level->LevelInfo->Sky1ScrollDelta,
			Level->LevelInfo->Sky2ScrollDelta, CurrentDoubleSky,
			!!(Level->LevelInfo->LevelInfoFlags &
			VLevelInfo::LIF_ForceNoSkyStretch), true, CurrentLightning);
	}
	unguard;
}

//==========================================================================
//
//	VAdvancedRenderLevel::AnimateSky
//
//==========================================================================

void VAdvancedRenderLevel::AnimateSky(float frametime)
{
	guard(VAdvancedRenderLevel::AnimateSky);
	InitSky();

	//	Update sky column offsets
	for (int i = 0; i < BaseSky.NumSkySurfs; i++)
	{
		BaseSky.sky[i].columnOffset1 += BaseSky.sky[i].scrollDelta1 * frametime;
		BaseSky.sky[i].columnOffset2 += BaseSky.sky[i].scrollDelta2 * frametime;
	}
	unguard;
}
