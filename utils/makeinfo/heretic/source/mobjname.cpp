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
//**************************************************************************
//**
//** Info.h
//**
//**************************************************************************

char* mobj_names[] =
{
    "misc_MISC0",
    "misc_ITEMSHIELD1",
    "misc_ITEMSHIELD2",
    "misc_MISC1",
    "misc_MISC2",
    "misc_ARTIINVISIBILITY",
    "misc_MISC3",
    "misc_ARTIFLY",
    "misc_ARTIINVULNERABILITY",
    "misc_ARTITOMEOFPOWER",
    "misc_ARTIEGG",
    "misc_EGGFX",
    "misc_ARTISUPERHEAL",
    "misc_MISC4",
    "misc_MISC5",
    "misc_FIREBOMB",
    "misc_ARTITELEPORT",
    "misc_POD",
    "misc_PODGOO",
    "misc_PODGENERATOR",
    "misc_SPLASH",
    "misc_SPLASHBASE",
    "misc_LAVASPLASH",
    "misc_LAVASMOKE",
    "misc_SLUDGECHUNK",
    "misc_SLUDGESPLASH",
    "misc_SKULLHANG70",
    "misc_SKULLHANG60",
    "misc_SKULLHANG45",
    "misc_SKULLHANG35",
    "misc_CHANDELIER",
    "misc_SERPTORCH",
    "misc_SMALLPILLAR",
    "misc_STALAGMITESMALL",
    "misc_STALAGMITELARGE",
    "misc_STALACTITESMALL",
    "misc_STALACTITELARGE",
    "misc_MISC6",
    "misc_BARREL",
    "misc_MISC7",
    "misc_MISC8",
    "misc_MISC9",
    "misc_MISC10",
    "misc_MISC11",
    "misc_KEYGIZMOBLUE",
    "misc_KEYGIZMOGREEN",
    "misc_KEYGIZMOYELLOW",
    "misc_KEYGIZMOFLOAT",
    "misc_MISC12",
    "misc_VOLCANOBLAST",
    "misc_VOLCANOTBLAST",
    "misc_TELEGLITGEN",
    "misc_TELEGLITGEN2",
    "misc_TELEGLITTER",
    "misc_TELEGLITTER2",
    "misc_TFOG",
    "misc_TELEPORTMAN",
    "misc_STAFFPUFF",
    "misc_STAFFPUFF2",
    "misc_BEAKPUFF",
    "misc_MISC13",
    "misc_GAUNTLETPUFF1",
    "misc_GAUNTLETPUFF2",
    "misc_MISC14",
    "misc_BLASTERFX1",
    "misc_BLASTERSMOKE",
    "misc_RIPPER",
    "misc_BLASTERPUFF1",
    "misc_BLASTERPUFF2",
    "misc_WMACE",
    "misc_MACEFX1",
    "misc_MACEFX2",
    "misc_MACEFX3",
    "misc_MACEFX4",
    "misc_WSKULLROD",
    "misc_HORNRODFX1",
    "misc_HORNRODFX2",
    "misc_RAINPLR1",
    "misc_RAINPLR2",
    "misc_RAINPLR3",
    "misc_RAINPLR4",
    "misc_GOLDWANDFX1",
    "misc_GOLDWANDFX2",
    "misc_GOLDWANDPUFF1",
    "misc_GOLDWANDPUFF2",
    "misc_WPHOENIXROD",
    "misc_PHOENIXFX1",
    "misc_PHOENIXPUFF",
    "misc_PHOENIXFX2",
    "misc_MISC15",
    "misc_CRBOWFX1",
    "misc_CRBOWFX2",
    "misc_CRBOWFX3",
    "misc_CRBOWFX4",
    "misc_BLOOD",
    "misc_BLOODSPLATTER",
    "misc_PLAYER",
    "misc_BLOODYSKULL",
    "misc_CHICPLAYER",
    "misc_CHICKEN",
    "misc_FEATHER",
    "misc_MUMMY",
    "misc_MUMMYLEADER",
    "misc_MUMMYGHOST",
    "misc_MUMMYLEADERGHOST",
    "misc_MUMMYSOUL",
    "misc_MUMMYFX1",
    "misc_BEAST",
    "misc_BEASTBALL",
    "misc_BURNBALL",
    "misc_BURNBALLFB",
    "misc_PUFFY",
    "misc_SNAKE",
    "misc_SNAKEPRO_A",
    "misc_SNAKEPRO_B",
    "misc_HEAD",
    "misc_HEADFX1",
    "misc_HEADFX2",
    "misc_HEADFX3",
    "misc_WHIRLWIND",
    "misc_CLINK",
    "misc_WIZARD",
    "misc_WIZFX1",
    "misc_IMP",
    "misc_IMPLEADER",
    "misc_IMPCHUNK1",
    "misc_IMPCHUNK2",
    "misc_IMPBALL",
    "misc_KNIGHT",
    "misc_KNIGHTGHOST",
    "misc_KNIGHTAXE",
    "misc_REDAXE",
    "misc_SORCERER1",
    "misc_SRCRFX1",
    "misc_SORCERER2",
    "misc_SOR2FX1",
    "misc_SOR2FXSPARK",
    "misc_SOR2FX2",
    "misc_SOR2TELEFADE",
    "misc_MINOTAUR",
    "misc_MNTRFX1",
    "misc_MNTRFX2",
    "misc_MNTRFX3",
    "misc_AKYY",
    "misc_BKYY",
    "misc_CKEY",
    "misc_AMGWNDWIMPY",
    "misc_AMGWNDHEFTY",
    "misc_AMMACEWIMPY",
    "misc_AMMACEHEFTY",
    "misc_AMCBOWWIMPY",
    "misc_AMCBOWHEFTY",
    "misc_AMSKRDWIMPY",
    "misc_AMSKRDHEFTY",
    "misc_AMPHRDWIMPY",
    "misc_AMPHRDHEFTY",
    "misc_AMBLSRWIMPY",
    "misc_AMBLSRHEFTY",
    "misc_SOUNDWIND",
    "misc_SOUNDWATERFALL",
};

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/09/20 16:34:58  dj_jl
//	Beautification
//
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
