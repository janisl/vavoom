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

#include <stdio.h>

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
#define	MF_TRANSSHIFT	26			// table for player colormaps

#define MF_TRANSLUCENT	0x80000000  // Translucency

// --- mobj.flags2 ---

#define MF2_LOGRAV			0x00000001	// alternate gravity setting
#define MF2_WINDTHRUST		0x00000002	// gets pushed around by the wind
										// specials
#define MF2_FLOORBOUNCE		0x00000004	// bounces off the floor
#define MF2_THRUGHOST		0x00000008	// missile will pass through ghosts
#define MF2_BLASTED			0x00000008	// missile will pass through ghosts
#define MF2_FLY				0x00000010	// fly mode is active
#define MF2_FOOTCLIP		0x00000020	// if feet are allowed to be clipped
#define MF2_FLOORCLIP		0x00000020	// if feet are allowed to be clipped
#define MF2_SPAWNFLOAT		0x00000040	// spawn random float z
#define MF2_NOTELEPORT		0x00000080	// does not teleport
#define MF2_RIP				0x00000100	// missile rips through solid
										// targets
#define MF2_PUSHABLE		0x00000200	// can be pushed by other moving
										// mobjs
#define MF2_SLIDE			0x00000400	// slides against walls
#define MF2_ONMOBJ			0x00000800	// mobj is resting on top of another
										// mobj
#define MF2_PASSMOBJ		0x00001000	// Enable z block checking.  If on,
										// this flag will allow the mobj to
										// pass over/under other mobjs.
#define MF2_CANNOTPUSH		0x00002000	// cannot push other pushable mobjs
#define MF2_FEETARECLIPPED	0x00004000	// a mobj's feet are now being cut
#define MF2_DROPPED			0x00004000	// dropped by a demon
#define MF2_BOSS			0x00008000	// mobj is a major boss
#define MF2_FIREDAMAGE		0x00010000	// does fire damage
#define MF2_NODMGTHRUST		0x00020000	// does not thrust target when
										// damaging
#define MF2_TELESTOMP		0x00040000	// mobj can stomp another
#define MF2_FLOATBOB		0x00080000	// use float bobbing z movement
#define MF2_DONTDRAW		0X00100000	// don't generate a vissprite

#define MF2_IMPACT			0x00200000 	// an MF_MISSILE mobj can activate
								 		// SPAC_IMPACT
#define MF2_PUSHWALL		0x00400000 	// mobj can push walls
#define MF2_MCROSS			0x00800000	// can activate monster cross lines
#define MF2_PCROSS			0x01000000	// can activate projectile cross lines
#define MF2_CANTLEAVEFLOORPIC 0x02000000 // stay within a certain floor type
#define MF2_NONSHOOTABLE	0x04000000	// mobj is totally non-shootable, 
										// but still considered solid
#define MF2_INVULNERABLE	0x08000000	// mobj is invulnerable
#define MF2_DORMANT			0x10000000	// thing is dormant
#define MF2_ICEDAMAGE		0x20000000  // does ice damage
#define MF2_SEEKERMISSILE	0x40000000	// is a seeker (for reflection)
#define MF2_REFLECTIVE		0x80000000	// reflects missiles

typedef struct
{
	char*			fname;
    int				weapon_action;
} state_action_info_t;

typedef struct
{
	int		sprite;
	int		frame;
	int		tics;
	int		action_num;
	int		nextstate;
	int		misc1;
    int		misc2;
	char	*model_name;
	int		model_frame;
} state_t;

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
	char *classname;
} mobjinfo_t;

// Weapon info: sprite frames, ammunition use.
typedef struct
{
	int			ammo;
	int			upstate;
	int 		downstate;
	int 		readystate;
	int 		atkstate;
	int 		holdatkstate;
	int 		flashstate;
} weaponinfo_t;

//
// SoundFX struct.
//
typedef struct 
{
	char	tagName[32];	// Name, by whitch sound is recognised in script
	char	lumpname[12];	// Only need 9 bytes, but padded out to be dword aligned
	int		priority;		// Higher priority takes precendence
	int 	numchannels;	// total number of channels a sound type may occupy
	int		changePitch;
	void*	snd_ptr;
    int		lumpnum;        // lump number of sfx
} sfxinfo_t;

struct string_def_t
{
	char	*macro;
    char	*def_val;
    char	*new_val;
};

struct map_info_t
{
	char	name[32];
    char	song[12];
};

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/09/20 16:33:14  dj_jl
//	Beautification
//
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
