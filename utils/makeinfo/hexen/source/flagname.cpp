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
    "MF_SPECIAL",
    "MF_SOLID",
    "MF_SHOOTABLE",
    "MF_NOSECTOR",
    "MF_NOBLOCKMAP",
    "MF_AMBUSH",
    "MF_JUSTHIT",
    "MF_JUSTATTACKED",

    "MF_SPAWNCEILING",
    "MF_NOGRAVITY",
    "MF_DROPOFF",
    "MF_PICKUP",
    "MF_NOCLIP",
    "MF_SLIDE",
    "MF_FLOAT",
    "MF_TELEPORT",

    "MF_MISSILE",
	"MF_ALTSHADOW",
    "MF_SHADOW",
    "MF_NOBLOOD",
    "MF_CORPSE",
    "MF_INFLOAT",
    "MF_COUNTKILL",
	"MF_ICECORPSE",

    "MF_SKULLFLY",
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
    "MF2_LOGRAV",
    "MF2_WINDTHRUST",
    "MF2_FLOORBOUNCE",
	"MF2_BLASTED",
    "MF2_FLY",
	"MF2_FLOORCLIP",
    "MF2_SPAWNFLOAT",
    "MF2_NOTELEPORT",

    "MF2_RIP",
    "MF2_PUSHABLE",
    "MF2_SLIDE",
    "MF2_ONMOBJ",
    "MF2_PASSMOBJ",
    "MF2_CANNOTPUSH",
	"MF2_DROPPED",
    "MF2_BOSS",

    "MF2_FIREDAMAGE",
    "MF2_NODMGTHRUST",
    "MF2_TELESTOMP",
    "MF2_FLOATBOB",
    "MF2_DONTDRAW",
    "MF2_IMPACT",
    "MF2_PUSHWALL",
    "MF2_MCROSS",

    "MF2_PCROSS",
    "MF2_CANTLEAVEFLOORPIC",
    "MF2_NONSHOOTABLE",
    "MF2_INVULNERABLE",
    "MF2_DORMANT",
    "MF2_ICEDAMAGE",
    "MF2_SEEKERMISSILE",
    "MF2_REFLECTIVE",
};

//**************************************************************************
//
//	$Log$
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
