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
//**	Copyright (C) 1999-2006 Jānis Legzdiņš
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

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// Player macros shared with client

#define MAXNAME			32

#define IT_ALL_MAP		1

#define SIGNONS		4

//
// Button/action code definitions.
//
#define	BT_ATTACK		1           // Press "Fire".
#define	BT_USE			2           // Use button, to open doors, activate switches.
#define BT_JUMP			4

// TYPES -------------------------------------------------------------------

// The data sampled per tick (single player)
// and transmitted to other peers (multiplayer).
// Mainly movements/button commands per game tick
struct ticcmd_t
{
	short		forwardmove;	// *2048 for move
	short		sidemove;		// *2048 for move
	short		flymove;		// fly up/down/centreing
	byte		buttons;
	byte		impulse;
};

struct VModel;

enum cactive_t
{
	ca_dedicated, 		// a dedicated server with no ability to start a client
	ca_disconnected, 	// full screen console with no connection
	ca_connected		// valid netcon, talking to a server
};

struct client_static_t
{
	cactive_t		state;

	// personalization data sent to server
	VStr			userinfo;

	// demo recording info must be here, because record is started before
	// entering a map (and clearing client_state_t)
	bool			demorecording;
	bool			demoplayback;
	bool			timedemo;
	VStream*		demofile;
	int				td_lastframe;	// to meter out one message a frame
	int				td_startframe;	// host_framecount at start
	double			td_starttime;	// realtime at second frame of timedemo

	// connection information
	int				signon;			// 0 to SIGNONS
	VSocketPublic*	netcon;
	VMessage		message;		// writing buffer to send to server
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

bool CL_Responder(event_t* ev);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern client_static_t	cls;
extern VBasePlayer*		cl;
