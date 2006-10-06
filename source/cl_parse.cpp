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

#include "gamedefs.h"
#include "cl_local.h"
#include "progdefs.h"

// MACROS ------------------------------------------------------------------

#define MAX_CLASS_LOOKUP		1024

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void CL_ClearInput();
void CL_PO_Update(int i, float x, float y, float angle);
void CL_SignonReply();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VEntity**		cl_mobjs;
clmobjbase_t*	cl_mo_base;
VEntity*		cl_weapon_mobjs[MAXPLAYERS];
VModel*			model_precache[1024];
VStr			skin_list[256];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VModel*			weapon_model_precache[1024];
static VClass*			ClassLookup[MAX_CLASS_LOOKUP];

// CODE --------------------------------------------------------------------

void CL_Clear()
{
	guard(CL_Clear);
	GClGame->serverinfo.Clean();
	GClGame->intermission = 0;
	GClGame->time = 0;
	VViewEntity* PrevVEnts[NUMPSPRITES];
	PrevVEnts[0] = cl->ViewEnts[0];
	PrevVEnts[1] = cl->ViewEnts[1];
	memset((byte*)cl + sizeof(VObject), 0, cl->GetClass()->ClassSize - sizeof(VObject));
	cl->ViewEnts[0] = PrevVEnts[0];
	cl->ViewEnts[1] = PrevVEnts[1];
	memset(&cl_level, 0, sizeof(cl_level));
	for (int i = 0; i < GMaxEntities; i++)
		if (cl_mobjs[i])
			cl_mobjs[i]->ConditionalDestroy();
	memset(cl_mobjs, 0, sizeof(VEntity*) * GMaxEntities);
	memset(cl_mo_base, 0, sizeof(clmobjbase_t) * GMaxEntities);
	for (int i = 0; i < MAXPLAYERS; i++)
		if (cl_weapon_mobjs[i])
			cl_weapon_mobjs[i]->ConditionalDestroy();
	memset(cl_weapon_mobjs, 0, sizeof(cl_weapon_mobjs));
	memset(cl_dlights, 0, sizeof(cl_dlights));
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		scores[i].name.Clean();
		scores[i].userinfo.Clean();
	}
	memset(scores, 0, sizeof(scores));
	CL_ClearInput();
#ifdef SERVER
	if (!sv.active)
#endif
	{
		// Make sure all sounds are stopped.
		GAudio->StopAllSound();
	}
	for (int i = 0; i < GMaxEntities; i++)
		cl_mobjs[i] = Spawn<VEntity>();
	for (int i = 0; i < MAXPLAYERS; i++)
		cl_weapon_mobjs[i] = Spawn<VEntity>();
	cls.signon = 0;
	unguard;
}

static void CL_ReadMobjBase(VMessage& msg, clmobjbase_t &mobj)
{
	mobj.Class = ClassLookup[(vuint16)msg.ReadShort()];
	check(mobj.Class);
	mobj.State = mobj.Class->StatesLookup[(vuint16)msg.ReadShort()];
	mobj.origin.x = msg.ReadShort();
	mobj.origin.y = msg.ReadShort();
	mobj.origin.z = msg.ReadShort();
	mobj.angles.yaw = ByteToAngle(msg.ReadByte());
	mobj.angles.pitch = ByteToAngle(msg.ReadByte());
	mobj.angles.roll = ByteToAngle(msg.ReadByte());
	mobj.spritetype = msg.ReadByte();
	mobj.translucency = msg.ReadByte();
	mobj.translation = msg.ReadByte();
	mobj.effects = msg.ReadByte();
}

static void CL_ParseBaseline(VMessage& msg)
{
	int		i;

	i = msg.ReadShort();

	CL_ReadMobjBase(msg, cl_mo_base[i]);
}

static void CL_ReadMobj(VMessage& msg, int bits, VEntity*& mobj, const clmobjbase_t &base)
{
	VClass* C;
	if (bits & MOB_CLASS)
	{
		if (bits & MOB_BIG_CLASS)
			C = ClassLookup[(vuint16)msg.ReadShort()];
		else
			C = ClassLookup[msg.ReadByte()];
	}
	else
	{
		check(base.Class);
		C = base.Class;
	}
	if (mobj->GetClass() != C)
	{
		if (mobj->GetClass() != VEntity::StaticClass())
			GClLevel->RemoveThinker(mobj);
		mobj->ConditionalDestroy();
		mobj = (VEntity*)VObject::StaticSpawnObject(C);
		GClLevel->AddThinker(mobj);
	}
	if (bits & MOB_STATE)
	{
		if (bits & MOB_BIG_STATE)
			mobj->State = mobj->GetClass()->StatesLookup[(vuint16)msg.ReadShort()];
		else
			mobj->State = mobj->GetClass()->StatesLookup[msg.ReadByte()];
	}
	else
	{
		mobj->State = base.State;
	}
	if (bits & MOB_X)
		mobj->Origin.x = msg.ReadShort();
	else
		mobj->Origin.x = base.origin.x;
	if (bits & MOB_Y)
		mobj->Origin.y = msg.ReadShort();
	else
		mobj->Origin.y = base.origin.y;
	if (bits & MOB_Z)
		mobj->Origin.z = msg.ReadShort();
	else
		mobj->Origin.z = base.origin.z;
	if (bits & MOB_ANGLE)
		mobj->Angles.yaw = ByteToAngle(msg.ReadByte());
	else
		mobj->Angles.yaw = base.angles.yaw;
	if (bits & MOB_ANGLEP)
		mobj->Angles.pitch = ByteToAngle(msg.ReadByte());
	else
		mobj->Angles.pitch = base.angles.pitch;
	if (bits & MOB_ANGLER)
		mobj->Angles.roll = ByteToAngle(msg.ReadByte());
	else
		mobj->Angles.roll = base.angles.roll;
	if (bits & MOB_SPRITE)
		mobj->SpriteType = msg.ReadByte();
	else
		mobj->SpriteType = base.spritetype;
	if (bits & MOB_FULL_BRIGHT)
		mobj->EntityFlags |= VEntity::EF_FullBright;
	else
		mobj->EntityFlags &= ~VEntity::EF_FullBright;
	if (bits & MOB_TRANSLUC)
		mobj->Translucency = msg.ReadByte();
	else
		mobj->Translucency = base.translucency;
	if (bits & MOB_TRANSL)
		mobj->Translation = msg.ReadByte();
	else
		mobj->Translation = base.translation;
	if (bits & MOB_EFFECTS)
		mobj->Effects = msg.ReadByte();
	else
		mobj->Effects = base.effects;
	mobj->EntityFlags &= ~VEntity::EF_FixedModel;
	if (bits & MOB_MODEL)
	{
		mobj->EntityFlags |= VEntity::EF_FixedModel;
		mobj->FixedModelIndex = msg.ReadShort();
	}
	mobj->ModelSkinIndex = 0;
	mobj->ModelSkinNum = 0;
	if (bits & MOB_SKIN_NUM)
		mobj->ModelSkinNum = msg.ReadByte();
	else if (bits & MOB_SKIN_IDX)
		mobj->ModelSkinIndex = msg.ReadByte();
}

static void CL_ParseUpdateMobj(VMessage& msg)
{
	guard(CL_ParseUpdateMobj);
	int		i;
	int		bits;

	bits = msg.ReadByte();
	if (bits & MOB_MORE_BITS)
		bits |= msg.ReadByte() << 8;
	if (bits & MOB_MORE_BITS2)
		bits |= msg.ReadByte() << 16;

	if (bits & MOB_BIG_NUM)
		i = msg.ReadShort();
	else
		i = msg.ReadByte();

	CL_ReadMobj(msg, bits, cl_mobjs[i], cl_mo_base[i]);

	//	Marking mobj in use
	cl_mobjs[i]->InUse = 2;

	if (bits & MOB_WEAPON && model_precache[cl_mobjs[i]->EntityFlags &
		VEntity::EF_FixedModel ? cl_mobjs[i]->FixedModelIndex :
		cl_mobjs[i]->State->ModelIndex] &&
		weapon_model_precache[cl_mobjs[i]->EntityFlags &
		VEntity::EF_FixedModel ? cl_mobjs[i]->FixedModelIndex :
		cl_mobjs[i]->State->ModelIndex])
	{
		VEntity* ent = cl_mobjs[i];
		VEntity* wpent = cl_weapon_mobjs[i];

		wpent->InUse = true;
		wpent->Origin = ent->Origin;
		wpent->Angles = ent->Angles;
		wpent->EntityFlags |= VEntity::EF_FixedModel;
		wpent->FixedModelIndex = msg.ReadShort();
		wpent->Translucency = ent->Translucency;

		R_PositionWeaponModel(wpent, weapon_model_precache[ent->EntityFlags &
			VEntity::EF_FixedModel ? ent->FixedModelIndex :
			ent->State->ModelIndex], 1);
	}
	else if (bits & MOB_WEAPON)
	{
		msg.ReadShort();
	}
	unguard;
}

static void CL_ParseSecUpdate(VMessage& msg)
{
	int			bits;
	int			i;

	bits = msg.ReadByte();
	if (bits & SUB_BIG_NUM)
		i = msg.ReadShort();
	else
		i = msg.ReadByte();

	if (bits & SUB_FLOOR)
		GClLevel->Sectors[i].floor.dist = msg.ReadShort();
	if (bits & SUB_CEIL)
		GClLevel->Sectors[i].ceiling.dist = msg.ReadShort();
	if (bits & SUB_LIGHT)
		GClLevel->Sectors[i].params.lightlevel = msg.ReadByte() << 2;
	if (bits & SUB_FLOOR_X)
		GClLevel->Sectors[i].floor.xoffs = msg.ReadByte() & 63;
	if (bits & SUB_FLOOR_Y)
		GClLevel->Sectors[i].floor.yoffs = msg.ReadByte() & 63;
	if (bits & SUB_CEIL_X)
		GClLevel->Sectors[i].ceiling.xoffs = msg.ReadByte() & 63;
	if (bits & SUB_CEIL_Y)
		GClLevel->Sectors[i].ceiling.yoffs = msg.ReadByte() & 63;
	if (bits & (SUB_FLOOR | SUB_CEIL))
		CalcSecMinMaxs(&GClLevel->Sectors[i]);
}

static void CL_ParseViewData(VMessage& msg)
{
	int		i;
	int		bits;

	msg >> cl->ViewOrg.x
		>> cl->ViewOrg.y
		>> cl->ViewOrg.z;
	cl->ExtraLight = msg.ReadByte();
	cl->FixedColourmap = msg.ReadByte();
	cl->Palette = msg.ReadByte();
	cl->ViewEntTranslucency = msg.ReadByte();
	cl->PSpriteSY = msg.ReadShort();

	cl->ViewEnts[0]->SpriteIndex = msg.ReadShort();
	if (cl->ViewEnts[0]->SpriteIndex != -1)
	{
		cl->ViewEnts[0]->SpriteFrame = msg.ReadByte();
		cl->ViewEnts[0]->ModelIndex = msg.ReadShort();
		cl->ViewEnts[0]->ModelFrame = msg.ReadByte();
		cl->ViewEnts[0]->SX = msg.ReadShort();
		cl->ViewEnts[0]->SY = msg.ReadShort();
	}

	cl->ViewEnts[1]->SpriteIndex = msg.ReadShort();
	if (cl->ViewEnts[1]->SpriteIndex != -1)
	{
		cl->ViewEnts[1]->SpriteFrame = msg.ReadByte();
		cl->ViewEnts[1]->ModelIndex = msg.ReadShort();
		cl->ViewEnts[1]->ModelFrame = msg.ReadByte();
		cl->ViewEnts[1]->SX = msg.ReadShort();
		cl->ViewEnts[1]->SY = msg.ReadShort();
	}

	cl->Health = msg.ReadByte();
	msg >> cl->Items;
	cl->Frags = msg.ReadShort();

	bits = msg.ReadByte();
	for (i = 0; i < NUM_CSHIFTS; i++)
	{
		if (bits & (1 << i))
			msg >> cl->CShifts[i];
		else
			cl->CShifts[i] = 0;
	}
}

static void CL_ParseStartSound(VMessage& msg)
{
	vuint16		sound_id;
	vuint16		origin_id;
	float		x = 0.0;
	float		y = 0.0;
	float		z = 0.0;
	vuint8		volume;
	vuint8		attenuation;
	int			channel;

	msg >> sound_id
		>> origin_id;

	channel = origin_id >> 13;
	origin_id &= 0x1fff;

	if (origin_id)
	{
		x = msg.ReadShort();
		y = msg.ReadShort();
		z = msg.ReadShort();
	}
	msg >> volume
		>> attenuation;

	GAudio->PlaySound(sound_id, TVec(x, y, z), TVec(0, 0, 0), origin_id,
		channel, volume / 127.0, attenuation / 64.0);
}

static void CL_ParseStopSound(VMessage& msg)
{
	word	origin_id;
	int		channel;

	msg >> origin_id;

	channel = origin_id >> 13;
	origin_id &= 0x1fff;

	GAudio->StopSound(origin_id, channel);
}

static void CL_ParseStartSeq(VMessage& msg)
{
	int OriginId = msg.ReadShort();
	float x = msg.ReadShort();
	float y = msg.ReadShort();
	float z = msg.ReadShort();
	const char* Name = msg.ReadString();
	int ModeNum = msg.ReadByte();

	GAudio->StartSequence(OriginId, TVec(x, y, z), Name, ModeNum);
}

static void CL_ParseAddSeqChoice(VMessage& msg)
{
	vuint16	origin_id;

	msg >> origin_id;
	VName Choice = msg.ReadString();

	GAudio->AddSeqChoice(origin_id, Choice);
}

static void CL_ParseStopSeq(VMessage& msg)
{
	word	origin_id;

	msg >> origin_id;

	GAudio->StopSequence(origin_id);
}

static void CL_ParseTime(VMessage& msg)
{
	guard(CL_ParseTime);
	float	new_time;
	int		i;

	if (cls.signon == SIGNONS - 1)
	{
		cls.signon = SIGNONS;
		CL_SignonReply();
		GCmdBuf << "HideConsole\n";
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
		if (cl_mobjs[i]->InUse)
		{
			cl_mobjs[i]->InUse--;
		}
	}

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (cl_weapon_mobjs[i]->InUse)
		{
			cl_weapon_mobjs[i]->InUse--;
		}
	}

	R_AnimateSurfaces();
	msg >> new_time;
	cl_level.tictime = int(new_time * 35);
	cl_level.time = new_time;
	cl->WorldTimer = cl_level.tictime;
	unguard;
}

//==========================================================================
//
//	CL_ReadFromServerInfo
//
//==========================================================================

static void CL_ReadFromServerInfo()
{
	VCvar::SetCheating(!!atoi(*Info_ValueForKey(GClGame->serverinfo, "sv_cheats")));
}

//==========================================================================
//
//	CL_AddModel
//
//==========================================================================

static void CL_AddModel(int i, const char *name)
{
	weapon_model_precache[i] = NULL;
	if (FL_FileExists(name))
	{
		model_precache[i] = Mod_FindName(name);
		if (strstr(name, "tris.md2"))
		{
			VStr wpname = VStr(name).ExtractFilePath() + "weapon.md2";
			if (FL_FileExists(wpname))
			{
				weapon_model_precache[i] = Mod_FindName(*wpname);
			}
			else
			{
				GCon->Logf("Can't find wepon info model %s", *wpname);
			}
		}
	}
	else if (VCvar::GetInt("r_models"))
	{
		GCon->Logf("Can't find %s", name);
	}
}

//==========================================================================
//
//	CL_DoLoadLevel
//
//==========================================================================

void CL_SetupLevel();

static void CL_ParseServerInfo(VMessage& msg)
{
	guard(CL_ParseServerInfo);
	byte		ver;

	msg >> ver;
	if (ver != PROTOCOL_VERSION)
		Host_Error("Server runs protocol %d, not %d", ver, PROTOCOL_VERSION);

	CL_Clear();

	GClGame->serverinfo = msg.ReadString();
	CL_ReadFromServerInfo();

	cl_level.MapName = msg.ReadString();
	cl_level.LevelName = msg.ReadString();

	cl->ClientNum = msg.ReadByte();
	GClGame->maxclients = msg.ReadByte();
	GClGame->deathmatch = msg.ReadByte();

	msg >> cl_level.totalkills
		>> cl_level.totalitems
		>> cl_level.totalsecret;
	cl_level.sky1Texture = (word)msg.ReadShort();
	cl_level.sky2Texture = (word)msg.ReadShort();
	msg >> cl_level.sky1ScrollDelta
		>> cl_level.sky2ScrollDelta;
	cl_level.doubleSky = msg.ReadByte();
	cl_level.lightning = msg.ReadByte();
	cl_level.SkyBox = msg.ReadString();
	cl_level.FadeTable = msg.ReadString();

	cl_level.SongLump = msg.ReadString();
	cl_level.cdTrack = msg.ReadByte();

	GCon->Log("---------------------------------------");
	GCon->Log(cl_level.LevelName);
	GCon->Log("");
	C_ClearNotify();

	CL_LoadLevel(cl_level.MapName);

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
	GAudio->Start();

	SB_Start();

	for (int i = 0; i < VClass::GSpriteNames.Num(); i++)
	{
		R_InstallSprite(*VClass::GSpriteNames[i], i);
	}

	for (int i = 1; i < VClass::GModelNames.Num(); i++)
	{
		CL_AddModel(i, va("models/%s", *VClass::GModelNames[i]));
	}

	for (int i = 0; i < VMemberBase::GMembers.Num(); i++)
	{
		if (VMemberBase::GMembers[i]->MemberType == MEMBER_Class)
		{
			VClass* C = static_cast<VClass*>(VMemberBase::GMembers[i]);
			if (C->IsChildOf(VThinker::StaticClass()))
			{
				ClassLookup[C->NetId] = C;
			}
		}
	}

	GCon->Log(NAME_Dev, "Client level loaded");
	unguard;
}

//==========================================================================
//
//	CL_ParseIntermission
//
//==========================================================================

static void CL_ParseIntermission(VMessage& msg)
{
	VName nextmap = msg.ReadString();

	im.Text.Clean();
	im.IMFlags = 0;

	const mapInfo_t& linfo = P_GetMapInfo(cl_level.MapName);
	im.LeaveMap = cl_level.MapName;
	im.LeaveCluster = linfo.Cluster;
	im.LeaveName = linfo.GetName();
	if (linfo.Flags & MAPINFOF_NoIntermission)
		im.IMFlags |= im_t::IMF_NoIntermission;

	const mapInfo_t& einfo = P_GetMapInfo(nextmap);
	im.EnterMap = nextmap;
	im.EnterCluster = einfo.Cluster;
	im.EnterName = einfo.GetName();

	if (linfo.Cluster != einfo.Cluster)
	{
		if (einfo.Cluster)
		{
			const VClusterDef* CDef = P_GetClusterDef(einfo.Cluster);
			if (CDef->EnterText.Length())
			{
				if (CDef->Flags & CLUSTERF_LookupEnterText)
				{
					im.Text = GLanguage[*CDef->EnterText];
				}
				else
				{
					im.Text = CDef->EnterText;
				}
				if (CDef->Flags & CLUSTERF_EnterTextIsLump)
				{
					im.IMFlags |= im_t::IMF_TextIsLump;
				}
				if (CDef->Flags & CLUSTERF_FinalePic)
				{
					im.TextFlat = NAME_None;
					im.TextPic = CDef->Flat;
				}
				else
				{
					im.TextFlat = CDef->Flat;
					im.TextPic = NAME_None;
				}
				im.TextMusic = CDef->Music;
				im.TextCDTrack = CDef->CDTrack;
				im.TextCDId = CDef->CDId;
			}
		}
		if (im.Text.Length() == 0 && linfo.Cluster)
		{
			const VClusterDef* CDef = P_GetClusterDef(linfo.Cluster);
			if (CDef->ExitText.Length())
			{
				if (CDef->Flags & CLUSTERF_LookupExitText)
				{
					im.Text = GLanguage[*CDef->ExitText];
				}
				else
				{
					im.Text = CDef->ExitText;
				}
				if (CDef->Flags & CLUSTERF_ExitTextIsLump)
				{
					im.IMFlags |= im_t::IMF_TextIsLump;
				}
				if (CDef->Flags & CLUSTERF_FinalePic)
				{
					im.TextFlat = NAME_None;
					im.TextPic = CDef->Flat;
				}
				else
				{
					im.TextFlat = CDef->Flat;
					im.TextPic = NAME_None;
				}
				im.TextMusic = CDef->Music;
				im.TextCDTrack = CDef->CDTrack;
				im.TextCDId = CDef->CDId;
			}
		}
	}

	im.TotalKills = cl_level.totalkills;
	im.TotalItems = cl_level.totalitems;
	im.TotalSecret = cl_level.totalsecret;
	im.Time = cl_level.time;
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (msg.ReadByte())
			scores[i].Flags |= scores_t::SF_Active;
		else
			scores[i].Flags &= ~scores_t::SF_Active;
		for (int j = 0; j < MAXPLAYERS; j++)
			scores[i].frags[j] = (char)msg.ReadByte();
		scores[i].killcount = msg.ReadShort();
		scores[i].itemcount = msg.ReadShort();
		scores[i].secretcount = msg.ReadShort();
	}
	IM_Start();
}

//==========================================================================
//
//	CL_ParseClassName
//
//==========================================================================

static void CL_ParseClassName(VMessage& msg)
{
	vint32 i = msg.ReadShort();
	const char* Name = msg.ReadString();
	ClassLookup[i] = VClass::FindClass(Name);
}

//==========================================================================
//
//	CL_ParseSpriteList
//
//==========================================================================

static void CL_ParseSpriteList(VMessage& msg)
{
	int count = msg.ReadShort();
	for (int i = 0; i < count; i++)
	{
		R_InstallSprite(msg.ReadString(), i);
	}
}

//==========================================================================
//
//	CL_ParseModel
//
//==========================================================================

static void CL_ParseModel(VMessage& msg)
{
	int i = msg.ReadShort();
	char *name = va("models/%s", msg.ReadString());
	CL_AddModel(i, name);
}

//==========================================================================
//
//	CL_ParseSkin
//
//==========================================================================

static void CL_ParseSkin(VMessage& msg)
{
	int i = msg.ReadByte();
	skin_list[i] = VStr("models/") + msg.ReadString();
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

static void CL_ParseLineTransuc(VMessage& msg)
{
	int i = msg.ReadShort();
	int fuzz = msg.ReadByte();
	GClLevel->Lines[i].translucency = fuzz;
}

//==========================================================================
//
//	CL_ParseExtraFloor
//
//==========================================================================

static void CL_ParseExtraFloor(VMessage& msg)
{
	int i = msg.ReadShort();
	int j = msg.ReadShort();
	AddExtraFloor(&GClLevel->Lines[i], &GClLevel->Sectors[j]);
}

//==========================================================================
//
//	CL_ParseHeightSec
//
//==========================================================================

static void CL_ParseHeightSec(VMessage& msg)
{
	sector_t* ToSec = &GClLevel->Sectors[(word)msg.ReadShort()];
	sector_t* HeightSec = &GClLevel->Sectors[(word)msg.ReadShort()];
	int flags = msg.ReadByte();

	ToSec->heightsec = HeightSec;
	if (flags & 2)
	{
		HeightSec->SectorFlags |= sector_t::SF_FakeFloorOnly;
	}
	if (flags & 4)
	{
		HeightSec->SectorFlags |= sector_t::SF_ClipFakePlanes;
	}
	if (flags & 8)
	{
		HeightSec->SectorFlags |= sector_t::SF_UnderWater;
	}
	if (flags & 16)
	{
		HeightSec->SectorFlags |= sector_t::SF_IgnoreHeightSec;
	}
	if (flags & 32)
	{
		HeightSec->SectorFlags |= sector_t::SF_NoFakeLight;
	}
	R_SetupFakeFloors(ToSec);
}

//==========================================================================
//
//	CL_PO_Update
//
//==========================================================================

void CL_PO_Update(int i, float x, float y, float angle)
{
	guard(CL_PO_Update);
	if (!GClLevel->NumPolyObjs)
		return;

	if (GClLevel->PolyObjs[i].angle != angle)
	{
		GClLevel->RotatePolyobj(GClLevel->PolyObjs[i].tag,
			angle - GClLevel->PolyObjs[i].angle);
	}

	if (x != GClLevel->PolyObjs[i].startSpot.x ||
		y != GClLevel->PolyObjs[i].startSpot.y)
	{
		GClLevel->MovePolyobj(GClLevel->PolyObjs[i].tag,
			x - GClLevel->PolyObjs[i].startSpot.x,
			y - GClLevel->PolyObjs[i].startSpot.y);
	}
	unguard;
}

//==========================================================================
//
//	CL_ParseServerMessage
//
//==========================================================================

void CL_ParseServerMessage(VMessage& msg)
{
	guard(CL_ParseServerMessage);
	int			i;
	byte		cmd_type;
	float		x;
	float		y;
	int			tag;
	float		angle;
	VStr		name;
	VStr		string;
	TVec		origin;
	float		radius;
	vuint32		colour;
	int			trans;
	sector_t*	sec;

	msg.BeginReading();

	// update command store from the packet
	while (1)
	{
		if (msg.BadRead)
		{
			GCon->Logf(NAME_Dev, "Length %d", msg.CurSize);
			for (i = 0; i < msg.CurSize; i++)
				GCon->Logf(NAME_Dev, "  %d", (int)msg.Data[i]);
			Host_Error("Packet corupted");
		}

		msg >> cmd_type;

		if (msg.BadRead)
			break; // Here this means end of packet

		switch (cmd_type)
		{
		case svc_nop:
			break;

		case svc_disconnect:
			Host_EndGame("Server disconnected");

		case svc_server_info:
			CL_ParseServerInfo(msg);
			break;

		case svc_spawn_baseline:
			CL_ParseBaseline(msg);
			break;

		case svc_update_mobj:
			CL_ParseUpdateMobj(msg);
			break;

		case svc_side_top:
			i = msg.ReadShort();
			GClLevel->Sides[i].toptexture = msg.ReadShort();
			break;

		case svc_side_mid:
			i = msg.ReadShort();
			GClLevel->Sides[i].midtexture = msg.ReadShort();
			break;

		case svc_side_bot:
			i = msg.ReadShort();
			GClLevel->Sides[i].bottomtexture = msg.ReadShort();
			break;

		case svc_side_ofs:
			i = msg.ReadShort();
			GClLevel->Sides[i].textureoffset = msg.ReadShort();
			GClLevel->Sides[i].rowoffset = msg.ReadShort();
			break;

		case svc_sec_floor:
			i = msg.ReadShort();
			GClLevel->Sectors[i].floor.pic = (word)msg.ReadShort();
			break;

		case svc_sec_ceil:
			i = msg.ReadShort();
			GClLevel->Sectors[i].ceiling.pic = (word)msg.ReadShort();
			break;

		case svc_sec_update:
			CL_ParseSecUpdate(msg);
			break;

		case svc_set_angles:
			cl->ViewAngles.pitch = AngleMod180(ByteToAngle(msg.ReadByte()));
			cl->ViewAngles.yaw = ByteToAngle(msg.ReadByte());
			cl->ViewAngles.roll = ByteToAngle(msg.ReadByte());
			break;

		case svc_centre_look:
//FIXME
			break;

		case svc_view_data:
			CL_ParseViewData(msg);
			break;

		case svc_start_sound:
			CL_ParseStartSound(msg);
			break;

		case svc_stop_sound:
			CL_ParseStopSound(msg);
			break;

		case svc_start_seq:
			CL_ParseStartSeq(msg);
			break;

		case svc_add_seq_choice:
			CL_ParseAddSeqChoice(msg);
			break;

		case svc_stop_seq:
			CL_ParseStopSeq(msg);
			break;

		case svc_print:
			C_NotifyMessage(msg.ReadString());
			break;

		case svc_centre_print:
			C_CentreMessage(msg.ReadString());
			break;

		case svc_time:
			CL_ParseTime(msg);
			break;

		case svc_poly_spawn:
			x = msg.ReadShort();
			y = msg.ReadShort();
			tag = msg.ReadByte();
			GClLevel->SpawnPolyobj(x, y, tag, 0);
			break;

		case svc_poly_translate:
			x = msg.ReadShort();
			y = msg.ReadShort();
			tag = msg.ReadByte();
			GClLevel->AddPolyAnchorPoint(x, y, tag);
			break;

		case svc_poly_update:
			i = msg.ReadByte();
			x = msg.ReadShort();
			y = msg.ReadShort();
			angle = ByteToAngle(msg.ReadByte());
			CL_PO_Update(i, x, y, angle);
			break;

		case svc_force_lightning:
			R_ForceLightning();
			break;

		case svc_intermission:
			CL_ParseIntermission(msg);
			break;

		case svc_pause:
			if (msg.ReadByte())
			{
				GClGame->ClientFlags |= VClientGameBase::CF_Paused;
				GAudio->PauseSound();
			}
			else
			{
				GClGame->ClientFlags &= ~VClientGameBase::CF_Paused;
				GAudio->ResumeSound();
			}
			break;

		case svc_stats_long:
			i = msg.ReadByte();
			check(i < (int)(cl->GetClass()->ClassSize - sizeof(VBasePlayer)) / 4);
			msg >> ((int*)((byte*)cl + sizeof(VBasePlayer)))[i];
			break;

		case svc_stats_short:
			i = msg.ReadByte();
			check(i < (int)(cl->GetClass()->ClassSize - sizeof(VBasePlayer)) / 4);
			((int*)((byte*)cl + sizeof(VBasePlayer)))[i] = msg.ReadShort();
			break;

		case svc_stats_byte:
			i = msg.ReadByte();
			check(i < (int)(cl->GetClass()->ClassSize - sizeof(VBasePlayer)) / 4);
			((int*)((byte*)cl + sizeof(VBasePlayer)))[i] = msg.ReadByte();
			break;

		case svc_stats_string:
			i = msg.ReadByte();
			check(i < (int)(cl->GetClass()->ClassSize - sizeof(VBasePlayer)) / 4);
			*(VStr*)((byte*)cl + sizeof(VBasePlayer) + i * 4) = msg.ReadString();
			break;

		case svc_stringcmd:
			GCmdBuf << msg.ReadString();
			break;

		case svc_signonnum:
			i = msg.ReadByte();
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
			i = msg.ReadShort();
			msg >> GClLevel->Sectors[i].floor.normal.x
				>> GClLevel->Sectors[i].floor.normal.y
				>> GClLevel->Sectors[i].floor.normal.z
				>> GClLevel->Sectors[i].floor.dist;
			GClLevel->Sectors[i].base_floorheight = GClLevel->Sectors[i].floor.dist;
			CalcSecMinMaxs(&GClLevel->Sectors[i]);
			break;

		case svc_sec_ceil_plane:
			i = msg.ReadShort();
			msg >> GClLevel->Sectors[i].ceiling.normal.x
				>> GClLevel->Sectors[i].ceiling.normal.y
				>> GClLevel->Sectors[i].ceiling.normal.z
				>> GClLevel->Sectors[i].ceiling.dist;
			GClLevel->Sectors[i].base_ceilingheight  = GClLevel->Sectors[i].ceiling.dist;
			CalcSecMinMaxs(&GClLevel->Sectors[i]);
			break;

		case svc_serverinfo:
			name = msg.ReadString();
			string = msg.ReadString();
			Info_SetValueForKey(GClGame->serverinfo, name, string);
			CL_ReadFromServerInfo();
			break;

		case svc_userinfo:
			i = msg.ReadByte();
			scores[i].userinfo = msg.ReadString();
			CL_ReadFromUserInfo(i);
			break;

		case svc_setinfo:
			i = msg.ReadByte();
			name = msg.ReadString();
			string = msg.ReadString();
			Info_SetValueForKey(scores[i].userinfo, name, string);
			CL_ReadFromUserInfo(i);
			break;

		case svc_sprites:
			CL_ParseSpriteList(msg);
			break;

		case svc_model:
			CL_ParseModel(msg);
			break;

		case svc_skin:
			CL_ParseSkin(msg);
			break;

		case svc_line_transluc:
			CL_ParseLineTransuc(msg);
			break;

		case svc_sec_transluc:
			i = msg.ReadShort();
			trans = msg.ReadByte();
			sec = &GClLevel->Sectors[i];
			sec->floor.translucency = trans;
			sec->ceiling.translucency = trans;
			for (i = 0; i < sec->linecount; i++)
			{
				sec->lines[i]->translucency = trans;
			}
			break;

		case svc_extra_floor:
			CL_ParseExtraFloor(msg);
			break;

		case svc_swap_planes:
			i = msg.ReadShort();
			SwapPlanes(&GClLevel->Sectors[i]);
			break;

		case svc_static_light:
			origin.x = msg.ReadShort();
			origin.y = msg.ReadShort();
			origin.z = msg.ReadShort();
			radius = (byte)msg.ReadByte() * 8;
			R_AddStaticLight(origin, radius, 0xffffffff);
			break;

		case svc_static_light_rgb:
			origin.x = msg.ReadShort();
			origin.y = msg.ReadShort();
			origin.z = msg.ReadShort();
			radius = (byte)msg.ReadByte() * 8;
			msg >> colour;
			R_AddStaticLight(origin, radius, colour);
			break;

		case svc_sec_light_colour:
			sec = &GClLevel->Sectors[msg.ReadShort()];
			sec->params.LightColour = (msg.ReadByte() << 16) |
				(msg.ReadByte() << 8) | msg.ReadByte();
			break;

		case svc_change_sky:
			cl_level.sky1Texture = (word)msg.ReadShort();
			cl_level.sky2Texture = (word)msg.ReadShort();
			R_SkyChanged();
			break;

		case svc_change_music:
			cl_level.SongLump = msg.ReadString();
			cl_level.cdTrack = msg.ReadByte();
			GAudio->MusicChanged();
			break;

		case svc_set_floor_light_sec:
			i = (word)msg.ReadShort();
			GClLevel->Sectors[i].floor.LightSourceSector = msg.ReadShort();
			break;

		case svc_set_ceil_light_sec:
			i = (word)msg.ReadShort();
			GClLevel->Sectors[i].ceiling.LightSourceSector = msg.ReadShort();
			break;

		case svc_set_heightsec:
			CL_ParseHeightSec(msg);
			break;

		case svc_class_name:
			CL_ParseClassName(msg);
			break;

		default:
			if (GClGame->eventParseServerCommand(cmd_type))
			{
				break;
			}
			GCon->Logf(NAME_Dev, "Length %d", msg.CurSize);
			for (i = 0; i < msg.CurSize; i++)
			{
				GCon->Logf(NAME_Dev, "  %d", (int)msg.Data[i]);
			}
			GCon->Logf(NAME_Dev, "ReadCount %d", msg.ReadCount);
			Host_Error("Invalid packet %d", cmd_type);
			break;
		}
	}
	unguard;
}
