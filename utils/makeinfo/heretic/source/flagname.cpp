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
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:55  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************

int					shadow = 33;
int					altshadow = 0;

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
    "MF_DROPPED",
    "MF_SHADOW",
    "MF_NOBLOOD",
    "MF_CORPSE",
    "MF_INFLOAT",
    "MF_COUNTKILL",
    "MF_COUNTITEM",

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
    "MF2_THRUGHOST",
    "MF2_FLY",
    "MF2_FOOTCLIP",
    "MF2_SPAWNFLOAT",
    "MF2_NOTELEPORT",

    "MF2_RIP",
    "MF2_PUSHABLE",
    "MF2_SLIDE",
    "MF2_ONMOBJ",
    "MF2_PASSMOBJ",
    "MF2_CANNOTPUSH",
    "MF2_FEETARECLIPPED",
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

