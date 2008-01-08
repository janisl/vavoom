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

class VLevelInfo : public VThinker
{
	DECLARE_CLASS(VLevelInfo, VThinker, 0)

	VGameInfo*		Game;
	VWorldInfo*		World;

	VStr			LevelName;
	vint32			LevelNum;
	vuint8			Cluster;

	VName			NextMap;
	VName			SecretMap;

	vint32			ParTime;
	vint32			SuckTime;

	vint32			Sky1Texture;
	vint32			Sky2Texture;
	float			Sky1ScrollDelta;
	float			Sky2ScrollDelta;
	VName			SkyBox;

	VName			FadeTable;
	vuint32			Fade;
	vuint32			OutsideFog;

	VName			SongLump;
	vuint8			CDTrack;

	enum
	{
		LIF_DoubleSky					= 0x00000001,
		LIF_Lightning					= 0x00000002,
		LIF_Map07Special				= 0x00000004,
		LIF_BaronSpecial				= 0x00000008,
		LIF_CyberDemonSpecial			= 0x00000010,
		LIF_SpiderMastermindSpecial		= 0x00000020,
		LIF_MinotaurSpecial				= 0x00000040,
		LIF_DSparilSpecial				= 0x00000080,
		LIF_IronLichSpecial				= 0x00000100,
		LIF_SpecialActionOpenDoor		= 0x00000200,
		LIF_SpecialActionLowerFloor		= 0x00000400,
		LIF_SpecialActionKillMonsters	= 0x00000800,
		LIF_NoIntermission				= 0x00001000,
		LIF_AllowMonsterTelefrags		= 0x00002000,
		LIF_NoAllies					= 0x00004000,
		LIF_DeathSlideShow				= 0x00008000,
		LIF_ForceNoSkyStretch			= 0x00010000,
		LIF_LookupName					= 0x00020000,
		LIF_FallingDamage				= 0x00040000,
		LIF_OldFallingDamage			= 0x00080000,
		LIF_StrifeFallingDamage			= 0x00100000,
		LIF_MonsterFallingDamage		= 0x00200000,
		LIF_ClusterHub					= 0x00400000,
		LIF_BegunPlay					= 0x00800000,
	};
	vuint32			LevelInfoFlags;

	int				TotalKills;
	int				TotalItems;
	int				TotalSecret;    // for intermission
	int				CurrentKills;
	int				CurrentItems;
	int				CurrentSecret;

	float			CompletitionTime;	//	For intermission

	// Maintain single and multi player starting spots.
	mthing_t		DeathmatchStarts[MAXDEATHMATCHSTARTS];  // Player spawn spots for deathmatch.
	int				NumDeathmatchStarts;
	mthing_t		PlayerStarts[MAX_PLAYER_STARTS * MAXPLAYERS];// Player spawn spots.

	TArray<int>			TIDList;	// +1 for termination marker
	TArray<VEntity*>	TIDMobj;

	float			Gravity;								// Level Gravity
	float			AirControl;

	VLevelInfo();

	void SetMapInfo(const mapInfo_t&);

	void SectorStartSound(const sector_t*, int, int, float, float);
	void SectorStopSound(const sector_t*, int);
	void SectorStartSequence(const sector_t*, VName, int);
	void SectorStopSequence(const sector_t*);
	void PolyobjStartSequence(const polyobj_t*, VName, int);
	void PolyobjStopSequence(const polyobj_t*);

	void ExitLevel(int Position);
	void SecretExitLevel(int Position);
	void Completed(int Map, int Position, int SaveAngle);

	bool ChangeSwitchTexture(int, bool, VName, bool&);
	bool StartButton(int, vuint8, int, VName, bool);

	void ForceLightning();

	VStr GetLevelName() const
	{
		return LevelInfoFlags & LIF_LookupName ? GLanguage[*LevelName] : LevelName;
	}

	//	Static lights
	DECLARE_FUNCTION(AddStaticLight)
	DECLARE_FUNCTION(AddStaticLightRGB)

	//	Sound sequences
	DECLARE_FUNCTION(SectorStartSequence)
	DECLARE_FUNCTION(SectorStopSequence)
	DECLARE_FUNCTION(PolyobjStartSequence)
	DECLARE_FUNCTION(PolyobjStopSequence)

	//	Exiting the level
	DECLARE_FUNCTION(ExitLevel)
	DECLARE_FUNCTION(SecretExitLevel)
	DECLARE_FUNCTION(Completed)

	//	Special thinker utilites
	DECLARE_FUNCTION(ChangeSwitchTexture)
	DECLARE_FUNCTION(ForceLightning)

	void eventSpawnSpecials()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_SpawnSpecials);
	}
	void eventUpdateSpecials()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_UpdateSpecials);
	}
	void eventAfterUnarchiveThinkers()
	{
		P_PASS_SELF;
		EV_RET_VOID(NAME_AfterUnarchiveThinkers);
	}
	line_t* eventFindLine(int lineTag, int *searchPosition)
	{
		P_PASS_SELF;
		P_PASS_INT(lineTag);
		P_PASS_PTR(searchPosition);
		EV_RET_PTR(line_t, NAME_FindLine);
	}
	void eventPolyThrustMobj(VEntity* A, TVec thrustDir, polyobj_t* po)
	{
		P_PASS_SELF;
		P_PASS_REF(A);
		P_PASS_VEC(thrustDir);
		P_PASS_PTR(po);
		EV_RET_VOID(NAME_PolyThrustMobj);
	}
	bool eventTagBusy(int tag)
	{
		P_PASS_SELF;
		P_PASS_INT(tag);
		EV_RET_BOOL(NAME_TagBusy);
	}
	bool eventPolyBusy(int polyobj)
	{
		P_PASS_SELF;
		P_PASS_INT(polyobj);
		EV_RET_BOOL(NAME_PolyBusy);
	}
	int eventThingCount(int type, VName TypeName, int tid)
	{
		P_PASS_SELF;
		P_PASS_INT(type);
		P_PASS_NAME(TypeName);
		P_PASS_INT(tid);
		EV_RET_INT(NAME_ThingCount);
	}
	VEntity* eventFindMobjFromTID(int tid, int *searchPosition)
	{
		P_PASS_SELF;
		P_PASS_INT(tid);
		P_PASS_PTR(searchPosition);
		EV_RET_REF(VEntity, NAME_FindMobjFromTID);
	}
	int eventExecuteActionSpecial(int Special, int Arg1, int Arg2, int Arg3,
		int Arg4, int Arg5, line_t* Line, int Side, VEntity* A)
	{
		P_PASS_SELF;
		P_PASS_INT(Special);
		P_PASS_INT(Arg1);
		P_PASS_INT(Arg2);
		P_PASS_INT(Arg3);
		P_PASS_INT(Arg4);
		P_PASS_INT(Arg5);
		P_PASS_PTR(Line);
		P_PASS_INT(Side);
		P_PASS_REF(A);
		EV_RET_INT(NAME_ExecuteActionSpecial);
	}
	int eventEV_ThingProjectile(int tid, int type, int angle, int speed,
		int vspeed, int gravity, int newtid, VName TypeName, VEntity* Activator)
	{
		P_PASS_SELF;
		P_PASS_INT(tid);
		P_PASS_INT(type);
		P_PASS_INT(angle);
		P_PASS_INT(speed);
		P_PASS_INT(vspeed);
		P_PASS_INT(gravity);
		P_PASS_INT(newtid);
		P_PASS_NAME(TypeName);
		P_PASS_REF(Activator);
		EV_RET_INT(NAME_EV_ThingProjectile);
	}
	void eventStartPlaneWatcher(VEntity* it, line_t* line, int lineSide,
		bool ceiling, int tag, int height, int special, int arg1, int arg2,
		int arg3, int arg4, int arg5)
	{
		P_PASS_SELF;
		P_PASS_REF(it);
		P_PASS_PTR(line);
		P_PASS_INT(lineSide);
		P_PASS_BOOL(ceiling);
		P_PASS_INT(tag);
		P_PASS_INT(height);
		P_PASS_INT(special);
		P_PASS_INT(arg1);
		P_PASS_INT(arg2);
		P_PASS_INT(arg3);
		P_PASS_INT(arg4);
		P_PASS_INT(arg5);
		EV_RET_VOID(NAME_StartPlaneWatcher);
	}
	void eventSpawnMapThing(mthing_t* mthing)
	{
		P_PASS_SELF;
		P_PASS_PTR(mthing);
		EV_RET_VOID(NAME_SpawnMapThing);
	}
	void eventUpdateParticle(particle_t* p, float DeltaTime)
	{
		P_PASS_SELF;
		P_PASS_PTR(p);
		P_PASS_FLOAT(DeltaTime);
		EV_RET_VOID(NAME_UpdateParticle);
	}
	int eventAcsSpawnThing(VName Name, TVec Org, int Tid, float Angle)
	{
		P_PASS_SELF;
		P_PASS_NAME(Name);
		P_PASS_VEC(Org);
		P_PASS_INT(Tid);
		P_PASS_FLOAT(Angle);
		EV_RET_INT(NAME_AcsSpawnThing);
	}
	int eventAcsSpawnSpot(VName Name, int SpotTid, int Tid, float Angle)
	{
		P_PASS_SELF;
		P_PASS_NAME(Name);
		P_PASS_INT(SpotTid);
		P_PASS_INT(Tid);
		P_PASS_FLOAT(Angle);
		EV_RET_INT(NAME_AcsSpawnSpot);
	}
	int eventAcsSpawnSpotFacing(VName Name, int SpotTid, int Tid)
	{
		P_PASS_SELF;
		P_PASS_NAME(Name);
		P_PASS_INT(SpotTid);
		P_PASS_INT(Tid);
		EV_RET_INT(NAME_AcsSpawnSpotFacing);
	}
	void eventSectorDamage(int Tag, int Amount, VName DamageType,
		VName ProtectionType, int Flags)
	{
		P_PASS_SELF;
		P_PASS_INT(Tag);
		P_PASS_INT(Amount);
		P_PASS_NAME(DamageType);
		P_PASS_NAME(ProtectionType);
		P_PASS_INT(Flags);
		EV_RET_VOID(NAME_SectorDamage);
	}
	int eventDoThingDamage(int Tid, int Amount, VName DmgType, VEntity* Activator)
	{
		P_PASS_SELF;
		P_PASS_INT(Tid);
		P_PASS_INT(Amount);
		P_PASS_NAME(DmgType);
		P_PASS_REF(Activator);
		EV_RET_INT(NAME_DoThingDamage);
	}
	void eventSetMarineWeapon(int Tid, int Weapon, VEntity* Activator)
	{
		P_PASS_INT(Tid);
		P_PASS_INT(Weapon);
		P_PASS_REF(Activator);
		EV_RET_VOID(NAME_SetMarineWeapon);
	}
	void eventSetMarineSprite(int Tid, VName SrcClass, VEntity* Activator)
	{
		P_PASS_INT(Tid);
		P_PASS_NAME(SrcClass);
		P_PASS_REF(Activator);
		EV_RET_VOID(NAME_SetMarineSprite);
	}
};
