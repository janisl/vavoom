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

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

// Player macros shared with client

#define MAXNAME			32

#define IT_ALL_MAP		1

#define NUM_CSHIFTS		8

#define SIGNONS		4

//
// Button/action code definitions.
//
#define	BT_ATTACK		1           // Press "Fire".
#define	BT_USE			2           // Use button, to open doors, activate switches.
#define BT_JUMP			4

// TYPES -------------------------------------------------------------------

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

// The data sampled per tick (single player)
// and transmitted to other peers (multiplayer).
// Mainly movements/button commands per game tick
struct ticcmd_t
{
	short		forwardmove;	// *2048 for move
	short		sidemove;		// *2048 for move
	short		flymove;		// fly up/down/centering
	byte		buttons;
	byte		impulse;
};

struct VModel;

struct cl_pspdef_t
{
	int			sprite;
	int			frame;
	VModel*		alias_model;
	int			alias_frame;
	int			alias_skinnum;
	float		sx;
	float		sy;
};

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
	char			userinfo[MAX_INFO_STRING];

	// demo recording info must be here, because record is started before
	// entering a map (and clearing client_state_t)
	boolean			demorecording;
	boolean			demoplayback;
	boolean			timedemo;
	VStream*		demofile;
	int				td_lastframe;	// to meter out one message a frame
	int				td_startframe;	// host_framecount at start
	double			td_starttime;	// realtime at second frame of timedemo

	// connection information
	int				signon;			// 0 to SIGNONS
	qsocket_t		*netcon;
	VMessage		message;		// writing buffer to send to server
};

class VClientState : public VObject
{
	DECLARE_CLASS(VClientState, VObject, 0)

	int				clientnum;		// cl_mobjs[cl.clientnum] = player

	int				pclass;			// player class type

	// Determine POV,
	//  including viewpoint bobbing during movement.
	// Focal origin above r.z
	TVec			vieworg;
	TAVec			viewangles;
	int				centering;

	// This is only used between levels,
	// mo->health is used during levels.
	int				health;

	int				items;

	// Frags, kills of other players.
	int				Frags;

	// So gun flashes light up areas.
	int				extralight;

	// For lite-amp and invulnarability powers
	int				fixedcolormap;

	// Current PLAYPAL index
	//  can be set to REDCOLORMAP for pain, etc.
	int				palette;
	int				prev_palette;

	dword			cshifts[NUM_CSHIFTS];		//	color shifts for damage,
	dword			prev_cshifts[NUM_CSHIFTS];	// powerups and content types

	// Overlay view sprites (gun, etc).
	cl_pspdef_t		psprites[NUMPSPRITES];
	float			pspriteSY;

	enum
	{
		// True if secret level has been done.
		CF_DidSecret	= 0x01,

		CF_Paused		= 0x02,
	};
	vuint32			ClientFlags;

	dword 			worldTimer;				// total time the player's been playing

	// Overlay view sprites (gun, etc).
	int				translucency;

	int				maxclients;
	int				deathmatch;

	float			mtime[2];
	float			time;
	float			oldtime;

	char			serverinfo[MAX_INFO_STRING];

	int				intermission;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

boolean CL_Responder(event_t* ev);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern client_static_t	cls;
extern VClientState*	cl;
extern TProgs			clpr;

//**************************************************************************
//
//	$Log$
//	Revision 1.16  2006/04/05 17:23:37  dj_jl
//	More dynamic string usage in console command class.
//	Added class for handling command line arguments.
//
//	Revision 1.15  2006/03/12 12:54:48  dj_jl
//	Removed use of bitfields for portability reasons.
//	
//	Revision 1.14  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//	
//	Revision 1.13  2006/02/20 22:52:56  dj_jl
//	Changed client state to a class.
//	
//	Revision 1.12  2006/02/05 14:11:00  dj_jl
//	Fixed conflict with Solaris.
//	
//	Revision 1.11  2005/04/04 07:45:45  dj_jl
//	Palette flash state fix.
//	
//	Revision 1.10  2002/09/07 16:31:51  dj_jl
//	Added Level class.
//	
//	Revision 1.9  2002/06/29 16:00:45  dj_jl
//	Added total frags count.
//	
//	Revision 1.8  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.7  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.6  2001/12/18 19:08:12  dj_jl
//	Beautification
//	
//	Revision 1.5  2001/08/15 17:08:59  dj_jl
//	Fixed finale
//	
//	Revision 1.4  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.3  2001/07/31 17:10:21  dj_jl
//	Localizing demo loop
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
