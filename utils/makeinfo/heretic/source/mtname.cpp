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

char* mt_names[] =
{
    "MT_MISC0",
    "MT_ITEMSHIELD1",
    "MT_ITEMSHIELD2",
    "MT_MISC1",
    "MT_MISC2",
    "MT_ARTIINVISIBILITY",
    "MT_MISC3",
    "MT_ARTIFLY",
    "MT_ARTIINVULNERABILITY",
    "MT_ARTITOMEOFPOWER",
    "MT_ARTIEGG",
    "MT_EGGFX",
    "MT_ARTISUPERHEAL",
    "MT_MISC4",
    "MT_MISC5",
    "MT_FIREBOMB",
    "MT_ARTITELEPORT",
    "MT_POD",
    "MT_PODGOO",
    "MT_PODGENERATOR",
    "MT_SPLASH",
    "MT_SPLASHBASE",
    "MT_LAVASPLASH",
    "MT_LAVASMOKE",
    "MT_SLUDGECHUNK",
    "MT_SLUDGESPLASH",
    "MT_SKULLHANG70",
    "MT_SKULLHANG60",
    "MT_SKULLHANG45",
    "MT_SKULLHANG35",
    "MT_CHANDELIER",
    "MT_SERPTORCH",
    "MT_SMALLPILLAR",
    "MT_STALAGMITESMALL",
    "MT_STALAGMITELARGE",
    "MT_STALACTITESMALL",
    "MT_STALACTITELARGE",
    "MT_MISC6",
    "MT_BARREL",
    "MT_MISC7",
    "MT_MISC8",
    "MT_MISC9",
    "MT_MISC10",
    "MT_MISC11",
    "MT_KEYGIZMOBLUE",
    "MT_KEYGIZMOGREEN",
    "MT_KEYGIZMOYELLOW",
    "MT_KEYGIZMOFLOAT",
    "MT_MISC12",
    "MT_VOLCANOBLAST",
    "MT_VOLCANOTBLAST",
    "MT_TELEGLITGEN",
    "MT_TELEGLITGEN2",
    "MT_TELEGLITTER",
    "MT_TELEGLITTER2",
    "MT_TFOG",
    "MT_TELEPORTMAN",
    "MT_STAFFPUFF",
    "MT_STAFFPUFF2",
    "MT_BEAKPUFF",
    "MT_MISC13",
    "MT_GAUNTLETPUFF1",
    "MT_GAUNTLETPUFF2",
    "MT_MISC14",
    "MT_BLASTERFX1",
    "MT_BLASTERSMOKE",
    "MT_RIPPER",
    "MT_BLASTERPUFF1",
    "MT_BLASTERPUFF2",
    "MT_WMACE",
    "MT_MACEFX1",
    "MT_MACEFX2",
    "MT_MACEFX3",
    "MT_MACEFX4",
    "MT_WSKULLROD",
    "MT_HORNRODFX1",
    "MT_HORNRODFX2",
    "MT_RAINPLR1",
    "MT_RAINPLR2",
    "MT_RAINPLR3",
    "MT_RAINPLR4",
    "MT_GOLDWANDFX1",
    "MT_GOLDWANDFX2",
    "MT_GOLDWANDPUFF1",
    "MT_GOLDWANDPUFF2",
    "MT_WPHOENIXROD",
    "MT_PHOENIXFX1",
    "MT_PHOENIXPUFF",
    "MT_PHOENIXFX2",
    "MT_MISC15",
    "MT_CRBOWFX1",
    "MT_CRBOWFX2",
    "MT_CRBOWFX3",
    "MT_CRBOWFX4",
    "MT_BLOOD",
    "MT_BLOODSPLATTER",
    "MT_PLAYER",
    "MT_BLOODYSKULL",
    "MT_CHICPLAYER",
    "MT_CHICKEN",
    "MT_FEATHER",
    "MT_MUMMY",
    "MT_MUMMYLEADER",
    "MT_MUMMYGHOST",
    "MT_MUMMYLEADERGHOST",
    "MT_MUMMYSOUL",
    "MT_MUMMYFX1",
    "MT_BEAST",
    "MT_BEASTBALL",
    "MT_BURNBALL",
    "MT_BURNBALLFB",
    "MT_PUFFY",
    "MT_SNAKE",
    "MT_SNAKEPRO_A",
    "MT_SNAKEPRO_B",
    "MT_HEAD",
    "MT_HEADFX1",
    "MT_HEADFX2",
    "MT_HEADFX3",
    "MT_WHIRLWIND",
    "MT_CLINK",
    "MT_WIZARD",
    "MT_WIZFX1",
    "MT_IMP",
    "MT_IMPLEADER",
    "MT_IMPCHUNK1",
    "MT_IMPCHUNK2",
    "MT_IMPBALL",
    "MT_KNIGHT",
    "MT_KNIGHTGHOST",
    "MT_KNIGHTAXE",
    "MT_REDAXE",
    "MT_SORCERER1",
    "MT_SRCRFX1",
    "MT_SORCERER2",
    "MT_SOR2FX1",
    "MT_SOR2FXSPARK",
    "MT_SOR2FX2",
    "MT_SOR2TELEFADE",
    "MT_MINOTAUR",
    "MT_MNTRFX1",
    "MT_MNTRFX2",
    "MT_MNTRFX3",
    "MT_AKYY",
    "MT_BKYY",
    "MT_CKEY",
    "MT_AMGWNDWIMPY",
    "MT_AMGWNDHEFTY",
    "MT_AMMACEWIMPY",
    "MT_AMMACEHEFTY",
    "MT_AMCBOWWIMPY",
    "MT_AMCBOWHEFTY",
    "MT_AMSKRDWIMPY",
    "MT_AMSKRDHEFTY",
    "MT_AMPHRDWIMPY",
    "MT_AMPHRDHEFTY",
    "MT_AMBLSRWIMPY",
    "MT_AMBLSRHEFTY",
    "MT_SOUNDWIND",
    "MT_SOUNDWATERFALL",
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
