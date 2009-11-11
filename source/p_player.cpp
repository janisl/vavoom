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
#include "sv_local.h"
#include "cl_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

IMPLEMENT_CLASS(V, BasePlayer)

static VCvarF			notify_time("notify_time", "5", CVAR_Archive);
static VCvarF			centre_msg_time("centre_message_time", "7", CVAR_Archive);
static VCvarI			msg_echo("msg_echo", "1", CVAR_Archive);
static VCvarI			font_colour("font_colour", "11", CVAR_Archive);
static VCvarI			font_colour2("font_colour2", "11", CVAR_Archive);

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	VBasePlayer::ExecuteNetMethod
//
//==========================================================================

bool VBasePlayer::ExecuteNetMethod(VMethod* Func)
{
	guard(VBasePlayer::ExecuteNetMethod);
	if (GDemoRecordingContext)
	{
		//	Find initial version of the method.
		VMethod* Base = Func;
		while (Base->SuperMethod)
		{
			Base = Base->SuperMethod;
		}
		//	Execute it's replication condition method.
		check(Base->ReplCond);
		P_PASS_REF(this);
		vuint32 SavedFlags = PlayerFlags;
		PlayerFlags &= ~VBasePlayer::PF_IsClient;
		bool ShouldSend = VObject::ExecuteFunction(Base->ReplCond).i;
		PlayerFlags = SavedFlags;

		if (ShouldSend)
		{
			//	Replication condition is true, the method must be replicated.
			GDemoRecordingContext->ClientConnections[0]->Channels[
				CHANIDX_Player]->SendRpc(Func, this);
		}
	}

#ifdef CLIENT
	if (GGameInfo->NetMode == NM_TitleMap ||
		GGameInfo->NetMode == NM_Standalone ||
		(GGameInfo->NetMode == NM_ListenServer && this == cl))
	{
		return false;
	}
#endif

	//	Find initial version of the method.
	VMethod* Base = Func;
	while (Base->SuperMethod)
	{
		Base = Base->SuperMethod;
	}
	//	Execute it's replication condition method.
	check(Base->ReplCond);
	P_PASS_REF(this);
	if (!VObject::ExecuteFunction(Base->ReplCond).i)
	{
		return false;
	}

	if (Net)
	{
		//	Replication condition is true, the method must be replicated.
		Net->Channels[CHANIDX_Player]->SendRpc(Func, this);
	}

	//	Clean up parameters
	guard(CleanUp);
	VStack* Param = pr_stackPtr - Func->ParamsSize + 1;	//	Skip self
	for (int i = 0; i < Func->NumParams; i++)
	{
		switch (Func->ParamTypes[i].Type)
		{
		case TYPE_Int:
		case TYPE_Byte:
		case TYPE_Bool:
		case TYPE_Float:
		case TYPE_Name:
		case TYPE_Pointer:
		case TYPE_Reference:
		case TYPE_Class:
		case TYPE_State:
			Param++;
			break;
		case TYPE_String:
			((VStr*)&Param->p)->Clean();
			Param++;
			break;
		case TYPE_Vector:
			Param += 3;
			break;
		default:
			Sys_Error("Bad method argument type %d", Func->ParamTypes[i].Type);
		}
		if (Func->ParamFlags[i] & FPARM_Optional)
		{
			Param++;
		}
	}
	pr_stackPtr -= Func->ParamsSize;
	unguard;

	//	Push null return value
	guard(RetVal);
	switch (Func->ReturnType.Type)
	{
	case TYPE_Void:
		break;
	case TYPE_Int:
	case TYPE_Byte:
	case TYPE_Bool:
	case TYPE_Name:
		PR_Push(0);
		break;
	case TYPE_Float:
		PR_Pushf(0);
		break;
	case TYPE_String:
		PR_PushStr(VStr());
		break;
	case TYPE_Pointer:
	case TYPE_Reference:
	case TYPE_Class:
	case TYPE_State:
		PR_PushPtr(NULL);
		break;
	case TYPE_Vector:
		PR_Pushf(0);
		PR_Pushf(0);
		PR_Pushf(0);
		break;
	default:
		Sys_Error("Bad return value type");
	}
	unguard;

	//	It's been handled here.
	return true;
	unguard;
}

//==========================================================================
//
//	VBasePlayer::SpawnClient
//
//==========================================================================

void VBasePlayer::SpawnClient()
{
	guard(VBasePlayer::SpawnClient);
	if (!sv_loading)
	{
		if (PlayerFlags & PF_Spawned)
		{
			GCon->Log(NAME_Dev, "Already spawned");
		}
		if (MO)
		{
			GCon->Log(NAME_Dev, "Mobj already spawned");
		}
		eventSpawnClient();
		for (int i = 0; i < Level->XLevel->ActiveSequences.Num(); i++)
		{
			eventClientStartSequence(
				Level->XLevel->ActiveSequences[i].Origin,
				Level->XLevel->ActiveSequences[i].OriginId,
				Level->XLevel->ActiveSequences[i].Name,
				Level->XLevel->ActiveSequences[i].ModeNum);
			for (int j = 0; j < Level->XLevel->ActiveSequences[i].Choices.Num(); j++)
			{
				eventClientAddSequenceChoice(
					Level->XLevel->ActiveSequences[i].OriginId,
					Level->XLevel->ActiveSequences[i].Choices[j]);
			}
		}
	}
	else
	{
		if (!MO)
		{
			Host_Error("Player without Mobj\n");
		}
	}

	ViewAngles.roll = 0;
	eventClientSetAngles(ViewAngles);
	PlayerFlags &= ~PF_FixAngle;

	PlayerFlags |= PF_Spawned;

	if ((GGameInfo->NetMode == NM_TitleMap ||
		GGameInfo->NetMode == NM_Standalone) && run_open_scripts)
	{
		//	Start open scripts.
		Level->XLevel->Acs->StartTypedACScripts(SCRIPT_Open, 0, 0, 0, NULL,
			false, false);
	}

	if (!sv_loading)
	{
		Level->XLevel->Acs->StartTypedACScripts(SCRIPT_Enter, 0, 0, 0, MO,
			true, false);
	}
	else if (sv_map_travel)
	{
		Level->XLevel->Acs->StartTypedACScripts(SCRIPT_Return, 0, 0, 0, MO,
			true, false);
	}

	if (GGameInfo->NetMode < NM_DedicatedServer ||
		svs.num_connected == sv_load_num_players)
	{
		sv_loading = false;
		sv_map_travel = false;
	}

	// For single play, save immediately into the reborn slot
	if (GGameInfo->NetMode < NM_DedicatedServer)
	{
		SV_SaveGame(SV_GetRebornSlot(), REBORN_DESCRIPTION);
	}
	unguard;
}

//==========================================================================
//
//	VBasePlayer::Printf
//
//==========================================================================

void VBasePlayer::Printf(const char *s, ...)
{
	guard(VBasePlayer::Printf);
	va_list	v;
	char	buf[1024];

	va_start(v, s);
	vsprintf(buf, s, v);
	va_end(v);

	eventClientPrint(buf);
	unguard;
}

//==========================================================================
//
//	VBasePlayer::CentrePrintf
//
//==========================================================================

void VBasePlayer::CentrePrintf(const char *s, ...)
{
	guard(VBasePlayer::CentrePrintf);
	va_list	v;
	char	buf[1024];

	va_start(v, s);
	vsprintf(buf, s, v);
	va_end(v);

	eventClientCentrePrint(buf);
	unguard;
}

//===========================================================================
//
//  VBasePlayer::SetViewState
//
//===========================================================================

void VBasePlayer::SetViewState(int position, VState* stnum)
{
	guard(VBasePlayer::SetViewState);
	VViewState& VSt = ViewStates[position];
	VState *state = stnum;
	do
	{
		if (!state)
		{
			// Object removed itself.
			VSt.State = NULL;
			VSt.StateTime = -1;
			break;
		}
		VSt.State = state;
		VSt.StateTime = state->Time;	// could be 0
		if (state->Misc1)
		{
			VSt.SX = state->Misc1;
		}
		if (state->Misc2)
		{
			VSt.SY = state->Misc2;
		}
		// Call action routine.
		if (state->Function)
		{
			Level->XLevel->CallingState = state;
			P_PASS_REF(MO);
			ExecuteFunction(state->Function);
			if (!VSt.State)
			{
				break;
			}
		}
		state = VSt.State->NextState;
	}
	while (!VSt.StateTime);	// An initial state of 0 could cycle through.
	unguard;
}

//==========================================================================
//
//	VBasePlayer::AdvanceViewStates
//
//==========================================================================

void VBasePlayer::AdvanceViewStates(float deltaTime)
{
	for (int i = 0; i < NUMPSPRITES; i++)
	{
		VViewState& St = ViewStates[i];
		// a null state means not active
		if (St.State)
		{
			// drop tic count and possibly change state
			// a -1 tic count never changes
			if (St.StateTime != -1.0)
			{
				St.StateTime -= deltaTime;
				if (St.StateTime <= 0.0)
				{
					St.StateTime = 0.0;
					SetViewState(i, St.State->NextState);
				}
			}
		}
	}
}

//==========================================================================
//
//	VBasePlayer::SetUserInfo
//
//==========================================================================

void VBasePlayer::SetUserInfo(const VStr& info)
{
	guard(VBasePlayer::SetUserInfo);
	if (!sv_loading)
	{
		UserInfo = info;
		ReadFromUserInfo();
	}
	unguard;
}

//==========================================================================
//
//	VBasePlayer::ReadFromUserInfo
//
//==========================================================================

void VBasePlayer::ReadFromUserInfo()
{
	guard(VBasePlayer::ReadFromUserInfo);
	if (!sv_loading)
	{
		BaseClass = atoi(*Info_ValueForKey(UserInfo, "class"));
	}
	PlayerName = Info_ValueForKey(UserInfo, "name");
	Colour = M_ParseColour(Info_ValueForKey(UserInfo, "colour"));
	eventUserinfoChanged();
	unguard;
}

//==========================================================================
//
//	VBasePlayer::DoClientStartSound
//
//==========================================================================

void VBasePlayer::DoClientStartSound(int SoundId, TVec Org, int OriginId,
	int Channel, float Volume, float Attenuation, bool Loop)
{
#ifdef CLIENT
	guard(VBasePlayer::DoClientStartSound);
	GAudio->PlaySound(SoundId, Org, TVec(0, 0, 0), OriginId, Channel, Volume,
		Attenuation, Loop);
	unguard;
#endif
}

//==========================================================================
//
//	VBasePlayer::DoClientStopSound
//
//==========================================================================

void VBasePlayer::DoClientStopSound(int OriginId, int Channel)
{
#ifdef CLIENT
	guard(VBasePlayer::DoClientStopSound);
	GAudio->StopSound(OriginId, Channel);
	unguard;
#endif
}

//==========================================================================
//
//	VBasePlayer::DoClientStartSequence
//
//==========================================================================

void VBasePlayer::DoClientStartSequence(TVec Origin, int OriginId, VName Name,
	int ModeNum)
{
#ifdef CLIENT
	guard(VBasePlayer::DoClientStartSequence);
	GAudio->StartSequence(OriginId, Origin, Name, ModeNum);
	unguard;
#endif
}

//==========================================================================
//
//	VBasePlayer::DoClientAddSequenceChoice
//
//==========================================================================

void VBasePlayer::DoClientAddSequenceChoice(int OriginId, VName Choice)
{
#ifdef CLIENT
	guard(VBasePlayer::DoClientAddSequenceChoice);
	GAudio->AddSeqChoice(OriginId, Choice);
	unguard;
#endif
}

//==========================================================================
//
//	VBasePlayer::DoClientStopSequence
//
//==========================================================================

void VBasePlayer::DoClientStopSequence(int OriginId)
{
#ifdef CLIENT
	guard(VBasePlayer::DoClientStopSequence);
	GAudio->StopSequence(OriginId);
	unguard;
#endif
}

//==========================================================================
//
//	VBasePlayer::DoClientPrint
//
//==========================================================================

void VBasePlayer::DoClientPrint(VStr AStr)
{
	guard(VBasePlayer::DoClientPrint);
	VStr Str(AStr);

	if (Str.IsEmpty())
	{
		return;
	}

	if (Str[0] == '$')
	{
		Str = GLanguage[*VStr(Str.ToLower(), 1, Str.Length() - 1)];
	}

	if (msg_echo)
	{
		GCon->Log(Str);
	}

	ClGame->eventAddNotifyMessage(Str);
	unguard;
}

//==========================================================================
//
//	VBasePlayer::DoClientCentrePrint
//
//==========================================================================

void VBasePlayer::DoClientCentrePrint(VStr Str)
{
	guard(VBasePlayer::DoClientCentrePrint);
	VStr Msg(Str);

	if (Msg.IsEmpty())
	{
		return;
	}

	if (Msg[0] == '$')
	{
		Msg = GLanguage[*VStr(Msg.ToLower(), 1, Msg.Length() - 1)];
	}

	if (msg_echo)
	{
		GCon->Log("<-------------------------------->");
		GCon->Log(Msg);
		GCon->Log("<-------------------------------->");
	}

	ClGame->eventAddCentreMessage(Msg);
	unguard;
}

//==========================================================================
//
//	VBasePlayer::DoClientSetAngles
//
//==========================================================================

void VBasePlayer::DoClientSetAngles(TAVec Angles)
{
	guard(VBasePlayer::DoClientSetAngles);
	ViewAngles = Angles;
	ViewAngles.pitch = AngleMod180(ViewAngles.pitch);
	unguard;
}

//==========================================================================
//
//	VBasePlayer::DoClientIntermission
//
//==========================================================================

void VBasePlayer::DoClientIntermission(VName NextMap)
{
	guard(VBasePlayer::DoClientIntermission);
	im_t& im = ClGame->im;

	im.Text.Clean();
	im.IMFlags = 0;

	const mapInfo_t& linfo = P_GetMapInfo(Level->XLevel->MapName);
	im.LeaveMap = Level->XLevel->MapName;
	im.LeaveCluster = linfo.Cluster;
	im.LeaveName = linfo.GetName();
	im.LeaveTitlePatch = linfo.TitlePatch;
	im.ExitPic = linfo.ExitPic;
	im.InterMusic = linfo.InterMusic;

	const mapInfo_t& einfo = P_GetMapInfo(NextMap);
	im.EnterMap = NextMap;
	im.EnterCluster = einfo.Cluster;
	im.EnterName = einfo.GetName();
	im.EnterTitlePatch = einfo.TitlePatch;
	im.EnterPic = einfo.EnterPic;

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

	ClGame->intermission = 1;
#ifdef CLIENT
	AM_Stop();
	GAudio->StopAllSequences();
#endif

	ClGame->eventIintermissionStart();
	unguard;
}

//==========================================================================
//
//	VBasePlayer::DoClientPause
//
//==========================================================================

void VBasePlayer::DoClientPause(bool Paused)
{
#ifdef CLIENT
	guard(VBasePlayer::DoClientPause);
	if (Paused)
	{
		ClGame->ClientFlags |= VClientGameBase::CF_Paused;
		GAudio->PauseSound();
	}
	else
	{
		ClGame->ClientFlags &= ~VClientGameBase::CF_Paused;
		GAudio->ResumeSound();
	}
	unguard;
#endif
}

//==========================================================================
//
//	VBasePlayer::DoClientSkipIntermission
//
//==========================================================================

void VBasePlayer::DoClientSkipIntermission()
{
	guard(VBasePlayer::DoClientSkipIntermission);
	ClGame->ClientFlags |= VClientGameBase::CF_SkipIntermission;
	unguard;
}

//==========================================================================
//
//	VBasePlayer::DoClientFinale
//
//==========================================================================

void VBasePlayer::DoClientFinale(VStr Type)
{
	guard(VBasePlayer::DoClientFinale);
	ClGame->intermission = 2;
#ifdef CLIENT
	AM_Stop();
#endif

	ClGame->eventStartFinale(*Type);
	unguard;
}

//==========================================================================
//
//	VBasePlayer::DoClientChangeMusic
//
//==========================================================================

void VBasePlayer::DoClientChangeMusic(VName Song, int CDTrack)
{
	guard(VBasePlayer::DoClientChangeMusic);
	Level->SongLump = Song;
	Level->CDTrack = CDTrack;
#ifdef CLIENT
	GAudio->MusicChanged();
#endif
	unguard;
}

//==========================================================================
//
//	VBasePlayer::DoClientSetServerInfo
//
//==========================================================================

void VBasePlayer::DoClientSetServerInfo(VStr Key, VStr Value)
{
	guard(VBasePlayer::DoClientSetServerInfo);
	Info_SetValueForKey(ClGame->serverinfo, Key, Value);
#ifdef CLIENT
	CL_ReadFromServerInfo();
#endif
	unguard;
}

//==========================================================================
//
//	VBasePlayer::DoClientHudMessage
//
//==========================================================================

void VBasePlayer::DoClientHudMessage(const VStr& Message, VName Font, int Type,
	int Id, int Colour, const VStr& ColourName, float x, float y,
	int HudWidth, int HudHeight, float HoldTime, float Time1, float Time2)
{
	guard(VBasePlayer::DoClientHudMessage);
	ClGame->eventAddHudMessage(Message, Font, Type, Id, Colour, ColourName,
		x, y, HudWidth, HudHeight, HoldTime, Time1, Time2);
	unguard;
}

//==========================================================================
//
//	VBasePlayer::WriteViewData
//
//==========================================================================

void VBasePlayer::WriteViewData()
{
	guard(VBasePlayer::WriteViewData);
	//	Update bam_angles (after teleportation)
	if (PlayerFlags & PF_FixAngle)
	{
		PlayerFlags &= ~PF_FixAngle;
		eventClientSetAngles(ViewAngles);
	}
	unguard;
}

//==========================================================================
//
//	COMMAND SetInfo
//
//==========================================================================

COMMAND(SetInfo)
{
	guard(COMMAND SetInfo);
	if (Source != SRC_Client)
	{
		GCon->Log("SetInfo is not valid from console");
		return;
	}

	if (Args.Num() != 3)
	{
		return;
	}

	Info_SetValueForKey(Player->UserInfo, *Args[1], *Args[2]);
	Player->ReadFromUserInfo();
	unguard;
}

//==========================================================================
//
//	Natives
//
//==========================================================================

IMPLEMENT_FUNCTION(VBasePlayer, cprint)
{
	VStr msg = PF_FormatString();
	P_GET_SELF;
	Self->eventClientPrint(*msg);
}

IMPLEMENT_FUNCTION(VBasePlayer, centreprint)
{
	VStr msg = PF_FormatString();
	P_GET_SELF;
	Self->eventClientCentrePrint(*msg);
}

IMPLEMENT_FUNCTION(VBasePlayer, GetPlayerNum)
{
	P_GET_SELF;
	RET_INT(SV_GetPlayerNum(Self));
}

IMPLEMENT_FUNCTION(VBasePlayer, ClearPlayer)
{
	P_GET_SELF;

	Self->PClass = 0;
	Self->ForwardMove = 0;
	Self->SideMove = 0;
	Self->FlyMove = 0;
	Self->Buttons = 0;
	Self->Impulse = 0;
	Self->MO = NULL;
	Self->PlayerState = 0;
	Self->ViewOrg = TVec(0, 0, 0);
	Self->PlayerFlags &= ~VBasePlayer::PF_FixAngle;
	Self->Health = 0;
	Self->PlayerFlags &= ~VBasePlayer::PF_AttackDown;
	Self->PlayerFlags &= ~VBasePlayer::PF_UseDown;
	Self->PlayerFlags &= ~VBasePlayer::PF_AutomapRevealed;
	Self->PlayerFlags &= ~VBasePlayer::PF_AutomapShowThings;
	Self->ExtraLight = 0;
	Self->FixedColourmap = 0;
	Self->CShift = 0;
	Self->PSpriteSY = 0;

	vuint8* Def = Self->GetClass()->Defaults;
	for (VField* F = Self->GetClass()->Fields; F; F = F->Next)
	{
		VField::CopyFieldValue(Def + F->Ofs, (vuint8*)Self + F->Ofs, F->Type);
	}
}

IMPLEMENT_FUNCTION(VBasePlayer, SetViewState)
{
	P_GET_PTR(VState, stnum);
	P_GET_INT(position);
	P_GET_SELF;
	Self->SetViewState(position, stnum);
}

IMPLEMENT_FUNCTION(VBasePlayer, AdvanceViewStates)
{
	P_GET_FLOAT(deltaTime);
	P_GET_SELF;
	Self->AdvanceViewStates(deltaTime);
}

IMPLEMENT_FUNCTION(VBasePlayer, DisconnectBot)
{
	P_GET_SELF;
	check(Self->PlayerFlags & PF_IsBot);
	SV_DropClient(Self, false);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientStartSound)
{
	P_GET_BOOL(Loop);
	P_GET_FLOAT(Attenuation);
	P_GET_FLOAT(Volume);
	P_GET_INT(Channel);
	P_GET_INT(OriginId);
	P_GET_VEC(Org);
	P_GET_INT(SoundId);
	P_GET_SELF;
	Self->DoClientStartSound(SoundId, Org, OriginId, Channel, Volume,
		Attenuation, Loop);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientStopSound)
{
	P_GET_INT(Channel);
	P_GET_INT(OriginId);
	P_GET_SELF;
	Self->DoClientStopSound(OriginId, Channel);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientStartSequence)
{
	P_GET_INT(ModeNum);
	P_GET_NAME(Name);
	P_GET_INT(OriginId);
	P_GET_VEC(Origin);
	P_GET_SELF;
	Self->DoClientStartSequence(Origin, OriginId, Name, ModeNum);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientAddSequenceChoice)
{
	P_GET_NAME(Choice);
	P_GET_INT(OriginId);
	P_GET_SELF;
	Self->DoClientAddSequenceChoice(OriginId, Choice);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientStopSequence)
{
	P_GET_INT(OriginId);
	P_GET_SELF;
	Self->DoClientStopSequence(OriginId);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientPrint)
{
	P_GET_STR(Str);
	P_GET_SELF;
	Self->DoClientPrint(Str);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientCentrePrint)
{
	P_GET_STR(Str);
	P_GET_SELF;
	Self->DoClientCentrePrint(Str);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientSetAngles)
{
	P_GET_AVEC(Angles);
	P_GET_SELF;
	Self->DoClientSetAngles(Angles);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientIntermission)
{
	P_GET_NAME(NextMap);
	P_GET_SELF;
	Self->DoClientIntermission(NextMap);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientPause)
{
	P_GET_BOOL(Paused);
	P_GET_SELF;
	Self->DoClientPause(Paused);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientSkipIntermission)
{
	P_GET_SELF;
	Self->DoClientSkipIntermission();
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientFinale)
{
	P_GET_STR(Type);
	P_GET_SELF;
	Self->DoClientFinale(Type);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientChangeMusic)
{
	P_GET_INT(CDTrack);
	P_GET_NAME(Song);
	P_GET_SELF;
	Self->DoClientChangeMusic(Song, CDTrack);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientSetServerInfo)
{
	P_GET_STR(Value);
	P_GET_STR(Key);
	P_GET_SELF;
	Self->DoClientSetServerInfo(Key, Value);
}

IMPLEMENT_FUNCTION(VBasePlayer, ClientHudMessage)
{
	P_GET_FLOAT(Time2);
	P_GET_FLOAT(Time1);
	P_GET_FLOAT(HoldTime);
	P_GET_INT(HudHeight);
	P_GET_INT(HudWidth);
	P_GET_FLOAT(y);
	P_GET_FLOAT(x);
	P_GET_STR(ColourName);
	P_GET_INT(Colour);
	P_GET_INT(Id);
	P_GET_INT(Type);
	P_GET_NAME(Font);
	P_GET_STR(Message);
	P_GET_SELF;
	Self->DoClientHudMessage(Message, Font, Type, Id, Colour, ColourName,
		x, y, HudWidth, HudHeight, HoldTime, Time1, Time2);
}

IMPLEMENT_FUNCTION(VBasePlayer, ServerSetUserInfo)
{
	P_GET_STR(Info);
	P_GET_SELF;
	Self->SetUserInfo(Info);
}
