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

// TYPES -------------------------------------------------------------------

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
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

bool CL_Responder(event_t* ev);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern client_static_t	cls;
extern VBasePlayer*		cl;
