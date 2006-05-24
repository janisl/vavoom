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

	DECLARE_FUNCTION(cprint)
	DECLARE_FUNCTION(centerprint)
	DECLARE_FUNCTION(GetPlayerNum)
	DECLARE_FUNCTION(ClearPlayer)
	DECLARE_FUNCTION(SelectClientMsg)

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
