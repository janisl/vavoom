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
void CL_SetupLevel();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	CL_Clear
//
//==========================================================================

void CL_Clear()
{
	guard(CL_Clear);
	GClGame->serverinfo.Clean();
	GClGame->intermission = 0;
	GClGame->time = 0;
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

//==========================================================================
//
//	CL_ReadFromServerInfo
//
//==========================================================================

static void CL_ReadFromServerInfo()
{
	guard(CL_ReadFromServerInfo);
	VCvar::SetCheating(!!atoi(*Info_ValueForKey(GClGame->serverinfo, "sv_cheats")));
	unguard;
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientStartSound)
{
	P_GET_FLOAT(Attenuation);
	P_GET_FLOAT(Volume);
	P_GET_INT(Channel);
	P_GET_INT(OriginId);
	P_GET_VEC(Org);
	P_GET_INT(SoundId);
	P_GET_SELF;
	GAudio->PlaySound(SoundId, Org, TVec(0, 0, 0), OriginId, Channel, Volume,
		Attenuation);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientStopSound)
{
	P_GET_INT(Channel);
	P_GET_INT(OriginId);
	P_GET_SELF;
	GAudio->StopSound(OriginId, Channel);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientStartSequence)
{
	P_GET_INT(ModeNum);
	P_GET_NAME(Name);
	P_GET_INT(OriginId);
	P_GET_VEC(Origin);
	P_GET_SELF;
	GAudio->StartSequence(OriginId, Origin, Name, ModeNum);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientAddSequenceChoice)
{
	P_GET_NAME(Choice);
	P_GET_INT(OriginId);
	P_GET_SELF;
	GAudio->AddSeqChoice(OriginId, Choice);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientStopSequence)
{
	P_GET_INT(OriginId);
	P_GET_SELF;
	GAudio->StopSequence(OriginId);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientForceLightning)
{
	P_GET_SELF;
	GClLevel->RenderData->ForceLightning();
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientPrint)
{
	P_GET_STR(Str);
	P_GET_SELF;
	C_NotifyMessage(*Str);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientCentrePrint)
{
	P_GET_STR(Str);
	P_GET_SELF;
	C_CentreMessage(*Str);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientSetAngles)
{
	P_GET_AVEC(Angles);
	P_GET_SELF;
	Self->ViewAngles = Angles;
	Self->ViewAngles.pitch = AngleMod180(Self->ViewAngles.pitch);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientIntermission)
{
	P_GET_NAME(NextMap);
	P_GET_SELF;

	im.Text.Clean();
	im.IMFlags = 0;

	const mapInfo_t& linfo = P_GetMapInfo(GClLevel->MapName);
	im.LeaveMap = GClLevel->MapName;
	im.LeaveCluster = linfo.Cluster;
	im.LeaveName = linfo.GetName();
	im.LeaveTitlePatch = linfo.TitlePatch;

	const mapInfo_t& einfo = P_GetMapInfo(NextMap);
	im.EnterMap = NextMap;
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

	IM_Start();
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientPause)
{
	P_GET_BOOL(Paused);
	P_GET_SELF;
	if (Paused)
	{
		GClGame->ClientFlags |= VClientGameBase::CF_Paused;
		GAudio->PauseSound();
	}
	else
	{
		GClGame->ClientFlags &= ~VClientGameBase::CF_Paused;
		GAudio->ResumeSound();
	}
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientSkipIntermission)
{
	P_GET_SELF;
	IM_SkipIntermission();
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientFinale)
{
	P_GET_STR(Type);
	P_GET_SELF;
	F_StartFinale(*Type);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientChangeMusic)
{
	P_GET_INT(CDTrack);
	P_GET_NAME(Song);
	P_GET_SELF;
	GClLevel->LevelInfo->SongLump = Song;
	GClLevel->LevelInfo->CDTrack = CDTrack;
	GAudio->MusicChanged();
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientSetServerInfo)
{
	P_GET_STR(Value);
	P_GET_STR(Key);
	P_GET_SELF;
	Info_SetValueForKey(GClGame->serverinfo, Key, Value);
	CL_ReadFromServerInfo();
}

//==========================================================================
//
//	CL_DoLoadLevel
//
//==========================================================================

void CL_ParseServerInfo(VMessageIn& msg)
{
	guard(CL_ParseServerInfo);
	CL_Clear();

	msg << GClGame->serverinfo;
	CL_ReadFromServerInfo();

	VName MapName = *msg.ReadString();

	GClGame->maxclients = msg.ReadInt(MAXPLAYERS + 1);
	GClGame->deathmatch = msg.ReadInt(256);

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

	GCon->Log(NAME_Dev, "Client level loaded");
	unguard;
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
		case svc_stringcmd:
			GCmdBuf << msg.ReadString();
			break;

		default:
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
