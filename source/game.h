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
//FIXME remove this

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

enum Game_t
{
 	Doom,
    Doom2,
    Heretic,
    Hexen,
	Strife	//	Just a test, not a true support
};

// Difficulty/skill settings/filters.
typedef enum
{
    sk_baby,
    sk_easy,
    sk_medium,
    sk_hard,
    sk_nightmare
} skill_t;

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void G_ExitLevel(void);
void G_SecretExitLevel(void);
void G_Completed(int map, int position);
void G_TeleportNewMap(int map, int position);
void G_WorldDone(void);
void G_PlayerReborn(int player);
void G_StartNewInit(void);

void G_PageDrawer(void);
void G_ForceTitle(void);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern player_t			players[MAXPLAYERS]; // Bookkeeping on players - state.

extern skill_t			gameskill;
 
extern boolean			netgame;                // only true if packets are broadcast
extern boolean			deathmatch;   			// only if started as net death

extern boolean			paused;

extern boolean			precache;        // if true, load all graphics at start

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
