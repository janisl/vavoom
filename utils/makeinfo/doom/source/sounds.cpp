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

//==========================================================================
//
//	Doom sounds
//
//==========================================================================

sfxinfo_t sfx[] =
{
	// tagname, lumpname, priority, numchannels, pitchshift,
  // S_sfx[0] needs to be a dummy for odd reasons.
  { "", "", 	0, 2, 1 },
  { "PistolFire", "dspistol",	64, 2, 1 },
  { "ShotgunFire", "dsshotgn",	64, 2, 1 },
  { "ShotgunCocking", "dssgcock",	64, 2, 1 },
  { "SuperShotgunFire", "dsdshtgn",	64, 2, 1 },
  { "SuperShotgunOpen", "dsdbopn", 	64, 2, 1 },
  { "SuperShotgunClose", "dsdbcls", 	64, 2, 1 },
  { "SuperShotgunLoad", "dsdbload",	64, 2, 1 },
  { "PlasmaFire", "dsplasma",	64, 2, 1 },
  { "BFGFire", "dsbfg", 	64, 2, 1 },
  { "ChainsawStart", "dssawup", 	64, 2, 1 },
  { "ChainsawIdle", "dssawidl",	118, 2, 1 },
  { "ChainsawTriggered", "dssawful",	64, 2, 1 },
  { "ChainsawHit", "dssawhit",	64, 2, 1 },
  { "LauncherFire", "dsrlaunc",	64, 2, 1 },
  { "RocketExplode", "dsrxplod",	70, 2, 1 },
  { "FireballShoot", "dsfirsht",	70, 2, 1 },
  { "FireballExplode", "dsfirxpl",	70, 2, 1 },
  { "PlatformStart", "dspstart",	100, 2, 1 },
  { "PlatformStop", "dspstop", 	100, 2, 1 },
  { "DoorOpen", "dsdoropn",	100, 2, 1 },
  { "DoorClose", "dsdorcls",	100, 2, 1 },
  { "StoneMoving", "dsstnmov",	119, 2, 1 },
  { "Switch", "dsswtchn",	78, 2, 1 },
  { "SwitchExit", "dsswtchx",	78, 2, 1 },

  { "PlayerPain", "dsplpain",	96, 2, 1 },
  { "DemonPain", "dsdmpain",	96, 2, 1 },
  { "ZombiePain", "dspopain",	96, 2, 1 },
  { "ArchVilePain", "dsvipain",	96, 2, 1 },
  { "MancubusPain", "dsmnpain",	96, 2, 1 },
  { "PainElementalPain", "dspepain",	96, 2, 1 },

  { "Slop", "dsslop", 	78, 2, 1 },
  { "PickupItem", "dsitemup",	78, 2, 0 },
  { "PickupWeapon", "dswpnup", 	78, 2, 1 },
  { "PlayerLand", "dsoof", 	96, 2, 1 },
  { "Teleport", "dstelept",	32, 2, 1 },

  { "ZombieSight1", "dsposit1",	98, 2, 1 },
  { "ZombieSight2", "dsposit2",	98, 2, 1 },
  { "ZombieSight3", "dsposit3",	98, 2, 1 },
  { "ImpSight1", "dsbgsit1",	98, 2, 1 },
  { "ImpSight2", "dsbgsit2",	98, 2, 1 },
  { "DemonSight", "dssgtsit",	98, 2, 1 },
  { "CacoSight", "dscacsit",	98, 2, 1 },
  { "BaronSight", "dsbrssit",	94, 2, 1 },
  { "CyberSight", "dscybsit",	92, 2, 1 },
  { "SpiderSight", "dsspisit",	90, 2, 1 },
  { "BabySight", "dsbspsit",	90, 2, 1 },
  { "KnightSight", "dskntsit",	90, 2, 1 },
  { "ArchVileSight", "dsvilsit",	90, 2, 1 },
  { "MancubusSight", "dsmansit",	90, 2, 1 },
  { "PainElementalSight", "dspesit", 	90, 2, 1 },

  { "LostSoulAttack", "dssklatk",	70, 2, 1 },
  { "DemonAttack", "dssgtatk",	70, 2, 1 },
  { "RavenantPunch", "dsskepch",	70, 2, 1 },
  { "ArchVileAttack", "dsvilatk",	70, 2, 1 },
  { "ImpAttack", "dsclaw", 	70, 2, 1 },
  { "RavenantSwing", "dsskeswg",	70, 2, 1 },

  { "PlayerDeath", "dspldeth",	32, 2, 1 },
  { "PlayerExtremeDeath", "dspdiehi",	32, 2, 1 },
  { "ZombieDeath1", "dspodth1",	70, 2, 1 },
  { "ZombieDeath2", "dspodth2",	70, 2, 1 },
  { "ZombieDeath3", "dspodth3",	70, 2, 1 },
  { "ImpDeath1", "dsbgdth1",	70, 2, 1 },
  { "ImpDeath2", "dsbgdth2",	70, 2, 1 },
  { "DemonDeath", "dssgtdth",	70, 2, 1 },
  { "CacoDeath", "dscacdth",	70, 2, 1 },

  { "PushWall", "dsskldth",	70, 2, 1 },

  { "BaronDeath", "dsbrsdth",	32, 2, 1 },
  { "CyberDeath", "dscybdth",	32, 2, 1 },
  { "SpiderDeath", "dsspidth",	32, 2, 1 },
  { "BabyDeath", "dsbspdth",	32, 2, 1 },
  { "ArchVileDeath", "dsvildth",	32, 2, 1 },
  { "KnightDeath", "dskntdth",	32, 2, 1 },
  { "PainElementalDeath", "dspedth", 	32, 2, 1 },
  { "RavenantDeath", "dsskedth",	32, 2, 1 },

  { "ZombieActive", "dsposact",	120, 2, 1 },
  { "ImpActive", "dsbgact", 	120, 2, 1 },
  { "DemonActive", "dsdmact", 	120, 2, 1 },
  { "BabyActive", "dsbspact",	100, 2, 1 },
  { "BabyWalk", "dsbspwlk",	100, 2, 1 },
  { "ArchVileActive", "dsvilact",	100, 2, 1 },

  { "Blocked", "dsnoway", 	78, 2, 1 },
  { "BarrelExplode", "dsbarexp",	60, 2, 1 },
  { "Punch", "dspunch", 	64, 2, 1 },
  { "CyberWalk1", "dshoof", 	70, 2, 1 },
  { "CyberWalk2", "dsmetal", 	70, 2, 1 },
  { "ChaingunFire", "dschgun", 	64, 2, 1 },
#ifdef DOOM2
  { "Chat2", "dstink", 	60, 2, 0 },
#else
  { "Chat", "dstink", 	60, 2, 0 },
#endif
  { "BlazeDoorOpen", "dsbdopn", 	100, 2, 1 },
  { "BlazeDoorClose", "dsbdcls", 	100, 2, 1 },
  { "ItemRespawn", "dsitmbk", 	100, 2, 1 },
  { "Flame", "dsflame", 	32, 2, 1 },
  { "FlameStart", "dsflamst",	32, 2, 1 },
  { "PickupPower", "dsgetpow",	60, 2, 1 },
  { "BossFire", "dsbospit",	70, 2, 1 },
  { "CubeFly", "dsboscub",	70, 2, 1 },
  { "BossSight", "dsbossit",	70, 2, 1 },
  { "BossPain", "dsbospn", 	70, 2, 1 },
  { "BossDeath", "dsbosdth",	70, 2, 1 },
  { "MancubusAttack", "dsmanatk",	70, 2, 1 },
  { "MancubusDeath", "dsmandth",	70, 2, 1 },
  { "SSNaziSight", "dssssit", 	70, 2, 1 },
  { "SSNaziDeath", "dsssdth", 	70, 2, 1 },
  { "KeenPain", "dskeenpn",	70, 2, 1 },
  { "KeenDeath", "dskeendt",	70, 2, 1 },
  { "RavenantActive", "dsskeact",	70, 2, 1 },
  { "RavenantSight", "dsskesit",	70, 2, 1 },
  { "RavenantAttack", "dsskeatk",	70, 2, 1 },
#ifdef DOOM2
  { "Chat", "dsradio", 	60, 2, 1 },
#else
  { "Chat2", "dsradio", 	60, 2, 1 },
#endif
  { "MenuActivate", "dsswtchn", 32, 2, 1 },
  { "MenuMove", "dspstop", 		32, 2, 1 },
  { "MenuSelect", "dspistol", 	32, 2, 1 },
  { "MenuSwitch", "dsstnmov", 	32, 2, 1 },
  { "MenuPrevious", "dsswtchn", 32, 2, 1 },
  { "MenuClose", "dsswtchx", 	32, 2, 1 },
};

int			num_sfx = sizeof(sfx) / sizeof(sfx[0]);

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
