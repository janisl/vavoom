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

map_info_t		map_info[] =
{
#ifdef DOOM2
	{ "entryway", "D_runnin" },
	{ "underhalls", "D_stalks" },
	{ "the gantlet", "D_countd" },
	{ "the focus", "D_betwee" },
	{ "the waste tunnels", "D_doom" },
	{ "the crusher", "D_the_da" },
	{ "dead simple", "D_shawn" },
	{ "tricks and traps", "D_ddtblu" },
	{ "the pit", "D_in_cit" },
	{ "refueling base", "D_dead" },
	{ "'o' of destruction!", "D_stlks2" },
	{ "the factory", "D_theda2" },
	{ "downtown", "D_doom2" },
	{ "the inmost dens", "D_ddtbl2" },
	{ "industrial zone", "D_runni2" },
	{ "suburbs", "D_dead2" },
	{ "tenements", "D_stlks3" },
	{ "the courtyard", "D_romero" },
	{ "the citadel", "D_shawn2" },
	{ "gotcha!", "D_messag" },
	{ "nirvana", "D_count2" },
	{ "the catacombs", "D_ddtbl3" },
	{ "barrels o' fun", "D_ampie" },
	{ "the chasm", "D_theda3" },
	{ "bloodfalls", "D_adrian" },
	{ "the abandoned mines", "D_messg2" },
	{ "monster condo", "D_romer2" },
	{ "the spirit world", "D_tense" },
	{ "the living end", "D_shawn3" },
	{ "icon of sin", "D_openin" },
	{ "wolfenstein", "D_evil" },
	{ "grosse", "D_ultima" },
#else
	{ "Hangar", "D_e1m1" },
	{ "Nuclear Plant", "D_e1m2" },
	{ "Toxin Refinery", "D_e1m3" },
	{ "Command Control", "D_e1m4" },
	{ "Phobos Lab", "D_e1m5" },
	{ "Central Processing", "D_e1m6" },
	{ "Computer Station", "D_e1m7" },
	{ "Phobos Anomaly", "D_e1m8" },
	{ "Military Base", "D_e1m9" },
	{ "Deimos Anomaly", "D_e2m1" },
	{ "Containment Area", "D_e2m2" },
	{ "Refinery", "D_e2m3" },
	{ "Deimos Lab", "D_e2m4" },
	{ "Command Center", "D_e2m5" },
	{ "Halls of the Damned", "D_e2m6" },
	{ "Spawning Vats", "D_e2m7" },
	{ "Tower of Babel", "D_e2m8" },
	{ "Fortress of Mystery", "D_e2m9" },
	{ "Hell Keep", "D_e3m1" },
	{ "Slough of Despair", "D_e3m2" },
	{ "Pandemonium", "D_e3m3" },
	{ "House of Pain", "D_e3m4" },
	{ "Unholy Cathedral", "D_e3m5" },
	{ "Mt. Erebus", "D_e3m6" },
	{ "Limbo", "D_e3m7" },
	{ "Dis", "D_e3m8" },
	{ "Warrens", "D_e3m9" },
	{ "Hell Beneath", "D_e3m4" },
	{ "Perfect Hatred", "D_e3m2" },
	{ "Sever the Wicked", "D_e3m3" },
	{ "Unruly Evil", "D_e1m5" },
	{ "They Will Repent", "D_e2m7" },
	{ "Against thee Wickedly", "D_e2m4" },
	{ "And Hell Followed", "D_e2m6" },
	{ "Unto the Cruel", "D_e2m5" },
	{ "Fear", "D_e1m9" },
#endif
};

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
