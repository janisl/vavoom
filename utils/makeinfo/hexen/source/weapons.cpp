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
	"MANA_1",
	"MANA_2",
	"NUMMANA",
	"MANA_BOTH",
	"MANA_NONE"
};

char* weapon_names[] =
{
	"set_weapon_fighter_first",
	"set_weapon_cleric_first",
	"set_weapon_mage_first",
	"set_weapon_pig_first",
	"set_weapon_fighter_second",
	"set_weapon_cleric_second",
	"set_weapon_mage_second",
	"set_weapon_pig_second",
	"set_weapon_fighter_third",
	"set_weapon_cleric_third",
	"set_weapon_mage_third",
	"set_weapon_pig_third",
	"set_weapon_fighter_fourth",
	"set_weapon_cleric_fourth",
	"set_weapon_mage_fourth",
	"set_weapon_pig_fourth",
};

enum
{
	MANA_1,
	MANA_2,
	NUMMANA,
	MANA_BOTH,
	MANA_NONE
};

int 		numweapons = 16;

weaponinfo_t weaponinfo[16] =
{
	// First Weapons
		{ // Fighter First Weapon - Punch
			MANA_NONE,			// mana
			S3_PUNCHUP,			// upstate
			S3_PUNCHDOWN,		// downstate
			S3_PUNCHREADY,		// readystate
			S3_PUNCHATK1_1,		// atkstate
			S3_PUNCHATK1_1,		// holdatkstate
			S3_NULL				// flashstate
		},
		{ // Cleric First Weapon - Mace
			MANA_NONE,			// mana
			S3_CMACEUP,			// upstate
			S3_CMACEDOWN,		// downstate
			S3_CMACEREADY,		// readystate
			S3_CMACEATK_1,		// atkstate
			S3_CMACEATK_1,		// holdatkstate
			S3_NULL				// flashstate
		},
		{ // Mage First Weapon - Wand
			MANA_NONE,
			S3_MWANDUP,
			S3_MWANDDOWN,
			S3_MWANDREADY,
			S3_MWANDATK_1,
			S3_MWANDATK_1,
			S3_NULL
		},
		{ // Pig - Snout
			MANA_NONE,			// mana
			S3_SNOUTUP,			// upstate
			S3_SNOUTDOWN,		// downstate
			S3_SNOUTREADY,		// readystate
			S3_SNOUTATK1,		// atkstate
			S3_SNOUTATK1,		// holdatkstate
			S3_NULL				// flashstate
		},
	// Second Weapons
		{ // Fighter - Axe
			MANA_NONE,			// mana
			S3_FAXEUP,			// upstate
			S3_FAXEDOWN,			// downstate
			S3_FAXEREADY,		// readystate
			S3_FAXEATK_1,		// atkstate
			S3_FAXEATK_1,		// holdatkstate
			S3_NULL				// flashstate
		},
		{ // Cleric - Serpent Staff
			MANA_1,			// mana
			S3_CSTAFFUP,		// upstate
			S3_CSTAFFDOWN,	// downstate
			S3_CSTAFFREADY,	// readystate
			S3_CSTAFFATK_1,	// atkstate
			S3_CSTAFFATK_1,	// holdatkstate
			S3_NULL			// flashstate
		},
		{ // Mage - Cone of shards
			MANA_1,			// mana
			S3_CONEUP,		// upstate
			S3_CONEDOWN,		// downstate
			S3_CONEREADY,	// readystate
			S3_CONEATK1_1,	// atkstate
			S3_CONEATK1_3,	// holdatkstate
			S3_NULL			// flashstate
		},
		{ // Pig - Snout
			MANA_NONE,			// mana
			S3_SNOUTUP,			// upstate
			S3_SNOUTDOWN,		// downstate
			S3_SNOUTREADY,		// readystate
			S3_SNOUTATK1,		// atkstate
			S3_SNOUTATK1,		// holdatkstate
			S3_NULL				// flashstate
		},
	// Third Weapons
		{ // Fighter - Hammer
			MANA_NONE,			// mana
			S3_FHAMMERUP,		// upstate
			S3_FHAMMERDOWN,		// downstate
			S3_FHAMMERREADY,		// readystate
			S3_FHAMMERATK_1,		// atkstate
			S3_FHAMMERATK_1,		// holdatkstate
			S3_NULL				// flashstate
		},
		{ // Cleric - Flame Strike
			MANA_2,				// mana
			S3_CFLAMEUP,			// upstate
			S3_CFLAMEDOWN,		// downstate
			S3_CFLAMEREADY1,		// readystate
			S3_CFLAMEATK_1,		// atkstate
			S3_CFLAMEATK_1,		// holdatkstate
			S3_NULL				// flashstate
		},
		{ // Mage - Lightning
			MANA_2,		// mana
			S3_MLIGHTNINGUP,		// upstate
			S3_MLIGHTNINGDOWN,	// downstate
			S3_MLIGHTNINGREADY,	// readystate
			S3_MLIGHTNINGATK_1,	// atkstate
			S3_MLIGHTNINGATK_1,	// holdatkstate
			S3_NULL				// flashstate
		},
		{ // Pig - Snout
			MANA_NONE,			// mana
			S3_SNOUTUP,			// upstate
			S3_SNOUTDOWN,		// downstate
			S3_SNOUTREADY,		// readystate
			S3_SNOUTATK1,		// atkstate
			S3_SNOUTATK1,		// holdatkstate
			S3_NULL				// flashstate
		},
	// Fourth Weapons
		{ // Fighter - Rune Sword
			MANA_BOTH,			// mana
			S3_FSWORDUP,			// upstate
			S3_FSWORDDOWN,		// downstate
			S3_FSWORDREADY,		// readystate
			S3_FSWORDATK_1,		// atkstate
			S3_FSWORDATK_1,		// holdatkstate
			S3_NULL				// flashstate
		},
		{ // Cleric - Holy Symbol
			MANA_BOTH,			// mana
			S3_CHOLYUP,		// upstate
			S3_CHOLYDOWN,		// downstate
			S3_CHOLYREADY,		// readystate
			S3_CHOLYATK_1,		// atkstate
			S3_CHOLYATK_1,		// holdatkstate
			S3_NULL				// flashstate
		},
		{ // Mage - Staff
			MANA_BOTH,		// mana
			S3_MSTAFFUP,		// upstate
			S3_MSTAFFDOWN,		// downstate
			S3_MSTAFFREADY,		// readystate
			S3_MSTAFFATK_1,	// atkstate
			S3_MSTAFFATK_1,	// holdatkstate
			S3_NULL				// flashstate
		},
		{ // Pig - Snout
			MANA_NONE,			// mana
			S3_SNOUTUP,			// upstate
			S3_SNOUTDOWN,		// downstate
			S3_SNOUTREADY,		// readystate
			S3_SNOUTATK1,		// atkstate
			S3_SNOUTATK1,		// holdatkstate
			S3_NULL				// flashstate
		}
};

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/09/20 16:35:58  dj_jl
//	Beautification
//
//	Revision 1.2  2001/07/27 14:27:56  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
