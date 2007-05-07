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
#include "progdefs.h"
#include "sv_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern bool			sv_loading;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	EvalCond
//
//==========================================================================

static bool EvalCond(VObject* Obj, VClass* Class, VMethod* M)
{
	guard(EvalCond);
	for (int i = 0; i < Class->RepInfos.Num(); i++)
	{
		for (int j = 0; j < Class->RepInfos[i].RepMembers.Num(); j++)
		{
			if (Class->RepInfos[i].RepMembers[j]->MemberType != MEMBER_Method)
				continue;
			if (Class->RepInfos[i].RepMembers[j]->Name != M->Name)
				continue;
			P_PASS_REF(Obj);
			return !!VObject::ExecuteFunction(Class->RepInfos[i].Cond).i;
		}
	}
	if (Class->GetSuperClass())
	{
		return EvalCond(Obj, Class->GetSuperClass(), M);
	}
	return false;
	unguard;
}

//==========================================================================
//
//	VBasePlayer::ExecuteNetMethod
//
//==========================================================================

bool VBasePlayer::ExecuteNetMethod(VMethod* Func)
{
	guard(VBasePlayer::ExecuteNetMethod);
	if (host_standalone)
	{
		return false;
	}

	if (!EvalCond(this, GetClass(), Func))
	{
		return false;
	}

	VMessageOut Msg(Net->Channels[CHANIDX_Player]);
	Msg.bReliable = !!(Func->Flags & FUNC_NetReliable);

	Msg.WriteInt(Func->NetIndex, GetClass()->NumNetFields);

	//	Serialise arguments
	guard(SerialiseArguments);
	VStack* Param = pr_stackPtr - Func->ParamsSize + 1;	//	Skip self
	for (int i = 0; i < Func->NumParams; i++)
	{
		switch (Func->ParamTypes[i].Type)
		{
		case ev_int:
		case ev_name:
		case ev_bool:
			VField::NetSerialiseValue(Msg, (vuint8*)&Param->i, Func->ParamTypes[i]);
			Param++;
			break;
		case ev_float:
			VField::NetSerialiseValue(Msg, (vuint8*)&Param->f, Func->ParamTypes[i]);
			Param++;
			break;
		case ev_string:
		case ev_pointer:
		case ev_reference:
		case ev_class:
		case ev_state:
			VField::NetSerialiseValue(Msg, (vuint8*)&Param->p, Func->ParamTypes[i]);
			Param++;
			break;
		case ev_vector:
			{
				TVec Vec;
				Vec.x = Param[0].f;
				Vec.y = Param[1].f;
				Vec.z = Param[2].f;
				VField::NetSerialiseValue(Msg, (vuint8*)&Vec, Func->ParamTypes[i]);
				Param += 3;
			}
			break;
		default:
			Sys_Error("Bad method argument type %d", Func->ParamTypes[i].Type);
		}
	}
	unguard;

	//	Send it.
	Net->Channels[CHANIDX_Player]->SendMessage(&Msg);

	//	Clean up parameters
	guard(CleanUp);
	VStack* Param = pr_stackPtr - Func->ParamsSize + 1;	//	Skip self
	for (int i = 0; i < Func->NumParams; i++)
	{
		switch (Func->ParamTypes[i].Type)
		{
		case ev_int:
		case ev_name:
		case ev_bool:
		case ev_float:
		case ev_pointer:
		case ev_reference:
		case ev_class:
		case ev_state:
			Param++;
			break;
		case ev_string:
			((VStr*)&Param->p)->Clean();
			Param++;
			break;
		case ev_vector:
			Param += 3;
			break;
		default:
			Sys_Error("Bad method argument type %d", Func->ParamTypes[i].Type);
		}
	}
	pr_stackPtr -= Func->ParamsSize;
	unguard;

	//	Push null return value
	guard(RetVal);
	switch (Func->ReturnType.Type)
	{
	case ev_void:
		break;
	case ev_int:
	case ev_name:
	case ev_bool:
		PR_Push(0);
		break;
	case ev_float:
		PR_Pushf(0);
		break;
	case ev_string:
		PR_PushStr(VStr());
		break;
	case ev_pointer:
	case ev_reference:
	case ev_class:
	case ev_state:
		PR_PushPtr(NULL);
		break;
	case ev_vector:
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

//==========================================================================
//
//	SV_ReadFromUserInfo
//
//==========================================================================

void SV_ReadFromUserInfo(VBasePlayer* Player)
{
	guard(SV_ReadFromUserInfo);
	if (!sv_loading)
	{
		Player->BaseClass = atoi(*Info_ValueForKey(Player->UserInfo, "class"));
	}
	Player->PlayerName = Info_ValueForKey(Player->UserInfo, "name");
	Player->Colour = atoi(*Info_ValueForKey(Player->UserInfo, "colour"));
	Player->eventUserinfoChanged();
	unguard;
}

//==========================================================================
//
//	SV_SetUserInfo
//
//==========================================================================

void SV_SetUserInfo(VBasePlayer* Player, const VStr& info)
{
	guard(SV_SetUserInfo);
	if (!sv_loading)
	{
		Player->UserInfo = info;
		SV_ReadFromUserInfo(Player);
	}
	unguard;
}

//==========================================================================
//
//	VServerNetContext::GetLevel
//
//==========================================================================

VLevel* VServerNetContext::GetLevel()
{
	return GLevel;
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
	SV_ReadFromUserInfo(Player);
	unguard;
}

//==========================================================================
//
//	Natives.
//
//==========================================================================

IMPLEMENT_FUNCTION(VBasePlayer, cprint)
{
	VStr msg = PF_FormatString();
	P_GET_SELF;
	Self->Printf(*msg);
}

IMPLEMENT_FUNCTION(VBasePlayer, centreprint)
{
	VStr msg = PF_FormatString();
	P_GET_SELF;
	Self->CentrePrintf(*msg);
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
	Self->Items = 0;
	Self->PlayerFlags &= ~VBasePlayer::PF_AttackDown;
	Self->PlayerFlags &= ~VBasePlayer::PF_UseDown;
	Self->ExtraLight = 0;
	Self->FixedColourmap = 0;
	memset(Self->CShifts, 0, sizeof(Self->CShifts));
	Self->PSpriteSY = 0;

	vuint8* Def = Self->GetClass()->Defaults;
	for (VField* F = Self->GetClass()->Fields; F; F = F->Next)
	{
		VField::CopyFieldValue(Def + F->Ofs, (vuint8*)Self + F->Ofs, F->Type);
	}
}

IMPLEMENT_FUNCTION(VBasePlayer, ServerSetUserInfo)
{
	P_GET_STR(Info);
	P_GET_SELF;
	SV_SetUserInfo(Self, Info);
}
