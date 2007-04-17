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
#include "network.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class VClientGenChannel : public VChannel
{
public:
	VClientGenChannel(VNetConnection* AConnection, vint32 AIndex, vuint8 AOpenedLocally = true)
	: VChannel(AConnection, CHANNEL_General, AIndex, AOpenedLocally)
	{}

	//	VChannel interface
	void ParsePacket(VMessageIn&);
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void CL_ClearInput();
void CL_PO_Update(int i, float x, float y, float angle);
void CL_SignonReply();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

VModel*			model_precache[1024];
VStr			skin_list[256];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

void CL_Clear()
{
	guard(CL_Clear);
	GClGame->serverinfo.Clean();
	GClGame->intermission = 0;
	GClGame->time = 0;
	VNetConnection* Net = cl->Net;
	VEntity* PrevVEnt = cl->ViewEnt;
	memset((byte*)cl + sizeof(VObject), 0, cl->GetClass()->ClassSize - sizeof(VObject));
	cl->PlayerFlags |= VBasePlayer::PF_IsClient;
	cl->ViewEnt = PrevVEnt;
	cl->Net = Net;
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
	cls.signon = 0;
	unguard;
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
				VMemberBase::GNetClassLookup[ClsIdx]->StatesLookup[msg.ReadShort()];
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

	if (cls.signon == SIGNONS - 1)
	{
		cls.signon = SIGNONS;
		CL_SignonReply();
		GCmdBuf << "HideConsole\n";
	}

	if (cls.signon != SIGNONS)
		Sys_Error("Update when at %d", cls.signon);

	R_AnimateSurfaces();
	msg << new_time;
	GClLevel->TicTime = int(new_time * 35);
	GClLevel->Time = new_time;
	cl->WorldTimer = GClLevel->TicTime;
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

	VName MapName = *msg.ReadString();

	cl->ClientNum = msg.ReadByte();
	GClGame->maxclients = msg.ReadByte();
	GClGame->deathmatch = msg.ReadByte();

	const mapInfo_t& LInfo = P_GetMapInfo(MapName);
	GCon->Log("---------------------------------------");
	GCon->Log(LInfo.GetName());
	GCon->Log("");
	C_ClearNotify();

	CL_LoadLevel(MapName);

	((VLevelChannel*)cl->Net->Channels[CHANIDX_Level])->SetLevel(GClLevel);

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

	const mapInfo_t& linfo = P_GetMapInfo(GClLevel->MapName);
	im.LeaveMap = GClLevel->MapName;
	im.LeaveCluster = linfo.Cluster;
	im.LeaveName = linfo.GetName();
	im.LeaveTitlePatch = linfo.TitlePatch;

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

	im.Time = GClLevel->Time;
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
	/*vint32 i = */msg.ReadShort();
	VStr Name = msg.ReadString();
//	ClassLookup[i] = VClass::FindClass(*Name);
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
//	CL_ReadFromUserInfo
//
//==========================================================================

static void CL_ReadFromUserInfo(int)
{
}

//==========================================================================
//
//	VClientGenChannel::ParsePacket
//
//==========================================================================

void VClientGenChannel::ParsePacket(VMessageIn& msg)
{
	guard(VClientGenChannel::ParsePacket);
	int			i;
	byte		cmd_type;
	VStr		name;
	VStr		string;
	TVec		origin;
	float		radius;
	vuint32		colour;

//	cl->last_received_message = realtime;

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
			// nop keepalive message
			GCon->Log("<-- server to client keepalive");
			break;

		case svc_disconnect:
			Host_EndGame("Server disconnected");

		case svc_server_info:
			CL_ParseServerInfo(msg);
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

		case svc_change_music:
			GClLevel->LevelInfo->SongLump = *msg.ReadString();
			GClLevel->LevelInfo->CDTrack = msg.ReadByte();
			GAudio->MusicChanged();
			break;

		case svc_class_name:
			CL_ParseClassName(msg);
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

//==========================================================================
//
//	VClientNetContext::GetLevel
//
//==========================================================================

VLevel* VClientNetContext::GetLevel()
{
	return GClLevel;
}

//==========================================================================
//
//	VClientNetContext::CreateGenChannel
//
//==========================================================================

VChannel* VClientNetContext::CreateGenChannel(VNetConnection* Connection)
{
	return new VClientGenChannel(Connection, 0);
}
