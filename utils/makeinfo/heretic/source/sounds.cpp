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
//	Heretic sounds
//
//==========================================================================

sfxinfo_t sfx[] =
{
	// tagname, lumpname, priority, numchannels, pitchshift,
	{ "", "", 0, 0, 1 },
	{ "GoldWandFire", "gldhit", 32, 2, 1 },
	{ "GauntletFull", "gntful", 32, -1, 1 },
	{ "GauntletHit", "gnthit", 32, -1, 1 },
	{ "GauntletHit2", "gntpow", 32, -1, 1 },
	{ "GauntletActive", "gntact", 32, -1, 1 },
	{ "GauntletUse", "gntuse", 32, -1, 1 },
	{ "PhoenixFire", "phosht", 32, 2, 1 },
	{ "PhoenixHit", "phohit", 32, -1, 1 },
	{ "PhoenixHit2", "hedat1", 32, 1, 1 },//Replaces phopow
	{ "MaceFire", "lobsht", 20, 2, 1 },
	{ "MaceHit", "lobhit", 20, 2, 1 },
	{ "MaceHit2", "lobpow", 20, 2, 1 },
	{ "SkullRodFire", "hrnsht", 32, 2, 1 },
	{ "SkullRodHit", "hrnhit", 32, 2, 1 },
	{ "SkullRodFire2", "hrnpow", 32, 2, 1 },
	{ "SkullRodHit", "ramphit", 32, 2, 1 },
	{ "SkullRodRain", "ramrain", 10, 2, 1 },
	{ "BowFire", "bowsht", 32, 2, 1 },
	{ "StaffHit", "stfhit", 32, 2, 1 },
	{ "StaffHit2", "stfpow", 32, 2, 1 },
	{ "StaffActive", "stfcrk", 32, 2, 1 },
	{ "ImpSight", "impsit", 32, 2, 1 },
	{ "ImpAttack1", "impat1", 32, 2, 1 },
	{ "ImpAttack2", "impat2", 32, 2, 1 },
	{ "ImpDeath", "impdth", 80, 2, 1 },
	{ "ImpActive", "impsit", 20, 2, 1 },
	{ "ImpPain", "imppai", 32, 2, 1 },
	{ "MumySight", "mumsit", 32, 2, 1 },
	{ "MumyAttack1", "mumat1", 32, 2, 1 },
	{ "MumyAttack2", "mumat2", 32, 2, 1 },
	{ "MumyDeath", "mumdth", 80, 2, 1 },
	{ "MumyActive", "mumsit", 20, 2, 1 },
	{ "MumyPain", "mumpai", 32, 2, 1 },
	{ "MumyHeadExplode", "mumhed", 32, 2, 1 },
	{ "BeastSight", "bstsit", 32, 2, 1 },
	{ "BeastAttack", "bstatk", 32, 2, 1 },
	{ "BeastDeath", "bstdth", 80, 2, 1 },
	{ "BeastActive", "bstact", 20, 2, 1 },
	{ "BeastPain", "bstpai", 32, 2, 1 },
	{ "ClinkSight", "clksit", 32, 2, 1 },
	{ "ClinkAttack", "clkatk", 32, 2, 1 },
	{ "ClinkDeath", "clkdth", 80, 2, 1 },
	{ "ClinkActive", "clkact", 20, 2, 1 },
	{ "ClinkPain", "clkpai", 32, 2, 1 },
	{ "SnakeSight", "snksit", 32, 2, 1 },
	{ "SnakeAttack", "snkatk", 32, 2, 1 },
	{ "SnakeDeath", "snkdth", 80, 2, 1 },
	{ "SnakeActive", "snkact", 20, 2, 1 },
	{ "SnakePain", "snkpai", 32, 2, 1 },
	{ "KnightSight", "kgtsit", 32, 2, 1 },
	{ "KnightAttack1", "kgtatk", 32, 2, 1 },
	{ "KnightAttack2", "kgtat2", 32, 2, 1 },
	{ "KnightDeath", "kgtdth", 80, 2, 1 },
	{ "KnightActive", "kgtsit", 20, 2, 1 },
	{ "KnightPain", "kgtpai", 32, 2, 1 },
	{ "WizardSight", "wizsit", 32, 2, 1 },
	{ "WizardAttack", "wizatk", 32, 2, 1 },
	{ "WizardDeath", "wizdth", 80, 2, 1 },
	{ "WizardActive", "wizact", 20, 2, 1 },
	{ "WizardPain", "wizpai", 32, 2, 1 },
	{ "MinotaurSight", "minsit", 32, 2, 1 },
	{ "MinotaurAttack1", "minat1", 32, 2, 1 },
	{ "MinotaurAttack2", "minat2", 32, 2, 1 },
	{ "MinotaurAttack3", "minat3", 32, 2, 1 },
	{ "MinotaurDeath", "mindth", 80, 2, 1 },
	{ "MinotaurActive", "minact", 20, 2, 1 },
	{ "MinotaurPain", "minpai", 32, 2, 1 },
	{ "HeadSight", "hedsit", 32, 2, 1 },
	{ "HeadAttack1", "hedat1", 32, 2, 1 },
	{ "HeadAttack2", "hedat2", 32, 2, 1 },
	{ "HeadAttack3", "hedat3", 32, 2, 1 },
	{ "HeadDeath", "heddth", 80, 2, 1 },
	{ "HeadActive", "hedact", 20, 2, 1 },
	{ "HeadPain", "hedpai", 32, 2, 1 },
	{ "SorcererZap", "sorzap", 32, 2, 1 },
	{ "SorcererRise", "sorrise", 32, 2, 1 },
	{ "SorcererSight", "sorsit", 200, 2, 1 },
	{ "SorcererAttack", "soratk", 32, 2, 1 },
	{ "SorcererActive", "soract", 200, 2, 1 },
	{ "SorcererPain", "sorpai", 200, 2, 1 },
	{ "SorcererDeath", "sordsph", 200, 2, 1 },
	{ "SorcererExplodes", "sordexp", 200, 2, 1 },
	{ "SorcererBones", "sordbon", 200, 2, 1 },
	{ "SorcererDemonSight", "bstsit", 32, 2, 1 },
	{ "SorcererDemonAttack", "bstatk", 32, 2, 1 },
	{ "SorcererDemonDeath", "sbtdth", 80, 2, 1 },
	{ "SorcererDemonActive", "sbtact", 20, 2, 1 },
	{ "SorcererDemonPain", "sbtpai", 32, 2, 1 },
	{ "PlayerLand", "plroof", 32, 2, 1 },
	{ "PlayerPain", "plrpai", 32, 2, 1 },
 	{ "PlayerDeath", "plrdth", 80, 2, 1 },
	{ "PlayerExtremeDeath", "gibdth", 100, 2, 1 },
	{ "PlayerWimpyDeath", "plrwdth", 80, 2, 1 },
	{ "PlayerCrazyDeath", "plrcdth", 100, 2, 1 },
	{ "PickupItem", "itemup", 32, 2, 1 },
	{ "PickupWeapon", "wpnup", 32, 2, 1 },
	{ "Teleport", "telept", 50, 2, 1 },
	{ "DoorOpen", "doropn", 40, 2, 1 },
	{ "DoorClose", "dorcls", 40, 2, 1 },
	{ "DoorMove", "dormov", 40, 2, 1 },
	{ "PickupArtifact", "artiup", 32, 2, 1 },
	{ "Switch", "switch", 40, 2, 1 },
	{ "PlatformStart", "pstart", 40, 2, 1 },
	{ "PlatformStop", "pstop", 40, 2, 1 },
	{ "PlatformMove", "stnmov", 40, 2, 1 },
	{ "ChickenPain", "chicpai", 32, 2, 1 },
	{ "ChickenAttack", "chicatk", 32, 2, 1 },
	{ "ChickenDeath", "chicdth", 40, 2, 1 },
	{ "ChickenActive", "chicact", 32, 2, 1 },
	{ "ChickenPeak1", "chicpk1", 32, 2, 1 },
	{ "ChickenPeak2", "chicpk2", 32, 2, 1 },
	{ "ChickenPeak3", "chicpk3", 32, 2, 1 },
	{ "PickupKey", "keyup", 50, 2, 1 },
	{ "RipSlop", "ripslop", 16, 2, 1 },
	{ "PodNew", "newpod", 16, -1, 1 },
	{ "PodExplode", "podexp", 40, -1, 1 },
	{ "Bounce", "bounce", 16, 2, 1 },
	{ "VolcanoBlast", "bstatk", 16, 2, 1 },
	{ "VolcanoHit", "lobhit", 16, 2, 1 },
	{ "Burn", "burn", 10, 2, 1 },
	{ "Splash", "splash", 10, 1, 1 },//Unused
	{ "WaterSplash", "gloop", 10, 2, 1 },
	{ "Respawn", "respawn", 10, 1, 1 },
	{ "BlasterFire", "blssht", 32, 2, 1 },
	{ "BlasterHit", "blshit", 32, 2, 1 },
	{ "Chat", "chat", 100, 1, 1 },
	{ "UseArtifact", "artiuse", 32, 1, 1 },
	{ "Frag", "gfrag", 100, 1, 1 },
	{ "Waterfall", "waterfl", 16, 2, 1 },

	// Monophonic sounds

	{ "Wind", "wind", 16, 1, 1 },
	{ "Ambient1", "amb1", 1, 1, 1 },
	{ "Ambient2", "amb2", 1, 1, 1 },
	{ "Ambient3", "amb3", 1, 1, 1 },
	{ "Ambient4", "amb4", 1, 1, 1 },
	{ "Ambient5", "amb5", 1, 1, 1 },
	{ "Ambient6", "amb6", 1, 1, 1 },
	{ "Ambient7", "amb7", 1, 1, 1 },
	{ "Ambient8", "amb8", 1, 1, 1 },
	{ "Ambient9", "amb9", 1, 1, 1 },
	{ "Ambient10", "amb10", 1, 1, 1 },
	{ "Ambient11", "amb11", 1, 0, 1 },
	{ "MenuActivate", "dorcls", 32, 2, 1 },
	{ "MenuMove", "switch", 32, 2, 1 },
	{ "MenuSelect", "dorcls", 32, 2, 1 },
	{ "MenuSwitch", "keyup", 32, 2, 1 },
	{ "MenuPrevious", "switch", 32, 2, 1 },
	{ "MenuClose", "dorcls", 32, 2, 1 },
};

int			num_sfx = sizeof(sfx) / sizeof(sfx[0]);

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
