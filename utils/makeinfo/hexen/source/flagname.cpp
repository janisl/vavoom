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

int					shadow = 66;
int					altshadow = 33;

char* flagnames1[32] =
{
    "bSpecial",
    "bSolid",
    "bShootable",
    "bNoClient",
    "bNoBlockmap",
    "bAmbush",
    "bJustHit",
    "bJustAttacked",

    "bSpawnCeiling",
    "bNoGravity",
    "bDropOff",
    "bPickUp",
    "bNoClip",
    "bSlide__",
    "bFloat",
    "bTeleport",

    "bMissile",
	"MF_ALTSHADOW",
    "MF_SHADOW",
    "bNoBlood",
    "bCorpse",
    "bInFloat",
    "bCountKill",
	"bIceCorpse",

    "bSkullFly",
    "MF_NOTDMATCH",
    "MF_TRANSLATION1",
    "MF_TRANSLATION2",
    "MF_TRANSLUCENT",
	"MF_UNUSED1",
	"MF_UNUSED2",
	"MF_UNUSED3",
};
char* flagnames2[32] =
{
    "bLowGravity",
    "bWindThrust",
    "bFloorBounce",
	"bBlasted",
    "bFly",
	"bFloorClip",
    "bSpawnFloat",
    "bNoTeleport",

    "bRip",
    "bPushable",
    "bSlide",
    "bOnMobj",
    "bNoPassMobj",
    "bCannotPush",
	"bDropped",
    "bBoss",

    "bFireDamage",
    "bNoDamageThrust",
    "bTelestomp",
    "bFloatBob",
    "MF2_DONTDRAW",
    "bActivateImpact",
    "bActivatePushWall",
    "bActivateMCross",

    "bActivatePCross",
    "bCantLeaveFloorpic",
    "bNonShootable",
    "bInvulnerable",
    "bDormant",
    "bIceDamage",
    "bSeekerMissile",
    "bReflective",
};

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2002/02/06 17:31:46  dj_jl
//	Replaced Actor flags with boolean variables.
//
//	Revision 1.5  2002/01/24 18:21:20  dj_jl
//	Inverted passmobj flag
//	
//	Revision 1.4  2002/01/07 12:30:05  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2001/09/20 16:34:58  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
