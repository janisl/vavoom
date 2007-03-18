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
VModel*			model_precache[1024];
VStr			skin_list[256];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static VClass*			ClassLookup[MAX_CLASS_LOOKUP];

// CODE --------------------------------------------------------------------

void CL_Clear()
{
	guard(CL_Clear);
	GClGame->serverinfo.Clean();
	GClGame->intermission = 0;
	GClGame->time = 0;
	VEntity* PrevVEnt = cl->ViewEnt;
	memset((byte*)cl + sizeof(VObject), 0, cl->GetClass()->ClassSize - sizeof(VObject));
	cl->ViewEnt = PrevVEnt;
	cl_level.LevelName.Clean();
	memset(&cl_level, 0, sizeof(cl_level));
	for (int i = 0; i < GMaxEntities; i++)
		if (cl_mobjs[i])
			cl_mobjs[i]->ConditionalDestroy();
	memset(cl_mobjs, 0, sizeof(VEntity*) * GMaxEntities);
	memset(cl_mo_base, 0, sizeof(clmobjbase_t) * GMaxEntities);
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
	{
		cl_mobjs[i] = Spawn<VEntity>();
		cl_mobjs[i]->Role = ROLE_Authority;
		cl_mobjs[i]->RemoteRole = ROLE_None;
	}
	cls.signon = 0;
	unguard;
}

static void CL_ReadMobjBase(VMessageIn& msg, clmobjbase_t &mobj)
{
	guard(CL_ReadMobjBase);
	vuint16 CIdx = (vuint16)msg.ReadShort();
	mobj.Class = ClassLookup[CIdx];
	check(mobj.Class);
	mobj.State = mobj.Class->StatesLookup[(vuint16)msg.ReadShort()];
	mobj.origin.x = msg.ReadShort();
	mobj.origin.y = msg.ReadShort();
	mobj.origin.z = msg.ReadShort();
	mobj.angles.yaw = ByteToAngle(msg.ReadByte());
	mobj.angles.pitch = ByteToAngle(msg.ReadByte());
	mobj.angles.roll = ByteToAngle(msg.ReadByte());
	unguard;
}

static void CL_ParseBaseline(VMessageIn& msg)
{
	guard(CL_ParseBaseline);
	int		i;

	i = msg.ReadShort();

	CL_ReadMobjBase(msg, cl_mo_base[i]);
	unguard;
}

static void CL_ReadMobj(VMessageIn& msg, VEntity*& mobj, const clmobjbase_t &base)
{
	VClass* C;
	if (msg.ReadBit())
	{
		if (msg.ReadBit())
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
		mobj = (VEntity*)GClLevel->SpawnThinker(C);
		mobj->Role = ROLE_DumbProxy;
		mobj->RemoteRole = ROLE_Authority;
	}
	if (msg.ReadBit())
	{
		if (msg.ReadBit())
			mobj->State = mobj->GetClass()->StatesLookup[(vuint16)msg.ReadShort()];
		else
			mobj->State = mobj->GetClass()->StatesLookup[msg.ReadByte()];
	}
	else
	{
		mobj->State = base.State;
	}
	if (msg.ReadBit())
	{
		vuint8 TimeFrac = msg.ReadByte();
		mobj->StateTime = mobj->State->Time * TimeFrac / 255.0;
	}
	else
	{
		mobj->StateTime = -1;
	}
	if (msg.ReadBit())
	{
		if (msg.ReadBit())
			mobj->Origin.x = msg.ReadShort();
		else
			mobj->Origin.x = base.origin.x;
		if (msg.ReadBit())
			mobj->Origin.y = msg.ReadShort();
		else
			mobj->Origin.y = base.origin.y;
		if (msg.ReadBit())
			mobj->Origin.z = msg.ReadShort();
		else
			mobj->Origin.z = base.origin.z;
	}
	else
	{
		mobj->Origin = base.origin;
	}
	if (msg.ReadBit())
	{
		if (msg.ReadBit())
			mobj->Angles.yaw = ByteToAngle(msg.ReadByte());
		else
			mobj->Angles.yaw = base.angles.yaw;
		if (msg.ReadBit())
			mobj->Angles.pitch = ByteToAngle(msg.ReadByte());
		else
			mobj->Angles.pitch = base.angles.pitch;
		if (msg.ReadBit())
			mobj->Angles.roll = ByteToAngle(msg.ReadByte());
		else
			mobj->Angles.roll = base.angles.roll;
	}
	else
	{
		mobj->Angles = base.angles;
	}
}

static void CL_ParseUpdateMobj(VMessageIn& msg)
{
	guard(CL_ParseUpdateMobj);
	int		i;

	if (msg.ReadBit())
		i = msg.ReadShort();
	else
		i = msg.ReadByte();

	CL_ReadMobj(msg, cl_mobjs[i], cl_mo_base[i]);

	//	Marking mobj in use
	cl_mobjs[i]->InUse = 2;
	unguard;
}

static void CL_ParseSecUpdate(VMessageIn& msg)
{
	int			bits;
	int			i;

	if (msg.ReadBit())
		i = msg.ReadShort();
	else
		i = msg.ReadByte();

	float PrevFloorDist = GClLevel->Sectors[i].floor.dist;
	float PrevCeilDist = GClLevel->Sectors[i].ceiling.dist;

	if (msg.ReadBit())
	{
		if (msg.ReadBit())
			GClLevel->Sectors[i].floor.dist = msg.ReadShort();
		if (msg.ReadBit())
			GClLevel->Sectors[i].floor.xoffs = msg.ReadByte() & 63;
		if (msg.ReadBit())
			GClLevel->Sectors[i].floor.yoffs = msg.ReadByte() & 63;
	}
	if (msg.ReadBit())
	{
		if (msg.ReadBit())
			GClLevel->Sectors[i].ceiling.dist = msg.ReadShort();
		if (msg.ReadBit())
			GClLevel->Sectors[i].ceiling.xoffs = msg.ReadByte() & 63;
		if (msg.ReadBit())
			GClLevel->Sectors[i].ceiling.yoffs = msg.ReadByte() & 63;
	}
	if (msg.ReadBit())
	{
		GClLevel->Sectors[i].params.lightlevel = msg.ReadByte() << 2;
	}
	if (PrevFloorDist != GClLevel->Sectors[i].floor.dist ||
		PrevCeilDist != GClLevel->Sectors[i].ceiling.dist)
	{
		CalcSecMinMaxs(&GClLevel->Sectors[i]);
	}
}

static void CL_ParseViewData(VMessageIn& msg)
{
	guard(CL_ParseViewData);
	int		i;

	msg << cl->ViewOrg.x
		<< cl->ViewOrg.y
		<< cl->ViewOrg.z;
	cl->ViewEntAlpha = (float)msg.ReadByte() / 255.0;

	for (i = 0; i < NUMPSPRITES; i++)
	{
		int ClsIdx = msg.ReadShort();
		if (ClsIdx != -1)
		{
			cl->ViewStates[i].State =
				ClassLookup[ClsIdx]->StatesLookup[msg.ReadShort()];
			vuint8 TimeFrac = msg.ReadByte();
			if (TimeFrac == 255)
				cl->ViewStates[i].StateTime = -1;
			else
				cl->ViewStates[i].StateTime = cl->ViewStates[i].State->Time *
					TimeFrac / 254.0;
			cl->ViewStates[i].SX = msg.ReadShort();
			cl->ViewStates[i].SY = msg.ReadShort();
		}
		else
		{
			cl->ViewStates[i].State = NULL;
		}
	}
	unguard;
}

static void CL_ParseStartSound(VMessageIn& msg)
{
	vuint16		sound_id;
	vuint16		origin_id;
	float		x = 0.0;
	float		y = 0.0;
	float		z = 0.0;
	vuint8		volume;
	vuint8		attenuation;
	int			channel;

	msg << sound_id
		<< origin_id;

	channel = origin_id >> 13;
	origin_id &= 0x1fff;

	if (origin_id)
	{
		x = msg.ReadShort();
		y = msg.ReadShort();
		z = msg.ReadShort();
	}
	msg << volume
		<< attenuation;

	GAudio->PlaySound(sound_id, TVec(x, y, z), TVec(0, 0, 0), origin_id,
		channel, volume / 127.0, attenuation / 64.0);
}

static void CL_ParseStopSound(VMessageIn& msg)
{
	word	origin_id;
	int		channel;

	msg << origin_id;

	channel = origin_id >> 13;
	origin_id &= 0x1fff;

	GAudio->StopSound(origin_id, channel);
}

static void CL_ParseStartSeq(VMessageIn& msg)
{
	int OriginId = msg.ReadShort();
	float x = msg.ReadShort();
	float y = msg.ReadShort();
	float z = msg.ReadShort();
	VStr Name = msg.ReadString();
	int ModeNum = msg.ReadByte();

	GAudio->StartSequence(OriginId, TVec(x, y, z), *Name, ModeNum);
}

static void CL_ParseAddSeqChoice(VMessageIn& msg)
{
	vuint16	origin_id;

	msg << origin_id;
	VName Choice = *msg.ReadString();

	GAudio->AddSeqChoice(origin_id, Choice);
}

static void CL_ParseStopSeq(VMessageIn& msg)
{
	word	origin_id;

	msg << origin_id;

	GAudio->StopSequence(origin_id);
}

static void CL_ParseTime(VMessageIn& msg)
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

	R_AnimateSurfaces();
	msg << new_time;
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
	if (FL_FileExists(name))
	{
		model_precache[i] = Mod_FindName(name);
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

static void CL_ParseServerInfo(VMessageIn& msg)
{
	guard(CL_ParseServerInfo);
	byte		ver;

	msg << ver;
	if (ver != PROTOCOL_VERSION)
		Host_Error("Server runs protocol %d, not %d", ver, PROTOCOL_VERSION);

	CL_Clear();

	msg << GClGame->serverinfo;
	CL_ReadFromServerInfo();

	cl_level.MapName = *msg.ReadString();
	cl_level.LevelName = msg.ReadString();

	cl->ClientNum = msg.ReadByte();
	GClGame->maxclients = msg.ReadByte();
	GClGame->deathmatch = msg.ReadByte();

	msg << cl_level.totalkills
		<< cl_level.totalitems
		<< cl_level.totalsecret;
	cl_level.sky1Texture = (word)msg.ReadShort();
	cl_level.sky2Texture = (word)msg.ReadShort();
	msg << cl_level.sky1ScrollDelta
		<< cl_level.sky2ScrollDelta;
	cl_level.doubleSky = msg.ReadByte();
	cl_level.lightning = msg.ReadByte();
	cl_level.SkyBox = *msg.ReadString();
	cl_level.FadeTable = *msg.ReadString();

	cl_level.SongLump = *msg.ReadString();
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

	R_Start(GClLevel);
	GAudio->Start();

	SB_Start();

	for (int i = 0; i < VClass::GSpriteNames.Num(); i++)
	{
		R_InstallSprite(*VClass::GSpriteNames[i], i);
	}

	VMemberBase::SetUpNetClasses();
	for (int i = 0; i < VMemberBase::GMembers.Num(); i++)
	{
		if (VMemberBase::GMembers[i]->MemberType == MEMBER_Class)
		{
			VClass* C = static_cast<VClass*>(VMemberBase::GMembers[i]);
			if (C->IsChildOf(VThinker::StaticClass()))
			{
				check(C->NetId >= 0);
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

static void CL_ParseIntermission(VMessageIn& msg)
{
	VName nextmap = *msg.ReadString();

	im.Text.Clean();
	im.IMFlags = 0;

	const mapInfo_t& linfo = P_GetMapInfo(cl_level.MapName);
	im.LeaveMap = cl_level.MapName;
	im.LeaveCluster = linfo.Cluster;
	im.LeaveName = linfo.GetName();
	im.LeaveTitlePatch = linfo.TitlePatch;
	im.ParTime = linfo.ParTime;
	im.SuckTime = linfo.SuckTime;
	if (linfo.Flags & MAPINFOF_NoIntermission)
		im.IMFlags |= im_t::IMF_NoIntermission;

	const mapInfo_t& einfo = P_GetMapInfo(nextmap);
	im.EnterMap = nextmap;
	im.EnterCluster = einfo.Cluster;
	im.EnterName = einfo.GetName();
	im.EnterTitlePatch = einfo.TitlePatch;

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

static void CL_ParseClassName(VMessageIn& msg)
{
	vint32 i = msg.ReadShort();
	VStr Name = msg.ReadString();
	ClassLookup[i] = VClass::FindClass(*Name);
}

//==========================================================================
//
//	CL_ParseModel
//
//==========================================================================

static void CL_ParseModel(VMessageIn& msg)
{
	int i = msg.ReadShort();
	VStr name = VStr("models/") + msg.ReadString();
	CL_AddModel(i, *name);
}

//==========================================================================
//
//	CL_ParseSkin
//
//==========================================================================

static void CL_ParseSkin(VMessageIn& msg)
{
	int i = msg.ReadByte();
	msg << skin_list[i];
}

//==========================================================================
//
//	CL_ParseSetProp
//
//==========================================================================

static void CL_ParseSetProp(VMessageIn& Msg)
{
	guard(CL_ParseSetProp);
	int Id = Msg.ReadByte();
	if (Id & 0x80)
	{
		Id = (Id & 0x7f) | (Msg.ReadByte() << 7);
	}
	VEntity* Ent = cl_mobjs[Id];
	int FldIdx = Msg.ReadByte();
	VField* F = NULL;
	for (VField* CF = Ent->GetClass()->NetFields; CF; CF = CF->NextNetField)
	{
		if (CF->NetIndex == FldIdx)
		{
			F = CF;
			break;
		}
	}
	if (!F)
	{
		Sys_Error("Bad net field %d", FldIdx);
	}
	VField::NetSerialiseValue(Msg, (vuint8*)Ent + F->Ofs, F->Type);
	unguard;
}

//==========================================================================
//
//	CL_ParseNewObj
//
//==========================================================================

static void CL_ParseNewObj(VMessageIn& msg)
{
	guard(CL_ParseNewObj);
	int i = msg.ReadByte();
	if (i & 0x80)
		i = (i & 0x7f) | (msg.ReadByte() << 7);

	int ci = msg.ReadByte();
	if (ci & 0x80)
		ci = (ci & 0x7f) | (msg.ReadByte() << 7);
	VClass* C = ClassLookup[ci];

	if (cl_mobjs[i]->GetClass() != VEntity::StaticClass())
		GClLevel->RemoveThinker(cl_mobjs[i]);
	cl_mobjs[i]->ConditionalDestroy();
	cl_mobjs[i] = (VEntity*)GClLevel->SpawnThinker(C);
	cl_mobjs[i]->Role = ROLE_DumbProxy;
	cl_mobjs[i]->RemoteRole = ROLE_Authority;
	unguard;
}

//==========================================================================
//
//	CL_ParseSetPlayerProp
//
//==========================================================================

static void CL_ParseSetPlayerProp(VMessageIn& Msg)
{
	guard(CL_ParseSetPlayerProp);
	int FldIdx = Msg.ReadByte();
	VField* F = NULL;
	for (VField* CF = cl->GetClass()->NetFields; CF; CF = CF->NextNetField)
	{
		if (CF->NetIndex == FldIdx)
		{
			F = CF;
			break;
		}
	}
	if (!F)
	{
		Sys_Error("Bad net field %d", FldIdx);
	}
	VField::NetSerialiseValue(Msg, (vuint8*)cl + F->Ofs, F->Type);
	unguard;
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

void CL_ParseServerMessage(VMessageIn& msg)
{
	guard(CL_ParseServerMessage);
	int			i;
	byte		cmd_type;
	float		x;
	float		y;
	float		angle;
	VStr		name;
	VStr		string;
	TVec		origin;
	float		radius;
	vuint32		colour;

	// update command store from the packet
	while (1)
	{
		if (msg.IsError())
		{
			GCon->Logf(NAME_Dev, "Length %d", msg.GetNumBits());
			for (i = 0; i < msg.GetNumBytes(); i++)
				GCon->Logf(NAME_Dev, "  %d", (int)msg.GetData()[i]);
			Host_Error("Packet corupted");
		}

		msg << cmd_type;

		if (msg.IsError())
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
			C_NotifyMessage(*msg.ReadString());
			break;

		case svc_centre_print:
			C_CentreMessage(*msg.ReadString());
			break;

		case svc_time:
			CL_ParseTime(msg);
			break;

		case svc_poly_update:
			i = msg.ReadByte();
			x = msg.ReadShort();
			y = msg.ReadShort();
			angle = ByteToAngle(msg.ReadByte());
			CL_PO_Update(i, x, y, angle);
			break;

		case svc_force_lightning:
			GClLevel->RenderData->ForceLightning();
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
			F_StartFinale(*msg.ReadString());
			break;

		case svc_serverinfo:
			msg << name
				<< string;
			Info_SetValueForKey(GClGame->serverinfo, name, string);
			CL_ReadFromServerInfo();
			break;

		case svc_userinfo:
			i = msg.ReadByte();
			msg << scores[i].userinfo;
			CL_ReadFromUserInfo(i);
			break;

		case svc_setinfo:
			i = msg.ReadByte();
			msg << name
				<< string;
			Info_SetValueForKey(scores[i].userinfo, name, string);
			CL_ReadFromUserInfo(i);
			break;

		case svc_model:
			CL_ParseModel(msg);
			break;

		case svc_skin:
			CL_ParseSkin(msg);
			break;

		case svc_static_light:
			origin.x = msg.ReadShort();
			origin.y = msg.ReadShort();
			origin.z = msg.ReadShort();
			radius = (byte)msg.ReadByte() * 8;
			GClLevel->RenderData->AddStaticLight(origin, radius, 0xffffffff);
			break;

		case svc_static_light_rgb:
			origin.x = msg.ReadShort();
			origin.y = msg.ReadShort();
			origin.z = msg.ReadShort();
			radius = (byte)msg.ReadByte() * 8;
			msg << colour;
			GClLevel->RenderData->AddStaticLight(origin, radius, colour);
			break;

		case svc_change_sky:
			cl_level.sky1Texture = (word)msg.ReadShort();
			cl_level.sky2Texture = (word)msg.ReadShort();
			GClLevel->RenderData->SkyChanged();
			break;

		case svc_change_music:
			cl_level.SongLump = *msg.ReadString();
			cl_level.cdTrack = msg.ReadByte();
			GAudio->MusicChanged();
			break;

		case svc_class_name:
			CL_ParseClassName(msg);
			break;

		case svc_set_prop:
			CL_ParseSetProp(msg);
			break;

		case svc_new_obj:
			CL_ParseNewObj(msg);
			break;

		case svc_set_player_prop:
			CL_ParseSetPlayerProp(msg);
			break;

		default:
			if (GClGame->eventParseServerCommand(cmd_type, &msg))
			{
				break;
			}
			GCon->Logf(NAME_Dev, "Length %d", msg.GetNumBits());
			for (i = 0; i < msg.GetNumBytes(); i++)
			{
				GCon->Logf(NAME_Dev, "  %d", (int)msg.GetData()[i]);
			}
			GCon->Logf(NAME_Dev, "ReadCount %d", msg.GetPosBits());
			Host_Error("Invalid packet %d", cmd_type);
			break;
		}
	}
	unguard;
}
