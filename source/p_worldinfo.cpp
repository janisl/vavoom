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

// HEADER FILES ------------------------------------------------------------

#include "gamedefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

IMPLEMENT_CLASS(V, WorldInfo)

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VWorldInfo::VWorldInfo
//
//==========================================================================

VWorldInfo::VWorldInfo()
{
	Acs = new VAcsGlobal;
}

//==========================================================================
//
//	VWorldInfo::Serialise
//
//==========================================================================

void VWorldInfo::Serialise(VStream& Strm)
{
	guard(VWorldInfo::Serialise);
	//	Serialise global script info.
	Acs->Serialise(Strm);

	Super::Serialise(Strm);
	unguard;
}

//==========================================================================
//
//	VWorldInfo::Destroy
//
//==========================================================================

void VWorldInfo::Destroy()
{
	guard(VWorldInfo::Destroy);
	delete Acs;

	Super::Destroy();
	unguard;
}

//==========================================================================
//
//	VWorldInfo::SetSkill
//
//==========================================================================

void VWorldInfo::SetSkill(int ASkill)
{
	guard(VWorldInfo::SetSkill);
	if (ASkill < 0)
	{
		GameSkill = 0;
	}
	else if (ASkill >= P_GetNumSkills())
	{
		GameSkill = P_GetNumSkills() - 1;
	}
	else
	{
		GameSkill = ASkill;
	}
	const VSkillDef* SDef = P_GetSkillDef(GameSkill);

	SkillAmmoFactor = SDef->AmmoFactor;
	SkillDoubleAmmoFactor = SDef->DoubleAmmoFactor;
	SkillDamageFactor = SDef->DamageFactor;
	SkillRespawnTime = SDef->RespawnTime;
	SkillRespawnLimit = SDef->RespawnLimit;
	SkillAggressiveness = SDef->Aggressiveness;
	SkillSpawnFilter = SDef->SpawnFilter;
	SkillAcsReturn = SDef->AcsReturn;
	Flags = (Flags & 0xfffffff0) | (SDef->Flags & 0x0000000f);
	unguard;
}

//==========================================================================
//
//	VWorldInfo
//
//==========================================================================

IMPLEMENT_FUNCTION(VWorldInfo, SetSkill)
{
	P_GET_INT(Skill);
	P_GET_SELF;
	Self->SetSkill(Skill);
}
