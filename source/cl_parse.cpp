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

#include "gamedefs.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void CL_ClearInput(void);
void CL_PO_SpawnPolyobj(float x, float y, int tag);
void CL_PO_TranslateToStartSpot(float originX, float originY, int tag);
void CL_PO_Update(int i, float x, float y, float angle);
void CL_SignonReply(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

clmobj_t		*cl_mobjs;
clmobjbase_t	*cl_mo_base;
clmobj_t		cl_weapon_mobjs[MAXPLAYERS];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static model_t			*model_precache[1024];
static model_t			*weapon_model_precache[1024];
static char				skin_list[256][MAX_VPATH];

static FFunction *pf_ParseServerCommand;

// CODE --------------------------------------------------------------------

void CL_Clear(void)
{
	guard(CL_Clear);
	memset(&cl, 0, sizeof(cl));
	memset(&cl_level, 0, sizeof(cl_level));
	memset(cl_mobjs, 0, sizeof(clmobj_t) * GMaxEntities);
	memset(cl_mo_base, 0, sizeof(clmobjbase_t) * GMaxEntities);
	memset(cl_weapon_mobjs, 0, sizeof(cl_weapon_mobjs));
	memset(cl_dlights, 0, sizeof(cl_dlights));
	memset(scores, 0, sizeof(scores));
	CL_ClearInput();
#ifdef SERVER
	if (!sv.active)
#endif
	{
	    // Make sure all sounds are stopped before Z_FreeTags.
	    S_StopAllSound();
		Z_FreeTag(PU_LEVEL);
	}
	cls.signon = 0;

	pf_ParseServerCommand = clpr.FuncForName("ParseServerCommand");
	unguard;
}

static void CL_ReadMobjBase(clmobjbase_t &mobj)
{
	mobj.origin.x = net_msg.ReadShort();
	mobj.origin.y = net_msg.ReadShort();
	mobj.origin.z = net_msg.ReadShort();
	mobj.angles.yaw = ByteToAngle(net_msg.ReadByte());
	mobj.angles.pitch = ByteToAngle(net_msg.ReadByte());
	mobj.angles.roll = ByteToAngle(net_msg.ReadByte());
	mobj.sprite = (word)net_msg.ReadShort();
	mobj.frame = (word)net_msg.ReadShort();
	mobj.translucency = net_msg.ReadByte();
	mobj.translation = net_msg.ReadByte();
	mobj.effects = net_msg.ReadByte();
	mobj.model_index = net_msg.ReadShort();
	mobj.alias_frame = net_msg.ReadByte();
	mobj.spritetype = mobj.sprite >> 10;
	mobj.sprite &= 0x3ff;
}

static void CL_ParseBaseline(void)
{
	int		i;

	i = net_msg.ReadShort();

	CL_ReadMobjBase(cl_mo_base[i]);
}

static void CL_ReadMobj(int bits, clmobj_t &mobj, const clmobjbase_t &base)
{
	if (bits & MOB_X)
		mobj.origin.x = net_msg.ReadShort();
	else
		mobj.origin.x = base.origin.x;
	if (bits & MOB_Y)
		mobj.origin.y = net_msg.ReadShort();
	else
		mobj.origin.y = base.origin.y;
	if (bits & MOB_Z)
		mobj.origin.z = net_msg.ReadShort();
	else
		mobj.origin.z = base.origin.z;
	if (bits & MOB_ANGLE)
		mobj.angles.yaw = ByteToAngle(net_msg.ReadByte());
	else
		mobj.angles.yaw = base.angles.yaw;
	if (bits & MOB_ANGLEP)
		mobj.angles.pitch = ByteToAngle(net_msg.ReadByte());
	else
		mobj.angles.pitch = base.angles.pitch;
	if (bits & MOB_ANGLER)
		mobj.angles.roll = ByteToAngle(net_msg.ReadByte());
	else
		mobj.angles.roll = base.angles.roll;
	if (bits & MOB_SPRITE)
	{
		mobj.sprite = (word)net_msg.ReadShort();
		mobj.spritetype = mobj.sprite >> 10;
		mobj.sprite &= 0x3ff;
	}
	else
	{
		mobj.sprite = base.sprite;
		mobj.spritetype = base.spritetype;
	}
	if (bits & MOB_FRAME)
		mobj.frame = (byte)net_msg.ReadByte();
	else
		mobj.frame = base.frame;
	if (bits & MOB_TRANSLUC)
		mobj.translucency = net_msg.ReadByte();
	else
		mobj.translucency = base.translucency;
	if (bits & MOB_TRANSL)
		mobj.translation = net_msg.ReadByte();
	else
		mobj.translation = base.translation;
	if (bits & MOB_EFFECTS)
		mobj.effects = net_msg.ReadByte();
	else
		mobj.effects = base.effects;
	if (bits & MOB_MODEL)
	{
		mobj.model_index = net_msg.ReadShort();
		mobj.alias_model = model_precache[mobj.model_index];
	}
	else
	{
		mobj.model_index = base.model_index;
		mobj.alias_model = model_precache[mobj.model_index];
	}
	if (bits & MOB_SKIN)
		strcpy(mobj.skin, skin_list[net_msg.ReadByte()]);
	else
		mobj.skin[0] = 0;
	if (mobj.model_index && (bits & MOB_FRAME))
		mobj.alias_frame = net_msg.ReadByte();
	else
		mobj.alias_frame = base.alias_frame;
}

static void CL_ParseUpdateMobj(void)
{
	guard(CL_ParseUpdateMobj);
	int		i;
	int		bits;

	bits = net_msg.ReadByte();
	if (bits & MOB_MORE_BITS)
		bits |= net_msg.ReadByte() << 8;

	if (bits & MOB_BIG_NUM)
		i = net_msg.ReadShort();
	else
		i = net_msg.ReadByte();

	CL_ReadMobj(bits, cl_mobjs[i], cl_mo_base[i]);

	//	Marking mobj in use
	cl_mobjs[i].in_use = 2;

	if (bits & MOB_WEAPON && cl_mobjs[i].alias_model &&
		weapon_model_precache[cl_mobjs[i].model_index])
	{
		clmobj_t &ent = cl_mobjs[i];
		clmobj_t &wpent = cl_weapon_mobjs[i];

		wpent.in_use = true;
		wpent.origin = ent.origin;
		wpent.angles = ent.angles;
		wpent.alias_model = model_precache[net_msg.ReadShort()];
		wpent.alias_frame = 1;
		wpent.translucency = ent.translucency;

		R_PositionWeaponModel(wpent, weapon_model_precache[ent.model_index],
			ent.alias_frame);
	}
	else if (bits & MOB_WEAPON)
	{
		net_msg.ReadShort();
	}
	unguard;
}

static void CL_ParseSecUpdate(void)
{
	int			bits;
	int			i;

	bits = net_msg.ReadByte();
	if (bits & SUB_BIG_NUM)
		i = net_msg.ReadShort();
	else
		i = net_msg.ReadByte();

	if (bits & SUB_FLOOR)
		GClLevel->Sectors[i].floor.dist = net_msg.ReadShort();
	if (bits & SUB_CEIL)
		GClLevel->Sectors[i].ceiling.dist = net_msg.ReadShort();
	if (bits & SUB_LIGHT)
		GClLevel->Sectors[i].params.lightlevel = net_msg.ReadByte() << 2;
	if (bits & SUB_FLOOR_X)
		GClLevel->Sectors[i].floor.xoffs = net_msg.ReadByte() & 63;
	if (bits & SUB_FLOOR_Y)
		GClLevel->Sectors[i].floor.yoffs = net_msg.ReadByte() & 63;
	if (bits & SUB_CEIL_X)
		GClLevel->Sectors[i].ceiling.xoffs = net_msg.ReadByte() & 63;
	if (bits & SUB_CEIL_Y)
		GClLevel->Sectors[i].ceiling.yoffs = net_msg.ReadByte() & 63;
	if (bits & (SUB_FLOOR | SUB_CEIL))
		CalcSecMinMaxs(&GClLevel->Sectors[i]);
}

static void CL_ParseViewData(void)
{
	int		i;
	int		bits;

	net_msg >> cl.vieworg.x
			>> cl.vieworg.y
			>> cl.vieworg.z;
	cl.extralight = net_msg.ReadByte();
	cl.fixedcolormap = net_msg.ReadByte();
	cl.palette = net_msg.ReadByte();
	cl.translucency = net_msg.ReadByte();
	cl.pspriteSY = net_msg.ReadShort();

	cl.psprites[0].sprite = net_msg.ReadShort();
	if (cl.psprites[0].sprite != -1)
	{
		cl.psprites[0].frame = net_msg.ReadByte();
		cl.psprites[0].alias_model = model_precache[net_msg.ReadShort()];
		cl.psprites[0].alias_frame = net_msg.ReadByte();
		cl.psprites[0].sx = net_msg.ReadShort();
		cl.psprites[0].sy = net_msg.ReadShort();
	}

	cl.psprites[1].sprite = net_msg.ReadShort();
	if (cl.psprites[1].sprite != -1)
	{
		cl.psprites[1].frame = net_msg.ReadByte();
		cl.psprites[1].alias_model = model_precache[net_msg.ReadShort()];
		cl.psprites[1].alias_frame = net_msg.ReadByte();
		cl.psprites[1].sx = net_msg.ReadShort();
		cl.psprites[1].sy = net_msg.ReadShort();
	}

	cl.health = net_msg.ReadByte();
	net_msg >> cl.items;
	cl.Frags = net_msg.ReadShort();

	bits = net_msg.ReadByte();
	for (i = 0; i < NUM_CSHIFTS; i++)
	{
		if (bits & (1 << i))
			net_msg >> cl.cshifts[i];
		else
			cl.cshifts[i] = 0;
	}
}

static void CL_ParseStartSound(void)
{
	word		sound_id;
	word		origin_id;
	float		x = 0.0;
	float		y = 0.0;
	float		z = 0.0;
	byte		volume;
	int			channel;

	net_msg	>> sound_id
			>> origin_id;

	channel = origin_id >> 13;
	origin_id &= 0x1fff;

	if (origin_id)
	{
		x = net_msg.ReadShort();
		y = net_msg.ReadShort();
		z = net_msg.ReadShort();
	}
	net_msg	>> volume;

	S_StartSound(sound_id, TVec(x, y, z), TVec(0, 0, 0), origin_id, channel, volume);
}

static void CL_ParseStopSound(void)
{
	word	origin_id;
	int		channel;

	net_msg >> origin_id;

	channel = origin_id >> 13;
	origin_id &= 0x1fff;

	S_StopSound(origin_id, channel);
}

static void CL_ParseStartSeq(void)
{
	int			origin_id;
	float		x;
	float		y;
	float		z;
	char		*name;

	origin_id = net_msg.ReadShort();
	x = net_msg.ReadShort();
	y = net_msg.ReadShort();
	z = net_msg.ReadShort();
	name = net_msg.ReadString();

	SN_StartSequenceName(origin_id, TVec(x, y, z), name);
}

static void CL_ParseStopSeq(void)
{
	word	origin_id;

	net_msg >> origin_id;

	SN_StopSequence(origin_id);
}

static void CL_ParseTime()
{
	guard(CL_ParseTime);
	float	new_time;
	int		i;

	if (cls.signon == SIGNONS - 1)
	{
		cls.signon = SIGNONS;
		CL_SignonReply();
		CmdBuf << "HideConsole\n";
	}

	if (cls.signon != SIGNONS)
		Sys_Error("Update when at %d", cls.signon);

	for (i = 0; i < GClLevel->NumSectors; i++)
	{
		sector_t &sec = GClLevel->Sectors[i];
		if (sec.floor.dist != sec.base_floorheight ||
			sec.ceiling.dist != sec.base_ceilingheight)
		{
			sec.floor.dist = sec.base_floorheight;
			sec.ceiling.dist = sec.base_ceilingheight;
			CalcSecMinMaxs(&sec);
		}
		sec.params.lightlevel = sec.base_lightlevel;
		sec.floor.xoffs = 0.0;
		sec.floor.yoffs = 0.0;
		sec.ceiling.xoffs = 0.0;
		sec.ceiling.yoffs = 0.0;
	}

	for (i = 0; i < GClLevel->NumSides; i++)
	{
		GClLevel->Sides[i].textureoffset = GClLevel->Sides[i].base_textureoffset;
		GClLevel->Sides[i].rowoffset = GClLevel->Sides[i].base_rowoffset;
	}

	for (i = 0; i < GMaxEntities; i++)
	{
		if (cl_mobjs[i].in_use)
		{
			cl_mobjs[i].in_use--;
		}
	}

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (cl_weapon_mobjs[i].in_use)
		{
			cl_weapon_mobjs[i].in_use--;
		}
	}

	R_AnimateSurfaces();
	net_msg >> new_time;
	cl_level.tictime = int(new_time * 35);
	cl_level.time = new_time;
	cl.mtime[1] = cl.mtime[0];
	cl.mtime[0] = new_time;
	unguard;
}

//==========================================================================
//
//	CL_ReadFromServerInfo
//
//==========================================================================

static void CL_ReadFromServerInfo(void)
{
	TCvar::SetCheating(!!atoi(Info_ValueForKey(cl.serverinfo, "sv_cheats")));
}

//==========================================================================
//
//	CL_DoLoadLevel
//
//==========================================================================

void CL_SetupLevel(void);

static void CL_ParseServerInfo(void)
{
	guard(CL_ParseServerInfo);
	byte		ver;

	net_msg >> ver;
	if (ver != PROTOCOL_VERSION)
		Host_Error("Server runs protocol %d, not %d", ver, PROTOCOL_VERSION);

	CL_Clear();

	strcpy(cl.serverinfo, net_msg.ReadString());
	CL_ReadFromServerInfo();

	strcpy(cl_level.mapname, net_msg.ReadString());
	strcpy(cl_level.level_name, net_msg.ReadString());

	cl.clientnum = net_msg.ReadByte();
	cl.maxclients = net_msg.ReadByte();
	cl.deathmatch = net_msg.ReadByte();

	net_msg >> cl_level.totalkills
			>> cl_level.totalitems
			>> cl_level.totalsecret;
	cl_level.sky1Texture = (word)net_msg.ReadShort();
	cl_level.sky2Texture = (word)net_msg.ReadShort();
	net_msg >> cl_level.sky1ScrollDelta
			>> cl_level.sky2ScrollDelta;
	cl_level.doubleSky = net_msg.ReadByte();
	cl_level.lightning = net_msg.ReadByte();
	strcpy(cl_level.skybox, net_msg.ReadString());
	strcpy(cl_level.fadetable, net_msg.ReadString());

	strcpy(cl_level.songLump, net_msg.ReadString());
	cl_level.cdTrack = net_msg.ReadByte();

	GCon->Log("---------------------------------------");
	GCon->Log(cl_level.level_name);
	GCon->Log("");
    C_ClearNotify();

	clpr.SetGlobal("netgame", cl.maxclients > 1);
	clpr.SetGlobal("deathmatch", cl.deathmatch);

	CL_LoadLevel(cl_level.mapname);

	//	Temporary hack to restore seen on automap flags.
#ifdef SERVER
	if (sv.active)
	{
		for (int i = 0; i < GClLevel->NumLines; i++)
		{
			GClLevel->Lines[i].flags |= GLevel->Lines[i].flags & ML_MAPPED;
		}
	}
#endif

	R_Start();
	S_Start();

	SB_Start();

	Z_CheckHeap();

	GCon->Log(NAME_Dev, "Client level loaded");
	unguard;
}

//==========================================================================
//
//	CL_ParseIntermission
//
//==========================================================================

static void CL_ParseIntermission(void)
{
	int			i;
	int			j;
	mapInfo_t	ninfo;
	char		*nextmap;

	strcpy(im.leavemap, cl_level.mapname);
	strcpy(im.leave_name, cl_level.level_name);
	P_GetMapInfo(cl_level.mapname, ninfo);
	im.leavecluster = ninfo.cluster;

	nextmap = net_msg.ReadString();
	P_GetMapInfo(nextmap, ninfo);
	strcpy(im.entermap, nextmap);
	strcpy(im.enter_name, ninfo.name);
	im.entercluster = ninfo.cluster;

	im.totalkills = cl_level.totalkills;
	im.totalitems = cl_level.totalitems;
	im.totalsecret = cl_level.totalsecret;
	im.time = cl_level.time;
	for (i = 0; i < MAXPLAYERS; i++)
	{
		scores[i].bActive = net_msg.ReadByte();
		for (j = 0; j < MAXPLAYERS; j++)
			scores[i].frags[j] = (char)net_msg.ReadByte();
		scores[i].killcount = net_msg.ReadShort();
		scores[i].itemcount = net_msg.ReadShort();
		scores[i].secretcount = net_msg.ReadShort();
	}
    IM_Start();
}

//==========================================================================
//
//	CL_ParseSpriteList
//
//==========================================================================

static void CL_ParseSpriteList(void)
{
	int count = net_msg.ReadShort();
	for (int i = 0; i < count; i++)
	{
		R_InstallSprite(net_msg.ReadString(), i);
	}
}

//==========================================================================
//
//	CL_ParseModel
//
//==========================================================================

static void CL_ParseModel(void)
{
	int i = net_msg.ReadShort();
	char *name = va("models/%s", net_msg.ReadString());
	weapon_model_precache[i] = NULL;
	if (FL_FindFile(name, NULL))
	{
		model_precache[i] = Mod_FindName(name);
		if (strstr(name, "tris.md2"))
		{
			char wpname[MAX_VPATH];

			strcpy(wpname, name);
			FL_StripFilename(wpname);
			strcat(wpname, "/weapon.md2");
			if (FL_FindFile(wpname, NULL))
			{
				weapon_model_precache[i] = Mod_FindName(wpname);
			}
			else
			{
				GCon->Logf("Can't find wepon info model %s", wpname);
			}
		}
	}
	else if (TCvar::Value("r_models"))
	{
		GCon->Logf("Can't find %s", name);
	}
}

//==========================================================================
//
//	CL_ParseSkin
//
//==========================================================================

static void CL_ParseSkin(void)
{
	int i = net_msg.ReadByte();
	strcpy(skin_list[i], va("models/%s", net_msg.ReadString()));
}

//==========================================================================
//
//	CL_ReadFromUserInfo
//
//==========================================================================

static void CL_ReadFromUserInfo(int)
{
}

//==========================================================================
//
//	CL_ParseLineTransuc
//
//==========================================================================

static void CL_ParseLineTransuc(void)
{
	int i = net_msg.ReadShort();
	int fuzz = net_msg.ReadByte();
	GClLevel->Lines[i].translucency = fuzz;
}

//==========================================================================
//
//	CL_ParseExtraFloor
//
//==========================================================================

static void CL_ParseExtraFloor(void)
{
	int i = net_msg.ReadShort();
	int j = net_msg.ReadShort();
	AddExtraFloor(&GClLevel->Lines[i], &GClLevel->Sectors[j]);
}

//==========================================================================
//
//	CL_ParseServerMessage
//
//==========================================================================

void CL_ParseServerMessage(void)
{
	guard(CL_ParseServerMessage);
	int			i;
	byte		cmd_type;
	float		x;
	float		y;
	int			tag;
	float		angle;
	char		name[MAX_INFO_KEY];
	char		string[MAX_INFO_VALUE];
	TVec		origin;
	float		radius;
	dword		color;
	int			trans;
	sector_t*	sec;

	net_msg.BeginReading();

	// update command store from the packet
	while (1)
	{
		if (net_msg.badread)
		{
			GCon->Logf(NAME_Dev, "Length %d", net_msg.CurSize);
			for (i = 0; i < net_msg.CurSize; i++)
				GCon->Logf(NAME_Dev, "  %d", (int)net_msg.Data[i]);
			Host_Error("Packet corupted");
		}

		net_msg >> cmd_type;

		if (net_msg.badread)
			break; // Here this means end of packet

		switch (cmd_type)
		{
		 case svc_nop:
			break;

		 case svc_disconnect:
			Host_EndGame("Server disconnected");

		 case svc_server_info:
			CL_ParseServerInfo();
			break;

		 case svc_spawn_baseline:
			CL_ParseBaseline();
			break;

		 case svc_update_mobj:
		 	CL_ParseUpdateMobj();
			break;

		 case svc_side_top:
			i = net_msg.ReadShort();
			GClLevel->Sides[i].toptexture = net_msg.ReadShort();
			break;

		 case svc_side_mid:
			i = net_msg.ReadShort();
			GClLevel->Sides[i].midtexture = net_msg.ReadShort();
			break;

		 case svc_side_bot:
			i = net_msg.ReadShort();
			GClLevel->Sides[i].bottomtexture = net_msg.ReadShort();
			break;

		 case svc_side_ofs:
			i = net_msg.ReadShort();
			GClLevel->Sides[i].textureoffset = net_msg.ReadShort();
			GClLevel->Sides[i].rowoffset = net_msg.ReadShort();
			break;

		 case svc_sec_floor:
			i = net_msg.ReadShort();
			GClLevel->Sectors[i].floor.pic = (word)net_msg.ReadShort();
			break;

		 case svc_sec_ceil:
			i = net_msg.ReadShort();
			GClLevel->Sectors[i].ceiling.pic = (word)net_msg.ReadShort();
			break;

		 case svc_sec_update:
		 	CL_ParseSecUpdate();
			break;

		 case svc_set_angles:
			cl.viewangles.pitch = AngleMod180(ByteToAngle(net_msg.ReadByte()));
			cl.viewangles.yaw = ByteToAngle(net_msg.ReadByte());
			cl.viewangles.roll = ByteToAngle(net_msg.ReadByte());
			break;

		 case svc_center_look:
//FIXME
			break;

		 case svc_view_data:
		 	CL_ParseViewData();
			break;

		 case svc_start_sound:
			CL_ParseStartSound();
			break;

		 case svc_stop_sound:
			CL_ParseStopSound();
			break;

		 case svc_start_seq:
			CL_ParseStartSeq();
			break;

		 case svc_stop_seq:
			CL_ParseStopSeq();
			break;

		 case svc_print:
			C_NotifyMessage(net_msg.ReadString());
			break;

		 case svc_center_print:
			C_CenterMessage(net_msg.ReadString());
			break;

		 case svc_time:
			CL_ParseTime();
			break;

		 case svc_poly_spawn:
			x = net_msg.ReadShort();
			y =	net_msg.ReadShort();
			tag = net_msg.ReadByte();
			CL_PO_SpawnPolyobj(x, y, tag);
			break;

		 case svc_poly_translate:
			x = net_msg.ReadShort();
			y =	net_msg.ReadShort();
			tag = net_msg.ReadByte();
			CL_PO_TranslateToStartSpot(x, y, tag);
			break;

		 case svc_poly_update:
			i = net_msg.ReadByte();
			x = net_msg.ReadShort();
			y =	net_msg.ReadShort();
			angle = ByteToAngle(net_msg.ReadByte());
			CL_PO_Update(i, x, y, angle);
			break;

		 case svc_force_lightning:
			R_ForceLightning();
			break;

		 case svc_intermission:
			CL_ParseIntermission();
			break;

		 case svc_pause:
		 	cl.bPaused = net_msg.ReadByte();
   			if (cl.bPaused)
				S_PauseSound();
   			else
				S_ResumeSound();
			break;

		 case svc_stats_long:
			i = net_msg.ReadByte();
			net_msg >> cl.user_fields[i];
			break;

		 case svc_stats_short:
			i = net_msg.ReadByte();
			cl.user_fields[i] = net_msg.ReadShort();
			break;

		 case svc_stats_byte:
			i = net_msg.ReadByte();
			cl.user_fields[i] = net_msg.ReadByte();
			break;

		 case svc_stringcmd:
		 	CmdBuf << net_msg.ReadString();
			break;

		 case svc_signonnum:
			i = net_msg.ReadByte();
			if (i <= cls.signon)
				Host_Error("Received signon %i when at %i", i, cls.signon);
			cls.signon = i;
			CL_SignonReply();
			break;

		 case svc_skip_intermission:
			IM_SkipIntermission();
			break;

		 case svc_finale:
			F_StartFinale();
			break;

		 case svc_sec_floor_plane:
			i = net_msg.ReadShort();
			net_msg >> GClLevel->Sectors[i].floor.normal.x
					>> GClLevel->Sectors[i].floor.normal.y
					>> GClLevel->Sectors[i].floor.normal.z
					>> GClLevel->Sectors[i].floor.dist;
			GClLevel->Sectors[i].base_floorheight = GClLevel->Sectors[i].floor.dist;
			CalcSecMinMaxs(&GClLevel->Sectors[i]);
			break;

		 case svc_sec_ceil_plane:
			i = net_msg.ReadShort();
			net_msg >> GClLevel->Sectors[i].ceiling.normal.x
					>> GClLevel->Sectors[i].ceiling.normal.y
					>> GClLevel->Sectors[i].ceiling.normal.z
					>> GClLevel->Sectors[i].ceiling.dist;
			GClLevel->Sectors[i].base_ceilingheight  = GClLevel->Sectors[i].ceiling.dist;
			CalcSecMinMaxs(&GClLevel->Sectors[i]);
			break;

		 case svc_serverinfo:
			strcpy(name, net_msg.ReadString());
			strcpy(string, net_msg.ReadString());
			Info_SetValueForKey(cl.serverinfo, name, string);
			CL_ReadFromServerInfo();
			break;

		 case svc_userinfo:
			i = net_msg.ReadByte();
			strcpy(scores[i].userinfo, net_msg.ReadString());
			CL_ReadFromUserInfo(i);
			break;

		 case svc_setinfo:
			i = net_msg.ReadByte();
			strcpy(name, net_msg.ReadString());
			strcpy(string, net_msg.ReadString());
			Info_SetValueForKey(scores[i].userinfo, name, string);
			CL_ReadFromUserInfo(i);
			break;

		 case svc_sprites:
			CL_ParseSpriteList();
			break;

		 case svc_model:
			CL_ParseModel();
			break;

		 case svc_skin:
			CL_ParseSkin();
			break;

		 case svc_line_transluc:
			CL_ParseLineTransuc();
			break;

		 case svc_sec_transluc:
			i = net_msg.ReadShort();
			trans = net_msg.ReadByte();
			sec = &GClLevel->Sectors[i];
			sec->floor.translucency = trans;
			sec->ceiling.translucency = trans;
			for (i = 0; i < sec->linecount; i++)
			{
				sec->lines[i]->translucency = trans;
			}
			break;

		 case svc_extra_floor:
			CL_ParseExtraFloor();
			break;

		 case svc_swap_planes:
			i = net_msg.ReadShort();
			SwapPlanes(&GClLevel->Sectors[i]);
			break;

		 case svc_static_light:
			origin.x = net_msg.ReadShort();
			origin.y = net_msg.ReadShort();
			origin.z = net_msg.ReadShort();
			radius = (byte)net_msg.ReadByte() * 8;
			R_AddStaticLight(origin, radius, 0xffffffff);
			break;

		 case svc_static_light_rgb:
			origin.x = net_msg.ReadShort();
			origin.y = net_msg.ReadShort();
			origin.z = net_msg.ReadShort();
			radius = (byte)net_msg.ReadByte() * 8;
			net_msg >> color;
			R_AddStaticLight(origin, radius, color);
			break;

		 case svc_sec_light_color:
			sec = &GClLevel->Sectors[net_msg.ReadShort()];
			sec->params.LightColor = (net_msg.ReadByte() << 16) |
				(net_msg.ReadByte() << 8) | net_msg.ReadByte();
			break;

		case svc_change_sky:
			cl_level.sky1Texture = (word)net_msg.ReadShort();
			cl_level.sky2Texture = (word)net_msg.ReadShort();
			R_SkyChanged();
			break;

		case svc_change_music:
			strcpy(cl_level.songLump, net_msg.ReadString());
			cl_level.cdTrack = net_msg.ReadByte();
			S_MusicChanged();
			break;

		case svc_set_floor_light_sec:
			i = (word)net_msg.ReadShort();
			GClLevel->Sectors[i].floor.LightSourceSector = net_msg.ReadShort();
			break;

		case svc_set_ceil_light_sec:
			i = (word)net_msg.ReadShort();
			GClLevel->Sectors[i].ceiling.LightSourceSector = net_msg.ReadShort();
			break;

		default:
			if (clpr.Exec(pf_ParseServerCommand, cmd_type))
			{
				break;
			}
			GCon->Logf(NAME_Dev, "Length %d", net_msg.CurSize);
			for (i = 0; i < net_msg.CurSize; i++)
			{
				GCon->Logf(NAME_Dev, "  %d", (int)net_msg.Data[i]);
			}
			GCon->Logf(NAME_Dev, "ReadCount %d", net_msg.readcount);
			Host_Error("Invalid packet %d", cmd_type);
			break;
		}
   	}
	unguard;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.33  2005/03/28 07:28:19  dj_jl
//	Transfer lighting and other BOOM stuff.
//
//	Revision 1.32  2004/12/27 12:23:16  dj_jl
//	Multiple small changes for version 1.16
//	
//	Revision 1.31  2003/03/08 11:30:07  dj_jl
//	Got rid of some warnings.
//	
//	Revision 1.30  2002/09/07 16:31:50  dj_jl
//	Added Level class.
//	
//	Revision 1.29  2002/08/28 16:39:19  dj_jl
//	Implemented sector light color.
//	
//	Revision 1.28  2002/08/05 17:20:00  dj_jl
//	Added guarding.
//	
//	Revision 1.27  2002/07/23 16:29:55  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.26  2002/06/29 16:00:45  dj_jl
//	Added total frags count.
//	
//	Revision 1.25  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.24  2002/03/28 17:59:04  dj_jl
//	Fixed negative frags.
//	
//	Revision 1.23  2002/03/12 19:21:55  dj_jl
//	No need for linefeed in client-printing
//	
//	Revision 1.22  2002/02/02 19:20:41  dj_jl
//	FFunction pointers used instead of the function numbers
//	
//	Revision 1.21  2002/01/11 08:09:34  dj_jl
//	Added sector plane swapping
//	
//	Revision 1.20  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.19  2001/12/27 17:36:47  dj_jl
//	Some speedup
//	
//	Revision 1.18  2001/12/18 19:05:03  dj_jl
//	Made TCvar a pure C++ class
//	
//	Revision 1.17  2001/12/12 19:28:49  dj_jl
//	Some little changes, beautification
//	
//	Revision 1.16  2001/12/04 18:16:28  dj_jl
//	Player models and skins handled by server
//	
//	Revision 1.15  2001/12/01 17:51:46  dj_jl
//	Little changes to compile with MSVC
//	
//	Revision 1.14  2001/11/09 14:28:23  dj_jl
//	Fixed parsing of sound starting
//	
//	Revision 1.13  2001/10/27 07:51:27  dj_jl
//	Beautification
//	
//	Revision 1.12  2001/10/22 17:25:55  dj_jl
//	Floatification of angles
//	
//	Revision 1.11  2001/10/18 17:36:31  dj_jl
//	A lots of changes for Alpha 2
//	
//	Revision 1.10  2001/10/09 17:24:05  dj_jl
//	Changes after bug chatching
//	
//	Revision 1.9  2001/10/08 17:33:01  dj_jl
//	Different client and server level structures
//	
//	Revision 1.8  2001/10/04 17:18:23  dj_jl
//	Implemented the rest of cvar flags
//	
//	Revision 1.7  2001/09/05 12:21:42  dj_jl
//	Release changes
//	
//	Revision 1.6  2001/08/29 17:55:42  dj_jl
//	Added sound channels
//	
//	Revision 1.5  2001/08/15 17:24:02  dj_jl
//	Improved object update on packet overflows
//	
//	Revision 1.4  2001/08/07 16:46:23  dj_jl
//	Added player models, skins and weapon
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
