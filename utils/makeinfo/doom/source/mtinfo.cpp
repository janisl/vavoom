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

#include "../../makeinfo.h"
#include "info.h"

int		nummobjtypes = NUMDOOMMOBJTYPES;

mobjinfo_t mobjinfo[NUMDOOMMOBJTYPES] = {

    {		//   PlayerEntity
	-1,		// doomednum
	S1_PLAY,		// spawnstate
	100,		// spawnhealth
	S1_PLAY_RUN1,		// seestate
	sfx1_None,		// seesound
	0,		// reactiontime
	sfx1_None,		// attacksound
	S1_PLAY_PAIN,		// painstate
	255,		// painchance
	sfx1_plpain,		// painsound
	S1_NULL,		// meleestate
	S1_PLAY_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_PLAY_DIE1,		// deathstate
	S1_PLAY_XDIE1,		// xdeathstate
	sfx1_pldeth,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	56*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_DROPOFF|MF_PICKUP|MF_NOTDMATCH,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//  Zombieman
	3004,		// doomednum
	S1_POSS_STND,		// spawnstate
	20,		// spawnhealth
	S1_POSS_RUN1,		// seestate
	sfx1_posit1,		// seesound
	8,		// reactiontime
	sfx1_pistol,		// attacksound
	S1_POSS_PAIN,		// painstate
	200,		// painchance
	sfx1_popain,		// painsound
	0,		// meleestate
	S1_POSS_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_POSS_DIE1,		// deathstate
	S1_POSS_XDIE1,		// xdeathstate
	sfx1_podth1,		// deathsound
	8,		// speed
	20*FRACUNIT,		// radius
	56*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_posact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_POSS_RAISE1		// raisestate
    },

    {		// ShotgunGuy
	9,		// doomednum
	S1_SPOS_STND,		// spawnstate
	30,		// spawnhealth
	S1_SPOS_RUN1,		// seestate
	sfx1_posit2,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S1_SPOS_PAIN,		// painstate
	170,		// painchance
	sfx1_popain,		// painsound
	0,		// meleestate
	S1_SPOS_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_SPOS_DIE1,		// deathstate
	S1_SPOS_XDIE1,		// xdeathstate
	sfx1_podth2,		// deathsound
	8,		// speed
	20*FRACUNIT,		// radius
	56*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_posact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_SPOS_RAISE1		// raisestate
    },

    {		// ArchVile
	64,		// doomednum
	S1_VILE_STND,		// spawnstate
	700,		// spawnhealth
	S1_VILE_RUN1,		// seestate
	sfx1_vilsit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S1_VILE_PAIN,		// painstate
	10,		// painchance
	sfx1_vipain,		// painsound
	0,		// meleestate
	S1_VILE_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_VILE_DIE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_vildth,		// deathsound
	15,		// speed
	20*FRACUNIT,		// radius
	56*FRACUNIT,		// height
	500,		// mass
	0,		// damage
	sfx1_vilact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_NULL		// raisestate
    },

    {		//	ArchvileFire
	-1,		// doomednum
	S1_FIRE1,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    0,	// flags2
	S1_NULL,		// raisestate
	0,
	"EF_DL_MISSILE",
    },

    {		//	Revenant
	66,		// doomednum
	S1_SKEL_STND,		// spawnstate
	300,		// spawnhealth
	S1_SKEL_RUN1,		// seestate
	sfx1_skesit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S1_SKEL_PAIN,		// painstate
	100,		// painchance
	sfx1_popain,		// painsound
	S1_SKEL_FIST1,		// meleestate
	S1_SKEL_MISS1,		// missilestate
	S1_NULL,		// crashstate
	S1_SKEL_DIE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_skedth,		// deathsound
	10,		// speed
	20*FRACUNIT,		// radius
	56*FRACUNIT,		// height
	500,		// mass
	0,		// damage
	sfx1_skeact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_SKEL_RAISE1		// raisestate
    },

    {		//	RevenantTracer
	-1,		// doomednum
	S1_TRACER,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_skeatk,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_TRACEEXP1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_barexp,		// deathsound
	10*FRACUNIT,		// speed
	11*FRACUNIT,		// radius
	8*FRACUNIT,		// height
	100,		// mass
	10,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    MF2_PCROSS,	// flags2
	S1_NULL,		// raisestate
	0,
	"EF_DL_RBALL"
    },

    {		//   RevenantTracerSmoke
	-1,		// doomednum
	S1_SMOKE1,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//	Mancubus
	67,		// doomednum
	S1_FATT_STND,		// spawnstate
	600,		// spawnhealth
	S1_FATT_RUN1,		// seestate
	sfx1_mansit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S1_FATT_PAIN,		// painstate
	80,		// painchance
	sfx1_mnpain,		// painsound
	0,		// meleestate
	S1_FATT_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_FATT_DIE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_mandth,		// deathsound
	8,		// speed
	48*FRACUNIT,		// radius
	64*FRACUNIT,		// height
	1000,		// mass
	0,		// damage
	sfx1_posact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_FATT_RAISE1		// raisestate
    },

    {		//	MancubusMissile
	-1,		// doomednum
	S1_FATSHOT1,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_firsht,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_FATSHOTX1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_firxpl,		// deathsound
	20*FRACUNIT,		// speed
	6*FRACUNIT,		// radius
	8*FRACUNIT,		// height
	100,		// mass
	8,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    MF2_PCROSS,	// flags2
	S1_NULL,		// raisestate
	0,
	"EF_DL_RBALL"
    },

    {		//	ChaingunGuy
	65,		// doomednum
	S1_CPOS_STND,		// spawnstate
	70,		// spawnhealth
	S1_CPOS_RUN1,		// seestate
	sfx1_posit2,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S1_CPOS_PAIN,		// painstate
	170,		// painchance
	sfx1_popain,		// painsound
	0,		// meleestate
	S1_CPOS_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_CPOS_DIE1,		// deathstate
	S1_CPOS_XDIE1,		// xdeathstate
	sfx1_podth2,		// deathsound
	8,		// speed
	20*FRACUNIT,		// radius
	56*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_posact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_CPOS_RAISE1		// raisestate
    },

    {		//	Imp
	3001,		// doomednum
	S1_TROO_STND,		// spawnstate
	60,		// spawnhealth
	S1_TROO_RUN1,		// seestate
	sfx1_bgsit1,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S1_TROO_PAIN,		// painstate
	200,		// painchance
	sfx1_popain,		// painsound
	S1_TROO_ATK1,		// meleestate
	S1_TROO_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_TROO_DIE1,		// deathstate
	S1_TROO_XDIE1,		// xdeathstate
	sfx1_bgdth1,		// deathsound
	8,		// speed
	20*FRACUNIT,		// radius
	56*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_bgact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_TROO_RAISE1		// raisestate
    },

    {		//	Demon
	3002,		// doomednum
	S1_SARG_STND,		// spawnstate
	150,		// spawnhealth
	S1_SARG_RUN1,		// seestate
	sfx1_sgtsit,		// seesound
	8,		// reactiontime
	sfx1_sgtatk,		// attacksound
	S1_SARG_PAIN,		// painstate
	180,		// painchance
	sfx1_dmpain,		// painsound
	S1_SARG_ATK1,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_SARG_DIE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_sgtdth,		// deathsound
	10,		// speed
	30*FRACUNIT,		// radius
	56*FRACUNIT,		// height
	400,		// mass
	0,		// damage
	sfx1_dmact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_SARG_RAISE1		// raisestate
    },

    {		//	Shadows
	58,		// doomednum
	S1_SARG_STND,		// spawnstate
	150,		// spawnhealth
	S1_SARG_RUN1,		// seestate
	sfx1_sgtsit,		// seesound
	8,		// reactiontime
	sfx1_sgtatk,		// attacksound
	S1_SARG_PAIN,		// painstate
	180,		// painchance
	sfx1_dmpain,		// painsound
	S1_SARG_ATK1,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_SARG_DIE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_sgtdth,		// deathsound
	10,		// speed
	30*FRACUNIT,		// radius
	56*FRACUNIT,		// height
	400,		// mass
	0,		// damage
	sfx1_dmact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_SHADOW|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_SARG_RAISE1		// raisestate
    },

    {		//	Cacodemon
	3005,		// doomednum
	S1_HEAD_STND,		// spawnstate
	400,		// spawnhealth
	S1_HEAD_RUN1,		// seestate
	sfx1_cacsit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S1_HEAD_PAIN,		// painstate
	128,		// painchance
	sfx1_dmpain,		// painsound
	0,		// meleestate
	S1_HEAD_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_HEAD_DIE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_cacdth,		// deathsound
	8,		// speed
	31*FRACUNIT,		// radius
	56*FRACUNIT,		// height
	400,		// mass
	0,		// damage
	sfx1_dmact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_FLOAT|MF_NOGRAVITY|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_HEAD_RAISE1		// raisestate
    },

    {		//	BaronOfHell
	3003,		// doomednum
	S1_BOSS_STND,		// spawnstate
	1000,		// spawnhealth
	S1_BOSS_RUN1,		// seestate
	sfx1_brssit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S1_BOSS_PAIN,		// painstate
	50,		// painchance
	sfx1_dmpain,		// painsound
	S1_BOSS_ATK1,		// meleestate
	S1_BOSS_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_BOSS_DIE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_brsdth,		// deathsound
	8,		// speed
	24*FRACUNIT,		// radius
	64*FRACUNIT,		// height
	1000,		// mass
	0,		// damage
	sfx1_dmact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_BOSS_RAISE1		// raisestate
    },

    {		//	BruiserShot
	-1,		// doomednum
	S1_BRBALL1,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_firsht,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_BRBALLX1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_firxpl,		// deathsound
	15*FRACUNIT,		// speed
	6*FRACUNIT,		// radius
	8*FRACUNIT,		// height
	100,		// mass
	8,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    MF2_PCROSS,	// flags2
	S1_NULL,		// raisestate
	0,
	"EF_DL_GBALL"
    },

    {		//	HellKnight
	69,		// doomednum
	S1_BOS2_STND,		// spawnstate
	500,		// spawnhealth
	S1_BOS2_RUN1,		// seestate
	sfx1_kntsit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S1_BOS2_PAIN,		// painstate
	50,		// painchance
	sfx1_dmpain,		// painsound
	S1_BOS2_ATK1,		// meleestate
	S1_BOS2_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_BOS2_DIE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_kntdth,		// deathsound
	8,		// speed
	24*FRACUNIT,		// radius
	64*FRACUNIT,		// height
	1000,		// mass
	0,		// damage
	sfx1_dmact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_BOS2_RAISE1		// raisestate
    },

    {		//	LostSoul
	3006,		// doomednum
	S1_SKULL_STND,		// spawnstate
	100,		// spawnhealth
	S1_SKULL_RUN1,		// seestate
	0,		// seesound
	8,		// reactiontime
	sfx1_sklatk,		// attacksound
	S1_SKULL_PAIN,		// painstate
	256,		// painchance
	sfx1_dmpain,		// painsound
	0,		// meleestate
	S1_SKULL_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_SKULL_DIE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_firxpl,		// deathsound
	8,		// speed
	16*FRACUNIT,		// radius
	56*FRACUNIT,		// height
	50,		// mass
	3,		// damage
	sfx1_dmact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_FLOAT|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    MF2_MCROSS,	// flags2
	S1_NULL,		// raisestate
	0,
	"EF_DL_MISSILE"
    },

    {		//	SpiderMastermind
	7,		// doomednum
	S1_SPID_STND,		// spawnstate
	3000,		// spawnhealth
	S1_SPID_RUN1,		// seestate
	sfx1_spisit,		// seesound
	8,		// reactiontime
	sfx1_shotgn,		// attacksound
	S1_SPID_PAIN,		// painstate
	40,		// painchance
	sfx1_dmpain,		// painsound
	0,		// meleestate
	S1_SPID_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_SPID_DIE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_spidth,		// deathsound
	12,		// speed
	128*FRACUNIT,		// radius
	100*FRACUNIT,		// height
	1000,		// mass
	0,		// damage
	sfx1_dmact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_NULL		// raisestate
    },

    {		//	Arachnotron
	68,		// doomednum
	S1_BSPI_STND,		// spawnstate
	500,		// spawnhealth
	S1_BSPI_SIGHT,		// seestate
	sfx1_bspsit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S1_BSPI_PAIN,		// painstate
	128,		// painchance
	sfx1_dmpain,		// painsound
	0,		// meleestate
	S1_BSPI_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_BSPI_DIE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_bspdth,		// deathsound
	12,		// speed
	64*FRACUNIT,		// radius
	64*FRACUNIT,		// height
	600,		// mass
	0,		// damage
	sfx1_bspact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_BSPI_RAISE1		// raisestate
    },

    {		//	Cyberdemon
	16,		// doomednum
	S1_CYBER_STND,		// spawnstate
	4000,		// spawnhealth
	S1_CYBER_RUN1,		// seestate
	sfx1_cybsit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S1_CYBER_PAIN,		// painstate
	20,		// painchance
	sfx1_dmpain,		// painsound
	0,		// meleestate
	S1_CYBER_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_CYBER_DIE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_cybdth,		// deathsound
	16,		// speed
	40*FRACUNIT,		// radius
	110*FRACUNIT,		// height
	1000,		// mass
	0,		// damage
	sfx1_dmact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_NULL		// raisestate
    },

    {		//	PainElemental
	71,		// doomednum
	S1_PAIN_STND,		// spawnstate
	400,		// spawnhealth
	S1_PAIN_RUN1,		// seestate
	sfx1_pesit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S1_PAIN_PAIN,		// painstate
	128,		// painchance
	sfx1_pepain,		// painsound
	0,		// meleestate
	S1_PAIN_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_PAIN_DIE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_pedth,		// deathsound
	8,		// speed
	31*FRACUNIT,		// radius
	56*FRACUNIT,		// height
	400,		// mass
	0,		// damage
	sfx1_dmact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_FLOAT|MF_NOGRAVITY|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_PAIN_RAISE1		// raisestate
    },

    {		//	WolfSS
	84,		// doomednum
	S1_SSWV_STND,		// spawnstate
	50,		// spawnhealth
	S1_SSWV_RUN1,		// seestate
	sfx1_sssit,		// seesound
	8,		// reactiontime
	0,		// attacksound
	S1_SSWV_PAIN,		// painstate
	170,		// painchance
	sfx1_popain,		// painsound
	0,		// meleestate
	S1_SSWV_ATK1,		// missilestate
	S1_NULL,		// crashstate
	S1_SSWV_DIE1,		// deathstate
	S1_SSWV_XDIE1,		// xdeathstate
	sfx1_ssdth,		// deathsound
	8,		// speed
	20*FRACUNIT,		// radius
	56*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_posact,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_SSWV_RAISE1		// raisestate
    },

    {		//	CommanderKeen
	72,		// doomednum
	S1_KEENSTND,		// spawnstate
	100,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_KEENPAIN,		// painstate
	256,		// painchance
	sfx1_keenpn,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_COMMKEEN,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_keendt,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	72*FRACUNIT,		// height
	10000000,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY|MF_SHOOTABLE|MF_COUNTKILL,		// flags
    MF2_MCROSS,	// flags2
	S1_NULL		// raisestate
    },

    {		//   BossBrain
	88,		// doomednum
	S1_BRAIN,		// spawnstate
	250,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_BRAIN_PAIN,		// painstate
	255,		// painchance
	sfx1_bospn,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_BRAIN_DIE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_bosdth,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	10000000,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SHOOTABLE,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//	BossEye
	89,		// doomednum
	S1_BRAINEYE,		// spawnstate
	1000,		// spawnhealth
	S1_BRAINEYESEE,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	32*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_NOSECTOR,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//	BossTarget
	87,		// doomednum
	S1_NULL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	32*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_NOSECTOR,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//	SpawnShot
	-1,		// doomednum
	S1_SPAWN1,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_bospit,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_firxpl,		// deathsound
	10*FRACUNIT,		// speed
	6*FRACUNIT,		// radius
	32*FRACUNIT,		// height
	100,		// mass
	3,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY|MF_NOCLIP,		// flags
    MF2_PCROSS,	// flags2
	S1_NULL		// raisestate
    },

    {		//	SpawnFire
	-1,		// doomednum
	S1_SPAWNFIRE1,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    0,	// flags2
	S1_NULL,		// raisestate
	0,
	"EF_DL_MISSILE"
    },

    {		//	Barrel
	2035,		// doomednum
	S1_BAR1,		// spawnstate
	20,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_BEXP,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_barexp,		// deathsound
	0,		// speed
	10*FRACUNIT,		// radius
	42*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SHOOTABLE|MF_NOBLOOD,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//	ImpMissile
	-1,		// doomednum
	S1_TBALL1,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_firsht,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_TBALLX1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_firxpl,		// deathsound
	10*FRACUNIT,		// speed
	6*FRACUNIT,		// radius
	8*FRACUNIT,		// height
	100,		// mass
	3,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    MF2_PCROSS,	// flags2
	S1_NULL,		// raisestate
	0,
	"EF_DL_RBALL"
    },

    {		//	CacodemonMissile
	-1,		// doomednum
	S1_RBALL1,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_firsht,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_RBALLX1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_firxpl,		// deathsound
	10*FRACUNIT,		// speed
	6*FRACUNIT,		// radius
	8*FRACUNIT,		// height
	100,		// mass
	5,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    MF2_PCROSS,	// flags2
	S1_NULL,		// raisestate
	0,
	"EF_DL_RBALL"
    },

    {		//	Rocket
	-1,		// doomednum
	S1_ROCKET,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_rlaunc,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_EXPLODE1,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_barexp,		// deathsound
	20*FRACUNIT,		// speed
	11*FRACUNIT,		// radius
	8*FRACUNIT,		// height
	100,		// mass
	20,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
    MF2_PCROSS,	// flags2
	S1_NULL,		// raisestate
	0,
	"EF_DL_MISSILE | EF_ROCKET"
    },

    {		//	PlasmaShot
	-1,		// doomednum
	S1_PLASBALL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_plasma,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_PLASEXP,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_firxpl,		// deathsound
	25*FRACUNIT,		// speed
	13*FRACUNIT,		// radius
	8*FRACUNIT,		// height
	100,		// mass
	5,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    MF2_PCROSS,	// flags2
	S1_NULL,		// raisestate
	0,
	"EF_DL_PLASMA"
    },

    {		//	BFGShot
	-1,		// doomednum
	S1_BFGSHOT,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	0,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_BFGLAND,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_rxplod,		// deathsound
	25*FRACUNIT,		// speed
	13*FRACUNIT,		// radius
	8*FRACUNIT,		// height
	100,		// mass
	100,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    MF2_PCROSS,	// flags2
	S1_NULL,		// raisestate
	0,
	"EF_DL_GBALL"
    },

    {		//	ArachnotronPlasma
	-1,		// doomednum
	S1_ARACH_PLAZ,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_plasma,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_ARACH_PLEX,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_firxpl,		// deathsound
	25*FRACUNIT,		// speed
	13*FRACUNIT,		// radius
	8*FRACUNIT,		// height
	100,		// mass
	5,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    MF2_PCROSS,	// flags2
	S1_NULL,		// raisestate
	0,
	"EF_DL_MISSILE"
    },

    {		//	Puff
	-1,		// doomednum
	S1_PUFF1,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//	Blood
	-1,		// doomednum
	S1_BLOOD1,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_TRANSLUCENT,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//	TeleportFog
	-1,		// doomednum
	S1_TFOG,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemRespawnFog
	-1,		// doomednum
	S1_IFOG,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   TeleportSpot
	14,		// doomednum
	S1_NULL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_NOSECTOR,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ExtraBFG
	-1,		// doomednum
	S1_BFGEXP,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP|MF_NOGRAVITY|MF_TRANSLUCENT,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemArmor1
	2018,		// doomednum
	S1_ARM1,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemArmor2
	2019,		// doomednum
	S1_ARM2,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemHealthBonus
	2014,		// doomednum
	S1_BON1,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_COUNTITEM|MF_TRANSLUCENT,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemArmorBonus
	2015,		// doomednum
	S1_BON2,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_COUNTITEM,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemKeyBlueCard
	5,		// doomednum
	S1_BKEY,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_NOTDMATCH,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemKeyRedCard
	13,		// doomednum
	S1_RKEY,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_NOTDMATCH,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemKeyYellowCard
	6,		// doomednum
	S1_YKEY,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_NOTDMATCH,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemKeyYellowSkull
	39,		// doomednum
	S1_YSKULL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_NOTDMATCH,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemKeyRedSkull
	38,		// doomednum
	S1_RSKULL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_NOTDMATCH,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemKeyBlueSkull
	40,		// doomednum
	S1_BSKULL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_NOTDMATCH,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemStimPack
	2011,		// doomednum
	S1_STIM,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemMedikit
	2012,		// doomednum
	S1_MEDI,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemSoulSphere
	2013,		// doomednum
	S1_SOUL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_COUNTITEM|MF_TRANSLUCENT,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//	ItemInvulnerability
	2022,		// doomednum
	S1_PINV,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_COUNTITEM|MF_TRANSLUCENT,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemBerserk
	2023,		// doomednum
	S1_PSTR,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// crashstate
	S1_NULL,		// missilestate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_COUNTITEM,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemInvisibility
	2024,		// doomednum
	S1_PINS,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_COUNTITEM|MF_TRANSLUCENT,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemRadiationSuit
	2025,		// doomednum
	S1_SUIT,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemComputerMap
	2026,		// doomednum
	S1_PMAP,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_COUNTITEM,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemInfrared
	2045,		// doomednum
	S1_PVIS,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_COUNTITEM,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemMegaSphere
	83,		// doomednum
	S1_MEGA,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL|MF_COUNTITEM|MF_TRANSLUCENT,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemAmmoBulletClip
	2007,		// doomednum
	S1_CLIP,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemAmmoBulletBox
	2048,		// doomednum
	S1_AMMO,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemAmmoRocket
	2010,		// doomednum
	S1_ROCK,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemAmmoRocketBox
	2046,		// doomednum
	S1_BROK,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemAmmoCells
	2047,		// doomednum
	S1_CELL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemAmmoCellsPack
	17,		// doomednum
	S1_CELP,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemAmmoShells
	2008,		// doomednum
	S1_SHEL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemAmmoShellsBox
	2049,		// doomednum
	S1_SBOX,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemBackpack
	8,		// doomednum
	S1_BPAK,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemWeaponBFG
	2006,		// doomednum
	S1_BFUG,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemWeaponChaingun
	2002,		// doomednum
	S1_MGUN,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemWeaponChainsaw
	2005,		// doomednum
	S1_CSAW,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemWeaponRocketLauncher
	2003,		// doomednum
	S1_LAUN,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemWeaponPlasmaGun
	2004,		// doomednum
	S1_PLAS,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemWeaponShotgun
	2001,		// doomednum
	S1_SHOT,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ItemWeaponSuperShotgun
	82,		// doomednum
	S1_SHOT2,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPECIAL,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   TechLamp
	85,		// doomednum
	S1_TECHLAMP,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL,		// raisestate
	0,
	0,
	"AddStaticLight(origin + vector(0.0, 0.0, 64.0),\n"
	"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 200.0);"
    },

    {		//   TechLamp2
	86,		// doomednum
	S1_TECH2LAMP,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL,		// raisestate
	0,
	0,
	"AddStaticLight(origin + vector(0.0, 0.0, 48.0),\n"
		"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 200.0);"
    },

    {		//   ColumnLamp
	2028,		// doomednum
	S1_COLU,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL,		// raisestate
	0,
	0,
	"AddStaticLight(origin + vector(0.0, 0.0, 32.0),\n"
		"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 200.0);"
    },

    {		//   ColumnGreenTall
	30,		// doomednum
	S1_TALLGRNCOL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ColumnGreenShort
	31,		// doomednum
	S1_SHRTGRNCOL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ColumnRedTall
	32,		// doomednum
	S1_TALLREDCOL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ColumnRedShort
	33,		// doomednum
	S1_SHRTREDCOL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ColumnWithSkull
	37,		// doomednum
	S1_SKULLCOL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ColumnWithHearth
	36,		// doomednum
	S1_HEARTCOL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   EvilEye
	41,		// doomednum
	S1_EVILEYE,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL,		// raisestate
	0,
	0,
	"AddStaticLight(origin + vector(0.0, 0.0, 32.0),\n"
		"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 128.0);"
    },

    {		//   FloatingSkull
	42,		// doomednum
	S1_FLOATSKULL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   TorchTree
	43,		// doomednum
	S1_TORCHTREE,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   TorchBlue
	44,		// doomednum
	S1_BLUETORCH,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL,		// raisestate
	0,
	0,
	"AddStaticLightRGB(origin + vector(0.0, 0.0, 32.0),\n"
		"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 200.0, RGB(0, 0, 255));"
    },

    {		//   TorchGreen
	45,		// doomednum
	S1_GREENTORCH,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL,		// raisestate
	0,
	0,
	"AddStaticLightRGB(origin + vector(0.0, 0.0, 32.0),\n"
		"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 200.0, RGB(0, 255, 0));"
    },

    {		//   TorchRed
	46,		// doomednum
	S1_REDTORCH,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL,		// raisestate
	0,
	0,
	"AddStaticLightRGB(origin + vector(0.0, 0.0, 32.0),\n"
		"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 200.0, RGB(255, 0, 0));"
    },

    {		//   TorchBlueShort
	55,		// doomednum
	S1_BTORCHSHRT,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL,		// raisestate
	0,
	0,
	"AddStaticLightRGB(origin + vector(0.0, 0.0, 16.0),\n"
		"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 200.0, RGB(0, 0, 255));"
    },

    {		//   TorchGreenShort
	56,		// doomednum
	S1_GTORCHSHRT,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL,		// raisestate
	0,
	0,
	"AddStaticLightRGB(origin + vector(0.0, 0.0, 16.0),\n"
		"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 200.0, RGB(0, 255, 0));"
    },

    {		//   TorchRedShort
	57,		// doomednum
	S1_RTORCHSHRT,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL,		// raisestate
	0,
	0,
	"AddStaticLightRGB(origin + vector(0.0, 0.0, 16.0),\n"
		"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 200.0, RGB(255, 0, 0));"
    },

    {		//   Stalagtite
	47,		// doomednum
	S1_STALAGTITE,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   TechPillar
	48,		// doomednum
	S1_TECHPILLAR,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   CandleStick
	34,		// doomednum
	S1_CANDLESTIK,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	0,		// flags
    0,	// flags2
	S1_NULL,		// raisestate
	0,
	0,
	"AddStaticLight(origin, mthing->arg1 ? itof(mthing->arg1) * 8.0 : 64.0);"
    },

    {		//   Candelabra
	35,		// doomednum
	S1_CANDELABRA,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL,		// raisestate
	0,
	0,
	"AddStaticLight(origin + vector(0.0, 0.0, 32.0),\n"
		"\t\tmthing->arg1 ? itof(mthing->arg1) * 8.0 : 128.0);"
    },

    {		//   BloodyTwitch
	49,		// doomednum
	S1_BLOODYTWITCH,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	68*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
	S1_NULL		// raisestate
    },

    {		//   Meat2
	50,		// doomednum
	S1_MEAT2,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	84*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   Meat3
	51,		// doomednum
	S1_MEAT3,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	84*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   Meat4
	52,		// doomednum
	S1_MEAT4,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	68*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   Meat5
	53,		// doomednum
	S1_MEAT5,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	52*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   Meat2NoBlock
	59,		// doomednum
	S1_MEAT2,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	84*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   Meat4NoBlock
	60,		// doomednum
	S1_MEAT4,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	68*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   Meat3NoBlock
	61,		// doomednum
	S1_MEAT3,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	52*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   Meat5NoBlock
	62,		// doomednum
	S1_MEAT5,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	52*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   BloodyTwitchNoBlock
	63,		// doomednum
	S1_BLOODYTWITCH,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	68*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   DeadCacodemon
	22,		// doomednum
	S1_HEAD_DIE6,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	0,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   DeadPlayer
	15,		// doomednum
	S1_PLAY_DIE7,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	0,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   DeadZombieman
	18,		// doomednum
	S1_POSS_DIE5,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	0,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   DeadDemon
	21,		// doomednum
	S1_SARG_DIE6,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	0,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   DeadLostSoul
	23,		// doomednum
	S1_SKULL_DIE6,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	0,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   DeadImp
	20,		// doomednum
	S1_TROO_DIE5,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	0,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   DeadShotgunGuy
	19,		// doomednum
	S1_SPOS_DIE5,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	0,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   DeadPlayerGibs
	10,		// doomednum
	S1_PLAY_XDIE9,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	0,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   DeadPlayerGibs2
	12,		// doomednum
	S1_PLAY_XDIE9,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	0,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   HeadsOnStick
	28,		// doomednum
	S1_HEADSONSTICK,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   Gibs
	24,		// doomednum
	S1_GIBS,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	0,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   HeadOnAStick
	27,		// doomednum
	S1_HEADONASTICK,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   HeadCandles
	29,		// doomednum
	S1_HEADCANDLES,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   DeadStick
	25,		// doomednum
	S1_DEADSTICK,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   LiveStick
	26,		// doomednum
	S1_LIVESTICK,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   BigTree
	54,		// doomednum
	S1_BIGTREE,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	32*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   BurningBarrel
	70,		// doomednum
	S1_BBAR1,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   HangingNoGuts
	73,		// doomednum
	S1_HANGNOGUTS,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	88*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   HangingNoBrain
	74,		// doomednum
	S1_HANGBNOBRAIN,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	88*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   HangingTorsoLookDown
	75,		// doomednum
	S1_HANGTLOOKDN,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	64*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   HangingTorsoSkull
	76,		// doomednum
	S1_HANGTSKULL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	64*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   HangingTorsoLookUp
	77,		// doomednum
	S1_HANGTLOOKUP,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	64*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   HangingTorsoNoBrain
	78,		// doomednum
	S1_HANGTNOBRAIN,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	64*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   ColonGibs
	79,		// doomednum
	S1_COLONGIBS,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   SmallPool
	80,		// doomednum
	S1_SMALLPOOL,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    },

    {		//   BrainStem
	81,		// doomednum
	S1_BRAINSTEM,		// spawnstate
	1000,		// spawnhealth
	S1_NULL,		// seestate
	sfx1_None,		// seesound
	8,		// reactiontime
	sfx1_None,		// attacksound
	S1_NULL,		// painstate
	0,		// painchance
	sfx1_None,		// painsound
	S1_NULL,		// meleestate
	S1_NULL,		// missilestate
	S1_NULL,		// crashstate
	S1_NULL,		// deathstate
	S1_NULL,		// xdeathstate
	sfx1_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx1_None,		// activesound
	MF_NOBLOCKMAP,		// flags
    0,	// flags2
	S1_NULL		// raisestate
    }
};

void FixupHeights(void)
{
	mobjinfo[MT1_MISC29].height = 72*FRACUNIT;
	mobjinfo[MT1_MISC30].height = 56*FRACUNIT;
	mobjinfo[MT1_MISC31].height = 48*FRACUNIT;
	mobjinfo[MT1_MISC32].height = 52*FRACUNIT;
	mobjinfo[MT1_MISC33].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC34].height = 52*FRACUNIT;
	mobjinfo[MT1_MISC35].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC36].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC37].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC39].height = 48*FRACUNIT;
	mobjinfo[MT1_MISC40].height = 64*FRACUNIT;
	mobjinfo[MT1_MISC41].height = 64*FRACUNIT;
	mobjinfo[MT1_MISC42].height = 64*FRACUNIT;
	mobjinfo[MT1_MISC43].height = 64*FRACUNIT;
	mobjinfo[MT1_MISC44].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC45].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC46].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC47].height = 40*FRACUNIT;
	mobjinfo[MT1_MISC48].height = 120*FRACUNIT;
	mobjinfo[MT1_MISC50].height = 56*FRACUNIT;
	mobjinfo[MT1_MISC70].height = 64*FRACUNIT;
	mobjinfo[MT1_MISC72].height = 56*FRACUNIT;
	mobjinfo[MT1_MISC73].height = 48*FRACUNIT;
	mobjinfo[MT1_MISC74].height = 64*FRACUNIT;
	mobjinfo[MT1_MISC75].height = 64*FRACUNIT;
	mobjinfo[MT1_MISC76].height = 96*FRACUNIT;
	mobjinfo[MT1_MISC77].height = 32*FRACUNIT;

	// Mark these unused super shotgun states
	MarkWeaponState(S1_DSNR1);
}

void MarkSpecialWeaponStates(void)
{
	MarkWeaponState(S1_CHAINFLASH2);
	MarkWeaponState(S1_PLASMAFLASH2);
}

//**************************************************************************
//
//	$Log$
//	Revision 1.8  2002/01/29 18:19:01  dj_jl
//	Added MCROSS and PCROSS flags.
//
//	Revision 1.7  2002/01/17 18:18:12  dj_jl
//	Renamed all map object classes
//	
//	Revision 1.6  2002/01/07 12:30:05  dj_jl
//	Changed copyright year
//	
//	Revision 1.5  2001/12/12 19:20:31  dj_jl
//	States using methods
//	
//	Revision 1.4  2001/09/27 17:04:39  dj_jl
//	Effects and static lights in mobjinfo, mobj classes
//	
//	Revision 1.3  2001/09/20 16:33:14  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
