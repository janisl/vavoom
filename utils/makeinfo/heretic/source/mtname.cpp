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
//**
//** Info.h
//**
//**************************************************************************

char* mt_names[] =
{
    "ItemHealthPotion",
    "ItemShield1",
    "ItemShield2",
    "ItemBagOfHolding",
    "ItemMap",
    "ArtifactInvisibility",
    "ArtifactHealthFlask",
    "ArtifactFly",
    "ArtifactInvulnerability",
    "ArtifactTomeOfPower",
    "ArtifactEgg",
    "EggMissile",
    "ArtifactSuperHeal",
    "ArtifactTorch",
    "ArtifactFireBomb",
    "FireBomb",
    "ArtifactTeleport",
    "Pod",
    "PodGoo",
    "PodGenerator",
    "Splash",
    "SplashBase",
    "LavaSplash",
    "LavaSmoke",
    "SludgeChunk",
    "SludgeSplash",
    "SkullHang70",
    "SkullHang60",
    "SkullHang45",
    "SkullHang35",
    "Chandelier",
    "SerpentTorch",
    "SmallPillar",
    "StalagmiteSmall",
    "StalagmiteLarge",
    "StalactiteSmall",
    "StalactiteLarge",
    "FireBrazier",
    "Barrel",
    "BrownPillar",
    "Moss1",
    "Moss2",
    "WallTorch",
    "HangingCorpse",
    "KeyGizmoBlue",
    "KeyGizmoGreen",
    "KeyGizmoYellow",
    "KeyGizmoFloat",
    "Volcano",
    "VolcanoBlast",
    "VolcanoTBlast",
    "TeleGlitGen",
    "TeleGlitGen2",
    "TeleGlitter",
    "TeleGlitter2",
    "TeleportFog",
    "TeleportSpot",
    "StaffPuff",
    "StaffPuff2",
    "BeakPuff",
    "ItemWeaponGauntlets",
    "GauntletPuff1",
    "GauntletPuff2",
    "ItemWeaponBlaster",
    "BlasterFX1",
    "BlasterSmoke",
    "Ripper",
    "BlasterPuff1",
    "BlasterPuff2",
    "ItemWeaponMace",
    "MaceFX1",
    "MaceFX2",
    "MaceFX3",
    "MaceFX4",
    "ItemWeaponSkullRod",
    "HornRodMissile1",
    "HornRodMissile2",
    "RainPlayer1",
    "RainPlayer2",
    "RainPlayer3",
    "RainPlayer4",
    "GoldWandMissile1",
    "GoldWandMissile2",
    "GoldWandPuff1",
    "GoldWandPuff2",
    "ItemWeaponPhoenixRod",
    "PhoenixFX1",
    "PhoenixPuff",
    "PhoenixFX2",
    "ItemWeaponCrossbow",
    "CrossbowFX1",
    "CrossbowFX2",
    "CrossbowFX3",
    "CrossbowFX4",
    "Blood",
    "BloodSplatter",
    "PlayerPawn",
    "BloodySkull",
    "ChickenPlayer",
    "Chicken",
    "Feather",
    "Mummy",
    "MummyLeader",
    "MummyGhost",
    "MummyLeaderGhost",
    "MummySoul",
    "MummyMissile",
    "Beast",
    "BeastBall",
    "BurnBall",
    "BurnBallFB",
    "Puffy",
    "Snake",
    "SnakeProjectileA",
    "SnakeProjectileB",
    "Head",
    "HeadMissile1",
    "HeadMissile2",
    "HeadMissile3",
    "WhirlWind",
    "Clink",
    "Wizard",
    "WizardMissile",
    "Imp",
    "ImpLeader",
    "ImpChunk1",
    "ImpChunk2",
    "ImpBall",
    "Knight",
    "KnightGhost",
    "KnightAxe",
    "KnightRedAxe",
    "Sorcerer1",
    "Sorcerer1Missile",
    "Sorcerer2",
    "Sorcerer2Missile1",
    "Sorcerer2MissileSpark",
    "Sorcerer2Missile2",
    "Sorcerer2TeleFade",
    "Minotaur",
    "MinotaurMissile",
    "MinotaurFloorFire",
    "MinotaurFloorFire2",
    "ItemKeyGreen",
    "ItemKeyBlue",
    "ItemKeyYellow",
    "ItemAmmoGoldWandWimpy",
    "ItemAmmoGoldWandHefty",
    "ItemAmmoMaceWimpy",
    "ItemAmmoMaceHefty",
    "ItemAmmoCrossbowWimpy",
    "ItemAmmoCrossbowHefty",
    "ItemAmmoSkullRodWimpy",
    "ItemAmmoSkullRodHefty",
    "ItemAmmoPhoenixWimpy",
    "ItemAmmoPhoenixHefty",
    "ItemAmmoBlasterWimpy",
    "ItemAmmoBlasterHefty",
    "SoundWind",
    "SoundWaterfall",
};

//**************************************************************************
//
//	$Log$
//	Revision 1.6  2002/02/22 18:11:01  dj_jl
//	Some renaming.
//
//	Revision 1.5  2002/01/17 18:18:13  dj_jl
//	Renamed all map object classes
//	
//	Revision 1.4  2002/01/07 12:30:05  dj_jl
//	Changed copyright year
//	
//	Revision 1.3  2001/09/20 16:34:58  dj_jl
//	Beautification
//	
//	Revision 1.2  2001/07/27 14:27:55  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
