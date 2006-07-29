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

	VSocketPublic*	NetCon;
	VMessage		Message;
	byte			MsgBuf[MAX_MSGLEN];
	int				MobjUpdateStart;
	float			LastMessage;

	VStr			UserInfo;

	VStr			PlayerName;
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
	vuint32			CShifts[NUM_CSHIFTS];

    // Overlay view sprites (gun, etc).
    VViewEntity		*ViewEnts[NUMPSPRITES];
	float			PSpriteSY;

	vuint32			WorldTimer;				// total time the player's been playing

	int*			OldStats;

	DECLARE_FUNCTION(cprint)
	DECLARE_FUNCTION(centerprint)
	DECLARE_FUNCTION(GetPlayerNum)
	DECLARE_FUNCTION(ClearPlayer)
	DECLARE_FUNCTION(SelectClientMsg)

	void eventPutClientIntoServer()
	{
		P_PASS_SELF;
		EV_RET_VOID("PutClientIntoServer");
	}

	void eventSpawnClient()
	{
		P_PASS_SELF;
		EV_RET_VOID("SpawnClient");
	}

	void eventNetGameReborn()
	{
		P_PASS_SELF;
		EV_RET_VOID("NetGameReborn");
	}

	void eventDisconnectClient()
	{
		P_PASS_SELF;
		EV_RET_VOID("DisconnectClient");
	}

	void eventUserinfoChanged()
	{
		P_PASS_SELF;
		EV_RET_VOID("UserinfoChanged");
	}

	void eventPlayerExitMap(bool clusterChange)
	{
		P_PASS_SELF;
		P_PASS_BOOL(clusterChange);
		EV_RET_VOID("PlayerExitMap");
	}

	void eventPlayerTick(float deltaTime)
	{
		P_PASS_SELF;
		P_PASS_FLOAT(deltaTime);
		EV_RET_VOID("PlayerTick");
	}

	void eventSetViewPos()
	{
		P_PASS_SELF;
		EV_RET_VOID("SetViewPos");
	}

	void eventCheat_God()
	{
		P_PASS_SELF;
		EV_RET_VOID("Cheat_God");
	}

	void eventCheat_NoClip()
	{
		P_PASS_SELF;
		EV_RET_VOID("Cheat_NoClip");
	}

	void eventCheat_Gimme()
	{
		P_PASS_SELF;
		EV_RET_VOID("Cheat_Gimme");
	}

	void eventCheat_KillAll()
	{
		P_PASS_SELF;
		EV_RET_VOID("Cheat_KillAll");
	}

	void eventCheat_Morph()
	{
		P_PASS_SELF;
		EV_RET_VOID("Cheat_Morph");
	}

	void eventCheat_NoWeapons()
	{
		P_PASS_SELF;
		EV_RET_VOID("Cheat_NoWeapons");
	}

	void eventCheat_Class()
	{
		P_PASS_SELF;
		EV_RET_VOID("Cheat_Class");
	}
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------
