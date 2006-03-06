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
//
//								ENTITY DATA
//
// 	NOTES: VEntity
//
// 	mobj_ts are used to tell the refresh where to draw an image, tell the
// world simulation when objects are contacted, and tell the sound driver
// how to position a sound.
//
// 	The refresh uses the next and prev links to follow lists of things in
// sectors as they are being drawn. The sprite, frame, and angle elements
// determine which patch_t is used to draw the sprite if it is visible.
// The sprite and frame values are allmost allways set from state_t
// structures. The statescr.exe utility generates the states.h and states.c
// files that contain the sprite/frame numbers from the statescr.txt source
// file. The xyz origin point represents a point at the bottom middle of the
// sprite (between the feet of a biped). This is the default origin position
// for patch_ts grabbed with lumpy.exe. A walking creature will have its z
// equal to the floor it is standing on.
//
// 	The sound code uses the x,y, and subsector fields to do stereo
// positioning of any sound effited by the VEntity.
//
// 	The play simulation uses the blocklinks, x,y,z, radius, height to
// determine when mobj_ts are touching each other, touching lines in the map,
// or hit by trace lines (gunshots, lines of sight, etc). The VEntity->flags
// element has various bit flags used by the simulation.
//
// 	Every VEntity is linked into a single sector based on its origin
// coordinates. The subsector_t is found with R_PointInSubsector(x,y), and
// the sector_t can be found with subsector->sector. The sector links are
// only used by the rendering code, the play simulation does not care about
// them at all.
//
// 	Any VEntity that needs to be acted upon by something else in the play
// world (block movement, be shot, etc) will also need to be linked into the
// blockmap. If the thing has the MF_NOBLOCK flag set, it will not use the
// block links. It can still interact with other things, but only as the
// instigator (missiles will run into other things, but nothing can run into
// a missile). Each block in the grid is 128*128 units, and knows about every
// line_t that it contains a piece of, and every interactable VEntity that has
// its origin contained.
//
// 	A valid VEntity is a VEntity that has the proper subsector_t filled in for
// its xy coordinates and is linked into the sector from which the subsector
// was made, or has the MF_NOSECTOR flag set (the subsector_t needs to be
// valid even if MF_NOSECTOR is set), and is linked into a blockmap block or
// has the MF_NOBLOCKMAP flag set. Links should only be modified by the
// P_[Un]SetThingPosition() functions. Do not change the MF_NO? flags while
// a thing is valid.
//
// 	Any questions?
//
//==========================================================================

class VEntity : public VThinker
{
	DECLARE_CLASS(VEntity, VThinker, 0)
	NO_DEFAULT_CONSTRUCTOR(VEntity)

	// Info for drawing: position.
	TVec			Origin;

	// Momentums, used to update position.
	TVec			Velocity;

	TAVec			Angles;	// orientation

	//More drawing info: to determine current sprite.
	int				SpriteType;
	int				SpriteIndex;	// used to find patch_t and flip value
	int				SpriteFrame;	// might be ORed with FF_FULLBRIGHT

	int				ModelIndex;
	int				ModelFrame;
	int				ModelSkinIndex;
	int				ModelSkinNum;

	int				Translucency;
	int				Translation;

	float			FloorClip;		// value to use for floor clipping

	int				Effects;

	subsector_t*	SubSector;
	sector_t*		Sector;

	// Interaction info, by BLOCKMAP.
	// Links in blocks (if needed).
	VEntity*		BlockMapNext;
	VEntity*		BlockMapPrev;

	// The closest interval over all contacted Sectors.
	float			FloorZ;
	float			CeilingZ;

	//	Closest floor and ceiling, source of floorz and ceilingz
	sec_plane_t		*Floor;
	sec_plane_t		*Ceiling;

	// If == validcount, already checked.
	int				ValidCount;

	//	Flags
	dword			bSolid:1;		// Blocks.
	dword			bHidden:1;		// don't update to clients
									// (invisible but touchable)
	dword			bNoBlockmap:1;	// don't use the blocklinks
									// (inert but displayable)
	dword			bIsPlayer:1;	// Player or player-bot
	dword bFixedModel:1;

	dword bNoGravity:1;		// don't apply gravity every tic
	dword bNoPassMobj:1;	// Disable z block checking.  If on,
							// this flag will prevent the mobj
							// from passing over/under other mobjs.
	dword bColideWithThings:1;
	dword bColideWithWorld:1;
	dword bCheckLineBlocking:1;
	dword bCheckLineBlockMonsters:1;
	dword bDropOff:1;		// allow jumps from high places
	dword bFloat:1;			// allow moves to any height, no gravity
	dword bFly:1;			// fly mode is active
	dword bBlasted:1;		// missile will pass through ghosts
	dword bCantLeaveFloorpic:1;	// stay within a certain floor type
	dword bFloorClip:1;		// if feet are allowed to be clipped
	dword bIgnoreCeilingStep:1;	// continue walk without lowering itself

	int				Health;

	// For movement checking.
	float			Radius;
	float			Height;

	// Additional info record for player avatars only.
	// Only valid if type == MT_PLAYER
	VBasePlayer		*Player;

	int				TID;			// thing identifier
	int				Special;		// special
	int				Args[5];		// special arguments

	int				NetID;

	int				InUse;			//	Counter for client side

	VName SpriteName;
	float StateTime;	// state tic counter
	int StateNum;
	int NextState;

	//  Params
	float Mass;
	float MaxStepHeight;

	//  Water
	int WaterLevel;
	int WaterType;

	//   For player sounds.
	VName SoundClass;
	VName SoundGender;

	static int FIndex_Destroyed;
	static int FIndex_Touch;
	static int FIndex_BlockedByLine;
	static int FIndex_ApplyFriction;
	static int FIndex_PushLine;
	static int FIndex_HandleFloorclip;
	static int FIndex_CrossSpecialLine;
	static int FIndex_SectorChanged;
	static int FIndex_RoughCheckThing;
	static int FIndex_GiveInventory;
	static int FIndex_TakeInventory;
	static int FIndex_CheckInventory;
	static int FIndex_GetSigilPieces;

	static void InitFuncIndexes(void);

	//	VObject interface.
	void Destroy();
	void Serialise(VStream&);

	void eventDestroyed()
	{
		svpr.Exec(GetVFunction(FIndex_Destroyed), (int)this);
	}
	boolean eventTouch(VEntity *Other)
	{
		return svpr.Exec(GetVFunction(FIndex_Touch), (int)this, (int)Other);
	}
	void eventBlockedByLine(line_t * ld)
	{
		svpr.Exec(GetVFunction(FIndex_BlockedByLine), (int)this, (int)ld);
	}
	void eventApplyFriction(void)
	{
		svpr.Exec(GetVFunction(FIndex_ApplyFriction), (int)this);
	}
	void eventPushLine(void)
	{
		svpr.Exec(GetVFunction(FIndex_PushLine), (int)this);
	}
	void eventHandleFloorclip(void)
	{
		svpr.Exec(GetVFunction(FIndex_HandleFloorclip), (int)this);
	}
	void eventCrossSpecialLine(line_t *ld, int side)
	{
		svpr.Exec(GetVFunction(FIndex_CrossSpecialLine), (int)this, (int)ld, side);
	}
	bool eventSectorChanged(int CrushChange)
	{
		return !!svpr.Exec(GetVFunction(FIndex_SectorChanged), (int)this, CrushChange);
	}
	bool eventRoughCheckThing(VEntity *Other)
	{
		return !!svpr.Exec(GetVFunction(FIndex_RoughCheckThing), (int)this, (int)Other);
	}
	void eventGiveInventory(VName ItemName, int Amount)
	{
		svpr.Exec(GetVFunction(FIndex_GiveInventory), (int)this, ItemName.GetIndex(), Amount);
	}
	void eventTakeInventory(VName ItemName, int Amount)
	{
		svpr.Exec(GetVFunction(FIndex_TakeInventory), (int)this, ItemName.GetIndex(), Amount);
	}
	int eventCheckInventory(VName ItemName)
	{
		return svpr.Exec(GetVFunction(FIndex_CheckInventory), (int)this, ItemName.GetIndex());
	}
	int eventGetSigilPieces()
	{
		return svpr.Exec(GetVFunction(FIndex_GetSigilPieces), (int)this);
	}

	void Remove()
	{
		SetFlags(_OF_DelayedDestroy);
	}

	bool SetState(int state);
	void SetInitialState(int state);
	int FindState(VName StateName);

	boolean CheckWater(void);
	boolean CheckPosition(TVec Pos);
	boolean CheckRelPosition(TVec Pos);
	boolean TryMove(TVec newPos);
	void SlideMove(void);
	void BounceWall(float overbounce);
	void UpdateVelocity(void);
	void FakeZMovement(void);
	VEntity *CheckOnmobj(void);
	VEntity *RoughBlockCheck(int index);
	VEntity *RoughMonsterSearch(int distance);

	void LinkToWorld(void);
	void UnlinkFromWorld(void);
	bool CanSee(VEntity* Other);

	DECLARE_FUNCTION(Remove)
	DECLARE_FUNCTION(SetState)
	DECLARE_FUNCTION(SetInitialState)
	DECLARE_FUNCTION(FindState)
	DECLARE_FUNCTION(PlaySound)
	DECLARE_FUNCTION(PlayFullVolumeSound)
	DECLARE_FUNCTION(StopSound)
	DECLARE_FUNCTION(CheckWater)
	DECLARE_FUNCTION(CheckPosition)
	DECLARE_FUNCTION(CheckRelPosition)
	DECLARE_FUNCTION(TryMove)
	DECLARE_FUNCTION(SlideMove)
	DECLARE_FUNCTION(BounceWall)
	DECLARE_FUNCTION(UpdateVelocity)
	DECLARE_FUNCTION(CheckOnmobj)
	DECLARE_FUNCTION(LinkToWorld)
	DECLARE_FUNCTION(UnlinkFromWorld)
	DECLARE_FUNCTION(CanSee)
	DECLARE_FUNCTION(RoughMonsterSearch)
};

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2006/03/06 13:12:12  dj_jl
//	Client now uses entity class.
//
//**************************************************************************
