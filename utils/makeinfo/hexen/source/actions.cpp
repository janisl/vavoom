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
	{"A_FreeTargMobj"},
    {"A_FlameCheck"},
    {"A_HideThing"},
    {"A_UnHideThing"},
    {"A_RestoreSpecialThing1"},
    {"A_RestoreSpecialThing2"},
    {"A_RestoreArtifact"},
    {"A_Summon"},
    {"A_ThrustInitUp"},
    {"A_ThrustInitDn"},

    {"A_ThrustRaise"},
    {"A_ThrustBlock"},
    {"A_ThrustImpale"},
    {"A_ThrustLower"},
    {"A_TeloSpawnC"},
    {"A_TeloSpawnB"},
    {"A_TeloSpawnA"},
    {"A_TeloSpawnD"},
    {"A_CheckTeleRing"},
    {"A_FogSpawn"},

    {"A_FogMove"},
    {"A_Quake"},
    {"A_ContMobjSound"},
    {"A_Scream"},
    {"A_Explode"},
    {"A_PoisonBagInit"},
    {"A_PoisonBagDamage"},
    {"A_PoisonBagCheck"},
    {"A_CheckThrowBomb"},
    {"A_NoGravity"},

    {"A_PotteryExplode"},
    {"A_PotteryChooseBit"},
    {"A_PotteryCheck"},
    {"A_CorpseBloodDrip"},
    {"A_CorpseExplode"},
    {"A_LeafSpawn"},
    {"A_LeafThrust"},
    {"A_LeafCheck"},
    {"A_BridgeInit"},
    {"A_BridgeOrbit"},

    {"A_TreeDeath"},
    {"A_PoisonShroom"},
    {"A_Pain"},
    {"A_SoAExplode"},
    {"A_BellReset1"},
    {"A_BellReset2"},
    {"A_NoBlocking"},
    {"A_Light0", 1},
    {"A_WeaponReady", 1},
    {"A_Lower", 1},

    {"A_Raise", 1},
    {"A_FPunchAttack", 1},
    {"A_ReFire", 1},
    {"A_FAxeAttack", 1},
    {"A_FHammerAttack", 1},
    {"A_FHammerThrow", 1},
    {"A_FSwordAttack", 1},
    {"A_FSwordFlames"},
    {"A_CMaceAttack", 1},
    {"A_CStaffInitBlink", 1},

    {"A_CStaffCheckBlink", 1},
    {"A_CStaffCheck", 1},
    {"A_CStaffAttack", 1},
    {"A_CStaffMissileSlither"},
    {"A_CFlameAttack", 1},
    {"A_CFlameRotate"},
    {"A_CFlamePuff"},
    {"A_CFlameMissile"},
    {"A_CHolyAttack", 1},
    {"A_CHolyPalette", 1},

    {"A_CHolySeek"},
    {"A_CHolyCheckScream"},
    {"A_CHolyTail"},
    {"A_CHolySpawnPuff"},
    {"A_CHolyAttack2"},
    {"A_MWandAttack", 1},
    {"A_LightningReady", 1},
    {"A_MLightningAttack", 1},
    {"A_LightningZap"},
    {"A_LightningClip"},

    {"A_LightningRemove"},
    {"A_LastZap"},
    {"A_ZapMimic"},
    {"A_MStaffAttack", 1},
    {"A_MStaffPalette", 1},
    {"A_MStaffWeave"},
    {"A_MStaffTrack"},
    {"A_SnoutAttack", 1},
    {"A_FireConePL1", 1},
    {"A_ShedShard"},

    {"A_AddPlayerCorpse"},
    {"A_SkullPop"},
    {"A_FreezeDeath"},
    {"A_FreezeDeathChunks"},
    {"A_CheckBurnGone"},
    {"A_CheckSkullFloor"},
    {"A_CheckSkullDone"},
    {"A_SpeedFade"},
    {"A_IceSetTics"},
	{"A_IceCheckHeadDone"},

    {"A_PigPain"},
    {"A_PigLook"},
    {"A_PigChase"},
    {"A_FaceTarget"},
    {"A_PigAttack"},
    {"A_QueueCorpse"},
    {"A_Look"},
    {"A_Chase"},
    {"A_CentaurAttack"},
    {"A_CentaurAttack2"},

    {"A_SetReflective"},
    {"A_CentaurDefend"},
    {"A_UnSetReflective"},
    {"A_CentaurDropStuff"},
    {"A_CheckFloor"},
    {"A_DemonAttack1"},
    {"A_DemonAttack2"},
    {"A_DemonDeath"},
    {"A_Demon2Death"},
    {"A_WraithRaiseInit"},

    {"A_WraithRaise"},
    {"A_WraithInit"},
    {"A_WraithLook"},
    {"A_WraithChase"},
    {"A_WraithFX3"},
    {"A_WraithMelee"},
    {"A_WraithMissile"},
    {"A_WraithFX2"},
    {"A_MinotaurFade1"},
    {"A_MinotaurFade2"},

    {"A_MinotaurLook"},
    {"A_MinotaurChase"},
    {"A_MinotaurRoam"},
    {"A_MinotaurAtk1"},
    {"A_MinotaurDecide"},
    {"A_MinotaurAtk2"},
    {"A_MinotaurAtk3"},
    {"A_MinotaurCharge"},
    {"A_SmokePuffExit"},
    {"A_MinotaurFade0"},

    {"A_MntrFloorFire"},
    {"A_SerpentChase"},
    {"A_SerpentHumpDecide"},
    {"A_SerpentUnHide"},
    {"A_SerpentRaiseHump"},
    {"A_SerpentLowerHump"},
    {"A_SerpentHide"},
    {"A_SerpentBirthScream"},
    {"A_SetShootable"},
    {"A_SerpentCheckForAttack"},

    {"A_UnSetShootable"},
    {"A_SerpentDiveSound"},
    {"A_SerpentWalk"},
    {"A_SerpentChooseAttack"},
    {"A_SerpentMeleeAttack"},
    {"A_SerpentMissileAttack"},
    {"A_SerpentHeadPop"},
    {"A_SerpentSpawnGibs"},
    {"A_SerpentHeadCheck"},
    {"A_FloatGib"},

    {"A_DelayGib"},
    {"A_SinkGib"},
    {"A_BishopDecide"},
    {"A_BishopDoBlur"},
    {"A_BishopSpawnBlur"},
    {"A_BishopChase"},
    {"A_BishopAttack"},
    {"A_BishopAttack2"},
    {"A_BishopPainBlur"},
    {"A_BishopPuff"},

    {"A_SetAltShadow"},
    {"A_BishopMissileWeave"},
    {"A_BishopMissileSeek"},
    {"A_DragonInitFlight"},
    {"A_DragonFlap"},
    {"A_DragonFlight"},
    {"A_DragonAttack"},
    {"A_DragonPain"},
    {"A_DragonCheckCrash"},
    {"A_DragonFX2"},

    {"A_ESound"},
    {"A_EttinAttack"},
    {"A_DropMace"},
    {"A_FiredRocks"},
    {"A_UnSetInvulnerable"},
    {"A_FiredChase"},
    {"A_FiredAttack"},
    {"A_FiredSplotch"},
    {"A_SmBounce"},
    {"A_IceGuyLook"},

    {"A_IceGuyChase"},
    {"A_IceGuyAttack"},
    {"A_IceGuyDie"},
    {"A_IceGuyMissilePuff"},
    {"A_IceGuyMissileExplode"},
    {"A_ClassBossHealth"},
    {"A_FastChase"},
    {"A_FighterAttack"},
    {"A_ClericAttack"},
    {"A_MageAttack"},

    {"A_SorcSpinBalls"},
    {"A_SpeedBalls"},
    {"A_SpawnFizzle"},
    {"A_SorcBossAttack"},
    {"A_SorcBallOrbit"},
    {"A_SorcBallPop"},
    {"A_BounceCheck"},
    {"A_SorcFX1Seek"},
    {"A_SorcFX2Split"},
    {"A_SorcFX2Orbit"},

    {"A_SorcererBishopEntry"},
    {"A_SpawnBishop"},
    {"A_SorcFX4Check"},
    {"A_KoraxStep2"},
    {"A_KoraxChase"},
    {"A_KoraxStep"},
    {"A_KoraxDecide"},
    {"A_KoraxMissile"},
    {"A_KoraxCommand"},
    {"A_KoraxBonePop"},

    {"A_KSpiritRoam"},
    {"A_KBoltRaise"},
    {"A_KBolt"},
    {"A_BatSpawnInit"},
    {"A_BatSpawn"},
    {"A_BatMove"},
	{NULL}
};

