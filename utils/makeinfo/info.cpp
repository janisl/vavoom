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

#include "makeinfo.h"
#include "info.h"

state_action_info_t StateActionInfo[] =
{
	{NULL},
	{"A_Light0"},
	{"A_WeaponReady"},
	{"A_Lower"},
	{"A_Raise"},
	{"A_Punch"},
	{"A_ReFire"},
	{"A_FirePistol"},
	{"A_Light1"},
	{"A_FireShotgun"},
	{"A_Light2"},

	{"A_FireShotgun2"},
	{"A_CheckReload"},
	{"A_OpenShotgun2"},
	{"A_LoadShotgun2"},
	{"A_CloseShotgun2"},
	{"A_FireCGun"},
	{"A_GunFlash"},
	{"A_FireMissile"},
	{"A_Saw"},
	{"A_FirePlasma"},

	{"A_BFGsound"},
	{"A_FireBFG"},
	{"A_BFGSpray"},
	{"A_Explode"},
	{"A_Pain"},
	{"A_PlayerScream"},
	{"A_Fall"},
	{"A_XScream"},
	{"A_Look"},
	{"A_Chase"},

	{"A_FaceTarget"},
	{"A_PosAttack"},
	{"A_Scream"},
	{"A_SPosAttack"},
	{"A_VileChase"},
	{"A_VileStart"},
	{"A_VileTarget"},
	{"A_VileAttack"},
	{"A_StartFire"},
	{"A_Fire"},

	{"A_FireCrackle"},
	{"A_Tracer"},
	{"A_SkelWhoosh"},
	{"A_SkelFist"},
	{"A_SkelMissile"},
	{"A_FatRaise"},
	{"A_FatAttack1"},
	{"A_FatAttack2"},
	{"A_FatAttack3"},
	{"A_BossDeath"},

	{"A_CPosAttack"},
	{"A_CPosRefire"},
	{"A_TroopAttack"},
	{"A_SargAttack"},
	{"A_HeadAttack"},
	{"A_BruisAttack"},
	{"A_SkullAttack"},
	{"A_Metal"},
	{"A_SpidRefire"},
	{"A_BabyMetal"},

	{"A_BspiAttack"},
	{"A_Hoof"},
	{"A_CyberAttack"},
	{"A_PainAttack"},
	{"A_PainDie"},
	{"A_KeenDie"},
	{"A_BrainPain"},
	{"A_BrainScream"},
	{"A_BrainDie"},
	{"A_BrainAwake"},

	{"A_BrainSpit"},
	{"A_SpawnSound"},
	{"A_SpawnFly"},
	{"A_BrainExplode"},
	{NULL},
};

state_t	states[NUMSTATES] = {
	{SPR1_TROO,0,-1,SA1_NULL,S1_NULL,0,0},	// S1_NULL
	{SPR1_SHTG,4,0,SA1_Light0,S1_NULL,0,0},	// S1_LIGHTDONE
	{SPR1_PUNG,0,1,SA1_WeaponReady,S1_PUNCH,0,0,"weapons/v_fists.md2",0},	// S1_PUNCH
	{SPR1_PUNG,0,1,SA1_Lower,S1_PUNCHDOWN,0,0,"weapons/v_fists.md2",1},	// S1_PUNCHDOWN
	{SPR1_PUNG,0,1,SA1_Raise,S1_PUNCHUP,0,0,"weapons/v_fists.md2",2},	// S1_PUNCHUP
	{SPR1_PUNG,1,4,SA1_NULL,S1_PUNCH2,0,0,"weapons/v_fists.md2",3},		// S1_PUNCH1
	{SPR1_PUNG,2,4,SA1_Punch,S1_PUNCH3,0,0,"weapons/v_fists.md2",4},	// S1_PUNCH2
	{SPR1_PUNG,3,5,SA1_NULL,S1_PUNCH4,0,0,"weapons/v_fists.md2",5},		// S1_PUNCH3
	{SPR1_PUNG,2,4,SA1_NULL,S1_PUNCH5,0,0,"weapons/v_fists.md2",6},		// S1_PUNCH4
	{SPR1_PUNG,1,5,SA1_ReFire,S1_PUNCH,0,0,"weapons/v_fists.md2",7},	// S1_PUNCH5
	{SPR1_PISG,0,1,SA1_WeaponReady,S1_PISTOL,0,0,"weapons/v_pistol.md2",0},// S1_PISTOL
	{SPR1_PISG,0,1,SA1_Lower,S1_PISTOLDOWN,0,0,"weapons/v_pistol.md2",1},	// S1_PISTOLDOWN
	{SPR1_PISG,0,1,SA1_Raise,S1_PISTOLUP,0,0,"weapons/v_pistol.md2",2},	// S1_PISTOLUP
	{SPR1_PISG,0,4,SA1_NULL,S1_PISTOL2,0,0,"weapons/v_pistol.md2",3},	// S1_PISTOL1
	{SPR1_PISG,1,6,SA1_FirePistol,S1_PISTOL3,0,0,"weapons/v_pistol.md2",4},// S1_PISTOL2
	{SPR1_PISG,2,4,SA1_NULL,S1_PISTOL4,0,0,"weapons/v_pistol.md2",5},	// S1_PISTOL3
	{SPR1_PISG,1,5,SA1_ReFire,S1_PISTOL,0,0,"weapons/v_pistol.md2",6},	// S1_PISTOL4
	{SPR1_PISF,32768,7,SA1_Light1,S1_LIGHTDONE,0,0},	// S1_PISTOLFLASH
	{SPR1_SHTG,0,1,SA1_WeaponReady,S1_SGUN,0,0,"weapons/v_shotg.md2",0},	// S1_SGUN
	{SPR1_SHTG,0,1,SA1_Lower,S1_SGUNDOWN,0,0,"weapons/v_shotg.md2",1},	// S1_SGUNDOWN
	{SPR1_SHTG,0,1,SA1_Raise,S1_SGUNUP,0,0,"weapons/v_shotg.md2",2},	// S1_SGUNUP
	{SPR1_SHTG,0,3,SA1_NULL,S1_SGUN2,0,0,"weapons/v_shotg.md2",3},	// S1_SGUN1
	{SPR1_SHTG,0,7,SA1_FireShotgun,S1_SGUN3,0,0,"weapons/v_shotg.md2",4},	// S1_SGUN2
	{SPR1_SHTG,1,5,SA1_NULL,S1_SGUN4,0,0,"weapons/v_shotg.md2",5},	// S1_SGUN3
	{SPR1_SHTG,2,5,SA1_NULL,S1_SGUN5,0,0,"weapons/v_shotg.md2",6},	// S1_SGUN4
	{SPR1_SHTG,3,4,SA1_NULL,S1_SGUN6,0,0,"weapons/v_shotg.md2",7},	// S1_SGUN5
	{SPR1_SHTG,2,5,SA1_NULL,S1_SGUN7,0,0,"weapons/v_shotg.md2",8},	// S1_SGUN6
	{SPR1_SHTG,1,5,SA1_NULL,S1_SGUN8,0,0,"weapons/v_shotg.md2",9},	// S1_SGUN7
	{SPR1_SHTG,0,3,SA1_NULL,S1_SGUN9,0,0,"weapons/v_shotg.md2",10},	// S1_SGUN8
	{SPR1_SHTG,0,7,SA1_ReFire,S1_SGUN,0,0,"weapons/v_shotg.md2",11},	// S1_SGUN9
	{SPR1_SHTF,32768,4,SA1_Light1,S1_SGUNFLASH2,0,0},	// S1_SGUNFLASH1
	{SPR1_SHTF,32769,3,SA1_Light2,S1_LIGHTDONE,0,0},	// S1_SGUNFLASH2
	{SPR1_SHT2,0,1,SA1_WeaponReady,S1_DSGUN,0,0,"weapons/v_super.md2",0},	// S1_DSGUN
	{SPR1_SHT2,0,1,SA1_Lower,S1_DSGUNDOWN,0,0,"weapons/v_super.md2",1},	// S1_DSGUNDOWN
	{SPR1_SHT2,0,1,SA1_Raise,S1_DSGUNUP,0,0,"weapons/v_super.md2",2},	// S1_DSGUNUP
	{SPR1_SHT2,0,3,SA1_NULL,S1_DSGUN2,0,0,"weapons/v_super.md2",3},	// S1_DSGUN1
	{SPR1_SHT2,0,7,SA1_FireShotgun2,S1_DSGUN3,0,0,"weapons/v_super.md2",4},	// S1_DSGUN2
	{SPR1_SHT2,1,7,SA1_NULL,S1_DSGUN4,0,0,"weapons/v_super.md2",5},	// S1_DSGUN3
	{SPR1_SHT2,2,7,SA1_CheckReload,S1_DSGUN5,0,0,"weapons/v_super.md2",6},	// S1_DSGUN4
	{SPR1_SHT2,3,7,SA1_OpenShotgun2,S1_DSGUN6,0,0,"weapons/v_super.md2",7},	// S1_DSGUN5
	{SPR1_SHT2,4,7,SA1_NULL,S1_DSGUN7,0,0,"weapons/v_super.md2",8},	// S1_DSGUN6
	{SPR1_SHT2,5,7,SA1_LoadShotgun2,S1_DSGUN8,0,0,"weapons/v_super.md2",9},	// S1_DSGUN7
	{SPR1_SHT2,6,6,SA1_NULL,S1_DSGUN9,0,0,"weapons/v_super.md2",10},	// S1_DSGUN8
	{SPR1_SHT2,7,6,SA1_CloseShotgun2,S1_DSGUN10,0,0,"weapons/v_super.md2",11},	// S1_DSGUN9
	{SPR1_SHT2,0,5,SA1_ReFire,S1_DSGUN,0,0,"weapons/v_super.md2",12},	// S1_DSGUN10
	{SPR1_SHT2,1,7,SA1_NULL,S1_DSNR2,0,0},	// S1_DSNR1
	{SPR1_SHT2,0,3,SA1_NULL,S1_DSGUNDOWN,0,0},	// S1_DSNR2
	{SPR1_SHT2,32776,5,SA1_Light1,S1_DSGUNFLASH2,0,0},	// S1_DSGUNFLASH1
	{SPR1_SHT2,32777,4,SA1_Light2,S1_LIGHTDONE,0,0},	// S1_DSGUNFLASH2
	{SPR1_CHGG,0,1,SA1_WeaponReady,S1_CHAIN,0,0,"weapons/v_chain.md2",0},	// S1_CHAIN
	{SPR1_CHGG,0,1,SA1_Lower,S1_CHAINDOWN,0,0,"weapons/v_chain.md2",1},	// S1_CHAINDOWN
	{SPR1_CHGG,0,1,SA1_Raise,S1_CHAINUP,0,0,"weapons/v_chain.md2",2},	// S1_CHAINUP
	{SPR1_CHGG,0,4,SA1_FireCGun,S1_CHAIN2,0,0,"weapons/v_chain.md2",3},	// S1_CHAIN1
	{SPR1_CHGG,1,4,SA1_FireCGun,S1_CHAIN3,0,0,"weapons/v_chain.md2",4},	// S1_CHAIN2
	{SPR1_CHGG,1,0,SA1_ReFire,S1_CHAIN,0,0,"weapons/v_chain.md2",4},	// S1_CHAIN3
	{SPR1_CHGF,32768,5,SA1_Light1,S1_LIGHTDONE,0,0},	// S1_CHAINFLASH1
	{SPR1_CHGF,32769,5,SA1_Light2,S1_LIGHTDONE,0,0},	// S1_CHAINFLASH2
	{SPR1_MISG,0,1,SA1_WeaponReady,S1_MISSILE,0,0,"weapons/v_launch.md2",0},	// S1_MISSILE
	{SPR1_MISG,0,1,SA1_Lower,S1_MISSILEDOWN,0,0,"weapons/v_launch.md2",1},	// S1_MISSILEDOWN
	{SPR1_MISG,0,1,SA1_Raise,S1_MISSILEUP,0,0,"weapons/v_launch.md2",2},	// S1_MISSILEUP
	{SPR1_MISG,1,8,SA1_GunFlash,S1_MISSILE2,0,0,"weapons/v_launch.md2",3},	// S1_MISSILE1
	{SPR1_MISG,1,12,SA1_FireMissile,S1_MISSILE3,0,0,"weapons/v_launch.md2",4},	// S1_MISSILE2
	{SPR1_MISG,1,0,SA1_ReFire,S1_MISSILE,0,0,"weapons/v_launch.md2",5},	// S1_MISSILE3
	{SPR1_MISF,32768,3,SA1_Light1,S1_MISSILEFLASH2,0,0},	// S1_MISSILEFLASH1
	{SPR1_MISF,32769,4,SA1_NULL,S1_MISSILEFLASH3,0,0},	// S1_MISSILEFLASH2
	{SPR1_MISF,32770,4,SA1_Light2,S1_MISSILEFLASH4,0,0},	// S1_MISSILEFLASH3
	{SPR1_MISF,32771,4,SA1_Light2,S1_LIGHTDONE,0,0},	// S1_MISSILEFLASH4
	{SPR1_SAWG,2,4,SA1_WeaponReady,S1_SAWB,0,0,"weapons/v_saw.md2",0},	// S1_SAW
	{SPR1_SAWG,3,4,SA1_WeaponReady,S1_SAW,0,0,"weapons/v_saw.md2",1},	// S1_SAWB
	{SPR1_SAWG,2,1,SA1_Lower,S1_SAWDOWN,0,0,"weapons/v_saw.md2",2},	// S1_SAWDOWN
	{SPR1_SAWG,2,1,SA1_Raise,S1_SAWUP,0,0,"weapons/v_saw.md2",3},	// S1_SAWUP
	{SPR1_SAWG,0,4,SA1_Saw,S1_SAW2,0,0,"weapons/v_saw.md2",4},	// S1_SAW1
	{SPR1_SAWG,1,4,SA1_Saw,S1_SAW3,0,0,"weapons/v_saw.md2",5},	// S1_SAW2
	{SPR1_SAWG,1,0,SA1_ReFire,S1_SAW,0,0,"weapons/v_saw.md2",5},	// S1_SAW3
	{SPR1_PLSG,0,1,SA1_WeaponReady,S1_PLASMA,0,0,"weapons/v_plasma.md2",0},	// S1_PLASMA
	{SPR1_PLSG,0,1,SA1_Lower,S1_PLASMADOWN,0,0,"weapons/v_plasma.md2",1},	// S1_PLASMADOWN
	{SPR1_PLSG,0,1,SA1_Raise,S1_PLASMAUP,0,0,"weapons/v_plasma.md2",2},	// S1_PLASMAUP
	{SPR1_PLSG,0,3,SA1_FirePlasma,S1_PLASMA2,0,0,"weapons/v_plasma.md2",3},	// S1_PLASMA1
	{SPR1_PLSG,1,20,SA1_ReFire,S1_PLASMA,0,0,"weapons/v_plasma.md2",4},	// S1_PLASMA2
	{SPR1_PLSF,32768,4,SA1_Light1,S1_LIGHTDONE,0,0},	// S1_PLASMAFLASH1
	{SPR1_PLSF,32769,4,SA1_Light1,S1_LIGHTDONE,0,0},	// S1_PLASMAFLASH2
	{SPR1_BFGG,0,1,SA1_WeaponReady,S1_BFG,0,0,"weapons/v_bfg.md2",0},	// S1_BFG
	{SPR1_BFGG,0,1,SA1_Lower,S1_BFGDOWN,0,0,"weapons/v_bfg.md2",1},	// S1_BFGDOWN
	{SPR1_BFGG,0,1,SA1_Raise,S1_BFGUP,0,0,"weapons/v_bfg.md2",2},	// S1_BFGUP
	{SPR1_BFGG,0,20,SA1_BFGsound,S1_BFG2,0,0,"weapons/v_bfg.md2",3},	// S1_BFG1
	{SPR1_BFGG,1,10,SA1_GunFlash,S1_BFG3,0,0,"weapons/v_bfg.md2",4},	// S1_BFG2
	{SPR1_BFGG,1,10,SA1_FireBFG,S1_BFG4,0,0,"weapons/v_bfg.md2",5},	// S1_BFG3
	{SPR1_BFGG,1,20,SA1_ReFire,S1_BFG,0,0,"weapons/v_bfg.md2",6},	// S1_BFG4
	{SPR1_BFGF,32768,11,SA1_Light1,S1_BFGFLASH2,0,0},	// S1_BFGFLASH1
	{SPR1_BFGF,32769,6,SA1_Light2,S1_LIGHTDONE,0,0},	// S1_BFGFLASH2
	{SPR1_BLUD,2,8,SA1_NULL,S1_BLOOD2,0,0},	// S1_BLOOD1
	{SPR1_BLUD,1,8,SA1_NULL,S1_BLOOD3,0,0},	// S1_BLOOD2
	{SPR1_BLUD,0,8,SA1_NULL,S1_NULL,0,0},	// S1_BLOOD3
	{SPR1_PUFF,32768,4,SA1_NULL,S1_PUFF2,0,0},	// S1_PUFF1
	{SPR1_PUFF,1,4,SA1_NULL,S1_PUFF3,0,0},	// S1_PUFF2
	{SPR1_PUFF,2,4,SA1_NULL,S1_PUFF4,0,0},	// S1_PUFF3
	{SPR1_PUFF,3,4,SA1_NULL,S1_NULL,0,0},	// S1_PUFF4
	{SPR1_BAL1,32768,4,SA1_NULL,S1_TBALL2,0,0,"missiles/impball.md2",0},	// S1_TBALL1
	{SPR1_BAL1,32769,4,SA1_NULL,S1_TBALL1,0,0,"missiles/impball.md2",1},	// S1_TBALL2
	{SPR1_BAL1,32770,6,SA1_NULL,S1_TBALLX2,0,0},	// S1_TBALLX1
	{SPR1_BAL1,32771,6,SA1_NULL,S1_TBALLX3,0,0},	// S1_TBALLX2
	{SPR1_BAL1,32772,6,SA1_NULL,S1_NULL,0,0},	// S1_TBALLX3
	{SPR1_BAL2,32768,4,SA1_NULL,S1_RBALL2,0,0,"missiles/cacoball.md2",0},	// S1_RBALL1
	{SPR1_BAL2,32769,4,SA1_NULL,S1_RBALL1,0,0,"missiles/cacoball.md2",1},	// S1_RBALL2
	{SPR1_BAL2,32770,6,SA1_NULL,S1_RBALLX2,0,0},	// S1_RBALLX1
	{SPR1_BAL2,32771,6,SA1_NULL,S1_RBALLX3,0,0},	// S1_RBALLX2
	{SPR1_BAL2,32772,6,SA1_NULL,S1_NULL,0,0},	// S1_RBALLX3
	{SPR1_PLSS,32768,6,SA1_NULL,S1_PLASBALL2,0,0,"missiles/plasma.md2",0},	// S1_PLASBALL
	{SPR1_PLSS,32769,6,SA1_NULL,S1_PLASBALL,0,0,"missiles/plasma.md2",1},	// S1_PLASBALL2
	{SPR1_PLSE,32768,4,SA1_NULL,S1_PLASEXP2,0,0},	// S1_PLASEXP
	{SPR1_PLSE,32769,4,SA1_NULL,S1_PLASEXP3,0,0},	// S1_PLASEXP2
	{SPR1_PLSE,32770,4,SA1_NULL,S1_PLASEXP4,0,0},	// S1_PLASEXP3
	{SPR1_PLSE,32771,4,SA1_NULL,S1_PLASEXP5,0,0},	// S1_PLASEXP4
	{SPR1_PLSE,32772,4,SA1_NULL,S1_NULL,0,0},	// S1_PLASEXP5
	{SPR1_MISL,32768,1,SA1_NULL,S1_ROCKET,0,0,"missiles/rocket.md2",0},	// S1_ROCKET
	{SPR1_BFS1,32768,4,SA1_NULL,S1_BFGSHOT2,0,0},	// S1_BFGSHOT
	{SPR1_BFS1,32769,4,SA1_NULL,S1_BFGSHOT,0,0},	// S1_BFGSHOT2
	{SPR1_BFE1,32768,8,SA1_NULL,S1_BFGLAND2,0,0},	// S1_BFGLAND
	{SPR1_BFE1,32769,8,SA1_NULL,S1_BFGLAND3,0,0},	// S1_BFGLAND2
	{SPR1_BFE1,32770,8,SA1_BFGSpray,S1_BFGLAND4,0,0},	// S1_BFGLAND3
	{SPR1_BFE1,32771,8,SA1_NULL,S1_BFGLAND5,0,0},	// S1_BFGLAND4
	{SPR1_BFE1,32772,8,SA1_NULL,S1_BFGLAND6,0,0},	// S1_BFGLAND5
	{SPR1_BFE1,32773,8,SA1_NULL,S1_NULL,0,0},	// S1_BFGLAND6
	{SPR1_BFE2,32768,8,SA1_NULL,S1_BFGEXP2,0,0},	// S1_BFGEXP
	{SPR1_BFE2,32769,8,SA1_NULL,S1_BFGEXP3,0,0},	// S1_BFGEXP2
	{SPR1_BFE2,32770,8,SA1_NULL,S1_BFGEXP4,0,0},	// S1_BFGEXP3
	{SPR1_BFE2,32771,8,SA1_NULL,S1_NULL,0,0},	// S1_BFGEXP4
	{SPR1_MISL,32769,8,SA1_Explode,S1_EXPLODE2,0,0},	// S1_EXPLODE1
	{SPR1_MISL,32770,6,SA1_NULL,S1_EXPLODE3,0,0},	// S1_EXPLODE2
	{SPR1_MISL,32771,4,SA1_NULL,S1_NULL,0,0},	// S1_EXPLODE3
	{SPR1_TFOG,32768,6,SA1_NULL,S1_TFOG01,0,0},	// S1_TFOG
	{SPR1_TFOG,32769,6,SA1_NULL,S1_TFOG02,0,0},	// S1_TFOG01
	{SPR1_TFOG,32768,6,SA1_NULL,S1_TFOG2,0,0},	// S1_TFOG02
	{SPR1_TFOG,32769,6,SA1_NULL,S1_TFOG3,0,0},	// S1_TFOG2
	{SPR1_TFOG,32770,6,SA1_NULL,S1_TFOG4,0,0},	// S1_TFOG3
	{SPR1_TFOG,32771,6,SA1_NULL,S1_TFOG5,0,0},	// S1_TFOG4
	{SPR1_TFOG,32772,6,SA1_NULL,S1_TFOG6,0,0},	// S1_TFOG5
	{SPR1_TFOG,32773,6,SA1_NULL,S1_TFOG7,0,0},	// S1_TFOG6
	{SPR1_TFOG,32774,6,SA1_NULL,S1_TFOG8,0,0},	// S1_TFOG7
	{SPR1_TFOG,32775,6,SA1_NULL,S1_TFOG9,0,0},	// S1_TFOG8
	{SPR1_TFOG,32776,6,SA1_NULL,S1_TFOG10,0,0},	// S1_TFOG9
	{SPR1_TFOG,32777,6,SA1_NULL,S1_NULL,0,0},	// S1_TFOG10
	{SPR1_IFOG,32768,6,SA1_NULL,S1_IFOG01,0,0},	// S1_IFOG
	{SPR1_IFOG,32769,6,SA1_NULL,S1_IFOG02,0,0},	// S1_IFOG01
	{SPR1_IFOG,32768,6,SA1_NULL,S1_IFOG2,0,0},	// S1_IFOG02
	{SPR1_IFOG,32769,6,SA1_NULL,S1_IFOG3,0,0},	// S1_IFOG2
	{SPR1_IFOG,32770,6,SA1_NULL,S1_IFOG4,0,0},	// S1_IFOG3
	{SPR1_IFOG,32771,6,SA1_NULL,S1_IFOG5,0,0},	// S1_IFOG4
	{SPR1_IFOG,32772,6,SA1_NULL,S1_NULL,0,0},	// S1_IFOG5
	{SPR1_PLAY,0,-1,SA1_NULL,S1_NULL,0,0,"players/doomguy/tris.md2",0},	// S1_PLAY
	{SPR1_PLAY,0,4,SA1_NULL,S1_PLAY_RUN2,0,0,"players/doomguy/tris.md2",1},	// S1_PLAY_RUN1
	{SPR1_PLAY,1,4,SA1_NULL,S1_PLAY_RUN3,0,0,"players/doomguy/tris.md2",2},	// S1_PLAY_RUN2
	{SPR1_PLAY,2,4,SA1_NULL,S1_PLAY_RUN4,0,0,"players/doomguy/tris.md2",3},	// S1_PLAY_RUN3
	{SPR1_PLAY,3,4,SA1_NULL,S1_PLAY_RUN1,0,0,"players/doomguy/tris.md2",4},	// S1_PLAY_RUN4
	{SPR1_PLAY,4,12,SA1_NULL,S1_PLAY,0,0,"players/doomguy/tris.md2",5},	// S1_PLAY_ATK1
	{SPR1_PLAY,32773,6,SA1_NULL,S1_PLAY_ATK1,0,0,"players/doomguy/tris.md2",6},	// S1_PLAY_ATK2
	{SPR1_PLAY,6,4,SA1_NULL,S1_PLAY_PAIN2,0,0,"players/doomguy/tris.md2",7},	// S1_PLAY_PAIN
	{SPR1_PLAY,6,4,SA1_Pain,S1_PLAY,0,0,"players/doomguy/tris.md2",8},	// S1_PLAY_PAIN2
	{SPR1_PLAY,7,10,SA1_NULL,S1_PLAY_DIE2,0,0,"players/doomguy/tris.md2",9},	// S1_PLAY_DIE1
	{SPR1_PLAY,8,10,SA1_PlayerScream,S1_PLAY_DIE3,0,0,"players/doomguy/tris.md2",10},	// S1_PLAY_DIE2
	{SPR1_PLAY,9,10,SA1_Fall,S1_PLAY_DIE4,0,0,"players/doomguy/tris.md2",11},	// S1_PLAY_DIE3
	{SPR1_PLAY,10,10,SA1_NULL,S1_PLAY_DIE5,0,0,"players/doomguy/tris.md2",12},	// S1_PLAY_DIE4
	{SPR1_PLAY,11,10,SA1_NULL,S1_PLAY_DIE6,0,0,"players/doomguy/tris.md2",13},	// S1_PLAY_DIE5
	{SPR1_PLAY,12,10,SA1_NULL,S1_PLAY_DIE7,0,0,"players/doomguy/tris.md2",14},	// S1_PLAY_DIE6
	{SPR1_PLAY,13,-1,SA1_NULL,S1_NULL,0,0,"players/doomguy/tris.md2",15},	// S1_PLAY_DIE7
	{SPR1_PLAY,14,5,SA1_NULL,S1_PLAY_XDIE2,0,0,"players/doomguy/xplode.md2",0},	// S1_PLAY_XDIE1
	{SPR1_PLAY,15,5,SA1_XScream,S1_PLAY_XDIE3,0,0,"players/doomguy/xplode.md2",1},	// S1_PLAY_XDIE2
	{SPR1_PLAY,16,5,SA1_Fall,S1_PLAY_XDIE4,0,0,"players/doomguy/xplode.md2",2},	// S1_PLAY_XDIE3
	{SPR1_PLAY,17,5,SA1_NULL,S1_PLAY_XDIE5,0,0,"players/doomguy/xplode.md2",3},	// S1_PLAY_XDIE4
	{SPR1_PLAY,18,5,SA1_NULL,S1_PLAY_XDIE6,0,0,"players/doomguy/xplode.md2",4},	// S1_PLAY_XDIE5
	{SPR1_PLAY,19,5,SA1_NULL,S1_PLAY_XDIE7,0,0,"players/doomguy/xplode.md2",5},	// S1_PLAY_XDIE6
	{SPR1_PLAY,20,5,SA1_NULL,S1_PLAY_XDIE8,0,0,"players/doomguy/xplode.md2",6},	// S1_PLAY_XDIE7
	{SPR1_PLAY,21,5,SA1_NULL,S1_PLAY_XDIE9,0,0,"players/doomguy/xplode.md2",7},	// S1_PLAY_XDIE8
	{SPR1_PLAY,22,-1,SA1_NULL,S1_NULL,0,0,"players/doomguy/xplode.md2",8},	// S1_PLAY_XDIE9
	{SPR1_POSS,0,10,SA1_Look,S1_POSS_STND2,0,0,"monsters/trooper.md2",0},	// S1_POSS_STND
	{SPR1_POSS,1,10,SA1_Look,S1_POSS_STND,0,0,"monsters/trooper.md2",1},	// S1_POSS_STND2
	{SPR1_POSS,0,4,SA1_Chase,S1_POSS_RUN2,0,0,"monsters/trooper.md2",2},	// S1_POSS_RUN1
	{SPR1_POSS,0,4,SA1_Chase,S1_POSS_RUN3,0,0,"monsters/trooper.md2",3},	// S1_POSS_RUN2
	{SPR1_POSS,1,4,SA1_Chase,S1_POSS_RUN4,0,0,"monsters/trooper.md2",4},	// S1_POSS_RUN3
	{SPR1_POSS,1,4,SA1_Chase,S1_POSS_RUN5,0,0,"monsters/trooper.md2",5},	// S1_POSS_RUN4
	{SPR1_POSS,2,4,SA1_Chase,S1_POSS_RUN6,0,0,"monsters/trooper.md2",6},	// S1_POSS_RUN5
	{SPR1_POSS,2,4,SA1_Chase,S1_POSS_RUN7,0,0,"monsters/trooper.md2",7},	// S1_POSS_RUN6
	{SPR1_POSS,3,4,SA1_Chase,S1_POSS_RUN8,0,0,"monsters/trooper.md2",8},	// S1_POSS_RUN7
	{SPR1_POSS,3,4,SA1_Chase,S1_POSS_RUN1,0,0,"monsters/trooper.md2",9},	// S1_POSS_RUN8
	{SPR1_POSS,4,10,SA1_FaceTarget,S1_POSS_ATK2,0,0,"monsters/trooper.md2",10},	// S1_POSS_ATK1
	{SPR1_POSS,5,8,SA1_PosAttack,S1_POSS_ATK3,0,0,"monsters/trooper.md2",11},	// S1_POSS_ATK2
	{SPR1_POSS,4,8,SA1_NULL,S1_POSS_RUN1,0,0,"monsters/trooper.md2",12},	// S1_POSS_ATK3
	{SPR1_POSS,6,3,SA1_NULL,S1_POSS_PAIN2,0,0,"monsters/trooper.md2",13},	// S1_POSS_PAIN
	{SPR1_POSS,6,3,SA1_Pain,S1_POSS_RUN1,0,0,"monsters/trooper.md2",14},	// S1_POSS_PAIN2
	{SPR1_POSS,7,5,SA1_NULL,S1_POSS_DIE2,0,0,"monsters/troopdie.md2",0},	// S1_POSS_DIE1
	{SPR1_POSS,8,5,SA1_Scream,S1_POSS_DIE3,0,0,"monsters/troopdie.md2",1},	// S1_POSS_DIE2
	{SPR1_POSS,9,5,SA1_Fall,S1_POSS_DIE4,0,0,"monsters/troopdie.md2",2},	// S1_POSS_DIE3
	{SPR1_POSS,10,5,SA1_NULL,S1_POSS_DIE5,0,0,"monsters/troopdie.md2",3},	// S1_POSS_DIE4
	{SPR1_POSS,11,-1,SA1_NULL,S1_NULL,0,0,"monsters/troopdie.md2",4},	// S1_POSS_DIE5
	{SPR1_POSS,12,5,SA1_NULL,S1_POSS_XDIE2,0,0,"gibs/troopgib.md2",0},	// S1_POSS_XDIE1
	{SPR1_POSS,13,5,SA1_XScream,S1_POSS_XDIE3,0,0,"gibs/troopgib.md2",1},	// S1_POSS_XDIE2
	{SPR1_POSS,14,5,SA1_Fall,S1_POSS_XDIE4,0,0,"gibs/troopgib.md2",2},	// S1_POSS_XDIE3
	{SPR1_POSS,15,5,SA1_NULL,S1_POSS_XDIE5,0,0,"gibs/troopgib.md2",3},	// S1_POSS_XDIE4
	{SPR1_POSS,16,5,SA1_NULL,S1_POSS_XDIE6,0,0,"gibs/troopgib.md2",4},	// S1_POSS_XDIE5
	{SPR1_POSS,17,5,SA1_NULL,S1_POSS_XDIE7,0,0,"gibs/troopgib.md2",5},	// S1_POSS_XDIE6
	{SPR1_POSS,18,5,SA1_NULL,S1_POSS_XDIE8,0,0,"gibs/troopgib.md2",6},	// S1_POSS_XDIE7
	{SPR1_POSS,19,5,SA1_NULL,S1_POSS_XDIE9,0,0,"gibs/troopgib.md2",7},	// S1_POSS_XDIE8
	{SPR1_POSS,20,-1,SA1_NULL,S1_NULL,0,0,"gibs/troopgib.md2",8},	// S1_POSS_XDIE9
	{SPR1_POSS,10,5,SA1_NULL,S1_POSS_RAISE2,0,0,"monsters/troopdie.md2",5},	// S1_POSS_RAISE1
	{SPR1_POSS,9,5,SA1_NULL,S1_POSS_RAISE3,0,0,"monsters/troopdie.md2",6},	// S1_POSS_RAISE2
	{SPR1_POSS,8,5,SA1_NULL,S1_POSS_RAISE4,0,0,"monsters/troopdie.md2",7},	// S1_POSS_RAISE3
	{SPR1_POSS,7,5,SA1_NULL,S1_POSS_RUN1,0,0,"monsters/troopdie.md2",8},	// S1_POSS_RAISE4
	{SPR1_SPOS,0,10,SA1_Look,S1_SPOS_STND2,0,0,"monsters/sargeant.md2",0},	// S1_SPOS_STND
	{SPR1_SPOS,1,10,SA1_Look,S1_SPOS_STND,0,0,"monsters/sargeant.md2",1},	// S1_SPOS_STND2
	{SPR1_SPOS,0,3,SA1_Chase,S1_SPOS_RUN2,0,0,"monsters/sargeant.md2",2},	// S1_SPOS_RUN1
	{SPR1_SPOS,0,3,SA1_Chase,S1_SPOS_RUN3,0,0,"monsters/sargeant.md2",3},	// S1_SPOS_RUN2
	{SPR1_SPOS,1,3,SA1_Chase,S1_SPOS_RUN4,0,0,"monsters/sargeant.md2",4},	// S1_SPOS_RUN3
	{SPR1_SPOS,1,3,SA1_Chase,S1_SPOS_RUN5,0,0,"monsters/sargeant.md2",5},	// S1_SPOS_RUN4
	{SPR1_SPOS,2,3,SA1_Chase,S1_SPOS_RUN6,0,0,"monsters/sargeant.md2",6},	// S1_SPOS_RUN5
	{SPR1_SPOS,2,3,SA1_Chase,S1_SPOS_RUN7,0,0,"monsters/sargeant.md2",7},	// S1_SPOS_RUN6
	{SPR1_SPOS,3,3,SA1_Chase,S1_SPOS_RUN8,0,0,"monsters/sargeant.md2",8},	// S1_SPOS_RUN7
	{SPR1_SPOS,3,3,SA1_Chase,S1_SPOS_RUN1,0,0,"monsters/sargeant.md2",9},	// S1_SPOS_RUN8
	{SPR1_SPOS,4,10,SA1_FaceTarget,S1_SPOS_ATK2,0,0,"monsters/sargeant.md2",10},	// S1_SPOS_ATK1
	{SPR1_SPOS,32773,10,SA1_SPosAttack,S1_SPOS_ATK3,0,0,"monsters/sargeant.md2",11},	// S1_SPOS_ATK2
	{SPR1_SPOS,4,10,SA1_NULL,S1_SPOS_RUN1,0,0,"monsters/sargeant.md2",12},	// S1_SPOS_ATK3
	{SPR1_SPOS,6,3,SA1_NULL,S1_SPOS_PAIN2,0,0,"monsters/sargeant.md2",13},	// S1_SPOS_PAIN
	{SPR1_SPOS,6,3,SA1_Pain,S1_SPOS_RUN1,0,0,"monsters/sargeant.md2",14},	// S1_SPOS_PAIN2
	{SPR1_SPOS,7,5,SA1_NULL,S1_SPOS_DIE2,0,0,"monsters/sargdie.md2",0},	// S1_SPOS_DIE1
	{SPR1_SPOS,8,5,SA1_Scream,S1_SPOS_DIE3,0,0,"monsters/sargdie.md2",1},	// S1_SPOS_DIE2
	{SPR1_SPOS,9,5,SA1_Fall,S1_SPOS_DIE4,0,0,"monsters/sargdie.md2",2},	// S1_SPOS_DIE3
	{SPR1_SPOS,10,5,SA1_NULL,S1_SPOS_DIE5,0,0,"monsters/sargdie.md2",3},	// S1_SPOS_DIE4
	{SPR1_SPOS,11,-1,SA1_NULL,S1_NULL,0,0,"monsters/sargdie.md2",4},	// S1_SPOS_DIE5
	{SPR1_SPOS,12,5,SA1_NULL,S1_SPOS_XDIE2,0,0,"gibs/sarggib.md2",0},	// S1_SPOS_XDIE1
	{SPR1_SPOS,13,5,SA1_XScream,S1_SPOS_XDIE3,0,0,"gibs/sarggib.md2",1},	// S1_SPOS_XDIE2
	{SPR1_SPOS,14,5,SA1_Fall,S1_SPOS_XDIE4,0,0,"gibs/sarggib.md2",2},	// S1_SPOS_XDIE3
	{SPR1_SPOS,15,5,SA1_NULL,S1_SPOS_XDIE5,0,0,"gibs/sarggib.md2",3},	// S1_SPOS_XDIE4
	{SPR1_SPOS,16,5,SA1_NULL,S1_SPOS_XDIE6,0,0,"gibs/sarggib.md2",4},	// S1_SPOS_XDIE5
	{SPR1_SPOS,17,5,SA1_NULL,S1_SPOS_XDIE7,0,0,"gibs/sarggib.md2",5},	// S1_SPOS_XDIE6
	{SPR1_SPOS,18,5,SA1_NULL,S1_SPOS_XDIE8,0,0,"gibs/sarggib.md2",6},	// S1_SPOS_XDIE7
	{SPR1_SPOS,19,5,SA1_NULL,S1_SPOS_XDIE9,0,0,"gibs/sarggib.md2",7},	// S1_SPOS_XDIE8
	{SPR1_SPOS,20,-1,SA1_NULL,S1_NULL,0,0,"gibs/sarggib.md2",8},	// S1_SPOS_XDIE9
	{SPR1_SPOS,11,5,SA1_NULL,S1_SPOS_RAISE2,0,0,"monsters/sargdie.md2",5},	// S1_SPOS_RAISE1
	{SPR1_SPOS,10,5,SA1_NULL,S1_SPOS_RAISE3,0,0,"monsters/sargdie.md2",6},	// S1_SPOS_RAISE2
	{SPR1_SPOS,9,5,SA1_NULL,S1_SPOS_RAISE4,0,0,"monsters/sargdie.md2",7},	// S1_SPOS_RAISE3
	{SPR1_SPOS,8,5,SA1_NULL,S1_SPOS_RAISE5,0,0,"monsters/sargdie.md2",8},	// S1_SPOS_RAISE4
	{SPR1_SPOS,7,5,SA1_NULL,S1_SPOS_RUN1,0,0,"monsters/sargdie.md2",9},	// S1_SPOS_RAISE5
	{SPR1_VILE,0,10,SA1_Look,S1_VILE_STND2,0,0,"monsters/vile.md2",0},	// S1_VILE_STND
	{SPR1_VILE,1,10,SA1_Look,S1_VILE_STND,0,0,"monsters/vile.md2",1},	// S1_VILE_STND2
	{SPR1_VILE,0,2,SA1_VileChase,S1_VILE_RUN2,0,0,"monsters/vile.md2",2},	// S1_VILE_RUN1
	{SPR1_VILE,0,2,SA1_VileChase,S1_VILE_RUN3,0,0,"monsters/vile.md2",3},	// S1_VILE_RUN2
	{SPR1_VILE,1,2,SA1_VileChase,S1_VILE_RUN4,0,0,"monsters/vile.md2",4},	// S1_VILE_RUN3
	{SPR1_VILE,1,2,SA1_VileChase,S1_VILE_RUN5,0,0,"monsters/vile.md2",5},	// S1_VILE_RUN4
	{SPR1_VILE,2,2,SA1_VileChase,S1_VILE_RUN6,0,0,"monsters/vile.md2",6},	// S1_VILE_RUN5
	{SPR1_VILE,2,2,SA1_VileChase,S1_VILE_RUN7,0,0,"monsters/vile.md2",7},	// S1_VILE_RUN6
	{SPR1_VILE,3,2,SA1_VileChase,S1_VILE_RUN8,0,0,"monsters/vile.md2",8},	// S1_VILE_RUN7
	{SPR1_VILE,3,2,SA1_VileChase,S1_VILE_RUN9,0,0,"monsters/vile.md2",9},	// S1_VILE_RUN8
	{SPR1_VILE,4,2,SA1_VileChase,S1_VILE_RUN10,0,0,"monsters/vile.md2",10},	// S1_VILE_RUN9
	{SPR1_VILE,4,2,SA1_VileChase,S1_VILE_RUN11,0,0,"monsters/vile.md2",11},	// S1_VILE_RUN10
	{SPR1_VILE,5,2,SA1_VileChase,S1_VILE_RUN12,0,0,"monsters/vile.md2",12},	// S1_VILE_RUN11
	{SPR1_VILE,5,2,SA1_VileChase,S1_VILE_RUN1,0,0,"monsters/vile.md2",13},	// S1_VILE_RUN12
	{SPR1_VILE,32774,0,SA1_VileStart,S1_VILE_ATK2,0,0,"monsters/vileatk.md2",0},	// S1_VILE_ATK1
	{SPR1_VILE,32774,10,SA1_FaceTarget,S1_VILE_ATK3,0,0,"monsters/vileatk.md2",0},	// S1_VILE_ATK2
	{SPR1_VILE,32775,8,SA1_VileTarget,S1_VILE_ATK4,0,0,"monsters/vileatk.md2",1},	// S1_VILE_ATK3
	{SPR1_VILE,32776,8,SA1_FaceTarget,S1_VILE_ATK5,0,0,"monsters/vileatk.md2",2},	// S1_VILE_ATK4
	{SPR1_VILE,32777,8,SA1_FaceTarget,S1_VILE_ATK6,0,0,"monsters/vileatk.md2",3},	// S1_VILE_ATK5
	{SPR1_VILE,32778,8,SA1_FaceTarget,S1_VILE_ATK7,0,0,"monsters/vileatk.md2",4},	// S1_VILE_ATK6
	{SPR1_VILE,32779,8,SA1_FaceTarget,S1_VILE_ATK8,0,0,"monsters/vileatk.md2",5},	// S1_VILE_ATK7
	{SPR1_VILE,32780,8,SA1_FaceTarget,S1_VILE_ATK9,0,0,"monsters/vileatk.md2",6},	// S1_VILE_ATK8
	{SPR1_VILE,32781,8,SA1_FaceTarget,S1_VILE_ATK10,0,0,"monsters/vileatk.md2",7},	// S1_VILE_ATK9
	{SPR1_VILE,32782,8,SA1_VileAttack,S1_VILE_ATK11,0,0,"monsters/vileatk.md2",8},	// S1_VILE_ATK10
	{SPR1_VILE,32783,20,SA1_NULL,S1_VILE_RUN1,0,0,"monsters/vileatk.md2",9},	// S1_VILE_ATK11
	{SPR1_VILE,32794,10,SA1_NULL,S1_VILE_HEAL2,0,0,"monsters/vile.md2",14},	// S1_VILE_HEAL1
	{SPR1_VILE,32795,10,SA1_NULL,S1_VILE_HEAL3,0,0,"monsters/vile.md2",15},	// S1_VILE_HEAL2
	{SPR1_VILE,32796,10,SA1_NULL,S1_VILE_RUN1,0,0,"monsters/vile.md2",16},	// S1_VILE_HEAL3
	{SPR1_VILE,16,5,SA1_NULL,S1_VILE_PAIN2,0,0,"monsters/vile.md2",17},	// S1_VILE_PAIN
	{SPR1_VILE,16,5,SA1_Pain,S1_VILE_RUN1,0,0,"monsters/vile.md2",18},	// S1_VILE_PAIN2
	{SPR1_VILE,16,7,SA1_NULL,S1_VILE_DIE2,0,0,"monsters/viledie.md2",0},	// S1_VILE_DIE1
	{SPR1_VILE,17,7,SA1_Scream,S1_VILE_DIE3,0,0,"monsters/viledie.md2",1},	// S1_VILE_DIE2
	{SPR1_VILE,18,7,SA1_Fall,S1_VILE_DIE4,0,0,"monsters/viledie.md2",2},	// S1_VILE_DIE3
	{SPR1_VILE,19,7,SA1_NULL,S1_VILE_DIE5,0,0,"monsters/viledie.md2",3},	// S1_VILE_DIE4
	{SPR1_VILE,20,7,SA1_NULL,S1_VILE_DIE6,0,0,"monsters/viledie.md2",4},	// S1_VILE_DIE5
	{SPR1_VILE,21,7,SA1_NULL,S1_VILE_DIE7,0,0,"monsters/viledie.md2",5},	// S1_VILE_DIE6
	{SPR1_VILE,22,7,SA1_NULL,S1_VILE_DIE8,0,0,"monsters/viledie.md2",6},	// S1_VILE_DIE7
	{SPR1_VILE,23,5,SA1_NULL,S1_VILE_DIE9,0,0,"monsters/viledie.md2",7},	// S1_VILE_DIE8
	{SPR1_VILE,24,5,SA1_NULL,S1_VILE_DIE10,0,0,"monsters/viledie.md2",8},	// S1_VILE_DIE9
	{SPR1_VILE,25,-1,SA1_NULL,S1_NULL,0,0,"monsters/viledie.md2",9},	// S1_VILE_DIE10
	{SPR1_FIRE,32768,2,SA1_StartFire,S1_FIRE2,0,0},	// S1_FIRE1
	{SPR1_FIRE,32769,2,SA1_Fire,S1_FIRE3,0,0},	// S1_FIRE2
	{SPR1_FIRE,32768,2,SA1_Fire,S1_FIRE4,0,0},	// S1_FIRE3
	{SPR1_FIRE,32769,2,SA1_Fire,S1_FIRE5,0,0},	// S1_FIRE4
	{SPR1_FIRE,32770,2,SA1_FireCrackle,S1_FIRE6,0,0},	// S1_FIRE5
	{SPR1_FIRE,32769,2,SA1_Fire,S1_FIRE7,0,0},	// S1_FIRE6
	{SPR1_FIRE,32770,2,SA1_Fire,S1_FIRE8,0,0},	// S1_FIRE7
	{SPR1_FIRE,32769,2,SA1_Fire,S1_FIRE9,0,0},	// S1_FIRE8
	{SPR1_FIRE,32770,2,SA1_Fire,S1_FIRE10,0,0},	// S1_FIRE9
	{SPR1_FIRE,32771,2,SA1_Fire,S1_FIRE11,0,0},	// S1_FIRE10
	{SPR1_FIRE,32770,2,SA1_Fire,S1_FIRE12,0,0},	// S1_FIRE11
	{SPR1_FIRE,32771,2,SA1_Fire,S1_FIRE13,0,0},	// S1_FIRE12
	{SPR1_FIRE,32770,2,SA1_Fire,S1_FIRE14,0,0},	// S1_FIRE13
	{SPR1_FIRE,32771,2,SA1_Fire,S1_FIRE15,0,0},	// S1_FIRE14
	{SPR1_FIRE,32772,2,SA1_Fire,S1_FIRE16,0,0},	// S1_FIRE15
	{SPR1_FIRE,32771,2,SA1_Fire,S1_FIRE17,0,0},	// S1_FIRE16
	{SPR1_FIRE,32772,2,SA1_Fire,S1_FIRE18,0,0},	// S1_FIRE17
	{SPR1_FIRE,32771,2,SA1_Fire,S1_FIRE19,0,0},	// S1_FIRE18
	{SPR1_FIRE,32772,2,SA1_FireCrackle,S1_FIRE20,0,0},	// S1_FIRE19
	{SPR1_FIRE,32773,2,SA1_Fire,S1_FIRE21,0,0},	// S1_FIRE20
	{SPR1_FIRE,32772,2,SA1_Fire,S1_FIRE22,0,0},	// S1_FIRE21
	{SPR1_FIRE,32773,2,SA1_Fire,S1_FIRE23,0,0},	// S1_FIRE22
	{SPR1_FIRE,32772,2,SA1_Fire,S1_FIRE24,0,0},	// S1_FIRE23
	{SPR1_FIRE,32773,2,SA1_Fire,S1_FIRE25,0,0},	// S1_FIRE24
	{SPR1_FIRE,32774,2,SA1_Fire,S1_FIRE26,0,0},	// S1_FIRE25
	{SPR1_FIRE,32775,2,SA1_Fire,S1_FIRE27,0,0},	// S1_FIRE26
	{SPR1_FIRE,32774,2,SA1_Fire,S1_FIRE28,0,0},	// S1_FIRE27
	{SPR1_FIRE,32775,2,SA1_Fire,S1_FIRE29,0,0},	// S1_FIRE28
	{SPR1_FIRE,32774,2,SA1_Fire,S1_FIRE30,0,0},	// S1_FIRE29
	{SPR1_FIRE,32775,2,SA1_Fire,S1_NULL,0,0},	// S1_FIRE30
	{SPR1_PUFF,1,4,SA1_NULL,S1_SMOKE2,0,0},	// S1_SMOKE1
	{SPR1_PUFF,2,4,SA1_NULL,S1_SMOKE3,0,0},	// S1_SMOKE2
	{SPR1_PUFF,1,4,SA1_NULL,S1_SMOKE4,0,0},	// S1_SMOKE3
	{SPR1_PUFF,2,4,SA1_NULL,S1_SMOKE5,0,0},	// S1_SMOKE4
	{SPR1_PUFF,3,4,SA1_NULL,S1_NULL,0,0},	// S1_SMOKE5
	{SPR1_FATB,32768,2,SA1_Tracer,S1_TRACER2,0,0},	// S1_TRACER
	{SPR1_FATB,32769,2,SA1_Tracer,S1_TRACER,0,0},	// S1_TRACER2
	{SPR1_FBXP,32768,8,SA1_NULL,S1_TRACEEXP2,0,0},	// S1_TRACEEXP1
	{SPR1_FBXP,32769,6,SA1_NULL,S1_TRACEEXP3,0,0},	// S1_TRACEEXP2
	{SPR1_FBXP,32770,4,SA1_NULL,S1_NULL,0,0},	// S1_TRACEEXP3
	{SPR1_SKEL,0,10,SA1_Look,S1_SKEL_STND2,0,0,"monsters/revenant.md2",0},	// S1_SKEL_STND
	{SPR1_SKEL,1,10,SA1_Look,S1_SKEL_STND,0,0,"monsters/revenant.md2",1},	// S1_SKEL_STND2
	{SPR1_SKEL,0,2,SA1_Chase,S1_SKEL_RUN2,0,0,"monsters/revenant.md2",2},	// S1_SKEL_RUN1
	{SPR1_SKEL,0,2,SA1_Chase,S1_SKEL_RUN3,0,0,"monsters/revenant.md2",3},	// S1_SKEL_RUN2
	{SPR1_SKEL,1,2,SA1_Chase,S1_SKEL_RUN4,0,0,"monsters/revenant.md2",4},	// S1_SKEL_RUN3
	{SPR1_SKEL,1,2,SA1_Chase,S1_SKEL_RUN5,0,0,"monsters/revenant.md2",5},	// S1_SKEL_RUN4
	{SPR1_SKEL,2,2,SA1_Chase,S1_SKEL_RUN6,0,0,"monsters/revenant.md2",6},	// S1_SKEL_RUN5
	{SPR1_SKEL,2,2,SA1_Chase,S1_SKEL_RUN7,0,0,"monsters/revenant.md2",7},	// S1_SKEL_RUN6
	{SPR1_SKEL,3,2,SA1_Chase,S1_SKEL_RUN8,0,0,"monsters/revenant.md2",8},	// S1_SKEL_RUN7
	{SPR1_SKEL,3,2,SA1_Chase,S1_SKEL_RUN9,0,0,"monsters/revenant.md2",9},	// S1_SKEL_RUN8
	{SPR1_SKEL,4,2,SA1_Chase,S1_SKEL_RUN10,0,0,"monsters/revenant.md2",10},	// S1_SKEL_RUN9
	{SPR1_SKEL,4,2,SA1_Chase,S1_SKEL_RUN11,0,0,"monsters/revenant.md2",11},	// S1_SKEL_RUN10
	{SPR1_SKEL,5,2,SA1_Chase,S1_SKEL_RUN12,0,0,"monsters/revenant.md2",12},	// S1_SKEL_RUN11
	{SPR1_SKEL,5,2,SA1_Chase,S1_SKEL_RUN1,0,0,"monsters/revenant.md2",13},	// S1_SKEL_RUN12
	{SPR1_SKEL,6,0,SA1_FaceTarget,S1_SKEL_FIST2,0,0,"monsters/revenant.md2",14},	// S1_SKEL_FIST1
	{SPR1_SKEL,6,6,SA1_SkelWhoosh,S1_SKEL_FIST3,0,0,"monsters/revenant.md2",14},	// S1_SKEL_FIST2
	{SPR1_SKEL,7,6,SA1_FaceTarget,S1_SKEL_FIST4,0,0,"monsters/revenant.md2",15},	// S1_SKEL_FIST3
	{SPR1_SKEL,8,6,SA1_SkelFist,S1_SKEL_RUN1,0,0,"monsters/revenant.md2",16},	// S1_SKEL_FIST4
	{SPR1_SKEL,32777,0,SA1_FaceTarget,S1_SKEL_MISS2,0,0,"monsters/revenant.md2",17},	// S1_SKEL_MISS1
	{SPR1_SKEL,32777,10,SA1_FaceTarget,S1_SKEL_MISS3,0,0,"monsters/revenant.md2",17},	// S1_SKEL_MISS2
	{SPR1_SKEL,10,10,SA1_SkelMissile,S1_SKEL_MISS4,0,0,"monsters/revenant.md2",18},	// S1_SKEL_MISS3
	{SPR1_SKEL,10,10,SA1_FaceTarget,S1_SKEL_RUN1,0,0,"monsters/revenant.md2",19},	// S1_SKEL_MISS4
	{SPR1_SKEL,11,5,SA1_NULL,S1_SKEL_PAIN2,0,0,"monsters/revenant.md2",20},	// S1_SKEL_PAIN
	{SPR1_SKEL,11,5,SA1_Pain,S1_SKEL_RUN1,0,0,"monsters/revenant.md2",21},	// S1_SKEL_PAIN2
	{SPR1_SKEL,11,7,SA1_NULL,S1_SKEL_DIE2,0,0,"monsters/revdie.md2",0},	// S1_SKEL_DIE1
	{SPR1_SKEL,12,7,SA1_NULL,S1_SKEL_DIE3,0,0,"monsters/revdie.md2",1},	// S1_SKEL_DIE2
	{SPR1_SKEL,13,7,SA1_Scream,S1_SKEL_DIE4,0,0,"monsters/revdie.md2",2},	// S1_SKEL_DIE3
	{SPR1_SKEL,14,7,SA1_Fall,S1_SKEL_DIE5,0,0,"monsters/revdie.md2",3},	// S1_SKEL_DIE4
	{SPR1_SKEL,15,7,SA1_NULL,S1_SKEL_DIE6,0,0,"monsters/revdie.md2",4},	// S1_SKEL_DIE5
	{SPR1_SKEL,16,-1,SA1_NULL,S1_NULL,0,0,"monsters/revdie.md2",5},	// S1_SKEL_DIE6
	{SPR1_SKEL,16,5,SA1_NULL,S1_SKEL_RAISE2,0,0,"monsters/revdie.md2",6},	// S1_SKEL_RAISE1
	{SPR1_SKEL,15,5,SA1_NULL,S1_SKEL_RAISE3,0,0,"monsters/revdie.md2",7},	// S1_SKEL_RAISE2
	{SPR1_SKEL,14,5,SA1_NULL,S1_SKEL_RAISE4,0,0,"monsters/revdie.md2",8},	// S1_SKEL_RAISE3
	{SPR1_SKEL,13,5,SA1_NULL,S1_SKEL_RAISE5,0,0,"monsters/revdie.md2",9},	// S1_SKEL_RAISE4
	{SPR1_SKEL,12,5,SA1_NULL,S1_SKEL_RAISE6,0,0,"monsters/revdie.md2",10},	// S1_SKEL_RAISE5
	{SPR1_SKEL,11,5,SA1_NULL,S1_SKEL_RUN1,0,0,"monsters/revdie.md2",11},	// S1_SKEL_RAISE6
	{SPR1_MANF,32768,4,SA1_NULL,S1_FATSHOT2,0,0},	// S1_FATSHOT1
	{SPR1_MANF,32769,4,SA1_NULL,S1_FATSHOT1,0,0},	// S1_FATSHOT2
	{SPR1_MISL,32769,8,SA1_NULL,S1_FATSHOTX2,0,0},	// S1_FATSHOTX1
	{SPR1_MISL,32770,6,SA1_NULL,S1_FATSHOTX3,0,0},	// S1_FATSHOTX2
	{SPR1_MISL,32771,4,SA1_NULL,S1_NULL,0,0},	// S1_FATSHOTX3
	{SPR1_FATT,0,15,SA1_Look,S1_FATT_STND2,0,0,"monsters/mancubus.md2",0},	// S1_FATT_STND
	{SPR1_FATT,1,15,SA1_Look,S1_FATT_STND,0,0,"monsters/mancubus.md2",1},	// S1_FATT_STND2
	{SPR1_FATT,0,4,SA1_Chase,S1_FATT_RUN2,0,0,"monsters/mancubus.md2",2},	// S1_FATT_RUN1
	{SPR1_FATT,0,4,SA1_Chase,S1_FATT_RUN3,0,0,"monsters/mancubus.md2",3},	// S1_FATT_RUN2
	{SPR1_FATT,1,4,SA1_Chase,S1_FATT_RUN4,0,0,"monsters/mancubus.md2",4},	// S1_FATT_RUN3
	{SPR1_FATT,1,4,SA1_Chase,S1_FATT_RUN5,0,0,"monsters/mancubus.md2",5},	// S1_FATT_RUN4
	{SPR1_FATT,2,4,SA1_Chase,S1_FATT_RUN6,0,0,"monsters/mancubus.md2",6},	// S1_FATT_RUN5
	{SPR1_FATT,2,4,SA1_Chase,S1_FATT_RUN7,0,0,"monsters/mancubus.md2",7},	// S1_FATT_RUN6
	{SPR1_FATT,3,4,SA1_Chase,S1_FATT_RUN8,0,0,"monsters/mancubus.md2",8},	// S1_FATT_RUN7
	{SPR1_FATT,3,4,SA1_Chase,S1_FATT_RUN9,0,0,"monsters/mancubus.md2",9},	// S1_FATT_RUN8
	{SPR1_FATT,4,4,SA1_Chase,S1_FATT_RUN10,0,0,"monsters/mancubus.md2",10},	// S1_FATT_RUN9
	{SPR1_FATT,4,4,SA1_Chase,S1_FATT_RUN11,0,0,"monsters/mancubus.md2",11},	// S1_FATT_RUN10
	{SPR1_FATT,5,4,SA1_Chase,S1_FATT_RUN12,0,0,"monsters/mancubus.md2",12},	// S1_FATT_RUN11
	{SPR1_FATT,5,4,SA1_Chase,S1_FATT_RUN1,0,0,"monsters/mancubus.md2",13},	// S1_FATT_RUN12
	{SPR1_FATT,6,20,SA1_FatRaise,S1_FATT_ATK2,0,0,"monsters/mancubus.md2",14},	// S1_FATT_ATK1
	{SPR1_FATT,32775,10,SA1_FatAttack1,S1_FATT_ATK3,0,0,"monsters/mancatk.md2",0},	// S1_FATT_ATK2
	{SPR1_FATT,8,5,SA1_FaceTarget,S1_FATT_ATK4,0,0,"monsters/mancubus.md2",15},	// S1_FATT_ATK3
	{SPR1_FATT,6,5,SA1_FaceTarget,S1_FATT_ATK5,0,0,"monsters/mancubus.md2",14},	// S1_FATT_ATK4
	{SPR1_FATT,32775,10,SA1_FatAttack2,S1_FATT_ATK6,0,0,"monsters/mancatk.md2",0},	// S1_FATT_ATK5
	{SPR1_FATT,8,5,SA1_FaceTarget,S1_FATT_ATK7,0,0,"monsters/mancubus.md2",15},	// S1_FATT_ATK6
	{SPR1_FATT,6,5,SA1_FaceTarget,S1_FATT_ATK8,0,0,"monsters/mancubus.md2",14},	// S1_FATT_ATK7
	{SPR1_FATT,32775,10,SA1_FatAttack3,S1_FATT_ATK9,0,0,"monsters/mancatk.md2",0},	// S1_FATT_ATK8
	{SPR1_FATT,8,5,SA1_FaceTarget,S1_FATT_ATK10,0,0,"monsters/mancubus.md2",15},	// S1_FATT_ATK9
	{SPR1_FATT,6,5,SA1_FaceTarget,S1_FATT_RUN1,0,0,"monsters/mancubus.md2",14},	// S1_FATT_ATK10
	{SPR1_FATT,9,3,SA1_NULL,S1_FATT_PAIN2,0,0,"monsters/mancubus.md2",16},	// S1_FATT_PAIN
	{SPR1_FATT,9,3,SA1_Pain,S1_FATT_RUN1,0,0,"monsters/mancubus.md2",17},	// S1_FATT_PAIN2
	{SPR1_FATT,10,6,SA1_NULL,S1_FATT_DIE2,0,0,"monsters/mancdie.md2",0},	// S1_FATT_DIE1
	{SPR1_FATT,11,6,SA1_Scream,S1_FATT_DIE3,0,0,"monsters/mancdie.md2",1},	// S1_FATT_DIE2
	{SPR1_FATT,12,6,SA1_Fall,S1_FATT_DIE4,0,0,"monsters/mancdie.md2",2},	// S1_FATT_DIE3
	{SPR1_FATT,13,6,SA1_NULL,S1_FATT_DIE5,0,0,"monsters/mancdie.md2",3},	// S1_FATT_DIE4
	{SPR1_FATT,14,6,SA1_NULL,S1_FATT_DIE6,0,0,"monsters/mancdie.md2",4},	// S1_FATT_DIE5
	{SPR1_FATT,15,6,SA1_NULL,S1_FATT_DIE7,0,0,"monsters/mancdie.md2",5},	// S1_FATT_DIE6
	{SPR1_FATT,16,6,SA1_NULL,S1_FATT_DIE8,0,0,"monsters/mancdie.md2",6},	// S1_FATT_DIE7
	{SPR1_FATT,17,6,SA1_NULL,S1_FATT_DIE9,0,0,"monsters/mancdie.md2",7},	// S1_FATT_DIE8
	{SPR1_FATT,18,6,SA1_NULL,S1_FATT_DIE10,0,0,"monsters/mancdie.md2",8},	// S1_FATT_DIE9
	{SPR1_FATT,19,-1,SA1_BossDeath,S1_NULL,0,0,"monsters/mancdie.md2",9},	// S1_FATT_DIE10
	{SPR1_FATT,17,5,SA1_NULL,S1_FATT_RAISE2,0,0,"monsters/mancdie.md2",10},	// S1_FATT_RAISE1
	{SPR1_FATT,16,5,SA1_NULL,S1_FATT_RAISE3,0,0,"monsters/mancdie.md2",11},	// S1_FATT_RAISE2
	{SPR1_FATT,15,5,SA1_NULL,S1_FATT_RAISE4,0,0,"monsters/mancdie.md2",12},	// S1_FATT_RAISE3
	{SPR1_FATT,14,5,SA1_NULL,S1_FATT_RAISE5,0,0,"monsters/mancdie.md2",13},	// S1_FATT_RAISE4
	{SPR1_FATT,13,5,SA1_NULL,S1_FATT_RAISE6,0,0,"monsters/mancdie.md2",14},	// S1_FATT_RAISE5
	{SPR1_FATT,12,5,SA1_NULL,S1_FATT_RAISE7,0,0,"monsters/mancdie.md2",15},	// S1_FATT_RAISE6
	{SPR1_FATT,11,5,SA1_NULL,S1_FATT_RAISE8,0,0,"monsters/mancdie.md2",16},	// S1_FATT_RAISE7
	{SPR1_FATT,10,5,SA1_NULL,S1_FATT_RUN1,0,0,"monsters/mancdie.md2",17},	// S1_FATT_RAISE8
	{SPR1_CPOS,0,10,SA1_Look,S1_CPOS_STND2,0,0,"monsters/chainguy.md2",0},	// S1_CPOS_STND
	{SPR1_CPOS,1,10,SA1_Look,S1_CPOS_STND,0,0,"monsters/chainguy.md2",1},	// S1_CPOS_STND2
	{SPR1_CPOS,0,3,SA1_Chase,S1_CPOS_RUN2,0,0,"monsters/chainguy.md2",2},	// S1_CPOS_RUN1
	{SPR1_CPOS,0,3,SA1_Chase,S1_CPOS_RUN3,0,0,"monsters/chainguy.md2",3},	// S1_CPOS_RUN2
	{SPR1_CPOS,1,3,SA1_Chase,S1_CPOS_RUN4,0,0,"monsters/chainguy.md2",4},	// S1_CPOS_RUN3
	{SPR1_CPOS,1,3,SA1_Chase,S1_CPOS_RUN5,0,0,"monsters/chainguy.md2",5},	// S1_CPOS_RUN4
	{SPR1_CPOS,2,3,SA1_Chase,S1_CPOS_RUN6,0,0,"monsters/chainguy.md2",6},	// S1_CPOS_RUN5
	{SPR1_CPOS,2,3,SA1_Chase,S1_CPOS_RUN7,0,0,"monsters/chainguy.md2",7},	// S1_CPOS_RUN6
	{SPR1_CPOS,3,3,SA1_Chase,S1_CPOS_RUN8,0,0,"monsters/chainguy.md2",8},	// S1_CPOS_RUN7
	{SPR1_CPOS,3,3,SA1_Chase,S1_CPOS_RUN1,0,0,"monsters/chainguy.md2",9},	// S1_CPOS_RUN8
	{SPR1_CPOS,4,10,SA1_FaceTarget,S1_CPOS_ATK2,0,0,"monsters/chainguy.md2",10},	// S1_CPOS_ATK1
	{SPR1_CPOS,32773,4,SA1_CPosAttack,S1_CPOS_ATK3,0,0,"monsters/chainguy.md2",11},	// S1_CPOS_ATK2
	{SPR1_CPOS,32772,4,SA1_CPosAttack,S1_CPOS_ATK4,0,0,"monsters/chainguy.md2",12},	// S1_CPOS_ATK3
	{SPR1_CPOS,5,1,SA1_CPosRefire,S1_CPOS_ATK2,0,0,"monsters/chainguy.md2",13},	// S1_CPOS_ATK4
	{SPR1_CPOS,6,3,SA1_NULL,S1_CPOS_PAIN2,0,0,"monsters/chainguy.md2",14},	// S1_CPOS_PAIN
	{SPR1_CPOS,6,3,SA1_Pain,S1_CPOS_RUN1,0,0,"monsters/chainguy.md2",15},	// S1_CPOS_PAIN2
	{SPR1_CPOS,7,5,SA1_NULL,S1_CPOS_DIE2,0,0,"monsters/chaindie.md2",0},	// S1_CPOS_DIE1
	{SPR1_CPOS,8,5,SA1_Scream,S1_CPOS_DIE3,0,0,"monsters/chaindie.md2",1},	// S1_CPOS_DIE2
	{SPR1_CPOS,9,5,SA1_Fall,S1_CPOS_DIE4,0,0,"monsters/chaindie.md2",2},	// S1_CPOS_DIE3
	{SPR1_CPOS,10,5,SA1_NULL,S1_CPOS_DIE5,0,0,"monsters/chaindie.md2",3},	// S1_CPOS_DIE4
	{SPR1_CPOS,11,5,SA1_NULL,S1_CPOS_DIE6,0,0,"monsters/chaindie.md2",4},	// S1_CPOS_DIE5
	{SPR1_CPOS,12,5,SA1_NULL,S1_CPOS_DIE7,0,0,"monsters/chaindie.md2",5},	// S1_CPOS_DIE6
	{SPR1_CPOS,13,-1,SA1_NULL,S1_NULL,0,0,"monsters/chaindie.md2",6},	// S1_CPOS_DIE7
	{SPR1_CPOS,14,5,SA1_NULL,S1_CPOS_XDIE2,0,0,"gibs/chaingib.md2",0},	// S1_CPOS_XDIE1
	{SPR1_CPOS,15,5,SA1_XScream,S1_CPOS_XDIE3,0,0,"gibs/chaingib.md2",1},	// S1_CPOS_XDIE2
	{SPR1_CPOS,16,5,SA1_Fall,S1_CPOS_XDIE4,0,0,"gibs/chaingib.md2",2},	// S1_CPOS_XDIE3
	{SPR1_CPOS,17,5,SA1_NULL,S1_CPOS_XDIE5,0,0,"gibs/chaingib.md2",3},	// S1_CPOS_XDIE4
	{SPR1_CPOS,18,5,SA1_NULL,S1_CPOS_XDIE6,0,0,"gibs/chaingib.md2",4},	// S1_CPOS_XDIE5
	{SPR1_CPOS,19,-1,SA1_NULL,S1_NULL,0,0,"gibs/chaingib.md2",5},	// S1_CPOS_XDIE6
	{SPR1_CPOS,13,5,SA1_NULL,S1_CPOS_RAISE2,0,0,"monsters/chaindie.md2",7},	// S1_CPOS_RAISE1
	{SPR1_CPOS,12,5,SA1_NULL,S1_CPOS_RAISE3,0,0,"monsters/chaindie.md2",8},	// S1_CPOS_RAISE2
	{SPR1_CPOS,11,5,SA1_NULL,S1_CPOS_RAISE4,0,0,"monsters/chaindie.md2",9},	// S1_CPOS_RAISE3
	{SPR1_CPOS,10,5,SA1_NULL,S1_CPOS_RAISE5,0,0,"monsters/chaindie.md2",10},	// S1_CPOS_RAISE4
	{SPR1_CPOS,9,5,SA1_NULL,S1_CPOS_RAISE6,0,0,"monsters/chaindie.md2",11},	// S1_CPOS_RAISE5
	{SPR1_CPOS,8,5,SA1_NULL,S1_CPOS_RAISE7,0,0,"monsters/chaindie.md2",12},	// S1_CPOS_RAISE6
	{SPR1_CPOS,7,5,SA1_NULL,S1_CPOS_RUN1,0,0,"monsters/chaindie.md2",13},	// S1_CPOS_RAISE7
	{SPR1_TROO,0,10,SA1_Look,S1_TROO_STND2,0,0,"monsters/imp.md2",0},	// S1_TROO_STND
	{SPR1_TROO,1,10,SA1_Look,S1_TROO_STND,0,0,"monsters/imp.md2",1},	// S1_TROO_STND2
	{SPR1_TROO,0,3,SA1_Chase,S1_TROO_RUN2,0,0,"monsters/imp.md2",8},	// S1_TROO_RUN1
	{SPR1_TROO,0,3,SA1_Chase,S1_TROO_RUN3,0,0,"monsters/imp.md2",9},	// S1_TROO_RUN2
	{SPR1_TROO,1,3,SA1_Chase,S1_TROO_RUN4,0,0,"monsters/imp.md2",10},	// S1_TROO_RUN3
	{SPR1_TROO,1,3,SA1_Chase,S1_TROO_RUN5,0,0,"monsters/imp.md2",11},	// S1_TROO_RUN4
	{SPR1_TROO,2,3,SA1_Chase,S1_TROO_RUN6,0,0,"monsters/imp.md2",12},	// S1_TROO_RUN5
	{SPR1_TROO,2,3,SA1_Chase,S1_TROO_RUN7,0,0,"monsters/imp.md2",13},	// S1_TROO_RUN6
	{SPR1_TROO,3,3,SA1_Chase,S1_TROO_RUN8,0,0,"monsters/imp.md2",14},	// S1_TROO_RUN7
	{SPR1_TROO,3,3,SA1_Chase,S1_TROO_RUN1,0,0,"monsters/imp.md2",15},	// S1_TROO_RUN8
	{SPR1_TROO,4,8,SA1_FaceTarget,S1_TROO_ATK2,0,0,"monsters/imp.md2",16},	// S1_TROO_ATK1
	{SPR1_TROO,5,8,SA1_FaceTarget,S1_TROO_ATK3,0,0,"monsters/imp.md2",17},	// S1_TROO_ATK2
	{SPR1_TROO,6,6,SA1_TroopAttack,S1_TROO_RUN1,0,0,"monsters/imp.md2",18},	// S1_TROO_ATK3
	{SPR1_TROO,7,2,SA1_NULL,S1_TROO_PAIN2,0,0,"monsters/imp.md2",19},	// S1_TROO_PAIN
	{SPR1_TROO,7,2,SA1_Pain,S1_TROO_RUN1,0,0,"monsters/imp.md2",20},	// S1_TROO_PAIN2
	{SPR1_TROO,8,8,SA1_NULL,S1_TROO_DIE2,0,0,"monsters/impdie.md2",0},	// S1_TROO_DIE1
	{SPR1_TROO,9,8,SA1_Scream,S1_TROO_DIE3,0,0,"monsters/impdie.md2",1},	// S1_TROO_DIE2
	{SPR1_TROO,10,6,SA1_NULL,S1_TROO_DIE4,0,0,"monsters/impdie.md2",2},	// S1_TROO_DIE3
	{SPR1_TROO,11,6,SA1_Fall,S1_TROO_DIE5,0,0,"monsters/impdie.md2",3},	// S1_TROO_DIE4
	{SPR1_TROO,12,-1,SA1_NULL,S1_NULL,0,0,"monsters/impdie.md2",4},	// S1_TROO_DIE5
	{SPR1_TROO,13,5,SA1_NULL,S1_TROO_XDIE2,0,0,"gibs/impgib.md2",0},	// S1_TROO_XDIE1
	{SPR1_TROO,14,5,SA1_XScream,S1_TROO_XDIE3,0,0,"gibs/impgib.md2",1},	// S1_TROO_XDIE2
	{SPR1_TROO,15,5,SA1_NULL,S1_TROO_XDIE4,0,0,"gibs/impgib.md2",2},	// S1_TROO_XDIE3
	{SPR1_TROO,16,5,SA1_Fall,S1_TROO_XDIE5,0,0,"gibs/impgib.md2",3},	// S1_TROO_XDIE4
	{SPR1_TROO,17,5,SA1_NULL,S1_TROO_XDIE6,0,0,"gibs/impgib.md2",4},	// S1_TROO_XDIE5
	{SPR1_TROO,18,5,SA1_NULL,S1_TROO_XDIE7,0,0,"gibs/impgib.md2",5},	// S1_TROO_XDIE6
	{SPR1_TROO,19,5,SA1_NULL,S1_TROO_XDIE8,0,0,"gibs/impgib.md2",6},	// S1_TROO_XDIE7
	{SPR1_TROO,20,-1,SA1_NULL,S1_NULL,0,0,"gibs/impgib.md2",7},	// S1_TROO_XDIE8
	{SPR1_TROO,12,8,SA1_NULL,S1_TROO_RAISE2,0,0,"monsters/impdie.md2",5},	// S1_TROO_RAISE1
	{SPR1_TROO,11,8,SA1_NULL,S1_TROO_RAISE3,0,0,"monsters/impdie.md2",6},	// S1_TROO_RAISE2
	{SPR1_TROO,10,6,SA1_NULL,S1_TROO_RAISE4,0,0,"monsters/impdie.md2",7},	// S1_TROO_RAISE3
	{SPR1_TROO,9,6,SA1_NULL,S1_TROO_RAISE5,0,0,"monsters/impdie.md2",8},	// S1_TROO_RAISE4
	{SPR1_TROO,8,6,SA1_NULL,S1_TROO_RUN1,0,0,"monsters/impdie.md2",9},	// S1_TROO_RAISE5
	{SPR1_SARG,0,10,SA1_Look,S1_SARG_STND2,0,0,"monsters/demon.md2",0},	// S1_SARG_STND
	{SPR1_SARG,1,10,SA1_Look,S1_SARG_STND,0,0,"monsters/demon.md2",1},	// S1_SARG_STND2
	{SPR1_SARG,0,2,SA1_Chase,S1_SARG_RUN2,0,0,"monsters/demon.md2",2},	// S1_SARG_RUN1
	{SPR1_SARG,0,2,SA1_Chase,S1_SARG_RUN3,0,0,"monsters/demon.md2",3},	// S1_SARG_RUN2
	{SPR1_SARG,1,2,SA1_Chase,S1_SARG_RUN4,0,0,"monsters/demon.md2",4},	// S1_SARG_RUN3
	{SPR1_SARG,1,2,SA1_Chase,S1_SARG_RUN5,0,0,"monsters/demon.md2",5},	// S1_SARG_RUN4
	{SPR1_SARG,2,2,SA1_Chase,S1_SARG_RUN6,0,0,"monsters/demon.md2",6},	// S1_SARG_RUN5
	{SPR1_SARG,2,2,SA1_Chase,S1_SARG_RUN7,0,0,"monsters/demon.md2",7},	// S1_SARG_RUN6
	{SPR1_SARG,3,2,SA1_Chase,S1_SARG_RUN8,0,0,"monsters/demon.md2",8},	// S1_SARG_RUN7
	{SPR1_SARG,3,2,SA1_Chase,S1_SARG_RUN1,0,0,"monsters/demon.md2",9},	// S1_SARG_RUN8
	{SPR1_SARG,4,8,SA1_FaceTarget,S1_SARG_ATK2,0,0,"monsters/demon.md2",10},	// S1_SARG_ATK1
	{SPR1_SARG,5,8,SA1_FaceTarget,S1_SARG_ATK3,0,0,"monsters/demon.md2",11},	// S1_SARG_ATK2
	{SPR1_SARG,6,8,SA1_SargAttack,S1_SARG_RUN1,0,0,"monsters/demon.md2",12},	// S1_SARG_ATK3
	{SPR1_SARG,7,2,SA1_NULL,S1_SARG_PAIN2,0,0,"monsters/demon.md2",13},	// S1_SARG_PAIN
	{SPR1_SARG,7,2,SA1_Pain,S1_SARG_RUN1,0,0,"monsters/demon.md2",14},	// S1_SARG_PAIN2
	{SPR1_SARG,8,8,SA1_NULL,S1_SARG_DIE2,0,0,"monsters/demondie.md2",0},	// S1_SARG_DIE1
	{SPR1_SARG,9,8,SA1_Scream,S1_SARG_DIE3,0,0,"monsters/demondie.md2",1},	// S1_SARG_DIE2
	{SPR1_SARG,10,4,SA1_NULL,S1_SARG_DIE4,0,0,"monsters/demondie.md2",2},	// S1_SARG_DIE3
	{SPR1_SARG,11,4,SA1_Fall,S1_SARG_DIE5,0,0,"monsters/demondie.md2",3},	// S1_SARG_DIE4
	{SPR1_SARG,12,4,SA1_NULL,S1_SARG_DIE6,0,0,"monsters/demondie.md2",4},	// S1_SARG_DIE5
	{SPR1_SARG,13,-1,SA1_NULL,S1_NULL,0,0,"monsters/demondie.md2",5},	// S1_SARG_DIE6
	{SPR1_SARG,13,5,SA1_NULL,S1_SARG_RAISE2,0,0,"monsters/demondie.md2",6},	// S1_SARG_RAISE1
	{SPR1_SARG,12,5,SA1_NULL,S1_SARG_RAISE3,0,0,"monsters/demondie.md2",7},	// S1_SARG_RAISE2
	{SPR1_SARG,11,5,SA1_NULL,S1_SARG_RAISE4,0,0,"monsters/demondie.md2",8},	// S1_SARG_RAISE3
	{SPR1_SARG,10,5,SA1_NULL,S1_SARG_RAISE5,0,0,"monsters/demondie.md2",9},	// S1_SARG_RAISE4
	{SPR1_SARG,9,5,SA1_NULL,S1_SARG_RAISE6,0,0,"monsters/demondie.md2",10},	// S1_SARG_RAISE5
	{SPR1_SARG,8,5,SA1_NULL,S1_SARG_RUN1,0,0,"monsters/demondie.md2",11},	// S1_SARG_RAISE6
	{SPR1_HEAD,0,10,SA1_Look,S1_HEAD_STND,0,0,"monsters/caco.md2",0},	// S1_HEAD_STND
	{SPR1_HEAD,0,3,SA1_Chase,S1_HEAD_RUN1,0,0,"monsters/caco.md2",1},	// S1_HEAD_RUN1
	{SPR1_HEAD,1,5,SA1_FaceTarget,S1_HEAD_ATK2,0,0,"monsters/caco.md2",2},	// S1_HEAD_ATK1
	{SPR1_HEAD,2,5,SA1_FaceTarget,S1_HEAD_ATK3,0,0,"monsters/caco.md2",3},	// S1_HEAD_ATK2
	{SPR1_HEAD,32771,5,SA1_HeadAttack,S1_HEAD_RUN1,0,0,"monsters/caco.md2",4},	// S1_HEAD_ATK3
	{SPR1_HEAD,4,3,SA1_NULL,S1_HEAD_PAIN2,0,0,"monsters/caco.md2",5},	// S1_HEAD_PAIN
	{SPR1_HEAD,4,3,SA1_Pain,S1_HEAD_PAIN3,0,0,"monsters/caco.md2",6},	// S1_HEAD_PAIN2
	{SPR1_HEAD,5,6,SA1_NULL,S1_HEAD_RUN1,0,0,"monsters/caco.md2",7},	// S1_HEAD_PAIN3
	{SPR1_HEAD,6,8,SA1_NULL,S1_HEAD_DIE2,0,0,"monsters/cacodie.md2",0},	// S1_HEAD_DIE1
	{SPR1_HEAD,7,8,SA1_Scream,S1_HEAD_DIE3,0,0,"monsters/cacodie.md2",1},	// S1_HEAD_DIE2
	{SPR1_HEAD,8,8,SA1_NULL,S1_HEAD_DIE4,0,0,"monsters/cacodie.md2",2},	// S1_HEAD_DIE3
	{SPR1_HEAD,9,8,SA1_NULL,S1_HEAD_DIE5,0,0,"monsters/cacodie.md2",3},	// S1_HEAD_DIE4
	{SPR1_HEAD,10,8,SA1_Fall,S1_HEAD_DIE6,0,0,"monsters/cacodie.md2",4},	// S1_HEAD_DIE5
	{SPR1_HEAD,11,-1,SA1_NULL,S1_NULL,0,0,"monsters/cacodie.md2",5},	// S1_HEAD_DIE6
	{SPR1_HEAD,11,8,SA1_NULL,S1_HEAD_RAISE2,0,0,"monsters/cacodie.md2",6},	// S1_HEAD_RAISE1
	{SPR1_HEAD,10,8,SA1_NULL,S1_HEAD_RAISE3,0,0,"monsters/cacodie.md2",7},	// S1_HEAD_RAISE2
	{SPR1_HEAD,9,8,SA1_NULL,S1_HEAD_RAISE4,0,0,"monsters/cacodie.md2",8},	// S1_HEAD_RAISE3
	{SPR1_HEAD,8,8,SA1_NULL,S1_HEAD_RAISE5,0,0,"monsters/cacodie.md2",9},	// S1_HEAD_RAISE4
	{SPR1_HEAD,7,8,SA1_NULL,S1_HEAD_RAISE6,0,0,"monsters/cacodie.md2",10},	// S1_HEAD_RAISE5
	{SPR1_HEAD,6,8,SA1_NULL,S1_HEAD_RUN1,0,0,"monsters/cacodie.md2",11},	// S1_HEAD_RAISE6
	{SPR1_BAL7,32768,4,SA1_NULL,S1_BRBALL2,0,0,"missiles/grenball.md2",0},	// S1_BRBALL1
	{SPR1_BAL7,32769,4,SA1_NULL,S1_BRBALL1,0,0,"missiles/grenball.md2",1},	// S1_BRBALL2
	{SPR1_BAL7,32770,6,SA1_NULL,S1_BRBALLX2,0,0},	// S1_BRBALLX1
	{SPR1_BAL7,32771,6,SA1_NULL,S1_BRBALLX3,0,0},	// S1_BRBALLX2
	{SPR1_BAL7,32772,6,SA1_NULL,S1_NULL,0,0},	// S1_BRBALLX3
	{SPR1_BOSS,0,10,SA1_Look,S1_BOSS_STND2,0,0,"monsters/baron.md2",0},	// S1_BOSS_STND
	{SPR1_BOSS,1,10,SA1_Look,S1_BOSS_STND,0,0,"monsters/baron.md2",1},	// S1_BOSS_STND2
	{SPR1_BOSS,0,3,SA1_Chase,S1_BOSS_RUN2,0,0,"monsters/baron.md2",2},	// S1_BOSS_RUN1
	{SPR1_BOSS,0,3,SA1_Chase,S1_BOSS_RUN3,0,0,"monsters/baron.md2",3},	// S1_BOSS_RUN2
	{SPR1_BOSS,1,3,SA1_Chase,S1_BOSS_RUN4,0,0,"monsters/baron.md2",4},	// S1_BOSS_RUN3
	{SPR1_BOSS,1,3,SA1_Chase,S1_BOSS_RUN5,0,0,"monsters/baron.md2",5},	// S1_BOSS_RUN4
	{SPR1_BOSS,2,3,SA1_Chase,S1_BOSS_RUN6,0,0,"monsters/baron.md2",6},	// S1_BOSS_RUN5
	{SPR1_BOSS,2,3,SA1_Chase,S1_BOSS_RUN7,0,0,"monsters/baron.md2",7},	// S1_BOSS_RUN6
	{SPR1_BOSS,3,3,SA1_Chase,S1_BOSS_RUN8,0,0,"monsters/baron.md2",8},	// S1_BOSS_RUN7
	{SPR1_BOSS,3,3,SA1_Chase,S1_BOSS_RUN1,0,0,"monsters/baron.md2",9},	// S1_BOSS_RUN8
	{SPR1_BOSS,4,8,SA1_FaceTarget,S1_BOSS_ATK2,0,0,"monsters/baron.md2",10},	// S1_BOSS_ATK1
	{SPR1_BOSS,5,8,SA1_FaceTarget,S1_BOSS_ATK3,0,0,"monsters/baron.md2",11},	// S1_BOSS_ATK2
	{SPR1_BOSS,6,8,SA1_BruisAttack,S1_BOSS_RUN1,0,0,"monsters/baron.md2",12},	// S1_BOSS_ATK3
	{SPR1_BOSS,7,2,SA1_NULL,S1_BOSS_PAIN2,0,0,"monsters/baron.md2",13},	// S1_BOSS_PAIN
	{SPR1_BOSS,7,2,SA1_Pain,S1_BOSS_RUN1,0,0,"monsters/baron.md2",14},	// S1_BOSS_PAIN2
	{SPR1_BOSS,8,8,SA1_NULL,S1_BOSS_DIE2,0,0,"monsters/barondie.md2",0},	// S1_BOSS_DIE1
	{SPR1_BOSS,9,8,SA1_Scream,S1_BOSS_DIE3,0,0,"monsters/barondie.md2",1},	// S1_BOSS_DIE2
	{SPR1_BOSS,10,8,SA1_NULL,S1_BOSS_DIE4,0,0,"monsters/barondie.md2",2},	// S1_BOSS_DIE3
	{SPR1_BOSS,11,8,SA1_Fall,S1_BOSS_DIE5,0,0,"monsters/barondie.md2",3},	// S1_BOSS_DIE4
	{SPR1_BOSS,12,8,SA1_NULL,S1_BOSS_DIE6,0,0,"monsters/barondie.md2",4},	// S1_BOSS_DIE5
	{SPR1_BOSS,13,8,SA1_NULL,S1_BOSS_DIE7,0,0,"monsters/barondie.md2",5},	// S1_BOSS_DIE6
	{SPR1_BOSS,14,-1,SA1_BossDeath,S1_NULL,0,0,"monsters/barondie.md2",6},	// S1_BOSS_DIE7
	{SPR1_BOSS,14,8,SA1_NULL,S1_BOSS_RAISE2,0,0,"monsters/barondie.md2",7},	// S1_BOSS_RAISE1
	{SPR1_BOSS,13,8,SA1_NULL,S1_BOSS_RAISE3,0,0,"monsters/barondie.md2",8},	// S1_BOSS_RAISE2
	{SPR1_BOSS,12,8,SA1_NULL,S1_BOSS_RAISE4,0,0,"monsters/barondie.md2",9},	// S1_BOSS_RAISE3
	{SPR1_BOSS,11,8,SA1_NULL,S1_BOSS_RAISE5,0,0,"monsters/barondie.md2",10},	// S1_BOSS_RAISE4
	{SPR1_BOSS,10,8,SA1_NULL,S1_BOSS_RAISE6,0,0,"monsters/barondie.md2",11},	// S1_BOSS_RAISE5
	{SPR1_BOSS,9,8,SA1_NULL,S1_BOSS_RAISE7,0,0,"monsters/barondie.md2",12},	// S1_BOSS_RAISE6
	{SPR1_BOSS,8,8,SA1_NULL,S1_BOSS_RUN1,0,0,"monsters/barondie.md2",13},	// S1_BOSS_RAISE7
	{SPR1_BOS2,0,10,SA1_Look,S1_BOS2_STND2,0,0,"monsters/knight.md2",0},	// S1_BOS2_STND
	{SPR1_BOS2,1,10,SA1_Look,S1_BOS2_STND,0,0,"monsters/knight.md2",1},	// S1_BOS2_STND2
	{SPR1_BOS2,0,3,SA1_Chase,S1_BOS2_RUN2,0,0,"monsters/knight.md2",2},	// S1_BOS2_RUN1
	{SPR1_BOS2,0,3,SA1_Chase,S1_BOS2_RUN3,0,0,"monsters/knight.md2",3},	// S1_BOS2_RUN2
	{SPR1_BOS2,1,3,SA1_Chase,S1_BOS2_RUN4,0,0,"monsters/knight.md2",4},	// S1_BOS2_RUN3
	{SPR1_BOS2,1,3,SA1_Chase,S1_BOS2_RUN5,0,0,"monsters/knight.md2",5},	// S1_BOS2_RUN4
	{SPR1_BOS2,2,3,SA1_Chase,S1_BOS2_RUN6,0,0,"monsters/knight.md2",6},	// S1_BOS2_RUN5
	{SPR1_BOS2,2,3,SA1_Chase,S1_BOS2_RUN7,0,0,"monsters/knight.md2",7},	// S1_BOS2_RUN6
	{SPR1_BOS2,3,3,SA1_Chase,S1_BOS2_RUN8,0,0,"monsters/knight.md2",8},	// S1_BOS2_RUN7
	{SPR1_BOS2,3,3,SA1_Chase,S1_BOS2_RUN1,0,0,"monsters/knight.md2",9},	// S1_BOS2_RUN8
	{SPR1_BOS2,4,8,SA1_FaceTarget,S1_BOS2_ATK2,0,0,"monsters/knight.md2",10},	// S1_BOS2_ATK1
	{SPR1_BOS2,5,8,SA1_FaceTarget,S1_BOS2_ATK3,0,0,"monsters/knight.md2",11},	// S1_BOS2_ATK2
	{SPR1_BOS2,6,8,SA1_BruisAttack,S1_BOS2_RUN1,0,0,"monsters/knight.md2",12},	// S1_BOS2_ATK3
	{SPR1_BOS2,7,2,SA1_NULL,S1_BOS2_PAIN2,0,0,"monsters/knight.md2",13},	// S1_BOS2_PAIN
	{SPR1_BOS2,7,2,SA1_Pain,S1_BOS2_RUN1,0,0,"monsters/knight.md2",14},	// S1_BOS2_PAIN2
	{SPR1_BOS2,8,8,SA1_NULL,S1_BOS2_DIE2,0,0,"monsters/knidie.md2",0},	// S1_BOS2_DIE1
	{SPR1_BOS2,9,8,SA1_Scream,S1_BOS2_DIE3,0,0,"monsters/knidie.md2",1},	// S1_BOS2_DIE2
	{SPR1_BOS2,10,8,SA1_NULL,S1_BOS2_DIE4,0,0,"monsters/knidie.md2",2},	// S1_BOS2_DIE3
	{SPR1_BOS2,11,8,SA1_Fall,S1_BOS2_DIE5,0,0,"monsters/knidie.md2",3},	// S1_BOS2_DIE4
	{SPR1_BOS2,12,8,SA1_NULL,S1_BOS2_DIE6,0,0,"monsters/knidie.md2",4},	// S1_BOS2_DIE5
	{SPR1_BOS2,13,8,SA1_NULL,S1_BOS2_DIE7,0,0,"monsters/knidie.md2",5},	// S1_BOS2_DIE6
	{SPR1_BOS2,14,-1,SA1_NULL,S1_NULL,0,0,"monsters/knidie.md2",6},	// S1_BOS2_DIE7
	{SPR1_BOS2,14,8,SA1_NULL,S1_BOS2_RAISE2,0,0,"monsters/knidie.md2",7},	// S1_BOS2_RAISE1
	{SPR1_BOS2,13,8,SA1_NULL,S1_BOS2_RAISE3,0,0,"monsters/knidie.md2",8},	// S1_BOS2_RAISE2
	{SPR1_BOS2,12,8,SA1_NULL,S1_BOS2_RAISE4,0,0,"monsters/knidie.md2",9},	// S1_BOS2_RAISE3
	{SPR1_BOS2,11,8,SA1_NULL,S1_BOS2_RAISE5,0,0,"monsters/knidie.md2",10},	// S1_BOS2_RAISE4
	{SPR1_BOS2,10,8,SA1_NULL,S1_BOS2_RAISE6,0,0,"monsters/knidie.md2",11},	// S1_BOS2_RAISE5
	{SPR1_BOS2,9,8,SA1_NULL,S1_BOS2_RAISE7,0,0,"monsters/knidie.md2",12},	// S1_BOS2_RAISE6
	{SPR1_BOS2,8,8,SA1_NULL,S1_BOS2_RUN1,0,0,"monsters/knidie.md2",13},	// S1_BOS2_RAISE7
	{SPR1_SKUL,32768,10,SA1_Look,S1_SKULL_STND2,0,0,"monsters/skull.md2",0},	// S1_SKULL_STND
	{SPR1_SKUL,32769,10,SA1_Look,S1_SKULL_STND,0,0,"monsters/skull.md2",1},	// S1_SKULL_STND2
	{SPR1_SKUL,32768,6,SA1_Chase,S1_SKULL_RUN2,0,0,"monsters/skull.md2",2},	// S1_SKULL_RUN1
	{SPR1_SKUL,32769,6,SA1_Chase,S1_SKULL_RUN1,0,0,"monsters/skull.md2",3},	// S1_SKULL_RUN2
	{SPR1_SKUL,32770,10,SA1_FaceTarget,S1_SKULL_ATK2,0,0,"monsters/skull.md2",4},	// S1_SKULL_ATK1
	{SPR1_SKUL,32771,4,SA1_SkullAttack,S1_SKULL_ATK3,0,0,"monsters/skull.md2",5},	// S1_SKULL_ATK2
	{SPR1_SKUL,32770,4,SA1_NULL,S1_SKULL_ATK4,0,0,"monsters/skull.md2",6},	// S1_SKULL_ATK3
	{SPR1_SKUL,32771,4,SA1_NULL,S1_SKULL_ATK3,0,0,"monsters/skull.md2",7},	// S1_SKULL_ATK4
	{SPR1_SKUL,32772,3,SA1_NULL,S1_SKULL_PAIN2,0,0,"monsters/skull.md2",8},	// S1_SKULL_PAIN
	{SPR1_SKUL,32772,3,SA1_Pain,S1_SKULL_RUN1,0,0,"monsters/skull.md2",9},	// S1_SKULL_PAIN2
	{SPR1_SKUL,32773,6,SA1_NULL,S1_SKULL_DIE2,0,0},	// S1_SKULL_DIE1
	{SPR1_SKUL,32774,6,SA1_Scream,S1_SKULL_DIE3,0,0},	// S1_SKULL_DIE2
	{SPR1_SKUL,32775,6,SA1_NULL,S1_SKULL_DIE4,0,0},	// S1_SKULL_DIE3
	{SPR1_SKUL,32776,6,SA1_Fall,S1_SKULL_DIE5,0,0},	// S1_SKULL_DIE4
	{SPR1_SKUL,9,6,SA1_NULL,S1_SKULL_DIE6,0,0},	// S1_SKULL_DIE5
	{SPR1_SKUL,10,6,SA1_NULL,S1_NULL,0,0},	// S1_SKULL_DIE6
	{SPR1_SPID,0,10,SA1_Look,S1_SPID_STND2,0,0,"monsters/spider.md2",0},	// S1_SPID_STND
	{SPR1_SPID,1,10,SA1_Look,S1_SPID_STND,0,0,"monsters/spider.md2",1},	// S1_SPID_STND2
	{SPR1_SPID,0,3,SA1_Metal,S1_SPID_RUN2,0,0,"monsters/spider.md2",2},	// S1_SPID_RUN1
	{SPR1_SPID,0,3,SA1_Chase,S1_SPID_RUN3,0,0,"monsters/spider.md2",3},	// S1_SPID_RUN2
	{SPR1_SPID,1,3,SA1_Chase,S1_SPID_RUN4,0,0,"monsters/spider.md2",4},	// S1_SPID_RUN3
	{SPR1_SPID,1,3,SA1_Chase,S1_SPID_RUN5,0,0,"monsters/spider.md2",5},	// S1_SPID_RUN4
	{SPR1_SPID,2,3,SA1_Metal,S1_SPID_RUN6,0,0,"monsters/spider.md2",6},	// S1_SPID_RUN5
	{SPR1_SPID,2,3,SA1_Chase,S1_SPID_RUN7,0,0,"monsters/spider.md2",7},	// S1_SPID_RUN6
	{SPR1_SPID,3,3,SA1_Chase,S1_SPID_RUN8,0,0,"monsters/spider.md2",8},	// S1_SPID_RUN7
	{SPR1_SPID,3,3,SA1_Chase,S1_SPID_RUN9,0,0,"monsters/spider.md2",9},	// S1_SPID_RUN8
	{SPR1_SPID,4,3,SA1_Metal,S1_SPID_RUN10,0,0,"monsters/spider.md2",10},	// S1_SPID_RUN9
	{SPR1_SPID,4,3,SA1_Chase,S1_SPID_RUN11,0,0,"monsters/spider.md2",11},	// S1_SPID_RUN10
	{SPR1_SPID,5,3,SA1_Chase,S1_SPID_RUN12,0,0,"monsters/spider.md2",12},	// S1_SPID_RUN11
	{SPR1_SPID,5,3,SA1_Chase,S1_SPID_RUN1,0,0,"monsters/spider.md2",13},	// S1_SPID_RUN12
	{SPR1_SPID,32768,20,SA1_FaceTarget,S1_SPID_ATK2,0,0,"monsters/spider.md2",14},	// S1_SPID_ATK1
	{SPR1_SPID,32774,4,SA1_SPosAttack,S1_SPID_ATK3,0,0,"monsters/spider.md2",15},	// S1_SPID_ATK2
	{SPR1_SPID,32775,4,SA1_SPosAttack,S1_SPID_ATK4,0,0,"monsters/spidatk.md2",0},	// S1_SPID_ATK3
	{SPR1_SPID,32775,1,SA1_SpidRefire,S1_SPID_ATK2,0,0,"monsters/spidatk.md2",0},	// S1_SPID_ATK4
	{SPR1_SPID,8,3,SA1_NULL,S1_SPID_PAIN2,0,0,"monsters/spider.md2",16},	// S1_SPID_PAIN
	{SPR1_SPID,8,3,SA1_Pain,S1_SPID_RUN1,0,0,"monsters/spider.md2",17},	// S1_SPID_PAIN2
	{SPR1_SPID,9,20,SA1_Scream,S1_SPID_DIE2,0,0,"monsters/spiddie.md2",0},	// S1_SPID_DIE1
	{SPR1_SPID,10,10,SA1_Fall,S1_SPID_DIE3,0,0,"monsters/spiddie.md2",1},	// S1_SPID_DIE2
	{SPR1_SPID,11,10,SA1_NULL,S1_SPID_DIE4,0,0,"monsters/spiddie.md2",2},	// S1_SPID_DIE3
	{SPR1_SPID,12,10,SA1_NULL,S1_SPID_DIE5,0,0,"monsters/spiddie.md2",3},	// S1_SPID_DIE4
	{SPR1_SPID,13,10,SA1_NULL,S1_SPID_DIE6,0,0,"monsters/spiddie.md2",4},	// S1_SPID_DIE5
	{SPR1_SPID,14,10,SA1_NULL,S1_SPID_DIE7,0,0,"monsters/spiddie.md2",5},	// S1_SPID_DIE6
	{SPR1_SPID,15,10,SA1_NULL,S1_SPID_DIE8,0,0,"monsters/spiddie.md2",6},	// S1_SPID_DIE7
	{SPR1_SPID,16,10,SA1_NULL,S1_SPID_DIE9,0,0,"monsters/spiddie.md2",7},	// S1_SPID_DIE8
	{SPR1_SPID,17,10,SA1_NULL,S1_SPID_DIE10,0,0,"monsters/spiddie.md2",8},	// S1_SPID_DIE9
	{SPR1_SPID,18,30,SA1_NULL,S1_SPID_DIE11,0,0,"monsters/spiddie.md2",9},	// S1_SPID_DIE10
	{SPR1_SPID,18,-1,SA1_BossDeath,S1_NULL,0,0,"monsters/spiddie.md2",10},	// S1_SPID_DIE11
	{SPR1_BSPI,0,10,SA1_Look,S1_BSPI_STND2,0,0,"monsters/arach.md2",0},	// S1_BSPI_STND
	{SPR1_BSPI,1,10,SA1_Look,S1_BSPI_STND,0,0,"monsters/arach.md2",1},	// S1_BSPI_STND2
	{SPR1_BSPI,0,20,SA1_NULL,S1_BSPI_RUN1,0,0,"monsters/arach.md2",2},	// S1_BSPI_SIGHT
	{SPR1_BSPI,0,3,SA1_BabyMetal,S1_BSPI_RUN2,0,0,"monsters/arach.md2",3},	// S1_BSPI_RUN1
	{SPR1_BSPI,0,3,SA1_Chase,S1_BSPI_RUN3,0,0,"monsters/arach.md2",4},	// S1_BSPI_RUN2
	{SPR1_BSPI,1,3,SA1_Chase,S1_BSPI_RUN4,0,0,"monsters/arach.md2",5},	// S1_BSPI_RUN3
	{SPR1_BSPI,1,3,SA1_Chase,S1_BSPI_RUN5,0,0,"monsters/arach.md2",6},	// S1_BSPI_RUN4
	{SPR1_BSPI,2,3,SA1_Chase,S1_BSPI_RUN6,0,0,"monsters/arach.md2",7},	// S1_BSPI_RUN5
	{SPR1_BSPI,2,3,SA1_Chase,S1_BSPI_RUN7,0,0,"monsters/arach.md2",8},	// S1_BSPI_RUN6
	{SPR1_BSPI,3,3,SA1_BabyMetal,S1_BSPI_RUN8,0,0,"monsters/arach.md2",9},	// S1_BSPI_RUN7
	{SPR1_BSPI,3,3,SA1_Chase,S1_BSPI_RUN9,0,0,"monsters/arach.md2",10},	// S1_BSPI_RUN8
	{SPR1_BSPI,4,3,SA1_Chase,S1_BSPI_RUN10,0,0,"monsters/arach.md2",11},	// S1_BSPI_RUN9
	{SPR1_BSPI,4,3,SA1_Chase,S1_BSPI_RUN11,0,0,"monsters/arach.md2",12},	// S1_BSPI_RUN10
	{SPR1_BSPI,5,3,SA1_Chase,S1_BSPI_RUN12,0,0,"monsters/arach.md2",13},	// S1_BSPI_RUN11
	{SPR1_BSPI,5,3,SA1_Chase,S1_BSPI_RUN1,0,0,"monsters/arach.md2",14},	// S1_BSPI_RUN12
	{SPR1_BSPI,32768,20,SA1_FaceTarget,S1_BSPI_ATK2,0,0,"monsters/arach.md2",15},	// S1_BSPI_ATK1
	{SPR1_BSPI,32774,4,SA1_BspiAttack,S1_BSPI_ATK3,0,0,"monsters/arach.md2",16},	// S1_BSPI_ATK2
	{SPR1_BSPI,32775,4,SA1_NULL,S1_BSPI_ATK4,0,0,"monsters/arachatk.md2",0},	// S1_BSPI_ATK3
	{SPR1_BSPI,32775,1,SA1_SpidRefire,S1_BSPI_ATK2,0,0,"monsters/arachatk.md2",0},	// S1_BSPI_ATK4
	{SPR1_BSPI,8,3,SA1_NULL,S1_BSPI_PAIN2,0,0,"monsters/arach.md2",17},	// S1_BSPI_PAIN
	{SPR1_BSPI,8,3,SA1_Pain,S1_BSPI_RUN1,0,0,"monsters/arach.md2",18},	// S1_BSPI_PAIN2
	{SPR1_BSPI,9,20,SA1_Scream,S1_BSPI_DIE2,0,0,"monsters/arachdie.md2",0},	// S1_BSPI_DIE1
	{SPR1_BSPI,10,7,SA1_Fall,S1_BSPI_DIE3,0,0,"monsters/arachdie.md2",1},	// S1_BSPI_DIE2
	{SPR1_BSPI,11,7,SA1_NULL,S1_BSPI_DIE4,0,0,"monsters/arachdie.md2",2},	// S1_BSPI_DIE3
	{SPR1_BSPI,12,7,SA1_NULL,S1_BSPI_DIE5,0,0,"monsters/arachdie.md2",3},	// S1_BSPI_DIE4
	{SPR1_BSPI,13,7,SA1_NULL,S1_BSPI_DIE6,0,0,"monsters/arachdie.md2",4},	// S1_BSPI_DIE5
	{SPR1_BSPI,14,7,SA1_NULL,S1_BSPI_DIE7,0,0,"monsters/arachdie.md2",5},	// S1_BSPI_DIE6
	{SPR1_BSPI,15,-1,SA1_BossDeath,S1_NULL,0,0,"monsters/arachdie.md2",6},	// S1_BSPI_DIE7
	{SPR1_BSPI,15,5,SA1_NULL,S1_BSPI_RAISE2,0,0,"monsters/arachdie.md2",7},	// S1_BSPI_RAISE1
	{SPR1_BSPI,14,5,SA1_NULL,S1_BSPI_RAISE3,0,0,"monsters/arachdie.md2",8},	// S1_BSPI_RAISE2
	{SPR1_BSPI,13,5,SA1_NULL,S1_BSPI_RAISE4,0,0,"monsters/arachdie.md2",9},	// S1_BSPI_RAISE3
	{SPR1_BSPI,12,5,SA1_NULL,S1_BSPI_RAISE5,0,0,"monsters/arachdie.md2",10},	// S1_BSPI_RAISE4
	{SPR1_BSPI,11,5,SA1_NULL,S1_BSPI_RAISE6,0,0,"monsters/arachdie.md2",11},	// S1_BSPI_RAISE5
	{SPR1_BSPI,10,5,SA1_NULL,S1_BSPI_RAISE7,0,0,"monsters/arachdie.md2",12},	// S1_BSPI_RAISE6
	{SPR1_BSPI,9,5,SA1_NULL,S1_BSPI_RUN1,0,0,"monsters/arachdie.md2",13},	// S1_BSPI_RAISE7
	{SPR1_APLS,32768,5,SA1_NULL,S1_ARACH_PLAZ2,0,0,"missiles/aplasma.md2",0},	// S1_ARACH_PLAZ
	{SPR1_APLS,32769,5,SA1_NULL,S1_ARACH_PLAZ,0,0,"missiles/aplasma.md2",1},	// S1_ARACH_PLAZ2
	{SPR1_APBX,32768,5,SA1_NULL,S1_ARACH_PLEX2,0,0},	// S1_ARACH_PLEX
	{SPR1_APBX,32769,5,SA1_NULL,S1_ARACH_PLEX3,0,0},	// S1_ARACH_PLEX2
	{SPR1_APBX,32770,5,SA1_NULL,S1_ARACH_PLEX4,0,0},	// S1_ARACH_PLEX3
	{SPR1_APBX,32771,5,SA1_NULL,S1_ARACH_PLEX5,0,0},	// S1_ARACH_PLEX4
	{SPR1_APBX,32772,5,SA1_NULL,S1_NULL,0,0},	// S1_ARACH_PLEX5
	{SPR1_CYBR,0,10,SA1_Look,S1_CYBER_STND2,0,0,"monsters/cyber.md2",0},	// S1_CYBER_STND
	{SPR1_CYBR,1,10,SA1_Look,S1_CYBER_STND,0,0,"monsters/cyber.md2",1},	// S1_CYBER_STND2
	{SPR1_CYBR,0,3,SA1_Hoof,S1_CYBER_RUN2,0,0,"monsters/cyber.md2",2},	// S1_CYBER_RUN1
	{SPR1_CYBR,0,3,SA1_Chase,S1_CYBER_RUN3,0,0,"monsters/cyber.md2",3},	// S1_CYBER_RUN2
	{SPR1_CYBR,1,3,SA1_Chase,S1_CYBER_RUN4,0,0,"monsters/cyber.md2",4},	// S1_CYBER_RUN3
	{SPR1_CYBR,1,3,SA1_Chase,S1_CYBER_RUN5,0,0,"monsters/cyber.md2",5},	// S1_CYBER_RUN4
	{SPR1_CYBR,2,3,SA1_Chase,S1_CYBER_RUN6,0,0,"monsters/cyber.md2",6},	// S1_CYBER_RUN5
	{SPR1_CYBR,2,3,SA1_Chase,S1_CYBER_RUN7,0,0,"monsters/cyber.md2",7},	// S1_CYBER_RUN6
	{SPR1_CYBR,3,3,SA1_Metal,S1_CYBER_RUN8,0,0,"monsters/cyber.md2",8},	// S1_CYBER_RUN7
	{SPR1_CYBR,3,3,SA1_Chase,S1_CYBER_RUN1,0,0,"monsters/cyber.md2",9},	// S1_CYBER_RUN8
	{SPR1_CYBR,4,6,SA1_FaceTarget,S1_CYBER_ATK2,0,0,"monsters/cyber.md2",10},	// S1_CYBER_ATK1
	{SPR1_CYBR,5,12,SA1_CyberAttack,S1_CYBER_ATK3,0,0,"monsters/cyberatk.md2",0},	// S1_CYBER_ATK2
	{SPR1_CYBR,4,12,SA1_FaceTarget,S1_CYBER_ATK4,0,0,"monsters/cyber.md2",10},	// S1_CYBER_ATK3
	{SPR1_CYBR,5,12,SA1_CyberAttack,S1_CYBER_ATK5,0,0,"monsters/cyberatk.md2",0},	// S1_CYBER_ATK4
	{SPR1_CYBR,4,12,SA1_FaceTarget,S1_CYBER_ATK6,0,0,"monsters/cyber.md2",10},	// S1_CYBER_ATK5
	{SPR1_CYBR,5,12,SA1_CyberAttack,S1_CYBER_RUN1,0,0,"monsters/cyberatk.md2",0},	// S1_CYBER_ATK6
	{SPR1_CYBR,6,10,SA1_Pain,S1_CYBER_RUN1,0,0,"monsters/cyber.md2",11},	// S1_CYBER_PAIN
	{SPR1_CYBR,7,10,SA1_NULL,S1_CYBER_DIE2,0,0,"monsters/cyberdie.md2",0},	// S1_CYBER_DIE1
	{SPR1_CYBR,8,10,SA1_Scream,S1_CYBER_DIE3,0,0,"monsters/cyberdie.md2",1},	// S1_CYBER_DIE2
	{SPR1_CYBR,9,10,SA1_NULL,S1_CYBER_DIE4,0,0,"monsters/cyberdie.md2",2},	// S1_CYBER_DIE3
	{SPR1_CYBR,10,10,SA1_NULL,S1_CYBER_DIE5,0,0,"monsters/cyberdie.md2",3},	// S1_CYBER_DIE4
	{SPR1_CYBR,11,10,SA1_NULL,S1_CYBER_DIE6,0,0,"monsters/cyberdie.md2",4},	// S1_CYBER_DIE5
	{SPR1_CYBR,12,10,SA1_Fall,S1_CYBER_DIE7,0,0,"monsters/cyberdie.md2",5},	// S1_CYBER_DIE6
	{SPR1_CYBR,13,10,SA1_NULL,S1_CYBER_DIE8,0,0,"monsters/cyberdie.md2",6},	// S1_CYBER_DIE7
	{SPR1_CYBR,14,10,SA1_NULL,S1_CYBER_DIE9,0,0,"monsters/cyberdie.md2",7},	// S1_CYBER_DIE8
	{SPR1_CYBR,15,30,SA1_NULL,S1_CYBER_DIE10,0,0,"monsters/cyberdie.md2",8},	// S1_CYBER_DIE9
	{SPR1_CYBR,15,-1,SA1_BossDeath,S1_NULL,0,0,"monsters/cyberdie.md2",9},	// S1_CYBER_DIE10
	{SPR1_PAIN,0,10,SA1_Look,S1_PAIN_STND,0,0,"monsters/pain.md2",0},	// S1_PAIN_STND
	{SPR1_PAIN,0,3,SA1_Chase,S1_PAIN_RUN2,0,0,"monsters/pain.md2",1},	// S1_PAIN_RUN1
	{SPR1_PAIN,0,3,SA1_Chase,S1_PAIN_RUN3,0,0,"monsters/pain.md2",2},	// S1_PAIN_RUN2
	{SPR1_PAIN,1,3,SA1_Chase,S1_PAIN_RUN4,0,0,"monsters/pain.md2",3},	// S1_PAIN_RUN3
	{SPR1_PAIN,1,3,SA1_Chase,S1_PAIN_RUN5,0,0,"monsters/pain.md2",4},	// S1_PAIN_RUN4
	{SPR1_PAIN,2,3,SA1_Chase,S1_PAIN_RUN6,0,0,"monsters/pain.md2",5},	// S1_PAIN_RUN5
	{SPR1_PAIN,2,3,SA1_Chase,S1_PAIN_RUN1,0,0,"monsters/pain.md2",6},	// S1_PAIN_RUN6
	{SPR1_PAIN,3,5,SA1_FaceTarget,S1_PAIN_ATK2,0,0,"monsters/painatk.md2",0},	// S1_PAIN_ATK1
	{SPR1_PAIN,4,5,SA1_FaceTarget,S1_PAIN_ATK3,0,0,"monsters/painatk.md2",1},	// S1_PAIN_ATK2
	{SPR1_PAIN,32773,5,SA1_FaceTarget,S1_PAIN_ATK4,0,0,"monsters/painatk.md2",2},	// S1_PAIN_ATK3
	{SPR1_PAIN,32773,0,SA1_PainAttack,S1_PAIN_RUN1,0,0,"monsters/painatk.md2",2},	// S1_PAIN_ATK4
	{SPR1_PAIN,6,6,SA1_NULL,S1_PAIN_PAIN2,0,0,"monsters/pain.md2",7},	// S1_PAIN_PAIN
	{SPR1_PAIN,6,6,SA1_Pain,S1_PAIN_RUN1,0,0,"monsters/pain.md2",8},	// S1_PAIN_PAIN2
	{SPR1_PAIN,32775,8,SA1_NULL,S1_PAIN_DIE2,0,0},	// S1_PAIN_DIE1
	{SPR1_PAIN,32776,8,SA1_Scream,S1_PAIN_DIE3,0,0},	// S1_PAIN_DIE2
	{SPR1_PAIN,32777,8,SA1_NULL,S1_PAIN_DIE4,0,0},	// S1_PAIN_DIE3
	{SPR1_PAIN,32778,8,SA1_NULL,S1_PAIN_DIE5,0,0},	// S1_PAIN_DIE4
	{SPR1_PAIN,32779,8,SA1_PainDie,S1_PAIN_DIE6,0,0},	// S1_PAIN_DIE5
	{SPR1_PAIN,32780,8,SA1_NULL,S1_NULL,0,0},	// S1_PAIN_DIE6
	{SPR1_PAIN,12,8,SA1_NULL,S1_PAIN_RAISE2,0,0},	// S1_PAIN_RAISE1
	{SPR1_PAIN,11,8,SA1_NULL,S1_PAIN_RAISE3,0,0},	// S1_PAIN_RAISE2
	{SPR1_PAIN,10,8,SA1_NULL,S1_PAIN_RAISE4,0,0},	// S1_PAIN_RAISE3
	{SPR1_PAIN,9,8,SA1_NULL,S1_PAIN_RAISE5,0,0},	// S1_PAIN_RAISE4
	{SPR1_PAIN,8,8,SA1_NULL,S1_PAIN_RAISE6,0,0},	// S1_PAIN_RAISE5
	{SPR1_PAIN,7,8,SA1_NULL,S1_PAIN_RUN1,0,0},	// S1_PAIN_RAISE6
	{SPR1_SSWV,0,10,SA1_Look,S1_SSWV_STND2,0,0,"monsters/ss.md2",0},	// S1_SSWV_STND
	{SPR1_SSWV,1,10,SA1_Look,S1_SSWV_STND,0,0,"monsters/ss.md2",1},	// S1_SSWV_STND2
	{SPR1_SSWV,0,3,SA1_Chase,S1_SSWV_RUN2,0,0,"monsters/ss.md2",2},	// S1_SSWV_RUN1
	{SPR1_SSWV,0,3,SA1_Chase,S1_SSWV_RUN3,0,0,"monsters/ss.md2",3},	// S1_SSWV_RUN2
	{SPR1_SSWV,1,3,SA1_Chase,S1_SSWV_RUN4,0,0,"monsters/ss.md2",4},	// S1_SSWV_RUN3
	{SPR1_SSWV,1,3,SA1_Chase,S1_SSWV_RUN5,0,0,"monsters/ss.md2",5},	// S1_SSWV_RUN4
	{SPR1_SSWV,2,3,SA1_Chase,S1_SSWV_RUN6,0,0,"monsters/ss.md2",6},	// S1_SSWV_RUN5
	{SPR1_SSWV,2,3,SA1_Chase,S1_SSWV_RUN7,0,0,"monsters/ss.md2",7},	// S1_SSWV_RUN6
	{SPR1_SSWV,3,3,SA1_Chase,S1_SSWV_RUN8,0,0,"monsters/ss.md2",8},	// S1_SSWV_RUN7
	{SPR1_SSWV,3,3,SA1_Chase,S1_SSWV_RUN1,0,0,"monsters/ss.md2",9},	// S1_SSWV_RUN8
	{SPR1_SSWV,4,10,SA1_FaceTarget,S1_SSWV_ATK2,0,0,"monsters/ss.md2",10},	// S1_SSWV_ATK1
	{SPR1_SSWV,5,10,SA1_FaceTarget,S1_SSWV_ATK3,0,0,"monsters/ss.md2",11},	// S1_SSWV_ATK2
	{SPR1_SSWV,32774,4,SA1_CPosAttack,S1_SSWV_ATK4,0,0,"monsters/ss.md2",12},	// S1_SSWV_ATK3
	{SPR1_SSWV,5,6,SA1_FaceTarget,S1_SSWV_ATK5,0,0,"monsters/ss.md2",13},	// S1_SSWV_ATK4
	{SPR1_SSWV,32774,4,SA1_CPosAttack,S1_SSWV_ATK6,0,0,"monsters/ss.md2",12},	// S1_SSWV_ATK5
	{SPR1_SSWV,5,1,SA1_CPosRefire,S1_SSWV_ATK2,0,0,"monsters/ss.md2",13},	// S1_SSWV_ATK6
	{SPR1_SSWV,7,3,SA1_NULL,S1_SSWV_PAIN2,0,0,"monsters/ss.md2",14},	// S1_SSWV_PAIN
	{SPR1_SSWV,7,3,SA1_Pain,S1_SSWV_RUN1,0,0,"monsters/ss.md2",15},	// S1_SSWV_PAIN2
	{SPR1_SSWV,8,5,SA1_NULL,S1_SSWV_DIE2,0,0,"monsters/ssdie.md2",0},	// S1_SSWV_DIE1
	{SPR1_SSWV,9,5,SA1_Scream,S1_SSWV_DIE3,0,0,"monsters/ssdie.md2",1},	// S1_SSWV_DIE2
	{SPR1_SSWV,10,5,SA1_Fall,S1_SSWV_DIE4,0,0,"monsters/ssdie.md2",2},	// S1_SSWV_DIE3
	{SPR1_SSWV,11,5,SA1_NULL,S1_SSWV_DIE5,0,0,"monsters/ssdie.md2",3},	// S1_SSWV_DIE4
	{SPR1_SSWV,12,-1,SA1_NULL,S1_NULL,0,0,"monsters/ssdie.md2",4},	// S1_SSWV_DIE5
	{SPR1_SSWV,13,5,SA1_NULL,S1_SSWV_XDIE2,0,0,"gibs/ssgib.md2",0},	// S1_SSWV_XDIE1
	{SPR1_SSWV,14,5,SA1_XScream,S1_SSWV_XDIE3,0,0,"gibs/ssgib.md2",1},	// S1_SSWV_XDIE2
	{SPR1_SSWV,15,5,SA1_Fall,S1_SSWV_XDIE4,0,0,"gibs/ssgib.md2",2},	// S1_SSWV_XDIE3
	{SPR1_SSWV,16,5,SA1_NULL,S1_SSWV_XDIE5,0,0,"gibs/ssgib.md2",3},	// S1_SSWV_XDIE4
	{SPR1_SSWV,17,5,SA1_NULL,S1_SSWV_XDIE6,0,0,"gibs/ssgib.md2",4},	// S1_SSWV_XDIE5
	{SPR1_SSWV,18,5,SA1_NULL,S1_SSWV_XDIE7,0,0,"gibs/ssgib.md2",5},	// S1_SSWV_XDIE6
	{SPR1_SSWV,19,5,SA1_NULL,S1_SSWV_XDIE8,0,0,"gibs/ssgib.md2",6},	// S1_SSWV_XDIE7
	{SPR1_SSWV,20,5,SA1_NULL,S1_SSWV_XDIE9,0,0,"gibs/ssgib.md2",7},	// S1_SSWV_XDIE8
	{SPR1_SSWV,21,-1,SA1_NULL,S1_NULL,0,0,"gibs/ssgib.md2",8},	// S1_SSWV_XDIE9
	{SPR1_SSWV,12,5,SA1_NULL,S1_SSWV_RAISE2,0,0,"monsters/ssdie.md2",5},	// S1_SSWV_RAISE1
	{SPR1_SSWV,11,5,SA1_NULL,S1_SSWV_RAISE3,0,0,"monsters/ssdie.md2",6},	// S1_SSWV_RAISE2
	{SPR1_SSWV,10,5,SA1_NULL,S1_SSWV_RAISE4,0,0,"monsters/ssdie.md2",7},	// S1_SSWV_RAISE3
	{SPR1_SSWV,9,5,SA1_NULL,S1_SSWV_RAISE5,0,0,"monsters/ssdie.md2",8},	// S1_SSWV_RAISE4
	{SPR1_SSWV,8,5,SA1_NULL,S1_SSWV_RUN1,0,0,"monsters/ssdie.md2",9},	// S1_SSWV_RAISE5
	{SPR1_KEEN,0,-1,SA1_NULL,S1_KEENSTND,0,0},	// S1_KEENSTND
	{SPR1_KEEN,0,6,SA1_NULL,S1_COMMKEEN2,0,0},	// S1_COMMKEEN
	{SPR1_KEEN,1,6,SA1_NULL,S1_COMMKEEN3,0,0},	// S1_COMMKEEN2
	{SPR1_KEEN,2,6,SA1_Scream,S1_COMMKEEN4,0,0},	// S1_COMMKEEN3
	{SPR1_KEEN,3,6,SA1_NULL,S1_COMMKEEN5,0,0},	// S1_COMMKEEN4
	{SPR1_KEEN,4,6,SA1_NULL,S1_COMMKEEN6,0,0},	// S1_COMMKEEN5
	{SPR1_KEEN,5,6,SA1_NULL,S1_COMMKEEN7,0,0},	// S1_COMMKEEN6
	{SPR1_KEEN,6,6,SA1_NULL,S1_COMMKEEN8,0,0},	// S1_COMMKEEN7
	{SPR1_KEEN,7,6,SA1_NULL,S1_COMMKEEN9,0,0},	// S1_COMMKEEN8
	{SPR1_KEEN,8,6,SA1_NULL,S1_COMMKEEN10,0,0},	// S1_COMMKEEN9
	{SPR1_KEEN,9,6,SA1_NULL,S1_COMMKEEN11,0,0},	// S1_COMMKEEN10
	{SPR1_KEEN,10,6,SA1_KeenDie,S1_COMMKEEN12,0,0},// S1_COMMKEEN11
	{SPR1_KEEN,11,-1,SA1_NULL,S1_NULL,0,0},		// S1_COMMKEEN12
	{SPR1_KEEN,12,4,SA1_NULL,S1_KEENPAIN2,0,0},	// S1_KEENPAIN
	{SPR1_KEEN,12,8,SA1_Pain,S1_KEENSTND,0,0},	// S1_KEENPAIN2
	{SPR1_BBRN,0,-1,SA1_NULL,S1_NULL,0,0},		// S1_BRAIN
	{SPR1_BBRN,1,36,SA1_BrainPain,S1_BRAIN,0,0},	// S1_BRAIN_PAIN
	{SPR1_BBRN,0,100,SA1_BrainScream,S1_BRAIN_DIE2,0,0},	// S1_BRAIN_DIE1
	{SPR1_BBRN,0,10,SA1_NULL,S1_BRAIN_DIE3,0,0},	// S1_BRAIN_DIE2
	{SPR1_BBRN,0,10,SA1_NULL,S1_BRAIN_DIE4,0,0},	// S1_BRAIN_DIE3
	{SPR1_BBRN,0,-1,SA1_BrainDie,S1_NULL,0,0},	// S1_BRAIN_DIE4
	{SPR1_SSWV,0,10,SA1_Look,S1_BRAINEYE,0,0},	// S1_BRAINEYE
	{SPR1_SSWV,0,181,SA1_BrainAwake,S1_BRAINEYE1,0,0},	// S1_BRAINEYESEE
	{SPR1_SSWV,0,150,SA1_BrainSpit,S1_BRAINEYE1,0,0},	// S1_BRAINEYE1
	{SPR1_BOSF,32768,3,SA1_SpawnSound,S1_SPAWN2,0,0,"monsters/cube.md2",0},	// S1_SPAWN1
	{SPR1_BOSF,32769,3,SA1_SpawnFly,S1_SPAWN3,0,0,"monsters/cube.md2",1},	// S1_SPAWN2
	{SPR1_BOSF,32770,3,SA1_SpawnFly,S1_SPAWN4,0,0,"monsters/cube.md2",2},	// S1_SPAWN3
	{SPR1_BOSF,32771,3,SA1_SpawnFly,S1_SPAWN1,0,0,"monsters/cube.md2",3},	// S1_SPAWN4
	{SPR1_FIRE,32768,4,SA1_Fire,S1_SPAWNFIRE2,0,0},	// S1_SPAWNFIRE1
	{SPR1_FIRE,32769,4,SA1_Fire,S1_SPAWNFIRE3,0,0},	// S1_SPAWNFIRE2
	{SPR1_FIRE,32770,4,SA1_Fire,S1_SPAWNFIRE4,0,0},	// S1_SPAWNFIRE3
	{SPR1_FIRE,32771,4,SA1_Fire,S1_SPAWNFIRE5,0,0},	// S1_SPAWNFIRE4
	{SPR1_FIRE,32772,4,SA1_Fire,S1_SPAWNFIRE6,0,0},	// S1_SPAWNFIRE5
	{SPR1_FIRE,32773,4,SA1_Fire,S1_SPAWNFIRE7,0,0},	// S1_SPAWNFIRE6
	{SPR1_FIRE,32774,4,SA1_Fire,S1_SPAWNFIRE8,0,0},	// S1_SPAWNFIRE7
	{SPR1_FIRE,32775,4,SA1_Fire,S1_NULL,0,0},		// S1_SPAWNFIRE8
	{SPR1_MISL,32769,10,SA1_NULL,S1_BRAINEXPLODE2,0,0},	// S1_BRAINEXPLODE1
	{SPR1_MISL,32770,10,SA1_NULL,S1_BRAINEXPLODE3,0,0},	// S1_BRAINEXPLODE2
	{SPR1_MISL,32771,10,SA1_BrainExplode,S1_NULL,0,0},	// S1_BRAINEXPLODE3
	{SPR1_ARM1,0,6,SA1_NULL,S1_ARM1A,0,0,"items/grnarmor.md2",0},	// S1_ARM1
	{SPR1_ARM1,32769,7,SA1_NULL,S1_ARM1,0,0,"items/grnarmor.md2",1},	// S1_ARM1A
	{SPR1_ARM2,0,6,SA1_NULL,S1_ARM2A,0,0,"items/bluarmor.md2",0},	// S1_ARM2
	{SPR1_ARM2,32769,6,SA1_NULL,S1_ARM2,0,0,"items/bluarmor.md2",1},	// S1_ARM2A
	{SPR1_BAR1,0,6,SA1_NULL,S1_BAR2,0,0,"objects/barrel.md2",0},	// S1_BAR1
	{SPR1_BAR1,1,6,SA1_NULL,S1_BAR1,0,0,"objects/barrel.md2",1},	// S1_BAR2
	{SPR1_BEXP,32768,5,SA1_NULL,S1_BEXP2,0,0},	// S1_BEXP
	{SPR1_BEXP,32769,5,SA1_Scream,S1_BEXP3,0,0},	// S1_BEXP2
	{SPR1_BEXP,32770,5,SA1_NULL,S1_BEXP4,0,0},	// S1_BEXP3
	{SPR1_BEXP,32771,10,SA1_Explode,S1_BEXP5,0,0},	// S1_BEXP4
	{SPR1_BEXP,32772,10,SA1_NULL,S1_NULL,0,0},	// S1_BEXP5
	{SPR1_FCAN,32768,4,SA1_NULL,S1_BBAR2,0,0,"lights/fbarrel.md2",0},	// S1_BBAR1
	{SPR1_FCAN,32769,4,SA1_NULL,S1_BBAR3,0,0,"lights/fbarrel.md2",1},	// S1_BBAR2
	{SPR1_FCAN,32770,4,SA1_NULL,S1_BBAR1,0,0,"lights/fbarrel.md2",2},	// S1_BBAR3
	{SPR1_BON1,0,6,SA1_NULL,S1_BON1A,0,0,"items/bottle.md2",0},	// S1_BON1
	{SPR1_BON1,1,6,SA1_NULL,S1_BON1B,0,0,"items/bottle.md2",1},	// S1_BON1A
	{SPR1_BON1,2,6,SA1_NULL,S1_BON1C,0,0,"items/bottle.md2",2},	// S1_BON1B
	{SPR1_BON1,3,6,SA1_NULL,S1_BON1D,0,0,"items/bottle.md2",3},	// S1_BON1C
	{SPR1_BON1,2,6,SA1_NULL,S1_BON1E,0,0,"items/bottle.md2",4},	// S1_BON1D
	{SPR1_BON1,1,6,SA1_NULL,S1_BON1,0,0,"items/bottle.md2",5},	// S1_BON1E
	{SPR1_BON2,0,6,SA1_NULL,S1_BON2A,0,0,"items/helmet.md2",0},	// S1_BON2
	{SPR1_BON2,1,6,SA1_NULL,S1_BON2B,0,0,"items/helmet.md2",1},	// S1_BON2A
	{SPR1_BON2,2,6,SA1_NULL,S1_BON2C,0,0,"items/helmet.md2",2},	// S1_BON2B
	{SPR1_BON2,3,6,SA1_NULL,S1_BON2D,0,0,"items/helmet.md2",3},	// S1_BON2C
	{SPR1_BON2,2,6,SA1_NULL,S1_BON2E,0,0,"items/helmet.md2",4},	// S1_BON2D
	{SPR1_BON2,1,6,SA1_NULL,S1_BON2,0,0,"items/helmet.md2",5},	// S1_BON2E
	{SPR1_BKEY,0,10,SA1_NULL,S1_BKEY2,0,0,"keys/bluecard.md2",0},	// S1_BKEY
	{SPR1_BKEY,32769,10,SA1_NULL,S1_BKEY,0,0,"keys/bluecard.md2",1},	// S1_BKEY2
	{SPR1_RKEY,0,10,SA1_NULL,S1_RKEY2,0,0,"keys/redcard.md2",0},	// S1_RKEY
	{SPR1_RKEY,32769,10,SA1_NULL,S1_RKEY,0,0,"keys/redcard.md2",1},	// S1_RKEY2
	{SPR1_YKEY,0,10,SA1_NULL,S1_YKEY2,0,0,"keys/yelcard.md2",0},	// S1_YKEY
	{SPR1_YKEY,32769,10,SA1_NULL,S1_YKEY,0,0,"keys/yelcard.md2",1},	// S1_YKEY2
	{SPR1_BSKU,0,10,SA1_NULL,S1_BSKULL2,0,0,"keys/blueskul.md2",0},	// S1_BSKULL
	{SPR1_BSKU,32769,10,SA1_NULL,S1_BSKULL,0,0,"keys/blueskul.md2",1},	// S1_BSKULL2
	{SPR1_RSKU,0,10,SA1_NULL,S1_RSKULL2,0,0,"keys/redskul.md2",0},	// S1_RSKULL
	{SPR1_RSKU,32769,10,SA1_NULL,S1_RSKULL,0,0,"keys/redskul.md2",1},	// S1_RSKULL2
	{SPR1_YSKU,0,10,SA1_NULL,S1_YSKULL2,0,0,"keys/yelskul.md2",0},	// S1_YSKULL
	{SPR1_YSKU,32769,10,SA1_NULL,S1_YSKULL,0,0,"keys/yelskul.md2",1},	// S1_YSKULL2
	{SPR1_STIM,0,-1,SA1_NULL,S1_NULL,0,0,"items/stim.md2",0},	// S1_STIM
	{SPR1_MEDI,0,-1,SA1_NULL,S1_NULL,0,0,"items/medi.md2",0},	// S1_MEDI
	{SPR1_SOUL,32768,6,SA1_NULL,S1_SOUL2,0,0,"items/soul.md2",0},	// S1_SOUL
	{SPR1_SOUL,32769,6,SA1_NULL,S1_SOUL3,0,0,"items/soul.md2",1},	// S1_SOUL2
	{SPR1_SOUL,32770,6,SA1_NULL,S1_SOUL4,0,0,"items/soul.md2",2},	// S1_SOUL3
	{SPR1_SOUL,32771,6,SA1_NULL,S1_SOUL5,0,0,"items/soul.md2",3},	// S1_SOUL4
	{SPR1_SOUL,32770,6,SA1_NULL,S1_SOUL6,0,0,"items/soul.md2",4},	// S1_SOUL5
	{SPR1_SOUL,32769,6,SA1_NULL,S1_SOUL,0,0,"items/soul.md2",5},	// S1_SOUL6
	{SPR1_PINV,32768,6,SA1_NULL,S1_PINV2,0,0,"items/invuln.md2",0},	// S1_PINV
	{SPR1_PINV,32769,6,SA1_NULL,S1_PINV3,0,0,"items/invuln.md2",1},	// S1_PINV2
	{SPR1_PINV,32770,6,SA1_NULL,S1_PINV4,0,0,"items/invuln.md2",2},	// S1_PINV3
	{SPR1_PINV,32771,6,SA1_NULL,S1_PINV,0,0,"items/invuln.md2",3},	// S1_PINV4
	{SPR1_PSTR,32768,-1,SA1_NULL,S1_NULL,0,0,"items/bersek.md2",0},	// S1_PSTR
	{SPR1_PINS,32768,6,SA1_NULL,S1_PINS2,0,0,"items/invis.md2",0},	// S1_PINS
	{SPR1_PINS,32769,6,SA1_NULL,S1_PINS3,0,0,"items/invis.md2",1},	// S1_PINS2
	{SPR1_PINS,32770,6,SA1_NULL,S1_PINS4,0,0,"items/invis.md2",2},	// S1_PINS3
	{SPR1_PINS,32771,6,SA1_NULL,S1_PINS,0,0,"items/invis.md2",3},	// S1_PINS4
	{SPR1_MEGA,32768,6,SA1_NULL,S1_MEGA2,0,0,"items/mega.md2",0},	// S1_MEGA
	{SPR1_MEGA,32769,6,SA1_NULL,S1_MEGA3,0,0,"items/mega.md2",1},	// S1_MEGA2
	{SPR1_MEGA,32770,6,SA1_NULL,S1_MEGA4,0,0,"items/mega.md2",2},	// S1_MEGA3
	{SPR1_MEGA,32771,6,SA1_NULL,S1_MEGA,0,0,"items/mega.md2",3},	// S1_MEGA4
	{SPR1_SUIT,32768,-1,SA1_NULL,S1_NULL,0,0,"items/suit.md2",0},	// S1_SUIT
	{SPR1_PMAP,32768,6,SA1_NULL,S1_PMAP2,0,0,"items/map.md2",0},	// S1_PMAP
	{SPR1_PMAP,32769,6,SA1_NULL,S1_PMAP3,0,0,"items/map.md2",1},	// S1_PMAP2
	{SPR1_PMAP,32770,6,SA1_NULL,S1_PMAP4,0,0,"items/map.md2",2},	// S1_PMAP3
	{SPR1_PMAP,32771,6,SA1_NULL,S1_PMAP5,0,0,"items/map.md2",3},	// S1_PMAP4
	{SPR1_PMAP,32770,6,SA1_NULL,S1_PMAP6,0,0,"items/map.md2",4},	// S1_PMAP5
	{SPR1_PMAP,32769,6,SA1_NULL,S1_PMAP,0,0,"items/map.md2",5},	// S1_PMAP6
	{SPR1_PVIS,32768,6,SA1_NULL,S1_PVIS2,0,0,"items/visor.md2",0},	// S1_PVIS
	{SPR1_PVIS,1,6,SA1_NULL,S1_PVIS,0,0,"items/visor.md2",1},	// S1_PVIS2
	{SPR1_CLIP,0,-1,SA1_NULL,S1_NULL,0,0,"ammo/clip.md2",0},	// S1_CLIP
	{SPR1_AMMO,0,-1,SA1_NULL,S1_NULL,0,0,"ammo/ammo.md2",0},	// S1_AMMO
	{SPR1_ROCK,0,-1,SA1_NULL,S1_NULL,0,0,"ammo/rocket.md2",0},	// S1_ROCK
	{SPR1_BROK,0,-1,SA1_NULL,S1_NULL,0,0,"ammo/rocktbox.md2",0},	// S1_BROK
	{SPR1_CELL,0,-1,SA1_NULL,S1_NULL,0,0,"ammo/cell.md2",0},	// S1_CELL
	{SPR1_CELP,0,-1,SA1_NULL,S1_NULL,0,0,"ammo/bigcells.md2",0},	// S1_CELP
	{SPR1_SHEL,0,-1,SA1_NULL,S1_NULL,0,0,"ammo/shells.md2",0},	// S1_SHEL
	{SPR1_SBOX,0,-1,SA1_NULL,S1_NULL,0,0,"ammo/shellbox.md2",0},	// S1_SBOX
	{SPR1_BPAK,0,-1,SA1_NULL,S1_NULL,0,0,"ammo/backpack.md2",0},	// S1_BPAK
	{SPR1_BFUG,0,-1,SA1_NULL,S1_NULL,0,0,"weapons/g_bfg.md2",0},	// S1_BFUG
	{SPR1_MGUN,0,-1,SA1_NULL,S1_NULL,0,0,"weapons/g_chain.md2",0},	// S1_MGUN
	{SPR1_CSAW,0,-1,SA1_NULL,S1_NULL,0,0,"weapons/g_saw.md2",0},	// S1_CSAW
	{SPR1_LAUN,0,-1,SA1_NULL,S1_NULL,0,0,"weapons/g_launch.md2",0},	// S1_LAUN
	{SPR1_PLAS,0,-1,SA1_NULL,S1_NULL,0,0,"weapons/g_plasma.md2",0},	// S1_PLAS
	{SPR1_SHOT,0,-1,SA1_NULL,S1_NULL,0,0,"weapons/g_shotg.md2",0},	// S1_SHOT
	{SPR1_SGN2,0,-1,SA1_NULL,S1_NULL,0,0,"weapons/g_super.md2",0},	// S1_SHOT2
	{SPR1_COLU,32768,-1,SA1_NULL,S1_NULL,0,0,"lights/lamp.md2",0},	// S1_COLU
	{SPR1_SMT2,0,-1,SA1_NULL,S1_NULL,0,0},	// S1_STALAG
	{SPR1_GOR1,0,10,SA1_NULL,S1_BLOODYTWITCH2,0,0,"bodies/twitch.md2",0},	// S1_BLOODYTWITCH
	{SPR1_GOR1,1,15,SA1_NULL,S1_BLOODYTWITCH3,0,0,"bodies/twitch.md2",1},	// S1_BLOODYTWITCH2
	{SPR1_GOR1,2,8,SA1_NULL,S1_BLOODYTWITCH4,0,0,"bodies/twitch.md2",2},	// S1_BLOODYTWITCH3
	{SPR1_GOR1,1,6,SA1_NULL,S1_BLOODYTWITCH,0,0,"bodies/twitch.md2",3},	// S1_BLOODYTWITCH4
	{SPR1_PLAY,13,-1,SA1_NULL,S1_NULL,0,0,"players/doomguy/tris.md2",15},	// S1_DEADTORSO
	{SPR1_PLAY,18,-1,SA1_NULL,S1_NULL,0,0,"players/doomguy/xplode.md2",8},	// S1_DEADBOTTOM
	{SPR1_POL2,0,-1,SA1_NULL,S1_NULL,0,0,"bodies/poleskls.md2",0},	// S1_HEADSONSTICK
	{SPR1_POL5,0,-1,SA1_NULL,S1_NULL,0,0,"gibs/gibs.md2",0},	// S1_GIBS
	{SPR1_POL4,0,-1,SA1_NULL,S1_NULL,0,0,"bodies/poleskul.md2",0},	// S1_HEADONASTICK
	{SPR1_POL3,32768,6,SA1_NULL,S1_HEADCANDLES2,0,0,"bodies/polescan.md2",0},	// S1_HEADCANDLES
	{SPR1_POL3,32769,6,SA1_NULL,S1_HEADCANDLES,0,0,"bodies/polescan.md2",1},	// S1_HEADCANDLES2
	{SPR1_POL1,0,-1,SA1_NULL,S1_NULL,0,0,"bodies/onstick1.md2",0},	// S1_DEADSTICK
	{SPR1_POL6,0,6,SA1_NULL,S1_LIVESTICK2,0,0,"bodies/onstick2.md2",0},	// S1_LIVESTICK
	{SPR1_POL6,1,8,SA1_NULL,S1_LIVESTICK,0,0,"bodies/onstick2.md2",1},	// S1_LIVESTICK2
	{SPR1_GOR2,0,-1,SA1_NULL,S1_NULL,0,0,"bodies/hangguy.md2",0},	// S1_MEAT2
	{SPR1_GOR3,0,-1,SA1_NULL,S1_NULL,0,0,"bodies/noleg.md2",0},	// S1_MEAT3
	{SPR1_GOR4,0,-1,SA1_NULL,S1_NULL,0,0,"bodies/legs.md2",0},	// S1_MEAT4
	{SPR1_GOR5,0,-1,SA1_NULL,S1_NULL,0,0,"bodies/leg.md2",0},	// S1_MEAT5
	{SPR1_SMIT,0,-1,SA1_NULL,S1_NULL,0,0,"objects/stub.md2",0},	// S1_STALAGTITE
	{SPR1_COL1,0,-1,SA1_NULL,S1_NULL,0,0,"objects/pillargt.md2",0},	// S1_TALLGRNCOL
	{SPR1_COL2,0,-1,SA1_NULL,S1_NULL,0,0,"objects/pillargs.md2",0},	// S1_SHRTGRNCOL
	{SPR1_COL3,0,-1,SA1_NULL,S1_NULL,0,0,"objects/pillarrt.md2",0},	// S1_TALLREDCOL
	{SPR1_COL4,0,-1,SA1_NULL,S1_NULL,0,0,"objects/pillarrs.md2",0},	// S1_SHRTREDCOL
	{SPR1_CAND,32768,-1,SA1_NULL,S1_NULL,0,0,"lights/candle.md2",0},	// S1_CANDLESTIK
	{SPR1_CBRA,32768,-1,SA1_NULL,S1_NULL,0,0,"lights/candabra.md2",0},	// S1_CANDELABRA
	{SPR1_COL6,0,-1,SA1_NULL,S1_NULL,0,0,"objects/pillarsk.md2",0},	// S1_SKULLCOL
	{SPR1_TRE1,0,-1,SA1_NULL,S1_NULL,0,0,"objects/tree.md2",0},	// S1_TORCHTREE
	{SPR1_TRE2,0,-1,SA1_NULL,S1_NULL,0,0,"objects/bigtree.md2",0},	// S1_BIGTREE
	{SPR1_ELEC,0,-1,SA1_NULL,S1_NULL,0,0,"objects/techpilr.md2",0},	// S1_TECHPILLAR
	{SPR1_CEYE,32768,6,SA1_NULL,S1_EVILEYE2,0,0,"objects/evileye.md2",0},	// S1_EVILEYE
	{SPR1_CEYE,32769,6,SA1_NULL,S1_EVILEYE3,0,0,"objects/evileye.md2",1},	// S1_EVILEYE2
	{SPR1_CEYE,32770,6,SA1_NULL,S1_EVILEYE4,0,0,"objects/evileye.md2",2},	// S1_EVILEYE3
	{SPR1_CEYE,32769,6,SA1_NULL,S1_EVILEYE,0,0,"objects/evileye.md2",3},	// S1_EVILEYE4
	{SPR1_FSKU,32768,6,SA1_NULL,S1_FLOATSKULL2,0,0,"objects/fskulls.md2",0},	// S1_FLOATSKULL
	{SPR1_FSKU,32769,6,SA1_NULL,S1_FLOATSKULL3,0,0,"objects/fskulls.md2",1},	// S1_FLOATSKULL2
	{SPR1_FSKU,32770,6,SA1_NULL,S1_FLOATSKULL,0,0,"objects/fskulls.md2",2},	// S1_FLOATSKULL3
	{SPR1_COL5,0,14,SA1_NULL,S1_HEARTCOL2,0,0,"objects/pillargh.md2",0},	// S1_HEARTCOL
	{SPR1_COL5,1,14,SA1_NULL,S1_HEARTCOL,0,0,"objects/pillargh.md2",1},	// S1_HEARTCOL2
	{SPR1_TBLU,32768,4,SA1_NULL,S1_BLUETORCH2,0,0,"lights/torchblu.md2",0},	// S1_BLUETORCH
	{SPR1_TBLU,32769,4,SA1_NULL,S1_BLUETORCH3,0,0,"lights/torchblu.md2",1},	// S1_BLUETORCH2
	{SPR1_TBLU,32770,4,SA1_NULL,S1_BLUETORCH4,0,0,"lights/torchblu.md2",2},	// S1_BLUETORCH3
	{SPR1_TBLU,32771,4,SA1_NULL,S1_BLUETORCH,0,0,"lights/torchblu.md2",3},	// S1_BLUETORCH4
	{SPR1_TGRN,32768,4,SA1_NULL,S1_GREENTORCH2,0,0,"lights/torchgrn.md2",0},	// S1_GREENTORCH
	{SPR1_TGRN,32769,4,SA1_NULL,S1_GREENTORCH3,0,0,"lights/torchgrn.md2",1},	// S1_GREENTORCH2
	{SPR1_TGRN,32770,4,SA1_NULL,S1_GREENTORCH4,0,0,"lights/torchgrn.md2",2},	// S1_GREENTORCH3
	{SPR1_TGRN,32771,4,SA1_NULL,S1_GREENTORCH,0,0,"lights/torchgrn.md2",3},	// S1_GREENTORCH4
	{SPR1_TRED,32768,4,SA1_NULL,S1_REDTORCH2,0,0,"lights/torchred.md2",0},	// S1_REDTORCH
	{SPR1_TRED,32769,4,SA1_NULL,S1_REDTORCH3,0,0,"lights/torchred.md2",1},	// S1_REDTORCH2
	{SPR1_TRED,32770,4,SA1_NULL,S1_REDTORCH4,0,0,"lights/torchred.md2",2},	// S1_REDTORCH3
	{SPR1_TRED,32771,4,SA1_NULL,S1_REDTORCH,0,0,"lights/torchred.md2",3},	// S1_REDTORCH4
	{SPR1_SMBT,32768,4,SA1_NULL,S1_BTORCHSHRT2,0,0,"lights/stickblu.md2",0},	// S1_BTORCHSHRT
	{SPR1_SMBT,32769,4,SA1_NULL,S1_BTORCHSHRT3,0,0,"lights/stickblu.md2",1},	// S1_BTORCHSHRT2
	{SPR1_SMBT,32770,4,SA1_NULL,S1_BTORCHSHRT4,0,0,"lights/stickblu.md2",2},	// S1_BTORCHSHRT3
	{SPR1_SMBT,32771,4,SA1_NULL,S1_BTORCHSHRT,0,0,"lights/stickblu.md2",3},	// S1_BTORCHSHRT4
	{SPR1_SMGT,32768,4,SA1_NULL,S1_GTORCHSHRT2,0,0,"lights/stickgrn.md2",0},	// S1_GTORCHSHRT
	{SPR1_SMGT,32769,4,SA1_NULL,S1_GTORCHSHRT3,0,0,"lights/stickgrn.md2",1},	// S1_GTORCHSHRT2
	{SPR1_SMGT,32770,4,SA1_NULL,S1_GTORCHSHRT4,0,0,"lights/stickgrn.md2",2},	// S1_GTORCHSHRT3
	{SPR1_SMGT,32771,4,SA1_NULL,S1_GTORCHSHRT,0,0,"lights/stickgrn.md2",3},	// S1_GTORCHSHRT4
	{SPR1_SMRT,32768,4,SA1_NULL,S1_RTORCHSHRT2,0,0,"lights/stickred.md2",0},	// S1_RTORCHSHRT
	{SPR1_SMRT,32769,4,SA1_NULL,S1_RTORCHSHRT3,0,0,"lights/stickred.md2",1},	// S1_RTORCHSHRT2
	{SPR1_SMRT,32770,4,SA1_NULL,S1_RTORCHSHRT4,0,0,"lights/stickred.md2",2},	// S1_RTORCHSHRT3
	{SPR1_SMRT,32771,4,SA1_NULL,S1_RTORCHSHRT,0,0,"lights/stickred.md2",3},	// S1_RTORCHSHRT4
	{SPR1_HDB1,0,-1,SA1_NULL,S1_NULL,0,0,"bodies/hnoguts.md2",0},	// S1_HANGNOGUTS
	{SPR1_HDB2,0,-1,SA1_NULL,S1_NULL,0,0,"bodies/hnobrain.md2",0},	// S1_HANGBNOBRAIN
	{SPR1_HDB3,0,-1,SA1_NULL,S1_NULL,0,0,"bodies/htlookdn.md2",0},	// S1_HANGTLOOKDN
	{SPR1_HDB4,0,-1,SA1_NULL,S1_NULL,0,0,"bodies/htskull.md2",0},	// S1_HANGTSKULL
	{SPR1_HDB5,0,-1,SA1_NULL,S1_NULL,0,0,"bodies/htlookup.md2",0},	// S1_HANGTLOOKUP
	{SPR1_HDB6,0,-1,SA1_NULL,S1_NULL,0,0,"bodies/htnobrn.md2",0},	// S1_HANGTNOBRAIN
	{SPR1_POB1,0,-1,SA1_NULL,S1_NULL,0,0,"gibs/blood.md2",0},	// S1_COLONGIBS
	{SPR1_POB2,0,-1,SA1_NULL,S1_NULL,0,0,"gibs/blood2.md2",0},	// S1_SMALLPOOL
	{SPR1_BRS1,0,-1,SA1_NULL,S1_NULL,0,0,"gibs/brain.md2",0},		// S1_BRAINSTEM
	{SPR1_TLMP,32768,4,SA1_NULL,S1_TECHLAMP2,0,0,"lights/techlamp.md2",0},	// S1_TECHLAMP
	{SPR1_TLMP,32769,4,SA1_NULL,S1_TECHLAMP3,0,0,"lights/techlamp.md2",1},	// S1_TECHLAMP2
	{SPR1_TLMP,32770,4,SA1_NULL,S1_TECHLAMP4,0,0,"lights/techlamp.md2",2},	// S1_TECHLAMP3
	{SPR1_TLMP,32771,4,SA1_NULL,S1_TECHLAMP,0,0,"lights/techlamp.md2",3},	// S1_TECHLAMP4
	{SPR1_TLP2,32768,4,SA1_NULL,S1_TECH2LAMP2,0,0,"lights/techlmp2.md2",0},	// S1_TECH2LAMP
	{SPR1_TLP2,32769,4,SA1_NULL,S1_TECH2LAMP3,0,0,"lights/techlmp2.md2",1},	// S1_TECH2LAMP2
	{SPR1_TLP2,32770,4,SA1_NULL,S1_TECH2LAMP4,0,0,"lights/techlmp2.md2",2},	// S1_TECH2LAMP3
	{SPR1_TLP2,32771,4,SA1_NULL,S1_TECH2LAMP,0,0,"lights/techlmp2.md2",3}	// S1_TECH2LAMP4
};

mobjinfo_t mobjinfo[NUMMOBJTYPES] = {

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

//
// PSPRITE ACTIONS for waepons.
// This struct controls the weapon animations.
//
// Each entry is:
//   ammo/amunition type
//  upstate
//  downstate
// readystate
// atkstate, i.e. attack/fire/hit frame
// flashstate, muzzle flash
//
weaponinfo_t weaponinfo[NUM_WEAPONS] =
{
	{
	// fist
	am_noammo,
	S1_PUNCHUP,
	S1_PUNCHDOWN,
	S1_PUNCH,
	S1_PUNCH1,
	S1_NULL,
	S1_NULL
	},	
	{
	// pistol
	am_clip,
	S1_PISTOLUP,
	S1_PISTOLDOWN,
	S1_PISTOL,
	S1_PISTOL1,
	S1_NULL,
	S1_PISTOLFLASH
	},	
	{
	// shotgun
	am_shell,
	S1_SGUNUP,
	S1_SGUNDOWN,
	S1_SGUN,
	S1_SGUN1,
	S1_NULL,
	S1_SGUNFLASH1
	},
	{
	// chaingun
	am_clip,
	S1_CHAINUP,
	S1_CHAINDOWN,
	S1_CHAIN,
	S1_CHAIN1,
	S1_NULL,
	S1_CHAINFLASH1
	},
	{
	// missile launcher
	am_misl,
	S1_MISSILEUP,
	S1_MISSILEDOWN,
	S1_MISSILE,
	S1_MISSILE1,
	S1_NULL,
	S1_MISSILEFLASH1
	},
	{
	// plasma rifle
	am_cell,
	S1_PLASMAUP,
	S1_PLASMADOWN,
	S1_PLASMA,
	S1_PLASMA1,
	S1_NULL,
	S1_PLASMAFLASH1
	},
	{
	// bfg 9000
	am_cell,
	S1_BFGUP,
	S1_BFGDOWN,
	S1_BFG,
	S1_BFG1,
	S1_NULL,
	S1_BFGFLASH1
	},
	{
	// chainsaw
	am_noammo,
	S1_SAWUP,
	S1_SAWDOWN,
	S1_SAW,
	S1_SAW1,
	S1_NULL,
	S1_NULL
	},
	{
	// super shotgun
	am_shell,
	S1_DSGUNUP,
	S1_DSGUNDOWN,
	S1_DSGUN,
	S1_DSGUN1,
	S1_NULL,
	S1_DSGUNFLASH1
	},	
};

sfxinfo_t sfx[] =
{
	// tagname, lumpname, priority, numchannels, pitchshift,
// S_sfx[0] needs to be a dummy for odd reasons.
{ "", "", 	0, 2, 1 },
{ "weapons/pistol", "dspistol",	64, 2, 1 },
{ "weapons/shotgf", "dsshotgn",	64, 2, 1 },
{ "weapons/shotgr", "dssgcock",	64, 2, 1 },
{ "weapons/sshotf", "dsdshtgn",	64, 2, 1 },
{ "weapons/sshoto", "dsdbopn", 	64, 2, 1 },
{ "weapons/sshotc", "dsdbcls", 	64, 2, 1 },
{ "weapons/sshotl", "dsdbload",	64, 2, 1 },
{ "weapons/plasmaf", "dsplasma",	64, 2, 1 },
{ "weapons/bfgf", "dsbfg", 	64, 2, 1 },
{ "weapons/sawup", "dssawup", 	64, 2, 1 },
{ "weapons/sawidle", "dssawidl",	118, 2, 1 },
{ "weapons/sawfull", "dssawful",	64, 2, 1 },
{ "weapons/sawhit", "dssawhit",	64, 2, 1 },
{ "weapons/rocklf", "dsrlaunc",	64, 2, 1 },
{ "weapons/bfgx", "dsrxplod",	70, 2, 1 },
{ "imp/attack", "dsfirsht",	70, 2, 1 },
{ "imp/shotx", "dsfirxpl",	70, 2, 1 },
{ "plats/pt1_strt", "dspstart",	100, 2, 1 },
{ "plats/pt1_stop", "dspstop", 	100, 2, 1 },
{ "doors/dr1_open", "dsdoropn",	100, 2, 1 },
{ "doors/dr1_clos", "dsdorcls",	100, 2, 1 },
{ "plats/pt1_mid", "dsstnmov",	119, 2, 1 },
{ "switches/normbutn", "dsswtchn",	78, 2, 1 },
{ "switches/exitbutn", "dsswtchx",	78, 2, 1 },

{ "*pain100", "dsplpain",	96, 2, 1 },
{ "demon/pain", "dsdmpain",	96, 2, 1 },
{ "grunt/pain", "dspopain",	96, 2, 1 },
{ "vile/pain", "dsvipain",	96, 2, 1 },
{ "fatso/pain", "dsmnpain",	96, 2, 1 },
{ "pain/pain", "dspepain",	96, 2, 1 },

{ "misc/gibbed", "dsslop", 	78, 2, 1 },
{ "misc/i_pkup", "dsitemup",	78, 2, 0 },
{ "misc/w_pkup", "dswpnup", 	78, 2, 1 },
{ "*grunt", "dsoof", 	96, 2, 1 },
{ "misc/teleport", "dstelept",	32, 2, 1 },

{ "grunt/sight1", "dsposit1",	98, 2, 1 },
{ "grunt/sight2", "dsposit2",	98, 2, 1 },
{ "grunt/sight3", "dsposit3",	98, 2, 1 },
{ "imp/sight1", "dsbgsit1",	98, 2, 1 },
{ "imp/sight2", "dsbgsit2",	98, 2, 1 },
{ "demon/sight", "dssgtsit",	98, 2, 1 },
{ "caco/sight", "dscacsit",	98, 2, 1 },
{ "baron/sight", "dsbrssit",	94, 2, 1 },
{ "cyber/sight", "dscybsit",	92, 2, 1 },
{ "spider/sight", "dsspisit",	90, 2, 1 },
{ "baby/sight", "dsbspsit",	90, 2, 1 },
{ "knight/sight", "dskntsit",	90, 2, 1 },
{ "vile/sight", "dsvilsit",	90, 2, 1 },
{ "fatso/sight", "dsmansit",	90, 2, 1 },
{ "pain/sight", "dspesit", 	90, 2, 1 },

{ "skull/melee", "dssklatk",	70, 2, 1 },
{ "demon/melee", "dssgtatk",	70, 2, 1 },
{ "skeleton/melee", "dsskepch",	70, 2, 1 },
{ "vile/start", "dsvilatk",	70, 2, 1 },
{ "imp/melee", "dsclaw", 	70, 2, 1 },
{ "skeleton/swing", "dsskeswg",	70, 2, 1 },

{ "*death", "dspldeth",	32, 2, 1 },
{ "*xdeath", "dspdiehi",	32, 2, 1 },
{ "grunt/death1", "dspodth1",	70, 2, 1 },
{ "grunt/death2", "dspodth2",	70, 2, 1 },
{ "grunt/death3", "dspodth3",	70, 2, 1 },
{ "imp/death1", "dsbgdth1",	70, 2, 1 },
{ "imp/death2", "dsbgdth2",	70, 2, 1 },
{ "demon/death", "dssgtdth",	70, 2, 1 },
{ "caco/death", "dscacdth",	70, 2, 1 },

{ "misc/unused", "dsskldth",	70, 2, 1 },

{ "baron/death", "dsbrsdth",	32, 2, 1 },
{ "cyber/death", "dscybdth",	32, 2, 1 },
{ "spider/death", "dsspidth",	32, 2, 1 },
{ "baby/death", "dsbspdth",	32, 2, 1 },
{ "vile/death", "dsvildth",	32, 2, 1 },
{ "knight/death", "dskntdth",	32, 2, 1 },
{ "pain/death", "dspedth", 	32, 2, 1 },
{ "skeleton/death", "dsskedth",	32, 2, 1 },

{ "grunt/active", "dsposact",	120, 2, 1 },
{ "imp/active", "dsbgact", 	120, 2, 1 },
{ "demon/active", "dsdmact", 	120, 2, 1 },
{ "baby/active", "dsbspact",	100, 2, 1 },
{ "baby/walk", "dsbspwlk",	100, 2, 1 },
{ "vile/active", "dsvilact",	100, 2, 1 },

{ "*usefail", "dsnoway", 	78, 2, 1 },
{ "world/barrelx", "dsbarexp",	60, 2, 1 },
{ "*fist", "dspunch", 	64, 2, 1 },
{ "cyber/hoof", "dshoof", 	70, 2, 1 },
{ "spider/walk", "dsmetal", 	70, 2, 1 },
{ "weapons/chngun", "dspistol", 	64, 2, 1 },
{ "misc/chat", "dstink", 	60, 2, 0 },
{ "doors/dr2_open", "dsbdopn", 	100, 2, 1 },
{ "doors/dr2_clos", "dsbdcls", 	100, 2, 1 },
{ "misc/spawn", "dsitmbk", 	100, 2, 1 },
{ "vile/firecrkl", "dsflame", 	32, 2, 1 },
{ "vile/firestrt", "dsflamst",	32, 2, 1 },
{ "misc/p_pkup", "dsgetpow",	60, 2, 1 },
{ "brain/spit", "dsbospit",	70, 2, 1 },
{ "brain/cube", "dsboscub",	70, 2, 1 },
{ "brain/sight", "dsbossit",	70, 2, 1 },
{ "brain/pain", "dsbospn", 	70, 2, 1 },
{ "brain/death", "dsbosdth",	70, 2, 1 },
{ "fatso/raiseguns", "dsmanatk",	70, 2, 1 },
{ "fatso/death", "dsmandth",	70, 2, 1 },
{ "wolfss/sight", "dssssit", 	70, 2, 1 },
{ "wolfss/death", "dsssdth", 	70, 2, 1 },
{ "keen/pain", "dskeenpn",	70, 2, 1 },
{ "keen/death", "dskeendt",	70, 2, 1 },
{ "skeleton/active", "dsskeact",	70, 2, 1 },
{ "skeleton/sight", "dsskesit",	70, 2, 1 },
{ "skeleton/attack", "dsskeatk",	70, 2, 1 },
{ "misc/chat2", "dsradio", 	60, 2, 1 },
{ "menu/activate", "dsswtchn", 32, 2, 1 },
{ "menu/cursor", "dspstop", 		32, 2, 1 },
{ "menu/choose", "dspistol", 	32, 2, 1 },
{ "menu/change", "dsstnmov", 	32, 2, 1 },
{ "menu/backup", "dsswtchn", 32, 2, 1 },
{ "menu/clear", "dsswtchx", 	32, 2, 1 },
};

int			num_sfx = sizeof(sfx) / sizeof(sfx[0]);

string_def_t	Strings[] =
{
	{ "pd_blueo", "You need a blue key to activate this object", NULL },
	{ "pd_redo", "You need a red key to activate this object", NULL },
	{ "pd_yellowo", "You need a yellow key to activate this object", NULL },
	{ "pd_bluek", "You need a blue key to open this door", NULL },
	{ "pd_redk", "You need a red key to open this door", NULL },
	{ "pd_yellowk", "You need a yellow key to open this door", NULL },

	{ "gotarmor", "Picked up the armor.", NULL },
	{ "gotmega", "Picked up the MegaArmor!", NULL },
	{ "goththbonus", "Picked up a health bonus.", NULL },
	{ "gotarmbonus", "Picked up an armor bonus.", NULL },
	{ "gotstim", "Picked up a stimpack.", NULL },
	{ "gotmedineed", "Picked up a medikit that you REALLY need!", NULL },
	{ "gotmedikit", "Picked up a medikit.", NULL },
	{ "gotsuper", "Supercharge!", NULL },

	{ "gotbluecard", "Picked up a blue keycard.", NULL },
	{ "gotyelwcard", "Picked up a yellow keycard.", NULL },
	{ "gotredcard", "Picked up a red keycard.", NULL },
	{ "gotblueskul", "Picked up a blue skull key.", NULL },
	{ "gotyelwskul", "Picked up a yellow skull key.", NULL },
	{ "gotredskull", "Picked up a red skull key.", NULL },

	{ "gotinvul", "Invulnerability!", NULL },
	{ "gotberserk", "Berserk!", NULL },
	{ "gotinvis", "Partial Invisibility", NULL },
	{ "gotsuit", "Radiation Shielding Suit", NULL },
	{ "gotmap", "Computer Area Map", NULL },
	{ "gotvisor", "Light Amplification Visor", NULL },
	{ "gotmsphere", "MegaSphere!", NULL },

	{ "gotclip", "Picked up a clip.", NULL },
	{ "gotclipbox", "Picked up a box of bullets.", NULL },
	{ "gotrocket", "Picked up a rocket.", NULL },
	{ "gotrockbox", "Picked up a box of rockets.", NULL },
	{ "gotcell", "Picked up an energy cell.", NULL },
	{ "gotcellbox", "Picked up an energy cell pack.", NULL },
	{ "gotshells", "Picked up 4 shotgun shells.", NULL },
	{ "gotshellbox", "Picked up a box of shotgun shells.", NULL },
	{ "gotbackpack", "Picked up a backpack full of ammo!", NULL },

	{ "gotbfg9000", "You got the BFG9000!  Oh, yes.", NULL },
	{ "gotchaingun", "You got the chaingun!", NULL },
	{ "gotchainsaw", "A chainsaw!  Find some meat!", NULL },
	{ "gotlauncher", "You got the rocket launcher!", NULL },
	{ "gotplasma", "You got the plasma gun!", NULL },
	{ "gotshotgun", "You got the shotgun!", NULL },
	{ "gotshotgun2", "You got the super shotgun!", NULL },

	{ "hustr_e1m1", "E1M1: Hangar", NULL },
	{ "hustr_e1m2", "E1M2: Nuclear Plant", NULL },
	{ "hustr_e1m3", "E1M3: Toxin Refinery", NULL },
	{ "hustr_e1m4", "E1M4: Command Control", NULL },
	{ "hustr_e1m5", "E1M5: Phobos Lab", NULL },
	{ "hustr_e1m6", "E1M6: Central Processing", NULL },
	{ "hustr_e1m7", "E1M7: Computer Station", NULL },
	{ "hustr_e1m8", "E1M8: Phobos Anomaly", NULL },
	{ "hustr_e1m9", "E1M9: Military Base", NULL },
	{ "hustr_e2m1", "E2M1: Deimos Anomaly", NULL },
	{ "hustr_e2m2", "E2M2: Containment Area", NULL },
	{ "hustr_e2m3", "E2M3: Refinery", NULL },
	{ "hustr_e2m4", "E2M4: Deimos Lab", NULL },
	{ "hustr_e2m5", "E2M5: Command Center", NULL },
	{ "hustr_e2m6", "E2M6: Halls of the Damned", NULL },
	{ "hustr_e2m7", "E2M7: Spawning Vats", NULL },
	{ "hustr_e2m8", "E2M8: Tower of Babel", NULL },
	{ "hustr_e2m9", "E2M9: Fortress of Mystery", NULL },
	{ "hustr_e3m1", "E3M1: Hell Keep", NULL },
	{ "hustr_e3m2", "E3M2: Slough of Despair", NULL },
	{ "hustr_e3m3", "E3M3: Pandemonium", NULL },
	{ "hustr_e3m4", "E3M4: House of Pain", NULL },
	{ "hustr_e3m5", "E3M5: Unholy Cathedral", NULL },
	{ "hustr_e3m6", "E3M6: Mt. Erebus", NULL },
	{ "hustr_e3m7", "E3M7: Limbo", NULL },
	{ "hustr_e3m8", "E3M8: Dis", NULL },
	{ "hustr_e3m9", "E3M9: Warrens", NULL },
	{ "hustr_e4m1", "E4M1: Hell Beneath", NULL },
	{ "hustr_e4m2", "E4M2: Perfect Hatred", NULL },
	{ "hustr_e4m3", "E4M3: Sever the Wicked", NULL },
	{ "hustr_e4m4", "E4M4: Unruly Evil", NULL },
	{ "hustr_e4m5", "E4M5: They Will Repent", NULL },
	{ "hustr_e4m6", "E4M6: Against thee Wickedly", NULL },
	{ "hustr_e4m7", "E4M7: And Hell Followed", NULL },
	{ "hustr_e4m8", "E4M8: Unto the Cruel", NULL },
	{ "hustr_e4m9", "E4M9: Fear", NULL },

	{ "hustr_1", "level 1: entryway", NULL },
	{ "hustr_2", "level 2: underhalls", NULL },
	{ "hustr_3", "level 3: the gantlet", NULL },
	{ "hustr_4", "level 4: the focus", NULL },
	{ "hustr_5", "level 5: the waste tunnels", NULL },
	{ "hustr_6", "level 6: the crusher", NULL },
	{ "hustr_7", "level 7: dead simple", NULL },
	{ "hustr_8", "level 8: tricks and traps", NULL },
	{ "hustr_9", "level 9: the pit", NULL },
	{ "hustr_10", "level 10: refueling base", NULL },
	{ "hustr_11", "level 11: 'o' of destruction!", NULL },
	{ "hustr_12", "level 12: the factory", NULL },
	{ "hustr_13", "level 13: downtown", NULL },
	{ "hustr_14", "level 14: the inmost dens", NULL },
	{ "hustr_15", "level 15: industrial zone", NULL },
	{ "hustr_16", "level 16: suburbs", NULL },
	{ "hustr_17", "level 17: tenements", NULL },
	{ "hustr_18", "level 18: the courtyard", NULL },
	{ "hustr_19", "level 19: the citadel", NULL },
	{ "hustr_20", "level 20: gotcha!", NULL },
	{ "hustr_21", "level 21: nirvana", NULL },
	{ "hustr_22", "level 22: the catacombs", NULL },
	{ "hustr_23", "level 23: barrels o' fun", NULL },
	{ "hustr_24", "level 24: the chasm", NULL },
	{ "hustr_25", "level 25: bloodfalls", NULL },
	{ "hustr_26", "level 26: the abandoned mines", NULL },
	{ "hustr_27", "level 27: monster condo", NULL },
	{ "hustr_28", "level 28: the spirit world", NULL },
	{ "hustr_29", "level 29: the living end", NULL },
	{ "hustr_30", "level 30: icon of sin", NULL },
	{ "hustr_31", "level 31: wolfenstein", NULL },
	{ "hustr_32", "level 32: grosse", NULL },

	{ "e1text",
		"Once you beat the big badasses and\n"
		"clean out the moon base you're supposed\n"
		"to win, aren't you? Aren't you? Where's\n"
		"your fat reward and ticket home? What\n"
		"the hell is this? It's not supposed to\n"
		"end this way!\n"
		"\n"
		"It stinks like rotten meat, but looks\n"
		"like the lost Deimos base.  Looks like\n"
		"you're stuck on The Shores of Hell.\n"
		"The only way out is through.\n"
		"\n"
		"To continue the DOOM experience, play\n"
		"The Shores of Hell and its amazing\n"
		"sequel, Inferno!\n", NULL },
	{ "e2text",
		"You've done it! The hideous cyber-\n"
		"demon lord that ruled the lost Deimos\n"
		"moon base has been slain and you\n"
		"are triumphant! But ... where are\n"
		"you? You clamber to the edge of the\n"
		"moon and look down to see the awful\n"
		"truth.\n"
		"\n"
		"Deimos floats above Hell itself!\n"
		"You've never heard of anyone escaping\n"
		"from Hell, but you'll make the bastards\n"
		"sorry they ever heard of you! Quickly,\n"
		"you rappel down to  the surface of\n"
		"Hell.\n"
		"\n"
		"Now, it's on to the final chapter of\n"
		"DOOM! -- Inferno.", NULL },
	{ "e3text",
		"The loathsome spiderdemon that\n"
		"masterminded the invasion of the moon\n"
		"bases and caused so much death has had\n"
		"its ass kicked for all time.\n"
		"\n"
		"A hidden doorway opens and you enter.\n"
		"You've proven too tough for Hell to\n"
		"contain, and now Hell at last plays\n"
		"fair -- for you emerge from the door\n"
		"to see the green fields of Earth!\n"
		"Home at last.\n"
		"\n"
		"You wonder what's been happening on\n"
		"Earth while you were battling evil\n"
		"unleashed. It's good that no Hell-\n"
		"spawn could have come through that\n"
		"door with you ...", NULL },
	{ "e4text",
		"the spider mastermind must have sent forth\n"
		"its legions of hellspawn before your\n"
		"final confrontation with that terrible\n"
		"east from hell.  but you stepped forward\n"
		"and brought forth eternal damnation and\n"
		"suffering upon the horde as a true hero\n"
		"would in the face of something so evil.\n"
		"\n"
		"besides, someone was gonna pay for what\n"
		"happened to daisy, your pet rabbit.\n"
		"\n"
		"but now, you see spread before you more\n"
		"potential pain and gibbitude as a nation\n"
		"of demons run amok among our cities.\n"
		"\n"
		"next stop, hell on earth!", NULL },

	{ "c1text",
		"YOU HAVE ENTERED DEEPLY INTO THE INFESTED\n"
		"STARPORT. BUT SOMETHING IS WRONG. THE\n"
		"MONSTERS HAVE BROUGHT THEIR OWN REALITY\n"
		"WITH THEM, AND THE STARPORT'S TECHNOLOGY\n"
		"IS BEING SUBVERTED BY THEIR PRESENCE.\n"
		"\n"
		"AHEAD, YOU SEE AN OUTPOST OF HELL, A\n"
		"FORTIFIED ZONE. IF YOU CAN GET PAST IT,\n"
		"YOU CAN PENETRATE INTO THE HAUNTED HEART\n"
		"OF THE STARBASE AND FIND THE CONTROLLING\n"
		"SWITCH WHICH HOLDS EARTH'S POPULATION\n"
		"HOSTAGE.", NULL },
	{ "c2text",
		"YOU HAVE WON! YOUR VICTORY HAS ENABLED\n"
		"HUMANKIND TO EVACUATE EARTH AND ESCAPE\n"
		"THE NIGHTMARE.  NOW YOU ARE THE ONLY\n"
		"HUMAN LEFT ON THE FACE OF THE PLANET.\n"
		"CANNIBAL MUTATIONS, CARNIVOROUS ALIENS,\n"
		"AND EVIL SPIRITS ARE YOUR ONLY NEIGHBORS.\n"
		"YOU SIT BACK AND WAIT FOR DEATH, CONTENT\n"
		"THAT YOU HAVE SAVED YOUR SPECIES.\n"
		"\n"
		"BUT THEN, EARTH CONTROL BEAMS DOWN A\n"
		"MESSAGE FROM SPACE: \"SENSORS HAVE LOCATED\n"
		"THE SOURCE OF THE ALIEN INVASION. IF YOU\n"
		"GO THERE, YOU MAY BE ABLE TO BLOCK THEIR\n"
		"ENTRY.  THE ALIEN BASE IS IN THE HEART OF\n"
		"YOUR OWN HOME CITY, NOT FAR FROM THE\n"
		"STARPORT.\" SLOWLY AND PAINFULLY YOU GET\n"
		"UP AND RETURN TO THE FRAY.", NULL },
	{ "c3text",
		"YOU ARE AT THE CORRUPT HEART OF THE CITY,\n"
		"SURROUNDED BY THE CORPSES OF YOUR ENEMIES.\n"
		"YOU SEE NO WAY TO DESTROY THE CREATURES'\n"
		"ENTRYWAY ON THIS SIDE, SO YOU CLENCH YOUR\n"
		"TEETH AND PLUNGE THROUGH IT.\n"
		"\n"
		"THERE MUST BE A WAY TO CLOSE IT ON THE\n"
		"OTHER SIDE. WHAT DO YOU CARE IF YOU'VE\n"
		"GOT TO GO THROUGH HELL TO GET TO IT?", NULL },
	{ "c4text",
		"THE HORRENDOUS VISAGE OF THE BIGGEST\n"
		"DEMON YOU'VE EVER SEEN CRUMBLES BEFORE\n"
		"YOU, AFTER YOU PUMP YOUR ROCKETS INTO\n"
		"HIS EXPOSED BRAIN. THE MONSTER SHRIVELS\n"
		"UP AND DIES, ITS THRASHING LIMBS\n"
		"DEVASTATING UNTOLD MILES OF HELL'S\n"
		"SURFACE.\n"
		"\n"
		"YOU'VE DONE IT. THE INVASION IS OVER.\n"
		"EARTH IS SAVED. HELL IS A WRECK. YOU\n"
		"WONDER WHERE BAD FOLKS WILL GO WHEN THEY\n"
		"DIE, NOW. WIPING THE SWEAT FROM YOUR\n"
		"FOREHEAD YOU BEGIN THE LONG TREK BACK\n"
		"HOME. REBUILDING EARTH OUGHT TO BE A\n"
		"LOT MORE FUN THAN RUINING IT WAS.\n", NULL },
	{ "c5text",
		"CONGRATULATIONS, YOU'VE FOUND THE SECRET\n"
		"LEVEL! LOOKS LIKE IT'S BEEN BUILT BY\n"
		"HUMANS, RATHER THAN DEMONS. YOU WONDER\n"
		"WHO THE INMATES OF THIS CORNER OF HELL\n"
		"WILL BE.", NULL },
	{ "c6text",
		"CONGRATULATIONS, YOU'VE FOUND THE\n"
		"SUPER SECRET LEVEL!  YOU'D BETTER\n"
		"BLAZE THROUGH THIS ONE!\n", NULL },

	{NULL, NULL, NULL}
};

map_info_t		map_info2[] =
{
	{ "lookup hustr_1", "d_runnin" },
	{ "lookup hustr_2", "d_stalks" },
	{ "lookup hustr_3", "d_countd" },
	{ "lookup hustr_4", "d_betwee" },
	{ "lookup hustr_5", "d_doom" },
	{ "lookup hustr_6", "d_the_da" },
	{ "lookup hustr_7", "d_shawn" },
	{ "lookup hustr_8", "d_ddtblu" },
	{ "lookup hustr_9", "d_in_cit" },
	{ "lookup hustr_10", "d_dead" },
	{ "lookup hustr_11", "d_stlks2" },
	{ "lookup hustr_12", "d_theda2" },
	{ "lookup hustr_13", "d_doom2" },
	{ "lookup hustr_14", "d_ddtbl2" },
	{ "lookup hustr_15", "d_runni2" },
	{ "lookup hustr_16", "d_dead2" },
	{ "lookup hustr_17", "d_stlks3" },
	{ "lookup hustr_18", "d_romero" },
	{ "lookup hustr_19", "d_shawn2" },
	{ "lookup hustr_20", "d_messag" },
	{ "lookup hustr_21", "d_count2" },
	{ "lookup hustr_22", "d_ddtbl3" },
	{ "lookup hustr_23", "d_ampie" },
	{ "lookup hustr_24", "d_theda3" },
	{ "lookup hustr_25", "d_adrian" },
	{ "lookup hustr_26", "d_messg2" },
	{ "lookup hustr_27", "d_romer2" },
	{ "lookup hustr_28", "d_tense" },
	{ "lookup hustr_29", "d_shawn3" },
	{ "lookup hustr_30", "d_openin" },
	{ "lookup hustr_31", "d_evil" },
	{ "lookup hustr_32", "d_ultima" },
};

map_info_t		map_info1[] =
{
	{ "lookup hustr_e1m1", "d_e1m1" },
	{ "lookup hustr_e1m2", "d_e1m2" },
	{ "lookup hustr_e1m3", "d_e1m3" },
	{ "lookup hustr_e1m4", "d_e1m4" },
	{ "lookup hustr_e1m5", "d_e1m5" },
	{ "lookup hustr_e1m6", "d_e1m6" },
	{ "lookup hustr_e1m7", "d_e1m7" },
	{ "lookup hustr_e1m8", "d_e1m8" },
	{ "lookup hustr_e1m9", "d_e1m9" },
	{ "lookup hustr_e2m1", "d_e2m1" },
	{ "lookup hustr_e2m2", "d_e2m2" },
	{ "lookup hustr_e2m3", "d_e2m3" },
	{ "lookup hustr_e2m4", "d_e2m4" },
	{ "lookup hustr_e2m5", "d_e2m5" },
	{ "lookup hustr_e2m6", "d_e2m6" },
	{ "lookup hustr_e2m7", "d_e2m7" },
	{ "lookup hustr_e2m8", "d_e2m8" },
	{ "lookup hustr_e2m9", "d_e2m9" },
	{ "lookup hustr_e3m1", "d_e3m1" },
	{ "lookup hustr_e3m2", "d_e3m2" },
	{ "lookup hustr_e3m3", "d_e3m3" },
	{ "lookup hustr_e3m4", "d_e3m4" },
	{ "lookup hustr_e3m5", "d_e3m5" },
	{ "lookup hustr_e3m6", "d_e3m6" },
	{ "lookup hustr_e3m7", "d_e3m7" },
	{ "lookup hustr_e3m8", "d_e3m8" },
	{ "lookup hustr_e3m9", "d_e3m9" },
	{ "lookup hustr_e4m1", "d_e3m4" },
	{ "lookup hustr_e4m2", "d_e3m2" },
	{ "lookup hustr_e4m3", "d_e3m3" },
	{ "lookup hustr_e4m4", "d_e1m5" },
	{ "lookup hustr_e4m5", "d_e2m7" },
	{ "lookup hustr_e4m6", "d_e2m4" },
	{ "lookup hustr_e4m7", "d_e2m6" },
	{ "lookup hustr_e4m8", "d_e2m5" },
	{ "lookup hustr_e4m9", "d_e1m9" },
};
