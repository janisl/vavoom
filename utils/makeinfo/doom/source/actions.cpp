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
#include "../../makeinfo.h"

state_action_info_t StateActionInfo[] =
{
	{NULL},
    {"A_Light0", 1},
    {"A_WeaponReady", 1},
    {"A_Lower", 1},
    {"A_Raise", 1},
    {"A_Punch", 1},
    {"A_ReFire", 1},
    {"A_FirePistol", 1},
	{"A_Light1", 1},
	{"A_FireShotgun", 1},
    {"A_Light2", 1},

    {"A_FireShotgun2", 1},
    {"A_CheckReload", 1},
    {"A_OpenShotgun2", 1},
    {"A_LoadShotgun2", 1},
    {"A_CloseShotgun2", 1},
    {"A_FireCGun", 1},
    {"A_GunFlash", 1},
    {"A_FireMissile", 1},
    {"A_Saw", 1},
    {"A_FirePlasma", 1},

    {"A_BFGsound", 1},
    {"A_FireBFG", 1},
    {"A_BFGSpray", 0},
    {"A_Explode", 0},
    {"A_Pain", 0},
    {"A_PlayerScream", 0},
    {"A_Fall", 0},
    {"A_XScream", 0},
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
