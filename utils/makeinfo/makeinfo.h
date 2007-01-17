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

#include "cmdlib.h"

#define FRACUNIT	(1 << 16)

#define	MF_SPECIAL		0x00000001	// call P_SpecialThing when touched
#define	MF_SOLID		0x00000002	// Blocks.
#define	MF_SHOOTABLE	0x00000004	// Can be hit.
#define	MF_NOSECTOR		0x00000008	// don't use the sector links
									// (invisible but touchable)
#define	MF_NOBLOCKMAP	0x00000010	// don't use the blocklinks
									// (inert but displayable)
#define	MF_AMBUSH		0x00000020
#define	MF_JUSTHIT		0x00000040	// try to attack right back
#define	MF_JUSTATTACKED	0x00000080	// take at least one step before attacking
#define	MF_SPAWNCEILING	0x00000100	// hang from ceiling instead of floor
#define	MF_NOGRAVITY	0x00000200	// don't apply gravity every tic

// movement flags
#define	MF_DROPOFF		0x00000400	// allow jumps from high places
#define	MF_PICKUP		0x00000800	// for players to pick up items
#define	MF_NOCLIP		0x00001000	// player cheat
#define	MF_SLIDE		0x00002000	// keep info about sliding along walls
#define	MF_FLOAT		0x00004000	// allow moves to any height, no gravity
#define	MF_TELEPORT		0x00008000	// don't cross lines or look at heights
#define MF_MISSILE		0x00010000	// don't hit same species, explode on block

#define	MF_ALTSHADOW	0x00020000	// alternate fuzzy draw
#define	MF_DROPPED		0x00020000	// dropped by a demon, not level spawned
#define	MF_SHADOW		0x00040000	// use fuzzy draw (shadow demons / invis)
#define	MF_NOBLOOD		0x00080000	// don't bleed when shot (use puff)
#define	MF_CORPSE		0x00100000	// don't stop moving halfway off a step
#define	MF_INFLOAT		0x00200000	// floating to a height for a move, don't
									// auto float to target's height

#define	MF_COUNTKILL	0x00400000	// count towards intermission kill total
#define	MF_COUNTITEM	0x00800000	// count towards intermission item total
#define	MF_ICECORPSE	0x00800000	// a frozen corpse (for blasting)

#define	MF_SKULLFLY		0x01000000	// skull in flight
#define	MF_NOTDMATCH	0x02000000	// don't spawn in death match (key cards)

#define	MF_TRANSLATION	0x0c000000	// if 0x4 0x8 or 0xc, use a translation
#define	MF_TRANSSHIFT	26			// table for player colourmaps

#define MF_TRANSLUCENT	0x80000000  // Translucency

typedef struct
{
	int	doomednum;
	int	spawnstate;
	int	spawnhealth;
	int	seestate;
	int	seesound;
	int	reactiontime;
	int	attacksound;
	int	painstate;
	int	painchance;
	int	painsound;
	int	meleestate;
	int	missilestate;
	int	crashstate;
	int	deathstate;
	int	xdeathstate;
	int	deathsound;
	int	speed;
	int	radius;
	int	height;
	int	mass;
	int	damage;
	int	activesound;
	int	flags;
	int	flags2;
	int	raisestate;
} mobjinfo_t;

extern bool					Doom2;

extern const char*			sfx[NUMSFX];
