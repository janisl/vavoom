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

class VViewEntity : public VObject
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
	VState*		State;
	VState*		NextState;
    float		StateTime;
	VBasePlayer	*Player;

	void SetState(VState* stnum);

	DECLARE_FUNCTION(SetState)
};

//
// Extended player object info: player_t
//
class VBasePlayer : public VObject
{
	DECLARE_CLASS(VBasePlayer, VObject, 0)
	NO_DEFAULT_CONSTRUCTOR(VBasePlayer)

	VLevelInfo*		Level;

	enum
	{
		PF_Active		= 0x0001,
		PF_Spawned		= 0x0002,
		PF_IsBot		= 0x0004,
		PF_FixAngle		= 0x0008,
		PF_AttackDown	= 0x0010,	// True if button down last tic.
		PF_UseDown		= 0x0020,
		PF_DidSecret	= 0x0040,	// True if secret level has been done.
		PF_NeedsUpdate	= 0x0080,
	};
	vuint32			PlayerFlags;

	qsocket_t		*NetCon;
	VMessage		Message;
	byte			MsgBuf[MAX_MSGLEN];
	int				MobjUpdateStart;
	float			LastMessage;

	char			UserInfo[MAX_INFO_STRING];

	char			PlayerName[MAXNAME];
	int				BaseClass;
	int				PClass;			// player class type
	int				Color;

	// Copied from cmd, needed for PROGS, which supports only 4 byte ints
	float			ForwardMove;	// *2048 for move
	float			SideMove;		// *2048 for move
	float			FlyMove;		// fly up/down/centering
	int				Buttons;		// fire, use
	int				Impulse;		// weapon changes, inventory, etc

    VEntity*		MO;
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
	int				Frags;
    int				FragsStats[MAXPLAYERS];

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

	int*			OldStats;

	void eventPutClientIntoServer()
	{
		svpr.Exec(GetVFunction("PutClientIntoServer"), (int)this);
	}

	void eventSpawnClient()
	{
		svpr.Exec(GetVFunction("SpawnClient"), (int)this);
	}

	void eventNetGameReborn()
	{
		svpr.Exec(GetVFunction("NetGameReborn"), (int)this);
	}

	void eventDisconnectClient()
	{
		svpr.Exec(GetVFunction("DisconnectClient"), (int)this);
	}

	void eventUserinfoChanged()
	{
		svpr.Exec(GetVFunction("UserinfoChanged"), (int)this);
	}

	void eventPlayerExitMap(bool clusterChange)
	{
		svpr.Exec(GetVFunction("PlayerExitMap"), (int)this, clusterChange);
	}

	void eventPlayerTick(float deltaTime)
	{
		svpr.Exec(GetVFunction("PlayerTick"), (int)this, PassFloat(deltaTime));
	}

	void eventSetViewPos()
	{
		svpr.Exec(GetVFunction("SetViewPos"), (int)this);
	}

	void eventCheat_God()
	{
		svpr.Exec(GetVFunction("Cheat_God"), (int)this);
	}

	void eventCheat_NoClip()
	{
		svpr.Exec(GetVFunction("Cheat_NoClip"), (int)this);
	}

	void eventCheat_Gimme()
	{
		svpr.Exec(GetVFunction("Cheat_Gimme"), (int)this);
	}

	void eventCheat_KillAll()
	{
		svpr.Exec(GetVFunction("Cheat_KillAll"), (int)this);
	}

	void eventCheat_Morph()
	{
		svpr.Exec(GetVFunction("Cheat_Morph"), (int)this);
	}

	void eventCheat_NoWeapons()
	{
		svpr.Exec(GetVFunction("Cheat_NoWeapons"), (int)this);
	}

	void eventCheat_Class()
	{
		svpr.Exec(GetVFunction("Cheat_Class"), (int)this);
	}
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

//**************************************************************************
//
//	$Log$
//	Revision 1.27  2006/04/05 17:21:00  dj_jl
//	Merged size buffer with message class.
//
//	Revision 1.26  2006/03/18 16:51:15  dj_jl
//	Renamed type class names, better code serialisation.
//	
//	Revision 1.25  2006/03/12 20:06:02  dj_jl
//	States as objects, added state variable type.
//	
//	Revision 1.24  2006/03/12 12:54:49  dj_jl
//	Removed use of bitfields for portability reasons.
//	
//	Revision 1.23  2006/03/06 13:05:50  dj_jl
//	Thunbker list in level, client now uses entity class.
//	
//	Revision 1.22  2006/02/26 20:52:48  dj_jl
//	Proper serialisation of level and players.
//	
//	Revision 1.21  2006/02/20 22:52:15  dj_jl
//	Removed player stats limit.
//	
//	Revision 1.20  2006/02/13 18:34:34  dj_jl
//	Moved all server progs global functions to classes.
//	
//	Revision 1.19  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.18  2003/11/12 16:47:40  dj_jl
//	Changed player structure into a class
//	
//	Revision 1.17  2003/03/08 16:02:52  dj_jl
//	A little multiplayer fix.
//	
//	Revision 1.16  2003/03/08 12:08:04  dj_jl
//	Beautification.
//	
//	Revision 1.15  2002/08/28 16:41:09  dj_jl
//	Merged VMapObject with VEntity, some natives.
//	
//	Revision 1.14  2002/06/29 16:00:45  dj_jl
//	Added total frags count.
//	
//	Revision 1.13  2002/03/09 18:05:34  dj_jl
//	Added support for defining native functions outside pr_cmds
//	
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
