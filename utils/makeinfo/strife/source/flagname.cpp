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

char* flagnames1[32] =
{
    "bSpecial",
    "bSolid",
    "bShootable",
    "bHidden",
    "bNoBlockmap",
    "bAmbush",
    "bJustHit",
    "bJustAttacked",

    "bSpawnCeiling",
    "bNoGravity",
    "bDropOff",
    "bPickUp",
    "bNoClip",
    "bSlide",
    "bFloat",
    "bUnknown4",

    "bMissile",
    "bDropped",
    "MF_SHADOW",
    "bNoBlood",
    "bCorpse",
    "bInFloat",
    "bCountKill",
    "bCountItem",

    "bSkullFly",
    "MF_NOTDMATCH",
    "bUnknown1",
    "bUnknown2",
	"MF_TRANSLATION1",
	"MF_TRANSLATION2",
	"MF_TRANSLATION3",
    "bUnknown3",
};
char* flagnames2[32] =
{
    "MF2_FLAG1",
    "MF2_FLAG2",
    "MF2_FLAG3",
    "MF2_FLAG4",
    "MF2_FLAG5",
    "MF2_FLAG6",
    "MF2_FLAG7",
    "MF2_FLAG8",

    "MF2_FLAG9",
    "MF2_FLAG10",
    "MF2_FLAG11",
    "MF2_FLAG12",
    "MF2_FLAG13",
    "MF2_FLAG14",
    "MF2_FLAG15",
    "MF2_FLAG16",

    "MF2_FLAG17",
    "MF2_FLAG18",
    "MF2_FLAG19",
    "MF2_FLAG20",
    "MF2_FLAG21",
    "MF2_FLAG22",
    "MF2_FLAG23",
    "bActivateMCross",

    "bActivatePCross",
    "MF2_FLAG26",
    "MF2_FLAG27",
    "MF2_FLAG28",
    "MF2_FLAG29",
    "MF2_FLAG30",
    "MF2_FLAG31",
    "MF2_FLAG32",
};

//**************************************************************************
//
//	$Log$
//	Revision 1.8  2002/03/20 19:12:23  dj_jl
//	Updated to current state.
//
//	Revision 1.7  2002/02/22 18:11:01  dj_jl
//	Some renaming.
//	
//	Revision 1.6  2002/02/06 17:31:46  dj_jl
//	Replaced Actor flags with boolean variables.
//	
//	Revision 1.5  2002/01/29 18:19:01  dj_jl
//	Added MCROSS and PCROSS flags.
//	
//	Revision 1.4  2002/01/07 12:31:35  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2001/09/20 16:35:58  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:56  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
