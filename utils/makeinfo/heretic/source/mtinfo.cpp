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

#include "../../makeinfo.h"
#include "info.h"

#define AMMO_GWND_WIMPY 10
#define AMMO_GWND_HEFTY 50
#define AMMO_CBOW_WIMPY 5
#define AMMO_CBOW_HEFTY 20
#define AMMO_BLSR_WIMPY 10
#define AMMO_BLSR_HEFTY 25
#define AMMO_SKRD_WIMPY 20
#define AMMO_SKRD_HEFTY 100
#define AMMO_PHRD_WIMPY 1
#define AMMO_PHRD_HEFTY 10
#define AMMO_MACE_WIMPY 20
#define AMMO_MACE_HEFTY 100

int		nummobjtypes = NUMHERETICMOBJTYPES;

mobjinfo_t mobjinfo[NUMHERETICMOBJTYPES] = {

{		// MT_MISC0
81,		// doomednum
S2_ITEM_PTN1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_ITEMSHIELD1
85,		// doomednum
S2_ITEM_SHLD1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_ITEMSHIELD2
31,		// doomednum
S2_ITEM_SHD2_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_MISC1
8,		// doomednum
S2_ITEM_BAGH1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_COUNTITEM,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_MISC2
35,		// doomednum
S2_ITEM_SPMP1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_COUNTITEM,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_ARTIINVISIBILITY
75,		// doomednum
S2_ARTI_INVS1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_SHADOW|MF_COUNTITEM,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_MISC3
82,		// doomednum
S2_ARTI_PTN2_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_COUNTITEM,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_ARTIFLY
83,		// doomednum
S2_ARTI_SOAR1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_COUNTITEM,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_ARTIINVULNERABILITY
84,		// doomednum
S2_ARTI_INVU1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_COUNTITEM,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_ARTITOMEOFPOWER
86,		// doomednum
S2_ARTI_PWBK1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_COUNTITEM,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_ARTIEGG
30,		// doomednum
S2_ARTI_EGGC1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_COUNTITEM,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_EGGFX
-1,		// doomednum
S2_EGGFX1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_EGGFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
18*FRACUNIT,		// speed
8*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
1,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT		// flags2
 },

{		// MT_ARTISUPERHEAL
32,		// doomednum
S2_ARTI_SPHL1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_COUNTITEM,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_MISC4
33,		// doomednum
S2_ARTI_TRCH1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_COUNTITEM,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_MISC5
34,		// doomednum
S2_ARTI_FBMB1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_COUNTITEM,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_FIREBOMB
-1,		// doomednum
S2_FIREBOMB1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_phohit,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOGRAVITY|MF_SHADOW,		// flags
0		// flags2
 },

{		// MT_ARTITELEPORT
36,		// doomednum
S2_ARTI_ATLP1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_COUNTITEM,		// flags
MF2_FLOATBOB		// flags2
 },

{		// MT_POD
2035,		// doomednum
S2_POD_WAIT1,		// spawnstate
45,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_POD_PAIN1,		// painstate
255,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_POD_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_podexp,		// deathsound
0,		// speed
16*FRACUNIT,		// radius
54*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID|MF_NOBLOOD|MF_SHOOTABLE|MF_DROPOFF,		// flags
MF2_WINDTHRUST|MF2_PUSHABLE|MF2_SLIDE|MF2_PASSMOBJ|MF2_TELESTOMP		// flags2
 },

{		// MT_PODGOO
-1,		// doomednum
S2_PODGOO1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_PODGOOX,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
2*FRACUNIT,		// radius
4*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF,		// flags
MF2_NOTELEPORT|MF2_LOGRAV|MF2_CANNOTPUSH		// flags2
 },

{		// MT_PODGENERATOR
43,		// doomednum
S2_PODGENERATOR,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOSECTOR,		// flags
0		// flags2
 },

{		// MT_SPLASH
-1,		// doomednum
S2_SPLASH1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_SPLASHX,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
2*FRACUNIT,		// radius
4*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF,		// flags
MF2_NOTELEPORT|MF2_LOGRAV|MF2_CANNOTPUSH		// flags2
 },

{		// MT_SPLASHBASE
-1,		// doomednum
S2_SPLASHBASE1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP,		// flags
0		// flags2
 },

{		// MT_LAVASPLASH
-1,		// doomednum
S2_LAVASPLASH1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP,		// flags
0		// flags2
 },

{		// MT_LAVASMOKE
-1,		// doomednum
S2_LAVASMOKE1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY|MF_SHADOW,		// flags
0		// flags2
 },

{		// MT_SLUDGECHUNK
-1,		// doomednum
S2_SLUDGECHUNK1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_SLUDGECHUNKX,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
2*FRACUNIT,		// radius
4*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF,		// flags
MF2_NOTELEPORT|MF2_LOGRAV|MF2_CANNOTPUSH		// flags2
 },

{		// MT_SLUDGESPLASH
-1,		// doomednum
S2_SLUDGESPLASH1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP,		// flags
0		// flags2
 },

{		// MT_SKULLHANG70
17,		// doomednum
S2_SKULLHANG70_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
70*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_SKULLHANG60
24,		// doomednum
S2_SKULLHANG60_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
60*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_SKULLHANG45
25,		// doomednum
S2_SKULLHANG45_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
45*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_SKULLHANG35
26,		// doomednum
S2_SKULLHANG35_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
35*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_CHANDELIER
28,		// doomednum
S2_CHANDELIER1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
60*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
0,		// flags2
0,
0,
0,
"AddStaticLight(origin,\n"
"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 200.0);"
 },

{		// MT_SERPTORCH
27,		// doomednum
S2_SERPTORCH1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
12*FRACUNIT,		// radius
54*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID,		// flags
0,		// flags2
0,
0,
0,
"AddStaticLight(origin + vector(0.0, 0.0, 32.0),\n"
"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 128.0);"
 },

{		// MT_SMALLPILLAR
29,		// doomednum
S2_SMALLPILLAR,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
16*FRACUNIT,		// radius
34*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID,		// flags
0		// flags2
 },

{		// MT_STALAGMITESMALL
37,		// doomednum
S2_STALAGMITESMALL,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
8*FRACUNIT,		// radius
32*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID,		// flags
0		// flags2
 },

{		// MT_STALAGMITELARGE
38,		// doomednum
S2_STALAGMITELARGE,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
12*FRACUNIT,		// radius
64*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID,		// flags
0		// flags2
 },

{		// MT_STALACTITESMALL
39,		// doomednum
S2_STALACTITESMALL,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
8*FRACUNIT,		// radius
36*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_STALACTITELARGE
40,		// doomednum
S2_STALACTITELARGE,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
12*FRACUNIT,		// radius
68*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_MISC6
76,		// doomednum
S2_FIREBRAZIER1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
16*FRACUNIT,		// radius
44*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID,		// flags
0,		// flags2
0,
0,
0,
"AddStaticLight(origin + vector(0.0, 0.0, 32.0),\n"
"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 200.0);"
 },

{		// MT_BARREL
44,		// doomednum
S2_BARREL,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
12*FRACUNIT,		// radius
32*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID,		// flags
0		// flags2
 },

{		// MT_MISC7
47,		// doomednum
S2_BRPILLAR,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
14*FRACUNIT,		// radius
128*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID,		// flags
0		// flags2
 },

{		// MT_MISC8
48,		// doomednum
S2_MOSS1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
23*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_MISC9
49,		// doomednum
S2_MOSS2,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
27*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_MISC10
50,		// doomednum
S2_WALLTORCH1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOGRAVITY,		// flags
0,		// flags2
0,
0,
0,
"AddStaticLight(origin + vector(0.0, 0.0, 80.0),\n"
"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 200.0);"
 },

{		// MT_MISC11
51,		// doomednum
S2_HANGINGCORPSE,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
8*FRACUNIT,		// radius
104*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_KEYGIZMOBLUE
94,		// doomednum
S2_KEYGIZMO1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
16*FRACUNIT,		// radius
50*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID,		// flags
0		// flags2
 },

{		// MT_KEYGIZMOGREEN
95,		// doomednum
S2_KEYGIZMO1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
16*FRACUNIT,		// radius
50*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID,		// flags
0		// flags2
 },

{		// MT_KEYGIZMOYELLOW
96,		// doomednum
S2_KEYGIZMO1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
16*FRACUNIT,		// radius
50*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID,		// flags
0		// flags2
 },

{		// MT_KEYGIZMOFLOAT
-1,		// doomednum
S2_KGZ_START,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
16*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_MISC12
87,		// doomednum
S2_VOLCANO1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
12*FRACUNIT,		// radius
20*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID,		// flags
0		// flags2
 },

{		// MT_VOLCANOBLAST
-1,		// doomednum
S2_VOLCANOBALL1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_VOLCANOBALLX1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_volhit,		// deathsound
2*FRACUNIT,		// speed
8*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
2,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF,		// flags
MF2_LOGRAV|MF2_NOTELEPORT|MF2_FIREDAMAGE,		// flags2
0,
0,
"EF_DL_RED"
 },

{		// MT_VOLCANOTBLAST
-1,		// doomednum
S2_VOLCANOTBALL1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_VOLCANOTBALLX1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
2*FRACUNIT,		// speed
8*FRACUNIT,		// radius
6*FRACUNIT,		// height
100,		// mass
1,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF,		// flags
MF2_LOGRAV|MF2_NOTELEPORT|MF2_FIREDAMAGE,		// flags2
0,
0,
"EF_DL_RED"
 },

{		// MT_TELEGLITGEN
74,		// doomednum
S2_TELEGLITGEN1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY|MF_NOSECTOR,		// flags
0		// flags2
 },

{		// MT_TELEGLITGEN2
52,		// doomednum
S2_TELEGLITGEN2,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY|MF_NOSECTOR,		// flags
0		// flags2
 },

{		// MT_TELEGLITTER
-1,		// doomednum
S2_TELEGLITTER1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY|MF_MISSILE,		// flags
0		// flags2
 },

{		// MT_TELEGLITTER2
-1,		// doomednum
S2_TELEGLITTER2_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY|MF_MISSILE,		// flags
0		// flags2
 },

{		// MT_TFOG
-1,		// doomednum
S2_TFOG1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_TELEPORTMAN
14,		// doomednum
S2_NULL,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOSECTOR,		// flags
0		// flags2
 },

{		// MT_STAFFPUFF
-1,		// doomednum
S2_STAFFPUFF1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_stfhit,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_STAFFPUFF2
-1,		// doomednum
S2_STAFFPUFF2_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_stfpow,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_BEAKPUFF
-1,		// doomednum
S2_STAFFPUFF1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_chicatk,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_MISC13
2005,		// doomednum
S2_WGNT,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_GAUNTLETPUFF1
-1,		// doomednum
S2_GAUNTLETPUFF1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY|MF_SHADOW,		// flags
0		// flags2
 },

{		// MT_GAUNTLETPUFF2
-1,		// doomednum
S2_GAUNTLETPUFF2_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY|MF_SHADOW,		// flags
0		// flags2
 },

{		// MT_MISC14
53,		// doomednum
S2_BLSR,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_BLASTERFX1
-1,		// doomednum
S2_BLASTERFX1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_BLASTERFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_blshit,		// deathsound
184*FRACUNIT,		// speed
12*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
2,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_BLUE"
 },

{		// MT_BLASTERSMOKE
-1,		// doomednum
S2_BLASTERSMOKE1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY|MF_SHADOW,		// flags
MF2_NOTELEPORT|MF2_CANNOTPUSH		// flags2
 },

{		// MT_RIPPER
-1,		// doomednum
S2_RIPPER1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_RIPPERX1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_hrnhit,		// deathsound
14*FRACUNIT,		// speed
8*FRACUNIT,		// radius
6*FRACUNIT,		// height
100,		// mass
1,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT|MF2_RIP		// flags2
 },

{		// MT_BLASTERPUFF1
-1,		// doomednum
S2_BLASTERPUFF1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_BLASTERPUFF2
-1,		// doomednum
S2_BLASTERPUFF2_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_WMACE
2002,		// doomednum
S2_WMCE,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_MACEFX1
-1,		// doomednum
S2_MACEFX1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_lobsht,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_MACEFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
20*FRACUNIT,		// speed
8*FRACUNIT,		// radius
6*FRACUNIT,		// height
100,		// mass
2,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_FLOORBOUNCE|MF2_THRUGHOST|MF2_NOTELEPORT		// flags2
 },

{		// MT_MACEFX2
-1,		// doomednum
S2_MACEFX2_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_MACEFXI2_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
10*FRACUNIT,		// speed
8*FRACUNIT,		// radius
6*FRACUNIT,		// height
100,		// mass
6,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF,		// flags
MF2_LOGRAV|MF2_FLOORBOUNCE|MF2_THRUGHOST|MF2_NOTELEPORT		// flags2
 },

{		// MT_MACEFX3
-1,		// doomednum
S2_MACEFX3_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_MACEFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
7*FRACUNIT,		// speed
8*FRACUNIT,		// radius
6*FRACUNIT,		// height
100,		// mass
4,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF,		// flags
MF2_LOGRAV|MF2_FLOORBOUNCE|MF2_THRUGHOST|MF2_NOTELEPORT		// flags2
 },

{		// MT_MACEFX4
-1,		// doomednum
S2_MACEFX4_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_MACEFXI4_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
7*FRACUNIT,		// speed
8*FRACUNIT,		// radius
6*FRACUNIT,		// height
100,		// mass
18,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF,		// flags
MF2_LOGRAV|MF2_FLOORBOUNCE|MF2_THRUGHOST|MF2_TELESTOMP		// flags2
 },

{		// MT_WSKULLROD
2004,		// doomednum
S2_WSKL,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_HORNRODFX1
-1,		// doomednum
S2_HRODFX1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_hrnsht,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_HRODFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_hrnhit,		// deathsound
22*FRACUNIT,		// speed
12*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
3,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_WINDTHRUST|MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_RED"
 },

{		// MT_HORNRODFX2
-1,		// doomednum
S2_HRODFX2_1,		// spawnstate
4*35,		// spawnhealth
S2_NULL,		// seestate
sfx2_hrnsht,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_HRODFXI2_1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_ramphit,		// deathsound
22*FRACUNIT,		// speed
12*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
10,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_RED"
 },

{		// MT_RAINPLR1
-1,		// doomednum
S2_RAINPLR1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_RAINPLR1X_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
12*FRACUNIT,		// speed
5*FRACUNIT,		// radius
12*FRACUNIT,		// height
100,		// mass
5,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT		// flags2
 },

{		// MT_RAINPLR2
-1,		// doomednum
S2_RAINPLR2_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_RAINPLR2X_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
12*FRACUNIT,		// speed
5*FRACUNIT,		// radius
12*FRACUNIT,		// height
100,		// mass
5,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT		// flags2
 },

{		// MT_RAINPLR3
-1,		// doomednum
S2_RAINPLR3_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_RAINPLR3X_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
12*FRACUNIT,		// speed
5*FRACUNIT,		// radius
12*FRACUNIT,		// height
100,		// mass
5,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT		// flags2
 },

{		// MT_RAINPLR4
-1,		// doomednum
S2_RAINPLR4_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_RAINPLR4X_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
12*FRACUNIT,		// speed
5*FRACUNIT,		// radius
12*FRACUNIT,		// height
100,		// mass
5,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT		// flags2
 },

{		// MT_GOLDWANDFX1
-1,		// doomednum
S2_GWANDFX1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_GWANDFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_gldhit,		// deathsound
22*FRACUNIT,		// speed
10*FRACUNIT,		// radius
6*FRACUNIT,		// height
100,		// mass
2,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT,	// flags2
0,
0,
"EF_DL_WHITE"
 },

{		// MT_GOLDWANDFX2
-1,		// doomednum
S2_GWANDFX2_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_GWANDFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
18*FRACUNIT,		// speed
10*FRACUNIT,		// radius
6*FRACUNIT,		// height
100,		// mass
1,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_WHITE"
 },

{		// MT_GOLDWANDPUFF1
-1,		// doomednum
S2_GWANDPUFF1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_GOLDWANDPUFF2
-1,		// doomednum
S2_GWANDFXI1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_WPHOENIXROD
2003,		// doomednum
S2_WPHX,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_PHOENIXFX1
-1,		// doomednum
S2_PHOENIXFX1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_phosht,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_PHOENIXFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_phohit,		// deathsound
20*FRACUNIT,		// speed
11*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
20,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_THRUGHOST|MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_RED"
 },

{		// MT_PHOENIXPUFF
-1,		// doomednum
S2_PHOENIXPUFF1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY|MF_SHADOW,		// flags
MF2_NOTELEPORT|MF2_CANNOTPUSH		// flags2
 },

{		// MT_PHOENIXFX2
-1,		// doomednum
S2_PHOENIXFX2_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_PHOENIXFXI2_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
10*FRACUNIT,		// speed
6*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
2,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT|MF2_FIREDAMAGE,		// flags2
0,
0,
"EF_DL_WHITE"
 },

{		// MT_MISC15
2001,		// doomednum
S2_WBOW,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_CRBOWFX1
-1,		// doomednum
S2_CRBOWFX1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_bowsht,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_CRBOWFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_hrnhit,		// deathsound
30*FRACUNIT,		// speed
11*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
10,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT		// flags2
 },

{		// MT_CRBOWFX2
-1,		// doomednum
S2_CRBOWFX2,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_bowsht,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_CRBOWFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_hrnhit,		// deathsound
32*FRACUNIT,		// speed
11*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
6,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_GREEN"
 },

{		// MT_CRBOWFX3
-1,		// doomednum
S2_CRBOWFX3,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_CRBOWFXI3_1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_hrnhit,		// deathsound
20*FRACUNIT,		// speed
11*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
2,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_WINDTHRUST|MF2_THRUGHOST|MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_GREEN"
 },

{		// MT_CRBOWFX4
-1,		// doomednum
S2_CRBOWFX4_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP,		// flags
MF2_LOGRAV		// flags2
 },

{		// MT_BLOOD
-1,		// doomednum
S2_BLOOD1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP,		// flags
0		// flags2
 },

{		// MT_BLOODSPLATTER
-1,		// doomednum
S2_BLOODSPLATTER1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_BLOODSPLATTERX,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
2*FRACUNIT,		// radius
4*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF,		// flags
MF2_NOTELEPORT|MF2_CANNOTPUSH		// flags2
 },

{		// MT_PLAYER
-1,		// doomednum
S2_PLAY,		// spawnstate
100,		// spawnhealth
S2_PLAY_RUN1,		// seestate
sfx2_None,		// seesound
0,		// reactiontime
sfx2_None,		// attacksound
S2_PLAY_PAIN,		// painstate
255,		// painchance
sfx2_plrpai,		// painsound
S2_NULL,		// meleestate
S2_PLAY_ATK1,		// missilestate
S2_NULL,		// crashstate
S2_PLAY_DIE1,		// deathstate
S2_PLAY_XDIE1,		// xdeathstate
sfx2_plrdth,		// deathsound
0,		// speed
16*FRACUNIT,		// radius
56*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_DROPOFF|MF_PICKUP|MF_NOTDMATCH,		// flags
MF2_WINDTHRUST|MF2_FOOTCLIP|MF2_SLIDE|MF2_PASSMOBJ|MF2_TELESTOMP		// flags2
 },

{		// MT_BLOODYSKULL
-1,		// doomednum
S2_BLOODYSKULL1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
4*FRACUNIT,		// radius
4*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_DROPOFF,		// flags
MF2_LOGRAV|MF2_CANNOTPUSH		// flags2
 },

{		// MT_CHICPLAYER
-1,		// doomednum
S2_CHICPLAY,		// spawnstate
100,		// spawnhealth
S2_CHICPLAY_RUN1,		// seestate
sfx2_None,		// seesound
0,		// reactiontime
sfx2_None,		// attacksound
S2_CHICPLAY_PAIN,		// painstate
255,		// painchance
sfx2_chicpai,		// painsound
S2_NULL,		// meleestate
S2_CHICPLAY_ATK1,		// missilestate
S2_NULL,		// crashstate
S2_CHICKEN_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_chicdth,		// deathsound
0,		// speed
16*FRACUNIT,		// radius
24*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_DROPOFF|MF_NOTDMATCH,		// flags
MF2_WINDTHRUST|MF2_SLIDE|MF2_PASSMOBJ|MF2_FOOTCLIP|MF2_LOGRAV|MF2_TELESTOMP		// flags2
 },

{		// MT_CHICKEN
-1,		// doomednum
S2_CHICKEN_LOOK1,		// spawnstate
10,		// spawnhealth
S2_CHICKEN_WALK1,		// seestate
sfx2_chicpai,		// seesound
8,		// reactiontime
sfx2_chicatk,		// attacksound
S2_CHICKEN_PAIN1,		// painstate
200,		// painchance
sfx2_chicpai,		// painsound
S2_CHICKEN_ATK1,		// meleestate
0,		// missilestate
S2_NULL,		// crashstate
S2_CHICKEN_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_chicdth,		// deathsound
4,		// speed
9*FRACUNIT,		// radius
22*FRACUNIT,		// height
40,		// mass
0,		// damage
sfx2_chicact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL|MF_DROPOFF,		// flags
MF2_WINDTHRUST|MF2_FOOTCLIP|MF2_PASSMOBJ		// flags2
 },

{		// MT_FEATHER
-1,		// doomednum
S2_FEATHER1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_FEATHERX,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
2*FRACUNIT,		// radius
4*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF,		// flags
MF2_NOTELEPORT|MF2_LOGRAV|MF2_CANNOTPUSH|MF2_WINDTHRUST		// flags2
 },

{		// MT_MUMMY
68,		// doomednum
S2_MUMMY_LOOK1,		// spawnstate
80,		// spawnhealth
S2_MUMMY_WALK1,		// seestate
sfx2_mumsit,		// seesound
8,		// reactiontime
sfx2_mumat1,		// attacksound
S2_MUMMY_PAIN1,		// painstate
128,		// painchance
sfx2_mumpai,		// painsound
S2_MUMMY_ATK1,		// meleestate
0,		// missilestate
S2_NULL,		// crashstate
S2_MUMMY_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_mumdth,		// deathsound
12,		// speed
22*FRACUNIT,		// radius
62*FRACUNIT,		// height
75,		// mass
0,		// damage
sfx2_mumact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
MF2_FOOTCLIP|MF2_PASSMOBJ		// flags2
 },

{		// MT_MUMMYLEADER
45,		// doomednum
S2_MUMMY_LOOK1,		// spawnstate
100,		// spawnhealth
S2_MUMMY_WALK1,		// seestate
sfx2_mumsit,		// seesound
8,		// reactiontime
sfx2_mumat1,		// attacksound
S2_MUMMY_PAIN1,		// painstate
64,		// painchance
sfx2_mumpai,		// painsound
S2_MUMMY_ATK1,		// meleestate
S2_MUMMYL_ATK1,		// missilestate
S2_NULL,		// crashstate
S2_MUMMY_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_mumdth,		// deathsound
12,		// speed
22*FRACUNIT,		// radius
62*FRACUNIT,		// height
75,		// mass
0,		// damage
sfx2_mumact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
MF2_FOOTCLIP|MF2_PASSMOBJ		// flags2
 },

{		// MT_MUMMYGHOST
69,		// doomednum
S2_MUMMY_LOOK1,		// spawnstate
80,		// spawnhealth
S2_MUMMY_WALK1,		// seestate
sfx2_mumsit,		// seesound
8,		// reactiontime
sfx2_mumat1,		// attacksound
S2_MUMMY_PAIN1,		// painstate
128,		// painchance
sfx2_mumpai,		// painsound
S2_MUMMY_ATK1,		// meleestate
0,		// missilestate
S2_NULL,		// crashstate
S2_MUMMY_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_mumdth,		// deathsound
12,		// speed
22*FRACUNIT,		// radius
62*FRACUNIT,		// height
75,		// mass
0,		// damage
sfx2_mumact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL|MF_SHADOW,		// flags
MF2_FOOTCLIP|MF2_PASSMOBJ		// flags2
 },

{		// MT_MUMMYLEADERGHOST
46,		// doomednum
S2_MUMMY_LOOK1,		// spawnstate
100,		// spawnhealth
S2_MUMMY_WALK1,		// seestate
sfx2_mumsit,		// seesound
8,		// reactiontime
sfx2_mumat1,		// attacksound
S2_MUMMY_PAIN1,		// painstate
64,		// painchance
sfx2_mumpai,		// painsound
S2_MUMMY_ATK1,		// meleestate
S2_MUMMYL_ATK1,		// missilestate
S2_NULL,		// crashstate
S2_MUMMY_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_mumdth,		// deathsound
12,		// speed
22*FRACUNIT,		// radius
62*FRACUNIT,		// height
75,		// mass
0,		// damage
sfx2_mumact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL|MF_SHADOW,		// flags
MF2_FOOTCLIP|MF2_PASSMOBJ		// flags2
 },

{		// MT_MUMMYSOUL
-1,		// doomednum
S2_MUMMY_SOUL1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
0		// flags2
 },

{		// MT_MUMMYFX1
-1,		// doomednum
S2_MUMMYFX1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_MUMMYFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
9*FRACUNIT,		// speed
8*FRACUNIT,		// radius
14*FRACUNIT,		// height
100,		// mass
4,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_WHITE"
 },

{		// MT_BEAST
70,		// doomednum
S2_BEAST_LOOK1,		// spawnstate
220,		// spawnhealth
S2_BEAST_WALK1,		// seestate
sfx2_bstsit,		// seesound
8,		// reactiontime
sfx2_bstatk,		// attacksound
S2_BEAST_PAIN1,		// painstate
100,		// painchance
sfx2_bstpai,		// painsound
0,		// meleestate
S2_BEAST_ATK1,		// missilestate
S2_NULL,		// crashstate
S2_BEAST_DIE1,		// deathstate
S2_BEAST_XDIE1,		// xdeathstate
sfx2_bstdth,		// deathsound
14,		// speed
32*FRACUNIT,		// radius
74*FRACUNIT,		// height
200,		// mass
0,		// damage
sfx2_bstact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
MF2_FOOTCLIP|MF2_PASSMOBJ		// flags2
 },

{		// MT_BEASTBALL
-1,		// doomednum
S2_BEASTBALL1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_BEASTBALLX1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
12*FRACUNIT,		// speed
9*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
4,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_WINDTHRUST|MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_RED"
 },

{		// MT_BURNBALL
-1,		// doomednum
S2_BURNBALL1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_BEASTBALLX1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
10*FRACUNIT,		// speed
6*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
2,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY|MF_MISSILE,		// flags
MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_RED"
 },

{		// MT_BURNBALLFB
-1,		// doomednum
S2_BURNBALLFB1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_BEASTBALLX1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
10*FRACUNIT,		// speed
6*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
2,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY|MF_MISSILE,		// flags
MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_RED"
 },

{		// MT_PUFFY
-1,		// doomednum
S2_PUFFY1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_PUFFY1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
10*FRACUNIT,		// speed
6*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
2,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY|MF_MISSILE,		// flags
MF2_NOTELEPORT		// flags2
 },

{		// MT_SNAKE
92,		// doomednum
S2_SNAKE_LOOK1,		// spawnstate
280,		// spawnhealth
S2_SNAKE_WALK1,		// seestate
sfx2_snksit,		// seesound
8,		// reactiontime
sfx2_snkatk,		// attacksound
S2_SNAKE_PAIN1,		// painstate
48,		// painchance
sfx2_snkpai,		// painsound
0,		// meleestate
S2_SNAKE_ATK1,		// missilestate
S2_NULL,		// crashstate
S2_SNAKE_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_snkdth,		// deathsound
10,		// speed
22*FRACUNIT,		// radius
70*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_snkact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
MF2_FOOTCLIP|MF2_PASSMOBJ		// flags2
 },

{		// MT_SNAKEPRO_A
-1,		// doomednum
S2_SNAKEPRO_A1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_SNAKEPRO_AX1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
14*FRACUNIT,		// speed
12*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
1,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_WINDTHRUST|MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_MAGENTA"
 },

{		// MT_SNAKEPRO_B
-1,		// doomednum
S2_SNAKEPRO_B1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_SNAKEPRO_BX1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
14*FRACUNIT,		// speed
12*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
3,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_WHITE"
 },

{		// MT_HEAD
6,		// doomednum
S2_HEAD_LOOK,		// spawnstate
700,		// spawnhealth
S2_HEAD_FLOAT,		// seestate
sfx2_hedsit,		// seesound
8,		// reactiontime
sfx2_hedat1,		// attacksound
S2_HEAD_PAIN1,		// painstate
32,		// painchance
sfx2_hedpai,		// painsound
0,		// meleestate
S2_HEAD_ATK1,		// missilestate
S2_NULL,		// crashstate
S2_HEAD_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_heddth,		// deathsound
6,		// speed
40*FRACUNIT,		// radius
72*FRACUNIT,		// height
325,		// mass
0,		// damage
sfx2_hedact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL|MF_NOBLOOD,		// flags
MF2_PASSMOBJ		// flags2
 },

{		// MT_HEADFX1
-1,		// doomednum
S2_HEADFX1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_HEADFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
13*FRACUNIT,		// speed
12*FRACUNIT,		// radius
6*FRACUNIT,		// height
100,		// mass
1,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT|MF2_THRUGHOST,		// flags2
0,
0,
"EF_DL_BLUE"
 },

{		// MT_HEADFX2
-1,		// doomednum
S2_HEADFX2_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_HEADFXI2_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
8*FRACUNIT,		// speed
12*FRACUNIT,		// radius
6*FRACUNIT,		// height
100,		// mass
3,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_BLUE"
 },

{		// MT_HEADFX3
-1,		// doomednum
S2_HEADFX3_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_HEADFXI3_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
10*FRACUNIT,		// speed
14*FRACUNIT,		// radius
12*FRACUNIT,		// height
100,		// mass
5,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_WINDTHRUST|MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_WHITE"
 },

{		// MT_WHIRLWIND
-1,		// doomednum
S2_HEADFX4_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_HEADFXI4_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
10*FRACUNIT,		// speed
16*FRACUNIT,		// radius
74*FRACUNIT,		// height
100,		// mass
1,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY|MF_SHADOW,		// flags
MF2_NOTELEPORT		// flags2
 },

{		// MT_CLINK
90,		// doomednum
S2_CLINK_LOOK1,		// spawnstate
150,		// spawnhealth
S2_CLINK_WALK1,		// seestate
sfx2_clksit,		// seesound
8,		// reactiontime
sfx2_clkatk,		// attacksound
S2_CLINK_PAIN1,		// painstate
32,		// painchance
sfx2_clkpai,		// painsound
S2_CLINK_ATK1,		// meleestate
0,		// missilestate
S2_NULL,		// crashstate
S2_CLINK_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_clkdth,		// deathsound
14,		// speed
20*FRACUNIT,		// radius
64*FRACUNIT,		// height
75,		// mass
0,		// damage
sfx2_clkact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL|MF_NOBLOOD,		// flags
MF2_FOOTCLIP|MF2_PASSMOBJ		// flags2
 },

{		// MT_WIZARD
15,		// doomednum
S2_WIZARD_LOOK1,		// spawnstate
180,		// spawnhealth
S2_WIZARD_WALK1,		// seestate
sfx2_wizsit,		// seesound
8,		// reactiontime
sfx2_wizatk,		// attacksound
S2_WIZARD_PAIN1,		// painstate
64,		// painchance
sfx2_wizpai,		// painsound
0,		// meleestate
S2_WIZARD_ATK1,		// missilestate
S2_NULL,		// crashstate
S2_WIZARD_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_wizdth,		// deathsound
12,		// speed
16*FRACUNIT,		// radius
68*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_wizact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL|MF_FLOAT|MF_NOGRAVITY,		// flags
MF2_PASSMOBJ		// flags2
 },

{		// MT_WIZFX1
-1,		// doomednum
S2_WIZFX1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_WIZFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
18*FRACUNIT,		// speed
10*FRACUNIT,		// radius
6*FRACUNIT,		// height
100,		// mass
3,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_MAGENTA"
 },

{		// MT_IMP
66,		// doomednum
S2_IMP_LOOK1,		// spawnstate
40,		// spawnhealth
S2_IMP_FLY1,		// seestate
sfx2_impsit,		// seesound
8,		// reactiontime
sfx2_impat1,		// attacksound
S2_IMP_PAIN1,		// painstate
200,		// painchance
sfx2_imppai,		// painsound
S2_IMP_MEATK1,		// meleestate
S2_IMP_MSATK1_1,		// missilestate
S2_IMP_CRASH1,		// crashstate
S2_IMP_DIE1,		// deathstate
S2_IMP_XDIE1,		// xdeathstate
sfx2_impdth,		// deathsound
10,		// speed
16*FRACUNIT,		// radius
36*FRACUNIT,		// height
50,		// mass
0,		// damage
sfx2_impact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_FLOAT|MF_NOGRAVITY|MF_COUNTKILL,		// flags
MF2_SPAWNFLOAT|MF2_PASSMOBJ		// flags2
 },

{		// MT_IMPLEADER
5,		// doomednum
S2_IMP_LOOK1,		// spawnstate
80,		// spawnhealth
S2_IMP_FLY1,		// seestate
sfx2_impsit,		// seesound
8,		// reactiontime
sfx2_impat2,		// attacksound
S2_IMP_PAIN1,		// painstate
200,		// painchance
sfx2_imppai,		// painsound
0,		// meleestate
S2_IMP_MSATK2_1,		// missilestate
S2_IMP_CRASH1,		// crashstate
S2_IMP_DIE1,		// deathstate
S2_IMP_XDIE1,		// xdeathstate
sfx2_impdth,		// deathsound
10,		// speed
16*FRACUNIT,		// radius
36*FRACUNIT,		// height
50,		// mass
0,		// damage
sfx2_impact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_FLOAT|MF_NOGRAVITY|MF_COUNTKILL,		// flags
MF2_SPAWNFLOAT|MF2_PASSMOBJ		// flags2
 },

{		// MT_IMPCHUNK1
-1,		// doomednum
S2_IMP_CHUNKA1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP,		// flags
0		// flags2
 },

{		// MT_IMPCHUNK2
-1,		// doomednum
S2_IMP_CHUNKB1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP,		// flags
0		// flags2
 },

{		// MT_IMPBALL
-1,		// doomednum
S2_IMPFX1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_IMPFXI1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
10*FRACUNIT,		// speed
8*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
1,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_WINDTHRUST|MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_WHITE"
 },

{		// MT_KNIGHT
64,		// doomednum
S2_KNIGHT_STND1,		// spawnstate
200,		// spawnhealth
S2_KNIGHT_WALK1,		// seestate
sfx2_kgtsit,		// seesound
8,		// reactiontime
sfx2_kgtatk,		// attacksound
S2_KNIGHT_PAIN1,		// painstate
100,		// painchance
sfx2_kgtpai,		// painsound
S2_KNIGHT_ATK1,		// meleestate
S2_KNIGHT_ATK1,		// missilestate
S2_NULL,		// crashstate
S2_KNIGHT_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_kgtdth,		// deathsound
12,		// speed
24*FRACUNIT,		// radius
78*FRACUNIT,		// height
150,		// mass
0,		// damage
sfx2_kgtact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
MF2_FOOTCLIP|MF2_PASSMOBJ		// flags2
 },

{		// MT_KNIGHTGHOST
65,		// doomednum
S2_KNIGHT_STND1,		// spawnstate
200,		// spawnhealth
S2_KNIGHT_WALK1,		// seestate
sfx2_kgtsit,		// seesound
8,		// reactiontime
sfx2_kgtatk,		// attacksound
S2_KNIGHT_PAIN1,		// painstate
100,		// painchance
sfx2_kgtpai,		// painsound
S2_KNIGHT_ATK1,		// meleestate
S2_KNIGHT_ATK1,		// missilestate
S2_NULL,		// crashstate
S2_KNIGHT_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_kgtdth,		// deathsound
12,		// speed
24*FRACUNIT,		// radius
78*FRACUNIT,		// height
150,		// mass
0,		// damage
sfx2_kgtact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL|MF_SHADOW,		// flags
MF2_FOOTCLIP|MF2_PASSMOBJ		// flags2
 },

{		// MT_KNIGHTAXE
-1,		// doomednum
S2_SPINAXE1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_SPINAXEX1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_hrnhit,		// deathsound
9*FRACUNIT,		// speed
10*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
2,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_WINDTHRUST|MF2_NOTELEPORT|MF2_THRUGHOST,		// flags2
0,
0,
"EF_DL_GREEN"
 },

{		// MT_REDAXE
-1,		// doomednum
S2_REDAXE1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_REDAXEX1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_hrnhit,		// deathsound
9*FRACUNIT,		// speed
10*FRACUNIT,		// radius
8*FRACUNIT,		// height
100,		// mass
7,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT|MF2_THRUGHOST,		// flags2
0,
0,
"EF_DL_RED"
 },

{		// MT_SORCERER1
7,		// doomednum
S2_SRCR1_LOOK1,		// spawnstate
2000,		// spawnhealth
S2_SRCR1_WALK1,		// seestate
sfx2_sbtsit,		// seesound
8,		// reactiontime
sfx2_sbtatk,		// attacksound
S2_SRCR1_PAIN1,		// painstate
56,		// painchance
sfx2_sbtpai,		// painsound
0,		// meleestate
S2_SRCR1_ATK1,		// missilestate
S2_NULL,		// crashstate
S2_SRCR1_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_sbtdth,		// deathsound
16,		// speed
28*FRACUNIT,		// radius
100*FRACUNIT,		// height
800,		// mass
0,		// damage
sfx2_sbtact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
MF2_FOOTCLIP|MF2_PASSMOBJ|MF2_BOSS		// flags2
 },

{		// MT_SRCRFX1
-1,		// doomednum
S2_SRCRFX1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_SRCRFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
20*FRACUNIT,		// speed
10*FRACUNIT,		// radius
10*FRACUNIT,		// height
100,		// mass
10,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT|MF2_FIREDAMAGE,		// flags2
0,
0,
"EF_DL_WHITE"
 },

{		// MT_SORCERER2
-1,		// doomednum
S2_SOR2_LOOK1,		// spawnstate
3500,		// spawnhealth
S2_SOR2_WALK1,		// seestate
sfx2_sorsit,		// seesound
8,		// reactiontime
sfx2_soratk,		// attacksound
S2_SOR2_PAIN1,		// painstate
32,		// painchance
sfx2_sorpai,		// painsound
0,		// meleestate
S2_SOR2_ATK1,		// missilestate
S2_NULL,		// crashstate
S2_SOR2_DIE1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
14,		// speed
16*FRACUNIT,		// radius
70*FRACUNIT,		// height
300,		// mass
0,		// damage
sfx2_soract,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL|MF_DROPOFF,		// flags
MF2_FOOTCLIP|MF2_PASSMOBJ|MF2_BOSS		// flags2
 },

{		// MT_SOR2FX1
-1,		// doomednum
S2_SOR2FX1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_SOR2FXI1_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
20*FRACUNIT,		// speed
10*FRACUNIT,		// radius
6*FRACUNIT,		// height
100,		// mass
1,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_BLUE"
 },

{		// MT_SOR2FXSPARK
-1,		// doomednum
S2_SOR2FXSPARK1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT|MF2_CANNOTPUSH		// flags2
 },

{		// MT_SOR2FX2
-1,		// doomednum
S2_SOR2FX2_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_SOR2FXI2_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
6*FRACUNIT,		// speed
10*FRACUNIT,		// radius
6*FRACUNIT,		// height
100,		// mass
10,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT,		// flags2
0,
0,
"EF_DL_BLUE"
 },

{		// MT_SOR2TELEFADE
-1,		// doomednum
S2_SOR2TELEFADE1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP,		// flags
0		// flags2
 },

{		// MT_MINOTAUR
9,		// doomednum
S2_MNTR_LOOK1,		// spawnstate
3000,		// spawnhealth
S2_MNTR_WALK1,		// seestate
sfx2_minsit,		// seesound
8,		// reactiontime
sfx2_minat1,		// attacksound
S2_MNTR_PAIN1,		// painstate
25,		// painchance
sfx2_minpai,		// painsound
S2_MNTR_ATK1_1,		// meleestate
S2_MNTR_ATK2_1,		// missilestate
S2_NULL,		// crashstate
S2_MNTR_DIE1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_mindth,		// deathsound
16,		// speed
28*FRACUNIT,		// radius
100*FRACUNIT,		// height
800,		// mass
7,		// damage
sfx2_minact,		// activesound
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL|MF_DROPOFF,		// flags
MF2_FOOTCLIP|MF2_PASSMOBJ|MF2_BOSS		// flags2
 },

{		// MT_MNTRFX1
-1,		// doomednum
S2_MNTRFX1_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_MNTRFXI1_1,		// deathstate
S2_NULL,		// xdeathstate
0,		// deathsound
20*FRACUNIT,		// speed
10*FRACUNIT,		// radius
6*FRACUNIT,		// height
100,		// mass
3,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT|MF2_FIREDAMAGE,		// flags2
0,
0,
"EF_DL_WHITE"
 },

{		// MT_MNTRFX2
-1,		// doomednum
S2_MNTRFX2_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_MNTRFXI2_1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_phohit,		// deathsound
14*FRACUNIT,		// speed
5*FRACUNIT,		// radius
12*FRACUNIT,		// height
100,		// mass
4,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT|MF2_FIREDAMAGE,		// flags2
0,
0,
"EF_DL_WHITE"
 },

{		// MT_MNTRFX3
-1,		// doomednum
S2_MNTRFX3_1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
0,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_MNTRFXI2_1,		// deathstate
S2_NULL,		// xdeathstate
sfx2_phohit,		// deathsound
0,		// speed
8*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
4,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
MF2_NOTELEPORT|MF2_FIREDAMAGE,		// flags2
0,
0,
"EF_DL_WHITE"
 },

{		// MT_AKYY
73,		// doomednum
S2_AKYY1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_NOTDMATCH,		// flags
0		// flags2
 },

{		// MT_BKYY
79,		// doomednum
S2_BKYY1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_NOTDMATCH,		// flags
0		// flags2
 },

{		// MT_CKEY
80,		// doomednum
S2_CKYY1,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL|MF_NOTDMATCH,		// flags
0		// flags2
 },

{		// MT_AMGWNDWIMPY
10,		// doomednum
S2_AMG1,		// spawnstate
AMMO_GWND_WIMPY,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_AMGWNDHEFTY
12,		// doomednum
S2_AMG2_1,		// spawnstate
AMMO_GWND_HEFTY,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_AMMACEWIMPY
13,		// doomednum
S2_AMM1,		// spawnstate
AMMO_MACE_WIMPY,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_AMMACEHEFTY
16,		// doomednum
S2_AMM2,		// spawnstate
AMMO_MACE_HEFTY,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_AMCBOWWIMPY
18,		// doomednum
S2_AMC1,		// spawnstate
AMMO_CBOW_WIMPY,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_AMCBOWHEFTY
19,		// doomednum
S2_AMC2_1,		// spawnstate
AMMO_CBOW_HEFTY,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_AMSKRDWIMPY
20,		// doomednum
S2_AMS1_1,		// spawnstate
AMMO_SKRD_WIMPY,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_AMSKRDHEFTY
21,		// doomednum
S2_AMS2_1,		// spawnstate
AMMO_SKRD_HEFTY,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_AMPHRDWIMPY
22,		// doomednum
S2_AMP1_1,		// spawnstate
AMMO_PHRD_WIMPY,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_AMPHRDHEFTY
23,		// doomednum
S2_AMP2_1,		// spawnstate
AMMO_PHRD_HEFTY,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_AMBLSRWIMPY
54,		// doomednum
S2_AMB1_1,		// spawnstate
AMMO_BLSR_WIMPY,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_AMBLSRHEFTY
55,		// doomednum
S2_AMB2_1,		// spawnstate
AMMO_BLSR_HEFTY,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_SPECIAL,		// flags
0		// flags2
 },

{		// MT_SOUNDWIND
42,		// doomednum
S2_SND_WIND,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOSECTOR,		// flags
0		// flags2
 },

{		// MT_SOUNDWATERFALL
41,		// doomednum
S2_SND_WATERFALL,		// spawnstate
1000,		// spawnhealth
S2_NULL,		// seestate
sfx2_None,		// seesound
8,		// reactiontime
sfx2_None,		// attacksound
S2_NULL,		// painstate
0,		// painchance
sfx2_None,		// painsound
S2_NULL,		// meleestate
S2_NULL,		// missilestate
S2_NULL,		// crashstate
S2_NULL,		// deathstate
S2_NULL,		// xdeathstate
sfx2_None,		// deathsound
0,		// speed
20*FRACUNIT,		// radius
16*FRACUNIT,		// height
100,		// mass
0,		// damage
sfx2_None,		// activesound
MF_NOBLOCKMAP|MF_NOSECTOR,		// flags
0		// flags2
 }
};

//**************************************************************************
//
//	$Log$
//	Revision 1.4  2001/09/27 17:04:39  dj_jl
//	Effects and static lights in mobjinfo, mobj classes
//
//	Revision 1.3  2001/09/20 16:34:58  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
