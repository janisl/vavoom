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
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
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
void CL_PO_Update(int i, float x, float y, angle_t angle);
void CL_SignonReply(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

clmobj_t		cl_mobjs[MAX_MOBJS];
clmobj_t		cl_mo_base[MAX_MOBJS];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static model_t	*model_precache[1024];

// CODE --------------------------------------------------------------------

void CL_Clear(void)
{
	memset(&cl, 0, sizeof(cl));
	memset(&cl_level, 0, sizeof(level));
	memset(cl_mobjs, 0, sizeof(cl_mobjs));
	memset(cl_mo_base, 0, sizeof(cl_mo_base));
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
		Z_FreeTag(PU_LEVSPEC);
	}
}

static void CL_ReadMobjBase(clmobj_t &mobj)
{
	mobj.origin.x = net_msg.ReadShort();
	mobj.origin.y = net_msg.ReadShort();
	mobj.origin.z = net_msg.ReadShort();
	mobj.angles.yaw = net_msg.ReadByte() << 24;
	mobj.angles.pitch = net_msg.ReadByte() << 24;
	mobj.angles.roll = net_msg.ReadByte() << 24;
	mobj.sprite = (word)net_msg.ReadShort();
	mobj.frame = (word)net_msg.ReadShort();
	mobj.translucency = net_msg.ReadByte();
	mobj.translation = net_msg.ReadByte();
	mobj.floorclip = net_msg.ReadShort();
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

static void CL_ReadMobj(int bits, clmobj_t &mobj, const clmobj_t &base)
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
		mobj.angles.yaw = net_msg.ReadByte() << 24;
	else
		mobj.angles.yaw = base.angles.yaw;
	if (bits & MOB_ANGLEP)
		mobj.angles.pitch = net_msg.ReadByte() << 24;
	else
		mobj.angles.pitch = base.angles.pitch;
	if (bits & MOB_ANGLER)
		mobj.angles.roll = net_msg.ReadByte() << 24;
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
	if (bits & MOB_FLOORCLIP)
		mobj.floorclip = net_msg.ReadShort();
	else
		mobj.floorclip = base.floorclip;
	if (bits & MOB_EFFECTS)
		mobj.effects = net_msg.ReadByte();
	else
		mobj.effects = base.effects;
	if (bits & MOB_MODEL)
		mobj.model_index = net_msg.ReadShort();
	else
		mobj.model_index = base.model_index;
	mobj.alias_model = model_precache[mobj.model_index];
	if (mobj.model_index && (bits & MOB_FRAME))
		mobj.alias_frame = net_msg.ReadByte();
	else
		mobj.alias_frame = base.alias_frame;
}

static void CL_ParseUpdateMobj(void)
{
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

	//	A dummy thinker marking mobjs in use
	cl_mobjs[i].in_use = true;
}

//==========================================================================
//
//	CalcSecMinMaxs
//
//==========================================================================

static void CalcSecMinMaxs(sector_t *sector)
{
	float	minz;
	float	maxz;

	minz = 99999.0;
	maxz = -99999.0;
	for (int i = 0; i < sector->linecount; i++)
	{
		float z;
		z = sector->floor.GetPointZ(*sector->lines[i]->v1);
		if (minz > z)
			minz = z;
		if (maxz < z)
			maxz = z;
	}
	sector->floor.minz = minz;
	sector->floor.maxz = maxz;

	minz = 99999.0;
	maxz = -99999.0;
	for (int i = 0; i < sector->linecount; i++)
	{
		float z;
		z = sector->ceiling.GetPointZ(*sector->lines[i]->v1);
		if (minz > z)
			minz = z;
		if (maxz < z)
			maxz = z;
	}
	sector->ceiling.minz = minz;
	sector->ceiling.maxz = maxz;
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
		cl_level.sectors[i].floor.dist = net_msg.ReadShort();
	if (bits & SUB_CEIL)
		cl_level.sectors[i].ceiling.dist = net_msg.ReadShort();
	if (bits & SUB_LIGHT)
		cl_level.sectors[i].params.lightlevel = net_msg.ReadByte() << 2;
	if (bits & SUB_FLOOR_X)
		cl_level.sectors[i].floor.xoffs = net_msg.ReadByte() & 63;
	if (bits & SUB_FLOOR_Y)
		cl_level.sectors[i].floor.yoffs = net_msg.ReadByte() & 63;
	if (bits & SUB_CEIL_X)
		cl_level.sectors[i].ceiling.xoffs = net_msg.ReadByte() & 63;
	if (bits & SUB_CEIL_Y)
		cl_level.sectors[i].ceiling.yoffs = net_msg.ReadByte() & 63;
	if (bits & (SUB_FLOOR | SUB_CEIL))
		CalcSecMinMaxs(&cl_level.sectors[i]);
}

static void CL_ParseViewData(void)
{
	int		i;
	int		bits;

	net_msg >> cl.vieworg.x
			>> cl.vieworg.y
			>> cl.vieworg.z;
	cl.origin_id = net_msg.ReadShort();
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
	for (i = 0; i < MAXPLAYERS; i++)
		cl.frags[i] = net_msg.ReadByte();

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

	net_msg	>> sound_id
			>> origin_id;
	if (origin_id)
	{
		x = net_msg.ReadShort();
		y = net_msg.ReadShort();
		z = net_msg.ReadShort();
	}
	net_msg	>> volume;

	S_StartSound(sound_id, TVec(x, y, z), TVec(0, 0, 0), origin_id, volume);
}

static void CL_ParseStopSound(void)
{
	word	origin_id;

	net_msg >> origin_id;

	S_StopSound(origin_id);
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
	int		new_time;
	int		i;

	if (cls.signon == SIGNONS - 1)
	{
		cls.signon = SIGNONS;
		CL_SignonReply();
		CmdBuf << "HideConsole\n";
	}

	if (cls.signon != SIGNONS)
		Sys_Error("Update when at %d", cls.signon);

	for (i = 0; i < cl_level.numsectors; i++)
	{
		sector_t &sec = cl_level.sectors[i];
		if (sec.floor.dist != sec.base_floorheight ||
			sec.ceiling.dist != sec.base_ceilingheight)
		{
			sec.floor.dist = sec.base_floorheight;
			sec.ceiling.dist = sec.base_ceilingheight;
			CalcSecMinMaxs(&sec);
		}
		cl_level.sectors[i].params.lightlevel = cl_level.sectors[i].base_lightlevel;
		cl_level.sectors[i].floor.xoffs = 0.0;
		cl_level.sectors[i].floor.yoffs = 0.0;
		cl_level.sectors[i].ceiling.xoffs = 0.0;
		cl_level.sectors[i].ceiling.yoffs = 0.0;
	}

	for (i = 0; i < cl_level.numsides; i++)
	{
		cl_level.sides[i].textureoffset = cl_level.sides[i].base_textureoffset;
		cl_level.sides[i].rowoffset = cl_level.sides[i].base_rowoffset;
	}

	for (i = 0; i < MAX_MOBJS; i++)
	{
		cl_mobjs[i].in_use = false;
	}

	net_msg >> new_time;
	for (i = cl_level.tictime; i < new_time; i++)
	{
		R_AnimateSurfaces();
	}
	cl_level.tictime = new_time;
	cl_level.time = (float)new_time / 35.0;
}

//==========================================================================
//
//	CL_ReadFromServerInfo
//
//==========================================================================

static void	CL_ReadFromServerInfo(void)
{
}

//==========================================================================
//
//	CL_DoLoadLevel
//
//==========================================================================

void CL_SetupLevel(void);
void P_GetMapInfo(const char *map, mapInfo_t &info);

static void CL_ParseServerInfo(void)
{
	byte		ver;
	mapInfo_t	info;

	net_msg >> ver;
	if (ver != PROTOCOL_VERSION)
		Host_Error("Server runs protocol %d, not %d", ver, PROTOCOL_VERSION);

	CL_Clear();

	strcpy(cl.serverinfo, net_msg.ReadString());
	CL_ReadFromServerInfo();

	strcpy(cl_level.mapname, net_msg.ReadString());
	P_GetMapInfo(cl_level.mapname, info);
	strcpy(cl_level.level_name, info.name);

	LoadLevel(cl_level, cl_level.mapname, false);

	R_Start(info);
   	S_Start(info);

	con << "---------------------------------------\n";
	con << cl_level.level_name << "\n\n";
    C_ClearNotify();

	cl.clientnum = net_msg.ReadByte();
	cl.maxclients =	net_msg.ReadByte();
	cl.deathmatch = net_msg.ReadByte();

	net_msg >> cl_level.totalkills
			>> cl_level.totalitems
			>> cl_level.totalsecret;

	clpr.SetGlobal("netgame", cl.maxclients > 1);
	clpr.SetGlobal("deathmatch", cl.deathmatch);

   	SB_Start();

    Z_CheckHeap();

	cond << "Client level loaded\n";
}

//==========================================================================
//
//	CL_ParseIntermission
//
//==========================================================================

static void	CL_ParseIntermission(void)
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
		scores[i].active = net_msg.ReadByte();
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

static void	CL_ParseSpriteList(void)
{
	int count = net_msg.ReadShort();
	for (int i = 0; i < count; i++)
	{
		R_InstallSprite(net_msg.ReadString(), i);
	}
}

//==========================================================================
//
//	CL_ParseModelList
//
//==========================================================================

static void	CL_ParseModelList(void)
{
	int count = net_msg.ReadShort();
	for (int i = 1; i < count; i++)
	{
		char *name = va("models/%s", net_msg.ReadString());
		if (FL_FindFile(name, NULL))
		{
			model_precache[i] = Mod_FindName(name);
		}
		else if (Cvar_Value("r_models"))
		{
			con << "Can't find " << name << endl;
		}
	}
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
	cl_level.lines[i].translucency = fuzz;
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
	AddExtraFloor(&cl_level.lines[i], &cl_level.sectors[j]);
}

//==========================================================================
//
//	CL_ParseServerMessage
//
//==========================================================================

void CL_ParseServerMessage(void)
{
	int			i;
	byte		cmd_type;
	float		x;
	float		y;
	int			tag;
	angle_t		angle;
	char		name[MAX_INFO_KEY];
	char		string[MAX_INFO_VALUE];
	TVec		origin;
	float		radius;
	dword		color;
	int			trans;
	sector_t	*sec;

	net_msg.BeginReading();

	// update command store from the packet
	while (1)
	{
		if (net_msg.badread)
		{
			cond <<	net_msg.CurSize << " [ ";
			for (i = 0; i < net_msg.CurSize; i++)
				cond << (int)net_msg.Data[i] << ' ';
			cond << "]\n";
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
			cl_level.sides[i].toptexture = net_msg.ReadShort();
			break;

		 case svc_side_mid:
			i = net_msg.ReadShort();
			cl_level.sides[i].midtexture = net_msg.ReadShort();
			break;

		 case svc_side_bot:
			i = net_msg.ReadShort();
			cl_level.sides[i].bottomtexture = net_msg.ReadShort();
			break;

		 case svc_side_ofs:
			i = net_msg.ReadShort();
			cl_level.sides[i].textureoffset = net_msg.ReadShort();
			cl_level.sides[i].rowoffset = net_msg.ReadShort();
			break;

		 case svc_sec_floor:
			i = net_msg.ReadShort();
			cl_level.sectors[i].floor.pic = (word)net_msg.ReadShort();
			break;

		 case svc_sec_ceil:
			i = net_msg.ReadShort();
			cl_level.sectors[i].ceiling.pic = (word)net_msg.ReadShort();
			break;

		 case svc_sec_update:
		 	CL_ParseSecUpdate();
			break;

		 case svc_set_angles:
			cl.viewangles.pitch = net_msg.ReadByte() << 24;
			cl.viewangles.yaw = net_msg.ReadByte() << 24;
			cl.viewangles.roll = net_msg.ReadByte() << 24;
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
			con << net_msg.ReadString();
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
			angle = net_msg.ReadByte() << 24;
			CL_PO_Update(i, x, y, angle);
			break;

		 case svc_force_lightning:
			R_ForceLightning();
			break;

		 case svc_intermission:
			CL_ParseIntermission();
			break;

		 case svc_pause:
		 	cl.paused = net_msg.ReadByte();
   			if (cl.paused)
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
			net_msg >> cl_level.sectors[i].floor.normal.x
					>> cl_level.sectors[i].floor.normal.y
					>> cl_level.sectors[i].floor.normal.z
					>> cl_level.sectors[i].floor.dist;
			cl_level.sectors[i].base_floorheight = cl_level.sectors[i].floor.dist;
			CalcSecMinMaxs(&cl_level.sectors[i]);
			break;

		 case svc_sec_ceil_plane:
			i = net_msg.ReadShort();
			net_msg >> cl_level.sectors[i].ceiling.normal.x
					>> cl_level.sectors[i].ceiling.normal.y
					>> cl_level.sectors[i].ceiling.normal.z
					>> cl_level.sectors[i].ceiling.dist;
			cl_level.sectors[i].base_ceilingheight  = cl_level.sectors[i].ceiling.dist;
			CalcSecMinMaxs(&cl_level.sectors[i]);
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

		 case svc_models:
			CL_ParseModelList();
			break;

		 case svc_line_transluc:
			CL_ParseLineTransuc();
			break;

		 case svc_sec_transluc:
			i = net_msg.ReadShort();
			trans = net_msg.ReadByte();
			sec = &cl_level.sectors[i];
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

		 default:
			if (clpr.Exec("ParseServerCommand", cmd_type))
			{
				break;
			}
			cond <<	net_msg.CurSize << " [ ";
			for (i = 0; i < net_msg.CurSize; i++)
				cond << (int)net_msg.Data[i] << ' ';
			cond << "]\n";
			Host_Error("Invalid packet %d", cmd_type);
			break;
		}
   	}
}

//**************************************************************************
//
//	$Log$
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
