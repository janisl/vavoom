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

char* ammo_names[] =
{
	"am_goldwand",
	"am_crossbow",
	"am_blaster",
	"am_skullrod",
	"am_phoenixrod",
	"am_mace",
	"NUMAMMO2",
	"am_noammo"
};

char* weapon_names[] =
{
	"set_weapon_staff",
	"set_weapon_goldwand",
	"set_weapon_crossbow",
	"set_weapon_blaster",
	"set_weapon_skullrod",
	"set_weapon_phoenixrod",
	"set_weapon_mace",
	"set_weapon_gauntlets",
	"set_weapon_beak",
	"set_weapon2_staff",
	"set_weapon2_goldwand",
	"set_weapon2_crossbow",
	"set_weapon2_blaster",
	"set_weapon2_skullrod",
	"set_weapon2_phoenixrod",
	"set_weapon2_mace",
	"set_weapon2_gauntlets",
	"set_weapon2_beak",
};

// Ammunition types defined.
enum
{
	am_goldwand,
	am_crossbow,
	am_blaster,
	am_skullrod,
	am_phoenixrod,
	am_mace,
	NUMAMMO2,
	am_noammo	// staff, gauntlets
};

int		numweapons = 18;

weaponinfo_t weaponinfo[18] =
{
	{ // Staff
		am_noammo,			// ammo
		S2_STAFFUP,			// upstate
		S2_STAFFDOWN,		// downstate
		S2_STAFFREADY,		// readystate
		S2_STAFFATK1_1,		// atkstate
		S2_STAFFATK1_1,		// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Gold wand
		am_goldwand,		// ammo
		S2_GOLDWANDUP,		// upstate
		S2_GOLDWANDDOWN,		// downstate
		S2_GOLDWANDREADY,	// readystate
		S2_GOLDWANDATK1_1,	// atkstate
		S2_GOLDWANDATK1_1,	// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Crossbow
		am_crossbow,		// ammo
		S2_CRBOWUP,			// upstate
		S2_CRBOWDOWN,		// downstate
		S2_CRBOW1,			// readystate
		S2_CRBOWATK1_1,		// atkstate
		S2_CRBOWATK1_1,		// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Blaster
		am_blaster,			// ammo
		S2_BLASTERUP,		// upstate
		S2_BLASTERDOWN,		// downstate
		S2_BLASTERREADY,		// readystate
		S2_BLASTERATK1_1,	// atkstate
		S2_BLASTERATK1_3,	// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Skull rod
		am_skullrod,		// ammo
		S2_HORNRODUP,		// upstate
		S2_HORNRODDOWN,		// downstate
		S2_HORNRODREADY,		// readystae
		S2_HORNRODATK1_1,	// atkstate
		S2_HORNRODATK1_1,	// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Phoenix rod
		am_phoenixrod,		// ammo
		S2_PHOENIXUP,		// upstate
		S2_PHOENIXDOWN,		// downstate
		S2_PHOENIXREADY,		// readystate
		S2_PHOENIXATK1_1,	// atkstate
		S2_PHOENIXATK1_1,	// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Mace
		am_mace,			// ammo
		S2_MACEUP,			// upstate
		S2_MACEDOWN,			// downstate
		S2_MACEREADY,		// readystate
		S2_MACEATK1_1,		// atkstate
		S2_MACEATK1_2,		// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Gauntlets
		am_noammo,			// ammo
		S2_GAUNTLETUP,		// upstate
		S2_GAUNTLETDOWN,		// downstate
		S2_GAUNTLETREADY,	// readystate
		S2_GAUNTLETATK1_1,	// atkstate
		S2_GAUNTLETATK1_3,	// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Beak
		am_noammo,			// ammo
		S2_BEAKUP,			// upstate
		S2_BEAKDOWN,			// downstate
		S2_BEAKREADY,		// readystate
		S2_BEAKATK1_1,		// atkstate
		S2_BEAKATK1_1,		// holdatkstate
		S2_NULL				// flashstate
	},
//	Level 2
	{ // Staff
		am_noammo,			// ammo
		S2_STAFFUP2,			// upstate
		S2_STAFFDOWN2,		// downstate
		S2_STAFFREADY2_1,	// readystate
		S2_STAFFATK2_1,		// atkstate
		S2_STAFFATK2_1,		// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Gold wand
		am_goldwand,		// ammo
		S2_GOLDWANDUP,		// upstate
		S2_GOLDWANDDOWN,		// downstate
		S2_GOLDWANDREADY,	// readystate
		S2_GOLDWANDATK2_1,	// atkstate
		S2_GOLDWANDATK2_1,	// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Crossbow
		am_crossbow,		// ammo
		S2_CRBOWUP,			// upstate
		S2_CRBOWDOWN,		// downstate
		S2_CRBOW1,			// readystate
		S2_CRBOWATK2_1,		// atkstate
		S2_CRBOWATK2_1,		// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Blaster
		am_blaster,			// ammo
		S2_BLASTERUP,		// upstate
		S2_BLASTERDOWN,		// downstate
		S2_BLASTERREADY,		// readystate
		S2_BLASTERATK2_1,	// atkstate
		S2_BLASTERATK2_3,	// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Skull rod
		am_skullrod,		// ammo
		S2_HORNRODUP,		// upstate
		S2_HORNRODDOWN,		// downstate
		S2_HORNRODREADY,		// readystae
		S2_HORNRODATK2_1,	// atkstate
		S2_HORNRODATK2_1,	// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Phoenix rod
		am_phoenixrod,		// ammo
		S2_PHOENIXUP,		// upstate
		S2_PHOENIXDOWN,		// downstate
		S2_PHOENIXREADY,		// readystate
		S2_PHOENIXATK2_1,	// atkstate
		S2_PHOENIXATK2_2,	// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Mace
		am_mace,			// ammo
		S2_MACEUP,			// upstate
		S2_MACEDOWN,			// downstate
		S2_MACEREADY,		// readystate
		S2_MACEATK2_1,		// atkstate
		S2_MACEATK2_1,		// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Gauntlets
		am_noammo,			// ammo
		S2_GAUNTLETUP2,		// upstate
		S2_GAUNTLETDOWN2,	// downstate
		S2_GAUNTLETREADY2_1,	// readystate
		S2_GAUNTLETATK2_1,	// atkstate
		S2_GAUNTLETATK2_3,	// holdatkstate
		S2_NULL				// flashstate
	},
	{ // Beak
		am_noammo,			// ammo
		S2_BEAKUP,			// upstate
		S2_BEAKDOWN,			// downstate
		S2_BEAKREADY,		// readystate
		S2_BEAKATK2_1,		// atkstate
		S2_BEAKATK2_1,		// holdatkstate
		S2_NULL				// flashstate
	}
};

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
