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

// TYPES -------------------------------------------------------------------

struct player_t;

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

class VViewEntity:public VObject
{
	DECLARE_CLASS(VViewEntity, VObject, 0)
	NO_DEFAULT_CONSTRUCTOR(VViewEntity)

	int			SpriteIndex;	// a -1 sprite means not active
	int			SpriteFrame;
	int			ModelIndex;
	int			ModelFrame;
	int			ModelSkinNum;
	float		SX;
	float		SY;
	int			StateNum;
	int			NextState;
    float		StateTime;
	player_t	*Player;
};

//
// Extended player object info: player_t
//
struct player_t
{
	dword			bActive:1;
	dword			bSpawned:1;
	dword			bIsBot:1;
	dword			bFixAngle:1;
	dword			bAttackDown:1;	// True if button down last tic.
	dword			bUseDown:1;
	dword			bDidSecret:1;	// True if secret level has been done.

	qsocket_t		*NetCon;
	TMessage		Message;
	byte			MsgBuf[MAX_MSGLEN];
	int				MobjUpdateStart;
	float			LastMessage;

	char			UserInfo[MAX_INFO_STRING];

	char			Name[MAXNAME];
	int				BaseClass;
	int				PClass;			// player class type
	int				Color;

	// Copied from cmd, needed for PROGS, which supports only 4 byte ints
	float			ForwardMove;	// *2048 for move
	float			SideMove;		// *2048 for move
	float			FlyMove;		// fly up/down/centering
	int				Buttons;		// fire, use
	int				Impulse;		// weapon changes, inventory, etc

    VMapObject*		MO;
    int				PlayerState;

	//	Model of current weapon
	int				WeaponModel;

    // Determine POV,
    //  including viewpoint bobbing during movement.
    // Focal origin above r.z
	TVec			ViewOrg;

	TAVec			ViewAngles;

    // This is only used between levels,
    // mo->health is used during levels.
    int				Health;

	int				Items;

    // Frags, kills of other players.
    int				Frags[MAXPLAYERS];

    // For intermission stats.
    int				KillCount;
    int				ItemCount;
    int				SecretCount;

    // So gun flashes light up areas.
    int				ExtraLight;

	// For lite-amp and invulnarability powers
    int				FixedColormap;

    // Current PLAYPAL index
    //  can be set to REDCOLORMAP for pain, etc.
    int				Palette;

	// Color shifts for damage, powerups and content types
	dword			CShifts[NUM_CSHIFTS];

    // Overlay view sprites (gun, etc).
    VViewEntity		*ViewEnts[NUMPSPRITES];
	float			PSpriteSY;

	dword 			WorldTimer;				// total time the player's been playing

	int				OldStats[96];

	//	256 integers for user defined fields in PROGS
	int				user_fields[256];
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//**************************************************************************
//
//	$Log$
//	Revision 1.12  2002/02/22 18:09:52  dj_jl
//	Some improvements, beautification.
//
//	Revision 1.11  2002/02/15 19:12:03  dj_jl
//	Property namig style change
//	
//	Revision 1.10  2002/01/07 12:16:43  dj_jl
//	Changed copyright year
//	
//	Revision 1.9  2001/12/18 19:03:16  dj_jl
//	A lots of work on VObject
//	
//	Revision 1.8  2001/12/12 19:28:49  dj_jl
//	Some little changes, beautification
//	
//	Revision 1.7  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.6  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.5  2001/08/30 17:46:52  dj_jl
//	Changes for better updates
//	
//	Revision 1.4  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.3  2001/07/31 17:16:31  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
