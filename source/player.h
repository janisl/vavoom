//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
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

#ifndef _PLAYER_H
#define _PLAYER_H

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

#define MAXNAME			32

#define IT_ALL_MAP		1

#define NUM_CSHIFTS		8

// TYPES -------------------------------------------------------------------

//
// Player states.
//
enum playerstate_t
{
    // Playing or camping.
    PST_LIVE,
    // Dead on the ground, view follows killer.
    PST_DEAD,
    // Ready to restart/respawn???
    PST_REBORN		
};

//
// Overlay psprites are scaled shapes
// drawn directly on the view screen,
// coordinates are given for a 320*200 view screen.
//
enum psprnum_t
{
    ps_weapon,
    ps_flash,	//	Only DOOM uses it
    NUMPSPRITES
};

struct pspdef_t
{
	int			sprite;	// a -1 sprite means not active
	int			frame;
	int			model_index;
	int			alias_frame;
	int			alias_skinnum;
	float		sx;
	float		sy;
	int			statenum;
	int			nextstate;
    float		time;
};

//
// Extended player object info: player_t
//
struct player_t
{
	boolean			active;
	boolean			spawned;
	boolean			is_bot;
	qsocket_t		*netcon;
	TMessage		message;
	byte			msgbuf[MAX_MSGLEN];

	char			userinfo[MAX_INFO_STRING];

	char			name[MAXNAME];
	int				baseclass;
	int				pclass;			// player class type
	int				color;

	// Copied from cmd, needed for PROGS, which supports only 4 byte ints
	float			forwardmove;	// *2048 for move
	float			sidemove;		// *2048 for move
	float			flymove;		// fly up/down/centering
	int				buttons;		// fire, use
	int				impulse;		// weapon changes, inventory, etc

    mobj_t*			mo;
    int				playerstate;

    // Determine POV,
    //  including viewpoint bobbing during movement.
    // Focal origin above r.z
	TVec			vieworg;

	boolean			fixangle;

    // This is only used between levels,
    // mo->health is used during levels.
    int				health;

	int				items;

    // Frags, kills of other players.
    int				frags[MAXPLAYERS];

    // True if button down last tic.
    int				attackdown;
    int				usedown;

     // For intermission stats.
    int				killcount;
    int				itemcount;
    int				secretcount;

    // So gun flashes light up areas.
    int				extralight;

	// For lite-amp and invulnarability powers
    int				fixedcolormap;

    // Current PLAYPAL index
    //  can be set to REDCOLORMAP for pain, etc.
    int				palette;

	// Color shifts for damage, powerups and content types
	dword			cshifts[NUM_CSHIFTS];

    // Overlay view sprites (gun, etc).
    pspdef_t		psprites[NUMPSPRITES];
	float			pspriteSY;

    // True if secret level has been done.
    boolean			didsecret;

	dword 			worldTimer;				// total time the player's been playing

	int				old_stats[96];

	//	128 integers for user defined fields in PROGS
	int				user_fields[128];
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif
