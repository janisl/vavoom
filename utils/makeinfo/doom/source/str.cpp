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

string_def_t	strings[] =
{
	{ "PD_BLUEO", "You need a blue key to activate this object", NULL },
	{ "PD_REDO", "You need a red key to activate this object", NULL },
	{ "PD_YELLOWO", "You need a yellow key to activate this object", NULL },
	{ "PD_BLUEK", "You need a blue key to open this door", NULL },
	{ "PD_REDK", "You need a red key to open this door", NULL },
	{ "PD_YELLOWK", "You need a yellow key to open this door", NULL },

	{ "GOTARMOR", "Picked up the armor.", NULL },
	{ "GOTMEGA", "Picked up the MegaArmor!", NULL },
	{ "GOTHTHBONUS", "Picked up a health bonus.", NULL },
	{ "GOTARMBONUS", "Picked up an armor bonus.", NULL },
	{ "GOTSTIM", "Picked up a stimpack.", NULL },
	{ "GOTMEDINEED", "Picked up a medikit that you REALLY need!", NULL },
	{ "GOTMEDIKIT", "Picked up a medikit.", NULL },
	{ "GOTSUPER", "Supercharge!", NULL },

	{ "GOTBLUECARD", "Picked up a blue keycard.", NULL },
	{ "GOTYELWCARD", "Picked up a yellow keycard.", NULL },
	{ "GOTREDCARD", "Picked up a red keycard.", NULL },
	{ "GOTBLUESKUL", "Picked up a blue skull key.", NULL },
	{ "GOTYELWSKUL", "Picked up a yellow skull key.", NULL },
	{ "GOTREDSKULL", "Picked up a red skull key.", NULL },

	{ "GOTINVUL", "Invulnerability!", NULL },
	{ "GOTBERSERK", "Berserk!", NULL },
	{ "GOTINVIS", "Partial Invisibility", NULL },
	{ "GOTSUIT", "Radiation Shielding Suit", NULL },
	{ "GOTMAP", "Computer Area Map", NULL },
	{ "GOTVISOR", "Light Amplification Visor", NULL },
	{ "GOTMSPHERE", "MegaSphere!", NULL },

	{ "GOTCLIP", "Picked up a clip.", NULL },
	{ "GOTCLIPBOX", "Picked up a box of bullets.", NULL },
	{ "GOTROCKET", "Picked up a rocket.", NULL },
	{ "GOTROCKBOX", "Picked up a box of rockets.", NULL },
	{ "GOTCELL", "Picked up an energy cell.", NULL },
	{ "GOTCELLBOX", "Picked up an energy cell pack.", NULL },
	{ "GOTSHELLS", "Picked up 4 shotgun shells.", NULL },
	{ "GOTSHELLBOX", "Picked up a box of shotgun shells.", NULL },
	{ "GOTBACKPACK", "Picked up a backpack full of ammo!", NULL },

	{ "GOTBFG9000", "You got the BFG9000!  Oh, yes.", NULL },
	{ "GOTCHAINGUN", "You got the chaingun!", NULL },
	{ "GOTCHAINSAW", "A chainsaw!  Find some meat!", NULL },
	{ "GOTLAUNCHER", "You got the rocket launcher!", NULL },
	{ "GOTPLASMA", "You got the plasma gun!", NULL },
	{ "GOTSHOTGUN", "You got the shotgun!", NULL },
	{ "GOTSHOTGUN2", "You got the super shotgun!", NULL },

	{NULL, NULL, NULL}
};

string_def_t	txtlumps[] =
{
#ifdef DOOM2
	{ "fin1text.txt",
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
	{ "fin2text.txt",
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
	{ "fin3text.txt",
		"YOU ARE AT THE CORRUPT HEART OF THE CITY,\n"
		"SURROUNDED BY THE CORPSES OF YOUR ENEMIES.\n"
		"YOU SEE NO WAY TO DESTROY THE CREATURES'\n"
		"ENTRYWAY ON THIS SIDE, SO YOU CLENCH YOUR\n"
		"TEETH AND PLUNGE THROUGH IT.\n"
		"\n"
		"THERE MUST BE A WAY TO CLOSE IT ON THE\n"
		"OTHER SIDE. WHAT DO YOU CARE IF YOU'VE\n"
		"GOT TO GO THROUGH HELL TO GET TO IT?", NULL },
	{ "fin4text.txt",
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
	{ "sec1text.txt",
		"CONGRATULATIONS, YOU'VE FOUND THE SECRET\n"
		"LEVEL! LOOKS LIKE IT'S BEEN BUILT BY\n"
		"HUMANS, RATHER THAN DEMONS. YOU WONDER\n"
		"WHO THE INMATES OF THIS CORNER OF HELL\n"
		"WILL BE.", NULL },
	{ "sec2text.txt",
		"CONGRATULATIONS, YOU'VE FOUND THE\n"
		"SUPER SECRET LEVEL!  YOU'D BETTER\n"
		"BLAZE THROUGH THIS ONE!\n", NULL },
#else
	{ "fin1text.txt",
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
"sequel, Inferno!", NULL },
	{ "fin2text.txt",
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
	{ "fin3text.txt",
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
	{ "fin4text.txt",
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
#endif
	{NULL, NULL, NULL}
};

