//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	Copyright (C) 1999-2000 JÆnis Legzdi·ý
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

char* ammo_names[] =
{
    "am_clip",	// Pistol / chaingun ammo.
    "am_shell",	// Shotgun / double barreled shotgun.
    "am_cell",	// Plasma rifle, BFG.
    "am_misl",	// Missile launcher.
    "NUMAMMO",
    "am_noammo"	// Unlimited for chainsaw / fist.
};

char* weapon_names[] =
{
    "set_weapon_fist",
    "set_weapon_pistol",
    "set_weapon_shotgun",
    "set_weapon_chaingun",
    "set_weapon_missile",
    "set_weapon_plasma",
    "set_weapon_bfg",
    "set_weapon_chainsaw",
    "set_weapon_supershotgun",
};

// Ammunition types defined.
enum
{
    am_clip,	// Pistol / chaingun ammo.
    am_shell,	// Shotgun / double barreled shotgun.
    am_cell,	// Plasma rifle, BFG.
    am_misl,	// Missile launcher.
    NUMAMMO,
    am_noammo	// Unlimited for chainsaw / fist.
};

int				numweapons = 9;

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
weaponinfo_t	weaponinfo[9] =
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
